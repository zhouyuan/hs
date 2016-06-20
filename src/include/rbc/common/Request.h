#include "../Message.h"
#include "../common/AioCompletion.h"
#include "../common/Cond.h"
#include <mutex>

#ifndef REQUEST_H
#define REQUEST_H

#define REQ_MESSENGER 0X0010
#define REQ_LIBRARY   0X0011
#define REQ_LIBRARY_AIO   0X0012

namespace rbc {
class Request;
//typedef int (*callback_t)(const char* data, uint64_t length, Request* req);

class Request{
public:
    Msg* msg;
    std::mutex req_lock;
    uint64_t uncomplete_count;
    char req_status;
    int socket_fd;
    SafeCond* cond;
    AioCompletion* comp;
    int source_type;

    Request( Msg* msg, int source_type, void* arg ):msg(msg), source_type(source_type){
        switch(source_type){
            case REQ_MESSENGER:
                socket_fd = *((int*)arg);
                cond = NULL;
                comp = NULL;
                break;
            case REQ_LIBRARY:
                cond = (SafeCond*)arg;
                socket_fd = 0;
                comp = NULL;
                break;
            case REQ_LIBRARY_AIO:
                comp = (AioCompletion*)arg;
                socket_fd = 0;
                cond = NULL;
                break;
            default:
                break;
        }
    }

    ~Request(){
        if(cond!=NULL)
            delete cond;
        if(comp!=NULL)
            delete comp;
        if(msg!=NULL)
            delete msg;
    }
};

}
#endif
