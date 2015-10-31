#include <cstdlib>
#include <iostream>
#include <pthread.h>

#define NUM_THREADS 2
struct worker_args_t {
  int tid; 
};

int work_array[NUM_THREADS];

void* worker_func ( void* args) {
  worker_args_t* my_args = (worker_args_t*) args;
  int my_tid = my_args->tid;
  int i;

  work_array[my_tid] = 0;
  for (i=0; i<5; i++) {
    work_array[my_tid] += i;
  }
  return NULL;
}

int main() {
  pthread_t t[NUM_THREADS];
  worker_args_t wargs[NUM_THREADS];
  int i;

  for (i=0; i<NUM_THREADS; i++) {
    wargs[i].tid = i;
    pthread_create(&t[i], NULL, &worker_func, (void*) &wargs[i]);
  }
  
  for (i=0; i<NUM_THREADS; i++) {
    pthread_join(t[i], NULL);
  }

  return 0;
}
