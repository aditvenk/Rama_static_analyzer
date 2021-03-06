#include "PointerAnalysis.h"
#include "clp/clp.h"

using namespace Rama;

unsigned int numThreads;
static bool has_changed;

std::set<BasicBlock*> BBInitList;
std::set<BasicBlock*> BBSkipList;

typedef std::map <abstractDom, SetabstractDom_t, abstractDomCompare> abstractMem;
std::map <abstractDom, int, abstractDomCompare> abstractMemUpdates;
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

/**
 * 1. Check if a bb is OK to analyse. A bb is OK to analyze if
 * 	- it has no prdecessors
 * 	- it has predecessors and at least one of them is already in the BBInitList
 * 2. Propagate constraints that are coming in (if there are any) from abstractConstraintMapIn and TIDConstraintMapIn into abstractConstraintMap and TIDConstraintMap
 * Assuming these are constraints that are arrving from the predecessor bb?
 */
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
		cerr << "\tbbStart: BB "<< cur_ptr<<" not ok to analyze\n";
		BBSkipList.insert(cur_ptr);
		return;
	}
	else {
		cerr << "\tbbStart: BB "<< cur_ptr<<" ok to analyze\n";
		BBSkipList.erase(cur_ptr);
		BBInitList.insert(cur_ptr);
	}

	abstractMem temp;

	if(abstractConstraintMapIn.find(cur_ptr)!=abstractConstraintMapIn.end()) {
		cerr << "\tbbStart: BB "<< cur_ptr<<" has entry in aCMapIn, copying constraints to aCMap\n";
		abstractConstraintMap[cur_ptr]=abstractConstraintMapIn[cur_ptr]; // if constraints are coming in, we copy them into aCMap. 
		abstractConstraintMapIn.erase(cur_ptr); // remove constraints from aCMin. 
	}
	else {
		abstractConstraintMap.erase(cur_ptr); // no constraints coming in, so no constraints in acM
	}

	if(TIDConstraintMapIn.find(cur_ptr)!=TIDConstraintMapIn.end()) {
		TIDConstraintMap[cur_ptr]=TIDConstraintMapIn[cur_ptr];
		TIDConstraintMapIn.erase(cur_ptr);
	}
	else {
		TIDConstraintMap.erase(cur_ptr);	
	}
}

/*
   void addConstraint(BasicBlock *basic_block_ptr,op_info *dst_ptr,abstractDomVec_t range) {
   if((abstractConstraintMap.find(basic_block_ptr)==abstractConstraintMap.end()) ||
   ((abstractConstraintMap[basic_block_ptr]).find(dst_ptr)==(abstractConstraintMap[basic_block_ptr]).end())
   ) {
   (abstractConstraintMap[basic_block_ptr])[dst_ptr]=range;
   }
   else {
   ((abstractConstraintMap[basic_block_ptr])[dst_ptr])=((abstractConstraintMap[basic_block_ptr])[dst_ptr]).binary_op(CLP_INTERSECT,range);
   }
   }
   */

SetabstractDom_t loadFromMemory(BasicBlock *basic_block_ptr,SetabstractDom_t  address) {
	SetabstractDom_t temp;
	bool empty_flag=true;
	basic_block_ptr=NULL;	// a single map is used
	std::set<abstractDom, abstractDomCompare>::iterator it;	// iterate over target addresses
	std::map<abstractDom,SetabstractDom_t,abstractDomCompare>::iterator it1; // iterate over map
	for(it=address.SetabstractDom.begin();it!=address.SetabstractDom.end();it++) {
		if(!(EMPTY(it->clp)))
			empty_flag=false;	// at least 1 non-empty address

		if((abstractMemMap[basic_block_ptr]).empty()) continue;

		it1=(abstractMemMap[basic_block_ptr]).find(*it);
		if(it1!=(abstractMemMap[basic_block_ptr]).end()) { // if address aD has an entry in abstractMem
			if(!(EMPTY(clp_fn(CLP_INTERSECT,(*it).clp,(*it1).first.clp,false)))) // addresses intersect
				temp=temp.binary_op(CLP_UNION,(*it1).second);
		}
	}
	if(temp.SetabstractDom.size())
		return temp;
	clp_t t;
	if(empty_flag) {
		CLEAR_CLP(t);
	}
	else {
		MAKE_TOP(t);
	}
	return SetabstractDom_t(t);
}

