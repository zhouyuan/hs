#include <queue>
#include <mutex>
#include <iostream>

#ifndef WORKQUEUE_H
#define WORKQUEUE_H

namespace rbc {
template <typename T> class WorkQueue{
public:
    typedef T queue_type;
    std::queue<queue_type> _queue;
    std::mutex _queue_lock;

    void enqueue( queue_type _work ){
        _queue_lock.lock();
        this->_queue.push( _work );
        _queue_lock.unlock();
    }

    queue_type dequeue(){
        _queue_lock.lock();
        if(this->_queue.empty()){
            _queue_lock.unlock();
            return NULL;
        }

        queue_type data = this->_queue.front();
        this->_queue.pop();
        _queue_lock.unlock();
        return data;
    }

    bool empty(){
        return this->_queue.empty();
    }
};
}
#endif
