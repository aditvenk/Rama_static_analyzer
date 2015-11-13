# first perform mem2reg
opt -mem2reg < simple_tester.bc > simple_tester.optbc
llvm-dis simple_tester.optbc 
# run Rama on optbc
opt -load /scratch/neelakandan/tools/llvm-3.8-build/lib/LLVMInlineAnalysis.so -rama_inline < simple_tester.optbc > simple_tester.optbc_inline  2> out.txt
llvm-dis simple_tester.optbc_inline 
