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

#ifndef _CXCORE_IPP_H_
#define _CXCORE_IPP_H_

/****************************************************************************************\
*                                      Copy/Set                                          *
\****************************************************************************************/

IPCXAPI_EX( CxStatus, icxCopy_8u_C1R, "ippiCopy_8u_C1R", CX_IPPLIBS1(CX_IPP_IPPI),
                  ( const uchar* src, int src_step,
                    uchar* dst, int dst_step, CxSize size ))

IPCXAPI_EX( CxStatus, icxSetZero_8u_C1R, "ippiSetZero_8u_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),
                  ( uchar* dst, int dst_step, CxSize size ))


#define IPCX_CXT_TO( flavor )                                                   \
IPCXAPI_EX( CxStatus, icxCxtTo_##flavor##_C1R, "ippiCxtTo_" #flavor "_C1R", 0,  \
    ( const void* src, int step1, void* dst, int step, CxSize size, int param ))

IPCX_CXT_TO( 8u )
IPCX_CXT_TO( 8s )
IPCX_CXT_TO( 16s )
IPCX_CXT_TO( 32s )
IPCX_CXT_TO( 32f )
IPCX_CXT_TO( 64f )

#undef IPCX_CXT_TO

IPCXAPI_EX( CxStatus, icxCxt_32f64f, "ippsCxt_32f64f", 0, ( const float* src, double* dst, int len ))
IPCXAPI_EX( CxStatus, icxCxt_64f32f, "ippsCxt_64f32f", 0, ( const double* src, float* dst, int len ))

/* dst(idx) = src(idx)*a + b */
IPCXAPI_EX( CxStatus, icxScale_32f, "ippsScale_32f", 0, ( const float* src, float* dst,
                                                          int len, float a, float b ))
IPCXAPI_EX( CxStatus, icxScale_64f, "ippsScale_64f", 0, ( const double* src, double* dst,
                                                          int len, double a, double b ))

#define IPCX_COPYSET( flavor, arrtype, scalartype )                         \
IPCXAPI_EX( CxStatus, icxCopy##flavor, "ippiCopy" #flavor,                  \
                                    CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),  \
                                   ( const arrtype* src, int srcstep,       \
                                     arrtype* dst, int dststep, CxSize size,\
                                     const uchar* mask, int maskstep ))     \
IPCXAPI_EX( CxStatus, icxSet##flavor, "ippiSet" #flavor,                    \
                                    CX_IPPLIBS1(CX_IPP_OPTCX),              \
                                  ( arrtype* dst, int dststep,              \
                                    const uchar* mask, int maskstep,        \
                                    CxSize size, const arrtype* scalar ))

IPCX_COPYSET( _8u_C1MR, uchar, int )
IPCX_COPYSET( _8u_C2MR, ushort, int )
IPCX_COPYSET( _8u_C3MR, uchar, int )
IPCX_COPYSET( _16u_C2MR, int, int )
IPCX_COPYSET( _16u_C3MR, ushort, int )
IPCX_COPYSET( _32s_C2MR, int64, int64 )
IPCX_COPYSET( _32s_C3MR, int, int )
IPCX_COPYSET( _64s_C2MR, int, int )
IPCX_COPYSET( _64s_C3MR, int64, int64 )
IPCX_COPYSET( _64s_C4MR, int64, int64 )


/****************************************************************************************\
*                                       Arithmetics                                      *
\****************************************************************************************/

#define IPCX_BIN_ARITHM_NON_MASK( name )                            \
IPCXAPI_EX( CxStatus, icx##name##_8u_C1R,                           \
    "ippcx" #name "_8u_C1R, ippi" #name "_8u_C1R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),\
( const uchar* src1, int srcstep1, const uchar* src2, int srcstep2, \
  uchar* dst, int dststep, CxSize size ))                           \
IPCXAPI_EX( CxStatus, icx##name##_16s_C1R,                          \
    "ippcx" #name "_16s_C1R, ippi" #name "_16s_C1R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),\
( const short* src1, int srcstep1, const short* src2, int srcstep2, \
  short* dst, int dststep, CxSize size ))                           \
