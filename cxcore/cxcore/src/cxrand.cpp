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
//  Filling CxMat/IplImage instances with random numbers
//
// */

#include "_cxcore.h"


///////////////////////////// Functions Declaration //////////////////////////////////////

/*
   Multiply-with-carry generator is used here:
   temp = ( A*X(n) + carry )
   X(n+1) = temp mod (2^32)
   carry = temp / (2^32)
*/
#define  ICX_RNG_NEXT(x)    ((uint64)(unsigned)(x)*1554115554 + ((x) >> 32))
#define  ICX_CXT_FLT(x)     (((unsigned)(x) >> 9)|CX_1F)
#define  ICX_1D             CX_BIG_INT(0x3FF0000000000000)
#define  ICX_CXT_DBL(x)     (((uint64)(unsigned)(x) << 20)|((x) >> 44)|ICX_1D)

/***************************************************************************************\
*                           Pseudo-Random Number Generators (PRNGs)                     *
\***************************************************************************************/

CX_IMPL void
cxRandSetRange( CxRandState * state, double lower, double upper, int index  )
{
    CX_FUNCNAME( "cxRandSetRange" );

    __BEGIN__;

    if( !state )
        CX_ERROR_FROM_CODE( CX_StsNullPtr );

    /*if( lower > upper )
        CX_ERROR( CX_StsOutOfRange,
        "lower boundary is greater than the upper one" );*/

    if( (unsigned)(index + 1) > 4 )
        CX_ERROR( CX_StsOutOfRange, "index is not in -1..3" );

    if( index < 0 )
    {
        state->param[0].val[0] = state->param[0].val[1] =
        state->param[0].val[2] = state->param[0].val[3] = lower;
        state->param[1].val[0] = state->param[1].val[1] = 
        state->param[1].val[2] = state->param[1].val[3] = upper;
    }
    else
    {
        state->param[0].val[index] = lower;
        state->param[1].val[index] = upper;
    }

    __END__;
}


CX_IMPL void
cxRandInit( CxRandState* state, double lower, double upper,
            int seed, int disttype )
{
    CX_FUNCNAME( "cxRandInit" );

    __BEGIN__;

    if( !state )
        CX_ERROR( CX_StsNullPtr, "" );

    if( disttype != CX_RAND_UNI && disttype != CX_RAND_NORMAL )
        CX_ERROR( CX_StsBadFlag, "Unknown distribution type" );

    state->state = (uint64)(seed ? seed : UINT_MAX);
    state->disttype = disttype;
    CX_CALL( cxRandSetRange( state, lower, upper ));

    __END__;
}


#define ICX_IMPL_RAND_BITS( flavor, arrtype, cast_macro )               \
static CxStatus CX_STDCALL                                              \
icxRandBits_##flavor##_C1R( arrtype* arr, int step, CxSize size,        \
                            uint64* state, const int* param )           \
{                                                                       \
    uint64 temp = *state;                                               \
    int small_flag = (param[12]|param[13]|param[14]|param[15]) <= 255;  \
                                                                        \
    for( ; size.height--; (char*&)arr += step )                         \
    {                                                                   \
        int i, k = 3;                                                   \
        const int* p = param;                                           \
                                                                        \
        if( !small_flag )                                               \
        {                                                               \
            for( i = 0; i <= size.width - 4; i += 4 )                   \
            {                                                           \
                unsigned t0, t1;                                        \
                                                                        \
                temp = ICX_RNG_NEXT(temp);                              \
                t0 = ((unsigned)temp & p[i + 12]) + p[i];               \
                temp = ICX_RNG_NEXT(temp);                              \
                t1 = ((unsigned)temp & p[i + 13]) + p[i+1];             \
                arr[i] = cast_macro((int)t0);                           \
                arr[i+1] = cast_macro((int)t1);                         \
                                                                        \
                temp = ICX_RNG_NEXT(temp);                              \
                t0 = ((unsigned)temp & p[i + 14]) + p[i+2];             \
                temp = ICX_RNG_NEXT(temp);                              \
                t1 = ((unsigned)temp & p[i + 15]) + p[i+3];             \
                arr[i+2] = cast_macro((int)t0);                         \
                arr[i+3] = cast_macro((int)t1);                         \
                                                                        \
                if( !--k )                                              \
                {                                                       \
                    k = 3;                                              \
                    p -= 12;                                            \
                }                                                       \
            }                                                           \
        }                                                               \
        else                                                            \
        {                                                               \
            for( i = 0; i <= size.width - 4; i += 4 )                   \
            {                                                           \
                unsigned t0, t1, t;                                     \
                                                                        \
                temp = ICX_RNG_NEXT(temp);                              \
                t = (unsigned)temp;                                     \
                t0 = (t & p[i + 12]) + p[i];                            \
                t1 = ((t >> 8) & p[i + 13]) + p[i+1];                   \
                arr[i] = cast_macro((int)t0);                           \
                arr[i+1] = cast_macro((int)t1);                         \
                                                                        \
                t0 = ((t >> 16) & p[i + 14]) + p[i + 2];                \
                t1 = ((t >> 24) & p[i + 15]) + p[i + 3];                \
                arr[i+2] = cast_macro((int)t0);                         \
                arr[i+3] = cast_macro((int)t1);                         \
                                                                        \
                if( !--k )                                              \
                {                                                       \
                    k = 3;                                              \
                    p -= 12;                                            \
                }                                                       \
            }                                                           \
        }                                                               \
                                                                        \
        for( ; i < size.width; i++ )                                    \
        {                                                               \
            unsigned t0;                                                \
            temp = ICX_RNG_NEXT(temp);                                  \
                                                                        \
            t0 = ((unsigned)temp & p[i + 12]) + p[i];                   \
            arr[i] = cast_macro((int)t0);                               \
        }                                                               \
    }                                                                   \
                                                                        \
    *state = temp;                                                      \
    return CX_OK;                                                       \
}


