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
#include <float.h>

/****************************************************************************************\
*                              Mean value over the region                                *
\****************************************************************************************/

#define ICX_IMPL_MEAN_1D_CASE_C1( _mask_op_, acctype, src, mask, len, sum, pix ) \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
                                                                    \
    for( i = 0; i <= (len) - 4; i += 4 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        acctype s;                                                  \
                                                                    \
        s = _mask_op_(m,(src)[i]);                                  \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        s += _mask_op_(m,(src)[i + 1]);                             \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 2] == 0) - 1;                               \
        s += _mask_op_(m,(src)[i + 2]);                             \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 3] == 0) - 1;                               \
        (sum)[0] += s + _mask_op_(m,(src)[i + 3]);                  \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        s1 += _mask_op_(m,(src)[i]);                                \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    (sum)[0] += s1;                                                 \
}


#define ICX_IMPL_MEAN_1D_CASE_COI( _mask_op_, acctype, src, mask, len, sum, pix, cn ) \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
                                                                    \
    for( i = 0; i <= (len) - 4; i += 4 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        acctype s;                                                  \
                                                                    \
        s = _mask_op_( m, (src)[i*(cn)]);                           \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        s += _mask_op_( m, (src)[(i + 1)*(cn)]);                    \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 2] == 0) - 1;                               \
        s += _mask_op_( m, (src)[(i + 2)*(cn)]);                    \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 3] == 0) - 1;                               \
        (sum)[0] += s + _mask_op_( m, (src)[(i + 3)*(cn)]);         \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        s1 += _mask_op_( m, (src)[i*(cn)]);                         \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    (sum)[0] += s1;                                                 \
}


#define ICX_IMPL_MEAN_1D_CASE_C2( _mask_op_, acctype, src, mask, len, sum, pix ) \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i <= (len) - 2; i += 2 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        (sum)[0] += _mask_op_( m, (src)[i*2]);                      \
        (sum)[1] += _mask_op_( m, (src)[i*2 + 1]);                  \
        (pix) -= m;                                                 \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        (sum)[0] += _mask_op_( m, (src)[i*2 + 2]);                  \
        (sum)[1] += _mask_op_( m, (src)[i*2 + 3]);                  \
        (pix) -= m;                                                 \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        (sum)[0] += _mask_op_( m, (src)[i*2]);                      \
        (sum)[1] += _mask_op_( m, (src)[i*2 + 1]);                  \
        (pix) -= m;                                                 \
    }                                                               \
}


#define ICX_IMPL_MEAN_1D_CASE_C3( _mask_op_, acctype, src, mask, len, sum, pix ) \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i++ )                                    \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        (sum)[0] += _mask_op_( m, (src)[i*3]);                      \
        (sum)[1] += _mask_op_( m, (src)[i*3 + 1]);                  \
        (sum)[2] += _mask_op_( m, (src)[i*3 + 2]);                  \
        (pix) -= m;                                                 \
    }                                                               \
}


#define ICX_IMPL_MEAN_1D_CASE_C4( _mask_op_, acctype, src, mask, len, sum, pix ) \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i++ )                                    \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
                                                                    \
        (sum)[0] += _mask_op_( m, (src)[i*4]);                      \
        (sum)[1] += _mask_op_( m, (src)[i*4 + 1]);                  \
        (sum)[2] += _mask_op_( m, (src)[i*4 + 2]);                  \
        (sum)[3] += _mask_op_( m, (src)[i*4 + 3]);                  \
        (pix) -= m;                                                 \
    }                                                               \
}



#define ICX_MEAN_ENTRY( sumtype ) \
    sumtype sum[4] = {0,0,0,0};  \
    int pix = 0


#define ICX_MEAN_ENTRY_FLT( sumtype ) \
    float  maskTab[] = { 1.f, 0.f }; \
    sumtype sum[4] = {0,0,0,0};      \
    int pix = 0


