# first perform mem2reg
opt -mem2reg < tid_overlap_array.bc > tid_overlap_array.optbc
# run inline anlysis on optbc
opt -load /scratch/adityav/llvm_build/lib/LLVMInlineAnalysis.so -rama_inline -always-inline < tid_overlap_array.optbc > tid_overlap_array.optbc_inline
llvm-dis tid_overlap_array.optbc_inline 

# run Rama on optbc
opt -load /scratch/adityav/llvm_build/lib/LLVMPointerAnalysis.so -rama < tid_overlap_array.optbc_inline
