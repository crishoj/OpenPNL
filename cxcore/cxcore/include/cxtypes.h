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

#ifndef _CXCORE_TYPES_H_
#define _CXCORE_TYPES_H_

#include <assert.h>
#include <stdlib.h>

#ifndef WIN32
    #define CX_CDECL
    #define CX_STDCALL
#else
    #define CX_CDECL __cdecl
    #define CX_STDCALL __stdcall
#endif

#ifndef CX_EXTERN_C
    #ifdef __cplusplus
        #define CX_EXTERN_C extern "C"
        #define CX_DEFAULT(val) = val
    #else
        #define CX_EXTERN_C
        #define CX_DEFAULT(val)
    #endif
#endif

#ifndef CX_EXTERN_C_FUNCPTR
    #ifdef __cplusplus
#define CX_EXTERN_C_FUNCPTR(x) extern "C" { typedef x; }
    #else
        #define CX_EXTERN_C_FUNCPTR(x) typedef x
    #endif
#endif

#ifndef CX_INLINE
#ifdef WIN32
    #define CX_INLINE __inline
#elif defined __cplusplus
    #define CX_INLINE inline
#else
    #define CX_INLINE static
#endif
#endif /* CX_INLINE */

#if defined _MSC_VER || defined __BORLANDC__
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef long long int64;
typedef unsigned long long uint64;
#endif

#ifndef __IPL_H__
typedef unsigned char uchar;
#endif


/* CxArr is used to pass arbitrary array-like data structures
   into the functions and where the particular
   array type is recognized at runtime */ 
typedef void CxArr;


/****************************************************************************************\
*                                  Image type (IplImage)                                 *
\****************************************************************************************/

#ifndef HAVE_IPL

/*
 * The following definitions (until #endif)
 * is an extract from IPL headers.
 * Copyright (c) 1995 Intel Corporation.
 */
#define IPL_DEPTH_SIGN 0x80000000

#define IPL_DEPTH_1U     1
#define IPL_DEPTH_8U     8
#define IPL_DEPTH_16U   16
#define IPL_DEPTH_32F   32

#define IPL_DEPTH_8S  (IPL_DEPTH_SIGN| 8)
#define IPL_DEPTH_16S (IPL_DEPTH_SIGN|16)
#define IPL_DEPTH_32S (IPL_DEPTH_SIGN|32)

#define IPL_DATA_ORDER_PIXEL  0
#define IPL_DATA_ORDER_PLANE  1

#define IPL_ORIGIN_TL 0
#define IPL_ORIGIN_BL 1

#define IPL_ALIGN_4BYTES   4
#define IPL_ALIGN_8BYTES   8
#define IPL_ALIGN_16BYTES 16
#define IPL_ALIGN_32BYTES 32

#define IPL_ALIGN_DWORD   IPL_ALIGN_4BYTES
#define IPL_ALIGN_QWORD   IPL_ALIGN_8BYTES

typedef struct _IplImage
{
    int  nSize;         /* sizeof(IplImage) */
    int  ID;            /* version (=0)*/
    int  nChannels;     /* Most of OpenCX functions support 1,2,3 or 4 channels */
    int  alphaChannel;  /* ignored by OpenCX */
    int  depth;         /* pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
                           IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported */
    char colorModel[4]; /* ignored by OpenCX */
    char channelSeq[4]; /* ditto */
    int  dataOrder;     /* 0 - interleaved color channels, 1 - separate color channels.
                           cxCreateImage can only create interleaved images */
    int  origin;        /* 0 - top-left origin,
                           1 - bottom-left origin (Windows bitmaps style) */
    int  align;         /* Alignment of image rows (4 or 8).
                           OpenCX ignores it and uses widthStep instead */
    int  width;         /* image width in pixels */
    int  height;        /* image height in pixels */
    struct _IplROI *roi;/* image ROI. if NULL, the whole image is selected */
    struct _IplImage *maskROI; /* must be NULL */
    void  *imageId;     /* ditto */
    struct _IplTileInfo *tileInfo; /* ditto */
    int  imageSize;     /* image data size in bytes
                           (==image->height*image->widthStep
                           in case of interleaved data)*/
    char *imageData;  /* pointer to aligned image data */
    int  widthStep;   /* size of aligned image row in bytes */
    int  BorderMode[4]; /* ignored by OpenCX */
    int  BorderConst[4]; /* ditto */
    char *imageDataOrigin; /* pointer to very origin of image data
                              (not necessarily aligned) -
                              needed for correct deallocation */
}
IplImage;

typedef struct _IplTileInfo IplTileInfo;

typedef struct _IplROI
{
    int  coi; /* 0 - no COI (all channels are selected), 1 - 0th channel is selected ...*/
    int  xOffset;
    int  yOffset;
    int  width;
    int  height;
}
IplROI;

typedef struct _IplConvKernel
{
    int  nCols;
    int  nRows;
    int  anchorX;
    int  anchorY;
    int *values;
    int  nShiftR;
}
IplConvKernel;

typedef struct _IplConvKernelFP
{
    int  nCols;
    int  nRows;
    int  anchorX;
    int  anchorY;
    float *values;
}
IplConvKernelFP;

#define IPL_IMAGE_HEADER 1
#define IPL_IMAGE_DATA   2
#define IPL_IMAGE_ROI    4

#endif/*HAVE_IPL*/

#define IPL_IMAGE_MAGIC_VAL  ((int)sizeof(IplImage))

/* for file storages make the value independent from arch */
#define IPL_IMAGE_FILE_MAGIC_VAL  112

#define CX_IS_IMAGE_HDR(img) \
    ((img) != NULL && ((const IplImage*)(img))->nSize == sizeof(IplImage))

#define CX_IS_IMAGE(img) \
    (CX_IS_IMAGE_HDR(img) && ((IplImage*)img)->imageData != NULL)

#define IPL_DEPTH_64F  64 /* for storing double-precision
                             floating point data in IplImage's */

/* get pointer to pixel at (col,row),
   for multi-channel images (col) should be multiplied by number of channels */
#define CX_IMAGE_ELEM( image, elemtype, row, col )       \
    (((elemtype*)((image)->imageData + (image)->widthStep*(row)))[(col)])


/****************************************************************************************\
*                                  Matrix type (CxMat)                                   *
\****************************************************************************************/

#define CX_DEPTH_MAX  5
#define CX_CN_MAX     4 

#define CX_8U   0
#define CX_8S   1
#define CX_16S  2
#define CX_32S  3
#define CX_32F  4
#define CX_64F  5
#define CX_USRTYPE1 6
#define CX_USRTYPE2 7

