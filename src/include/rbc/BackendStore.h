#ifndef BACKENDSTORE_H
#define BACKENDSTORE_H


#include <stdio.h>
#include <string>
#include <map>
#include <rbd/librbd.h>
#include <mutex>
#include "rbc/common/log.h"
#include "rbc/CacheEntry.h"
#include "rbc/C_AioBackendCompletion.h"

struct rbd_aio_unit {
    rbd_completion_t completion;
    rbc::C_AioBackendCompletion *onfinish;
    rbd_aio_unit( rbc::C_AioBackendCompletion *onfinish ):onfinish(onfinish){}
};

namespace rbc {
class BackendStore{
public:
    BackendStore( const char* clientname );
    ~BackendStore();
    int write( std::string rbd_name, uint64_t offset, uint64_t length, const char* data, std::string pool_name, C_AioBackendCompletion *onfinish);
    int read( std::string rbd_name, uint64_t offset, uint64_t length, char* data, std::string pool_name );
private:
    struct rbd_data {
        rados_ioctx_t io_ctx;
        rbd_image_t image;
        std::string pool_name;

        rbd_data( std::string _pool_name ){
            pool_name = _pool_name;
        }
    };

    std::map<std::string, rbd_data*> rbd_info_map;
    std::mutex rbd_info_map_lock;
    rados_t cluster;
    char* client_name;

    int _open( std::string rbd_name, std::string pool_name );
    int _close( std::string rbd_name );
    int _close(rbd_data* rbd );
    int _write( rbd_data* rbd, uint64_t offset, uint64_t length, const char* data, C_AioBackendCompletion *onfinish );
    int _read( rbd_data* rbd, uint64_t offset, uint64_t length, char* data );
    int _write( std::string rbd_name, uint64_t offset, uint64_t length, const char* data, C_AioBackendCompletion *onfinish );
    int _read( std::string rbd_name, uint64_t offset, uint64_t length, char* data );


};

}
#endif
