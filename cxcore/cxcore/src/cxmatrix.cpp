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
*                                         cxSetIdentity                                  *
\****************************************************************************************/

CX_IMPL void
cxSetIdentity( CxArr* array, CxScalar value )
{
    CX_FUNCNAME( "cxSetIdentity" );

    __BEGIN__;

    CxMat stub, *mat = (CxMat*)array;
    CxSize size;
    int i, len, step;
    int type, pix_size;
    uchar* data = 0;
    double buf[4];

    if( !CX_IS_MAT( mat ))
    {
        int coi = 0;
        CX_CALL( mat = cxGetMat( mat, &stub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "coi is not supported" );
    }

    size = cxGetMatSize( mat );
    len = CX_IMIN( size.width, size.height );

    type = CX_MAT_TYPE(mat->type);
    pix_size = icxPixSize[type];
    size.width *= pix_size;

    if( CX_IS_MAT_CONT( mat->type ))
    {
        size.width *= size.height;
        size.height = 1;
    }

    data = mat->data.ptr;
    step = mat->step;
    IPPI_CALL( icxSetZero_8u_C1R( data, step, size ));
    step += pix_size;

    if( type == CX_32FC1 )
    {
        if( value.val[0] == 1 )
            *((float*)buf) = 1.f;
        else
            cxScalarToRawData( &value, buf, type, 0 );

        for( i = 0; i < len; i++, (char*&)data += step )
            *((float*)data) = *((float*)buf);
    }
    else if( type == CX_64FC1 )
    {
        buf[0] = value.val[0];
        
        for( i = 0; i < len; i++, (char*&)data += step )
            *((double*)data) = buf[0];
    }
    else
    {
        cxScalarToRawData( &value, buf, type, 0 );

        for( i = 0; i < len; i++, (char*&)data += step )
            memcpy( data, buf, pix_size );
    }

    CX_CHECK_NANS( mat );

    __END__;
}


/****************************************************************************************\
*                                          cxTrace                                       *
\****************************************************************************************/

CX_IMPL CxScalar
cxTrace( const CxArr* array )
{
    CxScalar sum = {{ 0, 0, 0, 0 }};
    
    CX_FUNCNAME( "cxTrace" );

    __BEGIN__;

    CxMat stub, *mat = 0;

    if( CX_IS_MAT( array ))
    {
        mat = (CxMat*)array;
        int type = CX_MAT_TYPE(mat->type);
        int size = CX_MIN(mat->rows,mat->cols);
        uchar* data = mat->data.ptr;

        if( type == CX_32FC1 )
        {
            int step = mat->step + sizeof(float);

            for( ; size--; data += step )
                sum.val[0] += *(float*)data;
            EXIT;
        }
        
        if( type == CX_64FC1 )
        {
            int step = mat->step + sizeof(double);

            for( ; size--; data += step )
                sum.val[0] += *(double*)data;
            EXIT;
        }
    }

    CX_CALL( mat = cxGetDiag( array, &stub ));
    CX_CALL( sum = cxSum( mat ));

    __END__;

    return sum;
}


/****************************************************************************************\
*                                        cxTranspose                                     *
\****************************************************************************************/

/////////////////// macros for inplace transposition of square matrix ////////////////////

#define ICX_DEF_TRANSP_INP_CASE_C1( arrtype, arr, step, len )   \
{                                                               \
    arrtype* arr1 = arr;                                        \
                                                                \
    while( --len )                                              \
    {                                                           \
        (char*&)arr += step, arr1++;                            \
        arrtype* arr2 = arr;                                    \
        arrtype* arr3 = arr1;                                   \
                                                                \
        do                                                      \
        {                                                       \
            arrtype t0 = arr2[0];                               \
            arrtype t1 = arr3[0];                               \
            arr2[0] = t1;                                       \
            arr3[0] = t0;                                       \
                                                                \
            arr2++;                                             \
            (char*&)arr3 += step;                               \
        }                                                       \
        while( arr2 != arr3  );                                 \
    }                                                           \
}


#define ICX_DEF_TRANSP_INP_CASE_C3( arrtype, arr, step, len )   \
{                                                               \
    arrtype* arr1 = arr;                                        \
    int y;                                                      \
                                                                \
    for( y = 1; y < len; y++ )                                  \
    {                                                           \
        (char*&)arr += step, arr1 += 3;                         \
        arrtype* arr2 = arr;                                    \
        arrtype* arr3 = arr1;                                   \
                                                                \
        for( ; arr2 != arr3; arr2 += 3, (char*&)arr3 += step )  \
        {                                                       \
            arrtype t0 = arr2[0];                               \
            arrtype t1 = arr3[0];                               \
            arr2[0] = t1;                                       \
            arr3[0] = t0;                                       \
            t0 = arr2[1];                                       \
            t1 = arr3[1];                                       \
            arr2[1] = t1;                                       \
            arr3[1] = t0;                                       \
            t0 = arr2[2];                                       \
            t1 = arr3[2];                                       \
            arr2[2] = t1;                                       \
            arr3[2] = t0;                                       \
        }                                                       \
    }                                                           \
}


#define ICX_DEF_TRANSP_INP_CASE_C4( arrtype, arr, step, len )   \
{                                                               \
    arrtype* arr1 = arr;                                        \
    int y;                                                      \
                                                                \
    for( y = 1; y < len; y++ )                                  \
    {                                                           \
        (char*&)arr += step, arr1 += 4;                         \
        arrtype* arr2 = arr;                                    \
        arrtype* arr3 = arr1;                                   \
                                                                \
        for( ; arr2 != arr3; arr2 += 4, (char*&)arr3 += step )  \
        {                                                       \
            arrtype t0 = arr2[0];                               \
            arrtype t1 = arr3[0];                               \
            arr2[0] = t1;                                       \
            arr3[0] = t0;                                       \
            t0 = arr2[1];                                       \
            t1 = arr3[1];                                       \
            arr2[1] = t1;                                       \
            arr3[1] = t0;                                       \
            t0 = arr2[2];                                       \
            t1 = arr3[2];                                       \
            arr2[2] = t1;                                       \
            arr3[2] = t0;                                       \
            t0 = arr2[3];                                       \
            t1 = arr3[3];                                       \
            arr2[3] = t1;                                       \
            arr3[3] = t0;                                       \
        }                                                       \
    }                                                           \
}


//////////////// macros for non-inplace transposition of rectangular matrix //////////////

#define ICX_DEF_TRANSP_CASE_C1( arrtype, src, srcstep,                              \
                                dst, dststep, size )                                \
{                                                                                   \
    int x, y;                                                                       \
                                                                                    \
    for( y = 0; y <= size.height - 2; y += 2, (char*&)src += 2*srcstep, dst += 2 )  \
    {                                                                               \
        const arrtype* src1 = (const arrtype*)((char*)src + srcstep);               \
        uchar* dst1 = (uchar*)dst;                                                  \
                                                                                    \
        for( x = 0; x <= size.width - 2; x += 2, dst1 += dststep )                  \
        {                                                                           \
            arrtype t0 = src[x];                                                    \
            arrtype t1 = src1[x];                                                   \
            ((arrtype*)dst1)[0] = t0;                                               \
            ((arrtype*)dst1)[1] = t1;                                               \
                                                                                    \
            dst1 += dststep;                                                        \
                                                                                    \
            t0 = src[x + 1];                                                        \
            t1 = src1[x + 1];                                                       \
            ((arrtype*)dst1)[0] = t0;                                               \
            ((arrtype*)dst1)[1] = t1;                                               \
        }                                                                           \
                                                                                    \
        if( x < size.width )                                                        \
        {                                                                           \
            arrtype t0 = src[x];                                                    \
            arrtype t1 = src1[x];                                                   \
            ((arrtype*)dst1)[0] = t0;                                               \
            ((arrtype*)dst1)[1] = t1;                                               \
        }                                                                           \
    }                                                                               \
                                                                                    \
    if( y < size.height )                                                           \
    {                                                                               \
        uchar* dst1 = (uchar*)dst;                                                  \
        for( x = 0; x <= size.width - 2; x += 2, dst1 += 2*dststep )                \
        {                                                                           \
            arrtype t0 = src[x];                                                    \
            arrtype t1 = src[x + 1];                                                \
            ((arrtype*)dst1)[0] = t0;                                               \
            ((arrtype*)(dst1 + dststep))[0] = t1;                                   \
        }                                                                           \
                                                                                    \
        if( x < size.width )                                                        \
        {                                                                           \
            arrtype t0 = src[x];                                                    \
            ((arrtype*)dst1)[0] = t0;                                               \
        }                                                                           \
    }                                                                               \
}


