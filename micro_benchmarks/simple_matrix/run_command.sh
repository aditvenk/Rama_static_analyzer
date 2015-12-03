# first perform mem2reg
opt -mem2reg < simple_matrix.bc > simple_matrix.optbc
# perform safecode pass
opt -load /scratch/neelakandan/tools/llvm-3.8-build/lib/LLVMSafeAnalysis.so -break-constgeps < simple_matrix.optbc > simple_matrix.optbc_safe
# run inline anlysis on optbc
opt -load /scratch/neelakandan/tools/llvm-3.8-build/lib/LLVMInlineAnalysis.so -rama_inline -always-inline < simple_matrix.optbc_safe > simple_matrix.optbc_inline
llvm-dis simple_matrix.optbc_inline 

# run Rama on optbc
opt -load /scratch/neelakandan/tools/llvm-3.8-build/lib/LLVMPointerAnalysis.so -rama < simple_matrix.optbc_inline
