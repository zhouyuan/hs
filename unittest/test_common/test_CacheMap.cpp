#include <iostream>
#include <stdlib.h>
#include "../../common/CacheMap.h"

using std::cout;
using std::endl;

uint64_t change_value(const char &key){
    uint64_t value = (uint64_t) key;
    return value;
}

class TEST_CACHE_MAP{

public:

    TEST_CACHE_MAP():test_cache_map(change_value){};
    int test_p_case1();
    int test_p_case2();
    int test_p_case3();
    int test_p_case4();

private:

    CacheMap<char, uint64_t> test_cache_map;
};

int TEST_CACHE_MAP::test_p_case1(){
    char key = 'a';
    uint64_t value = 1;
    char &key_ref = key;
    uint64_t &value_ref = value;
    uint64_t value_idf;
    test_cache_map.insert(key_ref, value_ref);
    value_idf = test_cache_map.get_value(key_ref);
    if(1 == value_idf){
        cout << "get key value: " << value_idf << endl;
        return 0;
    }else{
        cout << "error: positive test case 1 failed" << endl;
        return 1;
    }
}

int TEST_CACHE_MAP::test_p_case2(){
    char keys[5] = {'b','c','d','e','f'};
    char (&keys_ref)[5] = keys;
    uint64_t values[5] = {2,3,4,5,6};
    uint64_t (&values_ref)[5] = values;
    uint64_t value_return;
    for(int i = 0; i < 5; i++){
        test_cache_map.insert(keys_ref[i], values_ref[i]);
    }
    value_return = test_cache_map.get_value(keys_ref[2]);
    if(4 == value_return){
        cout << "get key value: " << value_return << endl;
        return 0;
    }else{
        cout << "error: positive test case 2 failed" << endl;
        return 1;
    }
}

int TEST_CACHE_MAP::test_p_case3(){
    char key = 'd';
    char (&key_ref) = key;
    uint64_t value;
    test_cache_map.evict(key_ref);
    value = test_cache_map.get_value(key_ref);
    if(value != 4){
        cout << "evict value successfully" << endl;
        return 0;
    }else{
        cout << "error: positive test case 3 failed" << endl;
        return 1;
    }
}

int TEST_CACHE_MAP::test_p_case4(){
    char key = 'g';
    char (&key_ref) = key;
    uint64_t value;
    value = test_cache_map.get_value(key_ref);
    if(103 == value){
        cout << "get value: " << value << endl;
        return 0;
    }else{
        cout << "error: positive test case 4 failed" << endl;
        return 1;
    }
}

int main(){
    int flag;
    cout << "initiate" << endl;
    TEST_CACHE_MAP cache_map;
    cout << "begin test positive case 1" << endl;
    flag = cache_map.test_p_case1();
    cout << "flag = " << flag << endl;
    cout << "begin test positive case 2" << endl;
    flag = cache_map.test_p_case2();
    cout << "flag = " << flag << endl;
    cout << "begin test positive case 3" << endl;
    flag = cache_map.test_p_case3();
    cout << "flag = " << flag << endl;
    cout << "begin test positive case 4" << endl;
    flag = cache_map.test_p_case4();
    cout << "flag = " << flag << endl;
    return 0;
}
