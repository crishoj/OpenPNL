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
//  CxMat comparison functions: range checking, min, max
//
// */

#include "_cxcore.h"

/****************************************************************************************\
*                                      InRange[S]                                        *
\****************************************************************************************/

#define ICX_DEF_IN_RANGE_CASE_C1( worktype, _toggle_macro_ )    \
for( x = 0; x < size.width; x++ )                               \
{                                                               \
    worktype a1 = _toggle_macro_(src1[x]),                      \
             a2 = src2[x], a3 = src3[x];                        \
    dst[x] = (uchar)-(_toggle_macro_(a2) <= a1 &&               \
                     a1 < _toggle_macro_(a3));                  \
}


#define ICX_DEF_IN_RANGE_CASE_C2( worktype, _toggle_macro_ )        \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*2]),                        \
             a2 = src2[x*2], a3 = src3[x*2];                        \
    int f = _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);    \
    a1 = _toggle_macro_(src1[x*2+1]);                               \
    a2 = src2[x*2+1];                                               \
    a3 = src3[x*2+1];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    dst[x] = (uchar)-f;                                             \
}


#define ICX_DEF_IN_RANGE_CASE_C3( worktype, _toggle_macro_ )        \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*3]),                        \
             a2 = src2[x*3], a3 = src3[x*3];                        \
    int f = _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);    \
    a1 = _toggle_macro_(src1[x*3+1]);                               \
    a2 = src2[x*3+1];                                               \
    a3 = src3[x*3+1];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    a1 = _toggle_macro_(src1[x*3+2]);                               \
    a2 = src2[x*3+2];                                               \
    a3 = src3[x*3+2];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    dst[x] = (uchar)-f;                                             \
}


#define ICX_DEF_IN_RANGE_CASE_C4( worktype, _toggle_macro_ )        \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*4]),                        \
             a2 = src2[x*4], a3 = src3[x*4];                        \
    int f = _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);    \
    a1 = _toggle_macro_(src1[x*4+1]);                               \
    a2 = src2[x*4+1];                                               \
    a3 = src3[x*4+1];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    a1 = _toggle_macro_(src1[x*4+2]);                               \
    a2 = src2[x*4+2];                                               \
    a3 = src3[x*4+2];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    a1 = _toggle_macro_(src1[x*4+3]);                               \
    a2 = src2[x*4+3];                                               \
    a3 = src3[x*4+3];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    dst[x] = (uchar)-f;                                             \
}


#define ICX_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype,           \
                               _toggle_macro_, cn )                 \
static CxStatus CX_STDCALL                                          \
icxInRange_##flavor##_C##cn##R( const arrtype* src1, int step1,     \
                                const arrtype* src2, int step2,     \
                                const arrtype* src3, int step3,     \
                                uchar* dst, int step, CxSize size ) \
{                                                                   \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2, \
                          (char*&)src3 += step3, (char*&)dst += step )  \
    {                                                               \
        int x;                                                      \
        ICX_DEF_IN_RANGE_CASE_C##cn( worktype, _toggle_macro_ )     \
    }                                                               \
                                                                    \
    return CX_OK;                                                   \
}


#define ICX_DEF_IN_RANGE_CASE_CONST_C1( worktype, _toggle_macro_ )  \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x]);                          \
    dst[x] = (uchar)-(scalar[0] <= a1 && a1 < scalar[1]);           \
}


#define ICX_DEF_IN_RANGE_CASE_CONST_C2( worktype, _toggle_macro_ )  \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*2]);                        \
    int f = scalar[0] <= a1 && a1 < scalar[2];                      \
    a1 = _toggle_macro_(src1[x*2+1]);                               \
    f &= scalar[1] <= a1 && a1 < scalar[3];                         \
    dst[x] = (uchar)-f;                                             \
}


#define ICX_DEF_IN_RANGE_CASE_CONST_C3( worktype, _toggle_macro_ )  \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*3]);                        \
    int f = scalar[0] <= a1 && a1 < scalar[3];                      \
    a1 = _toggle_macro_(src1[x*3+1]);                               \
    f &= scalar[1] <= a1 && a1 < scalar[4];                         \
    a1 = _toggle_macro_(src1[x*3+2]);                               \
    f &= scalar[2] <= a1 && a1 < scalar[5];                         \
    dst[x] = (uchar)-f;                                             \
}


