#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "clp/clp.h"

#include <fstream>
#include <iostream>
#include <cstdio>
#include <set>
#include <vector>

using namespace llvm;
using namespace std;

// global macros go here
#define DEBUG_TYPE "PointerAnalysis"
#define NUM_THREADS   1
#define NUM_TRIALS    1000
#define POST_WIDEN_TRIALS   1000


// Rama - a multi-threaded pointer analyzer built by Aditya Venkataraman and Naveen Neelakantan

namespace Rama {
  
class abstractDom {
  public:
    
    abstractDom (void) {
      CLEAR_CLP(clp); name = ""; upper_changed=false; lower_changed=false;
    }
    
    abstractDom (clp_t x) {
      FILL_CLP(clp,x.l,x.u,x.d); name = ""; upper_changed=false; lower_changed=false;
    }

    abstractDom (clp_t x, std::string name) {
      if(EMPTY(x)) {
        CLEAR_CLP(clp);
        name="";
      }
      else {
	      FILL_CLP(clp,x.l,x.u,x.d); this->name = name; upper_changed=false; lower_changed=false;
      }
    }
    
    abstractDom (sword_t l,sword_t u, sword_t d) {
      FILL_CLP(clp,l,u,d); name = ""; upper_changed=false; lower_changed=false;
    }
    
    abstractDom (sword_t l,sword_t u, sword_t d, std::string name) {
      FILL_CLP(clp,l,u,d); this->name = name; upper_changed=false; lower_changed=false;
      if(EMPTY(clp)) {
	      this->name="";
      }
    }
    
    abstractDom (const abstractDom &rhs) {
      *this=rhs;
    }
    
    ~abstractDom (void) {}
    
    void reset() {CLEAR_CLP(clp); upper_changed=false; lower_changed=false;}
    
    bool operator != (const abstractDom rhs) const {
      return ((DIFFER_CLP(clp,rhs.clp))||(name!=rhs.name));
    }
    
    bool operator == (const abstractDom rhs) const {
      return ((SAME_CLP(clp,rhs.clp))&&(name==rhs.name));
    }
    
    // semantic of comparison looks at l first, then u and lastly d of the CLP. 
    bool operator < ( const abstractDom &rhs) const {
      if ( name < rhs.name ) return true;
      if ( name > rhs.name ) return false;
      if(SAME_CLP(clp,rhs.clp)) return false;
      if(clp.l<rhs.clp.l) return true;
      if(clp.l>rhs.clp.l) return false;
      if(clp.u<rhs.clp.u) return true;
      if(clp.u>rhs.clp.u) return false;
      if(clp.d<rhs.clp.d) return true;
      if(clp.d>rhs.clp.d) return false;
      return false;
    }
    
    abstractDom& operator=(const abstractDom &rhs) {
      clp=rhs.clp;
      name=rhs.name;
      upper_changed=rhs.upper_changed;
      lower_changed=rhs.lower_changed;
      return *this;
    }
    
    bool print ( ) const {
      
      if(EMPTY(clp)) return false;
      
      if ( name.empty() == false ) {
        cerr << "(" << name << ")";
      }
      PRINT_CLP(clp);
      
      return true;
    }

    abstractDom binary_op (clp_op_t op, const abstractDom &rhs2, std::string name) const {
      return abstractDom (clp_fn (op, this->clp, rhs2.clp, false), name);
    }
    
    clp_t clp;
    std::string name;
    bool upper_changed;
    bool lower_changed;
  };

  struct abstractDomCompare {
    bool operator()(abstractDom ip1, abstractDom ip2) {
      /*      if (ip1.name < ip2.name) {
	      return true;
	      }
	      return false;*/
      return (ip1<ip2);
    }
  };

  class SetabstractDom_t {
  
  public:
    SetabstractDom_t (void) {}

    SetabstractDom_t (abstractDom ip) { SetabstractDom.insert ( ip );}
    
    SetabstractDom_t (const SetabstractDom_t &rhs) {
      *this=rhs;
    }

    ~SetabstractDom_t (void) {}
    
    SetabstractDom_t& operator=(const SetabstractDom_t &rhs) {
      SetabstractDom=rhs.SetabstractDom;
      return *this;
    }
    
    SetabstractDom_t& operator=(SetabstractDom_t &rhs) {
      SetabstractDom=rhs.SetabstractDom;
      return *this;
    }
    
