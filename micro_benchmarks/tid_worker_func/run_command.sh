# first perform mem2reg
opt -mem2reg < tid_worker_func.bc > tid_worker_func.optbc
# run inline anlysis on optbc
opt -load /scratch/adityav/llvm_build/lib/LLVMInlineAnalysis.so -rama_inline -always-inline < tid_worker_func.optbc > tid_worker_func.optbc_inline
llvm-dis tid_worker_func.optbc_inline 

# run Rama on optbc
opt -load /scratch/adityav/llvm_build/lib/LLVMPointerAnalysis.so -rama < tid_worker_func.optbc_inline
