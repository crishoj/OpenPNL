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

typedef struct CxPoint2D64f
{
    double x, y;
}
CxPoint2D64f;


/****************************************************************************************\
                               Discrete Fourier Transform
\****************************************************************************************/


/****************************************************************************************\
    Power-2 DFT in this file is a translation to C of Sorensen's split radix FFT.
    See comments below.
\****************************************************************************************/

static int icxlog2( int n )
{
    int m = 0;
    while(n>1)
        m++, n >>= 1;
    return m;
}


const double icxDxtTab[][2] =
{
{ 1.00000000000000000, 0.00000000000000000 },
{-1.00000000000000000, 0.00000000000000000 },
{ 0.00000000000000000, 1.00000000000000000 },
{ 0.70710678118654757, 0.70710678118654746 },
{ 0.92387953251128674, 0.38268343236508978 },
{ 0.98078528040323043, 0.19509032201612825 },
{ 0.99518472667219693, 0.09801714032956060 },
{ 0.99879545620517241, 0.04906767432741802 },
{ 0.99969881869620425, 0.02454122852291229 },
{ 0.99992470183914450, 0.01227153828571993 },
{ 0.99998117528260111, 0.00613588464915448 },
{ 0.99999529380957619, 0.00306795676296598 },
{ 0.99999882345170188, 0.00153398018628477 },
{ 0.99999970586288223, 0.00076699031874270 },
{ 0.99999992646571789, 0.00038349518757140 },
{ 0.99999998161642933, 0.00019174759731070 },
{ 0.99999999540410733, 0.00009587379909598 },
{ 0.99999999885102686, 0.00004793689960307 },
{ 0.99999999971275666, 0.00002396844980842 },
{ 0.99999999992818922, 0.00001198422490507 },
{ 0.99999999998204725, 0.00000599211245264 },
{ 0.99999999999551181, 0.00000299605622633 },
{ 0.99999999999887801, 0.00000149802811317 },
{ 0.99999999999971945, 0.00000074901405658 },
{ 0.99999999999992983, 0.00000037450702829 },
{ 0.99999999999998246, 0.00000018725351415 },
{ 0.99999999999999567, 0.00000009362675707 },
{ 0.99999999999999889, 0.00000004681337854 },
{ 0.99999999999999978, 0.00000002340668927 },
{ 0.99999999999999989, 0.00000001170334463 },
{ 1.00000000000000000, 0.00000000585167232 },
{ 1.00000000000000000, 0.00000000292583616 }
};

static unsigned char icxRevTable[] =
{
  0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0,
  0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8,
  0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4,
  0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,
  0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2,
  0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
  0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
  0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
  0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
  0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
  0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
  0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
  0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
  0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
  0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
  0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff
};

#define icxBitRev(i,shift) \
   ((int)((((unsigned)icxRevTable[(i)&255] << 24)+ \
           ((unsigned)icxRevTable[((i)>> 8)&255] << 16)+ \
           ((unsigned)icxRevTable[((i)>>16)&255] <<  8)+ \
           ((unsigned)icxRevTable[((i)>>24)])) >> (shift)))


static CxStatus icxInitBitRevTab( int* itab, int m )
{
    int i, n = (1 << m) >> 2, s = 34 - m;
    for( i = 0; i < n; i++ )
        itab[i] = icxBitRev(i,s)*4;
    return CX_OK;
}

#define ICX_BITREV_FUNC( flavor, datatype )                         \
static CxStatus icxBitRev_##flavor( datatype* v, int m, int* itab ) \
{                                                                   \
    datatype t;                                                     \
    int  n = 1 << m;                                                \
    int  j, k, l = n>>2;                                            \
    int  s = 34 - m;                                                \
    datatype *tmp0 = v, *tmp1 = v+l, *tmp2 = v+l*2, *tmp3 = v+l*3;  \
                                                                    \
    if( n >= 16 )                                                   \
    {                                                               \
        for( j = 0; j < l; j += 4 )                                 \
        {                                                           \
            k = itab ? itab[j] : icxBitRev(j,s)*4;                  \
                                                                    \
            CX_SWAP(tmp0[j+2],tmp1[k],t);                           \
            CX_SWAP(tmp0[j+1],tmp2[k],t);                           \
            CX_SWAP(tmp0[j+3],tmp3[k],t);                           \
            CX_SWAP(tmp1[j+1],tmp2[k+2],t);                         \
            CX_SWAP(tmp1[j+3],tmp3[k+2],t);                         \
            CX_SWAP(tmp2[j+3],tmp3[k+1],t);                         \
            if( k > j )                                             \
            {                                                       \
                CX_SWAP(tmp0[j  ],tmp0[k  ],t);                     \
                CX_SWAP(tmp1[j+2],tmp1[k+2],t);                     \
                CX_SWAP(tmp2[j+1],tmp2[k+1],t);                     \
                CX_SWAP(tmp3[j+3],tmp3[k+3],t);                     \
            }                                                       \
        }                                                           \
    }                                                               \
    else if( n > 2 )                                                \
    {                                                               \
        CX_SWAP(tmp0[1],tmp0[n>>1],t);                              \
        if( n == 8 )                                                \
            CX_SWAP(tmp0[3],tmp0[6],t);                             \
    }                                                               \
    return CX_OK;                                                   \
}


ICX_BITREV_FUNC( 32fc, CxPoint2D32f )
ICX_BITREV_FUNC( 64fc, CxPoint2D64f )

/*
CC=================================================================CC
CC                                                                 CC
CC  Subroutine CFFTSR(X,Y,M):                                      CC
CC      An in-place, split-radix complex FFT program               CC
CC      Decimation-in-frequency, cos/sin in second loop            CC
CC      and is computed recursively                                CC
CC      The program is based on Tran ASSP Feb 1986 pp152-156       CC
CC                                                                 CC
CC  Input/output                                                   CC
CC      X    Array of real part of input/output (length >= n)      CC
CC      Y    Array of imaginary part of input/output (length >= n) CC
CC      M    Transform length is n=2**M                            CC
CC                                                                 CC
CC  Calls:                                                         CC
CC      CSTAGE,CBITREV                                             CC
CC                                                                 CC
CC  Author:                                                        CC
CC      H.V. Sorensen,   University of Pennsylvania,  Dec. 1984    CC
CC                       Arpa address: hvs@ee.upenn.edu            CC
CC  Modified:                                                      CC
CC      H.V. Sorensen,   University of Pennsylvania,  Jul. 1987    CC
CC                                                                 CC
CC  Reference:                                                     CC
CC      Sorensen, Heideman, Burrus :"On computing the split-radix  CC
CC      FFT", IEEE Tran. ASSP, Vol. ASSP-34, No. 1, pp. 152-156    CC
CC      Feb. 1986                                                  CC
CC      Mitra&Kaiser: "Digital Signal Processing Handbook, Chap.   CC
CC      8, page 491-610, John Wiley&Sons, 1993                     CC
CC                                                                 CC
CC      This program may be used and distributed freely as         CC
CC      as long as this header is included                         CC
CC                                                                 CC
CC=================================================================CC
*/
#define X1(i)  v[(i)].x
#define Y1(i)  v[(i)].y
#define X2(i)  v[(i)+n4].x
#define Y2(i)  v[(i)+n4].y
#define X3(i)  v3[(i)].x
#define Y3(i)  v3[(i)].y
#define X4(i)  v3[(i)+n4].x
#define Y4(i)  v3[(i)+n4].y
#define CX_SIN_45 0.70710678118654752440084436210485

