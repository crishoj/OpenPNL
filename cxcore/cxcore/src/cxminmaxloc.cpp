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

static CxStatus
icxWriteMinMaxResults( double min_val, double max_val,
                       int min_loc, int max_loc,
                       int width, void *minVal, void *maxVal,
                       CxPoint * minLoc, CxPoint * maxLoc, int is_double )
{
    if( (min_loc | max_loc) < 0 )
    {
        min_loc = max_loc = 0;
        min_val = max_val = 0.f;
    }

    if( is_double )
    {
        if( minVal )
            *(double*)minVal = min_val;
        if( maxVal )
            *(double*)maxVal = max_val;
    }
    else
    {
        if( minVal )
            *(float*)minVal = (float)min_val;
        if( maxVal )
            *(float*)maxVal = (float)max_val;
    }

    if( minLoc )
    {
        minLoc->y = min_loc / width;
        minLoc->x = min_loc - minLoc->y * width;
    }

    if( maxLoc )
    {
        maxLoc->y = max_loc / width;
        maxLoc->x = max_loc - maxLoc->y * width;
    }
    return CX_NO_ERR;
}


/****************************************************************************************\
*                                     MinMaxLoc                                          *
\****************************************************************************************/
                                                                    
#define CX_MINMAXLOC_ENTRY( _cast_macro_, _toggle_, srctype, temptype, cn )\
    temptype min_val, max_val;                      \
    int min_loc = 0, max_loc = 0;                   \
    int x, loc = 0, width = size.width;             \
                                                    \
    if( (int)(width*(cn)*sizeof(srctype)) == step ) \
    {                                               \
        width *= size.height;                       \
        size.height = 1;                            \
    }                                               \
                                                    \
    min_val = _cast_macro_((src)[0]);               \
    min_val = max_val = _toggle_( min_val )



#define CX_MINMAXLOC_EXIT( _toggle_, _fin_cast_macro_ ) \
    min_val = _toggle_( min_val );                  \
    max_val = _toggle_( max_val );                  \
                                                    \
    return  icxWriteMinMaxResults(                  \
        _fin_cast_macro_(min_val),                  \
        _fin_cast_macro_(max_val),                  \
        min_loc, max_loc, size.width,               \
        minVal, maxVal, minLoc, maxLoc,             \
        sizeof(*minVal) == sizeof(double) )



#define ICX_DEF_MINMAXLOC_1D_CASE_COI( _cast_macro_, _toggle_, temptype, src, len,  \
                                       min_val, max_val, min_loc, max_loc, loc, cn )\
{                                                                                   \
    for( x = 0; x < (len)*(cn); x += (cn), (loc)++ )                                \
    {                                                                               \
        temptype val = _cast_macro_((src)[x]);                                      \
        val = _toggle_(val);                                                        \
                                                                                    \
        if( val < (min_val) )                                                       \
        {                                                                           \
            (min_val) = val;                                                        \
            (min_loc) = (loc);                                                      \
        }                                                                           \
        else if( val > (max_val) )                                                  \
        {                                                                           \
            (max_val) = val;                                                        \
            (max_loc) = (loc);                                                      \
        }                                                                           \
    }                                                                               \
}



#define ICX_DEF_MINMAXLOC_1D_CASE_C1( _cast_macro_, _toggle_, temptype, src, len,   \
                                      min_val, max_val, min_loc, max_loc, loc )     \
    ICX_DEF_MINMAXLOC_1D_CASE_COI( _cast_macro_, _toggle_, temptype, src, len,      \
                                   min_val, max_val, min_loc, max_loc, loc, 1 )



#define ICX_DEF_MINMAXLOC_FUNC_2D( _cast_macro_, _toggle_, _fin_cast_macro_,        \
                                   _entry_, _exit_, flavor, srctype,                \
                                   temptype, extrtype )                             \
IPCXAPI_IMPL( CxStatus,                                                             \
icxMinMaxIndx_##flavor##_C1R,( const srctype* src, int step,                        \
                               CxSize size, extrtype* minVal, extrtype* maxVal,     \
                               CxPoint* minLoc, CxPoint* maxLoc ))                  \
{                                                                                   \
    _entry_( _cast_macro_, _toggle_, srctype, temptype, 1 );                        \
                                                                                    \
    for( ; size.height--; (char*&)src += step )                                     \
    {                                                                               \
        ICX_DEF_MINMAXLOC_1D_CASE_C1( _cast_macro_, _toggle_, temptype, src, width, \
                                      min_val, max_val, min_loc, max_loc, loc );    \
    }                                                                               \
                                                                                    \
    _exit_( _toggle_, _fin_cast_macro_ );                                           \
}


