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

#include "_cxcore.h"

/****************************************************************************************\
*                                         cxGEMM                                         *
\****************************************************************************************/

#define ICX_DEF_GEMM_FUNC( flavor, arrtype, worktype )                                  \
static CxStatus CX_STDCALL                                                              \
icxGEMM_##flavor( const arrtype* a_data, int a_step, const arrtype* b_data, int b_step, \
                 const arrtype* c_data, int c_step, arrtype* d_data, int d_step,        \
                 CxSize a_size, CxSize d_size, worktype alpha,                          \
                 worktype beta, int alg, int tABC )                                     \
{                                                                                       \
    int i, j, k, n = a_size.width, m = d_size.width;                                    \
    const arrtype *_a_data = a_data, *_b_data = b_data, *_c_data = c_data;              \
    arrtype* a_buf = 0;                                                                 \
    int a_step0, a_step1, c_step0, c_step1;                                             \
                                                                                        \
    a_step /= sizeof(a_data[0]);                                                        \
    b_step /= sizeof(b_data[0]);                                                        \
    c_step /= sizeof(c_data[0]);                                                        \
    d_step /= sizeof(d_data[0]);                                                        \
                                                                                        \
    a_step0 = a_step, a_step1 = 1;                                                      \
    c_step0 = c_step, c_step1 = c_data != 0;                                            \
                                                                                        \
    if( tABC & CX_GEMM_C_T )                                                            \
        CX_SWAP( c_step0, c_step1, n );                                                 \
                                                                                        \
    if( alg <= 1 && (tABC & 1) )                                                        \
    {                                                                                   \
        CX_SWAP( a_step0, a_step1, n );                                                 \
        n = a_size.height;                                                              \
        a_buf = (arrtype*)alloca(n*sizeof(a_data[0]));                                  \
    }                                                                                   \
                                                                                        \
    switch( alg )                                                                       \
    {                                                                                   \
    case 0:                                                                             \
        /* second operand is transposed */                                              \
        for( i = 0; i < d_size.height; i++, _a_data += a_step0,                         \
                                            _c_data += c_step0,                         \
                                            d_data += d_step )                          \
        {                                                                               \
            a_data = _a_data;                                                           \
            b_data = _b_data;                                                           \
            c_data = _c_data;                                                           \
                                                                                        \
            if( a_buf )                                                                 \
            {                                                                           \
                for( k = 0; k < n; k++ )                                                \
                    a_buf[k] = a_data[a_step1*k];                                       \
                a_data = a_buf;                                                         \
            }                                                                           \
                                                                                        \
            for( j = 0; j < d_size.width; j++, b_data += b_step,                        \
                                               c_data += c_step1 )                      \
            {                                                                           \
                worktype s = 0;                                                         \
                for( k = 0; k <= n - 4; k += 4 )                                        \
                    s += (worktype)a_data[k]*b_data[k] + (worktype)a_data[k+1]*b_data[k+1] +   \
                         (worktype)a_data[k+2]*b_data[k+2] + (worktype)a_data[k+3]*b_data[k+3];\
                for( ; k < n; k++ )                                                     \
                    s += (worktype)a_data[k]*b_data[k];                                 \
                if( !c_data )                                                           \
                    d_data[j] = (arrtype)(s*alpha);                                     \
                else                                                                    \
                    d_data[j] = (arrtype)(s*alpha + *c_data*beta);                      \
            }                                                                           \
        }                                                                               \
        break;                                                                          \
    case 1: /* second operand is not transposed */                                      \
        if( d_size.width*sizeof(a_data[0]) <= 1600 )                                    \
        {                                                                               \
            for( i = 0; i < d_size.height; i++, _a_data += a_step0,                     \
                                                _c_data += c_step0,                     \
                                                d_data += d_step )                      \
            {                                                                           \
                a_data = _a_data, b_data = _b_data, c_data = _c_data;                   \
                                                                                        \
                if( a_buf )                                                             \
                {                                                                       \
                    for( k = 0; k < n; k++ )                                            \
                        a_buf[k] = a_data[a_step1*k];                                   \
                    a_data = a_buf;                                                     \
                }                                                                       \
                                                                                        \
                for( j = 0; j <= m - 4; j += 4, c_data += 4*c_step1 )                   \
                {                                                                       \
                    worktype s0 = 0, s1 = 0, s2 = 0, s3 = 0;                            \
                    const arrtype* b = b_data + j;                                      \
                                                                                        \
                    for( k = 0; k < n; k++, b += b_step )                               \
                    {                                                                   \
                        worktype a = a_data[k];                                         \
                        s0 += a * b[0];                                                 \
                        s1 += a * b[1];                                                 \
                        s2 += a * b[2];                                                 \
                        s3 += a * b[3];                                                 \
                    }                                                                   \
                                                                                        \
                    if( !c_data )                                                       \
                    {                                                                   \
                        d_data[j] = (arrtype)(s0*alpha);                                \
                        d_data[j+1] = (arrtype)(s1*alpha);                              \
                        d_data[j+2] = (arrtype)(s2*alpha);                              \
                        d_data[j+3] = (arrtype)(s3*alpha);                              \
                    }                                                                   \
                    else                                                                \
                    {                                                                   \
                        d_data[j] = (arrtype)(s0*alpha + c_data[0]*beta);               \
                        d_data[j+1] = (arrtype)(s1*alpha + c_data[c_step1]*beta);       \
                        d_data[j+2] = (arrtype)(s2*alpha + c_data[c_step1*2]*beta);     \
                        d_data[j+3] = (arrtype)(s3*alpha + c_data[c_step1*3]*beta);     \
                    }                                                                   \
                }                                                                       \
                                                                                        \
                for( ; j < m; j++, c_data += c_step1 )                                  \
                {                                                                       \
                    worktype s0 = 0;                                                    \
                    const arrtype* b = b_data + j;                                      \
                                                                                        \
                    for( k = 0; k < n; k++, b += b_step )                               \
                        s0 += a_data[k] * b[0];                                         \
                                                                                        \
                    if( !c_data )                                                       \
                        d_data[j] = (arrtype)(s0*alpha);                                \
                    else                                                                \
                        d_data[j] = (arrtype)(s0*alpha + c_data[0]*beta);               \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            int m = d_size.width;                                                       \
            worktype* d_buf = (worktype*)alloca( m*sizeof(d_buf[0]));                   \
                                                                                        \
            for( i = 0; i < d_size.height; i++, _a_data += a_step0,                     \
                                                _c_data += c_step0,                     \
                                                d_data += d_step )                      \
            {                                                                           \
                a_data = _a_data, b_data = _b_data, c_data = _c_data;                   \
                worktype al;                                                            \
                                                                                        \
                if( a_buf )                                                             \
                {                                                                       \
                    for( k = 0; k < n; k++ )                                            \
                        a_buf[k] = a_data[a_step1*k];                                   \
                    a_data = a_buf;                                                     \
                }                                                                       \
                                                                                        \
                for( k = 0, al = a_data[0]; k < m; k++ )                                \
                    d_buf[k] = b_data[k]*al;                                            \
                                                                                        \
                for( j = 1; j < n - 1; j++ )                                            \
                {                                                                       \
                    b_data += b_step;                                                   \
                    al = a_data[j];                                                     \
                                                                                        \
                    for( k = 0; k <= m - 4; k += 4 )                                    \
                    {                                                                   \
                        worktype t0 = d_buf[k] + b_data[k]*al;                          \
                        worktype t1 = d_buf[k+1] + b_data[k+1]*al;                      \
                        d_buf[k] = t0;                                                  \
                        d_buf[k+1] = t1;                                                \
                        t0 = d_buf[k+2] + b_data[k+2]*al;                               \
                        t1 = d_buf[k+3] + b_data[k+3]*al;                               \
                        d_buf[k+2] = t0;                                                \
                        d_buf[k+3] = t1;                                                \
                    }                                                                   \
                                                                                        \
                    for( ; k < m; k++ )                                                 \
                        d_buf[k] += b_data[k]*al;                                       \
                }                                                                       \
                                                                                        \
                b_data += b_step;                                                       \
                al = a_data[n-1];                                                       \
                                                                                        \
                if( !c_data )                                                           \
                    for( k = 0; k < m; k++ )                                            \
                        d_data[k] = (arrtype)((d_buf[k] + b_data[k]*al)*alpha);         \
                else                                                                    \
                    for( k = 0; k < m; k++, c_data += c_step1 )                         \
                        d_data[k] = (arrtype)((d_buf[k]+b_data[k]*al)*alpha+c_data[0]*beta);\
            }                                                                           \
        }                                                                               \
        break;                                                                          \
    case 2:                                                                             \
        /* outer-product */                                                             \
        assert( a_size.width == 1 || a_size.height == 1 );                              \
        for( i = 0; i < d_size.height; i++, _c_data += c_step0, d_data += d_step )      \
        {                                                                               \
            c_data = _c_data;                                                           \
            if( !c_data )                                                               \
                for( j = 0; j < m; j++ )                                                \
                    d_data[j] = (arrtype)((alpha*a_data[i])*b_data[j]);                 \
            else                                                                        \
                for( j = 0; j < m; j++, c_data += c_step1 )                             \
                    d_data[j] = (arrtype)((alpha*a_data[i])*b_data[j] + beta*c_data[0]);\
        }                                                                               \
        break;                                                                          \
    default:                                                                            \
        assert(0);                                                                      \
    }                                                                                   \
                                                                                        \
    return CX_OK;                                                                       \
}

ICX_DEF_GEMM_FUNC( 32f_C1R, float, double)
ICX_DEF_GEMM_FUNC( 64f_C1R, double, double)
ICX_DEF_GEMM_FUNC( 32f_C2R, CxComplex32f, CxComplex64f )
ICX_DEF_GEMM_FUNC( 64f_C2R, CxComplex64f, CxComplex64f )

typedef CxStatus (CX_STDCALL *CxGEMMFunc)( const void* src1, int step1,
                   const void* src2, int step2, const void* src3, int step3,
                   void* dst, int dststep, CxSize srcsize, CxSize dstsize,
                   double alpha, double beta, int alg, int tABC );

static void icxInitGEMMTable( CxBigFuncTable* table )
{
    table->fn_2d[CX_32FC1] = (void*)icxGEMM_32f_C1R;
    table->fn_2d[CX_64FC1] = (void*)icxGEMM_64f_C1R;
    table->fn_2d[CX_32FC2] = (void*)icxGEMM_32f_C2R;
    table->fn_2d[CX_64FC2] = (void*)icxGEMM_64f_C2R;
}

CX_IMPL void
cxGEMM( const CxArr* src1arr, const CxArr* src2arr, double alpha,
        const CxArr* src3arr, double beta, CxArr* dstarr, int tABC )
{
    static CxBigFuncTable gemm_tab;
    static int inittab = 0;
    
    uchar* buffer = 0;
    int local_alloc = 0;
    
    CX_FUNCNAME( "cxGEMM" );

    __BEGIN__;

    CxMat stub1, *src1 = (CxMat*)src1arr;
    CxMat stub2, *src2 = (CxMat*)src2arr;
    CxMat stub3, *src3 = (CxMat*)src3arr;
    CxMat stub, *dst = (CxMat*)dstarr;
    int type, alg = 0;

    if( !CX_IS_MAT( src1 ))
    {
        int coi = 0;
        CX_CALL( src1 = cxGetMat( src1, &stub1, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT( src2 ))
    {
        int coi = 0;
        CX_CALL( src2 = cxGetMat( src2, &stub2, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT( dst ))
    {
        int coi = 0;
        CX_CALL( dst = cxGetMat( dst, &stub, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( tABC & ~(CX_GEMM_A_T|CX_GEMM_B_T|CX_GEMM_C_T))
        CX_ERROR( CX_StsBadFlag, "" );

    if( src3 )
    {
        if( !CX_IS_MAT( src3 ))
        {
            int coi = 0;
            CX_CALL( src3 = cxGetMat( src3, &stub3, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }

        if( !CX_ARE_TYPES_EQ( src3, dst ))
            CX_ERROR( CX_StsUnmatchedFormats, "" );

        if( (tABC&4) == 0 && (src3->cols != dst->cols || src3->rows != dst->rows) ||
            (tABC&4) != 0 && (src3->rows != dst->cols || src3->cols != dst->rows))
            CX_ERROR( CX_StsUnmatchedSizes, "" );
    }
    else
    {
        src3 = &stub3;
        src3->data.ptr = 0;
        src3->step = 0;
        src3->type = CX_MAT_CONT_FLAG;
    }

    if( !CX_ARE_TYPES_EQ( src1, src2 ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( !CX_ARE_TYPES_EQ( src1, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    switch( tABC & 3 )
    {
    case 0:
        if( src1->cols != src2->rows ||
            src1->rows != dst->rows ||
            src2->cols != dst->cols )
            CX_ERROR( CX_StsUnmatchedSizes, "" );
        // A*B
        alg = src1->cols == 1 && CX_IS_MAT_CONT(src1->type & src2->type) ? 2 :
              src2->cols == 1 && CX_IS_MAT_CONT(src2->type) ? 0 : 1;
        break;
    case 1:
        if( src1->rows != src2->rows ||
            src1->cols != dst->rows ||
            src2->cols != dst->cols )
            CX_ERROR( CX_StsUnmatchedSizes, "" );
        // At*B
        alg = src1->rows == 1 && CX_IS_MAT_CONT(src1->type & src2->type) ? 2 :
              src2->cols == 1 && CX_IS_MAT_CONT(src2->type) ? 0 : 1;
        break;
    case 2:
        if( src1->cols != src2->cols ||
            src1->rows != dst->rows ||
            src2->rows != dst->cols )
            CX_ERROR( CX_StsUnmatchedSizes, "" );
        // A*Bt
        alg = src1->cols == 1 && CX_IS_MAT_CONT(src1->type & src2->type) ? 2 : 0;
        break;
    case 3:
        {
        if( src1->rows != src2->cols ||
            src1->cols != dst->rows ||
            src2->rows != dst->cols )
            CX_ERROR( CX_StsUnmatchedSizes, "" );
        // (At*Bt) = (B*A)t = (A'*B')t, where A'=B and B'=A
        alg = src1->rows == 1 && CX_IS_MAT_CONT(src1->type & src2->type) ? 2 : 0;
        }
        break;
    }
    
    type = CX_MAT_TYPE( src1->type );

    // general case
    {
        CxGEMMFunc func;
        CxSize size = cxGetMatSize( src1 );
        CxSize dstsize = cxGetMatSize( dst );
        int buf_size = dstsize.width*dstsize.height*CX_ELEM_SIZE(type);
        CxMat tmat, *tdst = dst;
        
        if( !inittab )
        {
            icxInitGEMMTable( &gemm_tab );
            inittab = 1;
        }

        if( dst->data.ptr == src1->data.ptr || dst->data.ptr == src2->data.ptr ||
            (tABC & 4) != 0 && src3 != 0 && dst->data.ptr == src3->data.ptr )
        {
            if( size.width <= CX_MAX_LOCAL_MAT_SIZE )
            {
                buffer = (uchar*)alloca( buf_size + 8 );
                buffer = (uchar*)cxAlignPtr( buffer, 8 );
                local_alloc = 1;
            }
            else
                CX_CALL( buffer = (uchar*)cxAlloc( buf_size ));

            CX_CALL( cxInitMatHeader( &tmat, dstsize.height,
                                      dstsize.width, type, buffer ));
            tdst = &tmat;
        }

        func = (CxGEMMFunc)(gemm_tab.fn_2d[CX_MAT_TYPE(type)]);
        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src1->data.ptr, src1->step, src2->data.ptr, src2->step,
                         src3->data.ptr, src3->step, tdst->data.ptr, tdst->step,
                         size, dstsize, alpha, beta, alg, tABC ));
        if( tdst != dst )
            CX_CALL( cxCopy( tdst, dst ));
    }

    CX_CHECK_NANS( dst );

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );
}

/****************************************************************************************\
*                                        cxMatMulAdd                                     *
\****************************************************************************************/

#define _load_real_( temp, ptr, k )   \
    ((temp)[0] = (ptr) ? (ptr)[k] : 0)

#define _load_complex_( temp, ptr, k )\
    ((temp)[0] = (ptr) ? (ptr)[k] : 0, (temp)[1] = (ptr) ? (ptr)[(k)+1] : 0)

#define _mul_add_real_( a, b, s ) \
    ((s)[0] += (a)[0]*(b)[0])

#define _mul_add_complex_( a, b, s )            \
    ((s)[0] += (a)[0]*(b)[0] - (a)[1]*(b)[1],   \
    (s)[1] += (a)[0]*(b)[1] + (a)[1]*(b)[0])

#define _store_real_( temp, ptr, arrtype ) \
    ((ptr)[0] = (arrtype)(temp)[0])

#define _store_complex_( temp, ptr, arrtype ) \
    ((ptr)[0] = (arrtype)(temp)[0], (ptr)[1] = (arrtype)(temp)[1])

CX_IMPL  void
cxMatMulAdd( const CxArr* src1arr, const CxArr* src2arr,
             const CxArr* src3arr, CxArr* dstarr )
{
    static CxBigFuncTable gemm_tab;
    static int inittab = 0;
    
    uchar* buffer = 0;
    int local_alloc = 0;
    
    CX_FUNCNAME( "cxMatMulAdd" );

    __BEGIN__;

    CxMat stub1, *src1 = (CxMat*)src1arr;
    CxMat stub2, *src2 = (CxMat*)src2arr;
    CxMat stub3, *src3 = (CxMat*)src3arr;
    CxMat stub, *dst = (CxMat*)dstarr;
    int type;

    if( !CX_IS_MAT( src1 ))
    {
        int coi = 0;
        CX_CALL( src1 = cxGetMat( src1, &stub1, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT( src2 ))
    {
        int coi = 0;
        CX_CALL( src2 = cxGetMat( src2, &stub2, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT( dst ))
    {
        int coi = 0;
        CX_CALL( dst = cxGetMat( dst, &stub, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( src3 )
    {
        if( !CX_IS_MAT( src3 ))
        {
            int coi = 0;
            CX_CALL( src3 = cxGetMat( src3, &stub3, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }

        if( !CX_ARE_TYPES_EQ( src3, dst ))
            CX_ERROR( CX_StsUnmatchedFormats, "" );

        if( !CX_ARE_SIZES_EQ( src3, dst ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );
    }
    else
    {
        src3 = &stub3;
        src3->data.ptr = 0;
        src3->step = 0;
        src3->type = CX_MAT_CONT_FLAG;
    }

    if( !CX_ARE_TYPES_EQ( src1, src2 ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( !CX_ARE_TYPES_EQ( src1, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( src1->cols != src2->rows ||
        src1->rows != dst->rows ||
        src2->cols != dst->cols )
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    type = CX_MAT_TYPE( src1->type );

    // check case of a single equation and small matrix
    if( src2->cols == 1 && src1->cols <= 3 && src1->cols == src1->rows &&
        CX_IS_MAT_CONT( src1->type & src2->type & src3->type & dst->type ))
    {
        uchar* data1 = src1->data.ptr;
        uchar* data2 = src2->data.ptr;
        uchar* data3 = src3->data.ptr;
        uchar* datad = dst->data.ptr;

        #undef A
        #undef X
        #undef B
        #undef Y
        #undef N
        #undef arrtype
        #define A(y,x)  ((arrtype*)data1)[(x)+(y)*N]
        #define X(y)    ((arrtype*)data2)[y]
        #define B(y)    ((arrtype*)data3)[y]
        #define Y(y)    ((arrtype*)datad)[y]

        if( type == CX_32FC1 )
        {
            if( src1->cols == 2 )
            {
                #undef N
                #define N  2

                #undef arrtype                
                #define arrtype float

                float t[2];

                if( data3 )
                {
                    t[0] = (float)(A(0,0)*X(0) + A(0,1)*X(1) + B(0));
                    t[1] = (float)(A(1,0)*X(0) + A(1,1)*X(1) + B(1));
                }
                else
                {
                    t[0] = (float)(A(0,0)*X(0) + A(0,1)*X(1));
                    t[1] = (float)(A(1,0)*X(0) + A(1,1)*X(1));
                }

                Y(0) = t[0];
                Y(1) = t[1];
            }
            else if( src1->cols == 3 )
            {
                #undef N
                #define N  3

                float t[3];

                if( data3 )
                {
                    t[0] = (float)(A(0,0)*X(0) + A(0,1)*X(1) + A(0,2)*X(2) + B(0));
                    t[1] = (float)(A(1,0)*X(0) + A(1,1)*X(1) + A(1,2)*X(2) + B(1));
                    t[2] = (float)(A(2,0)*X(0) + A(2,1)*X(1) + A(2,2)*X(2) + B(2));
                }
                else
                {
                    t[0] = (float)(A(0,0)*X(0) + A(0,1)*X(1) + A(0,2)*X(2));
                    t[1] = (float)(A(1,0)*X(0) + A(1,1)*X(1) + A(1,2)*X(2));
                    t[2] = (float)(A(2,0)*X(0) + A(2,1)*X(1) + A(2,2)*X(2));
                }

                Y(0) = t[0];
                Y(1) = t[1];
                Y(2) = t[2];
            }
            else
            {
                Y(0) = data3 ? (float)(A(0,0)*X(0) + B(0)) : (float)(A(0,0)*X(0));
            }
            EXIT;
        }
        
        if( type == CX_64FC1 )
        {
            if( src1->cols == 2 )
            {
                #undef N
                #define N  2

                #undef arrtype                
                #define arrtype double

                double t[2];

                if( data3 )
                {
                    t[0] = A(0,0)*X(0) + A(0,1)*X(1) + B(0);
                    t[1] = A(1,0)*X(0) + A(1,1)*X(1) + B(1);
                }
                else
                {
                    t[0] = A(0,0)*X(0) + A(0,1)*X(1);
                    t[1] = A(1,0)*X(0) + A(1,1)*X(1);
                }

                Y(0) = t[0];
                Y(1) = t[1];
            }
            else if( src1->cols == 3 )
            {
                #undef N
                #define N  3

                double t[3];

                if( data3 )
                {
                    t[0] = A(0,0)*X(0) + A(0,1)*X(1) + A(0,2)*X(2) + B(0);
                    t[1] = A(1,0)*X(0) + A(1,1)*X(1) + A(1,2)*X(2) + B(1);
                    t[2] = A(2,0)*X(0) + A(2,1)*X(1) + A(2,2)*X(2) + B(2);
                }
                else
                {
                    t[0] = A(0,0)*X(0) + A(0,1)*X(1) + A(0,2)*X(2);
                    t[1] = A(1,0)*X(0) + A(1,1)*X(1) + A(1,2)*X(2);
                    t[2] = A(2,0)*X(0) + A(2,1)*X(1) + A(2,2)*X(2);
                }

                Y(0) = t[0];
                Y(1) = t[1];
                Y(2) = t[2];
            }
            else
            {
                Y(0) = data3 ? A(0,0)*X(0) + B(0) : A(0,0)*X(0);
            }

            EXIT;
        }
    }

    // general case
    {
        CxGEMMFunc func;
        CxSize size = cxGetMatSize( src1 );
        CxSize dstsize = cxGetMatSize( dst );
        CxMat tmat, *tdst = dst;

        int alg = src1->cols == 1 && CX_IS_MAT_CONT(src1->type & src2->type) ? 2 :
                  src2->cols == 1 && CX_IS_MAT_CONT(src2->type) ? 0 : 1;
        
        if( !inittab )
        {
            icxInitGEMMTable( &gemm_tab );
            inittab = 1;
        }

        if( dst->data.ptr == src1->data.ptr || dst->data.ptr == src2->data.ptr )
        {
            int buf_size = dstsize.width*dstsize.height*icxPixSize[type];
            if( buf_size <= CX_MAX_LOCAL_SIZE )
            {
                buffer = (uchar*)alloca( buf_size + 8 );
                buffer = (uchar*)cxAlignPtr( buffer, 8 );
                local_alloc = 1;
            }
            else
            {
                CX_CALL( buffer = (uchar*)cxAlloc( buf_size ));
            }

            CX_CALL( cxInitMatHeader( &tmat, dstsize.height,
                                      dstsize.width, type, buffer ));
            tdst = &tmat;
        }

        func = (CxGEMMFunc)(gemm_tab.fn_2d[type]);
        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src1->data.ptr, src1->step, src2->data.ptr, src2->step,
                         src3->data.ptr, src3->step, tdst->data.ptr, tdst->step,
                         size, dstsize, 1., 1., alg, 0 ));

        if( tdst != dst )
        {
            CX_CALL( cxCopy( tdst, dst ));
        }
    }

    CX_CHECK_NANS( dst );

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );
}

/****************************************************************************************\
*                                        cxMatMulAddS                                    *
\****************************************************************************************/


#define  ICX_DEF_MMULADDC_CASE_C2( arrtype, temptype, src, dst, mat,        \
                                  _cast_macro1_, _cast_macro2_ )            \
{                                                                           \
    temptype t0, t1;                                                        \
                                                                            \
    t0 = _cast_macro1_((mat)[0]*(src)[0] + (mat)[1]*(src)[1] + (mat)[2]);   \
    t1 = _cast_macro1_((mat)[3]*(src)[0] + (mat)[4]*(src)[1] + (mat)[5]);   \
                                                                            \
    (dst)[0] = _cast_macro2_(t0);                                           \
    (dst)[1] = _cast_macro2_(t1);                                           \
}


#define  ICX_DEF_MMULADDC_CASE_C3( arrtype, temptype, src, dst, mat,        \
                                  _cast_macro1_, _cast_macro2_ )            \
{                                                                           \
    temptype t0, t1, t2;                                                    \
                                                                            \
    t0 = _cast_macro1_((mat)[0]*(src)[0] + (mat)[1]*(src)[1] +              \
                       (mat)[2]*(src)[2] + (mat)[3]);                       \
    t1 = _cast_macro1_((mat)[4]*(src)[0] + (mat)[5]*(src)[1] +              \
                       (mat)[6]*(src)[2] + (mat)[7]);                       \
    t2 = _cast_macro1_((mat)[8]*(src)[0] + (mat)[9]*(src)[1] +              \
                       (mat)[10]*(src)[2] + (mat)[11]);                     \
                                                                            \
    (dst)[0] = _cast_macro2_(t0);                                           \
    (dst)[1] = _cast_macro2_(t1);                                           \
    (dst)[2] = _cast_macro2_(t2);                                           \
}


#define  ICX_DEF_MMULADDC_CASE_C4( arrtype, temptype, src, dst, mat,        \
                                  _cast_macro1_, _cast_macro2_ )            \
{                                                                           \
    temptype t0, t1, t2, t3;                                                \
                                                                            \
    t0 = _cast_macro1_((mat)[0]*(src)[0] + (mat)[1]*(src)[1] +              \
                       (mat)[2]*(src)[2] + (mat)[3]*(src)[3] + (mat)[4]);   \
    t1 = _cast_macro1_((mat)[5]*(src)[0] + (mat)[6]*(src)[1] +              \
                       (mat)[7]*(src)[2] + (mat)[8]*(src)[3] + (mat)[9]);   \
    t2 = _cast_macro1_((mat)[10]*(src)[0] + (mat)[11]*(src)[1] +            \
                       (mat)[12]*(src)[2] + (mat)[13]*(src)[3] + (mat)[14]);\
    t3 = _cast_macro1_((mat)[15]*(src)[0] + (mat)[16]*(src)[1] +            \
                       (mat)[17]*(src)[2] + (mat)[18]*(src)[3] + (mat)[19]);\
                                                                            \
    (dst)[0] = _cast_macro2_(t0);                                           \
    (dst)[1] = _cast_macro2_(t1);                                           \
    (dst)[2] = _cast_macro2_(t2);                                           \
    (dst)[3] = _cast_macro2_(t3);                                           \
}



#define  ICX_DEF_MATMULADDS_FUNC( flavor, arrtype, scalartype, temptype,    \
                                  _cast_macro1_, _cast_macro2_, cn  )       \
static CxStatus CX_STDCALL                                                  \
icxMatMulAddC_##flavor( const arrtype* src, int srcstep,                    \
                        arrtype* dst, int dststep, CxSize size,             \
                        const scalartype* mat )                             \
{                                                                           \
    size.width *= (cn);                                                     \
                                                                            \
    for( ; size.height--; (char*&)src += srcstep, (char*&)dst += dststep )  \
    {                                                                       \
        int i;                                                              \
        for( i = 0; i < size.width; i += (cn) )                             \
        {                                                                   \
            ICX_DEF_MMULADDC_CASE_C##cn( arrtype, temptype, src + i,        \
                         dst + i, mat, _cast_macro1_, _cast_macro2_ )       \
        }                                                                   \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}


ICX_DEF_MATMULADDS_FUNC( 8u_C2R, uchar, double, int, cxRound, CX_CAST_8U, 2 )
ICX_DEF_MATMULADDS_FUNC( 8u_C3R, uchar, double, int, cxRound, CX_CAST_8U, 3 )
ICX_DEF_MATMULADDS_FUNC( 8u_C4R, uchar, double, int, cxRound, CX_CAST_8U, 4 )

ICX_DEF_MATMULADDS_FUNC( 32s_C2R, int, double, int, cxRound, CX_NOP, 2 )
ICX_DEF_MATMULADDS_FUNC( 32s_C3R, int, double, int, cxRound, CX_NOP, 3 )
ICX_DEF_MATMULADDS_FUNC( 32s_C4R, int, double, int, cxRound, CX_NOP, 4 )

ICX_DEF_MATMULADDS_FUNC( 32f_C2R, float, double, double, CX_NOP, CX_CAST_32F, 2 )
ICX_DEF_MATMULADDS_FUNC( 32f_C3R, float, double, double, CX_NOP, CX_CAST_32F, 3 )
ICX_DEF_MATMULADDS_FUNC( 32f_C4R, float, double, double, CX_NOP, CX_CAST_32F, 4 )

ICX_DEF_MATMULADDS_FUNC( 64f_C2R, double, double, double, CX_NOP, CX_CAST_64F, 2 )
ICX_DEF_MATMULADDS_FUNC( 64f_C3R, double, double, double, CX_NOP, CX_CAST_64F, 3 )
ICX_DEF_MATMULADDS_FUNC( 64f_C4R, double, double, double, CX_NOP, CX_CAST_64F, 4 )


static void
icxInitMatMulAddCTable( CxBigFuncTable* tab )
{
    tab->fn_2d[CX_8UC2] = (void*)icxMatMulAddC_8u_C2R;
    tab->fn_2d[CX_8UC3] = (void*)icxMatMulAddC_8u_C3R;
    tab->fn_2d[CX_8UC4] = (void*)icxMatMulAddC_8u_C4R;

    tab->fn_2d[CX_32SC2] = (void*)icxMatMulAddC_32s_C2R;
    tab->fn_2d[CX_32SC3] = (void*)icxMatMulAddC_32s_C3R;
    tab->fn_2d[CX_32SC4] = (void*)icxMatMulAddC_32s_C4R;

    tab->fn_2d[CX_32FC2] = (void*)icxMatMulAddC_32f_C2R;
    tab->fn_2d[CX_32FC3] = (void*)icxMatMulAddC_32f_C3R;
    tab->fn_2d[CX_32FC4] = (void*)icxMatMulAddC_32f_C4R;

    tab->fn_2d[CX_64FC2] = (void*)icxMatMulAddC_64f_C2R;
    tab->fn_2d[CX_64FC3] = (void*)icxMatMulAddC_64f_C3R;
    tab->fn_2d[CX_64FC4] = (void*)icxMatMulAddC_64f_C4R;
}


CX_IMPL void
cxMatMulAddS( const CxArr* srcarr, CxArr* dstarr,
              const CxMat* transform, const CxMat* shiftvec )
{
    static CxBigFuncTable mmuladds_tab;
    static int inittab = 0;
    
    double buffer[20];
    
    CX_FUNCNAME( "cxMatMulAddS" );

    __BEGIN__;

    CxMat stub1, *src = (CxMat*)srcarr;
    CxMat stub, *dst = (CxMat*)dstarr;
    CxMat rstub, *rot = (CxMat*)transform;
    CxMat sstub, *shift = (CxMat*)shiftvec;
    CxSeq* seq = 0;
    int i, j, type, cn;

    if( !inittab )
    {
        icxInitMatMulAddCTable( &mmuladds_tab );
        inittab = 1;
    }

    if( !CX_IS_MAT( src ))
    {
        if( CX_IS_SEQ( src ))
        {
            seq = (CxSeq*)src;
            if( icxPixSize[CX_SEQ_ELTYPE(seq)] != seq->elem_size )
                CX_ERROR( CX_StsUnsupportedFormat,
                "Unsupported type of sequence elements" );
            if( dst != src && dst != 0 )
                CX_ERROR( CX_StsBadArg,
                "For sequences only inplace mode is supported" );
        }
        else
        {
            int coi = 0;
            CX_CALL( src = cxGetMat( src, &stub1, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    if( dst == 0 )
        dst = src;

    if( dst != src )
    {
        if( !CX_IS_MAT( dst ))
        {
            int coi = 0;
            CX_CALL( dst = cxGetMat( dst, &stub, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }

        if( !CX_ARE_TYPES_EQ( src, dst ))
            CX_ERROR( CX_StsUnmatchedFormats, "" );

        if( !CX_ARE_SIZES_EQ( src, dst ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );
    }

    type = CX_MAT_TYPE( src->type );
    cn = CX_MAT_CN( type );

    if( !CX_IS_MAT( rot ))
    {
        int coi = 0;
        CX_CALL( rot = cxGetMat( rot, &rstub, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( rot->rows != cn )
        CX_ERROR( CX_StsBadSize,
        "The height of transform matrix must be equal to number of channels" );

    if( rot->cols == cn + 1 || rot->cols == cn )
    {
        if( CX_MAT_TYPE( rot->type ) == CX_64FC1 )
        {
            for( i = 0; i < cn; i++ )
            {
                buffer[i*(cn+1) + cn] = 0;

                for( j = 0; j < rot->cols; j++ )
                    buffer[i*(cn+1) + j] = ((double*)(rot->data.ptr + rot->step*i))[j];
            }
        }
        else if( CX_MAT_TYPE( rot->type ) == CX_32FC1 )
        {
            for( i = 0; i < cn; i++ )
            {
                buffer[i*(cn+1) + cn] = 0;

                for( j = 0; j < rot->cols; j++ )
                    buffer[i*(cn+1) + j] = ((float*)(rot->data.ptr + rot->step*i))[j];
            }
        }
        else
        {
            CX_ERROR( CX_StsUnsupportedFormat, "Rotation matrix must be 32fC1 or 64fC1" );
        }
    }
    else
    {
        CX_ERROR( CX_StsUnmatchedSizes, "If the source array has <cn> channels, "
           "the transformation matrix must have <cn> x <cn>+1 or <cn> x <cn> size" );
    }

    if( shift )
    {
        if( !CX_IS_MAT( shift ))
        {
            int coi = 0;
            CX_CALL( shift = cxGetMat( shift, &sstub, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }

        if( shift->rows == 1 && (CX_MAT_CN( shift->type ) != 1 ||
            CX_MAT_CN( shift->type ) * shift->cols == cn) )
        {
            if( CX_MAT_DEPTH( shift->type ) == CX_64F )
            {
                for( i = 0; i < cn; i++ )
                    buffer[i*(cn+1) + cn] += shift->data.db[i];
            }
            else if( CX_MAT_DEPTH( shift->type ) == CX_32F )
            {
                for( i = 0; i < cn; i++ )
                    buffer[i*(cn+1) + cn] += shift->data.fl[i];
            }
            else
            {
                CX_ERROR( CX_StsUnsupportedFormat,
                    "Shift vector must be 32f or 64f" );
            }
        }
        else if( CX_MAT_CN( shift->type ) == 1 && shift->rows == cn )
        {
            if( CX_MAT_DEPTH( shift->type ) == CX_64F )
            {
                for( i = 0; i < cn; i++ )
                    buffer[i*(cn+1) + cn] += *(double*)(shift->data.ptr + shift->step*i);
            }
            if( CX_MAT_DEPTH( shift->type ) == CX_32F )
            {
                for( i = 0; i < cn; i++ )
                    buffer[i*(cn+1) + cn] += *(float*)(shift->data.ptr + shift->step*i);
            }
            else
            {
                CX_ERROR( CX_StsUnsupportedFormat,
                    "Shift vector must be 32f or 64f" );
            }
        }
        else
        {
            CX_ERROR( CX_StsUnmatchedSizes,
                "Shift (if present) must have a total length equal to number "
                "of channels in the processed arrays" );
        }
    }

    if( cn > 1 )
    {
        CxFunc2D_2A1P func = (CxFunc2D_2A1P)(mmuladds_tab.fn_2d[type]);
        CxSize size;

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        if( !seq )
        {
            size = cxGetMatSize( src );

            if( CX_IS_MAT_CONT( src->type & dst->type ))
            {
                size.width *= size.height;
                size.height = 1;
            }

            IPPI_CALL( func( src->data.ptr, src->step, dst->data.ptr,
                             dst->step, size, buffer));
        }
        else
        {
            CxSeqBlock* block = seq->first;

            if( block )
            {
                do
                {
                    IPPI_CALL( func( block->data, CX_STUB_STEP, block->data,
                               CX_STUB_STEP, cxSize( block->count, 1 ), buffer ));
                    block = block->next;
                }
                while( block != seq->first );
            }
        }
    }
    else // the simplest variant
    {
        CX_CALL( cxCxtScale( src, dst, buffer[0], buffer[1] ));
    }

    __END__;
}


/****************************************************************************************\
*                                       cxMulAddS                                        *
\****************************************************************************************/

#define  ICX_DEF_MULADDC_CASE_C1( arrtype, temptype, src1, src2, dst, len )     \
{                                                                               \
    int i;                                                                      \
                                                                                \
    for( i = 0; i <= (len) - 4; i += 4 )                                        \
    {                                                                           \
        temptype t0 = (src1)[i]*s0 + (src2)[i];                                 \
        temptype t1 = (src1)[i+1]*s0 + (src2)[i+1];                             \
                                                                                \
        (dst)[i] = (arrtype)t0;                                                 \
        (dst)[i+1] = (arrtype)t1;                                               \
                                                                                \
        t0 = (src1)[i+2]*s0 + (src2)[i+2];                                      \
        t1 = (src1)[i+3]*s0 + (src2)[i+3];                                      \
                                                                                \
        (dst)[i+2] = (arrtype)t0;                                               \
        (dst)[i+3] = (arrtype)t1;                                               \
    }                                                                           \
                                                                                \
    for( ; i < (len); i++ )                                                     \
    {                                                                           \
        temptype t0 = (src1)[i]*s0 + (src2)[i];                                 \
        (dst)[i] = (arrtype)t0;                                                 \
    }                                                                           \
}


#define  ICX_DEF_MULADDC_CASE_C2( arrtype, temptype, src1, src2, dst, len )     \
{                                                                               \
    int i;                                                                      \
                                                                                \
    for( i = 0; i <= (len) - 4; i += 4 )                                        \
    {                                                                           \
        temptype t0 = (src1)[i]*s0 - (src1)[i+1]*s1 + (src2)[i];                \
        temptype t1 = (src1)[i]*s1 + (src1)[i+1]*s0 + (src2)[i+1];              \
                                                                                \
        (dst)[i] = (arrtype)t0;                                                 \
        (dst)[i+1] = (arrtype)t1;                                               \
                                                                                \
        t0 = (src1)[i+2]*s0 - (src1)[i+3]*s1 + (src2)[i+2];                     \
        t1 = (src1)[i+2]*s1 + (src1)[i+3]*s0 + (src2)[i+3];                     \
                                                                                \
        (dst)[i+2] = (arrtype)t0;                                               \
        (dst)[i+3] = (arrtype)t1;                                               \
    }                                                                           \
                                                                                \
    for( ; i < (len); i += 2 )                                                  \
    {                                                                           \
        temptype t0 = (src1)[i]*s0 - (src1)[i+1]*s1 + (src2)[i];                \
        temptype t1 = (src1)[i]*s1 + (src1)[i+1]*s0 + (src2)[i+1];              \
                                                                                \
        (dst)[i] = (arrtype)t0;                                                 \
        (dst)[i+1] = (arrtype)t1;                                               \
    }                                                                           \
}


#define  ICX_DEF_MULADDS_FUNC( flavor, arrtype, scalartype, entry, cn )     \
static CxStatus CX_STDCALL                                                  \
icxMulAddC_##flavor( const arrtype* src1, int srcstep1,                     \
                      const arrtype* src2, int srcstep2,                    \
                      arrtype* dst, int dststep, CxSize size,               \
                      const scalartype* scalar )                            \
{                                                                           \
    entry(scalartype);                                                      \
    size.width *= (cn);                                                     \
                                                                            \
    for( ; size.height--; (char*&)src1 += srcstep1,                         \
                          (char*&)src2 += srcstep2,                         \
                          (char*&)dst += dststep )                          \
    {                                                                       \
        ICX_DEF_MULADDC_CASE_C##cn( arrtype, scalartype, src1, src2,        \
                                    dst, size.width )                       \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}


ICX_DEF_MULADDS_FUNC( 32f_C1R, float, double, CX_UN_ENTRY_C1, 1 )
ICX_DEF_MULADDS_FUNC( 32f_C2R, float, double, CX_UN_ENTRY_C2, 2 )
ICX_DEF_MULADDS_FUNC( 64f_C1R, double, double, CX_UN_ENTRY_C1, 1 )
ICX_DEF_MULADDS_FUNC( 64f_C2R, double, double, CX_UN_ENTRY_C2, 2 )


static void
icxInitMulAddCTable( CxBigFuncTable* tab )
{
    tab->fn_2d[CX_32FC1] = (void*)icxMulAddC_32f_C1R;
    tab->fn_2d[CX_32FC2] = (void*)icxMulAddC_32f_C2R;
    tab->fn_2d[CX_64FC1] = (void*)icxMulAddC_64f_C1R;
    tab->fn_2d[CX_64FC2] = (void*)icxMulAddC_64f_C2R;
}


CX_IMPL void
cxScaleAdd( const CxArr* srcarr1, CxScalar scale,
            const CxArr* srcarr2, CxArr* dstarr )
{
    static CxBigFuncTable muladds_tab;
    static int inittab = 0;
    
    CX_FUNCNAME( "cxScaleAdd" );

    __BEGIN__;

    CxMat stub1, *src1 = (CxMat*)srcarr1;
    CxMat stub2, *src2 = (CxMat*)srcarr2;
    CxMat stub, *dst = (CxMat*)dstarr;
    CxSize size;
    int type;

    if( !CX_IS_MAT( src1 ) || !CX_IS_MAT(src2) || !CX_IS_MAT(dst))
    {
        int coi1 = 0, coi2 = 0, coi3 = 0;
        CX_CALL( src1 = cxGetMat( src1, &stub1, &coi1 ));
        CX_CALL( src2 = cxGetMat( src2, &stub2, &coi2 ));
        CX_CALL( dst = cxGetMat( dst, &stub, &coi3 ));

        if( coi1 + coi2 + coi3 != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_ARE_TYPES_EQ( src1, dst ) || !CX_ARE_TYPES_EQ( src2, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( !CX_ARE_SIZES_EQ( src1, dst ) || !CX_ARE_SIZES_EQ( src2, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    type = CX_MAT_TYPE( src1->type );
    size = cxGetMatSize( src1 );

    if( CX_IS_MAT_CONT( src1->type & src2->type & dst->type ))
    {
        size.width *= size.height;

        if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE )
        {
            if( type == CX_32FC1 )
            {
                float* mA = src1->data.fl;
                float* mB = src2->data.fl;
                float* mC = dst->data.fl;

                do
                {
                    mC[size.width - 1] = (float)(mA[size.width - 1]*scale.val[0] +
                                         mB[size.width - 1]);
                }
                while( --size.width );

                EXIT;
            }

            if( type == CX_64FC1 )
            {
                double* mA = src1->data.db;
                double* mB = src2->data.db;
                double* mC = dst->data.db;

                do
                {
                    mC[size.width - 1] = mA[size.width - 1]*scale.val[0] +
                                         mB[size.width - 1];
                }
                while( --size.width );

                EXIT;
            }
        }

        size.height = 1;
    }

    if( !inittab )
    {
        icxInitMulAddCTable( &muladds_tab );
        inittab = 1;
    }

    {
        CxFunc2D_3A1P func = (CxFunc2D_3A1P)(muladds_tab.fn_2d[type]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src1->data.ptr, src1->step, src2->data.ptr, src2->step,
                         dst->data.ptr, dst->step, size, scale.val ));
    }

    CX_CHECK_NANS( dst );

    __END__;
}


/****************************************************************************************\
*                                    cxCalcCovarMatrix                                   *
\****************************************************************************************/

#define ICX_DOT_PRODUCT_CASE( flavor, srctype, avgtype, load_macro )                    \
static CxStatus CX_STDCALL                                                              \
icxDotProductShifted_##flavor##_C1R( const srctype* vec1, int vecstep1,                 \
                                     const srctype* vec2, int vecstep2,                 \
                                     const avgtype* avg, int avgstep,                   \
                                     CxSize size, double* _result )                     \
{                                                                                       \
    double result = 0;                                                                  \
                                                                                        \
    for( ; size.height--; (char*&)vec1 += vecstep1,                                     \
                          (char*&)vec2 += vecstep2,                                     \
                          (char*&)avg += avgstep )                                      \
    {                                                                                   \
        int x;                                                                          \
        for( x = 0; x <= size.width - 4; x += 4 )                                       \
            result += (load_macro(vec1[x]) - avg[x])*(load_macro(vec2[x]) - avg[x]) +   \
                (load_macro(vec1[x+1]) - avg[x+1])*(load_macro(vec2[x+1]) - avg[x+1]) + \
                (load_macro(vec1[x+2]) - avg[x+2])*(load_macro(vec2[x+2]) - avg[x+2]) + \
                (load_macro(vec1[x+3]) - avg[x+3])*(load_macro(vec2[x+3]) - avg[x+3]);  \
        for( ; x < size.width; x++ )                                                    \
            result += (load_macro(vec1[x]) - avg[x])*(load_macro(vec2[x]) - avg[x]);    \
    }                                                                                   \
                                                                                        \
    *_result = result;                                                                  \
    return CX_OK;                                                                       \
}


ICX_DOT_PRODUCT_CASE( 8u32f, uchar, float, CX_8TO32F )
ICX_DOT_PRODUCT_CASE( 8u64f, uchar, double, CX_8TO32F )
ICX_DOT_PRODUCT_CASE( 16s32f, short, float, CX_NOP )
ICX_DOT_PRODUCT_CASE( 16s64f, short, double, CX_NOP )
ICX_DOT_PRODUCT_CASE( 32f, float, float, CX_NOP )
ICX_DOT_PRODUCT_CASE( 32f64f, float, double, CX_NOP )
ICX_DOT_PRODUCT_CASE( 64f, double, double, CX_NOP )

static void  icxInitDotProductShiftedTable( CxFuncTable* tabfl, CxFuncTable* tabdb )
{
    tabfl->fn_2d[CX_8U] = (void*)icxDotProductShifted_8u32f_C1R;
    tabfl->fn_2d[CX_8S] = 0;
    tabfl->fn_2d[CX_16S] = (void*)icxDotProductShifted_16s32f_C1R;
    tabfl->fn_2d[CX_32S] = 0;
    tabfl->fn_2d[CX_32F] = (void*)icxDotProductShifted_32f_C1R;
    tabfl->fn_2d[CX_64F] = 0;

    tabdb->fn_2d[CX_8U] = (void*)icxDotProductShifted_8u64f_C1R;
    tabdb->fn_2d[CX_8S] = 0;
    tabdb->fn_2d[CX_16S] = (void*)icxDotProductShifted_16s64f_C1R;
    tabdb->fn_2d[CX_32S] = 0;
    tabdb->fn_2d[CX_32F] = (void*)icxDotProductShifted_32f64f_C1R;
    tabdb->fn_2d[CX_64F] = (void*)icxDotProductShifted_64f_C1R;
}

#define ICX_EXT_PRODUCT_CASE( flavor, srctype, avgtype, load_macro )                    \
static CxStatus CX_STDCALL                                                              \
icxExtProductShifted_##flavor##_C1R( const srctype* vec, int vecstep,                   \
                                     const avgtype* avg, int avgstep,                   \
                                     avgtype* dst, int dststep,                         \
                                     CxSize size, avgtype* tempbuf )                    \
{                                                                                       \
    int x, y, dstsize = size.width * size.height;                                       \
                                                                                        \
    for( y = 0; y < size.height; y++, (char*&)vec += vecstep, (char*&)avg += avgstep )  \
        for( x = 0; x < size.width; x++ )                                               \
            *tempbuf++ = load_macro(vec[x]) - avg[x];                                   \
    tempbuf -= dstsize;                                                                 \
                                                                                        \
    for( y = 0; y < dstsize; y++, (char*&)dst += dststep )                              \
    {                                                                                   \
        double ty = tempbuf[y];                                                         \
        for( x = 0; x <= y - 3; x += 4 )                                                \
        {                                                                               \
            double t0 = dst[x] + ty*tempbuf[x];                                         \
            double t1 = dst[x+1] + ty*tempbuf[x+1];                                     \
            dst[x] = (avgtype)t0;                                                       \
            dst[x+1] = (avgtype)t1;                                                     \
            t0 = dst[x+2] + ty*tempbuf[x+2];                                            \
            t1 = dst[x+3] + ty*tempbuf[x+3];                                            \
            dst[x+2] = (avgtype)t0;                                                     \
            dst[x+3] = (avgtype)t1;                                                     \
        }                                                                               \
        for( ; x <= y; x++ )                                                            \
            dst[x] = (avgtype)(dst[x] + ty*tempbuf[x]);                                 \
    }                                                                                   \
                                                                                        \
    return CX_OK;                                                                       \
}

ICX_EXT_PRODUCT_CASE( 8u32f, uchar, float, CX_8TO32F )
ICX_EXT_PRODUCT_CASE( 8u64f, uchar, double, CX_8TO32F )
ICX_EXT_PRODUCT_CASE( 16s32f, short, float, CX_NOP )
ICX_EXT_PRODUCT_CASE( 16s64f, short, double, CX_NOP )
ICX_EXT_PRODUCT_CASE( 32f, float, float, CX_NOP )
ICX_EXT_PRODUCT_CASE( 32f64f, float, double, CX_NOP )
ICX_EXT_PRODUCT_CASE( 64f, double, double, CX_NOP )


static void  icxInitExtProductShiftedTable( CxFuncTable* tabfl, CxFuncTable* tabdb )
{
    tabfl->fn_2d[CX_8U] = (void*)icxExtProductShifted_8u32f_C1R;
    tabfl->fn_2d[CX_8S] = 0;
    tabfl->fn_2d[CX_16S] = (void*)icxExtProductShifted_16s32f_C1R;
    tabfl->fn_2d[CX_32S] = 0;
    tabfl->fn_2d[CX_32F] = (void*)icxExtProductShifted_32f_C1R;
    tabfl->fn_2d[CX_64F] = 0;

    tabdb->fn_2d[CX_8U] = (void*)icxExtProductShifted_8u64f_C1R;
    tabdb->fn_2d[CX_8S] = 0;
    tabdb->fn_2d[CX_16S] = (void*)icxExtProductShifted_16s64f_C1R;
    tabdb->fn_2d[CX_32S] = 0;
    tabdb->fn_2d[CX_32F] = (void*)icxExtProductShifted_32f64f_C1R;
    tabdb->fn_2d[CX_64F] = (void*)icxExtProductShifted_64f_C1R;
}


typedef struct vec_data
{
    void* ptr;
    int step;
}
vec_data;

CX_IMPL void
cxCalcCovarMatrix( const CxArr** vecarr, CxArr* covarr,
                   CxArr* avgarr, int count, int flags )
{
    static CxFuncTable dot_tab[2];
    static CxFuncTable ext_tab[2];
    static int inittab = 0;
    vec_data* vecdata = 0;
    CxMat *tempvec = 0;
    
    CX_FUNCNAME( "cxCalcCovarMatrix" );

    __BEGIN__;

    CxMat covstub, *cov = (CxMat*)covarr;
    CxMat avgstub, *avg = (CxMat*)avgarr;
    CxSize srcsize, contsize;
    int srctype = 0, dsttype = 0;
    int i, j;
    int cont_flag;
    int is_covar_normal = (flags & 3) == CX_COVAR_NORMAL;
    double scale = 1;
    CxFunc2D_3A1P ext_func = 0;
    CxFunc2D_3A1P dot_func = 0;

    if( !inittab )
    {
        icxInitDotProductShiftedTable( dot_tab + 0, dot_tab + 1 );
        icxInitExtProductShiftedTable( ext_tab + 0, ext_tab + 1 );
        inittab = 1;
    }

    if( !vecarr )
        CX_ERROR( CX_StsNullPtr, "NULL vec pointer" );

    CX_CALL( cov = cxGetMat( cov, &covstub ));
    CX_CALL( avg = cxGetMat( avg, &avgstub ));

    if( !CX_ARE_TYPES_EQ( cov, avg ))
        CX_ERROR( CX_StsUnmatchedFormats,
        "Covariation matrix and average vector should have the same types" );

    dsttype = CX_MAT_TYPE( cov->type );
    if( dsttype != CX_32FC1 && dsttype != CX_64FC1 )
        CX_ERROR( CX_StsUnsupportedFormat, "Covariation matrix must be 32fC1 or 64fC1" );

    if( cov->rows != cov->cols )
        CX_ERROR( CX_StsBadSize, "Covariation matrix must be square" );

    srcsize = cxGetMatSize( avg );
    contsize.width = srcsize.width * srcsize.height;
    contsize.height = 1;

    if( is_covar_normal )
    {
        if( count <= 0 )
            CX_ERROR( CX_StsBadSize,
            "When normal (not scrambled) covariance matrix is calculated, "
            "a valid number of vectors must be specified" );
        if( cov->rows != contsize.width )
            CX_ERROR( CX_StsUnmatchedSizes,
            "When normal (not scrambled) covariance matrix is calculated, "
            "the matrix, average vector and all input vectors must have the same size" );

        CX_CALL( tempvec = cxCreateMat( avg->rows, avg->cols, dsttype ));
    }
    else if( count )
    {
        if( count != cov->rows )
            CX_ERROR( CX_StsUnmatchedSizes,
            "Passed vector count and covariance matrix size do not match" );
    }
    else
        count = cov->rows;

    CX_CALL( vecdata = (vec_data*)cxAlloc( count*sizeof(vecdata[0])));

    if( !(flags & CX_COVAR_USE_AVG) )
        cxZero( avg );
    
    if( flags & CX_COVAR_SCALE )
        scale = 1./(contsize.width - 1);

    cont_flag = avg->type;

    for( i = 0; i < count; i++ )
    {
        CxMat vecstub, *vec = (CxMat*)vecarr[i];
        CxMat* temp;

        if( !CX_IS_MAT(vec) )
            CX_CALL( vec = cxGetMat( vec, &vecstub ));

        if( !CX_ARE_SIZES_EQ( vec, avg ))
            CX_ERROR( CX_StsUnmatchedSizes,
            "All input vectors and average vector must have the same size" );

        vecdata[i].ptr = vec->data.ptr;
        vecdata[i].step = vec->step;
        cont_flag &= vec->type;
        temp = vec;

        if( i == 0 )
        {
            srctype = CX_MAT_TYPE( vec->type );
            if( CX_MAT_CN( srctype ) != 1 )
                CX_ERROR( CX_BadNumChannels, "All vectors must have a single channel" );
            if( srctype != dsttype && !tempvec && !(flags & CX_COVAR_USE_AVG))
                CX_CALL( tempvec = cxCreateMat( vec->rows, vec->cols, dsttype ));

            if( is_covar_normal )
            {
                ext_func = (CxFunc2D_3A1P)
                    ext_tab[dsttype == CX_64FC1].fn_2d[CX_MAT_DEPTH(srctype)];
                if( !ext_func )
                    CX_ERROR( CX_StsUnsupportedFormat,
                        "The format of input vectors is not supported" );
            }
            else
            {
                dot_func = (CxFunc2D_3A1P)
                    dot_tab[dsttype == CX_64FC1].fn_2d[CX_MAT_DEPTH(srctype)];
                if( !dot_func )
                    CX_ERROR( CX_StsUnsupportedFormat,
                        "The format of input vectors is not supported" );
            }
        }
        else if( CX_MAT_TYPE(vec->type) != srctype )
            CX_ERROR( CX_StsUnmatchedFormats,
            "All input vectors must have the same type" );

        if( !(flags & CX_COVAR_USE_AVG) )
        {
            if( tempvec )
            {
                temp = tempvec;
                cxConvert( vec, temp );
            }

            cxAdd( temp, avg, avg );
        }
    }

    if( !(flags & CX_COVAR_USE_AVG) )
        cxScale( avg, avg, 1./count );

    cont_flag = CX_IS_MAT_CONT( cont_flag );

    if( !is_covar_normal )
    {
        for( i = 0; i < count; i++ )
        {
            int a, b, delta;
            if( !(i & 1) )
                a = 0, b = i+1, delta = 1;
            else
                a = i, b = -1, delta = -1;

            for( j = a; j != b; j += delta )
            {
                double result = 0;
                if( cont_flag )
                {
                    dot_func( vecdata[i].ptr, CX_STUB_STEP, vecdata[j].ptr, CX_STUB_STEP,
                              avg->data.ptr, CX_STUB_STEP, contsize, &result );
                }
                else
                {
                    dot_func( vecdata[i].ptr, vecdata[i].step,
                              vecdata[j].ptr, vecdata[j].step,
                              avg->data.ptr, avg->step, srcsize, &result );
                }
                if( dsttype == CX_64FC1 )
                {
                    ((double*)(cov->data.ptr + i*cov->step))[j] =
                    ((double*)(cov->data.ptr + j*cov->step))[i] = result*scale;
                }
                else
                {
                    ((float*)(cov->data.ptr + i*cov->step))[j] =
                    ((float*)(cov->data.ptr + j*cov->step))[i] = (float)(result*scale);
                }
            }
        }
    }
    else
    {
        uchar* cov_ptr = cov->data.ptr;
        int cov_step = cov->step;
        int cov_size = cov->rows;
        
        cxZero( cov );
        
        for( i = 0; i < count; i++ )
        {
            if( cont_flag )
                ext_func( vecdata[i].ptr, CX_STUB_STEP,
                          avg->data.ptr, CX_STUB_STEP,
                          cov_ptr, cov_step,
                          contsize, tempvec->data.ptr );
            else
                ext_func( vecdata[i].ptr, vecdata[i].step,
                          avg->data.ptr, avg->step,
                          cov_ptr, cov_step,
                          contsize, tempvec->data.ptr );
        }

        if( dsttype == CX_64FC1 )
            for( i = 0; i < cov_size; i++ )
                for( j = 0; j <= i; j++ )
                {
                    double* cov1 = ((double*)(cov_ptr + i*cov_step)) + j;
                    double* cov2 = ((double*)(cov_ptr + j*cov_step)) + i;

                    if( flags & CX_COVAR_SCALE )
                        *cov1 = *cov2 = *cov1*scale;
                    else
                        *cov2 = *cov1;
                }
        else
            for( i = 0; i < cov_size; i++ )
                for( j = 0; j <= i; j++ )
                {
                    float* cov1 = ((float*)(cov_ptr + i*cov_step)) + j;
                    float* cov2 = ((float*)(cov_ptr + j*cov_step)) + i;

                    if( flags & CX_COVAR_SCALE )
                        *cov1 = *cov2 = (float)(*cov1*scale);
                    else
                        *cov2 = *cov1;
                }
    }

    __END__;

    cxFree( (void**)&vecdata );
    cxReleaseMat( &tempvec );
}

/****************************************************************************************\
*                                        cxMahalanobis                                   *
\****************************************************************************************/

CX_IMPL double
cxMahalanobis( const CxArr* srcAarr, const CxArr* srcB, CxArr* mat )
{
    uchar* buffer = 0;
    int local_alloc = 0;
    double dist = 0;

    CX_FUNCNAME( "cxMahalanobis" );

    __BEGIN__;

    int buf_size, pix_size;
    CxMat stubA, *srcA = (CxMat*)srcAarr;
    CxMat temp1, temp2;

    if( !CX_IS_MAT(srcA) )
        CX_CALL( srcA = cxGetMat( srcA, &stubA ));

    pix_size = icxPixSize[CX_MAT_TYPE(srcA->type)];
    buf_size = srcA->rows*2*pix_size;
    
    if( buf_size <= CX_MAX_LOCAL_SIZE )
    {
        buffer = (uchar*)alloca( buf_size );
        local_alloc = 1;
    }
    else
    {
        CX_CALL( buffer = (uchar*)cxAlloc( buf_size ));
    }

    cxInitMatHeader( &temp1, srcA->rows, 1, srcA->type, buffer );
    cxInitMatHeader( &temp2, srcA->rows, 1, srcA->type, buffer + buf_size/2);

    CX_CALL( cxSub( srcA, srcB, &temp1 ));
    CX_CALL( cxMatMulAdd( mat, &temp1, 0, &temp2 ));
    CX_CALL( dist = sqrt(cxDotProduct( &temp1, &temp2 )));

    /*icxCheckVector_64f( &dist, 1 );*/

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );

    return  dist;
}


/****************************************************************************************\
*                                        cxMulTransposed                                 *
\****************************************************************************************/

#define ICX_DEF_MULTRANS_R_FUNC( flavor, arrtype )                              \
static CxStatus CX_STDCALL                                                      \
icxMulTransposedR_##flavor( const arrtype* src, int srcstep,                    \
                       arrtype* dst, int dststep,                               \
                       const arrtype* delta, int deltastep,                     \
                       CxSize size )                                            \
{                                                                               \
    int i, j, k;                                                                \
    arrtype* tdst = dst;                                                        \
    arrtype* col_buf = 0;                                                       \
    int local_alloc = 0;                                                        \
    int buf_size = size.height*sizeof(arrtype);                                 \
                                                                                \
    if( buf_size <= CX_MAX_LOCAL_SIZE )                                         \
    {                                                                           \
        col_buf = (arrtype*)alloca( buf_size );                                 \
        local_alloc = 1;                                                        \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        col_buf = (arrtype*)cxAlloc( buf_size );                                \
        if( !col_buf )                                                          \
            return CX_OUTOFMEM_ERR;                                             \
    }                                                                           \
                                                                                \
    if( !delta )                                                                \
        for( i = 0; i < size.width; i++, (char*&)tdst += dststep )              \
        {                                                                       \
            for( k = 0; k < size.height; k++ )                                  \
                col_buf[k] = ((arrtype*)((char*)src + k*srcstep))[i];           \
                                                                                \
            for( j = i; j <= size.width - 4; j += 4 )                           \
            {                                                                   \
                double s0 = 0, s1 = 0, s2 = 0, s3 = 0;                          \
                const arrtype *tsrc = src + j;                                  \
                                                                                \
                for( k = 0; k < size.height; k++, (char*&)tsrc += srcstep )     \
                {                                                               \
                    double a = col_buf[k];                                      \
                    s0 += a * tsrc[0];                                          \
                    s1 += a * tsrc[1];                                          \
                    s2 += a * tsrc[2];                                          \
                    s3 += a * tsrc[3];                                          \
                }                                                               \
                                                                                \
                tdst[j] = (arrtype)s0;                                          \
                tdst[j+1] = (arrtype)s1;                                        \
                tdst[j+2] = (arrtype)s2;                                        \
                tdst[j+3] = (arrtype)s3;                                        \
            }                                                                   \
                                                                                \
            for( ; j < size.width; j++ )                                        \
            {                                                                   \
                double s0 = 0;                                                  \
                const arrtype *tsrc = src + j;                                  \
                                                                                \
                for( k = 0; k < size.height; k++, (char*&)tsrc += srcstep )     \
                    s0 += col_buf[k] * tsrc[0];                                 \
                                                                                \
                tdst[j] = (arrtype)s0;                                          \
            }                                                                   \
        }                                                                       \
    else                                                                        \
        for( i = 0; i < size.width; i++, (char*&)tdst += dststep )              \
        {                                                                       \
            for( k = 0; k < size.height; k++ )                                  \
                col_buf[k] = ((arrtype*)((char*)src + k*srcstep))[i] -          \
                             ((arrtype*)((char*)delta + k*deltastep))[i];       \
                                                                                \
            for( j = i; j <= size.width - 4; j += 4 )                           \
            {                                                                   \
                double s0 = 0, s1 = 0, s2 = 0, s3 = 0;                          \
                const arrtype *tsrc = src + j;                                  \
                const arrtype *d = delta + j;                                   \
                                                                                \
                for( k = 0; k < size.height; k++, (char*&)tsrc += srcstep,      \
                                                  (char*&)d += deltastep )      \
                {                                                               \
                    double a = col_buf[k];                                      \
                    s0 += a * (tsrc[0] - d[0]);                                 \
                    s1 += a * (tsrc[1] - d[1]);                                 \
                    s2 += a * (tsrc[2] - d[2]);                                 \
                    s3 += a * (tsrc[3] - d[3]);                                 \
                }                                                               \
                                                                                \
                tdst[j] = (arrtype)s0;                                          \
                tdst[j+1] = (arrtype)s1;                                        \
                tdst[j+2] = (arrtype)s2;                                        \
                tdst[j+3] = (arrtype)s3;                                        \
            }                                                                   \
                                                                                \
            for( ; j < size.width; j++ )                                        \
            {                                                                   \
                double s0 = 0;                                                  \
                const arrtype *tsrc = src + j;                                  \
                const arrtype *d = delta + j;                                   \
                                                                                \
                for( k = 0; k < size.height; k++, (char*&)tsrc += srcstep,      \
                                                  (char*&)d += deltastep )      \
                    s0 += col_buf[k] * (tsrc[0] - d[0]);                        \
                                                                                \
                tdst[j] = (arrtype)s0;                                          \
            }                                                                   \
        }                                                                       \
                                                                                \
    /* fill the lower part of the destination matrix */                         \
    for( i = 1; i < size.width; i++ )                                           \
        for( j = 0; j < i; j++ )                                                \
            ((arrtype*)((uchar*)dst + dststep*i))[j] =                          \
                ((arrtype*)((uchar*)dst + dststep*j))[i];                       \
                                                                                \
    if( col_buf && !local_alloc )                                               \
        cxFree( (void**)&col_buf );                                             \
                                                                                \
    return CX_NO_ERR;                                                           \
}


#define ICX_DEF_MULTRANS_L_FUNC( flavor, arrtype )                              \
static CxStatus CX_STDCALL                                                      \
icxMulTransposedL_##flavor( const arrtype* src, int srcstep,                    \
                            arrtype* dst, int dststep,                          \
                            arrtype* delta, int deltastep,                      \
                            CxSize size )                                       \
{                                                                               \
    int i, j, k;                                                                \
    arrtype* tdst = dst;                                                        \
                                                                                \
    if( !delta )                                                                \
        for( i = 0; i < size.height; i++, (char*&)tdst += dststep )             \
            for( j = i; j < size.height; j++ )                                  \
            {                                                                   \
                double s = 0;                                                   \
                const arrtype *tsrc1 =(const arrtype*)((uchar*)src + i*srcstep);\
                const arrtype *tsrc2 =(const arrtype*)((uchar*)src + j*srcstep);\
                                                                                \
                for( k = 0; k <= size.width - 4; k += 4 )                       \
                    s += tsrc1[k]*tsrc2[k] + tsrc1[k+1]*tsrc2[k+1] +            \
                         tsrc1[k+2]*tsrc2[k+2] + tsrc1[k+3]*tsrc2[k+3];         \
                for( ; k < size.width; k++ )                                    \
                    s += tsrc1[k] * tsrc2[k];                                   \
                tdst[j] = (arrtype)s;                                           \
            }                                                                   \
    else                                                                        \
    {                                                                           \
        arrtype* row_buf = 0;                                                   \
        int local_alloc = 0;                                                    \
        int buf_size = size.width*sizeof(arrtype);                              \
                                                                                \
        if( buf_size <= CX_MAX_LOCAL_SIZE )                                     \
        {                                                                       \
            row_buf = (arrtype*)alloca( buf_size );                             \
            local_alloc = 1;                                                    \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            row_buf = (arrtype*)cxAlloc( buf_size );                            \
            if( !row_buf )                                                      \
                return CX_OUTOFMEM_ERR;                                         \
        }                                                                       \
                                                                                \
        for( i = 0; i < size.height; i++, (char*&)tdst += dststep )             \
        {                                                                       \
            const arrtype *tsrc1 =(const arrtype*)((uchar*)src + i*srcstep);    \
            const arrtype *tdelta1 =(const arrtype*)((uchar*)delta+i*deltastep);\
                                                                                \
            for( k = 0; k < size.width; k++ )                                   \
                row_buf[k] = tsrc1[k] - tdelta1[k];                             \
                                                                                \
            for( j = i; j < size.height; j++ )                                  \
            {                                                                   \
                double s = 0;                                                   \
                const arrtype *tsrc2 =                                          \
                    (const arrtype*)((uchar*)src + j*srcstep);                  \
                const arrtype *tdelta2 =                                        \
                    (const arrtype*)((uchar*)delta + j*deltastep);              \
                                                                                \
                for( k = 0; k <= size.width - 4; k += 4 )                       \
                    s += row_buf[k]*(tsrc2[k] - tdelta2[k]) +                   \
                         row_buf[k+1]*(tsrc2[k+1] - tdelta2[k+1]) +             \
                         row_buf[k+2]*(tsrc2[k+2] - tdelta2[k+2]) +             \
                         row_buf[k+3]*(tsrc2[k+3] - tdelta2[k+3]);              \
                for( ; k < size.width; k++ )                                    \
                    s += row_buf[k]*(tsrc2[k] - tdelta2[k]);                    \
                tdst[j] = (arrtype)s;                                           \
            }                                                                   \
        }                                                                       \
                                                                                \
        if( row_buf && !local_alloc )                                           \
            cxFree( (void**)&row_buf );                                         \
    }                                                                           \
                                                                                \
    /* fill the lower part of the destination matrix */                         \
    for( j = 0; j < size.height - 1; j++ )                                      \
        for( i = j; i < size.height; i++ )                                      \
            ((arrtype*)((uchar*)dst + dststep*i))[j] =                          \
                ((arrtype*)((uchar*)dst + dststep*j))[i];                       \
                                                                                \
    return CX_NO_ERR;                                                           \
}

ICX_DEF_MULTRANS_R_FUNC( 32f, float )
ICX_DEF_MULTRANS_R_FUNC( 64f, double )
ICX_DEF_MULTRANS_L_FUNC( 32f, float )
ICX_DEF_MULTRANS_L_FUNC( 64f, double )

static void icxInitMulTransposedTable( CxFuncTable* tabL, CxFuncTable* tabR )   \
{                                                                               \
    tabL->fn_2d[CX_32F] = (void*)icxMulTransposedL_32f;                         \
    tabL->fn_2d[CX_64F] = (void*)icxMulTransposedL_64f;                         \
    tabR->fn_2d[CX_32F] = (void*)icxMulTransposedR_32f;                         \
    tabR->fn_2d[CX_64F] = (void*)icxMulTransposedR_64f;                         \
}

typedef CxStatus (CX_STDCALL * CxMulTransposedFunc)( const void* src, int srcstep,
            void* dst, int dststep, const void* delta, int deltastep, CxSize size );

CX_IMPL void
cxMulTransposed( const CxArr* srcarr, CxArr* dstarr,
                 int order, const CxArr* deltaarr )
{
    static CxFuncTable tab[2];
    static int inittab = 0;

    CX_FUNCNAME( "cxMulTransposed" );

    __BEGIN__;

    CxMat sstub, *src = (CxMat*)srcarr;
    CxMat dstub, *dst = (CxMat*)dstarr;
    CxMat deltastub, *delta = (CxMat*)deltaarr;
    CxMulTransposedFunc func = 0;
    int type;

    if( !inittab )
    {
        icxInitMulTransposedTable( tab + 0, tab + 1 );
        inittab = 1;
    }

    if( !CX_IS_MAT( src ))
        CX_CALL( src = cxGetMat( src, &sstub ));

    if( !CX_IS_MAT( dst ))
        CX_CALL( dst = cxGetMat( dst, &dstub ));

    if( !CX_ARE_TYPES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( delta )
    {
        if( !CX_IS_MAT( delta ))
            CX_CALL( delta = cxGetMat( delta, &deltastub ));

        if( !CX_ARE_TYPES_EQ( src, delta ))
            CX_ERROR( CX_StsUnmatchedFormats, "" );

        if( (delta->rows != src->rows && delta->rows != 1) || delta->cols != src->cols )
            CX_ERROR( CX_StsUnmatchedSizes, "" );
    }
    else
    {
        delta = &deltastub;
        delta->data.ptr = 0;
        delta->step = 0;
    }

    type = CX_MAT_TYPE( src->type );

    if( dst->rows != dst->cols )
        CX_ERROR( CX_StsBadSize, "The destination matrix must be square" );

    if( (order != 0 && src->cols != dst->cols) ||
        (order == 0 && src->rows != dst->rows))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    if( src->data.ptr == dst->data.ptr ) // inplace operation
        CX_ERROR( CX_StsBadArg, "Inplace operation is not allowed" );

    func = (CxMulTransposedFunc)(tab[order != 0].fn_2d[CX_MAT_DEPTH(type)]);

    if( !func )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src->data.ptr, src->step, dst->data.ptr, dst->step,
                     delta->data.ptr, delta->step, cxGetMatSize( src ) ));

    CX_CHECK_NANS( dst );

    __END__;
}


/****************************************************************************************\
*                                        cxDotProduct                                    *
\****************************************************************************************/

#define ICX_DEF_DOT_PROD_FUNC_2D( flavor, arrtype, temptype, sumtype )                  \
IPCXAPI_IMPL( CxStatus,                                                                 \
icxDotProduct_##flavor##_C1R, ( const arrtype* src1, int step1,                         \
                                const arrtype* src2, int step2,                         \
                                CxSize size, sumtype* _sum ))                           \
{                                                                                       \
    sumtype sum = 0;                                                                    \
                                                                                        \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2 )                \
    {                                                                                   \
        int i;                                                                          \
                                                                                        \
        for( i = 0; i <= size.width - 4; i += 4 )                                       \
        {                                                                               \
            (sum) += (temptype)src1[i]*src2[i] + (temptype)src1[i + 1]*src2[i + 1] +    \
                (temptype)src1[i + 2]*src2[i + 2] + (temptype)src1[i + 3]*src2[i + 3];  \
        }                                                                               \
                                                                                        \
        for( ; i < size.width; i++ )                                                    \
        {                                                                               \
            (sum) += (temptype)src1[i]*src2[i];                                         \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    *_sum = sum;                                                                        \
    return CX_OK;                                                                       \
}


ICX_DEF_DOT_PROD_FUNC_2D( 8u, uchar, int, int64 )
ICX_DEF_DOT_PROD_FUNC_2D( 16s, short, int64, int64 )
ICX_DEF_DOT_PROD_FUNC_2D( 32s, int, double, double )
ICX_DEF_DOT_PROD_FUNC_2D( 32f, float, double, double )
ICX_DEF_DOT_PROD_FUNC_2D( 64f, double, double, double )

#define icxDotProduct_8s_C1R 0

CX_DEF_INIT_FUNC_TAB_2D( DotProduct, C1R )

CX_IMPL double
cxDotProduct( const CxArr* srcAarr, const CxArr* srcBarr )
{
    static CxFuncTable tab_2d;
    static int inittab = 0;

    double result = 0;
    
    CX_FUNCNAME( "cxTranspose" );

    __BEGIN__;

    CxMat stubA, *srcA = (CxMat*)srcAarr;
    CxMat stubB, *srcB = (CxMat*)srcBarr;
    CxSize size;
    int type, depth;

    if( !inittab )
    {
        icxInitDotProductC1RTable( &tab_2d );
        inittab = 1;
    }

    if( !CX_IS_MAT( srcA ))
    {
        int coi = 0;
        CX_CALL( srcA = cxGetMat( srcA, &stubA, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "coi is not supported" );
    }

    if( srcBarr == srcAarr )
    {
        srcB = srcA; 
    }
    else
    {
        if( !CX_IS_MAT( srcB ))
        {
            int coi = 0;
            CX_CALL( srcB = cxGetMat( srcB, &stubB, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "coi is not supported" );
        }

        if( !CX_ARE_TYPES_EQ( srcA, srcB ))
            CX_ERROR( CX_StsUnmatchedFormats, "" );

        if( !CX_ARE_SIZES_EQ( srcA, srcB ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );
    }

    type = CX_MAT_TYPE( srcA->type );
    size = cxGetMatSize( srcA );

    size.width *= CX_MAT_CN( type );
    depth = CX_MAT_DEPTH( type );

    if( CX_IS_MAT_CONT( srcA->type & srcB->type ))
    {
        size.width *= size.height;

        if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE )
        {
            if( depth == CX_32F )
            {
                float* mA = srcA->data.fl;
                float* mB = srcB->data.fl;

                do
                {
                    result += mA[size.width - 1]*mB[size.width - 1];
                }
                while( --size.width );

                EXIT;
            }
            
            if( depth == CX_64F )
            {
                double* mA = srcA->data.db;
                double* mB = srcB->data.db;

                do
                {
                    result += mA[size.width - 1]*mB[size.width - 1];
                }
                while( --size.width );

                EXIT;
            }
        }

        size.height = 1;
    }

    {
        CxFunc2D_2A1P func = (CxFunc2D_2A1P)(tab_2d.fn_2d[depth]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( srcA->data.ptr, srcA->step,
                         srcB->data.ptr, srcB->step,
                         size, &result ));
    }

    if( depth < CX_32S )
        result = (double)(int64&)result;

    /*icxCheckVector_64f( &result, 1 );*/

    __END__;

    return result;
}

/* End of file. */
