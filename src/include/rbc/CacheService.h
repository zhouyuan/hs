#ifndef CACHESERVICE_H
#define CACHESERVICE_H

#include "rbc/AgentService.h"
#include "rbc/Message.h"
#include "rbc/CacheEntry.h"
#include "rbc/common/Request.h"
#include "rbc/common/WorkQueue.h"
#include "rbc/common/Context.h"
#include "rbc/common/Op.h"
#include "rbc/common/Cond.h"

#include <stdio.h>
#include <string.h>
#include <sstream>
#include <signal.h>
#define BUFSIZE 256

namespace rbc {
class CacheService{
public:

private:
    Context *cct;
    AgentService *agentservice;
    ThreadPool *threadpool;
    ThreadPool *process_tp;
    void do_op(Op *op);
    int do_read_op(Op *op);
    int do_write_op(Op *op);
    bool cache_lookup(Op *op, const char* oid, char* data_oid);
    void dequeue_op();
    void _process();
    std::string get_index( const char* location_id, uint64_t offset );
    int write_hit( CacheEntry* cache_entry, const char* data_oid, uint64_t offset, uint64_t length, const char* data);
    int write_miss( CacheEntry* cache_entry, const char* oid, const char* data_oid, uint64_t offset, uint64_t length, const char* data);
    int read_hit( CacheEntry* cache_entry, const char* data_oid, uint64_t offset, uint64_t length, char* data );
    int read_miss( CacheEntry* cache_entry, const char* oid, const char* data_oid, uint64_t offset, uint64_t length, char* data, Op* op );
    int datastore_update(const char* data_oid, uint64_t offset, uint64_t length, const char* data);
    int metastore_update(const char* oid, const char* data_oid);
    int datastore_get( const char* data_oid, uint64_t offset, uint64_t length, char* data );
    int backstore_get( Op* op, char* data );
    int delete_request(Request* req);
    int delete_op(Op* op);
    int commit_op_with_data(Op* op, const char* data);
    int commit_op(Op* op, const char* data);
    int reply_data( const char* data, uint64_t length, Request* req );
    std::list<Op*> map_request_by_entry( Request* req );

public:
    CacheService();
    ~CacheService();
    void start();
    int queue_io( Request* req ) ;
};
}

#endif
