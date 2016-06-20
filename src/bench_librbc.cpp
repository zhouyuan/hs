//g++ -o test_librbc  test_librbc.cpp ../utils/librbc.cpp  ../CacheService/CacheService.cpp ../DataStore/BlockCacher/SimpleBlockCacher.cpp ../common/BufferList.cpp ../BackendStore/BackendStore.cpp ../CacheService/AgentService.cpp ../MetaStore/MetaStore.cpp ../Messenger/Messenger.cpp ../CacheService/CacheEntry.cpp -std=c++11 -lrados -lrbd -lboost_thread -lboost_system -lpthread -lmemcached -lrocksdb
#include "rbc/librbc.h"
#include "stdlib.h"
#include "stdio.h"
#include <time.h>
#include <thread>

time_t start;
bool go = true;
struct cb_param{
   uint8_t inflight_ops;
   uint64_t io_complete_count;
};

cb_param arg;
static void _finish_aiocb(int r, void *data){
    cb_param* tmp = (cb_param*) data;
    tmp->inflight_ops--;
    tmp->io_complete_count++;
    //printf("%lu completed\n", offset);
    //inflight_ops--;
    return;
}

void monitor(){
    time_t now;
    while(1){
        sleep(1);
        time(&now);
        uint64_t elapsed = difftime(now, start);

        printf("%d secs: iops %d, inflight_ops:%u\n", elapsed, arg.io_complete_count/elapsed, arg.inflight_ops);
        if(elapsed > 30)
            go = false;
    }

}

int rbc_aio_write( rbc::librbc* rbc, const char* image_name, uint64_t off, uint64_t length){
    char* data = new char[4096]();
    memset(data, 'a', 4096);
    rbc::rbc_completion_t comp;
    rbc->rbc_aio_create_completion( (void*)&arg, _finish_aiocb, &comp );
    rbc->rbc_aio_write(image_name, off, length, data, comp);
    arg.inflight_ops++;
    delete[] data;
    return 0;
}

int main(int argc, char *argv[]){
    arg.inflight_ops = 0;
    arg.io_complete_count = 0;
    rbc::librbc *rbc = new rbc::librbc();
    rbc::ThreadPool *threadpool = new rbc::ThreadPool(2);
    time(&start);
    threadpool->schedule( &monitor );

    char* op_type = argv[1];
    if(op_type[0] == 'w'){
        char* image_name = argv[2];

        srand(time(NULL));

        while(go){
            uint64_t off = rand()%10737418240/4096*4096;
            uint64_t length = 4096;
            if( arg.inflight_ops < 128 )
                threadpool->schedule(boost::bind( rbc_aio_write, rbc, image_name, off, length) );
            else{

                //printf("inflight_ops:%lu, do no op\n",inflight_ops);
                std::this_thread::yield();
            }
        }

    }else if(op_type[0] == 'r'){
        char* image_name = argv[2];
        uint64_t off = strtoul(argv[3], NULL, 0);
        uint64_t length = strtoul(argv[4], NULL, 0);
        char* data = new char[length];
        rbc->rbc_read(image_name, off, length, data);
        std::cout << "rbc_read: " << data << std::endl;

    }else{
        printf("Unrecognised type: ./test_librbc read/write volume_name offset data/length\n");
    }

    delete rbc;
    return 0;
}
