/* This microbenchmark demonstrates how Rama can detect tid var and use that to figure out how different threads are accessing overlapping chunks of a common work array. Non-tid sensitive analysis will be more over-approximate */

#include<pthread.h>

#define NUMTHREADS 2
#define ARRAYSIZE 32
#define CHUNKSIZE 16

// use this macro to quickly change which var name we use for thread_identifier
#define THREAD_IDENTIFIER   tid

int a [ARRAYSIZE];
pthread_t threads[NUMTHREADS];
int ids[NUMTHREADS];

int w_func_tid (int THREAD_IDENTIFIER) {
  int i;
  int j;
  j = THREAD_IDENTIFIER*CHUNKSIZE;
  for (i=0; i<CHUNKSIZE; i++) {
    if (j > 0) {
      a[j+i] = a[j-1] + a[j+i];
    }
    else {
      a[j+i] = a[j] + a[j+i];
    }
  }
  return 0;
}
void* w_func (void* args) {
  int id = *(int*) args;
  w_func_tid(id);
}

int main () {
  int i;
  for(i=0; i<NUMTHREADS; i++) {
    ids[i] = i;
    pthread_create( &threads[i], NULL, &w_func, (void*) &ids[i]);
  }

  for(i=0; i<NUMTHREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  return 0;
}
