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

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "cvsvd.h"

/////////////////////////////////////////////////////////////////////////////////////////

#define CV_SWAP( a, b, t ) ((t) = (a), (a) = (b), (b) = (t))

#define icvGivens_64f( n, x, y, c, s ) \
{                                      \
    int _i;                            \
    double* _x = (x);                  \
    double* _y = (y);                  \
                                       \
    for( _i = 0; _i < n; _i++ )        \
    {                                  \
        double t0 = _x[_i];            \
        double t1 = _y[_i];            \
        _x[_i] = t0*c + t1*s;          \
        _y[_i] = -t0*s + t1*c;         \
    }                                  \
}

static  void
icvMatrAXPY_64f( int m, int n, const double* x, int dx,
                 const double* a, double* y, int dy )
{
    int i, j;

    for( i = 0; i < m; i++, x += dx, y += dy )
    {
        double s = a[i];

        for( j = 0; j <= n - 4; j += 4 )
        {
            double t0 = y[j]   + s*x[j];
            double t1 = y[j+1] + s*x[j+1];
            y[j]   = t0;
            y[j+1] = t1;
            t0 = y[j+2] + s*x[j+2];
            t1 = y[j+3] + s*x[j+3];
            y[j+2] = t0;
            y[j+3] = t1;
        }

        for( ; j < n; j++ ) y[j] += s*x[j];
    }
}


static void
icvMatrAXPY3_64f( int m, int n, const double* x, int l, double* y, double h )
{
    int i, j;

    for( i = 1; i < m; i++ )
    {
        double s = 0;

        y += l;

        for( j = 0; j <= n - 4; j += 4 )
            s += x[j]*y[j] + x[j+1]*y[j+1] + x[j+2]*y[j+2] + x[j+3]*y[j+3];

        for( ; j < n; j++ )  s += x[j]*y[j];

        s *= h;
        y[-1] = s*x[-1];

        for( j = 0; j <= n - 4; j += 4 )
        {
            double t0 = y[j]   + s*x[j];
            double t1 = y[j+1] + s*x[j+1];
            y[j]   = t0;
            y[j+1] = t1;
            t0 = y[j+2] + s*x[j+2];
            t1 = y[j+3] + s*x[j+3];
            y[j+2] = t0;
            y[j+3] = t1;
        }

        for( ; j < n; j++ ) y[j] += s*x[j];
    }
}


#define icvGivens_32f( n, x, y, c, s ) \
{                                      \
    int _i;                            \
    float* _x = (x);                   \
    float* _y = (y);                   \
                                       \
    for( _i = 0; _i < n; _i++ )        \
    {                                  \
        double t0 = _x[_i];            \
        double t1 = _y[_i];            \
        _x[_i] = (float)(t0*c + t1*s); \
        _y[_i] = (float)(-t0*s + t1*c);\
    }                                  \
}

static  void
icvMatrAXPY_32f( int m, int n, const float* x, int dx,
                 const float* a, float* y, int dy )
{
    int i, j;

    for( i = 0; i < m; i++, x += dx, y += dy )
    {
        double s = a[i];

        for( j = 0; j <= n - 4; j += 4 )
        {
            double t0 = y[j]   + s*x[j];
            double t1 = y[j+1] + s*x[j+1];
            y[j]   = (float)t0;
            y[j+1] = (float)t1;
            t0 = y[j+2] + s*x[j+2];
            t1 = y[j+3] + s*x[j+3];
            y[j+2] = (float)t0;
            y[j+3] = (float)t1;
        }

        for( ; j < n; j++ )
            y[j] = (float)(y[j] + s*x[j]);
    }
}


static void
icvMatrAXPY3_32f( int m, int n, const float* x, int l, float* y, double h )
{
    int i, j;

    for( i = 1; i < m; i++ )
    {
        double s = 0;
        y += l;

        for( j = 0; j <= n - 4; j += 4 )
            s += x[j]*y[j] + x[j+1]*y[j+1] + x[j+2]*y[j+2] + x[j+3]*y[j+3];

        for( ; j < n; j++ )  s += x[j]*y[j];

        s *= h;
        y[-1] = (float)(s*x[-1]);

        for( j = 0; j <= n - 4; j += 4 )
        {
            double t0 = y[j]   + s*x[j];
            double t1 = y[j+1] + s*x[j+1];
            y[j]   = (float)t0;
            y[j+1] = (float)t1;
            t0 = y[j+2] + s*x[j+2];
            t1 = y[j+3] + s*x[j+3];
            y[j+2] = (float)t0;
            y[j+3] = (float)t1;
        }

        for( ; j < n; j++ ) y[j] = (float)(y[j] + s*x[j]);
    }
}

