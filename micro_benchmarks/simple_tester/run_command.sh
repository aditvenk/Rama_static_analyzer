# first perform mem2reg
opt -mem2reg < simple_tester.bc > simple_tester.optbc
# generate human readable ll file
llvm-dis simple_tester.optbc 
# run Rama on optbc
if [[ $USER == "neelakandan" ]]
then 
	opt -load /scratch/neelakandan/tools/llvm-3.8-build/lib/LLVMPointerAnalysis.so -rama -mem2reg < simple_tester.optbc 
else
	opt -load /scratch/adityav/llvm_build/lib/LLVMPointerAnalysis.so -rama < simple_tester.optbc
fi 
