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
*                             Conversion pixel -> plane                                  *
\****************************************************************************************/


#define  ICX_DEF_PX2PL_C2( arrtype, len )           \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j < (len); j++, (src) += 2 )        \
    {                                               \
        arrtype t0 = (src)[0];                      \
        arrtype t1 = (src)[1];                      \
                                                    \
        plane[0][j] = t0;                           \
        plane[1][j] = t1;                           \
    }                                               \
}


#define  ICX_DEF_PX2PL_C3( arrtype, len )           \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j < (len); j++, (src) += 3 )        \
    {                                               \
        arrtype t0 = (src)[0];                      \
        arrtype t1 = (src)[1];                      \
        arrtype t2 = (src)[2];                      \
                                                    \
        plane[0][j] = t0;                           \
        plane[1][j] = t1;                           \
        plane[2][j] = t2;                           \
    }                                               \
}


#define  ICX_DEF_PX2PL_C4( arrtype, len )           \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j < (len); j++, (src) += 4 )        \
    {                                               \
        arrtype t0 = (src)[0];                      \
        arrtype t1 = (src)[1];                      \
                                                    \
        plane[0][j] = t0;                           \
        plane[1][j] = t1;                           \
                                                    \
        t0 = (src)[2];                              \
        t1 = (src)[3];                              \
                                                    \
        plane[2][j] = t0;                           \
        plane[3][j] = t1;                           \
    }                                               \
}


#define  ICX_DEF_PX2PL_COI( arrtype, len, cn )          \
{                                                       \
    int j;                                              \
                                                        \
    for( j = 0; j <= (len) - 4; j += 4, (src) += 4*(cn))\
    {                                                   \
        arrtype t0 = (src)[0];                          \
        arrtype t1 = (src)[(cn)];                       \
                                                        \
        (dst)[j] = t0;                                  \
        (dst)[j+1] = t1;                                \
                                                        \
        t0 = (src)[(cn)*2];                             \
        t1 = (src)[(cn)*3];                             \
                                                        \
        (dst)[j+2] = t0;                                \
        (dst)[j+3] = t1;                                \
    }                                                   \
                                                        \
    for( ; j < (len); j++, (src) += (cn))               \
    {                                                   \
        (dst)[j] = (src)[0];                            \
    }                                                   \
}


#define  ICX_DEF_COPY_PX2PL_FUNC_2D( arrtype, flavor, cn )              \
IPCXAPI_IMPL( CxStatus, icxCopy_##flavor##_C##cn##P##cn##R,             \
( const arrtype* src, int srcstep,                                      \
  arrtype** dst, int dststep, CxSize size ))                            \
{                                                                       \
    arrtype* plane[] = { 0, 0, 0, 0 };                                  \
    int i;                                                              \
                                                                        \
    for( i = 0; i < cn; i++ )                                           \
    {                                                                   \
        plane[i] = dst[i];                                              \
    }                                                                   \
                                                                        \
    for( ; size.height--; (char*&)src += srcstep,                       \
                          (char*&)(plane[0]) += dststep,                \
                          (char*&)(plane[1]) += dststep,                \
                          (char*&)(plane[2]) += dststep,                \
                          (char*&)(plane[3]) += dststep )               \
    {                                                                   \
        ICX_DEF_PX2PL_C##cn( arrtype, size.width );                     \
        src -= size.width*(cn);                                         \
    }                                                                   \
                                                                        \
    return CX_OK;                                                       \
}


#define  ICX_DEF_COPY_PX2PL_FUNC_2D_COI( arrtype, flavor )              \
IPCXAPI_IMPL( CxStatus, icxCopy_##flavor##_CnC1CR,                      \
( const arrtype* src, int srcstep, arrtype* dst, int dststep,           \
  CxSize size, int cn, int coi ))                                       \
{                                                                       \
    src += coi - 1;                                                     \
    for( ; size.height--; (char*&)src += srcstep,                       \
                          (char*&)dst += dststep )                      \
    {                                                                   \
        ICX_DEF_PX2PL_COI( arrtype, size.width, cn );                   \
        src -= size.width*(cn);                                         \
    }                                                                   \
                                                                        \
    return CX_OK;                                                       \
}


ICX_DEF_COPY_PX2PL_FUNC_2D( uchar, 8u, 2 )
ICX_DEF_COPY_PX2PL_FUNC_2D( uchar, 8u, 3 )
ICX_DEF_COPY_PX2PL_FUNC_2D( uchar, 8u, 4 )
ICX_DEF_COPY_PX2PL_FUNC_2D( ushort, 16u, 2 )
ICX_DEF_COPY_PX2PL_FUNC_2D( ushort, 16u, 3 )
ICX_DEF_COPY_PX2PL_FUNC_2D( ushort, 16u, 4 )
ICX_DEF_COPY_PX2PL_FUNC_2D( int, 32s, 2 )
ICX_DEF_COPY_PX2PL_FUNC_2D( int, 32s, 3 )
ICX_DEF_COPY_PX2PL_FUNC_2D( int, 32s, 4 )
ICX_DEF_COPY_PX2PL_FUNC_2D( int64, 64f, 2 )
ICX_DEF_COPY_PX2PL_FUNC_2D( int64, 64f, 3 )
ICX_DEF_COPY_PX2PL_FUNC_2D( int64, 64f, 4 )


ICX_DEF_COPY_PX2PL_FUNC_2D_COI( uchar, 8u )
ICX_DEF_COPY_PX2PL_FUNC_2D_COI( ushort, 16u )
ICX_DEF_COPY_PX2PL_FUNC_2D_COI( int, 32s )
ICX_DEF_COPY_PX2PL_FUNC_2D_COI( int64, 64f )


/****************************************************************************************\
*                             Conversion plane -> pixel                                  *
\****************************************************************************************/


#define  ICX_DEF_PL2PX_C2( arrtype, len )           \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j < (len); j++, (dst) += 2 )        \
    {                                               \
        arrtype t0 = plane[0][j];                   \
        arrtype t1 = plane[1][j];                   \
                                                    \
        dst[0] = t0;                                \
        dst[1] = t1;                                \
    }                                               \
}


#define  ICX_DEF_PL2PX_C3( arrtype, len )           \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j < (len); j++, (dst) += 3 )        \
    {                                               \
        arrtype t0 = plane[0][j];                   \
        arrtype t1 = plane[1][j];                   \
        arrtype t2 = plane[2][j];                   \
                                                    \
        dst[0] = t0;                                \
        dst[1] = t1;                                \
        dst[2] = t2;                                \
    }                                               \
}


#define  ICX_DEF_PL2PX_C4( arrtype, len )           \
{                                                   \
    int j;                                          \
                                                    \
    for( j = 0; j < (len); j++, (dst) += 4 )        \
    {                                               \
        arrtype t0 = plane[0][j];                   \
        arrtype t1 = plane[1][j];                   \
                                                    \
        dst[0] = t0;                                \
        dst[1] = t1;                                \
                                                    \
        t0 = plane[2][j];                           \
        t1 = plane[3][j];                           \
                                                    \
        dst[2] = t0;                                \
        dst[3] = t1;                                \
    }                                               \
}


