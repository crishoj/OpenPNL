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


#ifndef _CXCORE_H_
#define _CXCORE_H_

#ifdef __IPL_H__
#define HAVE_IPL
#endif

#ifdef _CV_H_
#define HAVE_IPL
#endif 

#if defined(_CH_)
#pragma package <opencx>
#include <chdl.h>
LOAD_CHDL_CODE(cx,Cx)
#endif

#if defined HAVE_IPL && !defined __IPL_H__ && !defined _CV_H_
#ifndef _INC_WINDOWS
    #define CX_PRETEND_WINDOWS
    #define _INC_WINDOWS
    typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER;
    typedef int BOOL;
#endif
#ifdef WIN32
#include "ipl.h"
#else
#include "ipl/ipl.h"
#endif
#ifdef CX_PRETEND_WINDOWS
    #undef _INC_WINDOWS
#endif
#endif

#include "cxtypes.h"
#include "cxerror.h"

#ifndef CXCOREAPI
#if defined WIN32 && defined CXCORE_DLL
    #define CXCOREAPI __declspec(dllexport)
#else
    #define CXCOREAPI
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************\
*                                     Allocation/deallocation                            *
\****************************************************************************************/

/* <malloc> wrapper.
   If there is no enough memory the function
   (as well as other OpenCX functions that call cxAlloc)
   raises an error. */
CXCOREAPI  void*  cxAlloc( size_t size );

/* <free> wrapper.
   Here and further all the memory releasing functions
   (that all call cxFree) take double pointer which is used
   to clear user pointer to the data after releasing it.
   Passing pointer to NULL pointer is Ok: nothing happens in this case
*/
CXCOREAPI  void   cxFree( void** ptr );

/* Allocates and initializes IplImage header */
CXCOREAPI  IplImage*  cxCreateImageHeader( CxSize size, int depth, int channels );

/* Inializes IplImage header */
CXCOREAPI IplImage* cxInitImageHeader( IplImage* image, CxSize size, int depth,
                                       int channels, int origin CX_DEFAULT(0),
                                       int align CX_DEFAULT(4));

/* Creates IPL image (header and data) */
CXCOREAPI  IplImage*  cxCreateImage( CxSize size, int depth, int channels );

/* Releases (i.e. deallocates) IPL image header */
CXCOREAPI  void  cxReleaseImageHeader( IplImage** image );

/* Releases IPL image header and data */
CXCOREAPI  void  cxReleaseImage( IplImage** image );

/* Creates a copy of IPL image (widthStep may differ) */
CXCOREAPI IplImage* cxCloneImage( const IplImage* image );

/* Sets a Channel Of Interest (only a few functions support COI) - 
   use cxCopy to extract the selected channel and/or put it back */
CXCOREAPI  void  cxSetImageCOI( IplImage* image, int coi );

/* Retrieves image Channel Of Interest */
CXCOREAPI  int  cxGetImageCOI( IplImage* image );

/* Sets image ROI (region of interest) (COI is not changed) */
CXCOREAPI  void  cxSetImageROI( IplImage* image, CxRect rect );

/* Resets image ROI and COI */
CXCOREAPI  void  cxResetImageROI( IplImage* image );

/* Retrieves image ROI */
CXCOREAPI  CxRect cxGetImageROI( const IplImage* image );

/* Allocates and initalizes CxMat header */
CXCOREAPI  CxMat*  cxCreateMatHeader( int rows, int cols, int type );

#define CX_AUTOSTEP  0x7fffffff

/* Initializes CxMat header */
CXCOREAPI CxMat* cxInitMatHeader( CxMat* mat, int rows, int cols,
                                  int type, void* data CX_DEFAULT(NULL),
                                  int step CX_DEFAULT(CX_AUTOSTEP) );

/* Allocates and initializes CxMat header and allocates data */
CXCOREAPI  CxMat*  cxCreateMat( int rows, int cols, int type );

/* Releases CxMat header and deallocates matrix data
   (reference counting is used for data) */
CXCOREAPI  void  cxReleaseMat( CxMat** mat );

/* Decrements CxMat data reference counter and deallocates the data if
   it reaches 0 */
CX_INLINE  void  cxDecRefData( CxArr* arr );
CX_INLINE  void  cxDecRefData( CxArr* arr )
{
    if( CX_IS_MAT( arr ) || CX_IS_MATND( arr ))
    {
        CxMat* mat = (CxMat*)arr; /* the first few fields of CxMat and CxMatND are the same */
        mat->data.ptr = NULL;
        if( mat->refcount != NULL && --*mat->refcount == 0 )
        {
            uchar* data = (uchar*)mat->refcount + 2*sizeof(mat->refcount);
            cxFree( (void**)&data );
        }
        mat->refcount = NULL;
    }
}

/* Increments CxMat data reference counter */
CX_INLINE  int  cxIncRefData( CxArr* arr );
CX_INLINE  int  cxIncRefData( CxArr* arr )
{
    int refcount = 0;
    if( CX_IS_MAT( arr ) || CX_IS_MATND( arr ))
    {
        CxMat* mat = (CxMat*)arr;
        if( mat->refcount != NULL )
            refcount = ++*mat->refcount;
    }
    return refcount;
}


/* Creates an exact copy of the input matrix (except, may be, step value) */
CXCOREAPI CxMat* cxCloneMat( const CxMat* mat );


/* Makes a new matrix from <rect> subrectangle of input array.
   No data is copied */
CXCOREAPI CxMat* cxGetSubRect( const CxArr* arr, CxMat* submat, CxRect rect );
#define cxGetSubArr cxGetSubRect

/* Selects row span of the input array: arr(start_row:delta_row:end_row,:)
    (end_row is not included into the span). */
CXCOREAPI CxMat* cxGetRows( const CxArr* arr, CxMat* submat,
                            int start_row, int end_row,
                            int delta_row CX_DEFAULT(1));

CX_INLINE  void  cxGetRow( const CxArr* arr, CxMat* submat, int row );
CX_INLINE  void  cxGetRow( const CxArr* arr, CxMat* submat, int row )
{
    cxGetRows( arr, submat, row, row + 1, 1 );
}


/* Selects column span of the input array: arr(:,start_col:end_col)
   (end_col is not included into the span) */
CXCOREAPI CxMat* cxGetCols( const CxArr* arr, CxMat* submat,
                            int start_col, int end_col );

CX_INLINE  void  cxGetCol( const CxArr* arr, CxMat* submat, int col );
CX_INLINE  void  cxGetCol( const CxArr* arr, CxMat* submat, int col )
{
    cxGetCols( arr, submat, col, col + 1 );
}

/* Select a diagonal of the input array.
   (diag = 0 means the main diagonal, >0 means a diagonal above the main one,
   <0 - below the main one).
   The diagonal will be represented as a column (nx1 matrix). */
CXCOREAPI CxMat* cxGetDiag( const CxArr* arr, CxMat* submat,
                            int diag CX_DEFAULT(0));

/* low-level scalar <-> raw data conversion functions */
CXCOREAPI void cxScalarToRawData( const CxScalar* scalar, void* data, int type,
                                  int extend_to_12 CX_DEFAULT(0) );

CXCOREAPI void cxRawDataToScalar( const void* data, int type, CxScalar* scalar );

/* Allocates and initializes CxMatND header */
CXCOREAPI  CxMatND*  cxCreateMatNDHeader( int dims, const int* sizes, int type );

/* Allocates and initializes CxMatND header and allocates data */
CXCOREAPI  CxMatND*  cxCreateMatND( int dims, const int* sizes, int type );

/* Initializes preallocated CxMatND header */
CXCOREAPI  CxMatND*  cxInitMatNDHeader( CxMatND* mat, int dims, const int* sizes,
                                        int type, void* data CX_DEFAULT(NULL) );

/* Releases CxMatND */
CX_INLINE  void  cxReleaseMatND( CxMatND** mat );
CX_INLINE  void  cxReleaseMatND( CxMatND** mat )
{
    cxReleaseMat( (CxMat**)mat );
}

/* Creates a copy of CxMatND (except, may be, steps) */
CXCOREAPI  CxMatND* cxCloneMatND( const CxMatND* mat );

/* Allocates and initializes CxSparseMat header and allocates data */
CXCOREAPI  CxSparseMat*  cxCreateSparseMat( int dims, const int* sizes, int type );

/* Releases CxSparseMat */
CXCOREAPI  void  cxReleaseSparseMat( CxSparseMat** mat );

/* Creates a copy of CxSparseMat (except, may be, zero items) */
CXCOREAPI  CxSparseMat* cxCloneSparseMat( const CxSparseMat* mat );

/* Initializes sparse array iterator
   (returns the first node or NULL if the array is empty) */
CXCOREAPI  CxSparseNode* cxInitSparseMatIterator( const CxSparseMat* mat,
                                                  CxSparseMatIterator* matIterator );

