all: emit

debug: build_debug emit

build: tid_worker_func.cpp
	clang++ -O0 -g tid_worker_func.cpp -o tid_worker_func -lpthread

build_debug: tid_worker_func.cpp
	clang++ tid_worker_func.cpp -o tid_worker_func -g -lpthread


emit: tid_worker_func.cpp
	clang++ -Wmain -O0 -emit-llvm tid_worker_func.cpp -c -o tid_worker_func.bc
	clang++ -Wmain -O0 -emit-llvm tid_worker_func.cpp -S -o tid_worker_func.ll

clean:
	rm -rf tid_worker_func tid_worker_func.bc tid_worker_func.ll tid_worker_func.optbc tid_worker_func.optbc.ll