#define  ICX_DEF_PL2PX_COI( arrtype, len, cn )          \
{                                                       \
    int j;                                              \
                                                        \
    for( j = 0; j <= (len) - 4; j += 4, (dst) += 4*(cn))\
    {                                                   \
        arrtype t0 = (src)[j];                          \
        arrtype t1 = (src)[j+1];                        \
                                                        \
        (dst)[0] = t0;                                  \
        (dst)[(cn)] = t1;                               \
                                                        \
        t0 = (src)[j+2];                                \
        t1 = (src)[j+3];                                \
                                                        \
        (dst)[(cn)*2] = t0;                             \
        (dst)[(cn)*3] = t1;                             \
    }                                                   \
                                                        \
    for( ; j < (len); j++, (dst) += (cn))               \
    {                                                   \
        (dst)[0] = (src)[j];                            \
    }                                                   \
}


#define  ICX_DEF_COPY_PL2PX_FUNC_2D( arrtype, flavor, cn )                      \
IPCXAPI_IMPL( CxStatus,                                                         \
icxCopy_##flavor##_P##cn##C##cn##R,( const arrtype** src, int srcstep,          \
                                    arrtype* dst, int dststep, CxSize size ))   \
{                                                                               \
    const arrtype* plane[] = { 0, 0, 0, 0 };                                    \
    int i;                                                                      \
                                                                                \
    for( i = 0; i < cn; i++ )                                                   \
    {                                                                           \
        plane[i] = src[i];                                                      \
    }                                                                           \
                                                                                \
    for( ; size.height--; (char*&)dst += dststep,                               \
                          (char*&)(plane[0]) += srcstep,                        \
                          (char*&)(plane[1]) += srcstep,                        \
                          (char*&)(plane[2]) += srcstep,                        \
                          (char*&)(plane[3]) += srcstep )                       \
    {                                                                           \
        ICX_DEF_PL2PX_C##cn( arrtype, size.width );                             \
        dst -= size.width*(cn);                                                 \
    }                                                                           \
                                                                                \
    return CX_OK;                                                               \
}


#define  ICX_DEF_COPY_PL2PX_FUNC_2D_COI( arrtype, flavor )              \
IPCXAPI_IMPL( CxStatus, icxCopy_##flavor##_C1CnCR,                      \
( const arrtype* src, int srcstep, arrtype* dst, int dststep,           \
  CxSize size, int cn, int coi ))                                       \
{                                                                       \
    dst += coi - 1;                                                     \
    for( ; size.height--; (char*&)src += srcstep,                       \
                          (char*&)dst += dststep )                      \
    {                                                                   \
        ICX_DEF_PL2PX_COI( arrtype, size.width, cn );                   \
        dst -= size.width*(cn);                                         \
    }                                                                   \
                                                                        \
    return CX_OK;                                                       \
}


ICX_DEF_COPY_PL2PX_FUNC_2D( uchar, 8u, 2 )
ICX_DEF_COPY_PL2PX_FUNC_2D( uchar, 8u, 3 )
ICX_DEF_COPY_PL2PX_FUNC_2D( uchar, 8u, 4 )
ICX_DEF_COPY_PL2PX_FUNC_2D( ushort, 16u, 2 )
ICX_DEF_COPY_PL2PX_FUNC_2D( ushort, 16u, 3 )
ICX_DEF_COPY_PL2PX_FUNC_2D( ushort, 16u, 4 )
ICX_DEF_COPY_PL2PX_FUNC_2D( int, 32s, 2 )
ICX_DEF_COPY_PL2PX_FUNC_2D( int, 32s, 3 )
ICX_DEF_COPY_PL2PX_FUNC_2D( int, 32s, 4 )
ICX_DEF_COPY_PL2PX_FUNC_2D( int64, 64f, 2 )
ICX_DEF_COPY_PL2PX_FUNC_2D( int64, 64f, 3 )
ICX_DEF_COPY_PL2PX_FUNC_2D( int64, 64f, 4 )

ICX_DEF_COPY_PL2PX_FUNC_2D_COI( uchar, 8u )
ICX_DEF_COPY_PL2PX_FUNC_2D_COI( ushort, 16u )
ICX_DEF_COPY_PL2PX_FUNC_2D_COI( int, 32s )
ICX_DEF_COPY_PL2PX_FUNC_2D_COI( int64, 64f )


#define  ICX_DEF_PXPLPX_TAB( name, FROM, TO )                           \
static void                                                             \
name( CxBigFuncTable* tab )                                             \
{                                                                       \
    tab->fn_2d[CX_8UC2] = (void*)icxCopy##_8u_##FROM##2##TO##2R;        \
    tab->fn_2d[CX_8UC3] = (void*)icxCopy##_8u_##FROM##3##TO##3R;        \
    tab->fn_2d[CX_8UC4] = (void*)icxCopy##_8u_##FROM##4##TO##4R;        \
                                                                        \
    tab->fn_2d[CX_8SC2] = (void*)icxCopy##_8u_##FROM##2##TO##2R;        \
    tab->fn_2d[CX_8SC3] = (void*)icxCopy##_8u_##FROM##3##TO##3R;        \
    tab->fn_2d[CX_8SC4] = (void*)icxCopy##_8u_##FROM##4##TO##4R;        \
                                                                        \
    tab->fn_2d[CX_16SC2] = (void*)icxCopy##_16u_##FROM##2##TO##2R;      \
    tab->fn_2d[CX_16SC3] = (void*)icxCopy##_16u_##FROM##3##TO##3R;      \
    tab->fn_2d[CX_16SC4] = (void*)icxCopy##_16u_##FROM##4##TO##4R;      \
                                                                        \
    tab->fn_2d[CX_32SC2] = (void*)icxCopy##_32s_##FROM##2##TO##2R;      \
    tab->fn_2d[CX_32SC3] = (void*)icxCopy##_32s_##FROM##3##TO##3R;      \
    tab->fn_2d[CX_32SC4] = (void*)icxCopy##_32s_##FROM##4##TO##4R;      \
                                                                        \
    tab->fn_2d[CX_32FC2] = (void*)icxCopy##_32s_##FROM##2##TO##2R;      \
    tab->fn_2d[CX_32FC3] = (void*)icxCopy##_32s_##FROM##3##TO##3R;      \
    tab->fn_2d[CX_32FC4] = (void*)icxCopy##_32s_##FROM##4##TO##4R;      \
                                                                        \
    tab->fn_2d[CX_64FC2] = (void*)icxCopy##_64f_##FROM##2##TO##2R;      \
    tab->fn_2d[CX_64FC3] = (void*)icxCopy##_64f_##FROM##3##TO##3R;      \
    tab->fn_2d[CX_64FC4] = (void*)icxCopy##_64f_##FROM##4##TO##4R;      \
}



