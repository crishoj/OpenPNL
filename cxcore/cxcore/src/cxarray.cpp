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
//  CxMat, CxMatND, CxSparceMat and IplImage support functions
//  (creation, deletion, copying, retrieving and setting elements etc.)
//
// */

#include "_cxcore.h"

static struct
{
    Cx_iplCreateImageHeader  createHeader;
    Cx_iplAllocateImageData  allocateData;
    Cx_iplDeallocate  deallocate;
    Cx_iplCreateROI  createROI;
    Cx_iplCloneImage  cloneImage;
}
CxIPL;

// Makes the library use native IPL image allocators
CX_IMPL void
cxSetIPLAllocators( Cx_iplCreateImageHeader createHeader,
                    Cx_iplAllocateImageData allocateData,
                    Cx_iplDeallocate deallocate,
                    Cx_iplCreateROI createROI,
                    Cx_iplCloneImage cloneImage )
{
    CX_FUNCNAME( "cxSetIPLAllocators" );

    __BEGIN__;

    if( !createHeader || !allocateData || !deallocate || !createROI || !cloneImage )
    {
        if( createHeader || allocateData || deallocate || createROI || cloneImage )
            CX_ERROR( CX_StsBadArg, "Either all the pointers should be null or "
                                    "they all should be non-null" );
    }

    CxIPL.createHeader = createHeader;
    CxIPL.allocateData = allocateData;
    CxIPL.deallocate = deallocate;
    CxIPL.createROI = createROI;
    CxIPL.cloneImage = cloneImage;

    __END__;
}


/****************************************************************************************\
*                               CxMat creation and basic operations                      *
\****************************************************************************************/

// Creates CxMat and underlying data
CX_IMPL CxMat*
cxCreateMat( int height, int width, int type )
{
    CxMat* arr = 0;

    CX_FUNCNAME( "cxCreateMat" );
    
    __BEGIN__;

    CX_CALL( arr = cxCreateMatHeader( height, width, type ));
    CX_CALL( cxCreateData( arr ));

    __END__;

    if( cxGetErrStatus() < 0 )
        cxReleaseMat( &arr );

    return arr;
}


CX_INLINE void icxCheckHuge( CxMat* arr );
CX_INLINE void icxCheckHuge( CxMat* arr )
{
    if( (int64)arr->step*arr->rows > INT_MAX )
        arr->type &= ~CX_MAT_CONT_FLAG;
}

// Creates CxMat header only
CX_IMPL CxMat*
cxCreateMatHeader( int rows, int cols, int type )
{
    CxMat* arr = 0;
    
    CX_FUNCNAME( "cxCreateMatHeader" );

    __BEGIN__;

    int min_step;
    type = CX_MAT_TYPE(type);

    if( rows <= 0 || cols <= 0 )
        CX_ERROR( CX_StsBadSize, "Non-positive width or height" );

    min_step = icxPixSize[type]*cols;
    if( min_step <= 0 )
        CX_ERROR( CX_StsUnsupportedFormat, "Invalid matrix type" );

    CX_CALL( arr = (CxMat*)cxAlloc( sizeof(*arr)));

    arr->step = rows == 1 ? 0 : (min_step + CX_DEFAULT_MAT_ROW_ALIGN-1)
                & -CX_DEFAULT_MAT_ROW_ALIGN;
    arr->type = CX_MAT_MAGIC_VAL | type |
                (arr->step == 0 || arr->step == min_step ? CX_MAT_CONT_FLAG : 0);
    arr->rows = rows;
    arr->cols = cols;
    arr->data.ptr = 0;
    arr->refcount = 0;

    icxCheckHuge( arr );

    __END__;

    if( cxGetErrStatus() < 0 )
        cxReleaseMat( &arr );

    return arr;
}


// Initializes CxMat header, allocated by the user
CX_IMPL CxMat*
cxInitMatHeader( CxMat* arr, int rows, int cols,
                 int type, void* data, int step )
{
    CX_FUNCNAME( "cxInitMatHeader" );
    
    __BEGIN__;

    int mask, pix_size, min_step;

    if( !arr )
        CX_ERROR_FROM_CODE( CX_StsNullPtr );

    if( (unsigned)CX_MAT_DEPTH(type) > CX_DEPTH_MAX )
        CX_ERROR_FROM_CODE( CX_BadNumChannels );

    if( rows <= 0 || cols <= 0 )
        CX_ERROR( CX_StsBadSize, "Non-positive cols or rows" );
 
    type = CX_MAT_TYPE( type );
    arr->type = type | CX_MAT_MAGIC_VAL;
    arr->rows = rows;
    arr->cols = cols;
    arr->data.ptr = (uchar*)data;
    arr->refcount = 0;

    mask = (arr->rows <= 1) - 1;
    pix_size = icxPixSize[type];
    min_step = arr->cols*pix_size & mask;

    if( step != CX_AUTOSTEP && step != 0 )
    {
        if( step < min_step )
            CX_ERROR_FROM_CODE( CX_BadStep );
        arr->step = step & mask;
    }
    else
    {
        arr->step = min_step;
    }

    arr->type = CX_MAT_MAGIC_VAL | type |
                (arr->step == min_step ? CX_MAT_CONT_FLAG : 0);

    icxCheckHuge( arr );

    __END__;

    return arr;
}


// Deallocates the CxMat structure and underlying data
CX_IMPL void
cxReleaseMat( CxMat** array )
{
    CX_FUNCNAME( "cxReleaseMat" );
    
    __BEGIN__;

    if( !array )
        CX_ERROR_FROM_CODE( CX_HeaderIsNull );

    if( *array )
    {
        CxMat* arr = *array;
        
        if( !CX_IS_MAT_HDR(arr) && !CX_IS_MATND_HDR(arr) )
            CX_ERROR_FROM_CODE( CX_StsBadFlag );

        *array = 0;

        cxDecRefData( arr );
        cxFree( (void**)&arr );
    }

    __END__;
}


// Creates a copy of matrix
CX_IMPL CxMat*
cxCloneMat( const CxMat* src )
{
    CxMat* dst = 0;
    CX_FUNCNAME( "cxCloneMat" );

    __BEGIN__;

    if( !CX_IS_MAT_HDR( src ))
        CX_ERROR( CX_StsBadArg, "Bad CxMat header" );

    CX_CALL( dst = cxCreateMatHeader( src->rows, src->cols, src->type ));

    if( src->data.ptr )
    {
        CX_CALL( cxCreateData( dst ));
        CX_CALL( cxCopy( src, dst ));
    }

    __END__;

    return dst;
}


/****************************************************************************************\
*                               CxMatND creation and basic operations                    *
\****************************************************************************************/

CxMatND*
cxInitMatNDHeader( CxMatND* mat, int dims, const int* sizes, int type, void* data )
{
    CxMatND* result = 0;

    CX_FUNCNAME( "cxInitMatNDHeader" );

    __BEGIN__;

    type = CX_MAT_TYPE(type);
    int i;
    int64 step = icxPixSize[type];

    if( !mat )
        CX_ERROR( CX_StsNullPtr, "NULL matrix header pointer" );

    if( step == 0 )
        CX_ERROR( CX_StsUnsupportedFormat, "invalid array data type" );

    if( (unsigned)dims > CX_MAX_DIM )
        CX_ERROR( CX_StsOutOfRange, "bad number of dimensions" );

    if( !sizes )
        CX_ERROR( CX_StsNullPtr, "NULL <sizes> pointer" );

    for( i = dims - 1; i >= 0; i-- )
    {
        if( sizes[i] <= 0 )
            CX_ERROR( CX_StsBadSize, "one of dimesion sizes is non-positive" );
        mat->dim[i].size = sizes[i];
        if( step > INT_MAX )
            CX_ERROR( CX_StsOutOfRange, "The array is too big" );
        mat->dim[i].step = (int)step;
        step *= sizes[i];
    }

    mat->type = CX_MATND_MAGIC_VAL | (step <= INT_MAX ? CX_MAT_CONT_FLAG : 0) | type;
    mat->dims = dims;
    mat->data.ptr = (uchar*)data;
    mat->refcount = 0;
    result = mat;

    __END__;

    if( cxGetErrStatus() < 0 && mat )
    {
        mat->type = 0;
        mat->data.ptr = 0;
    }

    return result;
}


// Creates CxMatND and underlying data
CX_IMPL CxMatND*
cxCreateMatND( int dims, const int* sizes, int type )
{
    CxMatND* arr = 0;

    CX_FUNCNAME( "cxCreateMatND" );
    
    __BEGIN__;

    CX_CALL( arr = cxCreateMatNDHeader( dims, sizes, type ));
    CX_CALL( cxCreateData( arr ));

    __END__;

    if( cxGetErrStatus() < 0 )
        cxReleaseMatND( &arr );

    return arr;
}


// Creates CxMatND header only
CX_IMPL CxMatND*
cxCreateMatNDHeader( int dims, const int* sizes, int type )
{
    CxMatND* arr = 0;
    
    CX_FUNCNAME( "cxCreateMatNDHeader" );

    __BEGIN__;

    CX_CALL( arr = (CxMatND*)cxAlloc( sizeof(*arr)));
    CX_CALL( cxInitMatNDHeader( arr, dims, sizes, type ));

    __END__;

    if( cxGetErrStatus() < 0 )
        cxReleaseMatND( &arr );

    return arr;
}


// Creates a copy of nD array
CX_IMPL CxMatND*
cxCloneMatND( const CxMatND* src )
{
    CxMatND* dst = 0;
    CX_FUNCNAME( "cxCloneMatND" );

    __BEGIN__;

    int i, sizes[CX_MAX_DIM];

    if( !CX_IS_MATND_HDR( src ))
        CX_ERROR( CX_StsBadArg, "Bad CxMatND header" );

    for( i = 0; i < src->dims; i++ )
        sizes[i] = src->dim[i].size;

    CX_CALL( dst = cxCreateMatNDHeader( src->dims, sizes, src->type ));

    if( src->data.ptr )
    {
        CX_CALL( cxCreateData( dst ));
        CX_CALL( cxCopy( src, dst ));
    }

    __END__;

    return dst;
}


CX_IMPL CxMatND*
cxGetMatND( const CxArr* arr, CxMatND* matnd, int* coi )
{
    CxMatND* result = 0;
    
    CX_FUNCNAME( "cxGetMat" );

    __BEGIN__;

    if( coi )
        *coi = 0;

    if( !matnd || !arr )
        CX_ERROR( CX_StsNullPtr, "NULL array pointer is passed" );

    if( CX_IS_MATND_HDR(arr))
    {
        if( !((CxMatND*)arr)->data.ptr )
            CX_ERROR( CX_StsNullPtr, "The matrix has NULL data pointer" );
        
        result = (CxMatND*)arr;
    }
    else
    {
        CxMat stub, *mat = (CxMat*)arr;
        
        if( CX_IS_IMAGE_HDR( mat ))
            CX_CALL( mat = cxGetMat( mat, &stub, coi ));

        if( !CX_IS_MAT_HDR( mat ))
            CX_ERROR( CX_StsBadArg, "Unrecognized or unsupported array type" );
        
        if( !mat->data.ptr )
            CX_ERROR( CX_StsNullPtr, "Input array has NULL data pointer" );

        matnd->data.ptr = mat->data.ptr;
        matnd->refcount = 0;
        matnd->type = mat->type;
        matnd->dims = 2;
        matnd->dim[0].size = mat->rows;
        matnd->dim[0].step = mat->step;
        matnd->dim[1].size = mat->cols;
        matnd->dim[1].step = icxPixSize[CX_MAT_TYPE(mat->type)];
        result = matnd;
    }

    __END__;

    return result;
}


