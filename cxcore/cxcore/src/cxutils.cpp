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
#include <time.h>

CX_IMPL CxRect
cxMaxRect( const CxRect* rect1, const CxRect* rect2 )
{
    if( rect1 && rect2 )
    {
        CxRect max_rect;
        int a, b;

        max_rect.x = a = rect1->x;
        b = rect2->x;
        if( max_rect.x > b )
            max_rect.x = b;

        max_rect.width = a += rect1->width;
        b += rect2->width;

        if( max_rect.width < b )
            max_rect.width = b;
        max_rect.width -= max_rect.x;

        max_rect.y = a = rect1->y;
        b = rect2->y;
        if( max_rect.y > b )
            max_rect.y = b;

        max_rect.height = a += rect1->height;
        b += rect2->height;

        if( max_rect.height < b )
            max_rect.height = b;
        max_rect.height -= max_rect.y;
        return max_rect;
    }
    else if( rect1 )
        return *rect1;
    else if( rect2 )
        return *rect2;
    else
        return cxRect(0,0,0,0);
}


CX_IMPL void
cxBoxPoints( CxBox2D box, CxPoint2D32f pt[4] )
{
    CX_FUNCNAME( "cxBoxPoints" );

    __BEGIN__;
    
    float a = (float)cos(box.angle)*0.5f;
    float b = (float)sin(box.angle)*0.5f;

    if( !pt )
        CX_ERROR( CX_StsNullPtr, "NULL vertex array pointer" );

    pt[0].x = box.center.x - a*box.size.height - b*box.size.width;
    pt[0].y = box.center.y + b*box.size.height - a*box.size.width;
    pt[1].x = box.center.x + a*box.size.height - b*box.size.width;
    pt[1].y = box.center.y - b*box.size.height - a*box.size.width;
    pt[2].x = 2*box.center.x - pt[0].x;
    pt[2].y = 2*box.center.y - pt[0].y;
    pt[3].x = 2*box.center.x - pt[1].x;
    pt[3].y = 2*box.center.y - pt[1].y;

    __END__;
}