#define CX_8UC1 (CX_8U + 0*8)
#define CX_8UC2 (CX_8U + 1*8)
#define CX_8UC3 (CX_8U + 2*8)
#define CX_8UC4 (CX_8U + 3*8)

#define CX_8SC1 (CX_8S + 0*8)
#define CX_8SC2 (CX_8S + 1*8)
#define CX_8SC3 (CX_8S + 2*8)
#define CX_8SC4 (CX_8S + 3*8)

#define CX_16SC1 (CX_16S + 0*8)
#define CX_16SC2 (CX_16S + 1*8)
#define CX_16SC3 (CX_16S + 2*8)
#define CX_16SC4 (CX_16S + 3*8)

#define CX_32SC1 (CX_32S + 0*8)
#define CX_32SC2 (CX_32S + 1*8)
#define CX_32SC3 (CX_32S + 2*8)
#define CX_32SC4 (CX_32S + 3*8)

#define CX_32FC1 (CX_32F + 0*8)
#define CX_32FC2 (CX_32F + 1*8)
#define CX_32FC3 (CX_32F + 2*8)
#define CX_32FC4 (CX_32F + 3*8)

#define CX_64FC1 (CX_64F + 0*8)
#define CX_64FC2 (CX_64F + 1*8)
#define CX_64FC3 (CX_64F + 2*8)
#define CX_64FC4 (CX_64F + 3*8)

#define CX_AUTO_STEP  0x7fffffff
#define CX_WHOLE_ARR  cxSlice( 0, 0x3fffffff )

#define CX_MAT_CN_MASK          (3 << 3)
#define CX_MAT_CN(flags)        ((((flags) & CX_MAT_CN_MASK) >> 3) + 1)
#define CX_MAT_DEPTH_MASK       7
#define CX_MAT_DEPTH(flags)     ((flags) & CX_MAT_DEPTH_MASK)
#define CX_MAT_TYPE_MASK        31
#define CX_MAT_TYPE(flags)      ((flags) & CX_MAT_TYPE_MASK)
#define CX_MAT_FMT_MASK         511
#define CX_MAT_FMT(flags)       ((flags) & CX_MAT_FMT_MASK)
#define CX_MAT_CONT_FLAG_SHIFT  9
#define CX_MAT_CONT_FLAG        (1 << CX_MAT_CONT_FLAG_SHIFT)
#define CX_IS_MAT_CONT(flags)   ((flags) & CX_MAT_CONT_FLAG) 
#define CX_IS_CONT_MAT          CX_IS_MAT_CONT
#define CX_MAT_TEMP_FLAG_SHIFT  10
#define CX_MAT_TEMP_FLAG        (1 << CX_MAT_TEMP_FLAG_SHIFT)
#define CX_IS_TEMP_MAT(flags)   ((flags) & CX_MAT_TEMP_FLAG)

#define CX_MAGIC_MASK       0xFFFF0000
#define CX_MAT_MAGIC_VAL    0x42420000

typedef struct CxMat
{
    int type;
    int step;

    /* for internal use only */
    int* refcount;

    union
    {
        uchar* ptr;
        short* s;
        int* i;
        float* fl;
        double* db;
    } data;

#ifdef __cplusplus
    union
    {
        int rows;
        int height;
    };

    union
    {
        int cols;
        int width;
    };
#else
    int rows;
    int cols;
#endif

}
CxMat;


#define CX_IS_MAT_HDR(mat) \
    ((mat) != NULL && (((const CxMat*)(mat))->type & CX_MAGIC_MASK) == CX_MAT_MAGIC_VAL)

#define CX_IS_MAT(mat) \
    (CX_IS_MAT_HDR(mat) && ((const CxMat*)(mat))->data.ptr != NULL)

#define CX_IS_MASK_ARR(mat) \
    (((mat)->type & (CX_MAT_TYPE_MASK & ~CX_8SC1)) == 0)

#define CX_ARE_TYPES_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & CX_MAT_TYPE_MASK) == 0)

#define CX_ARE_CNS_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & CX_MAT_CN_MASK) == 0)

#define CX_ARE_DEPTHS_EQ(mat1, mat2) \
    ((((mat1)->type ^ (mat2)->type) & CX_MAT_DEPTH_MASK) == 0)

#define CX_ARE_SIZES_EQ(mat1, mat2) \
    ((mat1)->height == (mat2)->height && (mat1)->width == (mat2)->width)

#define CX_IS_MAT_CONST(mat)  \
    (((mat)->height|(mat)->width) == 1)

#define CX_ELEM_SIZE(type) \
    (CX_MAT_CN(type) << ((0xe90 >> CX_MAT_DEPTH(type)*2) & 3))

/* inline constructor. No data is allocated internally!!!
   (use together with cxCreateData, or use cxCreateMat instead to
   get a matrix with allocated data */
CX_INLINE CxMat cxMat( int rows, int cols, int type, void* data CX_DEFAULT(NULL));
CX_INLINE CxMat cxMat( int rows, int cols, int type, void* data )
{
    CxMat m;

    assert( (unsigned)CX_MAT_DEPTH(type) <= CX_64F );
    type = CX_MAT_TYPE(type);
    m.type = CX_MAT_MAGIC_VAL | CX_MAT_CONT_FLAG | type;
    m.cols = cols;
    m.rows = rows;
    m.step = m.cols*CX_ELEM_SIZE(type);
    m.data.ptr = (uchar*)data;
    m.refcount = NULL;

    return m; 
}


#define CX_MAT_ELEM_PTR_FAST( mat, row, col, pix_size )  \
    (assert( (unsigned)(row) < (unsigned)(mat).rows &&   \
             (unsigned)(col) < (unsigned)(mat).cols ),   \
     (mat).data.ptr + (size_t)(mat).step*(row) + (pix_size)*(col))

#define CX_MAT_ELEM_PTR( mat, row, col )                 \
    CX_MAT_ELEM_PTR_FAST( mat, row, col, CX_ELEM_SIZE((mat).type) )

#define CX_MAT_ELEM( mat, elemtype, row, col )           \
    (*(elemtype*)CX_MAT_ELEM_PTR_FAST( mat, row, col, sizeof(elemtype)))


CX_INLINE  double  cxmGet( const CxMat* mat, int i, int j );
CX_INLINE  double  cxmGet( const CxMat* mat, int i, int j )
{
    int type;

    type = CX_MAT_TYPE(mat->type);
    assert( (unsigned)i < (unsigned)mat->rows &&
            (unsigned)j < (unsigned)mat->cols );

    if( type == CX_32FC1 )
        return ((float*)(mat->data.ptr + (size_t)mat->step*i))[j];
    else
    {
        assert( type == CX_64FC1 );
        return ((double*)(mat->data.ptr + (size_t)mat->step*i))[j];
    }
}


