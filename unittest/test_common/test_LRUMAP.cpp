#include <iostream>
#include <string>
#include <stdlib.h>
#include "../../common/LRU_MAP.h"
#define LRU_LIST_LENGTH 10

using std::cout;
using std::endl;
using std::string;


string fun_value(const char &key){
    string value = (string) &key;
    return value;
}

class TEST_LRU_MAP{
public:

    TEST_LRU_MAP():test_lru_map(fun_value, LRU_LIST_LENGTH){};
    int test_p_case1();
    int test_p_case2();

private:

    LRU_MAP<char, string> test_lru_map;

};

int TEST_LRU_MAP::test_p_case1(){
    char key1 = 'a';
    string value1 = "one";
    char &key_ref1 = key1;
    string &value_ref1 = value1;
    char key2 = 'b';
    string value2 = "two";
    char *pointer = &key2;
    test_lru_map.insert(key_ref1, value_ref1);
    test_lru_map.get_keys(pointer);
    if(key2 == 'a'){
        cout << "get latest key: " << key2 << endl;
    }else{
        cout << "error: positive test case 1 key failed" << endl;
        return 1;
    }
    value2 = test_lru_map.get_value(key_ref1);
    if(value2 == "one"){
        cout << "get latest value: " << value2 << endl;
        return 0;
    }else{
        cout << "error: positive test case 1 value failed" << endl;
        return 1;
    }
}

int TEST_LRU_MAP::test_p_case2(){
    char keys[10] = {'b','c','d','e','f','g','h','i','j','k'};
    char (&keys_ref)[10] = keys;
    string values[10] = {"two","three","four","five","six","seven","eight","nine","ten","eleven"};
    string (&values_ref)[10] = values;
    string value1 = "one";
    string value2 = "one";
    char key1[10] = {'m'};
    char *pointer = key1;
    for(int i = 0; i < 10; i++){
        test_lru_map.insert(keys_ref[i], values_ref[i]);
    }
    test_lru_map.get_keys(pointer);
    char &key_ref1 = key1[0];
    char &key_ref2 = key1[9];
    value1 = test_lru_map.get_value(key_ref1);
    value2 = test_lru_map.get_value(key_ref2);
    if((key1[0] == 'k')&&(key1[9] == 'b')&&(value1 == "eleven")&&(value2 == "two")){
        cout << "latest key: " << key1[0] << " value is : " << value1 << endl;
        cout << "oldest key: " << key1[9] << " value is : " << value2 << endl;
        return 0;
    }else{
        cout << "error: positive test case 2 failed" << endl;
        return 1;
    }
}

int main(){
    int flag;
    cout << "initiate LRU list" << endl;
    TEST_LRU_MAP lru_map;
    cout << "begin test positive case 1" << endl;
    flag = lru_map.test_p_case1();
    cout << "flag = " << flag << endl;
    cout << "begin test positive case 2" << endl;
    flag = lru_map.test_p_case2();
    cout << "flag = " << flag << endl;
    return 0;
}