IPCXAPI_EX( CxStatus, icx##name##_32s_C1R,                          \
    "ippcx" #name "_32s_C1R, ippi" #name "_32s_C1R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),\
( const int* src1, int srcstep1, const int* src2, int srcstep2,     \
  int* dst, int dststep, CxSize size ))                             \
IPCXAPI_EX( CxStatus, icx##name##_32f_C1R,                          \
    "ippcx" #name "_32f_C1R, ippi" #name "_32f_C1R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),\
( const float* src1, int srcstep1, const float* src2, int srcstep2, \
  float* dst, int dststep, CxSize size ))                           \
IPCXAPI_EX( CxStatus, icx##name##_64f_C1R,                          \
    "ippcx" #name "_64f_C1R, ippi" #name "_64f_C1R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),\
( const double* src1, int srcstep1, const double* src2, int srcstep2,\
  double* dst, int dststep, CxSize size ))


#define IPCX_UN_ARITHM_NON_MASK( name )                             \
IPCXAPI_EX( CxStatus, icx##name##_8u_C1R,                           \
    "ippcx" #name "_8u_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),             \
( const uchar* src, int srcstep, uchar* dst, int dststep,           \
  CxSize size, const int* scalar ))                                 \
IPCXAPI_EX( CxStatus, icx##name##_16s_C1R,                          \
    "ippcx" #name "_16s_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),            \
( const short* src, int srcstep, short* dst, int dststep,           \
  CxSize size, const int* scalar ))                                 \
IPCXAPI_EX( CxStatus, icx##name##_32s_C1R,                          \
    "ippcx" #name "_32s_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),            \
( const int* src, int srcstep, int* dst, int dststep,               \
  CxSize size, const int* scalar ))                                 \
IPCXAPI_EX( CxStatus, icx##name##_32f_C1R,                          \
    "ippcx" #name "_32f_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),            \
( const float* src, int srcstep, float* dst, int dststep,           \
  CxSize size, const float* scalar ))                               \
IPCXAPI_EX( CxStatus, icx##name##_64f_C1R,                          \
    "ippcx" #name "_64f_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),            \
( const double* src, int srcstep, double* dst, int dststep,         \
  CxSize size, const double* scalar ))


IPCX_BIN_ARITHM_NON_MASK( Add )
IPCX_BIN_ARITHM_NON_MASK( Sub )
IPCX_UN_ARITHM_NON_MASK( AddC )
IPCX_UN_ARITHM_NON_MASK( SubRC )

#undef IPCX_BIN_ARITHM_NON_MASK
#undef IPCX_UN_ARITHM_NON_MASK

