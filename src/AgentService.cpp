#include "rbc/AgentService.h"

namespace rbc {
AgentService::AgentService( Context* cct ){
    this->cct = cct;
    flush_op_count = new Count();
    int tp_size = stoi(this->cct->config->configValues["agent_threads_num"]);
    threadpool = new ThreadPool(tp_size);
    process_tp = new ThreadPool(2);
    cache_total_size = stoull(this->cct->config->configValues["cache_total_size"]);
    cache_dirty_ratio_min = stof(this->cct->config->configValues["cache_dirty_ratio_min"]);
    cache_dirty_ratio_max = stof(this->cct->config->configValues["cache_dirty_ratio_max"]);
    object_size = stoull(this->cct->config->configValues["object_size"]);
    cache_flush_interval = stoi(this->cct->config->configValues["cache_flush_interval"]);
    cache_evict_interval = stoi(this->cct->config->configValues["cache_evict_interval"]);
    cache_flush_queue_depth = stoi(this->cct->config->configValues["cache_flush_queue_depth"]);
    cache_ratio_max = stof(this->cct->config->configValues["cache_ratio_max"]);
    cache_ratio_health = stof(this->cct->config->configValues["cache_ratio_health"]);
    process_tp->schedule(boost::bind(&AgentService::_process_flush, this));
    process_tp->schedule(boost::bind(&AgentService::_process_evict, this));
    log_print("AgentService constructed\n");
}

AgentService::~AgentService(){
    log_print("delete agentservice started\n");
    cct->go = false;
    threadpool->wait();
    delete threadpool;
    process_tp->wait();
    delete process_tp;
    flush_all();
    log_print("after last time flush_all, still need to wait %" PRId32 "op to finish\n", flush_op_count->get());
    while(flush_op_count->get() > 0){
        sleep(0.01);
    }
    delete flush_op_count;
    log_print("delete agentservice complete\n");
}

void AgentService::dump_dirty(){
    sleep( cache_flush_interval );

    //char* c_entry_list = new char[1024];
    char* c_entry_list[1024] = {0};
    this->cct->lru_clean->get_keys( &c_entry_list[0], 1024 );
    log_print("*************Dump lru_clean***************\n");
    for(int i = 0; c_entry_list[i]!=0 && i < 1024; i++){
        CacheEntry* tmp = (CacheEntry*)c_entry_list[i];
        char buf[BUF_SIZE] = {0};
        sprintf( buf, "%s, if_fully_cached:%s\n",tmp->name.c_str(), tmp->if_fully_cached?"True":"False" );
        log_print( buf );
    }
}

void AgentService::_flush( CacheEntry* c_entry, bool only_fully_cached ){
    //log_print("_flush %p start\n", c_entry);
    if( c_entry == NULL ){
        log_err("c_entry is NULL\n");
        return;
    }
    try{
        c_entry->entry_lock.lock();
    }catch(...){
        log_print("AgentService::_flush lock c_entry %p failed\n", c_entry);
        return;
    }
    if( c_entry == NULL ){
        log_err("c_entry is NULL\n");
        return;
    }

    if(only_fully_cached && !c_entry->if_fully_cached){
        c_entry->entry_lock.unlock();
        return;
    }
    if(!c_entry->if_dirty || c_entry->inflight_flush_ops->get_size() ){
        c_entry->entry_lock.unlock();
        return;
    }
    if(c_entry->inflight_flush_ops->get_size()!=0){
        //which indicates the c_entry still pending for flush completion.
        c_entry->entry_lock.unlock();
        return;
    }


    //log_print("_flush %p start: cacher_read\n", c_entry);
    char* data_from_cache =new char[c_entry->object_size];
    char* data;
    uint64_t offset_by_cache_entry = 0;
    //int ret = this->cct->cacher->_read( c_entry->name.c_str(), data_from_cache, offset_by_cache_entry, c_entry->object_size );
    int ret = this->cct->cacher->_read( c_entry->name.c_str(), data_from_cache, c_entry->offset, c_entry->object_size );
    if(ret != 0){
        log_err( "AgentService::flush read_from_cache failed.Details: %s\n", c_entry->name.c_str() );
    }
    //create a char* list by cachemap
    //log_print("_flush %p start: backend_write offset:%lu\n", c_entry, c_entry->offset);
    EntryMap data_map = c_entry->get_data_map();
    flush_op_count->inc();
    for(EntryMap::iterator it = data_map.begin(); it != data_map.end(); it++){
        data =&data_from_cache[it->first];
        C_AioBackendCompletion *onfinish = new C_AioBackendCompletion( c_entry, flush_op_count );
        ret = cct->backendstore->write( c_entry->location_id, (c_entry->offset + it->first), it->second, data, "rbd", onfinish );
        if(ret != 0){
            log_print("AgentService::flush write_to_backend failed. Details: %s:%lu:%lu\n", c_entry->location_id.c_str(), (c_entry->offset + it->first), it->second);
        }
    }
    delete[] data_from_cache;
    //log_print("_flush %p start: change c_entry to dirty and set lru\n", c_entry);
    c_entry->entry_lock.unlock();

    this->cct->lru_clean->touch_key( (char*)c_entry );
    this->cct->lru_dirty->remove( (char*)c_entry );
    //log_print("_flush %p complete\n", c_entry);
}

void AgentService::flush( CacheEntry** c_entry_list, bool only_fully_cached=false ){
    int ret = 0;
    int total_entries = 0;
    //before we start flush, can we sort c_entry by offset ?
    std::map<uint64_t, CacheEntry*> tmp_sort;
    for(uint64_t i = 0; c_entry_list[i]!=0; i++){
        uint64_t off = c_entry_list[i]->get_offset();
        tmp_sort.insert( std::make_pair( off, c_entry_list[i] ) );
    }

    CacheEntry** c_entry_list_sort = new CacheEntry* [tmp_sort.size()+1]();
    uint64_t j = 0;
    for( std::map<uint64_t, CacheEntry*>::iterator it = tmp_sort.begin(); it!=tmp_sort.end(); it++ ){
        c_entry_list_sort[j++] = it->second;
        //log_print("flush off: %lu\n", it->first);
    }

    log_print("AgentService::flush start\n");
    log_print("AgentService::flush waiting %lu c_entry finish doing flush\n", j);
    for(uint64_t i = 0; c_entry_list_sort[i]!=0; i++){
        while(flush_op_count->get() >= cache_flush_queue_depth){
            sleep(0.01);
        }
        CacheEntry* c_entry = c_entry_list_sort[i];
        //log_print("_c_entry %s aio_write_to_librbd, flush_op_count: %" PRId32 "\n", c_entry->name.c_str(), flush_op_count->get());
        _flush( c_entry, only_fully_cached );
    }
    delete[] c_entry_list_sort;
    log_print("AgentService::flush complete\n");
    return;
}

void AgentService::flush_all(){
    //should scan on all caches
    uint64_t cache_count = this->cct->cache_map->size();
    char** c_entry_list = new char* [cache_count+1]();
    this->cct->cache_map->get_values( c_entry_list );
    flush( (CacheEntry**)c_entry_list );
    delete[] c_entry_list;
    return;
}

void AgentService::_evict( CacheEntry* c_entry ){
    if( c_entry == NULL ){
        std::this_thread::yield();
        return;
    }
    try{
        c_entry->entry_lock.lock();
    }catch(...){
        log_print("AgentService::_evict lock c_entry %p failed\n", c_entry);
        return;
    }
    if( c_entry == NULL ){
        std::this_thread::yield();
        return;
    }
    if( c_entry->if_dirty ){
        c_entry->entry_lock.unlock();
        return;
    }
    //this->cct->cacher->_remove( c_entry->name.c_str() );
    this->cct->cacher->_remove(c_entry->offset);
    //this->cct->cache_map->evict( c_entry->name );
    c_entry->entry_lock.unlock();
    this->cct->lru_clean->remove( (char*)c_entry );
    //free( c_entry );
    //c_entry = NULL;
}

void AgentService::evict( CacheEntry** c_entry_list ){
    log_print("AgentService::evict start\n");
    int ret = 0;
    bool if_dirty = false;
    uint64_t i = 0;
    for(;c_entry_list[i]!=0; i++){
        CacheEntry* c_entry = c_entry_list[i];
        //printf("CacheEntry ptr:%p\n", c_entry);
        threadpool->schedule(boost::bind(&AgentService::_evict, this, c_entry));
    }
    log_print("AgentService::evict waiting for %lu c_entry finish doing evict\n", i);
    threadpool->wait();
    log_print("AgentService::evict complete\n");
    return;
}

void AgentService::flush_by_ratio( float target_ratio=0 ){
    if(target_ratio==0)
        target_ratio = cache_dirty_ratio_max;
    uint64_t dirty_block_count = this->cct->lru_dirty->get_length();
    uint64_t total_block_count = cache_total_size / object_size;
    log_print( "AgentService::flush_by_ratio: dirty_ratio:%2.4f \n", ( 1.0*dirty_block_count/total_block_count ) );
    if( ( 1.0*dirty_block_count/total_block_count ) < cache_dirty_ratio_min ){
        return;
    }
    else{
        //we will do flush 2 times,
        //first time only flush fully cached cache_entry,
        //if still not meet request,
        //then flush not fully_cached

        //log_print("flush fully cached\n");
        uint64_t need_to_flush_count = dirty_block_count - total_block_count * cache_dirty_ratio_min;
        char** c_entry_list = new char* [need_to_flush_count+1]();
        this->cct->lru_dirty->get_keys( &c_entry_list[0], need_to_flush_count, false );
        flush( (CacheEntry**)c_entry_list );

        delete[] c_entry_list;
    }
    return;
}

void AgentService::evict_by_ratio(){
    uint64_t dirty_block_count = this->cct->lru_dirty->get_length();
    uint64_t clean_block_count = this->cct->lru_clean->get_length();
    uint64_t total_cached_block = dirty_block_count + clean_block_count;
    uint64_t total_block_count = cache_total_size / object_size;

    log_print( "AgentService::evict_by_ratio:  current cache ratio:%2.4f \n", ( 1.0*total_cached_block/total_block_count ) );
    if( (1.0*total_cached_block/total_block_count) < cache_ratio_max )
        return;

    boost::upgrade_lock<boost::shared_mutex> lock(this->cct->cachemap_access);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

    if( (1.0*dirty_block_count/total_block_count) > cache_ratio_health ){
        //because cached blocks are still in lru_dirty, 
        //we need to flush firstly so that we can do evict
        uint64_t need_to_flush_count = dirty_block_count - cache_ratio_health * total_block_count;
        char** c_entry_flush_list = new char* [need_to_flush_count+1]();
        this->cct->lru_dirty->get_keys( &c_entry_flush_list[0], need_to_flush_count, false );
        flush( (CacheEntry**)c_entry_flush_list );
        delete[] c_entry_flush_list;

    }

    uint64_t need_to_evict_count = total_cached_block - cache_ratio_health * total_block_count;
    char** c_entry_list = new char* [need_to_evict_count+1]();
    //memset( c_entry_list, 0, need_to_evict_count+1 );
    this->cct->lru_clean->get_keys( &c_entry_list[0], need_to_evict_count, false );
    evict( (CacheEntry**)c_entry_list );
    delete[] c_entry_list;

}

void AgentService::_process_flush(){
    int count = 0;
    while(this->cct->go){
        sleep( cache_flush_interval );
        if(this->cct->go){
            flush_by_ratio();
            count++;
            std::this_thread::yield();
        }
    }
    log_print("AgentService::_process_flush complete\n");
}
void AgentService::_process_evict(){
    int count = 0;
    while(this->cct->go){
        sleep( cache_evict_interval );
        if(this->cct->go){
            evict_by_ratio();
            //log_print("evict_by_ratio done, back to _process_evict\n");
            count++;
            std::this_thread::yield();
        }
    }
    log_print("AgentService::_process_evict complete\n");
}
}
