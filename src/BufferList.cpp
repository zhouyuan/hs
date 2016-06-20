#include "rbc/BufferList.h"

#include <iostream>
#include <sstream>
#include <stdio.h>

namespace rbc {
BufferList::BufferList( uint64_t length ){
  _buffers = (struct iovec*) malloc( sizeof(struct iovec) * length );
  _size = length;
  _index = 0;
  _total_bytes = 0;
}

BufferList::~BufferList(){
    for(int i=0; i<_index; i++)
        delete (char*)_buffers[i].iov_base;
    free(_buffers);
}

int BufferList::_to_buffer( char* buf, uint64_t offset, uint64_t length ){
  uint64_t block_off = 0;
  uint64_t written_len = 0;
  for( uint64_t i = 0; i < _index; i++ ){
    written_len = length<_buffers[i].iov_len?length:_buffers[i].iov_len;
    char* p = (char*) _buffers[i].iov_base;
    memcpy( &buf[block_off], &p[offset], written_len );
    offset = 0;
    length -= written_len;
    block_off += written_len;
  }
}

int BufferList::append( const char* s, ssize_t length ){
  _buffers[_index].iov_base = (void *)s;
  _buffers[_index].iov_len = length;
  _index++;
  _total_bytes += length;
}

int BufferList::append( std::string s ){
  append( s.c_str(), s.length() );
}

int BufferList::write_fd(int fd, uint64_t offset){
  ssize_t wrote;
  wrote = pwritev( fd, _buffers, _index, offset );
  if (wrote < _total_bytes) {
    return errno;
  }
}

int BufferList::read_fd(int fd, uint64_t offset){
  ssize_t read;
  read = preadv( fd, _buffers, _index, offset );
  if (read < _total_bytes) {
    return errno;
  }
}

}
