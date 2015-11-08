#include <cstdlib>
#include <iostream>
#include <pthread.h>

#define NUM_THREADS 2

int work_array[NUM_THREADS];
int tids[NUM_THREADS];

void* worker_func ( void* tid_ptr) {
  int id = *(int*) tid_ptr;
  work_array[tid] = 5;
  return NULL;
}

int main() {
  int i;
  int th[2];
  th[0] = 0;
  th[1] = 1;
  pthread_t t[NUM_THREADS];
  for (i=0; i<NUM_THREADS; i++) {
    pthread_create(&t[i], NULL, &worker_func, (void*) &th[i]);
  }
  
  for (i=0; i<NUM_THREADS; i++) {
    pthread_join(t[i], NULL);
  }

  return 0;
}
