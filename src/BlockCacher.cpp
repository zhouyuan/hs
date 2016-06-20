#include "rbc/BlockCacher.h"

namespace rbc {

BlockCacher::BlockCacher(std::string Device_Name, uint64_t cache_total_size){
    device_name = Device_Name;
    cache_total_size = cache_total_size;
    ufc = NULL;
    _open(device_name);
}

int BlockCacher::_open( std::string Device_Name ){
    _open( Device_Name.c_str() );
}

int BlockCacher::_open( const char* Device_Name ){
/*
    char path[BUF_SIZE];
    sprintf( path, "%s/%s", device_name.c_str(), Device_Name );
  char err_output[BUF_SIZE];
  //block_fd = open( Device_Name.c_str(), O_RDWR | O_DIRECT );
  int block_fd = ::open( path, O_RDWR | O_CREAT, 0666 );
  if ( block_fd <= 0 ){
    sprintf( err_output, "[ERROR] BlockCacher::BlockCacher, unable to open %s, error code: %d ", Device_Name, errno );
    perror( err_output );
    return block_fd;
  }
*/

    options = ufc_options_create(cache_total_size);
    int ret = ufc_open(Device_Name, options, &ufc);
    return ret;

}

int BlockCacher::_close( int block_fd ){
/*
    int ret = ::close(block_fd);
    if(ret < 0){
        perror( "close block_fd failed" );
        return -1;
    }
    return 0;
*/
    ufc_close(ufc);
    ufc_options_destroy(options);
}

int BlockCacher::_write( const char* filename, const char *buf, uint64_t offset, uint64_t length ){
/*
    int block_fd;
    if( (block_fd = _open( filename )) < 0 ){
        return -1;
    }
    int ret =  _write( block_fd, buf, offset, length );
    if(ret == 0){
        //::fsync( block_fd );
        ret = _close(block_fd);
    }else{
        _close(block_fd);
    }
    return ret;
*/
    return ufc_write(ufc, buf, length, offset);
}


int BlockCacher::_read( const char* filename, char *buf, uint64_t offset, uint64_t length ){
/*
    int block_fd;
    if( (block_fd = _open( filename )) < 0 ){
        return -1;
    }
    int ret = _read( block_fd, buf, offset, length );
    if(ret == 0){
        ret = _close(block_fd);
    }else{
        _close(block_fd);
    }
    return ret;

*/
    return ufc_read(ufc, buf, length, offset);
}


int BlockCacher::lookup( const char *buf, uint64_t offset, uint64_t length ){
  return 0;
}


int BlockCacher::_remove(uint64_t lba) {
    return ufc_remove(ufc, lba);
}

}
