
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
#include "rbc/common/log.h"


namespace rbc {
class SimpleBlockCacher{
public:
    std::string device_name;

    SimpleBlockCacher( std::string Device_Name );
    int _open(const char* Device_Name);
    int _open(std::string Device_Name);
    int _close(int block_fd);
    uint64_t get_block_index( uint64_t* index, uint64_t offset );
    int _write(  const char* filename, const char *buf, uint64_t offset, uint64_t length );
    int _write(  int block_fd, const char *buf, uint64_t offset, uint64_t length );
    int _read( const char* filename, char *buf, uint64_t offset, uint64_t length );
    int _read(  int block_fd, char *buf, uint64_t offset, uint64_t length );
    int _remove( const char* filename );
    int lookup(  const char *buf, uint64_t offset, uint64_t length );
};
}