// returns number of dimensions to iterate.
/*
Checks whether <count> arrays have equal type, sizes (mask is optional array
that needs to have the same size, but 8uC1 or 8sC1 type).
Returns number of dimensions to iterate through
(0 means that all arrays are continuous (total number of elements
is returned, 1 means that all arrays are vectors of continuous arrays etc.)
and the size of largest common continuous part of the arrays 
*/
CX_IMPL int
cxInitNArrayIterator( int count, CxArr** arrs,
                      const CxArr* mask, CxMatND* stubs,
                      CxNArrayIterator* iterator, int flags )
{
    int dims = -1;

    CX_FUNCNAME( "cxInitArrayOp" );
    
    __BEGIN__;

    int i, j, size, dim0 = -1;
    int64 step;
    CxMatND* hdr0 = 0;

    if( count < 1 || count > CX_MAX_ARR )
        CX_ERROR( CX_StsOutOfRange, "Incorrect number of arrays" );

    if( !arrs || !stubs )
        CX_ERROR( CX_StsNullPtr, "Some of required array pointers is NULL" );

    if( !iterator )
        CX_ERROR( CX_StsNullPtr, "Iterator pointer is NULL" );

    for( i = 0; i <= count; i++ )
    {
        const CxArr* arr = i < count ? arrs[i] : mask;
        CxMatND* hdr;
        
        if( !arr )
        {
            if( i < count )
                CX_ERROR( CX_StsNullPtr, "Some of required array pointers is NULL" );
            break;
        }

        if( CX_IS_MATND( arr ))
            hdr = (CxMatND*)arr;
        else
        {
            int coi = 0;
            CX_CALL( hdr = cxGetMatND( arr, stubs + i, &coi ));
            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "COI set is not allowed here" );
        }

        iterator->hdr[i] = hdr;

        if( i > 0 )
        {
            if( hdr->dims != hdr0->dims )
                CX_ERROR( CX_StsUnmatchedSizes,
                          "Number of dimensions is the same for all arrays" );
            
            if( i < count )
            {
                switch( flags & (CX_NO_DEPTH_CHECK|CX_NO_CN_CHECK))
                {
                case 0:
                    if( !CX_ARE_TYPES_EQ( hdr, hdr0 ))
                        CX_ERROR( CX_StsUnmatchedFormats,
                                  "Data type is not the same for all arrays" );
                    break;
                case CX_NO_DEPTH_CHECK:
                    if( !CX_ARE_CNS_EQ( hdr, hdr0 ))
                        CX_ERROR( CX_StsUnmatchedFormats,
                                  "Number of channels is not the same for all arrays" );
                    break;
                case CX_NO_CN_CHECK:
                    if( !CX_ARE_CNS_EQ( hdr, hdr0 ))
                        CX_ERROR( CX_StsUnmatchedFormats,
                                  "Depth is not the same for all arrays" );
                    break;
                }
            }
            else
            {
                if( !CX_IS_MASK_ARR( hdr ))
                    CX_ERROR( CX_StsBadMask, "Mask should have 8uC1 or 8sC1 data type" );
            }

            if( !(flags & CX_NO_SIZE_CHECK) )
            {
                for( j = 0; j < hdr->dims; j++ )
                    if( hdr->dim[j].size != hdr0->dim[j].size )
                        CX_ERROR( CX_StsUnmatchedSizes,
                                  "Dimension sizes are the same for all arrays" );
            }
        }
        else
            hdr0 = hdr;

        step = icxPixSize[CX_MAT_TYPE(hdr->type)];
        for( j = hdr->dims - 1; j > dim0; j-- )
        {
            if( step != hdr->dim[j].step )
                break;
            step *= hdr->dim[j].size;
        }

        if( j == dim0 && step > INT_MAX )
            j++;

        if( j > dim0 )
            dim0 = j;

        iterator->hdr[i] = (CxMatND*)hdr;
        iterator->ptr[i] = (uchar*)hdr->data.ptr;
    }

    size = 1;
    for( j = hdr0->dims - 1; j > dim0; j-- )
        size *= hdr0->dim[j].size;

    dims = dim0 + 1;
    iterator->dims = dims;
    iterator->count = count;
    iterator->size = cxSize(size,1);

    for( i = 0; i < dims; i++ )
        iterator->stack[i] = hdr0->dim[i].size;

    __END__;

    return dims;
}


// returns zero value if iteration is finished, non-zero otherwise
CX_IMPL  int  cxNextNArraySlice( CxNArrayIterator* iterator )
{
    assert( iterator != 0 );
    int i, dims, size = 0;

    for( dims = iterator->dims; dims > 0; dims-- )
    {
        for( i = 0; i < iterator->count; i++ )
            iterator->ptr[i] += iterator->hdr[i]->dim[dims-1].step;

        if( --iterator->stack[dims-1] > 0 )
            break;

        size = iterator->hdr[0]->dim[dims-1].size;

        for( i = 0; i < iterator->count; i++ )
            iterator->ptr[i] -= (size_t)size*iterator->hdr[i]->dim[dims-1].step;

        iterator->stack[dims-1] = size;
    }

    return dims > 0;
}


/****************************************************************************************\
*                            CxSparseMat creation and basic operations                   *
\****************************************************************************************/


// Creates CxMatND and underlying data
CX_IMPL CxSparseMat*
cxCreateSparseMat( int dims, const int* sizes, int type )
{
    CxSparseMat* arr = 0;

    CX_FUNCNAME( "cxCreateSparseMat" );
    
    __BEGIN__;

    type = CX_MAT_TYPE( type );
    int pix_size = icxPixSize[type];
    int pix_size1 = icxPixSize[type & -CX_MAT_CN_MASK];
    int i, size;
    CxMemStorage* storage;

    if( pix_size == 0 )
        CX_ERROR( CX_StsUnsupportedFormat, "invalid array data type" );

    if( (unsigned)dims > CX_MAX_DIM )
        CX_ERROR( CX_StsOutOfRange, "bad number of dimensions" );

    if( !sizes )
        CX_ERROR( CX_StsNullPtr, "NULL <sizes> pointer" );

    for( i = 0; i < dims; i++ )
    {
        if( sizes[i] <= 0 )
            CX_ERROR( CX_StsBadSize, "one of dimesion sizes is non-positive" );
    }

    CX_CALL( arr = (CxSparseMat*)cxAlloc(sizeof(*arr)));

    arr->type = CX_SPARSE_MAT_MAGIC_VAL | type;
    arr->dims = dims;
    arr->refcount = 0;
    arr->total = 0;
    memcpy( arr->size, sizes, dims*sizeof(sizes[0]));

    arr->valoffset = (sizeof(void*) + sizeof(int) + pix_size1 - 1) & -pix_size1;
    arr->idxoffset = (arr->valoffset + pix_size + sizeof(int) - 1) & -(int)sizeof(int);
    size = (arr->idxoffset + dims*sizeof(int) + sizeof(CxSetElem) - 1) &
           -(int)sizeof(CxSetElem);

    CX_CALL( storage = cxCreateMemStorage( CX_SPARSE_MAT_BLOCK ));
    CX_CALL( arr->heap = cxCreateSet( 0, sizeof(CxSet), size, storage ));

    arr->hashsize = CX_SPARSE_HASH_SIZE0;
    size = arr->hashsize*sizeof(arr->hashtable[0]);
    
    CX_CALL( arr->hashtable = (void**)cxAlloc( size ));
    memset( arr->hashtable, 0, size );

    __END__;

    if( cxGetErrStatus() < 0 )
        cxReleaseSparseMat( &arr );

    return arr;
}


// Creates CxMatND and underlying data
CX_IMPL void
cxReleaseSparseMat( CxSparseMat** array )
{
    CX_FUNCNAME( "cxReleaseSparseMat" );
    
    __BEGIN__;

    if( !array )
        CX_ERROR_FROM_CODE( CX_HeaderIsNull );

    if( *array )
    {
        CxSparseMat* arr = *array;
        
        if( !CX_IS_SPARSE_MAT_HDR(arr) )
            CX_ERROR_FROM_CODE( CX_StsBadFlag );

        *array = 0;

        cxReleaseMemStorage( &arr->heap->storage );
        cxFree( (void**)(&arr->hashtable) );
        cxFree( (void**)&arr );
    }

    __END__;
}


// Creates CxMatND and underlying data
CX_IMPL CxSparseMat*
cxCloneSparseMat( const CxSparseMat* src )
{
    CxSparseMat* dst = 0;
    
    CX_FUNCNAME( "cxCloneSparseMat" );
    
    __BEGIN__;

    if( !CX_IS_SPARSE_MAT_HDR(src) )
        CX_ERROR( CX_StsBadArg, "Invalid sparse array header" );

    CX_CALL( dst = cxCreateSparseMat( src->dims, src->size, src->type ));
    CX_CALL( cxCopy( src, dst )); 

    __END__;

    if( cxGetErrStatus() < 0 )
        cxReleaseSparseMat( &dst );
    
    return dst;
}


CxSparseNode*
cxInitSparseMatIterator( const CxSparseMat* mat, CxSparseMatIterator* iterator )
{
    CxSparseNode* node = 0;
    
    CX_FUNCNAME( "cxInitSparseMatIterator" );

    __BEGIN__;

    int idx;

    if( !CX_IS_SPARSE_MAT( mat ))
        CX_ERROR( CX_StsBadArg, "Invalid sparse matrix header" );

    if( !iterator )
        CX_ERROR( CX_StsNullPtr, "NULL iterator pointer" );

    iterator->mat = (CxSparseMat*)mat;
    iterator->node = 0;

    for( idx = 0; idx < mat->hashsize; idx++ )
        if( mat->hashtable[idx] )
        {
            node = iterator->node = (CxSparseNode*)mat->hashtable[idx];
            break;
        }

    iterator->curidx = idx;

    __END__;

    return node;
}

#define ICX_SPARSE_MAT_HASH_MULTIPLIER  33

/*CXCOREAPI*/ uchar*
icxGetNodePtr( CxSparseMat* mat, int* idx, int* _type,
               int create_node, unsigned* precalc_hashval )
{
    uchar* ptr = 0;
    
    CX_FUNCNAME( "icxGetNodePtr" );

    __BEGIN__;

    int i, tabidx;
    unsigned hashval = 0;
    CxSparseNode *node, *prev = 0;
    assert( CX_IS_SPARSE_MAT( mat ));

    if( !precalc_hashval )
    {
        for( i = 0; i < mat->dims; i++ )
        {
            int t = idx[i];
            if( (unsigned)t >= (unsigned)mat->size[i] )
                CX_ERROR( CX_StsOutOfRange, "One of indices is out of range" );
            hashval = hashval*ICX_SPARSE_MAT_HASH_MULTIPLIER + t;
        }
    }
    else
    {
        hashval = *precalc_hashval;
    }

    tabidx = hashval & (mat->hashsize - 1);
    hashval &= INT_MAX;

    for( node = (CxSparseNode*)mat->hashtable[tabidx];
         node != 0; prev = node, node = node->next )
    {
        if( node->hashval == hashval )
        {
            int* nodeidx = CX_NODE_IDX(mat,node);
            for( i = 0; i < mat->dims; i++ )
                if( idx[i] != nodeidx[i] )
                    break;
            if( i == mat->dims )
            {
                ptr = (uchar*)CX_NODE_VAL(mat,node);
                break;
            }
        }
    }

    if( ptr )
    {
        /*if( prev )
        {
            prev->next = node->next;
            node->next = (CxSparseNode*)mat->hashtable[tabidx];
            mat->hashtable[tabidx] = node;
        }*/
    }
    else if( create_node )
    {
        CxSparseNode* node;
        
        if( mat->total >= mat->hashsize*CX_SPARSE_HASH_RATIO )
        {
            void** newtable;
            int newsize = CX_MAX( mat->hashsize*2, CX_SPARSE_HASH_SIZE0);
            int newrawsize = newsize*sizeof(newtable[0]);
            
            CxSparseMatIterator iterator;
            assert( (newsize & (newsize - 1)) == 0 );

            // resize hash table
            CX_CALL( newtable = (void**)cxAlloc( newrawsize ));
            memset( newtable, 0, newrawsize );

            node = cxInitSparseMatIterator( mat, &iterator );
            while( node )
            {
                CxSparseNode* next = cxGetNextSparseNode( &iterator );
                int newidx = node->hashval & (newsize - 1);
                node->next = (CxSparseNode*)newtable[newidx];
                newtable[newidx] = node;
                node = next;
            }

            CX_CALL( cxFree( (void**)&mat->hashtable ));
            mat->hashtable = newtable;
            mat->hashsize = newsize;
            tabidx = hashval & (newsize - 1);
        }

        node = (CxSparseNode*)cxSetNew( mat->heap );
        node->hashval = hashval;
        node->next = (CxSparseNode*)mat->hashtable[tabidx];
        mat->hashtable[tabidx] = node;
        mat->total++;
        memcpy( CX_NODE_IDX(mat,node), idx, mat->dims*sizeof(idx[0]));
        ptr = (uchar*)CX_NODE_VAL(mat,node);
        if( create_node > 0 )
            memset( ptr, 0, icxPixSize[CX_MAT_TYPE(mat->type)]);
    }

    if( _type )
        *_type = CX_MAT_TYPE(mat->type);

    __END__;

    return ptr;
}