    bool operator != (const SetabstractDom_t rhs) {
      if (SetabstractDom.size() != rhs.SetabstractDom.size()) return true;
      std::set<abstractDom>::iterator it;
      std::set<abstractDom>::iterator it1;
      for(it=SetabstractDom.begin(),it1=rhs.SetabstractDom.begin();it!=SetabstractDom.end();it++,it1++) {
        if((*it)!=(*it1)) return true;
      }
      return false;
    }
    
    bool operator < (const SetabstractDom_t &rhs) const {
      if ( SetabstractDom.size() <  rhs.SetabstractDom.size() ) {
	      return true;
      }

      if ( SetabstractDom.size() >  rhs.SetabstractDom.size() ) {
	      return false;
      }

      std::set<abstractDom>::const_iterator it;
      std::set<abstractDom>::const_iterator it1;
      for (it=SetabstractDom.begin(),it1=rhs.SetabstractDom.begin(); it!=SetabstractDom.end(); it++,it1++) {
	      if ( (*it) < (*it1) ) {
	        return true;
	      }
      }
      return false;
    }
    
    void print (void) const {
      std::set<abstractDom>::iterator it;
      
      for (it = SetabstractDom.begin (); it!=SetabstractDom.end (); it++) {
	      bool non_empty=it->print ();
	      // it++;
	      /*
        if (it == SetabstractDom.end ()) {
	        break;
	      }
        */
	      if(non_empty)
	        cerr << ", ";
      }
    }
    
    bool find (const abstractDom& ip) const {
      std::set<abstractDom, abstractDomCompare>::iterator it2;
      for (it2 = SetabstractDom.begin () ; it2 != SetabstractDom.end (); it2 ++) {
	      if (it2->name == ip.name) {
	        return true;
	      }
      }
      return false;
    }
    
    void insert (abstractDom ip) {
      SetabstractDom.insert (ip);
    }
    
    void insert (SetabstractDom_t ip) {
      std::set<abstractDom, abstractDomCompare>::iterator it;
      for (it = ip.SetabstractDom.begin (); it != ip.SetabstractDom.end (); it++) {
	      SetabstractDom.insert ( *it );
      }
    }
    
    SetabstractDom_t binary_op (clp_op_t op, const SetabstractDom_t &rhs2) const {
      SetabstractDom_t temp;
      std::set<abstractDom, abstractDomCompare>::iterator it;
      std::set<abstractDom, abstractDomCompare>::iterator it2;
      
      switch (op) {
        case CLP_UNION:
	        for (it = rhs2.SetabstractDom.begin () ; it != rhs2.SetabstractDom.end (); it ++) {

	          for (it2 = SetabstractDom.begin () ; it2 != SetabstractDom.end (); it2 ++) {
              // If the name is the same then take union
	            if ( it->name == it2->name) {
	              temp.insert ( it2->binary_op (op, *it, it->name) );
	            }
            }
          }

	
          // For the ones which did not have a matching name in the other set we need to copy them over
	        // individually.
	        // First check the 1st set
	
          for (it2 = SetabstractDom.begin () ; it2 != SetabstractDom.end (); it2 ++) {
            if ( rhs2.find (*it2) == false ) {
              temp.insert ( it2->binary_op (op, abstractDom(0, 0, 0), it2->name) );
            }
          }
	        
          // Now check the 2nd set
	        for (it = rhs2.SetabstractDom.begin () ; it != rhs2.SetabstractDom.end (); it ++) {
	          if ( this->find (*it)  == false ) {
	            temp.insert ( it->binary_op (op, abstractDom(0, 0, 0), it->name) );
	          }
	        }
          break;

      case CLP_INTERSECT:
	      // find the names that are common in both sets and only process those
	      for (it = rhs2.SetabstractDom.begin () ; it != rhs2.SetabstractDom.end (); it ++) {
	        for (it2 = SetabstractDom.begin () ; it2 != SetabstractDom.end (); it2 ++) {
	          if ( it->name == it2->name) {
	            temp.insert ( it2->binary_op (op, *it, it->name) );
	          }
	        }
	      }
        break;

      case CLP_ADD:
      case CLP_ADD_EXPAND:
      case CLP_SUB:
	      for (it = rhs2.SetabstractDom.begin () ; it != rhs2.SetabstractDom.end (); it ++) {
	        for (it2 = SetabstractDom.begin () ; it2 != SetabstractDom.end (); it2 ++) {
	          if ( ((it->name).empty() == false ) & ( (it2->name).empty() == false) ) {
	            if ( it->name == it2->name) {
                temp.insert ( it2->binary_op (op, *it, it->name) );
	            } 
              else {
	      	      // Both have names but they dont match so do nothing
	            }
	          } 
            else {
	            // This is the case when at most one has a name
	            // we take union of names since at least one of them will be empty
	            temp.insert ( it2->binary_op (op, *it, it->name + it2->name) );
	          }
	        }
	      }
	      
	      for (it2 = SetabstractDom.begin () ; it2 != SetabstractDom.end (); it2 ++) {
	        if ((it2->name).empty() == false) {
	          if ( rhs2.find (*it2) == false ) {
	            temp.insert ( it2->binary_op (op, abstractDom(0, 0, 0), it2->name) );
	          }
	        }
	      }
	
        break;

      default:
	      // In all other cases only the literals are processed since those operators on
	      // pointers do not make sense.
	      for (it = rhs2.SetabstractDom.begin () ; it != rhs2.SetabstractDom.end (); it ++) {
	        for (it2 = SetabstractDom.begin () ; it2 != SetabstractDom.end (); it2 ++) {
	          if ( ((it->name).empty() == true ) & ( (it2->name).empty() == true ) ) {
	            temp.insert ( it2->binary_op (op, *it, it->name) );
	          }
          }
	      }
      }
      return temp;
    }

