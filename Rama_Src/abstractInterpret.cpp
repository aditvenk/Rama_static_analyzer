#include "PointerAnalysis.h"
#include "clp/clp.h"

using namespace Rama;

unsigned int numThreads;

std::set<BasicBlock*> BBInitList;
std::set<BasicBlock*> BBSkipList;

typedef std::map <abstractDom, SetabstractDom_t, abstractDomCompare> abstractMem;
std::map <BasicBlock*, abstractMem> abstractMemMap;

typedef std::map<op_info *,SetabstractDomVec_t> abstractConstraint;
std::map<BasicBlock *,abstractConstraint> abstractConstraintMap;
std::map<BasicBlock *,abstractConstraint> abstractConstraintMapIn;

std::map<BasicBlock *,int> TIDConstraintMap;
std::map<BasicBlock *,int> TIDConstraintMapIn;


void FunctionAnalysis::abstractInit() {
  init_clp();
  numThreads = NUM_THREADS;
}

void FunctionAnalysis::bbStart (BasicBlock* cur_ptr, std::vector<BasicBlock*> *pred_ptr_vec) {
  
  std::vector<BasicBlock *>::iterator pred_vec_iter;
	bool ok_to_analyze=false;
	bool pred_present=false;
	
  for(pred_vec_iter=pred_ptr_vec->begin();pred_vec_iter!=pred_ptr_vec->end();pred_vec_iter++) {
		if(cur_ptr!=*pred_vec_iter) {
			pred_present=true;
			if(BBInitList.find(*pred_vec_iter)!=BBInitList.end()) {	// check if at least one predecessor BBs (excluding self) analyzed
				ok_to_analyze=true;
				break;
			}
		}
	}
	if(!pred_present) {
		ok_to_analyze=true;
	}

	if(!ok_to_analyze) {
		BBSkipList.insert(cur_ptr);
		return;
	}
	else {
		BBSkipList.erase(cur_ptr);
		BBInitList.insert(cur_ptr);
	}

	abstractMem temp;
  
  if(abstractConstraintMapIn.find(cur_ptr)!=abstractConstraintMapIn.end()) {
		abstractConstraintMap[cur_ptr]=abstractConstraintMapIn[cur_ptr];
		abstractConstraintMapIn.erase(cur_ptr);
	}
	else {
		abstractConstraintMap.erase(cur_ptr);
	}

	if(TIDConstraintMapIn.find(cur_ptr)!=TIDConstraintMapIn.end()) {
		TIDConstraintMap[cur_ptr]=TIDConstraintMapIn[cur_ptr];
		TIDConstraintMapIn.erase(cur_ptr);
	}
	else {
		TIDConstraintMap.erase(cur_ptr);	
	}
}