static CxStatus icxFFT_fwd_32fc( CxPoint2D32f* v, int m, int* itab )
{
    int k, n = 1 << m, n2 = n;
    int is, id, i1, i2;

    /*C-----L shaped butterflies------------------------------------------C*/
    for( k = m - 1; k > 0; k--, n2 >>= 1 )
    {
        int n4 = n2 >> 2;
        int n8 = n4 >> 1;
        CxPoint2D32f* v3 = v + 2*n4;

        /* CSTAGE */
        /*C-------Zero butterfly----------------------------------------------C*/

        for( is = 0, id = 2*n2; is < n; is = 2*id - n2, id *= 4 )
        {
            for( i1 = is; i1 < n; i1 += id )
            {
                double T1, T2;
            
                T1     = X1(i1) - X3(i1);
                X1(i1) = X1(i1) + X3(i1);
                T2     = Y2(i1) - Y4(i1);
                Y2(i1) = Y2(i1) + Y4(i1);
                X3(i1) = (float)(T1 + T2);
                T2     = T1 - T2;
                T1     = X2(i1) - X4(i1);
                X2(i1) = X2(i1) + X4(i1);
                X4(i1) = (float)T2;
                T2     = Y1(i1) - Y3(i1);
                Y1(i1) = Y1(i1) + Y3(i1);
                Y3(i1) = (float)(T2 - T1);
                Y4(i1) = (float)(T2 + T1);
            }
        }

        if( n4 <= 1 )
            continue;

        /*C-------n/8 butterfly-----------------------------------------------C*/
        for( is = 0, id = 2*n2; is < n - 1; is = 2*id - n2, id *= 4 )
        {
            for( i1 = is + n8; i1 < n; i1 += id )
            {
                double T1, T2, T3, T4, T5;

                T1     = X1(i1) - X3(i1);
                X1(i1) = X1(i1) + X3(i1);
                T2     = X2(i1) - X4(i1);
                X2(i1) = X2(i1) + X4(i1);
                T3     = Y1(i1) - Y3(i1);
                Y1(i1) = Y1(i1) + Y3(i1);
                T4     = Y2(i1) - Y4(i1);
                Y2(i1) = Y2(i1) + Y4(i1);
                T5     = (T4 - T1)*CX_SIN_45;
                T1     = (T4 + T1)*CX_SIN_45;
                T4     = (T3 - T2)*CX_SIN_45;
                T2     = (T3 + T2)*CX_SIN_45;
                X3(i1) = (float)(T4 + T1);
                Y3(i1) = (float)(T4 - T1);
                X4(i1) = (float)(T5 + T2);
                Y4(i1) = (float)(T5 - T2);
            }
        }

        if( n8 <= 1 )
            continue;

        /*C-------General butterfly. Two at a time----------------------------C*/
        {
            double SD1 = icxDxtTab[k+1][1], SS1 = SD1;
            double SD3 = 3.*SD1-4.*SD1*SD1*SD1, SS3 = SD3;
            double CD1 = icxDxtTab[k+1][0], CC1 = CD1;
            double CD3 = 4.*CD1*CD1*CD1-3.*CD1, CC3 = CD3;
            double TT1, TT3;
            int j, jn;

            for( j = 1; j < n8; j++ )
            {
                jn = n4 - 2*j;
                
                for( is = 0, id = 2*n2; is < n; is = 2*id - n2, id *= 4 )
                {
                    for( i1 = is + j; i1 < n + j; i1 += id )
                    {
                        double T1, T2, T3, T4, T5;

                        T1     = X1(i1) - X3(i1);
                        X1(i1) = X1(i1) + X3(i1);
                        T2     = X2(i1) - X4(i1);
                        X2(i1) = X2(i1) + X4(i1);
                        T3     = Y1(i1) - Y3(i1);
                        Y1(i1) = Y1(i1) + Y3(i1);
                        T4     = Y2(i1) - Y4(i1);
                        Y2(i1) = Y2(i1) + Y4(i1);
                        T5 = T1 - T4;
                        T1 = T1 + T4;
                        T4 = T2 - T3;
                        T2 = T2 + T3;
                        X3(i1) = (float)( T1*CC1 - T4*SS1);
                        Y3(i1) = (float)(-T4*CC1 - T1*SS1);
                        X4(i1) = (float)( T5*CC3 + T2*SS3);
                        Y4(i1) = (float)( T2*CC3 - T5*SS3);
                        i2 = i1 + jn;
                        T1     = X1(i2) - X3(i2);
                        X1(i2) = X1(i2) + X3(i2);
                        T2     = X2(i2) - X4(i2);
                        X2(i2) = X2(i2) + X4(i2);
                        T3     = Y1(i2) - Y3(i2);
                        Y1(i2) = Y1(i2) + Y3(i2);
                        T4     = Y2(i2) - Y4(i2);
                        Y2(i2) = Y2(i2) + Y4(i2);
                        T5 = T1 - T4;
                        T1 = T1 + T4;
                        T4 = T2 - T3;
                        T2 = T2 + T3;
                        X3(i2) = (float)( T1*SS1 - T4*CC1);
                        Y3(i2) = (float)(-T4*SS1 - T1*CC1);
                        X4(i2) = (float)(-T5*SS3 - T2*CC3);
                        Y4(i2) = (float)(-T2*SS3 + T5*CC3);
                    }
                }

                TT1 = CC1*CD1 - SS1*SD1;
                SS1 = CC1*SD1 + SS1*CD1;
                CC1 = TT1;
                TT3 = CC3*CD3 - SS3*SD3;
                SS3 = CC3*SD3 + SS3*CD3;
                CC3 = TT3;
            }
        }
    }

    /*C-----Length two butterflies----------------------------------------C*/
    for( is = 0, id = 4; is < n; is = 2*id - 2, id = 4*id )
    {
        for( i1 = is; i1 < n - 1; i1 += id )
        {
            double T1;

            T1       = X1(i1);
            X1(i1)   = (float)(T1 + X1(i1+1));
            X1(i1+1) = (float)(T1 - X1(i1+1));
            T1       = Y1(i1);
            Y1(i1)   = (float)(T1 + Y1(i1+1));
            Y1(i1+1) = (float)(T1 - Y1(i1+1));
        }
    }
 
    /*C-------Digit reverse counter---------------------------------------C*/
    icxBitRev_32fc( v, m, itab );

    return CX_OK;
}


static CxStatus icxFFT_fwd_64fc( CxPoint2D64f* v, int m, int* itab )
{
    int k, n = 1 << m, n2 = n;
    int is, id, i1, i2;

    /*C-----L shaped butterflies------------------------------------------C*/
    for( k = m - 1; k > 0; k--, n2 >>= 1 )
    {
        int n4 = n2 >> 2;
        int n8 = n4 >> 1;
        CxPoint2D64f* v3 = v + 2*n4;

        /* CSTAGE */
        /*C-------Zero butterfly----------------------------------------------C*/

        for( is = 0, id = 2*n2; is < n; is = 2*id - n2, id *= 4 )
        {
            for( i1 = is; i1 < n; i1 += id )
            {
                double T1, T2;
            
                T1     = X1(i1) - X3(i1);
                X1(i1) = X1(i1) + X3(i1);
                T2     = Y2(i1) - Y4(i1);
                Y2(i1) = Y2(i1) + Y4(i1);
                X3(i1) = (T1 + T2);
                T2     = T1 - T2;
                T1     = X2(i1) - X4(i1);
                X2(i1) = X2(i1) + X4(i1);
                X4(i1) = T2;
                T2     = Y1(i1) - Y3(i1);
                Y1(i1) = Y1(i1) + Y3(i1);
                Y3(i1) = (T2 - T1);
                Y4(i1) = (T2 + T1);
            }
        }

        if( n4 <= 1 )
            continue;

        /*C-------n/8 butterfly-----------------------------------------------C*/
        for( is = 0, id = 2*n2; is < n - 1; is = 2*id - n2, id *= 4 )
        {
            for( i1 = is + n8; i1 < n; i1 += id )
            {
                double T1, T2, T3, T4, T5;

                T1     = X1(i1) - X3(i1);
                X1(i1) = X1(i1) + X3(i1);
                T2     = X2(i1) - X4(i1);
                X2(i1) = X2(i1) + X4(i1);
                T3     = Y1(i1) - Y3(i1);
                Y1(i1) = Y1(i1) + Y3(i1);
                T4     = Y2(i1) - Y4(i1);
                Y2(i1) = Y2(i1) + Y4(i1);
                T5     = (T4 - T1)*CX_SIN_45;
                T1     = (T4 + T1)*CX_SIN_45;
                T4     = (T3 - T2)*CX_SIN_45;
                T2     = (T3 + T2)*CX_SIN_45;
                X3(i1) = (T4 + T1);
                Y3(i1) = (T4 - T1);
                X4(i1) = (T5 + T2);
                Y4(i1) = (T5 - T2);
            }
        }

        if( n8 <= 1 )
            continue;

        /*C-------General butterfly. Two at a time----------------------------C*/
        {
            double SD1 = icxDxtTab[k+1][1], SS1 = SD1;
            double SD3 = 3.*SD1-4.*SD1*SD1*SD1, SS3 = SD3;
            double CD1 = icxDxtTab[k+1][0], CC1 = CD1;
            double CD3 = 4.*CD1*CD1*CD1-3.*CD1, CC3 = CD3;
            double TT1, TT3;
            int j, jn;

            for( j = 1; j < n8; j++ )
            {
                jn = n4 - 2*j;
                
                for( is = 0, id = 2*n2; is < n; is = 2*id - n2, id *= 4 )
                {
                    for( i1 = is + j; i1 < n + j; i1 += id )
                    {
                        double T1, T2, T3, T4, T5;

                        T1     = X1(i1) - X3(i1);
                        X1(i1) = X1(i1) + X3(i1);
                        T2     = X2(i1) - X4(i1);
                        X2(i1) = X2(i1) + X4(i1);
                        T3     = Y1(i1) - Y3(i1);
                        Y1(i1) = Y1(i1) + Y3(i1);
                        T4     = Y2(i1) - Y4(i1);
                        Y2(i1) = Y2(i1) + Y4(i1);
                        T5 = T1 - T4;
                        T1 = T1 + T4;
                        T4 = T2 - T3;
                        T2 = T2 + T3;
                        X3(i1) = ( T1*CC1 - T4*SS1);
                        Y3(i1) = (-T4*CC1 - T1*SS1);
                        X4(i1) = ( T5*CC3 + T2*SS3);
                        Y4(i1) = ( T2*CC3 - T5*SS3);
                        i2 = i1 + jn;
                        T1     = X1(i2) - X3(i2);
                        X1(i2) = X1(i2) + X3(i2);
                        T2     = X2(i2) - X4(i2);
                        X2(i2) = X2(i2) + X4(i2);
                        T3     = Y1(i2) - Y3(i2);
                        Y1(i2) = Y1(i2) + Y3(i2);
                        T4     = Y2(i2) - Y4(i2);
                        Y2(i2) = Y2(i2) + Y4(i2);
                        T5 = T1 - T4;
                        T1 = T1 + T4;
                        T4 = T2 - T3;
                        T2 = T2 + T3;
                        X3(i2) = ( T1*SS1 - T4*CC1);
                        Y3(i2) = (-T4*SS1 - T1*CC1);
                        X4(i2) = (-T5*SS3 - T2*CC3);
                        Y4(i2) = (-T2*SS3 + T5*CC3);
                    }
                }

                TT1 = CC1*CD1 - SS1*SD1;
                SS1 = CC1*SD1 + SS1*CD1;
                CC1 = TT1;
                TT3 = CC3*CD3 - SS3*SD3;
                SS3 = CC3*SD3 + SS3*CD3;
                CC3 = TT3;
            }
        }
    }

    /*C-----Length two butterflies----------------------------------------C*/
    for( is = 0, id = 4; is < n; is = 2*id - 2, id = 4*id )
    {
        for( i1 = is; i1 < n - 1; i1 += id )
        {
            double T1;

            T1       = X1(i1);
            X1(i1)   = (T1 + X1(i1+1));
            X1(i1+1) = (T1 - X1(i1+1));
            T1       = Y1(i1);
            Y1(i1)   = (T1 + Y1(i1+1));
            Y1(i1+1) = (T1 - Y1(i1+1));
        }
    }
 
    /*C-------Digit reverse counter---------------------------------------C*/
    icxBitRev_64fc( v, m, itab );

    return CX_OK;
}