void storeToMemory(BasicBlock *basic_block_ptr, SetabstractDom_t  address, SetabstractDom_t  value) {
	basic_block_ptr=NULL;	// a single map is used
	std::set<abstractDom, abstractDomCompare>::iterator it; // iterate over target addresses
	std::map<abstractDom,SetabstractDom_t,abstractDomCompare>::iterator it1; // iterate over map
	for(it=address.SetabstractDom.begin();it!=address.SetabstractDom.end();it++) {
		if(TOP((*it).clp)){ // don't know anything about the address
			if(!(abstractMemMap[basic_block_ptr]).empty()) {
				has_changed=true;
				abstractMemMap.clear();	// TODO: check this
				cerr << "\t\t\t\tStore address is T!! " << std::endl;
				assert(0);
				return;
			}
		}
		if((abstractMemMap[basic_block_ptr]).empty()) { // abstractMem is empty. Add a new mapping from address to value.
			abstractMem t1;
			t1[(*it)]=value;
			(abstractMemMap[basic_block_ptr])=t1;
			abstractMemUpdates[(*it)]=1;
			has_changed=true;
			continue;
		} 
		// abstractMem is not empty
		it1=(abstractMemMap[basic_block_ptr]).find(*it);
		if(it1!=(abstractMemMap[basic_block_ptr]).end()) { // there exists an entry in abstractMem for this address
			std::map<abstractDom,int,abstractDomCompare>::iterator count_it = abstractMemUpdates.find(*it);
			if(count_it != abstractMemUpdates.end()){
				cerr<< "Checking value of count:"<<count_it->second<<endl;
				if(count_it->second > 10){
					SetabstractDom_t top_setabstractDom;
					clp_t t;
					MAKE_TOP(t);
					abstractDom abs(t);
					top_setabstractDom.insert(abs);
					(*it1).second=top_setabstractDom;
					continue;
				}
				else{
					count_it->second++;
				}
			}
			if(!(EMPTY(clp_fn(CLP_INTERSECT,(*it).clp,(*it1).first.clp,false)))) { // addresses intersect
				SetabstractDom_t old_value=(*it1).second;
				(*it1).second=value.binary_op(CLP_UNION,(*it1).second); // the new value will be Union of old value and this update. Memory can get updated in any order by different threads. Hence, updates are weak.
				SetabstractDom_t new_value=(*it1).second;
				std::set<abstractDom, abstractDomCompare>::iterator it3;
				std::set<abstractDom, abstractDomCompare>::iterator it4;
				for(it3=old_value.SetabstractDom.begin();it3!=old_value.SetabstractDom.end();it3++){
					std::string n2=it3->name;
					clp_t x2=it3->clp;
					if(TOP(x2)) continue;
					bool found=false;
					std::string n1;
					clp_t x1;
					for(it4=(*it1).second.SetabstractDom.begin();it4!=(*it1).second.SetabstractDom.end();it4++){
						n1=it4->name;
						x1=it4->clp;
						if(n1==n2) {
							cerr << "\t\t\t\tFOUND\n";
							found=true;
							break;
						}
					}
					if(found) {
						abstractDom temp=x1;
						temp.name=it4->name;
						temp.lower_changed=it3->lower_changed;
						temp.upper_changed=it3->upper_changed;
						temp.lower_changed|=(x1.l!=x2.l);
						temp.upper_changed|=(x1.u!=x2.u);
						(*it1).second.SetabstractDom.erase(it4);
						(*it1).second.SetabstractDom.insert(temp);
						cerr << "\t\t\t\tflagged as changed " << temp.lower_changed << temp.upper_changed << "\n";
						has_changed=true;
					}
				}
			}
			else {
				(abstractMemMap[basic_block_ptr])[(*it)]=value;
				has_changed=true;
			}
		}
		else {
			(abstractMemMap[basic_block_ptr])[(*it)]=value;
			abstractMemUpdates[(*it)]=1;
			cerr<< "Adding to memUpdates"<<endl;
			has_changed=true;
		}
	}
}

SetabstractDomVec_t* getExistingConstraints(BasicBlock *basic_block_ptr, op_info *dst_ptr){
	std::map<op_info *,SetabstractDomVec_t>::iterator iter;
	if(abstractConstraintMap.find(basic_block_ptr)!=abstractConstraintMap.end()) {
		// iterate over all constraints in the basic block we are looking at
		if((iter=(abstractConstraintMap[basic_block_ptr]).find(dst_ptr))!=(abstractConstraintMap[basic_block_ptr]).end()) {
				return &((*iter).second);
		}
	}
	return NULL;
}

op_info applyConstraint(BasicBlock *basic_block_ptr,op_info *dst_ptr,op_info value) {

	// applyConstraint applies the constraints for this BB in the aCM and TIDCM on 'value' and creates a op_info*

	assert(basic_block_ptr);

	if((dst_ptr->isLiteral) ||
			(dst_ptr->isBasicBlockPtr) ||
			(dst_ptr->isFunctionPtr) ||
			(dst_ptr->isTID)
	  )
		return value;

	op_info result=value;	// to preserve the other boolean fields

	std::map<op_info *,SetabstractDomVec_t>::iterator iter;

	if(abstractConstraintMap.find(basic_block_ptr)!=abstractConstraintMap.end()) {
		// iterate over all constraints in the basic block we are looking at
		if((iter=(abstractConstraintMap[basic_block_ptr]).find(dst_ptr))!=(abstractConstraintMap[basic_block_ptr]).end()) {
			for(unsigned int i=0;i<numThreads;i++) {
				cerr << "hit in aC map for BB = "<< basic_block_ptr << endl;
				cerr << "\t\t\t\t Applying constraint ";
				(*iter).second.setabstractDomVec[i].print();
				cerr << endl;
				result.abstractDomain.setabstractDomVec[i]=
					(*iter).second.setabstractDomVec[i].binary_op(CLP_INTERSECT,value.abstractDomain.setabstractDomVec[i]);
				cerr << "\t\t\t\t Result ";
				result.abstractDomain.setabstractDomVec[i].print();
				cerr << endl;
			}
		}
	}

	if(result.abstractDomain.setabstractDomVec.size()<numThreads)
		return result;

	if(TIDConstraintMap.find(basic_block_ptr)!=TIDConstraintMap.end()) {
		for(unsigned int i=0;i<numThreads;i++) {
			if(!(TIDConstraintMap[basic_block_ptr] & (1<<i))) { // if this thread won't execute this BB, we don't care about abstract constraints for this value on this thread
				clp_t x;
				CLEAR_CLP(x);
				result.abstractDomain.setabstractDomVec[i]=abstractDom(x);
			}
		}
	}

	return result;
}

