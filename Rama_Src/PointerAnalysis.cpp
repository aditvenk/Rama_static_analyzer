#include "PointerAnalysis.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"

#include <sstream>

using namespace llvm;
using namespace std;
using namespace Rama;

typedef std::map <Instruction*, op_info*> InstrToInfoMap;
typedef std::map <std::string, op_info*> OpToInfoMap;
typedef std::map <Instruction*, std::string> InstrToNameMap;

InstrToNameMap instr_map;
InstrToInfoMap info_map;
OpToInfoMap operand_map;
int instr_count;


/* Register Passes with LLVM */
char PointerAnalysis::ID = 0;
static RegisterPass<PointerAnalysis> X("rama", "Multi-threaded pointer analyzer");

char FunctionAnalysis::ID = 0;
static RegisterPass<FunctionAnalysis> Y("rama_functions", "Rama function analysis", true, false);

// this function is called on each High Level Function -- i.e. Main and each WorkerThreadFunction
bool FunctionAnalysis::processFunction (Function& F, bool isSerial) { // second arg is true if F will be executed by a single flow of control
  string fnName = F.getName().str();
  cerr <<"processFunction called on "<<fnName<<endl;

  int instr_count = 0;
  int sub_instr_count = 0;

  bool has_changed = true;
  int num_passes = 0;
  bool isNonNumInstr;

  op_info_vec* perInstrOpInfo = new op_info_vec;
  abstractInit();

  while (has_changed == true) {
    // clear up the read and write sets of this function at the start of each iteration
    // since abstractCompute will attempt to populate it every time. 

    rd_set.clear();
    wr_set.clear();

    assert ( num_passes <= NUM_TRIALS /* + POST_WIDEN_TRIALS */); // TODO - implement post-widen

    has_changed = false;
    for (Function::iterator i = F.begin(); i != F.end(); i++) {
      cerr << "BASIC BLOCK: "<<(*i).getName().str() << "\n";
    
      // TODO - find out what is a landing pad basic block
      if ( i->isLandingPad() ) {
        cerr << " This is a landing pad basic block \n";
      }
      
      // before we go over each instruction in the bb, we need to set up bb maps.

      std::vector <BasicBlock*> *i_pred = new std::vector <BasicBlock*>;

      for (pred_iterator I = pred_begin(i), E = pred_end(i); I != E; ++I) {
        cerr << " PREDECESSOR = "<< (*I)->getName().str() << "\n";
        i_pred->push_back(*I);
      }

      bbStart (i, i_pred); // TODO - what does this function do?

      for (BasicBlock::iterator b = (*i).begin(); b != (*i).end(); b++) {
	      // If the instruction is of no interest to us skip its processing
	      if ( (*b).getOpcode ( ) == Instruction::Unreachable ) {
	        cerr << "  Found unreachable instr\n";
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
    
        // First check whether we have already encountered this instruction .
	      // If yes then retrive its op_info_vec, else 
	      // Allocate a new op_info structure and insert in the info_map

        InstrToInfoMap::iterator iit = info_map.find((Instruction *)&*(b));
        op_info* op_info_struct; // Holds the op_info_vec for this instruction
        if (iit == info_map.end()) { // not in info map
          // Prepare to log the instruction in instr_map and info_map
	        std::stringstream out;
	        out << instr_count;
	        if ( isNonNumInstr == true ) {
	          out << "." << sub_instr_count;
	          sub_instr_count++;
	        }
	        std::string instrName = "%" + fnName + "#" +  out.str ();
	        op_info_struct = new op_info;
	        op_info_struct->isInstruction = true;
	        std::string instr_name = (*b).getName ().str();
	        if (instr_name.find ("tid") != std::string::npos) {
	          op_info_struct->isTID = true;
	          cerr << "  Found TID instr : "<<instr_name<<endl;
	        }
          
	        // (*b) is the pointer to the instruction
	        info_map [b] = op_info_struct;
	        instr_map [b] = instrName; //_count;

	        if ((*b).getOpcode ( ) == Instruction::Alloca) { 
	          //cerr << "Found alloca";
	          // std::string name = "%" + fnName + "#";
	          // name = name + out.str();
            // op_info_struct->name = name;
            op_info_struct->name = instrName;
	          op_info_struct->isAlloca = true;
	        }
	        if ((*b).getOpcode ( ) == Instruction::Call) { 
	          //cerr << "Found Call: " << (*b).getOpcodeName () << "\n";
	          // std::string name = "%" + fnName + "#";
	          // name = name + out.str();
	          // op_info_struct->name = name;
            op_info_struct->name = instrName;
	          op_info_struct->isCall = true;
	        }
	        if (isNonNumInstr == false) {
	          instr_count++;
	          sub_instr_count = 0;
	        }
        }  
        else {
	        op_info_struct = iit->second;
	        // Now check whether the instruction is in our instr_map
	        // If not then we need to put it in
	        InstrToNameMap::iterator map_iter = instr_map.find((Instruction *)&*(b));
	        if (map_iter == instr_map.end()) {
            cerr << "  BAD BAD BAD BAD "<<endl;
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
	      cerr << "  "<<instr_map[b] << ":" << "\t opcode name = " << (*b).getOpcodeName () << "\n";
	      unsigned opcode = (*b).getOpcode(); // Holds the opcode of the instruction
        cerr<< "  "<<instr_map[b] << ":" << "\t opcode value = " << (*b).getOpcode() << endl;

        // If it is a cmp instruction extract the predicate
	      llvm::CmpInst::Predicate cmp_pred;
	      if (isa <ICmpInst> (*b)) {
	        cmp_pred = ((ICmpInst *)&* (b))->getSignedPredicate();
	      }

	      if (opcode == Instruction::Alloca) { 
	        //cerr << "Found alloca";
	        AllocaInst* a_inst = dyn_cast<AllocaInst> (b);
	        //cerr << "operand name = " << op_info_struct->name << (a_inst->getArraySize())->getName();
	        //cerr << "array size = " << (a_inst->getAllocatedType())->getArrayNumElements();
	        cerr<<"  ";
          (a_inst->getArraySize())->dump();
	        //cerr << "\n";
	        //if (isa<Constant> (a_inst->getArraySize()) ) {
	        //  cerr << "val = " << (a_inst->getArraySize())->getIntegerValue();
	        //}
	      }

        for (User::op_iterator operand = (*b).op_begin(); operand != (*b).op_end(); operand++) {
          cerr << "   Name = " << (*operand)->getName().str() << " Type = " << (*operand)->getType ()<<endl;
          // Find out interesting details about this operand
	        // First find the width of the "eventual" data in the operand.
	        unsigned widthInBits;
	        unsigned width;
	    
	        // Should really first check whether this is a primitive type operand before checking its width
	        const Type* dataType = (*operand)->getType();
	        widthInBits = dataType->getPrimitiveSizeInBits();

	        // If the type is of pointer then we need to find the data type it points to
	        if ( dataType->getTypeID() == Type::PointerTyID ) {
	          dataType = dataType->getContainedType (0);
	          // If the type is a pointer to a pointer then we need to find the data type it points to
	          if (dataType->getTypeID() == Type::PointerTyID) {
	            dataType = dataType->getContainedType(0);
	          }
	          
            // if the type is a pointer to an array we need to find the type of the array elements
	          if ( dataType->getTypeID() == Type::ArrayTyID ) {
	            cerr << "   ** Found Array ** " <<endl;
	            //cerr << (((ArrayType *)&*((*operand)->getType()->getContainedType(0)))->getElementType ())->getTypeID ();
	            dataType = ( (ArrayType *)&*(dataType) )->getElementType ();
	            //cerr << width;
	          }

	          widthInBits = dataType->getPrimitiveSizeInBits();

	          // If the operand is a structure then we need to go through the elements to compute
	          // its size. But we need to do this recurseively for any structs contained within.
	          // For now we assume there are no structs inside.
	      
	          if ( dataType->getTypeID() == Type::StructTyID ) {
		          unsigned numElements = dataType->getNumContainedTypes ();
		          unsigned z;
		          for (z = 0; z < numElements; z++) {
		            widthInBits += dataType->getContainedType(z)->getPrimitiveSizeInBits();
		          }
            }
          }
	        
          width = (unsigned) ceil ( (float) ( widthInBits )/8 ); // width of the operand in Bytes
          cerr << "   width(Bytes) = "<< width << " width(Bits) = "<<widthInBits<<endl;
          
          // every virtual register operand is of type Instruction and points to the instruction which generates it.
          if (isa <Instruction> (*operand)){
	          InstrToNameMap::iterator map_iter = instr_map.find((Instruction *)&*(*operand)); 
	          if (map_iter == instr_map.end()) {
		          cerr << "   IOpNotInMapYet" <<endl;
	          }
            else {
		          cerr << "   Instr found - " << map_iter->second <<endl;
	          }
	      
	          InstrToInfoMap::iterator InfoMap_iter = info_map.find((Instruction *)&*(*operand));
	          if (InfoMap_iter == info_map.end()) {
	  	        //cerr << "Not in info_map ";
		          // Allocate 
		          op_info* temp_op_info = new op_info;
		          temp_op_info->isInstruction = true;
		          temp_op_info->width = width;
		          // if the instruction was Alloca, i.e. the operand is an array then set that flag
		          if ( ((Instruction *)&*(*operand))->getOpcode () == Instruction::Alloca) {
		            temp_op_info->isAlloca = true;
		          }
		          info_map [(Instruction *)&*(*operand)] = temp_op_info;
		          perInstrOpInfo->push_back (temp_op_info);
            } 
            else {
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
          else if (isa<ConstantInt> (*operand)) {
		        op_info* temp_op_info = new op_info;
		        temp_op_info->isLiteral = true;
		        temp_op_info->value = ((ConstantInt *)&*(*operand))->getZExtValue();
		        temp_op_info->width = width;
		        perInstrOpInfo->push_back (temp_op_info);
		        cerr << "   Const " << temp_op_info->value <<endl;
          } 
          else if (isa<BasicBlock> (*operand)) {
		        op_info* temp_op_info = new op_info;
		        temp_op_info->isBasicBlockPtr = true;
		        temp_op_info->BasicBlockPtr = (BasicBlock *)&*(*operand);
		        temp_op_info->width = width;
		        perInstrOpInfo->push_back (temp_op_info);
		        cerr << "   BB " << (*operand)->getName().str()<<endl;
          } 
          else if ((*operand)->getType()->getTypeID() == Type::PointerTyID) { // operand is a ptr
            cerr << "   @" << (*operand)->getName ().str() <<endl;
            OpToInfoMap::iterator OpMap_iter = operand_map.find( (*operand)->getName () );
	            
            if (OpMap_iter == operand_map.end()) {
              // Following seems to get the type of data the pointer is pointing to
		          cerr << "   " << (*operand)->getType()->getTypeID() << " CT = " << ((*operand)->getType()->getContainedType(0))->getTypeID() << " width = " << ((*operand)->getType()->getContainedType(0))->getPrimitiveSizeInBits() <<endl;
              int size = 1;
		          bool isArray = false;
		          
              if (((*operand)->getType()->getContainedType(0))->getTypeID() == Type::ArrayTyID) {
		            cerr << "   ** Found Array ** "<<endl;
		            isArray = true;
		            size = ((ArrayType *)&*((*operand)->getType()->getContainedType(0)))->getNumElements();
              }
		          // Allocate 
		          op_info* temp_op_info = new op_info;
		          temp_op_info->isPointer = true;
		          temp_op_info->name = (*operand)->getName ().str();
		          temp_op_info->width = width;
		          temp_op_info->size = size;
		          temp_op_info->isArray = isArray;
		          operand_map [(*operand)->getName ()] = temp_op_info;
		          perInstrOpInfo->push_back (temp_op_info);
	                
            } 
            else {
		          perInstrOpInfo->push_back(OpMap_iter->second);
            }
          }
          else if (((*operand)->getName ().str()).find ("tid") != std::string::npos) {
            op_info* temp_op_info = new op_info;
		        temp_op_info->isTID = true;
		        temp_op_info->name = "tid";
		        temp_op_info->width = width;
		        perInstrOpInfo->push_back (temp_op_info);
		        cerr << "   Found tid operand"<<endl;
            exit(1); // TODO Remove this once you understand why this code is executed
          }
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
			          cerr << "   Found Float that is not of type double, nor of type single. valueFP = 0.0\n";
			        }
			      }
			      perInstrOpInfo->push_back (temp_op_info);
          }
          else {
			      op_info* temp_op_info = new op_info;
			      perInstrOpInfo->push_back (temp_op_info);
			      cerr << "   "<<(*operand)->getName ().str() << ": Found an operand that is not instruction/literal/basic block/tid. Skipping " <<endl;
			      (*operand)->getType()->dump();
			      cerr << (*operand)->getType()->getStructName().str() << (*operand)->getType()->getTypeID() << " " << "\n"; //(*operand)->getType()->getContainedType(0) <<"\n";
			      //cerr << "\n";
			      //exit (-1);
			      continue;
		      }
        } // for (User::op_iterator ...) Iterate over all the operands of the instruction
        cerr << "  Total OPs = " << perInstrOpInfo->size() <<endl;

        // Call the abstractCompute function
    	  bool result = abstractCompute (i, opcode, cmp_pred, op_info_struct, perInstrOpInfo, isSerial);

      } // for (BasicBlock iterator ...) Iterate over all the instructions of the BB
    } // for (FunctionIterator ...) Iterate over all BBs of the Function
    has_changed = false; //TODO - remove this --> AV Hack
  }
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
    changed = mainF.processFunction ( *(M.getFunction ("main")) , true); // isSerial = true
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


