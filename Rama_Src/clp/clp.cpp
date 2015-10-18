/****************************************************************************************/
/* */
/* clp.c: This file provides the source code for 32 bit CLP transfer functions */
/* */
/* Author: Rathijit Sen (rathi@csa.iisc.ernet.in) */
/* Department of Computer Science and Automation, */
/* Indian Institute of Science, Bangalore. */
/* */
/* This code appears as part of the report: */
/* "Executable Analysis with Circular Linear Progressions" */
/* by R. Sen, Y.N. Srikant */
/* IISc-CSA-TR-2007-3 */
/* http://archive.csa.iisc.ernet.in/TR/2007/3/ */
/* as an online companion for similarly titled submission towards MEMOCODE 2007 */
/* */
/* Contact: {rathi,srikant}@csa.iisc.ernet.in */
/* */
/****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "clp.h"
#define FILL_CLP_INTERNAL(t,x,y,z) { \
	assert(z); \
	if((sword_t)(x)==((sword_t)(y)+1)) \
	{ \
		t.l=((x)/(z))*(z); \
		t.u=((y)/(z))*(z); \
		t.d=(z); \
	} \
	else \
	{ \
		t.l=(x); \
		t.u=(y); \
		t.d=(z); \
	} \
}
const sword_t MAX_N=0x80000000;
const sword_t MAX_P=0x7fffffff;
bool repeat_flag=false;
clp_t zero,one,top;
bool print_hex=false;
////////////////////////////////////////// Auxiliary functions /////////////////////////////////////////////
sqword_t min(sqword_t a,sqword_t b)
{
	if((a>=0)&&(b<=0)) return b;
	if((b>=0)&&(a<=0)) return a;
	if(a<b) return a;
	return b;
}
sqword_t max(sqword_t a,sqword_t b)
{
	return -min(-a,-b);
}
int gcd(sword_t a,sword_t b) /* finds the gcd of 2 numbers */
{
	word_t divisor, dividend, remainder;
	if(!a) return b;
	if(!b) return a;
	if(a<0) a=-a; // gcd is always positive
	if(b<0) b=-b;
	if(a>b)
	{
		divisor=b;
		dividend=a;
	}
	else
	{
		divisor=a;
		dividend=b;
	}
	remainder=1; /* to start the loop */
	while(remainder)
	{
		remainder=dividend-(dividend/divisor)*divisor;
		dividend=divisor;
		divisor=remainder;
	}
	return dividend;
}
int lcm(int a, int b) // find the lcm of 2 numbers
{
	assert(a>0);
	assert(b>0);
	return (((qword_t)a)*b)/gcd(a,b);
}
int compute_alpha(clp_t a)
{
	int k;
	assert(!EMPTY(a));
	assert(a.u>0);
	if(CONSTANT(a)) return a.l;
	k=a.l+((-a.l)/a.d + 1)*a.d; // l+j*d<=0 ==> j<=(-l)/d; alpha=l+(j+1)*d;
	assert(k>0);
	assert(k<=a.u);
	return k;
}
int compute_beta(clp_t a)
{
	int k;
	assert(!EMPTY(a));
	assert(a.l<0);
	if(CONSTANT(a)) return a.l;
	k=a.u-(a.u/a.d + 1)*a.d; // u-j*d>=0 ==> j<=u/d; beta=u-(j+1)*d;
	assert(k<0);
	assert(k>=a.l);
	return k;
}
int num_trailing_zero(qword_t a) // returns the number of trailing zero bits of a
{
	int i;
	qword_t temp=1;
	if(!a) return sizeof(qword_t)*8;
	for(i=0;;i++) // for a!=0, this will terminate
	{
		if(a&temp) return i;
		temp*=2;
	}
}
int num_leading_zero(word_t a) // returns the number of the leading zero bits of a
{
	int i;
	word_t temp=1;
	if(!a) return sizeof(word_t)*8;
	temp=1<<((sizeof(word_t)*8)-1);
	for(i=0;;i++) // for a!=0, this will terminate
	{
		if(a&temp) return i;
		temp/=2;
	}
}
////////////////////////////////////////// INITIALIZATION /////////////////////////////////////////////
bool is_32bit(sqword_t n)
{
	if((n<=MAX_P)&&(n>=MAX_N)) return true;
	return false;
}
void fill_elt(clp_t *x, sqword_t l, sqword_t u, qword_t d)
{
	clp_t p,q;
	int i;
	sqword_t first,last;
	qword_t diff;
	if(!l && !u && !d)
	{
		CLEAR_CLP((*x));
		return;
	}
	if(!d)
	{
		(*x)=top; // maybe d overshot the range
		return;
	}
	if((l==u)||(is_32bit(l)&&is_32bit(u)))
	{
		assert(d);
		FILL_CLP_INTERNAL((*x),l,u,d);
		return;
	}
	i=num_trailing_zero(d);
	if(l>u)
	{
		(*x)=top;
		return;
	}
	// henceforth l<=u AND i>0
	if(is_32bit(l)) // u is not in 32 bits
	{
		// if l is +ve, u is also +ve as l<=u
		// if l is -ve, u can be +ve or -ve
		// if l is -ve, u is -ve, u>=l, l in 32 bits, u is also in 32 bits (contradiction)
		assert(u>=0);
		if(u>=0)
		{
			assert(d);
			first=u-((u-MAX_P)/d)*d+d;
			last=first-d;
			p.l=l;
			p.u=last;
			p.d=d;
			diff=u-first;
			if(is_32bit(diff)) FILL_CLP_INTERNAL(q,MAX_N+first-MAX_P-1,MAX_N+u-MAX_P-1,d)
			else
			{
				d=1<<i;
				first=first&0xffffffff; // get the lowest 32 bits
				first=first&(d-1);
				last=first+((MAX_32BIT_DATA-first)/d)*d;
				q.l=first;
				q.u=last;
				q.d=d;
			}
		}
		(*x)=clp_fn(CLP_UNION,p,q,false);
		return;
	}
	else if(is_32bit(u)) // l is not in 32 bits
	{
		// if u is -ve, l is also -ve as l<=u
		// if u is +ve, l can be +ve or -ve
		// if u is +ve, l is +ve, u>=l, u in 32 bits, l is also in 32 bits (contradiction)
		assert(l<=0);
		if(l<=0)
		{
			assert(d);
			first=u-((u-((sword_t)MAX_N))/d)*d;
			last=first-d;
			p.l=first;
			p.u=u;
			p.d=d;
			diff=last-l;
			if(is_32bit(diff)) FILL_CLP_INTERNAL(q,MAX_P-(((sword_t)MAX_N)-l-1),MAX_P-(((sword_t)MAX_N)-last-1),d)
			else
			{
				d=1<<i;
				first=last&0xffffffff; // get the lowest 32 bits
				first=first&(d-1);
				last=first+((MAX_32BIT_DATA-first)/d)*d;
				FILL_CLP_INTERNAL(q,first,last,d);
			}
		}
		(*x)=clp_fn(CLP_UNION,p,q,false);
		return;
	}
	else // both are beyond 32 bits
	{
		if(!i)
		{
			*x=top;
			return;
		}
		d=1<<i;
		first=l&0xffffffff;
		first=first&(d-1);
		last=first+((MAX_32BIT_DATA-first)/d)*d;
		FILL_CLP_INTERNAL((*x),first,last,d);
	}
}
void init_clp()
{
	FILL_CLP_INTERNAL(zero,0,0,1);
	FILL_CLP_INTERNAL(one,1,1,1);
	MAKE_TOP(top);
}
clp_t mk_clp(sword_t x)
{
	clp_t temp;
	FILL_CLP_INTERNAL(temp,x,x,1);
	return temp;
}
qword_t clp_size(clp_t a)
{
	sword_t first,last;
	qword_t cnt;
	if(EMPTY(a)) return 0;
	if(CONSTANT(a)) return 1;
	if(TOP(a)) return (qword_t)MAX_32BIT_DATA+1;
	if(a.u>=a.l) return (a.u-a.l)/a.d+1;
	cnt=((qword_t)(MAX_P-a.l))/a.d;
	last=cnt*a.d+a.l;
	first=last+a.d;
	return (last-a.l)/a.d+(a.u-first)/a.d+2;
}
clp_t tight_clp(clp_t a,clp_t b,bool *eq,bool *first) // favors the second
{
	if(EMPTY(b)&&(!EMPTY(a)))
	{
		*eq=false;
		*first=true;
		return a;
	}
	if(EMPTY(a)&&(!EMPTY(b)))
	{
		*eq=false;
		*first=false;
		return b;
	}
	if(NUM(a)<NUM(b))
	{
		*eq=false;
		*first=true;
		return a;
	}
	if(NUM(a)==NUM(b))
		*eq=true;
	else
		*eq=false;
	*first=false;
	return b;
}
////////////////////////////////////////// SET THEORETIC OPERATIONS /////////////////////////////////////////////
clp_t op_union(clp_t a, clp_t b)
{
	clp_t temp;
	word_t t1,t2,t3,t4;
	if(TOP(a)||TOP(b)) MAKE_TOP(temp)
	else if(EMPTY(b)) temp=a;
	else if(EMPTY(a)) temp=b;
	else if(CONSTANT(a)&&CONSTANT(b)&&(a.l==b.l)) temp=a;
	else if(CONSTANT(b))
	{
		if(CONSTANT(a)) // a.l!=b.l
		{
			if(b.l>a.l)
			{
				t1=b.l-a.l;
				t2=(MAX_P-b.l)+(a.l-MAX_N)+1;
				if(t1<t2) FILL_CLP_INTERNAL(temp,a.l,b.l,t1)
				else FILL_CLP_INTERNAL(temp,b.l,a.l,t2)
			}
			else //a.l>b.l
			{
				t1=a.l-b.l;
				t2=(MAX_P-a.l)+(b.l-MAX_N)+1;
				if(t1<t2) FILL_CLP_INTERNAL(temp,b.l,a.l,t1)
				else FILL_CLP_INTERNAL(temp,a.l,b.l,t2)
			}
		}
		else
		{
			if((b.l==a.u)||(b.l==a.l)) temp=a;
			else if(a.l<a.u)
			{
				if(b.l>a.u)
				{
					t1=(a.l-MAX_N)+(MAX_P-b.l)+1;
					t2=(b.l-a.u);
					t3=gcd(t1,a.d);
					t4=gcd(t2,a.d);
					if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,b.l,a.u,t3) //wrap
					else FILL_CLP_INTERNAL(temp,a.l,b.l,t4);
				}
				else if(b.l>a.l) FILL_CLP_INTERNAL(temp,a.l,a.u,gcd(b.l-a.l,a.d))
				else //(b.l<a.l)
				{
					t1=(b.l-MAX_N)+(MAX_P-a.u)+1;
					t2=(a.l-b.l);
					t3=gcd(t1,a.d);
					t4=gcd(t2,a.d);
					if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,a.l,b.l,t3) //wrap
					else FILL_CLP_INTERNAL(temp,b.l,a.u,t4)
				}
			}
			else //a.l>a.u
			{
				if(b.l>a.l)
					FILL_CLP_INTERNAL(temp,a.l,a.u,gcd(a.d,(b.l-a.l)))
				else if(b.l>a.u)
				{
					t1=a.l-b.l; // cannot be 0
					t2=b.l-a.u; // cannot be 0
					t3=gcd(t1,a.d);
					t4=gcd(t2,a.d);
					if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,b.l,a.u,t3)
					else FILL_CLP_INTERNAL(temp,a.l,b.l,t4)
				}
				else //(b.l<a.u)
					FILL_CLP_INTERNAL(temp,a.l,a.u,gcd(a.d,a.u-b.l))
			}
		}
	}
	else if(CONSTANT(a))
	{
		if(CONSTANT(b)) // a.l!=b.l
		{
			if(b.l>a.l)
			{
				t1=b.l-a.l;
				t2=(MAX_P-b.l)+(a.l-MAX_N)+1;
				if(t1<t2) FILL_CLP_INTERNAL(temp,a.l,b.l,t1)
				else FILL_CLP_INTERNAL(temp,b.l,a.l,t2)
			}
			else //a.l>b.l
			{
				t1=a.l-b.l;
				t2=(MAX_P-a.l)+(b.l-MAX_N)+1;
				if(t1<t2) FILL_CLP_INTERNAL(temp,b.l,a.l,t1)
				else FILL_CLP_INTERNAL(temp,a.l,b.l,t2)
			}
		}
		else
		{
			if((a.l==b.u)||(a.l==b.l)) temp=b;
			else if(b.l<b.u)
			{
				if(a.l>b.u)
				{
					t1=(b.l-MAX_N)+(MAX_P-a.l)+1;
					t2=(a.l-b.u);
					t3=gcd(t1,b.d);
					t4=gcd(t2,b.d);
					if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,a.l,b.u,t3) //wrap
					else FILL_CLP_INTERNAL(temp,b.l,a.l,t4)
				}
				else if(a.l>b.l) FILL_CLP_INTERNAL(temp,b.l,b.u,gcd(a.l-b.l,b.d))
				else //(a.l<b.l)
				{
					t1=(a.l-MAX_N)+(MAX_P-b.u)+1;
					t2=(b.l-a.l);
					t3=gcd(t1,b.d);
					t4=gcd(t2,b.d);
					if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,b.l,a.l,t3) //wrap
					else FILL_CLP_INTERNAL(temp,a.l,b.u,t4)
				}
			}
			else //b.l>b.u
			{
				if(a.l>b.l)
					FILL_CLP_INTERNAL(temp,b.l,b.u,gcd(b.d,(a.l-b.l)))
				else if(a.l>b.u)
				{
					t1=b.l-a.l; // cannot be 0
					t2=a.l-b.u; // cannot be 0
					t3=gcd(t1,b.d);
					t4=gcd(t2,b.d);
					if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,a.l,b.u,t3)
					else FILL_CLP_INTERNAL(temp,b.l,a.l,t4)
				}
				else //(a.l<b.u)
					FILL_CLP_INTERNAL(temp,b.l,b.u,gcd(b.d,b.u-a.l))
			}
		}
	}
	else
	{
		if((a.l==b.l)&&(a.u==b.u)) temp=a;
		else if((a.l<a.u)&&(b.l<b.u))
		{
			if(b.l>a.u)
			{
				t1=(a.l-MAX_N)+(MAX_P-b.u)+1;
				t2=(b.l-a.u);
				t3=gcd(gcd(t1,b.d),a.d);
				t4=gcd(gcd(t2,b.d),a.d);
				if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,b.l,a.u,t3) //wrap
				else FILL_CLP_INTERNAL(temp,a.l,b.u,t4)
			}
			else if(b.l>=a.l) FILL_CLP_INTERNAL(temp,a.l,max(b.u,a.u),gcd(gcd((a.l-b.u),b.d),a.d))
			else if(b.u>=a.l) FILL_CLP_INTERNAL(temp,b.l,max(b.u,a.u),gcd(gcd((b.l-a.u),b.d),a.d))
			else
			{
				t1=(b.l-MAX_N)+(MAX_P-a.u)+1;
				t2=(a.l-b.u);
				t3=gcd(gcd(t1,b.d),a.d);
				t4=gcd(gcd(t2,b.d),a.d);
				if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,a.l,b.u,t3) //wrap
				else FILL_CLP_INTERNAL(temp,b.l,a.u,t4)
			}
		}
		else if((a.l<a.u)&&(b.l>b.u))
		{
			if((a.l==b.u)&&(a.u==b.l)) FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(a.d,b.d))
			else if(a.l==b.u)
			{
				if(a.u>b.l) FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(a.d,b.d))
				else FILL_CLP_INTERNAL(temp,b.l,a.u,gcd(a.d,b.d)) // a.u<b.l
			}
			else if(a.u==b.l)
			{
				if(b.u>a.l) FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(a.d,b.d))
				else FILL_CLP_INTERNAL(temp,a.l,b.u,gcd(a.d,b.d)) // b.u<a.l
			}
			else
			{
				if(b.l<a.u)
				{
					if(b.l<=a.l) FILL_CLP_INTERNAL(temp,b.l,b.u,gcd(a.d,gcd(b.d,(a.u-b.l))))
					else // b.l lies in between a.l and a.u
					{
						if(b.u>=a.l) FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(a.d,gcd(b.d,(a.u-b.l))))
						else FILL_CLP(temp,a.l,b.u,gcd(a.d,gcd(b.d,(a.u-b.l))))
					}
				}
				else //b.l>a.u
				{
					if(b.u>=a.u) FILL_CLP_INTERNAL(temp,b.l,b.u,gcd(a.d,gcd(b.d,(b.u-a.l))))
					else if(b.u>=a.l) //b.u lies between a.l and a.u
						FILL_CLP_INTERNAL(temp,b.l,a.u,gcd(a.d,gcd(b.d,(a.u-b.u))))
					else // b.u < a.l
					{
						t1=(a.l-b.u);
						t2=(b.l-a.u);
						t3=gcd(a.d,gcd(b.d,(a.l-b.u)));
						t4=gcd(a.d,gcd(b.d,(b.l-a.u)));
						if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,b.l,a.u,t3)
						else FILL_CLP_INTERNAL(temp,a.l,b.u,t4)
					}
				}
			}
		}
		else if((b.l<b.u)&&(a.l>a.u))
		{
			if((b.l==a.u)&&(b.u==a.l)) FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(b.d,a.d))
			else if(b.l==a.u)
			{
				if(b.u>a.l) FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(b.d,a.d))
				else FILL_CLP_INTERNAL(temp,a.l,b.u,gcd(b.d,a.d)) // b.u<a.l
			}
			else if(b.u==a.l)
			{
				if(a.u>b.l) FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(b.d,a.d))
				else FILL_CLP_INTERNAL(temp,b.l,a.u,gcd(b.d,a.d)) // a.u<b.l
			}
			else
			{
				if(a.l<b.u)
				{
					if(a.l<=b.l) FILL_CLP_INTERNAL(temp,a.l,a.u,gcd(b.d,gcd(a.d,(b.u-a.l))))
					else // a.l lies in between b.l and b.u
					{
						if(a.u>=b.l) FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(b.d,gcd(a.d,(b.u-a.l))))
						else FILL_CLP(temp,b.l,a.u,gcd(b.d,gcd(a.d,(b.u-a.l))))
					}
				}
				else //a.l>b.u
				{
					if(a.u>=b.u) FILL_CLP_INTERNAL(temp,a.l,a.u,gcd(b.d,gcd(a.d,(a.u-b.l))))
					else if(a.u>=b.l) //a.u lies between b.l and b.u
						FILL_CLP_INTERNAL(temp,a.l,b.u,gcd(b.d,gcd(a.d,(b.u-a.u))))
					else // a.u < b.l
					{
						t1=(b.l-a.u);
						t2=(a.l-b.u);
						t3=gcd(b.d,gcd(a.d,(b.l-a.u)));
						t4=gcd(b.d,gcd(a.d,(a.l-b.u)));
						if((t1/t3)<(t2/t4)) FILL_CLP_INTERNAL(temp,a.l,b.u,t3)
						else FILL_CLP_INTERNAL(temp,b.l,a.u,t4)
					}
				}
			}
		}
		else //a.l>a.u AND b.l>b.u
		{
			if(max(a.u,b.u)>=min(a.l,b.l))
			{
				if(a.l==b.l) FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(a.d,b.d))
				else FILL_CLP_INTERNAL(temp,MAX_N,MAX_P,gcd(a.d,gcd(b.d,(a.l-b.l))))
			}
			else if(a.l!=b.l) FILL_CLP_INTERNAL(temp,min(a.l,b.l),max(a.u,b.u),gcd(gcd((a.l-b.l),a.d),b.d))
			else FILL_CLP_INTERNAL(temp,min(a.l,b.l),max(a.u,b.u),gcd(gcd((a.u-b.u),a.d),b.d))
		}
	}
	return temp;
}
clp_t op_intersect(clp_t a,clp_t b)
{
	clp_t temp;
	int delta,theta,phi;
	int j;
	if(TOP(a)) temp=b;
	else if(TOP(b)) temp=a;
	else if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)&&CONSTANT(b))
	{
		if(a.l==b.l) temp=a;
		else CLEAR_CLP(temp)
	}
	else if(CONSTANT(b))
	{
		if((b.l>=a.l)&&(b.l<=a.u)&&(((b.l-a.l)/a.d)*a.d==(b.l-a.l))) temp=b;
		else CLEAR_CLP(temp);
	}
	else
	{
		if(a.d>b.d) // swap the sets
		{
			temp=a;
			a=b;
			b=temp;
		}
		delta=lcm(a.d,b.d);
		for(j=0;;j++) // TODO: optimize
		{
			if(b.l>=a.l)
			{
				if((theta=(b.l+j*b.d))>min(a.u,b.u)) // trying to fix theta
				{
					CLEAR_CLP(temp); // intersection is empty
					return temp;
				}
				if((((b.l-a.l)+j*b.d)/a.d)*a.d==((b.l-a.l)+j*b.d)) break; // integer found
			}
			else
			{
				if((theta=(a.l+j*a.d))>min(a.u,b.u)) // trying to fix theta
				{
					CLEAR_CLP(temp); // intersection is empty
					return temp;
				}
				if((((a.l-b.l)+j*a.d)/b.d)*b.d==((a.l-b.l)+j*a.d)) break; // integer found
			}
		}
		phi=theta+delta*((min(a.u,b.u)-theta)/delta);
		FILL_CLP(temp,theta,phi,delta);
	}
	return temp;
}
clp_t op_difference(clp_t a,clp_t b)
{
	clp_t c,temp;
	sword_t l,u;
	word_t d;
	CLEAR_CLP(temp);
	if(EMPTY(a)) return temp;
	if(EMPTY(b)) return a;
	c=clp_fn(CLP_INTERSECT,a,b,false);
	if(EMPTY(c)) return a;
	if(SAME_CLP(c,a)) return temp;
	if(NUM(a)-NUM(c)==1)
	{
		if(a.l==c.l)
		{
			if(a.u==c.u)
			{
				assert(NUM(a)==3);
				return mk_clp(a.l+a.d);
			}
			assert(a.u!=c.u);
			return mk_clp(a.u);
		}
		assert(a.u==c.u);
		return mk_clp(a.l);
	}
	if(CONSTANT(c)&&(NUM(a)==3)) // NUM(a) cannot be <=2 here as it would have been caught in the above cases
	{
		if(a.l==c.l) temp=clp_fn(CLP_UNION,mk_clp(a.l+a.d),mk_clp(a.u),false);
		else if(a.u==c.u) temp=clp_fn(CLP_UNION,mk_clp(a.l),mk_clp(a.l+a.d),false);
		else
		{
			assert((a.l+a.d)==(c.l));
			temp=clp_fn(CLP_UNION,mk_clp(a.l),mk_clp(a.u),false);
		}
		return temp;
	}
	d=a.d;
	if(c.l!=a.l) l=a.l;
	else
	{
		if(c.d!=a.d) l=a.l+a.d;
		else l=c.u+a.d;
	}
	if(c.u!=a.u) u=a.u;
	else
	{
		if(c.d!=a.d) u=a.u-a.d;
		else u=c.l-a.d;
	}
	FILL_CLP(temp,l,u,d);
	return temp;
}
////////////////////////////////////////// ARITHMETIC OPERATIONS /////////////////////////////////////////////
clp_t op_add(clp_t a,clp_t b)
{
	clp_t temp;
	if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)&&CONSTANT(b)) FILL_CLP(temp,((sqword_t)a.l+b.l),((sqword_t)a.l+b.l),1)
	else if(CONSTANT(a)) FILL_CLP(temp,((sqword_t)a.l+b.l),((sqword_t)a.l+b.u),b.d)
	else if(CONSTANT(b)) FILL_CLP(temp,((sqword_t)a.l+b.l),((sqword_t)a.u+b.l),a.d)
	else FILL_CLP(temp,((sqword_t)a.l+b.l),((sqword_t)a.u+b.u),gcd(a.d,b.d))
		return temp;
}
clp_t op_sub(clp_t a,clp_t b)
{
	clp_t temp;
	if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)&&CONSTANT(b)) FILL_CLP(temp,((sqword_t)a.l-b.l),((sqword_t)a.l-b.l),1)
	else if(CONSTANT(a)) FILL_CLP(temp,((sqword_t)a.l-b.u),((sqword_t)a.l-b.l),b.d)
	else if(CONSTANT(b)) FILL_CLP(temp,((sqword_t)a.l-b.l),((sqword_t)a.u-b.l),a.d)
	else FILL_CLP(temp,((sqword_t)a.l-b.u),((sqword_t)a.u-b.l),gcd(a.d,b.d))
		return temp;
}
clp_t op_mult(clp_t a,clp_t b)
{
	clp_t temp;
	if((EMPTY(a)&&(ZERO(b)))||(EMPTY(b)&&(ZERO(a)))) temp=zero;
	else if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)&&CONSTANT(b)) FILL_CLP(temp,((sqword_t)a.l*b.l),((sqword_t)a.l*b.l),1)
	else if(CONSTANT(a))
	{
		if(!a.l) temp=zero;
		else FILL_CLP(temp,min((sqword_t)a.l*b.l,(sqword_t)a.l*b.u),max((sqword_t)a.l*b.l,(sqword_t)a.l*b.u),abs(b.d*a.l))
	}
	else if(CONSTANT(b))
	{
		if(!b.l) temp=zero;
		else FILL_CLP(temp,min((sqword_t)a.l*b.l,(sqword_t)a.u*b.l),max((sqword_t)a.l*b.l,(sqword_t)a.u*b.l),abs(a.d*b.l))
	}
	else if(repeat_flag&&(NUM(a)==2)) temp=op_union(mk_clp(a.l*a.l),mk_clp(a.u*a.u));
	else FILL_CLP(temp,min((sqword_t)a.l*b.l,min((sqword_t)a.l*b.u,min((sqword_t)a.u*b.u,(sqword_t)a.u*b.l))),\
			max((sqword_t)a.l*b.l,max((sqword_t)a.l*b.u,max((sqword_t)a.u*b.u,(sqword_t)a.u*b.l))),gcd(gcd(abs(a.l)*b.d,a.d*abs(b.l)),a.d*b.d))
		return temp;
}
clp_t op_div(clp_t a,clp_t b)
{
	clp_t temp;
	int sl1,sl2,su1,su2;
	int k;
	FILL_CLP(temp,0,0,1);
	assert(EMPTY(op_intersect(b,temp))); // b cannot contain zero
	if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)&&CONSTANT(b)) FILL_CLP(temp,(a.l/b.l),(a.l/b.l),1)
	else if(CONSTANT(b)) FILL_CLP(temp,min((sqword_t)a.l/b.l,(sqword_t)a.u/b.l),max((sqword_t)a.l/b.l,(sqword_t)a.u/b.l),(sword_t)a.d/b.l)
	else if(CONSTANT(a)) FILL_CLP(temp,min((sqword_t)a.l/b.l,(sqword_t)a.l/b.u),max((sqword_t)a.l/b.l,(sqword_t)a.l/b.u),1)
	else
	{ // first establish the signs
		if(a.l>=0) sl1=su1=1;
		else if(a.u<=0) sl1=su1=0;
		else { sl1=0;su1=1; }
		if(b.l>=0) sl2=su2=1;
		else if(b.u<=0) sl2=su2=0;
		else { sl2=0;su2=1; }
		k=((sl1&1)<<3)|((su1&1)<<2)|((sl2&1)<<1)|(su2&1);
		switch(k)
		{
			case 0: FILL_CLP(temp,(a.u/b.l),(a.l/b.u),1);break;
			case 1: FILL_CLP(temp,(a.l/compute_alpha(b)),(a.l/compute_beta(b)),1);break;
			case 3: FILL_CLP(temp,(a.l/b.l),(a.u/b.u),1);break;
			case 4: FILL_CLP(temp,(a.u/b.u),(a.l/b.u),1);break;
			case 5: FILL_CLP(temp,min((a.u/compute_beta(b)),(a.l/compute_alpha(b))),max((a.l/compute_beta(b)),(a.u/compute_alpha(b))),1);break;
			case 7: FILL_CLP(temp,(a.l/b.l),(a.u/b.l),1);break;
			case 12:FILL_CLP(temp,(a.u/b.u),(a.l/b.l),1);break;
			case 13:FILL_CLP(temp,(a.u/compute_beta(b)),(a.u/compute_alpha(b)),1);break;
			case 15:FILL_CLP(temp,(a.l/b.u),(a.u/b.l),1);break;
			default: assert(0);
		}
	}
	return temp;
}