#define  ICX_DEF_PXPLCOI_TAB( name, FROM, TO )                          \
static void                                                             \
name( CxFuncTable* tab )                                                \
{                                                                       \
    tab->fn_2d[CX_8U] = (void*)icxCopy##_8u_##FROM##TO##CR;             \
    tab->fn_2d[CX_8S] = (void*)icxCopy##_8u_##FROM##TO##CR;             \
    tab->fn_2d[CX_16S] = (void*)icxCopy##_16u_##FROM##TO##CR;           \
    tab->fn_2d[CX_32S] = (void*)icxCopy##_32s_##FROM##TO##CR;           \
    tab->fn_2d[CX_32F] = (void*)icxCopy##_32s_##FROM##TO##CR;           \
    tab->fn_2d[CX_64F] = (void*)icxCopy##_64f_##FROM##TO##CR;           \
}


ICX_DEF_PXPLPX_TAB( icxInitCxtPixToPlaneRTable, C, P )
ICX_DEF_PXPLCOI_TAB( icxInitCxtPixToPlaneRCoiTable, Cn, C1 )
ICX_DEF_PXPLPX_TAB( icxInitCxtPlaneToPixRTable, P, C )
ICX_DEF_PXPLCOI_TAB( icxInitCxtPlaneToPixRCoiTable, C1, Cn )

typedef CxStatus (CX_STDCALL *CxCxtPixToPlaneFunc)( const void* src, int srcstep,
                                                    void** dst, int dststep, CxSize size);

typedef CxStatus (CX_STDCALL *CxCxtPixToPlaneCoiFunc)( const void* src, int srcstep,
                                                       void* dst, int dststep,
                                                       CxSize size, int cn, int coi );

typedef CxStatus (CX_STDCALL *CxCxtPlaneToPixFunc)( const void** src, int srcstep,
                                                    void* dst, int dststep, CxSize size);

typedef CxStatus (CX_STDCALL *CxCxtPlaneToPixCoiFunc)( const void* src, int srcstep,
                                                       void* dst, int dststep,
                                                       CxSize size, int cn, int coi );

CX_IMPL void
cxCxtPixToPlane( const void* srcarr, void* dstarr0, void* dstarr1,
                 void* dstarr2, void* dstarr3 )
{
    static CxBigFuncTable  pxpl_tab;
    static CxFuncTable  pxplcoi_tab;
    static int inittab = 0;

    CX_FUNCNAME( "cxCxtPixToPlane" );

    __BEGIN__;

    CxMat stub[5], *dst[4], *src = (CxMat*)srcarr;
    CxSize size;
    void* dstptr[4] = { 0, 0, 0, 0 };
    int type, cn, coi = 0;
    int i, nzplanes = 0, nzidx = -1;
    int cont_flag;
    int src_step = 0, dst_step = 0;

    if( !inittab )
    {
        icxInitCxtPixToPlaneRTable( &pxpl_tab );
        icxInitCxtPixToPlaneRCoiTable( &pxplcoi_tab );
        inittab = 1;
    }

    dst[0] = (CxMat*)dstarr0;
    dst[1] = (CxMat*)dstarr1;
    dst[2] = (CxMat*)dstarr2;
    dst[3] = (CxMat*)dstarr3;

    CX_CALL( src = cxGetMat( src, stub + 4, &coi ));

    //if( coi != 0 )
    //    CX_ERROR( CX_BadCOI, "" );

    type = CX_MAT_TYPE( src->type );
    cn = CX_MAT_CN( type );

    cont_flag = src->type;

    if( cn == 1 )
        CX_ERROR( CX_BadNumChannels, "" );

    for( i = 0; i < 4; i++ )
    {
        if( dst[i] )
        {
            nzplanes++;
            nzidx = i;
            CX_CALL( dst[i] = cxGetMat( dst[i], stub + i ));
            if( CX_MAT_CN( dst[i]->type ) != 1 )
                CX_ERROR( CX_BadNumChannels, "" );
            if( !CX_ARE_DEPTHS_EQ( dst[i], src ))
                CX_ERROR( CX_StsUnmatchedFormats, "" );
            if( !CX_ARE_SIZES_EQ( dst[i], src ))
                CX_ERROR( CX_StsUnmatchedSizes, "" );
            if( nzplanes > i && i > 0 && dst[i]->step != dst[i-1]->step )
                CX_ERROR( CX_BadStep, "" );
            dst_step = dst[i]->step;
            dstptr[nzplanes-1] = dst[i]->data.ptr;

            cont_flag &= dst[i]->type;
        }
    }

    src_step = src->step;
    size = cxGetMatSize( src );

    if( CX_IS_MAT_CONT( cont_flag ))
    {
        size.width *= size.height;
        src_step = dst_step = CX_STUB_STEP;
        
        size.height = 1;
    }

    if( nzplanes == cn )
    {
        CxCxtPixToPlaneFunc func = (CxCxtPixToPlaneFunc)pxpl_tab.fn_2d[type];

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, src_step, dstptr, dst_step, size ));
    }
    else if( nzplanes == 1 )
    {
        CxCxtPixToPlaneCoiFunc func = (CxCxtPixToPlaneCoiFunc)
                                      pxplcoi_tab.fn_2d[CX_MAT_DEPTH(type)];

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, src_step,
                         dst[nzidx]->data.ptr, dst_step,
                         size, cn, nzidx + 1 ));
    }
    else
    {
        CX_ERROR( CX_StsBadArg,
            "Either all output planes or only one output plane should be non zero" );
    }

    __END__;
}



CX_IMPL void
cxCxtPlaneToPix( const void* srcarr0, const void* srcarr1, const void* srcarr2,
                 const void* srcarr3, void* dstarr )
{
    static CxBigFuncTable plpx_tab;
    static CxFuncTable plpxcoi_tab;
    static int inittab = 0;

    CX_FUNCNAME( "cxCxtPlaneToPix" );

    __BEGIN__;

    int src_step = 0, dst_step = 0;
    CxMat stub[5], *src[4], *dst = (CxMat*)dstarr;
    CxSize size;
    const void* srcptr[4] = { 0, 0, 0, 0 };
    int type, cn, coi = 0;
    int i, nzplanes = 0, nzidx = -1;
    int cont_flag;

    if( !inittab )
    {
        icxInitCxtPlaneToPixRTable( &plpx_tab );
        icxInitCxtPlaneToPixRCoiTable( &plpxcoi_tab );
        inittab = 1;
    }

    src[0] = (CxMat*)srcarr0;
    src[1] = (CxMat*)srcarr1;
    src[2] = (CxMat*)srcarr2;
    src[3] = (CxMat*)srcarr3;

    CX_CALL( dst = cxGetMat( dst, stub + 4, &coi ));

    type = CX_MAT_TYPE( dst->type );
    cn = CX_MAT_CN( type );

    cont_flag = dst->type;

    if( cn == 1 )
        CX_ERROR( CX_BadNumChannels, "" );

    for( i = 0; i < 4; i++ )
    {
        if( src[i] )
        {
            nzplanes++;
            nzidx = i;
            CX_CALL( src[i] = cxGetMat( src[i], stub + i ));
            if( CX_MAT_CN( src[i]->type ) != 1 )
                CX_ERROR( CX_BadNumChannels, "" );
            if( !CX_ARE_DEPTHS_EQ( src[i], dst ))
                CX_ERROR( CX_StsUnmatchedFormats, "" );
            if( !CX_ARE_SIZES_EQ( src[i], dst ))
                CX_ERROR( CX_StsUnmatchedSizes, "" );
            if( nzplanes > i && i > 0 && src[i]->step != src[i-1]->step )
                CX_ERROR( CX_BadStep, "" );
            src_step = src[i]->step;
            srcptr[nzplanes-1] = (const void*)(src[i]->data.ptr);

            cont_flag &= src[i]->type;
        }
    }

    size = cxGetMatSize( dst );
    dst_step = dst->step;

    if( CX_IS_MAT_CONT( cont_flag ))
    {
        size.width *= size.height;
        src_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }

    if( nzplanes == cn )
    {
        CxCxtPlaneToPixFunc func = (CxCxtPlaneToPixFunc)plpx_tab.fn_2d[type];

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( srcptr, src_step, dst->data.ptr, dst_step, size ));
    }
    else if( nzplanes == 1 )
    {
        CxCxtPlaneToPixCoiFunc func = (CxCxtPlaneToPixCoiFunc)
                                      plpxcoi_tab.fn_2d[CX_MAT_DEPTH(type)];

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src[nzidx]->data.ptr, src_step,
                         dst->data.ptr, dst_step,
                         size, cn, nzidx + 1 ));
    }
    else
    {
        CX_ERROR( CX_StsBadArg,
            "Either all input planes or only one input plane should be non zero" );
    }

    __END__;
}



