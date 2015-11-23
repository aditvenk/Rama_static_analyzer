#include "PointerAnalysis.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Casting.h"
#include <sstream>

using namespace llvm;
using namespace std;
using namespace Rama;

typedef std::map <Instruction*, op_info*> InstrToInfoMap;
typedef std::map <std::string, op_info*> OpToInfoMap;
typedef std::map <Instruction*, std::string> InstrToNameMap;

// maps a instruction to the unique name generated for that instruction
InstrToNameMap instr_map;
// maps a instruction to the op_info object generated for that instruction
InstrToInfoMap info_map;
// maps a operand's name (is this unique?) to the op_info object generated for that operand - this only seems to get updated in the case of the operand being a pointer to an array
OpToInfoMap operand_map;
int instr_count; //is this actually used? there is a local called instr_count inside of processFunction which takes precedence

/* Register Passes with LLVM */
char PointerAnalysis::ID = 0;
static RegisterPass<PointerAnalysis> X("rama", "Multi-threaded pointer analyzer");

char FunctionAnalysis::ID = 0;
static RegisterPass<FunctionAnalysis> Y("rama_functions", "Rama function analysis", true, false);


void printInfoMap () {
	InstrToInfoMap::iterator it;
	for (it = info_map.begin (); it != info_map.end (); it++) {
		cerr << instr_map[it->first] << " : ";
		(it->second)->print();
		cerr << "\n";
	}
}

void FunctionAnalysis::printOpInfoSet ( std::set < SetabstractDomVec_t > &info_set) {
	std::set < SetabstractDomVec_t >::iterator it;
	for (it = info_set.begin (); it != info_set.end (); it++) {
		(*it).print ();
		cerr << "\n";
	}
}

void FunctionAnalysis::addToReadSet ( SetabstractDomVec_t &ip ) {
	rd_set.insert (ip);
}

void FunctionAnalysis::addToWriteSet ( SetabstractDomVec_t &ip ) {
	wr_set.insert (ip);
}

void FunctionAnalysis::computeConflicts () {
    // Now compute conflicts between the read and write sets
    //op_info_set::iterator l, m;
    std::set < SetabstractDomVec_t >::iterator l, m;
      for (l = rd_set.begin (); l != rd_set.end (); l++) {
      for (m = wr_set.begin (); m != wr_set.end (); m++) {
	      rd_wr_conflicts.insert ( (*l).crossVec_intersect (*m) );
      }
    }

    // Now compute conflicts between the elements of the write sets
    for (l = wr_set.begin (); l != wr_set.end (); l++) {
      for (m = l; m != wr_set.end (); m++) {
	      wr_wr_conflicts.insert ( (*l).crossVec_intersect (*m) );
      }
    }

    // Now print the conflics
    cerr << "Function " << name << ": Conflicts between Read and Write Sets: \n"; 
    rd_wr_conflicts.print ();
    cerr << "\n";
    cerr << "Function " << name << ": Conflicts between Write and Write Sets: \n"; 
    wr_wr_conflicts.print ();
    cerr << "\n";
  }



