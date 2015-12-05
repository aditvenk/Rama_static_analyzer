#include <pthread.h>
#include <math.h>
#include <stdio.h>

#define NUMTHREADS 2
#define ROWS 2
#define COLUMNS 4
#define CHUNKSIZE COLUMNS/NUMTHREADS

// use this macro to quickly change which var name we use for thread_identifier
#define THREAD_IDENTIFIER  tid
//#define THREAD_IDENTIFIER  id

int matrix[ROWS][COLUMNS];
pthread_t threads[NUMTHREADS];
int ids[NUMTHREADS];

int matrix_mult_tid(int THREAD_IDENTIFIER){
	int j = THREAD_IDENTIFIER * CHUNKSIZE;
	int c = 0;
	for(c=j; c<j+(CHUNKSIZE); ++c){//hardcoding the 4 to save some llvm instructions
		for(int r=0;r<2; ++r){
			int right = c+1;
			int left = c-1;
			int rVal, lVal;
			if(!(right>COLUMNS-1)){
				rVal = matrix[r][right];
			}
			else{
				rVal = 0;
			}
			
			if(!(left<0)){
				lVal = matrix[r][left];
			}
			else{
				lVal = 0;
			}
			matrix[r][c] = rVal + lVal;
		}	
	}
}

/*
int matrix_mult_tid(int THREAD_IDENTIFIER){
	int j = THREAD_IDENTIFIER * CHUNKSIZE;
	int c = 0;
	for(c=j; c<4; ++c){//hardcoding the 4 to save some llvm instructions
		for(int r=0;r<2; ++r){
			int right = j+1;
			int left = j-1;
			int rVal, lVal;
			if(!(right>COLUMNS)){
				rVal = matrix[r][right];
			}
			else{
				rVal = 0;
			}
			if(!(left<0)){
				lVal = matrix[r][left];
			}
			else{
				lVal = 0;
			}
			matrix[r][c] = rVal + lVal;
		}	
	}
}
*/

void* worker(void* args) {
	int id = *(int*) args;
	matrix_mult_tid(id);
}

int main() {
	int i;
	for(i=0; i<NUMTHREADS; i++) {
		ids[i] = i;
		pthread_create(&threads[i], NULL, &worker, (void*) &ids[i]);
	}
	return 0;
}
