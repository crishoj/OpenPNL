#ifndef _INLINES_H_
#define _INLINES_H_

#include <math.h>
#include <ctype.h>
#include <inttypes.h>
#include "datadefs.h"

#ifdef WIN32
inline int MyRound( float f )
{
	int i;
	__asm
	{
		fld f
		fistp i
	}
	return i;
}

#ifdef INLINE_ASM_OPTIMIZE
	#define cvRound( val ) MyRound((float)val)
#endif
#endif

CART_INLINE int max_array(int* ar, int n)
{
	int imax = 0;
	int amax = *ar;
	for (int i = 0 ; i < n ; i++, ar++)
	{
		int a = *ar;
		if (a > amax)
		{
			amax = a;
			imax = i;
		}
	}
	return imax;
}

CART_INLINE int max_array(float* ar, int n)
{
	int imax = 0;
	float amax = *ar;
	for (int i = 0 ; i < n ; i++, ar++)
	{
		float a = *ar;
		if (a > amax)
		{
			amax = a;
			imax = i;
		}
	}
	return imax;
}

static void swap(void* a, void* b, unsigned int width)
{
	char* _a = (char*)a;
	char* _b = (char*)b;
	char tmp;
	while ( width-- ) 
	{
        tmp = *_a;
        *_a++ = *_b;
        *_b++ = tmp;
    }
}

static int compare_fl( const void* pf1,const void* pf2 )
{
	assert(pf1 && pf2);
	return (*(float*)pf1 < *(float*)pf2) ? -1 : 
		   (*(float*)pf1 > *(float*)pf2) ? 1 : 0;
}

static int compare_nodes( const void* node1,const void* node2 )
{
	assert(node1 && node2);
	int id1 = (*(CxCARTNode**)node1)->id;
	int id2 = (*(CxCARTNode**)node2)->id;
	return (id1 < id2) ? -1 : 
		   (id1 > id2) ? 1 : 0;
}

/*
static int compare_int( const void* pf1,const void* pf2 )
{
	assert(pf1 && pf2);
	return (*(int*)pf1 < *(int*)pf2) ? -1 : 
		   (*(int*)pf1 > *(int*)pf2) ? 1 : 0;
}*/
typedef struct _iflPair
{
	int i;
	float f;
} iflPair;

static int compare_pair( const void* pp1,const void* pp2 )
{
	assert(pp1 && pp2);
	return (((iflPair*)pp1)->f < ((iflPair*)pp2)->f ) ? -1 : 
		   (((iflPair*)pp1)->f > ((iflPair*)pp2)->f) ? 1 : 0;
}

static int compare_splits( const void* p1,const void* p2 )
{
	assert(p1 && p2);
	CxCARTSplit* split1 = *(CxCARTSplit**)p1;
	CxCARTSplit* split2 = *(CxCARTSplit**)p2;
	float w1 = split1->weight;
	float w2 = split2->weight;
	return (w2 < w1) ? -1 : (w2 > w1) ? 1 : 0;
}

#if 0
CART_INLINE double icxSafeDivide(double d , int i )
{ 
	double res = d/i;
	( (*(__int64*)&res) &= (__int64)(-(i!=0)) );
	return res;
}

CART_INLINE double icxSafeDivide(double d , double denom )
{ 
	double res = d/denom;
	__int64 denom_i = (*(__int64*)&denom);
	denom_i &= 0x7fffffffffffffff;
	( (*(__int64*)&res) &= (__int64)(-(denom_i!=0)));
	return res;
}
#endif

#pragma warning(disable : 4100)
CART_INLINE int icxVarToInt(CxClassifierVar var , BOOL floating = TRUE)
{
#if FLOAT_ONLY
	return cvRound(var.fl);
#else
	return  (floating ? cvRound(var.fl) : var.i);
#endif
}

CART_INLINE double icxVarToDouble(CxClassifierVar var , BOOL floating = TRUE)
{
#if FLOAT_ONLY
	return var.fl ;
#else
	return (floating ? var.fl : var.i);
#endif
}

CART_INLINE float icxVarToFloat(CxClassifierVar var , BOOL floating = TRUE)
{
#if FLOAT_ONLY
	return var.fl ;
#else
	return (floating ? var.fl : var.i);
#endif
}

CART_INLINE int icxSign(float f)
{
	return 1 - (int)(((*(int*)&f) & 0x80000000) >> 30);
}

CART_INLINE float icxFAbs(float f)
{
	int n = (*(int*)&f) & 0x7FFFFFFF ;
	assert(*(float*)&n >= 0.0);
	return *(float*)&n;
}

CART_INLINE float icxSetSign(float f, float sign)
{
	int n = ( ( (*(int*)&sign) & 0x80000000) ^ (*(int*)&f) );
	return *(float*)&n;
}

CART_INLINE void icxAssertAligned( void* ptr , const int align = CART_MEM_ALIGN)
{ 
	assert(ptr);
	assert ( ( ((intptr_t)ptr) & (align-1) ) == 0 );
}

CART_INLINE void* icxAlignedMalloc(int size , const int align = CART_MEM_ALIGN)
{
#if CART_ALIGN_MEMORY
	void* ptr = malloc( (size & (-align)) + 4 + (align << 1));
	int p = (int)ptr;
	void* ptr_ret = (void*) (( p + 4 + align) & (-align));
	*(((int*)ptr_ret) - 1) = p;
	return ptr_ret;
#else
	return malloc(size);
#endif
}

CART_INLINE void* icxAlignedCalloc(int num , int elem_size , const int align = CART_MEM_ALIGN)
{
#if CART_ALIGN_MEMORY
	int size = num * elem_size;
	void* ptr = malloc( size + 4 + align );
	memset(ptr , 0 , _msize(ptr));
	int p = (int)ptr;
	void* ptr_ret = (void*) (( p + 4 + align) & (-align));
	*(((int*)ptr_ret) - 1) = p;
	icxAssertAligned(ptr_ret);
	return ptr_ret;
#else
	return calloc(num , elem_size);
#endif
}

CART_INLINE void icxFreeAligned(void* ptr)
{
#if CART_ALIGN_MEMORY
	int* p = ((int*)ptr) - 1;
	free((void*)*p);
#else
	free(ptr);
#endif
}

CART_INLINE char* icxAlignUp(char* ptr , int align)
{
#if CART_ALIGN_MEMORY
	return ((align > 0) ? (char*)( (((long)ptr-1) & (-align)) + align) : ptr);
#else
	return ptr;
#endif
}

/*#pragma warning(disable : 4035)
static BOOL icxIsSSEEnabled()
{
	__asm
	{
		mov eax,1 
		cpuid
		xor eax , eax
		test edx , 002000000h
		setnz al
	}
}
#pragma warning(default : 4035)*/

#ifdef WIN32
#ifndef GET_TICKS
#define GET_TICKS
__declspec(naked) static __int64 GetTicks()
{
	_asm
	{
		rdtsc
		ret
	}
}
#endif
#else
#include <time.h>
static int GetTicks()
{
	return int(time(NULL));
}
#endif

CART_INLINE int icxGetClassifierType(CxClassifier* cfer)
{ return cfer->flags & CX_CLASSIFIER_KIND_MASK; }

CART_INLINE BOOL icxIsClassifierVar32f(int type )
{ return (((type) & CX_CLASSIFIER_VAR_TYPE_FLAG_32F) != 0); }

