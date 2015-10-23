#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <pthread.h>

using namespace std;

struct worker_args_t {
  int tid;
};

pthread_t t[2];

void displayMessage (int tid) {
  cout<<"Worker "<<tid<<" saying hello"<<endl;
}

void* workerFunction (void* args) {
  worker_args_t* my_args = (worker_args_t*) args;
  displayMessage(my_args->tid);
  return NULL;
}

int main() {
  worker_args_t wargs[2];
  int tid=0;

  for (int i=0; i<2; i++) {
    tid = i;
    wargs[i].tid = i+tid;
  }
  int *a = (int*) malloc(sizeof(int));
  

  int x = wargs[0].tid;
  
  for (int i=0; i<2; i++) {
    pthread_create(&t[i], NULL, &workerFunction, (void*) &wargs[i]);
  }

  for (int i=0; i<2; i++) {
    pthread_join(t[i], NULL);
  }
  free(a);

  return tid;
}

