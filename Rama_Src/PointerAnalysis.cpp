#include "PointerAnalysis.h"
#include "llvm/IR/CallSite.h"

#include "llvm/Support/CommandLine.h"
#include <iostream>
#include <set>
#include <vector>

using namespace llvm;
using namespace std;
using namespace Rama;

/* Register Passes with LLVM */
char PointerAnalysis::ID = 0;
static RegisterPass<PointerAnalysis> X("rama", "Multi-threaded pointer analyzer");

char FunctionAnalysis::ID = 0;
static RegisterPass<FunctionAnalysis> Y("rama_functions", "Rama function analysis", true, false);

bool FunctionAnalysis::processFunction (Function& F, bool isSerial) { // second arg is true if F will be executed by a single flow of control
  string fnName = F.getName().str();
  cerr <<"processFunction called on "<<fnName<<endl;

  return false;
}

bool PointerAnalysis::analyzeFunctions(Module &M) {
  Function* func = M.getFunction("pthread_create");
  if (func == 0) {
    cerr << "pthread_create not found" << endl;
    return false;
  }
  cerr << "pthread_create found = "<< func->getName().str() << endl;
  
  FunctionAnalysis mainF;
  set <Function*> thread_functions; // Holds Function* called as part of pthread_create
  pair < set <Function*>::iterator, bool > ret;
  vector <FunctionAnalysis*> threadF_vec;
  
  // iterate through each CallInst of pthread_create and identify thread's function. 
  for (Use &U : func->uses()) {
    User *UR = U.getUser();
    if (CallInst *CI = cast<CallInst>(UR)) {
      Value* fn_arg = CI->getOperand(2);
      errs() << "pthread create called with :" << fn_arg->getName().str() << "\n";
      ret = thread_functions.insert (M.getFunction(fn_arg->getName()));
      // If the function was not already in the set
      if (ret.second == true) {
        FunctionAnalysis* threadF = new FunctionAnalysis;
        threadF->setName (fn_arg->getName());
        threadF_vec.push_back (threadF);
        cerr << "pushing " << fn_arg->getName().str() << " to threadF_vec "<<endl;
      }
    }
    else {
      cerr << "use of pthread_create was not a CallInst!"<<endl;
    }
  }

  // now call abstractCompute on the main function and each thread function

  bool changed = true;
  while (changed == true) {
    changed = mainF.processFunction ( *(M.getFunction ("main")) , true);
    cerr << "changed = "<<changed<<endl;
    changed = false; //FIXME
  }

  return false;
}

bool PointerAnalysis::doInitialization(Module &M) {
  return false;
}

bool PointerAnalysis::doFinalization(Module &M) {
  return false;
}