CX_IMPL void
cxKMeans2( const CxArr* samples_arr, int cluster_count,
           CxArr* labels_arr, CxTermCriteria termcrit )
{
    CxMat* centers = 0;
    CxMat* old_centers = 0;
    CxMat* counters = 0;
    
    CX_FUNCNAME( "cxKMeans2" );

    __BEGIN__;

    CxMat samples_stub, *samples = (CxMat*)samples_arr;
    CxMat cluster_idx_stub, *labels = (CxMat*)labels_arr;
    CxMat* temp = 0;
    CxRandState rng;
    int i, k, sample_count, dims;
    int ids_delta, iter;
    double max_dist;
    int pix_size;

    if( !CX_IS_MAT( samples ))
        CX_CALL( samples = cxGetMat( samples, &samples_stub ));
    
    if( !CX_IS_MAT( labels ))
        CX_CALL( labels = cxGetMat( labels, &cluster_idx_stub ));

    if( cluster_count < 1 )
        CX_ERROR( CX_StsOutOfRange, "Number of clusters should be positive" );

    if( CX_MAT_DEPTH(samples->type) != CX_32F || CX_MAT_TYPE(labels->type) != CX_32SC1 )
        CX_ERROR( CX_StsUnsupportedFormat,
        "samples should be floating-point matrix, cluster_idx - integer vector" );

    pix_size = CX_ELEM_SIZE(samples->type);

    if( labels->rows != 1 && labels->cols != 1 || labels->rows + labels->cols - 1 != samples->rows )
        CX_ERROR( CX_StsUnmatchedSizes,
        "cluster_idx should be 1D vector of the same number of elements as samples' number of rows" ); 

    switch( termcrit.type )
    {
    case CX_TERMCRIT_EPS:
        if( termcrit.epsilon < 0 )
            termcrit.epsilon = 0;
        termcrit.maxIter = 100;
        break;
    case CX_TERMCRIT_ITER:
        if( termcrit.maxIter < 1 )
            termcrit.maxIter = 1;
        termcrit.epsilon = 1e-6;
        break;
    case CX_TERMCRIT_EPS|CX_TERMCRIT_ITER:
        if( termcrit.epsilon < 0 )
            termcrit.epsilon = 0;
        if( termcrit.maxIter < 1 )
            termcrit.maxIter = 1;
        break;
    default:
        CX_ERROR( CX_StsBadArg, "Invalid termination criteria" );
    }

    termcrit.epsilon *= termcrit.epsilon;

    sample_count = samples->rows;

    if( cluster_count > sample_count )
        cluster_count = sample_count;

    dims = samples->cols*CX_MAT_CN(samples->type);
    ids_delta = labels->step ? labels->step/(int)sizeof(int) : 1;

    cxRandInit( &rng, 0, 1, -1, CX_RAND_UNI );

    CX_CALL( centers = cxCreateMat( cluster_count, dims, CX_64FC1 ));
    CX_CALL( old_centers = cxCreateMat( cluster_count, dims, CX_64FC1 ));
    // samples_count >= cluster_count, <samples_count>
    // elements are used during initialization
#if 0    
    CX_CALL( counters = cxCreateMat( 1, sample_count, CX_32SC1 ));
    cxZero( counters );

    // init centers
    for( i = 0; i < cluster_count; i++ )
    {
        int j, idx;
        double* c = (double*)(centers->data.ptr + i*centers->step);
        float* s;
        do
            idx = cxRandNext( &rng ) % cluster_count;
        while( counters->data.i[idx] != 0 );
        counters->data.i[idx] = 1;

        s = (float*)(samples->data.ptr + idx*samples->step);
        for( j = 0; j < samples->cols; j++ )
            c[j] = s[j];
    }

    counters->cols = cluster_count;
#else
    CX_CALL( counters = cxCreateMat( 1, cluster_count, CX_32SC1 ));

    // init centers
    for( i = 0, k = 0; i < sample_count; i++ )
    {
        labels->data.i[i] = k;
        k = k < cluster_count-1 ? k+1 : 0;
    }
#endif

    counters->cols = cluster_count; // cut down counters
    max_dist = termcrit.epsilon*2;

    for( iter = 0; iter < termcrit.maxIter; iter++ )
    {
        int i, j, k;

        // computer centers
        cxZero( centers );
        cxZero( counters );

        for( i = 0; i < sample_count; i++ )
        {
            float* s = (float*)(samples->data.ptr + i*samples->step);
            int k = labels->data.i[i*ids_delta];
            double* c = (double*)(centers->data.ptr + k*centers->step);
            j = 0;
            for( ; j <= dims - 4; j += 4 )
            {
                double t0 = c[j] + s[j];
                double t1 = c[j+1] + s[j+1];

                c[j] = t0;
                c[j+1] = t1;

                t0 = c[j+2] + s[j+2];
                t1 = c[j+3] + s[j+3];

                c[j+2] = t0;
                c[j+3] = t1;
            }
            for( ; j < dims; j++ )
                c[j] += s[j];
            counters->data.i[k]++;
        }

        if( iter > 0 )
            max_dist = 0;

        for( k = 0; k < cluster_count; k++ )
        {
            double* c = (double*)(centers->data.ptr + k*centers->step);
            if( counters->data.i[k] != 0 )
            {
                double scale = 1./counters->data.i[k];
                for( j = 0; j < dims; j++ )
                    c[j] *= scale;
            }
            else
            {
                int i = cxRandNext( &rng ) % sample_count;
                float* s = (float*)(samples->data.ptr + i*samples->step);
                for( j = 0; j < dims; j++ )
                    c[j] = s[j];
            }
            
            if( iter > 0 )
            {
                double dist = 0;
                double* c_o = (double*)(old_centers->data.ptr + k*old_centers->step);
                for( j = 0; j < dims; j++ )
                {
                    double t = c[j] - c_o[j];
                    dist += t*t;
                }
                if( max_dist < dist )
                    max_dist = dist;
            }
        }

        // assign labels
        for( i = 0; i < sample_count; i++ )
        {
            float* s = (float*)(samples->data.ptr + i*samples->step);
            int k_best = 0;
            double min_dist = DBL_MAX;

            for( k = 0; k < cluster_count; k++ )
            {
                double* c = (double*)(centers->data.ptr + k*centers->step);
                double dist = 0;
                
                j = 0;
                for( ; j <= dims - 4; j += 4 )
                {
                    double t0 = c[j] - s[j];
                    double t1 = c[j+1] - s[j+1];
                    dist += t0*t0 + t1*t1;
                    t0 = c[j+2] - s[j+2];
                    t1 = c[j+3] - s[j+3];
                    dist += t0*t0 + t1*t1;
                }

                for( ; j < dims; j++ )
                {
                    double t = c[j] - s[j];
                    dist += t*t;
                }
                
                if( min_dist > dist )
                {
                    min_dist = dist;
                    k_best = k;
                }
            }

            labels->data.i[i*ids_delta] = k_best;
        }

        if( max_dist < termcrit.epsilon )
            break;

        CX_SWAP( centers, old_centers, temp );
    }

    __END__;

    cxReleaseMat( &centers );
    cxReleaseMat( &old_centers );
    cxReleaseMat( &counters );
}


typedef int64 (CX_CDECL * rdtsc_func)(void);

/* helper functions for RNG initialization and accurate time measurement: x86 only */
CX_IMPL  int64  cxGetTickCount( void )
{
#ifndef WIN32
    return clock();
#else
    static const char code[] = "\x0f\x31\xc3";
    rdtsc_func func = (rdtsc_func)(void*)code;
    return func();
#endif
}

CX_IMPL  double  cxGetTickFrequency()
{
#ifndef WIN32
    return CLOCKS_PER_SEC*1e-6;
#else
    int64 clocks1, clocks2;
    volatile int t;
    int dt = 100;
    int frequency = 0, old_frequency;

    do
    {
        old_frequency = frequency;
        t = clock();
        while( t==clock() );
        t = clock();

        clocks1 = cxGetTickCount();
        while( dt+t>clock() );
        clocks2 = cxGetTickCount();

        frequency = (int)(((double)(clocks2 - clocks1))/(1e3*dt)+.5) + 10;
        if( frequency % 50 <= 16 )
            frequency = (frequency/50)*50;
        else
            frequency = (frequency/100)*100 + ((frequency % 100)/33)*33;
    }
    while( frequency != old_frequency );
    return (double)frequency;
#endif
}

/* End of file. */
