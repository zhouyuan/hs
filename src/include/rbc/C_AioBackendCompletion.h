#ifndef C_AIOBACKENDCOMPLETION_H
#define C_AIOBACKENDCOMPLETION_H

#include <inttypes.h>
#include "rbc/CacheEntry.h"
#include "rbc/common/Count.h"
namespace rbc{
class C_AioBackendCompletion{
public:
    CacheEntry* _c_entry;
    void* _p_inflight;
    Count* _op_count;

    C_AioBackendCompletion( CacheEntry* c_entry, Count* backend_op_count ){
        _c_entry = c_entry;
        _op_count = backend_op_count;
        _p_inflight = (void*)_c_entry->inflight_flush_ops->insert();
        //log_print("C_AioBackendCompletion insert _c_entry:%s, hold_items_count:%" PRId16 ", _p_inflight:%p\n", _c_entry->name.c_str(), _c_entry->inflight_flush_ops->get_size(), _p_inflight);
    }
    void finish( int r ){
        //log_print("C_AioBackendCompletion finish, c_entry->name:%s\n", _c_entry->name.c_str());
        int64_t hold_items_count = _c_entry->inflight_flush_ops->remove(_p_inflight);
        //log_print("C_AioBackendCompletion finish _c_entry:%s, hold_items_count:%" PRId16 "\n", _c_entry->name.c_str(), hold_items_count );
        if(hold_items_count == 0 && !_c_entry->new_update_after_flush){
            _c_entry->entry_lock.lock();
            _c_entry->if_dirty = false;
            _c_entry->entry_lock.unlock();
            _op_count->dec();
            //log_print("_c_entry %s finish flush op count:%" PRId32 "\n",_c_entry->name.c_str(), _op_count->get());
        }
        if(hold_items_count == 0 && _c_entry->new_update_after_flush){
            _c_entry->entry_lock.lock();
            _c_entry->new_update_after_flush = false;
            _c_entry->entry_lock.unlock();
            _op_count->dec();
            //log_print("_c_entry %s finish flush op count:%" PRId32 "\n",_c_entry->name.c_str(), _op_count->get());
        }
    }
};
}

#endif