// returns next sparse array node (or NULL if there is no more nodes)
CX_INLINE CxSparseNode* cxGetNextSparseNode( CxSparseMatIterator* matIterator );
CX_INLINE CxSparseNode* cxGetNextSparseNode( CxSparseMatIterator* matIterator )
{
    if( matIterator->node->next )
        return matIterator->node = matIterator->node->next;
    else
    {
        int idx;
        for( idx = ++matIterator->curidx; idx < matIterator->mat->hashsize; idx++ )
        {
            CxSparseNode* node = (CxSparseNode*)matIterator->mat->hashtable[idx];
            if( node )
            {
                matIterator->curidx = idx;
                return matIterator->node = node;
            }
        }
        return NULL;
    }
}


#define CX_MAX_ARR 10

typedef struct CxNArrayIterator
{
    int count; /* number of arrays */
    int dims; /* number of dimensions to iterate */
    CxSize size; /* maximal common linear size: { width = size, height = 1 } */
    uchar* ptr[CX_MAX_ARR]; /* pointers to the array slices */
    int stack[CX_MAX_DIM]; /* for internal use */
    CxMatND* hdr[CX_MAX_ARR]; /* pointers to the headers of the
                                 matrices that are processed */
}
CxNArrayIterator;

#define CX_NO_DEPTH_CHECK     1
#define CX_NO_CN_CHECK        2
#define CX_NO_SIZE_CHECK      4

/* initializes iterator that traverses through several arrays simulteneously
   (the function together with cxNextArraySlice is used for
    N-ari element-wise operations) */
CXCOREAPI int cxInitNArrayIterator( int count, CxArr** arrs,
                                    const CxArr* mask, CxMatND* stubs,
                                    CxNArrayIterator* arrayIterator,
                                    int flags CX_DEFAULT(0) );

/* returns zero value if iteration is finished, non-zero (slice length) otherwise */
CXCOREAPI int cxNextNArraySlice( CxNArrayIterator* arrayIterator );


/* Returns type of array elements:
   CX_8UC1 ... CX_64FC4 ... */
CXCOREAPI  int cxGetElemType( const CxArr* arr );

/* Retrieves number of an array dimensions and
   optionally sizes of the dimensions */
CXCOREAPI  int cxGetDims( const CxArr* arr, int* sizes CX_DEFAULT(NULL) );


/* Retrieves size of a particular array dimension.
   For 2d arrays cxGetDimSize(arr,0) returns number of rows (image height)
   and cxGetDimSize(arr,1) returns number of columns (image width) */
CXCOREAPI  int cxGetDimSize( const CxArr* arr, int index );


/* ptr = &arr(idx1,idx2,...). All indexes are zero-based,
   the major dimensions go first (e.g. (y,x) for 2D, (z,y,x) for 3D */
CXCOREAPI uchar* cxPtr1D( const CxArr* arr, int idx1, int* type CX_DEFAULT(NULL));
CXCOREAPI uchar* cxPtr2D( const CxArr* arr, int idx1, int idx2, int* type CX_DEFAULT(NULL) );
CXCOREAPI uchar* cxPtr3D( const CxArr* arr, int idx1, int idx2, int idx3,
                          int* type CX_DEFAULT(NULL));

/* For CxMat or IplImage number of indices should be 2
   (row index (y) goes first, column index (x) goes next).
   For CxMatND or CxSparseMat number of infices should match number of <dims> and
   indices order should match the array dimension order. */
CXCOREAPI uchar* cxPtrND( const CxArr* arr, int* idx, int* type CX_DEFAULT(NULL),
                          int create_node CX_DEFAULT(1),
                          unsigned* precalc_hashval CX_DEFAULT(NULL));

/* value = arr(idx1,idx2,...) */
CXCOREAPI CxScalar cxGet1D( const CxArr* arr, int idx1 );
CXCOREAPI CxScalar cxGet2D( const CxArr* arr, int idx1, int idx2 );
CXCOREAPI CxScalar cxGet3D( const CxArr* arr, int idx1, int idx2, int idx3 );
CXCOREAPI CxScalar cxGetND( const CxArr* arr, int* idx );

/* for 1-channel arrays */
CXCOREAPI double cxGetReal1D( const CxArr* arr, int idx1 );
CXCOREAPI double cxGetReal2D( const CxArr* arr, int idx1, int idx2 );
CXCOREAPI double cxGetReal3D( const CxArr* arr, int idx1, int idx2, int idx3 );
CXCOREAPI double cxGetRealND( const CxArr* arr, int* idx );

/* arr(idx1,idx2,...) = value */
CXCOREAPI void cxSet1D( CxArr* arr, int idx1, CxScalar value );
CXCOREAPI void cxSet2D( CxArr* arr, int idx1, int idx2, CxScalar value );
CXCOREAPI void cxSet3D( CxArr* arr, int idx1, int idx2, int idx3, CxScalar value );
CXCOREAPI void cxSetND( CxArr* arr, int* idx, CxScalar value );

/* for 1-channel arrays */
CXCOREAPI void cxSetReal1D( CxArr* arr, int idx1, double value );
CXCOREAPI void cxSetReal2D( CxArr* arr, int idx1, int idx2, double value );
CXCOREAPI void cxSetReal3D( CxArr* arr, int idx1,
                            int idx2, int idx3, double value );
CXCOREAPI void cxSetRealND( CxArr* arr, int* idx, double value );

/* clears element of ND dense array,
   in case of sparse arrays deletes the specified node */
CXCOREAPI void cxClearND( CxArr* arr, int* idx );

/* Converts CxArr (IplImage or CxMat,...) to CxMat.
   If the last parameter is non-zero, function can
   convert multi(>2)-dimensional array to CxMat as long as
   the last array's dimension is continous. The resultant
   matrix will be have appropriate (a huge) number of rows */
CXCOREAPI CxMat* cxGetMat( const CxArr* src, CxMat* header,
                           int* coi CX_DEFAULT(NULL),
                           int allowND CX_DEFAULT(0));

/* Converts CxArr (IplImage or CxMat) to IplImage */
CXCOREAPI IplImage* cxGetImage( const CxArr* arr, IplImage* img );


/* Changes a shape of multi-dimensional array.
   new_cn == 0 means that number of channels remains unchanged.
   new_dims == 0 means that number and sizes of dimensions remain the same
   (unless they need to be changed to set the new number of channels)
   if new_dims == 1, there is no need to specify new dimension sizes
   The resultant configuration should be achievable w/o data copying.
   If the resultant array is sparse, CxSparseMat header should be passed
   to the function else if the result is 1 or 2 dimensional,
   CxMat header should be passed to the function
   else CxMatND header should be passed */
CXCOREAPI CxArr* cxReshapeMatND( const CxArr* arr,
                                 int sizeof_header, CxArr* header,
                                 int new_cn, int new_dims, int* new_sizes );

#define cxReshapeND( arr, header, new_cn, new_dims, new_sizes )   \
      cxReshapeMatND( (arr), sizeof(*(header)), (header),         \
                      (new_cn), (new_dims), (new_sizes))

CXCOREAPI CxMat* cxReshape( const CxArr* arr, CxMat* header,
                            int new_cn, int new_rows CX_DEFAULT(0) );

/* Repeats source 2d array several times in both horizontal and
   vertical direction to fit destination array */
CXCOREAPI void cxRepeat( const CxArr* src, CxArr* dst );

/* Allocates array data */
CXCOREAPI  void  cxCreateData( CxArr* arr );

/* Releases array data */
CXCOREAPI  void  cxReleaseData( CxArr* arr );

/* Attaches user data to the array header. The step is reffered to
   the pre-last dimension. That is, all the planes of the array
   must be joint (w/o gaps) */
CXCOREAPI  void  cxSetData( CxArr* arr, void* data, int step );

/* Retrieves raw data of CxMat, IplImage or CxMatND.
   In the latter case the function raises an error if
   the array can not be represented as a matrix */
CXCOREAPI void cxGetRawData( const CxArr* arr, uchar** data,
                             int* step CX_DEFAULT(NULL),
                             CxSize* roi_size CX_DEFAULT(NULL));

/* Returns width and height of array in elements */
CXCOREAPI  CxSize cxGetSize( const CxArr* arr );

/* Copies source array to destination array */
CXCOREAPI  void  cxCopy( const CxArr* src, CxArr* dst,
                         const CxArr* mask CX_DEFAULT(NULL) );

/* Sets all or "masked" elements of input array
   to the same <scalar> value*/
CXCOREAPI  void  cxSet( CxArr* arr, CxScalar scalar,
                        const CxArr* mask CX_DEFAULT(NULL) );

/* Clears all the array elements (sets them to 0) */
CXCOREAPI  void  cxSetZero( CxArr* mat );
#define cxZero  cxSetZero


/* Splits a multi-channel array into the set of single-channel arrays or
   extracts particular [color] plane */
CXCOREAPI  void  cxCxtPixToPlane( const void *src, void *dst0, void *dst1,
                                  void *dst2, void *dst3 );

/* Merges a set of single-channel arrays into the single multi-channel array
   or inserts one particular [color] plane to the array */
CXCOREAPI  void  cxCxtPlaneToPix( const void *src0, const void *src1,
                                  const void *src2, const void *src3,
                                  void *dst );