CART_INLINE BOOL icxIsClassifierVarNumeric(int type ) 
{ return (((type) & CX_CLASSIFIER_VAR_TYPE_FLAG_NUMERIC) != 0); }

CART_INLINE BOOL icxIsClassifierVarTranscendent(int type ) 
{ return (((type) & CX_CLASSIFIER_VAR_TYPE_FLAG_TRANSCENDENT) != 0) ; }

CART_INLINE BOOL icxIsClassifierVarCategoric(int type ) 
{ return (((type) & CX_CLASSIFIER_VAR_TYPE_FLAG_NUMERIC) == 0); }

CART_INLINE BOOL  icxIsClassifierVar32s(int type )
{ return (((type) & CX_CLASSIFIER_VAR_TYPE_FLAG_32F) == 0); }

CART_INLINE int icxGetVarNumStates(int type ) 
{ return (type & CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK); }

CART_INLINE BOOL icxIsChunkLoaded(CxClassifierSampleChunk* chunk )
{ return (intptr_t)(chunk) && (chunk)->is_loaded; }

CART_INLINE float icxGetSampleWeight(CxClassifierSampleChunk* chunk, int sample_idx )
{ return *(float *)((char *)(chunk)->weights_data + (sample_idx) * (chunk)->weights_step); }

//////// Returns true if node is pruned at step pruning_step ///////////
CART_INLINE BOOL icxIsNodePruned(CxCARTNode* node,int pruning_step)
{ return ( (node->pruning_step > 0) && (node->pruning_step <= pruning_step)); }

//////// Fast float comparison : -1 : f1 < f2 ; 0 - f1 = f2 ; 1 - f1 > f2 /////////
CART_INLINE int icxFlGe(float f1 , float f2)
{

//	int ret = ( ((*(int*)&diff) & 0x80000000) >> 31)^1;
//	assert( (f1 >= f2) == ret );
	float diff = f1-f2;
	return ( ((*(int*)&diff) & 0x80000000) >> 31)^1;
}

//////// Simple fast equality check , 0 - not equal , 1 - equal///
CART_INLINE int icxCompareVarsSimple( CxClassifierVar var1, CxClassifierVar var2)
{
	return (var1.i == var2.i);
}

//////// Determines if a variable fulfils split condition  ///////////////////////////////////
CART_INLINE BOOL icxIsVarSplitLeft(CxCARTBase* cart, CxCARTSplit* split, CxClassifierVar var)
{
	assert(cart && split);
	int feature = split->feature_idx;
	int type = cart->feature_type[feature];
#ifdef _DEBUG
	int eff_feature = cart->features_corr[feature];
	assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );
#endif
	BOOL go_left = TRUE;
	if (icxIsClassifierVarNumeric(type))
	{
#if FLOAT_ONLY
		go_left = (split->boundary.value.fl >= var.fl) ;
#else
		if (icxIsClassifierVar32f(type))
			go_left = (split->boundary.value.fl >= var.fl) ;
		else
			go_left =  (split->boundary.value.i >= var.i) ;
#endif
	}
	else
	{
#if FLOAT_ONLY
		int i = cvRound(var.fl) ;  
#else
		int i = icxIsClassifierVar32f(type) ? cvRound(var.fl) : var.i;
#endif
		assert((i >=0) && (i < cart->num_classes[feature]));
		char c = ((char*)split->boundary.ptr)[i];
		assert(c >= 0 && c < 3);
		go_left = c&1;
	}
	assert(split->revert == 0 || split->revert == 1);
	return split->revert ^ go_left ;
}
#pragma warning(default:4100)

CART_INLINE BOOL icxIsVarSplitLeft(CxCART* cart, CxCARTSplit* split, CxClassifierVar var)
{
	return icxIsVarSplitLeft((CxCARTBase*)cart, split, var);
}

CART_INLINE CxSplitBoundary 
icxGetHalfwayBetween(CxClassifierVar val1, CxClassifierVar val2, BOOL floating)
{
    CxSplitBoundary boundary;

    if (floating) {
	boundary.value.fl = (val1.fl + val2.fl) * 0.5f;

        // Idiotic but nessesary check!!!
	if (boundary.value.fl >= val2.fl) { 
	    boundary.value.fl = val1.fl;
	}
    } else {
	// Use truncation rather than ceil because icxIsVarSplitLeft uses 
	// boundary.value.i >= var.i, and f >= i iff floor(f) >= i.
	boundary.value.i = (int)((val1.i + val2.i) * 0.5f);
    }

    return boundary;
}

///// Iterates through all sequences of 0 and 1 of given length /////////
CART_INLINE BOOL icxGetNextSeq( char* ptr , int n)
{
	for (int i = n-1; i>=0 ; i--) 	
	{
		if (ptr[i] == 1)
			ptr[i] = 0;
		else
		{
			ptr[i] = 1;
			return TRUE;
		}
	}
	return FALSE;
}

////// Calculates Gini index for an array ///////////
CART_INLINE float icxCalcGiniIndexPriors(int* stats ,int n , float* priors_mult)
{
#if SSE2_OPTIMIZE
//	if (n > MIN_ALIGN)
//	{
		icxAssertAligned(stats);
		icxAssertAligned(priors_mult);
		float sum[2];
		float sumsq[2];
		__asm
		{
			mov edx , [stats]
			mov ebx, [priors_mult]
			mov	ecx , [n]	
			dec ecx
			shr ecx , 2
			inc ecx
			xorps xmm0 , xmm0				 // sum p*s in xmm0
			xorps xmm1 , xmm1				 // sum p*p*s*s in xmm1
	loop1 :
			movdqa xmm2 , XMMWORD PTR [edx]  // xmm2 <- stats
			cxtdq2ps xmm2, xmm2		         // convert to float
			movaps xmm4 , XMMWORD PTR [ebx]  // xmm4 <- priors
			mulps xmm2 , xmm4
			addps xmm0 , xmm2
			mulps xmm2 , xmm2
			addps xmm1 , xmm2
			add edx , 16
			add ebx , 16
			dec ecx
			jne loop1

			movhlps xmm2 , xmm0
			addps xmm0, xmm2
			movhlps xmm3 , xmm1
			addps xmm1 , xmm3
			movlps [sum] , xmm0
			movlps [sumsq] , xmm1
		}   // Gini index - integer with priors
		(*sum) += sum[1];
		(*sumsq) += sumsq[1];
		return ((*sum) > 0.0) ? (*sum) - (*sumsq)/ (*sum) : 0.0;
#else
	double sum = 0.0;
	double sum_sq = 0.0;
	for ( ; n-- ; stats++ , priors_mult++)
	{
		double f = (*stats) * (*priors_mult);
		sum += f;
		sum_sq += f * f;
	}
	return (float)((sum > 0.0) ? sum - sum_sq/ sum : 0.0);
#endif
}

