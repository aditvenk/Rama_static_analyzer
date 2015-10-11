all: build emit

build: pointer_analysis.cpp
	clang++ pointer_analysis.cpp -o pointer_analysis

emit: pointer_analysis.cpp
	clang++ -O3 -emit-llvm pointer_analysis.cpp -c -o pointer_analysis.bc
	clang++ -O3 -emit-llvm pointer_analysis.cpp -S -o pointer_analysis.ll

clean:
	rm -rf pointer_analysis pointer_analysis.bc pointer_analysis.ll

