#ifndef AGENTSERVICE_H
#define AGENTSERVICE_H

#include "rbc/common/Request.h"
#include "rbc/common/WorkQueue.h"
#include "rbc/common/Op.h"
#include "rbc/common/LRU_Linklist.h"
#include "rbc/common/Count.h"
#include "rbc/common/Context.h"
#include "rbc/CacheEntry.h"
#include "rbc/C_AioBackendCompletion.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <chrono>
#define BUFSIZE 256

namespace rbc {

class AgentService{
public:

private:
    Context *cct;
    ThreadPool *threadpool;
    ThreadPool *process_tp;
    int cache_evict_interval;
    int cache_flush_interval;
    uint16_t cache_flush_queue_depth;
    uint64_t object_size;
    uint64_t cache_total_size;
    float cache_free_ratio;
    float cache_dirty_ratio_min;
    float cache_dirty_ratio_max;
    float cache_ratio_max;
    float cache_ratio_health;
    void _process_evict();
    void _process_flush();
    void dump_dirty();
    void _flush( CacheEntry* c_entry, bool only_fully_cached );
    void flush( CacheEntry** c_entry_list, bool only_fully_cached );
    void flush_all();
    void flush_by_ratio( float target_ratio );
    void _evict( CacheEntry* c_entry );
    void evict( CacheEntry** c_entry_list );
    void evict_by_ratio();

public:
    AgentService( Context* cct );
    ~AgentService();
    Count* flush_op_count;
};


}
#endif