#define ICX_DEF_IN_RANGE_CASE_CONST_C4( worktype, _toggle_macro_ )  \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*4]);                        \
    int f = scalar[0] <= a1 && a1 < scalar[4];                      \
    a1 = _toggle_macro_(src1[x*4+1]);                               \
    f &= scalar[1] <= a1 && a1 < scalar[5];                         \
    a1 = _toggle_macro_(src1[x*4+2]);                               \
    f &= scalar[2] <= a1 && a1 < scalar[6];                         \
    a1 = _toggle_macro_(src1[x*4+3]);                               \
    f &= scalar[3] <= a1 && a1 < scalar[7];                         \
    dst[x] = (uchar)-f;                                             \
}


#define ICX_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype,     \
                                     _toggle_macro_, cn )           \
static CxStatus CX_STDCALL                                          \
icxInRangeC_##flavor##_C##cn##R( const arrtype* src1, int step1,    \
                                 uchar* dst, int step, CxSize size, \
                                 const worktype* scalar )           \
{                                                                   \
    for( ; size.height--; (char*&)src1 += step1, (char*&)dst += step )  \
    {                                                               \
        int x;                                                      \
        ICX_DEF_IN_RANGE_CASE_CONST_C##cn( worktype, _toggle_macro_)\
    }                                                               \
                                                                    \
    return CX_OK;                                                   \
}


#define ICX_DEF_IN_RANGE_ALL( flavor, arrtype, worktype, _toggle_macro_ )   \
ICX_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype, _toggle_macro_, 1 )       \
ICX_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype, _toggle_macro_, 2 )       \
ICX_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype, _toggle_macro_, 3 )       \
ICX_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype, _toggle_macro_, 4 )       \
                                                                            \
ICX_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype, _toggle_macro_, 1 ) \
ICX_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype, _toggle_macro_, 2 ) \
ICX_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype, _toggle_macro_, 3 ) \
ICX_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype, _toggle_macro_, 4 )

ICX_DEF_IN_RANGE_ALL( 8u, uchar, int, CX_NOP )
ICX_DEF_IN_RANGE_ALL( 16s, short, int, CX_NOP )
ICX_DEF_IN_RANGE_ALL( 32s, int, int, CX_NOP )
ICX_DEF_IN_RANGE_ALL( 32f, float, float, CX_NOP )
ICX_DEF_IN_RANGE_ALL( 64f, double, double, CX_NOP )

#define icxInRange_8s_C1R 0
#define icxInRange_8s_C2R 0
#define icxInRange_8s_C3R 0
#define icxInRange_8s_C4R 0

#define icxInRangeC_8s_C1R 0
#define icxInRangeC_8s_C2R 0
#define icxInRangeC_8s_C3R 0
#define icxInRangeC_8s_C4R 0

CX_DEF_INIT_BIG_FUNC_TAB_2D( InRange, R )
CX_DEF_INIT_BIG_FUNC_TAB_2D( InRangeC, R )

typedef CxStatus (CX_STDCALL * CxInRangeCFunc)( const void* src, int srcstep,
                                                uchar* dst, int dststep,
                                                CxSize size, const void* scalar );

/*************************************** InRange ****************************************/