/*CXCOREAPI*/ void
icxDeleteNode( CxSparseMat* mat, int* idx, unsigned* precalc_hashval )
{
    CX_FUNCNAME( "icxDeleteNode" );

    __BEGIN__;

    int i, tabidx;
    unsigned hashval = 0;
    CxSparseNode *node, *prev = 0;
    assert( CX_IS_SPARSE_MAT( mat ));

    if( !precalc_hashval )
    {
        for( i = 0; i < mat->dims; i++ )
        {
            int t = idx[i];
            if( (unsigned)t >= (unsigned)mat->size[i] )
                CX_ERROR( CX_StsOutOfRange, "One of indices is out of range" );
            hashval = hashval*ICX_SPARSE_MAT_HASH_MULTIPLIER + t;
        }
    }
    else
    {
        hashval = *precalc_hashval;
    }

    tabidx = hashval & (mat->hashsize - 1);
    hashval &= INT_MAX;

    for( node = (CxSparseNode*)mat->hashtable[tabidx];
         node != 0; prev = node, node = node->next )
    {
        if( node->hashval == hashval )
        {
            int* nodeidx = CX_NODE_IDX(mat,node);
            for( i = 0; i < mat->dims; i++ )
                if( idx[i] != nodeidx[i] )
                    break;
            if( i == mat->dims )
                break;
        }
    }

    if( node )
    {
        if( prev )
            prev->next = node->next;
        else
            mat->hashtable[tabidx] = node->next;
        mat->total--;
        cxSetRemoveByPtr( mat->heap, node );
    }

    __END__;
}



/****************************************************************************************\
*                          Common for multiple array types operations                    *
\****************************************************************************************/