#define ICX_DEF_MINMAXLOC_FUNC_2D_COI( _cast_macro_, _toggle_, _fin_cast_macro_,    \
                                       _entry_, _exit_, flavor,                     \
                                       srctype, temptype, extrtype  )               \
static CxStatus CX_STDCALL                                                          \
icxMinMaxIndx_##flavor##_CnCR( const srctype* src, int step,                        \
                          CxSize size, int cn, int coi,                             \
                          extrtype* minVal, extrtype* maxVal,                       \
                          CxPoint* minLoc, CxPoint* maxLoc )                        \
{                                                                                   \
    (src) += coi - 1;                                                               \
    _entry_( _cast_macro_, _toggle_, srctype, temptype, cn );                       \
                                                                                    \
    for( ; size.height--; (char*&)src += step )                                     \
    {                                                                               \
        ICX_DEF_MINMAXLOC_1D_CASE_COI( _cast_macro_, _toggle_, temptype, src, width,\
                                       min_val, max_val, min_loc, max_loc, loc, cn);\
    }                                                                               \
                                                                                    \
    _exit_( _toggle_, _fin_cast_macro_ );                                           \
}


#define ICX_DEF_MINMAXLOC_ALL( flavor, srctype, temptype, extrtype )                \
    ICX_DEF_MINMAXLOC_FUNC_2D( CX_NOP, CX_NOP, CX_CAST_64F, CX_MINMAXLOC_ENTRY,     \
                               CX_MINMAXLOC_EXIT, flavor,                           \
                               srctype, temptype, extrtype )                        \
    ICX_DEF_MINMAXLOC_FUNC_2D_COI( CX_NOP, CX_NOP, CX_CAST_64F, CX_MINMAXLOC_ENTRY, \
                                   CX_MINMAXLOC_EXIT, flavor, srctype, temptype, extrtype )

#define  _toggle_float_         CX_TOGGLE_FLT
#define  _toggle_double_        CX_TOGGLE_DBL
#define  _as_int_(x)            (*(int*)&(x))
#define  _as_float_(x)          (*(float*)&(x))
#define  _as_int64_(x)          (*(int64*)&(x))
#define  _as_double_(x)         (*(double*)&(x))


#define ICX_DEF_MINMAXLOC_ALL_FLT( flavor, srctype, temptype,                   \
                                   _cast_macro_, _toggle_,                      \
                                   _fin_cast_macro_, extrtype )                 \
                                                                                \
    ICX_DEF_MINMAXLOC_FUNC_2D( _cast_macro_, _toggle_, _fin_cast_macro_,        \
                               CX_MINMAXLOC_ENTRY, CX_MINMAXLOC_EXIT,           \
                               flavor, srctype, temptype, extrtype )            \
    ICX_DEF_MINMAXLOC_FUNC_2D_COI( _cast_macro_, _toggle_, _fin_cast_macro_,    \
                                   CX_MINMAXLOC_ENTRY, CX_MINMAXLOC_EXIT,       \
                                   flavor, srctype, temptype, extrtype )

ICX_DEF_MINMAXLOC_ALL( 8u, uchar, int, float )
ICX_DEF_MINMAXLOC_ALL( 16s, short, int, float )
ICX_DEF_MINMAXLOC_ALL( 32s, int, int, double )
ICX_DEF_MINMAXLOC_ALL_FLT( 32f, float, int, _as_int_,
                           _toggle_float_, _as_float_, float )
ICX_DEF_MINMAXLOC_ALL_FLT( 64f, double, int64, _as_int64_,
                           _toggle_double_, _as_double_, double )


/****************************************************************************************\
*                              MinMaxLoc with mask                                       *
\****************************************************************************************/


#define CX_MINMAXLOC_MASK_ENTRY( _cast_macro_, _toggle_,            \
                                 srctype, temptype, cn )            \
    temptype min_val = 0, max_val = 0;                              \
    int min_loc = -1, max_loc = -1;                                 \
    int x = 0, y, loc = 0, width = size.width;                      \
                                                                    \
    if( (int)(width*(cn)*sizeof(srctype)) == step &&                \
        width == maskStep )                                         \
    {                                                               \
        width *= size.height;                                       \
        size.height = 1;                                            \
    }                                                               \
                                                                    \
    for( y = 0; y < size.height; y++, (char*&)src += step,          \
                                      (char*&)mask += maskStep )    \
    {                                                               \
        for( x = 0; x < width; x++, loc++ )                         \
            if( mask[x] != 0 )                                      \
            {                                                       \
                min_loc = max_loc = loc;                            \
                min_val = _cast_macro_((src)[x*(cn)]);              \
                min_val = max_val = _toggle_( min_val );            \
                goto stop_scan;                                     \
            }                                                       \
    }                                                               \
                                                                    \
    stop_scan:;