#define ICX_IMPL_RAND( flavor, arrtype, worktype, cast_macro1, cast_macro2 )\
static CxStatus CX_STDCALL                                              \
icxRand_##flavor##_C1R( arrtype* arr, int step, CxSize size,            \
                        uint64* state, const double* param )            \
{                                                                       \
    uint64 temp = *state;                                               \
                                                                        \
    for( ; size.height--; (char*&)arr += step )                         \
    {                                                                   \
        int i, k = 3;                                                   \
        const double* p = param;                                        \
                                                                        \
        for( i = 0; i <= size.width - 4; i += 4 )                       \
        {                                                               \
            worktype f0, f1;                                            \
            unsigned t0, t1;                                            \
                                                                        \
            temp = ICX_RNG_NEXT(temp);                                  \
            t0 = ICX_CXT_FLT(temp);                                     \
            temp = ICX_RNG_NEXT(temp);                                  \
            t1 = ICX_CXT_FLT(temp);                                     \
            f0 = cast_macro1( (float&)t0 * p[i + 12] + p[i] );          \
            f1 = cast_macro1( (float&)t1 * p[i + 13] + p[i + 1] );      \
            arr[i] = cast_macro2(f0);                                   \
            arr[i+1] = cast_macro2(f1);                                 \
                                                                        \
            temp = ICX_RNG_NEXT(temp);                                  \
            t0 = ICX_CXT_FLT(temp);                                     \
            temp = ICX_RNG_NEXT(temp);                                  \
            t1 = ICX_CXT_FLT(temp);                                     \
            f0 = cast_macro1( (float&)t0 * p[i + 14] + p[i + 2] );      \
            f1 = cast_macro1( (float&)t1 * p[i + 15] + p[i + 3] );      \
            arr[i+2] = cast_macro2(f0);                                 \
            arr[i+3] = cast_macro2(f1);                                 \
                                                                        \
            if( !--k )                                                  \
            {                                                           \
                k = 3;                                                  \
                p -= 12;                                                \
            }                                                           \
        }                                                               \
                                                                        \
        for( ; i < size.width; i++ )                                    \
        {                                                               \
            worktype f0;                                                \
            unsigned t0;                                                \
                                                                        \
            temp = ICX_RNG_NEXT(temp);                                  \
            t0 = ICX_CXT_FLT(temp);                                     \
            f0 = cast_macro1( (float&)t0 * p[i + 12] + p[i] );          \
            arr[i] = cast_macro2(f0);                                   \
        }                                                               \
    }                                                                   \
                                                                        \
    *state = temp;                                                      \
    return CX_OK;                                                       \
}


