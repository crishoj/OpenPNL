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

/* ////////////////////////////////////////////////////////////////////
//
//  CxMat logical operations: &, |, ^ ...
//
// */

#include "_cxcore.h"

/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                             Macros for logic operations                             //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////

/* //////////////////////////////////////////////////////////////////////////////////////
                                Mat op Mat
////////////////////////////////////////////////////////////////////////////////////// */


#define ICX_DEF_BIN_LOG_OP_2D( __op__, name )                                       \
IPCXAPI_IMPL( CxStatus, icx##name##_8u_C1R,                                         \
( const uchar* src1, int step1, const uchar* src2, int step2,                       \
  uchar* dst, int step, CxSize size ))                                              \
{                                                                                   \
    for( ; size.height--; src1 += step1, src2 += step2, dst += step )               \
    {                                                                               \
        int i;                                                                      \
                                                                                    \
        for( i = 0; i <= size.width - 16; i += 16 )                                 \
        {                                                                           \
            int t0 = __op__(((const int*)(src1+i))[0], ((const int*)(src2+i))[0]);  \
            int t1 = __op__(((const int*)(src1+i))[1], ((const int*)(src2+i))[1]);  \
                                                                                    \
            ((int*)(dst+i))[0] = t0;                                                \
            ((int*)(dst+i))[1] = t1;                                                \
                                                                                    \
            t0 = __op__(((const int*)(src1+i))[2], ((const int*)(src2+i))[2]);      \
            t1 = __op__(((const int*)(src1+i))[3], ((const int*)(src2+i))[3]);      \
                                                                                    \
            ((int*)(dst+i))[2] = t0;                                                \
            ((int*)(dst+i))[3] = t1;                                                \
        }                                                                           \
                                                                                    \
        for( ; i <= size.width - 4; i += 4 )                                        \
        {                                                                           \
            int t = __op__(*(const int*)(src1+i), *(const int*)(src2+i));           \
            *(int*)(dst+i) = t;                                                     \
        }                                                                           \
                                                                                    \
        for( ; i < size.width; i++ )                                                \
        {                                                                           \
            int t = __op__(((const uchar*)src1)[i],((const uchar*)src2)[i]);        \
            dst[i] = (uchar)t;                                                      \
        }                                                                           \
    }                                                                               \
                                                                                    \
    return  CX_OK;                                                                  \
}


/* //////////////////////////////////////////////////////////////////////////////////////
                                     Mat op Scalar
////////////////////////////////////////////////////////////////////////////////////// */


