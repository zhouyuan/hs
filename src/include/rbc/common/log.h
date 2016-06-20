#ifndef LOG_H
#define LOG_H

#include <string>
#include <time.h>
#include <string.h>
#include <thread>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

#define BUF_SIZE 256

namespace rbc {
static int timespec2str(char *buf, uint16_t len, struct timespec *ts) {
    int ret;
    struct tm t;

    tzset();
    if (localtime_r(&(ts->tv_sec), &t) == NULL)
        return 1;

    ret = strftime(buf, len, "%F %T", &t);
    if (ret == 0)
        return 2;
    len -= ret - 1;

    ret = snprintf(&buf[strlen(buf)], len, ".%09ld", ts->tv_nsec);
    if (ret >= len)
        return 3;

    return 0;
}


static void get_time(char* time_str)
{
    /*time_t ltime;
    ltime=time(NULL);
    char* time_str = asctime( localtime(&ltime) );
    time_str[strlen(time_str) - 1] = 0;
    return time_str;*/
    const uint16_t TIME_FMT = strlen("2012-12-31 12:59:59.123456789") + 1;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    timespec2str( time_str, TIME_FMT, &spec);
}
static void log_err( const char* output, ... ){
    char time_str[BUF_SIZE] = {0};
    char buf[BUF_SIZE] = {0};
    va_list vl;
    va_start(vl, output);
    vsnprintf( buf, sizeof( buf), output, vl);
    //format_string(output, vl, formatted_string);
    va_end(vl);
    get_time(time_str);
    fprintf( stderr, "[%s] %s", time_str, output );
    //std::cerr <<  "[" << get_time() << " " <<  std::this_thread::get_id() << "]" << output << std::endl;
}

/*static void log_print( const char* output ){
    fprintf( stderr, "[%s] %s", get_time(), output );
    //std::cerr <<  "[" << get_time() << " " <<  std::this_thread::get_id() << "]" << output << std::endl;
}*/

static void log_print( const char* output, ... ){
    char time_str[BUF_SIZE] = {0};
    char buf[BUF_SIZE] = {0};
    va_list vl;
    va_start(vl, output);
    vsnprintf( buf, sizeof( buf), output, vl);
    //format_string(output, vl, formatted_string);
    va_end(vl);
    get_time(time_str);
    fprintf( stderr, "[%s] %s", time_str, buf );
    //std::cerr <<  "[" << get_time() << " " <<  std::this_thread::get_id() << "]" << output << std::endl;
}

}
#endif
