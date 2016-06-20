#include <mutex>
#include <rbc/common/log.h>
namespace rbc{
#ifndef LIST_H
#define LIST_H

class List{
public:
    List( int16_t len ){
        list = new bool[len];
        this->len = len;
        hold_items_count = 0;
        for( int16_t i = 0; i < len; i++ ){
            list[i] = false;
        }
    }
    ~List(){
        delete[] list;
    }
    bool* insert(){
        bool* ret;
        list_lock.lock();
        for( int16_t i = 0; i < len; i++ ){
            if( !list[i] ){
                list[i] = true;
                hold_items_count++;
                ret = &list[i];
                list_lock.unlock();
                return ret;
            }
        }
        list_lock.unlock();
        log_print("insert failed\n");
        return NULL;
    }
    int16_t remove( void* p ){
        list_lock.lock();
        bool* item = (bool*)p;
        *item = false;
        int16_t ret = --hold_items_count;
        list_lock.unlock();
        return ret;
    }

    int16_t get_size(){
        int16_t ret = hold_items_count;
        return ret;
    }
private:
    bool *list;
    std::mutex list_lock;
    int16_t len;
    int16_t hold_items_count;

};

#endif
}