#define ICX_DEF_UN_LOG_OP_2D( __op__, name )                                            \
IPCXAPI_IMPL( CxStatus, icx##name##_8u_C1R,                                             \
( const uchar* src0, int step1,                                                         \
  uchar* dst0, int step, CxSize size,                                                   \
  const uchar* scalar, int pix_size ))                                                  \
{                                                                                       \
    int delta = 12*pix_size;                                                            \
                                                                                        \
    for( ; size.height--; src0 += step1, dst0 += step )                                 \
    {                                                                                   \
        const uchar* src = (const uchar*)src0;                                          \
        uchar* dst = dst0;                                                              \
        int i, len = size.width;                                                        \
                                                                                        \
        while( (len -= delta) >= 0 )                                                    \
        {                                                                               \
            for( i = 0; i < (delta); i += 12 )                                          \
            {                                                                           \
                int t0 = __op__(((const int*)(src+i))[0], ((const int*)(scalar+i))[0]); \
                int t1 = __op__(((const int*)(src+i))[1], ((const int*)(scalar+i))[1]); \
                                                                                        \
                ((int*)(dst+i))[0] = t0;                                                \
                ((int*)(dst+i))[1] = t1;                                                \
                                                                                        \
                t0 = __op__(((const int*)(src+i))[2], ((const int*)(scalar+i))[2]);     \
                                                                                        \
                ((int*)(dst+i))[2] = t0;                                                \
            }                                                                           \
            src += delta;                                                               \
            dst += delta;                                                               \
        }                                                                               \
                                                                                        \
        for( len += delta, i = 0; i < len; i++ )                                        \
        {                                                                               \
            int t = __op__(src[i],scalar[i]);                                           \
            dst[i] = (uchar)t;                                                          \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    return CX_OK;                                                                       \
}


/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                             Macros for logic operations with mask                   //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////

/* //////////////////////////////////////////////////////////////////////////////////////
                                    Mat op Mat
////////////////////////////////////////////////////////////////////////////////////// */

#define ICX_DEF_BIN_LOG_OP_MASK( __op__, _mask_op_, arrtype, worktype,          \
                                 src, dst, mask, len, cn )                      \
{                                                                               \
    int i;                                                                      \
    for( i = 0; i <= (len) - 4; i += 4, src += 4*(cn), dst += 4*(cn) )          \
    {                                                                           \
        int k = cn - 1;                                                         \
        int m = 0;                                                              \
                                                                                \
        do                                                                      \
        {                                                                       \
            worktype t0 = (mask)[i] ? -1 : 0;                                   \
            worktype t1 = (mask)[i+1] ? -1 : 0;                                 \
                                                                                \
            t0 = _mask_op_( t0, (src)[k]);                                      \
            t1 = _mask_op_( t1, (src)[k+(cn)]);                                 \
                                                                                \
            t0 = __op__( t0, (dst)[k]);                                         \
            t1 = __op__( t1, (dst)[k+(cn)]);                                    \
                                                                                \
            (dst)[k] = (arrtype)t0;                                             \
            (dst)[k+(cn)] = (arrtype)t1;                                        \
                                                                                \
            t0 = (mask)[i+2] ? -1 : 0;                                          \
            t1 = (mask)[i+3] ? -1 : 0;                                          \
                                                                                \
            t0 = _mask_op_( t0, (src)[k+2*(cn)]);                               \
            t1 = _mask_op_( t1, (src)[k+3*(cn)]);                               \
                                                                                \
            t0 = __op__( t0, (dst)[k+2*(cn)]);                                  \
            t1 = __op__( t1, (dst)[k+3*(cn)]);                                  \
                                                                                \
            (dst)[k+2*(cn)] = (arrtype)t0;                                      \
            (dst)[k+3*(cn)] = (arrtype)t1;                                      \
        }                                                                       \
        while( k-- && (m || (m = (mask[i]|mask[i+1]|mask[i+2]|mask[i+3])) != 0));\
    }                                                                           \
                                                                                \
    for( ; i < (len); i++, src += cn, dst += cn )                               \
    {                                                                           \
        int k = cn - 1;                                                         \
        do                                                                      \
        {                                                                       \
            worktype t = (mask)[i] ? -1 : 0;                                    \
            t = _mask_op_( t, (src)[k] );                                       \
            t = __op__( t, (dst)[k] );                                          \
            (dst)[k] = (arrtype)t;                                              \
        }                                                                       \
        while( k-- && mask[i] != 0 );                                           \
    }                                                                           \
}

#define ICX_DEF_BIN_LOG_OP_MASK_2D( __op__, _mask_op_, name )                           \
static CxStatus CX_STDCALL                                                              \
icx##name##_8u_CnMR( const uchar* src, int step1, const uchar* mask, int step2,         \
                     uchar* dst, int step, CxSize size, int cn )                        \
{                                                                                       \
    for( ; size.height--; (char*&)src += step1, mask += step2, (char*&)dst += step )    \
    {                                                                                   \
        const uchar* tsrc = src;                                                        \
        uchar* tdst = dst;                                                              \
                                                                                        \
        ICX_DEF_BIN_LOG_OP_MASK( __op__, _mask_op_, uchar,                              \
                                 int, tsrc, tdst, mask, size.width, cn )                \
    }                                                                                   \
                                                                                        \
    return CX_OK;                                                                       \
}


/* //////////////////////////////////////////////////////////////////////////////////////
                                     Mat op Scalar
////////////////////////////////////////////////////////////////////////////////////// */


