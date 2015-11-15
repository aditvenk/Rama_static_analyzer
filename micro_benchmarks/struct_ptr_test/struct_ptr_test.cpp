// In this program we simulate a case in which two threads
// use a global pointer that takes different addresses in
// different threads. Existing multithreaded program analysis
// that is thread-insesitive will conclude over-approximate
// whereas our analysis will show that the two threads access
// different locations.
// main () represents the code executed by the two threads.

int x;
int y;

typedef struct {
 int d;
 bool e;
 } mystruct;

int main (int tid) {

 mystruct a[10];
  int *p;
  x = 3;
  y = 7;
	if (tid == 0) {
    a[x].d = 5;
	} 
	if (tid == 1) {
		a[y].d = 6;
	}
	return 0;
}
