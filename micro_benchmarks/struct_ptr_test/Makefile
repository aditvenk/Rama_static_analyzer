all: emit

debug: emit

build: struct_ptr_test.cpp
	clang++ -O0 -g struct_ptr_test.cpp -o struct_ptr_test -lpthread

build_debug: struct_ptr_test.cpp
	clang++ struct_ptr_test.cpp -o struct_ptr_test -g -lpthread


emit: struct_ptr_test.cpp
	clang++ -Wmain -O0 -emit-llvm struct_ptr_test.cpp -c -o struct_ptr_test.bc
	clang++ -Wmain -O0 -emit-llvm struct_ptr_test.cpp -S -o struct_ptr_test.ll

clean:
	rm -rf struct_ptr_test struct_ptr_test.bc struct_ptr_test.ll struct_ptr_test.optbc struct_ptr_test.optbc.ll

