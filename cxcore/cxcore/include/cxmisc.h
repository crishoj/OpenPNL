/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/* This is mostly internal use header, which content is likely to change.
   It contains some macro definitions that are used in cxcore, cx, cxaux
   and, probably, other libraries. If you need some of this functionality,
   the safe way is to copy it into your code and rename the macros.
*/
#ifndef _CXCORE_MISC_H_
#define _CXCORE_MISC_H_

/****************************************************************************************\
*                              Compile-time tuning parameters                            *
\****************************************************************************************/

/* maximal size of vector to run matrix operations on it inline (i.e. w/o ipp calls) */
#define  CX_MAX_INLINE_MAT_OP_SIZE  10

/* maximal linear size of matrix to allocate it on stack. */
#define  CX_MAX_LOCAL_MAT_SIZE  32

/* maximal size of local memory storage */
#define  CX_MAX_LOCAL_SIZE  \
    (CX_MAX_LOCAL_MAT_SIZE*CX_MAX_LOCAL_MAT_SIZE*(int)sizeof(double))

/* default image row align (in bytes) */
#define  CX_DEFAULT_IMAGE_ROW_ALIGN  4

/* matrices are continuous by default */
#define  CX_DEFAULT_MAT_ROW_ALIGN  1

/* maximum size of dynamic memory buffer.
   cxAlloc reports an error if larger block is requested. */
#define  CX_MAX_ALLOC_SIZE    (((size_t)1 << (sizeof(size_t)*8-2)))

/* the alignment of all the allocated buffers */
#define  CX_MALLOC_ALIGN    32

/* default alignment for dynamic data strucutures, resided in storages. */
#define  CX_STRUCT_ALIGN   ((int)sizeof(double))

/* default storage block size */
#define  CX_STORAGE_BLOCK_SIZE   ((1<<16) - 128)

/* default memory block for sparse array elements */
#define  CX_SPARSE_MAT_BLOCK    (1<<12)

/* initial hash table size */
#define  CX_SPARSE_HASH_SIZE0    (1<<10)

/* maximal average node_count/hash_size ratio beyond which hash table is resized */
#define  CX_SPARSE_HASH_RATIO    4

/* max length of strings */
#define  CX_MAX_STRLEN  1024

#if 0 /*def  CX_CHECK_FOR_NANS*/
    #define CX_CHECK_NANS( arr ) cxCheckArray((arr))  
#else
    #define CX_CHECK_NANS( arr )
#endif

/****************************************************************************************\
*                                  Common declarations                                   *
\****************************************************************************************/

/* get alloca declaration */
#ifdef WIN32
    #if defined _MSC_VER || defined __BORLANDC__
        #include <malloc.h>
    #endif
#endif

#ifdef __GNUC__
    #undef alloca
    #define alloca __builtin_alloca
#endif

#if defined _MSC_VER || defined __BORLANDC__ || defined __ICL
    #define CX_BIG_INT(n)   n##I64
    #define CX_BIG_UINT(n)  n##UI64
#else
    #define CX_BIG_INT(n)   n##LL
    #define CX_BIG_UINT(n)  n##ULL
#endif

#define CX_IMPL CX_EXTERN_C

/* default step, set in case of continuous data
   to work around checks for valid step in some ipp functions */
#define  CX_STUB_STEP     (1 << 30)

#define  CX_SIZEOF_FLOAT ((int)sizeof(float))
#define  CX_SIZEOF_SHORT ((int)sizeof(short))

#define  CX_ORIGIN_TL  0
#define  CX_ORIGIN_BL  1

/* IEEE754 constants and macros */
#define  CX_POS_INF       0x7f800000
#define  CX_NEG_INF       0x807fffff /* CX_TOGGLE_FLT(0xff800000) */
#define  CX_1F            0x3f800000
#define  CX_TOGGLE_FLT(x) ((x)^((int)(x) < 0 ? 0x7fffffff : 0))
#define  CX_TOGGLE_DBL(x) \
    ((x)^((int64)(x) < 0 ? CX_BIG_INT(0x7fffffffffffffff) : 0))