/****************************************************************************************\
*                                   cxConvertScaleAbs                                    *
\****************************************************************************************/


#define ICX_DEF_CXT_SCALE_ABS_CASE( worktype, cast_macro1,              \
                                    scale_macro, abs_macro,             \
                                    cast_macro2, src, a, b )            \
                                                                        \
{                                                                       \
    for( ; size.height--; (char*&)(src) += srcstep,                     \
                          (char*&)(dst) += dststep )                    \
    {                                                                   \
        int i;                                                          \
                                                                        \
        for( i = 0; i <= size.width - 4; i += 4 )                       \
        {                                                               \
            worktype t0 = scale_macro((a)*cast_macro1((src)[i]) + (b)); \
            worktype t1 = scale_macro((a)*cast_macro1((src)[i+1])+(b)); \
                                                                        \
            t0 = (worktype)abs_macro(t0);                               \
            t1 = (worktype)abs_macro(t1);                               \
                                                                        \
            dst[i] = cast_macro2(t0);                                   \
            dst[i+1] = cast_macro2(t1);                                 \
                                                                        \
            t0 = scale_macro((a)*cast_macro1((src)[i+2]) + (b));        \
            t1 = scale_macro((a)*cast_macro1((src)[i+3]) + (b));        \
                                                                        \
            t0 = (worktype)abs_macro(t0);                               \
            t1 = (worktype)abs_macro(t1);                               \
                                                                        \
            dst[i+2] = cast_macro2(t0);                                 \
            dst[i+3] = cast_macro2(t1);                                 \
        }                                                               \
                                                                        \
        for( ; i < size.width; i++ )                                    \
        {                                                               \
            worktype t0 = scale_macro((a)*cast_macro1((src)[i]) + (b)); \
            t0 = (worktype)abs_macro(t0);                               \
            dst[i] = cast_macro2(t0);                                   \
        }                                                               \
    }                                                                   \
}


#define ICX_FIX_SHIFT  15
#define ICX_SCALE(x)   (((x) + (1 << (ICX_FIX_SHIFT-1))) >> ICX_FIX_SHIFT)

static CxStatus CX_STDCALL
icxCxtScaleAbsTo_8u_C1R( const uchar* src, int srcstep,
                         uchar* dst, int dststep,
                         CxSize size, double scale, double shift,
                         int param )
{
    int srctype = param;

    size.width *= CX_MAT_CN(srctype);

    switch( CX_MAT_DEPTH(srctype) )
    {
    case  CX_8S:
        if( fabs( scale ) <= 128. &&
            fabs( shift ) <= (INT_MAX*0.5)/(1 << ICX_FIX_SHIFT))
        {
            int iscale = cxRound(scale*(1 << ICX_FIX_SHIFT));
            int ishift = cxRound(shift*(1 << ICX_FIX_SHIFT));

            if( iscale == ICX_FIX_SHIFT && ishift == 0 )
            {
                ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, CX_NOP, CX_IABS,
                                            CX_CAST_8U, (char*&)src, 1, 0 )
            }
            else
            {
                ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, ICX_SCALE, CX_IABS,
                                       CX_CAST_8U, (char*&)src, iscale, ishift )
            }
        }
        else
        {
            ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, cxRound, CX_IABS,
                                   CX_CAST_8U, (char*&)src, scale, shift )
        }
        break;
    case  CX_8U:
        if( fabs( scale ) <= 128. &&
            fabs( shift ) <= (INT_MAX*0.5)/(1 << ICX_FIX_SHIFT))
        {
            int iscale = cxRound(scale*(1 << ICX_FIX_SHIFT));
            int ishift = cxRound(shift*(1 << ICX_FIX_SHIFT));

            if( iscale == ICX_FIX_SHIFT && ishift == 0 )
            {
                icxCopy_8u_C1R( src, srcstep, dst, dststep, size );
            }
            else
            {
                ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, ICX_SCALE, CX_IABS,
                                   CX_CAST_8U, (uchar*&)src, iscale, ishift );
            }
        }
        else
        {
            ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, cxRound, CX_IABS,
                                   CX_CAST_8U, (uchar*&)src, scale, shift );
        }
        break;
    case  CX_16S:
        if( fabs( scale ) <= 1. &&
            fabs( shift ) <= (INT_MAX*0.5)/(1 << ICX_FIX_SHIFT))
        {
            int iscale = cxRound(scale*(1 << ICX_FIX_SHIFT));
            int ishift = cxRound(shift*(1 << ICX_FIX_SHIFT));

            if( iscale == ICX_FIX_SHIFT && ishift == 0 )
            {
                ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, CX_NOP, CX_IABS,
                                       CX_CAST_8U, (short*&)src, 1, 0 );
            }
            else
            {
                ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, ICX_SCALE, CX_IABS,
                                    CX_CAST_8U, (short*&)src, iscale, ishift );
            }
        }
        else
        {
            ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, cxRound, CX_IABS,
                                   CX_CAST_8U, (short*&)src, scale, shift );
        }
        break;
    case  CX_32S:
        ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, cxRound, CX_IABS,
                                    CX_CAST_8U, (int*&)src, scale, shift );
        break;
    case  CX_32F:
        ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, cxRound, CX_IABS,
                                    CX_CAST_8U, (float*&)src, scale, shift );
        break;
    case  CX_64F:
        ICX_DEF_CXT_SCALE_ABS_CASE( int, CX_NOP, cxRound, CX_IABS,
                                    CX_CAST_8U, (double*&)src, scale, shift );
        break;
    default:
        assert(0);
        return CX_BADFLAG_ERR;
    }

    return  CX_OK;
}


