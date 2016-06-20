#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#define LOCATION_BUFFER_SIZE 128
#define MSG_FLAG_SIZE 8
#define MSG_FLAG  0xFAFA0F0F
#define MSG_WRITE 0X0001
#define MSG_READ  0X0002
#define MSG_SUCCESS  0X000F
#define MSG_FAIL  0X0000

namespace rbc {
struct Msg_Header{
    uint64_t msg_flag;
    uint32_t seq_id;
    uint32_t type;
    char location_id[LOCATION_BUFFER_SIZE];
    uint64_t offset;
    uint64_t length;
    uint64_t content_size;
    uint64_t total_size;
};
struct Msg{
    Msg_Header header;
    char* content;
    ~Msg(){
        //printf("delete Msg %p\n", content);
        if(content)
            delete[] content;
    }
};
class Message{
public:
    Msg* _msg;
    Message( const char* location, uint64_t offset, const char* data, uint64_t data_len, uint32_t type=MSG_WRITE ){
        _msg = new Msg();
        _msg->header.msg_flag = MSG_FLAG;
        _msg->header.seq_id = 0;
        _msg->header.type = type;
        memcpy(_msg->header.location_id, location, strlen(location));
        _msg->header.offset = offset;
        _msg->content = NULL;
        if(type == MSG_WRITE){
            _msg->header.length = data_len;
            _msg->header.content_size = data_len;
            _msg->content = new char[ data_len ]();
            memcpy( _msg->content, data, data_len);
        }else if(type == MSG_READ){
            _msg->header.length = data_len;
            _msg->header.content_size = 0;
        }
        _msg->header.total_size = sizeof(Msg) + _msg->header.content_size;
    }
    ~Message(){
        delete _msg;
    }
};
}
#endif
