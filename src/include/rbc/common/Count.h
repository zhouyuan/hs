#include <mutex>

#ifndef COUNT_H
#define COUNT_H

class Count{
public:
    int32_t op_count;
    std::mutex op_count_lock;
    Count(){
        op_count = 0;
    }
    void inc(){
        op_count_lock.lock();
        op_count++;
        op_count_lock.unlock();
    }
    void dec(){
        op_count_lock.lock();
        op_count--;
        op_count_lock.unlock();
    }
    int32_t get(){
        return op_count;
    }
};

#endif
