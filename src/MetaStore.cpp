#include "rbc/MetaStore.h"

using namespace rocksdb;


namespace rbc {
MetaStore::MetaStore(std::string dbpath)
{
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;
  Status s = DB::Open(options, dbpath, &db);
  if (!s.ok())
  {
    return;
  }
  return;
}

MetaStore::~MetaStore(){
  if ( db )
    delete db;
}

int MetaStore::get(const char* oid, char* data){
    std::string value;
    Status s = db->Get(ReadOptions(), oid, &value);
    if (!s.ok()){
        return -1;
    }
    data = const_cast<char*>(value.c_str());
    return 0;

}

int MetaStore::put(const char* oid, const char* dkey){
    std::stringstream ss;
    std::string value;
    ss << dkey;
    ss >> value;

    Status s = db->Put(WriteOptions(), oid, value);
    if (!s.ok()){
        return -1;
    }
    return 0;
}
}