#define ICX_DEF_MINMAXLOC_MASK_FUNC_2D(_cast_macro_, _toggle_, _fin_cast_macro_,\
                                       _entry_, _exit_, flavor,                 \
                                       srctype, temptype, extrtype )            \
IPCXAPI_IMPL( CxStatus,                                                         \
icxMinMaxIndx_##flavor##_C1MR,( const srctype* src, int step,                   \
                                const uchar* mask, int maskStep,                \
                                CxSize size, extrtype* minVal, extrtype* maxVal,\
                                CxPoint* minLoc, CxPoint* maxLoc ))             \
{                                                                               \
    _entry_( _cast_macro_, _toggle_, srctype, temptype, 1 );                    \
                                                                                \
    for( ; y < size.height; y++, (char*&)src += step,                           \
                                 (char*&)mask += maskStep )                     \
    {                                                                           \
        for( ; x < width; x++, (loc)++ )                                        \
        {                                                                       \
            temptype val = _cast_macro_((src)[x]);                              \
            int m = (mask)[x] != 0;                                             \
            val = _toggle_(val);                                                \
                                                                                \
            if( val < (min_val) && m )                                          \
            {                                                                   \
                (min_val) = val;                                                \
                (min_loc) = (loc);                                              \
            }                                                                   \
            else if( val > (max_val) && m )                                     \
            {                                                                   \
                (max_val) = val;                                                \
                (max_loc) = (loc);                                              \
            }                                                                   \
        }                                                                       \
        x = 0;                                                                  \
    }                                                                           \
                                                                                \
    _exit_( _toggle_, _fin_cast_macro_ );                                       \
}


#define ICX_DEF_MINMAXLOC_MASK_FUNC_2D_COI( _cast_macro_, _toggle_, _fin_cast_macro_,\
                                     _entry_, _exit_, flavor,                   \
                                     srctype, temptype, extrtype )              \
static CxStatus CX_STDCALL                                                      \
icxMinMaxIndx_##flavor##_CnCMR( const srctype* src, int step,                   \
                           const uchar* mask, int maskStep,                     \
                           CxSize size, int cn, int coi,                        \
                           extrtype* minVal, extrtype* maxVal,                  \
                           CxPoint* minLoc, CxPoint* maxLoc )                   \
{                                                                               \
    (src) += coi - 1;                                                           \
    _entry_( _cast_macro_, _toggle_, srctype, temptype, cn );                   \
                                                                                \
    for( ; y < size.height; y++, (char*&)src += step,                           \
                                 (char*&)mask += maskStep )                     \
    {                                                                           \
        for( ; x < width; x++, (loc)++ )                                        \
        {                                                                       \
            temptype val = _cast_macro_((src)[x*(cn)]);                         \
            int m = (mask)[x] != 0;                                             \
            val = _toggle_(val);                                                \
                                                                                \
            if( val < (min_val) && m )                                          \
            {                                                                   \
                (min_val) = val;                                                \
                (min_loc) = (loc);                                              \
            }                                                                   \
            else if( val > (max_val) && m )                                     \
            {                                                                   \
                (max_val) = val;                                                \
                (max_loc) = (loc);                                              \
            }                                                                   \
        }                                                                       \
        x = 0;                                                                  \
    }                                                                           \
                                                                                \
    _exit_( _toggle_, _fin_cast_macro_ );                                       \
}



#define ICX_DEF_MINMAXLOC_MASK_ALL( flavor, srctype, temptype, extrtype )           \
                                                                                    \
    ICX_DEF_MINMAXLOC_MASK_FUNC_2D( CX_NOP, CX_NOP, CX_CAST_64F,                    \
                               CX_MINMAXLOC_MASK_ENTRY, CX_MINMAXLOC_EXIT,          \
                               flavor, srctype, temptype, extrtype )                \
    ICX_DEF_MINMAXLOC_MASK_FUNC_2D_COI( CX_NOP, CX_NOP, CX_CAST_64F,                \
                                   CX_MINMAXLOC_MASK_ENTRY, CX_MINMAXLOC_EXIT,      \
                                   flavor, srctype, temptype, extrtype )


#define ICX_DEF_MINMAXLOC_MASK_ALL_FLT( flavor, srctype, temptype,                  \
                                        _cast_macro_, _toggle_,                     \
                                        _fin_cast_macro_, extrtype )                \
                                                                                    \
    ICX_DEF_MINMAXLOC_MASK_FUNC_2D( _cast_macro_, _toggle_, _fin_cast_macro_,       \
                                    CX_MINMAXLOC_MASK_ENTRY, CX_MINMAXLOC_EXIT,     \
                                    flavor, srctype, temptype, extrtype )           \
    ICX_DEF_MINMAXLOC_MASK_FUNC_2D_COI( _cast_macro_, _toggle_, _fin_cast_macro_,   \
                                       CX_MINMAXLOC_MASK_ENTRY, CX_MINMAXLOC_EXIT,  \
                                       flavor, srctype, temptype, extrtype )


