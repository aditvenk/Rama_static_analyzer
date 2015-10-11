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
  
  // class op_info
  // This class holds the information for two entities:
  // 1. Information computed for each instruction
  // 2. Information parsed for each operand of an information
  class op_info {
    public:
      
      op_info(): isTID(false),
                 value(0),
                 valueFP(0.0),
                 size(1),
                 width(0),
                 name(""),
                 BasicBlockPtr(NULL),
                 FunctionPtr(NULL),
                 isLiteral(false),
                 isFixedPoint(false),
                 isInstruction(false),
                 isBasicBlockPtr(false),
                 isFunctionPtr(false),
                 isPointer(false),
                 isAlloca(false),
                 isArray(false),
                 isCall(false),
                 auxilliary_op(NULL),
                 try_widen(false)
      {
        // abstractDomain.clear(); // TODO - uncomment
      }

      ~op_info() {}

      bool isTID;
      int value;
      double valueFP; // TODO - AV - is this needed?
      int size;
      int width;
      string name;

      BasicBlock* BasicBlockPtr;
      Function* FunctionPtr;
      bool isLiteral;
      bool isFixedPoint;
      bool isInstruction;
      bool isBasicBlockPtr;
      bool isFunctionPtr;
      bool isPointer;
      bool isAlloca;
      bool isArray;
      bool isCall; // set if it is a call instruction. name will give the function name

      op_info* auxilliary_op;
      // abstractDomVec_t cmp_val; // TODO - uncomment
      CmpInst::Predicate cmp_pred;

      bool try_widen;

      // Each entry in this vector represents a thread
      // abstractDomVec_t abstractDomain; // TODO -uncomment
  };
        
  // Equivalent to printCode class in Gagan's code
  class FunctionAnalysis : public FunctionPass {
    public:
      static char ID; 
      FunctionAnalysis() : FunctionPass(ID) {}

      bool processFunction (Function& F, bool isSerial); // second arg is true if F will be executed by a single flow of control

      virtual bool runOnFunction (Function &F) {
        processFunction(F, false);
        return false;
      }

      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        // we don't modify the program, so we preserve all analyses
        AU.setPreservesAll();
      }

      void setName (string name) { this->name = name; }
    
    private:
      string name;

  };

  // PointerAnalysis - equivalent to abstractAnalysis class in Gagan's code
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

