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
*                             Mean and StdDev calculation                                *
\****************************************************************************************/


#define CX_IMPL_MEAN_SDV_1D_CASE_COI( temptype, acctype, accsqtype, \
                                      src, len, sum, sqsum, cn )    \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
    accsqtype sq1 = 0;                                              \
                                                                    \
    for( i = 0; i <= (len) - 4*(cn); i += 4*(cn) )                  \
    {                                                               \
        temptype t0 = (src)[i];                                     \
        temptype t1 = (src)[i + (cn)];                              \
        acctype  s;                                                 \
        accsqtype sq;                                               \
                                                                    \
        s  = (acctype)t0 + (acctype)t1;                             \
        sq = ((accsqtype)t0)*t0 + ((accsqtype)t1)*t1;               \
                                                                    \
        t0 = (src)[i + 2*(cn)];                                     \
        t1 = (src)[i + 3*(cn)];                                     \
                                                                    \
        (sum)[0] += s + (acctype)t0 + (acctype)t1;                  \
        (sqsum)[0] += sq + ((accsqtype)t0)*t0 + ((accsqtype)t1)*t1; \
    }                                                               \
                                                                    \
    for( ; i < (len); i += (cn) )                                   \
    {                                                               \
        temptype t = (src)[i];                                      \
                                                                    \
        s1 += (acctype)t;                                           \
        sq1 += ((accsqtype)t)*t;                                    \
    }                                                               \
                                                                    \
    (sum)[0] += s1;                                                 \
    (sqsum)[0] += sq1;                                              \
}


#define CX_IMPL_MEAN_SDV_1D_CASE_C1( temptype, acctype, accsqtype,  \
                                     src, len, sum, sqsum )         \
CX_IMPL_MEAN_SDV_1D_CASE_COI( temptype, acctype, accsqtype,         \
                              src, len, sum, sqsum, 1 )


#define CX_IMPL_MEAN_SDV_1D_CASE_C2( temptype, acctype, accsqtype,  \
                                     src, len, sum, sqsum )         \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i += 2 )                                 \
    {                                                               \
        temptype t0 = (src)[i];                                     \
        temptype t1 = (src)[i + 1];                                 \
                                                                    \
        (sum)[0] += t0;                                             \
        (sum)[1] += t1;                                             \
        (sqsum)[0] += ((accsqtype)t0)*t0;                           \
        (sqsum)[1] += ((accsqtype)t1)*t1;                           \
    }                                                               \
}


#define CX_IMPL_MEAN_SDV_1D_CASE_C3( temptype, acctype, accsqtype,  \
                                     src, len, sum, sqsum )         \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i += 3 )                                 \
    {                                                               \
        temptype t0 = (src)[i];                                     \
        temptype t1 = (src)[i + 1];                                 \
        temptype t2 = (src)[i + 2];                                 \
                                                                    \
        (sum)[0] += t0;                                             \
        (sum)[1] += t1;                                             \
        (sum)[2] += t2;                                             \
        (sqsum)[0] += ((accsqtype)t0)*t0;                           \
        (sqsum)[1] += ((accsqtype)t1)*t1;                           \
        (sqsum)[2] += ((accsqtype)t2)*t2;                           \
    }                                                               \
}


#define CX_IMPL_MEAN_SDV_1D_CASE_C4( temptype, acctype, accsqtype,  \
                                     src, len, sum, sqsum )         \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i += 4 )                                 \
    {                                                               \
        temptype t0 = (src)[i];                                     \
        temptype t1 = (src)[i + 1];                                 \
                                                                    \
        (sum)[0] += t0;                                             \
        (sum)[1] += t1;                                             \
        (sqsum)[0] += ((accsqtype)t0)*t0;                           \
        (sqsum)[1] += ((accsqtype)t1)*t1;                           \
                                                                    \
        t0 = (src)[i + 2];                                          \
        t1 = (src)[i + 3];                                          \
                                                                    \
        (sum)[2] += t0;                                             \
        (sum)[3] += t1;                                             \
        (sqsum)[2] += ((accsqtype)t0)*t0;                           \
        (sqsum)[3] += ((accsqtype)t1)*t1;                           \
    }                                                               \
}


