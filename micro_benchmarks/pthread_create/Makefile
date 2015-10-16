all: build emit

build: pointer_analysis.cpp
	clang++ pointer_analysis.cpp -o pointer_analysis -lpthread

emit: pointer_analysis.cpp
	clang++ -O0 -emit-llvm pointer_analysis.cpp -c -o pointer_analysis.bc 
	opt -mem2reg pointer_analysis.bc > pointer_analysis.opt
	llvm-dis < pointer_analysis.opt > pointer_analysis.ll

clean:
	rm -rf pointer_analysis pointer_analysis.bc pointer_analysis.ll

