all: emit

debug: build_debug emit

build: tid_overlap_array.cpp
	clang++ -O0 -g tid_overlap_array.cpp -o tid_overlap_array -lpthread

build_debug: tid_overlap_array.cpp
	clang++ tid_overlap_array.cpp -o tid_overlap_array -g -lpthread


emit: tid_overlap_array.cpp
	clang++ -Wmain -O0 -emit-llvm tid_overlap_array.cpp -c -o tid_overlap_array.bc
	clang++ -Wmain -O0 -emit-llvm tid_overlap_array.cpp -S -o tid_overlap_array.ll

clean:
	rm -rf tid_overlap_array tid_overlap_array.bc tid_overlap_array.ll tid_overlap_array.optbc tid_overlap_array.optbc.ll