// Allocates underlying array data
CX_IMPL void
cxCreateData( CxArr* arr )
{
    CX_FUNCNAME( "cxCreateData" );
    
    __BEGIN__;

    if( CX_IS_MAT_HDR( arr ))
    {
        int64 step, total_size;
        CxMat* mat = (CxMat*)arr;
        step = mat->step;

        if( mat->data.ptr != 0 )
            CX_ERROR( CX_StsError, "Data is already allocated" );

        if( step == 0 )
            step = icxPixSize[CX_MAT_TYPE(mat->type)]*mat->cols;

        total_size = step*mat->rows;
        CX_CALL( mat->data.ptr = (uchar*)cxAlloc( (size_t)total_size ));

        mat->refcount = (int*)(mat->data.ptr - sizeof(char*)*2);
        *mat->refcount = 1;
    }
    else if( CX_IS_IMAGE_HDR(arr))
    {
        IplImage* img = (IplImage*)arr;

        if( img->imageData != 0 )
            CX_ERROR( CX_StsError, "Data is already allocated" );

        if( !CxIPL.allocateData )
        {
            CX_CALL( img->imageData = img->imageDataOrigin = 
                        (char*)cxAlloc( (size_t)img->imageSize ));
        }
        else
        {
            int depth = img->depth;
            int width = img->width;

            if( img->depth == IPL_DEPTH_32F || img->nChannels == 64 )
            {
                img->width *= img->depth == IPL_DEPTH_32F ? sizeof(float) : sizeof(double);
                img->depth = IPL_DEPTH_8U;
            }

            CxIPL.allocateData( img, 0, 0 );

            img->width = width;
            img->depth = depth;
        }
    }
    else if( CX_IS_MATND_HDR( arr ))
    {
        CxMatND* mat = (CxMatND*)arr;
        int i;
        int64 total_size = icxPixSize[CX_MAT_TYPE(mat->type)];

        if( mat->data.ptr != 0 )
            CX_ERROR( CX_StsError, "Data is already allocated" );

        if( CX_IS_MAT_CONT( mat->type ))
        {
            total_size = (size_t)mat->dim[0].size*(mat->dim[0].step != 0 ?
                         mat->dim[0].step : total_size);
        }
        else
        {
            for( i = mat->dims - 1; i >= 0; i-- )
            {
                int64 size = (int64)mat->dim[i].step*mat->dim[i].size;

                if( total_size < size )
                    total_size = size;
            }
        }
        
        assert( total_size >= 0 );
        CX_CALL( mat->data.ptr = (uchar*)cxAlloc( (size_t)total_size ));

        mat->refcount = (int*)(mat->data.ptr - sizeof(char*)*2);
        *mat->refcount = 1;
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;
}


// Assigns external data to array
CX_IMPL void
cxSetData( CxArr* arr, void* data, int step )
{
    CX_FUNCNAME( "cxSetData" );

    __BEGIN__;

    int pix_size, min_step;

    if( CX_IS_MAT_HDR(arr) || CX_IS_MATND_HDR(arr) )
        cxReleaseData( arr );

    if( data )
    {
        if( CX_IS_MAT_HDR( arr ))
        {
            CxMat* mat = (CxMat*)arr;
        
            int type = CX_MAT_TYPE(mat->type);
            pix_size = icxPixSize[type];
            min_step = mat->cols*pix_size & ((mat->rows <= 1) - 1);

            if( step != CX_AUTOSTEP )
            {
                if( step < min_step && data != 0 )
                    CX_ERROR_FROM_CODE( CX_BadStep );
                mat->step = step & ((mat->rows <= 1) - 1);
            }
            else
            {
                mat->step = min_step;
            }
    
            mat->data.ptr = (uchar*)data;
            mat->type = CX_MAT_MAGIC_VAL | type |
                        (mat->step==min_step ? CX_MAT_CONT_FLAG : 0);
            icxCheckHuge( mat );
        }
        else if( CX_IS_IMAGE_HDR( arr ))
        {
            IplImage* img = (IplImage*)arr;
        
            pix_size = ((img->depth & 255) >> 3)*img->nChannels;
            min_step = img->width*pix_size;

            if( step != CX_AUTOSTEP && img->height > 1 )
            {
                if( step < min_step && data != 0 )
                    CX_ERROR_FROM_CODE( CX_BadStep );
                img->widthStep = step;
            }
            else
            {
                img->widthStep = min_step;
            }

            img->imageSize = img->widthStep * img->height;
            img->imageData = img->imageDataOrigin = (char*)data;

            if( (((int)(long)data | step) & 7) == 0 &&
                cxAlign(img->width * pix_size, 8) == step )
            {
                img->align = 8;
            }
            else
            {
                img->align = 4;
            }
        }
        else if( CX_IS_MATND_HDR( arr ))
        {
            CxMatND* mat = (CxMatND*)arr;
            int i;
            int64 cur_step;
        
            if( step != CX_AUTOSTEP )
                CX_ERROR( CX_BadStep,
                "For multidimensional array only CX_AUTOSTEP is allowed here" );

            mat->data.ptr = (uchar*)data;
            cur_step = icxPixSize[CX_MAT_TYPE(mat->type)];

            for( i = mat->dims - 1; i >= 0; i-- )
            {
                if( cur_step > INT_MAX )
                    CX_ERROR( CX_StsOutOfRange, "The array is too big" );
                mat->dim[i].step = (int)cur_step;
                cur_step *= mat->dim[i].size;
            }
        }
        else
        {
            CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
        }
    }

    __END__;
}


// Deallocates array's data
CX_IMPL void
cxReleaseData( CxArr* arr )
{
    CX_FUNCNAME( "cxReleaseData" );
    
    __BEGIN__;

    if( CX_IS_MAT_HDR( arr ) || CX_IS_MATND_HDR( arr ))
    {
        CxMat* mat = (CxMat*)arr;
        cxDecRefData( mat );
    }
    else if( CX_IS_IMAGE_HDR( arr ))
    {
        IplImage* img = (IplImage*)arr;

        if( !CxIPL.deallocate )
        {
            char* ptr = img->imageData;
            img->imageData = img->imageDataOrigin = 0;
            cxFree( (void**)&ptr );
        }
        else
        {
            CxIPL.deallocate( img, IPL_IMAGE_DATA );
        }
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;
}


// Retrieves essential information about image ROI or CxMat data
CX_IMPL void
cxGetRawData( const CxArr* arr, uchar** data, int* step, CxSize* roi_size )
{
    CX_FUNCNAME( "cxGetRawData" );

    __BEGIN__;

    if( CX_IS_MAT( arr ))
    {
        CxMat *mat = (CxMat*)arr;

        if( step )
            *step = mat->step;

        if( data )
            *data = mat->data.ptr;

        if( roi_size )
            *roi_size = cxGetMatSize( mat );
    }
    else if( CX_IS_IMAGE( arr ))
    {
        IplImage* img = (IplImage*)arr;

        if( step )
            *step = img->widthStep;

        if( data )
            CX_CALL( *data = cxPtr2D( img, 0, 0 ));

        if( roi_size )
        {
            if( img->roi )
            {
                *roi_size = cxSize( img->roi->width, img->roi->height );
            }
            else
            {
                *roi_size = cxSize( img->width, img->height );
            }
        }
    }
    else if( CX_IS_MATND( arr ))
    {
        CxMatND* mat = (CxMatND*)arr;

        if( !CX_IS_MAT_CONT( mat->type ))
            CX_ERROR( CX_StsBadArg, "Only continuous nD arrays are supported here" );

        if( data )
            *data = mat->data.ptr;

        if( roi_size || step )
        {
            int i, size1 = mat->dim[0].size, size2 = 1;

            if( mat->dims > 2 )
                for( i = 1; i < mat->dims; i++ )
                    size1 *= mat->dim[i].size;
            else
                size2 = mat->dim[1].size;

            if( roi_size )
            {
                roi_size->width = size2;
                roi_size->height = size1;
            }

            if( step )
                *step = size1 == 1 ? 0 : mat->dim[0].step;
        }
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;
}


CX_IMPL int
cxGetElemType( const CxArr* arr )
{
    int type = -1;

    CX_FUNCNAME( "cxGetElemType" );

    __BEGIN__;

    if( CX_IS_MAT_HDR(arr) || CX_IS_MATND_HDR(arr) || CX_IS_SPARSE_MAT_HDR(arr))
    {
        type = CX_MAT_TYPE( ((CxMat*)arr)->type );
    }
    else if( CX_IS_IMAGE(arr))
    {
        IplImage* img = (IplImage*)arr;
        
        if( img->nChannels > 4 )
            CX_ERROR( CX_BadNumChannels, "The image has more than 4 channels" );

        type = icxIplToCxDepth(img->depth) + (img->nChannels - 1)*8;
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return type;
}


// Returns a number of array dimensions
CX_IMPL int
cxGetDims( const CxArr* arr, int* sizes )
{
    int dims = -1;
    CX_FUNCNAME( "cxGetDims" );

    __BEGIN__;

    if( CX_IS_MAT_HDR( arr ))
    {
        CxMat* mat = (CxMat*)arr;
        
        dims = 2;
        if( sizes )
        {
            sizes[0] = mat->rows;
            sizes[1] = mat->cols;
        }
    }
    else if( CX_IS_IMAGE( arr ))
    {
        IplImage* img = (IplImage*)arr;
        dims = 2;

        if( sizes )
        {
            sizes[0] = img->height;
            sizes[1] = img->width;
        }
    }
    else if( CX_IS_MATND_HDR( arr ))
    {
        CxMatND* mat = (CxMatND*)arr;
        dims = mat->dims;
        
        if( sizes )
        {
            int i;
            for( i = 0; i < dims; i++ )
                sizes[i] = mat->dim[i].size;
        }
    }
    else if( CX_IS_SPARSE_MAT_HDR( arr ))
    {
        CxSparseMat* mat = (CxSparseMat*)arr;
        dims = mat->dims;
        
        if( sizes )
            memcpy( sizes, mat->size, dims*sizeof(sizes[0]));
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return dims;
}


// Returns the size of particular array dimension
CX_IMPL int
cxGetDimSize( const CxArr* arr, int index )
{
    int size = -1;
    CX_FUNCNAME( "cxGetDimSize" );

    __BEGIN__;

    if( CX_IS_MAT( arr ))
    {
        CxMat *mat = (CxMat*)arr;

        switch( index )
        {
        case 0:
            size = mat->rows;
            break;
        case 1:
            size = mat->cols;
            break;
        default:
            CX_ERROR( CX_StsOutOfRange, "bad dimension index" );
        }
    }
    else if( CX_IS_IMAGE( arr ))
    {
        IplImage* img = (IplImage*)arr;

        switch( index )
        {
        case 0:
            size = !img->roi ? img->height : img->roi->height;
            break;
        case 1:
            size = !img->roi ? img->width : img->roi->width;
            break;
        default:
            CX_ERROR( CX_StsOutOfRange, "bad dimension index" );
        }
    }
    else if( CX_IS_MATND_HDR( arr ))
    {
        CxMatND* mat = (CxMatND*)arr;
        
        if( (unsigned)index >= (unsigned)mat->dims )
            CX_ERROR( CX_StsOutOfRange, "bad dimension index" );

        size = mat->dim[index].size;
    }
    if( CX_IS_SPARSE_MAT_HDR( arr ))
    {
        CxSparseMat* mat = (CxSparseMat*)arr;
        
        if( (unsigned)index >= (unsigned)mat->dims )
            CX_ERROR( CX_StsOutOfRange, "bad dimension index" );

        size = mat->size[index];
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return size;
}


// Returns the size of CxMat or IplImage
CX_IMPL CxSize
cxGetSize( const CxArr* arr )
{
    CxSize size = { 0, 0 };

    CX_FUNCNAME( "cxGetSize" );

    __BEGIN__;

    if( CX_IS_MAT_HDR( arr ))
    {
        CxMat *mat = (CxMat*)arr;

        size.width = mat->cols;
        size.height = mat->rows;
    }
    else if( CX_IS_IMAGE_HDR( arr ))
    {
        IplImage* img = (IplImage*)arr;

        if( img->roi )
        {
            size.width = img->roi->width;
            size.height = img->roi->height;
        }
        else
        {
            size.width = img->width;
            size.height = img->height;
        }
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "Array should be CxMat or IplImage" );
    }

    __END__;

    return size;
}


// Selects sub-array (no data is copied)
CX_IMPL  CxMat*
cxGetSubRect( const CxArr* arr, CxMat* submat, CxRect rect )
{
    CxMat* res = 0;
    
    CX_FUNCNAME( "cxGetRect" );

    __BEGIN__;

    CxMat stub, *mat = (CxMat*)arr;

    if( !CX_IS_MAT( mat ))
        CX_CALL( mat = cxGetMat( mat, &stub ));

    if( !submat )
        CX_ERROR( CX_StsNullPtr, "" );

    if( (rect.x|rect.y|rect.width|rect.height) < 0 )
        CX_ERROR( CX_StsBadSize, "" );

    if( rect.x + rect.width > mat->cols ||
        rect.y + rect.height > mat->rows )
        CX_ERROR( CX_StsBadSize, "" );

    {
    /*
    int* refcount = mat->refcount;

    if( refcount )
        ++*refcount;

    cxDecRefData( submat );
    */
    submat->rows = rect.height;
    submat->cols = rect.width;
    submat->step = mat->step & (submat->rows > 1 ? -1 : 0);
    submat->data.ptr = mat->data.ptr + (size_t)rect.y*mat->step +
                       rect.x*icxPixSize[CX_MAT_TYPE(mat->type)];
    submat->type = (mat->type & (submat->cols < mat->cols ? ~CX_MAT_CONT_FLAG : -1)) |
                   (submat->step == 0 ? CX_MAT_CONT_FLAG : 0);
    submat->refcount = 0;
    res = submat;
    }
    
    __END__;

    return res;
}


// Selects array's row span.
CX_IMPL  CxMat*
cxGetRows( const CxArr* arr, CxMat* submat,
           int start_row, int end_row, int delta_row )
{
    CxMat* res = 0;
    
    CX_FUNCNAME( "cxGetRows" );

    __BEGIN__;

    CxMat stub, *mat = (CxMat*)arr;

    if( !CX_IS_MAT( mat ))
        CX_CALL( mat = cxGetMat( mat, &stub ));

    if( !submat )
        CX_ERROR( CX_StsNullPtr, "" );

    if( (unsigned)start_row >= (unsigned)mat->rows ||
        (unsigned)end_row > (unsigned)mat->rows || delta_row <= 0 )
        CX_ERROR( CX_StsOutOfRange, "" );

    {
    /*
    int* refcount = mat->refcount;

    if( refcount )
        ++*refcount;

    cxDecRefData( submat );
    */
    if( delta_row == 1 )
    {
        submat->rows = end_row - start_row;
        submat->step = mat->step & (submat->rows > 1 ? -1 : 0);
    }
    else
    {
        submat->rows = (end_row - start_row + delta_row - 1)/delta_row;
        submat->step = mat->step * delta_row;
    }

    submat->cols = mat->cols;
    submat->step &= submat->rows > 1 ? -1 : 0;
    submat->data.ptr = mat->data.ptr + (size_t)start_row*mat->step;
    submat->type = (mat->type | (submat->step == 0 ? CX_MAT_CONT_FLAG : 0)) &
                   (delta_row != 1 ? ~CX_MAT_CONT_FLAG : -1);
    submat->refcount = 0;
    res = submat;
    }
    
    __END__;

    return res;
}


// Selects array's column span.
CX_IMPL  CxMat*
cxGetCols( const CxArr* arr, CxMat* submat, int start_col, int end_col )
{
    CxMat* res = 0;
    
    CX_FUNCNAME( "cxGetCols" );

    __BEGIN__;

    CxMat stub, *mat = (CxMat*)arr;

    if( !CX_IS_MAT( mat ))
        CX_CALL( mat = cxGetMat( mat, &stub ));

    if( !submat )
        CX_ERROR( CX_StsNullPtr, "" );

    if( (unsigned)start_col >= (unsigned)mat->cols ||
        (unsigned)end_col > (unsigned)mat->cols )
        CX_ERROR( CX_StsOutOfRange, "" );

    {
    /*
    int* refcount = mat->refcount;

    if( refcount )
        ++*refcount;

    cxDecRefData( submat );
    */
    submat->rows = mat->rows;
    submat->cols = end_col - start_col;
    submat->step = mat->step & (submat->rows > 1 ? -1 : 0);
    submat->data.ptr = mat->data.ptr + (size_t)start_col*icxPixSize[CX_MAT_TYPE(mat->type)];
    submat->type = mat->type & (submat->step && submat->cols < mat->cols ?
                                ~CX_MAT_CONT_FLAG : -1);
    submat->refcount = 0;
    res = submat;
    }
    
    __END__;

    return res;
}


// Selects array diagonal
CX_IMPL  CxMat*
cxGetDiag( const CxArr* arr, CxMat* submat, int diag )
{
    CxMat* res = 0;
    
    CX_FUNCNAME( "cxGetDiag" );

    __BEGIN__;

    CxMat stub, *mat = (CxMat*)arr;
    int len, pix_size; 

    if( !CX_IS_MAT( mat ))
        CX_CALL( mat = cxGetMat( mat, &stub ));

    if( !submat )
        CX_ERROR( CX_StsNullPtr, "" );

    pix_size = CX_ELEM_SIZE(mat->type);

    /*{
    int* refcount = mat->refcount;

    if( refcount )
        ++*refcount;

    cxDecRefData( submat );
    }*/

    if( diag >= 0 )
    {
        len = mat->cols - diag;
        
        if( len <= 0 )
            CX_ERROR( CX_StsOutOfRange, "" );

        len = CX_IMIN( len, mat->rows );
        submat->data.ptr = mat->data.ptr + diag*pix_size;
    }
    else
    {
        len = mat->rows + diag;
        
        if( len <= 0 )
            CX_ERROR( CX_StsOutOfRange, "" );

        len = CX_IMIN( len, mat->cols );
        submat->data.ptr = mat->data.ptr - diag*mat->step;
    }

    submat->rows = len;
    submat->cols = 1;
    submat->step = (mat->step + pix_size) & (submat->rows > 1 ? -1 : 0);
    submat->type = mat->type & (submat->step ? ~CX_MAT_CONT_FLAG : -1);
    submat->refcount = 0;
    res = submat;
    
    __END__;

    return res;
}


/****************************************************************************************\
*                      Operations on CxScalar and accessing array elements               *
\****************************************************************************************/

// Converts CxScalar to specified type
CX_IMPL void
cxScalarToRawData( const CxScalar* scalar, void* data, int type, int extend_to_12 )
{
    CX_FUNCNAME( "cxScalarToRawData" );

    type = CX_MAT_TYPE(type);
    
    __BEGIN__;

    int cn = CX_MAT_CN( type );
    int depth = type & CX_MAT_DEPTH_MASK;

    assert( scalar && data );
    assert( (unsigned)(cn - 1) < 4 );

    switch( depth )
    {
    case CX_8UC1:
        while( cn-- )
        {
            int t = cxRound( scalar->val[cn] );
            ((uchar*)data)[cn] = CX_CAST_8U(t);
        }
        break;
    case CX_8SC1:
        while( cn-- )
        {
            int t = cxRound( scalar->val[cn] );
            ((char*)data)[cn] = CX_CAST_8S(t);
        }
        break;
    case CX_16SC1:
        while( cn-- )
        {
            int t = cxRound( scalar->val[cn] );
            ((short*)data)[cn] = CX_CAST_16S(t);
        }
        break;
    case CX_32SC1:
        while( cn-- )
            ((int*)data)[cn] = cxRound( scalar->val[cn] );
        break;
    case CX_32FC1:
        while( cn-- )
            ((float*)data)[cn] = (float)(scalar->val[cn]);
        break;
    case CX_64FC1:
        while( cn-- )
            ((double*)data)[cn] = (double)(scalar->val[cn]);
        break;
    default:
        assert(0);
        CX_ERROR_FROM_CODE( CX_BadDepth );
    }

    if( extend_to_12 )
    {
        int pix_size = icxPixSize[type];
        int offset = icxPixSize[depth]*12;

        do
        {
            offset -= pix_size;
            memcpy( (char*)data + offset, data, pix_size );
        }
        while( offset > pix_size );
    }

    __END__;
}


// Converts data of specified type to CxScalar
CX_IMPL void
cxRawDataToScalar( const void* data, int flags, CxScalar* scalar )
{
    CX_FUNCNAME( "cxRawDataToScalar" );
    
    __BEGIN__;

    int cn = CX_MAT_CN( flags );

    assert( scalar && data );
    assert( (unsigned)(cn - 1) < 4 );

    memset( scalar->val, 0, sizeof(scalar->val));

    switch( CX_MAT_DEPTH( flags ))
    {
    case CX_8U:
        while( cn-- )
            scalar->val[cn] = CX_8TO32F(((uchar*)data)[cn]);
        break;
    case CX_8S:
        while( cn-- )
            scalar->val[cn] = CX_8TO32F(((char*)data)[cn]);
        break;
    case CX_16S:
        while( cn-- )
            scalar->val[cn] = ((short*)data)[cn];
        break;
    case CX_32S:
        while( cn-- )
            scalar->val[cn] = ((int*)data)[cn];
        break;
    case CX_32F:
        while( cn-- )
            scalar->val[cn] = ((float*)data)[cn];
        break;
    case CX_64F:
        while( cn-- )
            scalar->val[cn] = ((double*)data)[cn];
        break;
    default:
        assert(0);
        CX_ERROR_FROM_CODE( CX_BadDepth );
    }

    __END__;
}


static double icxGetReal( const void* data, int type )
{
    switch( type )
    {
    case CX_8U:
        return *(uchar*)data;
    case CX_8S:
        return *(char*)data;
    case CX_16S:
        return *(short*)data;
    case CX_32S:
        return *(int*)data;
    case CX_32F:
        return *(float*)data;
    case CX_64F:
        return *(double*)data;
    }

    return 0;
}


static void icxSetReal( double value, const void* data, int type )
{
    if( type < CX_32F )
    {
        int ivalue = cxRound(value);
        switch( type )
        {
        case CX_8U:
            *(uchar*)data = CX_CAST_8U(ivalue);
            break;
        case CX_8S:
            *(char*)data = CX_CAST_8S(ivalue);
            break;
        case CX_16S:
            *(short*)data = CX_CAST_16S(ivalue);
            break;
        case CX_32S:
            *(int*)data = CX_CAST_32S(ivalue);
            break;
        }
    }
    else
    {
        switch( type )
        {
        case CX_32F:
            *(float*)data = (float)value;
            break;
        case CX_64F:
            *(double*)data = value;
            break;
        }
    }
}


// Returns pointer to specified element of array (linear index is used)
CX_IMPL  uchar*
cxPtr1D( const CxArr* arr, int idx, int* _type )
{
    uchar* ptr = 0;
    
    CX_FUNCNAME( "cxPtr1D" );

    __BEGIN__;

    if( CX_IS_MAT( arr ))
    {
        CxMat* mat = (CxMat*)arr;

        int type = CX_MAT_TYPE(mat->type);
        int pix_size = icxPixSize[type];

        if( _type )
            *_type = type;
        
        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        if( CX_IS_MAT_CONT(mat->type))
        {
            ptr = mat->data.ptr + (size_t)idx*pix_size;
        }
        else
        {
            int row = idx/mat->cols;
            ptr = mat->data.ptr + (size_t)row*mat->step + (idx - row*mat->cols)*pix_size;
        }
    }
    else if( CX_IS_IMAGE_HDR( arr ))
    {
        IplImage* img = (IplImage*)arr;
        int width = !img->roi ? img->width : img->roi->width;
        int y = idx/width, x = idx - y*width;

        ptr = cxPtr2D( arr, y, x, _type );
    }
    else if( CX_IS_MATND( arr ))
    {
        CxMatND* mat = (CxMatND*)arr;
        int j, type = CX_MAT_TYPE(mat->type);
        size_t size = mat->dim[0].size;

        if( _type )
            *_type = type;

        for( j = 1; j < mat->dims; j++ )
            size *= mat->dim[j].size;

        if((unsigned)idx >= (unsigned)size )
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        if( CX_IS_MAT_CONT(mat->type))
        {
            int pix_size = icxPixSize[type];
            ptr = mat->data.ptr + (size_t)idx*pix_size;
        }
        else
        {
            ptr = mat->data.ptr;
            for( j = mat->dims - 1; j >= 0; j-- )
            {
                int sz = mat->dim[j].size;
                if( sz )
                {
                    int t = idx/sz;
                    ptr += (idx - t*sz)*mat->dim[j].step;
                    idx = t;
                }
            }
        }
    }
    else if( CX_IS_SPARSE_MAT( arr ))
    {
        ptr = icxGetNodePtr( (CxSparseMat*)arr, &idx, _type, 1, 0 );
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return ptr;
}


// Returns pointer to specified element of 2d array
CX_IMPL  uchar*
cxPtr2D( const CxArr* arr, int y, int x, int* _type )
{
    uchar* ptr = 0;
    
    CX_FUNCNAME( "cxPtr2D" );

    __BEGIN__;

    if( CX_IS_MAT( arr ))
    {
        CxMat* mat = (CxMat*)arr;
        int type;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        type = CX_MAT_TYPE(mat->type);
        if( _type )
            *_type = type;

        ptr = mat->data.ptr + (size_t)y*mat->step + x*icxPixSize[type];
    }
    else if( CX_IS_IMAGE( arr ))
    {
        IplImage* img = (IplImage*)arr;
        int pix_size = (img->depth & 255) >> 3;
        int width, height;
        ptr = (uchar*)img->imageData;

        if( img->dataOrder == 0 )
            pix_size *= img->nChannels;

        if( img->roi )
        {
            width = img->roi->width;
            height = img->roi->height;

            ptr += img->roi->yOffset*img->widthStep +
                   img->roi->xOffset*pix_size;

            if( img->dataOrder )
            {
                int coi = img->roi->coi;
                if( !coi )
                    CX_ERROR( CX_BadCOI,
                        "COI must be non-null in case of planar images" );
                ptr += (coi - 1)*img->imageSize;
            }
        }
        else
        {
            width = img->width;
            height = img->height;
        }

        if( (unsigned)y >= (unsigned)height ||
            (unsigned)x >= (unsigned)width )
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        ptr += y*img->widthStep + x*pix_size;

        if( _type )
        {
            int type = icxIplToCxDepth(img->depth);
            if( type < 0 || (unsigned)(img->nChannels - 1) > 3 )
                CX_ERROR( CX_StsUnsupportedFormat, "" );

            *_type = type + (img->nChannels - 1)*8;
        }
    }
    else if( CX_IS_MATND( arr ))
    {
        CxMatND* mat = (CxMatND*)arr;

        if( mat->dims != 2 || 
            (unsigned)y >= (unsigned)(mat->dim[0].size) ||
            (unsigned)x >= (unsigned)(mat->dim[1].size) )
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)y*mat->dim[0].step + x*mat->dim[1].step;
        if( _type )
            *_type = CX_MAT_TYPE(mat->type);
    }
    else if( CX_IS_SPARSE_MAT( arr ))
    {
        int idx[] = { y, x };
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, _type, 1, 0 );
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return ptr;
}


// Returns pointer to specified element of 3d array
CX_IMPL  uchar*
cxPtr3D( const CxArr* arr, int z, int y, int x, int* _type )
{
    uchar* ptr = 0;
    
    CX_FUNCNAME( "cxPtr3D" );

    __BEGIN__;

    if( CX_IS_MATND( arr ))
    {
        CxMatND* mat = (CxMatND*)arr;

        if( mat->dims != 3 || 
            (unsigned)z >= (unsigned)(mat->dim[0].size) ||
            (unsigned)y >= (unsigned)(mat->dim[1].size) ||
            (unsigned)x >= (unsigned)(mat->dim[2].size) )
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)z*mat->dim[0].step +
              (size_t)y*mat->dim[1].step + x*mat->dim[2].step;

        if( _type )
            *_type = CX_MAT_TYPE(mat->type);
    }
    else if( CX_IS_SPARSE_MAT( arr ))
    {
        int idx[] = { z, y, x };
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, _type, 1, 0 );
    }
    else
    {
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return ptr;
}


// Returns pointer to specified element of n-d array
CX_IMPL  uchar*
cxPtrND( const CxArr* arr, int* idx, int* _type,
         int create_node, unsigned* precalc_hashval )
{
    uchar* ptr = 0;
    CX_FUNCNAME( "cxPtrND" );

    __BEGIN__;

    if( !idx )
        CX_ERROR( CX_StsNullPtr, "NULL pointer to indices" );

    if( CX_IS_SPARSE_MAT( arr ))
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, 
                             _type, create_node, precalc_hashval );
    else if( CX_IS_MATND( arr ))
    {
        CxMatND* mat = (CxMatND*)arr;
        int i;
        ptr = mat->data.ptr;

        for( i = 0; i < mat->dims; i++ )
        {
            if( (unsigned)idx[i] >= (unsigned)(mat->dim[i].size) )
                CX_ERROR( CX_StsOutOfRange, "index is out of range" );
            ptr += (size_t)idx[i]*mat->dim[i].step;
        }

        if( _type )
            *_type = CX_MAT_TYPE(mat->type);
    }
    else if( CX_IS_MAT_HDR(arr) || CX_IS_IMAGE_HDR(arr) )
        ptr = cxPtr2D( arr, idx[0], idx[1], _type );
    else
        CX_ERROR( CX_StsBadArg, "unrecognized or unsupported array type" );

    __END__;

    return ptr;
}


// Returns specifed element of n-D array given linear index
CX_IMPL  CxScalar
cxGet1D( const CxArr* arr, int idx )
{
    CxScalar scalar = {{ 0, 0, 0, 0 }};

    CX_FUNCNAME( "cxGet1D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CX_IS_MAT( arr ) && CX_IS_MAT_CONT( ((CxMat*)arr)->type ))
    {
        CxMat* mat = (CxMat*)arr;

        type = CX_MAT_TYPE(mat->type);
        int pix_size = icxPixSize[type];

        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)idx*pix_size;
    }
    else if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr1D( arr, idx, &type );
    else
        ptr = icxGetNodePtr( (CxSparseMat*)arr, &idx, &type, 0, 0 );

    cxRawDataToScalar( ptr, type, &scalar );

    __END__;

    return scalar;
}


// Returns specifed element of 2D array
CX_IMPL  CxScalar
cxGet2D( const CxArr* arr, int y, int x )
{
    CxScalar scalar = {{ 0, 0, 0, 0 }};

    CX_FUNCNAME( "cxGet2D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( CX_IS_MAT( arr ))
    {
        CxMat* mat = (CxMat*)arr;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        type = CX_MAT_TYPE(mat->type);
        ptr = mat->data.ptr + (size_t)y*mat->step + x*icxPixSize[type];
    }
    else if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr2D( arr, y, x, &type );
    else
    {
        int idx[] = { y, x };
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, 0, 0 );
    }

    cxRawDataToScalar( ptr, type, &scalar );

    __END__;

    return scalar;
}


// Returns specifed element of 3D array
CX_IMPL  CxScalar
cxGet3D( const CxArr* arr, int z, int y, int x )
{
    CxScalar scalar = {{ 0, 0, 0, 0 }};

    /*CX_FUNCNAME( "cxGet3D" );*/

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr3D( arr, z, y, x, &type );
    else
    {
        int idx[] = { z, y, x };
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, 0, 0 );
    }

    cxRawDataToScalar( ptr, type, &scalar );

    __END__;

    return scalar;
}


// Returns specifed element of nD array
CX_IMPL  CxScalar
cxGetND( const CxArr* arr, int* idx )
{
    CxScalar scalar = {{ 0, 0, 0, 0 }};

    /*CX_FUNCNAME( "cxGetND" );*/

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtrND( arr, idx, &type );
    else
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, 0, 0 );

    cxRawDataToScalar( ptr, type, &scalar );

    __END__;

    return scalar;
}


// Returns specifed element of n-D array given linear index
CX_IMPL  double
cxGetReal1D( const CxArr* arr, int idx )
{
    double value = 0;

    CX_FUNCNAME( "cxGetReal1D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( CX_IS_MAT( arr ) && CX_IS_MAT_CONT( ((CxMat*)arr)->type ))
    {
        CxMat* mat = (CxMat*)arr;

        type = CX_MAT_TYPE(mat->type);
        int pix_size = icxPixSize[type];

        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)idx*pix_size;
    }
    else if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr1D( arr, idx, &type );
    else
        ptr = icxGetNodePtr( (CxSparseMat*)arr, &idx, &type, 0, 0 );

    if( ptr )
    {
        if( CX_MAT_CN( type ) > 1 )
            CX_ERROR( CX_BadNumChannels, "cxGetReal* support only single-channel arrays" );

        value = icxGetReal( ptr, type );
    }

    __END__;

    return value;
}


// Returns specifed element of 2D array
CX_IMPL  double
cxGetReal2D( const CxArr* arr, int y, int x )
{
    double value = 0;

    CX_FUNCNAME( "cxGetReal2D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CX_IS_MAT( arr ))
    {
        CxMat* mat = (CxMat*)arr;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        type = CX_MAT_TYPE(mat->type);
        ptr = mat->data.ptr + (size_t)y*mat->step + x*icxPixSize[type];
    }
    else if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr2D( arr, y, x, &type );
    else
    {
        int idx[] = { y, x };
        
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, 0, 0 );
    }

    if( ptr )
    {
        if( CX_MAT_CN( type ) > 1 )
            CX_ERROR( CX_BadNumChannels, "cxGetReal* support only single-channel arrays" );

        value = icxGetReal( ptr, type );
    }

    __END__;

    return value;
}


// Returns specifed element of 3D array
CX_IMPL  double
cxGetReal3D( const CxArr* arr, int z, int y, int x )
{
    double value = 0;

    CX_FUNCNAME( "cxGetReal3D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr3D( arr, z, y, x, &type );
    else
    {
        int idx[] = { z, y, x };
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, 0, 0 );
    }
    
    if( ptr )
    {
        if( CX_MAT_CN( type ) > 1 )
            CX_ERROR( CX_BadNumChannels, "cxGetReal* support only single-channel arrays" );

        value = icxGetReal( ptr, type );
    }

    __END__;

    return value;
}


// Returns specifed element of nD array
CX_IMPL  double
cxGetRealND( const CxArr* arr, int* idx )
{
    double value = 0;

    CX_FUNCNAME( "cxGetRealND" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtrND( arr, idx, &type );
    else
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, 0, 0 );

    if( ptr )
    {
        if( CX_MAT_CN( type ) > 1 )
            CX_ERROR( CX_BadNumChannels, "cxGetReal* support only single-channel arrays" );

        value = icxGetReal( ptr, type );
    }

    __END__;

    return value;
}


// Assigns new value to specifed element of nD array given linear index
CX_IMPL  void
cxSet1D( CxArr* arr, int idx, CxScalar scalar )
{
    CX_FUNCNAME( "cxSet1D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CX_IS_MAT( arr ) && CX_IS_MAT_CONT( ((CxMat*)arr)->type ))
    {
        CxMat* mat = (CxMat*)arr;

        type = CX_MAT_TYPE(mat->type);
        int pix_size = icxPixSize[type];

        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)idx*pix_size;
    }
    else if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr1D( arr, idx, &type );
    else
        ptr = icxGetNodePtr( (CxSparseMat*)arr, &idx, &type, -1, 0 );

    cxScalarToRawData( &scalar, ptr, type );

    __END__;
}


// Assigns new value to specifed element of 2D array
CX_IMPL  void
cxSet2D( CxArr* arr, int y, int x, CxScalar scalar )
{
    CX_FUNCNAME( "cxSet2D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CX_IS_MAT( arr ))
    {
        CxMat* mat = (CxMat*)arr;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        type = CX_MAT_TYPE(mat->type);
        ptr = mat->data.ptr + (size_t)y*mat->step + x*icxPixSize[type];
    }
    else if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr2D( arr, y, x, &type );
    else
    {
        int idx[] = { y, x };
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, -1, 0 );
    }
    cxScalarToRawData( &scalar, ptr, type );

    __END__;
}


// Assigns new value to specifed element of 3D array
CX_IMPL  void
cxSet3D( CxArr* arr, int z, int y, int x, CxScalar scalar )
{
    /*CX_FUNCNAME( "cxSet3D" );*/

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr3D( arr, z, y, x, &type );
    else
    {
        int idx[] = { z, y, x };
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, -1, 0 );
    }
    cxScalarToRawData( &scalar, ptr, type );

    __END__;
}