////// Calculates Gini index for an array ///////////
CART_INLINE double icxCalcGiniIndex(int* stats ,int n)
{
#if SSE2_OPTIMIZE
//	assert( n % CART_ELEM_ALIGN == 0); // Check n(mod 4) == 0
	icxAssertAligned(stats);
	float sum[2];
	float sumsq[2];
	__asm
	{
		mov edx , [stats]
		mov	ecx , [n]	
		dec ecx
		shr ecx , 2
		inc ecx
		xorps xmm0 , xmm0
		xorps xmm1 , xmm1
loop1 :
		movaps xmm2 , XMMWORD PTR [edx]
		addps xmm0 , xmm2            // xmm0 <- sum 
		mulps xmm2 , xmm2         
		addps xmm1 , xmm2            // xmm1 <- sum of squares 
		add edx , 16
		dec ecx
		jnz loop1
		
		movhlps xmm2 , xmm0
		addps xmm0, xmm2			 // sum xmm0 
		movhlps xmm3 , xmm1
		addps xmm1 , xmm3			 // sum xmm1
		movlps [sum] , xmm0
		movlps [sumsq] , xmm1
	}    // Gini index - floating - point	
	(*sum) += sum[1];
	(*sumsq) += sumsq[1];
	return ((*sum) <= 0) ? 0.0 : (*sum) - (*sumsq)/ (*sum);
#else
	int sum = 0;
	double sum_sq = 0.0;
	for ( ; n-- ; stats++)
	{
		int stat = (*stats);
		sum += stat;
		double f = stat;
		sum_sq += f * f;
	}
	return (float)( sum ? (sum - sum_sq/sum) : 0.0);

#endif
}

CART_INLINE float icxCalcGiniIndexWithCosts(int* stats ,int n , float* cost_gini_mult, float* priors_mult = NULL)
{
	assert(cost_gini_mult);
	double sum_sq = 0.0;
	float* _cost_gini_mult = cost_gini_mult;
	if (priors_mult)
	{
		double sum = 0.0;
		for (int i = 0 ; i < n ; i++, _cost_gini_mult += n)
		{
			float stat = (float)stats[i];
			sum += priors_mult[i] * stat;
			for (int j = i + 1 ; j < n ; j++)
			{
				sum_sq += stat * stats[j] * _cost_gini_mult[j];
			}
		}
		return (float)( sum ? (sum_sq/sum) : 0.0);
	}
	else
	{
		int sum = 0;
		for (int i = 0 ; i < n ; i++, _cost_gini_mult += n)
		{
			int stat = stats[i];
			sum += stat;
			for (int j = i + 1 ; j < n ; j++)
			{
				sum_sq += ((float)stat) * stats[j] * _cost_gini_mult[j];
			}
		}
		return (float)( sum ? (sum_sq/sum) : 0.0);
	}

}

////// Calculates Gini index for an array ///////////
CART_INLINE double icxCalcEntropy(int* stats ,int n)
{
	int sum = 0;
	double sum_log = 0.0;
	for ( ; n-- ; stats++)
	{
		int stat = (*stats);
		if (stat)
		{
			sum += stat;
			double f = stat;
			sum_log += f * log(f);
		}
	}
	return (float)( sum * log(float(sum)) - sum_log );
}

////// Calculates Gini index for an array ///////////
CART_INLINE float icxCalcEntropyPriors(int* stats ,int n , float* priors_mult)
{
	double sum = 0.0;
	double sum_log = 0.0;
	for ( ; n-- ; stats++ , priors_mult++)
	{
		int stat = (*stats);
		if (stat)
		{
			double f = (*stats) * (*priors_mult);
			sum += f;
			sum_log += f * log(f);
		}
	}
	return (float)( sum * log(sum) - sum_log );
}

////// Summmarizes integer vector ///////////
CART_INLINE int icxSumVector(int* v ,int n)
{
#if SSE2_OPTIMIZE
//	assert( n % CART_ELEM_ALIGN == 0); // Check n(mod 4) == 0
	icxAssertAligned(v);
//	if (n > MIN_ALIGN + 4)
//	{
		int sum[2];
		__asm
		{
			mov edx , [v]
			mov	ecx , [n]	
			dec ecx
			shr ecx , 2
			inc ecx
			xorps xmm0 , xmm0
	loop1 :
			movdqa xmm1 , XMMWORD PTR [edx]
			paddd xmm0 , xmm1            // xmm0 <- sum 
			add edx , 16
			dec ecx
			jnz loop1
			
			movhlps xmm1 , xmm0
			paddd xmm0, xmm1			 // sum xmm0 
			movlps [sum] , xmm0
		}    // Gini index - floating - point	
		return sum[0]+sum[1];
/*	}
	else
	{
		int sum = 0;
		for ( ; n-- ; v++)
			sum +=	(*v);
		return sum;
	}*/
#else
	int sum = 0;
	for ( ; n-- ; v++)
		sum +=	(*v);
	return sum;
#endif
}

////// Summmarizes integer vector ///////////
CART_INLINE float icxSumVector(float* v ,int n)
{
#if SSE2_OPTIMIZE
	icxAssertAligned(v);

	float sum[2];
	__asm
	{
		mov edx , [v]
		mov	ecx , [n]	
		dec ecx
		shr ecx , 2
		inc ecx
		xorps xmm0 , xmm0
loop1 :
		movaps xmm1 , XMMWORD PTR [edx]
		addps xmm0 , xmm1            // xmm0 <- sum 
		add edx , 16
		dec ecx
		jnz loop1
		
		movhlps xmm1 , xmm0
		addps xmm0, xmm1			 // sum xmm0 
		movlps [sum] , xmm0
	}    // Gini index - floating - point	
	return sum[0]+sum[1];
#else
	float sum = 0.0;
	for ( ; n-- ; v++)
		sum +=	(*v);
	return sum;
#endif
}



#pragma warning(disable : 4244)
/////// Converts an array of integers to floats ////////////
CART_INLINE float* icxArrToFloat(int* arr , int n)
{
#if SSE2_OPTIMIZE
	icxAssertAligned(arr);
__asm
{
	mov edx , [arr]
	mov	ecx , [n]	
	dec ecx
	shr ecx , 2
	inc ecx
loop1 :
	movdqa xmm0 , XMMWORD PTR [edx]
	cxtdq2ps xmm0, xmm0		
	movaps [edx] , xmm0
	add edx , 16
	dec ecx
	jnz loop1
	mov eax , [n]
}
#else
	int* _arr = arr;
	float* _farr = (float*)arr;
	for (  ; n-- ; _arr++ , _farr++)
		(*_farr) =(*_arr) ;
#endif
	return (float*)arr;
}

/////// Converts an array of floats to integers ////////////
CART_INLINE int* icxArrToInt(float* farr , int n)
{
#if SSE2_OPTIMIZE
	icxAssertAligned(farr);
__asm
{
	mov edx , [farr]
	mov	ecx , [n]	
	dec ecx
	shr ecx , 2
	inc ecx
loop1 :
	movaps xmm0 , XMMWORD PTR [edx]
	cxtps2dq xmm0, xmm0		
	movdqa [edx] , xmm0
	add edx , 16
	dec ecx
	jnz loop1
	mov eax , [n]
}
#else
	float* _farr = farr;
	int* _iarr = (int*)farr;
	for (  ; n-- ; _iarr++ , _farr++)
		(*_iarr) = cvRound(*_farr) ;
#endif
	return (int*)farr;
}
#pragma warning(default : 4244)