// this function is called on each High Level Function -- i.e. Main and each WorkerThreadFunction. Each high-level function corresponds to a thread
// so if the program had 5 pthread_create functions, this function will be called with the 6 different high level functions as argument
// This function iterates through each instruction and each operand of an instruction of a function to create a op_info for each one
// The op_infos collected for the operands and the instruction are then passed to abstractCompute which performs the operation in the 
// abstract domain. The above process is repeated for all the high-level functions in the program untill equilibirum is reached. 
// We then compute the conflicting memory access of each thread(high-level) function
bool FunctionAnalysis::processFunction (Function& F, bool isSerial) { // second arg is true if F will be executed by a single flow of control
	string fnName = F.getName().str();
	cerr <<"processFunction called on "<<fnName<<endl;

	// initialize a bunch of locals
	// the instr_count and sub_instr_count taken together will help is identify a instruction uniquely in a function
	int instr_count = 0;
	int sub_instr_count = 0;
	bool has_changed = true;
	int num_passes = 0;
	bool first_pass = 0;
	bool widened = false;
	bool isNonNumInstr;

	// this structure holds the op_info constructed for each operand of an instruction
	op_info_vec* perInstrOpInfo = new op_info_vec;
	// sets number of threads, makes some calls to set internal clp - TODO: see what init_clp() does
	abstractInit();

	while (has_changed == true) {// check if we obtained any new symbolic information

		// clear up the read and write sets of this function at the start of each iteration
		// since abstractCompute will attempt to populate it every time. 
		rd_set.clear();
		wr_set.clear();

		assert ( num_passes <= NUM_TRIALS /* + POST_WIDEN_TRIALS */); // TODO - implement post-widen
		cerr << "PASS #"<< num_passes<<endl;

		has_changed = false;

		/****************************************Basic Block*********************************/
		for (Function::iterator i = F.begin(); i != F.end(); i++) {
			cerr << "BASIC BLOCK: "<<(*i).getName().str() << "\n";

			// TODO - find out what is a landing pad basic block
			if ( i->isLandingPad() ) {
				cerr << " This is a landing pad basic block \n";
			}

			// before we go over each instruction in the bb, we need to set up bb maps.
			// we need predecessors information for each basic block for this.
			std::vector <BasicBlock*> *i_pred = new std::vector <BasicBlock*>;
			for (pred_iterator I = pred_begin(i), E = pred_end(i); I != E; ++I) {
				cerr << "\tPREDECESSOR = "<< (*I)->getName().str() << "\n";
				i_pred->push_back(*I);
			}

			// ensure at least one predecessor in bbInitList
			// propagate incoming constraints	
			bbStart (i, i_pred); // TODO - what does this function do?

			/****************************************Instruction*********************************/
			// iterate over each instruction in the bb
			for (BasicBlock::iterator b = (*i).begin(); b != (*i).end(); b++) {

				// If the instruction is of no interest to us skip its processing
				if ( (*b).getOpcode ( ) == Instruction::Unreachable ) {
					cerr << "\t\tFound unreachable instr\n";
					continue;
				}

				// reset the operand info vector
				perInstrOpInfo->clear();
				isNonNumInstr = false;

				// We will assign numbers to each instruction to match the llvm-dis output
				// So first identify the instruction to which llvm-dis does not assign a number
				if (   ((*b).getOpcode ( ) == Instruction::Store)
						| ((*b).getOpcode ( ) == Instruction::Br)
						| ((*b).getOpcode ( ) == Instruction::Alloca)
						| (( ((*b).getOpcode ( ) == Instruction::BitCast) & (instr_count == 0) ))
						| ((*b).getOpcode ( ) == Instruction::Ret)
						| ((*b).getOpcode ( ) == Instruction::PHI)
				   ) {
					isNonNumInstr = true;
				}

				op_info* op_info_struct; // Holds the op_info_vec for this instruction
				// First check whether we have already encountered this instruction .
				// If yes then retrive its op_info_vec, else 
				// Allocate a new op_info structure and insert in the info_map
				InstrToInfoMap::iterator iit = info_map.find((Instruction *)&*(b));
				if (iit == info_map.end()) { // not in info map - TODO: NN - what are the cases when an instruction will already be in the info/instr maps?
					// Prepare to log the instruction in instr_map and info_map
					std::stringstream out;
					out << instr_count;

					// if it is a non numbered instruction, we only increment the sub_instr_count
					if ( isNonNumInstr == true ) {
						out << "." << sub_instr_count;
						sub_instr_count++;
					}
					std::string instrName = "%" + fnName + "#" +  out.str (); // this is the unique name we will store in instr_map. #<function_name>#number.sub-number

					op_info_struct = new op_info;
					op_info_struct->isInstruction = true;

					std::string instr_name = (*b).getName ().str(); // this is the instruction's name as given by llvm. typically the assigned var's name eg: %tid = .... --> instruction name is tid
					if (instr_name.find ("tid") != std::string::npos) {// a tid instruction is found - means that we are assigning a tid value
						op_info_struct->isTID = true;
						cerr << "\t\tFound a TID instr : "<<instr_name<<endl;
					}

					// (*b) is the pointer to the instruction
					// lets add make entries in info_map and instr_map for this instruction (since we have asserted that it has not been seen yet)
					info_map [b] = op_info_struct;
					instr_map [b] = instrName; //_count;

					// check for alloca instruction
					if ((*b).getOpcode ( ) == Instruction::Alloca) { 
						op_info_struct->name = instrName;
						op_info_struct->isAlloca = true;
					}
					// check for call instruction
					if ((*b).getOpcode ( ) == Instruction::Call) { 
						op_info_struct->name = instrName;
						op_info_struct->isCall = true;
					}

					// TODO: NN - Does this check being here make sense?
					// what happens is the case of hitting a numbered instruction after a series of non numbered instructions?
					if (isNonNumInstr == false) {
						instr_count++;
						sub_instr_count = 0;
					}
				}  
				else {  // instr present in info_map
					op_info_struct = iit->second;
					// Now check whether the instruction is in our instr_map
					// If not then we need to put it in
					InstrToNameMap::iterator map_iter = instr_map.find((Instruction *)&*(b));
					if (map_iter == instr_map.end()) { // this case will occur when we encounter an instruction first as an operand to a prev instr. We will create an op_info and add to info_map, but not in instr_map. Why? I don't know. 
						std::stringstream out;
						out << instr_count;
						if ( isNonNumInstr == true ) {
							out << "." << sub_instr_count;
							sub_instr_count++;
						}
						std::string instrName = "%" + fnName + "#" + out.str ();
						instr_map [b] = instrName; 
						if (isNonNumInstr == false) {
							instr_count++;
							sub_instr_count = 0;
						}
					}
				}


				// Print out instruction
				cerr << "\t\t"<<instr_map[b] << ":" << "\t opcode name = " << (*b).getOpcodeName () << "\n";
				unsigned opcode = (*b).getOpcode(); // Holds the opcode of the instruction
				cerr<< "\t\t"<<instr_map[b] << ":" << "\t opcode value = " << (*b).getOpcode() << endl;

				// If it is a cmp instruction extract the predicate
				llvm::CmpInst::Predicate cmp_pred;
				if (isa <ICmpInst> (*b)) {
					cmp_pred = ((ICmpInst *)&* (b))->getSignedPredicate();
				}

				// Just some logging in case of a alloca instruction
				if (opcode == Instruction::Alloca) { 
					//cerr << "Found alloca";
					AllocaInst* a_inst = dyn_cast<AllocaInst> (b);
					cerr << "\t\talloca name = " << op_info_struct->name << (a_inst->getArraySize())->getName().str() << "  ";
					// cerr << "array size = " << (a_inst->getAllocatedType())->getArrayNumElements() << endl;
				}

				int temp_op_itr = 0;//keep track of the number of operands 


				/****************************************Operand*********************************/
				// Now lets go over each operand in the instruction 
				for (User::op_iterator operand = (*b).op_begin(); operand != (*b).op_end(); operand++, temp_op_itr++) {

					cerr << "\t\t\toperand #"<< temp_op_itr<<" Name = " << (*operand)->getName().str() << " Type = " << ((*operand)->getType ())->getTypeID()<<endl;

					// Find out interesting details about this operand
					// First find the width of the "eventual" data in the operand. - TODO: - NN: Understand how we use the width in the abstractDomain
					unsigned widthInBits;
					unsigned width;

					// Should really first check whether this is a primitive type operand before checking its width
					const Type* dataType = (*operand)->getType();
					widthInBits = dataType->getPrimitiveSizeInBits();

					// If the type is of pointer then the above getPrimitiveSizeInBits() function does not work. We first need to find the data type it points to
					if (dataType->getTypeID() == Type::PointerTyID ) {
						dataType = dataType->getContainedType (0);

						// If the type is a pointer to a pointer then we need to find the data type it points to
						if (dataType->getTypeID() == Type::PointerTyID) {
							dataType = dataType->getContainedType(0);
						}

						// if the type is a pointer to an array we need to find the type of the array elements
						if ( dataType->getTypeID() == Type::ArrayTyID ) {
							cerr << "\t\t\t** Found Array ** " <<endl;
							cerr << "\t\t\tArray element type = " << (((ArrayType *)&*((*operand)->getType()->getContainedType(0)))->getElementType ())->getTypeID ()<<endl;
							dataType = ( (ArrayType *)&*(dataType) )->getElementType ();
							//cerr << width;
						}

						widthInBits = dataType->getPrimitiveSizeInBits();

						// If the operand is a structure then we need to go through the elements to compute
						// its size. But we need to do this recurseively for any structs contained within.
						// For now we assume there are no structs inside. i.e. We do not handle nested structs
						if ( dataType->getTypeID() == Type::StructTyID ) {
							unsigned numElements = dataType->getNumContainedTypes ();
							unsigned z;
							for (z = 0; z < numElements; z++) {
								widthInBits += dataType->getContainedType(z)->getPrimitiveSizeInBits();
							}
						}
					}

					// width of the operand in Bytes 
					width = (unsigned) ceil ( (float) ( widthInBits )/8 ); 
					cerr << "\t\t\twidth(Bytes) = "<< width << " width(Bits) = "<<widthInBits<<endl;

					// every virtual register operand is of type Instruction and points to the instruction which generates it.
					if (isa <Instruction> (*operand)){
						// check if this operand comes from a instruction already seen
						InstrToNameMap::iterator map_iter = instr_map.find((Instruction *)&*(*operand)); 
						if (map_iter == instr_map.end()) {
							cerr << "\t\t\tIns Op not in instr map" <<endl;
						}
						else {
							cerr << "\t\t\tInstr found - " << map_iter->second <<endl;
						}

						InstrToInfoMap::iterator InfoMap_iter = info_map.find((Instruction *)&*(*operand));
						if (InfoMap_iter == info_map.end()) {
							// this instruction has not already been seen - TODO: NN: Check when this case will be hit
							cerr << "\t\t\tIns Op not in info_map, adding now "<<endl;
							// Allocate 
							op_info* temp_op_info = new op_info;
							temp_op_info->isInstruction = true;
							temp_op_info->width = width;
							// if the instruction was Alloca, i.e. the operand is an array then set that flag
							if ( ((Instruction *)&*(*operand))->getOpcode () == Instruction::Alloca) {
								temp_op_info->isAlloca = true;
							}
							// update the info map with the op_info for the instruction corresponding to this operand 
							// why is the instr_map not updated with this instruction?
							info_map [(Instruction *)&*(*operand)] = temp_op_info;
							perInstrOpInfo->push_back (temp_op_info);
						} 
						else {
							cerr << "\t\t\t";
							InfoMap_iter->second->abstractDomain.print();
							cerr << endl;
							// update the width of the operand
							InfoMap_iter->second->width = width;
							//cerr << "*" << (InfoMap_iter->second)->width << "*";
							perInstrOpInfo->push_back(InfoMap_iter->second);
						}
					}

					// TODO - why is this code needed? 
					/*
					   else {
					   if (((*operand)->hasName () | isa<Value> (*operand)) & (strcmp((*b).getOpcodeName (), "store") != 0)) {
					   cerr << " " << (*operand)->getName();
					   } else {
					   cerr << " XXX";
					   }
					   }
					   */

					// case of operand is a constant
					else if (isa<ConstantInt> (*operand)) {
						op_info* temp_op_info = new op_info;
						temp_op_info->isLiteral = true;
						temp_op_info->isFixedPoint = true;
						temp_op_info->value = ((ConstantInt *)&*(*operand))->getZExtValue();
						temp_op_info->width = width;
						perInstrOpInfo->push_back (temp_op_info);
						cerr << "\t\t\tConst " << temp_op_info->value <<endl;
					} 
					// case of operand is a basic block
					else if (isa<BasicBlock> (*operand)) {
						op_info* temp_op_info = new op_info;
						temp_op_info->isBasicBlockPtr = true;
						temp_op_info->BasicBlockPtr = (BasicBlock *)&*(*operand);
						temp_op_info->width = width;
						perInstrOpInfo->push_back (temp_op_info);
						cerr << "\t\t\tBB " << (*operand)->getName().str()<<endl;
					}
					// case of operand is a pointer - yes we check again 
					else if ((*operand)->getType()->getTypeID() == Type::PointerTyID) { // operand is a ptr
						cerr << "\t\t\t@" << (*operand)->getName ().str() <<endl;
						OpToInfoMap::iterator OpMap_iter = operand_map.find( (*operand)->getName () );
						if (OpMap_iter == operand_map.end()) {
							// Following seems to get the type of data the pointer is pointing to
							cerr << "\t\t\t" << (*operand)->getType()->getTypeID() << " CT = " << ((*operand)->getType()->getContainedType(0))->getTypeID() << " width = " << ((*operand)->getType()->getContainedType(0))->getPrimitiveSizeInBits() <<endl;
							int size = 1;
							bool isArray = false;
							op_info* temp_op_info = new op_info;
							if (((*operand)->getType()->getContainedType(0))->getTypeID() == Type::ArrayTyID) {
								cerr << "\t\t\t** Found Array ** "<<endl;
								isArray = true;
								size = ((ArrayType *)&*((*operand)->getType()->getContainedType(0)))->getNumElements();
								// check if operand is 2-d array
								if(((*operand)->getType()->getContainedType(0))->getContainedType(0)->getTypeID()== Type::ArrayTyID) {
									// check if instruction is a getelementptr - we are indexing into a 2-d array
									if ((*b).getOpcode( ) == Instruction::GetElementPtr) {
										ArrayType* arrayType = dyn_cast<ArrayType>(((*operand)->getType()->getContainedType(0))->getContainedType(0));
										widthInBits = arrayType->getElementType()->getPrimitiveSizeInBits();
										width = (unsigned) ceil ( (float) ( widthInBits )/8 ); 
										width =  width*(arrayType->getNumElements());
										temp_op_info->noOfColumns = arrayType->getNumElements(); 
									}
								}
							}
							// Allocate 
							temp_op_info->isPointer = true;
							temp_op_info->name = (*operand)->getName().str();
							temp_op_info->width = width;
							temp_op_info->size = size;
							temp_op_info->isArray = isArray;
							// Note that the operand_map is getting updated only in the case the pointer is to an array
							operand_map [(*operand)->getName ()] = temp_op_info;
							perInstrOpInfo->push_back (temp_op_info);
						} 
						else {
							perInstrOpInfo->push_back(OpMap_iter->second);
						}
					}
					// case of operand is a tid?
					else if (((*operand)->getName ().str()).find ("tid") != std::string::npos) {
						op_info* temp_op_info = new op_info;
						temp_op_info->isTID = true;
						temp_op_info->name = "tid";
						temp_op_info->width = width;
						perInstrOpInfo->push_back (temp_op_info);
						cerr << "\t\t\tFound tid operand"<<endl;
					}
					// case of operand being a floating point
					else if ( isa<ConstantFP> (*operand) ) {
						op_info* temp_op_info = new op_info;
						temp_op_info->isLiteral = true;
						temp_op_info->isFixedPoint = false;
						temp_op_info->width = width;
						const fltSemantics * FPtype = &((ConstantFP *)&*(*operand))->getValueAPF().getSemantics();
						if ( FPtype == (const llvm::fltSemantics*)&llvm::APFloat::IEEEdouble ) {
							temp_op_info->valueFP = ((ConstantFP *)&*(*operand))->getValueAPF().convertToDouble();
						} else {
							if ( FPtype == (const llvm::fltSemantics*) &llvm::APFloat::IEEEsingle ) {
								temp_op_info->valueFP = ((ConstantFP *)&*(*operand))->getValueAPF().convertToFloat();
							} else {
								cerr << "\t\t\tFound Float that is not of type double, nor of type single. valueFP = 0.0\n";
							}
						}
						perInstrOpInfo->push_back (temp_op_info);
					}
					// case of unclassfied operand - we just push back an empty op_info
					else {
						op_info* temp_op_info = new op_info;
						perInstrOpInfo->push_back (temp_op_info);
						cerr << "\t\t\t"<<(*operand)->getName ().str() << ": Found an operand that is not instruction/literal/basic block/tid. Skipping " <<endl;
						// (*operand)->getType()->dump();
						// cerr << (*operand)->getType()->getStructName().str() << (*operand)->getType()->getTypeID() << " " << "\n"; //(*operand)->getType()->getContainedType(0) <<"\n";
						//cerr << "\n";
						//exit (-1);
						continue;
					}
				} 
				// for (User::op_iterator ...) Iterate over all the operands of the instruction
				cerr << "\t\tTotal OPs = " << perInstrOpInfo->size() <<endl;

				// Call the abstractCompute function
				bool result = abstractCompute (i, opcode, cmp_pred, op_info_struct, perInstrOpInfo, isSerial);
				has_changed |= result;
				cerr << "\t(changed = " << result;
				cerr << ")   ";
				op_info_struct->print();
				cerr << "\n";

				//Need to clear up the perInstrOpInfo vector here
				//We clear the perInstrOpInfo, how does the constraints from each iteration get saved -- in the abstractConstraintsMap?
				unsigned k;
				unsigned size = perInstrOpInfo->size();
				for (k = 0; k < size; k++) {
					if ( ((*perInstrOpInfo)[k]->isInstruction == true) |
							((*perInstrOpInfo)[k]->isPointer == true)
					   ) {
						//Do nothing - we have to keep this information around
					} else {
						delete (*perInstrOpInfo)[k];
					}
				}
			} // for (BasicBlock iterator ...) Iterate over all the instructions of the BB
		} // for (FunctionIterator ...) Iterate over all BBs of the Function

		first_pass = false;
		num_passes++;

		// If we have iterated over the code for more than the specified number of times then it is time
		// to widen the ranges and bring the analysis to closure. So we'll initiate a special procedure as
		// below. After this we should be terminating.

		if ((num_passes == NUM_TRIALS)&&(!widened)) {
			// Widen the range of each op_info in our map
			InstrToInfoMap::iterator InfoMap_iter;
			for (InfoMap_iter = info_map.begin (); InfoMap_iter != info_map.end (); InfoMap_iter++) {
				op_info* oi_struct = InfoMap_iter->second;
				if(!oi_struct->try_widen) continue;
				for (unsigned l = 0; l < oi_struct->abstractDomain.setabstractDomVec.size(); l++) {
					std::set<abstractDom, abstractDomCompare>::iterator ait;
					SetabstractDom_t temp;
					for (ait = oi_struct->abstractDomain.setabstractDomVec[l].SetabstractDom.begin (); ait != oi_struct->abstractDomain.setabstractDomVec[l].SetabstractDom.end (); ait++) {
						abstractDom aD = (*ait);
						abstractWiden ( aD );
						temp.insert(aD);
					}
					oi_struct->abstractDomain.setabstractDomVec[l]=temp;
				}
			}
			abstractWidenMemory ();
			widened=true;
		}
	}

	//Go through the info_map
	printInfoMap();
	cerr << "Read Set: \n"; 
	printOpInfoSet (rd_set);
	cerr << "Write Set:\n";
	printOpInfoSet (wr_set);

	cerr << "Number of passes = " << num_passes << "\n";

	// If we had to abstractCompute for num_passes > 1, then things had changed, so
	// we need to convey that back - so that this entire process can be repeated for
	// the entire program. The information gained from this run can be leveraged 
	// for analyzing the other functions
	if (num_passes > 1) {
		return true;
	} 
	else {
		return false;  
	}

	return false;
}

