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
//  CxMat arithmetic operations: +, - ...
//
// */

#include "_cxcore.h"

static const uchar icxSaturate8u[] = 
{
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
     96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255
};

#define CX_FAST_CAST_8U(t)   (assert(-256 <= (t) || (t) <= 512), icxSaturate8u[t+256])

/****************************************************************************************\
*                      Arithmetic operations (+, -) without mask                         *
\****************************************************************************************/

#define ICX_DEF_BIN_ARI_OP_CASE( __op__, worktype, cast_macro, len )\
{                                                                   \
    int i;                                                          \
                                                                    \
    for( i = 0; i <= (len) - 4; i += 4 )                            \
    {                                                               \
        worktype t0 = __op__((src1)[i], (src2)[i]);                 \
        worktype t1 = __op__((src1)[i+1], (src2)[i+1]);             \
                                                                    \
        (dst)[i] = cast_macro( t0 );                                \
        (dst)[i+1] = cast_macro( t1 );                              \
                                                                    \
        t0 = __op__((src1)[i+2],(src2)[i+2]);                       \
        t1 = __op__((src1)[i+3],(src2)[i+3]);                       \
                                                                    \
        (dst)[i+2] = cast_macro( t0 );                              \
        (dst)[i+3] = cast_macro( t1 );                              \
    }                                                               \
                                                                    \
    for( ; i < (len); i++ )                                         \
    {                                                               \
        worktype t0 = __op__((src1)[i],(src2)[i]);                  \
        (dst)[i] = cast_macro( t0 );                                \
    }                                                               \
}