CX_IMPL void
cxInRange( const void* srcarr1, const void* srcarr2,
           const void* srcarr3, void* dstarr )
{
    static CxBigFuncTable inrange_tab;
    static int inittab = 0;

    CX_FUNCNAME( "cxInRange" );

    __BEGIN__;

    int type, coi = 0;
    int src1_step, src2_step, src3_step, dst_step;
    CxMat srcstub1, *src1 = (CxMat*)srcarr1;
    CxMat srcstub2, *src2 = (CxMat*)srcarr2;
    CxMat srcstub3, *src3 = (CxMat*)srcarr3;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxSize size;
    CxFunc2D_4A func;

    if( !inittab )
    {
        icxInitInRangeRTable( &inrange_tab );
        inittab = 1;
    }

    if( !CX_IS_MAT(src1) )
    {
        CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(src2) )
    {
        CX_CALL( src2 = cxGetMat( src2, &srcstub2, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(src3) )
    {
        CX_CALL( src3 = cxGetMat( src3, &srcstub3, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(dst) )
    {
        CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_ARE_TYPES_EQ( src1, src2 ) ||
        !CX_ARE_TYPES_EQ( src1, src3 ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_IS_MASK_ARR( dst ))
        CX_ERROR( CX_StsUnsupportedFormat, "Destination image should be 8uC1 or 8sC1");

    if( !CX_ARE_SIZES_EQ( src1, src2 ) ||
        !CX_ARE_SIZES_EQ( src1, src3 ) ||
        !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    type = CX_MAT_TYPE(src1->type);
    size = cxGetMatSize( src1 );

    if( CX_IS_MAT_CONT( src1->type & src2->type & src3->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = src2_step = src3_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        src2_step = src2->step;
        src3_step = src3->step;
        dst_step = dst->step;
    }

    func = (CxFunc2D_4A)(inrange_tab.fn_2d[type]);

    if( !func )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                     src3->data.ptr, src3_step, dst->data.ptr, dst_step, size ));

    __END__;
}


/************************************** InRangeS ****************************************/

CX_IMPL void
cxInRangeS( const void* srcarr, CxScalar lower,
            CxScalar upper, void* dstarr )
{
    static CxBigFuncTable inrange_tab;
    static int inittab = 0;

    CX_FUNCNAME( "cxInRangeS" );

    __BEGIN__;

    int sctype, type, coi = 0;
    int src1_step, dst_step;
    CxMat srcstub1, *src1 = (CxMat*)srcarr;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxSize size;
    CxInRangeCFunc func;
    double buf[8];

    if( !inittab )
    {
        icxInitInRangeCRTable( &inrange_tab );
        inittab = 1;
    }

    if( !CX_IS_MAT(src1) )
    {
        CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(dst) )
    {
        CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MASK_ARR( dst ))
        CX_ERROR( CX_StsUnsupportedFormat, "Destination image should be 8uC1 or 8sC1");

    if( !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    sctype = type = CX_MAT_TYPE(src1->type);
    if( CX_MAT_DEPTH(sctype) < CX_32S )
        sctype = (type & CX_MAT_CN_MASK) | CX_32SC1;

    size = cxGetMatSize( src1 );

    if( CX_IS_MAT_CONT( src1->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        dst_step = dst->step;
    }

    func = (CxInRangeCFunc)(inrange_tab.fn_2d[type]);

    if( !func )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    cxScalarToRawData( &lower, buf, sctype, 0 );
    cxScalarToRawData( &upper, (char*)buf + icxPixSize[sctype], sctype, 0 );

    IPPI_CALL( func( src1->data.ptr, src1_step, dst->data.ptr,
                     dst_step, size, buf ));

    __END__;
}


/****************************************************************************************\
*                                         Cmp                                            *
\****************************************************************************************/

#define ICX_DEF_CMP_CASE_C1( __op__, _toggle_macro_ )                   \
for( x = 0; x <= size.width - 4; x += 4 )                               \
{                                                                       \
    int f0 = __op__( _toggle_macro_(src1[x]), _toggle_macro_(src2[x])); \
    int f1 = __op__( _toggle_macro_(src1[x+1]), _toggle_macro_(src2[x+1])); \
    dst[x] = (uchar)-f0;                                                \
    dst[x+1] = (uchar)-f1;                                              \
    f0 = __op__( _toggle_macro_(src1[x+2]), _toggle_macro_(src2[x+2])); \
    f1 = __op__( _toggle_macro_(src1[x+3]), _toggle_macro_(src2[x+3])); \
    dst[x+2] = (uchar)-f0;                                              \
    dst[x+3] = (uchar)-f1;                                              \
}                                                                       \
                                                                        \
for( ; x < size.width; x++ )                                            \
{                                                                       \
    int f0 = __op__( _toggle_macro_(src1[x]), _toggle_macro_(src2[x])); \
    dst[x] = (uchar)-f0;                                                \
}


#define ICX_DEF_CMP_FUNC( __op__, name, flavor, arrtype,                \
                          worktype, _toggle_macro_ )                    \
IPCXAPI( CxStatus,                                                      \
icx##name##_##flavor##_C1R,( const arrtype* src1, int step1,            \
                             const arrtype* src2, int step2,            \
                             uchar* dst, int step, CxSize size ));      \
IPCXAPI_IMPL( CxStatus,                                                 \
icx##name##_##flavor##_C1R,( const arrtype* src1, int step1,            \
                             const arrtype* src2, int step2,            \
                             uchar* dst, int step, CxSize size ))       \
{                                                                       \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2, \
                          (char*&)dst += step )                         \
    {                                                                   \
        int x;                                                          \
        ICX_DEF_CMP_CASE_C1( __op__, _toggle_macro_ )                   \
    }                                                                   \
                                                                        \
    return CX_OK;                                                       \
}


#define ICX_DEF_CMP_CONST_CASE_C1( __op__, _toggle_macro_ )             \
for( x = 0; x <= size.width - 4; x += 4 )                               \
{                                                                       \
    int f0 = __op__( _toggle_macro_(src1[x]), scalar );                 \
    int f1 = __op__( _toggle_macro_(src1[x+1]), scalar );               \
    dst[x] = (uchar)-f0;                                                \
    dst[x+1] = (uchar)-f1;                                              \
    f0 = __op__( _toggle_macro_(src1[x+2]), scalar );                   \
    f1 = __op__( _toggle_macro_(src1[x+3]), scalar );                   \
    dst[x+2] = (uchar)-f0;                                              \
    dst[x+3] = (uchar)-f1;                                              \
}                                                                       \
                                                                        \
for( ; x < size.width; x++ )                                            \
{                                                                       \
    int f0 = __op__( _toggle_macro_(src1[x]), scalar );                 \
    dst[x] = (uchar)-f0;                                                \
}


#define ICX_DEF_CMP_CONST_FUNC( __op__, name, flavor, arrtype,          \
                                worktype, _toggle_macro_)               \
IPCXAPI( CxStatus,                                                      \
icx##name##C_##flavor##_C1R,( const arrtype* src1, int step1,           \
                             uchar* dst, int step,                      \
                             CxSize size, worktype* pScalar ));         \
IPCXAPI_IMPL( CxStatus,                                                 \
icx##name##C_##flavor##_C1R,( const arrtype* src1, int step1,           \
                             uchar* dst, int step,                      \
                             CxSize size, worktype* pScalar ))          \
{                                                                       \
    worktype scalar = *pScalar;                                         \
    for( ; size.height--; (char*&)src1 += step1, (char*&)dst += step )  \
    {                                                                   \
        int x;                                                          \
        ICX_DEF_CMP_CONST_CASE_C1( __op__, _toggle_macro_ )             \
    }                                                                   \
                                                                        \
    return CX_OK;                                                       \
}