#define CX_IMPL_MEAN_SDV_MASK_1D_CASE_C1( _mask_op_, temptype, acctype, accsqtype, \
                                          src, mask, len, sum, sqsum, pix )        \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
    accsqtype sq1 = 0;                                              \
                                                                    \
    for( i = 0; i <= (len) - 4; i += 4 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
        acctype s;                                                  \
        accsqtype sq;                                               \
                                                                    \
        t = _mask_op_(m, (src)[i]);                                 \
        (pix) -= m;                                                 \
        s = t;                                                      \
        sq = ((accsqtype)t)*t;                                      \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        t = _mask_op_(m, (src)[i + 1]);                             \
        (pix) -= m;                                                 \
        s += t;                                                     \
        sq += ((accsqtype)t)*t;                                     \
                                                                    \
        m = ((mask)[i + 2] == 0) - 1;                               \
        t = _mask_op_(m, (src)[i + 2]);                             \
        (pix) -= m;                                                 \
        s += t;                                                     \
        sq += ((accsqtype)t)*t;                                     \
                                                                    \
        m = ((mask)[i + 3] == 0) - 1;                               \
        t = _mask_op_(m, (src)[i + 3]);                             \
        (pix) -= m;                                                 \
        sum[0] += s + t;                                            \
        sqsum[0] += sq + ((accsqtype)t)*t;                          \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t = _mask_op_(m, (src)[i]);                        \
        (pix) -= m;                                                 \
        s1 += t;                                                    \
        sq1 += ((accsqtype)t)*t;                                    \
    }                                                               \
                                                                    \
    sum[0] += s1;                                                   \
    sqsum[0] += sq1;                                                \
}


#define CX_IMPL_MEAN_SDV_MASK_1D_CASE_COI( _mask_op_, temptype, acctype, accsqtype, \
                                           src, mask, len, sum, sqsum, pix, cn )    \
{                                                                   \
    int i;                                                          \
    acctype s1 = 0;                                                 \
    accsqtype sq1 = 0;                                              \
                                                                    \
    for( i = 0; i <= (len) - 4; i += 4 )                            \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
        acctype s;                                                  \
        accsqtype sq;                                               \
                                                                    \
        t = _mask_op_(m, (src)[i*(cn)]);                            \
        (pix) -= m;                                                 \
        s = t;                                                      \
        sq = ((accsqtype)t)*t;                                      \
                                                                    \
        m = ((mask)[i + 1] == 0) - 1;                               \
        t = _mask_op_(m, (src)[(i + 1)*(cn)]);                      \
        (pix) -= m;                                                 \
        s += t;                                                     \
        sq += ((accsqtype)t)*t;                                     \
                                                                    \
        m = ((mask)[i + 2] == 0) - 1;                               \
        t = _mask_op_(m, (src)[(i + 2)*(cn)]);                      \
        (pix) -= m;                                                 \
        s += t;                                                     \
        sq += ((accsqtype)t)*t;                                     \
                                                                    \
        m = ((mask)[i + 3] == 0) - 1;                               \
        t = _mask_op_(m, (src)[(i + 3)*(cn)]);                      \
        (pix) -= m;                                                 \
        sum[0] += s + t;                                            \
        sqsum[0] += sq + ((accsqtype)t)*t;                          \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t = _mask_op_(m, (src)[i*(cn)]);                   \
        (pix) -= m;                                                 \
        s1 += t;                                                    \
        sq1 += ((accsqtype)t)*t;                                    \
    }                                                               \
                                                                    \
    sum[0] += s1;                                                   \
    sqsum[0] += sq1;                                                \
}


#define CX_IMPL_MEAN_SDV_MASK_1D_CASE_C2( _mask_op_, temptype, acctype, accsqtype, \
                                          src, mask, len, sum, sqsum, pix )        \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i += 2 )                                 \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
                                                                    \
        (pix) -= m;                                                 \
        t = _mask_op_(m, (src)[i]);                                 \
        (sum)[0] += t;                                              \
        (sqsum)[0] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i + 1]);                             \
        (sum)[1] += t;                                              \
        (sqsum)[1] += ((accsqtype)t)*t;                             \
    }                                                               \
}


