#include "rbc/BackendStore.h"

static void rbc_backend_finish_aiocb( rbd_completion_t comp, void *data ){
    rbd_aio_unit *io_u = (rbd_aio_unit*)data;
    int ret = rbd_aio_get_return_value(comp);
    //rbc::log_print("rbc_backend_finish_aiocb, get_return_value: %d\n", ret);
    io_u->onfinish->finish( ret );
}

namespace rbc {

BackendStore::BackendStore( const char* client_name ){
    int r;

    r = rados_create(&cluster, client_name);
    if (r < 0) {
        log_err("rados_create failed.\n");
        goto failed_early;
    }

    r = rados_conf_read_file(cluster, NULL);
    if (r < 0) {
        log_err("rados_conf_read_file failed.\n");
        goto failed_early;
    }

    r = rados_connect(cluster);
    if (r < 0) {
        log_err("rados_connect failed.\n");
        goto failed_shutdown;
    }
    return;
failed_shutdown:
    log_err("failed_shutdown\n");
    rados_shutdown(cluster);
    cluster = NULL;
failed_early:
    log_err("failed_early\n");
    return;
}

BackendStore::~BackendStore(){
    log_print("BackendStore destruction\n");
    for(std::map<std::string, rbd_data*>::iterator it=rbd_info_map.begin(); it!=rbd_info_map.end(); ++it){
        _close( it->second );
    }
    if(cluster){
        rados_shutdown(cluster);
        cluster = NULL;
    }
    log_print("BackendStore destruction complete\n");
}

int BackendStore::_open( std::string rbd_name, std::string pool_name ){
    char buf[BUF_SIZE] = {0};
    sprintf( buf, "BackendStore::_open %s\n", rbd_name.c_str());
    log_print(buf);
    rbd_data *rbd = new rbd_data( pool_name );

    //printf("connect to rados\n");
    int r = rados_ioctx_create(cluster, pool_name.c_str(), &rbd->io_ctx);
    if (r < 0) {
        log_err("rados_ioctx_create failed.\n");
        goto failed_shutdown;
    }

    //printf("rbd_open\n");
    r = rbd_open(rbd->io_ctx, rbd_name.c_str(), &rbd->image, NULL /*snap */ );
    if (r < 0) {
        log_err("rbd_open failed.\n");
        goto failed_open;
    }

    //printf("add rbd into rbd_info_map\n");
    rbd_info_map[rbd_name] = rbd;
    return 0;

failed_open:
    log_err("failed_open\n");
    rados_ioctx_destroy(rbd->io_ctx);
    rbd->io_ctx = NULL;
failed_shutdown:
    log_err("failed_shutdown\n");
    rados_shutdown(cluster);
    cluster = NULL;
    return -1;
}

int BackendStore::_close( std::string rbd_name ){
    std::map<std::string, rbd_data*>::iterator it = rbd_info_map.find(rbd_name);
    if(it==rbd_info_map.end())
        return 0;
    rbd_data* rbd = it->second;
    return _close( rbd );
}

int BackendStore::_close( rbd_data* rbd ){
    rados_ioctx_destroy(rbd->io_ctx);
    rbd->io_ctx = NULL;
    return 0;
}

int BackendStore::write( std::string rbd_name, uint64_t offset, uint64_t length, const char* data, std::string pool_name, C_AioBackendCompletion* onfinish ){
    int r = 0;
    rbd_info_map_lock.lock();
    std::map<std::string, rbd_data*>::iterator it = rbd_info_map.find(rbd_name);
    if( it == rbd_info_map.end() ){
        r = _open( rbd_name, pool_name );
        if( r != 0 ){
            rbd_info_map_lock.unlock();
            return r;
        }
        it = rbd_info_map.find(rbd_name);
    }
    rbd_info_map_lock.unlock();
    r = _write( it->second, offset, length, data, onfinish );
    return r;
}

int BackendStore::read( std::string rbd_name, uint64_t offset, uint64_t length, char* data, std::string pool_name ){
    int r = 0;
    rbd_info_map_lock.lock();
    std::map<std::string, rbd_data*>::iterator it = rbd_info_map.find(rbd_name);
    if( it == rbd_info_map.end() ){
        //printf("%s not open, open now\n", rbd_name.c_str());
        r = _open( rbd_name, pool_name );
        if( r != 0 ){
            rbd_info_map_lock.unlock();
            return r;
        }
        it = rbd_info_map.find(rbd_name);
    }
    rbd_info_map_lock.unlock();
    r = _read( it->second, offset, length, data );
    return r;
}

int BackendStore::_write( std::string rbd_name, uint64_t offset, uint64_t length, const char* data, C_AioBackendCompletion* onfinish ){
    std::map<std::string, rbd_data*>::iterator it = rbd_info_map.find(rbd_name);
    rbd_data* rbd = it->second;
    return _write( rbd, offset, length, data, onfinish );
}

int BackendStore::_write( rbd_data* rbd, uint64_t offset, uint64_t length, const char* data, C_AioBackendCompletion* onfinish ){
    rbd_aio_unit *io_u = new rbd_aio_unit( onfinish );
    //log_print("BackendStore::_write offset:%lu, length:%lu\n", offset, length);
    int r = rbd_aio_create_completion(io_u, rbc_backend_finish_aiocb, &io_u->completion);
    r = rbd_aio_write2(rbd->image, offset, length, data, io_u->completion, 1 );
    if (r < 0) {
        log_err("queue rbd_aio_write failed.\n");
        return -1;
    }
    return 0;
}

int BackendStore::_read( std::string rbd_name, uint64_t offset, uint64_t length, char* data ){
    std::map<std::string, rbd_data*>::iterator it = rbd_info_map.find(rbd_name);
    rbd_data* rbd = it->second;
    return _read(rbd, offset, length, data);
}

int BackendStore::_read( rbd_data* rbd, uint64_t offset, uint64_t length, char* data ){
    //printf("do rbd_read\n");
    int r = rbd_read2(rbd->image, offset, length, data, 1 );
    if (r < 0) {
        log_err("rbd_read failed.\n");
        return -1;
    }
    return 0;
}
}