CX_IMPL void
cxConvertScaleAbs( const void* srcarr, void* dstarr,
                   double scale, double shift )
{
    CX_FUNCNAME( "cxConvertScaleAbs" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    CxMat  srcstub, *src = (CxMat*)srcarr;
    CxMat  dststub, *dst = (CxMat*)dstarr;
    CxSize size;
    int src_step, dst_step;

    CX_CALL( src = cxGetMat( src, &srcstub, &coi1 ));
    CX_CALL( dst = cxGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CX_ERROR( CX_BadCOI, "" );

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    if( !CX_ARE_CNS_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( CX_MAT_DEPTH( dst->type ) != CX_8U )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    size = cxGetMatSize( src );
    src_step = src->step;
    dst_step = dst->step;

    if( CX_IS_MAT_CONT( src->type & dst->type ))
    {
        size.width *= size.height;
        src_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }

    IPPI_CALL( icxCxtScaleAbsTo_8u_C1R( src->data.ptr, src_step,
                             (uchar*)(dst->data.ptr), dst_step,
                             size, scale, shift, CX_MAT_TYPE(src->type)));
    __END__;
}


/****************************************************************************************\
*                                    LUT Transform                                       *
\****************************************************************************************/

#define  ICX_DEF_LUT_FUNC( flavor, dsttype )                        \
static CxStatus CX_STDCALL                                          \
icxLUT_Transform_##flavor##_C1R( const void* srcptr, int srcstep,   \
                           void* dstptr, int dststep, CxSize size,  \
                           const void* lutptr )                     \
{                                                                   \
    const uchar* src = (const uchar*)srcptr;                        \
    dsttype* dst = (dsttype*)dstptr;                                \
    const dsttype* lut = (const dsttype*)lutptr;                    \
                                                                    \
    for( ; size.height--; src += srcstep,                           \
                          (char*&)dst += dststep )                  \
    {                                                               \
        int i;                                                      \
                                                                    \
        for( i = 0; i <= size.width - 4; i += 4 )                   \
        {                                                           \
            dsttype t0 = lut[src[i]];                               \
            dsttype t1 = lut[src[i+1]];                             \
                                                                    \
            dst[i] = t0;                                            \
            dst[i+1] = t1;                                          \
                                                                    \
            t0 = lut[src[i+2]];                                     \
            t1 = lut[src[i+3]];                                     \
                                                                    \
            dst[i+2] = t0;                                          \
            dst[i+3] = t1;                                          \
        }                                                           \
                                                                    \
        for( ; i < size.width; i++ )                                \
        {                                                           \
            dsttype t0 = lut[src[i]];                               \
            dst[i] = t0;                                            \
        }                                                           \
    }                                                               \
                                                                    \
    return CX_OK;                                                   \
}


ICX_DEF_LUT_FUNC( 8u, uchar )
ICX_DEF_LUT_FUNC( 16s, ushort )
ICX_DEF_LUT_FUNC( 32s, int )
ICX_DEF_LUT_FUNC( 64f, int64 )

#define  icxLUT_Transform_8s_C1R   icxLUT_Transform_8u_C1R  
#define  icxLUT_Transform_32f_C1R   icxLUT_Transform_32s_C1R

CX_DEF_INIT_FUNC_TAB_2D( LUT_Transform, C1R )


CX_IMPL  void
cxLUT( const void* srcarr, void* dstarr, const void* lutarr )
{
    static  CxFuncTable  lut_tab;
    static  int inittab = 0;

    CX_FUNCNAME( "cxLUT" );

    __BEGIN__;

    int  coi1 = 0, coi2 = 0;
    CxMat  srcstub, *src = (CxMat*)srcarr;
    CxMat  dststub, *dst = (CxMat*)dstarr;
    CxMat  lutstub, *lut = (CxMat*)lutarr;
    uchar* lut_data;
    uchar  shuffled_lut[256*sizeof(double)];
    CxFunc2D_2A1P func = 0;
    CxSize size;
    int src_step, dst_step;

    if( !inittab )
    {
        icxInitLUT_TransformC1RTable( &lut_tab );
        inittab = 1;
    }

    CX_CALL( src = cxGetMat( src, &srcstub, &coi1 ));
    CX_CALL( dst = cxGetMat( dst, &dststub, &coi2 ));
    CX_CALL( lut = cxGetMat( lut, &lutstub ));

    if( coi1 != 0 || coi2 != 0 )
        CX_ERROR( CX_BadCOI, "" );

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    if( !CX_ARE_CNS_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( CX_MAT_DEPTH( src->type ) > CX_8S )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    if( !CX_IS_MAT_CONT(lut->type) || CX_MAT_CN(lut->type) != 1 ||
        !CX_ARE_DEPTHS_EQ( dst, lut ) || lut->width*lut->height != 256 )
        CX_ERROR( CX_StsBadArg, "The LUT must be continuous, single-channel array \n"
                                "with 256 elements of the same type as destination" );

    size = cxGetMatSize( src );
    size.width *= CX_MAT_CN( src->type );
    src_step = src->step;
    dst_step = dst->step;

    if( CX_IS_MAT_CONT( src->type & dst->type ))
    {
        size.width *= size.height;
        src_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }

    lut_data = lut->data.ptr;

    if( CX_MAT_DEPTH( src->type ) == CX_8S )
    {
        int half_size = icxPixSize[CX_MAT_TYPE(lut->type)]*128;

        // shuffle lut
        memcpy( shuffled_lut, lut_data + half_size, half_size );
        memcpy( shuffled_lut + half_size, lut_data, half_size );

        lut_data = shuffled_lut;
    }

    func = (CxFunc2D_2A1P)(lut_tab.fn_2d[CX_MAT_DEPTH(src->type)]);
    if( !func )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src->data.ptr, src_step, dst->data.ptr,
                     dst_step, size, lut_data));
    __END__;
}



/****************************************************************************************\
*                                      cxConvertScale                                    *
\****************************************************************************************/

#define ICX_DEF_CXT_SCALE_CASE( worktype, cast_macro1,                  \
                                scale_macro, cast_macro2, src, a, b )   \
                                                                        \
{                                                                       \
    for( ; size.height--; (char*&)(src) += srcstep,                     \
                          (char*&)(dst) += dststep )                    \
    {                                                                   \
        for( i = 0; i <= size.width - 4; i += 4 )                       \
        {                                                               \
            worktype t0 = scale_macro((a)*cast_macro1((src)[i])+(b));   \
            worktype t1 = scale_macro((a)*cast_macro1((src)[i+1])+(b)); \
                                                                        \
            dst[i] = cast_macro2(t0);                                   \
            dst[i+1] = cast_macro2(t1);                                 \
                                                                        \
            t0 = scale_macro((a)*cast_macro1((src)[i+2]) + (b));        \
            t1 = scale_macro((a)*cast_macro1((src)[i+3]) + (b));        \
                                                                        \
            dst[i+2] = cast_macro2(t0);                                 \
            dst[i+3] = cast_macro2(t1);                                 \
        }                                                               \
                                                                        \
        for( ; i < size.width; i++ )                                    \
        {                                                               \
            worktype t0 = scale_macro((a)*cast_macro1((src)[i]) + (b)); \
            dst[i] = cast_macro2(t0);                                   \
        }                                                               \
    }                                                                   \
}


