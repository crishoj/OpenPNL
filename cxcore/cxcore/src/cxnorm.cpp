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
*                                  N o r m                                               *
\****************************************************************************************/

#define ICX_DEF_NORM_CASE( _op_, _update_op_, temptype, len ) \
{                                                       \
    for( x = 0; x <= (len) - 4; x += 4 )                \
    {                                                   \
        temptype t0 = (src)[x];                         \
        temptype t1 = (src)[x+1];                       \
                                                        \
        t0 = (temptype)_op_(t0);                        \
        t1 = (temptype)_op_(t1);                        \
                                                        \
        norm = _update_op_( norm, t0 );                 \
        norm = _update_op_( norm, t1 );                 \
                                                        \
        t0 = (src)[x+2];                                \
        t1 = (src)[x+3];                                \
                                                        \
        t0 = (temptype)_op_(t0);                        \
        t1 = (temptype)_op_(t1);                        \
                                                        \
        norm = _update_op_( norm, t0 );                 \
        norm = _update_op_( norm, t1 );                 \
    }                                                   \
                                                        \
    for( ; x < (len); x++ )                             \
    {                                                   \
        temptype t0 = (src)[x];                         \
        t0 = (temptype)_op_(t0);                        \
        norm = _update_op_( norm, t0 );                 \
    }                                                   \
}


#define ICX_DEF_NORM_DIFF_CASE( _op_, _diff_op_, _update_op_,\
                                temptype, len ) \
{                                               \
    for( x = 0; x <= (len) - 4; x += 4 )        \
    {                                           \
        temptype t0 = (src1)[x] - (src2)[x];    \
        temptype t1 = (src1)[x+1] - (src2)[x+1];\
                                                \
        t0 = (temptype)_diff_op_(t0);           \
        t1 = (temptype)_diff_op_(t1);           \
                                                \
        norm = _update_op_( norm, t0 );         \
        norm = _update_op_( norm, t1 );         \
                                                \
        t0 = (src1)[x+2] - (src2)[x+2];         \
        t1 = (src1)[x+3] - (src2)[x+3];         \
                                                \
        t0 = (temptype)_diff_op_(t0);           \
        t1 = (temptype)_diff_op_(t1);           \
                                                \
        norm = _update_op_( norm, t0 );         \
        norm = _update_op_( norm, t1 );         \
    }                                           \
                                                \
    for( ; x < (len); x++ )                     \
    {                                           \
        temptype t0 = (src1)[x] - (src2)[x];    \
        t0 = (temptype)_diff_op_(t0);           \
        norm = _update_op_( norm, t0 );         \
    }                                           \
}


#define ICX_DEF_NORM_COI_CASE( _op_, _update_op_, temptype, len, cn ) \
{                                                          \
    for( x = 0; x < (len); x++ )                           \
    {                                                      \
        temptype t0 = (src)[x*(cn)];                       \
        t0 = (temptype)_op_(t0);                           \
        norm = _update_op_( norm, t0 );                    \
    }                                                      \
}


#define ICX_DEF_NORM_DIFF_COI_CASE( _op_, _diff_op_, _update_op_,   \
                                temptype, len, cn )     \
{                                                       \
    for( x = 0; x < (len); x++ )                        \
    {                                                   \
        temptype t0 = (src1)[x*(cn)] - (src2)[x*(cn)];  \
        t0 = (temptype)_diff_op_(t0);                   \
        norm = _update_op_( norm, t0 );                 \
    }                                                   \
}


#define  CX_NORM_ENTRY( normtype )  \
    int  x;                         \
    normtype norm = 0

#define  CX_NORM_REL_ENTRY( normtype )  \
    int  x;                             \
    normtype norm = 0, norm_diff = 0

#define  CX_NORM_EXIT( cn )  *_norm = (double)norm

#define  CX_NORM_REL_EXIT( cn )  \
    *_norm = ((double)norm_diff)/((double)norm + DBL_EPSILON)

#define  CX_NORM_L2_EXIT( cn )  *_norm = sqrt((double)norm)

#define  CX_NORM_REL_L2_EXIT( cn )  \
    *_norm = sqrt((double)norm_diff)/sqrt((double)norm + DBL_EPSILON)


#define  ICX_DEF_NORM_NOHINT_FUNC_2D( _op_, _update_op_, _entry_, _case_, _exit_,\
                               name, srctype, normtype, temptype )  \
IPCXAPI_IMPL( CxStatus, name, ( const srctype* src, int step,       \
                                CxSize size, double* _norm ))       \
{                                                                   \
    _entry_( normtype );                                            \
                                                                    \
    for( ; size.height--; (char*&)src += step )                     \
    {                                                               \
        _case_( _op_, _update_op_, temptype, size.width );          \
    }                                                               \
                                                                    \
    _exit_(1);                                                      \
                                                                    \
    return CX_OK;                                                   \
}

#define  ICX_DEF_NORM_HINT_FUNC_2D( _op_, _update_op_, _entry_, _case_, _exit_,\
                               name, srctype, normtype, temptype )  \
IPCXAPI_IMPL( CxStatus, name, ( const srctype* src, int step,       \
                                CxSize size, double* _norm,         \
                                CxHintAlgorithm /* hint */ ))       \
{                                                                   \
    _entry_( normtype );                                            \
                                                                    \
    for( ; size.height--; (char*&)src += step )                     \
    {                                                               \
        _case_( _op_, _update_op_, temptype, size.width );          \
    }                                                               \
                                                                    \
    _exit_(1);                                                      \
                                                                    \
    return CX_OK;                                                   \
}