#define ICX_DEF_TRANSP_CASE_C3( arrtype, src, srcstep,                              \
                                dst, dststep, size )                                \
{                                                                                   \
    size.width *= 3;                                                                \
                                                                                    \
    for( ; size.height--; (char*&)src += srcstep, dst++ )                           \
    {                                                                               \
        int x;                                                                      \
        arrtype* dst1 = dst;                                                        \
                                                                                    \
        for( x = 0; x < size.width; x += 3, (char*&)dst1 += dststep )               \
        {                                                                           \
            arrtype t0 = src[x];                                                    \
            arrtype t1 = src[x + 1];                                                \
            arrtype t2 = src[x + 2];                                                \
                                                                                    \
            dst1[0] = t0;                                                           \
            dst1[1] = t1;                                                           \
            dst1[2] = t2;                                                           \
        }                                                                           \
    }                                                                               \
}


#define ICX_DEF_TRANSP_CASE_C4( arrtype, src, srcstep,                              \
                                dst, dststep, size )                                \
{                                                                                   \
    size.width *= 4;                                                                \
                                                                                    \
    for( ; size.height--; (char*&)src += srcstep, dst++ )                           \
    {                                                                               \
        int x;                                                                      \
        arrtype* dst1 = dst;                                                        \
                                                                                    \
        for( x = 0; x < size.width; x += 4, (char*&)dst1 += dststep )               \
        {                                                                           \
            arrtype t0 = src[x];                                                    \
            arrtype t1 = src[x + 1];                                                \
                                                                                    \
            dst1[0] = t0;                                                           \
            dst1[1] = t1;                                                           \
                                                                                    \
            t0 = src[x + 2];                                                        \
            t1 = src[x + 3];                                                        \
                                                                                    \
            dst1[2] = t0;                                                           \
            dst1[3] = t1;                                                           \
        }                                                                           \
    }                                                                               \
}


#define ICX_DEF_TRANSP_INP_FUNC( flavor, arrtype, cn )      \
static CxStatus CX_STDCALL                                  \
icxTranspose_##flavor( arrtype* arr, int step, CxSize size )\
{                                                           \
    assert( size.width == size.height );                    \
                                                            \
    ICX_DEF_TRANSP_INP_CASE_C##cn( arrtype, arr,            \
                                   step, size.width )       \
                                                            \
    return CX_OK;                                           \
}


#define ICX_DEF_TRANSP_FUNC( flavor, arrtype, cn )          \
static CxStatus CX_STDCALL                                  \
icxTranspose_##flavor( const arrtype* src, int srcstep,     \
                    arrtype* dst, int dststep, CxSize size )\
{                                                           \
    ICX_DEF_TRANSP_CASE_C##cn( arrtype, src, srcstep,       \
                               dst, dststep, size )         \
                                                            \
    return CX_OK;                                           \
}