#define CX_IMPL_MEAN_SDV_MASK_1D_CASE_C3( _mask_op_, temptype, acctype, accsqtype, \
                                          src, mask, len, sum, sqsum, pix )        \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i += 3 )                                 \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
                                                                    \
        (pix) -= m;                                                 \
        t = _mask_op_(m, (src)[i]);                                 \
        (sum)[0] += t;                                              \
        (sqsum)[0] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i + 1]);                             \
        (sum)[1] += t;                                              \
        (sqsum)[1] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i + 2]);                             \
        (sum)[2] += t;                                              \
        (sqsum)[2] += ((accsqtype)t)*t;                             \
    }                                                               \
}


#define CX_IMPL_MEAN_SDV_MASK_1D_CASE_C4( _mask_op_, temptype, acctype, accsqtype, \
                                          src, mask, len, sum, sqsum, pix )        \
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i < (len); i += 4 )                                 \
    {                                                               \
        int m = ((mask)[i] == 0) - 1;                               \
        temptype t;                                                 \
                                                                    \
        (pix) -= m;                                                 \
        t = _mask_op_(m, (src)[i]);                                 \
        (sum)[0] += t;                                              \
        (sqsum)[0] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i + 1]);                             \
        (sum)[1] += t;                                              \
        (sqsum)[1] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i + 2]);                             \
        (sum)[2] += t;                                              \
        (sqsum)[2] += ((accsqtype)t)*t;                             \
                                                                    \
        t = _mask_op_(m, (src)[i + 3]);                             \
        (sum)[3] += t;                                              \
        (sqsum)[3] += ((accsqtype)t)*t;                             \
    }                                                               \
}



#define CX_MEAN_SDV_ENTRY( sumtype, sumsqtype ) \
    sumtype sum[4] = {0,0,0,0};                 \
    sumsqtype sqsum[4] = {0,0,0,0}


#define CX_MEAN_SDV_MASK_ENTRY( sumtype, sumsqtype )\
    sumtype sum[4] = {0,0,0,0};                     \
    sumtype sqsum[4] = {0,0,0,0};                   \
    int pix = 0


#define CX_MEAN_SDV_MASK_ENTRY_FLT( sumtype, sumsqtype )\
    float  maskTab[] = { 1.f, 0.f };                    \
    sumtype sum[4] = {0,0,0,0};                         \
    sumtype sqsum[4] = {0,0,0,0};                       \
    int pix = 0


#define CX_MEAN_SDV_EXIT( pix, cn )             \
{                                               \
    double scale = pix ? 1./pix : 0;            \
    for( int k = 0; k < cn; k++ )               \
    {                                           \
        double mn = sum[k]*scale;               \
        mean[k] = mn;                           \
        sdv[k] = sqrt( sqsum[k]*scale - mn*mn );\
    }                                           \
}


#define CX_IMPL_MEAN_SDV_FUNC_2D( _entry_, _exit_,                  \
                            flavor, cn, srctype, sumtype, sumsqtype,\
                            temptype, acctype, accsqtype )          \
IPCXAPI_IMPL( CxStatus, icxMean_StdDev_##flavor##_C##cn##R,         \
                        ( const srctype* src, int step,             \
                          CxSize size, double* mean, double* sdv )) \
{                                                                   \
    _entry_( sumtype, sumsqtype );                                  \
    int len = size.width*(cn), height = size.height;                \
                                                                    \
    for( ; size.height--; (char*&)src += step )                     \
    {                                                               \
        CX_IMPL_MEAN_SDV_1D_CASE_C##cn( temptype, acctype, accsqtype,\
                                        src, len, sum, sqsum );     \
    }                                                               \
                                                                    \
    len = size.width*height;                                        \
    _exit_( len, cn );                                              \
                                                                    \
    return CX_OK;                                                   \
}


#define CX_IMPL_MEAN_SDV_FUNC_2D_COI( _entry_, _exit_,              \
                              flavor, srctype, sumtype, sumsqtype,  \
                              temptype, acctype, accsqtype )        \
