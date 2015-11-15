#include<pthread.h>

#define NUMTHREADS 4
#define ARRAYSIZE 32
#define CHUNKSIZE 8 

int a [ARRAYSIZE];

int main (int tid) {
	int i;
  int j;
  j = tid*CHUNKSIZE;
  for (i=0; i<CHUNKSIZE; i++) {
    a[j] = a[j] + 1;
  }

  return 0;
}