static double
pythag( double a, double b )
{
    a = fabs( a );
    b = fabs( b );
    if( a > b )
    {
        b /= a;
        a *= sqrt( 1. + b * b );
    }
    else if( b != 0 )
    {
        a /= b;
        a = b * sqrt( 1. + a * a );
    }

    return a;
}

/****************************************************************************************/
/****************************************************************************************/

#define MAX_ITERS  30

void PNL_API icvSVD_64f( double* a, int lda, int m, int n,
                 double* w,
                 double* uT, int lduT, int nu,
                 double* vT, int ldvT,
                 double* buffer )
{
    double* e;
    double* temp;
    double *w1, *e1;
    double *hv;
    double ku0 = 0, kv0 = 0;
    double anorm = 0;
    double *a1 = a, *u0 = uT, *v0 = vT;
    double scale, h;
    int i, j, k, l;
    int nm, m1, n1;
    int nv = n;
    int iters = 0;
    double* hv0 = (double*)malloc( (m+1)*sizeof(hv0[0])); 

    e = buffer;

    w1 = w;
    e1 = e + 1;
    nm = n;
    
    temp = buffer + nm;

    memset( w, 0, nm * sizeof( w[0] ));
    memset( e, 0, nm * sizeof( e[0] ));

    m1 = m;
    n1 = n;

    for( ;; )
    {
        int update_u;
        int update_v;
        
        if( m1 == 0 )
            break;

        scale = h = 0;

        update_u = uT && m1 > m - nu;

        a = a1;
        hv = update_u ? uT : hv0;

        for( j = 0; j < m1; j++, a += lda )
        {
            double t = a[0];
            scale += fabs( hv[j] = t );
        }

        if( scale != 0 )
        {
            double f = 1./scale, g, s = 0;

            for( j = 0; j < m1; j++ )
            {
                double t = (hv[j] *= f);
                s += t * t;
            }

            g = sqrt( s );
            f = hv[0];
            if( f >= 0 )
                g = -g;
            hv[0] = f - g;
            h = 1. / (f * g - s);

            memset( temp, 0, n1 * sizeof( temp[0] ));

            a = a1;

            /* calc temp[0:n-i] = a[i:m,i:n]'*hv[0:m-i] */
            icvMatrAXPY_64f( m1, n1 - 1, a + 1, lda, hv, temp + 1, 0 );
            for( k = 1; k < n1; k++ ) temp[k] *= h;

            /* modify a: a[i:m,i:n] = a[i:m,i:n] + hv[0:m-i]*temp[0:n-i]' */
            icvMatrAXPY_64f( m1, n1 - 1, temp + 1, 0, hv, a + 1, lda );
            *w1 = g*scale;
        }
        w1++;

        /* store -2/(hv'*hv) */
        if( update_u )
        {
            if( m1 == m )
                ku0 = h;
            else
                hv[-1] = h;
        }

        a1++;
        n1--;
        if( vT )
            vT += ldvT + 1;

        if( n1 == 0 )
            break;

        scale = h = 0;
        update_v = vT && n1 > n - nv;

        a = a1;
        hv = update_v ? vT : hv0;

        for( j = 0; j < n1; j++ )
        {
            double t = a[j];
            scale += fabs( hv[j] = t );
        }

        if( scale != 0 )
        {
            double f = 1./scale, g, s = 0;

            for( j = 0; j < n1; j++ )
            {
                double t = (hv[j] *= f);
                s += t * t;
            }

            g = sqrt( s );
            f = hv[0];
            if( f >= 0 )
                g = -g;
            hv[0] = f - g;
            h = 1. / (f * g - s);

            /* update a[i:m:i+1:n] = a[i:m,i+1:n] + (a[i:m,i+1:n]*hv[0:m-i])*... */
            icvMatrAXPY3_64f( m1, n1, hv, lda, a, h );

            *e1 = g*scale;
        }
        e1++;

        /* store -2/(hv'*hv) */
        if( update_v )
        {
            if( n1 == n )
                kv0 = h;
            else
                hv[-1] = h;
        }

        a1 += lda;
        m1--;
        if( uT )
            uT += lduT + 1;
    }

    m1 -= m1 != 0;
    n1 -= n1 != 0;

    /* accumulate left transformations */
    if( uT )
    {
        m1 = m - m1;
        uT = u0 + m1 * lduT;
        for( i = m1; i < nu; i++, uT += lduT )
        {
            memset( uT + m1, 0, (m - m1) * sizeof( uT[0] ));
            uT[i] = 1.;
        }

        for( i = m1 - 1; i >= 0; i-- )
        {
            double h, s;
            int lh = nu - i;

            l = m - i;

            hv = u0 + (lduT + 1) * i;
            h = i == 0 ? ku0 : hv[-1];

            assert( h <= 0 );

            if( h != 0 )
            {
                uT = hv;
                icvMatrAXPY3_64f( lh, l-1, hv+1, lduT, uT+1, h );

                s = hv[0] * h;
                for( k = 0; k < l; k++ ) hv[k] *= s;
                hv[0] += 1;
            }
            else
            {
                for( j = 1; j < l; j++ )
                    hv[j] = 0;
                for( j = 1; j < lh; j++ )
                    hv[j * lduT] = 0;
                hv[0] = 1;
            }
        }
        uT = u0;
    }

    /* accumulate right transformations */
    if( vT )
    {
        n1 = n - n1;
        vT = v0 + n1 * ldvT;
        for( i = n1; i < nv; i++, vT += ldvT )
        {
            memset( vT + n1, 0, (n - n1) * sizeof( vT[0] ));
            vT[i] = 1.;
        }

        for( i = n1 - 1; i >= 0; i-- )
        {
            double h, s;
            int lh = nv - i;

            l = n - i;
            hv = v0 + (ldvT + 1) * i;
            h = i == 0 ? kv0 : hv[-1];

            assert( h <= 0 );

            if( h != 0 )
            {
                vT = hv;
                icvMatrAXPY3_64f( lh, l-1, hv+1, ldvT, vT+1, h );

                s = hv[0] * h;
                for( k = 0; k < l; k++ ) hv[k] *= s;
                hv[0] += 1;
            }
            else
            {
                for( j = 1; j < l; j++ )
                    hv[j] = 0;
                for( j = 1; j < lh; j++ )
                    hv[j * ldvT] = 0;
                hv[0] = 1;
            }
        }
        vT = v0;
    }

    for( i = 0; i < nm; i++ )
    {
        double tnorm = fabs( w[i] ) + fabs( e[i] );

        if( anorm < tnorm )
            anorm = tnorm;
    }

    anorm *= DBL_EPSILON;

    /* diagonalization of the bidiagonal form */
    for( k = nm - 1; k >= 0; k-- )
    {
        double z = 0;
        iters = 0;

        for( ;; )               /* do iterations */
        {
            double c, s, f, g, h, x, y;
            int flag = 0;

            /* test for splitting */
            for( l = k; l >= 0; l-- )
            {
                if( fabs(e[l]) <= anorm )
                {
                    flag = 1;
                    break;
                }
                assert( l > 0 );
                if( fabs(w[l - 1]) <= anorm )
                    break;
            }

            if( !flag )
            {
                c = 0;
                s = 1;

                for( i = l; i <= k; i++ )
                {
                    double f = s * e[i];

                    e[i] *= c;

                    if( anorm + fabs( f ) == anorm )
                        break;

                    g = w[i];
                    h = pythag( f, g );
                    w[i] = h;
                    c = g / h;
                    s = -f / h;

                    if( uT )
                        icvGivens_64f( m, uT + lduT * (i - 1), uT + lduT * i, c, s );
                }
            }

            z = w[k];
            if( l == k || iters++ == MAX_ITERS )
                break;

            /* shift from bottom 2x2 minor */
            x = w[l];
            y = w[k - 1];
            g = e[k - 1];
            h = e[k];
            f = 0.5 * (((g + z) / h) * ((g - z) / y) + y / h - h / y);
            g = pythag( f, 1 );
            if( f < 0 )
                g = -g;
            f = x - (z / x) * z + (h / x) * (y / (f + g) - h);
            /* next QR transformation */
            c = s = 1;

            for( i = l + 1; i <= k; i++ )
            {
                g = e[i];
                y = w[i];
                h = s * g;
                g *= c;
                z = pythag( f, h );
                e[i - 1] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = -x * s + g * c;
                h = y * s;
                y *= c;

                if( vT )
                    icvGivens_64f( n, vT + ldvT * (i - 1), vT + ldvT * i, c, s );

                z = pythag( f, h );
                w[i - 1] = z;

                /* rotation can be arbitrary if z == 0 */
                if( z != 0 )
                {
                    c = f / z;
                    s = h / z;
                }
                f = c * g + s * y;
                x = -s * g + c * y;

                if( uT )
                    icvGivens_64f( m, uT + lduT * (i - 1), uT + lduT * i, c, s );
            }

            e[l] = 0;
            e[k] = f;
            w[k] = x;
        }                       /* end of iteration loop */

        if( iters > MAX_ITERS )
            break;

        if( z < 0 )
        {
            w[k] = -z;
            if( vT )
            {
                for( j = 0; j < n; j++ )
                    vT[j + k * ldvT] = -vT[j + k * ldvT];
            }
        }
    }                           /* end of diagonalization loop */

    /* sort singular values */
    for( i = 0; i < nm; i++ )
    {
        k = i;
        for( j = i + 1; j < nm; j++ )
            if( w[k] < w[j] )
                k = j;

        if( k != i )
        {
            /* swap i & k values */
            double t;
            CV_SWAP( w[i], w[k], t );

            if( vT )
                for( j = 0; j < n; j++ )
                    CV_SWAP( vT[j + ldvT*k], vT[j + ldvT*i], t );

            if( uT )
                for( j = 0; j < m; j++ )
                    CV_SWAP( uT[j + lduT*k], uT[j + lduT*i], t );
        }
    }
	free( hv0 );
}


