#include "rbc/BlockCacheClient.h"

namespace rbc {
int BlockCacheClient::set(char* key, char* value, char* value_size, uint64_t start_off){
    SimpleBlockCacher *cacher = new SimpleBlockCacher(key);

}

int BlockCacheClient::get(char* key, char* value, char* value_size, uint64_t start_off){

}
}