#define  ICX_DEF_NORM_FUNC_2D_COI( _op_, _update_op_, _entry_, _case_, _exit_,\
                                   name, srctype, normtype, temptype )\
static CxStatus CX_STDCALL name( const srctype* src, int step,  \
                  CxSize size, int cn, int coi, double* _norm ) \
{                                                               \
    _entry_( normtype );                                        \
                                                                \
    src += coi - 1;                                             \
    for( ; size.height--; (char*&)src += step )                 \
    {                                                           \
        _case_( _op_, _update_op_, temptype, size.width, cn );  \
    }                                                           \
                                                                \
    _exit_(1);                                                  \
                                                                \
    return CX_OK;                                               \
}



#define  ICX_DEF_NORM_DIFF_NOHINT_FUNC_2D( _op_, _diff_op_, _update_op_,\
                                    _entry_, _case_, _exit_,            \
                                    name, srctype, normtype, temptype ) \
IPCXAPI_IMPL( CxStatus, name,( const srctype* src1, int step1,          \
                               const srctype* src2, int step2,          \
                               CxSize size, double* _norm ))            \
{                                                                       \
    _entry_( normtype );                                                \
                                                                        \
    for( ; size.height--; (char*&)src1 += step1,                        \
                          (char*&)src2 += step2 )                       \
    {                                                                   \
        _case_( _op_, _diff_op_, _update_op_, temptype, size.width );   \
    }                                                                   \
                                                                        \
    _exit_(1);                                                          \
                                                                        \
    return CX_OK;                                                       \
}


#define  ICX_DEF_NORM_DIFF_HINT_FUNC_2D( _op_, _diff_op_, _update_op_,  \
                                    _entry_, _case_, _exit_,            \
                                    name, srctype, normtype, temptype ) \
IPCXAPI_IMPL( CxStatus, name,( const srctype* src1, int step1,          \
                               const srctype* src2, int step2,          \
                               CxSize size, double* _norm,              \
                               CxHintAlgorithm /* hint */ ))            \
{                                                                       \
    _entry_( normtype );                                                \
                                                                        \
    for( ; size.height--; (char*&)src1 += step1,                        \
                          (char*&)src2 += step2 )                       \
    {                                                                   \
        _case_( _op_, _diff_op_, _update_op_, temptype, size.width );   \
    }                                                                   \
                                                                        \
    _exit_(1);                                                          \
                                                                        \
    return CX_OK;                                                       \
}


#define  ICX_DEF_NORM_DIFF_FUNC_2D_COI( _op_, _diff_op_, _update_op_,   \
                                        _entry_, _case_, _exit_, name,  \
                                        srctype, normtype, temptype )   \
static CxStatus CX_STDCALL name( const srctype* src1, int step1,        \
                       const srctype* src2, int step2,                  \
                       CxSize size, int cn, int coi, double* _norm )    \
{                                                                       \
    _entry_( normtype );                                                \
                                                                        \
    src1 += coi - 1;                                                    \
    src2 += coi - 1;                                                    \
    for( ; size.height--; (char*&)src1 += step1,                        \
                          (char*&)src2 += step2 )                       \
    {                                                                   \
        _case_( _op_, _diff_op_, _update_op_, temptype, size.width, cn );\
    }                                                                   \
                                                                        \
    _exit_(1);                                                          \
                                                                        \
    return CX_OK;                                                       \
}



#define ICX_DEF_NORM_FUNC_ALL( _abs_macro_, _abs_diff_macro_, flavor, srctype,          \
                               c_normtype, l_normtype, temptype, hint )                 \
                                                                                        \
