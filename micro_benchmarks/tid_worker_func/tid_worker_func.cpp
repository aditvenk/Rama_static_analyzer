#include<pthread.h>

#define NUMTHREADS 4
#define ARRAYSIZE 32
#define CHUNKSIZE 8 

int a [ARRAYSIZE];
pthread_t threads[NUMTHREADS];
int ids[NUMTHREADS];
void call_func (void (*f)(int), int param);

void w_func_tid (int tid) {
  int i;
  int j;
  j = tid*CHUNKSIZE;
  for (i=0; i<CHUNKSIZE; i++) {
    a[j+i] = a[j] + a[j+i];
  }
}

void call_func ( void (*f) (int), int param) {
  f(param);
}

void* w_func (void* args) {
  int id = *(int*) args;
  w_func_tid(id);

  call_func( &w_func_tid, id);

}

int main () {
  int i;
  for(i=0; i<NUMTHREADS; i++) {
    ids[i] = i;
    pthread_create( &threads[i], NULL, &w_func, (void*) &ids[i]);
    call_func(&w_func_tid, ids[i]);
  }

  for(i=0; i<NUMTHREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  return 0;
}
