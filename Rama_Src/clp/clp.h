/****************************************************************************************/
/*
 */
/*
 * clp.h: This file provides the header code for 32 bit CLP data structure 
 * and 
 */
/*
 * transfer functions 
 */
/*
 */
/*
 * Author: Rathijit Sen (rathi@csa.iisc.ernet.in) 
 */
/*
 * Department of Computer Science and Automation, 
 */
/*
 * Indian Institute of Science, Bangalore. 
 */
/*
 */
/*
 * This code appears as part of the report: 
 */
/*
 * "Executable Analysis with Circular Linear Progressions" 
 */
/*
 * by R. Sen, Y.N. Srikant 
 */
/*
 * IISc-CSA-TR-2007-3 
 */
/*
 * http://archive.csa.iisc.ernet.in/TR/2007/3/ 
 */
/*
 * as an online companion for similarly titled submission towards MEMOCODE 
 * 2007 
 */
/*
 */
/*
 * Contact: {rathi,srikant}@csa.iisc.ernet.in 
 */
/*
 */
/****************************************************************************************/
#ifndef _CLP_H
#define _CLP_H

//extern bool print_hex;

#define MAX_32BIT_DATA 0xffffffffL
#define true 1
#define false 0
#define TOP(x) (((x).l==((x).u+1))&&((x).d==1))
#define CONSTANT(x) (((x).l==(x).u)&&((x).d))

#define EMPTY(x) ((x).d==0)
#define ONE(t) (CONSTANT(t)&&(t.l==1))
#define ZERO(t) (CONSTANT(t)&&(t.l==0))
#define MAKE_TOP(t) { \
(t).l=(t).u+1; \
(t).d=1; \
}
#define CLEAR_CLP(t) { \
(t).l=0; \
(t).u=0; \
(t).d=0; \
}
#define FILL_CLP(t,x,y,z) {fill_elt(&t,(x),(y),(z));}
#define DIFFER_CLP(t1,t2) ((!(TOP(t1)&&TOP(t2)))&&(!(EMPTY(t1)&&EMPTY(t2)))&&((t1.l!=t2.l)||(t1.u!=t2.u)||(t1.d!=t2.d)))
#define SAME_CLP(t1,t2) (!DIFFER_CLP(t1,t2))
#define PRINT_CLP(t) { \
if(EMPTY(t)) \
fprintf(stderr,"E "); \
else if(TOP(t)) \
fprintf(stderr,"T "); \
else if(CONSTANT(t)) \
{ \
if(print_hex) \
fprintf(stderr,"%x ",t.l); \
else \
fprintf(stderr,"%d ",t.l); \
} \
else \
{ \
if(print_hex) \
fprintf(stderr,"{%x,%x,%x} ",t.l,t.u,t.d); \
else \
fprintf(stderr,"{%d,%d,%u} ",t.l,t.u,t.d); \
} \
}
#define NUM(x) clp_size(x)
#define MAY_NZERO(t) (!(EMPTY(t)||(CONSTANT(t)&&(t.l==0))))
typedef unsigned int word_t;	// n=32
typedef signed int sword_t;
typedef unsigned long long qword_t;	// 2n=64
typedef signed long long sqword_t;
struct clp_t
{
  sword_t l;
  sword_t u;
  word_t d;
};
typedef struct clp_t clp_t;
// //////////////////////////// Transfer functions
// //////////////////////////////////
enum clp_op_t
{
  CLP_UNION = 0,
  CLP_INTERSECT = 1,
  CLP_DIFFERENCE = 2,
  CLP_ADD = 3,
  CLP_SUB = 4,
  CLP_MULT = 5,
  CLP_DIV = 6,
  CLP_LSHIFT = 7,
  CLP_RSHIFT = 8,
  CLP_BITC = 9,
  CLP_AND = 10,
  CLP_OR = 11,
  CLP_XOR = 12,
  CLP_EQ = 13,
  CLP_NEQ = 14,
  CLP_LEQ = 15,
  CLP_GEQ = 16,
  CLP_LESSER = 17,
  CLP_GREATER = 18,
  CLP_ANDL = 19,
  CLP_ORL = 20,
  CLP_NOTL = 21,
  CLP_ADD_EXPAND = 22
};
typedef enum clp_op_t clp_op_t;
typedef clp_t (*clp_fn_ptr) (clp_t, clp_t);
clp_t clp_fn (clp_op_t clp_op, clp_t a, clp_t b, bool repeat);
clp_t clp_fn_const (clp_op_t clp_op, clp_t a, sword_t x);
clp_t clp_const_fn (clp_op_t clp_op, sword_t x, clp_t a);
// //////////////////////////// constants
// ///////////////////////////////////////
extern const sword_t MAX_N, MAX_P;
extern clp_t zero, one, top;
extern bool print_hex;
// /////////////////////////// other functions
// //////////////////////////////////////
void init_clp ();
clp_t mk_lp (sword_t x);
qword_t clp_size (clp_t x);
clp_t tight_clp (clp_t a, clp_t b, bool * eq, bool * first);
int gcd (sword_t a, sword_t b);
void fill_elt (clp_t * x, sqword_t l, sqword_t u, qword_t d);

extern const sword_t MAX_N;
extern const sword_t MAX_P;
#endif
