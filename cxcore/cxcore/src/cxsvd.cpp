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

/////////////////////////////////////////////////////////////////////////////////////////

#define CX_SWAP( a, b, t ) ((t) = (a), (a) = (b), (b) = (t))

#define icxGivens_64f( n, x, y, c, s ) \
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
icxMatrAXPY_64f( int m, int n, const double* x, int dx,
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
icxMatrAXPY3_64f( int m, int n, const double* x, int l, double* y, double h )
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


#define icxGivens_32f( n, x, y, c, s ) \
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
icxMatrAXPY_32f( int m, int n, const float* x, int dx,
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
icxMatrAXPY3_32f( int m, int n, const float* x, int l, float* y, double h )
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

void icxSVD_64f( double* a, int lda, int m, int n,
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
    double* hv0 = (double*)alloca( (m+1)*sizeof(hv0[0])); 

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
            icxMatrAXPY_64f( m1, n1 - 1, a + 1, lda, hv, temp + 1, 0 );
            for( k = 1; k < n1; k++ ) temp[k] *= h;

            /* modify a: a[i:m,i:n] = a[i:m,i:n] + hv[0:m-i]*temp[0:n-i]' */
            icxMatrAXPY_64f( m1, n1 - 1, temp + 1, 0, hv, a + 1, lda );
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
            icxMatrAXPY3_64f( m1, n1, hv, lda, a, h );

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
                icxMatrAXPY3_64f( lh, l-1, hv+1, lduT, uT+1, h );

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
                icxMatrAXPY3_64f( lh, l-1, hv+1, ldvT, vT+1, h );

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
                        icxGivens_64f( m, uT + lduT * (i - 1), uT + lduT * i, c, s );
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
                    icxGivens_64f( n, vT + ldvT * (i - 1), vT + ldvT * i, c, s );

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
                    icxGivens_64f( m, uT + lduT * (i - 1), uT + lduT * i, c, s );
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
            CX_SWAP( w[i], w[k], t );

            if( vT )
                for( j = 0; j < n; j++ )
                    CX_SWAP( vT[j + ldvT*k], vT[j + ldvT*i], t );

            if( uT )
                for( j = 0; j < m; j++ )
                    CX_SWAP( uT[j + lduT*k], uT[j + lduT*i], t );
        }
    }
}


void icxSVD_32f( float* a, int lda, int m, int n,
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
    float* hv0 = (float*)alloca( (m+1)*sizeof(hv0[0]));

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
            icxMatrAXPY_32f( m1, n1 - 1, a + 1, lda, hv, temp + 1, 0 );

            for( k = 1; k < n1; k++ ) temp[k] = (float)(temp[k]*h);

            /* modify a: a[i:m,i:n] = a[i:m,i:n] + hv[0:m-i]*temp[0:n-i]' */
            icxMatrAXPY_32f( m1, n1 - 1, temp + 1, 0, hv, a + 1, lda );
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
            icxMatrAXPY3_32f( m1, n1, hv, lda, a, h );

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
                icxMatrAXPY3_32f( lh, l-1, hv+1, lduT, uT+1, h );

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
                icxMatrAXPY3_32f( lh, l-1, hv+1, ldvT, vT+1, h );

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
                        icxGivens_32f( m, uT + lduT * (i - 1), uT + lduT * i, c, s );
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
                    icxGivens_32f( n, vT + ldvT * (i - 1), vT + ldvT * i, c, s );

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
                    icxGivens_32f( m, uT + lduT * (i - 1), uT + lduT * i, c, s );
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
            CX_SWAP( w[i], w[k], t );

            if( vT )
                for( j = 0; j < n; j++ )
                    CX_SWAP( vT[j + ldvT*k], vT[j + ldvT*i], t );

            if( uT )
                for( j = 0; j < m; j++ )
                    CX_SWAP( uT[j + lduT*k], uT[j + lduT*i], t );
        }
    }
}


static void
icxSVBkSb_64f( int m, int n, const double* w,
               const double* uT, int lduT,
               const double* vT, int ldvT,
               const double* b, int ldb, int nb,
               double* x, int ldx, double* buffer )
{
    double threshold = 0;
    int i, j, nm = MIN( m, n );

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
                    icxMatrAXPY_64f( m, nb, b, ldb, uT, buffer, 0 );
                    for( j = 0; j < nb; j++ )
                        buffer[j] *= wi;
                }
                else
                {
                    for( j = 0; j < nb; j++ )
                        buffer[j] = uT[j]*wi;
                }
                icxMatrAXPY_64f( n, nb, buffer, 0, vT, x, ldx );
            }
        }
    }
}