#define ICX_DEF_UN_LOG_OP_MASK( __op__, _mask_op_, arrtype, worktype,           \
                                dst, mask, len, cn )                            \
{                                                                               \
    int i;                                                                      \
    for( i = 0; i <= (len) - 4; i += 4, dst += 4*(cn) )                         \
    {                                                                           \
        int k = cn - 1;                                                         \
        int m = 0;                                                              \
                                                                                \
        do                                                                      \
        {                                                                       \
            arrtype value = scalar[k];                                          \
            worktype t0 = (mask)[i] ? -1 : 0;                                   \
            worktype t1 = (mask)[i+1] ? -1 : 0;                                 \
                                                                                \
            t0 = _mask_op_( t0, value );                                        \
            t1 = _mask_op_( t1, value );                                        \
                                                                                \
            t0 = __op__( t0, (dst)[k]);                                         \
            t1 = __op__( t1, (dst)[k+(cn)]);                                    \
                                                                                \
            (dst)[k] = (arrtype)t0;                                             \
            (dst)[k+(cn)] = (arrtype)t1;                                        \
                                                                                \
            t0 = (mask)[i+2] ? -1 : 0;                                          \
            t1 = (mask)[i+3] ? -1 : 0;                                          \
                                                                                \
            t0 = _mask_op_( t0, value );                                        \
            t1 = _mask_op_( t1, value );                                        \
                                                                                \
            t0 = __op__( t0, (dst)[k+2*(cn)]);                                  \
            t1 = __op__( t1, (dst)[k+3*(cn)]);                                  \
                                                                                \
            (dst)[k+2*(cn)] = (arrtype)t0;                                      \
            (dst)[k+3*(cn)] = (arrtype)t1;                                      \
        }                                                                       \
        while( k-- && (m || (m = (mask[i]|mask[i+1]|mask[i+2]|mask[i+3])) != 0));\
    }                                                                           \
                                                                                \
    for( ; i < (len); i++, dst += cn )                                          \
    {                                                                           \
        int k = cn - 1;                                                         \
        do                                                                      \
        {                                                                       \
            worktype t = (mask)[i] ? -1 : 0;                                    \
            t = _mask_op_( t, scalar[k] );                                      \
            t = __op__( t, (dst)[k] );                                          \
            (dst)[k] = (arrtype)t;                                              \
        }                                                                       \
        while( k-- && mask[i] != 0 );                                           \
    }                                                                           \
}


#define ICX_DEF_UN_LOG_OP_MASK_2D( __op__, _mask_op_, name )                    \
static CxStatus CX_STDCALL                                                      \
icx##name##C_8u_CnMR( uchar* dst, int step, const uchar* mask, int maskstep,    \
                      CxSize size, const uchar* scalar, int cn )                \
{                                                                               \
    for( ; size.height--; mask += maskstep, (char*&)dst += step )               \
    {                                                                           \
        uchar* tdst = dst;                                                      \
                                                                                \
        ICX_DEF_UN_LOG_OP_MASK( __op__, _mask_op_, uchar, int,                  \
                                tdst, mask, size.width, cn )                    \
    }                                                                           \
                                                                                \
    return  CX_OK;                                                              \
}


/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                                LOGIC OPERATIONS                                     //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////