#define  CX_PI   3.1415926535897932384626433832795

#define  CX_NOP(a)      (a)
#define  CX_ADD(a, b)   ((a) + (b))
#define  CX_SUB(a, b)   ((a) - (b))
#define  CX_MUL(a, b)   ((a) * (b))
#define  CX_AND(a, b)   ((a) & (b))
#define  CX_OR(a, b)    ((a) | (b))
#define  CX_XOR(a, b)   ((a) ^ (b))
#define  CX_ANDN(a, b)  (~(a) & (b))
#define  CX_ORN(a, b)   (~(a) | (b))
#define  CX_SQR(a)      ((a) * (a))
#define  CX_MIN(a, b)   ((a) <= (b) ? (a) : (b))
#define  CX_MAX(a, b)   ((a) >= (b) ? (a) : (b))

#define  CX_LT(a, b)    ((a) < (b))
#define  CX_LE(a, b)    ((a) <= (b))
#define  CX_EQ(a, b)    ((a) == (b))
#define  CX_NE(a, b)    ((a) != (b))
#define  CX_GT(a, b)    ((a) > (b))
#define  CX_GE(a, b)    ((a) >= (b))

#define  CX_NONZERO(a)      ((a) != 0)
#define  CX_NONZERO_FLT(a)  (((a)+(a)) != 0)

/* general-purpose saturation macros */ 
#define  CX_CAST_8U(t)  (uchar)(!((t) & ~255) ? (t) : (t) > 0 ? 255 : 0)
#define  CX_CAST_8S(t)  (char)(!(((t)+128) & ~255) ? (t) : (t) > 0 ? 127 : -128)
#define  CX_CAST_16S(t) (short)(!(((t)+32768) & ~65535) ? (t) : (t) > 0 ? 32767 : -32768)
#define  CX_CAST_32S(t) (int)(t)
#define  CX_CAST_64S(t) (int64)(t)
#define  CX_CAST_32F(t) (float)(t)
#define  CX_CAST_64F(t) (double)(t)

#define  CX_PASTE2(a,b) a##b
#define  CX_PASTE(a,b)  CX_PASTE2(a,b)

#define  CX_EMPTY
#define  CX_MAKE_STR(a) #a

#define  CX_DEFINE_MASK         \
    float maskTab[2]; maskTab[0] = 0.f; maskTab[1] = 1.f;
#define  CX_ANDMASK( m, x )     ((x) & (((m) == 0) - 1))

/* (x) * ((m) == 1 ? 1.f : (m) == 0 ? 0.f : <ERR> */
#define  CX_MULMASK( m, x )       (maskTab[(m) != 0]*(x))

/* (x) * ((m) == -1 ? 1.f : (m) == 0 ? 0.f : <ERR> */
#define  CX_MULMASK1( m, x )      (maskTab[(m)+1]*(x))

#define  CX_ZERO_OBJ(x)  memset((x), 0, sizeof(*(x)))

#define  CX_UN_ENTRY_C1(worktype)           \
    worktype s0 = scalar[0]
    
#define  CX_UN_ENTRY_C2(worktype)           \
    worktype s0 = scalar[0], s1 = scalar[1]

#define  CX_UN_ENTRY_C3(worktype)           \
    worktype s0 = scalar[0], s1 = scalar[1], s2 = scalar[2]

#define  CX_UN_ENTRY_C4(worktype)           \
    worktype s0 = scalar[0], s1 = scalar[1], s2 = scalar[2], s3 = scalar[3]

#define  cxUnsupportedFormat "Unsupported format"

CX_INLINE void* cxAlignPtr( void* ptr, int align = 32 );
CX_INLINE void* cxAlignPtr( void* ptr, int align )
{
    return (void*)( ((size_t)ptr + align - 1) & -align );
}

CX_INLINE int cxAlign( int size, int align );
CX_INLINE int cxAlign( int size, int align )
{
    return (size + align - 1) & -align;
}