// Assigns new value to specifed element of nD array
CX_IMPL  void
cxSetND( CxArr* arr, int* idx, CxScalar scalar )
{
    /*CX_FUNCNAME( "cxSetND" );*/

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtrND( arr, idx, &type );
    else
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, -1, 0 );
    cxScalarToRawData( &scalar, ptr, type );

    __END__;
}


CX_IMPL  void
cxSetReal1D( CxArr* arr, int idx, double value )
{
    CX_FUNCNAME( "cxSetReal1D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CX_IS_MAT( arr ) && CX_IS_MAT_CONT( ((CxMat*)arr)->type ))
    {
        CxMat* mat = (CxMat*)arr;

        type = CX_MAT_TYPE(mat->type);
        int pix_size = icxPixSize[type];

        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)idx*pix_size;
    }
    else if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr1D( arr, idx, &type );
    else
        ptr = icxGetNodePtr( (CxSparseMat*)arr, &idx, &type, -1, 0 );

    if( CX_MAT_CN( type ) > 1 )
        CX_ERROR( CX_BadNumChannels, "cxSetReal* support only single-channel arrays" );

    if( ptr )
        icxSetReal( value, ptr, type );

    __END__;
}


CX_IMPL  void
cxSetReal2D( CxArr* arr, int y, int x, double value )
{
    CX_FUNCNAME( "cxSetReal2D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CX_IS_MAT( arr ))
    {
        CxMat* mat = (CxMat*)arr;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CX_ERROR( CX_StsOutOfRange, "index is out of range" );

        type = CX_MAT_TYPE(mat->type);
        ptr = mat->data.ptr + (size_t)y*mat->step + x*icxPixSize[type];
    }
    else if( !CX_IS_SPARSE_MAT( arr ))
    {
        ptr = cxPtr2D( arr, y, x, &type );
    }
    else
    {
        int idx[] = { y, x };
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, -1, 0 );
    }
    if( CX_MAT_CN( type ) > 1 )
        CX_ERROR( CX_BadNumChannels, "cxSetReal* support only single-channel arrays" );

    if( ptr )
        icxSetReal( value, ptr, type );

    __END__;
}