    std::set<abstractDom, abstractDomCompare> SetabstractDom;
  };


  class SetabstractDomVec_t {
  
    public:
      SetabstractDomVec_t (void) { }
      
      SetabstractDomVec_t (const SetabstractDomVec_t &rhs) {
        *this=rhs;
      }
      
      ~SetabstractDomVec_t (void) { }

      SetabstractDomVec_t& operator=(const SetabstractDomVec_t &rhs) {
        setabstractDomVec=rhs.setabstractDomVec;
        return *this;
      }

      SetabstractDomVec_t binary_op (clp_op_t op, const SetabstractDomVec_t &rhs2) const {
        assert (setabstractDomVec.size() == rhs2.setabstractDomVec.size());
        SetabstractDomVec_t temp;
        for (unsigned i = 0; i < setabstractDomVec.size(); i++) {
          temp.setabstractDomVec.push_back ( setabstractDomVec[i].binary_op (op, rhs2.setabstractDomVec[i] ));
        }
        return temp;
      }

    
      bool operator != (SetabstractDomVec_t &rhs) {
        if(setabstractDomVec.size()!=rhs.setabstractDomVec.size()) return true;
        std::vector<SetabstractDom_t>::iterator it;
        std::vector<SetabstractDom_t>::iterator it1;
      
        for(it=setabstractDomVec.begin(),it1=rhs.setabstractDomVec.begin();it!=setabstractDomVec.end();it++,it1++) {
	        if((*it)!=(*it1)) return true;
        }
        return false;
      }

      bool operator < (const SetabstractDomVec_t &rhs) const {
        if (setabstractDomVec.size() < rhs.setabstractDomVec.size()) {
	        return true;
        }
      
        if (setabstractDomVec.size() > rhs.setabstractDomVec.size()) {
	        return false;
        }
      
        std::vector<SetabstractDom_t>::const_iterator it;
        std::vector<SetabstractDom_t>::const_iterator it1;
        for (it=setabstractDomVec.begin(),it1=rhs.setabstractDomVec.begin(); it!=setabstractDomVec.end(); it++,it1++) {
	        if ( (*it) < (*it1) ) {
	          return true;
	        }
        }
        return false;
      }

      void clear (void) { setabstractDomVec.clear(); }

    
      void print (void) const {
        
        for (unsigned i = 0; i < setabstractDomVec.size (); i++) {
	        cerr << "[ ";
	        setabstractDomVec[i].print();
	        cerr << "] ";
        }
      }
      
      void push_back (abstractDom ip) {
        setabstractDomVec.push_back(ip);
      }

      void push_back (SetabstractDom_t ip) {
        setabstractDomVec.push_back(ip);
      }

      // This function gives a cross-thread intersection. 
      // setabstractDomVec is vector in which each element represents a thread. Each thread
      // has a set of abstractDomains.
      