static void
icxLogicS( const void* srcarr, CxScalar* scalar,
           void* dstarr, CxFunc2D_2A1P1I fn_2d )
{
    CX_FUNCNAME( "icxLogicS" );
    
    __BEGIN__;

    CxMat srcstub, *src = (CxMat*)srcarr;
    CxMat dststub, *dst = (CxMat*)dstarr;
    
    int coi1 = 0, coi2 = 0;
    int is_nd = 0;
    int pix_size, type;
    double buf[12];
    CxSize size;
    int src_step, dst_step;

    if( !CX_IS_MAT(src))
    {
        if( CX_IS_MATND(src) )
            is_nd = 1;
        else
            CX_CALL( src = cxGetMat( src, &srcstub, &coi1 ));
    }

    if( !CX_IS_MAT(dst))
    {
        if( CX_IS_MATND(dst) )
            is_nd = 1;
        else
            CX_CALL( dst = cxGetMat( dst, &dststub, &coi2 ));
    }

    if( is_nd )
    {
        CxArr* arrs[] = { src, dst };
        CxMatND stubs[2];
        CxNArrayIterator iterator;

        CX_CALL( cxInitNArrayIterator( 2, arrs, 0, stubs, &iterator ));

        type = CX_MAT_TYPE(iterator.hdr[0]->type);
        iterator.size.width *= icxPixSize[type];
        pix_size = icxPixSize[type & CX_MAT_DEPTH_MASK];

        CX_CALL( cxScalarToRawData( scalar, buf, type, 1 ));

        do
        {
            IPPI_CALL( fn_2d( iterator.ptr[0], CX_STUB_STEP,
                              iterator.ptr[1], CX_STUB_STEP,
                              iterator.size, buf, pix_size ));
        }
        while( cxNextNArraySlice( &iterator ));
        EXIT;
    }

    if( coi1 != 0 || coi2 != 0 )
        CX_ERROR( CX_BadCOI, "" );

    if( !CX_ARE_TYPES_EQ( src, dst ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src, dst ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    size = cxGetMatSize( src );
    src_step = src->step;
    dst_step = dst->step;

    if( CX_IS_MAT_CONT( src->type & dst->type ))
    {
        size.width *= size.height;
        src_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }

    type = CX_MAT_TYPE( src->type );
    size.width *= icxPixSize[type];
    pix_size = icxPixSize[type & CX_MAT_DEPTH_MASK];

    CX_CALL( cxScalarToRawData( scalar, buf, type, 1 ));

    IPPI_CALL( fn_2d( src->data.ptr, src_step, dst->data.ptr, dst_step,
                      size, buf, pix_size ));

    __END__;
}


static void
icxLogic( const void* srcimg1, const void* srcimg2,
          void* dstarr, CxFunc2D_3A fn_2d )
{
    CX_FUNCNAME( "icxLogic" );
    
    __BEGIN__;

    int coi1 = 0, coi2 = 0, coi3 = 0;
    int is_nd = 0;
    CxMat  srcstub1, *src1 = (CxMat*)srcimg1;
    CxMat  srcstub2, *src2 = (CxMat*)srcimg2;
    CxMat  dststub,  *dst = (CxMat*)dstarr;
    int src1_step, src2_step, dst_step;
    CxSize size;

    if( !CX_IS_MAT(src1))
    {
        if( CX_IS_MATND(src1) )
            is_nd = 1;
        else
            CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi1 ));
    }

    if( !CX_IS_MAT(src2))
    {
        if( CX_IS_MATND(src2) )
            is_nd = 1;
        else
            CX_CALL( src2 = cxGetMat( src2, &srcstub2, &coi2 ));
    }

    if( !CX_IS_MAT(dst))
    {
        if( CX_IS_MATND(dst) )
            is_nd = 1;
        else
            CX_CALL( dst = cxGetMat( dst, &dststub, &coi3 ));
    }

    if( is_nd )
    {
        CxArr* arrs[] = { src1, src2, dst };
        CxMatND stubs[3];
        CxNArrayIterator iterator;
        int type;

        CX_CALL( cxInitNArrayIterator( 3, arrs, 0, stubs, &iterator ));

        type = CX_MAT_TYPE(iterator.hdr[0]->type);
        iterator.size.width *= icxPixSize[type];

        do
        {
            IPPI_CALL( fn_2d( iterator.ptr[0], CX_STUB_STEP,
                              iterator.ptr[1], CX_STUB_STEP,
                              iterator.ptr[2], CX_STUB_STEP,
                              iterator.size ));
        }
        while( cxNextNArraySlice( &iterator ));
        EXIT;
    }

    if( coi1 != 0 || coi2 != 0 || coi3 != 0 )
        CX_ERROR_FROM_CODE( CX_BadCOI );

    if( !CX_ARE_TYPES_EQ( src1, src2 ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src1, src2 ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    size = cxGetMatSize( src1 );
        
    if( !CX_ARE_TYPES_EQ( src1, dst ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );
    
    if( !CX_ARE_SIZES_EQ( src1, dst ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    src1_step = src1->step;
    src2_step = src2->step;
    dst_step = dst->step;

    if( CX_IS_MAT_CONT( src1->type & src2->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = src2_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }

    size.width *= icxPixSize[CX_MAT_TYPE(src1->type)];

    IPPI_CALL( fn_2d( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                      dst->data.ptr, dst_step, size ));

    __END__;
}


/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                       LOGIC OPERATIONS WITH MASK SUPPORT                            //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////

static void
icxLogicSM( const void* srcarr, CxScalar* scalar, void* dstarr,
            const void* maskarr, CxArithmUniMaskFunc2D func )
{
    CX_FUNCNAME( "icxLogicSM" );
    
    __BEGIN__;

    double buf[12];
    int coi1 = 0, coi2 = 0;
    CxMat  srcstub, *src = (CxMat*)srcarr;
    CxMat  dststub, *dst = (CxMat*)dstarr;
    CxMat  maskstub, *mask = (CxMat*)maskarr;
    int pix_size, type;
    int dst_step, mask_step;
    CxSize size;

    if( !CX_IS_MAT(src))
        CX_CALL( src = cxGetMat( src, &srcstub, &coi1 ));

    if( !CX_IS_MAT(dst))
        CX_CALL( dst = cxGetMat( dst, &dststub ));

    if( coi1 != 0 || coi2 != 0 )
        CX_ERROR( CX_BadCOI, "" );

    CX_CALL( mask = cxGetMat( mask, &maskstub ));

    if( !CX_IS_MASK_ARR(mask) )
        CX_ERROR_FROM_CODE( CX_StsBadMask );

    if( !CX_ARE_SIZES_EQ( mask, dst ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    if( src->data.ptr != dst->data.ptr )
    {
        CX_CALL( cxCopy( src, dst, mask ));
    }

    size = cxGetMatSize( dst );
    dst_step = dst->step;
    mask_step = mask->step;

    if( CX_IS_MAT_CONT( mask->type & dst->type ))
    {
        size.width *= size.height;
        dst_step = mask_step = CX_STUB_STEP;
        size.height = 1;
    }

    type = CX_MAT_TYPE( src->type );
    pix_size = icxPixSize[type];
    
    CX_CALL( cxScalarToRawData( scalar, buf, type, 0 ));
    IPPI_CALL( func( dst->data.ptr, dst_step, mask->data.ptr,
                     mask_step, size, buf, pix_size ));

    __END__;
}


CX_IMPL void
icxLogicM( const void* srcimg1, const void* srcimg2,
           void* dstarr, const void* maskarr,
           CxArithmBinMaskFunc2D func )
{
    CX_FUNCNAME( "icxLogicM" );
    
    __BEGIN__;

    int  coi1 = 0, coi2 = 0, coi3 = 0;
    CxMat  srcstub1, *src1 = (CxMat*)srcimg1;
    CxMat  srcstub2, *src2 = (CxMat*)srcimg2;
    CxMat  dststub, *dst = (CxMat*)dstarr;
    CxMat  maskstub, *mask = (CxMat*)maskarr;
    int src_step, dst_step, mask_step; 
    int pix_size;
    CxSize size;

    CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi1 ));
    CX_CALL( src2 = cxGetMat( src2, &srcstub2, &coi2 ));
    CX_CALL( dst = cxGetMat( dst, &dststub, &coi3 ));
    CX_CALL( mask = cxGetMat( mask, &maskstub ));

    if( coi1 != 0 || coi2 != 0 || coi3 != 0 )
        CX_ERROR( CX_BadCOI, "" );

    if( !CX_IS_MASK_ARR(mask) )
        CX_ERROR_FROM_CODE( CX_StsBadMask );

    if( !CX_ARE_SIZES_EQ( mask, dst ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    if( !CX_ARE_SIZES_EQ( src1, src2 ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    size = cxGetMatSize( src1 );
    pix_size = icxPixSize[CX_MAT_TYPE(src1->type)];
        
    if( src2->data.ptr != dst->data.ptr )
    {
        if( src1->data.ptr != dst->data.ptr )
        {
            CX_CALL( cxCopy( src2, dst, mask ));
        }
        else
            src1 = src2;
    }
    
    if( !CX_ARE_TYPES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    src_step = src1->step;
    dst_step = dst->step;
    mask_step = mask->step;
    
    if( CX_IS_MAT_CONT( src1->type & dst->type & mask->type ))
    {
        size.width *= size.height;
        src_step = dst_step = mask_step = CX_STUB_STEP;
        size.height = 1;
    }

    IPPI_CALL( func( src1->data.ptr, src_step, mask->data.ptr, mask_step,
                     dst->data.ptr, dst_step, size, pix_size ));

    __END__;
}


#define ICX_DEF_LOG_OP_MASK_2D( __op__, _mask_op_, name )  \
    ICX_DEF_BIN_LOG_OP_MASK_2D( __op__, _mask_op_, name )  \
    ICX_DEF_UN_LOG_OP_MASK_2D( __op__, _mask_op_, name )

ICX_DEF_LOG_OP_MASK_2D( CX_XOR, CX_AND, Xor )
ICX_DEF_LOG_OP_MASK_2D( CX_AND, CX_ORN, And )
ICX_DEF_LOG_OP_MASK_2D( CX_OR,  CX_AND, Or )

ICX_DEF_BIN_LOG_OP_2D( CX_XOR, Xor )
ICX_DEF_UN_LOG_OP_2D( CX_XOR, XorC )

ICX_DEF_BIN_LOG_OP_2D( CX_AND, And )
ICX_DEF_UN_LOG_OP_2D( CX_AND, AndC )

ICX_DEF_BIN_LOG_OP_2D( CX_OR, Or )
ICX_DEF_UN_LOG_OP_2D( CX_OR, OrC )


/////////////////////////////////////////////////////////////////////////////////////////
//                                    X O R                                            //
/////////////////////////////////////////////////////////////////////////////////////////

CX_IMPL void
cxXorS( const void* src, CxScalar scalar, void* dst, const void* mask )
{
    CX_FUNCNAME( "cxXorS" );
    
    __BEGIN__;

    if( !mask )
    {
        CX_CALL( icxLogicS( src, &scalar, dst, (CxFunc2D_2A1P1I)icxXorC_8u_C1R ));
    }
    else
    {
        CX_CALL( icxLogicSM( src, &scalar, dst, mask,
                             (CxArithmUniMaskFunc2D)icxXorC_8u_CnMR ));
    }

    __END__;
}


CX_IMPL void
cxXor( const void* src1, const void* src2, void* dst, const void* mask )
{
    CX_FUNCNAME( "cxXor" );
    
    __BEGIN__;

    if( !mask )
    {
        CX_CALL( icxLogic( src1, src2, dst,
                           (CxFunc2D_3A)icxXor_8u_C1R ));
    }
    else
    {
        CX_CALL( icxLogicM( src1, src2, dst, mask,
                            (CxArithmBinMaskFunc2D)icxXor_8u_CnMR ));
    }

    __END__;
}

/////////////////////////////////////////////////////////////////////////////////////////
//                                    A N D                                            //
/////////////////////////////////////////////////////////////////////////////////////////

CX_IMPL void
cxAndS( const void* src, CxScalar scalar, void* dst, const void* mask )
{
    CX_FUNCNAME( "cxAndS" );
    
    __BEGIN__;

    if( !mask )
    {
        CX_CALL( icxLogicS( src, &scalar, dst, (CxFunc2D_2A1P1I)icxAndC_8u_C1R ));
    }
    else
    {
        CX_CALL( icxLogicSM( src, &scalar, dst, mask,
                             (CxArithmUniMaskFunc2D)icxAndC_8u_CnMR ));
    }

    __END__;
}


CX_IMPL void
cxAnd( const void* src1, const void* src2, void* dst, const void* mask )
{
    CX_FUNCNAME( "cxAnd" );
    
    __BEGIN__;

    if( !mask )
    {
        CX_CALL( icxLogic( src1, src2, dst,
                           (CxFunc2D_3A)icxAnd_8u_C1R ));
    }
    else
    {
        CX_CALL( icxLogicM( src1, src2, dst, mask,
                            (CxArithmBinMaskFunc2D)icxAnd_8u_CnMR ));
    }

    __END__;
}


/////////////////////////////////////////////////////////////////////////////////////////
//                                      O R                                            //
/////////////////////////////////////////////////////////////////////////////////////////

CX_IMPL void
cxOrS( const void* src, CxScalar scalar, void* dst, const void* mask )
{
    CX_FUNCNAME( "cxOrS" );
    
    __BEGIN__;

    if( !mask )
    {
        CX_CALL( icxLogicS( src, &scalar, dst, (CxFunc2D_2A1P1I)icxOrC_8u_C1R ));
    }
    else
    {
        CX_CALL( icxLogicSM( src, &scalar, dst, mask,
                             (CxArithmUniMaskFunc2D)icxOrC_8u_CnMR ));
    }

    __END__;
}


CX_IMPL void
cxOr( const void* src1, const void* src2, void* dst, const void* mask )
{
    CX_FUNCNAME( "cxOr" );
    
    __BEGIN__;

    if( !mask )
    {
        CX_CALL( icxLogic( src1, src2, dst,
                           (CxFunc2D_3A)icxOr_8u_C1R ));
    }
    else
    {
        CX_CALL( icxLogicM( src1, src2, dst, mask,
                            (CxArithmBinMaskFunc2D)icxOr_8u_CnMR ));
    }

    __END__;
}


/////////////////////////////////////////////////////////////////////////////////////////
//                                      N O T                                          //
/////////////////////////////////////////////////////////////////////////////////////////


IPCXAPI_IMPL( CxStatus, icxNot_8u_C1R,
( const uchar* src1, int step1, uchar* dst, int step, CxSize size ))
{
    for( ; size.height--; src1 += step1, dst += step )
    {
        int i;

        for( i = 0; i <= size.width - 16; i += 16 )
        {
            int t0 = ~((const int*)(src1+i))[0];
            int t1 = ~((const int*)(src1+i))[1];

            ((int*)(dst+i))[0] = t0;
            ((int*)(dst+i))[1] = t1;

            t0 = ~((const int*)(src1+i))[2];
            t1 = ~((const int*)(src1+i))[3];

            ((int*)(dst+i))[2] = t0;
            ((int*)(dst+i))[3] = t1;
        }

        for( ; i <= size.width - 4; i += 4 )
        {
            int t = ~*(const int*)(src1+i);
            *(int*)(dst+i) = t;
        }

        for( ; i < size.width; i++ )
        {
            int t = ~((const uchar*)src1)[i];
            dst[i] = (uchar)t;
        }
    }

    return  CX_OK;
}


CX_IMPL void
cxNot( const void* srcarr, void* dstarr )
{
    CX_FUNCNAME( "cxNot" );
    
    __BEGIN__;

    CxMat srcstub, *src = (CxMat*)srcarr;
    CxMat dststub, *dst = (CxMat*)dstarr;
    
    int coi1 = 0, coi2 = 0;
    int type, is_nd = 0;
    CxSize size;
    int src_step, dst_step;

    if( !CX_IS_MAT(src))
    {
        if( CX_IS_MATND(src) )
            is_nd = 1;
        else
            CX_CALL( src = cxGetMat( src, &srcstub, &coi1 ));
    }

    if( !CX_IS_MAT(dst))
    {
        if( CX_IS_MATND(src) )
            is_nd = 1;
        else
            CX_CALL( dst = cxGetMat( dst, &dststub, &coi2 ));
    }

    if( is_nd )
    {
        CxArr* arrs[] = { src, dst };
        CxMatND stubs[2];
        CxNArrayIterator iterator;

        CX_CALL( cxInitNArrayIterator( 2, arrs, 0, stubs, &iterator ));

        type = CX_MAT_TYPE(iterator.hdr[0]->type);
        iterator.size.width *= icxPixSize[type];

        do
        {
            IPPI_CALL( icxNot_8u_C1R( iterator.ptr[0], CX_STUB_STEP,
                                      iterator.ptr[1], CX_STUB_STEP,
                                      iterator.size ));
        }
        while( cxNextNArraySlice( &iterator ));
        EXIT;
    }

    if( coi1 != 0 || coi2 != 0 )
        CX_ERROR( CX_BadCOI, "" );

    if( !CX_ARE_TYPES_EQ( src, dst ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src, dst ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    size = cxGetMatSize( src );
    src_step = src->step;
    dst_step = dst->step;

    if( CX_IS_MAT_CONT( src->type & dst->type ))
    {
        size.width *= size.height;
        src_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }

    type = CX_MAT_TYPE( src->type );
    size.width *= icxPixSize[type];

    IPPI_CALL( icxNot_8u_C1R( src->data.ptr, src_step, dst->data.ptr, dst_step, size ));

    __END__;
}


/* End of file. */