ICX_DEF_NORM_NOHINT_FUNC_2D( _abs_macro_, MAX,                                          \
                      CX_NORM_ENTRY, ICX_DEF_NORM_CASE, CX_NORM_EXIT,                   \
                      icxNorm_Inf_##flavor##_C1R, srctype, c_normtype, temptype )       \
ICX_DEF_NORM_##hint##_FUNC_2D( _abs_macro_, CX_ADD,                                     \
                      CX_NORM_ENTRY, ICX_DEF_NORM_CASE, CX_NORM_EXIT,                   \
                      icxNorm_L1_##flavor##_C1R, srctype, l_normtype, temptype )        \
ICX_DEF_NORM_##hint##_FUNC_2D( CX_SQR, CX_ADD,                                          \
                      CX_NORM_ENTRY, ICX_DEF_NORM_CASE, CX_NORM_L2_EXIT,                \
                      icxNorm_L2_##flavor##_C1R, srctype, l_normtype, temptype )        \
                                                                                        \
ICX_DEF_NORM_DIFF_NOHINT_FUNC_2D( _abs_macro_, _abs_diff_macro_, MAX,                   \
                        CX_NORM_ENTRY, ICX_DEF_NORM_DIFF_CASE, CX_NORM_EXIT,            \
                        icxNormDiff_Inf_##flavor##_C1R, srctype, c_normtype, temptype ) \
ICX_DEF_NORM_DIFF_##hint##_FUNC_2D( _abs_macro_, _abs_diff_macro_, CX_ADD,              \
                        CX_NORM_ENTRY, ICX_DEF_NORM_DIFF_CASE, CX_NORM_EXIT,            \
                        icxNormDiff_L1_##flavor##_C1R, srctype, l_normtype, temptype )  \
ICX_DEF_NORM_DIFF_##hint##_FUNC_2D( CX_SQR, CX_SQR, CX_ADD,                             \
                          CX_NORM_ENTRY, ICX_DEF_NORM_DIFF_CASE, CX_NORM_L2_EXIT,       \
                          icxNormDiff_L2_##flavor##_C1R, srctype, l_normtype, temptype) \
                                                                                        \
ICX_DEF_NORM_FUNC_2D_COI( _abs_macro_, MAX,                                             \
                         CX_NORM_ENTRY, ICX_DEF_NORM_COI_CASE, CX_NORM_EXIT,            \
                         icxNorm_Inf_##flavor##_CnCR, srctype, c_normtype, temptype )   \
ICX_DEF_NORM_FUNC_2D_COI( _abs_macro_, CX_ADD,                                          \
                          CX_NORM_ENTRY, ICX_DEF_NORM_COI_CASE, CX_NORM_EXIT,           \
                          icxNorm_L1_##flavor##_CnCR, srctype, l_normtype, temptype )   \
ICX_DEF_NORM_FUNC_2D_COI( CX_SQR, CX_ADD,                                               \
                          CX_NORM_ENTRY, ICX_DEF_NORM_COI_CASE, CX_NORM_L2_EXIT,        \
                          icxNorm_L2_##flavor##_CnCR, srctype, l_normtype, temptype )   \
                                                                                        \
ICX_DEF_NORM_DIFF_FUNC_2D_COI( _abs_macro_, _abs_diff_macro_, MAX,                      \
                          CX_NORM_ENTRY, ICX_DEF_NORM_DIFF_COI_CASE, CX_NORM_EXIT,      \
                          icxNormDiff_Inf_##flavor##_CnCR, srctype, c_normtype, temptype)\
ICX_DEF_NORM_DIFF_FUNC_2D_COI( _abs_macro_, _abs_diff_macro_, CX_ADD,                   \
                          CX_NORM_ENTRY, ICX_DEF_NORM_DIFF_COI_CASE, CX_NORM_EXIT,      \
                          icxNormDiff_L1_##flavor##_CnCR,                               \
                          srctype, l_normtype, temptype )                               \
ICX_DEF_NORM_DIFF_FUNC_2D_COI( CX_SQR, CX_SQR, CX_ADD,                                  \
                          CX_NORM_ENTRY, ICX_DEF_NORM_DIFF_COI_CASE, CX_NORM_L2_EXIT,   \
                          icxNormDiff_L2_##flavor##_CnCR,                               \
                          srctype, l_normtype, temptype )


ICX_DEF_NORM_FUNC_ALL( CX_NOP, CX_IABS, 8u, uchar, int, int64, int, NOHINT )
ICX_DEF_NORM_FUNC_ALL( CX_IABS, CX_IABS, 16s, short, int, int64, int, NOHINT )
ICX_DEF_NORM_FUNC_ALL( fabs, fabs, 32s, int, double, double, double, NOHINT )
ICX_DEF_NORM_FUNC_ALL( fabs, fabs, 32f, float, double, double, double, HINT )
ICX_DEF_NORM_FUNC_ALL( fabs, fabs, 64f, double, double, double, double, NOHINT )


#define ICX_DEF_INIT_NORM_TAB_2D( FUNCNAME, FLAG )              \
static void icxInit##FUNCNAME##FLAG##Table( CxFuncTable* tab )  \
{                                                               \
    tab->fn_2d[CX_8U] = (void*)icx##FUNCNAME##_8u_##FLAG;       \
    tab->fn_2d[CX_8S] = 0;                                      \
    tab->fn_2d[CX_16S] = (void*)icx##FUNCNAME##_16s_##FLAG;     \
    tab->fn_2d[CX_32S] = (void*)icx##FUNCNAME##_32s_##FLAG;     \
    tab->fn_2d[CX_32F] = (void*)icx##FUNCNAME##_32f_##FLAG;     \
    tab->fn_2d[CX_64F] = (void*)icx##FUNCNAME##_64f_##FLAG;     \
}

ICX_DEF_INIT_NORM_TAB_2D( Norm_Inf, C1R )
ICX_DEF_INIT_NORM_TAB_2D( Norm_L1, C1R )
ICX_DEF_INIT_NORM_TAB_2D( Norm_L2, C1R )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_Inf, C1R )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_L1, C1R )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_L2, C1R )

ICX_DEF_INIT_NORM_TAB_2D( Norm_Inf, CnCR )
ICX_DEF_INIT_NORM_TAB_2D( Norm_L1, CnCR )
ICX_DEF_INIT_NORM_TAB_2D( Norm_L2, CnCR )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_Inf, CnCR )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_L1, CnCR )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_L2, CnCR )


/****************************************************************************************\
*                             N o r m   with    M A S K                                  *
\****************************************************************************************/


#define ICX_DEF_NORM_MASK_CASE( _mask_op_, _op_, _update_op_,\
                                temptype, len )         \
{                                                       \
    for( x = 0; x <= (len) - 2; x += 2 )                \
    {                                                   \
        temptype t0 = _mask_op_(mask[x],(src)[x]);      \
        temptype t1 = _mask_op_(mask[x+1],(src)[x+1]);  \
                                                        \
        t0 = (temptype)_op_(t0);                        \
        t1 = (temptype)_op_(t1);                        \
                                                        \
        norm = _update_op_( norm, t0 );                 \
        norm = _update_op_( norm, t1 );                 \
    }                                                   \
                                                        \
    if( x < (len) )                                     \
    {                                                   \
        temptype t0 = _mask_op_(mask[x],(src)[x]);      \
        t0 = (temptype)_op_(t0);                        \
        norm = _update_op_( norm, t0 );                 \
    }                                                   \
}


