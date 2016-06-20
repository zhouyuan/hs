#include "rbc/librbc.h"

namespace rbc {

int librbc::rbc_aio_create_completion(void *cb_arg, callback_t complete_cb, rbc_completion_t *c){
    AioCompletion *comp = new AioCompletion(cb_arg, complete_cb);
    *c = (rbc_completion_t) comp;
    return 0;
}

int librbc::rbc_aio_read( const char* location, uint64_t offset, uint64_t length, char* data, rbc_completion_t c ){
    void* arg = (void*)c;
    Message* msg = new Message( location, offset, data, length, MSG_READ );
    Request *req = new Request( msg->_msg, REQ_LIBRARY_AIO, arg );
    csd.queue_io(req);

    data = req->msg->content;
    return 0;
}

int librbc::rbc_aio_write( const char* location, uint64_t offset, uint64_t length, const char* data, rbc_completion_t c ){
    void* arg = (void*)c;
    Message *msg = new Message( location, offset, data, length, MSG_WRITE );
    Request *req = new Request( msg->_msg, REQ_LIBRARY_AIO, arg );
    csd.queue_io(req);

    return 0;
}

int librbc::rbc_read( const char* location, uint64_t offset, uint64_t length, char* data ){
    SafeCond* cond = new SafeCond();
    void* arg = (void*)cond;
    Message* msg = new Message( location, offset, data, length, MSG_READ );
    Request *req = new Request( msg->_msg, REQ_LIBRARY, arg );
    //std::cout << "enqueue_op req:" << req << "  " << req->msg->header.location_id << "-" << req->msg->header.offset << std::endl;
    csd.queue_io(req);

    cond->wait();
    uint64_t len = req->msg->header.length;
    memcpy(data, req->msg->content, len);
    delete req;
    return len;
}

int librbc::rbc_write( const char* location, uint64_t offset, uint64_t length, const char* data ){
    SafeCond* cond = new SafeCond();
    void* arg = (void*)cond;
    Message *msg = new Message( location, offset, data, length, MSG_WRITE );
    Request *req = new Request( msg->_msg, REQ_LIBRARY, arg );
    //std::cout << "enqueue_op req:" << req << "  " << req->msg->header.location_id << "-" << req->msg->header.offset << std::endl;
    csd.queue_io(req);

    cond->wait();
    if(*req->msg->content == MSG_SUCCESS){
        delete req;
        return 0;
    }
    else if(*req->msg->content == MSG_FAIL){
        delete req;
        return -1;
    }
    return -1;
}
}