static CxStatus CX_STDCALL icxMean_StdDev_##flavor##_CnCR           \
                        ( const srctype* src, int step,             \
                          CxSize size, int cn, int coi,             \
                          double* mean, double* sdv )               \
{                                                                   \
    _entry_( sumtype, sumsqtype );                                  \
    int len = size.width*(cn), height = size.height;                \
    (src) += coi - 1;                                               \
                                                                    \
    for( ; size.height--; (char*&)src += step )                     \
    {                                                               \
        CX_IMPL_MEAN_SDV_1D_CASE_COI( temptype, acctype, accsqtype, \
                                      src, len, sum, sqsum, cn );   \
    }                                                               \
                                                                    \
    len = size.width*height;                                        \
    _exit_( len, 1 );                                               \
                                                                    \
    return CX_OK;                                                   \
}


#define CX_IMPL_MEAN_SDV_MASK_FUNC_2D( _mask_op_, _entry_, _exit_,  \
                            flavor, cn, srctype, sumtype, sumsqtype,\
                            temptype, acctype, accsqtype )          \
IPCXAPI_IMPL( CxStatus, icxMean_StdDev_##flavor##_C##cn##MR,        \
                        ( const srctype* src, int step,             \
                          const uchar* mask, int maskStep,          \
                          CxSize size, double* mean, double* sdv )) \
{                                                                   \
    _entry_( sumtype, sumsqtype );                                  \
                                                                    \
    for( ; size.height--;                                           \
         (char*&)src += step, (char*&)mask += maskStep )            \
    {                                                               \
        CX_IMPL_MEAN_SDV_MASK_1D_CASE_C##cn( _mask_op_, temptype,   \
            acctype, accsqtype, src, mask, size.width, sum, sqsum, pix);\
    }                                                               \
                                                                    \
    _exit_( pix, cn );                                              \
                                                                    \
    return CX_OK;                                                   \
}


#define CX_IMPL_MEAN_SDV_MASK_FUNC_2D_COI( _mask_op_, _entry_, _exit_,  \
                              flavor, srctype, sumtype, sumsqtype,  \
                              temptype, acctype, accsqtype )        \
static CxStatus CX_STDCALL icxMean_StdDev_##flavor##_CnCMR          \
                        ( const srctype* src, int step,             \
                          const uchar* mask, int maskStep,          \
                          CxSize size, int cn, int coi,             \
                          double* mean, double* sdv )               \
{                                                                   \
    _entry_( sumtype, sumsqtype );                                  \
    (src) += coi - 1;                                               \
                                                                    \
    for( ; size.height--;                                           \
         (char*&)src += step, (char*&)mask += maskStep )            \
    {                                                               \
        CX_IMPL_MEAN_SDV_MASK_1D_CASE_COI( _mask_op_, temptype,     \
            acctype, accsqtype, src, mask, size.width, sum, sqsum, pix, cn);\
    }                                                               \
                                                                    \
    _exit_( pix, 1 );                                               \
                                                                    \
    return CX_OK;                                                   \
}


#define CX_IMPL_MEAN_SDV_ALL( flavor, srctype, sumtype, sumsqtype, temptype,    \
                              acctype, accsqtype )                              \
                                                                                \
    CX_IMPL_MEAN_SDV_FUNC_2D( CX_MEAN_SDV_ENTRY, CX_MEAN_SDV_EXIT,              \
                              flavor, 1, srctype, sumtype,                      \
                              sumsqtype, temptype, acctype, accsqtype )         \
                                                                                \
    CX_IMPL_MEAN_SDV_FUNC_2D( CX_MEAN_SDV_ENTRY, CX_MEAN_SDV_EXIT,              \
                              flavor, 2, srctype, sumtype,                      \
                              sumsqtype, temptype, acctype, accsqtype )         \
                                                                                \
    CX_IMPL_MEAN_SDV_FUNC_2D( CX_MEAN_SDV_ENTRY, CX_MEAN_SDV_EXIT,              \
                              flavor, 3, srctype, sumtype,                      \
                              sumsqtype, temptype, acctype, accsqtype )         \
                                                                                \
    CX_IMPL_MEAN_SDV_FUNC_2D( CX_MEAN_SDV_ENTRY, CX_MEAN_SDV_EXIT,              \
                              flavor, 4, srctype, sumtype,                      \
                              sumsqtype, temptype, acctype, accsqtype )         \
                                                                                \
    CX_IMPL_MEAN_SDV_FUNC_2D_COI( CX_MEAN_SDV_ENTRY, CX_MEAN_SDV_EXIT,          \
                                  flavor, srctype, sumtype,                     \
                                  sumsqtype, temptype, acctype, accsqtype )


