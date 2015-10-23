#include "PointerAnalysis.h"
#include "clp/clp.h"

using namespace Rama;

unsigned int numThreads;
static bool has_changed;

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

op_info applyConstraint(BasicBlock *basic_block_ptr,op_info *dst_ptr,op_info value) {

	assert(basic_block_ptr);

	if((dst_ptr->isLiteral) ||
	   (dst_ptr->isBasicBlockPtr) ||
	   (dst_ptr->isFunctionPtr) ||
	   (dst_ptr->isTID)
	  )
		return value;

	op_info result=value;	// to preserve the other boolean fields

	std::map<op_info *,abstractDomVec_t>::iterator iter;

	if(abstractConstraintMap.find(basic_block_ptr)!=abstractConstraintMap.end()) {
		if((iter=(abstractConstraintMap[basic_block_ptr]).find(dst_ptr))!=(abstractConstraintMap[basic_block_ptr]).end()) {
			for(unsigned int i=0;i<numThreads;i++) {
				result.abstractDomain.abstractDomVec[i]=
				(*iter).second.abstractDomVec[i].binary_op(CLP_INTERSECT,value.abstractDomain.abstractDomVec[i]);
			}
		}
	}

	if(result.abstractDomain.abstractDomVec.size()<numThreads)
		return result;

	if(TIDConstraintMap.find(basic_block_ptr)!=TIDConstraintMap.end()) {
		for(unsigned int i=0;i<numThreads;i++) {
			if(!(TIDConstraintMap[basic_block_ptr] & (1<<i))) {
				clp_t x;
				CLEAR_CLP(x);
				result.abstractDomain.abstractDomVec[i]=abstractDom(x);
			}
		}
	}

	return result;
}

void propagateConstraintMap(BasicBlock* cur_ptr, BasicBlock* target_ptr, op_info* special_op=NULL, SetabstractDomVec_t *special_op_val=NULL) {

	bool special_op_added=false;
	if(abstractConstraintMap.find(cur_ptr)!=abstractConstraintMap.end()) { // if cur_ptr has entry in abstractConstraintMap
		std::map<op_info *,SetabstractDomVec_t>::iterator iter;
		for(iter=(abstractConstraintMap[cur_ptr]).begin();iter!=(abstractConstraintMap[cur_ptr]).end();iter++) {
			SetabstractDomVec_t x;
			if(special_op && ((*iter).first==special_op)) {
				x=(*special_op_val);
				special_op_added=true;
			}
			else
				x=(*iter).second;

			if( (abstractConstraintMapIn.find(target_ptr)==abstractConstraintMapIn.end()) || // target_ptr has no abstract constraints coming in

			   ((abstractConstraintMapIn[target_ptr]).find((*iter).first)==(abstractConstraintMapIn[target_ptr]).end()) // target_ptr has some abstract constraints, but no entry for op_info* that we are examining
			  ) 
      {
        (abstractConstraintMapIn[target_ptr])[(*iter).first]=x;
			}
			else 
      { 
				// target_ptr has abstract constraints already for current op_info*. take union with existing constraints
				((abstractConstraintMapIn[target_ptr])[(*iter).first])=((abstractConstraintMapIn[target_ptr])[(*iter).first]).binary_op(CLP_UNION,x);
			}
		}
	} 

	if(special_op && !special_op_added) {
		abstractDomVec_t x;
		x=(*special_op_val);
		if(abstractConstraintMapIn.find(target_ptr)==abstractConstraintMapIn.end() ||
		  (abstractConstraintMapIn[target_ptr]).find(special_op)==(abstractConstraintMapIn[target_ptr]).end()
		) {
			(abstractConstraintMapIn[target_ptr])[special_op]=x;
		}
		else { 
			// take union with existing constraints
			((abstractConstraintMapIn[target_ptr])[special_op])=((abstractConstraintMapIn[target_ptr])[special_op]).binary_op(CLP_UNION,x);
		}	
	}
}

