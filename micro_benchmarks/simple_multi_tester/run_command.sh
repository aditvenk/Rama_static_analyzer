# first perform mem2reg
opt -mem2reg < simple_multi_tester.bc > simple_multi_tester.optbc
# generate human readable ll file
llvm-dis simple_multi_tester.optbc 
# run Rama on optbc
opt -load /scratch/adityav/llvm_build/lib/LLVMPointerAnalysis.so -rama < simple_multi_tester.optbc
