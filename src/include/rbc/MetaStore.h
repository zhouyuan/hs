#ifndef METASTORE_H
#define METASTORE_H

#include <stdio.h>
#include <string>
#include <sstream>

#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>

namespace rbc {
class MetaStore {

public:
  MetaStore();
  ~MetaStore();

  int get(const char* oid, char* data);
  int put(const char* oid, const char* dkey);

private:
  rocksdb::DB *db;
  rocksdb::Options options;

};
}

#endif