#define  ICX_DEF_CXT_SCALE_FUNC_INT( flavor, dsttype, cast_macro )      \
static  CxStatus  CX_STDCALL                                            \
icxCxtScaleTo_##flavor##_C1R( const char* src, int srcstep,             \
                              dsttype* dst, int dststep,                \
                              CxSize size, double scale, double shift,  \
                              int param )                               \
{                                                                       \
    int i, srctype = param;                                             \
    dsttype lut[256];                                                   \
                                                                        \
    switch( CX_MAT_DEPTH(srctype) )                                     \
    {                                                                   \
    case  CX_8U:                                                        \
        if( size.width*size.height >= 256 )                             \
        {                                                               \
            for( i = 0; i < 256; i++ )                                  \
            {                                                           \
                int t = cxRound( i*scale + shift );                     \
                lut[i] = cast_macro(t);                                 \
            }                                                           \
                                                                        \
            icxLUT_Transform_##flavor##_C1R( src, srcstep, dst,         \
                                        dststep, size, lut );           \
        }                                                               \
        else if( fabs( scale ) <= 128. &&                               \
                 fabs( shift ) <= (INT_MAX*0.5)/(1 << ICX_FIX_SHIFT))   \
        {                                                               \
            int iscale = cxRound(scale*(1 << ICX_FIX_SHIFT));           \
            int ishift = cxRound(shift*(1 << ICX_FIX_SHIFT));           \
                                                                        \
            ICX_DEF_CXT_SCALE_CASE( int, CX_NOP, ICX_SCALE, cast_macro, \
                                    (uchar*&)src, iscale, ishift );     \
        }                                                               \
        else                                                            \
        {                                                               \
            ICX_DEF_CXT_SCALE_CASE( int, CX_NOP, cxRound, cast_macro,   \
                                    (uchar*&)src, scale, shift );       \
        }                                                               \
        break;                                                          \
    case  CX_8S:                                                        \
        if( size.width*size.height >= 256 )                             \
        {                                                               \
            for( i = 0; i < 256; i++ )                                  \
            {                                                           \
                int t = cxRound( (char)i*scale + shift );               \
                lut[i] = CX_CAST_8U(t);                                 \
            }                                                           \
                                                                        \
            icxLUT_Transform_##flavor##_C1R( src, srcstep, dst,         \
                                             dststep, size, lut );      \
        }                                                               \
        else if( fabs( scale ) <= 128. &&                               \
                 fabs( shift ) <= (INT_MAX*0.5)/(1 << ICX_FIX_SHIFT))   \
        {                                                               \
            int iscale = cxRound(scale*(1 << ICX_FIX_SHIFT));           \
            int ishift = cxRound(shift*(1 << ICX_FIX_SHIFT));           \
                                                                        \
            ICX_DEF_CXT_SCALE_CASE( int, CX_NOP, ICX_SCALE, cast_macro, \
                                    (char*&)src, iscale, ishift );      \
        }                                                               \
        else                                                            \
        {                                                               \
            ICX_DEF_CXT_SCALE_CASE( int, CX_NOP, cxRound, cast_macro,   \
                                    (char*&)src, scale, shift );        \
        }                                                               \
        break;                                                          \
    case  CX_16S:                                                       \
        if( fabs( scale ) <= 1. &&                                      \
            fabs( shift ) <= (INT_MAX*0.5)/(1 << ICX_FIX_SHIFT))        \
        {                                                               \
            int iscale = cxRound(scale*(1 << ICX_FIX_SHIFT));           \
            int ishift = cxRound(shift*(1 << ICX_FIX_SHIFT));           \
                                                                        \
            ICX_DEF_CXT_SCALE_CASE( int, CX_NOP, ICX_SCALE, cast_macro, \
                                    (short*&)src, iscale, ishift );     \
        }                                                               \
        else                                                            \
        {                                                               \
            ICX_DEF_CXT_SCALE_CASE( int, CX_NOP, cxRound, cast_macro,   \
                                    (short*&)src, scale, shift );       \
        }                                                               \
        break;                                                          \
    case  CX_32S:                                                       \
        ICX_DEF_CXT_SCALE_CASE( int, CX_NOP, cxRound, cast_macro,       \
                                (int*&)src, scale, shift )              \
        break;                                                          \
    case  CX_32F:                                                       \
        ICX_DEF_CXT_SCALE_CASE( int, CX_NOP, cxRound, cast_macro,       \
                                (float*&)src, scale, shift )            \
        break;                                                          \
    case  CX_64F:                                                       \
        ICX_DEF_CXT_SCALE_CASE( int, CX_NOP, cxRound, cast_macro,       \
                                (double*&)src, scale, shift )           \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
        return CX_BADFLAG_ERR;                                          \
    }                                                                   \
                                                                        \
    return  CX_OK;                                                      \
}


#define  ICX_DEF_CXT_SCALE_FUNC_FLT( flavor, dsttype, cast_macro )      \
static  CxStatus  CX_STDCALL                                            \
icxCxtScaleTo_##flavor##_C1R( const char* src, int srcstep,             \
                              dsttype* dst, int dststep,                \
                              CxSize size, double scale, double shift,  \
                              int param )                               \
{                                                                       \
    int i, srctype = param;                                             \
    dsttype lut[256];                                                   \
                                                                        \
    switch( CX_MAT_DEPTH(srctype) )                                     \
    {                                                                   \
    case  CX_8U:                                                        \
        if( size.width*size.height >= 256 )                             \
        {                                                               \
            for( i = 0; i < 256; i++ )                                  \
            {                                                           \
                lut[i] = (dsttype)(i*scale + shift);                    \
            }                                                           \
                                                                        \
            icxLUT_Transform_##flavor##_C1R( src, srcstep, dst,         \
                                        dststep, size, lut );           \
        }                                                               \
        else                                                            \
        {                                                               \
            ICX_DEF_CXT_SCALE_CASE( dsttype, CX_8TO32F, cast_macro,     \
                                CX_NOP, (uchar*&)src, scale, shift );   \
        }                                                               \
        break;                                                          \
    case  CX_8S:                                                        \
        if( size.width*size.height >= 256 )                             \
        {                                                               \
            for( i = 0; i < 256; i++ )                                  \
            {                                                           \
                lut[i] = (dsttype)((char)i*scale + shift);              \
            }                                                           \
                                                                        \
            icxLUT_Transform_##flavor##_C1R( src, srcstep, dst,         \
                                             dststep, size, lut );      \
        }                                                               \
        else                                                            \
        {                                                               \
            ICX_DEF_CXT_SCALE_CASE( dsttype, CX_8TO32F, cast_macro,     \
                                CX_NOP, (char*&)src, scale, shift );    \
        }                                                               \
        break;                                                          \
    case  CX_16S:                                                       \
        ICX_DEF_CXT_SCALE_CASE( dsttype, CX_NOP, cast_macro, CX_NOP,    \
                                (short*&)src, scale, shift );           \
        break;                                                          \
    case  CX_32S:                                                       \
        ICX_DEF_CXT_SCALE_CASE( dsttype, CX_NOP, cast_macro, CX_NOP,    \
                                (int*&)src, scale, shift )              \
        break;                                                          \
    case  CX_32F:                                                       \
        ICX_DEF_CXT_SCALE_CASE( dsttype, CX_NOP, cast_macro, CX_NOP,    \
                                (float*&)src, scale, shift )            \
        break;                                                          \
    case  CX_64F:                                                       \
        ICX_DEF_CXT_SCALE_CASE( dsttype, CX_NOP, cast_macro, CX_NOP,    \
                                (double*&)src, scale, shift )           \
        break;                                                          \
    default:                                                            \
        assert(0);                                                      \
        return CX_BADFLAG_ERR;                                          \
    }                                                                   \
                                                                        \
    return  CX_OK;                                                      \
}