#define ICX_MEAN_EXIT(cn)                  \
{                                         \
    double scale = pix ? 1./pix : 0;      \
    for( int k = 0; k < cn; k++ )         \
        mean[k] = sum[k]*scale;           \
}                                         \
return CX_OK;


#define ICX_IMPL_MEAN_FUNC_2D( _mask_op_, _entry_, _exit_,          \
                              flavor, cn, srctype, sumtype, acctype)\
IPCXAPI_IMPL( CxStatus, icxMean_##flavor##_C##cn##MR,               \
                          ( const srctype* src, int step,           \
                            const uchar* mask, int maskStep,        \
                            CxSize size, double* mean ))            \
{                                                                   \
    _entry_( sumtype );                                             \
                                                                    \
    for( ; size.height--;                                           \
         (char*&)src += step, (char*&)mask += maskStep )            \
    {                                                               \
        ICX_IMPL_MEAN_1D_CASE_C##cn( _mask_op_, acctype, src, mask, \
                                    size.width, sum, pix );         \
    }                                                               \
                                                                    \
    _exit_(cn);                                                     \
}


#define ICX_IMPL_MEAN_FUNC_2D_COI( _mask_op_, _entry_, _exit_,      \
                                  flavor, srctype, sumtype, acctype)\
static CxStatus CX_STDCALL                                          \
icxMean_##flavor##_CnCMR( const srctype* src, int step,             \
                        const uchar* mask, int maskStep,            \
                        CxSize size, int cn, int coi, double* mean )\
{                                                                   \
    _entry_( sumtype );                                             \
    (src) += coi - 1;                                               \
                                                                    \
    for( ; size.height--;                                           \
         (char*&)src += step, (char*&)mask += maskStep )            \
    {                                                               \
        ICX_IMPL_MEAN_1D_CASE_COI( _mask_op_, acctype, src, mask,   \
                                  size.width, sum, pix, cn );       \
    }                                                               \
                                                                    \
    mean[0] = sum[0]*(pix ? 1./pix : 0);                            \
                                                                    \
    return CX_OK;                                                   \
}


#define ICX_IMPL_MEAN_ALL( flavor, srctype, sumtype, acctype )        \
    ICX_IMPL_MEAN_FUNC_2D( CX_AND, ICX_MEAN_ENTRY, ICX_MEAN_EXIT,     \
                           flavor, 1, srctype, sumtype, acctype )     \
    ICX_IMPL_MEAN_FUNC_2D( CX_AND, ICX_MEAN_ENTRY, ICX_MEAN_EXIT,     \
                           flavor, 2, srctype, sumtype, acctype )     \
    ICX_IMPL_MEAN_FUNC_2D( CX_AND, ICX_MEAN_ENTRY, ICX_MEAN_EXIT,     \
                           flavor, 3, srctype, sumtype, acctype )     \
    ICX_IMPL_MEAN_FUNC_2D( CX_AND, ICX_MEAN_ENTRY, ICX_MEAN_EXIT,     \
                           flavor, 4, srctype, sumtype, acctype )     \
    ICX_IMPL_MEAN_FUNC_2D_COI( CX_AND, ICX_MEAN_ENTRY, ICX_MEAN_EXIT, \
                               flavor, srctype, sumtype, acctype )


#define ICX_IMPL_MEAN_ALL_FLT( flavor, srctype, sumtype, acctype )              \
    ICX_IMPL_MEAN_FUNC_2D( CX_MULMASK1, ICX_MEAN_ENTRY_FLT, ICX_MEAN_EXIT,      \
                           flavor, 1, srctype, sumtype, acctype )               \
    ICX_IMPL_MEAN_FUNC_2D( CX_MULMASK1, ICX_MEAN_ENTRY_FLT, ICX_MEAN_EXIT,      \
                           flavor, 2, srctype, sumtype, acctype )               \
    ICX_IMPL_MEAN_FUNC_2D( CX_MULMASK1, ICX_MEAN_ENTRY_FLT, ICX_MEAN_EXIT,      \
                           flavor, 3, srctype, sumtype, acctype )               \
    ICX_IMPL_MEAN_FUNC_2D( CX_MULMASK1, ICX_MEAN_ENTRY_FLT, ICX_MEAN_EXIT,      \
                           flavor, 4, srctype, sumtype, acctype )               \
    ICX_IMPL_MEAN_FUNC_2D_COI( CX_MULMASK1, ICX_MEAN_ENTRY_FLT, ICX_MEAN_EXIT,  \
                               flavor, srctype, sumtype, acctype )