ICX_DEF_TRANSP_INP_FUNC( 8u_C1IR, uchar, 1 )
ICX_DEF_TRANSP_INP_FUNC( 8u_C2IR, ushort, 1 )
ICX_DEF_TRANSP_INP_FUNC( 8u_C3IR, uchar, 3 )
ICX_DEF_TRANSP_INP_FUNC( 16u_C2IR, int, 1 )
ICX_DEF_TRANSP_INP_FUNC( 16u_C3IR, ushort, 3 )
ICX_DEF_TRANSP_INP_FUNC( 32s_C2IR, int64, 1 )
ICX_DEF_TRANSP_INP_FUNC( 32s_C3IR, int, 3 )
ICX_DEF_TRANSP_INP_FUNC( 64s_C2IR, int, 4 )
ICX_DEF_TRANSP_INP_FUNC( 64s_C3IR, int64, 3 )
ICX_DEF_TRANSP_INP_FUNC( 64s_C4IR, int64, 4 )


ICX_DEF_TRANSP_FUNC( 8u_C1R, uchar, 1 )
ICX_DEF_TRANSP_FUNC( 8u_C2R, ushort, 1 )
ICX_DEF_TRANSP_FUNC( 8u_C3R, uchar, 3 )
ICX_DEF_TRANSP_FUNC( 16u_C2R, int, 1 )
ICX_DEF_TRANSP_FUNC( 16u_C3R, ushort, 3 )
ICX_DEF_TRANSP_FUNC( 32s_C2R, int64, 1 )
ICX_DEF_TRANSP_FUNC( 32s_C3R, int, 3 )
ICX_DEF_TRANSP_FUNC( 64s_C2R, int, 4 )
ICX_DEF_TRANSP_FUNC( 64s_C3R, int64, 3 )
ICX_DEF_TRANSP_FUNC( 64s_C4R, int64, 4 )


CX_DEF_INIT_PIXSIZE_TAB_2D( Transpose, R )
CX_DEF_INIT_PIXSIZE_TAB_2D( Transpose, IR )


CX_IMPL void
cxTranspose( const CxArr* srcarr, CxArr* dstarr )
{
    static CxBtFuncTable tab, inp_tab;
    static int inittab = 0;
    
    CX_FUNCNAME( "cxTranspose" );

    __BEGIN__;

    CxMat sstub, *src = (CxMat*)srcarr;
    CxMat dstub, *dst = (CxMat*)dstarr;
    CxSize size;
    int type, pix_size;

    if( !inittab )
    {
        icxInitTransposeIRTable( &inp_tab );
        icxInitTransposeRTable( &tab );
        inittab = 1;
    }

    if( !CX_IS_MAT( src ))
    {
        int coi = 0;
        CX_CALL( src = cxGetMat( src, &sstub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "coi is not supported" );
    }

    type = CX_MAT_TYPE( src->type );
    pix_size = icxPixSize[type];
    size = cxGetMatSize( src );

    if( dstarr == srcarr )
    {
        dst = src; 
    }
    else
    {
        if( !CX_IS_MAT( dst ))
        {
            int coi = 0;
            CX_CALL( dst = cxGetMat( dst, &dstub, &coi ));

            if( coi != 0 )
            CX_ERROR( CX_BadCOI, "coi is not supported" );
        }

        if( !CX_ARE_TYPES_EQ( src, dst ))
            CX_ERROR( CX_StsUnmatchedFormats, "" );

        if( size.width != dst->height || size.height != dst->width )
            CX_ERROR( CX_StsUnmatchedSizes, "" );
    }

    if( src->data.ptr == dst->data.ptr )
    {
        if( size.width == size.height )
        {
            CxFunc2D_1A func = (CxFunc2D_1A)(inp_tab.fn_2d[pix_size]);

            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            IPPI_CALL( func( src->data.ptr, src->step, size ));
        }
        else
        {
            if( size.width != 1 && size.height != 1 )
                CX_ERROR( CX_StsBadSize,
                    "Rectangular matrix can not be transposed inplace" );
            
            if( !CX_IS_MAT_CONT( src->type & dst->type ))
                CX_ERROR( CX_StsBadFlag, "In case of inplace column/row transposition "
                                       "both source and destination must be continuous" );

            if( dst == src )
            {
                int t;
                CX_SWAP( dst->width, dst->height, t );
                dst->step = dst->height == 1 ? 0 : pix_size;
            }
        }
    }
    else
    {
        CxFunc2D_2A func = (CxFunc2D_2A)(tab.fn_2d[pix_size]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, src->step,
                         dst->data.ptr, dst->step, size ));
    }

    CX_CHECK_NANS( dst );

    __END__;
}


/****************************************************************************************\
*              Linear system solution via LU decomposition and related functions         *
\****************************************************************************************/

#define arrtype float
#define temptype double

typedef  CxStatus (CX_STDCALL * CxLUDecompFunc)( void* src, int srcstep, CxSize srcsize,
                                                 void* dst, int dststep, CxSize dstsize,
                                                 double* det );

typedef  CxStatus (CX_STDCALL * CxLUBackFunc)( void* src, int srcstep, CxSize srcsize,
                                               void* dst, int dststep, CxSize dstsize );