bool PointerAnalysis::analyzeFunctions(Module &M) {
	Function* func = M.getFunction("pthread_create");
	bool multi_threaded_analysis = false; 
	set <Function*> thread_functions; // Holds Function* called as part of pthread_create
	pair < set <Function*>::iterator, bool > ret;
	vector <FunctionAnalysis*> threadF_vec;

	FunctionAnalysis mainF;
	if (func == 0) {
		cerr << "pthread_create not found" << endl;
		// return false; // HACK
	}
	else {
		multi_threaded_analysis = true;
		cerr << "pthread_create found = "<< func->getName().str() << endl;

		// iterate through each CallInst of pthread_create and identify thread's function. 
		for (Use &U : func->uses()) {
			User *UR = U.getUser();
			if (CallInst *CI = cast<CallInst>(UR)) {
				Value* fn_arg = CI->getOperand(2);
				errs() << "pthread create called with :" << fn_arg->getName().str() << "\n";
				string threadFunction;
				Function *worker = M.getFunction(fn_arg->getName().str());
				for (Function::iterator i = worker->begin(); i != worker->end(); i++) {
					for (BasicBlock::iterator b = (*i).begin(); b != (*i).end(); b++) {
						if (CallInst* callInst = dyn_cast <CallInst> (b) ) {
							//cerr << "found a callinst of " << callInst->getCalledFunction()->getName().str() << std::endl;
							threadFunction = callInst->getCalledFunction()->getName().str();
              if (threadFunction.find("tid") != string::npos) 
                break;
						}
					} 		
				}
				ret = thread_functions.insert(M.getFunction(threadFunction));
				// If the function was not already in the set
				if (ret.second == true) {
					FunctionAnalysis* threadF = new FunctionAnalysis;
					threadF->setName (threadFunction);
					threadF_vec.push_back (threadF);
					cerr << "pushing " << threadFunction << " to threadF_vec "<<endl;
				}
			}
			else {
				cerr << "use of pthread_create was not a CallInst!"<<endl;
			}
		}
	}
	
	// now call abstractCompute on the main function and each thread function
	bool changed = true;
	while (changed == true) {
		changed = mainF.processFunction ( *(M.getFunction ("main")) , true); // isSerial = true
		cerr<<"Main analysis over. changed = "<<changed<<endl;
		if (multi_threaded_analysis) {
			std::set < Function *>::iterator f_it;
			unsigned i=0;
			for (f_it = thread_functions.begin (); f_it != thread_functions.end (); f_it++) {
				changed |= threadF_vec[i]->processFunction (*(*f_it), false);
				i++;
			}
      
      for (unsigned i = 0; i < threadF_vec.size (); i++) {
        threadF_vec[i]->computeConflicts ();
      }

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


