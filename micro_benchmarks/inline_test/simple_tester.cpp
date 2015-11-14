#include <cstdlib>
#include <iostream>
#include <pthread.h>

using namespace std;

int work_array[1000];

int a_func(int param){
	int var;
	cin>>var;
	if(var > param)
		return var;
	else
		return param;
}

int b_func(int param){
	int var;
	cin>>var;
	if(var > param)
		return param;
	else
		return var;
}

bool c_func(int param){
	int var = 0;
	if(var == param)
		return true;
	else
		return false;
}

void* worker_func(void* tid_ptr) {
	int id = *(int*) tid_ptr;
	work_array[id] = 5;
	return NULL;
}

void* worker_func_2(void* tid_ptr) {
	int id = *(int*) tid_ptr;
	int in;
	cin>>in;	
	work_array[id] = (int)c_func(in);
	return NULL;
}

int main(){
	int temp = a_func(4);
	int temp_2 = b_func(0);
  	int tid = temp + temp_2;
  	pthread_t t;
    	pthread_create(&t, NULL, &worker_func, (void*) &tid);
  	pthread_t t_2;
  	tid++;
    	pthread_create(&t, NULL, &worker_func_2, (void*) &tid);
	return work_array[tid];
}