// iterate over a set of ADs and return CLP of AD of matching name
clp_t getCLP(SetabstractDom_t s,std::string name) {
	clp_t t;
	CLEAR_CLP(t);
	std::set<abstractDom, abstractDomCompare>::iterator it;
	for(it=s.SetabstractDom.begin();it!=s.SetabstractDom.end();it++) {
		if(it->name==name) {
			t=it->clp;
			break;
		}
	}
	return t;
}

void computeConstraints(clp_t val, llvm::CmpInst::Predicate pred,clp_t &true_clp,clp_t &false_clp)
{
	if(EMPTY(val)) { // if val is empty (not populated), we don't know anything. So TOP on both t and f paths
		MAKE_TOP(true_clp);
		MAKE_TOP(false_clp);
		return;
	}
	if(((pred==llvm::CmpInst::ICMP_SGE)||
				(pred==llvm::CmpInst::ICMP_SLT)||
				(pred==llvm::CmpInst::ICMP_SGT)||
				(pred==llvm::CmpInst::ICMP_SLE)
	   ) && (val.l>val.u)) { // val.l > val.u implies val is TOP. Comparing <, >, <=, >= with TOP, will yield TOP on both t and f paths
		MAKE_TOP(true_clp);
		MAKE_TOP(false_clp);
		return;
	}
	// val is not empty or TOP, so we know something about op2 of icmp instruction.
	switch(pred)
	{
		case llvm::CmpInst::ICMP_EQ:
			FILL_CLP(true_clp,val.l,val.u,1);
			MAKE_TOP(false_clp);
			break;		
		case llvm::CmpInst::ICMP_NE:
			MAKE_TOP(true_clp);
			FILL_CLP(false_clp,val.l,val.u,1);
			break;
		case llvm::CmpInst::ICMP_UGE:
			FILL_CLP(false_clp,0,val.u-1,1);
			FILL_CLP(true_clp,val.l,-1,1);
			break;
		case llvm::CmpInst::ICMP_ULT:
			FILL_CLP(true_clp,0,val.u-1,1);
			FILL_CLP(false_clp,val.l,-1,1);
			break;
		case llvm::CmpInst::ICMP_UGT: 
			FILL_CLP(false_clp,0,val.u,1);
			FILL_CLP(true_clp,val.l+1,-1,1);
			break;
		case llvm::CmpInst::ICMP_ULE: 
			FILL_CLP(true_clp,0,val.u,1);
			FILL_CLP(false_clp,val.l+1,-1,1);
			break;
		case llvm::CmpInst::ICMP_SGE: 
			FILL_CLP(false_clp,MAX_N,val.u-1,1);
			FILL_CLP(true_clp,val.l,MAX_P,1);
			break;
		case llvm::CmpInst::ICMP_SLT: 
			FILL_CLP(true_clp,MAX_N,val.u-1,1);
			FILL_CLP(false_clp,val.l,MAX_P,1);
			break;
		case llvm::CmpInst::ICMP_SGT:
			FILL_CLP(false_clp,MAX_N,val.u,1);
			FILL_CLP(true_clp,val.l+1,MAX_P,1);
			break;
		case llvm::CmpInst::ICMP_SLE: 
			FILL_CLP(true_clp,MAX_N,val.u,1);
			FILL_CLP(false_clp,val.l+1,MAX_P,1);
			break;
		default:
			MAKE_TOP(true_clp);
			MAKE_TOP(false_clp);
			break;
	};
}