/* Performs linear transformation on every source array element:
   dst(x,y,c) = scale*src(x,y,c)+shift.
   Arbitrary combination of input and output array depths are allowed
   (number of channels must be the same), thus the function can be used
   for type conversion */
CXCOREAPI  void  cxConvertScale( const CxArr *src, CxArr *dst,
                                 double scale CX_DEFAULT(1),
                                 double shift CX_DEFAULT(0) );
#define cxCxtScale cxConvertScale
#define cxScale  cxConvertScale
#define cxConvert( src, dst )  cxConvertScale( (src), (dst), 1, 0 )


/* Performs linear transformation on every source array element,
   stores absolute value of the result:
   dst(x,y,c) = abs(scale*src(x,y,c)+shift).
   destination array must have 8u type.
   In other cases one may use cxConvertScale + cxAbsDiffS */
CXCOREAPI  void  cxConvertScaleAbs( const void *src, void *dst,
                                    double scale CX_DEFAULT(1),
                                    double shift CX_DEFAULT(0) );
#define cxCxtScaleAbs  cxConvertScaleAbs


/* Finds minimum rectangle containing two given rectangles */
CXCOREAPI  CxRect  cxMaxRect( const CxRect* rect1, const CxRect* rect2 );

/* Finds coordinates of the box vertices */
CXCOREAPI  void cxBoxPoints( CxBox2D box, CxPoint2D32f pt[4] );

/* checks termination criteria validity and
   sets eps to default_eps (if it is not set),
   maxIter to default_max_iters (if it is not set)
*/
CXCOREAPI  CxTermCriteria cxCheckTermCriteria( CxTermCriteria criteria,
                                               double default_eps,
                                               int default_max_iters );

/****************************************************************************************\
*                   Arithmetic, logic and comparison operations                          *
\****************************************************************************************/

/* dst(mask) = srcA(mask) + srcB(mask) */
CXCOREAPI  void  cxAdd( const CxArr* srcA, const CxArr* srcB, CxArr* dst,
                        const CxArr* mask CX_DEFAULT(NULL));

/* dst(mask) = src(mask) + value */
CXCOREAPI  void  cxAddS( const CxArr* src, CxScalar value, CxArr* dst,
                         const CxArr* mask CX_DEFAULT(NULL));

/* dst(mask) = srcA(mask) - srcB(mask) */
CXCOREAPI  void  cxSub( const CxArr* srcA, const CxArr* srcB, CxArr* dst,
                        const CxArr* mask CX_DEFAULT(NULL));

/* dst(mask) = src(mask) - value = src(mask) + (-value) */
CX_INLINE  void  cxSubS( const CxArr* src, CxScalar value, CxArr* dst,
                         const CxArr* mask CX_DEFAULT(NULL));
CX_INLINE  void  cxSubS( const CxArr* src, CxScalar value, CxArr* dst,
                         const CxArr* mask )
{
    cxAddS( src, cxScalar( -value.val[0], -value.val[1], -value.val[2], -value.val[3]),
            dst, mask );
}

/* dst(mask) = value - src(mask) */
CXCOREAPI  void  cxSubRS( const CxArr* src, CxScalar value, CxArr* dst,
                          const CxArr* mask CX_DEFAULT(NULL));

/* dst(idx) = srcA(idx) * srcB(idx) * scale
   (scaled element-wise multiplication of 2 arrays) */
CXCOREAPI  void  cxMul( const CxArr* srcA, const CxArr* srcB,
                        CxArr* dst, double scale CX_DEFAULT(1) );

/* element-wise division/inversion with scaling: 
    dst(idx) = srcA(idx) * scale / srcB(idx)
    or dst(idx) = scale / srcB(idx) if srcA == 0 */
CXCOREAPI  void  cxDiv( const CxArr* srcA, const CxArr* srcB,
                        CxArr* dst, double scale CX_DEFAULT(1));

/* dst = srcA * scale + srcB */
CXCOREAPI  void  cxScaleAdd( const CxArr* srcA, CxScalar scale,
                             const CxArr* srcB, CxArr* dst );
#define cxAXPY( A, real_scalar, B, C ) cxScaleAdd(A, cxRealScalar(real_scalar), B, C)

/* dst = srcA * alpha + srcB * beta + gamma */
CXCOREAPI  void  cxAddWeighted( const CxArr* srcA, double alpha,
                                const CxArr* srcB, double beta,
                                double gamma, CxArr* dst );

/* result = Re sum_i(srcA(i) * srcB*(i))  (srcB is conjugated) */
CXCOREAPI  double  cxDotProduct( const CxArr* srcA, const CxArr* srcB );

/* dst(idx) = src1(idx) & src2(idx) */
CXCOREAPI void cxAnd( const CxArr* src1, const CxArr* src2,
                      CxArr* dst, const CxArr* mask CX_DEFAULT(NULL));

/* dst(idx) = src(idx) & value */
CXCOREAPI void cxAndS( const CxArr* src, CxScalar value,
                       CxArr* dst, const CxArr* mask CX_DEFAULT(NULL));

/* dst(idx) = src1(idx) | src2(idx) */
CXCOREAPI void cxOr( const CxArr* src1, const CxArr* src2,
                     CxArr* dst, const CxArr* mask CX_DEFAULT(NULL));

/* dst(idx) = src(idx) | value */
CXCOREAPI void cxOrS( const CxArr* src, CxScalar value,
                      CxArr* dst, const CxArr* mask CX_DEFAULT(NULL));

/* dst(idx) = src1(idx) ^ src2(idx) */
CXCOREAPI void cxXor( const CxArr* src1, const CxArr* src2,
                      CxArr* dst, const CxArr* mask CX_DEFAULT(NULL));

/* dst(idx) = src(idx) ^ value */
CXCOREAPI void cxXorS( const CxArr* src, CxScalar value,
                       CxArr* dst, const CxArr* mask CX_DEFAULT(NULL));

/* dst(idx) = ~src(idx) */
CXCOREAPI void cxNot( const CxArr* src, CxArr* dst );

/* dst(idx) = lower(idx) <= src(idx) < upper(idx) */
CXCOREAPI void cxInRange( const CxArr* src, const CxArr* lower,
                          const CxArr* upper, CxArr* dst );

/* dst(idx) = lower <= src(idx) < upper */
CXCOREAPI void cxInRangeS( const CxArr* src, CxScalar lower,
                           CxScalar upper, CxArr* dst );

#define CX_CMP_EQ   0
#define CX_CMP_GT   1
#define CX_CMP_GE   2
#define CX_CMP_LT   3
#define CX_CMP_LE   4
#define CX_CMP_NE   5

/* The comparison operation support single-channel arrays only.
   Destination image should be 8uC1 or 8sC1 */

/* dst(idx) = src1(idx) _cmp_op_ src2(idx) */
CXCOREAPI void cxCmp( const CxArr* src1, const CxArr* src2, CxArr* dst, int cmpOp );

/* dst(idx) = src1(idx) _cmp_op_ scalar */
CXCOREAPI void cxCmpS( const CxArr* src1, double scalar, CxArr* dst, int cmpOp );

/* dst(idx) = min(src1(idx),src2(idx)) */
CXCOREAPI void cxMin( const CxArr* src1, const CxArr* src2, CxArr* dst );

/* dst(idx) = max(src1(idx),src2(idx)) */
CXCOREAPI void cxMax( const CxArr* src1, const CxArr* src2, CxArr* dst );

/* dst(idx) = min(src(idx),scalar) */
CXCOREAPI void cxMinS( const CxArr* src, double scalar, CxArr* dst );

/* dst(idx) = max(src(idx),scalar) */
CXCOREAPI void cxMaxS( const CxArr* src, double scalar, CxArr* dst );


/****************************************************************************************\
*                                Math operations                                         *
\****************************************************************************************/

/* Does cartesian->polar coordinates conversion.
   Either of output components (magnitude or angle) is optional */
CXCOREAPI  void  cxCartToPolar( const CxArr* x, const CxArr* y,
                                CxArr* magnitude, CxArr* angle CX_DEFAULT(NULL),
                                int angle_in_degrees CX_DEFAULT(0));

/* Does polar->cartesian coordinates conversion.
   Either of output components (magnitude or angle) is optional.
   If magnitude is missing it is assumed to be all 1's */
CXCOREAPI  void  cxPolarToCart( const CxArr* magnitude, const CxArr* angle,
                                CxArr* x, CxArr* y,
                                int angle_in_degrees CX_DEFAULT(0));

/* Does powering: dst(idx) = src(idx)^power */
CXCOREAPI  void  cxPow( const CxArr* src, CxArr* dst, double power );

/* Does exponention: dst(idx) = exp(src(idx)).
   Overflow is not handled yet. Underflow is handled.
   Maximal relative error is ~7e-6 for single-precision input */
CXCOREAPI  void  cxExp( const CxArr* src, CxArr* dst );

/* Calculates natural logarithms: dst(idx) = log(abs(src(idx))).
   Logarithm of 0 gives large negative number(~-700)
   Maximal relative error is ~3e-7 for single-precision output
*/
CXCOREAPI  void  cxLog( const CxArr* src, CxArr* dst );

