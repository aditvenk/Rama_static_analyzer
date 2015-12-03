all: emit

debug: build_debug emit

build: simple_matrix.cpp
	clang++ -O0 -g simple_matrix.cpp -o simple_matrix -lpthread

build_debug: simple_matrix.cpp
	clang++ simple_matrix.cpp -o simple_matrix -g -lpthread


emit: simple_matrix.cpp
	clang++ -Wmain -O0 -emit-llvm simple_matrix.cpp -c -o simple_matrix.bc
	clang++ -Wmain -O0 -emit-llvm simple_matrix.cpp -S -o simple_matrix.ll

clean:
	rm -rf simple_matrix simple_matrix.bc simple_matrix.optbc_safe simple_matrix.optbc_inline simple_matrix.ll simple_matrix.optbc_inline.ll simple_matrix.optbc simple_matrix.optbc.ll