void PNL_API icvSVD_32f( float* a, int lda, int m, int n,
                 float* w,
                 float* uT, int lduT, int nu,
                 float* vT, int ldvT,
                 float* buffer )
{
    float* e;
    float* temp;
    float *w1, *e1;
    float *hv;
    double ku0 = 0, kv0 = 0;
    double anorm = 0;
    float *a1 = a, *u0 = uT, *v0 = vT;
    double scale, h;
    int i, j, k, l;
    int nm, m1, n1;
    int nv = n;
    int iters = 0;
    float* hv0 = (float*)malloc( (m+1)*sizeof(hv0[0]));

    e = buffer;

    w1 = w;
    e1 = e + 1;
    nm = n;
    
    temp = buffer + nm;

    memset( w, 0, nm * sizeof( w[0] ));
    memset( e, 0, nm * sizeof( e[0] ));

    m1 = m;
    n1 = n;

    for( ;; )
    {
        int update_u;
        int update_v;
        
        if( m1 == 0 )
            break;

        scale = h = 0;

        update_u = uT && m1 > m - nu;

        a = a1;
        hv = update_u ? uT : hv0;

        for( j = 0; j < m1; j++, a += lda )
        {
            double t = a[0];
            scale += fabs( hv[j] = (float)t );
        }

        if( scale != 0 )
        {
            double f = 1./scale, g, s = 0;

            for( j = 0; j < m1; j++ )
            {
                double t = (hv[j] = (float)(hv[j]*f));
                s += t * t;
            }

            g = sqrt( s );
            f = hv[0];
            if( f >= 0 )
                g = -g;
            hv[0] = (float)(f - g);
            h = 1. / (f * g - s);

            memset( temp, 0, n1 * sizeof( temp[0] ));

            a = a1;

            /* calc temp[0:n-i] = a[i:m,i:n]'*hv[0:m-i] */
            icvMatrAXPY_32f( m1, n1 - 1, a + 1, lda, hv, temp + 1, 0 );

            for( k = 1; k < n1; k++ ) temp[k] = (float)(temp[k]*h);

            /* modify a: a[i:m,i:n] = a[i:m,i:n] + hv[0:m-i]*temp[0:n-i]' */
            icvMatrAXPY_32f( m1, n1 - 1, temp + 1, 0, hv, a + 1, lda );
            *w1 = (float)(g*scale);
        }
        w1++;
        
        /* store -2/(hv'*hv) */
        if( update_u )
        {
            if( m1 == m )
                ku0 = h;
            else
                hv[-1] = (float)h;
        }

        a1++;
        n1--;
        if( vT )
            vT += ldvT + 1;

        if( n1 == 0 )
            break;

        scale = h = 0;
        update_v = vT && n1 > n - nv;

        a = a1;
        hv = update_v ? vT : hv0;

        for( j = 0; j < n1; j++ )
        {
            double t = a[j];
            scale += fabs( hv[j] = (float)t );
        }

        if( scale != 0 )
        {
            double f = 1./scale, g, s = 0;

            for( j = 0; j < n1; j++ )
            {
                double t = (hv[j] = (float)(hv[j]*f));
                s += t * t;
            }

            g = sqrt( s );
            f = hv[0];
            if( f >= 0 )
                g = -g;
            hv[0] = (float)(f - g);
            h = 1. / (f * g - s);

            /* update a[i:m:i+1:n] = a[i:m,i+1:n] + (a[i:m,i+1:n]*hv[0:m-i])*... */
            icvMatrAXPY3_32f( m1, n1, hv, lda, a, h );

            *e1 = (float)(g*scale);
        }
        e1++;

        /* store -2/(hv'*hv) */
        if( update_v )
        {
            if( n1 == n )
                kv0 = h;
            else
                hv[-1] = (float)h;
        }

        a1 += lda;
        m1--;
        if( uT )
            uT += lduT + 1;
    }

    m1 -= m1 != 0;
    n1 -= n1 != 0;

    /* accumulate left transformations */
    if( uT )
    {
        m1 = m - m1;
        uT = u0 + m1 * lduT;
        for( i = m1; i < nu; i++, uT += lduT )
        {
            memset( uT + m1, 0, (m - m1) * sizeof( uT[0] ));
            uT[i] = 1.;
        }

        for( i = m1 - 1; i >= 0; i-- )
        {
            double h, s;
            int lh = nu - i;

            l = m - i;

            hv = u0 + (lduT + 1) * i;
            h = i == 0 ? ku0 : hv[-1];

            assert( h <= 0 );

            if( h != 0 )
            {
                uT = hv;
                icvMatrAXPY3_32f( lh, l-1, hv+1, lduT, uT+1, h );

                s = hv[0] * h;
                for( k = 0; k < l; k++ ) hv[k] = (float)(hv[k]*s);
                hv[0] += 1;
            }
            else
            {
                for( j = 1; j < l; j++ )
                    hv[j] = 0;
                for( j = 1; j < lh; j++ )
                    hv[j * lduT] = 0;
                hv[0] = 1;
            }
        }
        uT = u0;
    }

    /* accumulate right transformations */
    if( vT )
    {
        n1 = n - n1;
        vT = v0 + n1 * ldvT;
        for( i = n1; i < nv; i++, vT += ldvT )
        {
            memset( vT + n1, 0, (n - n1) * sizeof( vT[0] ));
            vT[i] = 1.;
        }

        for( i = n1 - 1; i >= 0; i-- )
        {
            double h, s;
            int lh = nv - i;

            l = n - i;
            hv = v0 + (ldvT + 1) * i;
            h = i == 0 ? kv0 : hv[-1];

            assert( h <= 0 );

            if( h != 0 )
            {
                vT = hv;
                icvMatrAXPY3_32f( lh, l-1, hv+1, ldvT, vT+1, h );

                s = hv[0] * h;
                for( k = 0; k < l; k++ ) hv[k] = (float)(hv[k]*s);
                hv[0] += 1;
            }
            else
            {
                for( j = 1; j < l; j++ )
                    hv[j] = 0;
                for( j = 1; j < lh; j++ )
                    hv[j * ldvT] = 0;
                hv[0] = 1;
            }
        }
        vT = v0;
    }

    for( i = 0; i < nm; i++ )
    {
        double tnorm = fabs( w[i] ) + fabs( e[i] );

        if( anorm < tnorm )
            anorm = tnorm;
    }

    anorm *= FLT_EPSILON;

    /* diagonalization of the bidiagonal form */
    for( k = nm - 1; k >= 0; k-- )
    {
        double z = 0;
        iters = 0;

        for( ;; )               /* do iterations */
        {
            double c, s, f, g, h, x, y;
            int flag = 0;

            /* test for splitting */
            for( l = k; l >= 0; l-- )
            {
                if( fabs( e[l] ) <= anorm )
                {
                    flag = 1;
                    break;
                }
                assert( l > 0 );
                if( fabs( w[l - 1] ) <= anorm )
                    break;
            }

            if( !flag )
            {
                c = 0;
                s = 1;

                for( i = l; i <= k; i++ )
                {
                    double f = s * e[i];

                    e[i] = (float)(e[i]*c);

                    if( anorm + fabs( f ) == anorm )
                        break;

                    g = w[i];
                    h = pythag( f, g );
                    w[i] = (float)h;
                    c = g / h;
                    s = -f / h;

                    if( uT )
                        icvGivens_32f( m, uT + lduT * (i - 1), uT + lduT * i, c, s );
                }
            }

            z = w[k];
            if( l == k || iters++ == MAX_ITERS )
                break;

            /* shift from bottom 2x2 minor */
            x = w[l];
            y = w[k - 1];
            g = e[k - 1];
            h = e[k];
            f = 0.5 * (((g + z) / h) * ((g - z) / y) + y / h - h / y);
            g = pythag( f, 1 );
            if( f < 0 )
                g = -g;
            f = x - (z / x) * z + (h / x) * (y / (f + g) - h);
            /* next QR transformation */
            c = s = 1;

            for( i = l + 1; i <= k; i++ )
            {
                g = e[i];
                y = w[i];
                h = s * g;
                g *= c;
                z = pythag( f, h );
                e[i - 1] = (float)z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = -x * s + g * c;
                h = y * s;
                y *= c;

                if( vT )
                    icvGivens_32f( n, vT + ldvT * (i - 1), vT + ldvT * i, c, s );

                z = pythag( f, h );
                w[i - 1] = (float)z;

                /* rotation can be arbitrary if z == 0 */
                if( z != 0 )
                {
                    c = f / z;
                    s = h / z;
                }
                f = c * g + s * y;
                x = -s * g + c * y;

                if( uT )
                    icvGivens_32f( m, uT + lduT * (i - 1), uT + lduT * i, c, s );
            }

            e[l] = 0;
            e[k] = (float)f;
            w[k] = (float)x;
        }                       /* end of iteration loop */

        if( iters > MAX_ITERS )
            break;

        if( z < 0 )
        {
            w[k] = (float)(-z);
            if( vT )
            {
                for( j = 0; j < n; j++ )
                    vT[j + k * ldvT] = -vT[j + k * ldvT];
            }
        }
    }                           /* end of diagonalization loop */

    /* sort singular values */
    for( i = 0; i < nm; i++ )
    {
        k = i;
        for( j = i + 1; j < nm; j++ )
            if( w[k] < w[j] )
                k = j;

        if( k != i )
        {
            /* swap i & k values */
            float t;
            CV_SWAP( w[i], w[k], t );

            if( vT )
                for( j = 0; j < n; j++ )
                    CV_SWAP( vT[j + ldvT*k], vT[j + ldvT*i], t );

            if( uT )
                for( j = 0; j < m; j++ )
                    CV_SWAP( uT[j + lduT*k], uT[j + lduT*i], t );
        }
    }
	free( hv0 );
}