ICX_DEF_CXT_SCALE_FUNC_INT( 8u, uchar, CX_CAST_8U )
ICX_DEF_CXT_SCALE_FUNC_INT( 8s, char, CX_CAST_8S )
ICX_DEF_CXT_SCALE_FUNC_INT( 16s, short, CX_CAST_16S )
ICX_DEF_CXT_SCALE_FUNC_INT( 32s, int, CX_CAST_32S )

ICX_DEF_CXT_SCALE_FUNC_FLT( 32f, float, CX_CAST_32F )
ICX_DEF_CXT_SCALE_FUNC_FLT( 64f, double, CX_CAST_64F )

CX_DEF_INIT_FUNC_TAB_2D( CxtScaleTo, C1R )


/****************************************************************************************\
*                             Conversion w/o scaling macros                              *
\****************************************************************************************/

#define ICX_DEF_CXT_CASE_2D( src_type, dst_type, work_type, cast_macro1, cast_macro2, \
                             src_ptr, src_step, dst_ptr, dst_step, size )           \
{                                                                                   \
    for( ; (size).height--; (src_ptr) += (src_step), (dst_ptr) += (dst_step))       \
    {                                                                               \
        int i;                                                                      \
                                                                                    \
        for( i = 0; i <= (size).width - 4; i += 4 )                                 \
        {                                                                           \
            work_type t0 = cast_macro1(((src_type*)(src_ptr))[i]);                  \
            work_type t1 = cast_macro1(((src_type*)(src_ptr))[i+1]);                \
                                                                                    \
            ((dst_type*)(dst_ptr))[i] = cast_macro2(t0);                            \
            ((dst_type*)(dst_ptr))[i+1] = cast_macro2(t1);                          \
                                                                                    \
            t0 = cast_macro1(((src_type*)(src_ptr))[i+2]);                          \
            t1 = cast_macro1(((src_type*)(src_ptr))[i+3]);                          \
                                                                                    \
            ((dst_type*)(dst_ptr))[i+2] = cast_macro2(t0);                          \
            ((dst_type*)(dst_ptr))[i+3] = cast_macro2(t1);                          \
        }                                                                           \
                                                                                    \
        for( ; i < (size).width; i++ )                                              \
        {                                                                           \
            work_type t0 = cast_macro1(((src_type*)(src_ptr))[i]);                  \
            ((dst_type*)(dst_ptr))[i] = cast_macro2(t0);                            \
        }                                                                           \
    }                                                                               \
}


#define ICX_DEF_CXT_FUNC_2D( flavor, dst_type, work_type, cast_macro2,  \
                             src_depth1, src_type1, cast_macro11,       \
                             src_depth2, src_type2, cast_macro12,       \
                             src_depth3, src_type3, cast_macro13,       \
                             src_depth4, src_type4, cast_macro14,       \
                             src_depth5, src_type5, cast_macro15 )      \