////// Adds integer vectors : v2 to v1, v1 <- v1+v2 /////////////
CART_INLINE void icxAddVector(int* v1 , int* v2 , int n) 
{
#if SSE2_OPTIMIZE
//	if (n > MIN_ALIGN)
//	{
		icxAssertAligned(v1);
		icxAssertAligned(v2);
		__asm
		{
		//	mov eax , 16
			mov edx , [v1]
			mov ebx , [v2]
			mov	ecx , [n]
			dec ecx
			shr ecx , 2
			inc ecx
	loop1 :
			movdqa xmm0 , XMMWORD PTR [edx]  // xmm0 <- v1
			movdqa xmm1 , XMMWORD PTR [ebx]  // xmm1 <- v2
			paddd xmm0 , xmm1
			add ebx , 16
			movdqa XMMWORD PTR [edx] ,  xmm0
			add edx , 16
			dec ecx
			jnz loop1
		}
/*	}
	else
	{
		for ( ; n-- ; v1++ , v2++)
			(*v1) += (*v2) ;
	}*/
#else
	for ( ; n-- ; v1++ , v2++)
		(*v1) += (*v2) ;
#endif
}

////// Adds float vectors : v2 to v1, v1 <- v1+v2 /////////////
CART_INLINE void icxAddVector(float* v1 , float* v2 , int n) 
{
#if SSE2_OPTIMIZE
	icxAssertAligned(v1);
	icxAssertAligned(v2);
__asm
{
		mov edx , [v1]
		mov ebx , [v2]
		mov	ecx , [n]	
		dec ecx
		shr ecx , 2
		inc ecx
loop1 :
		movaps xmm0 , XMMWORD PTR [edx]  // xmm0 <- v1
		movaps xmm1 , XMMWORD PTR [ebx]  // xmm1 <- v2
		addps xmm0 , xmm1
		movaps [edx] , xmm0
		add edx , 16
		add ebx , 16
		dec ecx
		jnz loop1
}
#else
	for ( ; n-- ; v1++ , v2++)
		(*v1) += (*v2) ;
#endif
}

////// Subtracts integer vectors : v1 <- v1-v2 /////////////
CART_INLINE void icxSubVector(int* v1 , int* v2 , int n) 
{
#if SSE2_OPTIMIZE
	icxAssertAligned(v1);
	icxAssertAligned(v2);
__asm
{
		mov edx , [v1]
		mov ebx , [v2]
		mov	ecx , [n]	
		dec ecx
		shr ecx , 2
		inc ecx
loop1 :
		movdqa xmm0 , XMMWORD PTR [edx]  // xmm0 <- v1
		movdqa xmm1 , XMMWORD PTR [ebx]  // xmm1 <- v2
		psubd xmm0 , xmm1
		movdqa [edx] , xmm0
		add edx , 16
		add ebx , 16
		dec ecx
		jnz loop1
}
#else
	for ( ; n-- ; v1++ , v2++)
		(*v1) -= (*v2) ;
#endif
}

////// Subtracts float vectors : v1 <- v1-v2 /////////////
CART_INLINE void icxSubVector(float* v1 , float* v2 , int n) 
{
#if SSE2_OPTIMIZE
	icxAssertAligned(v1);
	icxAssertAligned(v2);
__asm
{
		mov edx , [v1]
		mov ebx , [v2]
		mov	ecx , [n]	
		dec ecx
		shr ecx , 2
		inc ecx
loop1 :
		movaps xmm0 , XMMWORD PTR [edx]  // xmm0 <- v1
		movaps xmm1 , XMMWORD PTR [ebx]  // xmm1 <- v2
		subps xmm0 , xmm1
		movaps [edx] , xmm0
		add edx , 16
		add ebx , 16
		dec ecx
		jnz loop1
}
#else
	for ( ; n-- ; v1++ , v2++)
		(*v1) -= (*v2) ;
#endif
}

CART_INLINE float icxScalarProd(float* v1 , float* v2 , int n)
{
	float f = 0.0;
	for ( ; n-- ; v1++ , v2++)
		f += ((*v1) * (*v2) );	
	return f;
}

#ifdef _DEBUG
#include <math.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcDistance
//    Purpose: calculates chi-square distance between two frequency distributions
//    Parameters:
//      freq1 - first frequencu distribution 
//      freq2 - second frequencu distribution 
//      mult1 - multiplier for first frequency distribution (usuallu inverted sum of frequencies)
//      mult2 - multiplier for second frequency distribution 
//      num_resp - number of response classes
//    Returns:
//		distance between cluster1 and cluster2
///////////////////////////////////////////////////////////////////////////////////////

CART_INLINE float cxCalcDistance(float* freq1, float* freq2, 
  			   				     float mult1, float mult2, 
					             float* inv_class_freq , int num_resp)
{
	if (mult1 < EPS || mult2 < EPS)
		return 0.0f;
	mult1 = 1.0f/mult1;
	mult2 = 1.0f/mult2;

#if SSE2_OPTIMIZE
//	if (num_resp > MIN_ALIGN)
//	{
		icxAssertAligned(freq1);
		icxAssertAligned(freq2);
		icxAssertAligned(inv_class_freq);
		float sum[2];
		__asm
		{
			movss xmm2 , [mult1]
		//	rcpss xmm2 , xmm2
			shufps xmm2, xmm2 , 0 // xmm2 <- mult1 x 4
			movss xmm3 , [mult2]
		//	rcpss xmm3 , xmm3
			shufps xmm3, xmm3 , 0 // xmm3 <- mult2 x 4  
		
			mov eax , [inv_class_freq]
			mov ebx, [freq1]
			mov edx, [freq2]
			mov	ecx , [num_resp]	
			dec ecx
			shr ecx , 2
			inc ecx
			xorps xmm5 , xmm5
	loop1 :
			movaps xmm0 , XMMWORD PTR [ebx]	// xmm0 <- freq1 
			mulps xmm0 , xmm2
			movaps xmm1 , XMMWORD PTR [edx] // xmm1 <- freq2
			mulps xmm1 , xmm3
			subps xmm0 , xmm1
			movaps xmm4 , XMMWORD PTR [eax] // xmm4 <- inv_class_freq
			mulps xmm0 , xmm0
			mulps xmm0 , xmm4
			addps xmm5 , xmm0
			add edx , 16
			add ebx , 16
			add eax , 16
			dec ecx
			jne loop1

			movhlps xmm2 , xmm5
			addps xmm5, xmm2
			movlps [sum] , xmm5
		}   
		return sum[0] + sum [1];
/*	}
	else
	{
		float sum = 0.0;
		for ( ; num_resp-- ; freq1++ , freq2++ , inv_class_freq++)
		{
			float d = (*freq1) * mult1 - (*freq2) * mult2;
			sum += d * d * (*inv_class_freq);
		}
		return sum;
	}*/
/*	assert( fabs(sum1 - sum[0] - sum[1]) < EPS);*/

/*#if INLINE_ASM_OPTIMIZE
	__asm
	{
		mov ecx , num_resp
		mov esi , freq1 
		mov edi , freq2
		mov edx , inv_class_freq 
		fld dword ptr [mult1]
		fld dword ptr [mult2]
		fldz
loop1:
		fld dword ptr [esi]
		add esi , 4
		fmul st(0) , st(3)
		fld dword ptr [edi]
		add edi , 4
		fmul st(0) , st(3)
		fsubp st(1) , st(0)
		fmul st(0),st(0)
		fld dword ptr [edx]
		add edx , 4
		fmulp st(1) , st(0)
		faddp st(1) , st(0)
		dec ecx
		jnz loop1
		fstp dword ptr [sum]
		fstp st(0)
		fstp st(0)
	}
#ifdef _DEBUG
	float* _freq1 = freq1;
	float* _freq2 = freq2;
	float* _inv_class_freq = inv_class_freq;
	float sum1 = 0.0;
	for ( ; num_resp-- ; _freq1++ , _freq2++ , _inv_class_freq++)
	{
		float d = (*_freq1) * mult1 - (*_freq2) * mult2;
		sum1 += d * d * (*_inv_class_freq);
	}
	assert ((sum1 == 0.0 && sum == 0.0 ) || fabs (1.0 - sum/sum1) < EPS);
#endif*/
#else	
	float sum = 0.0;
	for ( ; num_resp-- ; freq1++ , freq2++ , inv_class_freq++)
	{
		float d = (*freq1) * mult1 - (*freq2) * mult2;
		sum += d * d * (*inv_class_freq);
	}
	return sum;
#endif

}