CX_INLINE  void  cxmSet( CxMat* mat, int i, int j, double val );
CX_INLINE  void  cxmSet( CxMat* mat, int i, int j, double val )
{
    int type;
    type = CX_MAT_TYPE(mat->type);
    assert( (unsigned)i < (unsigned)mat->rows &&
            (unsigned)j < (unsigned)mat->cols );

    if( type == CX_32FC1 )
        ((float*)(mat->data.ptr + (size_t)mat->step*i))[j] = (float)val;
    else
    {
        assert( type == CX_64FC1 );
        ((double*)(mat->data.ptr + (size_t)mat->step*i))[j] = (double)val;
    }
}

/****************************************************************************************\
*                       Multi-dimensional dense array (CxMatND)                          *
\****************************************************************************************/

#define CX_MATND_MAGIC_VAL    0x42430000
#define CX_MAX_DIM 256

#define CX_MAT_LIKE_FLAG_SHIFT  11
#define CX_MAT_LIKE_FLAG        (1 << CX_MAT_LIKE_FLAG_SHIFT)

typedef struct CxMatND
{
    int type;
    int dims;

    int* refcount;
    union
    {
        uchar* ptr;
        float* fl;
        double* db;
        int* i;
        short* s;
    } data;

    struct
    {
        int size;
        int step;
    }
    dim[CX_MAX_DIM];
}
CxMatND;

#define CX_IS_MATND_HDR(mat) \
    ((mat) != NULL && (((const CxMatND*)(mat))->type & CX_MAGIC_MASK) == CX_MATND_MAGIC_VAL)

#define CX_IS_MATND(mat) \
    (CX_IS_MATND_HDR(mat) && ((const CxMatND*)(mat))->data.ptr != NULL)


/****************************************************************************************\
*                      Multi-dimensional sparse array (CxSparseMat)                      *
\****************************************************************************************/

#define CX_SPARSE_MAT_MAGIC_VAL    0x42440000

struct CxSet;

typedef struct CxSparseMat
{
    int type;
    int dims;
    int* refcount;
    struct CxSet* heap;
    void** hashtable;
    int hashsize;
    int total;
    int valoffset;
    int idxoffset;
    int size[CX_MAX_DIM];   
}
CxSparseMat;

#define CX_IS_SPARSE_MAT_HDR(mat) \
    ((mat) != NULL && \
    (((const CxSparseMat*)(mat))->type & CX_MAGIC_MASK) == CX_SPARSE_MAT_MAGIC_VAL)

#define CX_IS_SPARSE_MAT(mat) \
    CX_IS_SPARSE_MAT_HDR(mat)

/**************** iteration through a sparse array *****************/

typedef struct CxSparseNode
{
    unsigned hashval;
    struct CxSparseNode* next;
}
CxSparseNode;

typedef struct CxSparseMatIterator
{
    CxSparseMat* mat;
    CxSparseNode* node;
    int curidx;
}
CxSparseMatIterator;

#define CX_NODE_VAL(mat,node)   ((void*)((uchar*)(node) + (mat)->valoffset))
#define CX_NODE_IDX(mat,node)   ((int*)((uchar*)(node) + (mat)->idxoffset))

/****************************************************************************************\
*                                         Histogram                                      *
\****************************************************************************************/

typedef int CxHistType;

#define CX_HIST_MAGIC_VAL     0x42450000
#define CX_HIST_UNIFORM_FLAG  (1 << 10)

/* indicates whether bin ranges are set already or not */
#define CX_HIST_RANGES_FLAG   (1 << 11)

#define CX_HIST_ARRAY         0
#define CX_HIST_SPARSE        1
#define CX_HIST_TREE          CX_HIST_SPARSE

#define CX_HIST_UNIFORM       1 /* should be used as a parameter only,
                                   it turns to CX_HIST_UNIFORM_FLAG of hist->type */

typedef struct CxHistogram
{
    int     type;
    CxArr*  bins;
    float   thresh[CX_MAX_DIM][2]; /* for uniform histograms */
    float** thresh2; /* for non-uniform histograms */
    CxMatND mat; /* embedded matrix header for array histograms */
}
CxHistogram;

#define CX_IS_HIST( hist ) \
    ((hist) != NULL  && \
     (((CxHistogram*)(hist))->type & CX_MAGIC_MASK) == CX_HIST_MAGIC_VAL && \
     (hist)->bins != NULL)

#define CX_IS_UNIFORM_HIST( hist ) \
    (((hist)->type & CX_HIST_UNIFORM_FLAG) != 0)

#define CX_IS_SPARSE_HIST( hist ) \
    CX_IS_SPARSE_MAT((hist)->bins)

#define CX_HIST_HAS_RANGES( hist ) \
    (((hist)->type & CX_HIST_RANGES_FLAG) != 0) 

/****************************************************************************************\
*                      Other supplementary data type definitions                         *
\****************************************************************************************/

/* ************************************************************* *\
   substitutions for round(x), floor(x), ceil(x):
   the algorithm was taken from Agner Fog's optimization guide
   at http://www.agner.org/assem
\* ************************************************************* */
CX_INLINE  int  cxRound( double val );
CX_INLINE  int  cxRound( double val )
{
    double temp = val + 6755399441055744.0;
    return (int)*((uint64*)&temp);
}


CX_INLINE  int  cxFloor( double val );
CX_INLINE  int  cxFloor( double val )
{
    double temp = val + 6755399441055744.0;
    float diff = (float)(val - (int)*((uint64*)&temp));

    return (int)*((uint64*)&temp) - (*(int*)&diff < 0);
}


CX_INLINE  int  cxCeil( double val );
CX_INLINE  int  cxCeil( double val )
{
    double temp = val + 6755399441055744.0;
    float diff = (float)((int)*((uint64*)&temp) - val);

    return (int)*((uint64*)&temp) + (*(int*)&diff < 0);
}

/*************************************** CxRect *****************************************/

typedef struct CxRect
{
    int x;
    int y;
    int width;
    int height;
}
CxRect;

CX_INLINE  CxRect  cxRect( int x, int y, int width, int height );
CX_INLINE  CxRect  cxRect( int x, int y, int width, int height )
{
    CxRect r;

    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;

    return r;
}