CX_IMPL  void
cxSetReal3D( CxArr* arr, int z, int y, int x, double value )
{
    CX_FUNCNAME( "cxSetReal3D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtr3D( arr, z, y, x, &type );
    else
    {
        int idx[] = { z, y, x };
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, -1, 0 );
    }
    if( CX_MAT_CN( type ) > 1 )
        CX_ERROR( CX_BadNumChannels, "cxSetReal* support only single-channel arrays" );

    if( ptr )
        icxSetReal( value, ptr, type );

    __END__;
}


CX_IMPL  void
cxSetRealND( CxArr* arr, int* idx, double value )
{
    CX_FUNCNAME( "cxSetRealND" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CX_IS_SPARSE_MAT( arr ))
        ptr = cxPtrND( arr, idx, &type );
    else
        ptr = icxGetNodePtr( (CxSparseMat*)arr, idx, &type, -1, 0 );

    if( CX_MAT_CN( type ) > 1 )
        CX_ERROR( CX_BadNumChannels, "cxSetReal* support only single-channel arrays" );

    if( ptr )
        icxSetReal( value, ptr, type );

    __END__;
}


CX_IMPL void
cxClearND( CxArr* arr, int* idx )
{
    /*CX_FUNCNAME( "cxClearND" );*/

    __BEGIN__;

    if( !CX_IS_SPARSE_MAT( arr ))
    {
        int type;
        uchar* ptr;
        ptr = cxPtrND( arr, idx, &type );
        if( ptr )
            memset( ptr, 0, icxPixSize[type] );
    }
    else
    {
        icxDeleteNode( (CxSparseMat*)arr, idx, 0 );
    }

    __END__;
}


/****************************************************************************************\
*                             Conversion to CxMat or IplImage                            *
\****************************************************************************************/

// convert array (CxMat or IplImage) to CxMat
CX_IMPL CxMat*
cxGetMat( const CxArr* array, CxMat* mat,
          int* pCOI, int allowND )
{
    CxMat* result = 0;
    CxMat* src = (CxMat*)array;
    int coi = 0;
    
    CX_FUNCNAME( "cxGetMat" );

    __BEGIN__;

    if( !mat || !src )
        CX_ERROR( CX_StsNullPtr, "NULL array pointer is passed" );

    if( CX_IS_MAT_HDR(src))
    {
        if( !src->data.ptr )
            CX_ERROR( CX_StsNullPtr, "The matrix has NULL data pointer" );
        
        result = (CxMat*)src;
    }
    else if( CX_IS_IMAGE_HDR(src) )
    {
        const IplImage* img = (const IplImage*)src;
        int depth, order;

        if( img->imageData == 0 )
            CX_ERROR( CX_StsNullPtr, "The image has NULL data pointer" );

        depth = icxIplToCxDepth( img->depth );
        if( depth < 0 )
            CX_ERROR_FROM_CODE( CX_BadDepth );

        order = img->dataOrder & (img->nChannels > 1 ? -1 : 0);

        if( img->roi )
        {
            if( order == IPL_DATA_ORDER_PLANE )
            {
                int type = depth;

                if( img->roi->coi == 0 )
                    CX_ERROR( CX_StsBadFlag,
                    "Images with planar data layout should be used with COI selected" );

                CX_CALL( cxInitMatHeader( mat, img->roi->height,
                                   img->roi->width, type,
                                   img->imageData + (img->roi->coi-1)*img->imageSize +
                                   img->roi->yOffset*img->widthStep +
                                   img->roi->xOffset*icxPixSize[type],
                                   img->widthStep ));
            }
            else /* pixel order */
            {
                int type = depth + (img->nChannels - 1)*8;
                coi = img->roi->coi;

                if( img->nChannels > 4 )
                    CX_ERROR( CX_BadNumChannels,
                        "The image is interleaved and has more than 4 channels" );

                CX_CALL( cxInitMatHeader( mat, img->roi->height, img->roi->width,
                                          type, img->imageData +
                                          img->roi->yOffset*img->widthStep +
                                          img->roi->xOffset*icxPixSize[type],
                                          img->widthStep ));
            }
        }
        else
        {
            int type = depth + (img->nChannels - 1)*8;

            if( order != IPL_DATA_ORDER_PIXEL )
                CX_ERROR( CX_StsBadFlag, "Pixel order should be used with coi == 0" );

            CX_CALL( cxInitMatHeader( mat, img->height, img->width, type,
                                      img->imageData, img->widthStep ));
        }

        result = mat;
    }
    else if( allowND && CX_IS_MATND_HDR(src) )
    {
        CxMatND* matnd = (CxMatND*)src;
        int i;
        int size1 = matnd->dim[0].size, size2 = 1;
        
        if( !src->data.ptr )
            CX_ERROR( CX_StsNullPtr, "Input array has NULL data pointer" );

        if( !CX_IS_MAT_CONT( matnd->type ))
            CX_ERROR( CX_StsBadArg, "Only continuous nD arrays are supported here" );

        if( matnd->dims > 2 )
            for( i = 1; i < matnd->dims; i++ )
                size1 *= matnd->dim[i].size;
        else
            size2 = matnd->dims == 1 ? 1 : matnd->dim[1].size;

        mat->refcount = 0;
        mat->data.ptr = matnd->data.ptr;
        mat->rows = size1;
        mat->cols = size2;
        mat->type = CX_MAT_TYPE(matnd->type) | CX_MAT_MAGIC_VAL | CX_MAT_CONT_FLAG;
        mat->step = size2*icxPixSize[CX_MAT_TYPE(matnd->type)];
        mat->step &= size1 > 1 ? -1 : 0;

        icxCheckHuge( mat );
        result = mat;
    }
    else
    {
        CX_ERROR( CX_StsBadFlag, "Unrecognized or unsupported array type" );
    }

    __END__;

    if( pCOI )
        *pCOI = coi;

    return result;
}