void propagateConstraintMap(BasicBlock* cur_ptr, BasicBlock* target_ptr, op_info* special_op=NULL, SetabstractDomVec_t *special_op_val=NULL) {

	bool special_op_added=false;
	if(abstractConstraintMap.find(cur_ptr)!=abstractConstraintMap.end()) { // if cur_ptr has entry in abstractConstraintMap
		std::map<op_info *,SetabstractDomVec_t>::iterator iter;
		for(iter=(abstractConstraintMap[cur_ptr]).begin();iter!=(abstractConstraintMap[cur_ptr]).end();iter++) {
			SetabstractDomVec_t x;
			cerr << "propagateConstraintMap: iterating over op_info for cur_ptr = "<<cur_ptr<<"; op_info* = "<<(*iter).first<<endl;
			if(special_op && ((*iter).first==special_op)) {
				x=(*special_op_val);
				special_op_added=true;
			}
			else
				x=(*iter).second;
			cerr<< "propagateConstraintMap: x = ";
			x.print();
			cerr<<endl;
			if( (abstractConstraintMapIn.find(target_ptr)==abstractConstraintMapIn.end()) || // target_ptr has no abstract constraints coming in

					((abstractConstraintMapIn[target_ptr]).find((*iter).first)==(abstractConstraintMapIn[target_ptr]).end()) // target_ptr has some abstract constraints, but no entry for op_info* that we are examining
			  ) 
			{
				cerr<<"Adding entry for BB "<<target_ptr<<" op_info* = "<<(*iter).first<<" to acMapIn\n";
				(abstractConstraintMapIn[target_ptr])[(*iter).first]=x;
			}
			else 
			{ 
				// target_ptr has abstract constraints already for current op_info*. take union with existing constraints
				cerr<<"Unioning entry for BB "<<target_ptr<<" op_info* = "<<(*iter).first<<" to acMapIn\n";
				((abstractConstraintMapIn[target_ptr])[(*iter).first])=((abstractConstraintMapIn[target_ptr])[(*iter).first]).binary_op(CLP_UNION,x);
			}
		}
	} 

	if(special_op && !special_op_added) {
		SetabstractDomVec_t x;
		x=(*special_op_val);
		if(abstractConstraintMapIn.find(target_ptr)==abstractConstraintMapIn.end() ||
				(abstractConstraintMapIn[target_ptr]).find(special_op)==(abstractConstraintMapIn[target_ptr]).end()
		  ) {
			cerr<<"(2) Adding entry for BB "<<target_ptr<<" to acMapIn\n";
			(abstractConstraintMapIn[target_ptr])[special_op]=x;
		}
		else { 
			// take union with existing constraints
			cerr<<"(2) Union-ing entry for BB "<<target_ptr<<" to acMapIn\n";
			((abstractConstraintMapIn[target_ptr])[special_op])=((abstractConstraintMapIn[target_ptr])[special_op]).binary_op(CLP_UNION,x);
		}	
	}
}

void propagateTIDConstraintMap(BasicBlock* cur_ptr, BasicBlock* target_ptr, int value=-1) {

	bool value_added=false;
	if(TIDConstraintMap.find(cur_ptr)!=TIDConstraintMap.end()) { // if cur_ptr has an entry in TIDConstraintMap
		if(TIDConstraintMapIn.find(target_ptr)==TIDConstraintMapIn.end()) { // if target_ptr has no entry in TIDCMin
			if(value>=0) {
				value_added=true;
				TIDConstraintMapIn[target_ptr]=value;
			}
			else {
				TIDConstraintMapIn[target_ptr]=TIDConstraintMap[cur_ptr];
			}
		}
		else {// take union with existing constraints
			if(value>=0) {
				value_added=true;
				TIDConstraintMapIn[target_ptr] |= value;
			}
			TIDConstraintMapIn[target_ptr] |= TIDConstraintMap[cur_ptr];
		}
	}

	if(value>=0 && (!value_added)) {
		if(TIDConstraintMapIn.find(target_ptr)==TIDConstraintMapIn.end()) {
			TIDConstraintMapIn[target_ptr]=value;			
		}
		else {
			TIDConstraintMapIn[target_ptr] |= value;
		}
	}
}