static void
icxSVBkSb_32f( int m, int n, const float* w,
               const float* uT, int lduT,
               const float* vT, int ldvT,
               const float* b, int ldb, int nb,
               float* x, int ldx, float* buffer )
{
    float threshold = 0.f;
    int i, j, nm = MIN( m, n );

    if( !b )
        nb = m;

    memset( x, 0, nb*n*sizeof(x[0]));

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
                    icxMatrAXPY_32f( m, nb, b, ldb, uT, buffer, 0 );
                    for( j = 0; j < nb; j++ )
                        buffer[j] = (float)(buffer[j]*wi);
                }
                else
                {
                    for( j = 0; j < nb; j++ )
                        buffer[j] = (float)(uT[j]*wi);
                }
                icxMatrAXPY_32f( n, nb, buffer, 0, vT, x, nb );
            }
        }
    }
}


CX_IMPL  void
cxSVD( CxArr* aarr, CxArr* warr, CxArr* uarr, CxArr* varr, int flags )
{
    uchar* buffer = 0;
    int local_alloc = 0;

    CX_FUNCNAME( "cxSVD" );

    __BEGIN__;

    CxMat astub, *a = (CxMat*)aarr;
    CxMat wstub, *w = (CxMat*)warr;
    CxMat ustub, *u;
    CxMat vstub, *v;
    CxMat tmat;
    uchar* tw = 0;
    int type;
    int a_buf_offset = 0, u_buf_offset = 0, buf_size, pix_size;
    int temp_u = 0, // temporary storage for U is needed
        t_svd; // special case: a->rows < a->cols
    int m, n;
    int w_rows, w_cols;
    int u_rows = 0, u_cols = 0;

    if( !CX_IS_MAT( a ))
        CX_CALL( a = cxGetMat( a, &astub ));

    if( !CX_IS_MAT( w ))
        CX_CALL( w = cxGetMat( w, &wstub ));

    if( !CX_ARE_TYPES_EQ( a, w ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( a->rows >= a->cols )
    {
        m = a->rows;
        n = a->cols;
        w_rows = w->rows;
        w_cols = w->cols;
        t_svd = 0;
    }
    else
    {
        CxArr* t;
        CX_SWAP( uarr, varr, t );

        flags = (flags & CX_SVD_U_T ? CX_SVD_V_T : 0)|
                (flags & CX_SVD_V_T ? CX_SVD_U_T : 0);
        m = a->cols;
        n = a->rows;
        w_rows = w->cols;
        w_cols = w->rows;
        t_svd = 1;
    }

    u = (CxMat*)uarr;
    v = (CxMat*)varr;

    if( (w_cols == 1 || w_rows == 1) &&
        CX_IS_MAT_CONT(w->type) && w_cols + w_rows - 1 == n )
        tw = w->data.ptr;

    if( u )
    {
        if( !CX_IS_MAT( u ))
            CX_CALL( u = cxGetMat( u, &ustub ));

        if( !(flags & CX_SVD_U_T) )
        {
            u_rows = u->rows;
            u_cols = u->cols;
        }
        else
        {
            u_rows = u->cols;
            u_cols = u->rows;
        }

        if( !CX_ARE_TYPES_EQ( a, u ))
            CX_ERROR( CX_StsUnmatchedFormats, "" );

        if( u_rows != m || (u_cols != m && u_cols != n))
            CX_ERROR( CX_StsUnmatchedSizes, "U matrix has unappropriate size" );
            
        temp_u = (u_rows != u_cols && !(flags & CX_SVD_U_T)) || u->data.ptr==a->data.ptr;

        if( tw == 0 && u_cols != w_rows )
            CX_ERROR( CX_StsUnmatchedSizes, "U and W have incompatible sizes" );
    }
    else
    {
        u = &ustub;
        u->data.ptr = 0;
        u->step = 0;
    }

    if( v )
    {
        int v_rows, v_cols;

        if( !CX_IS_MAT( v ))
            CX_CALL( v = cxGetMat( v, &vstub ));

        if( !(flags & CX_SVD_V_T) )
        {
            v_rows = v->rows;
            v_cols = v->cols;
        }
        else
        {
            v_rows = v->cols;
            v_cols = v->rows;
        }

        if( !CX_ARE_TYPES_EQ( a, v ))
            CX_ERROR( CX_StsUnmatchedFormats, "" );

        if( v_rows != n || v_cols != n )
            CX_ERROR( CX_StsUnmatchedSizes, "V matrix has unappropriate size" );

        if( tw == 0 && w_cols != v_cols )
            CX_ERROR( CX_StsUnmatchedSizes, "V and W have incompatible sizes" );
    }
    else
    {
        v = &vstub;
        v->data.ptr = 0;
        v->step = 0;
    }

    type = CX_MAT_TYPE( a->type );
    pix_size = icxPixSize[type];
    buf_size = n*2 + m;

    if( !(flags & CX_SVD_MODIFY_A) )
    {
        a_buf_offset = buf_size;
        buf_size += a->rows*a->cols;
    }

    if( temp_u )
    {
        u_buf_offset = buf_size;
        buf_size += u->rows*u->cols;
    }

    buf_size *= pix_size;

    if( buf_size <= CX_MAX_LOCAL_SIZE )
    {
        buffer = (uchar*)alloca( buf_size );
        local_alloc = 1;
    }
    else
    {
        CX_CALL( buffer = (uchar*)cxAlloc( buf_size ));
    }
    
    if( !(flags & CX_SVD_MODIFY_A) )
    {
        cxInitMatHeader( &tmat, m, n, type,
                         buffer + a_buf_offset*pix_size );
        if( !t_svd )
            cxCopy( a, &tmat );
        else
            cxT( a, &tmat );
        a = &tmat;
    }

    if( temp_u )
    {
        cxInitMatHeader( &ustub, u_cols, u_rows, type, buffer + u_buf_offset*pix_size );
        u = &ustub;
    }

    if( !tw )
        tw = buffer + (n + m)*pix_size;

    if( type == CX_32FC1 )
    {
        icxSVD_32f( a->data.fl, a->step/sizeof(float), a->rows, a->cols,
                   (float*)tw, u->data.fl, u->step/sizeof(float), u_cols,
                   v->data.fl, v->step/sizeof(float), (float*)buffer );
    }
    else if( type == CX_64FC1 )
    {
        icxSVD_64f( a->data.db, a->step/sizeof(double), a->rows, a->cols,
                    (double*)tw, u->data.db, u->step/sizeof(double), u_cols,
                    v->data.db, v->step/sizeof(double), (double*)buffer );
    }
    else
    {
        CX_ERROR( CX_StsUnsupportedFormat, "" );
    }

    if( tw != w->data.ptr )
    {
        cxSetZero( w );
        if( type == CX_32FC1 )
            for( int i = 0; i < n; i++ )
                ((float*)(w->data.ptr + i*w->step))[i] = ((float*)tw)[i];
        else
            for( int i = 0; i < n; i++ )
                ((double*)(w->data.ptr + i*w->step))[i] = ((double*)tw)[i];
    }

    if( uarr )
    {
        if( !(flags & CX_SVD_U_T))
            cxT( u, uarr );
        else if( temp_u )
            cxCopy( u, uarr );
        /*CX_CHECK_NANS( uarr );*/
    }

    if( varr )
    {
        if( !(flags & CX_SVD_V_T))
            cxT( v, varr );
        /*CX_CHECK_NANS( varr );*/
    }

    CX_CHECK_NANS( w );

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );
}


CX_IMPL void
cxSVBkSb( const CxArr* warr, const CxArr* uarr,
          const CxArr* varr, const CxArr* barr,
          CxArr* xarr, int flags )
{
    uchar* buffer = 0;
    int local_alloc = 0;

    CX_FUNCNAME( "cxSVBkSb" );

    __BEGIN__;

    CxMat wstub, *w = (CxMat*)warr;
    CxMat bstub, *b = (CxMat*)barr;
    CxMat xstub, *x = (CxMat*)xarr;
    CxMat ustub, ustub2, *u = (CxMat*)uarr;
    CxMat vstub, vstub2, *v = (CxMat*)varr;
    uchar* tw = 0;
    int type;
    int temp_u = 0, temp_v = 0;
    int u_buf_offset = 0, v_buf_offset = 0, w_buf_offset = 0, t_buf_offset = 0;
    int buf_size = 0, pix_size;
    int m, n, nm;
    int u_rows, u_cols;
    int v_rows, v_cols;

    if( !CX_IS_MAT( w ))
        CX_CALL( w = cxGetMat( w, &wstub ));

    if( !CX_IS_MAT( u ))
        CX_CALL( w = cxGetMat( u, &ustub ));

    if( !CX_IS_MAT( v ))
        CX_CALL( v = cxGetMat( v, &vstub ));

    if( !CX_IS_MAT( x ))
        CX_CALL( x = cxGetMat( x, &xstub ));

    if( !CX_ARE_TYPES_EQ( w, u ) || !CX_ARE_TYPES_EQ( w, v ) || !CX_ARE_TYPES_EQ( w, x ))
        CX_ERROR( CX_StsUnmatchedFormats, "All matrices must have the same type" );

    type = CX_MAT_TYPE( w->type );
    pix_size = icxPixSize[type];

    if( !(flags & CX_SVD_U_T) )
    {
        temp_u = 1;
        u_buf_offset = buf_size;
        buf_size += u->cols*u->rows*pix_size;
        u_rows = u->rows;
        u_cols = u->cols;
    }
    else
    {
        u_rows = u->cols;
        u_cols = u->rows;
    }

    if( !(flags & CX_SVD_V_T) )
    {
        temp_v = 1;
        v_buf_offset = buf_size;
        buf_size += v->cols*v->rows*pix_size;
        v_rows = v->rows;
        v_cols = v->cols;
    }
    else
    {
        v_rows = v->cols;
        v_cols = v->rows;
    }

    m = u_rows;
    n = v_rows;
    nm = MIN(n,m);

    if( (u_rows != u_cols && v_rows != v_cols) || x->rows != v_rows )
        CX_ERROR( CX_StsBadSize, "V or U matrix must be square" );

    if( (w->rows == 1 || w->cols == 1) && CX_IS_MAT_CONT(w->type) &&
        w->rows + w->cols - 1 == MIN( m, n ))
    {
        tw = w->data.ptr;
    }
    else
    {
        if( w->cols != v_cols || w->rows != u_cols )
            CX_ERROR( CX_StsBadSize, "W must be 1d array of MIN(m,n) elements or "
                                    "matrix which size matches to U and V" );
        w_buf_offset = buf_size;
        buf_size += n*pix_size;
    }

    if( b )
    {
        if( !CX_IS_MAT( b ))
            CX_CALL( b = cxGetMat( b, &bstub ));
        if( !CX_ARE_TYPES_EQ( w, b ))
            CX_ERROR( CX_StsUnmatchedFormats, "All matrices must have the same type" );
        if( b->cols != x->cols || b->rows != m )
            CX_ERROR( CX_StsUnmatchedSizes, "b matrix must have (m x x->cols) size" );
    }
    else
    {
        b = &bstub;
        memset( b, 0, sizeof(*b));
    }

    t_buf_offset = buf_size;
    buf_size += MAX(m,n)*pix_size;

    if( buf_size <= CX_MAX_LOCAL_SIZE )
    {
        buffer = (uchar*)alloca( buf_size );
        local_alloc = 1;
    }
    else
        CX_CALL( buffer = (uchar*)cxAlloc( buf_size ));

    if( temp_u )
    {
        cxInitMatHeader( &ustub2, u_cols, u_rows, type, buffer + u_buf_offset );
        cxT( u, &ustub2 );
        u = &ustub2;
    }

    if( temp_v )
    {
        cxInitMatHeader( &vstub2, v_cols, v_rows, type, buffer + v_buf_offset );
        cxT( v, &vstub2 );
        v = &vstub2;
    }

    if( !tw )
    {
        int i;
        tw = buffer + w_buf_offset;
        for( i = 0; i < nm; i++ )
            memcpy( tw + i*pix_size, w->data.ptr + i*(w->step + pix_size), pix_size );
    }

    if( type == CX_32FC1 )
    {
        icxSVBkSb_32f( m, n, (float*)tw, u->data.fl, u->step/sizeof(float),
                       v->data.fl, v->step/sizeof(float),
                       b->data.fl, b->step/sizeof(float), b->cols,
                       x->data.fl, x->step/sizeof(float),
                       (float*)(buffer + t_buf_offset) );
    }
    else if( type == CX_64FC1 )
    {
        icxSVBkSb_64f( m, n, (double*)tw, u->data.db, u->step/sizeof(double),
                       v->data.db, v->step/sizeof(double),
                       b->data.db, b->step/sizeof(double), b->cols,
                       x->data.db, x->step/sizeof(double),
                       (double*)(buffer + t_buf_offset) );
    }
    else
    {
        CX_ERROR( CX_StsUnsupportedFormat, "" );
    }

    __END__;

    if( buffer && !local_alloc )
        cxFree( (void**)&buffer );
}

/* End of file. */
