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

#ifndef _CXCORE_INTERNAL_H_
#define _CXCORE_INTERNAL_H_

#if _MSC_VER >= 1200
    /* disable warnings related to inline functions */
    #pragma warning( disable: 4711 4710 4514 )
#endif

typedef unsigned short ushort;
typedef unsigned long ulong;

#ifdef __BORLANDC__
    #define     WIN32
    #define     CX_DLL
    #undef      _CX_ALWAYS_PROFILE_
    #define     _CX_ALWAYS_NO_PROFILE_
#endif

#include "cxcore.h"
#include "cxmisc.h"
#include "_cxipp.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>

// -128.f ... 255.f
extern const float icx8x32fTab[];
#define CX_8TO32F(x)  icx8x32fTab[(x)+128]

#define CX_TXT_FONT_SHIFT     9
#define CX_TXT_SIZE_SHIFT     8
#define CX_TXT_BASE_WIDTH     (12 << CX_TXT_FONT_SHIFT)
#define CX_TXT_BASE_HEIGHT    (24 << CX_TXT_FONT_SHIFT)
#define CX_FONT_HDR_SIZE       5

extern const short icxTextFacesFn0[];
extern const int icxTextHdrsFn0[];

extern const int icxPixSize[];
extern const char icxDepthToType[];
extern const int icxTypeToDepth[];

#define icxIplToCxDepth( depth ) \
    icxDepthToType[(((depth) & 255) >> 2) + ((depth) < 0)]

#define icxCxToIplDepth( type )  \
    icxTypeToDepth[(type)]

typedef CxFunc2D_3A1I CxArithmBinMaskFunc2D;
typedef CxFunc2D_2A1P1I CxArithmUniMaskFunc2D;


/****************************************************************************************\
*                                   Complex arithmetics                                  *
\****************************************************************************************/

struct CxComplex32f;
struct CxComplex64f;

struct CxComplex32f
{
    float re, im;

    CxComplex32f( float _re, float _im=0 ) : re(_re), im(_im) {}
    CxComplex32f() : re(0), im(0) {}
    operator CxComplex64f() const;
};

struct CxComplex64f
{
    double re, im;

    CxComplex64f( double _re, double _im=0 ) : re(_re), im(_im) {}
    CxComplex64f() : re(0), im(0) {}
    operator CxComplex32f() const;
};

inline CxComplex32f operator + (CxComplex32f a, CxComplex32f b)
{
    return CxComplex32f( a.re + b.re, a.im + b.im );
}

inline CxComplex32f& operator += (CxComplex32f& a, CxComplex32f b)
{
    a.re += b.re;
    a.im += b.im;
    return a;
}

inline CxComplex32f operator - (CxComplex32f a, CxComplex32f b)
{
    return CxComplex32f( a.re - b.re, a.im - b.im );
}

inline CxComplex32f& operator -= (CxComplex32f& a, CxComplex32f b)
{
    a.re -= b.re;
    a.im -= b.im;
    return a;
}

inline CxComplex32f operator - (CxComplex32f a)
{
    return CxComplex32f( -a.re, -a.im );
}

inline CxComplex32f operator * (CxComplex32f a, CxComplex32f b)
{
    return CxComplex32f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

inline double abs(CxComplex32f a)
{
    return sqrt( (double)a.re*a.re + (double)a.im*a.im );
}

inline CxComplex32f conj(CxComplex32f a)
{
    return CxComplex32f( a.re, -a.im );
}


inline CxComplex32f operator / (CxComplex32f a, CxComplex32f b)
{
    double t = 1./((double)b.re*b.re + (double)b.im*b.im);
    return CxComplex32f( (float)((a.re*b.re + a.im*b.im)*t),
                         (float)((-a.re*b.im + a.im*b.re)*t) );
}

inline CxComplex32f operator * (double a, CxComplex32f b)
{
    return CxComplex32f( (float)(a*b.re), (float)(a*b.im) );
}

inline CxComplex32f operator * (CxComplex32f a, double b)
{
    return CxComplex32f( (float)(a.re*b), (float)(a.im*b) );
}

inline CxComplex32f::operator CxComplex64f() const
{
    return CxComplex64f(re,im);
}


inline CxComplex64f operator + (CxComplex64f a, CxComplex64f b)
{
    return CxComplex64f( a.re + b.re, a.im + b.im );
}

inline CxComplex64f& operator += (CxComplex64f& a, CxComplex64f b)
{
    a.re += b.re;
    a.im += b.im;
    return a;
}

inline CxComplex64f operator - (CxComplex64f a, CxComplex64f b)
{
    return CxComplex64f( a.re - b.re, a.im - b.im );
}

inline CxComplex64f& operator -= (CxComplex64f& a, CxComplex64f b)
{
    a.re -= b.re;
    a.im -= b.im;
    return a;
}

inline CxComplex64f operator - (CxComplex64f a)
{
    return CxComplex64f( -a.re, -a.im );
}

inline CxComplex64f operator * (CxComplex64f a, CxComplex64f b)
{
    return CxComplex64f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

inline double abs(CxComplex64f a)
{
    return sqrt( (double)a.re*a.re + (double)a.im*a.im );
}

inline CxComplex64f operator / (CxComplex64f a, CxComplex64f b)
{
    double t = 1./((double)b.re*b.re + (double)b.im*b.im);
    return CxComplex64f( (a.re*b.re + a.im*b.im)*t,
                         (-a.re*b.im + a.im*b.re)*t );
}

inline CxComplex64f operator * (double a, CxComplex64f b)
{
    return CxComplex64f( a*b.re, a*b.im );
}

inline CxComplex64f operator * (CxComplex64f a, double b)
{
    return CxComplex64f( a.re*b, a.im*b );
}

inline CxComplex64f::operator CxComplex32f() const
{
    return CxComplex32f((float)re,(float)im);
}

inline CxComplex64f conj(CxComplex64f a)
{
    return CxComplex64f( a.re, -a.im );
}

inline CxComplex64f operator + (CxComplex64f a, CxComplex32f b)
{
    return CxComplex64f( a.re + b.re, a.im + b.im );
}

inline CxComplex64f operator + (CxComplex32f a, CxComplex64f b)
{
    return CxComplex64f( a.re + b.re, a.im + b.im );
}

inline CxComplex64f operator - (CxComplex64f a, CxComplex32f b)
{
    return CxComplex64f( a.re - b.re, a.im - b.im );
}

inline CxComplex64f operator - (CxComplex32f a, CxComplex64f b)
{
    return CxComplex64f( a.re - b.re, a.im - b.im );
}

inline CxComplex64f operator * (CxComplex64f a, CxComplex32f b)
{
    return CxComplex64f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

inline CxComplex64f operator * (CxComplex32f a, CxComplex64f b)
{
    return CxComplex64f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

#endif /*_CXCORE_INTERNAL_H_*/