#define ICX_DEF_LU_DECOMP_FUNC( flavor, arrtype )                               \
IPCXAPI_IMPL( CxStatus,                                                         \
icxLUDecomp_##flavor, ( arrtype* A, int stepA, CxSize sizeA,                    \
                        arrtype* B, int stepB, CxSize sizeB,                    \
                        double* _det ))                                         \
{                                                                               \
    int n = sizeA.width;                                                        \
    int m = 0, i;                                                               \
    double det = 1;                                                             \
                                                                                \
    assert( sizeA.width == sizeA.height );                                      \
                                                                                \
    if( B )                                                                     \
    {                                                                           \
        assert( sizeA.height == sizeB.height );                                 \
        m = sizeB.width;                                                        \
    }                                                                           \
                                                                                \
    for( i = 0; i < n; i++, (char*&)A += stepA, (char*&)B += stepB )            \
    {                                                                           \
        int j, k = i;                                                           \
        arrtype *tA = A, *tB = 0;                                               \
        arrtype kval = (arrtype)fabs(A[i]);                                     \
        double inv_val;                                                         \
                                                                                \
        /* find the pivot element */                                            \
        for( j = i + 1; j < n; j++ )                                            \
        {                                                                       \
            arrtype tval;                                                       \
            (char*&)tA += stepA;                                                \
                                                                                \
            tval = (arrtype)fabs(tA[i]);                                        \
            if( tval > kval )                                                   \
            {                                                                   \
                kval = tval;                                                    \
                k = j;                                                          \
            }                                                                   \
        }                                                                       \
                                                                                \
        if( kval == 0 )                                                         \
        {                                                                       \
            det = 0;                                                            \
            break;                                                              \
        }                                                                       \
                                                                                \
        /* swap rows */                                                         \
        if( k != i )                                                            \
        {                                                                       \
            tA = (arrtype*)((char*)A + stepA*(k - i));                          \
            det = -det;                                                         \
                                                                                \
            for( j = i; j < n; j++ )                                            \
            {                                                                   \
                arrtype t;                                                      \
                CX_SWAP( A[j], tA[j], t );                                      \
            }                                                                   \
                                                                                \
            if( m > 0 )                                                         \
            {                                                                   \
                tB = (arrtype*)((char*)B + stepB*(k - i));                      \
                                                                                \
                for( j = 0; j < m; j++ )                                        \
                {                                                               \
                    arrtype t = B[j];                                           \
                    CX_SWAP( B[j], tB[j], t );                                  \
                }                                                               \
            }                                                                   \
        }                                                                       \
                                                                                \
        inv_val = 1./A[i];                                                      \
        det *= A[i];                                                            \
        tA = A;                                                                 \
        tB = B;                                                                 \
        A[i] = (arrtype)inv_val;                                                \
                                                                                \
        /* update matrix and the right side of the system */                    \
        for( j = i + 1; j < n; j++ )                                            \
        {                                                                       \
            double alpha;                                                       \
                                                                                \
            (char*&)tA += stepA;                                                \
            (char*&)tB += stepB;                                                \
                                                                                \
            alpha = -tA[i]*inv_val;                                             \
                                                                                \
            for( k = i + 1; k < n; k++ )                                        \
                tA[k] = (arrtype)(tA[k] + alpha*A[k]);                          \
                                                                                \
            if( m > 0 )                                                         \
                for( k = 0; k < m; k++ )                                        \
                    tB[k] = (arrtype)(tB[k] + alpha*B[k]);                      \
        }                                                                       \
    }                                                                           \
                                                                                \
    if( _det )                                                                  \
        *_det = det;                                                            \
                                                                                \
    return CX_OK;                                                               \
}


ICX_DEF_LU_DECOMP_FUNC( 32f, float )
ICX_DEF_LU_DECOMP_FUNC( 64f, double )


#define ICX_DEF_LU_BACK_FUNC( flavor, arrtype )                                 \
IPCXAPI_IMPL( CxStatus,                                                         \
icxLUBack_##flavor, ( arrtype* A, int stepA, CxSize sizeA,                      \
                      arrtype* B, int stepB, CxSize sizeB ))                    \
{                                                                               \
    int n = sizeA.width;                                                        \
    int m = sizeB.width, i;                                                     \
                                                                                \
    assert( m > 0 && sizeA.width == sizeA.height &&                             \
            sizeA.height == sizeB.height );                                     \
                                                                                \
    (char*&)A += stepA*(n - 1);                                                 \
    (char*&)B += stepB*(n - 1);                                                 \
                                                                                \
    for( i = n - 1; i >= 0; i--, (char*&)A -= stepA )                           \
    {                                                                           \
        int j, k;                                                               \
                                                                                \
        for( j = 0; j < m; j++ )                                                \
        {                                                                       \
            arrtype* tB = B + j;                                                \
            double x = 0;                                                       \
                                                                                \
            for( k = n - 1; k > i; k--, (char*&)tB -= stepB )                   \
                x += A[k]*tB[0];                                                \
                                                                                \
            tB[0] = (arrtype)((tB[0] - x)*A[i]);                                \
        }                                                                       \
    }                                                                           \
                                                                                \
    return CX_OK;                                                               \
}


ICX_DEF_LU_BACK_FUNC( 32f, float )
ICX_DEF_LU_BACK_FUNC( 64f, double )

static CxFuncTable lu_decomp_tab, lu_back_tab;
static int lu_inittab = 0;

static void icxInitLUTable( CxFuncTable* decomp_tab,
                            CxFuncTable* back_tab )
{
    decomp_tab->fn_2d[0] = (void*)icxLUDecomp_32f;
    decomp_tab->fn_2d[1] = (void*)icxLUDecomp_64f;
    back_tab->fn_2d[0] = (void*)icxLUBack_32f;
    back_tab->fn_2d[1] = (void*)icxLUBack_64f;
}



/****************************************************************************************\
*                                     Determinant                                        *
\****************************************************************************************/

#define det2(m)   (m(0,0)*m(1,1) - m(0,1)*m(1,0))
#define det3(m)   (m(0,0)*(m(1,1)*m(2,2) - m(1,2)*m(2,1)) -  \
                   m(0,1)*(m(1,0)*m(2,2) - m(1,2)*m(2,0)) +  \
                   m(0,2)*(m(1,0)*m(2,1) - m(1,1)*m(2,0)))