#define ICX_DEF_CMP_ALL( flavor, arrtype, worktype, _toggle_macro_ )            \
ICX_DEF_CMP_FUNC( CX_GT, CmpGT, flavor, arrtype, worktype, _toggle_macro_ )     \
ICX_DEF_CMP_FUNC( CX_EQ, CmpEQ, flavor, arrtype, worktype, _toggle_macro_ )     \
ICX_DEF_CMP_CONST_FUNC( CX_GT, CmpGT, flavor, arrtype, worktype, _toggle_macro_)\
ICX_DEF_CMP_CONST_FUNC( CX_GE, CmpGE, flavor, arrtype, worktype, _toggle_macro_)\
ICX_DEF_CMP_CONST_FUNC( CX_EQ, CmpEQ, flavor, arrtype, worktype, _toggle_macro_)

ICX_DEF_CMP_ALL( 8u, uchar, int, CX_NOP )
ICX_DEF_CMP_ALL( 16s, short, int, CX_NOP )
ICX_DEF_CMP_ALL( 32s, int, int, CX_NOP )
ICX_DEF_CMP_ALL( 32f, float, double, CX_NOP )
ICX_DEF_CMP_ALL( 64f, double, double, CX_NOP )

#define icxCmpGT_8s_C1R     0
#define icxCmpEQ_8s_C1R     0
#define icxCmpGTC_8s_C1R    0
#define icxCmpGEC_8s_C1R    0
#define icxCmpEQC_8s_C1R    0