/* Fast arctangent calculation */
CXCOREAPI  float cxFastArctan( float y, float x );

/* Checks array values for NaNs, Infs or simply for too large numbers
   (if CX_CHECK_RANGE is set). If CX_CHECK_QUIET is set,
   no runtime errors is raised (function returns zero value in case of "bad" values).
   Otherwise cxError is called */ 
#define  CX_CHECK_RANGE    1
#define  CX_CHECK_QUIET    2
CXCOREAPI  int  cxCheckArr( const CxArr* arr, int flags CX_DEFAULT(0),
                            double minVal CX_DEFAULT(0), double maxVal CX_DEFAULT(0));
#define cxCheckArray cxCheckArr

/* RNG state */
typedef struct CxRandState
{
    uint64    state;    /* RNG state (the current seed and carry)*/
    int       disttype; /* distribution type */
    CxScalar  param[2]; /* parameters of RNG */
}
CxRandState;

/* Initalized RNG state */
#define CX_RAND_UNI      0
#define CX_RAND_NORMAL   1
CXCOREAPI  void  cxRandInit( CxRandState* state, double param1,
                             double param2, int seed,
                             int disttype CX_DEFAULT(CX_RAND_UNI));

/* Changes RNG range while preserving RNG state */
CXCOREAPI  void  cxRandSetRange( CxRandState* state, double param1, double param2,
                                 int index CX_DEFAULT(-1));

/* Fills array with random numbers */
CXCOREAPI  void  cxRand( CxRandState* state, CxArr* arr );

/* Returns 32-bit random number (ranges are not used)
   and updates RNG state */
CX_INLINE  unsigned  cxRandNext( CxRandState* state );
CX_INLINE  unsigned  cxRandNext( CxRandState* state )
{
    uint64 temp = state->state;
    temp = (uint64)(unsigned)temp*1554115554 + (temp >> 32);
    state->state = temp;

    return (unsigned)temp;
}


/****************************************************************************************\
*                                Matrix operations                                       *
\****************************************************************************************/

/* Calculates cross product of two 3d vectors */
CXCOREAPI  void  cxCrossProduct( const CxArr* srcA, const CxArr* srcB, CxArr* dst );

/* Matrix transform: dst = A*B + C, C is optional */
CXCOREAPI  void  cxMatMulAdd( const CxArr* srcA, const CxArr* srcB,
                              const CxArr* srcC, CxArr* dst );
#define cxMatMul( srcA, srcB, dst )  cxMatMulAdd( (srcA), (srcB), NULL, (dst))

#define CX_GEMM_A_T 1
#define CX_GEMM_B_T 2
#define CX_GEMM_C_T 4
/* Extended matrix transform:
   dst = alpha*op(A)*op(B) + beta*op(C), where op(X) is X or X^T */
CXCOREAPI  void  cxGEMM( const CxArr* srcA, const CxArr* srcB, double alpha,
                         const CxArr* srcC, double beta, CxArr* dst,
                         int tABC CX_DEFAULT(0));
#define cxMatMulAddEx cxGEMM

/* Transforms each element of source array and stores
   resultant vectors in destination array */
CXCOREAPI  void  cxMatMulAddS( const CxArr* src, CxArr* dst,
                               const CxMat* transform,
                               const CxMat* shiftvec CX_DEFAULT(NULL));
#define cxTransform cxMatMulAddS

/* Calculates (A-delta)*(A-delta)^T (order=0) or (A-delta)^T*(A-delta) (order=1) */
CXCOREAPI void cxMulTransposed( const CxArr* src, CxArr* dst, int order,
                                const CxArr* delta CX_DEFAULT(NULL) );

/* Tranposes matrix. Square matrices can be transposed in-place */
CXCOREAPI  void  cxTranspose( const CxArr* src, CxArr* dst );
#define cxT cxTranspose


/* Mirror array data around horizontal (flip=0),
   vertical (flip=1) or both(flip=-1) axises:
   cxFlip(src) flips images vertically and sequences horizontally (inplace) */
CXCOREAPI  void  cxFlip( const CxArr* src, CxArr* dst CX_DEFAULT(NULL),
                         int flip_mode CX_DEFAULT(0));
#define cxMirror cxFlip


#define CX_SVD_MODIFY_A   1
#define CX_SVD_U_T        2
#define CX_SVD_V_T        4

/* Performs Singular Value Decomposition of a matrix */
CXCOREAPI  void   cxSVD( CxArr* A, CxArr* W CX_DEFAULT(NULL),
                         CxArr* U CX_DEFAULT(NULL),
                         CxArr* V CX_DEFAULT(NULL),
                         int flags CX_DEFAULT(0));

/* Performs Singular Value Back Substitution:
   flags must be the same as in cxSVD */
CXCOREAPI  void   cxSVBkSb( const CxArr* warr, const CxArr* uarr,
                            const CxArr* varr, const CxArr* barr,
                            CxArr* xarr, int flags );

#define CX_LU  0
#define CX_SVD 1
/* Inverts matrix */
CXCOREAPI  double  cxInvert( const CxArr* src, CxArr* dst,
                             int method CX_DEFAULT(CX_LU));
#define cxInv cxInvert

/* Solves linear system Ax = b
   (returns 0 if A is singular and CX_LU method is chosen) */
CXCOREAPI  int  cxSolve( const CxArr* A, const CxArr* b, CxArr* x,
                         int method CX_DEFAULT(CX_LU));

/* Calculates determinant of input matrix */
CXCOREAPI  double cxDet( const CxArr* mat );

/* Calculates trace of the matrix (sum of elements on the main diagonal) */
CXCOREAPI  CxScalar cxTrace( const CxArr* mat );

/* Finds eigen values and vectors of a symmetric matrix */
CXCOREAPI  void  cxEigenVV( CxArr* src, CxArr* evects,
                            CxArr* evals, double eps CX_DEFAULT(0));

/* Makes an identity matrix (mat_ij = i == j) */
CXCOREAPI  void  cxSetIdentity( CxArr* mat, CxScalar value CX_DEFAULT(cxScalar(1)) );

/* Calculates covariation matrix for a set of vectors */
/* transpose([v1-avg, v2-avg,...]) * [v1-avg,v2-avg,...] */
#define CX_COVAR_SCRAMBLED 0

/* [v1-avg, v2-avg,...] * transpose([v1-avg,v2-avg,...]) */
#define CX_COVAR_NORMAL    1

/* do not calc average (i.e. mean vector) - use the input vector instead
   (useful for calculating covariance matrix by parts) */
#define CX_COVAR_USE_AVG   2

/* scale the covariance matrix coefficients by number of the vectors */
#define CX_COVAR_SCALE     4

CXCOREAPI  void  cxCalcCovarMatrix( const CxArr** vects, CxArr* covarMatrix,
                                    CxArr* avg, int count, int flags );

/* Calculates Mahalanobis(weighted) distance */
CXCOREAPI  double  cxMahalanobis( const CxArr* srcA, const CxArr* srcB, CxArr* mat );
#define cxMahalonobis  cxMahalanobis

/****************************************************************************************\
*                                    Array Statistics                                    *
\****************************************************************************************/

/* Finds sum of array elements */
CXCOREAPI  CxScalar  cxSum( const CxArr* arr );

/* Calculates number of non-zero pixels */
CXCOREAPI  int  cxCountNonZero( const CxArr* arr );

/* Calculates mean value of array elements */
CXCOREAPI  CxScalar  cxAvg( const CxArr* arr, const CxArr* mask CX_DEFAULT(NULL) );

/* Calculates mean and standard deviation of pixel values */
CXCOREAPI  void  cxAvgSdv( const CxArr* arr, CxScalar* mean, CxScalar* std_dev,
                           const CxArr* mask CX_DEFAULT(NULL) );

/* Finds global minimum, maximum among the input array elements and positions
   of the extremums */
CXCOREAPI  void  cxMinMaxLoc( const CxArr* arr, double* min_val, double* max_val,
                              CxPoint* min_loc CX_DEFAULT(NULL),
                              CxPoint* max_loc CX_DEFAULT(NULL),
                              const CxArr* mask CX_DEFAULT(NULL) );

/* types of array norm */
#define CX_C            1
#define CX_L1           2
#define CX_L2           4
#define CX_NORM_MASK    7
#define CX_RELATIVE     8
#define CX_DIFF         16

#define CX_DIFF_C       (CX_DIFF | CX_C)
#define CX_DIFF_L1      (CX_DIFF | CX_L1)
#define CX_DIFF_L2      (CX_DIFF | CX_L2)
#define CX_RELATIVE_C   (CX_RELATIVE | CX_C)
#define CX_RELATIVE_L1  (CX_RELATIVE | CX_L1)
#define CX_RELATIVE_L2  (CX_RELATIVE | CX_L2)

/* Finds norm, difference norm or relative difference norm for an array (two arrays) */
CXCOREAPI  double  cxNorm( const CxArr* arrA, const CxArr* arrB CX_DEFAULT(NULL),
                           int normType CX_DEFAULT(CX_L2),
                           const CxArr* mask CX_DEFAULT(NULL) );