//////// Distance between clusters ///////////////////
CART_INLINE float cxCalcDistance(CxVarCategoryCluster* cluster1,
		 					     CxVarCategoryCluster* cluster2, 
							     float* inv_class_freq , int num_resp)
{
	return cxCalcDistance(cluster1->frequencies,
		                  cluster2->frequencies,
						  cluster1->sum_frequencies,
						  cluster2->sum_frequencies,
						  inv_class_freq, num_resp);
}

//////// Distance between cluster and category ///////////////////
CART_INLINE float cxCalcDistance(CxVarCategoryCluster* cluster,
							     CxVarCategory* var, 
							     float* inv_class_freq , 
							     int num_resp)
{
	return cxCalcDistance(cluster->frequencies,
		                  var->frequencies,
						  cluster->sum_frequencies,
						  var->sum_frequencies,
						  inv_class_freq, num_resp);

}

////// Calculates average for an array ///////////
CART_INLINE double icxCalcAverage(float* pf , int n)
{
	double sum = 0.0;
	for (int i = 0 ; i < n ; i++)
    	sum += pf[i];
	return sum / n;
}

////// Calculates standard deviation for an array ///////////
CART_INLINE double icxCalcDeviation(float* pf , int n)
{
	double average = icxCalcAverage(pf,n);
	double sum = 0.0;
	for (int i = 0 ; i < n ; i++)
		sum += (pf[i] - average) * (pf[i] - average);
	return sum;
}

/////// Determines if a node is left child of its parent /////
CART_INLINE bool icxIsNodeLeft( CxCARTNode* node )
{
	assert(node);
	CxCARTNode* parent_node = node->parent;
	if (parent_node)
		return (node == parent_node->child_left);
	else
		return FALSE;
}

CART_INLINE BOOL icxIsSubjFeatureMissed(CxCARTSubj* subj, int sample_idx,int feature_idx )  
{
	assert((sample_idx >= 0) && (feature_idx >= 0));
	return subj->body[sample_idx].missed_mask ? *( subj->body[sample_idx].missed_mask + subj->missed_mask_step * feature_idx) : FALSE;
//	return ( !subj->body[sample_idx].missed_mask || *( subj->body[sample_idx].missed_mask + subj->missed_mask_step * feature_idx));
}

CART_INLINE CxClassifierVar icxGetSubjFeature(CxCARTSubj* subj, int sample_idx,int feature_idx )  
{
	assert((sample_idx >= 0) && (feature_idx >= 0));
	assert( !icxIsSubjFeatureMissed(subj , sample_idx, feature_idx));
	return *(CxClassifierVar *)((char *)subj->body[sample_idx].features + subj->features_step * feature_idx) ;
}

CART_INLINE CxClassifierVar icxGetSubjResponse(CxCARTSubj* subj, int sample_idx )  
{
	assert(sample_idx >= 0);
	return subj->body[sample_idx].response;
}


CART_INLINE BOOL icxIsChunkFeatureMissed(CxClassifierSampleChunk* chunk, int sample_idx,int feature_idx )  
{
	assert((sample_idx >= 0) && (feature_idx >= 0));
	int ofs = sample_idx * chunk->missed_step[0] + feature_idx * chunk->missed_step[1];
	return *(chunk->missed_data + ofs) ;
}

CART_INLINE CxClassifierVar icxGetChunkFeature(CxClassifierSampleChunk* chunk, int sample_idx,int feature_idx )  
{
	assert((sample_idx >= 0) && (feature_idx >= 0));
	assert( !icxIsChunkFeatureMissed(chunk , sample_idx , feature_idx));
	int ofs = sample_idx * chunk->features_step[0] + feature_idx * chunk->features_step[1];
	return *(CxClassifierVar*)( ((char*)chunk->features_data) + ofs);
}

CART_INLINE CxClassifierVar icxGetChunkResponse(CxClassifierSampleChunk* chunk, int sample_idx )  
{
	assert(sample_idx >= 0);
	return *(CxClassifierVar *)((char *)chunk->responses_data + (sample_idx) * chunk->responses_step);
}

//// Get the number of progress steps
CART_INLINE int icxGetProgressCount(CxClassifier* cfer)
{
	return cfer->progress_info.progress_data_size;
}

//// Get the progress data 
CART_INLINE CxProgressData* icxGetProgressData(CxClassifier* cfer)
{
	return cfer->progress_info.progress_data;
}

//// Get the progress data n-th element
CART_INLINE CxProgressData* icxGetProgressData(CxClassifier* cfer, int n)
{
	assert(n >= 0 && n < cfer->progress_info.progress_data_size);
	return  (CxProgressData*)((char*)cfer->progress_info.progress_data + 
							  n * cfer->progress_info.progress_data_step);
}

//// Get the number of pruning steps
CART_INLINE int icxGetPruningDataCount(CxCART* cart )
{ return cart->pruning_storage.size; }

//// Get the pruning data on step n
CART_INLINE CxPruningData* icxGetPruningData(CxCART* cart , int n)
{ 
	assert( n >= 0 && n < icxGetPruningDataCount(cart));
	return cart->pruning_storage.buf + n;
}

CART_INLINE CxPruningData* icxGetPruningSeq(CxCART* cart)
{  return cart->pruning_storage.buf; }

CART_INLINE int icxIsClassificator(CxClassifier* cfer)
{ return (cfer->num_response_classes > 0) ; }

/////////// Finds pruning step for given alpha
CART_INLINE CxPruningData* icxFindPruningStep(CxPruningData* alpha_error_seq , 
											  int prune_steps , float alpha)
{
	assert( alpha >= 0.0 );
	assert( alpha_error_seq);
	CxPruningData* data = NULL;
	CxPruningData* data_prev = NULL;
	for (int i = 0 ; i < prune_steps ; i++)
	{
		data_prev = data;
		data = alpha_error_seq + i;
		if (data->alpha > alpha)
			return data_prev; 
	}	
	return data;	
}

