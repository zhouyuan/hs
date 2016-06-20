#include <iostream>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <exception>
#include "../../common/LRU_List.h"
#define LRU_LIST_LENGTH 10
#define ENTRY_LIST_LENGTH 5
//using namespace std;
using std::cout;
using std::endl;
using std::string;


class TEST_LRU_LIST{
public:

	TEST_LRU_LIST():test_lru_list(LRU_LIST_LENGTH){};
	int test_p_case1();
    int test_p_case2();
	int test_p_case3();
	int test_p_case4();
	int test_p_case5();
	int test_p_case6();
	int test_p_case7();
	int test_p_case8();
	int test_p_case9();
	int test_n_case2();

private:
	LRU_LIST<string> test_lru_list;
	int test_length;
};

int TEST_LRU_LIST::test_p_case1(){
	string key1 = "first";
	string key2 = "second";
	string *pointer = &key2;
	string &key_ref1 = key1;
	test_length = 1;
	test_lru_list.insert(key_ref1);
	test_lru_list.get_keys(pointer, test_length);
	if(*pointer == key1){
		cout << "get current new key: " << *pointer << endl;
		return 0;
	}else{
	    cout << "error: positive test case 1 failed" << endl;
	    return 1;
	}

}

int TEST_LRU_LIST::test_p_case2(){
	string key_array1[5] = {"one","two","three","four","five"};
	string key_array2[5];
	string *pointer = key_array2;
	string (&key_ref2)[5] = key_array1;
	test_length = 5;
	for(int i = 0; i < 5; i++){
		test_lru_list.insert(key_ref2[i]);
	}
	test_lru_list.get_keys(pointer, test_length);
	if(key_array2[0] == key_array1[4]){
		cout << "get latest new key: " << key_array2[0] << endl;
		return 0;
	}else{
		cout << "error: positive test case 2 failed" << endl;
		return 1;
	}

}

int TEST_LRU_LIST::test_p_case3(){
	string key;
	string *pointer = &key;
	test_length = 1;
	test_lru_list.get_keys(pointer, test_length, false);
	if(*pointer == "first"){
		cout << "get oldest key: " << key << endl;
		return 0;
	}else{
		cout << "error: positive test case 3 failed" << endl;
		return 1;
	}
}

int TEST_LRU_LIST::test_p_case4(){
	string key1 = "five";
	string &key_ref = key1;
	string *pointer = &key1;
	test_length = 1;
	test_lru_list.remove(key_ref);
	test_lru_list.get_keys(pointer, test_length);
	if(*pointer == "four"){
		cout << "get current new key: " << key1 << endl;
		return 0;
	}else{
		cout << "error: positive test case 4 failed" << endl;
		return 1;
	}
}

int TEST_LRU_LIST::test_p_case5(){
	string key1 = "first";
	string &key_ref = key1;
	string *pointer = &key1;
	test_length = 1;
	test_lru_list.remove(key_ref);
	test_lru_list.get_keys(pointer, test_length, false);
	if(*pointer == "one"){
		cout << "get oldest key: " << key1 << endl;
		return 0;
	}else{
		cout << "error: positive test case 5 failed" << endl;
		return 1;
	}

}

int TEST_LRU_LIST::test_p_case6(){
	int current_length;
	current_length = test_lru_list.get_length();
	if(4 == current_length){
		cout << "get right list length" << endl;
		return 0;
	}else{
		cout << "error: positive test case 6 failed" << endl;
		return 1;
	}
}

int TEST_LRU_LIST::test_p_case7(){
	string key_array1[7] = {"five","six","seven","eight","nine","ten","eleven"};
	string (&key_ref2)[7] = key_array1;
	string key;
	string key1;
	string *pointer = &key;
	string *pointer1 = &key1;
	test_length = 1;
	for(int i = 0; i < 7; i++){
		test_lru_list.insert(key_ref2[i]);
	}
	test_lru_list.get_keys(pointer, test_length);
	test_lru_list.get_keys(pointer1, test_length, false);
	if(("eleven" == *pointer)&&("two" == *pointer1)){
		cout << "get latest key: " << *pointer << endl;
        cout << "get oldest key: " << *pointer1 << endl;
		return 0;
	}else{
		cout << "error: positive test case 7 failed" << endl;
		return 1;
	}

}

int TEST_LRU_LIST::test_p_case8(){
	string key = "twelve";
	string key1 = "one";
	string &key_ref = key;
	string *pointer = &key1;
	test_length = 1;
	test_lru_list.touch_key(key_ref);
	test_lru_list.get_keys(pointer, test_length);
	if("twelve" == *pointer){
		cout << "get latest key: " << *pointer << endl;
		return 0;
	}else{
		cout << "error: positive test case 8 failed" << endl;
		return 1;
	}
}

int TEST_LRU_LIST::test_p_case9(){
	string key = "five";
	string &key_ref = key;
	test_length = 10;
	string key_array[10];
	string *pointer = key_array;
	test_lru_list.touch_key(key_ref);
	test_lru_list.get_keys(pointer, test_length);
	if(("five" == key_array[0])&&(key_array[7] == "six")){
	    cout << "get latest key: " << key_array[0] << endl;
	    return 0;
	}else{
	    cout << "error: positive test case 9 failed" << endl;
	    return 1;
	}
}

int TEST_LRU_LIST::test_n_case2(){
	string key = "three";
	string &key_ref = key;
	test_lru_list.insert(key_ref);
}




int main(){
	int flag;
	cout << "initiate LRU list" << endl;
	TEST_LRU_LIST lru_list;
	cout << "begin test positive case 1" << endl;
	flag = lru_list.test_p_case1();
	cout << "flag = " << flag << endl;
	cout << "begin test positive case 2" << endl;
	flag = lru_list.test_p_case2();
	cout << "flag = " << flag << endl;
	cout << "begin test positive case 3" << endl;
	flag = lru_list.test_p_case3();
	cout << "flag = " << flag << endl;
	cout << "begin test positive case 4" << endl;
	flag = lru_list.test_p_case4();
	cout << "flag = " << flag << endl;
	cout << "begin test positive case 5" << endl;
	flag = lru_list.test_p_case5();
	cout << "flag = " << flag << endl;
	cout << "begin test positive case 6" << endl;
	flag = lru_list.test_p_case6();
	cout << "flag = " << flag << endl;
	cout << "begin test positive case 7" << endl;
	flag = lru_list.test_p_case7();
	cout << "flag = " << flag << endl;
	cout << "begin test positive case 8" << endl;
	flag = lru_list.test_p_case8();
	cout << "flag = " << flag << endl;
	cout << "begin test positive case 9" << endl;
	flag = lru_list.test_p_case9();
	cout << "flag = " << flag << endl;
	return 0;
}
