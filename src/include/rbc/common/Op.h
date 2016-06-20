#ifndef OP_H
#define OP_H
#include "Request.h"
#include "rbc/CacheEntry.h"

namespace rbc {
class Op{
public:
    char* location_id;
    uint64_t offset;
    uint64_t length;
    char* data;
    CacheEntry* cache_entry;
    Request* req;
    Op(char* location_id, uint64_t offset, char* data, uint64_t length, Request* req):location_id(location_id),offset(offset),data(data),length(length),req(req){};
};

}
#endif