/****************************************************************************************\
*                                 Spectral Transforms                                    *
\****************************************************************************************/

#define CX_DXT_FORWARD  0
#define CX_DXT_INVERSE  1
#define CX_DXT_SCALE    2 /* divide result by size of array */
#define CX_DXT_INV_SCALE (CX_DXT_INVERSE + CX_DXT_SCALE)
#define CX_DXT_INVERSE_SCALE CX_DXT_INV_SCALE

/* Discrete Fourier Transform:
    complex->complex,
    real->ccs (forward),
    ccs->real (inverse) */
CXCOREAPI  void  cxDFT( const CxArr* src, CxArr* dst, int flags );
#define cxFFT cxDFT

/* Multiply results of real DFT:
   ReA(0) * ReB(0), ReA(N/2)*ReB(N/2), A(1)*B*(1), ..., A(N/2-1)*B*(N/2-1) */
CXCOREAPI  void  cxMulCcs( const CxArr* srcA, const CxArr* srcB, CxArr* dst );

/* Discrete Cosine Transform */
CXCOREAPI  void  cxDCT( const CxArr* src, CxArr* dst, int flags );

/****************************************************************************************\
*                              Dynamic data structures                                   *
\****************************************************************************************/

/* Calculates length of sequence slice (with support of negative indices). */
CXCOREAPI  int cxSliceLength( CxSlice slice, CxSeq* seq );


/* Creates new memory storage.
   block_size == 0 means that default,
   somewhat optimal size, is used (currently, it is 64K) */
CXCOREAPI  CxMemStorage*  cxCreateMemStorage( int block_size CX_DEFAULT(0));


/* Creates a memory storage that will borrow memory blocks from parent storage */
CXCOREAPI  CxMemStorage*  cxCreateChildMemStorage( CxMemStorage* parent );


/* Releases memory storage. All the children of a parent must be released before
   the parent. A child storage returns all the blocks to parent when it is released */
CXCOREAPI  void  cxReleaseMemStorage( CxMemStorage** storage );


/* Clears memory storage. This is the only way(!!!) (besides cxRestoreMemStoragePos)
   to reuse memory allocated for the storage - cxClearSeq,cxClearSet ...
   do not free any memory.
   A child storage returns all the blocks to the parent when it is cleared */
CXCOREAPI  void  cxClearMemStorage( CxMemStorage* storage );

/* Remember a storage "free memory" position */
CXCOREAPI  void  cxSaveMemStoragePos( const CxMemStorage* storage, CxMemStoragePos* pos );

/* Restore a storage "free memory" position */
CXCOREAPI  void  cxRestoreMemStoragePos( CxMemStorage* storage, CxMemStoragePos* pos );

/* Allocates continuous buffer of the specified size in the storage */
CXCOREAPI  void* cxMemStorageAlloc( CxMemStorage* storage, int size );

/* Creates new empty sequence that will reside in the specified storage */
CXCOREAPI  CxSeq*  cxCreateSeq( int seq_flags, int header_size,
                             int elem_size, CxMemStorage* storage );

/* Changes default size (granularity) of sequence blocks.
   The default size is ~1Kbyte */
CXCOREAPI  void  cxSetSeqBlockSize( CxSeq* seq, int delta_elements );


/* Adds new element to the end of sequence. Returns pointer to the element */
CXCOREAPI  char*  cxSeqPush( CxSeq* seq, void* element CX_DEFAULT(NULL));


/* Adds new element to the beginning of sequence. Returns pointer to it */
CXCOREAPI  char*  cxSeqPushFront( CxSeq* seq, void* element CX_DEFAULT(NULL));


/* Removes the last element from sequence and optionally saves it */
CXCOREAPI  void  cxSeqPop( CxSeq* seq, void* element CX_DEFAULT(NULL));


/* Removes the first element from sequence and optioanally saves it */
CXCOREAPI  void  cxSeqPopFront( CxSeq* seq, void* element CX_DEFAULT(NULL));


#define CX_FRONT 1
#define CX_BACK 0
/* Adds several new elements to the end of sequence */
CXCOREAPI  void  cxSeqPushMulti( CxSeq* seq, void* elements,
                                 int count, int in_front CX_DEFAULT(0) );

/* Removes several elements from the end of sequence and optionally saves them */
CXCOREAPI  void  cxSeqPopMulti( CxSeq* seq, void* elements,
                                int count, int in_front CX_DEFAULT(0) );

/* Inserts a new element in the middle of sequence.
   cxSeqInsert(seq,0,elem) == cxSeqPushFront(seq,elem) */
CXCOREAPI  char*  cxSeqInsert( CxSeq* seq, int before_index,
                               void* element CX_DEFAULT(NULL));

/* Removes specified sequence element */
CXCOREAPI  void  cxSeqRemove( CxSeq* seq, int index );


/* Removes all the elements from the sequence. The freed memory
   can be reused later only by the same sequence unless cxClearMemStorage
   or cxRestoreMemStoragePos is called */
CXCOREAPI  void  cxClearSeq( CxSeq* seq );


/* Retrives pointer to specified sequence element.
   Negative indices are supported and mean counting from the end
   (e.g -1 means the last sequence element) */
CXCOREAPI  char*  cxGetSeqElem( CxSeq* seq, int index, CxSeqBlock** block CX_DEFAULT(NULL) );


/* Calculates index of the specified sequence element.
   Returns -1 if element does not belong to the sequence */
CXCOREAPI int  cxSeqElemIdx( const CxSeq* seq, const void* element,
                             CxSeqBlock** block CX_DEFAULT(NULL) );

/* Initializes sequence writer. The new elements will be added to the end of sequence */
CXCOREAPI  void  cxStartAppendToSeq( CxSeq* seq, CxSeqWriter* writer );


/* Combination of cxCreateSeq and cxStartAppendToSeq */
CXCOREAPI  void  cxStartWriteSeq( int seq_flags, int header_size,
                                  int elem_size, CxMemStorage* storage,
                                  CxSeqWriter* writer );

/* Closes sequence writer, updates sequence header and returns pointer
   to the resultant sequence
   (which may be useful if the sequence was created using cxStartWriteSeq))
*/
CXCOREAPI  CxSeq*  cxEndWriteSeq( CxSeqWriter* writer );


/* Updates sequence header. May be useful to get access to some of previously
   written elements via cxGetSeqElem or sequence reader */
CXCOREAPI  void   cxFlushSeqWriter( CxSeqWriter* writer );


/* Initializes sequence reader.
   The sequence can be read in forward or backward direction */
CXCOREAPI void cxStartReadSeq( const CxSeq* seq, CxSeqReader* reader,
                               int reverse CX_DEFAULT(0) );


/* Returns current sequence reader position (currently observed sequence element) */
CXCOREAPI  int  cxGetSeqReaderPos( CxSeqReader* reader );


/* Changes sequence reader position. It may seek to an absolute or
   to relative to the current position */
CXCOREAPI  void   cxSetSeqReaderPos( CxSeqReader* reader, int index,
                                     int is_relative CX_DEFAULT(0));

/* Copies sequence content to an array */
CXCOREAPI  void*  cxCxtSeqToArray( CxSeq* seq, CxArr* arr,
                                   CxSlice slice CX_DEFAULT(CX_WHOLE_SEQ) );

/* Creates sequence header for array.
   After that all the operations on sequences that do not alter the content
   can be applied to the resultant sequence */
CXCOREAPI  CxSeq* cxMakeSeqHeaderForArray( int seq_type, int header_size,
                                           int elem_size, void* elements, int total,
                                           CxSeq* seq, CxSeqBlock* block );

/* Extracts sequence slice (with or without copying sequence elements */
CXCOREAPI CxSeq* cxSeqSlice( CxSeq* seq, CxSlice slice,
                             CxMemStorage* storage CX_DEFAULT(NULL),
                             int copy_data CX_DEFAULT(0));

CX_INLINE CxSeq* cxCloneSeq( CxSeq* seq, CxMemStorage* storage CX_DEFAULT(NULL));
CX_INLINE CxSeq* cxCloneSeq( CxSeq* seq, CxMemStorage* storage )
{
    return cxSeqSlice( seq, CX_WHOLE_SEQ, storage, 1 );
}

/* Removes sequence slice */
CXCOREAPI  void  cxSeqRemoveSlice( CxSeq* seq, CxSlice slice );

/* Inserts a sequence or array into another sequence */
CXCOREAPI  void  cxSeqInsertSlice( CxSeq* seq, int index, const CxArr* from_arr );

/* a < b ? -1 : a > b ? 1 : 0 */
CX_EXTERN_C_FUNCPTR( int (CX_CDECL* CxCmpFunc)
                     (const void* a, const void* b, void* userdata ));

/* Sorts sequence in-place given element comparison function */
CXCOREAPI  void cxSeqSort( CxSeq* seq, CxCmpFunc func, void* userdata );

/* Reverses order of sequence elements in-place */
CXCOREAPI  void cxSeqInvert( CxSeq* seq );

