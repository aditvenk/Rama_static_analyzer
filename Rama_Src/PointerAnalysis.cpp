#include "PointerAnalysis.h"

#include "llvm/Support/CommandLine.h"
#include <iostream>
#include <set>

using namespace llvm;
using namespace std;
using namespace Rama;

char PointerAnalysis::ID = 0;

static RegisterPass<PointerAnalysis> X("rama", "Multi-threaded pointer analyzer");

bool PointerAnalysis::analyzeFunctions(Module &M) {
  Function* func = M.getFunction("pthread_create");
  if (func == 0) {
    cerr << "pthread_create not found" << endl;
    return false;
  }
  cerr << "pthread_create found = "<< func->getName().str() << endl;
  set <Function*> thread_functions; // Holds Function* called as part of pthread_create
  Value::use_iterator vu_it;
  for (vu_it = func->use_begin(); vu_it != func->use_end(); vu_it++) {
    cerr << (*vu_it) << endl;
    CallInst* callInst = dyn_cast <CallInst> (*vu_it);
    cerr << callInst << endl;

    if (CallInst* callInst = dyn_cast <CallInst> (*vu_it)) {
      Value* fn_arg = callInst->getOperand(3);
      cerr << "pthread create called with : "<< fn_arg->getName().str() << endl;
    }
  }

  return false;
}

bool PointerAnalysis::doInitialization(Module &M) {
  return false;
}

bool PointerAnalysis::doFinalization(Module &M) {
  return false;
}


