#include <rbc/CacheService.h>

namespace rbc {

typedef void *rbc_completion_t;

class librbc{
public:
    //librbc();
    //~librbc();
    //int rbc_open();
    //int rbc_close();
    int rbc_aio_create_completion(void *cb_arg, callback_t complete_cb, rbc_completion_t* c);
    int rbc_aio_read( const char* location, uint64_t offset, uint64_t length, char* data, rbc_completion_t comp );
    int rbc_aio_write( const char* location, uint64_t offset, uint64_t length, const char* data, rbc_completion_t comp );
    int rbc_read( const char* location, uint64_t offset, uint64_t length, char* data );
    int rbc_write( const char* location, uint64_t offset, uint64_t length, const char* data );
private:
    CacheService csd;
};
}