CX_INLINE  IplROI  cxRectToROI( CxRect rect, int coi CX_DEFAULT(0));
CX_INLINE  IplROI  cxRectToROI( CxRect rect, int coi )
{
    IplROI roi;
    roi.xOffset = rect.x;
    roi.yOffset = rect.y;
    roi.width = rect.width;
    roi.height = rect.height;
    roi.coi = coi;

    return roi;
}


CX_INLINE  CxRect  cxROIToRect( IplROI roi );
CX_INLINE  CxRect  cxROIToRect( IplROI roi )
{
    return cxRect( roi.xOffset, roi.yOffset, roi.width, roi.height );
}

/*********************************** CxTermCriteria *************************************/

#define CX_TERMCRIT_ITER    1
#define CX_TERMCRIT_NUMB    CX_TERMCRIT_ITER
#define CX_TERMCRIT_EPS     2

typedef struct CxTermCriteria
{
    int    type;  /* may be combination of
                     CX_TERMCRIT_ITER
                     CX_TERMCRIT_EPS */
    int    maxIter;
    double epsilon;
}
CxTermCriteria;

CX_INLINE  CxTermCriteria  cxTermCriteria( int type, int maxIter, double epsilon );
CX_INLINE  CxTermCriteria  cxTermCriteria( int type, int maxIter, double epsilon )
{
    CxTermCriteria t;

    t.type = type;
    t.maxIter = maxIter;
    t.epsilon = (float)epsilon;

    return t;
}


/******************************* CxPoint and variants ***********************************/

typedef struct CxPoint
{
    int x;
    int y;
}
CxPoint;


CX_INLINE  CxPoint  cxPoint( int x, int y );
CX_INLINE  CxPoint  cxPoint( int x, int y )
{
    CxPoint p;

    p.x = x;
    p.y = y;

    return p;
}


typedef struct CxPoint2D32f
{
    float x;
    float y;
}
CxPoint2D32f;


CX_INLINE  CxPoint2D32f  cxPoint2D32f( double x, double y );
CX_INLINE  CxPoint2D32f  cxPoint2D32f( double x, double y )
{
    CxPoint2D32f p;

    p.x = (float)x;
    p.y = (float)y;

    return p;
}


CX_INLINE  CxPoint2D32f  cxPointTo32f( CxPoint pt );
CX_INLINE  CxPoint2D32f  cxPointTo32f( CxPoint pt )
{
    return cxPoint2D32f( (float)pt.x, (float)pt.y );    
}


CX_INLINE  CxPoint  cxPointFrom32f( CxPoint2D32f pt );
CX_INLINE  CxPoint  cxPointFrom32f( CxPoint2D32f pt )
{
    CxPoint ipt;
    ipt.x = cxRound(pt.x);
    ipt.y = cxRound(pt.y);

    return ipt;
}


typedef struct CxPoint3D32f
{
    float x;
    float y;
    float z;
}
CxPoint3D32f;


CX_INLINE  CxPoint3D32f  cxPoint3D32f( double x, double y, double z );
CX_INLINE  CxPoint3D32f  cxPoint3D32f( double x, double y, double z )
{
    CxPoint3D32f p;

    p.x = (float)x;
    p.y = (float)y;
    p.z = (float)z;

    return p;
}           


typedef struct CxPoint2D64d
{
    double x;
    double y;
}
CxPoint2D64d;


typedef struct CxPoint3D64d
{
    double x;
    double y;
    double z;
}
CxPoint3D64d;


/******************************** CxSize's & CxBox **************************************/

typedef struct
{
    int width;
    int height;
}
CxSize;

CX_INLINE  CxSize  cxSize( int width, int height );
CX_INLINE  CxSize  cxSize( int width, int height )
{
    CxSize s;

    s.width = width;
    s.height = height;

    return s;
}

typedef struct CxSize2D32f
{
    float width;
    float height;
}
CxSize2D32f;


CX_INLINE  CxSize2D32f  cxSize2D32f( double width, double height );
CX_INLINE  CxSize2D32f  cxSize2D32f( double width, double height )
{
    CxSize2D32f s;

    s.width = (float)width;
    s.height = (float)height;

    return s;
}

typedef struct CxBox2D
{
    CxPoint2D32f center;  /* center of the box */
    CxSize2D32f  size;    /* box width and length */
    float angle;          /* angle between the horizontal axis
                             and the first side (i.e. length) in radians */
}
CxBox2D;

/************************************* CxSlice ******************************************/

typedef struct CxSlice
{
    int  startIndex, endIndex;
}
CxSlice;

CX_INLINE  CxSlice  cxSlice( int start, int end );
CX_INLINE  CxSlice  cxSlice( int start, int end )
{
    CxSlice slice;
    slice.startIndex = start;
    slice.endIndex = end;

    return slice;
}

#define CX_WHOLE_SEQ  cxSlice(0, 0x3fffffff)


/************************************* CxScalar *****************************************/

typedef struct CxScalar
{
    double val[4];
}
CxScalar;


CX_INLINE  CxScalar  cxScalar( double a, double b CX_DEFAULT(0),
                               double c CX_DEFAULT(0), double d CX_DEFAULT(0));
CX_INLINE  CxScalar  cxScalar( double a, double b, double c, double d )
{
    CxScalar scalar;
    scalar.val[0] = a; scalar.val[1] = b;
    scalar.val[2] = c; scalar.val[3] = d;
    return scalar;
}


CX_INLINE  CxScalar  cxRealScalar( double a );
CX_INLINE  CxScalar  cxRealScalar( double a )
{
    CxScalar scalar;
    scalar.val[0] = a;
    scalar.val[1] = scalar.val[2] = scalar.val[3] = 0;
    return scalar;
}

CX_INLINE  CxScalar  cxScalarAll( double a );
CX_INLINE  CxScalar  cxScalarAll( double a )
{
    CxScalar scalar;
    scalar.val[0] = scalar.val[1] = scalar.val[2] = scalar.val[3] = a;
    return scalar;
}


/*************** Utility definitions, macros and inline functions ***********************/

#define CX_PI   3.1415926535897932384626433832795

#define CX_SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

/* min & max without jumps */
#define  CX_IMIN(a, b)  ((a) ^ (((a)^(b)) & (((a) < (b)) - 1)))

#define  CX_IMAX(a, b)  ((a) ^ (((a)^(b)) & (((a) > (b)) - 1)))

/* absolute value without jumps */
#define  CX_IABS(a)     (((a) ^ ((a) < 0 ? -1 : 0)) - ((a) < 0 ? -1 : 0))
#define  CX_SIGN(a)     (((a) < 0 ? -1 : 0) | ((a) > 0))