CX_IMPL double
cxDet( const CxArr* arr )
{
    double result = 0;
    uchar* buffer = 0;
    int local_alloc = 0;
    
    CX_FUNCNAME( "cxDet" );

    __BEGIN__;

    CxMat stub, *mat = (CxMat*)arr;
    int type;

    if( !CX_IS_MAT( mat ))
    {
        CX_CALL( mat = cxGetMat( mat, &stub ));
    }

    type = CX_MAT_TYPE( mat->type );

    if( mat->width != mat->height )
        CX_ERROR( CX_StsBadSize, "The matrix must be square" );

    #define Mf( y, x ) ((float*)(m + y*step))[x]
    #define Md( y, x ) ((double*)(m + y*step))[x]

    if( mat->width == 2 )
    {
        uchar* m = mat->data.ptr;
        int step = mat->step;

        if( type == CX_32FC1 )
        {
            result = det2(Mf);
        }
        else if( type == CX_64FC1 )
        {
            result = det2(Md);
        }
        else
        {
            CX_ERROR( CX_StsUnsupportedFormat, "" );
        }
    }
    else if( mat->width == 3 )
    {
        uchar* m = mat->data.ptr;
        int step = mat->step;
        
        if( type == CX_32FC1 )
        {
            result = det3(Mf);
        }
        else if( type == CX_64FC1 )
        {
            result = det3(Md);
        }
        else
        {
            CX_ERROR( CX_StsUnsupportedFormat, "" );
        }
    }
    else if( mat->width == 1 )
    {
        if( type == CX_32FC1 )
        {
            result = mat->data.fl[0];
        }
        else if( type == CX_64FC1 )
        {
            result = mat->data.db[0];
        }
        else
        {
            CX_ERROR( CX_StsUnsupportedFormat, "" );
        }
    }
    else
    {
        CxLUDecompFunc decomp_func;
        CxSize size = cxGetMatSize( mat );
        int buf_size = size.width*size.height*icxPixSize[type];
        CxMat tmat;
        
        if( !lu_inittab )
        {
            icxInitLUTable( &lu_decomp_tab, &lu_back_tab );
            lu_inittab = 1;
        }

        if( CX_MAT_CN( type ) != 1 || CX_MAT_DEPTH( type ) < CX_32F )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        if( size.width <= CX_MAX_LOCAL_MAT_SIZE )
        {
            buffer = (uchar*)alloca( buf_size + 8 );
            buffer = (uchar*)cxAlignPtr( buffer, 8 );
            local_alloc = 1;
        }
        else
        {
            CX_CALL( buffer = (uchar*)cxAlloc( buf_size ));
        }

        CX_CALL( cxInitMatHeader( &tmat, size.height, size.width, type, buffer ));
        CX_CALL( cxCopy( mat, &tmat ));
        
        decomp_func = (CxLUDecompFunc)(lu_decomp_tab.fn_2d[CX_MAT_DEPTH(type)-CX_32F]);
        assert( decomp_func );

        IPPI_CALL( decomp_func( tmat.data.ptr, tmat.step, size, 0, 0, size, &result ));
    }

    #undef Mf
    #undef Md

    /*icxCheckVector_64f( &result, 1 );*/

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );

    return result;
}



/****************************************************************************************\
*                                     Inverse Matrix                                     *
\****************************************************************************************/

#define Sf( y, x ) ((float*)(srcdata + y*srcstep))[x]
#define Sd( y, x ) ((double*)(srcdata + y*srcstep))[x]
#define Df( y, x ) ((float*)(dstdata + y*dststep))[x]
#define Dd( y, x ) ((double*)(dstdata + y*dststep))[x]

