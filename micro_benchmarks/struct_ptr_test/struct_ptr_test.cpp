// In this program we simulate a case in which two threads
// use a global pointer that takes different addresses in
// different threads. Existing multithreaded program analysis
// that is thread-insesitive will conclude over-approximate
// whereas our analysis will show that the two threads access
// different locations.
// main () represents the code executed by the two threads.

#include <pthread.h>

#define NUMTHREADS  2
#define THREAD_IDENTIFIER id
typedef struct {
 int d;
 bool e;
 } mystruct;

 
mystruct a[10];
 
pthread_t threads[NUMTHREADS];
int ids[NUMTHREADS];

int w_func_tid (int THREAD_IDENTIFIER) {
  int x;
  int y;
  x = 3;
  y = 7;
	if (THREAD_IDENTIFIER == 0) {
    a[x].d = 5;
	} 
	if (THREAD_IDENTIFIER == 1) {
		a[y].d = 6;
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