CX_IMPL CxArr*
cxReshapeMatND( const CxArr* arr,
                int sizeof_header, CxArr* _header,
                int new_cn, int new_dims, int* new_sizes )
{
    CxArr* result = 0;
    CX_FUNCNAME( "cxReshapeMatND" );

    __BEGIN__;

    int dims, coi = 0;

    if( !arr || !_header )
        CX_ERROR( CX_StsNullPtr, "NULL pointer to array or destination header" );

    if( new_cn == 0 && new_dims == 0 )
        CX_ERROR( CX_StsBadArg, "None of array parameters is changed: dummy call?" );

    if( (unsigned)new_cn > 4 )
        CX_ERROR( CX_BadNumChannels,
        "Number of channels should be 0 (not changed) or 1..4" );
    
    CX_CALL( dims = cxGetDims( arr ));

    if( new_dims == 0 )
    {
        new_sizes = 0;
        new_dims = dims;
    }
    else if( new_dims == 1 )
    {
        new_sizes = 0;
    }
    else
    {
        if( (unsigned)new_dims > CX_MAX_DIM )
            CX_ERROR( CX_StsOutOfRange, "Bad number of dimensions" );
        if( !new_sizes )
            CX_ERROR( CX_StsNullPtr, "New dimension sizes are not specified" );
    }

    if( new_dims <= 2 )
    {
        CxMat* mat = (CxMat*)arr;
        CxMat* header = (CxMat*)_header;
        int* refcount = 0;
        int  total_width, new_rows, cn;

        if( sizeof_header != sizeof(CxMat))
            CX_ERROR( CX_StsBadArg, "The header should be CxMat" );

        if( mat == header )
            refcount = mat->refcount;
        else if( !CX_IS_MAT( mat ))
            CX_CALL( mat = cxGetMat( mat, header, &coi, 1 ));

        cn = CX_MAT_CN( mat->type );
        total_width = mat->cols * cn;

        if( new_cn == 0 )
            new_cn = cn;

        if( new_sizes )
            new_rows = new_sizes[0];
        else if( new_dims == 1 )
            new_rows = total_width*mat->rows/new_cn;
        else
        {
            new_rows = mat->rows;
            if( new_cn > total_width )
                new_rows = mat->rows * total_width / new_cn;
        }

        if( new_rows != mat->rows )
        {
            int total_size = total_width * mat->rows;

            if( !CX_IS_MAT_CONT( mat->type ))
                CX_ERROR( CX_BadStep,
                "The matrix is not continuous so the number of rows can not be changed" );

            total_width = total_size / new_rows;

            if( total_width * new_rows != total_size )
                CX_ERROR( CX_StsBadArg, "The total number of matrix elements "
                                        "is not divisible by the new number of rows" );
        }

        header->rows = new_rows;
        header->cols = total_width / new_cn;

        if( header->cols * new_cn != total_width ||
            new_sizes && header->cols != new_sizes[1] )
            CX_ERROR( CX_StsBadArg, "The total matrix width is not "
                            "divisible by the new number of columns" );

        header->type = (mat->type & ~CX_MAT_CN_MASK) + (new_cn - 1)*8;
        header->step = header->cols * icxPixSize[CX_MAT_TYPE(mat->type)];
        header->step &= new_rows > 1 ? -1 : 0;
        header->refcount = refcount;
    }
    else
    {
        CxMatND* header = (CxMatND*)_header;

        if( sizeof_header != sizeof(CxMatND))
            CX_ERROR( CX_StsBadSize, "The header should be CxMatND" );
        
        if( !new_sizes )
        {
            if( !CX_IS_MATND( arr ))
                CX_ERROR( CX_StsBadArg, "The source array must be CxMatND" );

            {
            CxMatND* mat = (CxMatND*)arr;
            assert( new_cn > 0 );
            int last_dim_size = mat->dim[mat->dims-1].size*CX_MAT_CN(mat->type);
            int new_size = last_dim_size/new_cn;

            if( new_size*new_cn != last_dim_size )
                CX_ERROR( CX_StsBadArg,
                "The last dimension full size is not divisible by new number of channels");

            if( mat != header )
            {
                memcpy( header, mat, sizeof(*header));
                header->refcount = 0;
            }

            header->dim[header->dims-1].size = new_size;
            header->type = (header->type & ~CX_MAT_CN_MASK) + (new_cn - 1)*8;
            }
        }
        else
        {
            CxMatND stub;
            CxMatND* mat = (CxMatND*)arr;
            int i, size1, size2;
            int step;
            
            if( new_cn != 0 )
                CX_ERROR( CX_StsBadArg,
                "Simultaneous change of shape and number of channels is not supported. "
                "Do it by 2 separate calls" );
            
            if( !CX_IS_MATND( mat ))
            {
                CX_CALL( cxGetMatND( mat, &stub, &coi ));
                mat = &stub;
            }

            if( CX_IS_MAT_CONT( mat->type ))
                CX_ERROR( CX_StsBadArg, "Non-continuous nD arrays are not supported" );

            size1 = mat->dim[0].size;
            for( i = 1; i < dims; i++ )
                size1 *= mat->dim[i].size;

            size2 = 1;
            for( i = 0; i < new_dims; i++ )
            {
                if( new_sizes[i] <= 0 )
                    CX_ERROR( CX_StsBadSize,
                    "One of new dimension sizes is non-positive" );
                size2 *= new_sizes[i];
            }

            if( size1 != size2 )
                CX_ERROR( CX_StsBadSize,
                "Number of elements in the original and reshaped array is different" );

            if( header != mat )
                header->refcount = 0;

            header->dims = new_dims;
            header->type = mat->type;
            header->data.ptr = mat->data.ptr;
            step = icxPixSize[CX_MAT_TYPE(header->type)];

            for( i = new_dims - 1; i >= 0; i-- )
            {
                header->dim[i].size = new_sizes[i];
                header->dim[i].step = step;
                step *= new_sizes[i];
            }
        }
    }

    if( !coi )
        CX_ERROR( CX_BadCOI, "COI is not supported by this operation" );

    result = _header;
    
    __END__;

    return result;
}


CX_IMPL CxMat*
cxReshape( const CxArr* array, CxMat* header,
           int new_cn, int new_rows )
{
    CxMat* result = 0;
    CX_FUNCNAME( "cxReshape" );

    __BEGIN__;

    CxMat *mat = (CxMat*)array;
    int total_width, new_width;

    if( !header )
        CX_ERROR( CX_StsNullPtr, "" );

    if( !CX_IS_MAT( mat ))
    {
        int coi = 0;
        CX_CALL( mat = cxGetMat( mat, header, &coi, 1 ));
        if( coi )
            CX_ERROR( CX_BadCOI, "COI is not supported" );
    }

    if( new_cn == 0 )
        new_cn = CX_MAT_CN(mat->type);
    else if( (unsigned)(new_cn - 1) > 3 )
        CX_ERROR( CX_BadNumChannels, "" );

    if( mat != header )
    {
        *header = *mat;
        header->refcount = 0;
    }

    total_width = mat->cols * CX_MAT_CN( mat->type );

    if( new_cn > total_width )
        new_rows = mat->rows * total_width / new_cn;

    if( new_rows != 0 )
    {
        int total_size = total_width * mat->rows;

        if( !CX_IS_MAT_CONT( mat->type ))
            CX_ERROR( CX_BadStep,
            "The matrix is not continuous, thus its number of rows can not be changed" );

        if( (unsigned)new_rows > (unsigned)total_size )
            CX_ERROR( CX_StsOutOfRange, "Bad new number of rows" );

        total_width = total_size / new_rows;

        if( total_width * new_rows != total_size )
            CX_ERROR( CX_StsBadArg, "The total number of matrix elements "
                                    "is not divisible by the new number of rows" );

        header->rows = new_rows;
        header->step = total_width * icxPixSize[mat->type & CX_MAT_DEPTH_MASK];
    }

    new_width = total_width / new_cn;

    if( new_width * new_cn != total_width )
        CX_ERROR( CX_BadNumChannels,
        "The total width is not divisible by the new number of channels" );

    header->cols = new_width;
    header->type = (mat->type & ~CX_MAT_CN_MASK) + ((new_cn - 1)*8);

    result = header;

    __END__;

    return  result;
}


// convert array (CxMat or IplImage) to IplImage
CX_IMPL IplImage*
cxGetImage( const CxArr* array, IplImage* img )
{
    IplImage* result = 0;
    const IplImage* src = (const IplImage*)array;
    
    CX_FUNCNAME( "cxGetImage" );

    __BEGIN__;

    if( !img )
        CX_ERROR_FROM_CODE( CX_StsNullPtr );

    if( !CX_IS_IMAGE_HDR(src) )
    {
        const CxMat* mat = (const CxMat*)src;
        
        if( !CX_IS_MAT_HDR(mat))
            CX_ERROR_FROM_CODE( CX_StsBadFlag );

        if( mat->data.ptr == 0 )
            CX_ERROR_FROM_CODE( CX_StsNullPtr );

        cxInitImageHeader( img, cxSize(mat->cols, mat->rows),
                           icxCxToIplDepth(CX_MAT_DEPTH(mat->type)),
                           CX_MAT_CN(mat->type) );
        cxSetData( img, mat->data.ptr, mat->step );

        result = img;
    }
    else
    {
        result = (IplImage*)src;
    }

    __END__;

    return result;
}


/****************************************************************************************\
*                               IplImage-specific functions                              *
\****************************************************************************************/

static IplROI* icxCreateROI( int coi, int xOffset, int yOffset, int width, int height )
{
    IplROI *roi = 0;

    CX_FUNCNAME( "icxCreateROI" );

    __BEGIN__;

    if( !CxIPL.createROI )
    {
        CX_CALL( roi = (IplROI*)cxAlloc( sizeof(*roi)));

        roi->coi = coi;
        roi->xOffset = xOffset;
        roi->yOffset = yOffset;
        roi->width = width;
        roi->height = height;
    }
    else
    {
        roi = CxIPL.createROI( coi, xOffset, yOffset, width, height );
    }

    __END__;

    return roi;
}