/* initializes 8-element array for fast access to 3x3 neighborhood of a pixel */
#define  CX_INIT_3X3_DELTAS( deltas, step, nch )            \
    ((deltas)[0] =  (nch),  (deltas)[1] = -(step) + (nch),  \
     (deltas)[2] = -(step), (deltas)[3] = -(step) - (nch),  \
     (deltas)[4] = -(nch),  (deltas)[5] =  (step) - (nch),  \
     (deltas)[6] =  (step), (deltas)[7] =  (step) + (nch))

/* ************************************************************************** *\
   Fast square root and inverse square root by
   Bruce W. Holloway, Jeremy M., James Van Buskirk, Vesa Karvonen and others.
   Taken from Paul Hsieh's site http://www.azillionmonkeys.com/qed/sqroot.html.
\* ************************************************************************** */
#define CX_SQRT_MAGIC  0xbe6f0000

CX_INLINE  float  cxInvSqrt( float arg );
CX_INLINE  float  cxInvSqrt( float arg )
{
    float x, y;
    *((unsigned*)&x) = (CX_SQRT_MAGIC - *((unsigned*)&arg))>>1;

    y = arg*0.5f;
    x*= 1.5f - y*x*x;
    x*= 1.5f - y*x*x;
    x*= 1.5f - y*x*x;

    return x;
}

#define cxSqrt(arg)  ((float)sqrt(arg))

/* ************************************************************************** *\
   Fast cube root by Ken Turkowski
   (http://www.worldserver.com/turk/computergraphics/papers.html)
\* ************************************************************************** */
CX_INLINE  float  cxCbrt( float x );
CX_INLINE  float  cxCbrt( float x )
{
    float fr;
    int ix = *(int*)&x, s = ix & 0x80000000;
    int ex, shx;
    ix &= 0x7fffffff;
    ex = (ix >> 23) - 127;
    shx = ex % 3;
    shx -= shx >= 0 ? 3 : 0;
    ex = (ex - shx) / 3; /* exponent of cube root */
    *(int*)&fr = (ix & ((1<<23)-1)) | ((shx + 127)<<23);

    /* 0.125 <= fr < 1.0 */
    /* Use quartic rational polynomial with error < 2^(-24) */
    fr = (float)(((((45.2548339756803022511987494 * fr +
    192.2798368355061050458134625) * fr +
    119.1654824285581628956914143) * fr +
    13.43250139086239872172837314) * fr +
    0.1636161226585754240958355063)/
    ((((14.80884093219134573786480845 * fr +
    151.9714051044435648658557668) * fr +
    168.5254414101568283957668343) * fr +
    33.9905941350215598754191872) * fr +
    1.0));

    /* fr *= 2^ex * sign */
    *(int*)&fr = (*(int*)&fr + (ex << 23) + s) & (*(int*)&x*2 != 0 ? -1 : 0);

    return(fr);
}


CX_INLINE int cxIsNaN( double val );
CX_INLINE int cxIsNaN( double val )
{
    unsigned lo = (unsigned)*(uint64*)&val;
    unsigned hi = (unsigned)(*(uint64*)&val >> 32);
    return (hi & 0x7fffffff) + (lo != 0) > 0x7ff00000;
}


CX_INLINE int cxIsInf( double val );
CX_INLINE int cxIsInf( double val )
{
    unsigned lo = (unsigned)*(uint64*)&val;
    unsigned hi = (unsigned)(*(uint64*)&val >> 32);
    return (hi & 0x7fffffff) == 0x7ff00000 && lo == 0;
}

/****************************************************************************************\
*                                   Dynamic Data structures                              *
\****************************************************************************************/

/******************************** Memory storage ****************************************/

typedef struct CxMemBlock
{
    struct CxMemBlock*  prev;
    struct CxMemBlock*  next;
}
CxMemBlock;

#define CX_STORAGE_MAGIC_VAL    0x42890000

typedef struct CxMemStorage
{
    int     signature;
    CxMemBlock* bottom;/* first allocated block */
    CxMemBlock* top;   /* current memory block - top of the stack */
    struct  CxMemStorage* parent; /* borrows new blocks from */
    int     block_size;  /* block size */
    int     free_space;  /* free space in the current block */
}
CxMemStorage;

#define CX_IS_STORAGE(storage)  \
    ((storage) != NULL &&       \
    (((CxMemStorage*)(storage))->signature & CX_MAGIC_MASK) == CX_STORAGE_MAGIC_VAL)


typedef struct CxMemStoragePos
{
    CxMemBlock* top;
    int  free_space;
}
CxMemStoragePos;


/*********************************** Sequence *******************************************/

typedef struct CxSeqBlock
{
    struct CxSeqBlock*  prev; /* previous sequence block */
    struct CxSeqBlock*  next; /* next sequence block */
    int    start_index;       /* index of the first element in the block +
                                 sequence->first->start_index */
    int    count;             /* number of elements in the block */
    char*  data;              /* pointer to the first element of the block */
}
CxSeqBlock;


#define CX_TREE_NODE_FIELDS(node_type)                          \
    int       flags;         /* micsellaneous flags */          \
    int       header_size;   /* size of sequence header */      \
    struct    node_type* h_prev; /* previous sequence */        \
    struct    node_type* h_next; /* next sequence */            \
    struct    node_type* v_prev; /* 2nd previous sequence */    \
    struct    node_type* v_next; /* 2nd next sequence */

/*
   Read/Write sequence.
   Elements can be dynamically inserted to or deleted from the sequence.
*/
#define CX_SEQUENCE_FIELDS()                                            \
    CX_TREE_NODE_FIELDS(CxSeq)                                          \
    int       total;          /* total number of elements */            \
    int       elem_size;      /* size of sequence element in bytes */   \
    char*     block_max;      /* maximal bound of the last block */     \
    char*     ptr;            /* current write pointer */               \
    int       delta_elems;    /* how many elements allocated when the seq grows */  \
    CxMemStorage* storage;    /* where the seq is stored */             \
    CxSeqBlock* free_blocks;  /* free blocks list */                    \
    CxSeqBlock* first; /* pointer to the first sequence block */


typedef struct CxSeq
{
    CX_SEQUENCE_FIELDS()
}
CxSeq;


/*************************************** Set ********************************************/
/*
  Set.
  Order isn't keeped. There can be gaps between sequence elements.
  After the element has been inserted it stays on the same place all the time.
  The MSB(most-significant or sign bit) of the first field is 0 iff the element exists.
*/
#define CX_SET_ELEM_FIELDS(elem_type)   \
    int  flags;                         \
    struct elem_type* next_free;

typedef struct CxSetElem
{
    CX_SET_ELEM_FIELDS(CxSetElem)
}
CxSetElem;

