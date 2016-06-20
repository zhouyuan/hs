#ifndef CACHEENTRY_H
#define CACHEENTRY_H

#include <mutex>
#include <map>
#include <stdio.h>
#include "rbc/common/List.h"

namespace rbc {
typedef std::map<uint64_t, uint64_t> EntryMap;

class CacheEntry{
public:
    std::string name;
    std::string location_id;
    uint64_t offset;
    std::mutex  entry_lock;
    uint64_t history_count;
    uint64_t object_size;
    bool if_dirty;
    bool new_update_after_flush;
    List *inflight_flush_ops;
    //std::mutex inflight_flush_ops_lock;
    bool if_fully_cached;

    CacheEntry( uint64_t object_size );
    ~CacheEntry();
    void init( uint64_t object_size );
    bool data_map_lookup( uint64_t offset, uint64_t length );
    void data_map_update( uint64_t offset, uint64_t length );
    uint64_t get_size( uint64_t &bl_count );
    uint64_t get_offset();
    EntryMap get_data_map();

private:
    EntryMap data_map;
};
}

#endif