CX_IMPL double
cxInvert( const CxArr* srcarr, CxArr* dstarr, int method )
{
    CxMat* u = 0;
    CxMat* v = 0;
    CxMat* w = 0;

    uchar* buffer = 0;
    int local_alloc = 0;
    double result = 0;
    
    CX_FUNCNAME( "cxInvert" );

    __BEGIN__;

    CxMat sstub, *src = (CxMat*)srcarr;
    CxMat dstub, *dst = (CxMat*)dstarr;
    int type;

    if( !CX_IS_MAT( src ))
        CX_CALL( src = cxGetMat( src, &sstub ));

    if( !CX_IS_MAT( dst ))
        CX_CALL( dst = cxGetMat( dst, &dstub ));

    type = CX_MAT_TYPE( src->type );

    if( method == CX_SVD )
    {
        int n = MIN(src->rows,src->cols);

        CX_CALL( u = cxCreateMat( n, src->rows, src->type ));
        CX_CALL( v = cxCreateMat( n, src->cols, src->type ));
        CX_CALL( w = cxCreateMat( n, 1, src->type ));
        CX_CALL( cxSVD( src, w, u, v, CX_SVD_U_T + CX_SVD_V_T ));
        CX_CALL( cxSVBkSb( w, u, v, 0, dst, CX_SVD_U_T + CX_SVD_V_T ));

        if( type == CX_32FC1 )
            result = w->data.fl[0] >= FLT_EPSILON ?
                     w->data.fl[w->rows-1]/w->data.fl[0] : 0;
        else
            result = w->data.db[0] >= FLT_EPSILON ?
                     w->data.db[w->rows-1]/w->data.db[0] : 0;

        CX_CALL( cxSVBkSb( w, u, v, 0, dst, CX_SVD_U_T + CX_SVD_V_T ));
        EXIT;
    }
    else if( method != CX_LU )
        CX_ERROR( CX_StsBadArg, "Unknown inversion method" );

    if( !CX_ARE_TYPES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( src->width != src->height )
        CX_ERROR( CX_StsBadSize, "The matrix must be square" );

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    if( type != CX_32FC1 && type != CX_64FC1 )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    if( src->width <= 3 )
    {
        uchar* srcdata = src->data.ptr;
        uchar* dstdata = dst->data.ptr;
        int srcstep = src->step;
        int dststep = dst->step;

        if( src->width == 2 )
        {
            if( type == CX_32FC1 )
            {
                double d = det2(Sf);
                if( d != 0. )
                {
                    double t0, t1;
                    result = d;
                    d = 1./d;
                    t0 = Sf(0,0)*d;
                    t1 = Sf(1,1)*d;
                    Df(1,1) = (float)t0;
                    Df(0,0) = (float)t1;
                    t0 = -Sf(0,1)*d;
                    t1 = -Sf(1,0)*d;
                    Df(0,1) = (float)t0;
                    Df(1,0) = (float)t1;
                }
            }
            else
            {
                double d = det2(Sd);
                if( d != 0. )
                {
                    double t0, t1;
                    result = d;
                    d = 1./d;
                    t0 = Sd(0,0)*d;
                    t1 = Sd(1,1)*d;
                    Dd(1,1) = t0;
                    Dd(0,0) = t1;
                    t0 = -Sd(0,1)*d;
                    t1 = -Sd(1,0)*d;
                    Dd(0,1) = t0;
                    Dd(1,0) = t1;
                }
            }
        }
        else if( src->width == 3 )
        {
            if( type == CX_32FC1 )
            {
                double d = det3(Sf);
                if( d != 0. )
                {
                    float t[9];
                    result = d;
                    d = 1./d;

                    t[0] = (float)((Sf(1,1) * Sf(2,2) - Sf(1,2) * Sf(2,1)) * d);
                    t[1] = (float)((Sf(0,2) * Sf(2,1) - Sf(0,1) * Sf(2,2)) * d);
                    t[2] = (float)((Sf(0,1) * Sf(1,2) - Sf(0,2) * Sf(1,1)) * d);
                                  
                    t[3] = (float)((Sf(1,2) * Sf(2,0) - Sf(1,0) * Sf(2,2)) * d);
                    t[4] = (float)((Sf(0,0) * Sf(2,2) - Sf(0,2) * Sf(2,0)) * d);
                    t[5] = (float)((Sf(0,2) * Sf(1,0) - Sf(0,0) * Sf(1,2)) * d);
                                  
                    t[6] = (float)((Sf(1,0) * Sf(2,1) - Sf(1,1) * Sf(2,0)) * d);
                    t[7] = (float)((Sf(0,1) * Sf(2,0) - Sf(0,0) * Sf(2,1)) * d);
                    t[8] = (float)((Sf(0,0) * Sf(1,1) - Sf(0,1) * Sf(1,0)) * d);

                    Df(0,0) = t[0]; Df(0,1) = t[1]; Df(0,2) = t[2];
                    Df(1,0) = t[3]; Df(1,1) = t[4]; Df(1,2) = t[5];
                    Df(2,0) = t[6]; Df(2,1) = t[7]; Df(2,2) = t[8];
                }
            }
            else
            {
                double d = det3(Sd);
                if( d != 0. )
                {
                    double t[9];
                    result = d;
                    d = 1./d;

                    t[0] = (Sd(1,1) * Sd(2,2) - Sd(1,2) * Sd(2,1)) * d;
                    t[1] = (Sd(0,2) * Sd(2,1) - Sd(0,1) * Sd(2,2)) * d;
                    t[2] = (Sd(0,1) * Sd(1,2) - Sd(0,2) * Sd(1,1)) * d;
                           
                    t[3] = (Sd(1,2) * Sd(2,0) - Sd(1,0) * Sd(2,2)) * d;
                    t[4] = (Sd(0,0) * Sd(2,2) - Sd(0,2) * Sd(2,0)) * d;
                    t[5] = (Sd(0,2) * Sd(1,0) - Sd(0,0) * Sd(1,2)) * d;
                           
                    t[6] = (Sd(1,0) * Sd(2,1) - Sd(1,1) * Sd(2,0)) * d;
                    t[7] = (Sd(0,1) * Sd(2,0) - Sd(0,0) * Sd(2,1)) * d;
                    t[8] = (Sd(0,0) * Sd(1,1) - Sd(0,1) * Sd(1,0)) * d;

                    Dd(0,0) = t[0]; Dd(0,1) = t[1]; Dd(0,2) = t[2];
                    Dd(1,0) = t[3]; Dd(1,1) = t[4]; Dd(1,2) = t[5];
                    Dd(2,0) = t[6]; Dd(2,1) = t[7]; Dd(2,2) = t[8];
                }
            }
        }
        else
        {
            assert( src->width == 1 );

            if( type == CX_32FC1 )
            {
                double d = Sf(0,0);
                if( d != 0. )
                {
                    result = d;
                    Df(0,0) = (float)(1./d);
                }
            }
            else
            {
                double d = Sd(0,0);
                if( d != 0. )
                {
                    result = d;
                    Dd(0,0) = 1./d;
                }
            }
        }
    }
    else
    {
        CxLUDecompFunc decomp_func;
        CxLUBackFunc back_func;
        CxSize size = cxGetMatSize( src );
        int buf_size = size.width*size.height*icxPixSize[type];
        CxMat tmat;
        
        if( !lu_inittab )
        {
            icxInitLUTable( &lu_decomp_tab, &lu_back_tab );
            lu_inittab = 1;
        }

        if( size.width <= CX_MAX_LOCAL_MAT_SIZE )
        {
            buffer = (uchar*)alloca( buf_size + 8 );
            buffer = (uchar*)cxAlignPtr( buffer, 8 );
            local_alloc = 1;
        }
        else
        {
            CX_CALL( buffer = (uchar*)cxAlloc( buf_size ));
        }

        CX_CALL( cxInitMatHeader( &tmat, size.height, size.width, type, buffer ));
        CX_CALL( cxCopy( src, &tmat ));
        CX_CALL( cxSetIdentity( dst ));

        decomp_func = (CxLUDecompFunc)(lu_decomp_tab.fn_2d[CX_MAT_DEPTH(type)-CX_32F]);
        back_func = (CxLUBackFunc)(lu_back_tab.fn_2d[CX_MAT_DEPTH(type)-CX_32F]);
        assert( decomp_func && back_func );

        IPPI_CALL( decomp_func( tmat.data.ptr, tmat.step, size,
                                dst->data.ptr, dst->step, size, &result ));

        if( result != 0 )
        {
            IPPI_CALL( back_func( tmat.data.ptr, tmat.step, size,
                                  dst->data.ptr, dst->step, size ));
        }
    }

    if( !result )
        CX_CALL( cxSetZero( dst ));

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );

    if( u || v || w )
    {
        cxReleaseMat( &u );
        cxReleaseMat( &v );
        cxReleaseMat( &w );
    }

    return result;
}

/****************************************************************************************\
*                                  Solving Linear Systems                                *
\****************************************************************************************/