ICX_DEF_MINMAXLOC_MASK_ALL( 8u, uchar, int, float )
ICX_DEF_MINMAXLOC_MASK_ALL( 16s, short, int, float )
ICX_DEF_MINMAXLOC_MASK_ALL( 32s, int, int, double )
ICX_DEF_MINMAXLOC_MASK_ALL_FLT( 32f, float, int, _as_int_,
                                _toggle_float_, _as_float_, float )
ICX_DEF_MINMAXLOC_MASK_ALL_FLT( 64f, double, int64, _as_int64_,
                                _toggle_double_, _as_double_, double )

#define icxMinMaxIndx_8s_C1R    0
#define icxMinMaxIndx_8s_CnCR   0
#define icxMinMaxIndx_8s_C1MR   0
#define icxMinMaxIndx_8s_CnCMR  0

CX_DEF_INIT_FUNC_TAB_2D( MinMaxIndx, C1R )
CX_DEF_INIT_FUNC_TAB_2D( MinMaxIndx, CnCR )
CX_DEF_INIT_FUNC_TAB_2D( MinMaxIndx, C1MR )
CX_DEF_INIT_FUNC_TAB_2D( MinMaxIndx, CnCMR )


CX_IMPL  void
cxMinMaxLoc( const void* img, double* minVal, double* maxVal,
             CxPoint* minLoc, CxPoint* maxLoc, const void* mask )
{
    static CxFuncTable minmax_tab, minmaxcoi_tab;
    static CxFuncTable minmaxmask_tab, minmaxmaskcoi_tab;
    static int inittab = 0;

    CX_FUNCNAME("cxMinMaxLoc");

    __BEGIN__;

    int type, depth, cn, coi = 0;
    int mat_step, mask_step = 0;
    CxSize size;
    CxMat stub, maskstub, *mat = (CxMat*)img, *matmask = (CxMat*)mask;

    if( !inittab )
    {
        icxInitMinMaxIndxC1RTable( &minmax_tab );
        icxInitMinMaxIndxCnCRTable( &minmaxcoi_tab );
        icxInitMinMaxIndxC1MRTable( &minmaxmask_tab );
        icxInitMinMaxIndxCnCMRTable( &minmaxmaskcoi_tab );
        inittab = 1;
    }
    
    CX_CALL( mat = cxGetMat( mat, &stub, &coi ));

    type = CX_MAT_TYPE( mat->type );
    depth = CX_MAT_DEPTH( type );
    cn = CX_MAT_CN( type );
    size = cxGetMatSize( mat );

    if( cn > 1 && coi == 0 )
        CX_ERROR( CX_StsBadArg, "" );
    
    mat_step = mat->step;

    if( !mask )
    {
        if( size.height == 1 )
            mat_step = CX_STUB_STEP;

        if( CX_MAT_CN(type) == 1 || coi == 0 )
        {
            CxFunc2D_1A4P func = (CxFunc2D_1A4P)(minmax_tab.fn_2d[depth]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size,
                             minVal, maxVal, minLoc, maxLoc ));
        }
        else
        {
            CxFunc2DnC_1A4P func = (CxFunc2DnC_1A4P)(minmaxcoi_tab.fn_2d[depth]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size, cn, coi,
                             minVal, maxVal, minLoc, maxLoc ));
        }
    }
    else
    {
        CX_CALL( matmask = cxGetMat( matmask, &maskstub ));

        if( !CX_IS_MASK_ARR( matmask ))
            CX_ERROR( CX_StsBadMask, "" );

        if( !CX_ARE_SIZES_EQ( mat, matmask ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );

        mask_step = matmask->step;

        if( size.height == 1 )
            mat_step = mask_step = CX_STUB_STEP;

        if( CX_MAT_CN(type) == 1 || coi == 0 )
        {
            CxFunc2D_2A4P func = (CxFunc2D_2A4P)(minmaxmask_tab.fn_2d[depth]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, matmask->data.ptr,
                             mask_step, size,
                             minVal, maxVal, minLoc, maxLoc ));
        }
        else
        {
            CxFunc2DnC_2A4P func = (CxFunc2DnC_2A4P)(minmaxmaskcoi_tab.fn_2d[depth]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step,
                             matmask->data.ptr, mask_step, size, cn, coi,
                             minVal, maxVal, minLoc, maxLoc ));
        }
    }

    if( depth < CX_32S || depth == CX_32F )
    {
        if( minVal )
            *minVal = *(float*)minVal;

        if( maxVal )
            *maxVal = *(float*)maxVal;
    }

    __END__;
}


/*  End of file  */
