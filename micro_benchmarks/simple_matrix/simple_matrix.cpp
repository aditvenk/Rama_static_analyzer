#include <pthread.h>
#include <math.h>

#define NUMTHREADS 2
#define ROWS 2
#define COLUMNS 2
#define ITERATIONS 1

// use this macro to quickly change which var name we use for thread_identifier
//#define THREAD_IDENTIFIER  tid
#define THREAD_IDENTIFIER  id

int matrix[ROWS][COLUMNS];
int mat[COLUMNS];
pthread_t threads[NUMTHREADS];
int ids[NUMTHREADS];

int matrix_mult_tid(int THREAD_IDENTIFIER){
	int i;
	for(i=0; i<ITERATIONS; ++i){
		//matrix[0][THREAD_IDENTIFIER] = sqrt(matrix[0][THREAD_IDENTIFIER]);
		//matrix[1][THREAD_IDENTIFIER] = sqrt(matrix[1][THREAD_IDENTIFIER]);
		matrix[THREAD_IDENTIFIER][0] = sqrt(matrix[THREAD_IDENTIFIER][0]);
		matrix[THREAD_IDENTIFIER][1] = sqrt(matrix[THREAD_IDENTIFIER][1]);
	}
}

void* worker(void* args) {
	int id = *(int*) args;
	matrix_mult_tid(id);
}

int main() {
	// init matrix
	/*
	int r, c;
	for(r=0; r<ROWS; ++r){
		for(c=0; c<COLUMNS; ++c){
			matrix[r][c] = 1;	
		}	
	}
	A[0] = M_PI/2;
	A[1] = M_PI/4;
	B[0] = M_PI/8;
	B[1] = M_PI/16;
	*/
	int i;
	for(i=0; i<NUMTHREADS; i++) {
		ids[i] = i;
		pthread_create(&threads[i], NULL, &worker, (void*) &ids[i]);
	}
	for(i=0; i<NUMTHREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	return 0;
}