CX_DEF_INIT_FUNC_TAB_2D( CmpGT, C1R )
CX_DEF_INIT_FUNC_TAB_2D( CmpEQ, C1R )
CX_DEF_INIT_FUNC_TAB_2D( CmpGTC, C1R )
CX_DEF_INIT_FUNC_TAB_2D( CmpGEC, C1R )
CX_DEF_INIT_FUNC_TAB_2D( CmpEQC, C1R )

/***************************************** cxCmp ****************************************/

CX_IMPL void
cxCmp( const void* srcarr1, const void* srcarr2,
       void* dstarr, int cmp_op )
{
    static CxFuncTable cmp_tab[2];
    static int inittab = 0;

    CX_FUNCNAME( "cxCmp" );

    __BEGIN__;

    int type, coi = 0;
    int invflag = 0;
    int src1_step, src2_step, dst_step;
    CxMat srcstub1, *src1 = (CxMat*)srcarr1;
    CxMat srcstub2, *src2 = (CxMat*)srcarr2;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxMat *temp;
    CxSize size;
    CxFunc2D_3A func;

    if( !inittab )
    {
        icxInitCmpGTC1RTable( &cmp_tab[0] );
        icxInitCmpEQC1RTable( &cmp_tab[1] );
        inittab = 1;
    }

    switch( cmp_op )
    {
    case CX_CMP_GT:
    case CX_CMP_EQ:
        break;
    case CX_CMP_GE:
        CX_SWAP( src1, src2, temp );
        invflag = 1;
        break;
    case CX_CMP_LT:
        CX_SWAP( src1, src2, temp );
        break;
    case CX_CMP_LE:
        invflag = 1;
        break;
    case CX_CMP_NE:
        cmp_op = CX_CMP_EQ;
        invflag = 1;
        break;
    default:
        CX_ERROR( CX_StsBadArg, "Unknown comparison operation" );
    }

    if( !CX_IS_MAT(src1) )
    {
        CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(src2) )
    {
        CX_CALL( src2 = cxGetMat( src2, &srcstub2, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(dst) )
    {
        CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_ARE_TYPES_EQ( src1, src2 ) )
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( CX_MAT_CN( src1->type ) != 1 )
        CX_ERROR( CX_StsUnsupportedFormat, "Input arrays must be single-channel");

    if( !CX_IS_MASK_ARR( dst ))
        CX_ERROR( CX_StsUnsupportedFormat, "Destination array should be 8uC1 or 8sC1");

    if( !CX_ARE_SIZES_EQ( src1, src2 ) ||
        !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    type = CX_MAT_TYPE(src1->type);
    size = cxGetMatSize( src1 );

    if( CX_IS_MAT_CONT( src1->type & src2->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = src2_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        src2_step = src2->step;
        dst_step = dst->step;
    }

    func = (CxFunc2D_3A)(cmp_tab[cmp_op == CX_CMP_EQ].fn_2d[type]);

    if( !func )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                     dst->data.ptr, dst_step, size ));

    if( invflag )
        IPPI_CALL( icxNot_8u_C1R( dst->data.ptr, dst_step,
                           dst->data.ptr, dst_step, size ));

    __END__;
}


/*************************************** cxCmpS *****************************************/

CX_IMPL void
cxCmpS( const void* srcarr, double value, void* dstarr, int cmp_op )
{
    static CxFuncTable cmps_tab[3];
    static int inittab = 0;

    CX_FUNCNAME( "cxCmpS" );

    __BEGIN__;

    int type, coi = 0;
    int invflag = 0;
    int src1_step, dst_step;
    CxMat srcstub1, *src1 = (CxMat*)srcarr;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxSize size;
    CxFunc2D_2A1P func;
    double buf;

    if( !inittab )
    {
        icxInitCmpEQCC1RTable( &cmps_tab[CX_CMP_EQ] );
        icxInitCmpGTCC1RTable( &cmps_tab[CX_CMP_GT] );
        icxInitCmpGECC1RTable( &cmps_tab[CX_CMP_GE] );
        inittab = 1;
    }

    switch( cmp_op )
    {
    case CX_CMP_GT:
    case CX_CMP_EQ:
    case CX_CMP_GE:
        break;
    case CX_CMP_LT:
        invflag = 1;
        cmp_op = CX_CMP_GE;
        break;
    case CX_CMP_LE:
        invflag = 1;
        cmp_op = CX_CMP_GT;
        break;
    case CX_CMP_NE:
        invflag = 1;
        cmp_op = CX_CMP_EQ;
        break;
    default:
        CX_ERROR( CX_StsBadArg, "Unknown comparison operation" );
    }

    if( !CX_IS_MAT(src1) )
    {
        CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(dst) )
    {
        CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MASK_ARR( dst ))
        CX_ERROR( CX_StsUnsupportedFormat, "Destination array should be 8uC1 or 8sC1");

    if( CX_MAT_CN( src1->type ) != 1 )
        CX_ERROR( CX_StsUnsupportedFormat, "Input array must be single-channel");

    if( !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    type = CX_MAT_TYPE(src1->type);

    if( CX_MAT_DEPTH(type) <= CX_32S )
        (int&)buf = cxRound(value);
    else
        buf = value;

    size = cxGetMatSize( src1 );

    if( CX_IS_MAT_CONT( src1->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        dst_step = dst->step;
    }

    func = (CxFunc2D_2A1P)(cmps_tab[cmp_op].fn_2d[type]);

    if( !func )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, dst->data.ptr,
                     dst_step, size, &buf ));

    if( invflag )
        IPPI_CALL( icxNot_8u_C1R( dst->data.ptr, dst_step,
                           dst->data.ptr, dst_step, size ));

    __END__;
}


/****************************************************************************************\
*                                       Min/Max                                          *
\****************************************************************************************/


#define ICX_DEF_MINMAX_CASE_C1( __op__, arrtype, worktype, _toggle_macro_ )\
for( x = 0; x <= size.width - 4; x += 4 )                               \
{                                                                       \
    worktype a0 = _toggle_macro_(src1[x]), b0 = _toggle_macro_(src2[x]);\
    worktype a1 = _toggle_macro_(src1[x+1]), b1 = _toggle_macro_(src2[x+1]);\
    a0 = __op__( a0, b0 );                                              \
    a1 = __op__( a1, b1 );                                              \
    dst[x] = (arrtype)_toggle_macro_(a0);                               \
    dst[x+1] = (arrtype)_toggle_macro_(a1);                             \
    a0 = _toggle_macro_(src1[x+2]), b0 = _toggle_macro_(src2[x+2]);     \
    a1 = _toggle_macro_(src1[x+3]), b1 = _toggle_macro_(src2[x+3]);     \
    a0 = __op__( a0, b0 );                                              \
    a1 = __op__( a1, b1 );                                              \
    a0 = _toggle_macro_(a0);                                            \
    a1 = _toggle_macro_(a1);                                            \
    dst[x+2] = (arrtype)_toggle_macro_(a0);                             \
    dst[x+3] = (arrtype)_toggle_macro_(a1);                             \
}                                                                       \
                                                                        \
for( ; x < size.width; x++ )                                            \
{                                                                       \
    worktype a0 = _toggle_macro_(src1[x]), b0 = _toggle_macro_(src2[x]);\
    a0 = __op__( a0, b0 );                                              \
    dst[x] = (arrtype)_toggle_macro_(a0);                               \
}


#define ICX_DEF_MINMAX_FUNC( __op__, name, flavor, arrtype,             \
                             worktype, _toggle_macro_ )                 \
IPCXAPI( CxStatus,                                                      \
icx##name##_##flavor##_C1R,( const arrtype* src1, int step1,            \
                             const arrtype* src2, int step2,            \
                             arrtype* dst, int step, CxSize size ));    \