/* Splits sequence into set of equivalency classes
   using specified equivalency criteria */
CXCOREAPI  int  cxPartitionSeq( CxSeq* seq, CxMemStorage* storage, CxSeq** comps,
                                CxCmpFunc is_equal, void* userdata, int is_set );

/************ Internal sequence functions ************/
CXCOREAPI  void  cxChangeSeqBlock( CxSeqReader* reader, int direction );
CXCOREAPI  void  cxCreateSeqBlock( CxSeqWriter* writer );


/* Creates a new set */
CXCOREAPI  CxSet*  cxCreateSet( int set_flags, int header_size,
                                int elem_size, CxMemStorage* storage );

/* Adds new element to the set and returns pointer to it */
CXCOREAPI  int  cxSetAdd( CxSet* set_header, CxSetElem* element CX_DEFAULT(NULL),
                          CxSetElem** inserted_element CX_DEFAULT(NULL) );

/* Fast variant of cxSetAdd */
CX_INLINE  CxSetElem* cxSetNew( CxSet* set_header );
CX_INLINE  CxSetElem* cxSetNew( CxSet* set_header )
{
    CxSetElem* elem = set_header->free_elems;
    if( elem )
    {
        set_header->free_elems = elem->next_free;
        elem->flags = elem->flags & CX_SET_ELEM_IDX_MASK;
    }
    else
        cxSetAdd( set_header, NULL, (CxSetElem**)&elem );
    return elem;
}

/* Removes set element given its pointer */
CX_INLINE  void cxSetRemoveByPtr( CxSet* set_header, void* _elem );
CX_INLINE  void cxSetRemoveByPtr( CxSet* set_header, void* _elem )
{
    CxSetElem* elem = (CxSetElem*)_elem;
    assert( elem->flags >= 0 /*&& (elem->flags & CX_SET_ELEM_IDX_MASK) < set_header->total*/ );
    elem->next_free = set_header->free_elems;
    elem->flags = (elem->flags & CX_SET_ELEM_IDX_MASK) | CX_SET_ELEM_FREE_FLAG;
    set_header->free_elems = elem;
}


/* Removes element from the set by its index  */
CXCOREAPI  void   cxSetRemove( CxSet* set_header, int index );


/* Returns a set element by index. If the element doesn't belong to the set,
   NULL is returned */
CXCOREAPI  CxSetElem*  cxGetSetElem( CxSet* set_header, int index );


/* Removes all the elements from the set */
CXCOREAPI  void  cxClearSet( CxSet* set_header );


/* Creates new graph */
CXCOREAPI  CxGraph*   cxCreateGraph( int graph_flags, int header_size,
                                  int vtx_size, int edge_size,
                                  CxMemStorage* storage );

/* Adds new vertex to the graph */
CXCOREAPI  int  cxGraphAddVtx( CxGraph* graph, CxGraphVtx* vertex CX_DEFAULT(NULL),
                               CxGraphVtx** inserted_vertex CX_DEFAULT(NULL) );


/* Removes vertex from the graph together with all incident edges */
CXCOREAPI  int  cxGraphRemoveVtx( CxGraph* graph, int index );
CXCOREAPI  int  cxGraphRemoveVtxByPtr( CxGraph* graph, CxGraphVtx* vtx );


/* Link two vertices specifed by indices or pointers if they
   are not connected or return pointer to already existing edge
   connecting the vertices.
   Functions return 1 if a new edge was created, 0 otherwise */
CXCOREAPI  int  cxGraphAddEdge( CxGraph* graph,
                                int start_idx, int end_idx,
                                CxGraphEdge* edge CX_DEFAULT(NULL),
                                CxGraphEdge** inserted_edge CX_DEFAULT(NULL) );

CXCOREAPI  int  cxGraphAddEdgeByPtr( CxGraph* graph,
                               CxGraphVtx* start_vtx, CxGraphVtx* end_vtx,
                               CxGraphEdge* edge CX_DEFAULT(NULL),
                               CxGraphEdge** inserted_edge CX_DEFAULT(NULL) );

/* Remove edge connecting two vertices */
CXCOREAPI  void  cxGraphRemoveEdge( CxGraph* graph, int start_idx, int end_idx );
CXCOREAPI  void  cxGraphRemoveEdgeByPtr( CxGraph* graph, CxGraphVtx* start_vtx,
                                         CxGraphVtx* end_vtx );

/* Find edge connecting two vertices */
CXCOREAPI  CxGraphEdge*  cxFindGraphEdge( CxGraph* graph, int start_idx, int end_idx );
CXCOREAPI  CxGraphEdge*  cxFindGraphEdgeByPtr( CxGraph* graph, CxGraphVtx* start_vtx,
                                               CxGraphVtx* end_vtx );
#define cxGraphFindEdge cxFindGraphEdge
#define cxGraphFindEdgeByPtr cxFindGraphEdgeByPtr

/* Remove all vertices and edges from the graph */
CXCOREAPI  void  cxClearGraph( CxGraph* graph );


/* Count number of edges incident to the vertex */
CXCOREAPI  int  cxGraphVtxDegree( CxGraph* graph, int vtx_idx );
CXCOREAPI  int  cxGraphVtxDegreeByPtr( CxGraph* graph, CxGraphVtx* vtx );


/* Retrieves graph vertex by given index */
#define cxGetGraphVtx( graph, idx ) (CxGraphVtx*)cxGetSetElem((CxSet*)(graph), (idx))

/* Retrieves index of a graph vertex given its pointer */
#define cxGraphVtxIdx( graph, vtx ) ((vtx)->flags & CX_SET_ELEM_IDX_MASK)

/* Retrieves index of a graph edge given its pointer */
#define cxGraphEdgeIdx( graph, edge ) ((edge)->flags & CX_SET_ELEM_IDX_MASK)


#define  CX_GRAPH_VERTEX        1
#define  CX_GRAPH_TREE_EDGE     2
#define  CX_GRAPH_BACK_EDGE     4
#define  CX_GRAPH_FORWARD_EDGE  8
#define  CX_GRAPH_CROSS_EDGE    16
#define  CX_GRAPH_ANY_EDGE      30
#define  CX_GRAPH_NEW_TREE      32
#define  CX_GRAPH_BACKTRACKING  64
#define  CX_GRAPH_OVER          -1

#define  CX_GRAPH_ALL_ITEMS    -1

/* flags for graph vertices and edges */
#define  CX_GRAPH_ITEM_VISITED_FLAG  (1 << 30)
#define  CX_IS_GRAPH_VERTEX_VISITED(vtx) \
    (((CxGraphVtx*)(vtx))->flags & CX_GRAPH_ITEM_VISITED_FLAG)
#define  CX_IS_GRAPH_EDGE_VISITED(edge) \
    (((CxGraphEdge*)(edge))->flags & CX_GRAPH_ITEM_VISITED_FLAG)
#define  CX_GRAPH_SEARCH_TREE_NODE_FLAG   (1 << 29)
#define  CX_GRAPH_FORWARD_EDGE_FLAG       (1 << 28)

typedef struct CxGraphScanner
{
    CxGraphVtx* vtx;       /* current graph vertex (or current edge origin) */
    CxGraphVtx* dst;       /* current graph edge destination vertex */
    CxGraphEdge* edge;     /* current edge */

    CxGraph* graph;        /* the graph */
    CxSeq*   stack;        /* the graph vertex stack */
    int      index;        /* the lower bound of certainly visited vertices */
    int      mask;         /* event mask */
}
CxGraphScanner;

/* Initializes graph traversal process.
   <mask> indicates what events one wants to handle. */
CXCOREAPI void  cxStartScanGraph( CxGraph* graph, CxGraphScanner* scanner,
                                  CxGraphVtx* vtx CX_DEFAULT(NULL),
                                  int mask CX_DEFAULT(CX_GRAPH_ALL_ITEMS));

/* Initializes graph traversal process.
   <mask> indicates what events one wants to handle. */
CXCOREAPI void  cxEndScanGraph( CxGraphScanner* scanner );

/* Get next graph element */
CXCOREAPI int  cxNextGraphItem( CxGraphScanner* scanner );

/* Creates a copy of graph */
CXCOREAPI CxGraph* cxCloneGraph( const CxGraph* graph, CxMemStorage* storage );

/****************************************************************************************\
*                                     Drawing                                            *
\****************************************************************************************/

/****************************************************************************************\
*       Drawing functions work with arbitrary 8-bit images or single-channel images      *
*       with larger depth: 16s, 32s, 32f, 64f                                            *
*       All the functions include parameter color that means rgb value (that may be      *
*       constructed with CX_RGB macro) for color images and brightness                   *
*       for grayscale images.                                                            *
*       If a drawn figure is partially or completely outside the image, it is clipped.   *
\****************************************************************************************/

#define CX_RGB( r, g, b )  (int)((uchar)(b) + ((uchar)(g) << 8) + ((uchar)(r) << 16))
#define CX_FILLED -1

/* Draws 4-connected or 8-connected line segment connecting two points */
CXCOREAPI  void  cxLine( CxArr* arr, CxPoint pt1, CxPoint pt2,
                         double color, int thickness CX_DEFAULT(1),
                         int connectivity CX_DEFAULT(8) );

