all: emit

debug: build_debug emit

build: simple_multi_tester.cpp
	clang++ -O0 -g simple_multi_tester.cpp -o simple_multi_tester -lpthread

build_debug: simple_multi_tester.cpp
	clang++ simple_multi_tester.cpp -o simple_multi_tester -g -lpthread


emit: simple_multi_tester.cpp
	clang++ -Wmain -O0 -emit-llvm simple_multi_tester.cpp -c -o simple_multi_tester.bc
	clang++ -Wmain -O0 -emit-llvm simple_multi_tester.cpp -S -o simple_multi_tester.ll

clean:
	rm -rf simple_multi_tester simple_multi_tester.bc simple_multi_tester.ll simple_multi_tester.optbc simple_multi_tester.optbc.ll

