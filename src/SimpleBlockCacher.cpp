#include "rbc/SimpleBlockCacher.h"
#define BLOCK_SIZE 4096
#define OBJ_SIZE 4194304

namespace rbc {
SimpleBlockCacher::SimpleBlockCacher( std::string Device_Name ){
    device_name = Device_Name;
}

int SimpleBlockCacher::_open( std::string Device_Name ){
    _open( Device_Name.c_str() );
}

int SimpleBlockCacher::_open( const char* Device_Name ){
    char path[BUF_SIZE];
    sprintf( path, "%s/%s", device_name.c_str(), Device_Name );
  char err_output[BUF_SIZE];
  //block_fd = open( Device_Name.c_str(), O_RDWR | O_DIRECT );
  int block_fd = ::open( path, O_RDWR | O_CREAT, 0666 );
  if ( block_fd <= 0 ){
    sprintf( err_output, "[ERROR] SimpleBlockCacher::SimpleBlockCacher, unable to open %s, error code: %d ", Device_Name, errno );
    perror( err_output );
    return block_fd;
  }
}

int SimpleBlockCacher::_close( int block_fd ){
    int ret = ::close(block_fd);
    if(ret < 0){
        perror( "close block_fd failed" );
        return -1;
    }
    return 0;

}

uint64_t SimpleBlockCacher::get_block_index( uint64_t* index, uint64_t offset ){
  *index = offset / BLOCK_SIZE;
  return offset % BLOCK_SIZE;
}

int SimpleBlockCacher::_write( const char* filename, const char *buf, uint64_t offset, uint64_t length ){
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
}

int SimpleBlockCacher::_write( int block_fd, const char *buf, uint64_t offset, uint64_t length ){
  uint64_t index = 0;
  uint64_t block_index = 0;
  uint64_t off_by_block = get_block_index( &index, offset );
  uint64_t left = off_by_block + length;
  uint64_t start = 0;
  uint64_t buffers_len = (left + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;
  BufferList bl( buffers_len );

  char* alignedBuff;
  while( left > 0 ){
    uint64_t write_len = left < BLOCK_SIZE?length:BLOCK_SIZE;
    alignedBuff = new char[BLOCK_SIZE];

    // handle partial write

    if( off_by_block > 0 || left < BLOCK_SIZE ){
      _read( block_fd, alignedBuff, (index + block_index) * BLOCK_SIZE, BLOCK_SIZE);
    }else{
      memset( alignedBuff, 0, BLOCK_SIZE );
    }
    memcpy( alignedBuff+off_by_block, &buf[start], write_len );
    bl.append( alignedBuff, BLOCK_SIZE );

    left -= write_len==BLOCK_SIZE?BLOCK_SIZE:(write_len + off_by_block);
    off_by_block = 0;
    start += write_len;
    block_index++;
  }

  char err_output[BUF_SIZE];
  int ret = bl.write_fd( block_fd, index * BLOCK_SIZE );
  if ( ret < 0 ){
    sprintf( err_output, "[ERROR] SimpleBlockCacher::write_fd, unable to write data, error code: %d ", errno );
    perror( err_output );
    assert( ret < 0 );
  }

  return 0;
}

int SimpleBlockCacher::_read( const char* filename, char *buf, uint64_t offset, uint64_t length ){
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

}

int SimpleBlockCacher::_read( int block_fd, char *buf, uint64_t offset, uint64_t length ){
    char err_output[BUF_SIZE];
    uint64_t index = 0;
    uint64_t off_by_block = get_block_index( &index, offset );
    uint64_t left = off_by_block + length;
    uint64_t buffers_len = (left + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;
    BufferList bl = BufferList( buffers_len );

    while( left > 0 ){
        uint64_t read_len = left < BLOCK_SIZE?length:BLOCK_SIZE;
        char *alignedBuff = (char*) malloc(BLOCK_SIZE);
        bl.append( alignedBuff, BLOCK_SIZE );
        left -= read_len==BLOCK_SIZE?BLOCK_SIZE:(read_len + off_by_block);
    }

    int ret = bl.read_fd( block_fd, index * BLOCK_SIZE );
    if ( ret < 0 ){
        sprintf( err_output, "[ERROR] SimpleBlockCacher::read_fd, unable to read data, error code: %d ", errno );
        perror( err_output );
        assert( ret < 0 );
    }

    bl._to_buffer( buf, off_by_block, length );

    return 0;
}

int SimpleBlockCacher::_remove( const char* filename ){
    char path[BUF_SIZE];
    sprintf( path, "%s/%s", device_name.c_str(), filename );
    char err_output[BUF_SIZE];
    //log_print("SimpleBlockCacher::_remove remove file name:%s\n", path);
    int ret = remove(path);
    if ( ret < 0 ){
        sprintf( err_output, "[ERROR] SimpleBlockCacher::_remove, unable to remove data %s, error code: %d ", filename, errno );
        perror( err_output );
        assert( ret < 0 );
    }
    return ret;
}

int SimpleBlockCacher::lookup( const char *buf, uint64_t offset, uint64_t length ){
  return 0;
}
}