static  void
icxGetColorModel( int nchannels, char** colorModel, char** channelSeq )
{
    static char* tab[][2] =
    {
        {"GRAY", "GRAY"},
        {"",""},
        {"RGB","BGR"},
        {"RGB","BGRA"}
    };

    nchannels--;
    *colorModel = *channelSeq = "";

    if( (unsigned)nchannels <= 3 )
    {
        *colorModel = tab[nchannels][0];
        *channelSeq = tab[nchannels][1];
    }
}


// create IplImage header
CX_IMPL IplImage *
cxCreateImageHeader( CxSize size, int depth, int channels )
{
    IplImage *img = 0;

    CX_FUNCNAME( "cxCreateImageHeader" );

    __BEGIN__;

    if( !CxIPL.createHeader )
    {
        CX_CALL( img = (IplImage *)cxAlloc( sizeof( *img )));
        CX_CALL( cxInitImageHeader( img, size, depth, channels, IPL_ORIGIN_TL,
                                    CX_DEFAULT_IMAGE_ROW_ALIGN ));
    }
    else
    {
        char *colorModel;
        char *channelSeq;

        icxGetColorModel( channels, &colorModel, &channelSeq );

        img = CxIPL.createHeader( channels, 0, depth, colorModel, channelSeq,
                                  IPL_DATA_ORDER_PIXEL, IPL_ORIGIN_TL,
                                  CX_DEFAULT_IMAGE_ROW_ALIGN,
                                  size.width, size.height, 0, 0, 0, 0 );
    }

    __END__;

    if( cxGetErrStatus() < 0 && img )
        cxReleaseImageHeader( &img );

    return img;
}


// create IplImage header and allocate underlying data
CX_IMPL IplImage *
cxCreateImage( CxSize size, int depth, int channels )
{
    IplImage *img = 0;

    CX_FUNCNAME( "cxCreateImage" );

    __BEGIN__;

    CX_CALL( img = cxCreateImageHeader( size, depth, channels ));
    assert( img );
    CX_CALL( cxCreateData( img ));

    __END__;

    if( cxGetErrStatus() < 0 )
        cxReleaseImage( &img );

    return img;
}


// initalize IplImage header, allocated by the user
CX_IMPL IplImage*
cxInitImageHeader( IplImage * image, CxSize size, int depth,
                   int channels, int origin, int align )
{
    IplImage* result = 0;

    CX_FUNCNAME( "cxInitImageHeader" );

    __BEGIN__;

    char *colorModel, *channelSeq;

    if( !image )
        CX_ERROR( CX_HeaderIsNull, "null pointer to header" );

    memset( image, 0, sizeof( *image ));
    image->nSize = sizeof( *image );

    CX_CALL( icxGetColorModel( channels, &colorModel, &channelSeq ));
    strncpy( image->colorModel, colorModel, 4 );
    strncpy( image->channelSeq, channelSeq, 4 );

    if( size.width < 0 || size.height < 0 )
        CX_ERROR( CX_BadROISize, "Bad input roi" );

    if( (depth != (int)IPL_DEPTH_1U && depth != (int)IPL_DEPTH_8U &&
         depth != (int)IPL_DEPTH_8S && depth != (int)IPL_DEPTH_16U &&
         depth != (int)IPL_DEPTH_16S && depth != (int)IPL_DEPTH_32S &&
         depth != (int)IPL_DEPTH_32F && depth != (int)IPL_DEPTH_64F) ||
         channels < 0 )
        CX_ERROR( CX_BadDepth, "Unsupported format" );
    if( origin != CX_ORIGIN_BL && origin != CX_ORIGIN_TL )
        CX_ERROR( CX_BadOrigin, "Bad input origin" );

    if( align != 4 && align != 8 )
        CX_ERROR( CX_BadAlign, "Bad input align" );

    image->width = size.width;
    image->height = size.height;

    if( image->roi )
    {
        image->roi->coi = 0;
        image->roi->xOffset = image->roi->yOffset = 0;
        image->roi->width = size.width;
        image->roi->height = size.height;
    }

    image->nChannels = MAX( channels, 1 );
    image->depth = depth;
    image->align = align;
    image->widthStep = (((image->width * image->nChannels *
         (image->depth & ~IPL_DEPTH_SIGN) + 7)/8)+ align - 1) & (~(align - 1));
    image->origin = origin;
    image->imageSize = image->widthStep * image->height;

    result = image;

    __END__;

    return result;
}


CX_IMPL void
cxReleaseImageHeader( IplImage** image )
{
    CX_FUNCNAME( "cxReleaseImageHeader" );

    __BEGIN__;

    if( !image )
        CX_ERROR( CX_StsNullPtr, "" );

    if( *image )
    {
        IplImage* img = *image;
        *image = 0;
        
        if( !CxIPL.deallocate )
        {
            cxFree( (void**)&(img->roi) );
            cxFree( (void**)&img );
        }
        else
        {
            CxIPL.deallocate( img, IPL_IMAGE_HEADER | IPL_IMAGE_ROI );
        }
    }
    __END__;
}


CX_IMPL void
cxReleaseImage( IplImage ** image )
{
    CX_FUNCNAME( "cxReleaseImage" );

    __BEGIN__

    if( !image )
        CX_ERROR( CX_StsNullPtr, "" );

    if( *image )
    {
        IplImage* img = *image;
        *image = 0;
        
        cxReleaseData( img );
        cxReleaseImageHeader( &img );
    }

    __END__;
}


CX_IMPL void
cxSetImageROI( IplImage* image, CxRect rect )
{
    CX_FUNCNAME( "cxSetImageROI" );

    __BEGIN__;

    if( !image )
        CX_ERROR( CX_HeaderIsNull, 0 );

    if( rect.x > image->width || rect.y > image->height )
        CX_ERROR( CX_BadROISize, 0 );

    if( rect.x + rect.width < 0 || rect.y + rect.height < 0 )
        CX_ERROR( CX_BadROISize, 0 );

    if( rect.x < 0 )
    {
        rect.width += rect.x;
        rect.x = 0;
    }

    if( rect.y < 0 )
    {
        rect.height += rect.y;
        rect.y = 0;
    }

    if( rect.x + rect.width > image->width )
        rect.width = image->width - rect.x;

    if( rect.y + rect.height > image->height )
        rect.height = image->height - rect.y;

    if( image->roi )
    {
        image->roi->xOffset = rect.x;
        image->roi->yOffset = rect.y;
        image->roi->width = rect.width;
        image->roi->height = rect.height;
    }
    else
    {
        CX_CALL( image->roi = icxCreateROI( 0, rect.x, rect.y, rect.width, rect.height ));
    }

    __END__;
}


CX_IMPL void
cxResetImageROI( IplImage* image )
{
    CX_FUNCNAME( "cxResetImageROI" );

    __BEGIN__;

    if( !image )
        CX_ERROR( CX_HeaderIsNull, 0 );

    if( image->roi )
    {
        if( !CxIPL.deallocate )
        {
            CX_CALL( cxFree( (void**)&(image->roi) ));
        }
        else
        {
            CxIPL.deallocate( image, IPL_IMAGE_ROI );
            image->roi = 0;
        }
    }

    __END__;
}


CX_IMPL CxRect
cxGetImageROI( const IplImage* img )
{
    CxRect rect = { 0, 0, 0, 0 };
    
    CX_FUNCNAME( "cxGetImageROI" );

    __BEGIN__;

    if( !img )
        CX_ERROR( CX_StsNullPtr, "Null pointer to image" );

    if( img->roi )
        rect = cxRect( img->roi->xOffset, img->roi->yOffset,
                       img->roi->width, img->roi->height );
    else
        rect = cxRect( 0, 0, img->width, img->height );

    __END__;
    
    return rect;
}


CX_IMPL void
cxSetImageCOI( IplImage* image, int coi )
{
    CX_FUNCNAME( "cxSetImageCOI" );

    __BEGIN__;

    if( !image )
        CX_ERROR( CX_HeaderIsNull, 0 );

    if( (unsigned)coi > (unsigned)(image->nChannels) )
        CX_ERROR( CX_BadCOI, 0 );

    if( image->roi || coi != 0 )
    {
        if( image->roi )
        {
            image->roi->coi = coi;
        }
        else
        {
            CX_CALL( image->roi = icxCreateROI( coi, 0, 0, image->width, image->height ));
        }
    }

    __END__;
}


CX_IMPL int
cxGetImageCOI( IplImage* image )
{
    int coi = -1;
    CX_FUNCNAME( "cxGetImageCOI" );

    __BEGIN__;

    if( !image )
        CX_ERROR( CX_HeaderIsNull, 0 );

    coi = image->roi ? image->roi->coi : 0;

    __END__;

    return coi;
}


CX_IMPL IplImage*
cxCloneImage( const IplImage* src )
{
    IplImage* dst = 0;
    CX_FUNCNAME( "cxCloneImage" );

    __BEGIN__;

    if( !CX_IS_IMAGE_HDR( src ))
        CX_ERROR( CX_StsBadArg, "Bad image header" );

    if( !CxIPL.cloneImage )
    {
        CX_CALL( dst = (IplImage*)cxAlloc( sizeof(*dst)));

        memcpy( dst, src, sizeof(*src));
        dst->imageData = dst->imageDataOrigin = 0;
        dst->roi = 0;

        if( src->roi )
        {
            dst->roi = icxCreateROI( src->roi->coi, src->roi->xOffset,
                          src->roi->yOffset, src->roi->width, src->roi->height );
        }

        if( src->imageData )
        {
            int size = src->imageSize;
            cxCreateData( dst );
            memcpy( dst->imageData, src->imageData, size );
        }
    }
    else
    {
        dst = CxIPL.cloneImage( src );
    }

    __END__;

    return dst;
}


/****************************************************************************************\
*                            Additional operations on CxTermCriteria                     *
\****************************************************************************************/

CX_IMPL CxTermCriteria
cxCheckTermCriteria( CxTermCriteria criteria, double default_eps,
                     int default_max_iters )
{
    CX_FUNCNAME( "cxCheckTermCriteria" );

    CxTermCriteria crit;

    crit.type = CX_TERMCRIT_ITER|CX_TERMCRIT_EPS;
    crit.maxIter = default_max_iters;
    crit.epsilon = (float)default_eps;
    
    __BEGIN__;

    if( (criteria.type & ~(CX_TERMCRIT_EPS | CX_TERMCRIT_ITER)) != 0 )
        CX_ERROR( CX_StsBadArg,
                  "Unknown type of term criteria" );

    if( (criteria.type & CX_TERMCRIT_ITER) != 0 )
    {
        if( criteria.maxIter <= 0 )
            CX_ERROR( CX_StsBadArg,
                  "Iterations flag is set and maximum number of iterations is <= 0" );
        crit.maxIter = criteria.maxIter;
    }
    
    if( (criteria.type & CX_TERMCRIT_EPS) != 0 )
    {
        if( criteria.epsilon < 0 )
            CX_ERROR( CX_StsBadArg, "Accuracy flag is set and epsilon is < 0" );

        crit.epsilon = criteria.epsilon;
    }

    if( (criteria.type & (CX_TERMCRIT_EPS | CX_TERMCRIT_ITER)) == 0 )
        CX_ERROR( CX_StsBadArg,
                  "Neither accuracy nor maximum iterations "
                  "number flags are set in criteria type" );

    __END__;

    crit.epsilon = (float)MAX( 0, crit.epsilon );
    crit.maxIter = MAX( 1, crit.maxIter );

    return crit;
}


/* End of file. */