#define CX_SET_FIELDS()      \
    CX_SEQUENCE_FIELDS()     \
    CxSetElem* free_elems;

typedef struct CxSet
{
    CX_SET_FIELDS()
}
CxSet;


#define CX_SET_ELEM_IDX_MASK   ((1 << 24) - 1)
#define CX_SET_ELEM_FREE_FLAG  (1 << (sizeof(int)*8-1))

/* Checks whether the element pointed by ptr belongs to a set or not */
#define CX_IS_SET_ELEM( ptr )  (((CxSetElem*)(ptr))->flags >= 0)

/************************************* Graph ********************************************/

/*
  Graph is represented as a set of vertices.
  Vertices contain their adjacency lists (more exactly, pointers to first incoming or
  outcoming edge (or 0 if isolated vertex)). Edges are stored in another set.
  There is a single-linked list of incoming/outcoming edges for each vertex.

  Each edge consists of:
    two pointers to the starting and the ending vertices (vtx[0] and vtx[1],
    respectively). Graph may be oriented or not. In the second case, edges between
    vertex i to vertex j are not distingueshed (during the search operations).

    two pointers to next edges for the starting and the ending vertices.
    next[0] points to the next edge in the vtx[0] adjacency list and
    next[1] points to the next edge in the vtx[1] adjacency list.
*/
#define CX_GRAPH_EDGE_FIELDS()      \
    int flags;                      \
    float weight;                   \
    struct CxGraphEdge* next[2];    \
    struct CxGraphVtx* vtx[2];
    

#define CX_GRAPH_VERTEX_FIELDS()    \
    int flags;                      \
    struct CxGraphEdge* first;
    

typedef struct CxGraphEdge
{
    CX_GRAPH_EDGE_FIELDS()
}
CxGraphEdge;

typedef struct CxGraphVtx
{
    CX_GRAPH_VERTEX_FIELDS()
}
CxGraphVtx;

typedef struct CxGraphVtx2D
{
    CX_GRAPH_VERTEX_FIELDS()
    CxPoint2D32f* ptr;
}
CxGraphVtx2D;

/*
   Graph is "derived" from the set (this is set a of vertices)
   and includes another set (edges)
*/
#define  CX_GRAPH_FIELDS()   \
    CX_SET_FIELDS()          \
    CxSet* edges;

typedef struct CxGraph
{
    CX_GRAPH_FIELDS()
}
CxGraph;

/*********************************** Chain/Countour *************************************/

typedef struct CxChain
{
    CX_SEQUENCE_FIELDS()
    CxPoint  origin;
}
CxChain;

#define CX_CONTOUR_FIELDS()  \
    CX_SEQUENCE_FIELDS()     \
    CxRect rect;             \
    int color;               \
    int reserved[3];

typedef struct CxContour
{
    CX_CONTOUR_FIELDS()
}
CxContour;

typedef CxContour CxPoint2DSeq;

/****************************************************************************************\
*                                    Sequence types                                      *
\****************************************************************************************/

#define CX_SEQ_MAGIC_VAL             0x42990000
#define CX_IS_SEQ(seq) \
    ((seq) != NULL && (((CxSeq*)(seq))->flags & CX_MAGIC_MASK) == CX_SEQ_MAGIC_VAL)

#define CX_SET_MAGIC_VAL             0x42980000
#define CX_IS_SET(set) \
    ((set) != NULL && (((CxSeq*)(set))->flags & CX_MAGIC_MASK) == CX_SET_MAGIC_VAL)

#define CX_SEQ_ELTYPE_BITS           5
#define CX_SEQ_ELTYPE_MASK           ((1 << CX_SEQ_ELTYPE_BITS) - 1)

#define CX_SEQ_ELTYPE_POINT          CX_32SC2  /* (x,y) */
#define CX_SEQ_ELTYPE_CODE           CX_8UC1   /* freeman code: 0..7 */
#define CX_SEQ_ELTYPE_GENERIC        0
#define CX_SEQ_ELTYPE_PTR            CX_USRTYPE1 
#define CX_SEQ_ELTYPE_PPOINT         CX_SEQ_ELTYPE_PTR  /* &(x,y) */
#define CX_SEQ_ELTYPE_INDEX          CX_32SC1  /* #(x,y) */
#define CX_SEQ_ELTYPE_GRAPH_EDGE     0  /* &next_o, &next_d, &vtx_o, &vtx_d */
#define CX_SEQ_ELTYPE_GRAPH_VERTEX   0  /* first_edge, &(x,y) */
#define CX_SEQ_ELTYPE_TRIAN_ATR      0  /* vertex of the binary tree   */
#define CX_SEQ_ELTYPE_CONNECTED_COMP 0  /* connected component  */
#define CX_SEQ_ELTYPE_POINT3D        CX_32FC3  /* (x,y,z)  */

#define CX_SEQ_KIND_BITS        5
#define CX_SEQ_KIND_MASK        (((1 << CX_SEQ_KIND_BITS) - 1)<<CX_SEQ_ELTYPE_BITS)

/* types of sequences */
#define CX_SEQ_KIND_GENERIC     (0 << CX_SEQ_ELTYPE_BITS)
#define CX_SEQ_KIND_CURVE       (1 << CX_SEQ_ELTYPE_BITS)
#define CX_SEQ_KIND_BIN_TREE    (2 << CX_SEQ_ELTYPE_BITS)

/* types of sparse sequences (sets) */
#define CX_SEQ_KIND_GRAPH       (3 << CX_SEQ_ELTYPE_BITS)
#define CX_SEQ_KIND_SUBDIV2D    (4 << CX_SEQ_ELTYPE_BITS)

#define CX_SEQ_FLAG_SHIFT       (CX_SEQ_KIND_BITS + CX_SEQ_ELTYPE_BITS)

/* flags for curves */
#define CX_SEQ_FLAG_CLOSED     (1 << CX_SEQ_FLAG_SHIFT)
#define CX_SEQ_FLAG_SIMPLE     (2 << CX_SEQ_FLAG_SHIFT)
#define CX_SEQ_FLAG_CONVEX     (4 << CX_SEQ_FLAG_SHIFT)
#define CX_SEQ_FLAG_HOLE       (8 << CX_SEQ_FLAG_SHIFT)

/* flags for graphs */
#define CX_GRAPH_FLAG_ORIENTED (1 << CX_SEQ_FLAG_SHIFT)

#define CX_GRAPH               CX_SEQ_KIND_GRAPH
#define CX_ORIENTED_GRAPH      (CX_SEQ_KIND_GRAPH|CX_GRAPH_FLAG_ORIENTED)