static CxStatus CX_STDCALL
icxRand_64f_C1R( double* arr, int step, CxSize size,
                 uint64* state, const double* param )
{
    uint64 temp = *state;

    for( ; size.height--; (char*&)arr += step )
    {
        int i, k = 3;
        const double* p = param;

        for( i = 0; i <= size.width - 4; i += 4 )
        {
            double f0, f1;
            uint64 t0, t1;

            temp = ICX_RNG_NEXT(temp);
            t0 = ICX_CXT_DBL(temp);
            temp = ICX_RNG_NEXT(temp);
            t1 = ICX_CXT_DBL(temp);
            f0 = (double&)t0 * p[i + 12] + p[i];
            f1 = (double&)t1 * p[i + 13] + p[i + 1];
            arr[i] = f0;
            arr[i+1] = f1;

            temp = ICX_RNG_NEXT(temp);
            t0 = ICX_CXT_DBL(temp);
            temp = ICX_RNG_NEXT(temp);
            t1 = ICX_CXT_DBL(temp);
            f0 = (double&)t0 * p[i + 14] + p[i + 2];
            f1 = (double&)t1 * p[i + 15] + p[i + 3];
            arr[i+2] = f0;
            arr[i+3] = f1;

            if( !--k )
            {
                k = 3;
                p -= 12;
            }
        }

        for( ; i < size.width; i++ )
        {
            double f0;
            uint64 t0;

            temp = ICX_RNG_NEXT(temp);
            t0 = ICX_CXT_DBL(temp);
            f0 = (double&)t0 * p[i + 12] + p[i];
            arr[i] = f0;
        }
    }

    *state = temp;
    return CX_OK;
}


/***************************************************************************************\
    The code below implements algorithm from the paper:

    G. Marsaglia and W.W. Tsang,
    The Monty Python method for generating random variables,
    ACM Transactions on Mathematical Software, Vol. 24, No. 3,
    Pages 341-350, September, 1998.
\***************************************************************************************/

static CxStatus CX_STDCALL
icxRandn_0_1_32f_C1R( float* arr, int len, uint64* state )
{
    uint64 temp = *state;
    int i;

    for( i = 0; i < len; i++ )
    {
        double x;
        for(;;)
        {
            double y, v, ax, bx;

            temp = ICX_RNG_NEXT(temp);
            x=((int)temp)*1.167239e-9;
            ax = fabs(x);

            if( ax < 1.17741 )
                break;

            temp = ICX_RNG_NEXT(temp);
            y=((unsigned)temp)*2.328306e-10;

            v = 2.8658 - ax*(2.0213 - 0.3605*ax);
            if( y < v )
                break;

            bx = x > 0 ? 0.8857913*(2.506628 - x) :
                        -0.8857913*(2.506628 + x);
            
            if( y > v + 0.0506 )
            {
                x = bx;
                break;
            }

            if( log(y) < .6931472 - .5*x*x )
                break;

            x = bx;

            if( log(1.8857913 - y) < .5718733-.5*x*x )
                break;

            do
            {
                temp = ICX_RNG_NEXT(temp);
                v = ((int)temp)*4.656613e-10;
                x = -log(fabs(v))*.3989423;
                temp = ICX_RNG_NEXT(temp);
                y = -log(((unsigned)temp)*2.328306e-10);
            }
            while( y+y < x*x );

            x = v > 0 ? 2.506628 + x : -2.506628 - x;
            break;
        }

        arr[i] = (float)x;
    }

    *state = temp;
    return CX_OK;
}


#define RAND_BUF_SIZE  96