/* Draws 8-connected line segment connecting two points with antialiazing.
   Ending coordinates may be specified with sub-pixel accuracy
   (scale is number of fractional bits in the coordinates) */
CXCOREAPI  void  cxLineAA( CxArr* arr, CxPoint pt1, CxPoint pt2,
                           double color, int scale CX_DEFAULT(0));

/* Draws a rectangle given two opposite corners of the rectangle (pt1 & pt2),
   if thickness<0 (e.g. thickness == CX_FILLED), the filled box is drawn */
CXCOREAPI  void  cxRectangle( CxArr* arr, CxPoint pt1, CxPoint pt2,
                              double color, int thickness CX_DEFAULT(1));

/* Draws a circle with specified center and radius.
   Thickness works in the same way as with cxRectangle */
CXCOREAPI  void  cxCircle( CxArr* arr, CxPoint center, int radius,
                           double color, int thickness CX_DEFAULT(1));

/* Draws antialiazed circle with specified center and radius.
   Both the center and radius can be specified with sub-pixel accuracy */
CXCOREAPI  void  cxCircleAA( CxArr* arr, CxPoint center, int radius,
                             double color, int scale CX_DEFAULT(0) );

/* Draws ellipse outline, filled ellipse, elliptic arc or filled elliptic sector,
   depending on <thickness>, <startAngle> and <endAngle> parameters. The resultant figure
   is rotated by <angle>. All the angles are in degrees */
CXCOREAPI  void  cxEllipse( CxArr* arr, CxPoint center, CxSize axes,
                            double angle, double startAngle, double endAngle,
                            double color, int thickness CX_DEFAULT(1));

CX_INLINE  void  cxEllipseBox( CxArr* arr, CxBox2D box,
                               double color, int thickness CX_DEFAULT(1));
CX_INLINE  void  cxEllipseBox( CxArr* arr, CxBox2D box,
                               double color, int thickness )
{
    CxSize axes;
    axes.width = cxRound(box.size.height*0.5);
    axes.height = cxRound(box.size.width*0.5);
    
    cxEllipse( arr, cxPointFrom32f( box.center ), axes,
               box.angle*180/CX_PI, 0, 360, color, thickness );
}


/* Draws the whole ellipse or elliptic arc with antialiazing */
CXCOREAPI  void  cxEllipseAA( CxArr* arr, CxPoint center, CxSize axes,
                              double angle, double startAngle,
                              double endAngle, double color,
                              int scale CX_DEFAULT(0) );

/* Fills convex or monotonous (every horizontal line intersects the polygon twice at the most,
   except, may be, horizontal sides) polygon. Connectivity or monotony is not checked */
CXCOREAPI  void  cxFillConvexPoly( CxArr* arr, CxPoint* pts, int npts, double color );


/* Fills an area bounded by one or more arbitrary polygons (with possible intersections or
   self-intersections */
CXCOREAPI  void  cxFillPoly( CxArr* arr, CxPoint** pts,
                             int* npts, int contours, double color );

/* Draws one or more polygonal curves */
CXCOREAPI  void  cxPolyLine( CxArr* arr, CxPoint** pts, int* npts, int contours,
                             int closed, double color,
                             int thickness CX_DEFAULT(1),
                             int connectivity CX_DEFAULT(8));

/* Draws one or more antialiazed polygonal curves */
CXCOREAPI  void  cxPolyLineAA( CxArr* arr, CxPoint** pts, int* npts, int contours,
                               int closed, double color, int scale CX_DEFAULT(0) );

/* Font metrics and structure */
#define CX_FONT_VECTOR0  0

typedef struct CxFont
{
    const int*  data; /* font data and metrics */
    CxSize      size; /* horizontal and vertical scale factors,
                         (8:8) fix-point numbers */
    int         italic_scale; /* slope coefficient: 0 - normal, >0 - italic */
    int         thickness; /* letters thickness */
    int         dx; /* horizontal interval between letters */
}
CxFont;

/* Initializes font structure used further in cxPutText */
CXCOREAPI  void  cxInitFont( CxFont* font, int font_face,
                             double hscale, double vscale,
                             double italic_scale CX_DEFAULT(0),
                             int thickness CX_DEFAULT(1) );

/* Renders text stroke with specified font and color at specified location.
   CxFont should be initialized with cxInitFont */
CXCOREAPI  void  cxPutText( CxArr* arr, const char* text, CxPoint org,
                            CxFont* font, double color );

/* Calculates bounding box of text stroke (useful for alignment) */
CXCOREAPI  void  cxGetTextSize( const char* text_string, CxFont* font,
                                CxSize* text_size, int* ymin );

/* Unpacks color value, if arrtype is CX_8UC?, <color> is treated as
   packed color value, otherwise the first channels (depending on arrtype)
   of destination scalar are set to the same value = <color> */
CXCOREAPI  CxScalar  cxColorToScalar( double color, int arrtype );

/* Does look-up transformation. Elements of the source array
   (that should be 8uC1 or 8sC1) are used as indexes in lutarr 256-element table */
CXCOREAPI  void cxLUT( const CxArr* srcarr, CxArr* dstarr, const CxArr* lutarr );

/* Draws contour outlines or filled interiors on the image */
CXCOREAPI void  cxDrawContours( CxArr *img, CxSeq* contour,
                                double external_color, double hole_color,
                                int max_level, int thickness CX_DEFAULT(1),
                                int connectivity CX_DEFAULT(8));

/******************* Iteration through the sequence tree *****************/
typedef struct CxTreeNodeIterator
{
    const void* node;
    int level;
    int maxLevel;
}
CxTreeNodeIterator;

CXCOREAPI void cxInitTreeNodeIterator( CxTreeNodeIterator* treeIterator,
                                   const void* first, int maxLevel );
CXCOREAPI void* cxNextTreeNode( CxTreeNodeIterator* treeIterator );
CXCOREAPI void* cxPrevTreeNode( CxTreeNodeIterator* treeIterator );

/* Inserts sequence into tree with specified "parent" sequence.
   If parent is equal to frame (e.g. the most external contour),
   then added contour will have null pointer to parent. */
CXCOREAPI void cxInsertNodeIntoTree( void* node, void* parent, void* frame );

/* Removes contour from tree (together with the contour children). */
CXCOREAPI void cxRemoveNodeFromTree( void* node, void* frame );

/* Gathers pointers to all the sequences,
   accessible from the <first>, to the single sequence */
CXCOREAPI CxSeq* cxTreeToNodeSeq( const void* first, int header_size,
                                  CxMemStorage* storage );

/* The function implements the K-means algorithm for clustering an array of sample
   vectors in a specified number of classes */
CXCOREAPI  void  cxKMeans2( const CxArr* samples, int cluster_count,
                            CxArr* cluster_idx, CxTermCriteria termcrit );

/****************************************************************************************\
*                                    System functions                                    *
\****************************************************************************************/

typedef struct CxIPPFuncInfo
{
    void** func_addr;
    void* default_func_addr;
    const char* func_names;
    int search_modules;
    int loaded_from;
}
CxIPPFuncInfo;

/* Add the function pointers table with associated information to the IPP primitives list */
CXCOREAPI  int  cxRegisterPrimitives( CxIPPFuncInfo* ipp_func_tab );

/* Loads optimized functions from IPP */
CXCOREAPI  int  cxLoadPrimitives( const char* processor );

/* Retrieves information about CX core and loaded optimized primitives */
CXCOREAPI  void  cxGetLibraryInfo_cxcore( const char** version,
                                          const char** loaded_modules );

/* Get current OpenCX error status */
CXCOREAPI CXStatus cxGetErrStatus( void );

/* Sets error status silently */
CXCOREAPI void cxSetErrStatus( CXStatus status );

#define CX_ErrModeLeaf     0   /* Print error and exit program */
#define CX_ErrModeParent   1   /* Print error and continue */
#define CX_ErrModeSilent   2   /* Don't print and continue */

/* Retrives current error processing mode */
CXCOREAPI int  cxGetErrMode( void );

/* Sets error processing mode */
CXCOREAPI void cxSetErrMode( int mode );

/* Sets error status and performs some additonal actions (error message box,
   writing message to stderr, terminate application etc.)
   depending on the current error mode */
CXCOREAPI CXStatus cxError( CXStatus code, const char *func,
                         const char *context, const char *file, int line);

/* Retrieves textual description of the error given its code */
CXCOREAPI const char* cxErrorStr( CXStatus status );

/* Converts IPP error status to OpenCX error code */
CXCOREAPI CXStatus cxErrorFromStatus( CxStatus ipp_status );

typedef int (CX_CDECL *CXErrorCallBack) (CXStatus status, const char *func,
                                         const char *context, const char *file,int line);

/* Assigns a new error-handling function */
CXCOREAPI CXErrorCallBack cxRedirectError(CXErrorCallBack cxErrorFunc);

