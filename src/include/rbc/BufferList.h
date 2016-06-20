#ifndef BUFFERLIST_H
#define BUFFERLIST_H

#include <sys/uio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <string>
#include <string.h>
#include <errno.h>
#define IOV_MAX 1024

namespace rbc {
class BufferList{
public:
  iovec *_buffers;
  uint64_t _size;
  uint64_t _index;
  uint64_t _total_bytes;

  BufferList( uint64_t length );
  ~BufferList();
  int write_fd( int fd, uint64_t offset );
  int read_fd( int fd, uint64_t offset );
  int _to_buffer( char* buf, uint64_t offset, uint64_t length );
  int append( const char *p, ssize_t length );
  int append( std::string s );
};
}
#endif