#define IPCX_MUL( flavor, arrtype )                                 \
IPCXAPI_EX( CxStatus, icxMul_##flavor##_C1R,                        \
    "ippcxMul_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),          \
( const arrtype* src1, int step1, const arrtype* src2, int step2,   \
  arrtype* dst, int step, CxSize size, double scale ))

IPCX_MUL( 8u, uchar )
IPCX_MUL( 16s, short )
IPCX_MUL( 32s, int )
IPCX_MUL( 32f, float )
IPCX_MUL( 64f, double )

#undef IPCX_MUL

/****************************************************************************************\
*                                     Logical operations                                 *
\****************************************************************************************/

#define IPCX_LOGIC( name )                                              \
IPCXAPI_EX( CxStatus, icx##name##_8u_C1R,                               \
    "ippcx" #name "_8u_C1R, ippi" #name "_8u_C1R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),\
( const uchar* src1, int srcstep1, const uchar* src2, int srcstep2,     \
  uchar* dst, int dststep, CxSize size ))                               \
IPCXAPI_EX( CxStatus, icx##name##C_8u_C1R,                              \
    "ippcx" #name "C_8u_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),                \
( const uchar* src1, int srcstep1, uchar* dst, int dststep,             \
  CxSize, const uchar* scalar, int pix_size ))

IPCX_LOGIC( And )
IPCX_LOGIC( Or )
IPCX_LOGIC( Xor )

#undef IPCX_LOGIC

IPCXAPI_EX( CxStatus, icxNot_8u_C1R, "ippcxNot_8u_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),
( const uchar* src, int step1, uchar* dst, int step, CxSize size ))

/****************************************************************************************\
*                                Image Statistics                                        *
\****************************************************************************************/

///////////////////////////////////////// Mean //////////////////////////////////////////

#define IPCX_DEF_MEAN_MASK( flavor, srctype )           \
IPCXAPI_EX( CxStatus, icxMean_##flavor##_C1MR,          \
"ippiMean_" #flavor "_C1MR", CX_IPPLIBS1(CX_IPP_IPPCX), \
( const srctype* img, int imgstep, const uchar* mask,   \
  int maskStep, CxSize size, double* mean ))            \
IPCXAPI_EX( CxStatus, icxMean_##flavor##_C2MR,          \
"ippiMean_" #flavor "_C2MR", CX_IPPLIBS1(CX_IPP_OPTCX), \
( const srctype* img, int imgstep, const uchar* mask,   \
  int maskStep, CxSize size, double* mean ))            \
IPCXAPI_EX( CxStatus, icxMean_##flavor##_C3MR,          \
"ippiMean_" #flavor "_C3MR", CX_IPPLIBS1(CX_IPP_OPTCX), \
( const srctype* img, int imgstep, const uchar* mask,   \
  int maskStep, CxSize size, double* mean ))            \
IPCXAPI_EX( CxStatus, icxMean_##flavor##_C4MR,          \
"ippiMean_" #flavor "_C4MR", CX_IPPLIBS1(CX_IPP_OPTCX), \
( const srctype* img, int imgstep, const uchar* mask,   \
  int maskStep, CxSize size, double* mean ))

IPCX_DEF_MEAN_MASK( 8u, uchar )
IPCX_DEF_MEAN_MASK( 16s, short )
IPCX_DEF_MEAN_MASK( 32s, int )
IPCX_DEF_MEAN_MASK( 32f, float )
IPCX_DEF_MEAN_MASK( 64f, double )

#undef IPCX_DEF_MEAN_MASK

//////////////////////////////////// Mean_StdDev ////////////////////////////////////////

#define IPCX_DEF_MEAN_SDV( flavor, srctype )                                \
IPCXAPI_EX( CxStatus, icxMean_StdDev_##flavor##_C1R,                        \
"ippiMean_StdDev_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPCX),               \
( const srctype* img, int imgstep, CxSize size, double* mean, double* sdv ))\
IPCXAPI_EX( CxStatus, icxMean_StdDev_##flavor##_C2R,                        \
"ippiMean_StdDev_" #flavor "_C2R", CX_IPPLIBS1(CX_IPP_OPTCX),               \
( const srctype* img, int imgstep, CxSize size, double* mean, double* sdv ))\
IPCXAPI_EX( CxStatus, icxMean_StdDev_##flavor##_C3R,                        \
"ippiMean_StdDev_" #flavor "_C3R", CX_IPPLIBS1(CX_IPP_OPTCX),               \
( const srctype* img, int imgstep, CxSize size, double* mean, double* sdv ))\
IPCXAPI_EX( CxStatus, icxMean_StdDev_##flavor##_C4R,                        \
"ippiMean_StdDev_" #flavor "_C4R", CX_IPPLIBS1(CX_IPP_OPTCX),               \
( const srctype* img, int imgstep, CxSize size, double* mean, double* sdv ))\
                                                                            \
IPCXAPI_EX( CxStatus, icxMean_StdDev_##flavor##_C1MR,                       \
"ippiMean_StdDev_" #flavor "_C1MR", CX_IPPLIBS1(CX_IPP_IPPCX),              \
( const srctype* img, int imgstep,                                          \
  const uchar* mask, int maskStep,                                          \
  CxSize size, double* mean, double* sdv ))                                 \
IPCXAPI_EX( CxStatus, icxMean_StdDev_##flavor##_C2MR,                       \
"ippiMean_StdDev_" #flavor "_C2MR", CX_IPPLIBS1(CX_IPP_OPTCX),              \
( const srctype* img, int imgstep,  const uchar* mask, int maskStep,        \
  CxSize size, double* mean, double* sdv ))                                 \
IPCXAPI_EX( CxStatus, icxMean_StdDev_##flavor##_C3MR,                       \
"ippiMean_StdDev_" #flavor "_C3MR", CX_IPPLIBS1(CX_IPP_OPTCX),              \
( const srctype* img, int imgstep,                                          \
  const uchar* mask, int maskStep,                                          \
  CxSize size, double* mean, double* sdv ))                                 \
IPCXAPI_EX( CxStatus, icxMean_StdDev_##flavor##_C4MR,                       \
"ippiMean_StdDev_" #flavor "_C4MR", CX_IPPLIBS1(CX_IPP_OPTCX),              \
( const srctype* img, int imgstep,                                          \
  const uchar* mask, int maskStep,                                          \
  CxSize size, double* mean, double* sdv ))

IPCX_DEF_MEAN_SDV( 8u, uchar )
IPCX_DEF_MEAN_SDV( 16s, short )
IPCX_DEF_MEAN_SDV( 32s, int )
IPCX_DEF_MEAN_SDV( 32f, float )
IPCX_DEF_MEAN_SDV( 64f, double )

#undef IPCX_DEF_MEAN_SDV

//////////////////////////////////// MinMaxIndx /////////////////////////////////////////


#define IPCX_DEF_MIN_MAX_LOC( flavor, srctype, extrtype )       \
IPCXAPI_EX( CxStatus, icxMinMaxIndx_##flavor##_C1R,             \
"ippiMinMaxIndx_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPCX),    \
( const srctype* img, int imgstep,                              \
  CxSize size, extrtype* minVal, extrtype* maxVal,              \
  CxPoint* minLoc, CxPoint* maxLoc ))                           \
                                                                \
IPCXAPI_EX( CxStatus, icxMinMaxIndx_##flavor##_C1MR,            \
"ippiMinMaxIndx_" #flavor "_C1MR", CX_IPPLIBS1(CX_IPP_IPPCX),   \
( const srctype* img, int imgstep,                              \
  const uchar* mask, int maskStep,                              \
  CxSize size, extrtype* minVal, extrtype* maxVal,              \
  CxPoint* minLoc, CxPoint* maxLoc ))

IPCX_DEF_MIN_MAX_LOC( 8u, uchar, float )
IPCX_DEF_MIN_MAX_LOC( 16s, short, float )
IPCX_DEF_MIN_MAX_LOC( 32s, int, double )
IPCX_DEF_MIN_MAX_LOC( 32f, float, float )
IPCX_DEF_MIN_MAX_LOC( 64f, double, double )

#undef IPCX_MIN_MAX_LOC

////////////////////////////////////////// Sum //////////////////////////////////////////

#define IPCX_DEF_SUM_NOHINT( flavor, srctype )                              \
IPCXAPI_EX( CxStatus, icxSum_##flavor##_C1R,                                \
            "ippiSum_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),            \
                                         ( const srctype* img, int imgstep, \
                                           CxSize size, double* sum ))      \
IPCXAPI_EX( CxStatus, icxSum_##flavor##_C2R,                                \
           "ippiSum_" #flavor "_C2R", CX_IPPLIBS1(CX_IPP_IPPI),             \
                                         ( const srctype* img, int imgstep, \
                                           CxSize size, double* sum ))      \
IPCXAPI_EX( CxStatus, icxSum_##flavor##_C3R,                                \
           "ippiSum_" #flavor "_C3R", CX_IPPLIBS1(CX_IPP_IPPI),             \
                                         ( const srctype* img, int imgstep, \
                                           CxSize size, double* sum ))      \
IPCXAPI_EX( CxStatus, icxSum_##flavor##_C4R,                                \
           "ippiSum_" #flavor "_C4R", CX_IPPLIBS1(CX_IPP_IPPI),             \
                                         ( const srctype* img, int imgstep, \
                                           CxSize size, double* sum ))

#define IPCX_DEF_SUM_HINT( flavor, srctype )                                \
IPCXAPI_EX( CxStatus, icxSum_##flavor##_C1R,                                \
            "ippiSum_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),            \
                        ( const srctype* img, int imgstep,                  \
                          CxSize size, double* sum, CxHintAlgorithm ))      \
IPCXAPI_EX( CxStatus, icxSum_##flavor##_C2R,                                \
           "ippiSum_" #flavor "_C2R", CX_IPPLIBS1(CX_IPP_IPPI),             \
                        ( const srctype* img, int imgstep,                  \
                          CxSize size, double* sum, CxHintAlgorithm ))      \
IPCXAPI_EX( CxStatus, icxSum_##flavor##_C3R,                                \
           "ippiSum_" #flavor "_C3R", CX_IPPLIBS1(CX_IPP_IPPI),             \
                        ( const srctype* img, int imgstep,                  \
                          CxSize size, double* sum, CxHintAlgorithm ))      \
IPCXAPI_EX( CxStatus, icxSum_##flavor##_C4R,                                \
           "ippiSum_" #flavor "_C4R", CX_IPPLIBS1(CX_IPP_IPPI),             \
                        ( const srctype* img, int imgstep,                  \
                          CxSize size, double* sum, CxHintAlgorithm ))

IPCX_DEF_SUM_NOHINT( 8u, uchar )
IPCX_DEF_SUM_NOHINT( 16s, short )
IPCX_DEF_SUM_NOHINT( 32s, int )
IPCX_DEF_SUM_HINT( 32f, float )
IPCX_DEF_SUM_NOHINT( 64f, double )

#undef IPCX_DEF_SUM_NOHINT
#undef IPCX_DEF_SUM_HINT

////////////////////////////////////////// CountNonZero /////////////////////////////////

#define IPCX_DEF_NON_ZERO( flavor, srctype )                        \
IPCXAPI_EX( CxStatus, icxCountNonZero_##flavor##_C1R,               \
    "ippiCountNonZero_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),  \
    ( const srctype* img, int imgstep, CxSize size, int* nonzero ))

IPCX_DEF_NON_ZERO( 8u, uchar )
IPCX_DEF_NON_ZERO( 16s, ushort )
IPCX_DEF_NON_ZERO( 32s, int )
IPCX_DEF_NON_ZERO( 32f, int )
IPCX_DEF_NON_ZERO( 64f, int64 )

#undef IPCX_DEF_NON_ZERO

////////////////////////////////////////// Norms /////////////////////////////////

#define IPCX_DEF_NORM_NOHINT_C1( flavor, srctype )                                      \
IPCXAPI_EX( CxStatus, icxNorm_Inf_##flavor##_C1R,                                       \
            "ippiNorm_Inf_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                   \
                                             ( const srctype* img, int imgstep,         \
                                               CxSize size, double* norm ))             \
IPCXAPI_EX( CxStatus, icxNorm_L1_##flavor##_C1R,                                        \
           "ippiNorm_L1_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                     \
                                             ( const srctype* img, int imgstep,         \
                                               CxSize size, double* norm ))             \
IPCXAPI_EX( CxStatus, icxNorm_L2_##flavor##_C1R,                                        \
           "ippiNorm_L2_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                     \
                                             ( const srctype* img, int imgstep,         \
                                               CxSize size, double* norm ))             \
IPCXAPI_EX( CxStatus, icxNormDiff_Inf_##flavor##_C1R,                                   \
           "ippiNormDiff_Inf_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               CxSize size, double* norm ))             \
IPCXAPI_EX( CxStatus, icxNormDiff_L1_##flavor##_C1R,                                    \
           "ippiNormDiff_L1_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                 \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               CxSize size, double* norm ))             \
IPCXAPI_EX( CxStatus, icxNormDiff_L2_##flavor##_C1R,                                    \
           "ippiNormDiff_L2_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                 \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               CxSize size, double* norm ))

#define IPCX_DEF_NORM_HINT_C1( flavor, srctype )                                        \
IPCXAPI_EX( CxStatus, icxNorm_Inf_##flavor##_C1R,                                       \
            "ippiNorm_Inf_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                   \
                                        ( const srctype* img, int imgstep,              \
                                          CxSize size, double* norm ))                  \
IPCXAPI_EX( CxStatus, icxNorm_L1_##flavor##_C1R,                                        \
           "ippiNorm_L1_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                     \
                                        ( const srctype* img, int imgstep,              \
                                          CxSize size, double* norm, CxHintAlgorithm )) \
IPCXAPI_EX( CxStatus, icxNorm_L2_##flavor##_C1R,                                        \
           "ippiNorm_L2_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                     \
                                        ( const srctype* img, int imgstep,              \
                                          CxSize size, double* norm, CxHintAlgorithm )) \
IPCXAPI_EX( CxStatus, icxNormDiff_Inf_##flavor##_C1R,                                   \
           "ippiNormDiff_Inf_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                \
                                        ( const srctype* img1, int imgstep1,            \
                                          const srctype* img2, int imgstep2,            \
                                          CxSize size, double* norm ))                  \
IPCXAPI_EX( CxStatus, icxNormDiff_L1_##flavor##_C1R,                                    \
           "ippiNormDiff_L1_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                 \
                                        ( const srctype* img1, int imgstep1,            \
                                          const srctype* img2, int imgstep2,            \
                                          CxSize size, double* norm, CxHintAlgorithm )) \
IPCXAPI_EX( CxStatus, icxNormDiff_L2_##flavor##_C1R,                                    \
           "ippiNormDiff_L2_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_IPPI),                 \
                                        ( const srctype* img1, int imgstep1,            \
                                          const srctype* img2, int imgstep2,            \
                                          CxSize size, double* norm, CxHintAlgorithm ))

#define IPCX_DEF_NORM_MASK_C1( flavor, srctype )                                        \
IPCXAPI_EX( CxStatus, icxNorm_Inf_##flavor##_C1MR,                                      \
           "ippiNorm_Inf_" #flavor "_C1MR", CX_IPPLIBS1(CX_IPP_IPPCX),                  \
                                             ( const srctype* img, int imgstep,         \
                                               const uchar* mask, int maskstep,         \
                                               CxSize size, double* norm ))             \
IPCXAPI_EX( CxStatus, icxNorm_L1_##flavor##_C1MR,                                       \
            "ippiNorm_L1_" #flavor "_C1MR", CX_IPPLIBS1(CX_IPP_IPPCX),                  \
                                             ( const srctype* img, int imgstep,         \
                                                const uchar* mask, int maskstep,        \
                                                CxSize size, double* norm ))            \
IPCXAPI_EX( CxStatus, icxNorm_L2_##flavor##_C1MR,                                       \
           "ippiNorm_L2_" #flavor "_C1MR", CX_IPPLIBS1(CX_IPP_IPPCX),                   \
                                             ( const srctype* img, int imgstep,         \
                                               const uchar* mask, int maskstep,         \
                                               CxSize size, double* norm ))             \
IPCXAPI_EX( CxStatus, icxNormDiff_Inf_##flavor##_C1MR,                                  \
           "ippiNormDiff_Inf_" #flavor "_C1MR", CX_IPPLIBS1(CX_IPP_IPPCX),              \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               const uchar* mask, int maskstep,         \
                                               CxSize size, double* norm ))             \
IPCXAPI_EX( CxStatus, icxNormDiff_L1_##flavor##_C1MR,                                   \
           "ippiNormDiff_L1_" #flavor "_C1MR", CX_IPPLIBS1(CX_IPP_IPPCX),               \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               const uchar* mask, int maskstep,         \
                                               CxSize size, double* norm ))             \
IPCXAPI_EX( CxStatus, icxNormDiff_L2_##flavor##_C1MR,                                   \
           "ippiNormDiff_L2_" #flavor "_C1MR", CX_IPPLIBS1(CX_IPP_IPPCX),               \
                                             ( const srctype* img1, int imgstep1,       \
                                               const srctype* img2, int imgstep2,       \
                                               const uchar* mask, int maskstep,         \
                                               CxSize size, double* norm ))

IPCX_DEF_NORM_NOHINT_C1( 8u, uchar )
IPCX_DEF_NORM_MASK_C1( 8u, uchar )

IPCX_DEF_NORM_NOHINT_C1( 16s, short )
IPCX_DEF_NORM_MASK_C1( 16s, short )

IPCX_DEF_NORM_NOHINT_C1( 32s, int )
IPCX_DEF_NORM_MASK_C1( 32s, int )

IPCX_DEF_NORM_HINT_C1( 32f, float )
IPCX_DEF_NORM_MASK_C1( 32f, float )

IPCX_DEF_NORM_NOHINT_C1( 64f, double )
IPCX_DEF_NORM_MASK_C1( 64f, double )

#undef IPCX_DEF_NORM_HONINT_C1
#undef IPCX_DEF_NORM_HINT_C1
#undef IPCX_DEF_NORM_MASK_C1

/****************************************************************************************\
*                                       Utilities                                        *
\****************************************************************************************/

////////////////////////////// Copy Pixel <-> Plane /////////////////////////////////

#define IPCX_PIX_PLANE( flavor, arrtype )                                           \
IPCXAPI_EX( CxStatus, icxCopy_##flavor##_C2P2R,                                     \
    "ippiCopy_" #flavor "_C2P2R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),            \
    ( const arrtype* src, int srcstep, arrtype** dst, int dststep, CxSize size ))   \
IPCXAPI_EX( CxStatus, icxCopy_##flavor##_C3P3R,                                     \
    "ippiCopy_" #flavor "_C3P3R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),            \
    ( const arrtype* src, int srcstep, arrtype** dst, int dststep, CxSize size ))   \
IPCXAPI_EX( CxStatus, icxCopy_##flavor##_C4P4R,                                     \
    "ippiCopy_" #flavor "_C4P4R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),            \
    ( const arrtype* src, int srcstep, arrtype** dst, int dststep, CxSize size ))   \
IPCXAPI_EX( CxStatus, icxCopy_##flavor##_CnC1CR,                                    \
    "ippiCopy_" #flavor "_CnC1CR", CX_IPPLIBS1(CX_IPP_OPTCX),                       \
    ( const arrtype* src, int srcstep, arrtype* dst, int dststep,                   \
      CxSize size, int cn, int coi ))                                               \
IPCXAPI_EX( CxStatus, icxCopy_##flavor##_C1CnCR,                                    \
    "ippiCopy_" #flavor "_CnC1CR", CX_IPPLIBS1(CX_IPP_OPTCX),                       \
    ( const arrtype* src, int srcstep, arrtype* dst, int dststep,                   \
      CxSize size, int cn, int coi ))                                               \
IPCXAPI_EX( CxStatus, icxCopy_##flavor##_P2C2R,                                     \
    "ippiCopy_" #flavor "_P2C2R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),            \
    ( const arrtype** src, int srcstep, arrtype* dst, int dststep, CxSize size ))   \
IPCXAPI_EX( CxStatus, icxCopy_##flavor##_P3C3R,                                     \
    "ippiCopy_" #flavor "_P3C3R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),            \
    ( const arrtype** src, int srcstep, arrtype* dst, int dststep, CxSize size ))   \
IPCXAPI_EX( CxStatus, icxCopy_##flavor##_P4C4R,                                     \
    "ippiCopy_" #flavor "_P4C4R", CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPI),            \
    ( const arrtype** src, int srcstep, arrtype* dst, int dststep, CxSize size ))

IPCX_PIX_PLANE( 8u, uchar )
IPCX_PIX_PLANE( 16u, ushort )
IPCX_PIX_PLANE( 32s, int )
IPCX_PIX_PLANE( 64f, int64 )

#undef IPCX_PIX_PLANE

/****************************************************************************************/
/*                            Math routines and RNGs                                    */
/****************************************************************************************/

IPCXAPI_EX( CxStatus, icxbInvSqrt_32f, "ippibInvSqrt_32f, ippsInvSqrt_32f_A21",
           CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPVM),
           ( const float* src, float* dst, int len ))