static void
icvSVBkSb_64f_( int m, int n, const double* w,
               const double* uT, int lduT,
               const double* vT, int ldvT,
               const double* b, int ldb, int nb,
               double* x, int ldx, double* buffer )
{
    double threshold = 0;
    int i, j, nm = (m > n ? n : m);

    if( !b )
        nb = m;

    for( i = 0; i < n; i++ )
        memset( x + i*ldx, 0, nb*sizeof(x[0]));

    for( i = 0; i < nm; i++ )
        threshold += w[i];
    threshold *= 2*DBL_EPSILON;

    /* vT * inv(w) * uT * b */
    for( i = 0; i < nm; i++, uT += lduT, vT += ldvT )
    {
        double wi = w[i];

        if( wi > threshold )
        {
            wi = 1./wi;

            if( nb == 1 )
            {
                double s = 0;
                if( b )
                {
                    if( ldb == 1 )
                    {
                        for( j = 0; j <= m - 4; j += 4 )
                            s += uT[j]*b[j] + uT[j+1]*b[j+1] + uT[j+2]*b[j+2] + uT[j+3]*b[j+3];
                        for( ; j < m; j++ )
                            s += uT[j]*b[j];
                    }
                    else
                    {
                        for( j = 0; j < m; j++ )
                            s += uT[j]*b[j*ldb];
                    }
                }
                else
                    s = uT[0];
                s *= wi;
                if( ldx == 1 )
                {
                    for( j = 0; j <= n - 4; j += 4 )
                    {
                        double t0 = x[j] + s*vT[j];
                        double t1 = x[j+1] + s*vT[j+1];
                        x[j] = t0;
                        x[j+1] = t1;
                        t0 = x[j+2] + s*vT[j+2];
                        t1 = x[j+3] + s*vT[j+3];
                        x[j+2] = t0;
                        x[j+3] = t1;
                    }

                    for( ; j < n; j++ )
                        x[j] += s*vT[j];
                }
                else
                {
                    for( j = 0; j < n; j++ )
                        x[j*ldx] += s*vT[j];
                }
            }
            else
            {
                if( b )
                {
                    memset( buffer, 0, nb*sizeof(buffer[0]));
                    icvMatrAXPY_64f( m, nb, b, ldb, uT, buffer, 0 );
                    for( j = 0; j < nb; j++ )
                        buffer[j] *= wi;
                }
                else
                {
                    for( j = 0; j < nb; j++ )
                        buffer[j] = uT[j]*wi;
                }
                icvMatrAXPY_64f( n, nb, buffer, 0, vT, x, ldx );
            }
        }
    }
}