      // the result[0] will be the memory regions touched by current thread (=0) on (this) vector & other threads on other vector
      SetabstractDom_t crossVec_intersect (const SetabstractDomVec_t& ip) const {
        SetabstractDom_t result;
        assert (setabstractDomVec.size () == ip.setabstractDomVec.size ());

        for (unsigned z = 0; z < setabstractDomVec.size (); z++) {
	        for (unsigned y = 0; y < ip.setabstractDomVec.size (); y++) {
	          if (z != y) {
	            result.insert ( setabstractDomVec[z].binary_op ( CLP_INTERSECT, ip.setabstractDomVec[y] ) );
	          }
	        }
        }
        return result;
      }

      std::vector<SetabstractDom_t> setabstractDomVec;
  };

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
        abstractDomain.clear(); 
      }
      
      op_info (SetabstractDomVec_t ip) {
        *this=op_info();
        this->abstractDomain = ip;
      }

      ~op_info() {}

      op_info& operator=(const op_info &rhs) {
        isTID=rhs.isTID;
        value=rhs.value;
        valueFP=rhs.valueFP;
        size=rhs.size;
        width=rhs.width;
        name=rhs.name;
        
        BasicBlockPtr=rhs.BasicBlockPtr;
        FunctionPtr=rhs.FunctionPtr;
        isLiteral=rhs.isLiteral;
        isFixedPoint=rhs.isFixedPoint;
        isInstruction=rhs.isInstruction;
        isBasicBlockPtr=rhs.isBasicBlockPtr;
        isFunctionPtr=rhs.isFunctionPtr;
        isPointer=rhs.isPointer;
        isAlloca=rhs.isAlloca;
        isCall=rhs.isCall;
        
        auxilliary_op=rhs.auxilliary_op;
        
        abstractDomain = rhs.abstractDomain;
        
        try_widen = rhs.try_widen;
        return *this;
      }
    
      op_info (const op_info &rhs) {
        *this=rhs;
      }
      
      void print ( ) {
        abstractDomain.print ( );
      }

      void pushToDomVec (abstractDom ip) {
        abstractDomain.push_back (ip);
      }

      op_info binary_op (clp_op_t op, const op_info &rhs2) {
        return abstractDomain.binary_op ( op, rhs2.abstractDomain  );
      }

      op_info operator + (const op_info &rhs2) {
        return binary_op (CLP_ADD, rhs2);
      }

      op_info operator - (const op_info &rhs2) {
        return binary_op (CLP_SUB, rhs2);
      }

      op_info operator * (const op_info &rhs2) {
        return binary_op (CLP_MULT, rhs2);
      }

      op_info operator / (const op_info &rhs2) {
        return binary_op (CLP_DIV, rhs2);
      }

     
      // This function gives a cross-thread intersection. 
      // abstractDomVec is vector in which each element represents a thread. Each thread
      // has a set of abstractDomains.
      SetabstractDom_t crossVec_intersect (op_info& ip) {
        return abstractDomain.crossVec_intersect (ip.abstractDomain);
      }

      bool isTID;
      int value;
      double valueFP; 
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
      SetabstractDomVec_t cmp_val; 
      CmpInst::Predicate cmp_pred;

      bool try_widen;

      // Each entry in this vector represents a thread
      SetabstractDomVec_t abstractDomain; 
  };
  
  typedef std::set <op_info*> op_info_set;
  typedef std::set < SetabstractDom_t >  set_SetAbstractDom;
  typedef std::vector <op_info*> op_info_vec;
  typedef std::vector<op_info>  op_info_struct_vec;

        
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

      virtual void print (std::ostream &O, const Module *M) const {
        O << "This is FunctionAnalysis\n";
      }

      virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        // we don't modify the program, so we preserve all analyses
        AU.setPreservesAll();
      }
      
      void computeConflicts();

      void setName (string name) { this->name = name; }
    
    private:

      void printOpInfo (op_info*);
      void printOpInfoSet ( std::set <SetabstractDomVec_t> &);
      void printAbstractInfo (abstractDom&);

      bool abstractCompute (BasicBlock*, unsigned, llvm::CmpInst::Predicate, op_info*, op_info_vec*, bool);
      void abstractInit();
      void bbStart (BasicBlock*, std::vector <BasicBlock*>*);
      void abstractWiden(abstractDom &);
      void abstractWidenMemory();
      void addToReadSet ( SetabstractDomVec_t &);
      void addToWriteSet ( SetabstractDomVec_t &);

      string name;
      std::set < SetabstractDomVec_t > rd_set;
      std::set < SetabstractDomVec_t > wr_set;
      SetabstractDom_t rd_wr_conflicts;
      SetabstractDom_t wr_wr_conflicts;
      
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

      // per module callback to be registered with llvm
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