ICX_IMPL_MEAN_ALL( 8u, uchar, int64, int )
ICX_IMPL_MEAN_ALL( 16s, short, int64, int )
ICX_IMPL_MEAN_ALL( 32s, int, int64, int64 )
ICX_IMPL_MEAN_ALL_FLT( 32f, float, double, double )
ICX_IMPL_MEAN_ALL_FLT( 64f, double, double, double )

#define icxMean_8s_C1MR 0
#define icxMean_8s_C2MR 0
#define icxMean_8s_C3MR 0
#define icxMean_8s_C4MR 0
#define icxMean_8s_CnCMR 0

CX_DEF_INIT_BIG_FUNC_TAB_2D( Mean, MR )
CX_DEF_INIT_FUNC_TAB_2D( Mean, CnCMR )

CX_IMPL  CxScalar
cxAvg( const void* img, const void* mask )
{
    CxScalar mean = {{0,0,0,0}};

    static CxBigFuncTable mean_tab;
    static CxFuncTable meancoi_tab;
    static int inittab = 0;

    CX_FUNCNAME("cxAvg");

    __BEGIN__;

    CxSize size;
    double scale;

    if( !mask )
    {
        CX_CALL( mean = cxSum(img));
        cxGetRawData( img, 0, 0, &size );

        size.width *= size.height;
        scale = size.width ? 1./size.width : 0;

        mean.val[0] *= scale;
        mean.val[1] *= scale;
        mean.val[2] *= scale;
        mean.val[3] *= scale;
    }
    else
    {
        int type, coi = 0;
        int mat_step, mask_step;

        CxMat stub, maskstub, *mat = (CxMat*)img, *matmask = (CxMat*)mask;

        if( !inittab )
        {
            icxInitMeanMRTable( &mean_tab );
            icxInitMeanCnCMRTable( &meancoi_tab );
            inittab = 1;
        }

        CX_CALL( mat = cxGetMat( mat, &stub, &coi ));
        CX_CALL( matmask = cxGetMat( matmask, &maskstub ));

        if( !CX_IS_MASK_ARR( matmask) )
            CX_ERROR( CX_StsBadMask, "" );

        if( !CX_ARE_SIZES_EQ( mat, matmask ) )
            CX_ERROR( CX_StsUnmatchedSizes, "" );

        type = CX_MAT_TYPE( mat->type );
        size = cxGetMatSize( mat );

        mat_step = mat->step;
        mask_step = matmask->step;

        if( CX_IS_MAT_CONT( mat->type & matmask->type ))
        {
            size.width *= size.height;
            size.height = 1;
            mat_step = mask_step = CX_STUB_STEP;
        }

        if( CX_MAT_CN(type) == 1 || coi == 0 )
        {
            CxFunc2D_2A1P func = (CxFunc2D_2A1P)(mean_tab.fn_2d[type]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, matmask->data.ptr,
                             mask_step, size, mean.val ));
        }
        else
        {
            CxFunc2DnC_2A1P func = (CxFunc2DnC_2A1P)(
                meancoi_tab.fn_2d[CX_MAT_DEPTH(type)]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat->step,
                             matmask->data.ptr, matmask->step,
                             size, CX_MAT_CN(type), coi, mean.val ));
        }
    }

    __END__;

    return  mean;
}

/*  End of file  */