static void
icvSVBkSb_32f_( int m, int n, const float* w,
               const float* uT, int lduT,
               const float* vT, int ldvT,
               const float* b, int ldb, int nb,
               float* x, int ldx, float* buffer )
{
    float threshold = 0.f;
    int i, j, nm = (m > n ? n : m);

    if( !b )
        nb = m;

    for( i = 0; i < n; i++ )
        memset( x + i*ldx, 0, nb*sizeof(x[0]));

    for( i = 0; i < nm; i++ )
        threshold += w[i];
    threshold *= 2*FLT_EPSILON;

    /* vT * inv(w) * uT * b */
    for( i = 0; i < nm; i++, uT += lduT, vT += ldvT )
    {
        double wi = w[i];
        
        if( wi > threshold )
        {
            wi = 1./wi;

            if( nb == 1 )
            {
                double s = 0;
                if( b )
                {
                    if( ldb == 1 )
                    {
                        for( j = 0; j <= m - 4; j += 4 )
                            s += uT[j]*b[j] + uT[j+1]*b[j+1] + uT[j+2]*b[j+2] + uT[j+3]*b[j+3];
                        for( ; j < m; j++ )
                            s += uT[j]*b[j];
                    }
                    else
                    {
                        for( j = 0; j < m; j++ )
                            s += uT[j]*b[j*ldb];
                    }
                }
                else
                    s = uT[0];
                s *= wi;

                if( ldx == 1 )
                {
                    for( j = 0; j <= n - 4; j += 4 )
                    {
                        double t0 = x[j] + s*vT[j];
                        double t1 = x[j+1] + s*vT[j+1];
                        x[j] = (float)t0;
                        x[j+1] = (float)t1;
                        t0 = x[j+2] + s*vT[j+2];
                        t1 = x[j+3] + s*vT[j+3];
                        x[j+2] = (float)t0;
                        x[j+3] = (float)t1;
                    }

                    for( ; j < n; j++ )
                        x[j] = (float)(x[j] + s*vT[j]);
                }
                else
                {
                    for( j = 0; j < n; j++ )
                        x[j*ldx] = (float)(x[j*ldx] + s*vT[j]);
                }
            }
            else
            {
                if( b )
                {
                    memset( buffer, 0, nb*sizeof(buffer[0]));
                    icvMatrAXPY_32f( m, nb, b, ldb, uT, buffer, 0 );
                    for( j = 0; j < nb; j++ )
                        buffer[j] = (float)(buffer[j]*wi);
                }
                else
                {
                    for( j = 0; j < nb; j++ )
                        buffer[j] = (float)(uT[j]*wi);
                }
                icvMatrAXPY_32f( n, nb, buffer, 0, vT, x, nb );
            }
        }
    }
}


void PNL_API
icvSVBkSb_64f( int m, int n, const double* w,
               const double* uT, int lduT, int nu,
               const double* vT, int ldvT,
               const double* b, int ldb, int nb,
               double* x, double* buffer )
{
    icvSVBkSb_64f_( m, n,  w,
                uT,  lduT,
                vT,  ldvT,
                b,  ldb,  nb,
               x,  nb, buffer );
}


void PNL_API
icvSVBkSb_32f( int m, int n, const float* w,
               const float* uT, int lduT, int nu,
               const float* vT, int ldvT,
               const float* b, int ldb, int nb,
               float* x, float* buffer )
{
    icvSVBkSb_32f_( m, n,  w,
                uT,  lduT,
                vT,  ldvT,
                b,  ldb,  nb,
               x,  nb, buffer );
}


/* End of file. */
