all: build emit

debug: build_debug emit

build: simple_tester.cpp
	clang++ simple_tester.cpp -o simple_tester -lpthread

build_debug: simple_tester.cpp
	clang++ simple_tester.cpp -o simple_tester -g -lpthread


emit: simple_tester.cpp
	clang++ -O0 -emit-llvm simple_tester.cpp -c -o simple_tester.bc
	clang++ -O0 -emit-llvm simple_tester.cpp -S -o simple_tester.ll

clean:
	rm -rf simple_tester simple_tester.bc simple_tester.ll simple_tester.optbc simple_tester.optbc.ll simple_tester.optbc_inline simple_tester.optbc_inline.ll simple_tester.optbc2 simple_tester.optbc_inlined simple_tester.optbc_inlined.ll simple_tester.optbc2.ll out.txt