IPCXAPI_EX( CxStatus, icxbSqrt_32f, "ippibSqrt_32f, ippsSqrt_32f, ippsSqrt_32f_A21",
           CX_IPPLIBS3(CX_IPP_OPTCX,CX_IPP_IPPS,CX_IPP_IPPVM),
           ( const float* src, float* dst, int len ))
IPCXAPI_EX( CxStatus, icxbInvSqrt_64f, "ippibInvSqrt_64f, ippsInvSqrt_64f_A50",
           CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPVM),
           ( const double* src, double* dst, int len ))
IPCXAPI_EX( CxStatus, icxbSqrt_64f, "ippibInvSqrt_64f, ippsSqrt_64f, ippsInvSqrt_64f_A50",
           CX_IPPLIBS3(CX_IPP_OPTCX,CX_IPP_IPPS,CX_IPP_IPPVM),
           ( const double* src, double* dst, int len ))

IPCXAPI_EX( CxStatus, icxbLog_32f, "ippibLog_32f, ippsLn_32f",
           CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPS),
           ( const float *x, float *y, int n ) )
IPCXAPI_EX( CxStatus, icxbLog_64f32f, "ippibLog_64f32f, ippsLn_64f32f",
           CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPS),
           ( const double *x, float *y, int n ) )
IPCXAPI_EX( CxStatus, icxbLog_64f, "ippibLog_64f, ippsLn_64f",
           CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPS),
           ( const double *x, double *y, int n ) )