/* point sets */
#define CX_SEQ_POINT_SET       (CX_SEQ_KIND_GENERIC| CX_SEQ_ELTYPE_POINT)
#define CX_SEQ_POINT3D_SET     (CX_SEQ_KIND_GENERIC| CX_SEQ_ELTYPE_POINT3D)
#define CX_SEQ_POLYLINE        (CX_SEQ_KIND_CURVE  | CX_SEQ_ELTYPE_POINT)
#define CX_SEQ_POLYGON         (CX_SEQ_FLAG_CLOSED | CX_SEQ_POLYLINE )
#define CX_SEQ_CONTOUR         CX_SEQ_POLYGON
#define CX_SEQ_SIMPLE_POLYGON  (CX_SEQ_FLAG_SIMPLE | CX_SEQ_POLYGON  )

/* chain-coded curves */
#define CX_SEQ_CHAIN           (CX_SEQ_KIND_CURVE  | CX_SEQ_ELTYPE_CODE)
#define CX_SEQ_CHAIN_CONTOUR   (CX_SEQ_FLAG_CLOSED | CX_SEQ_CHAIN)

/* binary tree for the contour */
#define CX_SEQ_POLYGON_TREE    (CX_SEQ_KIND_BIN_TREE  | CX_SEQ_ELTYPE_TRIAN_ATR)

/* sequence of the connected components */
#define CX_SEQ_CONNECTED_COMP  (CX_SEQ_KIND_GENERIC  | CX_SEQ_ELTYPE_CONNECTED_COMP)

/* sequence of the integer numbers */
#define CX_SEQ_INDEX           (CX_SEQ_KIND_GENERIC  | CX_SEQ_ELTYPE_INDEX)

#define CX_SEQ_ELTYPE( seq )   ((seq)->flags & CX_SEQ_ELTYPE_MASK)
#define CX_SEQ_KIND( seq )     ((seq)->flags & CX_SEQ_KIND_MASK )

/* flag checking */
#define CX_IS_SEQ_INDEX( seq )      ((CX_SEQ_ELTYPE(seq) == CX_SEQ_ELTYPE_INDEX) && \
                                     (CX_SEQ_KIND(seq) == CX_SEQ_KIND_GENERIC))

#define CX_IS_SEQ_CURVE( seq )      (CX_SEQ_KIND(seq) == CX_SEQ_KIND_CURVE)
#define CX_IS_SEQ_CLOSED( seq )     (((seq)->flags & CX_SEQ_FLAG_CLOSED) != 0)
#define CX_IS_SEQ_CONVEX( seq )     (((seq)->flags & CX_SEQ_FLAG_CONVEX) != 0)
#define CX_IS_SEQ_HOLE( seq )       (((seq)->flags & CX_SEQ_FLAG_HOLE) != 0)
#define CX_IS_SEQ_SIMPLE( seq )     ((((seq)->flags & CX_SEQ_FLAG_SIMPLE) != 0) || \
                                    CX_IS_SEQ_CONVEX(seq))

/* type checking macros */
#define CX_IS_SEQ_POINT_SET( seq ) \
    ((CX_SEQ_ELTYPE(seq) == CX_32SC2 || CX_SEQ_ELTYPE(seq) == CX_32FC2))

#define CX_IS_SEQ_POINT_SUBSET( seq ) \
    (CX_IS_SEQ_INDEX( seq ) || CX_SEQ_ELTYPE(seq) == CX_SEQ_ELTYPE_PPOINT)

#define CX_IS_SEQ_POLYLINE( seq )   \
    (CX_SEQ_KIND(seq) == CX_SEQ_KIND_CURVE && CX_IS_SEQ_POINT_SET(seq))

#define CX_IS_SEQ_POLYGON( seq )   \
    (CX_IS_SEQ_POLYLINE(seq) && CX_IS_SEQ_CLOSED(seq))

#define CX_IS_SEQ_CHAIN( seq )   \
    (CX_SEQ_KIND(seq) == CX_SEQ_KIND_CURVE && (seq)->elem_size == 1)

#define CX_IS_SEQ_CONTOUR( seq )   \
    (CX_IS_SEQ_CLOSED(seq) && (CX_IS_SEQ_POLYLINE(seq) || CX_IS_SEQ_CHAIN(seq)))

#define CX_IS_SEQ_CHAIN_CONTOUR( seq ) \
    (CX_IS_SEQ_CHAIN( seq ) && CX_IS_SEQ_CLOSED( seq ))

#define CX_IS_SEQ_POLYGON_TREE( seq ) \
    (CX_SEQ_ELTYPE (seq) ==  CX_SEQ_ELTYPE_TRIAN_ATR &&    \
    CX_SEQ_KIND( seq ) ==  CX_SEQ_KIND_BIN_TREE )

#define CX_IS_GRAPH( seq )    \
    (CX_IS_SET(seq) && CX_SEQ_KIND((CxSet*)(seq)) == CX_SEQ_KIND_GRAPH)

#define CX_IS_GRAPH_ORIENTED( seq )   \
    (((seq)->flags & CX_GRAPH_FLAG_ORIENTED) != 0)

#define CX_IS_SUBDIV2D( seq )  \
    (CX_IS_SET(seq) && CX_SEQ_KIND((CxSet*)(seq)) == CX_SEQ_KIND_SUBDIV2D)

/****************************************************************************************/
/*                            Sequence writer & reader                                  */
/****************************************************************************************/

#define CX_SEQ_WRITER_FIELDS()                                     \
    int          header_size;                                      \
    CxSeq*       seq;        /* the sequence written */            \
    CxSeqBlock*  block;      /* current block */                   \
    char*        ptr;        /* pointer to free space */           \
    char*        block_min;  /* pointer to the beginning of block*/\
    char*        block_max;  /* pointer to the end of block */

typedef struct CxSeqWriter
{
    CX_SEQ_WRITER_FIELDS()
    int  reserved[4]; /* some reserved fields */
}
CxSeqWriter;


#define CX_SEQ_READER_FIELDS()                                      \
    int          header_size;                                       \
    CxSeq*       seq;        /* sequence, beign read */             \
    CxSeqBlock*  block;      /* current block */                    \
    char*        ptr;        /* pointer to element be read next */  \
    char*        block_min;  /* pointer to the beginning of block */\
    char*        block_max;  /* pointer to the end of block */      \
    int          delta_index;/* = seq->first->start_index   */      \
    char*        prev_elem;  /* pointer to previous element */


typedef struct CxSeqReader
{
    CX_SEQ_READER_FIELDS()
    int  reserved[4];
}
CxSeqReader;

