#ifndef AIOCOMPLETION_H
#define AIOCOMPLETION_H

typedef void (*callback_t)(int r, void *arg);
namespace rbc{

typedef void *completion_t;
typedef void (*callback_t)(int r, void *arg);

class AioCompletion{
private:
    callback_t complete_cb;
    void *complete_arg;
    void set_complete_cb(void *cb_arg, callback_t cb) {
        complete_cb = cb;
        complete_arg = cb_arg;
    }

public:
    AioCompletion( void *cb_arg, callback_t cb ):complete_arg(NULL),complete_cb(NULL){
        set_complete_cb(cb_arg, cb);
    }

    void complete(int r){
        if (complete_cb) {
            complete_cb(r, complete_arg);
        }
    }

};

}
#endif