#define ICX_IMPL_RANDN( flavor, arrtype, worktype, cast_macro1, cast_macro2 )   \
static CxStatus CX_STDCALL                                                      \
icxRandn_##flavor##_C1R( arrtype* arr, int step, CxSize size,                   \
                         uint64* state, const double* param )                   \
{                                                                               \
    float buffer[RAND_BUF_SIZE];                                                \
                                                                                \
    for( ; size.height--; (char*&)arr += step )                                 \
    {                                                                           \
        int i, j, len = RAND_BUF_SIZE;                                          \
                                                                                \
        for( i = 0; i < size.width; i += RAND_BUF_SIZE )                        \
        {                                                                       \
            int k = 3;                                                          \
            const double* p = param;                                            \
                                                                                \
            if( i + len > size.width )                                          \
                len = size.width - i;                                           \
                                                                                \
            icxRandn_0_1_32f_C1R( buffer, len, state );                         \
                                                                                \
            for( j = 0; j <= len - 4; j += 4 )                                  \
            {                                                                   \
                worktype f0, f1;                                                \
                                                                                \
                f0 = cast_macro1( buffer[j]*p[j+12] + p[j] );                   \
                f1 = cast_macro1( buffer[j+1]*p[j+13] + p[j+1] );               \
                arr[i+j] = cast_macro2(f0);                                     \
                arr[i+j+1] = cast_macro2(f1);                                   \
                                                                                \
                f0 = cast_macro1( buffer[j+2]*p[j+14] + p[j+2] );               \
                f1 = cast_macro1( buffer[j+3]*p[j+15] + p[j+3] );               \
                arr[i+j+2] = cast_macro2(f0);                                   \
                arr[i+j+3] = cast_macro2(f1);                                   \
                                                                                \
                if( --k == 0 )                                                  \
                {                                                               \
                    k = 3;                                                      \
                    p -= 12;                                                    \
                }                                                               \
            }                                                                   \
                                                                                \
            for( ; j < len; j++ )                                               \
            {                                                                   \
                worktype f0 = cast_macro1( buffer[j]*p[j+12] + p[j] );          \
                arr[i+j] = cast_macro2(f0);                                     \
            }                                                                   \
        }                                                                       \
    }                                                                           \
                                                                                \
    return CX_OK;                                                               \
}


ICX_IMPL_RAND_BITS( 8u, uchar, CX_CAST_8U )
ICX_IMPL_RAND_BITS( 16s, short, CX_CAST_16S )
ICX_IMPL_RAND_BITS( 32s, int, CX_CAST_32S )

ICX_IMPL_RAND( 8u, uchar, int, cxFloor, CX_CAST_8U )
ICX_IMPL_RAND( 16s, short, int, cxFloor, CX_CAST_16S )
ICX_IMPL_RAND( 32s, int, int, cxFloor, CX_CAST_32S )
ICX_IMPL_RAND( 32f, float, float, CX_CAST_32F, CX_NOP )

ICX_IMPL_RANDN( 8u, uchar, int, cxRound, CX_CAST_8U )
ICX_IMPL_RANDN( 16s, short, int, cxRound, CX_CAST_16S )
ICX_IMPL_RANDN( 32s, int, int, cxRound, CX_CAST_32S )
ICX_IMPL_RANDN( 32f, float, float, CX_CAST_32F, CX_NOP )
ICX_IMPL_RANDN( 64f, double, double, CX_CAST_64F, CX_NOP )

static void icxInitRandTable( CxFuncTable* fastrng_tab,
                              CxFuncTable* rng_tab,
                              CxFuncTable* normal_tab )
{
    fastrng_tab->fn_2d[CX_8U] = (void*)icxRandBits_8u_C1R;
    fastrng_tab->fn_2d[CX_8S] = 0;
    fastrng_tab->fn_2d[CX_16S] = (void*)icxRandBits_16s_C1R;
    fastrng_tab->fn_2d[CX_32S] = (void*)icxRandBits_32s_C1R;

    rng_tab->fn_2d[CX_8U] = (void*)icxRand_8u_C1R;
    rng_tab->fn_2d[CX_8S] = 0;
    rng_tab->fn_2d[CX_16S] = (void*)icxRand_16s_C1R;
    rng_tab->fn_2d[CX_32S] = (void*)icxRand_32s_C1R;
    rng_tab->fn_2d[CX_32F] = (void*)icxRand_32f_C1R;
    rng_tab->fn_2d[CX_64F] = (void*)icxRand_64f_C1R;

    normal_tab->fn_2d[CX_8U] = (void*)icxRandn_8u_C1R;
    normal_tab->fn_2d[CX_8S] = 0;
    normal_tab->fn_2d[CX_16S] = (void*)icxRandn_16s_C1R;
    normal_tab->fn_2d[CX_32S] = (void*)icxRandn_32s_C1R;
    normal_tab->fn_2d[CX_32F] = (void*)icxRandn_32f_C1R;
    normal_tab->fn_2d[CX_64F] = (void*)icxRandn_64f_C1R;
}


