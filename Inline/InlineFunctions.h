//===----------------------------------------------------------------------===//
//
// This pass inlines all functions that are not main or a pthread_create
//
//===----------------------------------------------------------------------===//

// Note that this is a ModulePass and not a FunctionPass - also defined in inlineFunctions namespace

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/CallGraph.h"
#include "string.h"
#include "stdio.h"

#include <set>
#include <iostream>
#include <fstream>
#include <cstdio>

#ifndef INLINEFUNCTIONS_H
#define INLINEFUNCTIONS_H

using namespace llvm;
using namespace std;

class InlineFunctions : public ModulePass {
	public:
		static char ID; // Pass identification, replacement for typeid
		InlineFunctions() : ModulePass(ID) { }

		//**********************************************************************
		// runOnModule
		//**********************************************************************
		virtual bool runOnModule (Module &M) {
			// print fn name
			cerr << "Module " << M.getModuleIdentifier() << "\n";
			// initial pass of algoritm to inline all functions in the module
			inlineFnsInModule (M);
			return false;  // because we have NOT changed this function
		}

		//**********************************************************************
		// print (do not change this method)
		//
		// If this pass is run with -f -analyze, this method will be called
		// after each call to runOnFunction.
		//**********************************************************************
		virtual void print(std::ostream &O, const Module *M) const {
			O << "This is abstractAnalysis.\n";
		}

		//**********************************************************************
		// getAnalysisUsage
		//**********************************************************************

		// We don't modify the program, so we preserve all analyses
		virtual void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.setPreservesAll();
			AU.addRequired<CallGraphWrapperPass>();
		};

	private:
		//defined in processFunction.cpp
		bool inlineFnsInModule (Module &);
};
#endif