void propagateTIDConstraintMap(BasicBlock* cur_ptr, BasicBlock* target_ptr, int value=-1) {

	bool value_added=false;
	if(TIDConstraintMap.find(cur_ptr)!=TIDConstraintMap.end()) { // if cur_ptr has an entry in TIDConstraintMap
		if(TIDConstraintMapIn.find(target_ptr)==TIDConstraintMapIn.end()) {
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


  if(isSerial)
		numThreads=1;
	else
		numThreads=NUM_THREADS;

  std::vector<op_info> p;
	op_info op1,op2;
	op_info result;

	std::vector<op_info*>::iterator iter;
	std::vector<abstractDom>::iterator iter2;

	has_changed=false;

	if(BBSkipList.find(basic_block_ptr)!=BBSkipList.end())	// not yet seen at least one predecessors
		return true;

	op_info c_op;
	// process the operands
	for(iter=op_vec_ptr->begin();iter!=op_vec_ptr->end();iter++) {

    // printOpInfo (*iter);
    c_op=**iter; // c_op is op_info of operand
		if( (dst_ptr->isCall) && (iter==op_vec_ptr->end()-1 )) { // we are looking at last operand of a Call instruction (first argument passed to the function)
      op_info temp;
			temp=c_op;
			temp.abstractDomain.clear();
			if(c_op.name=="malloc") {
				c_op.name+=dst_ptr->name; // unique name for the call --> malloc @ this instruction 
				for(unsigned int i=0;i<numThreads;i++) {
					abstractDom k(zero,c_op.name); // zero is a global clp with values 0,0,1
					temp.pushToDomVec(k);
				}
			}
			else {
				clp_t t;
				MAKE_TOP(t);
				for(unsigned int i=0;i<numThreads;i++) {
					temp.pushToDomVec(t);
				}
			}
			p.push_back(temp);
		}
    // Note that the c_op.isTID field actually gets set in the case where the operand is checked to see if it is a instruction
		else if(c_op.isTID) {
      op_info temp;
			temp=c_op;
			temp.isTID=true;
			temp.abstractDomain.clear();
			for(unsigned int i=0;i<numThreads;i++) {
				abstractDom k(i,i,1);
				temp.pushToDomVec(k);
			}
			p.push_back(temp);
		}
		else if(c_op.isLiteral){
			op_info temp;
			temp=c_op;
			temp.isTID=false;
			temp.abstractDomain.clear();
			abstractDom k;
			if(c_op.isFixedPoint) {
				k=abstractDom(c_op.value,c_op.value,1,c_op.name);
				temp.value=c_op.value;
			}
			else {
				MAKE_TOP(k.clp);
				temp.valueFP=c_op.valueFP;
			}
			for(unsigned int i=0;i<numThreads;i++) {
				temp.pushToDomVec(k);
			}
			p.push_back(temp);			
		}
    else if(c_op.isPointer){
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
    else if(c_op.isBasicBlockPtr) {

			if(opcode==2) { // br TODO -- fix this
				if(op_vec_ptr->size()==1) { // unconditional branch
					propagateConstraintMap(basic_block_ptr,c_op.BasicBlockPtr);
					propagateTIDConstraintMap(basic_block_ptr,c_op.BasicBlockPtr);
					return false;
				}
				else {
					assert(op_vec_ptr->size()==3);
					// add true-path and false-path constraints
					abstractDomVec_t temp_vec1, temp_vec2;
					llvm::CmpInst::Predicate pred = (*op_vec_ptr)[2]->cmp_pred;
					if((*op_vec_ptr)[2]->auxilliary_op) {
						for(unsigned int i=0;i<numThreads;i++) {
							SetabstractDom_t true_constraint;
							SetabstractDom_t false_constraint;
							clp_t t,f;
							std::set<abstractDom, abstractDomCompare>::iterator it;
							SetabstractDom_t s=((*op_vec_ptr)[2]->cmp_val).abstractDomVec[i];
							//cerr << "^^^^^^^\n";
							for(it=s.SetabstractDom.begin();it!=s.SetabstractDom.end();it++) {
								computeConstraints(it->clp,pred,t,f);
								true_constraint.insert(t);
								false_constraint.insert(f);
								//cerr << "TC "; PRINT_CLP(t); cerr << "\n";
								//cerr << "FC "; PRINT_CLP(f); cerr << "\n";
							}
							//cerr << "^^^^^^^\n";
							temp_vec1.abstractDomVec.push_back(true_constraint);
							temp_vec2.abstractDomVec.push_back(false_constraint);
						}
						// propagate constraint maps
						propagateConstraintMap(basic_block_ptr,((*op_vec_ptr)[0])->BasicBlockPtr,(*op_vec_ptr)[2]->auxilliary_op,&temp_vec1);
						propagateConstraintMap(basic_block_ptr,((*op_vec_ptr)[1])->BasicBlockPtr,(*op_vec_ptr)[2]->auxilliary_op,&temp_vec2);
						propagateTIDConstraintMap(basic_block_ptr,((*op_vec_ptr)[0])->BasicBlockPtr);
						propagateTIDConstraintMap(basic_block_ptr,((*op_vec_ptr)[1])->BasicBlockPtr);
					}
					else if(((*op_vec_ptr)[2]->cmp_val).abstractDomVec.size()){	// TID constraint
						int val1=0;
						int val2=0;
						for(unsigned int i=0;i<numThreads;i++) {
							clp_t value=getCLP(((*op_vec_ptr)[2]->cmp_val).abstractDomVec[i],"");
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
						propagateConstraintMap(basic_block_ptr,((*op_vec_ptr)[0])->BasicBlockPtr);
						propagateConstraintMap(basic_block_ptr,((*op_vec_ptr)[1])->BasicBlockPtr);
						propagateTIDConstraintMap(basic_block_ptr,((*op_vec_ptr)[0])->BasicBlockPtr,val1);
						propagateTIDConstraintMap(basic_block_ptr,((*op_vec_ptr)[1])->BasicBlockPtr,val2);
					}
					else	// FP constraints? do nothing
					;
					return false; // conditional branch
				}
			}
			else if(opcode==1)
				return false;	// return
			else if(opcode==42) { // phi
				continue;
			}
			else
				return true;
		}
  }
    /*
		else if(!(c_op.abstractDomain.abstractDomVec).size()) {
		    op_info temp=c_op;
			temp.abstractDomain.clear();
		    for (unsigned int i = 0; i < numThreads; i++) {
			  temp.pushToDomVec (abstractDom());
		    }
			p.push_back (temp);
		}
		else {
			// copy the operand
			p.push_back(c_op);
		}
	}

	for(unsigned int i=0;i<p.size();i++){
		p[i]=applyConstraint(basic_block_ptr,(*op_vec_ptr)[i],p[i]);
	}

	// compute the result
	switch(opcode) {
	case 7:		// Add
		op1=p[0]; 
		op2=p[1]; 
		result=op1+op2; 
		break; 
	case 8:		// Sub
		op1=p[0]; 
		op2=p[1]; 
		result=op1-op2; 
		break; 
	case 9:		// Mult
		op1=p[0]; 
		op2=p[1]; 
		result=op1*op2; 
		break; 
	case 11:	// Sdiv
		op1=p[0]; 
		op2=p[1]; 
		result=op1/op2; 
		break; 
	case 24: // Alloca. TODO: fix allocation size
		op1=p[0];
		result=op1;
		result.width=p[0].width;
		break;
	case 25: // Load
		op1=p[0];
		for(unsigned int i=0;i<numThreads;i++) {
			int w;
			if(op1.width)
				w=op1.width-1;
			else
				w=0;
			result.abstractDomain.abstractDomVec.push_back(loadFromMemory(basic_block_ptr,op1.abstractDomain.abstractDomVec[i]));
			op1.abstractDomain.abstractDomVec[i]=op1.abstractDomain.abstractDomVec[i].binary_op(CLP_ADD_EXPAND,abstractDom(w,w,1));
		}
		//cerr << "\nLoad address = ";
		//op1.abstractDomain.print();
		//cerr << " ";
		addToReadSet(op1.abstractDomain);
		result=applyConstraint(basic_block_ptr,dst_ptr,result);
		break;
	case 26: // Store
		op1=p[0];
		op2=p[1];
		for(unsigned int i=0;i<numThreads;i++) {
			int w;
			if(op2.width)
				w=op2.width-1;
			else
				w=0;
			storeToMemory(basic_block_ptr,op2.abstractDomain.abstractDomVec[i],op1.abstractDomain.abstractDomVec[i]);
			op2.abstractDomain.abstractDomVec[i]=op2.abstractDomain.abstractDomVec[i].binary_op(CLP_ADD_EXPAND,abstractDom(w,w,1));
		}
		result=op1;
		cerr << "\nStore address = ";
		op2.abstractDomain.print();
		cerr << " ";
		addToWriteSet(op2.abstractDomain);
		break;
	case 27: // getelementptr 
		{
			unsigned int s=p.size();
			unsigned int start_pos;
			std::string name="";
            unsigned int op_width=p[0].width;
            clp_t w;
            FILL_CLP(w,op_width,op_width,1);
			if(s==3) {
				start_pos=1;
				name=p[0].name;
				for(unsigned int i=0;i<numThreads;i++) {
					clp_t t=(p[start_pos].abstractDomain.abstractDomVec[i].SetabstractDom.begin())->clp;
					clp_t z=(p[start_pos+1].abstractDomain.abstractDomVec[i].SetabstractDom.begin())->clp;
					for(s=start_pos+2;s<p.size();s++)
						z=clp_fn(CLP_ADD,(p[s].abstractDomain.abstractDomVec[i].SetabstractDom.begin())->clp,z,false);
					result.pushToDomVec(abstractDom(clp_fn(CLP_ADD,t,clp_fn(CLP_MULT,z,w,false),false),name)); 
				}
			}
			else if(s==2) 
			{ 
				start_pos=0;
				for(unsigned int i=0;i<numThreads;i++) {
					SetabstractDom_t s;
					std::set<abstractDom, abstractDomCompare>::iterator it;
					for(it=(p[start_pos].abstractDomain.abstractDomVec[i].SetabstractDom.begin());it!=(p[start_pos].abstractDomain.abstractDomVec[i].SetabstractDom.end());it++) {
						clp_t t=it->clp;
						name=it->name;
						clp_t z=(p[start_pos+1].abstractDomain.abstractDomVec[i].SetabstractDom.begin())->clp;
						cerr << "##################" << (*op_vec_ptr)[0]->width;// << "\n";
						//assert ((*op_vec_ptr)[0]->width != 0);
						PRINT_CLP(w);
						PRINT_CLP(z);
						cerr << "##################\n";
						s.insert(abstractDom(clp_fn(CLP_ADD,t,clp_fn(CLP_MULT,z,w,false),false),name));
					}
					result.abstractDomain.abstractDomVec.push_back(s); 
				}
			}
			else assert(0);
			result.width=p[0].width;
		}
		break;
	case 35:
	case 28:// Trunc. TODO: check if this is correct 
		op1=p[0];
		result=op1;
		break;
	case 36:
	case 31:
	case 32:
	case 29: // zero extend. TODO: check if this is correct
	case 30: // sign extend. TODO: check if this is correct 
		op1=p[0];
		result=op1;
		break;
	case 38: // int to ptr. TODO: check if this is correct
	case 37: // ptr to int. TODO: check if this is correct
		op1=p[0]; 
		result=op1;
		break;
	case 39: // bitcast. TODO: check if this is correct
		op1=p[0]; 
		result=op1;
		break;	
	case 41:
	case 40: // icmp
		op1=p[0];
		op2=p[1];
		result=op1;
		result.cmp_val=op2.abstractDomain;
		if(!p[0].isTID) {
			result.auxilliary_op=(*op_vec_ptr)[0];
		}
		break;
	case 42: // phi
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
			//		else assert(0); // TODO: check what happens?
		}
		break;
	case 43: //calls
		op1=p[0];
		result=op1;
		break;
	default:
//		cerr << "Unimplemented opcode " << opcode << " \n";
//		assert(0);
		op1=p[0];
		result=op1;
//		return false;
	}

	// apply constraints
//	result=applyConstraint(basic_block_ptr,dst_ptr,result);

	// copy back & check if anything has changed
	has_changed=false;
	if(dst_ptr->abstractDomain!=result.abstractDomain)
	{
		abstractDomVec_t old_val=dst_ptr->abstractDomain;
		dst_ptr->abstractDomain=result.abstractDomain;
		has_changed=true;
		if(old_val.abstractDomVec.size()==result.abstractDomain.abstractDomVec.size()) {
			int size=old_val.abstractDomVec.size();
			for(int i=0;i<size;i++) {
				SetabstractDom_t *s1=&(old_val.abstractDomVec[i]);
				SetabstractDom_t *s2=&(dst_ptr->abstractDomain.abstractDomVec[i]);
				std::set<abstractDom, abstractDomCompare>::iterator it1;
				std::set<abstractDom, abstractDomCompare>::iterator it2;
				for(it2=s2->SetabstractDom.begin();it2!=s2->SetabstractDom.end();){
					std::string n2=it2->name;
					clp_t x2=it2->clp;
					if(TOP(x2)) continue;
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
  */
}