#undef X1
#undef Y1
#undef X2
#undef Y2
#undef X3
#undef Y3
#undef X4
#undef Y4


/* FFT of real vector */
/* output vector format: */
/* re[0], re[1], im[1], ... , re[n/2-1], im[n/2-1], re[n/2] */
#define ICX_REAL_FWD_AND_CCS_INV_FFT_FUNCS( flavor, datatype )              \
static CxStatus icxFFT_fwd_##flavor( datatype *v, int m, int* itab )        \
{                                                                           \
    if( m == 0 )                                                            \
    {                                                                       \
    }                                                                       \
    else if( m == 1 )                                                       \
    {                                                                       \
        double t = v[0] + v[1];                                             \
        v[1] = v[0] - v[1];                                                 \
        v[0] = (datatype)t;                                                 \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        double w1_re, w1_im, wi_re, wi_im, t0, t;                           \
        int n = 1 << m;                                                     \
        int n2 = n >> 1;                                                    \
        int j;                                                              \
                                                                            \
        icxFFT_fwd_##flavor##c( (CxPoint2D##flavor*)v, m - 1, itab );       \
                                                                            \
        t = v[0] - v[1];                                                    \
        v[0] = v[0] + v[1];                                                 \
        v[1] = (datatype)t;                                                 \
                                                                            \
        wi_re = w1_re = icxDxtTab[m][0];                                    \
        wi_im = w1_im = -icxDxtTab[m][1];                                   \
                                                                            \
        t0 = v[n2];                                                         \
        t = v[n-1];                                                         \
        v[n-1] = v[1];                                                      \
                                                                            \
        for( j = 2; j < n2; j += 2 )                                        \
        {                                                                   \
            double h1_re, h1_im, h2_re, h2_im;                              \
                                                                            \
            /* calc even */                                                 \
            h1_re = 0.5*(v[j] + v[n-j]);                                    \
            h1_im = 0.5*(v[j+1] - t);                                       \
                                                                            \
            /* calc odd */                                                  \
            h2_re = 0.5*(v[j+1] + t);                                       \
            h2_im = 0.5*(v[n-j] - v[j]);                                    \
                                                                            \
            /* rotate */                                                    \
            t = h2_re*wi_re - h2_im*wi_im;                                  \
            h2_im = h2_re*wi_im + h2_im*wi_re;                              \
            h2_re = t;                                                      \
                                                                            \
            t = wi_re*w1_re - wi_im*w1_im;                                  \
            wi_im = wi_re*w1_im + wi_im*w1_re;                              \
            wi_re = t;                                                      \
                                                                            \
            t = v[n-j-1];                                                   \
                                                                            \
            v[j-1] = (datatype)(h1_re + h2_re);                             \
            v[n-j-1] = (datatype)(h1_re - h2_re);                           \
            v[j] = (datatype)(h1_im + h2_im);                               \
            v[n-j] = (datatype)(h2_im - h1_im);                             \
        }                                                                   \
                                                                            \
        v[n2-1] = (datatype)t0;                                             \
        v[n2] = (datatype)-t;                                               \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}                                                                           \
                                                                            \
/* FFT of complex conjugate-symmetric vector */                             \
/* input vector format: */                                                  \
/* re[0], re[1], im[1], ... , re[n/2-1], im[n/2-1], re[n/2] */              \
static CxStatus icxFFT_inv_##flavor( datatype *v, int m, int* itab )        \
{                                                                           \
    if( m == 0 )                                                            \
    {                                                                       \
    }                                                                       \
    else if( m == 1 )                                                       \
    {                                                                       \
        double t = v[0] + v[1];                                             \
        v[1] = v[0] - v[1];                                                 \
        v[0] = (datatype)t;                                                 \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        double w1_re, w1_im, wi_re, wi_im, t0, t;                           \
        int n = 1 << m;                                                     \
        int n2 = n >> 1;                                                    \
        int j;                                                              \
                                                                            \
        t = v[1];                                                           \
        t0 = (v[0] + v[n-1]);                                               \
        v[1] = (datatype)(v[n-1] - v[0]);                                   \
        v[0] = (datatype)t0;                                                \
                                                                            \
        wi_re = w1_re = icxDxtTab[m][0];                                    \
        wi_im = w1_im = icxDxtTab[m][1];                                    \
                                                                            \
        for( j = 2; j < n2; j += 2 )                                        \
        {                                                                   \
            double h1_re, h1_im, h2_re, h2_im;                              \
                                                                            \
            h1_re = (t + v[n-j-1]);                                         \
            h1_im = (v[j] - v[n-j]);                                        \
                                                                            \
            h2_re = (t - v[n-j-1]);                                         \
            h2_im = (v[j] + v[n-j]);                                        \
                                                                            \
            t = h2_re*wi_re - h2_im*wi_im;                                  \
            h2_im = h2_re*wi_im + h2_im*wi_re;                              \
            h2_re = t;                                                      \
                                                                            \
            t = wi_re*w1_re - wi_im*w1_im;                                  \
            wi_im = wi_re*w1_im + wi_im*w1_re;                              \
            wi_re = t;                                                      \
                                                                            \
            t = v[j+1];                                                     \
            v[j] = (datatype)(h1_re - h2_im);                               \
            v[j+1] = -(datatype)(h1_im + h2_re);                            \
                                                                            \
            v[n-j] = (datatype)(h1_re + h2_im);                             \
            v[n-j+1]= (datatype)(h1_im - h2_re);                            \
        }                                                                   \
                                                                            \
        v[n2+1] = (datatype)(v[n2]*2);                                      \
        v[n2] = (datatype)(t*2);                                            \
                                                                            \
        icxFFT_fwd_##flavor##c( (CxPoint2D##flavor*)v, m-1, itab );         \
        for( j = 0; j < n; j += 2 )                                         \
            v[j+1] = -v[j+1];                                               \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}


ICX_REAL_FWD_AND_CCS_INV_FFT_FUNCS( 32f, float )
ICX_REAL_FWD_AND_CCS_INV_FFT_FUNCS( 64f, double )


#define ICX_DFT_FUNCS( flavor, complex_datatype, real_datatype )            \
/* simple (direct) DFT of arbitrary size complex vector (fwd or inv) */     \
static CxStatus icxDFT_##flavor##c( const complex_datatype* src,            \
                                    complex_datatype* dst, int n, int inv ) \
{                                                                           \
    double scale = CX_PI*2/n*(inv ? 1 : -1);                                \
    double c00 = cos(scale);                                                \
    double s00 = sin(scale);                                                \
    double c0 = c00, s0 = s00;                                              \
    int i, j, n2 = (n+1) >> 1;                                              \
                                                                            \
    if( n % 2 == 0 )                                                        \
    {                                                                       \
        double s0_re = 0, s0_im = 0;                                        \
        double s1_re = 0, s1_im = 0;                                        \
        complex_datatype p, m;                                              \
                                                                            \
        p.x = src[0].x + src[n2].x;                                         \
        m.x = src[0].x - src[n2].x;                                         \
        p.y = src[0].y + src[n2].y;                                         \
        m.y = src[0].y - src[n2].y;                                         \
                                                                            \
        for( i = 0; i < n; i += 2 )                                         \
        {                                                                   \
            double t0, t1;                                                  \
                                                                            \
            t0 = src[i].x;                                                  \
            t1 = src[i+1].x;                                                \
                                                                            \
            s0_re += t0 + t1;                                               \
            s1_re += t0 - t1;                                               \
                                                                            \
            t0 = src[i].y;                                                  \
            t1 = src[i+1].y;                                                \
                                                                            \
            s0_im += t0 + t1;                                               \
            s1_im += t0 - t1;                                               \
                                                                            \
            dst[i] = p;                                                     \
            dst[i+1] = m;                                                   \
        }                                                                   \
                                                                            \
        dst[0].x = (real_datatype)s0_re;                                    \
        dst[0].y = (real_datatype)s0_im;                                    \
        dst[n2].x = (real_datatype)s1_re;                                   \
        dst[n2].y = (real_datatype)s1_im;                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        double s_re = 0, s_im = 0;                                          \
        complex_datatype p0 = src[0];                                       \
                                                                            \
        for( i = 0; i < n; i++ )                                            \
        {                                                                   \
            s_re += src[i].x;                                               \
            s_im += src[i].y;                                               \
            dst[i] = p0;                                                    \
        }                                                                   \
                                                                            \
        dst[0].x = (real_datatype)s_re;                                     \
        dst[0].y = (real_datatype)s_im;                                     \
    }                                                                       \
                                                                            \
    for( i = 1; i < n2; i++ )                                               \
    {                                                                       \
        double s0_re = dst[i].x, s0_im = dst[i].y;                          \
        double s1_re = dst[n-i].x, s1_im = dst[n-i].y;                      \
        double c = c0, s = s0, t;                                           \
                                                                            \
        for( j = 1; j < n2; j++ )                                           \
        {                                                                   \
            double t0 = c*(src[j].x + src[n-j].x);                          \
            double t1 = s*(src[j].y - src[n-j].y);                          \
                                                                            \
            s0_re += t0 - t1;                                               \
            s1_re += t0 + t1;                                               \
                                                                            \
            t0 = c*(src[j].y + src[n-j].y);                                 \
            t1 = s*(src[j].x - src[n-j].x);                                 \
                                                                            \
            s0_im += t0 + t1;                                               \
            s1_im += t0 - t1;                                               \
                                                                            \
            t = c*c0 - s*s0;                                                \
            s = s*c0 + c*s0;                                                \
            c = t;                                                          \
        }                                                                   \
                                                                            \
        dst[i].x = (real_datatype)s0_re;                                    \
        dst[i].y = (real_datatype)s0_im;                                    \
        dst[n-i].x = (real_datatype)s1_re;                                  \
        dst[n-i].y = (real_datatype)s1_im;                                  \
                                                                            \
        t = c0*c00 - s0*s00;                                                \
        s0 = s0*c00 + c0*s00;                                               \
        c0 = t;                                                             \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}                                                                           \
                                                                            \
                                                                            \
/* simple (direct) forward DFT of arbitrary size real vector */             \
static CxStatus icxDFT_fwd_##flavor( const real_datatype* src,              \
                                     real_datatype* dst, int n, int )       \
{                                                                           \
    double scale = -CX_PI*2/n;                                              \
    double c00 = cos(scale);                                                \
    double s00 = sin(scale);                                                \
    double c0 = c00, s0 = s00;                                              \
    int i, j, n2 = (n+1) >> 1;                                              \
                                                                            \
    if( n % 2 == 0 )                                                        \
    {                                                                       \
        double s0 = 0, s1 = 0;                                              \
        real_datatype p, m;                                                 \
                                                                            \
        p = src[0] + src[n2];                                               \
        m = src[0] - src[n2];                                               \
                                                                            \
        for( i = 0; i < n; i += 2 )                                         \
        {                                                                   \
            double t0, t1;                                                  \
                                                                            \
            t0 = src[i];                                                    \
            t1 = src[i+1];                                                  \
                                                                            \
            s0 += t0 + t1;                                                  \
            s1 += t0 - t1;                                                  \
        }                                                                   \
                                                                            \
        dst[0] = (real_datatype)s0;                                         \
        for( i = 1; i <= n-5; i += 4 )                                      \
        {                                                                   \
            dst[i] = m;                                                     \
            dst[i+2] = p;                                                   \
        }                                                                   \
                                                                            \
        if( i < n-1 )                                                       \
            dst[i] = m;                                                     \
        dst[n-1] = (real_datatype)s1;                                       \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        double s = 0;                                                       \
        real_datatype p = src[0];                                           \
                                                                            \
        for( i = 0; i < n; i++ )                                            \
            s += src[i];                                                    \
                                                                            \
        dst[0] = (real_datatype)s;                                          \
        for( i = 1; i < n; i += 2 )                                         \
            dst[i] = p;                                                     \
    }                                                                       \
                                                                            \
    for( i = 1; i < n - 1; i += 2 )                                         \
    {                                                                       \
        double s_re = dst[i], s_im = 0;                                     \
        double c = c0, s = s0, t;                                           \
                                                                            \
        for( j = 1; j < n2; j++ )                                           \
        {                                                                   \
            s_re += c*(src[j] + src[n-j]);                                  \
            s_im += s*(src[j] - src[n-j]);                                  \
                                                                            \
            t = c*c0 - s*s0;                                                \
            s = s*c0 + c*s0;                                                \
            c = t;                                                          \
        }                                                                   \
                                                                            \
        dst[i] = (real_datatype)s_re;                                       \
        dst[i+1] = (real_datatype)s_im;                                     \
                                                                            \
        t = c0*c00 - s0*s00;                                                \
        s0 = s0*c00 + c0*s00;                                               \
        c0 = t;                                                             \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}                                                                           \
                                                                            \
                                                                            \
/* simple (direct) inverse DFT of arbitrary size ccs vector */              \
static CxStatus icxDFT_inv_##flavor( const real_datatype* src,              \
                                     real_datatype* dst, int n, int )       \
{                                                                           \
    double scale = CX_PI*2/n;                                               \
    double c00 = cos(scale);                                                \
    double s00 = sin(scale);                                                \
    double c0 = c00, s0 = s00;                                              \
    int i, j, n2 = (n+1) >> 1;                                              \
                                                                            \
    if( n % 2 == 0 )                                                        \
    {                                                                       \
        double s0_re, s1_re;                                                \
        double p, m;                                                        \
                                                                            \
        p = src[0] + src[n-1];                                              \
        m = src[0] - src[n-1];                                              \
                                                                            \
        s0_re = 0;                                                          \
        s1_re = 0;                                                          \
                                                                            \
        for( i = 1; i <= n2-2; i += 2 )                                     \
        {                                                                   \
            double t0, t1;                                                  \
                                                                            \
            t0 = src[i*2-1];                                                \
            t1 = src[i*2+1];                                                \
                                                                            \
            s0_re += t1 + t0;                                               \
            s1_re += t1 - t0;                                               \
                                                                            \
            dst[i] = dst[n-i] = (real_datatype)m;                           \
            dst[i+1] = dst[n-i-1] = (real_datatype)p;                       \
        }                                                                   \
                                                                            \
        s0_re *= 2;                                                         \
        s1_re *= 2;                                                         \
                                                                            \
        if( i < n2 )                                                        \
        {                                                                   \
            s0_re += src[n-3]*2;                                            \
            s1_re += (src[n-1] - src[n-3])*2;                               \
            dst[n2-1] = dst[n2+1] = (real_datatype)m;                       \
        }                                                                   \
                                                                            \
        dst[0] = (real_datatype)(s0_re + p);                                \
        dst[n2] = (real_datatype)(s1_re + m);                               \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        double s_re = 0;                                                    \
        double p0 = src[0];                                                 \
                                                                            \
        for( i = 1; i < n2; i++ )                                           \
        {                                                                   \
            s_re += src[i*2-1];                                             \
            dst[i] = dst[n-i] = (real_datatype)p0;                          \
        }                                                                   \
                                                                            \
        dst[0] = (real_datatype)(s_re*2 + p0);                              \
    }                                                                       \
                                                                            \
    for( i = 1; i < n2; i++ )                                               \
    {                                                                       \
        double s0_re = 0, s1_re = 0;                                        \
        double c = c0, s = s0, t;                                           \
                                                                            \
        for( j = 1; j < n-1; j += 2 )                                       \
        {                                                                   \
            double t0 = c*src[j];                                           \
            double t1 = s*src[j+1];                                         \
                                                                            \
            s0_re += t0 - t1;                                               \
            s1_re += t0 + t1;                                               \
                                                                            \
            t = c*c0 - s*s0;                                                \
            s = s*c0 + c*s0;                                                \
            c = t;                                                          \
        }                                                                   \
                                                                            \
        dst[i] += (real_datatype)(s0_re*2);                                 \
        dst[n-i] += (real_datatype)(s1_re*2);                               \
                                                                            \
        t = c0*c00 - s0*s00;                                                \
        s0 = s0*c00 + c0*s00;                                               \
        c0 = t;                                                             \
    }                                                                       \
                                                                            \
    return CX_OK;                                                           \
}


ICX_DFT_FUNCS( 32f, CxPoint2D32f, float )
ICX_DFT_FUNCS( 64f, CxPoint2D64f, double )

static void
icxConjugate( CxMat* mat )
{
    /*CX_FUNCNAME( "icxConjugate" );*/

    __BEGIN__;

    int j, cols = mat->cols*2, rows = mat->rows;

    if( CX_MAT_DEPTH(mat->type) == CX_32F )
    {
        int* data = (int*)mat->data.ptr;
        for( ; rows-- ; (char*&)data += mat->step )
        {
            for( j = 0; j <= cols - 4; j += 4 )
            {
                int t0 = data[j+1] ^ INT_MIN;
                int t1 = data[j+3] ^ INT_MIN;
                data[j+1] = t0;
                data[j+3] = t1;
            }

            if( j < cols )
                data[j+1] ^= INT_MIN;
        }
    }
    else
    {
        double* data = (double*)mat->data.ptr;
        for( ; rows-- ; (char*&)data += mat->step )
        {
            for( j = 0; j <= cols - 4; j += 4 )
            {
                double t0 = -data[j+1];
                double t1 = -data[j+3];
                data[j+1] = t0;
                data[j+3] = t1;
            }

            if( j < cols )
                data[j+1] = -data[j+1];
        }
    }

    __END__;
}


static void
icxScale( CxMat* mat, double _re_scale, double _im_scale )
{
    /*CX_FUNCNAME( "icxScale" );*/

    __BEGIN__;

    int j, cols = mat->cols*CX_MAT_CN(mat->type), rows = mat->rows;
    int is_1d = rows == 1 || (cols == 1 && CX_IS_MAT_CONT(mat->type));

    if( CX_MAT_DEPTH(mat->type) == CX_32F )
    {
        float re_scale = (float)_re_scale;
        float im_scale = (float)_im_scale;
        float* data = (float*)mat->data.ptr;

        if( is_1d )
        {
            int len = cols + rows - 1;

            if( CX_MAT_CN(mat->type) == 1 )
            {
                data[0] *= re_scale;
                if( len % 2 == 0 )
                    data[len-1] *= re_scale;
                data += 1;
                len = (len - 1) & -2;
            }

            for( j = 0; j < len; j += 2 )
            {
                float t0 = data[j]*re_scale;
                float t1 = data[j+1]*im_scale;

                data[j] = t0;
                data[j+1] = t1;
            }
        }
        else
        {
            int step = mat->step/sizeof(float);

            if( CX_MAT_CN(mat->type) == 1 )
            {
                data[0] *= re_scale;
                if( rows % 2 == 0 )
                    data[(rows-1)*step] *= re_scale;

                for( j = 1; j <= rows - 2; j += 2 )
                {
                    data[j*step] *= re_scale;
                    data[(j+1)*step] *= im_scale;
                }

                if( cols % 2 == 0 )
                {
                    data[cols - 1] *= re_scale;
                    if( rows % 2 == 0 )
                        data[(rows-1)*step + cols-1] *= re_scale;

                    for( j = 1; j <= rows - 2; j += 2 )
                    {
                        data[j*step + cols-1] *= re_scale;
                        data[(j+1)*step + cols-1] *= im_scale;
                    }
                }

                data += 1;
                cols = (cols - 1) & -2;
            }

            for( ; rows--; data += step )
            {
                for( j = 0; j < cols; j += 2 )
                {
                    float t0 = data[j]*re_scale;
                    float t1 = data[j+1]*im_scale;
                    data[j] = t0;
                    data[j+1] = t1;
                }
            }
        }
    }
    else
    {
        double re_scale = _re_scale;
        double im_scale = _im_scale;
        double* data = (double*)mat->data.ptr;

        if( is_1d )
        {
            int len = cols + rows - 1;

            if( CX_MAT_CN(mat->type) == 1 )
            {
                data[0] *= re_scale;
                if( len % 2 == 0 )
                    data[len-1] *= re_scale;
                data += 1;
                len = (len - 1) & -2;
            }

            for( j = 0; j < len; j += 2 )
            {
                double t0 = data[j]*re_scale;
                double t1 = data[j+1]*im_scale;

                data[j] = t0;
                data[j+1] = t1;
            }
        }
        else
        {
            int step = mat->step/sizeof(double);

            if( CX_MAT_CN(mat->type) == 1 )
            {
                data[0] *= re_scale;
                if( rows % 2 == 0 )
                    data[(rows-1)*step] *= re_scale;

                for( j = 1; j <= rows - 2; j += 2 )
                {
                    data[j*step] *= re_scale;
                    data[(j+1)*step] *= im_scale;
                }

                if( cols % 2 == 0 )
                {
                    data[cols - 1] *= re_scale;
                    if( rows % 2 == 0 )
                        data[(rows-1)*step + cols-1] *= re_scale;

                    for( j = 1; j <= rows - 2; j += 2 )
                    {
                        data[j*step + cols-1] *= re_scale;
                        data[(j+1)*step + cols-1] *= im_scale;
                    }
                }

                data += 1;
                cols = (cols - 1) & -2;
            }

            for( ; rows--; data += step )
            {
                for( j = 0; j < cols; j += 2 )
                {
                    double t0 = data[j]*re_scale;
                    double t1 = data[j+1]*im_scale;
                    data[j] = t0;
                    data[j+1] = t1;
                }
            }
        }
    }

    __END__;
}

typedef CxStatus (*CxDXTFastFunc)( void* array, int log_len, int* bitrev_tab );
typedef CxStatus (*CxDXTSlowFunc)( const void* src, void* dst, int len, int inv );

static void icxCopyColumn( const CxMat* src, CxMat* dst, int pix_size )
{
    int i, rows = src->rows;
    int src_step = src->step, dst_step = dst->step;
    uchar *src_data = src->data.ptr, *dst_data = dst->data.ptr;

    for( i = 0; i < rows; i++ )
        memcpy( dst_data + i*dst_step, src_data + i*src_step, pix_size );
}


CX_IMPL void
cxDFT( const CxArr* srcarr, CxArr* dstarr, int flags )
{
    static CxDXTFastFunc fast_tbl[6];
    static CxDXTSlowFunc slow_tbl[6];
    static int inittab = 0;
    
    uchar* buffer = 0;
    int local_alloc = 0;

    int* itab = 0;
    int local_itab_alloc = 0;
    
    CX_FUNCNAME( "cxDFT" );

    __BEGIN__;

    double re_scale = 1, im_scale = 1;
    CxMat srcstub, *src = (CxMat*)srcarr;
    CxMat dststub, *dst = (CxMat*)dstarr;
    CxMat buf, *dst_tmp;
    CxSize size;
    int is_1d;
    int type, pix_size;
    int width_flag, height_flag;
    int tbl_offset;
    int inv = (flags & CX_DXT_INVERSE) != 0;

    if( !inittab )
    {
        fast_tbl[0] = (CxDXTFastFunc)icxFFT_fwd_32fc;
        fast_tbl[1] = (CxDXTFastFunc)icxFFT_fwd_32f;
        fast_tbl[2] = (CxDXTFastFunc)icxFFT_inv_32f;

        fast_tbl[3] = (CxDXTFastFunc)icxFFT_fwd_64fc;
        fast_tbl[4] = (CxDXTFastFunc)icxFFT_fwd_64f;
        fast_tbl[5] = (CxDXTFastFunc)icxFFT_inv_64f;

        slow_tbl[0] = (CxDXTSlowFunc)icxDFT_32fc;
        slow_tbl[1] = (CxDXTSlowFunc)icxDFT_fwd_32f;
        slow_tbl[2] = (CxDXTSlowFunc)icxDFT_inv_32f;

        slow_tbl[3] = (CxDXTSlowFunc)icxDFT_64fc;
        slow_tbl[4] = (CxDXTSlowFunc)icxDFT_fwd_64f;
        slow_tbl[5] = (CxDXTSlowFunc)icxDFT_inv_64f;
        
        inittab = 1;
    }

    if( !CX_IS_MAT( src ))
    {
        int coi = 0;
        CX_CALL( src = cxGetMat( src, &srcstub, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT( dst ))
    {
        int coi = 0;
        CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_ARE_TYPES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    dst_tmp = dst;
    type = CX_MAT_TYPE( dst->type );

    if( CX_MAT_CN(type) > 2 || CX_MAT_DEPTH(type) < CX_32F )
        CX_ERROR( CX_StsUnsupportedFormat,
        "Only 32fC1, 32fC2, 64fC1 and 64fC2 formats are supported" );

    pix_size = icxPixSize[type];
    tbl_offset = (CX_MAT_DEPTH(type)==CX_64F ? 3 : 0);

    size = cxGetMatSize( dst );
    is_1d = size.width == 1 || size.height == 1;

    if( flags & CX_DXT_SCALE )
        re_scale = im_scale = 1./(size.width*size.height);

    width_flag = (size.width&(size.width-1)) == 0;
    height_flag = (size.height&(size.height-1)) == 0;

    if( !CX_IS_MAT_CONT(dst->type) || !is_1d || !width_flag || !height_flag )
    {
        int buffer_size = pix_size*MAX(size.width,size.height)*4;
        if( buffer_size <= CX_MAX_LOCAL_SIZE )
        {
            buffer = (uchar*)alloca( buffer_size );
            local_alloc = 1;
        }
        else
            CX_CALL( buffer = (uchar*)cxAlloc( buffer_size ));
        /* buf is initialized for 1D case, for 2D case it needs to be reinitialized */
        buf = cxMat( size.height, size.width, type, buffer );
        dst_tmp = &buf;
    }

    if( is_1d )
    {
        int n = size.width + size.height - 1;
        if( (n & (n-1)) == 0 )
        {
            int m = icxlog2(n);

            if( dst_tmp->data.ptr != src->data.ptr )
                cxCopy( src, dst_tmp, 0 );

            if( CX_MAT_CN(type) == 2 )
            {
                if( inv )
                {
                    icxConjugate( dst_tmp );
                    im_scale = -re_scale;
                }

                fast_tbl[0 + tbl_offset]( (CxPoint2D32f*)(dst_tmp->data.ptr), m, 0 );
            }
            else
                fast_tbl[inv + 1 + tbl_offset]((float*)(dst_tmp->data.ptr), m, 0);
        }
        else
        {
            CxMat buf2;
            CxMat* src_tmp = src;

            if( CX_IS_MAT_CONT(dst->type))
                dst_tmp = dst;
            if( !CX_IS_MAT_CONT(src->type) || src->data.ptr == dst_tmp->data.ptr )
            {
                buf2 = cxMat( size.height, size.width, type, buffer + n*pix_size );
                src_tmp = &buf2;
                cxCopy( src, src_tmp, 0 );
            }

            if( CX_MAT_CN(type) == 2 )
                slow_tbl[0 + tbl_offset]( (CxPoint2D32f*)(src_tmp->data.ptr),
                                          (CxPoint2D32f*)(dst_tmp->data.ptr), n, inv );
            else
                slow_tbl[inv+1+tbl_offset]( (CxPoint2D32f*)(src_tmp->data.ptr),
                                            (CxPoint2D32f*)(dst_tmp->data.ptr), n, inv );
        }

        if( dst_tmp->data.ptr != dst->data.ptr )
            cxCopy( dst_tmp, dst, 0 );
    }
    else
    {
        CxMat src_part, dst_part, src_buf, dst_buf;
        int i, stage;

        if( width_flag || height_flag )
        {
            int buffer_size = sizeof(int)*(MAX(size.width,size.height)+4)/4;
            if( buffer_size <= CX_MAX_LOCAL_SIZE )
            {
                itab = (int*)alloca( buffer_size );
                local_itab_alloc = 1;
            }
            else
                CX_CALL( itab = (int*)cxAlloc( buffer_size ));
        }

        /* stage is changed as:
           1->4 in case of inverse css transform and
           0->3 in other cases.
           This trick is used because column-wise transform
           needs to be done first in inv_css case and
           row-wise transform needs to be done first in case of forward real transform.
           So the following loop passes these two stages in either order.
           For complex matrix case the order does not matter, however row-wise transform
           done first seems to be better from cache utilization point of view */
        stage = inv && CX_MAT_CN(type) == 1;

        for(;;)
        {
            if( stage % 2 == 0 ) // row transforms
            {
                src_part = cxMat( 1, size.width, type );
                dst_part = src_part;
                src_part.data.ptr = src->data.ptr;
                dst_part.data.ptr = dst->data.ptr;

                if( width_flag )
                {
                    int m = icxlog2(size.width);
                    CxDXTFastFunc fast_func = fast_tbl[(CX_MAT_CN(type)==2?0:inv+1) + tbl_offset];

                    /* for real-value transforms a twice smaller table is used */
                    icxInitBitRevTab( itab, m - (CX_MAT_CN(type)==1) );

                    for( i = 0; i < size.height; i++ )
                    {
                        if( dst_part.data.ptr != src_part.data.ptr )
                            memcpy( dst_part.data.ptr, src_part.data.ptr, size.width * pix_size );

                        if( inv && CX_MAT_CN(type) == 2 )
                            icxConjugate( &dst_part );

                        fast_func( dst_part.data.ptr, m, itab );

                        if( inv && CX_MAT_CN(type) == 2 )
                            icxConjugate( &dst_part );

                        src_part.data.ptr += src->step;
                        dst_part.data.ptr += dst->step;
                    }
                }
                else
                {
                    CxDXTSlowFunc slow_func = slow_tbl[(CX_MAT_CN(type)==2?0:inv+1) + tbl_offset];

                    for( i = 0; i < size.height; i++ )
                    {
                        void* tmp_src = src_part.data.ptr;
                        if( tmp_src == dst_part.data.ptr )
                        {
                            tmp_src = buffer;
                            memcpy( tmp_src, src_part.data.ptr, size.width*pix_size );
                        }

                        slow_func( tmp_src, dst_part.data.ptr, size.width, inv );
                        src_part.data.ptr += src->step;
                        dst_part.data.ptr += dst->step;
                    }
                }

                src = dst;
                if( stage >= 3 )
                    break;
                stage += 3;
            }
            else // column transforms
            {
                int col_pix_size = pix_size;
                
                src_part = cxMat( size.height, 1, type );
                dst_part = src_part;
                src_buf = src_part;
        
                cxSetData( &src_part, src->data.ptr, src->step );
                cxSetData( &dst_part, dst->data.ptr, dst->step );
                cxSetData( &src_buf, buffer, pix_size );

                if( height_flag )
                {
                    int m = icxlog2(size.height), width = size.width;
                    CxDXTFastFunc fast_func = fast_tbl[(CX_MAT_CN(type)==2?0:inv+1) + tbl_offset];

                    /* for real-value transforms a twice smaller table is used */
                    icxInitBitRevTab( itab, m );

                    if( CX_MAT_CN(type) == 1 )
                    {
                        icxCopyColumn( &src_part, &src_buf, pix_size );
                        /* do not use bit-rev table for
                           real transforms here, because they
                           are done via twice short complex transforms,
                           whereas other columns are transformed with usual m-order transform */
                        fast_func( src_buf.data.ptr, m, 0 );
                        icxCopyColumn( &src_buf, &dst_part, pix_size );
                        src_part.data.ptr = src->data.ptr + (size.width-1)*pix_size;
                        dst_part.data.ptr = dst->data.ptr + (size.width-1)*pix_size;
                        icxCopyColumn( &src_part, &src_buf, pix_size );
                        /* the same about bit-rev table */
                        fast_func( src_buf.data.ptr, m, 0 );
                        icxCopyColumn( &src_buf, &dst_part, pix_size );
                        src_part.data.ptr = src->data.ptr + pix_size;
                        dst_part.data.ptr = dst->data.ptr + pix_size;
                        fast_func = fast_tbl[0 + tbl_offset];
                        width = (width - 2)/2;
                        col_pix_size *= 2;
                        src_buf.step *= 2;
                        src_buf.type = (src_buf.type & ~CX_MAT_CN_MASK) + (2-1)*8;
                    }
            
                    for( i = 0; i < width; i++ )
                    {
                        icxCopyColumn( &src_part, &src_buf, col_pix_size );
                        
                        if( inv )
                            icxConjugate( &src_buf );

                        fast_func( src_buf.data.ptr, m, itab );

                        if( inv )
                            icxConjugate( &src_buf );
                        
                        icxCopyColumn( &src_buf, &dst_part, col_pix_size );

                        src_part.data.ptr += col_pix_size;
                        dst_part.data.ptr += col_pix_size;
                    }
                }
                else
                {
                    int n = size.height, width = size.width;
                    CxDXTSlowFunc slow_func = slow_tbl[(CX_MAT_CN(type)==2?0:inv+1) + tbl_offset];
                    dst_buf = src_buf;
                    dst_buf.data.ptr = buffer + size.height*pix_size*2;

                    if( CX_MAT_CN(type) == 1 )
                    {
                        icxCopyColumn( &src_part, &src_buf, pix_size );
                        slow_func( src_buf.data.ptr, dst_buf.data.ptr, n, inv );
                        icxCopyColumn( &dst_buf, &dst_part, pix_size );
                        if( width % 2 == 0 )
                        {
                            src_part.data.ptr = src->data.ptr + (size.width-1)*pix_size;
                            dst_part.data.ptr = dst->data.ptr + (size.width-1)*pix_size;
                            icxCopyColumn( &src_part, &src_buf, pix_size );
                            slow_func( src_buf.data.ptr, dst_buf.data.ptr, n, inv );
                            icxCopyColumn( &dst_buf, &dst_part, pix_size );
                        }
                        src_part.data.ptr = src->data.ptr + pix_size;
                        dst_part.data.ptr = dst->data.ptr + pix_size;
                        slow_func = slow_tbl[0 + tbl_offset];
                        width = (width - 1)/2;
                        col_pix_size *= 2;
                        src_buf.step *= 2;
                        dst_buf.step *= 2;
                    }
            
                    for( i = 0; i < width; i++ )
                    {
                        icxCopyColumn( &src_part, &src_buf, col_pix_size );
                        slow_func( src_buf.data.ptr, dst_buf.data.ptr, n, inv );
                        icxCopyColumn( &dst_buf, &dst_part, col_pix_size );
                        src_part.data.ptr += col_pix_size;
                        dst_part.data.ptr += col_pix_size;
                    }
                }

                src = dst;
                if( stage >= 3 )
                    break;
                stage += 3;
            }
        }
    }

    if( flags & CX_DXT_SCALE )
        icxScale( dst, re_scale, im_scale );

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );

    if( itab && !local_itab_alloc )
        cxFree( (void**)&itab );
}


CX_IMPL void
cxMulCcs( const CxArr* srcAarr, const CxArr* srcBarr, CxArr* dstarr )
{
    CX_FUNCNAME( "cxMulCcs" );

    __BEGIN__;

    CxMat stubA, *srcA = (CxMat*)srcAarr;
    CxMat stubB, *srcB = (CxMat*)srcBarr;
    CxMat dststub, *dst = (CxMat*)dstarr;
    int type, is_1d;
    int j, rows, cols;

    if( !CX_IS_MAT(srcA))
        CX_CALL( srcA = cxGetMat( srcA, &stubA, 0 ));

    if( !CX_IS_MAT(srcB))
        CX_CALL( srcB = cxGetMat( srcB, &stubB, 0 ));

    if( !CX_IS_MAT(dst))
        CX_CALL( dst = cxGetMat( dst, &dststub, 0 ));

    if( !CX_ARE_TYPES_EQ( srcA, srcB ) || !CX_ARE_TYPES_EQ( srcA, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( !CX_ARE_SIZES_EQ( srcA, dst ) || !CX_ARE_SIZES_EQ( srcA, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    type = CX_MAT_TYPE( dst->type );
    rows = srcA->rows;
    cols = srcA->cols;
    is_1d = cols == 1 || rows == 1;

    if( type == CX_32FC1 )
    {
        float* dataA = (float*)srcA->data.ptr;
        float* dataB = (float*)srcB->data.ptr;
        float* dstdata = (float*)dst->data.ptr;

        if( is_1d )
        {
            int len = cols + rows - 1;

            dstdata[0] = dataA[0]*dataB[0];
            if( len % 2 == 0 )
            {
                dstdata[len-1] = dataA[len-1]*dataB[len-1];
                dstdata += 1;
                dataA += 1;
                dataB += 1;
                len = (len - 1) & -2;
            }

            for( j = 0; j < len; j += 2 )
            {
                double re = (double)dataA[j]*dataB[j] - (double)dataA[j+1]*dataB[j+1];
                double im = (double)dataA[j]*dataB[j+1] + (double)dataA[j+1]*dataB[j];

                dstdata[j] = (float)re;
                dstdata[j+1] = (float)im;
            }
        }
        else
        {
            dstdata[0] = dataA[0]*dataB[0];
            if( rows % 2 == 0 )
                dstdata[(rows-1)*dst->step] = dataA[(rows-1)*srcA->step]*
                                              dataB[(rows-1)*srcB->step];
            for( j = 1; j <= rows - 2; j += 2 )
            {
                double re = (double)dataA[j*srcA->step]*dataB[j*srcB->step] -
                           (double)dataA[(j+1)*srcA->step]*dataB[(j+1)*srcB->step];
                double im = (double)dataA[j*srcA->step]*dataB[(j+1)*srcB->step] +
                           (double)dataA[(j+1)*srcA->step]*dataB[j*srcB->step];
                dstdata[j*dst->step] = (float)re;
                dstdata[(j+1)*dst->step] = (float)im;
            }

            if( cols % 2 == 0 )
            {
                dstdata[cols - 1] = dataA[cols - 1]*dataB[cols - 1];
                if( rows % 2 == 0 )
                    dstdata[(rows-1)*dst->step+cols-1] =
                    dataA[(rows-1)*srcA->step+cols-1]*dataB[(rows-1)*srcB->step+cols-1];

                for( j = 1; j <= rows - 2; j += 2 )
                {
                    double re = (double)dataA[j*srcA->step+cols-1]*dataB[j*srcB->step+cols-1] -
                                (double)dataA[(j+1)*srcA->step+cols-1]*dataB[(j+1)*srcB->step+cols-1];
                    double im = (double)dataA[j*srcA->step+cols-1]*dataB[(j+1)*srcB->step+cols-1] +
                                (double)dataA[(j+1)*srcA->step+cols-1]*dataB[j*srcB->step+cols-1];
                    dstdata[j*dst->step] = (float)re;
                    dstdata[(j+1)*dst->step] = (float)im;
                }

                dataA += 1;
                dataB += 1;
                dstdata += 1;
                cols = (cols - 1) & -2;
            }

            for( ; rows--; dataA += srcA->step,
                           dataB += srcB->step,
                           dstdata += dst->step )
            {
                for( j = 0; j < cols; j += 2 )
                {
                    double re = (double)dataA[j]*dataB[j] - (double)dataA[j+1]*dataB[j+1];
                    double im = (double)dataA[j]*dataB[j+1] + (double)dataA[j+1]*dataB[j];
                    dstdata[j] = (float)re;
                    dstdata[j+1] = (float)im;
                }
            }
        }
    }
    else if( type == CX_64FC1 )
    {
        double* dataA = (double*)srcA->data.ptr;
        double* dataB = (double*)srcB->data.ptr;
        double* dstdata = (double*)dst->data.ptr;

        if( is_1d )
        {
            int len = cols + rows - 1;

            dstdata[0] = dataA[0]*dataB[0];
            if( len % 2 == 0 )
            {
                dstdata[len-1] = dataA[len-1]*dataB[len-1];
                dstdata += 1;
                dataA += 1;
                dataB += 1;
                len = (len - 1) & -2;
            }

            for( j = 0; j < len; j += 2 )
            {
                double re = dataA[j]*dataB[j] - dataA[j+1]*dataB[j+1];
                double im = dataA[j]*dataB[j+1] + dataA[j+1]*dataB[j];

                dstdata[j] = re;
                dstdata[j+1] = im;
            }
        }
        else
        {
            dstdata[0] = dataA[0]*dataB[0];
            if( rows % 2 == 0 )
                dstdata[(rows-1)*dst->step] = dataA[(rows-1)*srcA->step]*
                                              dataB[(rows-1)*srcB->step];
            for( j = 1; j <= rows - 2; j += 2 )
            {
                double re = dataA[j*srcA->step]*dataB[j*srcB->step] -
                            dataA[(j+1)*srcA->step]*dataB[(j+1)*srcB->step];
                double im = dataA[j*srcA->step]*dataB[(j+1)*srcB->step] +
                            dataA[(j+1)*srcA->step]*dataB[j*srcB->step];
                dstdata[j*dst->step] = re;
                dstdata[(j+1)*dst->step] = im;
            }

            if( cols % 2 == 0 )
            {
                dstdata[cols - 1] = dataA[cols - 1]*dataB[cols - 1];
                if( rows % 2 == 0 )
                    dstdata[(rows-1)*dst->step+cols-1] =
                    dataA[(rows-1)*srcA->step+cols-1]*dataB[(rows-1)*srcB->step+cols-1];

                for( j = 1; j <= rows - 2; j += 2 )
                {
                    double re = dataA[j*srcA->step+cols-1]*dataB[j*srcB->step+cols-1] -
                                dataA[(j+1)*srcA->step+cols-1]*dataB[(j+1)*srcB->step+cols-1];
                    double im = dataA[j*srcA->step+cols-1]*dataB[(j+1)*srcB->step+cols-1] +
                                dataA[(j+1)*srcA->step+cols-1]*dataB[j*srcB->step+cols-1];
                    dstdata[j*dst->step] = re;
                    dstdata[(j+1)*dst->step] = im;
                }

                dataA += 1;
                dataB += 1;
                dstdata += 1;
                cols = (cols - 1) & -2;
            }

            for( ; rows--; dataA += srcA->step,
                           dataB += srcB->step,
                           dstdata += dst->step )
            {
                for( j = 0; j < cols; j += 2 )
                {
                    double re = dataA[j]*dataB[j] - dataA[j+1]*dataB[j+1];
                    double im = dataA[j]*dataB[j+1] + dataA[j+1]*dataB[j];
                    dstdata[j] = re;
                    dstdata[j+1] = im;
                }
            }
        }
    }
    else
    {
        CX_ERROR( CX_StsUnsupportedFormat, "Only 32fC1 and 64fC1 formats are supported" );
    }

    __END__;
}


/****************************************************************************************\
                               Discrete Cosine Transform
\****************************************************************************************/

static const double icxDctScale[] =
{
    1.414213562373095100,
    1.000000000000000000,
    0.707106781186547570,
    0.500000000000000000,
    0.353553390593273790,
    0.250000000000000000,
    0.176776695296636890,
    0.125000000000000000,
    0.088388347648318447,
    0.062500000000000000,
    0.044194173824159223,
    0.031250000000000000,
    0.022097086912079612,
    0.015625000000000000,
    0.011048543456039806,
    0.007812500000000000,
    0.005524271728019903,
    0.003906250000000000,
    0.002762135864009952,
    0.001953125000000000,
    0.001381067932004976,
    0.000976562500000000,
    0.000690533966002488,
    0.000488281250000000,
    0.000345266983001244,
    0.000244140625000000,
    0.000172633491500622,
    0.000122070312500000,
    0.000086316745750311,
    0.000061035156250000,
    0.000043158372875155
};

static const double icxIDctScale[] =
{
    0.707106781186547570,
    1.000000000000000000,
    1.414213562373095100,
    2.000000000000000000,
    2.828427124746190300,
    4.000000000000000000,
    5.656854249492380600,
    8.000000000000000000,
    11.31370849898476100,
    16.00000000000000000,
    22.62741699796952200,
    32.00000000000000000,
    45.25483399593904500,
    64.00000000000000000,
    90.50966799187808900,
    128.0000000000000000,
    181.0193359837561800,
    256.0000000000000000,
    362.0386719675123600,
    512.0000000000000000,
    724.0773439350247100,
    1024.000000000000000,
    1448.154687870049400,
    2048.000000000000000,
    2896.309375740098900,
    4096.000000000000000,
    5792.618751480197700,
    8192.000000000000000,
    11585.23750296039500,
    16384.00000000000000,
    23170.47500592079100
};

/* DCT is calculated using DFT, as described here:
   http://www.ece.utexas.edu/~bevans/courses/ee381k/lectures/09_DCT/lecture9/:
*/
#define ICX_DCT_FUNCS( flavor, datatype )                       \
static CxStatus icxDCT_fwd_##flavor( const datatype* src,       \
                                     datatype* dst, int n, int )\
{                                                               \
    if( (n & (n-1)) == 0 )                                      \
    {                                                           \
        int local_alloc = 0;                                    \
        datatype* buffer = dst;                                 \
        int j, n2 = n >> 1, m;                                  \
        int buf_size = n*sizeof(buffer[0]);                     \
        double c0, s0, c, s, t;                                 \
        double scale;                                           \
                                                                \
        if( n == 1 )                                            \
        {                                                       \
            dst[0] = src[0];                                    \
            return CX_OK;                                       \
        }                                                       \
                                                                \
        if( buf_size < CX_MAX_LOCAL_SIZE )                      \
        {                                                       \
            buffer = (datatype*)alloca(buf_size);               \
            local_alloc = 1;                                    \
        }                                                       \
        else                                                    \
            buffer = (datatype*)cxAlloc(buf_size);              \
                                                                \
        for( j = 0; j < n2; j++ )                               \
        {                                                       \
            buffer[j] = src[j*2];                               \
            buffer[n-j-1] = src[j*2+1];                         \
        }                                                       \
                                                                \
        m = icxlog2(n);                                         \
        icxFFT_fwd_##flavor( buffer, m, 0 );                    \
        src = buffer;                                           \
                                                                \
        c0 = c = icxDxtTab[m+2][0];                             \
        s0 = s = -icxDxtTab[m+2][1];                            \
                                                                \
        scale = icxDctScale[m];                                 \
        c *= scale;                                             \
        s *= scale;                                             \
                                                                \
        dst[0] = (datatype)(src[0]*scale*CX_SIN_45);            \
                                                                \
        for( j = 1; j < n2; j++ )                               \
        {                                                       \
            double t0 = c*src[j*2-1] - s*src[j*2];              \
            double t1 = -s*src[j*2-1] - c*src[j*2];             \
            dst[j] = (datatype)t0;                              \
            dst[n-j] = (datatype)t1;                            \
                                                                \
            t = c*c0 - s*s0;                                    \
            s = c*s0 + s*c0;                                    \
            c = t;                                              \
        }                                                       \
                                                                \
        dst[n2] = (datatype)(src[n-1]*c);                       \
        if( !local_alloc )                                      \
            cxFree( (void**)&buffer );                          \
    }                                                           \
                                                                \
    return CX_OK;                                               \
}                                                               \
                                                                \
static CxStatus icxDCT_inv_##flavor( const datatype* src,       \
                                     datatype* dst, int n, int )\
{                                                               \
    if( (n & (n-1)) == 0 )                                      \
    {                                                           \
        int local_alloc = 0;                                    \
        datatype* buffer = dst;                                 \
        int j, n2 = n >> 1, m;                                  \
        int buf_size = n*sizeof(buffer[0]);                     \
        double c0, s0, c, s, t;                                 \
        double scale;                                           \
                                                                \
        if( n == 1 )                                            \
        {                                                       \
            dst[0] = src[0];                                    \
            return CX_OK;                                       \
        }                                                       \
                                                                \
        if( buf_size < CX_MAX_LOCAL_SIZE )                      \
        {                                                       \
            buffer = (datatype*)alloca(buf_size);               \
            local_alloc = 1;                                    \
        }                                                       \
        else                                                    \
            buffer = (datatype*)cxAlloc(buf_size);              \
                                                                \
        m = icxlog2(n);                                         \
        scale = icxIDctScale[m];                                \
                                                                \
        buffer[0] = (datatype)(src[0]*scale*(2*CX_SIN_45));     \
        c0 = c = icxDxtTab[m+2][0];                             \
        s0 = s = icxDxtTab[m+2][1];                             \
                                                                \
        c *= scale;                                             \
        s *= scale;                                             \
                                                                \
        for( j = 1; j < n2; j++ )                               \
        {                                                       \
            double t0 = c*src[j] + s*src[n-j];                  \
            double t1 = s*src[j] - c*src[n-j];                  \
            buffer[j*2-1] = (datatype)t0;                       \
            buffer[j*2] = (datatype)t1;                         \
                                                                \
            t = c*c0 - s*s0;                                    \
            s = c*s0 + s*c0;                                    \
            c = t;                                              \
        }                                                       \
                                                                \
        buffer[n-1] = (datatype)(src[n2]*2*c);                  \
                                                                \
        icxFFT_inv_##flavor( buffer, m, 0 );                    \
                                                                \
        for( j = 0; j < n2; j++ )                               \
        {                                                       \
            dst[j*2] = buffer[j];                               \
            dst[j*2+1] = buffer[n-j-1];                         \
        }                                                       \
                                                                \
        if( !local_alloc )                                      \
            cxFree( (void**)&buffer );                          \
    }                                                           \
                                                                \
    return CX_OK;                                               \
}

ICX_DCT_FUNCS( 32f, float )
ICX_DCT_FUNCS( 64f, double )

static void icxCopyFrom2RealColumns( const CxMat* src, CxMat* dst, CxMat* dst2, int pix_size )
{
    int i, rows = src->rows;

    if( pix_size == sizeof(float) )
    {
        float *src_data = src->data.fl,
              *dst_data = dst->data.fl,
              *dst2_data = dst2->data.fl;
        int src_step = src->step/sizeof(float);
        
        for( i = 0; i < rows; i++ )
        {
            dst_data[i] = src_data[0];
            dst2_data[i] = src_data[1];
            src_data += src_step;
        }
    }
    else
    {
        assert( pix_size == sizeof(double));
        double *src_data = src->data.db,
               *dst_data = dst->data.db,
               *dst2_data = dst2->data.db;
        int src_step = src->step/sizeof(double);
        
        for( i = 0; i < rows; i++ )
        {
            dst_data[i] = src_data[0];
            dst2_data[i] = src_data[1];
            src_data += src_step;
        }
    }
}


static void icxCopyTo2RealColumns( const CxMat* src, const CxMat* src2,
                                   CxMat* dst, int pix_size )
{
    int i, rows = src->rows;

    if( pix_size == sizeof(float) )
    {
        float *src_data = src->data.fl,
              *src2_data = src2->data.fl,
              *dst_data = dst->data.fl;
        int dst_step = dst->step/sizeof(float);
        
        for( i = 0; i < rows; i++ )
        {
            dst_data[0] = src_data[i];
            dst_data[1] = src2_data[i];
            dst_data += dst_step;
        }
    }
    else
    {
        assert( pix_size == sizeof(double));
        double *src_data = src->data.db,
               *src2_data = src2->data.db,
               *dst_data = dst->data.db;
        int dst_step = dst->step/sizeof(double);
        
        for( i = 0; i < rows; i++ )
        {
            dst_data[0] = src_data[i];
            dst_data[1] = src2_data[i];
            dst_data += dst_step;
        }
    }
}


CX_IMPL void
cxDCT( const CxArr* srcarr, CxArr* dstarr, int flags )
{
    static CxDXTSlowFunc slow_tbl[4];
    static int inittab = 0;
    
    uchar* buffer = 0;
    int local_alloc = 0;

    /*int* itab = 0;
    int local_itab_alloc = 0;*/
    
    CX_FUNCNAME( "cxDCT" );

    __BEGIN__;

    double scale = 1;
    CxMat srcstub, *src = (CxMat*)srcarr;
    CxMat dststub, *dst = (CxMat*)dstarr;
    CxMat buf, *dst_tmp;
    CxSize size;
    int is_1d;
    int type, pix_size;
    int width_flag, height_flag;
    int tbl_offset;
    int inv = (flags & CX_DXT_INVERSE) != 0;

    if( !inittab )
    {
        slow_tbl[0] = (CxDXTSlowFunc)icxDCT_fwd_32f;
        slow_tbl[1] = (CxDXTSlowFunc)icxDCT_inv_32f;

        slow_tbl[2] = (CxDXTSlowFunc)icxDCT_fwd_64f;
        slow_tbl[3] = (CxDXTSlowFunc)icxDCT_inv_64f;
        
        inittab = 1;
    }

    if( !CX_IS_MAT( src ))
    {
        int coi = 0;
        CX_CALL( src = cxGetMat( src, &srcstub, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_IS_MAT( dst ))
    {
        int coi = 0;
        CX_CALL( dst = cxGetMat( dst, &dststub, &coi ));

        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "" );
    }

    if( !CX_ARE_TYPES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    dst_tmp = dst;
    type = CX_MAT_TYPE( dst->type );

    if( CX_MAT_CN(type) != 1 || CX_MAT_DEPTH(type) < CX_32F )
        CX_ERROR( CX_StsUnsupportedFormat,
        "Only 32fC1 and 64fC1 formats are supported" );

    pix_size = icxPixSize[type];
    tbl_offset = (CX_MAT_DEPTH(type)==CX_64F ? 2 : 0);

    size = cxGetMatSize( dst );
    is_1d = size.width == 1 || size.height == 1;

    if( flags & CX_DXT_SCALE )
        scale = 1./(size.width*size.height);

    width_flag = (size.width&(size.width-1)) == 0;
    height_flag = (size.height&(size.height-1)) == 0;

    if( !width_flag || !height_flag )
        CX_ERROR( CX_StsBadSize, "non-power-2 DCT is not implemented yet" );

    if( !CX_IS_MAT_CONT(dst->type) || !CX_IS_MAT_CONT(src->type) || !is_1d )
    {
        int buffer_size = pix_size*MAX(size.width,size.height)*4;
        if( buffer_size <= CX_MAX_LOCAL_SIZE )
        {
            buffer = (uchar*)alloca( buffer_size );
            local_alloc = 1;
        }
        else
            CX_CALL( buffer = (uchar*)cxAlloc( buffer_size ));
        buf = cxMat( size.height, size.width, type, buffer );
        dst_tmp = &buf;
    }

    if( is_1d )
    {
        int n = size.width + size.height - 1;
        CxMat buf2;
        CxMat* src_tmp = src;

        if( CX_IS_MAT_CONT(dst->type))
            dst_tmp = dst;
        if( !CX_IS_MAT_CONT(src->type) )
        {
            buf2 = cxMat( size.height, size.width, type, buffer + n*pix_size );
            src_tmp = &buf2;
            cxCopy( src, src_tmp, 0 );
        }

        slow_tbl[inv+tbl_offset]( (CxPoint2D32f*)(src_tmp->data.ptr),
                                  (CxPoint2D32f*)(dst_tmp->data.ptr), n, inv );

        if( dst_tmp->data.ptr != dst->data.ptr )
            cxCopy( dst_tmp, dst, 0 );
    }
    else
    {
        CxMat src_part, dst_part, buf2;
        CxDXTSlowFunc slow_func = slow_tbl[inv + tbl_offset];
        int i;

        /*if( width_flag || height_flag )
        {
            int buffer_size = sizeof(int)*(MAX(size.width,size.height)+4)/4;
            if( buffer_size <= CX_MAX_LOCAL_SIZE )
            {
                itab = (int*)alloca( buffer_size );
                local_itab_alloc = 1;
            }
            else
                CX_CALL( itab = (int*)cxAlloc( buffer_size ));
        }*/

        src_part = cxMat( 1, size.width, type );
        dst_part = src_part;
        src_part.data.ptr = src->data.ptr;
        dst_part.data.ptr = dst->data.ptr;

        for( i = 0; i < size.height; i++ )
        {
            slow_func( src_part.data.ptr, dst_part.data.ptr, size.width, inv );
            src_part.data.ptr += src->step;
            dst_part.data.ptr += dst->step;
        }

        // column transforms
        dst_part = cxMat( size.height, 1, type );
        cxSetData( &dst_part, dst->data.ptr, dst->step );
        buf = dst_part;
        cxSetData( &buf, buffer, pix_size );
        buf2 = buf;
        buf2.data.ptr += size.height*pix_size;

        for( i = 0; i < size.width; i += 2 )
        {
            icxCopyFrom2RealColumns( &dst_part, &buf, &buf2, pix_size );
            slow_func( buf.data.ptr, buf.data.ptr, size.height, inv );
            slow_func( buf2.data.ptr, buf2.data.ptr, size.height, inv );
            icxCopyTo2RealColumns( &buf, &buf2, &dst_part, pix_size );
            dst_part.data.ptr += pix_size*2;
        }
    }

    if( flags & CX_DXT_SCALE )
        icxScale( dst, scale, scale );

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );

    /*if( itab && !local_itab_alloc )
        cxFree( (void**)&itab );*/
}

/* End of file. */
