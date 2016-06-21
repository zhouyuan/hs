//g++ test_CacheService.cpp ../Messenger/Messenger.cpp ../DataStore/BlockCacher/SimpleBlockCacher.cpp ../common/BufferList.cpp -o test -std=c++11 -lboost_thread -lboost_system -lpthread -lmemcached
#include "rbc/CacheService.h"

namespace rbc {

CacheService::CacheService(){
    log_print("Create new CacheService instance\n");
    this->cct = new Context();
    int tp_size = stoi(this->cct->config->configValues["cacheservice_threads_num"]);
    threadpool = new ThreadPool(tp_size);
    process_tp = new ThreadPool(1);
    process_tp->schedule(boost::bind(&CacheService::_process, this));
    agentservice = new AgentService( this->cct );
    log_print("CacheService constructed\n");
}

CacheService::~CacheService(){
    log_print("enter CacheService destruction\n");
    cct->go = false;
    delete threadpool;
    delete process_tp;
    delete agentservice;
    delete this->cct;
}

void CacheService::start(){
}

int CacheService::queue_io( Request* req ){
    this->cct->request_queue.enqueue((void*)req);
    return 0;
}

void CacheService::_process(){
    while(cct->go){
        if( this->cct->request_queue.empty() ){
            std::this_thread::yield();
            continue;
        }
        dequeue_op();
    };
}

void CacheService::dequeue_op(){
    boost::shared_lock<boost::shared_mutex> lock(this->cct->cachemap_access);
    Request *req;
    {
        req = (Request*)this->cct->request_queue.dequeue();
    }while(req == NULL);
    //log_print("dequeue new request:%llu\n", req->msg->header.offset);
    std::list<Op*> op_list = map_request_by_entry( req );
    req->uncomplete_count = op_list.size();
    req->req_status = MSG_SUCCESS;
    for( std::list<Op*>::iterator it = op_list.begin(); it != op_list.end(); ++it ){
        threadpool->schedule(boost::bind(&CacheService::do_op, this, *it));
    }
}

void CacheService::do_op(Op *op){
    //lock cache entry
    bool fail = false;
    std::string oid_string = get_index(op->location_id, op->offset);
    CacheEntry* cache_entry = (CacheEntry*)this->cct->cache_map->get_value( oid_string );
    try{
        cache_entry->entry_lock.lock();
    }catch(...){
        log_print("CacheService::do_op lock c_entry %p failed\n", cache_entry);
        fail = true;
    }
    if( cache_entry == NULL ) fail = true;
    if(fail){
        char status = MSG_FAIL;
        int ret = commit_op( op, &status );
        return;
    }

    cache_entry->name = oid_string;
    cache_entry->location_id = op->location_id;
    cache_entry->offset = op->offset;
    op->cache_entry = cache_entry;
    switch( op->req->msg->header.type ){
        case MSG_READ:
            do_read_op(op);
            break;
        case MSG_WRITE:
            do_write_op(op);
            break;
        default:
            break;
    }
    cache_entry->entry_lock.unlock();
}

std::string CacheService::get_index( const char* location_id, uint64_t offset ){
    std::ostringstream ss;
    uint64_t start_off = offset/this->cct->object_size;
    ss << location_id << "-" << start_off;
    return ss.str();
}

std::list<Op*> CacheService::map_request_by_entry( Request* req ){
    std::list<Op*> op_list;
    uint64_t cur_offset = req->msg->header.offset;
    uint64_t cur_offset_by_req = 0;
    uint64_t remain_length = req->msg->header.length;
    uint64_t max_length = this->cct->object_size;
    uint64_t op_len = 0;

    if(req->msg->header.type == MSG_READ){
        req->msg->content = new char[req->msg->header.length];
    }

    while( remain_length > 0 ){
        if(cur_offset % this->cct->object_size > 0)
            max_length = this->cct->object_size - cur_offset % this->cct->object_size;
        else
            max_length = this->cct->object_size;

        if( remain_length < max_length )
            op_len = remain_length;
        else
            op_len = max_length;
        Op *new_op = new Op(req->msg->header.location_id, cur_offset, &req->msg->content[cur_offset_by_req], op_len, req);
        op_list.push_back(new_op);
        remain_length -= op_len;
        cur_offset += op_len;
        cur_offset_by_req += op_len;
    }
    return op_list;
}

int CacheService::do_read_op(Op *op){
    uint64_t offset = op->offset;
    uint64_t length = op->length;
    std::string oid_string = get_index( op->location_id, op->offset );
    const char* oid = oid_string.c_str();
    char* data = op->data;
    char data_oid[BUFSIZE] = {0};
    int ret = 0;

    //std::cout << "socket: " << op->req->socket_fd << "   oid: " << oid << "    length:" << length << std::endl;
    if( cache_lookup(op, oid, data_oid) ){
        ret = read_hit( op->cache_entry, oid, offset, length, data );
    }else{
        ret = read_miss( op->cache_entry, oid, oid, offset, length, data, op );
    }
    if(ret == 0){
        if(op->cache_entry->if_dirty)
            this->cct->lru_dirty->touch_key( (char*)op->cache_entry );
        else
            this->cct->lru_clean->touch_key( (char*)op->cache_entry );

        ret = commit_op_with_data( op, data );
    }else{
        char status = MSG_FAIL;
        ret = commit_op( op, &status );
    }
    return ret;
}

int CacheService::do_write_op( Op *op ){
    uint64_t offset = op->offset;
    uint64_t length = op->length;
    std::string oid_string = get_index( op->location_id, op->offset );
    const char* oid = oid_string.c_str();
    char* data = op->data;
    char data_oid[BUFSIZE] = {0};
    int ret = 0;

    //std::cerr << "socket: " << op->req->socket_fd << "   oid: " << oid << "    length:" << length << std::endl;
    if( cache_lookup(op, oid, data_oid) ){
        ret = write_hit( op->cache_entry, oid, offset, length, data );
    }else{
        ret = write_miss( op->cache_entry, oid, oid, offset, length, data );
    }

    if(ret == 0){
        if(op->cache_entry->if_dirty){
            this->cct->lru_dirty->touch_key( (char*)op->cache_entry );
            if(op->cache_entry->inflight_flush_ops->get_size())
                op->cache_entry->new_update_after_flush = true;
        }else{
            this->cct->lru_clean->remove( (char*)op->cache_entry);
            this->cct->lru_dirty->touch_key( (char*)op->cache_entry );
            op->cache_entry->if_dirty = true;
        }
        char status = MSG_SUCCESS;
        ret = commit_op( op, &status );
    }else{
        char status = MSG_FAIL;
        ret = commit_op( op, &status );
    }
    return ret;
}

bool CacheService::cache_lookup( Op* op, const char* oid, char* data_oid ){
    //std::cout << "cache_lookup" << std::endl;
    CacheEntry* cache_entry = op->cache_entry;
    //check if request data cached
    //check upon cache entry
    if( !cache_entry->data_map_lookup(op->offset, op->length) ){
        return false;
    }
    /*
    if( (this->cct->metastore->get( oid, data_oid )) == -1 ){
        log_err("cache_entry says cached, but unable to find a metastore");
        return false;
    }*/
    //std::cout << "get oid name from rocksdb: " << data_oid << std::endl;
    return true;
}

int CacheService::metastore_update( const char* oid, const char* data_oid ){
    int ret = this->cct->metastore->put( oid, data_oid );
    return ret;
}

int CacheService::datastore_update( const char* data_oid, uint64_t offset, uint64_t length, const char* data ){
    //uint64_t start_off = offset%this->cct->object_size;
    int ret = this->cct->cacher->_write( data_oid,  data, offset, length );
    return ret;
}

int CacheService::datastore_get( const char* data_oid, uint64_t offset, uint64_t length, char* data ){
    //uint64_t start_off = offset%this->cct->object_size;
    int ret = this->cct->cacher->_read( data_oid,  data, offset, length );
    return ret;
}

int CacheService::backstore_get( Op* op, char* data ){
    std::string image_name;
    std::stringstream ss;
    ss << op->location_id;
    ss >> image_name;
    int ret = this->cct->backendstore->read( image_name, op->offset, op->length, data, "rbd" );
    return ret;
}

int CacheService::write_hit( CacheEntry* cache_entry, const char* data_oid, uint64_t offset, uint64_t length, const char* data ){
    int ret = datastore_update( data_oid, offset, length, data );
    return ret;
}

int CacheService::read_hit( CacheEntry* cache_entry, const char* data_oid, uint64_t offset, uint64_t length, char* data ){
    std::cout << data_oid << "-" << offset << " read_hit" << std::endl;
    int ret = datastore_get( data_oid, offset, length, data );
    return ret;
}

int CacheService::write_miss( CacheEntry* cache_entry, const char* oid, const char* data_oid, uint64_t offset, uint64_t length, const char* data ){
    int ret = datastore_update( data_oid, offset, length, data );
    if( ret == 0 ){
        ret = metastore_update( oid, data_oid );
        cache_entry->data_map_update( offset % this->cct->object_size, length );
    }
    return ret;
}

int CacheService::read_miss( CacheEntry* cache_entry, const char* oid, const char* data_oid, uint64_t offset, uint64_t length, char* data, Op* op ){
    std::cout << data_oid << "-" << offset << " read_miss" << std::endl;
    int ret = backstore_get( op, data );
    ret = datastore_update( data_oid, offset, length, data );
    if( ret == 0 ){
        ret = metastore_update( oid, data_oid );
        cache_entry->data_map_update( offset % this->cct->object_size, length );
    }
    return ret;
}

int CacheService::commit_op( Op* op, const char* data ){
    int ret;
    op->req->req_lock.lock();
    op->req->uncomplete_count -= 1;
    op->req->req_status &= *data;
    op->req->req_lock.unlock();
    if(op->req->uncomplete_count == 0){
        //if( op->req->source_type == REQ_MESSENGER ){
        //    ret = cct->messenger_d->reply_data( &op->req->req_status, sizeof(op->req->req_status), op->req );
        //}else if( op->req->source_type == REQ_LIBRARY ){
        //    ret = reply_data( &op->req->req_status, sizeof(op->req->req_status), op->req );
        //}

        ret = reply_data( &op->req->req_status, sizeof(op->req->req_status), op->req );
    }
    return ret;
}

int CacheService::commit_op_with_data( Op* op, const char* data ){
    int ret = 0;
    op->req->req_lock.lock();
    op->req->uncomplete_count -= 1;
    op->req->req_lock.unlock();
    if(op->req->uncomplete_count == 0){
        //if( op->req->source_type == REQ_MESSENGER ){
        //    ret = cct->messenger_d->reply_data( data, op->req->msg->header.length, op->req );
        //}else if( op->req->source_type == REQ_LIBRARY ){
        //    ret = reply_data( data, op->req->msg->header.length, op->req );
        //}
        ret = reply_data( data, op->req->msg->header.length, op->req );
    }
    return ret;
}

int CacheService::reply_data( const char* data, uint64_t length, Request* req ){
    //log_print("reply_data\n");
    Msg* r_msg = req->msg;
    Message* reply_msg = new Message( r_msg->header.location_id, r_msg->header.offset, data, length, MSG_WRITE );
    req->msg = reply_msg->_msg; 
    delete r_msg;
    if( req->source_type == REQ_LIBRARY ){
        req->cond->Signal();
    }else if( req->source_type == REQ_LIBRARY_AIO ){
        int ret = 0;
        if(*req->msg->content == MSG_SUCCESS){
            ret = 0;
        }
        else if(*req->msg->content == MSG_FAIL){
            ret = -1;
        }
        req->comp->complete(ret);
        delete_request(req);
    }
    return 0;
}

int CacheService::delete_op( Op* op ){
    delete op;
    return 0;
}

int CacheService::delete_request( Request* req ){
    //delete req->msg;
    delete req;
    return 0;
}
}