CX_IMPL int
cxSolve( const CxArr* A, const CxArr* b, CxArr* x, int method )
{
    CxMat* u = 0;
    CxMat* v = 0;
    CxMat* w = 0;
    
    uchar* buffer = 0;
    int local_alloc = 0;
    int result = 1;
    
    CX_FUNCNAME( "cxSolve" );

    __BEGIN__;

    CxMat sstub, *src = (CxMat*)A;
    CxMat dstub, *dst = (CxMat*)x;
    CxMat bstub, *src2 = (CxMat*)b;
    int type;

    if( !CX_IS_MAT( src ))
        CX_CALL( src = cxGetMat( src, &sstub ));

    if( !CX_IS_MAT( src2 ))
        CX_CALL( src2 = cxGetMat( src2, &bstub ));

    if( !CX_IS_MAT( dst ))
        CX_CALL( dst = cxGetMat( dst, &dstub ));

    if( method == CX_SVD )
    {
        int n = MIN(src->rows,src->cols);
        CX_CALL( u = cxCreateMat( n, src->rows, src->type ));
        CX_CALL( v = cxCreateMat( n, src->cols, src->type ));
        CX_CALL( w = cxCreateMat( n, 1, src->type ));
        CX_CALL( cxSVD( src, w, u, v, CX_SVD_U_T + CX_SVD_V_T ));
        CX_CALL( cxSVBkSb( w, u, v, src2, dst, CX_SVD_U_T + CX_SVD_V_T ));
        EXIT;
    }
    else if( method != CX_LU )
        CX_ERROR( CX_StsBadArg, "Unknown inversion method" );

    type = CX_MAT_TYPE( src->type );

    if( !CX_ARE_TYPES_EQ( src, dst ) || !CX_ARE_TYPES_EQ( src, src2 ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( src->width != src->height )
        CX_ERROR( CX_StsBadSize, "The matrix must be square" );

    if( !CX_ARE_SIZES_EQ( src2, dst ) || src->width != src2->height )
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    if( type != CX_32FC1 && type != CX_64FC1 )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    // check case of a single equation and small matrix
    if( src->width <= 3 && src2->width == 1 )
    {
        #define bf(y) ((float*)(bdata + y*src2step))[0]
        #define bd(y) ((double*)(bdata + y*src2step))[0]

        uchar* srcdata = src->data.ptr;
        uchar* bdata = src2->data.ptr;
        uchar* dstdata = dst->data.ptr;
        int srcstep = src->step;
        int src2step = src2->step;
        int dststep = dst->step;

        if( src->width == 2 )
        {
            if( type == CX_32FC1 )
            {
                double d = det2(Sf);
                if( d != 0. )
                {
                    float t;
                    d = 1./d;
                    t = (float)((bf(0)*Sf(1,1) - bf(1)*Sf(0,1))*d);
                    Df(0,1) = (float)((bf(1)*Sf(0,0) - bf(0)*Sf(1,0))*d);
                    Df(0,0) = t;
                }
                else
                    result = 0;
            }
            else
            {
                double d = det2(Sd);
                if( d != 0. )
                {
                    double t;
                    d = 1./d;
                    t = (float)((bd(0)*Sd(1,1) - bd(1)*Sd(0,1))*d);
                    Dd(1,0) = (float)((bd(1)*Sd(0,0) - bd(0)*Sd(1,0))*d);
                    Dd(0,0) = t;
                }
                else
                    result = 0;
            }
        }
        else if( src->width == 3 )
        {
            if( type == CX_32FC1 )
            {
                double d = det3(Sf);
                if( d != 0. )
                {
                    float t[3];
                    d = 1./d;

                    t[0] = (float)(d*
                           (bf(0)*(Sf(1,1)*Sf(2,2) - Sf(1,2)*Sf(2,1)) -
                            Sf(0,1)*(bf(1)*Sf(2,2) - Sf(1,2)*bf(2)) +
                            Sf(0,2)*(bf(1)*Sf(2,1) - Sf(1,1)*bf(2))));

                    t[1] = (float)(d*
                           (Sf(0,0)*(bf(1)*Sf(2,2) - Sf(1,2)*bf(2)) -
                            bf(0)*(Sf(1,0)*Sf(2,2) - Sf(1,2)*Sf(2,0)) +
                            Sf(0,2)*(Sf(1,0)*bf(2) - bf(1)*Sf(2,0))));

                    t[2] = (float)(d*
                           (Sf(0,0)*(Sf(1,1)*bf(2) - bf(1)*Sf(2,1)) -
                            Sf(0,1)*(Sf(1,0)*bf(2) - bf(1)*Sf(2,0)) +
                            bf(0)*(Sf(1,0)*Sf(2,1) - Sf(1,1)*Sf(2,0))));

                    Df(0,0) = t[0];
                    Df(1,0) = t[1];
                    Df(2,0) = t[2];
                }
                else
                    result = 0;
            }
            else
            {
                double d = det3(Sd);
                if( d != 0. )
                {
                    double t[9];

                    d = 1./d;
                    
                    t[0] = ((Sd(1,1) * Sd(2,2) - Sd(1,2) * Sd(2,1))*bd(0) +
                            (Sd(0,2) * Sd(2,1) - Sd(0,1) * Sd(2,2))*bd(1) +
                            (Sd(0,1) * Sd(1,2) - Sd(0,2) * Sd(1,1))*bd(2))*d;

                    t[1] = ((Sd(1,2) * Sd(2,0) - Sd(1,0) * Sd(2,2))*bd(0) +
                            (Sd(0,0) * Sd(2,2) - Sd(0,2) * Sd(2,0))*bd(1) +
                            (Sd(0,2) * Sd(1,0) - Sd(0,0) * Sd(1,2))*bd(2))*d;

                    t[2] = ((Sd(1,0) * Sd(2,1) - Sd(1,1) * Sd(2,0))*bd(0) +
                            (Sd(0,1) * Sd(2,0) - Sd(0,0) * Sd(2,1))*bd(1) +
                            (Sd(0,0) * Sd(1,1) - Sd(0,1) * Sd(1,0))*bd(2))*d;

                    Dd(0,0) = t[0];
                    Dd(1,0) = t[1];
                    Dd(2,0) = t[2];
                }
                else
                    result = 0;
            }
        }
        else
        {
            assert( src->width == 1 );

            if( type == CX_32FC1 )
            {
                double d = Sf(0,0);
                if( d != 0. )
                    Df(0,0) = (float)(bf(0)/d);
                else
                    result = 0;
            }
            else
            {
                double d = Sd(0,0);
                if( d != 0. )
                    Dd(0,0) = (bd(0)/d);
                else
                    result = 0;
            }
        }
    }
    else
    {
        CxLUDecompFunc decomp_func;
        CxLUBackFunc back_func;
        CxSize size = cxGetMatSize( src );
        CxSize dstsize = cxGetMatSize( dst );
        int buf_size = size.width*size.height*icxPixSize[type];
        double d = 0;
        CxMat tmat;
        
        if( !lu_inittab )
        {
            icxInitLUTable( &lu_decomp_tab, &lu_back_tab );
            lu_inittab = 1;
        }

        if( size.width <= CX_MAX_LOCAL_MAT_SIZE )
        {
            buffer = (uchar*)alloca( buf_size + 8 );
            buffer = (uchar*)cxAlignPtr( buffer, 8 );
            local_alloc = 1;
        }
        else
        {
            CX_CALL( buffer = (uchar*)cxAlloc( buf_size ));
        }

        CX_CALL( cxInitMatHeader( &tmat, size.height, size.width, type, buffer ));
        CX_CALL( cxCopy( src, &tmat ));
        
        if( src2->data.ptr != dst->data.ptr )
        {
            CX_CALL( cxCopy( src2, dst ));
        }

        decomp_func = (CxLUDecompFunc)(lu_decomp_tab.fn_2d[CX_MAT_DEPTH(type)-CX_32F]);
        back_func = (CxLUBackFunc)(lu_back_tab.fn_2d[CX_MAT_DEPTH(type)-CX_32F]);
        assert( decomp_func && back_func );

        IPPI_CALL( decomp_func( tmat.data.ptr, tmat.step, size,
                                dst->data.ptr, dst->step, dstsize, &d ));

        if( d != 0 )
        {
            IPPI_CALL( back_func( tmat.data.ptr, tmat.step, size,
                                  dst->data.ptr, dst->step, dstsize ));
        }
        else
            result = 0;
    }

    if( !result )
        CX_CALL( cxSetZero( dst ));

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );

    if( u || v || w )
    {
        cxReleaseMat( &u );
        cxReleaseMat( &v );
        cxReleaseMat( &w );
    }

    return result;
}


CX_IMPL void
cxCrossProduct( const CxArr* srcAarr, const CxArr* srcBarr, CxArr* dstarr )
{
    CX_FUNCNAME( "cxCrossProduct" );
    
    __BEGIN__;

    CxMat stubA, *srcA = (CxMat*)srcAarr;
    CxMat stubB, *srcB = (CxMat*)srcBarr;
    CxMat dstub, *dst = (CxMat*)dstarr;
    int type;

    if( !CX_IS_MAT(srcA))
        CX_CALL( srcA = cxGetMat( srcA, &stubA ));

    type = CX_MAT_TYPE( srcA->type );

    if( !CX_IS_MAT_CONT(srcA->type) || srcA->width*srcA->height*CX_MAT_CN(type) != 3 )
        CX_ERROR( CX_StsBadArg, "All the input arrays must be continuous 3-vectors" );

    if( !srcB || !dst )
        CX_ERROR( CX_StsNullPtr, "" );

    if( srcA->type == srcB->type && srcA->type == dst->type )
    {
        if( !srcB->data.ptr || !dst->data.ptr )
            CX_ERROR( CX_StsNullPtr, "" );
    }
    else
    {
        if( !CX_IS_MAT(srcB))
            CX_CALL( srcB = cxGetMat( srcB, &stubB ));

        if( !CX_IS_MAT(dst))
            CX_CALL( dst = cxGetMat( dst, &dstub ));

        if( !CX_ARE_TYPES_EQ( srcA, srcB ) ||
            !CX_ARE_TYPES_EQ( srcB, dst ))
            CX_ERROR( CX_StsUnmatchedFormats, "" );

        if( !CX_IS_MAT_CONT( srcB->type & dst->type ))
            CX_ERROR( CX_StsBadArg, "All the input arrays must be continuous 3-vectors" );
    }

    if( !CX_ARE_SIZES_EQ( srcA, srcB ) || !CX_ARE_SIZES_EQ( srcB, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    if( CX_MAT_DEPTH(type) == CX_32F )
    {
        float* dstdata = (float*)(dst->data.ptr);
        const float* src1data = (float*)(srcA->data.ptr);
        const float* src2data = (float*)(srcB->data.ptr);
        
        dstdata[2] = src1data[0] * src2data[1] - src1data[1] * src2data[0];
        dstdata[0] = src1data[1] * src2data[2] - src1data[2] * src2data[1];
        dstdata[1] = src1data[2] * src2data[0] - src1data[0] * src2data[2];
    }
    else if( CX_MAT_DEPTH(type) == CX_64F )
    {
        double* dstdata = (double*)(dst->data.ptr);
        const double* src1data = (double*)(srcA->data.ptr);
        const double* src2data = (double*)(srcB->data.ptr);
        
        dstdata[2] = src1data[0] * src2data[1] - src1data[1] * src2data[0];
        dstdata[0] = src1data[1] * src2data[2] - src1data[2] * src2data[1];
        dstdata[1] = src1data[2] * src2data[0] - src1data[0] * src2data[2];
    }
    else
    {
        CX_ERROR( CX_StsUnsupportedFormat, "" );
    }

    CX_CHECK_NANS( srcA );
    CX_CHECK_NANS( srcB );
    CX_CHECK_NANS( dst );

    __END__;
}

/* End of file. */