CX_INLINE  CxSize  cxGetMatSize( const CxMat* mat );
CX_INLINE  CxSize  cxGetMatSize( const CxMat* mat )
{
    CxSize size = { mat->width, mat->height };
    return size;
}

#define  CX_DESCALE(x,n)     (((x) + (1 << ((n)-1))) >> (n))
#define  CX_FLT_TO_FIX(x,n)  cxRound((x)*(1<<(n)))

/* This is tiny engine for performing fast division of multiple numbers
   by the same constant. Most compilers do it too if they know the divisor value
   at compile-time. The algorithm was taken from Agner Fog's optimization guide
   at http://www.agner.org/assem */
typedef struct CxFastDiv
{
    unsigned delta, scale, divisor;
}
CxFastDiv;

#define CX_FAST_DIV_SHIFT 32

CX_INLINE CxFastDiv cxFastDiv( int divisor );
CX_INLINE CxFastDiv cxFastDiv( int divisor )
{
    CxFastDiv fastdiv;
    
    assert( divisor >= 1 );
    uint64 temp = ((uint64)1 << CX_FAST_DIV_SHIFT)/divisor;

    fastdiv.divisor = divisor;
    fastdiv.delta = (unsigned)(((temp & 1) ^ 1) + divisor - 1);
    fastdiv.scale = (unsigned)((temp + 1) >> 1);
    
    return fastdiv;
}

#define CX_FAST_DIV( x, fastdiv )  \
    ((int)(((int64)((x)*2 + (int)(fastdiv).delta))*(int)(fastdiv).scale>>CX_FAST_DIV_SHIFT))

#define CX_FAST_UDIV( x, fastdiv )  \
    ((int)(((uint64)((x)*2 + (fastdiv).delta))*(fastdiv).scale>>CX_FAST_DIV_SHIFT))


/****************************************************************************************\
  
  Generic implementation of Quick Sort algorithm.
  ----------------------------------------------
  It allows to define qsort function for any certain type and
  any certain precedence rule.
  To use it you should define comparsion macro which takes two arguments
  and returns non-zero if first argument should precede the second and zero else.

  Example:

  Suppose that the task is to sort points by ascending of y coordinates and if
  y's are equal x's should ascend.

    The code is:
    ------------------------------------------------------------------------------
           #define cmp_pts( pt1, pt2 ) \
               ((pt1).y < (pt2).y || (pt1).y == (pt2).y && (pt1).x < (pt2).x)

           [static] IPCX_IMPLEMENT_QSORT( icxSortPoints, CxPoint, cmp_pts )
    ------------------------------------------------------------------------------

    This code generates function void icxSortPoints( CxPoint* array, int length )
    which can be used further.

    aux is an additional parameter, which can be used when comparing elements.

\****************************************************************************************/

