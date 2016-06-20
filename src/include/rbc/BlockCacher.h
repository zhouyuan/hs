#include "rbc/ufc.h"
#include "rbc/BufferList.h"

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <stdint.h>


namespace rbc {
class BlockCacher{
public:
    std::string device_name;
    ufc_t* ufc;
    ufc_options_t* options;
    uint64_t cache_total_size;

    BlockCacher(std::string Device_Name, uint64_t cache_total_size);
    int _open(std::string Device_Name);
    int _open(const char* Device_Name);
    int _close(int block_fd);
    int _write(  const char* filename, const char *buf, uint64_t offset, uint64_t length );
    int _read( const char* filename, char *buf, uint64_t offset, uint64_t length );
    int lookup(  const char *buf, uint64_t offset, uint64_t length );
    int _remove(uint64_t lba);
};
}