#define ICX_DEF_NORM_DIFF_MASK_CASE( _mask_op_, _op_, _diff_op_,    \
                                     _update_op_, temptype, len )   \
{                                                                   \
    for( x = 0; x <= (len) - 2; x += 2 )                            \
    {                                                               \
        temptype t0 = _mask_op_(mask[x],(src1)[x] - (src2)[x]);     \
        temptype t1 = _mask_op_(mask[x+1],(src1)[x+1] - (src2)[x+1]);\
                                                                    \
        t0 = (temptype)_diff_op_(t0);                               \
        t1 = (temptype)_diff_op_(t1);                               \
                                                                    \
        norm = _update_op_( norm, t0 );                             \
        norm = _update_op_( norm, t1 );                             \
    }                                                               \
                                                                    \
    if( x < (len) )                                                 \
    {                                                               \
        temptype t0 = _mask_op_(mask[x],(src1)[x] - (src2)[x]);     \
        t0 = (temptype)_diff_op_(t0);                               \
        norm = _update_op_( norm, t0 );                             \
    }                                                               \
}


#define ICX_DEF_NORM_MASK_COI_CASE( _mask_op_, _op_, _update_op_,\
                                temptype, len, cn )     \
{                                                       \
    for( x = 0; x < (len); x++ )                        \
    {                                                   \
        temptype t0 = _mask_op_(mask[x],(src)[x*(cn)]); \
        t0 = (temptype)_op_(t0);                        \
        norm = _update_op_( norm, t0 );                 \
    }                                                   \
}


#define ICX_DEF_NORM_DIFF_MASK_COI_CASE( _mask_op_, _op_, _diff_op_, \
                                     _update_op_, temptype, len, cn )\
{                                                                   \
    for( x = 0; x < (len); x++ )                                    \
    {                                                               \
        temptype t0 = _mask_op_(mask[x],(src1)[x*(cn)]-(src2)[x*(cn)]);\
        t0 = (temptype)_diff_op_(t0);                               \
        norm = _update_op_( norm, t0 );                             \
    }                                                               \
}


#define  CX_NORM_MASK_ENTRY_FLT( normtype )  \
    int  x;                                  \
    float maskTab[] = { 0.f, 1.f };          \
    normtype norm = 0



#define  ICX_DEF_NORM_MASK_FUNC_2D( _mask_op_, _op_, _update_op_,   \
                                    _entry_, _case_, _exit_,        \
                                    name, srctype, normtype, temptype )  \
IPCXAPI_IMPL( CxStatus, name, ( const srctype* src, int step,       \
                       const uchar* mask, int maskStep,             \
                       CxSize size, double* _norm ))                \
{                                                                   \
    _entry_( normtype );                                            \
                                                                    \
    for( ; size.height--; (char*&)src += step, mask += maskStep )   \
    {                                                               \
        _case_( _mask_op_, _op_, _update_op_,                       \
                temptype, size.width );                             \
    }                                                               \
                                                                    \
    _exit_(1);                                                      \
                                                                    \
    return CX_OK;                                                   \
}


#define  ICX_DEF_NORM_MASK_FUNC_2D_COI( _mask_op_, _op_, _update_op_,\
                                        _entry_, _case_, _exit_,    \
                                        name, srctype, normtype,    \
                                        temptype )                  \
static CxStatus CX_STDCALL name( const srctype* src, int step,      \
                       const uchar* mask, int maskStep,             \
                       CxSize size, int cn, int coi, double* _norm )\
{                                                                   \
    _entry_( normtype );                                            \
                                                                    \
    src += coi - 1;                                                 \
    for( ; size.height--; (char*&)src += step, mask += maskStep )   \
    {                                                               \
        _case_( _mask_op_, _op_, _update_op_,                       \
                temptype, size.width, cn );                         \
    }                                                               \
                                                                    \
    _exit_(1);                                                      \
                                                                    \
    return CX_OK;                                                   \
}



#define  ICX_DEF_NORM_DIFF_MASK_FUNC_2D( _mask_op_, _op_, _diff_op_, _update_op_,   \
                                         _entry_, _case_, _exit_, name,             \
                                         srctype, normtype, temptype )              \
IPCXAPI_IMPL( CxStatus, name, ( const srctype* src1, int step1,                     \
                       const srctype* src2, int step2,                              \
                       const uchar* mask, int maskStep,                             \
                       CxSize size, double* _norm ))                                \
{                                                                                   \
    _entry_( normtype );                                                            \
                                                                                    \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2, mask += maskStep )\
    {                                                                               \
        _case_( _mask_op_, _op_, _diff_op_, _update_op_, temptype, size.width );    \
    }                                                                               \
                                                                                    \
    _exit_(1);                                                                      \
                                                                                    \
    return CX_OK;                                                                   \
}


#define  ICX_DEF_NORM_DIFF_MASK_FUNC_2D_COI(_mask_op_, _op_, _diff_op_, _update_op_,\
                                            _entry_, _case_, _exit_, name,          \
                                             srctype, normtype, temptype )          \
static CxStatus CX_STDCALL name( const srctype* src1, int step1,                    \
                                 const srctype* src2, int step2,                    \
                                 const uchar* mask, int maskStep,                   \
                                 CxSize size, int cn, int coi, double* _norm )      \
{                                                                                   \
    _entry_( normtype );                                                            \
                                                                                    \
    src1 += coi - 1;                                                                \
    src2 += coi - 1;                                                                \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2, mask += maskStep )\
    {                                                                               \
        _case_( _mask_op_, _op_, _diff_op_, _update_op_, temptype, size.width, cn );\
    }                                                                               \
                                                                                    \
    _exit_(1);                                                                      \
                                                                                    \
    return CX_OK;                                                                   \
}