clp_t op_add_expand(clp_t a, clp_t b)
{
	clp_t temp;
	if(EMPTY(a)||EMPTY(b)) 
	{
		CLEAR_CLP(temp);
		return temp;
	}
	if(TOP(a)||TOP(b)) 
	{
		MAKE_TOP(temp);
		return temp;
	}
	if(!CONSTANT(a)) a.d=1;
	if(!CONSTANT(b)) b.d=1;
	clp_t x=op_add(a,b);
	temp=op_union(x,a);
	temp.d=1;
	return temp;
}
////////////////////////////////////////// SHIFT OPERATIONS /////////////////////////////////////////////
clp_t op_lshift(clp_t a,clp_t b)
{
	clp_t temp;
	if(EMPTY(b)||EMPTY(a))
	{
		CLEAR_CLP(temp)
			return temp;
	}
	if(TOP(a)||TOP(b)) MAKE_TOP(temp)
	else if(CONSTANT(a)&&CONSTANT(b)) FILL_CLP(temp,((sqword_t)a.l<<b.l),((sqword_t)a.l<<b.l),1)
	else if(CONSTANT(a)) FILL_CLP(temp,((sqword_t)b.l<<a.l),((sqword_t)b.u<<a.l),(b.d<<a.l))
	else if(CONSTANT(b)) FILL_CLP(temp,((sqword_t)a.l<<b.l),((sqword_t)a.u<<b.l),(a.d<<b.l))
	else if(repeat_flag&&(NUM(a)==2)) temp=op_union(mk_clp(a.l<<a.l),mk_clp(a.u<<a.u));
	else FILL_CLP(temp,((sqword_t)a.l<<b.l),((sqword_t)a.u<<b.u),gcd(abs(a.l),a.d)<<b.l)
		assert(b.l>=0);
	return temp;
}
clp_t op_rshift(clp_t a,clp_t b)
{
	clp_t temp;
	word_t d;
	if(EMPTY(b)||EMPTY(a))
	{
		CLEAR_CLP(temp)
			return temp;
	}
	if(TOP(a)||TOP(b)) MAKE_TOP(temp)
	else if(CONSTANT(a)&&CONSTANT(b)) FILL_CLP(temp,(a.l>>b.l),(a.l>>b.l),1)
	else if(CONSTANT(b))
	{
		d=a.d>>b.l;
		if(!d) d=1;
		FILL_CLP(temp,(a.l>>b.l),(a.u>>b.l),d)
	}
	else
	{
		if(a.l*a.u<0) FILL_CLP(temp,(a.l>>b.l),(a.u>>b.l),1)
		else if((a.l>=0)&&(a.u>=0)) FILL_CLP(temp,(a.l>>b.u),(a.u>>b.l),1)
		else FILL_CLP(temp,(a.l>>b.l),(a.u>>b.u),1)
	}
	assert(b.l>=0);
	return temp;
}
////////////////////////////////////////// LOGICAL OPERATIONS /////////////////////////////////////////////
clp_t op_andl(clp_t a,clp_t b)
{
	clp_t temp;
	int v; // TODO: change to bool
	if(TOP(a)||TOP(b)) MAKE_TOP(temp)
	else if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)&&CONSTANT(b)) FILL_CLP(temp,(a.l&&b.l),(a.l&&b.l),1)
	else if(CONSTANT(a)&&(!a.l)) temp=zero;
	else if(CONSTANT(b)&&(!b.l)) temp=zero;
	else // at least 1 combination of the result will be 1
	{
		// if 0 is present in any series, v will be 1
		v=EMPTY(op_intersect(a,zero))||EMPTY(op_intersect(b,zero));
		if(v) FILL_CLP(temp,0,1,1)
		else temp=one;
	}
	return temp;
}
clp_t op_notl(clp_t a)
{
	clp_t temp;
	if(TOP(a)) MAKE_TOP(temp)
	else if(EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)) FILL_CLP(temp,!a.l,!a.l,1)
	else if(EMPTY(op_intersect(a,zero))) temp=zero;
	else FILL_CLP(temp,0,1,1); // since a has at least 2 terms and one of them is 0
	return temp;
}
clp_t op_orl(clp_t a,clp_t b)
{
	clp_t temp;
	if(TOP(a)||TOP(b)) MAKE_TOP(temp)
	else if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else temp=op_notl(op_andl(op_notl(a),op_notl(b)));
	return temp;
}
////////////////////////////////////////// BIT OPERATIONS /////////////////////////////////////////////
clp_t op_and(clp_t a, clp_t b)
{
	clp_t temp;
	int i,j,t1,t2,flag1,flag2;
	word_t alpha,alpha1,alpha2,theta1,theta2,slb,sub,k,k1;
	word_t beta, gamma;
	word_t delta,mask,a1,a2;
	if(TOP(a)||TOP(b)) MAKE_TOP(temp)
	else if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(b)&&CONSTANT(a)) FILL_CLP(temp,(a.l&b.l),(a.l&b.l),1)
	else if(ZERO(a)||ZERO(b)) temp=zero;
	else if(!DIFFER_CLP(a,b)) temp=a;
	else if(CONSTANT(b)||CONSTANT(a))
	{
		if(CONSTANT(a)) // swap to make b constant
		{
			temp=a;
			a=b;
			b=temp;
		}
		if(((unsigned)a.l)>((unsigned)a.u))
		{
			j=a.l;
			a.l=a.u;
			a.u=j;
		}
		alpha1=num_trailing_zero(a.d);
		alpha2=num_trailing_zero(b.l);
		j=max(alpha1,alpha2);
		delta=1;
		for(i=0;i<j;i++) delta*=2;
		t1=num_leading_zero(a.l);
		t2=num_leading_zero(a.u);
		theta1=(sizeof(word_t)*8)-1-t1;
		theta2=(sizeof(word_t)*8)-1-t2;
		mask=-1; // all 1’s
		if(t2<(sizeof(word_t)*8))
		{
			mask<<=t2;
			mask>>=t2; // the 2 steps clear the leading bits
		}
		else mask=0;
		if(alpha2<(sizeof(word_t)*8))
		{
			mask>>=alpha2;
			mask<<=alpha2; // the 2 steps clear the trailing bots
		}
		else mask=0;
		k=b.l;
		if((b.l&mask)==mask)
		{
			beta=a.l&b.l;
			gamma=a.u&b.l;
		}
		else
		{
			if(((unsigned)a.u)>k) sub=k;
			else sub=a.u;
			a2=a.l;
			if(alpha1>0)
			{
				a2<<=(sizeof(word_t)*8)-alpha1;
				a2>>=(sizeof(word_t)*8)-alpha1; // to clear the upper bits
			}
			else a2=0;
			if((k>>theta2)&1)
			{
				for(i=theta2;i>=0;i--)
					if(!((k>>i)&1)) break;
				i++;
				if(i<=theta1)
				{
					a1=a.l;
					if(t1<(sizeof(word_t)*8))
					{
						a1<<=t1;
						a1>>=t1; // the 2 steps clear the leading bits
					}
					else a1=0;
					if(i<(sizeof(word_t)*8))
					{
						a1>>=i;
						a1<<=i; // the 2 steps clear the trailing bits
					}
					else a1=0;
					if(alpha1<i)
						a1|=a2; // to get the lower bits
					slb=a1&k;
				}
				else slb=a2&k;
			}
			else slb=a2&k;
			beta=((unsigned)a.l&k)+delta*((slb-((unsigned)a.l&k))/delta);
			gamma=((unsigned)a.l&k)+delta*((sub-((unsigned)a.l&k))/delta);
		}
		if((signed)beta>(signed)gamma)
		{
			j=beta;
			beta=gamma;
			gamma=j;
		}
		FILL_CLP(temp,beta,gamma,delta);
	}
	else
	{
		if(((unsigned)a.l)>((unsigned)a.u))
		{
			j=a.l;
			a.l=a.u;
			a.u=j;
		}
		if(((unsigned)b.l)>((unsigned)b.u))
		{
			j=b.l;
			b.l=b.u;
			b.u=j;
		}
		alpha1=num_trailing_zero(a.d);
		alpha2=num_trailing_zero(b.d);
		alpha=min(alpha1,alpha2);
		delta=1;
		for(i=0;i<alpha;i++) delta*=2;
		if(((unsigned)a.u)>((unsigned)b.u)) sub=b.u;
		else sub=a.u;
		k=b.l;
		if(alpha>0)
		{
			k<<=(sizeof(word_t)*8)-alpha;
			k>>=(sizeof(word_t)*8)-alpha; // to clear the upper bits
		}
		else k=0;
		t1=num_leading_zero(a.l);
		t2=num_leading_zero(a.u);
		theta1=(sizeof(word_t)*8)-1-t1;
		theta2=(sizeof(word_t)*8)-1-t2;
		flag1=0;
		flag2=0;
		if((b.l>>theta2)&1)
		{
			for(i=theta2;i>=0;i--)
				if(!((b.l>>i)&1)) break;
			i++;
			if(i<=theta1) flag1=1;
		}
		if(flag1&&(b.u>>theta2)&1)
		{
			for(j=theta2;j>=i;j--)
				if(!((b.u>>j)&1)) break;
			j++;
			if(j<=theta1) flag2=1;
		}
		a2=a.l;
		if(alpha>0)
		{
			a2<<=(sizeof(word_t)*8)-alpha;
			a2>>=(sizeof(word_t)*8)-alpha; // to clear the upper bits
		}
		else a2=0;
		if(flag2)
		{
			k1=b.l;
			if(t1<(sizeof(word_t)*8))
			{
				k1<<=t1;
				k1>>=t1; // the 2 steps clear the leading bits
			}
			else k1=0;
			if(j<(sizeof(word_t)*8))
			{
				k1>>=j;
				k1<<=j; // the 2 steps clear the trailing bits
			}
			else k1=0;
			if(alpha<j)
				k1|=k; // to get the lower bits
			k=k1;
			a1=a.l;
			if(t1<(sizeof(word_t)*8))
			{
				a1<<=t1;
				a1>>=t1; // the 2 steps clear the leading bits
			}
			else a1=0;
			if(j<(sizeof(word_t)*8))
			{
				a1>>=j;
				a1<<=j; // the 2 steps clear the trailing bits
			}
			else a1=0;
			if(alpha<j)
				a1|=a2; // to get the lower bits
			slb=a1&k;
		}
		else slb=a2&k;
		beta=((unsigned)a.l&(unsigned)b.l)+delta*((slb-((unsigned)a.l&(unsigned)b.l))/delta);
		gamma=((unsigned)a.l&(unsigned)b.l)+delta*((sub-((unsigned)a.l&(unsigned)b.l))/delta);
		if((signed)beta>(signed)gamma)
		{
			j=beta;
			beta=gamma;
			gamma=j;
		}
		FILL_CLP(temp,beta,gamma,delta);
	}
	return temp;
}
clp_t op_bitc(clp_t a)
{
	clp_t temp;
	if(TOP(a)) MAKE_TOP(temp)
	else if(EMPTY(a)) CLEAR_CLP(temp)
	else FILL_CLP(temp,~a.u,~a.l,a.d);
	return temp;
}
clp_t op_or(clp_t a, clp_t b)
{
	clp_t temp;
	if(TOP(a)||TOP(b)) MAKE_TOP(temp)
	else if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(b)&&CONSTANT(a)) FILL_CLP(temp,(a.l|b.l),(a.l|b.l),1)
	else if(SAME_CLP(b,zero)) temp=a;
	else if(SAME_CLP(a,zero)) temp=b;
	else if(!DIFFER_CLP(a,b)) temp=a;
	else temp=op_bitc(op_and(op_bitc(a),op_bitc(b)));
	return temp;
}
clp_t op_xor(clp_t a, clp_t b)
{
	clp_t temp;
	if(TOP(a)||TOP(b)) MAKE_TOP(temp)
	else if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(b)&&CONSTANT(a)) FILL_CLP(temp,(a.l^b.l),(a.l^b.l),1)
	else if(!DIFFER_CLP(a,b)) FILL_CLP(temp,0,0,1)
	else temp=op_or(op_and(a,op_bitc(b)),op_and(op_bitc(a),b));
	return temp;
}
////////////////////////////////////////// COMPARISON OPERATIONS /////////////////////////////////////////////
clp_t op_eq(clp_t a,clp_t b)
{
	clp_t temp;
	if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)&&CONSTANT(b))
	{
		if(a.l==b.l) temp=one;
		else temp=zero;
	}
	else if(EMPTY(op_intersect(a,b))) temp=zero;
	else FILL_CLP(temp,0,1,1)
		return temp;
}
clp_t op_neq(clp_t a,clp_t b)
{
	return op_notl(op_eq(a,b));
}
clp_t op_lesser(clp_t a,clp_t b)
{
	clp_t temp;
	if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)&&CONSTANT(b))
	{
		if(a.l<b.l) temp=one;
		else temp=zero;
	}
	else if(a.u<b.l) temp=one; // condition is true
	else if(b.u<a.l) temp=zero; // condition is false
	else FILL_CLP(temp,0,1,1)
		return temp;
}
clp_t op_leq(clp_t a,clp_t b)
{
	return op_orl(op_lesser(a,b),op_eq(a,b));
}
clp_t op_greater(clp_t a,clp_t b)
{
	clp_t temp;
	if(EMPTY(b)||EMPTY(a)) CLEAR_CLP(temp)
	else if(CONSTANT(a)&&CONSTANT(b))
	{
		if(a.l>b.l) temp=one;
		else temp=zero;
	}
	else if(a.l>b.u) temp=one; // condition is true
	else if(b.l>a.u) temp=zero; // condition is false
	else FILL_CLP(temp,0,1,1)
		return temp;
}
clp_t op_geq(clp_t a,clp_t b)
{
	return op_orl(op_greater(a,b),op_eq(a,b));
}
////////////////////////////////////////// MASTER FUNCTION /////////////////////////////////////////////
clp_t clp_fn(clp_op_t clp_op,clp_t a,clp_t b,bool repeat)
{
	clp_t p,q,r,s,result;
	bool flag1,flag2;
	sword_t first, last;
	word_t cnt;
	clp_fn_ptr fn_ptr;
	CLEAR_CLP(p);
	CLEAR_CLP(q);
	CLEAR_CLP(r);
	CLEAR_CLP(s);
	if(EMPTY(a)&&EMPTY(b)) return p;
	if(repeat) assert(!DIFFER_CLP(a,b));
	switch(clp_op)
	{
		case CLP_UNION: if(repeat) return a; return op_union(a,b);
		case CLP_INTERSECT: if(repeat) return a; fn_ptr=op_intersect;break;
		case CLP_DIFFERENCE: if(repeat) return zero; return op_difference(a,b);
		case CLP_ADD: if(repeat)
			      {
				      a=mk_clp(2); // a+a=2a
				      fn_ptr=op_mult;
			      }
			      else fn_ptr=op_add;
			      break;
		case CLP_SUB: if(repeat) return zero; fn_ptr=op_sub;break;
		case CLP_MULT: fn_ptr=op_mult;break;
		case CLP_DIV: if(repeat) return one; fn_ptr=op_div;break;
		case CLP_LSHIFT: fn_ptr=op_lshift;break;
		case CLP_RSHIFT: fn_ptr=op_rshift;break;
		case CLP_BITC: return op_bitc(a);
		case CLP_AND: if(repeat) return a; fn_ptr=op_and;break;
		case CLP_OR: if(repeat) return a; fn_ptr=op_or;break;
		case CLP_XOR: if(repeat) return zero; fn_ptr=op_xor;break; //a^a=0
		case CLP_EQ: if(repeat) return one; fn_ptr=op_eq;break;
		case CLP_NEQ: if(repeat) return zero; fn_ptr=op_neq;break;
		case CLP_LEQ: if(repeat) return one; fn_ptr=op_leq;break;
		case CLP_GEQ: if(repeat) return one; fn_ptr=op_geq;break;
		case CLP_LESSER: if(repeat) return zero; fn_ptr=op_lesser;break;
		case CLP_GREATER: if(repeat) return zero; fn_ptr=op_greater;break;
		case CLP_ANDL: fn_ptr=op_andl;break;
		case CLP_ORL: fn_ptr=op_orl;break;
			      //case CLP_NOTL: fn_ptr=op_notl;break;
		case CLP_ADD_EXPAND: fn_ptr=op_add_expand;break;
		default: assert(0);
	};
	if(!EMPTY(a))
	{
		flag1=false;
		if(a.l<=a.u) p=a;
		else
		{
			flag1=true;
			cnt=(MAX_P-a.l)/a.d;
			last=a.l+cnt*a.d;
			first=last+a.d;
			FILL_CLP(p,a.l,last,a.d);
			FILL_CLP(q,first,a.u,a.d);
		}
	}
	if(!EMPTY(b))
	{
		flag2=false;
		if(b.l<=b.u) r=b;
		else
		{
			flag2=true;
			cnt=(MAX_P-b.l)/b.d;
			last=b.l+cnt*b.d;
			first=last+b.d;
			FILL_CLP_INTERNAL(r,b.l,last,b.d);
			FILL_CLP_INTERNAL(s,first,b.u,b.d);
		}
	}
	if(repeat)
	{
		repeat_flag=true;
		assert(flag1==flag2);
		if(!flag1) result=fn_ptr(p,p);
		else result=op_union(fn_ptr(p,p),fn_ptr(q,q));
		repeat_flag=false;
		return result;
	}
	if((!flag1)&&(!flag2)) return fn_ptr(p,r);
	if(flag1&&(!flag2)) return op_union(fn_ptr(q,r),fn_ptr(p,r));
	if((!flag1)&&flag2) return op_union(fn_ptr(p,s),fn_ptr(p,r));
	return op_union(fn_ptr(p,s),op_union(fn_ptr(q,r),op_union(fn_ptr(p,r),fn_ptr(q,s))));
}
clp_t clp_fn_const(clp_op_t clp_op, clp_t a, sword_t x)
{
	clp_t b;
	FILL_CLP_INTERNAL(b,x,x,1);
	return clp_fn(clp_op,a,b,false);
}
clp_t clp_const_fn(clp_op_t clp_op, sword_t x, clp_t a)
{
	clp_t b;
	FILL_CLP_INTERNAL(b,x,x,1);
	return clp_fn(clp_op,b,a,false);
}