IPCXAPI_EX( CxStatus, icxbExp_32f, "ippibExp_32f, ippsExp_32f",
           CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPS),
           ( const float *x, float *y, int n ) )
IPCXAPI_EX( CxStatus, icxbExp_32f64f, "ippibExp_32f64f, ippsExp_32f64f",
           CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPS),
           ( const float *x, double *y, int n ) )
IPCXAPI_EX( CxStatus, icxbExp_64f, "ippibExp_64f, ippsExp_64f",
           CX_IPPLIBS2(CX_IPP_OPTCX,CX_IPP_IPPS),
           ( const double *x, double *y, int n ) )
IPCXAPI_EX( CxStatus, icxbFastArctan_32f, "ippibFastArctan_32f",
           CX_IPPLIBS1(CX_IPP_IPPCX),
           ( const float* y, const float* x, float* angle, int len ))

/****************************************************************************************/
/*                                  Error handling functions                            */
/****************************************************************************************/

IPCXAPI_EX( CxStatus, icxCheckArray_32f_C1R,
           "ippiCheckArray_32f_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),
           ( const float* src, int srcstep,
             CxSize size, int flags,
             double min_val, double max_val ))

IPCXAPI_EX( CxStatus, icxCheckArray_64f_C1R,
           "ippiCheckArray_64f_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),
           ( const double* src, int srcstep,
             CxSize size, int flags,
             double min_val, double max_val ))