#define ICX_DEF_BIN_ARI_OP_2D( __op__, name, type, worktype, cast_macro )   \
IPCXAPI_IMPL( CxStatus, name, ( const type* src1, int step1,                \
                                const type* src2, int step2,                \
                                type* dst, int step, CxSize size ))         \
{                                                                           \
    if( size.width == 1 )                                                   \
    {                                                                       \
        for( ; size.height--; (char*&)src1 += step1,                        \
                              (char*&)src2 += step2,                        \
                              (char*&)dst += step )                         \
        {                                                                   \
            worktype t0 = __op__((src1)[0],(src2)[0]);                      \
            (dst)[0] = cast_macro( t0 );                                    \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        for( ; size.height--; (char*&)src1 += step1,                        \
                              (char*&)src2 += step2,                        \
                              (char*&)dst += step )                         \
        {                                                                   \
            ICX_DEF_BIN_ARI_OP_CASE( __op__, worktype,                      \
                                     cast_macro, size.width );              \
        }                                                                   \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}


#define ICX_DEF_UN_ARI_OP_CASE( __op__, worktype, cast_macro,               \
                                src, scalar, dst, len )                     \
{                                                                           \
    int i;                                                                  \
                                                                            \
    for( ; ((len) -= 12) >= 0; (dst) += 12, (src) += 12 )                   \
    {                                                                       \
        worktype t0 = __op__((scalar)[0], (src)[0]);                        \
        worktype t1 = __op__((scalar)[1], (src)[1]);                        \
                                                                            \
        (dst)[0] = cast_macro( t0 );                                        \
        (dst)[1] = cast_macro( t1 );                                        \
                                                                            \
        t0 = __op__((scalar)[2], (src)[2]);                                 \
        t1 = __op__((scalar)[3], (src)[3]);                                 \
                                                                            \
        (dst)[2] = cast_macro( t0 );                                        \
        (dst)[3] = cast_macro( t1 );                                        \
                                                                            \
        t0 = __op__((scalar)[4], (src)[4]);                                 \
        t1 = __op__((scalar)[5], (src)[5]);                                 \
                                                                            \
        (dst)[4] = cast_macro( t0 );                                        \
        (dst)[5] = cast_macro( t1 );                                        \
                                                                            \
        t0 = __op__((scalar)[6], (src)[6]);                                 \
        t1 = __op__((scalar)[7], (src)[7]);                                 \
                                                                            \
        (dst)[6] = cast_macro( t0 );                                        \
        (dst)[7] = cast_macro( t1 );                                        \
                                                                            \
        t0 = __op__((scalar)[8], (src)[8]);                                 \
        t1 = __op__((scalar)[9], (src)[9]);                                 \
                                                                            \
        (dst)[8] = cast_macro( t0 );                                        \
        (dst)[9] = cast_macro( t1 );                                        \
                                                                            \
        t0 = __op__((scalar)[10], (src)[10]);                               \
        t1 = __op__((scalar)[11], (src)[11]);                               \
                                                                            \
        (dst)[10] = cast_macro( t0 );                                       \
        (dst)[11] = cast_macro( t1 );                                       \
    }                                                                       \
                                                                            \
    for( (len) += 12, i = 0; i < (len); i++ )                               \
    {                                                                       \
        worktype t0 = __op__((scalar)[i],(src)[i]);                         \
        (dst)[i] = cast_macro( t0 );                                        \
    }                                                                       \
}


#define ICX_DEF_UN_ARI_OP_2D( __op__, name, type, worktype, cast_macro )    \
IPCXAPI_IMPL( CxStatus,                                                     \
name,( const type* src, int step1, type* dst, int step,                     \
      CxSize size, const worktype* scalar ))                                \
{                                                                           \
    if( size.width == 1 )                                                   \
    {                                                                       \
        for( ; size.height--; (char*&)src += step1,                         \
                              (char*&)dst += step )                         \
        {                                                                   \
            worktype t0 = __op__(*(scalar),*(src));                         \
            *(dst) = cast_macro( t0 );                                      \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        for( ; size.height--; (char*&)src += step1,                         \
                              (char*&)dst += step )                         \
        {                                                                   \
            const type *tsrc = src;                                         \
            type *tdst = dst;                                               \
            int width = size.width;                                         \
                                                                            \
            ICX_DEF_UN_ARI_OP_CASE( __op__, worktype, cast_macro,           \
                                    tsrc, scalar, tdst, width );            \
        }                                                                   \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}


#define ICX_DEF_BIN_ARI_ALL( __op__, name, cast_8u )                                \
ICX_DEF_BIN_ARI_OP_2D( __op__, icx##name##_8u_C1R, uchar, int, cast_8u )            \
ICX_DEF_BIN_ARI_OP_2D( __op__, icx##name##_16s_C1R, short, int, CX_CAST_16S )       \
ICX_DEF_BIN_ARI_OP_2D( __op__, icx##name##_32s_C1R, int, int, CX_CAST_32S )         \
ICX_DEF_BIN_ARI_OP_2D( __op__, icx##name##_32f_C1R, float, float, CX_CAST_32F )     \
ICX_DEF_BIN_ARI_OP_2D( __op__, icx##name##_64f_C1R, double, double, CX_CAST_64F )

#define ICX_DEF_UN_ARI_ALL( __op__, name )                                          \
ICX_DEF_UN_ARI_OP_2D( __op__, icx##name##_8u_C1R, uchar, int, CX_CAST_8U )          \
ICX_DEF_UN_ARI_OP_2D( __op__, icx##name##_16s_C1R, short, int, CX_CAST_16S )        \
ICX_DEF_UN_ARI_OP_2D( __op__, icx##name##_32s_C1R, int, int, CX_CAST_32S )          \
ICX_DEF_UN_ARI_OP_2D( __op__, icx##name##_32f_C1R, float, float, CX_CAST_32F )      \
ICX_DEF_UN_ARI_OP_2D( __op__, icx##name##_64f_C1R, double, double, CX_CAST_64F )

ICX_DEF_BIN_ARI_ALL( CX_ADD, Add, CX_FAST_CAST_8U )
ICX_DEF_BIN_ARI_ALL( CX_SUB, Sub, CX_FAST_CAST_8U )

ICX_DEF_UN_ARI_ALL( CX_ADD, AddC )
ICX_DEF_UN_ARI_ALL( CX_SUB, SubRC )

#define ICX_DEF_INIT_ARITHM_FUNC_TAB( FUNCNAME, FLAG )          \
static  void  icxInit##FUNCNAME##FLAG##Table( CxFuncTable* tab )\
{                                                               \
    tab->fn_2d[CX_8U] = (void*)icx##FUNCNAME##_8u_##FLAG;       \
    tab->fn_2d[CX_8S] = 0;                                      \
    tab->fn_2d[CX_16S] = (void*)icx##FUNCNAME##_16s_##FLAG;     \
    tab->fn_2d[CX_32S] = (void*)icx##FUNCNAME##_32s_##FLAG;     \
    tab->fn_2d[CX_32F] = (void*)icx##FUNCNAME##_32f_##FLAG;     \
    tab->fn_2d[CX_64F] = (void*)icx##FUNCNAME##_64f_##FLAG;     \
}

ICX_DEF_INIT_ARITHM_FUNC_TAB( Sub, C1R );
ICX_DEF_INIT_ARITHM_FUNC_TAB( SubRC, C1R );
ICX_DEF_INIT_ARITHM_FUNC_TAB( Add, C1R );
ICX_DEF_INIT_ARITHM_FUNC_TAB( AddC, C1R );

/****************************************************************************************\
*                      Arithmetic operations (+, -, *) with mask                         *
\****************************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////


#define ICX_INT_MASK(m)  (((m) == 0) - 1)
#define ICX_FLT_MASK(m)  maskTab[(m) != 0]

/////////////////////////// Binary operations ////////////////////////////////

#define ICX_DEF_BIN_ARI_OP_MASK( __op__, _mask_op_, _process_mask_,             \
                             cast_macro, worktype,                              \
                             src, dst, mask, len, cn )                          \
{                                                                               \
    int i;                                                                      \
    for( i = 0; i <= (len) - 4; i += 4, src += 4*(cn), dst += 4*(cn) )          \
    {                                                                           \
        int k = cn - 1;                                                         \
        int m = 0;                                                              \
                                                                                \
        do                                                                      \
        {                                                                       \
            worktype t0 = _process_mask_((mask)[i]);                            \
            worktype t1 = _process_mask_((mask)[i+1]);                          \
                                                                                \
            t0 = _mask_op_( t0, (src)[k]);                                      \
            t1 = _mask_op_( t1, (src)[k+(cn)]);                                 \
                                                                                \
            t0 = __op__( (dst)[k], t0 );                                        \
            t1 = __op__( (dst)[k+(cn)], t1 );                                   \
                                                                                \
            (dst)[k] = cast_macro( t0 );                                        \
            (dst)[k+(cn)] = cast_macro( t1 );                                   \
                                                                                \
            t0 = _process_mask_((mask)[i+2]);                                   \
            t1 = _process_mask_((mask)[i+3]);                                   \
                                                                                \
            t0 = _mask_op_( t0, (src)[k+2*(cn)]);                               \
            t1 = _mask_op_( t1, (src)[k+3*(cn)]);                               \
                                                                                \
            t0 = __op__( (dst)[k+2*(cn)], t0 );                                 \
            t1 = __op__( (dst)[k+3*(cn)], t1 );                                 \
                                                                                \
            (dst)[k+2*(cn)] = cast_macro( t0 );                                 \
            (dst)[k+3*(cn)] = cast_macro( t1 );                                 \
        }                                                                       \
        while( k-- && (m || (m = (mask[i]|mask[i+1]|mask[i+2]|mask[i+3])) != 0));\
    }                                                                           \
                                                                                \
    for( ; i < (len); i++, src += cn, dst += cn )                               \
    {                                                                           \
        int k = cn - 1;                                                         \
        do                                                                      \
        {                                                                       \
            worktype t = _process_mask_((mask)[i]);                             \
            t = _mask_op_( t, (src)[k] );                                       \
            t = __op__( (dst)[k], t );                                          \
            (dst)[k] = cast_macro( t );                                         \
        }                                                                       \
        while( k-- && mask[i] != 0 );                                           \
    }                                                                           \
}


// dst = src - dst
#define ICX_DEF_BIN_ARI_OP_REV_MASK( __op__, _mask_op_, _process_mask_,         \
                                 cast_macro,worktype,                           \
                                 src, dst, mask, len, cn )                      \
{                                                                               \
    int i;                                                                      \
    for( i = 0; i <= (len) - 4; i += 4, src += 4*cn, dst += 4*cn )              \
    {                                                                           \
        int k = cn - 1;                                                         \
        int m = 0;                                                              \
                                                                                \
        do                                                                      \
        {                                                                       \
            worktype t0 = _process_mask_( (mask)[i] );                          \
            worktype t1 = _process_mask_( (mask)[i+1] );                        \
            worktype t2 = (dst)[k];                                             \
            worktype t3 = (dst)[k+(cn)];                                        \
                                                                                \
            t2 += _mask_op_( t0, (src)[k] - t2 - t2 );                          \
            t3 += _mask_op_( t1, (src)[k+(cn)] - t3 - t3 );                     \
                                                                                \
            (dst)[k] = cast_macro(t2);                                          \
            (dst)[k+(cn)] = cast_macro(t3);                                     \
                                                                                \
            t0 = _process_mask_( (mask)[i+2] );                                 \
            t1 = _process_mask_( (mask)[i+3] );                                 \
            t2 = (dst)[k+(cn)*2];                                               \
            t3 = (dst)[k+(cn)*3];                                               \
                                                                                \
            t2 += _mask_op_( t0, (src)[k+(cn)*2] - t2 - t2 );                   \
            t3 += _mask_op_( t1, (src)[k+(cn)*3] - t3 - t3 );                   \
                                                                                \
            (dst)[k+(cn)*2] = cast_macro(t2);                                   \
            (dst)[k+(cn)*3] = cast_macro(t3);                                   \
        }                                                                       \
        while( k-- && (m || (m = (mask[i]|mask[i+1]|mask[i+2]|mask[i+3])) != 0));\
    }                                                                           \
                                                                                \
    for( ; i < (len); i++, src += cn, dst += cn )                               \
    {                                                                           \
        int k = cn - 1;                                                         \
        do                                                                      \
        {                                                                       \
            worktype t0 = _process_mask_( (mask)[i] );                          \
            worktype t1 = (dst)[k];                                             \
                                                                                \
            t1 += _mask_op_( t0, (src)[k] - t1 - t1 );                          \
            (dst)[k] = cast_macro(t1);                                          \
        }                                                                       \
        while( k-- && mask[i] != 0 );                                           \
    }                                                                           \
}


#define ICX_DEF_BIN_ARI_OP_MASK_2D( __op__, name, _mask_op_, _process_mask_,\
                                entry, arrtype, worktype, cast_macro, flag )\
                                                                            \
static CxStatus CX_STDCALL                                                  \
name( const arrtype* src, int srcstep, const uchar* mask, int maskstep,     \
      arrtype* dst, int dststep, CxSize size, int cn )                      \
{                                                                           \
    entry;                                                                  \
                                                                            \
    for( ; size.height--; (char*&)src += srcstep,                           \
                          (char*&)dst += dststep,                           \
                          mask += maskstep )                                \
    {                                                                       \
        const arrtype* tsrc = src;                                          \
        arrtype* tdst = dst;                                                \
                                                                            \
        ICX_DEF_BIN_ARI_OP_##flag( __op__, _mask_op_, _process_mask_,       \
                 cast_macro, worktype, tsrc, tdst, mask, size.width, cn );  \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}


/////////////////////////// Unary operations ////////////////////////////////


#define ICX_DEF_UN_ARI_OP_MASK( __op__, _mask_op_, _process_mask_,          \
                                cast_macro, worktype,                       \
                                dst, mask, len, cn )                        \
{                                                                           \
    int i;                                                                  \
    for( i = 0; i <= (len) - 4; i += 4, dst += 4*(cn) )                     \
    {                                                                       \
        int k = cn - 1;                                                     \
        int m = 0;                                                          \
                                                                            \
        do                                                                  \
        {                                                                   \
            worktype value = scalar[k];                                     \
            worktype t0 = _process_mask_((mask)[i]);                        \
            worktype t1 = _process_mask_((mask)[i+1]);                      \
                                                                            \
            t0 = _mask_op_( t0, value );                                    \
            t1 = _mask_op_( t1, value );                                    \
                                                                            \
            t0 = __op__( (dst)[k], t0 );                                    \
            t1 = __op__( (dst)[k+(cn)], t1 );                               \
                                                                            \
            (dst)[k] = cast_macro( t0 );                                    \
            (dst)[k+(cn)] = cast_macro( t1 );                               \
                                                                            \
            t0 = _process_mask_((mask)[i+2]);                               \
            t1 = _process_mask_((mask)[i+3]);                               \
                                                                            \
            t0 = _mask_op_( t0, value );                                    \
            t1 = _mask_op_( t1, value );                                    \
                                                                            \
            t0 = __op__( (dst)[k+2*(cn)], t0 );                             \
            t1 = __op__( (dst)[k+3*(cn)], t1 );                             \
                                                                            \
            (dst)[k+2*(cn)] = cast_macro( t0 );                             \
            (dst)[k+3*(cn)] = cast_macro( t1 );                             \
        }                                                                   \
        while( k-- && (m || (m = (mask[i]|mask[i+1]|mask[i+2]|mask[i+3])) != 0));\
    }                                                                       \
                                                                            \
    for( ; i < (len); i++, dst += cn )                                      \
    {                                                                       \
        int k = cn - 1;                                                     \
        do                                                                  \
        {                                                                   \
            worktype t = _process_mask_((mask)[i]);                         \
            t = _mask_op_( t, scalar[k] );                                  \
            t = __op__( (dst)[k], t );                                      \
            (dst)[k] = cast_macro( t );                                     \
        }                                                                   \
        while( k-- && mask[i] != 0 );                                       \
    }                                                                       \
}


// dst = src - dst
#define ICX_DEF_UN_ARI_OP_REV_MASK( __op__, _mask_op_, _process_mask_,      \
                                    cast_macro,worktype,                    \
                                    dst, mask, len, cn )                    \
{                                                                           \
    int i;                                                                  \
    for( i = 0; i <= (len) - 4; i += 4, dst += 4*cn )                       \
    {                                                                       \
        int k = cn - 1;                                                     \
        int m = 0;                                                          \
                                                                            \
        do                                                                  \
        {                                                                   \
            worktype value = scalar[k];                                     \
            worktype t0 = _process_mask_( (mask)[i] );                      \
            worktype t1 = _process_mask_( (mask)[i+1] );                    \
            worktype t2 = (dst)[k];                                         \
            worktype t3 = (dst)[k+(cn)];                                    \
                                                                            \
            t2 += _mask_op_( t0, value - t2 - t2 );                         \
            t3 += _mask_op_( t1, value - t3 - t3 );                         \
                                                                            \
            (dst)[k] = cast_macro(t2);                                      \
            (dst)[k+(cn)] = cast_macro(t3);                                 \
                                                                            \
            t0 = _process_mask_( (mask)[i+2] );                             \
            t1 = _process_mask_( (mask)[i+3] );                             \
            t2 = (dst)[k+(cn)*2];                                           \
            t3 = (dst)[k+(cn)*3];                                           \
                                                                            \
            t2 += _mask_op_( t0, value - t2 - t2 );                         \
            t3 += _mask_op_( t1, value - t3 - t3 );                         \
                                                                            \
            (dst)[k+(cn)*2] = cast_macro(t2);                               \
            (dst)[k+(cn)*3] = cast_macro(t3);                               \
        }                                                                   \
        while( k-- && (m || (m = (mask[i]|mask[i+1]|mask[i+2]|mask[i+3])) != 0));\
    }                                                                       \
                                                                            \
    for( ; i < (len); i++, dst += cn )                                      \
    {                                                                       \
        int k = cn - 1;                                                     \
        do                                                                  \
        {                                                                   \
            worktype t0 = _process_mask_( (mask)[i] );                      \
            worktype t1 = (dst)[k];                                         \
                                                                            \
            t1 += _mask_op_( t0, scalar[k] - t1 - t1 );                     \
            (dst)[k] = cast_macro(t1);                                      \
        }                                                                   \
        while( k-- && mask[i] != 0 );                                       \
    }                                                                       \
}

#define ICX_DEF_UN_ARI_OP_MASK_2D( __op__, name, _mask_op_, _process_mask_, \
                                entry, arrtype, worktype, cast_macro, flag )\
                                                                            \
static CxStatus CX_STDCALL                                                  \
name( arrtype* dst, int dststep, const uchar* mask, int maskstep,           \
      CxSize size, const worktype* scalar, int cn )                         \
{                                                                           \
    entry;                                                                  \
                                                                            \
    for( ; size.height--; (char*&)dst += dststep,                           \
                          mask += maskstep )                                \
    {                                                                       \
        arrtype* tdst = dst;                                                \
                                                                            \
        ICX_DEF_UN_ARI_OP_##flag( __op__, _mask_op_, _process_mask_,        \
                       cast_macro, worktype, tdst, mask, size.width, cn );  \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}


#define  ICX_STUB_ENTRY      dst = dst
#define  ICX_BIN_ENTRY_FLT   CX_DEFINE_MASK


#define ICX_DEF_ALL_BIN_MASK( __op__, name, flag )                                  \
ICX_DEF_BIN_ARI_OP_MASK_2D( __op__, icx##name##_8u_CnMR, CX_AND, ICX_INT_MASK,      \
                            ICX_STUB_ENTRY, uchar, int, CX_FAST_CAST_8U, flag )     \
ICX_DEF_BIN_ARI_OP_MASK_2D( __op__, icx##name##_16s_CnMR, CX_AND, ICX_INT_MASK,     \
                            ICX_STUB_ENTRY, short, int, CX_CAST_16S, flag )         \
ICX_DEF_BIN_ARI_OP_MASK_2D( __op__, icx##name##_32s_CnMR, CX_AND, ICX_INT_MASK,     \
                            ICX_STUB_ENTRY, int, int, CX_CAST_32S, flag )           \
ICX_DEF_BIN_ARI_OP_MASK_2D( __op__, icx##name##_32f_CnMR, CX_MUL, ICX_FLT_MASK,     \
                            ICX_BIN_ENTRY_FLT, float, float, CX_CAST_32F, flag )    \
ICX_DEF_BIN_ARI_OP_MASK_2D( __op__, icx##name##_64f_CnMR, CX_MUL, ICX_FLT_MASK,     \
                            ICX_BIN_ENTRY_FLT, double, double, CX_CAST_64F, flag )

#define ICX_DEF_ALL_UN_MASK( __op__, name, flag )                                   \
ICX_DEF_UN_ARI_OP_MASK_2D( __op__, icx##name##_8u_CnMR, CX_AND, ICX_INT_MASK,       \
                            ICX_STUB_ENTRY, uchar, int, CX_CAST_8U, flag )          \
ICX_DEF_UN_ARI_OP_MASK_2D( __op__, icx##name##_16s_CnMR, CX_AND, ICX_INT_MASK,      \
                            ICX_STUB_ENTRY, short, int, CX_CAST_16S, flag )         \
ICX_DEF_UN_ARI_OP_MASK_2D( __op__, icx##name##_32s_CnMR, CX_AND, ICX_INT_MASK,      \
                            ICX_STUB_ENTRY, int, int, CX_CAST_32S, flag )           \
ICX_DEF_UN_ARI_OP_MASK_2D( __op__, icx##name##_32f_CnMR, CX_MUL, ICX_FLT_MASK,      \
                            CX_DEFINE_MASK, float, float, CX_CAST_32F, flag )       \
ICX_DEF_UN_ARI_OP_MASK_2D( __op__, icx##name##_64f_CnMR, CX_MUL, ICX_FLT_MASK,      \
                            CX_DEFINE_MASK, double, double, CX_CAST_64F, flag )


ICX_DEF_ALL_BIN_MASK( CX_ADD, Add, MASK )
ICX_DEF_ALL_BIN_MASK( CX_SUB, Sub, MASK )
ICX_DEF_ALL_BIN_MASK( CX_SUB, SubR, REV_MASK )

ICX_DEF_ALL_UN_MASK( CX_ADD, AddC, MASK )
ICX_DEF_ALL_UN_MASK( CX_SUB, SubRC, REV_MASK )


#define ICX_DEF_INIT_ARITHM_MASK_FUNC_TAB( FUNCNAME, FLAG )     \
static  void  icxInit##FUNCNAME##FLAG##Table( CxFuncTable* tab )\
{                                                               \
    tab->fn_2d[CX_8U] = (void*)icx##FUNCNAME##_8u_##FLAG;       \
    tab->fn_2d[CX_8S] = 0;                                      \
    tab->fn_2d[CX_16S] = (void*)icx##FUNCNAME##_16s_##FLAG;     \
    tab->fn_2d[CX_32S] = (void*)icx##FUNCNAME##_32s_##FLAG;     \
    tab->fn_2d[CX_32F] = (void*)icx##FUNCNAME##_32f_##FLAG;     \
    tab->fn_2d[CX_64F] = (void*)icx##FUNCNAME##_64f_##FLAG;     \
}


ICX_DEF_INIT_ARITHM_MASK_FUNC_TAB( Add, CnMR )
ICX_DEF_INIT_ARITHM_MASK_FUNC_TAB( AddC, CnMR )
ICX_DEF_INIT_ARITHM_MASK_FUNC_TAB( Sub, CnMR )
ICX_DEF_INIT_ARITHM_MASK_FUNC_TAB( SubR, CnMR )
ICX_DEF_INIT_ARITHM_MASK_FUNC_TAB( SubRC, CnMR )

/****************************************************************************************\
*                       External Functions for Arithmetic Operations                     *
\****************************************************************************************/


/*************************************** S U B ******************************************/

CX_IMPL void
cxSub( const void* srcarr1, const void* srcarr2,
       void* dstarr, const void* maskarr )
{
    static CxFuncTable submask_tab[2];
    static CxFuncTable sub_tab;
    static int inittab = 0, initmasktab = 0;

    CX_FUNCNAME( "cxSub" );

    __BEGIN__;

    int type;
    int src1_step, src2_step, dst_step;
    CxMat srcstub1, *src1 = (CxMat*)srcarr1;
    CxMat srcstub2, *src2 = (CxMat*)srcarr2;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxSize size;

    if( !CX_IS_MAT(src1) || !CX_IS_MAT(src2) || !CX_IS_MAT(dst))
    {
        if( CX_IS_MATND(src1) || CX_IS_MATND(src2) || CX_IS_MATND(dst))
        {
            CxArr* arrs[] = { src1, src2, dst };
            CxMatND stubs[3];
            CxNArrayIterator iterator;
            CxFunc2D_3A func;

            if( maskarr )
                CX_ERROR( CX_StsBadMask,
                "This operation on multi-dimensional arrays does not support mask" );

            CX_CALL( cxInitNArrayIterator( 3, arrs, 0, stubs, &iterator ));

            type = iterator.hdr[0]->type;
            iterator.size.width *= CX_MAT_CN(type);

            if( !inittab )
            {
                icxInitSubC1RTable( &sub_tab );
                inittab = 1;
            }

            func = (CxFunc2D_3A)(sub_tab.fn_2d[CX_MAT_DEPTH(type)]);
            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            do
            {
                IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                                 iterator.ptr[1], CX_STUB_STEP,
                                 iterator.ptr[2], CX_STUB_STEP,
                                 iterator.size ));
            }
            while( cxNextNArraySlice( &iterator ));
            EXIT;
        }
        else
        {
            int coi1 = 0, coi2 = 0, coi3 = 0;
        
            CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi1 ));
            CX_CALL( src2 = cxGetMat( src2, &srcstub2, &coi2 ));
            CX_CALL( dst = cxGetMat( dst, &dststub, &coi3 ));
            if( coi1 + coi2 + coi3 != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    if( !CX_ARE_TYPES_EQ( src1, src2 ) || !CX_ARE_TYPES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src1, src2 ) || !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    type = CX_MAT_TYPE(src1->type);
    size = cxGetMatSize( src1 );

    if( !maskarr )
    {
        int depth = CX_MAT_DEPTH(type);
        size.width *= CX_MAT_CN( type );

        if( CX_IS_MAT_CONT( src1->type & src2->type & dst->type ))
        {
            size.width *= size.height;
            if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE*
                              CX_MAX_INLINE_MAT_OP_SIZE )
            {
                if( depth == CX_32F )
                {
                    const float* src1data = (const float*)(src1->data.ptr);
                    const float* src2data = (const float*)(src2->data.ptr);
                    float* dstdata = (float*)(dst->data.ptr);
                
                    do
                    {
                        dstdata[size.width-1] = (float)
                            (src1data[size.width-1] - src2data[size.width-1]);
                    }
                    while( --size.width );

                    EXIT;
                }

                if( depth == CX_64F )
                {
                    const double* src1data = (const double*)(src1->data.ptr);
                    const double* src2data = (const double*)(src2->data.ptr);
                    double* dstdata = (double*)(dst->data.ptr);
                
                    do
                    {
                        dstdata[size.width-1] =
                            src1data[size.width-1] - src2data[size.width-1];
                    }
                    while( --size.width );

                    EXIT;
                }
            }

            src1_step = src2_step = dst_step = CX_STUB_STEP;
            size.height = 1;
        }
        else
        {
            src1_step = src1->step;
            src2_step = src2->step;
            dst_step = dst->step;
        }

        if( !inittab )
        {
            icxInitSubC1RTable( &sub_tab );
            inittab = 1;
        }

        {
            CxFunc2D_3A func = (CxFunc2D_3A)(sub_tab.fn_2d[depth]);

            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                             dst->data.ptr, dst_step, size ));
        }
    }
    else
    {
        CxMat maskstub, *mask = (CxMat*)maskarr;
        CxArithmBinMaskFunc2D func;
        int inv = 0;
        int mask_step;

        if( !CX_IS_MAT(mask) )
            CX_CALL( mask = cxGetMat( mask, &maskstub ));

        if( !CX_IS_MASK_ARR(mask))
            CX_ERROR( CX_StsBadMask, "" );

        if( !CX_ARE_SIZES_EQ( mask, dst ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );

        if( dst->data.ptr == src1->data.ptr )
            ;
        else if( dst->data.ptr == src2->data.ptr )
        {
            inv = 1;
            src2 = src1;
        }
        else
        {
            CX_CALL( cxCopy( src1, dst, mask ));
        }

        if( CX_IS_MAT_CONT( src2->type & dst->type & mask->type ))
        {
            size.width *= size.height;
            src2_step = dst_step = mask_step = CX_STUB_STEP;
            size.height = 1;
        }
        else
        {
            src2_step = src2->step;
            dst_step = dst->step;
            mask_step = mask->step;
        }

        if( !initmasktab )
        {
            icxInitSubCnMRTable( &submask_tab[0] );
            icxInitSubRCnMRTable( &submask_tab[1] );
            initmasktab = 1;
        }
        
        func = (CxArithmBinMaskFunc2D)
                (submask_tab[inv].fn_2d[CX_MAT_DEPTH(type)]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src2->data.ptr, src2_step,
                         mask->data.ptr, mask_step,
                         dst->data.ptr, dst_step, size, CX_MAT_CN(type) ));
    }

    __END__;
}


CX_IMPL void
cxSubRS( const void* srcarr, CxScalar scalar, void* dstarr, const void* maskarr )
{
    static CxFuncTable subrmask_tab;
    static CxFuncTable subr_tab;
    static int inittab = 0;

    CX_FUNCNAME( "cxSubRS" );

    __BEGIN__;

    int sctype, type, coi = 0;
    int src_step, dst_step;
    CxMat srcstub, *src = (CxMat*)srcarr;
    CxMat dststub, *dst = (CxMat*)dstarr;
    double buf[12];
    int is_nd = 0;
    CxSize size; 

    if( !inittab )
    {
        icxInitSubRCC1RTable( &subr_tab );
        icxInitSubRCCnMRTable( &subrmask_tab );
        inittab = 1;
    }

    if( !CX_IS_MAT(src) )
    {
        if( CX_IS_MATND(src) )
            is_nd = 1;
        else
        {
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
        CxFunc2D_2A1P func;

        if( maskarr )
            CX_ERROR( CX_StsBadMask,
            "This operation on multi-dimensional arrays does not support mask" );

        CX_CALL( cxInitNArrayIterator( 2, arrs, 0, stubs, &iterator ));

        sctype = type = CX_MAT_TYPE(iterator.hdr[0]->type);
        if( CX_MAT_DEPTH(sctype) < CX_32S )
            sctype = (type & CX_MAT_CN_MASK) | CX_32SC1;
        iterator.size.width *= CX_MAT_CN(type);

        func = (CxFunc2D_2A1P)(subr_tab.fn_2d[CX_MAT_DEPTH(type)]);
        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );
       
        CX_CALL( cxScalarToRawData( &scalar, buf, sctype, 0 ));

        do
        {
            IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                             iterator.ptr[1], CX_STUB_STEP,
                             iterator.size, buf ));
        }
        while( cxNextNArraySlice( &iterator ));
        EXIT;
    }

    if( !CX_ARE_TYPES_EQ( src, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    sctype = type = CX_MAT_TYPE(src->type);
    if( CX_MAT_DEPTH(sctype) < CX_32S )
        sctype = (type & CX_MAT_CN_MASK) | CX_32SC1;

    size = cxGetMatSize( src );

    if( !maskarr )
    {
        if( CX_IS_MAT_CONT( src->type & dst->type ))
        {
            size.width *= size.height;
            if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE )
            {
                if( type == CX_32FC1 )
                {
                    const float* srcdata = (const float*)(src->data.ptr);
                    float* dstdata = (float*)(dst->data.ptr);
                
                    do
                    {
                        dstdata[size.width-1] = (float)
                            (scalar.val[0] - srcdata[size.width-1]);
                    }
                    while( --size.width );

                    EXIT;
                }

                if( type == CX_64FC1 )
                {
                    const double* srcdata = (const double*)(src->data.ptr);
                    double* dstdata = (double*)(dst->data.ptr);
                
                    do
                    {
                        dstdata[size.width-1] =
                            scalar.val[0] - srcdata[size.width-1];
                    }
                    while( --size.width );

                    EXIT;
                }
            }

            src_step = dst_step = CX_STUB_STEP;
            size.height = 1;
        }
        else
        {
            src_step = src->step;
            dst_step = dst->step;
        }

        {
            size.width *= CX_MAT_CN( type );
            CxFunc2D_2A1P func = (CxFunc2D_2A1P)(subr_tab.fn_2d[CX_MAT_DEPTH(type)]);

            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            CX_CALL( cxScalarToRawData( &scalar, buf, sctype, 1 ));

            IPPI_CALL( func( src->data.ptr, src_step,
                             dst->data.ptr, dst_step, size, buf ));
        }
    }
    else
    {
        CxMat maskstub, *mask = (CxMat*)maskarr;
        CxArithmUniMaskFunc2D func;
        int mask_step;

        if( !CX_IS_MAT(mask) )
            CX_CALL( mask = cxGetMat( mask, &maskstub ));

        if( !CX_IS_MASK_ARR(mask))
            CX_ERROR( CX_StsBadMask, "" );

        if( !CX_ARE_SIZES_EQ( mask, dst ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );

        if( src->data.ptr != dst->data.ptr )
        {
            CX_CALL( cxCopy( src, dst, mask ));
        }

        if( CX_IS_MAT_CONT( dst->type & mask->type ))
        {
            size.width *= size.height;
            dst_step = mask_step = CX_STUB_STEP;
            size.height = 1;
        }
        else
        {
            dst_step = dst->step;
            mask_step = mask->step;
        }
        
        func = (CxArithmUniMaskFunc2D)(subrmask_tab.fn_2d[CX_MAT_DEPTH(type)]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        CX_CALL( cxScalarToRawData( &scalar, buf, sctype, 0 ));

        IPPI_CALL( func( dst->data.ptr, dst_step, mask->data.ptr,
                         mask_step, size, buf, CX_MAT_CN(type) ));
    }

    __END__;
}


/******************************* A D D ********************************/

CX_IMPL void
cxAdd( const void* srcarr1, const void* srcarr2,
       void* dstarr, const void* maskarr )
{
    static CxFuncTable addmask_tab;
    static CxFuncTable add_tab;
    static int inittab = 0, initmasktab = 0;

    CX_FUNCNAME( "cxAdd" );

    __BEGIN__;

    int type;
    int src1_step, src2_step, dst_step;
    CxMat srcstub1, *src1 = (CxMat*)srcarr1;
    CxMat srcstub2, *src2 = (CxMat*)srcarr2;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxSize size;

    if( !CX_IS_MAT(src1) || !CX_IS_MAT(src2) || !CX_IS_MAT(dst))
    {
        if( CX_IS_MATND(src1) || CX_IS_MATND(src2) || CX_IS_MATND(dst))
        {
            CxArr* arrs[] = { src1, src2, dst };
            CxMatND stubs[3];
            CxNArrayIterator iterator;
            CxFunc2D_3A func;

            if( maskarr )
                CX_ERROR( CX_StsBadMask,
                "This operation on multi-dimensional arrays does not support mask" );

            CX_CALL( cxInitNArrayIterator( 3, arrs, 0, stubs, &iterator ));

            type = iterator.hdr[0]->type;
            iterator.size.width *= CX_MAT_CN(type);

            if( !inittab )
            {
                icxInitAddC1RTable( &add_tab );
                inittab = 1;
            }

            func = (CxFunc2D_3A)(add_tab.fn_2d[CX_MAT_DEPTH(type)]);
            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            do
            {
                IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                                 iterator.ptr[1], CX_STUB_STEP,
                                 iterator.ptr[2], CX_STUB_STEP,
                                 iterator.size ));
            }
            while( cxNextNArraySlice( &iterator ));
            EXIT;
        }
        else
        {
            int coi1 = 0, coi2 = 0, coi3 = 0;
            
            CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi1 ));
            CX_CALL( src2 = cxGetMat( src2, &srcstub2, &coi2 ));
            CX_CALL( dst = cxGetMat( dst, &dststub, &coi3 ));
            if( coi1 + coi2 + coi3 != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    if( !CX_ARE_TYPES_EQ( src1, src2 ) || !CX_ARE_TYPES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src1, src2 ) || !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    type = CX_MAT_TYPE(src1->type);
    size = cxGetMatSize( src1 );

    if( !maskarr )
    {
        int depth = CX_MAT_DEPTH(type);
        size.width *= CX_MAT_CN( type );

        if( CX_IS_MAT_CONT( src1->type & src2->type & dst->type ))
        {
            size.width *= size.height;
            if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE*
                              CX_MAX_INLINE_MAT_OP_SIZE )
            {
                if( depth == CX_32F )
                {
                    const float* src1data = (const float*)(src1->data.ptr);
                    const float* src2data = (const float*)(src2->data.ptr);
                    float* dstdata = (float*)(dst->data.ptr);
                
                    do
                    {
                        dstdata[size.width-1] = (float)
                            (src1data[size.width-1] + src2data[size.width-1]);
                    }
                    while( --size.width );

                    EXIT;
                }

                if( depth == CX_64F )
                {
                    const double* src1data = (const double*)(src1->data.ptr);
                    const double* src2data = (const double*)(src2->data.ptr);
                    double* dstdata = (double*)(dst->data.ptr);
                
                    do
                    {
                        dstdata[size.width-1] =
                            src1data[size.width-1] + src2data[size.width-1];
                    }
                    while( --size.width );

                    EXIT;
                }
            }

            src1_step = src2_step = dst_step = CX_STUB_STEP;
            size.height = 1;
        }
        else
        {
            src1_step = src1->step;
            src2_step = src2->step;
            dst_step = dst->step;
        }

        if( !inittab )
        {
            icxInitAddC1RTable( &add_tab );
            inittab = 1;
        }

        {
            CxFunc2D_3A func = (CxFunc2D_3A)(add_tab.fn_2d[depth]);

            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                             dst->data.ptr, dst_step, size ));
        }
    }
    else
    {
        CxMat maskstub, *mask = (CxMat*)maskarr;
        CxArithmBinMaskFunc2D func;
        int mask_step;

        if( !CX_IS_MAT(mask) )
            CX_CALL( mask = cxGetMat( mask, &maskstub ));

        if( !CX_IS_MASK_ARR(mask))
            CX_ERROR( CX_StsBadMask, "" );

        if( !CX_ARE_SIZES_EQ( mask, dst ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );

        if( dst->data.ptr == src1->data.ptr )
            ;
        else if( dst->data.ptr == src2->data.ptr )
        {
            src2 = src1;
        }
        else
        {
            CX_CALL( cxCopy( src1, dst, mask ));
        }

        if( CX_IS_MAT_CONT( src2->type & dst->type & mask->type ))
        {
            size.width *= size.height;
            src2_step = dst_step = mask_step = CX_STUB_STEP;
            size.height = 1;
        }
        else
        {
            src2_step = src2->step;
            dst_step = dst->step;
            mask_step = mask->step;
        }

        if( !initmasktab )
        {
            icxInitAddCnMRTable( &addmask_tab );
            initmasktab = 1;
        }
        
        func = (CxArithmBinMaskFunc2D)
                (addmask_tab.fn_2d[CX_MAT_DEPTH(type)]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src2->data.ptr, src2_step,
                         mask->data.ptr, mask_step,
                         dst->data.ptr, dst_step, size, CX_MAT_CN(type) ));
    }

    __END__;
}


CX_IMPL void
cxAddS( const void* srcarr, CxScalar scalar, void* dstarr, const void* maskarr )
{
    static CxFuncTable addmask_tab;
    static CxFuncTable add_tab;
    static int inittab = 0;

    CX_FUNCNAME( "cxAddS" );

    __BEGIN__;

    int sctype, type, coi = 0;
    int src_step, dst_step;
    int is_nd = 0;
    CxMat srcstub, *src = (CxMat*)srcarr;
    CxMat dststub, *dst = (CxMat*)dstarr;
    double buf[12];
    CxSize size;

    if( !inittab )
    {
        icxInitAddCC1RTable( &add_tab );
        icxInitAddCCnMRTable( &addmask_tab );
        inittab = 1;
    }

    if( !CX_IS_MAT(src) )
    {
        if( CX_IS_MATND(src) )
            is_nd = 1;
        else
        {
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
        CxFunc2D_2A1P func;

        if( maskarr )
            CX_ERROR( CX_StsBadMask,
            "This operation on multi-dimensional arrays does not support mask" );

        CX_CALL( cxInitNArrayIterator( 2, arrs, 0, stubs, &iterator ));

        sctype = type = CX_MAT_TYPE(iterator.hdr[0]->type);
        if( CX_MAT_DEPTH(sctype) < CX_32S )
            sctype = (type & CX_MAT_CN_MASK) | CX_32SC1;
        iterator.size.width *= CX_MAT_CN(type);

        func = (CxFunc2D_2A1P)(add_tab.fn_2d[CX_MAT_DEPTH(type)]);
        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );
       
        CX_CALL( cxScalarToRawData( &scalar, buf, sctype, 0 ));

        do
        {
            IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                             iterator.ptr[1], CX_STUB_STEP,
                             iterator.size, buf ));
        }
        while( cxNextNArraySlice( &iterator ));
        EXIT;
    }

    if( !CX_ARE_TYPES_EQ( src, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    sctype = type = CX_MAT_TYPE(src->type);
    if( CX_MAT_DEPTH(sctype) < CX_32S )
        sctype = (type & CX_MAT_CN_MASK) | CX_32SC1;

    size = cxGetMatSize( src );

    if( !maskarr )
    {
        if( CX_IS_MAT_CONT( src->type & dst->type ))
        {
            size.width *= size.height;
            if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE )
            {
                if( type == CX_32FC1 )
                {
                    const float* srcdata = (const float*)(src->data.ptr);
                    float* dstdata = (float*)(dst->data.ptr);
                
                    do
                    {
                        dstdata[size.width-1] = (float)
                            (scalar.val[0] + srcdata[size.width-1]);
                    }
                    while( --size.width );

                    EXIT;
                }

                if( type == CX_64FC1 )
                {
                    const double* srcdata = (const double*)(src->data.ptr);
                    double* dstdata = (double*)(dst->data.ptr);
                
                    do
                    {
                        dstdata[size.width-1] =
                            scalar.val[0] + srcdata[size.width-1];
                    }
                    while( --size.width );

                    EXIT;
                }
            }

            src_step = dst_step = CX_STUB_STEP;
            size.height = 1;
        }
        else
        {
            src_step = src->step;
            dst_step = dst->step;
        }

        {
            size.width *= CX_MAT_CN( type );
            CxFunc2D_2A1P func = (CxFunc2D_2A1P)(add_tab.fn_2d[CX_MAT_DEPTH(type)]);

            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            CX_CALL( cxScalarToRawData( &scalar, buf, sctype, 1 ));

            IPPI_CALL( func( src->data.ptr, src_step,
                             dst->data.ptr, dst_step, size, buf ));
        }
    }
    else
    {
        CxMat maskstub, *mask = (CxMat*)maskarr;
        CxArithmUniMaskFunc2D func;
        int mask_step;

        if( !CX_IS_MAT(mask) )
            CX_CALL( mask = cxGetMat( mask, &maskstub ));

        if( !CX_IS_MASK_ARR(mask))
            CX_ERROR( CX_StsBadMask, "" );

        if( !CX_ARE_SIZES_EQ( mask, dst ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );

        if( src->data.ptr != dst->data.ptr )
        {
            CX_CALL( cxCopy( src, dst, mask ));
        }

        if( CX_IS_MAT_CONT( dst->type & mask->type ))
        {
            size.width *= size.height;
            dst_step = mask_step = CX_STUB_STEP;
            size.height = 1;
        }
        else
        {
            dst_step = dst->step;
            mask_step = mask->step;
        }
        
        func = (CxArithmUniMaskFunc2D)(addmask_tab.fn_2d[CX_MAT_DEPTH(type)]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        CX_CALL( cxScalarToRawData( &scalar, buf, sctype, 0 ));

        IPPI_CALL( func( dst->data.ptr, dst_step, mask->data.ptr,
                         mask_step, size, buf, CX_MAT_CN(type) ));
    }

    __END__;
}


/***************************************** M U L ****************************************/

#define ICX_DEF_MUL_OP_CASE( flavor, arrtype, worktype, _cast_macro1_,                  \
                             _cast_macro2_, _cxt_macro_ )                               \
                                                                                        \
IPCXAPI_IMPL( CxStatus, icxMul_##flavor##_C1R,( const arrtype* src1, int step1,         \
                                          const arrtype* src2, int step2,               \
                                          arrtype* dst, int step,                       \
                                          CxSize size, double scale ))                  \
{                                                                                       \
    if( scale == 1 )                                                                    \
    {                                                                                   \
        for( ; size.height--; (char*&)src1+=step1,                                      \
                              (char*&)src2+=step2,                                      \
                              (char*&)dst+=step )                                       \
        {                                                                               \
            int i;                                                                      \
            for( i = 0; i <= size.width - 4; i += 4 )                                   \
            {                                                                           \
                worktype t0 = src1[i] * src2[i];                                        \
                worktype t1 = src1[i+1] * src2[i+1];                                    \
                                                                                        \
                dst[i] = _cast_macro2_(t0);                                             \
                dst[i+1] = _cast_macro2_(t1);                                           \
                                                                                        \
                t0 = src1[i+2] * src2[i+2];                                             \
                t1 = src1[i+3] * src2[i+3];                                             \
                                                                                        \
                dst[i+2] = _cast_macro2_(t0);                                           \
                dst[i+3] = _cast_macro2_(t1);                                           \
            }                                                                           \
                                                                                        \
            for( ; i < size.width; i++ )                                                \
            {                                                                           \
                worktype t0 = src1[i] * src2[i];                                        \
                dst[i] = _cast_macro2_(t0);                                             \
            }                                                                           \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        for( ; size.height--; (char*&)src1+=step1,                                      \
                              (char*&)src2+=step2,                                      \
                              (char*&)dst+=step )                                       \
        {                                                                               \
            int i;                                                                      \
            for( i = 0; i <= size.width - 4; i += 4 )                                   \
            {                                                                           \
                worktype t0, t1;                                                        \
                t0 = _cast_macro1_(scale*_cxt_macro_(src1[i])*_cxt_macro_(src2[i]));    \
                t1 = _cast_macro1_(scale*_cxt_macro_(src1[i+1])*_cxt_macro_(src2[i+1]));\
                                                                                        \
                dst[i] = _cast_macro2_(t0);                                             \
                dst[i+1] = _cast_macro2_(t1);                                           \
                                                                                        \
                t0 = _cast_macro1_(scale*_cxt_macro_(src1[i+2])*_cxt_macro_(src2[i+2]));\
                t1 = _cast_macro1_(scale*_cxt_macro_(src1[i+3])*_cxt_macro_(src2[i+3]));\
                                                                                        \
                dst[i+2] = _cast_macro2_(t0);                                           \
                dst[i+3] = _cast_macro2_(t1);                                           \
            }                                                                           \
                                                                                        \
            for( ; i < size.width; i++ )                                                \
            {                                                                           \
                worktype t0;                                                            \
                t0 = _cast_macro1_(scale*_cxt_macro_(src1[i])*_cxt_macro_(src2[i]));    \
                dst[i] = _cast_macro2_(t0);                                             \
            }                                                                           \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    return CX_OK;                                                                       \
}


ICX_DEF_MUL_OP_CASE( 8u, uchar, int, cxRound, CX_CAST_8U, CX_8TO32F )
ICX_DEF_MUL_OP_CASE( 16s, short, int, cxRound, CX_CAST_16S, CX_NOP )
ICX_DEF_MUL_OP_CASE( 32s, int, int, cxRound, CX_CAST_32S, CX_NOP )
ICX_DEF_MUL_OP_CASE( 32f, float, double, CX_NOP, CX_CAST_32F, CX_NOP )
ICX_DEF_MUL_OP_CASE( 64f, double, double, CX_NOP, CX_CAST_64F, CX_NOP )


ICX_DEF_INIT_ARITHM_FUNC_TAB( Mul, C1R );


typedef CxStatus (CX_STDCALL * CxScaledElWiseFunc)( const void* src1, int step1,
                                                    const void* src2, int step2,
                                                    void* dst, int step,
                                                    CxSize size, double scale );

CX_IMPL void
cxMul( const void* srcarr1, const void* srcarr2, void* dstarr, double scale )
{
    static CxFuncTable mul_tab;
    static int inittab = 0;

    CX_FUNCNAME( "cxMul" );

    __BEGIN__;

    int type, depth, coi = 0;
    int src1_step, src2_step, dst_step;
    int is_nd = 0;
    CxMat srcstub1, *src1 = (CxMat*)srcarr1;
    CxMat srcstub2, *src2 = (CxMat*)srcarr2;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxSize size;
    CxScaledElWiseFunc func;

    if( !inittab )
    {
        icxInitMulC1RTable( &mul_tab );
        inittab = 1;
    }

    if( !CX_IS_MAT(src1) )
    {
        if( CX_IS_MATND(src1) )
            is_nd = 1;
        else
        {
            CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi ));
            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    if( !CX_IS_MAT(src2) )
    {
        if( CX_IS_MATND(src2) )
            is_nd = 1;
        else
        {
            CX_CALL( src2 = cxGetMat( src2, &srcstub2, &coi ));
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
            CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));
            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    if( is_nd )
    {
        CxArr* arrs[] = { src1, src2, dst };
        CxMatND stubs[3];
        CxNArrayIterator iterator;
        CxScaledElWiseFunc func;

        CX_CALL( cxInitNArrayIterator( 3, arrs, 0, stubs, &iterator ));

        type = iterator.hdr[0]->type;
        iterator.size.width *= CX_MAT_CN(type);

        func = (CxScaledElWiseFunc)(mul_tab.fn_2d[CX_MAT_DEPTH(type)]);
        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        do
        {
            IPPI_CALL( func( iterator.ptr[0], CX_STUB_STEP,
                             iterator.ptr[1], CX_STUB_STEP,
                             iterator.ptr[2], CX_STUB_STEP,
                             iterator.size, scale ));
        }
        while( cxNextNArraySlice( &iterator ));
        EXIT;
    }

    if( !CX_ARE_TYPES_EQ( src1, src2 ) || !CX_ARE_TYPES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src1, src2 ) || !CX_ARE_SIZES_EQ( src1, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    type = CX_MAT_TYPE(src1->type);
    size = cxGetMatSize( src1 );

    depth = CX_MAT_DEPTH(type);
    size.width *= CX_MAT_CN( type );

    if( CX_IS_MAT_CONT( src1->type & src2->type & dst->type ))
    {
        size.width *= size.height;

        if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE && scale == 1 )
        {
            if( depth == CX_32F )
            {
                const float* src1data = (const float*)(src1->data.ptr);
                const float* src2data = (const float*)(src2->data.ptr);
                float* dstdata = (float*)(dst->data.ptr);
            
                do
                {
                    dstdata[size.width-1] = (float)
                        (src1data[size.width-1] * src2data[size.width-1]);
                }
                while( --size.width );

                EXIT;
            }

            if( depth == CX_64F )
            {
                const double* src1data = (const double*)(src1->data.ptr);
                const double* src2data = (const double*)(src2->data.ptr);
                double* dstdata = (double*)(dst->data.ptr);
            
                do
                {
                    dstdata[size.width-1] =
                        src1data[size.width-1] * src2data[size.width-1];
                }
                while( --size.width );

                EXIT;
            }
        }

        src1_step = src2_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        src2_step = src2->step;
        dst_step = dst->step;
    }

    func = (CxScaledElWiseFunc)(mul_tab.fn_2d[CX_MAT_DEPTH(type)]);

    if( !func )
        CX_ERROR( CX_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                     dst->data.ptr, dst_step, size, scale ));

    __END__;
}


/***************************************** D I V ****************************************/

#define ICX_DEF_DIV_OP_CASE( flavor, arrtype, worktype, _cast_macro1_,                  \
                             _cast_macro2_, _cxt_macro_, _check_macro_ )                \
                                                                                        \
IPCXAPI( CxStatus, icxDiv_##flavor##_C1R,( const arrtype* src1, int step1,              \
                                          const arrtype* src2, int step2,               \
                                          arrtype* dst, int step,                       \
                                          CxSize size, double scale ))                  \
                                                                                        \
IPCXAPI_IMPL( CxStatus, icxDiv_##flavor##_C1R,( const arrtype* src1, int step1,         \
                                          const arrtype* src2, int step2,               \
                                          arrtype* dst, int step,                       \
                                          CxSize size, double scale ))                  \
{                                                                                       \
    for( ; size.height--; (char*&)src1+=step1, (char*&)src2+=step2, (char*&)dst+=step ) \
    {                                                                                   \
        int i;                                                                          \
        for( i = 0; i <= size.width - 4; i += 4 )                                       \
        {                                                                               \
            if( _check_macro_(src2[i]) && _check_macro_(src2[i+1]) &&                   \
                _check_macro_(src2[i+2]) && _check_macro_(src2[i+3]))                   \
            {                                                                           \
                double a = _cxt_macro_(src2[i]) * _cxt_macro_(src2[i+1]);               \
                double b = _cxt_macro_(src2[i+2]) * _cxt_macro_(src2[i+3]);             \
                double d = scale/(a * b);                                               \
                                                                                        \
                b *= d;                                                                 \
                a *= d;                                                                 \
                                                                                        \
                worktype z0 = _cast_macro1_(src2[i+1] * _cxt_macro_(src1[i]) * b);      \
                worktype z1 = _cast_macro1_(src2[i] * _cxt_macro_(src1[i+1]) * b);      \
                worktype z2 = _cast_macro1_(src2[i+3] * _cxt_macro_(src1[i+2]) * a);    \
                worktype z3 = _cast_macro1_(src2[i+2] * _cxt_macro_(src1[i+3]) * a);    \
                                                                                        \
                dst[i] = _cast_macro2_(z0);                                             \
                dst[i+1] = _cast_macro2_(z1);                                           \
                dst[i+2] = _cast_macro2_(z2);                                           \
                dst[i+3] = _cast_macro2_(z3);                                           \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                worktype z0 = _check_macro_(src2[i]) ?                                  \
                   _cast_macro1_(_cxt_macro_(src1[i])*scale/_cxt_macro_(src2[i])) : 0;  \
                worktype z1 = _check_macro_(src2[i+1]) ?                                \
                   _cast_macro1_(_cxt_macro_(src1[i+1])*scale/_cxt_macro_(src2[i+1])):0;\
                worktype z2 = _check_macro_(src2[i+2]) ?                                \
                   _cast_macro1_(_cxt_macro_(src1[i+2])*scale/_cxt_macro_(src2[i+2])):0;\
                worktype z3 = _check_macro_(src2[i+3]) ?                                \
                   _cast_macro1_(_cxt_macro_(src1[i+3])*scale/_cxt_macro_(src2[i+3])):0;\
                                                                                        \
                dst[i] = _cast_macro2_(z0);                                             \
                dst[i+1] = _cast_macro2_(z1);                                           \
                dst[i+2] = _cast_macro2_(z2);                                           \
                dst[i+3] = _cast_macro2_(z3);                                           \
            }                                                                           \
        }                                                                               \
                                                                                        \
        for( ; i < size.width; i++ )                                                    \
        {                                                                               \
            worktype z0 = _check_macro_(src2[i]) ?                                      \
                _cast_macro1_(_cxt_macro_(src1[i])*scale/_cxt_macro_(src2[i])) : 0;     \
                                                                                        \
            dst[i] = _cast_macro2_(z0);                                                 \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    return CX_OK;                                                                       \
}


#define ICX_DEF_RECIP_OP_CASE( flavor, arrtype, worktype, _cast_macro1_,        \
                             _cast_macro2_, _cxt_macro_, _check_macro_ )        \
                                                                                \
IPCXAPI( CxStatus, icxRecip_##flavor##_C1R,( const arrtype* src, int step1,     \
                                          arrtype* dst, int step,               \
                                          CxSize size, double scale ))          \
                                                                                \
IPCXAPI_IMPL( CxStatus,                                                         \
    icxRecip_##flavor##_C1R,( const arrtype* src, int step1,                    \
                              arrtype* dst, int step,                           \
                              CxSize size, double scale ))                      \
{                                                                               \
    for( ; size.height--; (char*&)src+=step1, (char*&)dst+=step )               \
    {                                                                           \
        int i;                                                                  \
        for( i = 0; i <= size.width - 4; i += 4 )                               \
        {                                                                       \
            if( _check_macro_(src[i]) && _check_macro_(src[i+1]) &&             \
                _check_macro_(src[i+2]) && _check_macro_(src[i+3]))             \
            {                                                                   \
                double a = _cxt_macro_(src[i]) * _cxt_macro_(src[i+1]);         \
                double b = _cxt_macro_(src[i+2]) * _cxt_macro_(src[i+3]);       \
                double d = scale/(a * b);                                       \
                                                                                \
                b *= d;                                                         \
                a *= d;                                                         \
                                                                                \
                worktype z0 = _cast_macro1_(src[i+1] * b);                      \
                worktype z1 = _cast_macro1_(src[i] * b);                        \
                worktype z2 = _cast_macro1_(src[i+3] * a);                      \
                worktype z3 = _cast_macro1_(src[i+2] * a);                      \
                                                                                \
                dst[i] = _cast_macro2_(z0);                                     \
                dst[i+1] = _cast_macro2_(z1);                                   \
                dst[i+2] = _cast_macro2_(z2);                                   \
                dst[i+3] = _cast_macro2_(z3);                                   \
            }                                                                   \
            else                                                                \
            {                                                                   \
                worktype z0 = _check_macro_(src[i]) ?                           \
                   _cast_macro1_(scale/_cxt_macro_(src[i])) : 0;                \
                worktype z1 = _check_macro_(src[i+1]) ?                         \
                   _cast_macro1_(scale/_cxt_macro_(src[i+1])):0;                \
                worktype z2 = _check_macro_(src[i+2]) ?                         \
                   _cast_macro1_(scale/_cxt_macro_(src[i+2])):0;                \
                worktype z3 = _check_macro_(src[i+3]) ?                         \
                   _cast_macro1_(scale/_cxt_macro_(src[i+3])):0;                \
                                                                                \
                dst[i] = _cast_macro2_(z0);                                     \
                dst[i+1] = _cast_macro2_(z1);                                   \
                dst[i+2] = _cast_macro2_(z2);                                   \
                dst[i+3] = _cast_macro2_(z3);                                   \
            }                                                                   \
        }                                                                       \
                                                                                \
        for( ; i < size.width; i++ )                                            \
        {                                                                       \
            worktype z0 = _check_macro_(src[i]) ?                               \
                _cast_macro1_(scale/_cxt_macro_(src[i])) : 0;                   \
                                                                                \
            dst[i] = _cast_macro2_(z0);                                         \
        }                                                                       \
    }                                                                           \
                                                                                \
    return CX_OK;                                                               \
}


#define div_check_zero_flt(x)  (((int&)(x) & 0x7fffffff) != 0)
#define div_check_zero_dbl(x)  (((int64&)(x) & CX_BIG_INT(0x7fffffffffffffff)) != 0)

ICX_DEF_DIV_OP_CASE( 8u, uchar, int, cxRound, CX_CAST_8U, CX_8TO32F, CX_NONZERO )
ICX_DEF_DIV_OP_CASE( 8s, char, int, cxRound, CX_CAST_8S, CX_8TO32F, CX_NONZERO )
ICX_DEF_DIV_OP_CASE( 16s, short, int, cxRound, CX_CAST_16S, CX_NOP, CX_NONZERO )
ICX_DEF_DIV_OP_CASE( 32s, int, int, cxRound, CX_CAST_32S, CX_NOP, CX_NONZERO )
ICX_DEF_DIV_OP_CASE( 32f, float, double, CX_NOP, CX_CAST_32F, CX_NOP, div_check_zero_flt )
ICX_DEF_DIV_OP_CASE( 64f, double, double, CX_NOP, CX_CAST_64F, CX_NOP, div_check_zero_dbl )

ICX_DEF_RECIP_OP_CASE( 8u, uchar, int, cxRound, CX_CAST_8U, CX_8TO32F, CX_NONZERO )
ICX_DEF_RECIP_OP_CASE( 8s, char, int, cxRound, CX_CAST_8S, CX_8TO32F, CX_NONZERO )
ICX_DEF_RECIP_OP_CASE( 16s, short, int, cxRound, CX_CAST_16S, CX_NOP, CX_NONZERO )
ICX_DEF_RECIP_OP_CASE( 32s, int, int, cxRound, CX_CAST_32S, CX_NOP, CX_NONZERO )
ICX_DEF_RECIP_OP_CASE( 32f, float, double, CX_NOP, CX_CAST_32F, CX_NOP, div_check_zero_flt )
ICX_DEF_RECIP_OP_CASE( 64f, double, double, CX_NOP, CX_CAST_64F, CX_NOP, div_check_zero_dbl )

ICX_DEF_INIT_ARITHM_FUNC_TAB( Div, C1R )
ICX_DEF_INIT_ARITHM_FUNC_TAB( Recip, C1R )

typedef CxStatus (CX_STDCALL * CxRecipFunc)( const void* src, int step1,
                                             void* dst, int step,
                                             CxSize size, double scale );

CX_IMPL void
cxDiv( const void* srcarr1, const void* srcarr2, void* dstarr, double scale )
{
    static CxFuncTable div_tab;
    static CxFuncTable recip_tab;
    static int inittab = 0;

    CX_FUNCNAME( "cxDiv" );

    __BEGIN__;

    int type, depth, coi = 0;
    int is_nd = 0;
    int src1_step, src2_step, dst_step;
    int src1_cont_flag = CX_MAT_CONT_FLAG;
    CxMat srcstub1, *src1 = (CxMat*)srcarr1;
    CxMat srcstub2, *src2 = (CxMat*)srcarr2;
    CxMat dststub,  *dst = (CxMat*)dstarr;
    CxSize size;

    if( !inittab )
    {
        icxInitDivC1RTable( &div_tab );
        icxInitRecipC1RTable( &recip_tab );
        inittab = 1;
    }

    if( !CX_IS_MAT(src2) )
    {
        if( CX_IS_MATND(src2))
            is_nd = 1;
        else
        {
            CX_CALL( src2 = cxGetMat( src2, &srcstub2, &coi ));
            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    if( src1 )
    {
        if( CX_IS_MATND(src1))
            is_nd = 1;
        else
        {
            if( !CX_IS_MAT(src1) )
            {
                CX_CALL( src1 = cxGetMat( src1, &srcstub1, &coi ));
                if( coi != 0 )
                    CX_ERROR( CX_BadCOI, "" );
            }

            if( !CX_ARE_TYPES_EQ( src1, src2 ))
                CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

            if( !CX_ARE_SIZES_EQ( src1, src2 ))
                CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );
            src1_cont_flag = src1->type;
        }
    }

    if( !CX_IS_MAT(dst) )
    {
        if( CX_IS_MATND(dst))
            is_nd = 1;
        else
        {
            CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));
            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    if( is_nd )
    {
        CxArr* arrs[] = { dst, src2, src1 };
        CxMatND stubs[3];
        CxNArrayIterator iterator;

        CX_CALL( cxInitNArrayIterator( 2 + (src1 != 0), arrs, 0, stubs, &iterator ));

        type = iterator.hdr[0]->type;
        iterator.size.width *= CX_MAT_CN(type);

        if( src1 )
        {
            CxScaledElWiseFunc func =
                (CxScaledElWiseFunc)(div_tab.fn_2d[CX_MAT_DEPTH(type)]);
            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            do
            {
                IPPI_CALL( func( iterator.ptr[2], CX_STUB_STEP,
                                 iterator.ptr[1], CX_STUB_STEP,
                                 iterator.ptr[0], CX_STUB_STEP,
                                 iterator.size, scale ));
            }
            while( cxNextNArraySlice( &iterator ));
        }
        else
        {
            CxRecipFunc func = (CxRecipFunc)(recip_tab.fn_2d[CX_MAT_DEPTH(type)]);

            if( !func )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            do
            {
                IPPI_CALL( func( iterator.ptr[1], CX_STUB_STEP,
                                 iterator.ptr[0], CX_STUB_STEP,
                                 iterator.size, scale ));
            }
            while( cxNextNArraySlice( &iterator ));
        }
        EXIT;
    }

    if( !CX_ARE_TYPES_EQ( src2, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src2, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    type = CX_MAT_TYPE(src2->type);
    size = cxGetMatSize( src2 );

    depth = CX_MAT_DEPTH(type);
    size.width *= CX_MAT_CN( type );

    if( CX_IS_MAT_CONT( src1_cont_flag & src2->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = src2_step = dst_step = CX_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1 ? src1->step : 0;
        src2_step = src2->step;
        dst_step = dst->step;
    }

    if( src1 )
    {
        CxScaledElWiseFunc func = (CxScaledElWiseFunc)(div_tab.fn_2d[CX_MAT_DEPTH(type)]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                         dst->data.ptr, dst_step, size, scale ));
    }
    else
    {
        CxRecipFunc func = (CxRecipFunc)(recip_tab.fn_2d[CX_MAT_DEPTH(type)]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src2->data.ptr, src2_step,
                         dst->data.ptr, dst_step, size, scale ));
    }

    __END__;
}

/******************************* A D D   W E I G T E D ******************************/

#define ICX_DEF_ADD_WEIGHTED_OP(flavor, arrtype, worktype, load_macro,                  \
                                     cast_macro1, cast_macro2)                          \
IPCXAPI( CxStatus,                                                                      \
    icxAddWeighted_##flavor##_C1R,( const arrtype* src1, int step1, double alpha,       \
                                    const arrtype* src2, int step2, double beta,        \
                                    double gamma, arrtype* dst, int step, CxSize size ))\
                                                                                        \
IPCXAPI_IMPL( CxStatus,                                                                 \
    icxAddWeighted_##flavor##_C1R,( const arrtype* src1, int step1, double alpha,       \
                                    const arrtype* src2, int step2, double beta,        \
                                    double gamma, arrtype* dst, int step, CxSize size ))\
{                                                                                       \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2,                 \
                          (char*&)dst += step )                                         \
    {                                                                                   \
        int i;                                                                          \
                                                                                        \
        for( i = 0; i <= size.width - 4; i += 4 )                                       \
        {                                                                               \
            worktype t0 = cast_macro1(load_macro((src1)[i])*alpha +                     \
                                      load_macro((src2)[i])*beta + gamma);              \
            worktype t1 = cast_macro1(load_macro((src1)[i+1])*alpha +                   \
                                      load_macro((src2)[i+1])*beta + gamma);            \
                                                                                        \
            (dst)[i] = cast_macro2( t0 );                                               \
            (dst)[i+1] = cast_macro2( t1 );                                             \
                                                                                        \
            t0 = cast_macro1(load_macro((src1)[i+2])*alpha +                            \
                             load_macro((src2)[i+2])*beta + gamma);                     \
            t1 = cast_macro1(load_macro((src1)[i+3])*alpha +                            \
                             load_macro((src2)[i+3])*beta + gamma);                     \
                                                                                        \
            (dst)[i+2] = cast_macro2( t0 );                                             \
            (dst)[i+3] = cast_macro2( t1 );                                             \
        }                                                                               \
                                                                                        \
        for( ; i < size.width; i++ )                                                    \
        {                                                                               \
            worktype t0 = cast_macro1(load_macro((src1)[i])*alpha +                     \
                                      load_macro((src2)[i])*beta + gamma);              \
            (dst)[i] = cast_macro2( t0 );                                               \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    return CX_OK;                                                                       \
}


#undef shift
#define shift 11

IPCXAPI( CxStatus,
    icxAddWeighted_8u_fast_C1R, ( const uchar* src1, int step1, double alpha,
                                  const uchar* src2, int step2, double beta,
                                  double gamma, uchar* dst, int step, CxSize size ))

IPCXAPI_IMPL( CxStatus,
    icxAddWeighted_8u_fast_C1R, ( const uchar* src1, int step1, double alpha,
                                  const uchar* src2, int step2, double beta,
                                  double gamma, uchar* dst, int step, CxSize size ))
{
    int tab1[256], tab2[256];
    double t = 0;
    int j, t0, t1, t2, t3;

    alpha *= 1 << shift;
    gamma *= 1 << shift;
    beta *= 1 << shift;

    for( j = 0; j < 256; j++ )
    {
        tab1[j] = cxRound(t);
        tab2[j] = cxRound(gamma);
        t += alpha;
        gamma += beta;
    }

    t0 = CX_DESCALE( tab1[0] + tab2[0], shift );
    t1 = CX_DESCALE( tab1[0] + tab2[255], shift );
    t2 = CX_DESCALE( tab1[255] + tab2[0], shift );
    t3 = CX_DESCALE( tab1[255] + tab2[255], shift );

    if( (unsigned)(t0+256) <= 768 && (unsigned)(t1+256) <= 768 &&
        (unsigned)(t2+256) <= 768 && (unsigned)(t3+256) <= 768 )
    {
        // use faster table-based convertion back to 8u
        for( ; size.height--; src1 += step1, src2 += step2, dst += step )
        {
            int i;

            for( i = 0; i <= size.width - 4; i += 4 )
            {
                int t0 = CX_DESCALE( tab1[src1[i]] + tab2[src2[i]], shift );
                int t1 = CX_DESCALE( tab1[src1[i+1]] + tab2[src2[i+1]], shift );

                (dst)[i] = CX_FAST_CAST_8U( t0 );
                (dst)[i+1] = CX_FAST_CAST_8U( t1 );

                t0 = CX_DESCALE( tab1[src1[i+2]] + tab2[src2[i+2]], shift );
                t1 = CX_DESCALE( tab1[src1[i+3]] + tab2[src2[i+3]], shift );

                (dst)[i+2] = CX_FAST_CAST_8U( t0 );
                (dst)[i+3] = CX_FAST_CAST_8U( t1 );
            }

            for( ; i < size.width; i++ )
            {
                int t0 = CX_DESCALE( tab1[src1[i]] + tab2[src2[i]], shift );
                (dst)[i] = CX_FAST_CAST_8U( t0 );
            }
        }
    }
    else
    {
        // use universal macro for convertion back to 8u
        for( ; size.height--; src1 += step1, src2 += step2, dst += step )
        {
            int i;
            
            for( i = 0; i <= size.width - 4; i += 4 )
            {
                int t0 = CX_DESCALE( tab1[src1[i]] + tab2[src2[i]], shift );
                int t1 = CX_DESCALE( tab1[src1[i+1]] + tab2[src2[i+1]], shift );

                (dst)[i] = CX_CAST_8U( t0 );
                (dst)[i+1] = CX_CAST_8U( t1 );

                t0 = CX_DESCALE( tab1[src1[i+2]] + tab2[src2[i+2]], shift );
                t1 = CX_DESCALE( tab1[src1[i+3]] + tab2[src2[i+3]], shift );

                (dst)[i+2] = CX_CAST_8U( t0 );
                (dst)[i+3] = CX_CAST_8U( t1 );
            }

            for( ; i < size.width; i++ )
            {
                int t0 = CX_DESCALE( tab1[src1[i]] + tab2[src2[i]], shift );
                (dst)[i] = CX_CAST_8U( t0 );
            }
        }
    }

    return CX_OK;
}


ICX_DEF_ADD_WEIGHTED_OP( 8u, uchar, int, CX_8TO32F, cxRound, CX_CAST_8U )
ICX_DEF_ADD_WEIGHTED_OP( 8s, char, int, CX_8TO32F, cxRound, CX_CAST_8S )
ICX_DEF_ADD_WEIGHTED_OP( 16s, short, int, CX_NOP, cxRound, CX_CAST_16S )
ICX_DEF_ADD_WEIGHTED_OP( 32s, int, int, CX_NOP, cxRound, CX_CAST_32S )
ICX_DEF_ADD_WEIGHTED_OP( 32f, float, double, CX_NOP, CX_NOP, CX_CAST_32F )
ICX_DEF_ADD_WEIGHTED_OP( 64f, double, double, CX_NOP, CX_NOP, CX_CAST_64F )


ICX_DEF_INIT_ARITHM_FUNC_TAB( AddWeighted, C1R );

typedef CxStatus (CX_STDCALL *CxAddWeightedFunc)( const void* src1, int step1, double alpha,
                                                  const void* src2, int step2, double beta,
                                                  double gamma, void* dst,
                                                  int step, CxSize size );

CX_IMPL void
cxAddWeighted( const CxArr* srcAarr, double alpha,
               const CxArr* srcBarr, double beta,
               double gamma, CxArr* dstarr )
{
    static CxFuncTable addw_tab;
    static int inittab = 0;
    
    CX_FUNCNAME( "cxAddWeighted" );

    __BEGIN__;

    CxMat   srcA_stub, *srcA = (CxMat*)srcAarr;
    CxMat   srcB_stub, *srcB = (CxMat*)srcBarr;
    CxMat   dst_stub, *dst = (CxMat*)dstarr;
    int  coi1, coi2, coi;
    int  srcA_step, srcB_step, dst_step;
    int  type;
    CxAddWeightedFunc func;
    CxSize size;

    if( !inittab )
    {
        icxInitAddWeightedC1RTable( &addw_tab );
        inittab = 1;
    }

    CX_CALL( srcA = cxGetMat( srcA, &srcA_stub, &coi1 ));
    CX_CALL( srcB = cxGetMat( srcB, &srcB_stub, &coi2 ));
    CX_CALL( dst = cxGetMat( dst, &dst_stub, &coi ));

    if( coi1 || coi2 || coi )
        CX_ERROR( CX_BadCOI, "COI must not be set" );

    if( !CX_ARE_TYPES_EQ( srcA, srcB ) ||
        !CX_ARE_TYPES_EQ( srcA, dst ))
        CX_ERROR( CX_StsUnmatchedFormats,
        "All input/output arrays should have the same type");

    if( !CX_ARE_SIZES_EQ( srcA, srcB ) ||
        !CX_ARE_SIZES_EQ( srcA, dst ))
        CX_ERROR( CX_StsUnmatchedSizes,
        "All input/output arrays should have the same sizes");

    size = cxGetMatSize( srcA );
    type = CX_MAT_TYPE( srcA->type );
    size.width *= CX_MAT_CN( type );
    srcA_step = srcA->step;
    srcB_step = srcB->step;
    dst_step = dst->step;

    if( CX_IS_MAT_CONT( type & srcB->type & dst->type ))
    {
        size.width *= size.height;
        size.height = 1;
        srcA_step = srcB_step = dst_step = CX_AUTOSTEP;
    }

    if( type == CX_8UC1 && size.width * size.height > 1024 &&
        fabs(alpha) < 1024 && fabs(beta) < 1024 && fabs(gamma) < 1024*256 )
    {
        func = (CxAddWeightedFunc)icxAddWeighted_8u_fast_C1R;
    }
    else
    {
        func = (CxAddWeightedFunc)addw_tab.fn_2d[CX_MAT_DEPTH(type)];
        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "This array type is not supported" );
    }

    IPPI_CALL( func( srcA->data.ptr, srcA_step, alpha, srcB->data.ptr, srcB_step,
                     beta, gamma, dst->data.ptr, dst_step, size ));

    __END__;
}


/* End of file. */
