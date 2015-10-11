#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

#include <fstream>
#include <iostream>
#include <cstdio>

#include "llvm/Support/raw_ostream.h"
using namespace llvm;
using namespace std;

#define DEBUG_TYPE "PointerAnalysis"

// Rama - a multi-threaded pointer analyzer built by Aditya Venkataraman and Naveen Neelakantan

namespace Rama {
  
  // PointerAnalysis 
  class PointerAnalysis : public ModulePass {
    private:
      // initial stuff
      bool doInitialization(Module &M);

      // clean up after all module runs
      bool doFinalization(Module &M);
      
      // all function analysis goes here
      bool analyzeFunctions(Module &M);

      // stuff for each function
      bool runOnModule(Module &M) {
        cerr << "Module " << M.getModuleIdentifier() << endl;
        analyzeFunctions (M);
        return false;
      }

    public:
      static char ID; // Pass identification, replacement for typeid
      PointerAnalysis() : ModulePass(ID) {}

      virtual const char *getPassName() const {
        return "Rama, multi-threaded pointer analyzer";
      }
      // TODO - Aditya - Find out what this function does and whether it is needed 
      // We don't modify the program, so we preserve all analyses.
      void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.setPreservesAll();
      }
  };
}

