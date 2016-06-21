#ifndef CONTEXT_H
#define CONTEXT_H

#include "Config.h"
#include "Request.h"
#include "LRU_Linklist.h"
#include "CacheMap.h"
#include "../Message.h"
#include "WorkQueue.h"
#include "ThreadPool.h"
#include "../CacheEntry.h"
#include "../SimpleBlockCacher.h"
#include "rbc/BlockCacher.h"
#include "../MetaStore.h"
#include "../BackendStore.h"
#include <boost/thread/shared_mutex.hpp>
#define THREADPOOLSIZE 64

namespace rbc {

//class AgentService;
static uint64_t g_object_size;

static char* _new_cache_entry(const std::string& key ){
    CacheEntry* value = new CacheEntry( g_object_size );
    return (char*)value;
}

class Context{
public:
    bool go;
    Config *config;
    WorkQueue<void*> request_queue;
    BlockCacher *cacher;
    MetaStore *metastore;
    BackendStore *backendstore;

    uint64_t object_size;
    uint64_t cache_lru_length;

    CacheMap<std::string, char*> *cache_map;
    boost::shared_mutex cachemap_access;
    LRU_LIST<char*> *lru_dirty;
    LRU_LIST<char*> *lru_clean;

    Context(){
        go = true;
        config = new Config();

        object_size = stoull(config->configValues["object_size"]);
        g_object_size = object_size;
        cache_lru_length = stoull(config->configValues["cache_lru_length"]);

        cache_map = new CacheMap<std::string, char*>( _new_cache_entry );
        lru_dirty = new LRU_LIST<char*>;
        lru_clean = new LRU_LIST<char*>;

        cacher = new BlockCacher(config->configValues["DataStoreDev"],
                                 stoull(config->configValues["cache_total_size"]));
        //TODO use different metastore prefixed with volume_name?
        metastore = new MetaStore(config->configValues["MetaStoreDir"]);
        log_print( "backendstore construction start\n");
        backendstore = new BackendStore("rbcclient");
        log_print( "backendstore construction finish\n");
    }
    ~Context(){
        delete cache_map;
        delete lru_dirty;
        delete lru_clean;
        log_print("going to delete cacher\n");
        delete cacher;
        log_print("going to delete metastore\n");
        delete metastore;
        log_print("going to delete backendstore\n");
        delete backendstore;
        delete config;
    }
};
}

#endif