/****************************************************************************************/
/*                                Vector operations                                     */
/****************************************************************************************/

#define IPCX_DOTPRODUCT_2D( flavor, arrtype, sumtype )                  \
IPCXAPI_EX( CxStatus, icxDotProduct_##flavor##_C1R,                     \
           "ippiDotProduct_" #flavor "_C1R", CX_IPPLIBS1(CX_IPP_OPTCX), \
                              ( const arrtype* src1, int step1,         \
                                const arrtype* src2, int step2,         \
                                CxSize size, sumtype* _sum ))

IPCX_DOTPRODUCT_2D( 8u, uchar, int64 )
IPCX_DOTPRODUCT_2D( 16s, short, int64 )
IPCX_DOTPRODUCT_2D( 32s, int, double )
IPCX_DOTPRODUCT_2D( 32f, float, double )
IPCX_DOTPRODUCT_2D( 64f, double, double )

#undef IPCX_DOTPRODUCT_2D

/****************************************************************************************/
/*                                    Linear Algebra                                    */
/****************************************************************************************/

IPCXAPI_EX( CxStatus, icxLUDecomp_32f, "ippiLUDecomp_32f_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),
                                    ( float* A, int stepA, CxSize sizeA, 
                                      float* B, int stepB, CxSize sizeB,
                                      double* _det ))

IPCXAPI_EX( CxStatus, icxLUDecomp_64f, "ippiLUDecomp_64f_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),
                                    ( double* A, int stepA, CxSize sizeA, 
                                      double* B, int stepB, CxSize sizeB,
                                      double* _det ))

IPCXAPI_EX( CxStatus, icxLUBack_32f, "ippiLUBack_32f_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),
                                  ( float* A, int stepA, CxSize sizeA, 
                                    float* B, int stepB, CxSize sizeB ))

IPCXAPI_EX( CxStatus, icxLUBack_64f, "ippiLUBack_64f_C1R", CX_IPPLIBS1(CX_IPP_OPTCX),
                                  ( double* A, int stepA, CxSize sizeA, 
                                    double* B, int stepB, CxSize sizeB ))

#endif /*_CXCORE_IPP_H_*/