#define ICX_DEF_NORM_MASK_FUNC_ALL( _mask_op_, _abs_macro_, _abs_diff_macro_, _entry_,      \
                                    flavor, srctype, c_normtype, l_normtype, temptype )     \
                                                                                            \
ICX_DEF_NORM_MASK_FUNC_2D( _mask_op_, _abs_macro_, MAX,                                     \
                      _entry_, ICX_DEF_NORM_MASK_CASE, CX_NORM_EXIT,                        \
                      icxNorm_Inf_##flavor##_C1MR, srctype, c_normtype, temptype )          \
ICX_DEF_NORM_MASK_FUNC_2D( _mask_op_, _abs_macro_, CX_ADD,                                  \
                      _entry_, ICX_DEF_NORM_MASK_CASE, CX_NORM_EXIT,                        \
                      icxNorm_L1_##flavor##_C1MR, srctype, l_normtype, temptype )           \
ICX_DEF_NORM_MASK_FUNC_2D( _mask_op_, CX_SQR, CX_ADD,                                       \
                      _entry_, ICX_DEF_NORM_MASK_CASE, CX_NORM_L2_EXIT,                     \
                      icxNorm_L2_##flavor##_C1MR, srctype, l_normtype, temptype )           \
                                                                                            \
ICX_DEF_NORM_DIFF_MASK_FUNC_2D( _mask_op_, _abs_macro_, _abs_diff_macro_, MAX,              \
                        _entry_, ICX_DEF_NORM_DIFF_MASK_CASE, CX_NORM_EXIT,                 \
                        icxNormDiff_Inf_##flavor##_C1MR, srctype, c_normtype, temptype )    \
ICX_DEF_NORM_DIFF_MASK_FUNC_2D( _mask_op_, _abs_macro_, _abs_diff_macro_, CX_ADD,           \
                        _entry_, ICX_DEF_NORM_DIFF_MASK_CASE, CX_NORM_EXIT,                 \
                        icxNormDiff_L1_##flavor##_C1MR, srctype, l_normtype, temptype )     \
ICX_DEF_NORM_DIFF_MASK_FUNC_2D( _mask_op_, CX_SQR, CX_SQR, CX_ADD,                          \
                          _entry_, ICX_DEF_NORM_DIFF_MASK_CASE, CX_NORM_L2_EXIT,            \
                          icxNormDiff_L2_##flavor##_C1MR, srctype, l_normtype, temptype)    \
                                                                                            \
ICX_DEF_NORM_MASK_FUNC_2D_COI( _mask_op_, _abs_macro_, MAX,                                 \
                         _entry_, ICX_DEF_NORM_MASK_COI_CASE, CX_NORM_EXIT,                 \
                         icxNorm_Inf_##flavor##_CnCMR, srctype, c_normtype, temptype )      \
ICX_DEF_NORM_MASK_FUNC_2D_COI( _mask_op_, _abs_macro_, CX_ADD,                              \
                          _entry_, ICX_DEF_NORM_MASK_COI_CASE, CX_NORM_EXIT,                \
                          icxNorm_L1_##flavor##_CnCMR, srctype, l_normtype, temptype )      \
ICX_DEF_NORM_MASK_FUNC_2D_COI( _mask_op_, CX_SQR, CX_ADD,                                   \
                          _entry_, ICX_DEF_NORM_MASK_COI_CASE, CX_NORM_L2_EXIT,             \
                          icxNorm_L2_##flavor##_CnCMR, srctype, l_normtype, temptype )      \
                                                                                            \
ICX_DEF_NORM_DIFF_MASK_FUNC_2D_COI( _mask_op_, _abs_macro_, _abs_diff_macro_, MAX,          \
                          _entry_, ICX_DEF_NORM_DIFF_MASK_COI_CASE, CX_NORM_EXIT,           \
                          icxNormDiff_Inf_##flavor##_CnCMR, srctype, c_normtype, temptype ) \
ICX_DEF_NORM_DIFF_MASK_FUNC_2D_COI( _mask_op_, _abs_macro_, _abs_diff_macro_, CX_ADD,       \
                          _entry_, ICX_DEF_NORM_DIFF_MASK_COI_CASE, CX_NORM_EXIT,           \
                          icxNormDiff_L1_##flavor##_CnCMR,                                  \
                          srctype, l_normtype, temptype )                                   \
ICX_DEF_NORM_DIFF_MASK_FUNC_2D_COI( _mask_op_, CX_SQR, CX_SQR, CX_ADD,                      \
                          _entry_, ICX_DEF_NORM_DIFF_MASK_COI_CASE, CX_NORM_L2_EXIT,        \
                          icxNormDiff_L2_##flavor##_CnCMR,                                  \
                          srctype, l_normtype, temptype )


ICX_DEF_NORM_MASK_FUNC_ALL( CX_ANDMASK, CX_NOP, CX_IABS, CX_NORM_ENTRY,
                            8u, uchar, int, int64, int )
ICX_DEF_NORM_MASK_FUNC_ALL( CX_ANDMASK, CX_IABS, CX_IABS, CX_NORM_ENTRY,
                            16s, short, int, int64, int )
ICX_DEF_NORM_MASK_FUNC_ALL( CX_MULMASK, fabs, fabs, CX_NORM_MASK_ENTRY_FLT,
                            32s, int, double, double, double )
ICX_DEF_NORM_MASK_FUNC_ALL( CX_MULMASK, fabs, fabs, CX_NORM_MASK_ENTRY_FLT,
                            32f, float, double, double, double )
ICX_DEF_NORM_MASK_FUNC_ALL( CX_MULMASK, fabs, fabs, CX_NORM_MASK_ENTRY_FLT,
                            64f, double, double, double, double )


ICX_DEF_INIT_NORM_TAB_2D( Norm_Inf, C1MR )
ICX_DEF_INIT_NORM_TAB_2D( Norm_L1, C1MR )
ICX_DEF_INIT_NORM_TAB_2D( Norm_L2, C1MR )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_Inf, C1MR )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_L1, C1MR )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_L2, C1MR )

ICX_DEF_INIT_NORM_TAB_2D( Norm_Inf, CnCMR )
ICX_DEF_INIT_NORM_TAB_2D( Norm_L1, CnCMR )
ICX_DEF_INIT_NORM_TAB_2D( Norm_L2, CnCMR )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_Inf, CnCMR )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_L1, CnCMR )
ICX_DEF_INIT_NORM_TAB_2D( NormDiff_L2, CnCMR )


static void icxInitNormTabs( CxFuncTable* norm_tab, CxFuncTable* normmask_tab )
{
    icxInitNorm_InfC1RTable( &norm_tab[0] );
    icxInitNorm_L1C1RTable( &norm_tab[1] );
    icxInitNorm_L2C1RTable( &norm_tab[2] );
    icxInitNormDiff_InfC1RTable( &norm_tab[3] );
    icxInitNormDiff_L1C1RTable( &norm_tab[4] );
    icxInitNormDiff_L2C1RTable( &norm_tab[5] );

    icxInitNorm_InfCnCRTable( &norm_tab[6] );
    icxInitNorm_L1CnCRTable( &norm_tab[7] );
    icxInitNorm_L2CnCRTable( &norm_tab[8] );
    icxInitNormDiff_InfCnCRTable( &norm_tab[9] );
    icxInitNormDiff_L1CnCRTable( &norm_tab[10] );
    icxInitNormDiff_L2CnCRTable( &norm_tab[11] );

    icxInitNorm_InfC1MRTable( &normmask_tab[0] );
    icxInitNorm_L1C1MRTable( &normmask_tab[1] );
    icxInitNorm_L2C1MRTable( &normmask_tab[2] );
    icxInitNormDiff_InfC1MRTable( &normmask_tab[3] );
    icxInitNormDiff_L1C1MRTable( &normmask_tab[4] );
    icxInitNormDiff_L2C1MRTable( &normmask_tab[5] );

    icxInitNorm_InfCnCMRTable( &normmask_tab[6] );
    icxInitNorm_L1CnCMRTable( &normmask_tab[7] );
    icxInitNorm_L2CnCMRTable( &normmask_tab[8] );
    icxInitNormDiff_InfCnCMRTable( &normmask_tab[9] );
    icxInitNormDiff_L1CnCMRTable( &normmask_tab[10] );
    icxInitNormDiff_L2CnCMRTable( &normmask_tab[11] );
}


CX_IMPL  double
cxNorm( const void* imgA, const void* imgB, int normType, const void* mask )
{
    static CxFuncTable norm_tab[12];
    static CxFuncTable normmask_tab[12];
    static int inittab = 0;

    double  norm = 0, norm_diff = 0;

    CX_FUNCNAME("cxNorm");

    __BEGIN__;

    int type, depth, cn, is_relative;
    CxSize size;
    CxMat stub1, *mat1 = (CxMat*)imgB;
    CxMat stub2, *mat2 = (CxMat*)imgA;
    int mat2_flag = CX_MAT_CONT_FLAG;
    int mat1_step, mat2_step, mask_step = 0;
    int coi = 0, coi2 = 0;

    if( !mat1 )
    {
        mat1 = mat2;
        mat2 = 0;
    }

    is_relative = mat2 && (normType & CX_RELATIVE);
    normType &= ~CX_RELATIVE;

    switch( normType )
    {
    case CX_C:
    case CX_L1:
    case CX_L2:
    case CX_DIFF_C:
    case CX_DIFF_L1:
    case CX_DIFF_L2:
        normType = (normType & 7) >> 1;
        break;
    default:
        CX_ERROR( CX_StsBadFlag, "" );
    }

    /* light variant */
    if( CX_IS_MAT(mat1) && (!mat2 || CX_IS_MAT(mat2)) && !mask )
    {
        if( mat2 )
        {
            if( !CX_ARE_TYPES_EQ( mat1, mat2 ))
                CX_ERROR( CX_StsUnmatchedFormats, "" );

            if( !CX_ARE_SIZES_EQ( mat1, mat2 ))
                CX_ERROR( CX_StsUnmatchedSizes, "" );

            mat2_flag = mat2->type;
        }

        size = cxGetMatSize( mat1 );
        type = CX_MAT_TYPE(mat1->type);
        depth = CX_MAT_DEPTH(type);
        cn = CX_MAT_CN(type);

        if( CX_IS_MAT_CONT( mat1->type & mat2_flag ))
        {
            size.width *= size.height;

            if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE && normType == 2 /* CX_L2 */ )
            {
                if( depth == CX_32F )
                {
                    const float* src1data = mat1->data.fl;
                    int size0 = size.width *= cn;
                
                    if( !mat2 || is_relative )
                    {
                        do
                        {
                            double t = src1data[size.width-1];
                            norm += t*t;
                        }
                        while( --size.width );
                    }

                    if( mat2 )
                    {
                        const float* src2data = mat2->data.fl;
                        size.width = size0;

                        do
                        {
                            double t = src1data[size.width-1] - src2data[size.width-1];
                            norm_diff += t*t;
                        }
                        while( --size.width );

                        if( is_relative )
                            norm = norm_diff/(norm + DBL_EPSILON);
                        else
                            norm = norm_diff;
                    }
                    norm = sqrt(norm);
                    EXIT;
                }

                if( depth == CX_64F )
                {
                    const double* src1data = mat1->data.db;
                    int size0 = size.width *= cn;

                    if( !mat2 || is_relative )
                    {
                        do
                        {
                            double t = src1data[size.width-1];
                            norm += t*t;
                        }
                        while( --size.width );
                    }

                    if( mat2 )
                    {
                        const double* src2data = mat2->data.db;
                        size.width = size0;

                        do
                        {
                            double t = src1data[size.width-1] - src2data[size.width-1];
                            norm_diff += t*t;
                        }
                        while( --size.width );

                        if( is_relative )
                            norm = norm_diff/(norm + DBL_EPSILON);
                        else
                            norm = norm_diff;
                    }
                    norm = sqrt(norm);
                    EXIT;
                }
            }
            size.height = 1;
            mat1_step = mat2_step = CX_STUB_STEP;
        }
        else
        {
            mat1_step = mat1->step;
            mat2_step = mat2 ? mat2->step : 0;
        }
    }
    else if( !CX_IS_MATND(mat1) && !CX_IS_MATND(mat2) )
    {
        CX_CALL( mat1 = cxGetMat( mat1, &stub1, &coi ));
        
        if( mat2 )
        {
            CX_CALL( mat2 = cxGetMat( mat2, &stub2, &coi2 ));

            if( !CX_ARE_TYPES_EQ( mat1, mat2 ))
                CX_ERROR( CX_StsUnmatchedFormats, "" );

            if( !CX_ARE_SIZES_EQ( mat1, mat2 ))
                CX_ERROR( CX_StsUnmatchedSizes, "" );

            if( coi != coi2 && CX_MAT_CN( mat1->type ) > 1 )
                CX_ERROR( CX_BadCOI, "" );

            mat2_flag = mat2->type;
        }

        size = cxGetMatSize( mat1 );
        type = CX_MAT_TYPE(mat1->type);
        depth = CX_MAT_DEPTH(type);
        cn = CX_MAT_CN(type);
        mat1_step = mat1->step;
        mat2_step = mat2 ? mat2->step : 0;

        if( !mask && CX_IS_MAT_CONT( mat1->type & mat2_flag ))
        {
            size.width *= size.height;
            size.height = 1;
            mat1_step = mat2_step = CX_STUB_STEP;
        }
    }
    else
    {
        CxArr* arrs[] = { mat1, mat2 };
        CxMatND stubs[2];
        CxNArrayIterator iterator;
        int pass_hint;

        if( !inittab )
        {
            icxInitNormTabs( norm_tab, normmask_tab );
            inittab = 1;
        }

        if( mask )
            CX_ERROR( CX_StsBadMask,
            "This operation on multi-dimensional arrays does not support mask" );

        CX_CALL( cxInitNArrayIterator( 1 + (mat2 != 0), arrs, 0, stubs, &iterator ));

        type = CX_MAT_TYPE(iterator.hdr[0]->type);
        depth = CX_MAT_DEPTH(type);
        iterator.size.width *= CX_MAT_CN(type);

        pass_hint = normType != 0 && (depth == CX_32F); 

        if( !mat2 || is_relative )
        {
            if( !pass_hint )
            {
                CxFunc2D_1A1P func;
        
                CX_GET_FUNC_PTR( func, (CxFunc2D_1A1P)norm_tab[normType].fn_2d[depth]);

                do
                {
                    double temp = 0;
                    IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                                     iterator.size, &temp ));
                    norm += temp;
                }
                while( cxNextNArraySlice( &iterator ));
            }
            else
            {
                CxFunc2D_1A1P1I func;

                CX_GET_FUNC_PTR( func, (CxFunc2D_1A1P1I)norm_tab[normType].fn_2d[depth]);

                do
                {
                    double temp = 0;
                    IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                                     iterator.size, &temp, cxAlgHintAccurate ));
                    norm += temp;
                }
                while( cxNextNArraySlice( &iterator ));
            }
        }

        if( mat2 )
        {
            if( !pass_hint )
            {
                CxFunc2D_2A1P func;
                CX_GET_FUNC_PTR( func, (CxFunc2D_2A1P)norm_tab[3 + normType].fn_2d[depth]);

                do
                {
                    double temp = 0;
                    IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                                     iterator.ptr[1], CX_STUB_STEP,
                                     iterator.size, &temp ));
                    norm_diff += temp;
                }
                while( cxNextNArraySlice( &iterator ));
            }
            else
            {
                CxFunc2D_2A1P1I func;
                CX_GET_FUNC_PTR( func, (CxFunc2D_2A1P1I)norm_tab[3 + normType].fn_2d[depth]);

                do
                {
                    double temp = 0;
                    IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                                     iterator.ptr[1], CX_STUB_STEP,
                                     iterator.size, &temp, cxAlgHintAccurate ));
                    norm_diff += temp;
                }
                while( cxNextNArraySlice( &iterator ));
            }

            if( is_relative )
                norm = norm_diff/(norm + DBL_EPSILON);
            else
                norm = norm_diff;
        }
        EXIT;
    }

    if( !inittab )
    {
        icxInitNormTabs( norm_tab, normmask_tab );
        inittab = 1;
    }

    if( !mask )
    {
        if( cn == 1 || coi == 0 )
        {
            int pass_hint = depth == CX_32F && normType != 0;
            size.width *= cn;

            if( !mat2 || is_relative )
            {
                if( !pass_hint )
                {
                    CxFunc2D_1A1P func;
                    CX_GET_FUNC_PTR( func, (CxFunc2D_1A1P)norm_tab[normType].fn_2d[depth]);

                    IPPI_CALL( func( mat1->data.ptr, mat1_step, size, &norm ));
                }
                else
                {
                    CxFunc2D_1A1P1I func;
                    CX_GET_FUNC_PTR( func, (CxFunc2D_1A1P1I)norm_tab[normType].fn_2d[depth]);

                    IPPI_CALL( func( mat1->data.ptr, mat1_step, size, &norm, cxAlgHintAccurate ));
                }
            }
        
            if( mat2 )
            {
                if( !pass_hint )
                {
                    CxFunc2D_2A1P func;
                    CX_GET_FUNC_PTR( func, (CxFunc2D_2A1P)norm_tab[3 + normType].fn_2d[depth]);

                    IPPI_CALL( func( mat1->data.ptr, mat1_step, mat2->data.ptr, mat2_step,
                                     size, &norm_diff ));
                }
                else
                {
                    CxFunc2D_2A1P1I func;
                    CX_GET_FUNC_PTR( func, (CxFunc2D_2A1P1I)norm_tab[3 + normType].fn_2d[depth]);

                    IPPI_CALL( func( mat1->data.ptr, mat1_step, mat2->data.ptr, mat2_step,
                                     size, &norm_diff, cxAlgHintAccurate ));
                }

                if( is_relative )
                    norm = norm_diff/(norm + DBL_EPSILON);
                else
                    norm = norm_diff;
            }
        }
        else
        {
            if( !mat2 || is_relative )
            {
                CxFunc2DnC_1A1P func;
                CX_GET_FUNC_PTR( func, (CxFunc2DnC_1A1P)norm_tab[6 + normType].fn_2d[depth]);

                IPPI_CALL( func( mat1->data.ptr, mat1_step, size, cn, coi, &norm ));
            }
        
            if( mat2 )
            {
                CxFunc2DnC_2A1P func;
                CX_GET_FUNC_PTR( func, (CxFunc2DnC_2A1P)norm_tab[9 + normType].fn_2d[depth]);

                IPPI_CALL( func( mat1->data.ptr, mat1_step, mat2->data.ptr, mat2_step,
                                 size, cn, coi, &norm_diff ));

                if( is_relative )
                    norm = norm_diff/(norm + DBL_EPSILON);
                else
                    norm = norm_diff;
            }
        }
    }
    else
    {
        CxMat maskstub, *matmask = (CxMat*)mask;

        if( CX_MAT_CN(type) > 1 && coi == 0 )
            CX_ERROR( CX_StsBadArg, "" );

        CX_CALL( matmask = cxGetMat( matmask, &maskstub ));

        if( !CX_IS_MASK_ARR( matmask ))
            CX_ERROR( CX_StsBadMask, "" );

        if( !CX_ARE_SIZES_EQ( mat1, matmask ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );
        
        mask_step = matmask->step;

        if( CX_IS_MAT_CONT( mat1->type & mat2_flag & matmask->type ))
        {
            size.width *= size.height;
            size.height = 1;
            mat1_step = mat2_step = mask_step = CX_STUB_STEP;
        }

        if( CX_MAT_CN(type) == 1 || coi == 0 )
        {
            if( !mat2 || is_relative )
            {
                CxFunc2D_2A1P func;
                CX_GET_FUNC_PTR( func,
                    (CxFunc2D_2A1P)normmask_tab[normType].fn_2d[depth]);

                IPPI_CALL( func( mat1->data.ptr, mat1_step,
                                 matmask->data.ptr, mask_step, size, &norm ));
            }
        
            if( mat2 )
            {
                CxFunc2D_3A1P func;
                CX_GET_FUNC_PTR( func,
                    (CxFunc2D_3A1P)normmask_tab[3 + normType].fn_2d[depth]);

                IPPI_CALL( func( mat1->data.ptr, mat1_step, mat2->data.ptr, mat2_step,
                                 matmask->data.ptr, mask_step, size, &norm_diff ));

                if( is_relative )
                    norm = norm_diff/(norm + DBL_EPSILON);
                else
                    norm = norm_diff;
            }
        }
        else
        {
            if( !mat2 || is_relative )
            {
                CxFunc2DnC_2A1P func;
                CX_GET_FUNC_PTR( func,
                    (CxFunc2DnC_2A1P)normmask_tab[6 + normType].fn_2d[depth]);

                IPPI_CALL( func( mat1->data.ptr, mat1_step,
                                 matmask->data.ptr, mask_step,
                                 size, cn, coi, &norm ));
            }
        
            if( mat2 )
            {
                CxFunc2DnC_3A1P func;
                CX_GET_FUNC_PTR( func,
                    (CxFunc2DnC_3A1P)normmask_tab[9 + normType].fn_2d[depth]);

                IPPI_CALL( func( mat1->data.ptr, mat1_step,
                                 mat2->data.ptr, mat2_step,
                                 matmask->data.ptr, mask_step,
                                 size, cn, coi, &norm_diff ));

                if( is_relative )
                    norm = norm_diff/(norm + DBL_EPSILON);
                else
                    norm = norm_diff;
            }
        }
    }

    __END__;

    return norm;
}

/* End of file. */