CX_IMPL void
cxRand( CxRandState* state, CxArr* arr )
{
    static CxFuncTable rng_tab[2], fastrng_tab;
    static int inittab = 0;

    CX_FUNCNAME( "cxRand" );

    __BEGIN__;

    int is_nd = 0;
    CxMat stub, *mat = (CxMat*)arr;
    int type, depth, channels;
    double dparam[2][12];
    int iparam[2][12];
    void* param = dparam;
    int i, fast_int_mode = 0;
    int mat_step = 0;
    CxSize size;
    CxFunc2D_1A2P func = 0;
    CxMatND stub_nd;
    CxNArrayIterator iterator_state, *iterator = 0;

    if( !inittab )
    {
        icxInitRandTable( &fastrng_tab, &rng_tab[CX_RAND_UNI],
                          &rng_tab[CX_RAND_NORMAL] );
        inittab = 1;
    }

    if( !state )
        CX_ERROR_FROM_CODE( CX_StsNullPtr );

    if( CX_IS_MATND(mat) )
    {
        iterator = &iterator_state;
        CX_CALL( cxInitNArrayIterator( 1, (void**)&mat, 0, &stub_nd, iterator ));
        type = CX_MAT_TYPE(iterator->hdr[0]->type);
        size = iterator->size;
        is_nd = 1;
    }
    else
    {
        if( !CX_IS_MAT(mat))
        {
            int coi = 0;
            CX_CALL( mat = cxGetMat( mat, &stub, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "COI is not supported" );
        }

        type = CX_MAT_TYPE( mat->type );
        size = cxGetMatSize( mat );
        mat_step = mat->step;

        if( mat->height > 1 && CX_IS_MAT_CONT( mat->type ))
        {
            size.width *= size.height;
            mat_step = CX_STUB_STEP;
            size.height = 1;
        }
    }

    depth = CX_MAT_DEPTH( type );
    channels = CX_MAT_CN( type );
    size.width *= channels;

    if( state->disttype == CX_RAND_UNI )
    {
        if( depth <= CX_32S )
        {
            for( i = 0, fast_int_mode = 1; i < channels; i++ )
            {
                int t0 = iparam[0][i] = cxCeil( state->param[0].val[i] );
                int t1 = iparam[1][i] = cxFloor( state->param[1].val[i] ) - t0;

                fast_int_mode &= (t1 & (t1 - 1)) == 0;
            }
        }

        if( fast_int_mode )
        {
            for( i = 0; i < channels; i++ )
                iparam[1][i]--;
        
            for( ; i < 12; i++ )
            {
                int t0 = iparam[0][i - channels];
                int t1 = iparam[1][i - channels];

                iparam[0][i] = t0;
                iparam[1][i] = t1;
            }

            CX_GET_FUNC_PTR( func, (CxFunc2D_1A2P)(fastrng_tab.fn_2d[depth]));
            param = iparam;
        }
        else
        {
            for( i = 0; i < channels; i++ )
            {
                double t0 = state->param[0].val[i];
                double t1 = state->param[1].val[i];

                dparam[0][i] = t0 - (t1 - t0);
                dparam[1][i] = t1 - t0;
            }

            CX_GET_FUNC_PTR( func, (CxFunc2D_1A2P)(rng_tab[0].fn_2d[depth]));
        }
    }
    else if( state->disttype == CX_RAND_NORMAL )
    {
        for( i = 0; i < channels; i++ )
        {
            double t0 = state->param[0].val[i];
            double t1 = state->param[1].val[i];

            dparam[0][i] = t1;
            dparam[1][i] = t0;
        }

        CX_GET_FUNC_PTR( func, (CxFunc2D_1A2P)(rng_tab[1].fn_2d[depth]));
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "Unknown distribution type" );
    }

    if( !fast_int_mode )
    {
        for( i = channels; i < 12; i++ )
        {
            double t0 = dparam[0][i - channels];
            double t1 = dparam[1][i - channels];

            dparam[0][i] = t0;
            dparam[1][i] = t1;
        }
    }

    if( !is_nd )
    {
        IPPI_CALL( func( mat->data.ptr, mat_step, size, &(state->state), param ));
    }
    else
    {
        do
        {
            IPPI_CALL( func( iterator->ptr[0], CX_STUB_STEP, size,
                             &(state->state), param ));
        }
        while( cxNextNArraySlice( iterator ));
    }

    __END__;
}

/* End of file. */





