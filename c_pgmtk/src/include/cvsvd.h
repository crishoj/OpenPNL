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

/*
 * OpenCV SVD: interface
 */

#include "pnlConfig.h"

#ifndef _CVSVD_H_
#define _CVSVD_H_

/****************************************************************************\
  icvSVD_64f finds SVD for double-precision matrix,
  icvSVD_32f finds SVD for single-precision matrix:

  a = u w vT    (vT means "v tranposed")
  
  Parameters:
  
  a - input (m x n) matrix, modified by the function
  lda - a's stride (total number of columns in a) (lda >= n)
  m - number of rows in <a>,
  n - number of columns in <a>
  m >= n!
  w - [output] vector of signular values, should contain n elements.
      the singular values are stored in descenting order
  uT - transposed u. u is (m x nu), so uT is (nu x m).
       the pointer may be NULL if the matrix is not needed.
  lduT - uT's stride (total number of columns in uT) (lduT >= m)
  nu - number of columns in u (not in uT) to calculate.
       if uT != NULL, nu should be =n or =m.
       First n columns of u (rows of uT)
       will correspond to singular values in w.
       The rest (if present) is completion
       of the first n vectors to basis of m-dim space.

  vT - transposed v. v is (n x n), so vT is.
       The pointer may be NULL if the matrix is not needed.
  ldvT - vT's stride (total number of columns in vT) (ldvT >= n)

  buffer - temporary buffer of (n+m) elements, used by the function.

\****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif //cpluspus	
	
	void PNL_API icvSVD_64f( double* a, int lda, int m, int n,
                 double* w,
                 double* uT, int lduT, int nu,
                 double* vT, int ldvT, double* buffer );

	void PNL_API icvSVD_32f( float* a, int lda, int m, int n,
                 float* w,
                 float* uT, int lduT, int nu,
                 float* vT, int ldvT,
                 float* buffer );

/****************************************************************************\
  icvSVBkSb_64f performs SV Back Substitution for double-precision matrix,
  icvSVBkSb_32f performs SV Back Substitution for single-precision matrix:

  ax = b, x = inv(a)*b = v * inv(w) * uT * b
  
  Parameters:
  
  m, n, w, uT, lduT, nu, vT, ldvT - the same as in icvSVD_XXX
  b - right-side of equation,
  ldb - b's stride (total number of columns in b)
  nb - number of columns in b to consider (ldb >= nb)
  [number of rows in b should be m]
  x - the back substitution result (continuous array)
  [number of columns in x == x's stride = nb, number of rows == n]

  buffer - temporary buffer of nb elements, used by the function
  (in case of nb==1 the buffer is not used, so the pointer might be 0).
\****************************************************************************/

void PNL_API icvSVBkSb_64f( int m, int n, const double* w,
                    const double* uT, int lduT, int nu,
                    const double* vT, int ldvT,
                    const double* b, int ldb, int nb,
                    double* x, double* buffer );

void PNL_API icvSVBkSb_32f( int m, int n, const float* w,
                    const float* uT, int lduT, int nu,
                    const float* vT, int ldvT,
                    const float* b, int ldb, int nb,
                    float* x, float* buffer );

#ifdef __cplusplus
}
#endif //cpluspus	

#endif/*_CVSVD_H_*/

