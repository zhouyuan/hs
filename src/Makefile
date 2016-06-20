LIBRBC_VERSION = 0.1.0

CC=cc
CXX=g++
PLATFORM=OS_LINUX
OPTIMIZATION?=-O2
OPT=$(OPTIMIZATION)
CXXFLAGS=-std=c++11 ${OPT} -fPIC -g -D__STDC_FORMAT_MACROS -DLIBRBC_VERSION='"${LIBRBC_VERSION}"'
CCFLAGS=${OPT} -fPIC -g -D__STDC_FORMAT_MACROS -DLIBRBC_VERSION='"${LIBRBC_VERSION}"'
PLATFORM_LDFLAGS=-lrocksdb -lpthread -lboost_thread -lboost_system -lrados -lrbd
INCS=-I./include 

DIST_LIB = librbc.so

.PHONY : all 
all: $(DIST_LIB)

%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

%.o : %.c
	${CC} -c ${CCFLAGS} ${INCS} $< -o $@

COMMON_OBJECTS:= CacheService.o CacheEntry.o MetaStore.o AgentService.o BackendStore.o BlockCacheClient.o SimpleBlockCacher.o BlockCacher.o ufc.o BufferList.o

$(DIST_LIB): $(COMMON_OBJECTS)
	${CXX} ${CXXFLAGS} ${INCS} librbc.cpp -shared -o $@ $^ ${PLATFORM_LDFLAGS}

test: $(DIST_LIB)
	${CXX} ${CXXFLAGS} ${INCS} test_librbc.cpp -o $@ $^ ${PLATFORM_LDFLAGS}

bench: $(DIST_LIB)
	${CXX} ${CXXFLAGS} ${INCS} bench_librbc.cpp -o $@ $^ ${PLATFORM_LDFLAGS}

bench_lru:
	g++ bench_lru.cpp -o bench_lru -lpthread -lboost_thread -lboost_system -std=c++11

install:
	cp librbc.so /usr/lib/
	cp librbc.so /usr/local/lib/
	cp -r include/rbc /usr/include/
	cp -r include/rbc /usr/local/include/

uninstall:
	rm -rf /usr/lib/librbc.so
	rm -rf /usr/local/lib/librbc.so
	rm -rf /usr/local/include/rbc
	rm -rf /usr/include/rbc

clean:
	rm -rf *.o *.so test bench