/*
    Output to:
        cxNulDevReport - nothing
        cxStdErrReport - console(printf)
        cxGuiBoxReport - MessageBox(WIN32)
*/
CXCOREAPI CXStatus cxNulDevReport( CXStatus status, const char *funcName,
                                const char *context, const char *file, int line );

CXCOREAPI CXStatus cxStdErrReport( CXStatus status, const char *funcName,
                                const char *context, const char *file, int line );

CXCOREAPI CXStatus cxGuiBoxReport( CXStatus status, const char *funcName,
                                const char *context, const char *file, int line);

/* Get call stack */
CXCOREAPI void cxGetCallStack(CxStackRecord** stack, int* size);

/* Push the record to the call stack */
CXCOREAPI void cxStartProfile( const char* call, const char* file, int line );

/* Pop the record from the stack */
CXCOREAPI void cxEndProfile( const char* file, int line );

CX_EXTERN_C_FUNCPTR(void (CX_CDECL* CxStartProfileFunc)(const char*,const char*,int));
CX_EXTERN_C_FUNCPTR(void (CX_CDECL* CxEndProfileFunc)(const char*,int));

/* management functions */
CXCOREAPI void cxSetProfile( CxStartProfileFunc startProfile,
                             CxEndProfileFunc endProfile );
 
CXCOREAPI void cxRemoveProfile();                  


CX_EXTERN_C_FUNCPTR(void* (CX_CDECL *CxAllocFunc)(size_t, const char*, int));
CX_EXTERN_C_FUNCPTR(int (CX_CDECL *CxFreeFunc)(void**, const char*, int));

/* Set user-defined memory managment functions (substitutors for malloc and free) that
   will be called by cxAlloc, cxFree and higher-level functions (e.g. cxCreateImage) */
CXCOREAPI void cxSetMemoryManager( CxAllocFunc allocFunc CX_DEFAULT(0),
                                   CxFreeFunc freeFunc CX_DEFAULT(0));


CX_EXTERN_C_FUNCPTR(IplImage* (CX_STDCALL* Cx_iplCreateImageHeader)
                            (int,int,int,char*,char*,int,int,int,int,int,
                            IplROI*,IplImage*,void*,IplTileInfo*));
CX_EXTERN_C_FUNCPTR(void (CX_STDCALL* Cx_iplAllocateImageData)(IplImage*,int,int));

CX_EXTERN_C_FUNCPTR(void (CX_STDCALL* Cx_iplDeallocate)(IplImage*,int));

CX_EXTERN_C_FUNCPTR(IplROI* (CX_STDCALL* Cx_iplCreateROI)(int,int,int,int,int));

CX_EXTERN_C_FUNCPTR(IplImage* (CX_STDCALL* Cx_iplCloneImage)(const IplImage*));


/* Makes OpenCX use IPL functions for IplImage allocation/deallocation */
CXCOREAPI void cxSetIPLAllocators( Cx_iplCreateImageHeader createHeader,
                                   Cx_iplAllocateImageData allocateData,
                                   Cx_iplDeallocate deallocate,
                                   Cx_iplCreateROI createROI,
                                   Cx_iplCloneImage cloneImage );

#define CX_TURN_ON_IPL_COMPATIBILITY()                                  \
    cxSetIPLAllocators( iplCreateImageHeader, iplAllocateImage,         \
                        iplDeallocate, iplCreateROI, iplCloneImage )

/****************************************************************************************\
*                                    Data Persistence                                    *
\****************************************************************************************/

/********************************** High-level functions ********************************/

/* "black box" file storage */
typedef struct CxFileStorage CxFileStorage;

/* storage flags */
#define CX_STORAGE_READ          0
#define CX_STORAGE_WRITE_TEXT    1
#define CX_STORAGE_WRITE_BINARY  2

/* write flags */
#define CX_WRITE_TREE      2 /* flag for storing sequence trees */

/* opens existing or creates new file storage */
CXCOREAPI  CxFileStorage*  cxOpenFileStorage( const char* filename,
                                              CxMemStorage* storage,
                                              int flags );

/* closes file storage and deallocates buffers */
CXCOREAPI  void cxReleaseFileStorage( CxFileStorage** storage );

/* list of attributes */
typedef struct CxAttrList
{
    char** attr; /* NULL-terminated array of (attribute_name,attribute_value) pairs */
    struct CxAttrList* next; /* pointer to next chunk of the attributes list */
}
CxAttrList;

CX_INLINE CxAttrList cxAttrList( char** attr CX_DEFAULT(NULL),
                                 CxAttrList* next CX_DEFAULT(NULL) );
CX_INLINE CxAttrList cxAttrList( char** attr, CxAttrList* next )
{
    CxAttrList l;
    l.attr = attr;
    l.next = next;

    return l;
}

CXCOREAPI const char* cxAttrValue( const CxAttrList* attr, const char* attr_name );

struct CxTypeInfo;

typedef struct CxFileNode
{
    CX_TREE_NODE_FIELDS(CxFileNode)
    const char* tagname;
    const char* name;
    CxAttrList* attr;
    struct CxFileNode* hash_next;
    unsigned hash_val;
    int elem_size;
    struct CxTypeInfo* typeinfo;
    const char* body;
    const void* content;
}
CxFileNode;


/* writes matrix, image, sequence, graph etc. */
CXCOREAPI  void cxWrite( CxFileStorage* storage, const char* name,
                         const void* struct_ptr,
                         CxAttrList attributes CX_DEFAULT(cxAttrList()),
                         int flags CX_DEFAULT(0));

/* writes opening tag of a compound object (used internally by cxWrite) */
CXCOREAPI  void cxStartWriteStruct( CxFileStorage* storage, const char* name,
                                    const char* type_name CX_DEFAULT(NULL),
                                    const void* struct_ptr CX_DEFAULT(NULL),
                                    CxAttrList attributes CX_DEFAULT(cxAttrList()));

/* writes closing tag of a compound object (used internally by cxWrite) */
CXCOREAPI  void cxEndWriteStruct( CxFileStorage* storage );

/* writes a basic type value or a C structure of such values */
CXCOREAPI  void cxWriteElem( CxFileStorage* storage,
                             const char* name,
                             const char* elem_spec,
                             const void* data_ptr );

/* finds the specified noe of file storage */
CXCOREAPI  CxFileNode* cxGetFileNode( CxFileStorage* storage, const char* name );

/* reads matrix, image, sequence, graph etc. */
CXCOREAPI  const void* cxReadFileNode( CxFileStorage* storage, CxFileNode* node,
                                       CxAttrList** attr_list CX_DEFAULT(NULL));

CX_INLINE  const void* cxRead( CxFileStorage* storage, const char* name,
                               CxAttrList** attr_list CX_DEFAULT(NULL) );
CX_INLINE  const void* cxRead( CxFileStorage* storage, const char* name, CxAttrList** attr_list )
{
    return cxReadFileNode( storage, cxGetFileNode( storage, name ), attr_list );
}

/* read a basic type value or a C structure of such values */
CXCOREAPI  int cxReadElem( CxFileStorage* storage, const char* name, void* data_ptr );

/*********************************** Adding own types ***********************************/

CX_EXTERN_C_FUNCPTR(int (CX_CDECL *CxIsInstanceFunc)(const void* struct_ptr));
CX_EXTERN_C_FUNCPTR(void (CX_CDECL *CxReleaseFunc)(void** struct_dblptr));
CX_EXTERN_C_FUNCPTR(void* (CX_CDECL *CxReadFunc)( CxFileStorage* storage,
                                                  CxFileNode* node ));
CX_EXTERN_C_FUNCPTR(void (CX_CDECL *CxWriteFunc)( CxFileStorage* storage,
                                                  const char* name,
                                                  const void* struct_ptr,
                                                  CxAttrList attributes,
                                                  int flags ));
CX_EXTERN_C_FUNCPTR(void* (CX_CDECL *CxCloneFunc)( const void* struct_ptr));

typedef struct CxTypeInfo
{
    int flags;
    int header_size;
    struct CxTypeInfo* prev;
    struct CxTypeInfo* next;
    const char* type_name;
    CxIsInstanceFunc is_instance;
    CxReleaseFunc release;
    CxReadFunc read;
    CxWriteFunc write;
    CxCloneFunc clone;
}
CxTypeInfo;

CXCOREAPI  CxTypeInfo* cxRegisterType( CxTypeInfo* info_data );
CXCOREAPI  void        cxUnregisterType( const char* type_name );
CXCOREAPI  CxTypeInfo* cxFirstType(void);
CXCOREAPI  CxTypeInfo* cxFindType( const char* type_name );
CXCOREAPI  CxTypeInfo* cxTypeOf( const void* struct_ptr );

/* universal functions */
CXCOREAPI  void cxRelease( void** struct_ptr );
CXCOREAPI  void* cxClone( const void* struct_ptr );

/*********************************** Measuring Execution Time ***************************/

/* helper functions for RNG initialization and accurate time measurement:
   uses internal clock counter on x86 */
CXCOREAPI  int64  cxGetTickCount( void );
CXCOREAPI  double cxGetTickFrequency( void ); 

#ifdef __cplusplus
}
#endif

#endif /*_CXCORE_H_*/
