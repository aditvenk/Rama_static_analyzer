#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Analysis/CallGraph.h"

#include "InlineFunctions.h"

// Identifier variable for the pass
char InlineFunctions::ID = 0;

// Register the pass
static RegisterPass<InlineFunctions> X("inline", "inline functions", true, false);

// define the inline function called as a part of the runonmodule callback
bool InlineFunctions::inlineFnsInModule (Module &M) {
	// Main is now a handle to the pthread_create function object.
	// Can there not be more than one pthread_create in the module?
	Function *Main = M.getFunction ("pthread_create");
	if (Main == 0) {
		cerr << "pthread_create not found\n";
		return false;
	}
	cerr << "pthread_create found = " << Main->getName().str() << std::endl; 
	std::set <Function*> threads; // Holds Function*s that are called within the body of pthread_create - why is this called threads? - not every function here from a new thread?
	// value is a superclass of User (instruction) and Function
	Value::use_iterator vu_it;
	// Iterate over all uses(instructions) of the pthread_create and collect the thread functions 
	// called from there.
	for (vu_it = Main->use_begin (); vu_it != Main->use_end (); vu_it++) {
		if ( CallInst* callInst = dyn_cast <CallInst> (*vu_it) ) {
			// should we check for declarations and NULL functions and ignore them?
			//if ( !callInst->isDeclaration() ) {
			// fn_arg is the function being called by the callInst
			Value* fn_arg = callInst->getOperand (3);
			cerr << "found a callinst of pthread_create; 3rd arg = " << fn_arg->getName().str() << std::endl;
			// insert into the threads set
			threads.insert ( M.getFunction (fn_arg->getName ()) );
			//}
		} else {
			cerr << "Use of pthread_create was not a CallInst!\n";
		}
	}

	CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();

	// Performs inlining of the main module
	// Create a reverse post order list of functions in the program (the functions in the program taken together represent the call graph)
	// The reverse of a post order traversal is a pre-order traversal where the right subtree is visited first
	ReversePostOrderTraversal < CallGraph* > RPOT (&CG); //, CG[M.getFunction ("pthread_create")]);
	ReversePostOrderTraversal < CallGraph* >::rpo_iterator rpo_it;
	// this will hold the functions which are eligible for inlining
	std::vector < Function* > rpo_fns;
	// iterate over the functions in a reverse post order manner
	for (rpo_it = RPOT.begin (); rpo_it != RPOT.end (); rpo_it++) {
		Function* F = (*rpo_it)->getFunction ();
		if (F != NULL) {
			cerr << "rpo fn = " << F->getName().str() << std::endl;
			// collect all functions that are not pthread_create nor the starting thread functions
			// these are the functions that will be marked for inlining
			if ( (F->getName () != "pthread_create") & (threads.find ( F ) == threads.end ()) ) {
				rpo_fns.push_back ( F );
			}
		}
	}

	// Now go through the list and mark all functions, except the "main" function
	// for inlining. - where does the actual inlining take place?
	for (unsigned j = 1; j < rpo_fns.size(); j++) {
		const AttributeSet attr = rpo_fns[j]->getAttributes(); 
		const AttributeSet attr_new = attr.addAttribute(rpo_fns[j]->getContext(), ~0U, Attribute::AlwaysInline); 
		// it appears that just setting this AlwaysInline attribute on this function is enough to automatically make llvm inline this
		rpo_fns[j]->setAttributes(attr_new); 
		cerr << "Marked function " << rpo_fns[j]->getName().str() << " for inlining\n";
		//llvm::BasicInliner::addFunction ( rpo_fns[j] );
	}
	return false;
}
