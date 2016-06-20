/*Please complile by
  g++ WorkQueue.cpp -o test -lboost_thread -lboost_system -lpthread
 */
#include "lib/boost/threadpool/threadpool.hpp"
#include <stdio.h>
namespace rbc {
typedef boost::threadpool::pool ThreadPool;
}