IPCXAPI_IMPL( CxStatus,                                                 \
icx##name##_##flavor##_C1R,( const arrtype* src1, int step1,            \
                             const arrtype* src2, int step2,            \
                             arrtype* dst, int step, CxSize size ))     \
{                                                                       \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2, \
                          (char*&)dst += step )                         \
    {                                                                   \
        int x;                                                          \
        ICX_DEF_MINMAX_CASE_C1( __op__, arrtype,                        \
                                worktype, _toggle_macro_ )              \
    }                                                                   \
                                                                        \
    return CX_OK;                                                       \
}


#define ICX_DEF_MINMAX_CONST_CASE_C1( __op__, arrtype, worktype, _toggle_macro_ )\
for( x = 0; x <= size.width - 4; x += 4 )                       \
{                                                               \
    worktype a0 = _toggle_macro_(src1[x]);                      \
    worktype a1 = _toggle_macro_(src1[x+1]);                    \
    a0 = __op__( a0, scalar );                                  \
    a1 = __op__( a1, scalar );                                  \
    dst[x] = (arrtype)_toggle_macro_(a0);                       \
    dst[x+1] = (arrtype)_toggle_macro_(a1);                     \
    a0 = _toggle_macro_(src1[x+2]);                             \
    a1 = _toggle_macro_(src1[x+3]);                             \
    a0 = __op__( a0, scalar );                                  \
    a1 = __op__( a1, scalar );                                  \
    dst[x+2] = (arrtype)_toggle_macro_(a0);                     \
    dst[x+3] = (arrtype)_toggle_macro_(a1);                     \
}                                                               \
                                                                \