bool FunctionAnalysis::abstractCompute (BasicBlock* basic_block_ptr, unsigned opcode, llvm::CmpInst::Predicate cmp_pred, op_info* dst_ptr, op_info_vec* op_vec_ptr, bool isSerial) {

	// TODO - NN: Why is numThreads being set here again when it was set in abstractInit() already?
	if(isSerial)
		numThreads=1;
	else
		numThreads=NUM_THREADS;

	// for each operand, p will hold the abstractDomains corresponding to the operand as seen at the different threads
	// thus p[2] will refer to the second operand and p[2].abstractDom[0] will refer to the abstractDomain corresponding to the second operand as seen at thread 0
	std::vector<op_info> p;
	op_info op1,op2;
	op_info result;

	std::vector<op_info*>::iterator iter;
	std::vector<abstractDom>::iterator iter2;

	has_changed=false;

	if(BBSkipList.find(basic_block_ptr)!=BBSkipList.end()){	// not yet seen at least one predecessors - thus we cannot process this instruction
		return true;
	}

	op_info c_op;

	// iterate over each operand and process the operands to obtain their corresponding representation in the abstract domain
	for(iter=op_vec_ptr->begin();iter!=op_vec_ptr->end();iter++) {

		// set c_op to be the op_info of operand being processed
		c_op=**iter;
		// we are looking at last operand of a Call instruction (first argument passed to the function). The last operand is the name of the function
		if( (dst_ptr->isCall) && (iter==op_vec_ptr->end()-1 )) {
			cerr << "\t\t\t\t Operand is a call inst \n";
			op_info temp;
			temp=c_op;
			temp.abstractDomain.clear();
			if(c_op.name=="malloc") {
				cerr << "\t\t\t\t\t Is a malloc call inst \n";
				c_op.name+=dst_ptr->name; // unique name for the call --> malloc @ this instruction 
				for(unsigned int i=0;i<numThreads;i++) {
					abstractDom k(zero,c_op.name); // zero is a global clp with values 0,0,1 - this represents the malloc'ed address in the abstractDomain
					temp.pushToDomVec(k);
				}
			}
			else { // NOT a malloc
				cerr << "\t\t\t\t\t Is NOT a malloc call inst \n";
				clp_t t;
				MAKE_TOP(t); // TODO - NN: Understand what MAKE_TOP does
				for(unsigned int i=0;i<numThreads;i++) {
					temp.pushToDomVec(t);
				}
			}
			p.push_back(temp);
		}
		// case where operand is a TID
		// Note that the c_op.isTID field actually gets set in the case where the operand is checked to see if it is a instruction (-NN referring to the case where we check if the operand name has tid?)
		else if(c_op.isTID) {
			cerr << "\t\t\t\t Operand is a TID \n";
			op_info temp;
			temp=c_op;
			// cerr<<"c_op = "; 
			// temp.print();
			// cerr<<endl;
			temp.isTID=true;
			temp.abstractDomain.clear();
			for(unsigned int i=0;i<numThreads;i++) {
				abstractDom k(i,i,1);
				temp.pushToDomVec(k);
			}
			// cerr<<"temp after = ";
			// temp.print();
			// cerr<<endl;
			p.push_back(temp);
		}
		// case where operand is a literal
		else if(c_op.isLiteral){
			cerr << "\t\t\t\t Operand is a literal \n";
			op_info temp;
			temp=c_op;
			temp.isTID=false;
			temp.abstractDomain.clear();
			abstractDom k;
			if(c_op.isFixedPoint) {
				//cerr << "\t\t\t\t\t Operand is FixedPoint " << c_op.value << "\n";
				k=abstractDom(c_op.value,c_op.value,1,c_op.name);
				temp.value=c_op.value;
			}
			else {
				//cerr << "\t\t\t\t\t Operand is TOP \n";
				MAKE_TOP(k.clp);
				temp.valueFP=c_op.valueFP;
			}
			for(unsigned int i=0;i<numThreads;i++) {
				temp.pushToDomVec(k);
			}
			p.push_back(temp);			
		}
		// case where operand is a pointer
		else if(c_op.isPointer){
			cerr << "\t\t\t\t Operand is a pointer \n";
			op_info temp;
			temp=c_op;
			temp.isPointer=false; // TODO why are they setting this to false?
			temp.abstractDomain.clear();
			for(unsigned int i=0;i<numThreads;i++) {
				abstractDom k(zero,c_op.name);
				temp.pushToDomVec(k);
			}
			p.push_back(temp);						
		}
		else if(c_op.isGEP) {
			cerr<<"\t\t\t\t Operand is result of GEP. Need to evaluate GEP first \n";
			exit(1);
		}

		// case where operand is a basic block pointer
		else if(c_op.isBasicBlockPtr) {
			cerr << "\t\t\t\t Operand is a Basic Block \n";
			// the following instructions could have a BB as operand
			if(opcode==Instruction::Br) { // br 
				if(op_vec_ptr->size()==1) { // unconditional branch
					cerr << "\t\t\t\t Operand is a BB, Unconditional branch inst \n";
					propagateConstraintMap(basic_block_ptr,c_op.BasicBlockPtr); // propagate constraints from cur BB to target BB and exit analysis
					propagateTIDConstraintMap(basic_block_ptr,c_op.BasicBlockPtr);
					return false; // has_changed = false
				}
				else {
					// cerr << "\t\t\t\t Operand is a BB, conditional branch inst \n";
					assert(op_vec_ptr->size()==3); // assert that this is a conditional branch

					// add true-path and false-path constraints
					SetabstractDomVec_t temp_vec_true, temp_vec_false;
					llvm::CmpInst::Predicate pred = (*op_vec_ptr)[0]->cmp_pred; // check operand 0 of cond br instruction, which is the condn-flag (result of icmp instruction)
					if((*op_vec_ptr)[0]->auxilliary_op) {
						for(unsigned int i=0;i<numThreads;i++) {
							SetabstractDom_t true_constraint;
							SetabstractDom_t false_constraint;
							clp_t t,f;
							std::set<abstractDom, abstractDomCompare>::iterator it;
							SetabstractDom_t s=((*op_vec_ptr)[0]->cmp_val).setabstractDomVec[i];
							cerr << "\t\t\t\t^^^^^^^\n";
							for(it=s.SetabstractDom.begin();it!=s.SetabstractDom.end();it++) {
								computeConstraints(it->clp,pred,t,f); // we are generating the aDs for true and false path of this predicate
								true_constraint.insert(t);
								false_constraint.insert(f);
								cerr << "\t\t\t\tTC "; PRINT_CLP(t); cerr << "\n";
								cerr << "\t\t\t\tFC "; PRINT_CLP(f); cerr << "\n";
							}
							cerr << "\t\t\t\t^^^^^^^\n";
							temp_vec_true.setabstractDomVec.push_back(true_constraint);
							temp_vec_false.setabstractDomVec.push_back(false_constraint);
						}
						// propagate constraint maps
						SetabstractDomVec_t* existingConstraints = getExistingConstraints(basic_block_ptr, (*op_vec_ptr)[0]->auxilliary_op); 
						if(existingConstraints != NULL){
							cerr << "\t\t\t\t Intersecting Existing Constraints as well \n";
							temp_vec_true=(temp_vec_true).binary_op(CLP_INTERSECT, (*existingConstraints));
							temp_vec_false=(temp_vec_false).binary_op(CLP_INTERSECT, (*existingConstraints));
						}
						propagateConstraintMap(basic_block_ptr,((*op_vec_ptr)[2])->BasicBlockPtr,(*op_vec_ptr)[0]->auxilliary_op, &temp_vec_true); //  op_vec_ptr[2] is true path BB
						propagateConstraintMap(basic_block_ptr,((*op_vec_ptr)[1])->BasicBlockPtr,(*op_vec_ptr)[0]->auxilliary_op, &temp_vec_false); // false path BB
						propagateTIDConstraintMap(basic_block_ptr,((*op_vec_ptr)[2])->BasicBlockPtr);
						propagateTIDConstraintMap(basic_block_ptr,((*op_vec_ptr)[1])->BasicBlockPtr);
					}
					else if(((*op_vec_ptr)[0]->cmp_val).setabstractDomVec.size()){	// TID constraint
						int val1=0;
						int val2=0;
						for(unsigned int i=0;i<numThreads;i++) {
							clp_t value=getCLP(((*op_vec_ptr)[0]->cmp_val).setabstractDomVec[i],"");
							clp_t t,f;
							computeConstraints(value,pred,t,f);
							clp_t x;
							FILL_CLP(x,i,i,1);
							if(!EMPTY(clp_fn(CLP_INTERSECT,t,x,false))) {
								val1 |= (1<<i);	// enabled on true-path
							}
							else {
								val2 |= (1<<i); // enabled on false-path
							}
						}
						propagateConstraintMap(basic_block_ptr,((*op_vec_ptr)[2])->BasicBlockPtr);
						propagateConstraintMap(basic_block_ptr,((*op_vec_ptr)[1])->BasicBlockPtr);
						propagateTIDConstraintMap(basic_block_ptr,((*op_vec_ptr)[2])->BasicBlockPtr,val1);
						propagateTIDConstraintMap(basic_block_ptr,((*op_vec_ptr)[1])->BasicBlockPtr,val2);
					}
					else	// FP constraints? do nothing
					{}

					return false; // conditional branch
				}
			}
			else if(opcode==Instruction::Ret){
				cerr << "\t\t\t\t Operand is a BB, Inst is a Return \n";
				return false;	// return
			}
			else if(opcode==Instruction::PHI) { // phi
				cerr << "\t\t\t\t Operand is a BB, Inst is a PHI \n";
				continue;
			}
			else
				return true;
		}
		else if(!(c_op.abstractDomain.setabstractDomVec).size()) {
			cerr << "\t\t\t\t Empty Operand \n";
			op_info temp=c_op;
			temp.abstractDomain.clear();
			for (unsigned int i = 0; i < numThreads; i++) {
				temp.pushToDomVec (abstractDom());
			}
			p.push_back (temp);
		}
		else {
			cerr << "\t\t\t\t Unclassified operand \n";
			cerr << "\t\t\t\t Is instruction " << c_op.isInstruction << endl;
			/*
			cerr << "\t\t\t\t";
			c_op.abstractDomain.print();
			cerr << endl;
			*/
			// copy the operand
			p.push_back(c_op);
		}
	} // finish iterate over operands

	for(unsigned int i=0;i<p.size();i++){
		// cerr << "\t\t\t\t op_vec_ptr - before applyConstraint, i="<<i<<" : " ;
		// ((*op_vec_ptr)[i])->print();
		// cerr << endl;
		// cerr << "\t\t\t\t p - before applyConstraint, i="<<i<<" : name = "<<p[i].name.c_str()<<" value : " ;
		// (p[i]).print();
		// cerr << endl;

		p[i]=applyConstraint(basic_block_ptr,(*op_vec_ptr)[i],p[i]);
		// cerr << "\t\t\t\t p - after applyConstraint, i="<<i<<" : name = "<<p[i].name.c_str()<<" value : " ;
		// p[i].print();
		// cerr << endl;
	}

	// we have the abstract representation of the operands, now compute the symbolic result of the operation 
	switch(opcode) {

		case Instruction::Add: // 7:		// Add
			op1=p[0]; 
			op2=p[1]; 
			result=op1+op2; 
			break; 
		case Instruction::Sub:		// Sub
			op1=p[0]; 
			op2=p[1]; 
			result=op1-op2; 
			break; 
		case Instruction::Mul:		// Mul
			op1=p[0]; 
			op2=p[1]; 
			result=op1*op2; 
			break; 
		case Instruction::SDiv:	// Sdiv
			op1=p[0]; 
			op2=p[1]; 
			result=op1/op2; 
			break; 
		case Instruction::Alloca: // Alloca. TODO: fix allocation size
			op1=p[0];
			result=op1;
			result.width=p[0].width;
			break;
		case Instruction::Ret: // ret
			op1=p[0];
			result=op1;
			break;
		case Instruction::Load: // Load
			op1=p[0];
			for(unsigned int i=0;i<numThreads;i++) {
				int w;
				if(op1.width)
					w=op1.width-1;
				else
					w=0;
				result.abstractDomain.setabstractDomVec.push_back(loadFromMemory(basic_block_ptr,op1.abstractDomain.setabstractDomVec[i]));
				op1.abstractDomain.setabstractDomVec[i]=op1.abstractDomain.setabstractDomVec[i].binary_op(CLP_ADD_EXPAND,abstractDom(w,w,1));
			}
			cerr << "\t\t\t\tLoad address = ";
			op1.abstractDomain.print();
			cerr << "  ";
			cerr << "Load value = "; 
			result.abstractDomain.print();
			cerr << endl;
			addToReadSet(op1.abstractDomain);
			result=applyConstraint(basic_block_ptr,dst_ptr,result);
			break;
		case Instruction::Store: // Store
			op1=p[0]; //value
			op2=p[1]; // address
			for(unsigned int i=0;i<numThreads;i++) {
				int w;
				if(op2.width)
					w=op2.width-1;
				else
					w=0;
				storeToMemory(basic_block_ptr,op2.abstractDomain.setabstractDomVec[i],op1.abstractDomain.setabstractDomVec[i]);
				op2.abstractDomain.setabstractDomVec[i]=op2.abstractDomain.setabstractDomVec[i].binary_op(CLP_ADD_EXPAND,abstractDom(w,w,1));
			}
			result=op1;
			cerr << "\t\t\t\tStore address = ";
			op2.abstractDomain.print();
			cerr << " ";
			cerr << "Store value = ";
			op1.abstractDomain.print();
			cerr <<endl;
			addToWriteSet(op2.abstractDomain);
			break;
		case Instruction::GetElementPtr: // getelementptr 
			{
				unsigned int s=p.size();
				unsigned int start_pos;
				std::string name="";
				unsigned int op_width=p[0].width;
				//cerr<<"Op Width " << op_width <<endl;
				clp_t w;
				FILL_CLP(w,op_width,op_width,1); // w stores the size of address computation (eg. 4 for int array)
				// p[0] is base address of the aggregate data-structure. Gives width of address calculations
				// p[1] indexes the pointer value
				// p[2] .. are further indices into the data structure
				name = p[0].name;

				if (name.compare("") != 0) {
					start_pos=1;
					// cerr << "getelementptr: name = "<<name.c_str()<<endl;
					// cerr<<"getelementptr: w = "; PRINT_CLP(w); cerr<<endl;
					for(unsigned int i=0;i<numThreads;i++) {
						clp_t t=(p[start_pos].abstractDomain.setabstractDomVec[i].SetabstractDom.begin())->clp; // indexes to give value of pointer
						clp_t z=(p[start_pos+1].abstractDomain.setabstractDomVec[i].SetabstractDom.begin())->clp; // indexes within the pointer. 
						/*
						for(s=start_pos+2;s<p.size();s++) {
							// Does this loop even get executed - dont think so - seems like it does get executed when getelemptr has > 2 operands
							// cerr<<"getelementptr: s = "; PRINT_CLP((p[s].abstractDomain.setabstractDomVec[i].SetabstractDom.begin())->clp); cerr<<endl;
							z=clp_fn(CLP_ADD,(p[s].abstractDomain.setabstractDomVec[i].SetabstractDom.begin())->clp,z,false);
							// cerr<<"getelementptr: z after = "; PRINT_CLP(z); cerr<<endl;
						}*/
						// I don't think we are going to see getElemPtr instructions with 5 operands so just handling case of 2,3 operands
						if(p.size() == 3){
							result.pushToDomVec(abstractDom(clp_fn(CLP_ADD,t,clp_fn(CLP_MULT,z,w,false),false),name));
						}
						if(p.size() == 4){
							unsigned element_width = (*op_vec_ptr)[0]->noOfColumns;
							clp_t el_w;
							FILL_CLP(el_w, element_width, element_width, 1);
							clp_t third = clp_fn(CLP_MULT, (p[start_pos+2].abstractDomain.setabstractDomVec[i].SetabstractDom.begin())->clp, el_w, false);
							z = clp_fn(CLP_MULT, z, w, false);
							z = clp_fn(CLP_ADD, z, third, false);
							result.pushToDomVec(abstractDom(clp_fn(CLP_ADD, t, z, false),name));
						}
					}
				}
				else { // we need to index wrt first argument
					start_pos=0;
					for(unsigned int i=0;i<numThreads;i++) {
						SetabstractDom_t s;
						std::set<abstractDom, abstractDomCompare>::iterator it;
						for(it=(p[start_pos].abstractDomain.setabstractDomVec[i].SetabstractDom.begin());it!=(p[start_pos].abstractDomain.setabstractDomVec[i].SetabstractDom.end());it++) {
							clp_t t=it->clp;
							name=it->name;
							clp_t z=(p[start_pos+2].abstractDomain.setabstractDomVec[i].SetabstractDom.begin())->clp;
							cerr << "\t\t\t\t##################" << (*op_vec_ptr)[0]->width<<endl;// << "\n";
							s.insert(abstractDom(clp_fn(CLP_ADD,t,clp_fn(CLP_MULT,z,w,false),false),name));
						}
						result.abstractDomain.setabstractDomVec.push_back(s); 
					}
				}


				result.width=p[0].width;
			}
			break;
			/*
			   case 35:
			   case 28:// Trunc. TODO: check if this is correct 
			   op1=p[0];
			   result=op1;
			   break;
			   case 36:
			   case 31:
			   case 32:
			   */

		case Instruction::ZExt: // zero extend. TODO: check if this is correct
		case Instruction::SExt: // sign extend. TODO: check if this is correct 
			op1=p[0];
			result=op1;
			break;
		case Instruction::SIToFP: // signed integer to double
			op1 = p[0];
			result = op1;
			break; 
			/*
			   case 38: // int to ptr. TODO: check if this is correct
			   case 37: // ptr to int. TODO: check if this is correct
			   op1=p[0]; 
			   result=op1;
			   break;
			   */
		case Instruction::BitCast: // bitcast. TODO: check if this is correct
			op1=p[0]; 
			result=op1;
			break;	
			/*
			   case 41:
			   */
		case Instruction::ICmp: // icmp
			op1=p[0]; 
			op2=p[1];
			cerr << "\t\t\t\t Op1 ";
			op1.abstractDomain.print();
			cerr << endl;
			cerr << "\t\t\t\t Op2 ";
			op2.abstractDomain.print();
			cerr << endl;
			result=op1;
			result.cmp_val=op2.abstractDomain;
			if(!p[0].isTID) {
				result.auxilliary_op=(*op_vec_ptr)[0];
				cerr << "\t\t\t\t Result ";
				result.abstractDomain.print();
				cerr << endl;
			}
			break;
		case Instruction::PHI: // phi
			op1=p[0];
			if(p.size()==1) {	//TODO: check how can this happen?
				result=op1;
				result.width=p[0].width;
			}
			else {
				op2=p[1];
				result=op1.binary_op(CLP_UNION,op2);
				if(p[0].width==p[1].width)
					result.width=p[0].width;
				else assert(0); // TODO: check what happens?
			}
			break;
		case Instruction::Call: //calls
			op1=p[0];
			result=op1;
			break;
		default:
			cerr << "\t\t\t\tUnimplemented opcode " << opcode << " \n";
			// assert(0);
			op1=p[0];
			result=op1;

			//		return false;
	}

	// print results for debug
	cerr << "\t\t\t\top1: ";
	op1.print();
	cerr <<  endl;
	cerr << "\t\t\t\top2: ";
	op2.print();
	cerr <<  endl;
	cerr << "\t\t\t\tresult: name = "<<result.name.c_str()<<" : ";
	result.print();
	cerr <<  endl;


	// apply constraints
	//	result=applyConstraint(basic_block_ptr,dst_ptr,result);

	// copy back & check if anything has changed
	has_changed=false;
	if(dst_ptr->abstractDomain!=result.abstractDomain)
	{
		cerr<<"Not same "<<endl;
		SetabstractDomVec_t old_val=dst_ptr->abstractDomain;
		dst_ptr->abstractDomain=result.abstractDomain;
		has_changed=true;
		if(old_val.setabstractDomVec.size()==result.abstractDomain.setabstractDomVec.size()) {
			cerr<<"Checked sizze"<<endl;
			int size=old_val.setabstractDomVec.size();
			for(int i=0;i<size;i++) {
				SetabstractDom_t *s1=&(old_val.setabstractDomVec[i]);
				SetabstractDom_t *s2=&(dst_ptr->abstractDomain.setabstractDomVec[i]);
				std::set<abstractDom, abstractDomCompare>::iterator it1;
				std::set<abstractDom, abstractDomCompare>::iterator it2;
				for(it2=s2->SetabstractDom.begin();it2!=s2->SetabstractDom.end();){
					std::string n2=it2->name;
					clp_t x2=it2->clp;
					if(TOP(x2)){
						it2++;
						continue;
					}
					bool found=false;
					std::string n1;
					clp_t x1;
					for(it1=s1->SetabstractDom.begin();it1!=s1->SetabstractDom.end();it1++){
						n1=it1->name;
						x1=it1->clp;
						if(n1==n2) {
							found=true;
							break;
						}
					}
					if(found) {
						abstractDom temp=x2;
						temp.name=it2->name;
						temp.lower_changed=(x1.l!=x2.l);
						temp.upper_changed=(x1.u!=x2.u);
						s2->SetabstractDom.erase(it2++);
						s2->insert(temp);
					}
					else
						it2++;
					dst_ptr->try_widen=true;
				}
			}
		}
	}
	else {
		dst_ptr->try_widen=false;
	}
	// the following two assignments are needed for icmp processing
	dst_ptr->auxilliary_op=result.auxilliary_op;
	dst_ptr->cmp_val=result.cmp_val;
	dst_ptr->cmp_pred=cmp_pred;
	dst_ptr->width = result.width;
	return has_changed;
}