#define CX_IMPLEMENT_QSORT_EX( func_name, T, less_than, user_data_type )\
void func_name( T* array, int length, user_data_type aux )              \
{                                                                       \
    const int bubble_level = 8;                                         \
                                                                        \
    struct                                                              \
    {                                                                   \
        int lb, ub;                                                     \
    }                                                                   \
    stack[48];                                                          \
                                                                        \
    int sp = 0;                                                         \
                                                                        \
    T   temp;                                                           \
    T   lb_val;                                                         \
                                                                        \
    aux = aux;                                                          \
                                                                        \
    stack[0].lb = 0;                                                    \
    stack[0].ub = length - 1;                                           \
                                                                        \
    while( sp >= 0 )                                                    \
    {                                                                   \
        int lb = stack[sp].lb;                                          \
        int ub = stack[sp--].ub;                                        \
                                                                        \
        for(;;)                                                         \
        {                                                               \
            int diff = ub - lb;                                         \
            if( diff < bubble_level )                                   \
            {                                                           \
                int i, j;                                               \
                T* arr = array + lb;                                    \
                                                                        \
                for( i = diff; i > 0; i-- )                             \
                {                                                       \
                    int f = 0;                                          \
                    for( j = 0; j < i; j++ )                            \
                        if( less_than( arr[j+1], arr[j] ))              \
                        {                                               \
                            temp = arr[j];                              \
                            arr[j] = arr[j+1];                          \
                            arr[j+1] = temp;                            \
                            f = 1;                                      \
                        }                                               \
                    if( !f ) break;                                     \
                }                                                       \
                break;                                                  \
            }                                                           \
            else                                                        \
            {                                                           \
                /* select pivot and exchange with 1st element */        \
                int  m = lb + (diff >> 1);                              \
                int  i = lb + 1, j = ub;                                \
                                                                        \
                lb_val = array[m];                                      \
                                                                        \
                array[m]  = array[lb];                                  \
                array[lb] = lb_val;                                     \
                                                                        \
                /* partition into two segments */                       \
                for(;;)                                                 \
                {                                                       \
                    for( ;i < j && less_than(array[i], lb_val); i++ );  \
                    for( ;j >= i && less_than(lb_val, array[j]); j-- ); \
                                                                        \
                    if( i >= j ) break;                                 \
                    temp = array[i];                                    \
                    array[i++] = array[j];                              \
                    array[j--] = temp;                                  \
                }                                                       \
                                                                        \
                /* pivot belongs in A[j] */                             \
                array[lb] = array[j];                                   \
                array[j]  = lb_val;                                     \
                                                                        \
                /* keep processing smallest segment, and stack largest*/\
                if( j - lb <= ub - j )                                  \
                {                                                       \
                    if( j + 1 < ub )                                    \
                    {                                                   \
                        stack[++sp].lb   = j + 1;                       \
                        stack[sp].ub = ub;                              \
                    }                                                   \
                    ub = j - 1;                                         \
                }                                                       \
                else                                                    \
                {                                                       \
                    if( j - 1 > lb)                                     \
                    {                                                   \
                        stack[++sp].lb = lb;                            \
                        stack[sp].ub = j - 1;                           \
                    }                                                   \
                    lb = j + 1;                                         \
                }                                                       \
            }                                                           \
        }                                                               \
    }                                                                   \
}

#define CX_IMPLEMENT_QSORT( func_name, T, less_than )  \
    CX_IMPLEMENT_QSORT_EX( func_name, T, less_than, int )

/****************************************************************************************\
*                     Structures and macros for integration with IPP                     *
\****************************************************************************************/

#define CX_IPP_NONE  0
#define CX_IPP_OPTCX 1
#define CX_IPP_IPPCX 2
#define CX_IPP_IPPI  3
#define CX_IPP_IPPS  4
#define CX_IPP_IPPVM 5
#define CX_IPP_IPPM  6

#define CX_IPP_MAX   8

#define CX_IPPLIBS1(lib1) ((lib1)&7)
#define CX_IPPLIBS2(lib1,lib2) (((lib1)&7)|(((lib2)&7)<<3))
#define CX_IPPLIBS3(lib1,lib2,lib3) (((lib1)&7)|(((lib2)&7)<<3)|(((lib2)&7)<<6))