for( ; x < size.width; x++ )                                    \
{                                                               \
    worktype a0 = _toggle_macro_(src1[x]);                      \
    a0 = __op__( a0, scalar );                                  \
    dst[x] = (arrtype)_toggle_macro_(a0);                       \
}


#define ICX_DEF_MINMAX_CONST_FUNC( __op__, name, flavor, arrtype,       \
                                   worktype, _toggle_macro_)            \
IPCXAPI( CxStatus,                                                      \
icx##name##C_##flavor##_C1R,( const arrtype* src1, int step1,           \
                             arrtype* dst, int step,                    \
                             CxSize size, worktype* pScalar ));         \
IPCXAPI_IMPL( CxStatus,                                                 \
icx##name##C_##flavor##_C1R,( const arrtype* src1, int step1,           \
                             arrtype* dst, int step,                    \
                             CxSize size, worktype* pScalar ))          \
{                                                                       \
    worktype scalar = _toggle_macro_(*pScalar);                         \
    for( ; size.height--; (char*&)src1 += step1, (char*&)dst += step )  \
    {                                                                   \
        int x;                                                          \
        ICX_DEF_MINMAX_CONST_CASE_C1( __op__, arrtype,                  \
                                      worktype, _toggle_macro_ )        \
    }                                                                   \
                                                                        \
    return CX_OK;                                                       \
}


#define ICX_DEF_MINMAX_ALL( flavor, arrtype, worktype,                             \
                            _toggle_macro_, _min_op_, _max_op_ )                   \
ICX_DEF_MINMAX_FUNC( _min_op_, Min, flavor, arrtype, worktype, _toggle_macro_ )    \
ICX_DEF_MINMAX_FUNC( _max_op_, Max, flavor, arrtype, worktype, _toggle_macro_ )    \
ICX_DEF_MINMAX_CONST_FUNC(_min_op_, Min, flavor, arrtype, worktype, _toggle_macro_)\
ICX_DEF_MINMAX_CONST_FUNC(_max_op_, Max, flavor, arrtype, worktype, _toggle_macro_)


ICX_DEF_MINMAX_ALL( 8u, uchar, int, CX_NOP, CX_IMIN, CX_IMAX )
ICX_DEF_MINMAX_ALL( 16s, short, int, CX_NOP, CX_IMIN, CX_IMAX )
ICX_DEF_MINMAX_ALL( 32s, int, int, CX_NOP, CX_IMIN, CX_IMAX )
ICX_DEF_MINMAX_ALL( 32f, int, int, CX_TOGGLE_FLT, CX_IMIN, CX_IMAX )
ICX_DEF_MINMAX_ALL( 64f, double, double, CX_NOP, CX_MIN, CX_MAX )

#define icxMin_8s_C1R     0
#define icxMax_8s_C1R     0
#define icxMinC_8s_C1R    0
#define icxMaxC_8s_C1R    0

CX_DEF_INIT_FUNC_TAB_2D( Min, C1R )
CX_DEF_INIT_FUNC_TAB_2D( Max, C1R )
CX_DEF_INIT_FUNC_TAB_2D( MinC, C1R )
CX_DEF_INIT_FUNC_TAB_2D( MaxC, C1R )

