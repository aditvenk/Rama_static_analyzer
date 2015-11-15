# first perform mem2reg
opt -mem2reg < tid_worker_func.bc > tid_worker_func.optbc
# generate human readable ll file
llvm-dis tid_worker_func.optbc 
# run Rama on optbc
opt -load /scratch/adityav/llvm_build/lib/LLVMPointerAnalysis.so -rama < tid_worker_func.optbc
