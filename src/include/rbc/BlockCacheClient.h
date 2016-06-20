#ifndef BLOCKCACHECLIENT_H
#define BLOCKCACHECLIENT_H
#include "rbc/SimpleBlockCacher.h"

namespace rbc {
struct BlockCacheClient{
public:
    int set(char* key, char* value, char* value_size, uint64_t start_off);
    int get(char* key, char* value, char* value_size, uint64_t start_off);
};

}
#endif