IPCXAPI_IMPL( CxStatus,                                                 \
icxCxtTo_##flavor##_C1R,( const void* pSrc, int step1,                  \
                          void* pDst, int step,                         \
                          CxSize size, int param ))                     \
{                                                                       \
    int srctype = param;                                                \
    const char* src = (const char*)pSrc;                                \
    char* dst = (char*)pDst;                                            \
                                                                        \
    switch( CX_MAT_DEPTH(srctype) )                                     \
    {                                                                   \
    case src_depth1:                                                    \
        ICX_DEF_CXT_CASE_2D( src_type1, dst_type, work_type,            \
                             cast_macro11, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    case src_depth2:                                                    \
        ICX_DEF_CXT_CASE_2D( src_type2, dst_type, work_type,            \
                             cast_macro12, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    case src_depth3:                                                    \
        ICX_DEF_CXT_CASE_2D( src_type3, dst_type, work_type,            \
                             cast_macro13, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    case src_depth4:                                                    \
        ICX_DEF_CXT_CASE_2D( src_type4, dst_type, work_type,            \
                             cast_macro14, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    case src_depth5:                                                    \
        ICX_DEF_CXT_CASE_2D( src_type5, dst_type, work_type,            \
                             cast_macro15, cast_macro2,                 \
                             src, step1, dst, step, size );             \
        break;                                                          \
    }                                                                   \
                                                                        \
    return  CX_OK;                                                      \
}


ICX_DEF_CXT_FUNC_2D( 8u, uchar, int, CX_CAST_8U,
                     CX_8S,  char,   CX_NOP,
                     CX_16S, short,  CX_NOP,
                     CX_32S, int,    CX_NOP,
                     CX_32F, float,  cxRound,
                     CX_64F, double, cxRound )

ICX_DEF_CXT_FUNC_2D( 8s, char, int, CX_CAST_8S,
                     CX_8U,  uchar,  CX_NOP,
                     CX_16S, short,  CX_NOP,
                     CX_32S, int,    CX_NOP,
                     CX_32F, float,  cxRound,
                     CX_64F, double, cxRound )

ICX_DEF_CXT_FUNC_2D( 16s, short, int, CX_CAST_16S,
                     CX_8U,  uchar,  CX_NOP,
                     CX_8S,  char,   CX_NOP,
                     CX_32S, int,    CX_NOP,
                     CX_32F, float,  cxRound,
                     CX_64F, double, cxRound )

ICX_DEF_CXT_FUNC_2D( 32s, int, int, CX_NOP,
                     CX_8U,  uchar,  CX_NOP,
                     CX_8S,  char,   CX_NOP,
                     CX_16S, short,  CX_NOP,
                     CX_32F, float,  cxRound,
                     CX_64F, double, cxRound )

ICX_DEF_CXT_FUNC_2D( 32f, float, float, CX_NOP,
                     CX_8U,  uchar,  CX_8TO32F,
                     CX_8S,  char,   CX_8TO32F,
                     CX_16S, short,  CX_NOP,
                     CX_32S, int,    CX_CAST_32F,
                     CX_64F, double, CX_CAST_32F )

ICX_DEF_CXT_FUNC_2D( 64f, double, double, CX_NOP,
                     CX_8U,  uchar,  CX_8TO32F,
                     CX_8S,  char,   CX_8TO32F,
                     CX_16S, short,  CX_NOP,
                     CX_32S, int,    CX_NOP,
                     CX_32F, float,  CX_NOP )

CX_DEF_INIT_FUNC_TAB_2D( CxtTo, C1R )


typedef  CxStatus (CX_STDCALL *CxCxtFunc)( const void* src, int srcstep,
                                           void* dst, int dststep, CxSize size,
                                           int param );

typedef  CxStatus (CX_STDCALL *CxCxtScaleFunc)( const void* src, int srcstep,
                                             void* dst, int dststep, CxSize size,
                                             double scale, double shift,
                                             int param );

CX_IMPL void
cxConvertScale( const void* srcarr, void* dstarr,
                double scale, double shift )
{
    static CxFuncTable cxt_tab, cxtscale_tab;
    static int inittab = 0;
    
    CX_FUNCNAME( "cxConvertScale" );

    __BEGIN__;

    int type;
    int is_nd = 0;
    CxMat  srcstub, *src = (CxMat*)srcarr;
    CxMat  dststub, *dst = (CxMat*)dstarr;
    CxSize size;
    int src_step, dst_step;

    if( !CX_IS_MAT(src) )
    {
        if( CX_IS_MATND(src) )
            is_nd = 1;
        else
        {
            int coi = 0;
            CX_CALL( src = cxGetMat( src, &srcstub, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    if( !CX_IS_MAT(dst) )
    {
        if( CX_IS_MATND(dst) )
            is_nd = 1;
        else
        {
            int coi = 0;
            CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    if( is_nd )
    {
        CxArr* arrs[] = { src, dst };
        CxMatND stubs[2];
        CxNArrayIterator iterator;
        int dsttype;

        CX_CALL( cxInitNArrayIterator( 2, arrs, 0, stubs, &iterator, CX_NO_DEPTH_CHECK ));

        type = iterator.hdr[0]->type;
        dsttype = iterator.hdr[1]->type;
        iterator.size.width *= CX_MAT_CN(type);

        if( !inittab )
        {
            icxInitCxtToC1RTable( &cxt_tab );
            icxInitCxtScaleToC1RTable( &cxtscale_tab );
            inittab = 1;
        }

        if( scale == 1 && shift == 0 )
        {
            CxCxtFunc func = (CxCxtFunc)(cxt_tab.fn_2d[CX_MAT_DEPTH(dsttype)]);
            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            do
            {
                IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                                 iterator.ptr[1], CX_STUB_STEP,
                                 iterator.size, type ));
            }
            while( cxNextNArraySlice( &iterator ));
        }
        else
        {
            CxCxtScaleFunc func =
                (CxCxtScaleFunc)(cxtscale_tab.fn_2d[CX_MAT_DEPTH(dsttype)]);
            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            do
            {
                IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                                 iterator.ptr[1], CX_STUB_STEP,
                                 iterator.size, scale, shift, type ));
            }
            while( cxNextNArraySlice( &iterator ));
        }
        EXIT;
    }

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    size = cxGetMatSize( src );
    type = CX_MAT_TYPE(src->type);
    src_step = src->step;
    dst_step = dst->step;

    if( CX_IS_MAT_CONT( src->type & dst->type ))
    {
        size.width *= size.height;
        src_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }

    size.width *= CX_MAT_CN( type );

    if( CX_ARE_TYPES_EQ( src, dst ) && size.height == 1 &&
        size.width <= CX_MAX_INLINE_MAT_OP_SIZE )
    {
        if( CX_MAT_DEPTH(type) == CX_32F )
        {
            const float* srcdata = (const float*)(src->data.ptr);
            float* dstdata = (float*)(dst->data.ptr);

            do
            {
                dstdata[size.width - 1] = (float)(srcdata[size.width-1]*scale + shift);
            }
            while( --size.width );

            EXIT;
        }

        if( CX_MAT_DEPTH(type) == CX_64F )
        {
            const double* srcdata = (const double*)(src->data.ptr);
            double* dstdata = (double*)(dst->data.ptr);

            do
            {
                dstdata[size.width - 1] = srcdata[size.width-1]*scale + shift;
            }
            while( --size.width );

            EXIT;
        }
    }

    if( !inittab )
    {
        icxInitCxtToC1RTable( &cxt_tab );
        icxInitCxtScaleToC1RTable( &cxtscale_tab );
        inittab = 1;
    }

    if( !CX_ARE_CNS_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( scale == 1 && shift == 0 )
    {
        if( !CX_ARE_DEPTHS_EQ( src, dst ))
        {
            CxCxtFunc func = (CxCxtFunc)(cxt_tab.fn_2d[CX_MAT_DEPTH(dst->type)]);

            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            IPPI_CALL( func( src->data.ptr, src_step,
                       dst->data.ptr, dst_step, size, type ));
            EXIT;
        }
    }
    else
    {
        CxCxtScaleFunc func = (CxCxtScaleFunc)
            (cxtscale_tab.fn_2d[CX_MAT_DEPTH(dst->type)]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, src_step,
                   dst->data.ptr, dst_step, size,
                   scale, shift, type ));
        EXIT;
    }

    CX_CALL( cxCopy( src, dst ));

    __END__;
}

/********************* helper functions for converting 32f<->64f ************************/

IPCXAPI_IMPL( CxStatus, icxCxt_32f64f, ( const float* src, double* dst, int len ))
{
    int i;
    for( i = 0; i <= len - 4; i += 4 )
    {
        double t0 = src[i];
        double t1 = src[i+1];

        dst[i] = t0;
        dst[i+1] = t1;

        t0 = src[i+2];
        t1 = src[i+3];

        dst[i+2] = t0;
        dst[i+3] = t1;
    }

    for( ; i < len; i++ )
        dst[i] = src[i];

    return CX_OK;
}


IPCXAPI_IMPL( CxStatus, icxCxt_64f32f, ( const double* src, float* dst, int len ))
{
    int i = 0;
    for( ; i <= len - 4; i += 4 )
    {
        double t0 = src[i];
        double t1 = src[i+1];

        dst[i] = (float)t0;
        dst[i+1] = (float)t1;

        t0 = src[i+2];
        t1 = src[i+3];

        dst[i+2] = (float)t0;
        dst[i+3] = (float)t1;
    }

    for( ; i < len; i++ )
        dst[i] = (float)src[i];

    return CX_OK;
}


IPCXAPI_IMPL( CxStatus, icxScale_32f, ( const float* src, float* dst,
                                        int len, float a, float b ))
{
    int i;
    for( i = 0; i <= len - 4; i += 4 )
    {
        double t0 = src[i]*a + b;
        double t1 = src[i+1]*a + b;

        dst[i] = (float)t0;
        dst[i+1] = (float)t1;

        t0 = src[i+2]*a + b;
        t1 = src[i+3]*a + b;

        dst[i+2] = (float)t0;
        dst[i+3] = (float)t1;
    }

    for( ; i < len; i++ )
        dst[i] = (float)(src[i]*a + b);

    return CX_OK;
}


IPCXAPI_IMPL( CxStatus, icxScale_64f, ( const double* src, double* dst,
                                        int len, double a, double b ))
{
    int i;
    for( i = 0; i <= len - 4; i += 4 )
    {
        double t0 = src[i]*a + b;
        double t1 = src[i+1]*a + b;

        dst[i] = t0;
        dst[i+1] = t1;

        t0 = src[i+2]*a + b;
        t1 = src[i+3]*a + b;

        dst[i+2] = t0;
        dst[i+3] = t1;
    }

    for( ; i < len; i++ )
        dst[i] = src[i]*a + b;

    return CX_OK;
}

/* End of file. */