#define CX_IMPL_MEAN_SDV_MASK_ALL( flavor, srctype, sumtype, sumsqtype,                 \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D( CX_AND, CX_MEAN_SDV_MASK_ENTRY, CX_MEAN_SDV_EXIT,    \
                                   flavor, 1, srctype, sumtype, sumsqtype,              \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D( CX_AND, CX_MEAN_SDV_MASK_ENTRY, CX_MEAN_SDV_EXIT,    \
                                   flavor, 2, srctype, sumtype, sumsqtype,              \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D( CX_AND, CX_MEAN_SDV_MASK_ENTRY, CX_MEAN_SDV_EXIT,    \
                                   flavor, 3, srctype, sumtype, sumsqtype,              \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D( CX_AND, CX_MEAN_SDV_MASK_ENTRY, CX_MEAN_SDV_EXIT,    \
                                   flavor, 4, srctype, sumtype, sumsqtype,              \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D_COI( CX_AND, CX_MEAN_SDV_MASK_ENTRY, CX_MEAN_SDV_EXIT,\
                                       flavor, srctype, sumtype, sumsqtype,             \
                                       temptype, acctype, accsqtype )


#define CX_IMPL_MEAN_SDV_MASK_ALL_FLT( flavor, srctype, sumtype, sumsqtype,             \
                                       temptype, acctype, accsqtype )                   \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D( CX_MULMASK1, CX_MEAN_SDV_MASK_ENTRY_FLT,             \
                                   CX_MEAN_SDV_EXIT, flavor, 1,                         \
                                   srctype, sumtype, sumsqtype,                         \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D( CX_MULMASK1, CX_MEAN_SDV_MASK_ENTRY_FLT,             \
                                   CX_MEAN_SDV_EXIT, flavor, 2,                         \
                                   srctype, sumtype, sumsqtype,                         \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D( CX_MULMASK1, CX_MEAN_SDV_MASK_ENTRY_FLT,             \
                                   CX_MEAN_SDV_EXIT, flavor, 3,                         \
                                   srctype, sumtype, sumsqtype,                         \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D( CX_MULMASK1, CX_MEAN_SDV_MASK_ENTRY_FLT,             \
                                   CX_MEAN_SDV_EXIT, flavor, 4,                         \
                                   srctype, sumtype, sumsqtype,                         \
                                   temptype, acctype, accsqtype )                       \
                                                                                        \
    CX_IMPL_MEAN_SDV_MASK_FUNC_2D_COI( CX_MULMASK1, CX_MEAN_SDV_MASK_ENTRY_FLT,         \
                                       CX_MEAN_SDV_EXIT, flavor,                        \
                                       srctype, sumtype, sumsqtype,                     \
                                       temptype, acctype, accsqtype )


CX_IMPL_MEAN_SDV_ALL( 8u, uchar, int64, int64, int, int, int )
CX_IMPL_MEAN_SDV_ALL( 16s, short, int64, int64, int, int, int64 )
CX_IMPL_MEAN_SDV_ALL( 32s, int, double, double, double, double, double )
CX_IMPL_MEAN_SDV_ALL( 32f, float, double, double, double, double, double )
CX_IMPL_MEAN_SDV_ALL( 64f, double, double, double, double, double, double )

CX_IMPL_MEAN_SDV_MASK_ALL( 8u, uchar, int64, int64, int, int, int )
CX_IMPL_MEAN_SDV_MASK_ALL( 16s, short, int64, int64, int, int, int64 )
CX_IMPL_MEAN_SDV_MASK_ALL_FLT( 32s, int, double, double, double, double, double )
CX_IMPL_MEAN_SDV_MASK_ALL_FLT( 32f, float, double, double, double, double, double )
CX_IMPL_MEAN_SDV_MASK_ALL_FLT( 64f, double, double, double, double, double, double )

#define icxMean_StdDev_8s_C1R  0
#define icxMean_StdDev_8s_C2R  0
#define icxMean_StdDev_8s_C3R  0
#define icxMean_StdDev_8s_C4R  0
#define icxMean_StdDev_8s_CnCR 0

#define icxMean_StdDev_8s_C1MR  0
#define icxMean_StdDev_8s_C2MR  0
#define icxMean_StdDev_8s_C3MR  0
#define icxMean_StdDev_8s_C4MR  0
#define icxMean_StdDev_8s_CnCMR 0

CX_DEF_INIT_BIG_FUNC_TAB_2D( Mean_StdDev, R )
CX_DEF_INIT_FUNC_TAB_2D( Mean_StdDev, CnCR )
CX_DEF_INIT_BIG_FUNC_TAB_2D( Mean_StdDev, MR )
CX_DEF_INIT_FUNC_TAB_2D( Mean_StdDev, CnCMR )

CX_IMPL  void
cxAvgSdv( const void* img, CxScalar* _mean, CxScalar* _sdv, const void* mask )
{
    CxScalar mean = {{0,0,0,0}};
    CxScalar sdv = {{0,0,0,0}};

    static CxBigFuncTable meansdv_tab;
    static CxFuncTable meansdvcoi_tab;
    static CxBigFuncTable meansdvmask_tab;
    static CxFuncTable meansdvmaskcoi_tab;
    static int inittab = 0;

    CX_FUNCNAME("cxMean_StdDev");

    __BEGIN__;

    int type, coi = 0;
    int mat_step, mask_step = 0;
    CxSize size;
    CxMat stub, maskstub, *mat = (CxMat*)img, *matmask = (CxMat*)mask;

    if( !inittab )
    {
        icxInitMean_StdDevRTable( &meansdv_tab );
        icxInitMean_StdDevCnCRTable( &meansdvcoi_tab );
        icxInitMean_StdDevMRTable( &meansdvmask_tab );
        icxInitMean_StdDevCnCMRTable( &meansdvmaskcoi_tab );
        inittab = 1;
    }
    
    CX_CALL( mat = cxGetMat( mat, &stub, &coi ));

    type = CX_MAT_TYPE( mat->type );
    size = cxGetMatSize( mat );
    
    mat_step = mat->step;

    if( !mask )
    {
        if( CX_IS_MAT_CONT( mat->type ))
        {
            size.width *= size.height;
            size.height = 1;
            mat_step = CX_STUB_STEP;
        }

        if( CX_MAT_CN(type) == 1 || coi == 0 )
        {
            CxFunc2D_1A2P func = (CxFunc2D_1A2P)(meansdv_tab.fn_2d[type]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size, mean.val, sdv.val ));
        }
        else
        {
            CxFunc2DnC_1A2P func = (CxFunc2DnC_1A2P)
                (meansdvcoi_tab.fn_2d[CX_MAT_DEPTH(type)]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, size,
                             CX_MAT_CN(type), coi, mean.val, sdv.val ));
        }
    }
    else
    {
        CX_CALL( matmask = cxGetMat( matmask, &maskstub ));

        mask_step = matmask->step;

        if( !CX_IS_MASK_ARR( matmask ))
            CX_ERROR( CX_StsBadMask, "" );

        if( !CX_ARE_SIZES_EQ( mat, matmask ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );

        if( CX_IS_MAT_CONT( mat->type & matmask->type ))
        {
            size.width *= size.height;
            size.height = 1;
            mat_step = mask_step = CX_STUB_STEP;
        }

        if( CX_MAT_CN(type) == 1 || coi == 0 )
        {
            CxFunc2D_2A2P func = (CxFunc2D_2A2P)(meansdvmask_tab.fn_2d[type]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step, matmask->data.ptr,
                             mask_step, size, mean.val, sdv.val ));
        }
        else
        {
            CxFunc2DnC_2A2P func = (CxFunc2DnC_2A2P)
                (meansdvmaskcoi_tab.fn_2d[CX_MAT_DEPTH(type)]);

            if( !func )
                CX_ERROR( CX_StsBadArg, cxUnsupportedFormat );

            IPPI_CALL( func( mat->data.ptr, mat_step,
                             matmask->data.ptr, mask_step,
                             size, CX_MAT_CN(type), coi, mean.val, sdv.val ));
        }
    }

    __END__;

    if( _mean )
        *_mean = mean;

    if( _sdv )
        *_sdv = sdv;
}


/*  End of file  */