/*********************************** cxMin & cxMax **************************************/

static void
icxMinMax( const void* srcarr1, const void* srcarr2,
           void* dstarr, int is_max )
{
    static CxFuncTable minmax_tab[2];
    static int inittab = 0;

    CX_FUNCNAME( "icxMinMax" );

    __BEGIN__;

    int type, coi = 0;
    int src1_step, src2_step, dst_step;
    CxMat srcstub1, *src1 = (CxMat*)srcarr1;
    CxMat srcstub2, *src2 = (CxMat*)srcarr2;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxSize size;
    CxFunc2D_3A func;

    if( !inittab )
    {
        icxInitMinC1RTable( &minmax_tab[0] );
        icxInitMaxC1RTable( &minmax_tab[1] );
        inittab = 1;
    }

    if( !CX_IS_MAT(src1) )
    {
        CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(src2) )
    {
        CX_CALL( src2 = cxGetMat( src2, &srcstub2, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(dst) )
    {
        CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_ARE_TYPES_EQ( src1, src2 ) ||
        !CX_ARE_TYPES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( CX_MAT_CN( src1->type ) != 1 )
        CX_ERROR( CX_StsUnsupportedFormat, "Input arrays must be single-channel");

    if( !CX_ARE_SIZES_EQ( src1, src2 ) ||
        !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    type = CX_MAT_TYPE(src1->type);
    size = cxGetMatSize( src1 );

    if( CX_IS_MAT_CONT( src1->type & src2->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = src2_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        src2_step = src2->step;
        dst_step = dst->step;
    }

    func = (CxFunc2D_3A)(minmax_tab[is_max != 0].fn_2d[type]);

    if( !func )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                     dst->data.ptr, dst_step, size ));

    __END__;
}


CX_IMPL void
cxMin( const void* srcarr1, const void* srcarr2, void* dstarr )
{
    icxMinMax( srcarr1, srcarr2, dstarr, 0 );
}


CX_IMPL void
cxMax( const void* srcarr1, const void* srcarr2, void* dstarr )
{
    icxMinMax( srcarr1, srcarr2, dstarr, 1 );
}


/********************************* cxMinS / cxMaxS **************************************/

static void
icxMinMaxS( const void* srcarr, double value, void* dstarr, int is_max )
{
    static CxFuncTable minmaxs_tab[2];
    static int inittab = 0;

    CX_FUNCNAME( "icxMinMaxS" );

    __BEGIN__;

    int type, coi = 0;
    int src1_step, dst_step;
    CxMat srcstub1, *src1 = (CxMat*)srcarr;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxSize size;
    CxFunc2D_2A1P func;
    union
    {
        int i;
        float f;
        double d;
    }
    buf;

    if( !inittab )
    {
        icxInitMinCC1RTable( &minmaxs_tab[0] );
        icxInitMaxCC1RTable( &minmaxs_tab[1] );
        inittab = 1;
    }

    if( !CX_IS_MAT(src1) )
    {
        CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT(dst) )
    {
        CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_ARE_TYPES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( CX_MAT_CN( src1->type ) != 1 )
        CX_ERROR( CX_StsUnsupportedFormat, "Input array must be single-channel");

    if( !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    type = CX_MAT_TYPE(src1->type);
    
    if( CX_MAT_DEPTH(type) <= CX_32S )
        buf.i = cxRound(value);
    else if( CX_MAT_DEPTH(type) == CX_32F )
        buf.f = (float)value;
    else
        buf.d = value;

    size = cxGetMatSize( src1 );

    if( CX_IS_MAT_CONT( src1->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        dst_step = dst->step;
    }

    func = (CxFunc2D_2A1P)(minmaxs_tab[is_max].fn_2d[type]);

    if( !func )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, dst->data.ptr,
                     dst_step, size, &buf ));

    __END__;
}


CX_IMPL void
cxMinS( const void* srcarr, double value, void* dstarr )
{
    icxMinMaxS( srcarr, value, dstarr, 0 );
}


CX_IMPL void
cxMaxS( const void* srcarr, double value, void* dstarr )
{
    icxMinMaxS( srcarr, value, dstarr, 1 );
}


/* End of file. */