/****************************************************************************************/
/*                                Operations on sequences                               */
/****************************************************************************************/

#define  CX_GET_SEQ_ELEM( elem_type, seq, index )                \
/* assert gives some guarantee that <seq> parameter is valid */  \
(   assert(sizeof((seq)->first[0]) == sizeof(CxSeqBlock) &&      \
    (seq)->elem_size == sizeof(elem_type)),                      \
    (elem_type*)((seq)->first && (unsigned)index <               \
    (unsigned)((seq)->first->count) ?                            \
    (seq)->first->data + (index) * sizeof(elem_type) :           \
    cxGetSeqElem( (CxSeq*)(seq), (index), NULL )))


/* macro that adds element to sequence */
#define CX_WRITE_SEQ_ELEM_VAR( elem_ptr, writer )     \
{                                                     \
    if( (writer).ptr >= (writer).block_max )          \
    {                                                 \
        cxCreateSeqBlock( &writer);                   \
    }                                                 \
    memcpy((writer).ptr, elem_ptr, (writer).seq->elem_size);\
    (writer).ptr += (writer).seq->elem_size;          \
}

#define CX_WRITE_SEQ_ELEM( elem, writer )             \
{                                                     \
    assert( (writer).seq->elem_size == sizeof(elem)); \
    if( (writer).ptr >= (writer).block_max )          \
    {                                                 \
        cxCreateSeqBlock( &writer);                   \
    }                                                 \
    assert( (writer).ptr <= (writer).block_max - sizeof(elem));\
    memcpy((writer).ptr, &elem, sizeof(elem));        \
    (writer).ptr += sizeof(elem);                     \
}


/* move reader position forward */
#define CX_NEXT_SEQ_ELEM( elem_size, reader )                 \
{                                                             \
    if( ((reader).ptr += (elem_size)) >= (reader).block_max ) \
    {                                                         \
        cxChangeSeqBlock( &(reader), 1 );                     \
    }                                                         \
}


/* move reader position backward */
#define CX_PREV_SEQ_ELEM( elem_size, reader )                \
{                                                            \
    if( ((reader).ptr -= (elem_size)) < (reader).block_min ) \
    {                                                        \
        cxChangeSeqBlock( &(reader), -1 );                   \
    }                                                        \
}

/* read element and move read position forward */
#define CX_READ_SEQ_ELEM( elem, reader )                       \
{                                                              \
    assert( (reader).seq->elem_size == sizeof(elem));          \
    memcpy( &(elem), (reader).ptr, sizeof((elem)));            \
    CX_NEXT_SEQ_ELEM( sizeof(elem), reader )                   \
}

/* read element and move read position backward */
#define CX_REV_READ_SEQ_ELEM( elem, reader )                     \
{                                                                \
    assert( (reader).seq->elem_size == sizeof(elem));            \
    memcpy(&(elem), (reader).ptr, sizeof((elem)));               \
    CX_PREV_SEQ_ELEM( sizeof(elem), reader )                     \
}


#define CX_READ_CHAIN_POINT( _pt, reader )                              \
{                                                                       \
    (_pt) = (reader).pt;                                                \
    if( (reader).ptr )                                                  \
    {                                                                   \
        CX_READ_SEQ_ELEM( (reader).code, (*((CxSeqReader*)&(reader)))); \
        assert( ((reader).code & ~7) == 0 );                            \
        (reader).pt.x += (reader).deltas[(reader).code][0];             \
        (reader).pt.y += (reader).deltas[(reader).code][1];             \
    }                                                                   \
}

#define CX_CURRENT_POINT( reader )  (*((CxPoint*)((reader).ptr)))
#define CX_PREV_POINT( reader )     (*((CxPoint*)((reader).prev_elem)))

#define CX_READ_EDGE( pt1, pt2, reader )               \
{                                                      \
    assert( sizeof(pt1) == sizeof(CxPoint) &&          \
            sizeof(pt2) == sizeof(CxPoint) &&          \
            reader.seq->elem_size == sizeof(CxPoint)); \
    (pt1) = CX_PREV_POINT( reader );                   \
    (pt2) = CX_CURRENT_POINT( reader );                \
    (reader).prev_elem = (reader).ptr;                 \
    CX_NEXT_SEQ_ELEM( sizeof(CxPoint), (reader));      \
}

/************ Graph macros ************/

/* returns next graph edge for given vertex */
#define  CX_NEXT_GRAPH_EDGE( edge, vertex )                              \
     (assert((edge)->vtx[0] == (vertex) || (edge)->vtx[1] == (vertex)),  \
      (edge)->next[(edge)->vtx[1] == (vertex)])

/****************************************************************************************/

/**** For error processing and debugging purposes ******/ 
typedef struct
{
    const char* file;
    int         line;
}
CxStackRecord;                               

/* IPP-compatible return codes */
typedef enum CxStatus
{         
    CX_INPLACE_NOT_SUPPORTED_ERR= -112,
    CX_UNMATCHED_ROI_ERR        = -111,
    CX_NOTFOUND_ERR             = -110,
    CX_BADCONVERGENCE_ERR       = -109,

    CX_BADDEPTH_ERR             = -107,
    CX_BADROI_ERR               = -106,
    CX_BADHEADER_ERR            = -105,
    CX_UNMATCHED_FORMATS_ERR    = -104,
    CX_UNSUPPORTED_COI_ERR      = -103,
    CX_UNSUPPORTED_CHANNELS_ERR = -102,
    CX_UNSUPPORTED_DEPTH_ERR    = -101,
    CX_UNSUPPORTED_FORMAT_ERR   = -100,

    CX_BADARG_ERR      = -49,  //ipp comp
    CX_NOTDEFINED_ERR  = -48,  //ipp comp

    CX_BADCHANNELS_ERR = -47,  //ipp comp
    CX_BADRANGE_ERR    = -44,  //ipp comp
    CX_BADSTEP_ERR     = -29,  //ipp comp

    CX_BADFLAG_ERR     =  -12,
    CX_DIV_BY_ZERO_ERR =  -11, //ipp comp
    CX_BADCOEF_ERR     =  -10,

    CX_BADFACTOR_ERR   =  -7,
    CX_BADPOINT_ERR    =  -6,
    CX_BADSCALE_ERR    =  -4,
    CX_OUTOFMEM_ERR    =  -3,
    CX_NULLPTR_ERR     =  -2,
    CX_BADSIZE_ERR     =  -1,
    CX_NO_ERR          =   0,
    CX_OK              =   CX_NO_ERR
}
CxStatus;

#endif /*_CXCORE_TYPES_H_*/

/* End of file. */