////////// Calculates normalizer for complexity penalty and reestimation error.
CART_INLINE double icxGetNormalizer(CxClassifier* cfer)
{
	int size = icxGetProgressCount(cfer);
	if (!cfer || (size < 1))
		return 0.0;
	CxProgressData* data = icxGetProgressData(cfer , size - 1);
	return data->reestimation_error;	
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxIsStringSplitLeft
//    Purpose: determines a direction to which a given split node directs features
//    Parameters:
//      cart - classification and regression tree
//      node - node of a tree
//      features - pointer to features
//      pruning_step - pruned subtree number
//      features_step - optional step of features disposition
//      missed_mask - optional pointer to array-mask of missed parameters
//            nonzero values indicate miss
//      missed_mask_step - optional step of missed_mask disposition
//      split - a split to test, NULL default value for node split ( must be nonzero in this case)
//      direction - preferred direction when no more surrogates are avaliable : -1 - left, 1 - right ,
//      0 - direction of majority
//    Returns:
//      1 if a string goes left , 0 - right
//F*/
CART_INLINE BOOL cxIsStringSplitLeft( CxCARTBase* cart,
								      CxForestNode* node,
                                      void *features,
								      int features_step CV_DEFAULT(sizeof( CxClassifierVar )),
                                      char *missed_mask CV_DEFAULT(0),
                                      int missed_mask_step CV_DEFAULT(1),
								      CxCARTSplit* split CV_DEFAULT(NULL),
								      int direction CV_DEFAULT(0))
{
    CV_FUNCNAME( "cxIsStringSplitLeft" );
    __BEGIN__;
	CxCARTSplit* used_split = split ? split : node->split;
	assert(used_split);
	while (used_split)
	{
		int feature = used_split->feature_idx;
		if (!missed_mask || (*(missed_mask + missed_mask_step * feature) == 0))
		{
			CxClassifierVar var = (*(CxClassifierVar*)((char *)features + features_step * feature));
			if (icxIsClassifierVarCategoric(cart->feature_type[feature]) && (((char*)used_split->boundary.ptr)[icxVarToInt(var)] == 2))
				used_split = used_split->next_surrogate;
			else
				return icxIsVarSplitLeft(cart, used_split , var);
		}
		else 
			used_split = used_split->next_surrogate;
	}
//	return (direction) ? -((direction - 1) >> 1) :  (node->child_left->num_fallens > node->child_right->num_fallens);
	int dir = direction ? direction : node->direction;
	return (dir == -1);
	__CLEANUP__;
    __END__;
}

CART_INLINE BOOL cxIsStringSplitLeft( CxCART* cart,
								      CxCARTNode* node,
                                      void *features,
								      int features_step CV_DEFAULT(sizeof( CxClassifierVar )),
                                      char *missed_mask CV_DEFAULT(0),
                                      int missed_mask_step CV_DEFAULT(1),
								      CxCARTSplit* split CV_DEFAULT(NULL),
								      int direction CV_DEFAULT(0))
{
	return cxIsStringSplitLeft((CxCARTBase*)cart, (CxForestNode*)node , 
								features, features_step, missed_mask, 
								missed_mask_step, split, direction);
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxIsAtomSplitLeft
//    Purpose: determines a direction to which a given subj atom is split
//    Parameters:
//      cart - classification and regression tree
//      node - node of a tree
//      atom_idx - node subj atom number
//    Returns:
//      1 if an atom goes left , 0 - right
//F*/
CART_INLINE BOOL cxIsAtomSplitLeft( CxCART *cart,
		  						    CxCARTNode* node,
                                    int atom_idx,
								   	CxCARTSplit* split = NULL,
								    int direction = 0)
{
	CxCARTSubj* subj = node->subj;
	CxCARTAtom* atom = subj->body + atom_idx;
	return cxIsStringSplitLeft(cart, node,
		                       atom->features , subj->features_step,
							   atom->missed_mask, subj->missed_mask_step,
							   split, direction);
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    icxGetCatProportions
//    Purpose: retrieves category proportions for given categoric feature
//             in strings fallen to given node 
//    Parameters:
//      cart - classification and regression tree
//      node - node of a tree
//      eff_feature - effective number of feature
//    Returns:
//      Array of numbers of feature categories. 
//    Notes:
//		Allocates memory in classification mode
//F*/
CART_INLINE int* icxGetCatProportions(CxCART* cart, CxCARTNode* node, int eff_feature)
{
	int cur_feature = cart->features_back_corr[eff_feature];
	assert( icxIsClassifierVarCategoric(cart->feature_type[cur_feature]) );
	BOOL classification = icxIsClassifierVarCategoric(cart->response_type);
	assert(node->cat_proportions);
	if (classification)
	{
		int num_resp = cart->num_response_classes;
#if CART_ALIGN_MEMORY
		int num_resp_aligned = num_resp;
		if (num_resp > MIN_ALIGN)
			num_resp_aligned = ((num_resp-1) & (-CART_ELEM_ALIGN)) + CART_ELEM_ALIGN;
#else
		int num_resp_aligned = num_resp;
#endif
//		int* idx = node->fallen_idx + eff_feature * node->num_fallens;
		int num_classes = cart->num_classes[cur_feature];
		int* num_var_classes = (int*) calloc(num_classes , sizeof(int));
		int* cat_proportions = node->cat_proportions[eff_feature];
		for (int j = 0 ; j < num_classes ; j++)
		{
			int sum = 0;
			int* _cat_proportions = cat_proportions + j * num_resp_aligned;
			for (int resp = 0 ; resp < num_resp ; resp++)
				sum += _cat_proportions[resp];
			num_var_classes[j] = sum;
		}
		return num_var_classes;
	}
	else
		return node->cat_proportions[eff_feature];

}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcNodeError
//    Purpose: Calculates node error, R(t) = r(t) * p(t)
//    Parameters:
//      cart - classification and regression tree
//      node - node
//    Returns:
//      Node error
//F*/
/*CART_INLINE double cxCalcNodeError(CxCART *cart, CxCARTNode *node)
{
	return node->error/cart->root->num_fallens;
}*/

//// Sets classifier error and description
CART_INLINE void icxSetCARTError(CxCARTBase* cart , int code , const char* description, int status)
{
	cart->error.code = code;
	strcpy(cart->error.description , description);
	cart->error.status = status;
}

CART_INLINE void icxSetCARTError(CxCART* cart , int code , const char* description, int status)
{
	icxSetCARTError((CxCARTBase*)cart, code, description, status);
}

CART_INLINE float* icxAllocFloatBuf(CxCART* cart)
{ return cart->storage->calc_storage.buf_resp; }

CART_INLINE CxClassifierVar* icxAllocFeatureVals(CxCART* cart)
{ return cart->storage->calc_storage.buf_feat_vals; }

//// Returns internal buffer for splits of any type
CART_INLINE CxCARTSplit* icxGetSplitBuffer(CxCART* cart)
{ return (CxCARTSplit*)cart->storage->calc_storage.buf_splits; }

//// Returns number of samples of interest in sample
CART_INLINE int icxGetNumSamplesOfInterest(CxClassifierSample* sample)
{
	CxClassifierSampleChunk* chunk = sample->chunk[0];
	assert(sample->num_chunks == 1);
	return chunk->indices_of_interest ? chunk->num_indices_of_interest : chunk->num_points;
}

//// Allocates memory for maximum split size
CART_INLINE CxCARTSplit* icxAllocMaxSplitSize(CxCARTStorage* storage)
{
	assert(storage);
	int s = storage->node_storage.manual_split_space;
	assert(s > 0);
	CxCARTSplit* split = (CxCARTSplit*)malloc(s);
	split->next_competitor = split->next_surrogate = NULL;	
	return split;
}

//// Get node position in node storage
CART_INLINE CxCARTNode* icxGetNodePos(CxCARTNodeStorage& node_storage , 
									  int id , int depth)
{
	assert(depth <= node_storage.max_depth);
	assert(node_storage.num_blocks > 0);
	CxCARTLevel& level = node_storage.levels[depth];
	assert(level.buf_nodes);
	int node_size = node_storage.node_size;
	assert(node_size > 0);
	char* buf = (char*)level.buf_nodes + node_size * (id - (1 << depth));
	return (CxCARTNode*)buf;
}

//// Get size of split are for given split type
CART_INLINE int icxGetSplitAreaSize(CxCART* cart, int split_type)
{
	CxCARTNodeStorage& node_storage = cart->storage->node_storage;
	return (split_type == SPLIT_PRIMARY || split_type == SPLIT_COMPETITOR) ? node_storage.competitors_space :
		   (split_type == SPLIT_SURROGATE) ? node_storage.surrogates_space : node_storage.manual_split_space ; 
}

//// Get split area base pointer for given type of split
CART_INLINE char* icxGetSplitArea(CxCART* cart, CxCARTNode* node , int split_type)
{
	CxCARTNodeStorage& node_storage = cart->storage->node_storage;
	char* bp = (char*)(node + 1);
	if (split_type == SPLIT_PRIMARY || split_type == SPLIT_COMPETITOR)
		return bp;
	else if (split_type == SPLIT_SURROGATE)
		return bp + node_storage.competitors_space;
	else 
	{
		assert(split_type == SPLIT_MANUAL);
		return bp + node_storage.competitors_space + node_storage.surrogates_space;
	}
}

CART_INLINE BOOL icxIsSplitManual(CxCART* cart , CxCARTNode* node)
{
	CxCARTSplit* split = node->split;
	if (!split)
		return FALSE;
	return (icxGetSplitArea(cart , node , SPLIT_MANUAL) == (char*)split);
}

//// Get size of calc storage level
/*CART_INLINE int icxGetLevelSize(int level , int num_samples, int node_size, BOOL for_idx = TRUE)
{
	assert ( level >= 0 ) ;
	assert ( num_samples > 0 && node_size >= sizeof(CxCARTNode) );
	return (1 << level) * node_size + (for_idx ? num_samples * sizeof(int) : 0);
}*/

//// Get size of calc storage block
CART_INLINE int icxGetNodeBlockSize(int level_low , int level_high, int node_size)
{
	assert( level_high >= -1 && level_low >= -1);
	assert( level_high > level_low);
	assert( node_size >= sizeof(CxCARTNode) );
	return ((1 << (level_high + 1)) - (1 << (level_low + 1))) * node_size ;    // For nodes 
}

CART_INLINE int icxGetIdxBlockSize(int level_low , int level_high, int num_samples)
{
	assert( level_high >= -1 && level_low >= -1);
	assert( level_high > level_low);
	assert( num_samples > 0 );
	return (level_high - level_low) * num_samples * sizeof(int);   // For shrunk_fallen_idx;
}

//// Reduces size of pruning storage , leaving only pruning info. Use after calculating sample errors
CART_INLINE void icxFinalizePruningStorage(CxCARTPruningStorage& pruning_storage)
{
	int size = pruning_storage.size ;
	pruning_storage.buf = (CxPruningData*)realloc(pruning_storage.buf , size * (sizeof(CxPruningData)));
	pruning_storage.pruned_nodes = NULL;
	CxPruningData* data = pruning_storage.buf;
	for (int i = 0 ; i < size ; i++ , data++)
		data->nodes_seq = NULL;
}

//// Increase size of cross - validation storage
CART_INLINE BOOL icxGrowCrossValStorage(CxCART* cart , CxCARTCrossValStorage& crval_storage , int new_size)
{
	int& max_size = crval_storage.max_size;
	CxPruningData*& buf = crval_storage.buf;
	if (max_size < new_size)
	{
		int new_max_size = MAX( new_size , max_size * 2 + 4);
		buf = (CxPruningData*)realloc(buf , new_max_size * sizeof(CxPruningData));
		if (!buf)
		{
			icxSetCARTError((CxCARTBase*)cart , CART_MEMORY_ERROR , 
							"Memory allocation error" , CART_ERROR_STATUS_FATAL);
			return FALSE;
		}
		crval_storage.max_size = new_max_size;
	}	
	return TRUE;
}

//// Add pruning sequence to cross - validation storage
CART_INLINE BOOL icxAddToCrossValStorage(CxCART* cart , CxCARTCrossValStorage& crval_storage ,
										 CxPruningData* pruning_seq , int add_size)
{
	int& cur_size = crval_storage.cur_size;
	if (!icxGrowCrossValStorage(cart , crval_storage , cur_size + add_size))
		return FALSE;
	crval_storage.sizes[crval_storage.num_pruning_seqs++] = add_size;
	assert(crval_storage.num_pruning_seqs <= crval_storage.V );
	memcpy(crval_storage.buf + cur_size , pruning_seq , add_size * sizeof(CxPruningData));
	cur_size += add_size;
	return TRUE;
}

//// Fill seq_ptrs, and shrink buffer , freeng extra memory
CART_INLINE void icxFinalizeCrossValStorage(CxCARTCrossValStorage& crval_storage)
{
	int V = crval_storage.V;
	CxPruningData* data = crval_storage.buf;
	for (int i = 0 ; i < V ; i++)
	{
		crval_storage.seq_ptrs[i] = data;
		data += crval_storage.sizes[i];
	}
	assert(crval_storage.cur_size <= crval_storage.max_size);
	crval_storage.buf = (CxPruningData*)realloc(crval_storage.buf , crval_storage.cur_size * sizeof(CxPruningData));
}

/// Free pruning storage
CART_INLINE void icxFreePruningStorage(CxCARTPruningStorage& pruning_storage)
{
	if (pruning_storage.buf)
	{
		free(pruning_storage.buf);
		memset( &pruning_storage , 0 , sizeof(CxCARTPruningStorage));
	}
}

//// Frees calculation storage
CART_INLINE void icxFreeCalcStorage(CxCARTCalcStorage& calc_storage)
{
	if (calc_storage.buf_fallen_idx)
	{
		free(calc_storage.buf_fallen_idx);
		memset( &calc_storage , 0 , sizeof(CxCARTCalcStorage));
	}
}

//// Frees cross - validation storage
CART_INLINE void icxFreeCrossValStorage(CxCARTCrossValStorage& crval_storage)
{
	if (crval_storage.buf)
	{
		free(crval_storage.buf);
	}
	if (crval_storage.seq_ptrs)
	{
		free(crval_storage.seq_ptrs);
	}
	memset( &crval_storage , 0 , sizeof(CxCARTCrossValStorage));
}

//// Frees node storage
CART_INLINE void icxFreeNodeStorage(CxCARTNodeStorage& node_storage)
{
	for (int i = 0 ; i < node_storage.num_blocks ; i++)
	{
		assert(node_storage.blocks[i].buf_nodes);
		free(node_storage.blocks[i].buf_nodes);
		if (node_storage.blocks[i].buf_shrunk_idx)
		{
			free(node_storage.blocks[i].buf_shrunk_idx);
			node_storage.blocks[i].buf_shrunk_idx = NULL;
		}
	}
	memset(&node_storage , 0 , sizeof(CxCARTNodeStorage));
	node_storage.depth = node_storage.max_depth = -1;
}

//// Frees both node and calculation storage
CART_INLINE void icxFreeStorage(CxCARTStorage* storage)
{
	icxFreeNodeStorage(storage->node_storage);
	icxFreeCalcStorage(storage->calc_storage);
}

//// Allocate node cat_proportions
CART_INLINE int* icxAllocateNodeCatProportions(CxCARTCalcStorage& calc_storage , CxCARTNode* node)
{
	int size_cat_proportions = calc_storage.size_cat_proportions;
	int& top = calc_storage.cat_prop_top;
	assert(node->cur_cat_prop_pos == 0 ); /// Not allocated
	assert(calc_storage.buf_cat_proportions && calc_storage.buf_fallen_idx);
	int* top_block = calc_storage.free_blocks + top;
	assert(*top_block > 0);
	node->cur_cat_prop_pos = *top_block;
	*top_block = 0;
	top++;
	assert( top <= calc_storage.max_depth + 2 );
	return calc_storage.buf_cat_proportions + size_cat_proportions * (node->cur_cat_prop_pos - 1);

}
 
//// De-allocate node cat_proportions
CART_INLINE void icxFreeCatProportions(CxCARTCalcStorage& calc_storage, CxCARTNode* node)
{
	int& top = calc_storage.cat_prop_top;
	assert(calc_storage.buf_cat_proportions && calc_storage.buf_fallen_idx);
	assert(node->cur_cat_prop_pos > 0);
	calc_storage.free_blocks[--top] = node->cur_cat_prop_pos;
	assert(top >= 0);
	node->cur_cat_prop_pos = 0;
}

//// Creates a copy of a split
CART_INLINE CxCARTSplit* icxCopySplit(CxCART* cart, CxCARTSplit* split)
{
	int type = cart->feature_type[split->feature_idx];
	int n = 0;
	if (icxIsClassifierVarCategoric(type))
		n = cart->num_classes[split->feature_idx];
	CxCARTSplit* split_new = (CxCARTSplit* )malloc( sizeof(CxCARTSplit) + n * sizeof(char));
	memcpy(split_new , split , sizeof(CxCARTSplit));
	if (n > 0)
	{
		split_new->boundary.ptr = split_new + 1;
		memcpy ( split_new->boundary.ptr , split->boundary.ptr , n);
	}
	return split_new;
}

//// Copy node to storage
CART_INLINE CxCARTNode* icxCopyNodeToStorage(CxCART* cart , CxCARTNode* node)
{
	CxCARTNodeStorage& node_storage = cart->storage->node_storage;
	CxCARTNode* copy = icxGetNodePos(node_storage , node->id , node->depth);
	memcpy(copy , node , sizeof(CxCARTNode) );
	copy->is_copy = TRUE;
	copy->child_left = copy->child_right = NULL;
	copy->split = NULL;
	copy->direction = 0;
	copy->subj->ref_count++;
	int depth = node->depth;
	assert(depth <= node_storage.max_depth );
	assert(node_storage.num_blocks > 0);

	if ( node_storage.depth < depth)
	{
		assert (depth == node_storage.depth + 1 ); //Cannot grow too fast!
		node_storage.depth = depth;
	}
	return copy;
//	return icxAllocNode(cart , node->id, node->depth , FALSE);
}

//// Allocates node's shrunk_idx position ////
CART_INLINE void icxAllocNodeShrunkIdx(CxCART* cart, CxCARTNode* node)
{
	assert(node->num_fallens > 0);

	CxCARTStorage* storage = cart->storage;
	CxCARTNodeStorage& node_storage = storage->node_storage;

	assert(node->depth <= node_storage.max_depth );
	assert(node_storage.num_blocks > 0);
	assert(node->num_fallens <= storage->num_samples);
	int depth = node->depth;
	CxCARTNode* parent = node->parent;
	//// Calculating position for shrunk_idx
	int offset = 0;
	if (depth > 0)
	{	
		assert(parent);
		assert(parent->num_fallens > node->num_fallens);

		offset = parent->offset;
		CxCARTNode* child_right = parent->child_right;
		if (child_right == node)
			offset += (parent->num_fallens - node->num_fallens);
		node->offset = offset;
	}
	CxCARTLevel& level = node_storage.levels[depth];
	node->shrunk_fallen_idx = level.buf_shrunk_idx + offset;
}

/* Build path for passeng all binary sequences in such a way
   that it only changes in one position on each step */
CART_INLINE int* icxBuildPath( int n , int& path_len)
{
	assert(n > 0 && n <= 20);
	path_len = (1 << n);
	int* path = (int*)malloc(path_len * sizeof(int) ); 
	path[0] = 0;
	int bound = 2;
	int i;
	for (i = 1 ; i < n ; i++ , bound <<= 1)
	{
		path [ bound-1 ] = i;
		memcpy ( path + bound , path , bound * sizeof(int));
	}
	path [ bound-1 ] = i;
	return path;
}

/// Restore binary sequence by its Grey code number
CART_INLINE void icxGetSeq( char* seq , int n , int step )
{
	assert(n > 0 && n <= 20);
	memset (seq , 0 , n * sizeof(char));
	if (step < 0)
		return;
	step += 1;
	seq[0] = (char)(step & 1);
	step >>= 1;
	for (int i = 1 ; (i < n) && step  ; i++ , step >>= 1)
	{
		char bit = (char)(step & 1);
		step -= bit;
		seq[i - 1] ^= bit;
		seq[i] ^= bit;
	}
}

CART_INLINE BOOL icxWriteIndent(FILE* file , int indent)
{
	for ( ; indent -- ; )
		fputs("  ", file);
	return !ferror(file);
}

CART_INLINE void eat_white(FILE* file)
{
	char c = EOF;
	while (isspace(c = (char)fgetc(file)));
	if (c != EOF)
		ungetc(c, file);
}

CART_INLINE int read_str(FILE* file , char* str, int len, char delim)
{
	eat_white(file);
	char c = (char)fgetc(file);
	int _len = len;
	char* _str = str;
	for (; c && (c != delim) && _len-- ; _str++)
	{
		*_str = c;
		c = (char)fgetc(file);
	}
	if (c == 0 || c == delim && !ferror(file))
	{
		*_str = 0;
		return len - _len;
	}
	return -1;
}

CART_INLINE int read_str0( FILE* file , char* str )
{ return read_str(file, str, _MAX_PATH, '\n'); } 

CART_INLINE float Rand(float low , float high)
{
	float f = 1.0f * rand() / (RAND_MAX  + 1) ;
	return low + (high - low) * f;
}

CART_INLINE int  GetResponse(float* prob, int n, float f)
{
	float sum = 0.0;
	int i = 0;	
	while ( (f >= sum) && (i < n) )
	{
		sum += prob[i++];
	}
//	if (i==1)
//		int mmm = 1;
	return i-1;
}

CART_INLINE int SelectWithProbs(float* probs, int n)
{
	float* new_probs = (float*)malloc(n * sizeof(float));
	float sum = 0.0;
	for (int i = 0 ; i < n ; i++)
		sum += probs[i];
	float sum_inv = 1.0f/sum;
	for (int j = 0 ; j < n ; j++)
		new_probs[j] = probs[j] * sum_inv;
	float f = Rand(0 , 1.0f);
	int res = GetResponse(new_probs , n , f);
	free(new_probs);
	return res;
}

CART_INLINE float icxGetNodeVariance(CxCART* cart, CxCARTNode* node)
{
	return (node->error * cart->root->num_fallens) / node->num_fallens;
}

#endif
