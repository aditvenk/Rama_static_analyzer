# first perform mem2reg
opt -mem2reg < struct_ptr_test.bc > struct_ptr_test.optbc
# generate human readable ll file
llvm-dis struct_ptr_test.optbc 
# run Rama on optbc
opt -load /scratch/adityav/llvm_build/lib/LLVMPointerAnalysis.so -rama < struct_ptr_test.optbc