#ifndef IPCXAPI
#define IPCXAPI(type,name,args)                               \
    CX_EXTERN_C type CX_STDCALL name##_f args;                \
    /* function pointer */                                    \
    typedef type (CX_STDCALL* name##_t) args;                 \
    extern name##_t name;
#endif

#define IPCXAPI_EX(type,name,ipp_name,ipp_search_modules,args)\
    IPCXAPI(type,name,args) 

#ifndef IPCXAPI_IMPL
#define IPCXAPI_IMPL(type,name,arg)                           \
    /*typedef type (CX_STDCALL* name##_t) arg;*/              \
    CX_EXTERN_C type CX_STDCALL name##_f arg;                 \
    name##_t name = name##_f;                                 \
    CX_EXTERN_C type CX_STDCALL name##_f arg
#endif

/* IPP types' enumeration */
typedef enum CxDataType {
   cx1u,
   cx8u, cx8s,
   cx16u, cx16s, cx16sc,
   cx32u, cx32s, cx32sc,
   cx32f, cx32fc,
   cx64u, cx64s, cx64sc,
   cx64f, cx64fc
} CxDataType;

typedef enum CxHintAlgorithm {
   cxAlgHintNone,
   cxAlgHintFast,
   cxAlgHintAccurate
} CxHintAlgorithm;

typedef struct CxFuncTable
{
    void*   fn_2d[CX_DEPTH_MAX + 1];
}
CxFuncTable;

typedef struct CxBigFuncTable
{
    void*   fn_2d[CX_MAT_TYPE_MASK+1];
}
CxBigFuncTable;


typedef struct CxBtFuncTable
{
    void*   fn_2d[33];
}
CxBtFuncTable;

typedef CxStatus (CX_STDCALL *CxFunc2D_1A)(void* arr, int step, CxSize size);

typedef CxStatus (CX_STDCALL *CxFunc2D_1A1P)(void* arr, int step, CxSize size, void* param);

typedef CxStatus (CX_STDCALL *CxFunc2D_1A1P1I)(void* arr, int step, CxSize size,
                                               void* param, int flag);

typedef CxStatus (CX_STDCALL *CxFunc2DnC_1A1P)( void* arr, int step, CxSize size,
                                                int cn, int coi, void* param );

typedef CxStatus (CX_STDCALL *CxFunc2DnC_1A1P)( void* arr, int step, CxSize size,
                                                int cn, int coi, void* param );

typedef CxStatus (CX_STDCALL *CxFunc2D_1A2P)( void* arr, int step, CxSize size,
                                              void* param1, void* param2 );

typedef CxStatus (CX_STDCALL *CxFunc2DnC_1A2P)( void* arr, int step,
                                                CxSize size, int cn, int coi,
                                                void* param1, void* param2 );

typedef CxStatus (CX_STDCALL *CxFunc2D_1A4P)( void* arr, int step, CxSize size,
                                              void* param1, void* param2,
                                              void* param3, void* param4 );

typedef CxStatus (CX_STDCALL *CxFunc2DnC_1A4P)( void* arr, int step,
                                                CxSize size, int cn, int coi,
                                                void* param1, void* param2,
                                                void* param3, void* param4 );

typedef CxStatus (CX_STDCALL *CxFunc2D_2A)( void* arr0, int step0,
                                            void* arr1, int step1, CxSize size );

typedef CxStatus (CX_STDCALL *CxFunc2D_2A1P)( void* arr0, int step0,
                                              void* arr1, int step1,
                                              CxSize size, void* param );

typedef CxStatus (CX_STDCALL *CxFunc2DnC_2A1P)( void* arr0, int step0,
                                                void* arr1, int step1,
                                                CxSize size, int cn,
                                                int coi, void* param );

typedef CxStatus (CX_STDCALL *CxFunc2DnC_2A1P)( void* arr0, int step0,
                                                void* arr1, int step1,
                                                CxSize size, int cn,
                                                int coi, void* param );

typedef CxStatus (CX_STDCALL *CxFunc2D_2A2P)( void* arr0, int step0,
                                              void* arr1, int step1, CxSize size,
                                              void* param1, void* param2 );

typedef CxStatus (CX_STDCALL *CxFunc2DnC_2A2P)( void* arr0, int step0,
                                                void* arr1, int step1,
                                                CxSize size, int cn, int coi,
                                                void* param1, void* param2 );

typedef CxStatus (CX_STDCALL *CxFunc2D_2A1P1I)( void* arr0, int step0,
                                                void* arr1, int step1, CxSize size,
                                                void* param, int flag );

typedef CxStatus (CX_STDCALL *CxFunc2D_2A4P)( void* arr0, int step0,
                                              void* arr1, int step1, CxSize size,
                                              void* param1, void* param2,
                                              void* param3, void* param4 );

typedef CxStatus (CX_STDCALL *CxFunc2DnC_2A4P)( void* arr0, int step0,
                                                void* arr1, int step1, CxSize size,
                                                int cn, int coi,
                                                void* param1, void* param2,
                                                void* param3, void* param4 );

typedef CxStatus (CX_STDCALL *CxFunc2D_3A)( void* arr0, int step0,
                                            void* arr1, int step1,
                                            void* arr2, int step2, CxSize size );

typedef CxStatus (CX_STDCALL *CxFunc2D_3A1P)( void* arr0, int step0,
                                              void* arr1, int step1,
                                              void* arr2, int step2,
                                              CxSize size, void* param );

typedef CxStatus (CX_STDCALL *CxFunc2D_3A1I)( void* arr0, int step0,
                                              void* arr1, int step1,
                                              void* arr2, int step2,
                                              CxSize size, int flag );

typedef CxStatus (CX_STDCALL *CxFunc2DnC_3A1P)( void* arr0, int step0,
                                                void* arr1, int step1,
                                                void* arr2, int step2,
                                                CxSize size, int cn,
                                                int coi, void* param );

typedef CxStatus (CX_STDCALL *CxFunc2D_4A)( void* arr0, int step0,
                                            void* arr1, int step1,
                                            void* arr2, int step2,
                                            void* arr3, int step3,
                                            CxSize size );

typedef CxStatus (CX_STDCALL *CxFunc0D)( const void* src, void* dst, int param );

#define CX_DEF_INIT_FUNC_TAB_2D( FUNCNAME, FLAG )                   \
static void  icxInit##FUNCNAME##FLAG##Table( CxFuncTable* tab )     \
{                                                                   \
    assert( tab );                                                  \
                                                                    \
    tab->fn_2d[CX_8U]  = (void*)icx##FUNCNAME##_8u_##FLAG;          \
    tab->fn_2d[CX_8S]  = (void*)icx##FUNCNAME##_8s_##FLAG;          \
    tab->fn_2d[CX_16S] = (void*)icx##FUNCNAME##_16s_##FLAG;         \
    tab->fn_2d[CX_32S] = (void*)icx##FUNCNAME##_32s_##FLAG;         \
    tab->fn_2d[CX_32F] = (void*)icx##FUNCNAME##_32f_##FLAG;         \
    tab->fn_2d[CX_64F] = (void*)icx##FUNCNAME##_64f_##FLAG;         \
}


#define CX_DEF_INIT_BIG_FUNC_TAB_2D( FUNCNAME, FLAG )               \
static void  icxInit##FUNCNAME##FLAG##Table( CxBigFuncTable* tab )  \
{                                                                   \
    assert( tab );                                                  \
                                                                    \
    tab->fn_2d[CX_8UC1]  = (void*)icx##FUNCNAME##_8u_C1##FLAG;      \
    tab->fn_2d[CX_8UC2]  = (void*)icx##FUNCNAME##_8u_C2##FLAG;      \
    tab->fn_2d[CX_8UC3]  = (void*)icx##FUNCNAME##_8u_C3##FLAG;      \
    tab->fn_2d[CX_8UC4]  = (void*)icx##FUNCNAME##_8u_C4##FLAG;      \
                                                                    \
    tab->fn_2d[CX_8SC1]  = (void*)icx##FUNCNAME##_8s_C1##FLAG;      \
    tab->fn_2d[CX_8SC2]  = (void*)icx##FUNCNAME##_8s_C2##FLAG;      \
    tab->fn_2d[CX_8SC3]  = (void*)icx##FUNCNAME##_8s_C3##FLAG;      \
    tab->fn_2d[CX_8SC4]  = (void*)icx##FUNCNAME##_8s_C4##FLAG;      \
                                                                    \
    tab->fn_2d[CX_16SC1] = (void*)icx##FUNCNAME##_16s_C1##FLAG;     \
    tab->fn_2d[CX_16SC2] = (void*)icx##FUNCNAME##_16s_C2##FLAG;     \
    tab->fn_2d[CX_16SC3] = (void*)icx##FUNCNAME##_16s_C3##FLAG;     \
    tab->fn_2d[CX_16SC4] = (void*)icx##FUNCNAME##_16s_C4##FLAG;     \
                                                                    \
    tab->fn_2d[CX_32SC1] = (void*)icx##FUNCNAME##_32s_C1##FLAG;     \
    tab->fn_2d[CX_32SC2] = (void*)icx##FUNCNAME##_32s_C2##FLAG;     \
    tab->fn_2d[CX_32SC3] = (void*)icx##FUNCNAME##_32s_C3##FLAG;     \
    tab->fn_2d[CX_32SC4] = (void*)icx##FUNCNAME##_32s_C4##FLAG;     \
                                                                    \
    tab->fn_2d[CX_32FC1] = (void*)icx##FUNCNAME##_32f_C1##FLAG;     \
    tab->fn_2d[CX_32FC2] = (void*)icx##FUNCNAME##_32f_C2##FLAG;     \
    tab->fn_2d[CX_32FC3] = (void*)icx##FUNCNAME##_32f_C3##FLAG;     \
    tab->fn_2d[CX_32FC4] = (void*)icx##FUNCNAME##_32f_C4##FLAG;     \
                                                                    \
    tab->fn_2d[CX_64FC1] = (void*)icx##FUNCNAME##_64f_C1##FLAG;     \
    tab->fn_2d[CX_64FC2] = (void*)icx##FUNCNAME##_64f_C2##FLAG;     \
    tab->fn_2d[CX_64FC3] = (void*)icx##FUNCNAME##_64f_C3##FLAG;     \
    tab->fn_2d[CX_64FC4] = (void*)icx##FUNCNAME##_64f_C4##FLAG;     \
}

#define CX_DEF_INIT_FUNC_TAB_0D( FUNCNAME )                         \
static void  icxInit##FUNCNAME##Table( CxFuncTable* tab )           \
{                                                                   \
    tab->fn_2d[CX_8U]  = (void*)icx##FUNCNAME##_8u;                 \
    tab->fn_2d[CX_8S]  = (void*)icx##FUNCNAME##_8s;                 \
    tab->fn_2d[CX_16S] = (void*)icx##FUNCNAME##_16s;                \
    tab->fn_2d[CX_32S] = (void*)icx##FUNCNAME##_32s;                \
    tab->fn_2d[CX_32F] = (void*)icx##FUNCNAME##_32f;                \
    tab->fn_2d[CX_64F] = (void*)icx##FUNCNAME##_64f;                \
}

#define CX_DEF_INIT_FUNC_TAB_1D  CX_DEF_INIT_FUNC_TAB_0D


#define CX_DEF_INIT_PIXSIZE_TAB_2D( FUNCNAME, FLAG )                \
static void icxInit##FUNCNAME##FLAG##Table( CxBtFuncTable* table )  \
{                                                                   \
    table->fn_2d[1]  = (void*)icx##FUNCNAME##_8u_C1##FLAG;          \
    table->fn_2d[2]  = (void*)icx##FUNCNAME##_8u_C2##FLAG;          \
    table->fn_2d[3]  = (void*)icx##FUNCNAME##_8u_C3##FLAG;          \
    table->fn_2d[4]  = (void*)icx##FUNCNAME##_16u_C2##FLAG;         \
    table->fn_2d[6]  = (void*)icx##FUNCNAME##_16u_C3##FLAG;         \
    table->fn_2d[8]  = (void*)icx##FUNCNAME##_32s_C2##FLAG;         \
    table->fn_2d[12] = (void*)icx##FUNCNAME##_32s_C3##FLAG;         \
    table->fn_2d[16] = (void*)icx##FUNCNAME##_64s_C2##FLAG;         \
    table->fn_2d[24] = (void*)icx##FUNCNAME##_64s_C3##FLAG;         \
    table->fn_2d[32] = (void*)icx##FUNCNAME##_64s_C4##FLAG;         \
}

#define  CX_GET_FUNC_PTR( func, table_entry )  \
    func = (table_entry);                      \
                                               \
    if( !func )                                \
        CX_ERROR( CX_StsUnsupportedFormat, "" )


#endif /*_CXCORE_MISC_H_*/
