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
//  CxMat basic operations: cxCopy, cxSet
//
// */

#include "_cxcore.h"

/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                                  L/L COPY & SET FUNCTIONS                           //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////


IPCXAPI_IMPL( CxStatus, icxCopy_8u_C1R, ( const uchar* src, int src_step,
                                          uchar* dst, int dst_step, CxSize size ))
{
    for( ; size.height--; src += src_step, dst += dst_step )
        memcpy( dst, src, size.width );

    return  CX_OK;
}


static CxStatus CX_STDCALL
icxSet_8u_C1R( uchar* dst, int dst_step, CxSize size,
               const void* scalar, int pix_size )
{
    int copy_len = 12*pix_size;

    for( ; size.height--; )
    {
        uchar* dst_limit = dst + size.width;

        while( dst + copy_len <= dst_limit )
        {
            memcpy( dst, scalar, copy_len );
            dst += copy_len;
        }

        memcpy( dst, scalar, dst_limit - dst );
        dst = dst_limit - size.width + dst_step;
    }

    return CX_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                                L/L COPY WITH MASK FUNCTIONS                         //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////


#define ICX_DEF_COPY_MASK_C1_CASE( type, worktype, src, dst, mask, len )\
{                                                                       \
    int i;                                                              \
                                                                        \
    for( i = 0; i <= (len) - 4; i += 4 )                                \
    {                                                                   \
        worktype m0 = (mask)[i] ? -1 : 0;                               \
        worktype m1 = (mask)[i+1] ? -1 : 0;                             \
        worktype t0 = (dst)[i];                                         \
        worktype t1 = (dst)[i+1];                                       \
                                                                        \
        t0 ^= (t0 ^ (src)[i]) & m0;                                     \
        t1 ^= (t1 ^ (src)[i+1]) & m1;                                   \
                                                                        \
        (dst)[i] = (type)t0;                                            \
        (dst)[i+1] = (type)t1;                                          \
                                                                        \
        m0 = (mask)[i+2] ? -1 : 0;                                      \
        m1 = (mask)[i+3] ? -1 : 0;                                      \
        t0 = (dst)[i+2];                                                \
        t1 = (dst)[i+3];                                                \
                                                                        \
        t0 ^= (t0 ^ (src)[i+2]) & m0;                                   \
        t1 ^= (t1 ^ (src)[i+3]) & m1;                                   \
                                                                        \
        (dst)[i+2] = (type)t0;                                          \
        (dst)[i+3] = (type)t1;                                          \
    }                                                                   \
                                                                        \
    for( ; i < (len); i++ )                                             \
    {                                                                   \
        worktype m = (mask)[i] ? -1 : 0;                                \
        worktype t = (dst)[i];                                          \
                                                                        \
        t ^= (t ^ (src)[i]) & m;                                        \
                                                                        \
        (dst)[i] = (type)t;                                             \
    }                                                                   \
}


#define ICX_DEF_COPY_MASK_C3_CASE( type, worktype, src, dst, mask, len )\
{                                                                       \
    int i;                                                              \
                                                                        \
    for( i = 0; i < (len); i++ )                                        \
    {                                                                   \
        worktype m  = (mask)[i] ? -1 : 0;                               \
        worktype t0 = (dst)[i*3];                                       \
        worktype t1 = (dst)[i*3+1];                                     \
        worktype t2 = (dst)[i*3+2];                                     \
                                                                        \
        t0 ^= (t0 ^ (src)[i*3]) & m;                                    \
        t1 ^= (t1 ^ (src)[i*3+1]) & m;                                  \
        t2 ^= (t2 ^ (src)[i*3+2]) & m;                                  \
                                                                        \
        (dst)[i*3] = (type)t0;                                          \
        (dst)[i*3+1] = (type)t1;                                        \
        (dst)[i*3+2] = (type)t2;                                        \
    }                                                                   \
}


#define ICX_DEF_COPY_MASK_C4_CASE( type, worktype, src, dst, mask, len )\
{                                                                       \
    int i;                                                              \
                                                                        \
    for( i = 0; i < (len); i++ )                                        \
    {                                                                   \
        worktype m  = (mask)[i] ? -1 : 0;                               \
        worktype t0 = (dst)[i*4];                                       \
        worktype t1 = (dst)[i*4+1];                                     \
                                                                        \
        t0 ^= (t0 ^ (src)[i*4]) & m;                                    \
        t1 ^= (t1 ^ (src)[i*4+1]) & m;                                  \
                                                                        \
        (dst)[i*4] = (type)t0;                                          \
        (dst)[i*4+1] = (type)t1;                                        \
                                                                        \
        t0 = (dst)[i*4+2];                                              \
        t1 = (dst)[i*4+3];                                              \
                                                                        \
        t0 ^= (t0 ^ (src)[i*4+2]) & m;                                  \
        t1 ^= (t1 ^ (src)[i*4+3]) & m;                                  \
                                                                        \
        (dst)[i*4+2] = (type)t0;                                        \
        (dst)[i*4+3] = (type)t1;                                        \
    }                                                                   \
}


#define ICX_DEF_COPY_MASK_2D( name, type, worktype, cn )                \
IPCXAPI_IMPL( CxStatus,                                                 \
name,( const type* src, int step1, type* dst, int step,                 \
       CxSize size, const uchar* mask, int step2 ))                     \
{                                                                       \
    for( ; size.height--; (char*&)src += step1,                         \
                          (char*&)dst += step,                          \
                          mask += step2 )                               \
    {                                                                   \
        ICX_DEF_COPY_MASK_C##cn##_CASE( type, worktype, src,            \
                                        dst, mask, size.width )         \
    }                                                                   \
                                                                        \
    return  CX_OK;                                                      \
}



#define ICX_DEF_SET_MASK_C1_CASE( type, worktype, src, dst, mask, len ) \
{                                                                       \
    int i;                                                              \
                                                                        \
    for( i = 0; i <= (len) - 4; i += 4 )                                \
    {                                                                   \
        worktype m0 = (mask)[i] ? -1 : 0;                               \
        worktype m1 = (mask)[i+1] ? -1 : 0;                             \
        worktype t0 = (dst)[i];                                         \
        worktype t1 = (dst)[i+1];                                       \
                                                                        \
        t0 ^= (t0 ^ s0) & m0;                                           \
        t1 ^= (t1 ^ s0) & m1;                                           \
                                                                        \
        (dst)[i] = (type)t0;                                            \
        (dst)[i+1] = (type)t1;                                          \
                                                                        \
        m0 = (mask)[i+2] ? -1 : 0;                                      \
        m1 = (mask)[i+3] ? -1 : 0;                                      \
        t0 = (dst)[i+2];                                                \
        t1 = (dst)[i+3];                                                \
                                                                        \
        t0 ^= (t0 ^ s0) & m0;                                           \
        t1 ^= (t1 ^ s0) & m1;                                           \
                                                                        \
        (dst)[i+2] = (type)t0;                                          \
        (dst)[i+3] = (type)t1;                                          \
    }                                                                   \
                                                                        \
    for( ; i < (len); i++ )                                             \
    {                                                                   \
        worktype m = (mask)[i] ? -1 : 0;                                \
        worktype t = (dst)[i];                                          \
                                                                        \
        t ^= (t ^ s0) & m;                                              \
                                                                        \
        (dst)[i] = (type)t;                                             \
    }                                                                   \
}


#define ICX_DEF_SET_MASK_C3_CASE( type, worktype, src, dst, mask, len ) \
{                                                                       \
    int i;                                                              \
                                                                        \
    for( i = 0; i < (len); i++ )                                        \
    {                                                                   \
        worktype m  = (mask)[i] ? -1 : 0;                               \
        worktype t0 = (dst)[i*3];                                       \
        worktype t1 = (dst)[i*3+1];                                     \
        worktype t2 = (dst)[i*3+2];                                     \
                                                                        \
        t0 ^= (t0 ^ s0) & m;                                            \
        t1 ^= (t1 ^ s1) & m;                                            \
        t2 ^= (t2 ^ s2) & m;                                            \
                                                                        \
        (dst)[i*3] = (type)t0;                                          \
        (dst)[i*3+1] = (type)t1;                                        \
        (dst)[i*3+2] = (type)t2;                                        \
    }                                                                   \
}


#define ICX_DEF_SET_MASK_C4_CASE( type, worktype, src, dst, mask, len ) \
{                                                                       \
    int i;                                                              \
                                                                        \
    for( i = 0; i < (len); i++ )                                        \
    {                                                                   \
        worktype m  = (mask)[i] ? -1 : 0;                               \
        worktype t0 = (dst)[i*4];                                       \
        worktype t1 = (dst)[i*4+1];                                     \
                                                                        \
        t0 ^= (t0 ^ s0) & m;                                            \
        t1 ^= (t1 ^ s1) & m;                                            \
                                                                        \
        (dst)[i*4] = (type)t0;                                          \
        (dst)[i*4+1] = (type)t1;                                        \
                                                                        \
        t0 = (dst)[i*4+2];                                              \
        t1 = (dst)[i*4+3];                                              \
                                                                        \
        t0 ^= (t0 ^ s2) & m;                                            \
        t1 ^= (t1 ^ s3) & m;                                            \
                                                                        \
        (dst)[i*4+2] = (type)t0;                                        \
        (dst)[i*4+3] = (type)t1;                                        \
    }                                                                   \
}


#define ICX_DEF_SET_MASK_2D( name, type, worktype, cn )                 \
IPCXAPI_IMPL( CxStatus,                                                 \
name,( type* dst, int step, const uchar* mask, int step2,               \
       CxSize size, const type* scalar ))                               \
{                                                                       \
    CX_UN_ENTRY_C##cn( worktype );                                      \
                                                                        \
    for( ; size.height--; mask += step2, (char*&)dst += step )          \
    {                                                                   \
        ICX_DEF_SET_MASK_C##cn##_CASE( type, worktype, buf,             \
                                       dst, mask, size.width )          \
    }                                                                   \
                                                                        \
    return CX_OK;                                                       \
}


ICX_DEF_SET_MASK_2D( icxSet_8u_C1MR, uchar, int, 1 )
ICX_DEF_SET_MASK_2D( icxSet_8u_C2MR, ushort, int, 1 )
ICX_DEF_SET_MASK_2D( icxSet_8u_C3MR, uchar, int, 3 )
ICX_DEF_SET_MASK_2D( icxSet_16u_C2MR, int, int, 1 )
ICX_DEF_SET_MASK_2D( icxSet_16u_C3MR, ushort, int, 3 )
ICX_DEF_SET_MASK_2D( icxSet_32s_C2MR, int64, int64, 1 )
ICX_DEF_SET_MASK_2D( icxSet_32s_C3MR, int, int, 3 )
ICX_DEF_SET_MASK_2D( icxSet_64s_C2MR, int, int, 4 )
ICX_DEF_SET_MASK_2D( icxSet_64s_C3MR, int64, int64, 3 )
ICX_DEF_SET_MASK_2D( icxSet_64s_C4MR, int64, int64, 4 )

ICX_DEF_COPY_MASK_2D( icxCopy_8u_C1MR, uchar, int, 1  )
ICX_DEF_COPY_MASK_2D( icxCopy_8u_C2MR, ushort, int, 1 )
ICX_DEF_COPY_MASK_2D( icxCopy_8u_C3MR, uchar, int, 3 )
ICX_DEF_COPY_MASK_2D( icxCopy_16u_C2MR, int, int, 1 )
ICX_DEF_COPY_MASK_2D( icxCopy_16u_C3MR, ushort, int, 3 )
ICX_DEF_COPY_MASK_2D( icxCopy_32s_C2MR, int64, int64, 1 )
ICX_DEF_COPY_MASK_2D( icxCopy_32s_C3MR, int, int, 3 )
ICX_DEF_COPY_MASK_2D( icxCopy_64s_C2MR, int, int, 4 )
ICX_DEF_COPY_MASK_2D( icxCopy_64s_C3MR, int64, int64, 3 )
ICX_DEF_COPY_MASK_2D( icxCopy_64s_C4MR, int64, int64, 4 )


CX_DEF_INIT_PIXSIZE_TAB_2D( Set, MR )
CX_DEF_INIT_PIXSIZE_TAB_2D( Copy, MR )

typedef CxStatus (CX_STDCALL * CxCopyMaskFunc)(const void* src, int src_step,
                                               void* dst, int dst_step, CxSize size,
                                               const void* mask, int mask_step);

/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                                H/L COPY & SET FUNCTIONS                             //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////

/* dst = src */
CX_IMPL void
cxCopy( const void* srcarr, void* dstarr, const void* maskarr )
{
    static CxBtFuncTable copym_tab;
    static int inittab = 0;
    
    CX_FUNCNAME( "cxCopy" );
    
    __BEGIN__;

    int pix_size;
    CxMat srcstub, *src = (CxMat*)srcarr;
    CxMat dststub, *dst = (CxMat*)dstarr;
    CxSize size;

    if( !CX_IS_MAT(src) || !CX_IS_MAT(dst) )
    {
        if( CX_IS_SPARSE_MAT(src) && CX_IS_SPARSE_MAT(dst))
        {
            CxSparseMat* src1 = (CxSparseMat*)src;
            CxSparseMat* dst1 = (CxSparseMat*)dst;
            CxSparseMatIterator iterator;
            CxSparseNode* node;

            dst1->dims = src1->dims;
            memcpy( dst1->size, src1->size, src1->dims*sizeof(src1->size[0]));
            dst1->valoffset = src1->valoffset;
            dst1->idxoffset = src1->idxoffset;
            dst1->total = src1->total;
            cxClearSet( dst1->heap );

            if( dst1->total >= dst1->hashsize*CX_SPARSE_HASH_RATIO )
            {
                CX_CALL( cxFree( (void**)(&dst1->hashtable) ));
                dst1->hashsize = src1->hashsize;
                CX_CALL( dst1->hashtable =
                    (void**)cxAlloc( dst1->hashsize*sizeof(dst1->hashtable[0])));
            }

            memset( dst1->hashtable, 0, dst1->hashsize*sizeof(dst1->hashtable[0]));

            for( node = cxInitSparseMatIterator( src1, &iterator );
                 node != 0; node = cxGetNextSparseNode( &iterator ))
            {
                CxSparseNode* node_copy = (CxSparseNode*)cxSetNew( dst1->heap );
                int tabidx = node->hashval & (dst1->hashsize - 1);
                memcpy( node_copy, node, dst1->heap->elem_size );
                node_copy->next = (CxSparseNode*)dst1->hashtable[tabidx];
                dst1->hashtable[tabidx] = node_copy;
            }
            EXIT;
        }
        else if( CX_IS_MATND(src) || CX_IS_MATND(dst) )
        {
            CxArr* arrs[] = { src, dst };
            CxMatND stubs[3];
            CxNArrayIterator iterator;

            CX_CALL( cxInitNArrayIterator( 2, arrs, maskarr, stubs, &iterator ));
            pix_size = icxPixSize[CX_MAT_TYPE(iterator.hdr[0]->type)];

            if( !maskarr )
            {
                iterator.size.width *= pix_size;
                if( iterator.size.width <= CX_MAX_INLINE_MAT_OP_SIZE*(int)sizeof(double))
                {
                    do
                    {
                        memcpy( iterator.ptr[1], iterator.ptr[0], iterator.size.width );
                    }
                    while( cxNextNArraySlice( &iterator ));
                }
                else
                {
                    do
                    {
                        icxCopy_8u_C1R( iterator.ptr[0], CX_STUB_STEP,
                                        iterator.ptr[1], CX_STUB_STEP, iterator.size );
                    }
                    while( cxNextNArraySlice( &iterator ));
                }
            }
            else
            {
                CxFunc2D_3A func = (CxFunc2D_3A)(copym_tab.fn_2d[pix_size]);
                if( !func )
                    CX_ERROR( CX_StsUnsupportedFormat, "" );

                do
                {
                    func( iterator.ptr[0], CX_STUB_STEP,
                          iterator.ptr[1], CX_STUB_STEP,
                          iterator.ptr[2], CX_STUB_STEP, iterator.size );
                }
                while( cxNextNArraySlice( &iterator ));
            }
            EXIT;
        }
        else
        {
            int coi1 = 0, coi2 = 0;
            CX_CALL( src = cxGetMat( src, &srcstub, &coi1 ));
            CX_CALL( dst = cxGetMat( dst, &dststub, &coi2 ));

            if( coi1 )
            {
                CxArr* planes[] = { 0, 0, 0, 0 };

                if( maskarr )
                    CX_ERROR( CX_StsBadArg, "COI + mask are not supported" );

                planes[coi1-1] = dst;
                CX_CALL( cxCxtPixToPlane( src, planes[0], planes[1], planes[2], planes[3] ));
                EXIT;
            }
            else if( coi2 )
            {
                CxArr* planes[] = { 0, 0, 0, 0 };
            
                if( maskarr )
                    CX_ERROR( CX_StsBadArg, "COI + mask are not supported" );

                planes[coi2-1] = src;
                CX_CALL( cxCxtPlaneToPix( planes[0], planes[1], planes[2], planes[3], dst ));
                EXIT;
            }
        }
    }

    if( !CX_ARE_TYPES_EQ( src, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedFormats );

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR_FROM_CODE( CX_StsUnmatchedSizes );

    size = cxGetMatSize( src );
    pix_size = CX_ELEM_SIZE(src->type);

    if( !maskarr )
    {
        size.width *= pix_size;
        if( CX_IS_MAT_CONT( src->type & dst->type ))
        {
            size.width *= size.height;

            if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE*
                              CX_MAX_INLINE_MAT_OP_SIZE*(int)sizeof(double))
            {
                memcpy( dst->data.ptr, src->data.ptr, size.width );
                EXIT;
            }

            size.height = 1;
        }

        icxCopy_8u_C1R( src->data.ptr, src->step,
                        dst->data.ptr, dst->step, size );
    }
    else
    {
        CxCopyMaskFunc func;
        CxMat maskstub, *mask = (CxMat*)maskarr;
        int src_step = src->step;
        int dst_step = dst->step;
        int mask_step;

        if( !CX_IS_MAT( mask ))
            CX_CALL( mask = cxGetMat( mask, &maskstub ));
        if( !CX_IS_MASK_ARR( mask ))
            CX_ERROR( CX_StsBadMask, "" );

        if( !inittab )
        {
            icxInitCopyMRTable( &copym_tab );
            inittab = 1;
        }

        if( !CX_ARE_SIZES_EQ( src, mask ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );

        mask_step = mask->step;
        
        if( CX_IS_MAT_CONT( src->type & dst->type & mask->type ))
        {
            size.width *= size.height;
            size.height = 1;
            src_step = dst_step = mask_step = CX_STUB_STEP;
        }

        func = (CxCopyMaskFunc)(copym_tab.fn_2d[pix_size]);
        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, src_step, dst->data.ptr, dst_step,
                         size, mask->data.ptr, mask_step ));
    }

    __END__;
}


/* dst(idx) = value */
CX_IMPL void
cxSet( void* arr, CxScalar value, const void* maskarr )
{
    static CxBtFuncTable setm_tab;
    static int inittab = 0;
    
    CX_FUNCNAME( "cxSet" );

    __BEGIN__;

    CxMat stub, *mat = (CxMat*)arr;
    int pix_size, type;
    double buf[12];
    int mat_step;
    CxSize size;

    if( !CX_IS_MAT(mat))
    {
        if( CX_IS_MATND(mat))
        {
            CxMatND stub;
            CxNArrayIterator iterator;
            int pix_size1;
            
            CX_CALL( cxInitNArrayIterator( 1, &arr, maskarr, &stub, &iterator ));

            type = CX_MAT_TYPE(iterator.hdr[0]->type);
            pix_size = icxPixSize[type];
            pix_size1 = icxPixSize[type & ~CX_MAT_CN_MASK];

            CX_CALL( cxScalarToRawData( &value, buf, type, maskarr == 0 ));

            if( !maskarr )
            {
                iterator.size.width *= pix_size;
                do
                {
                    icxSet_8u_C1R( iterator.ptr[0], CX_STUB_STEP,
                                   iterator.size, buf, pix_size1 );
                }
                while( cxNextNArraySlice( &iterator ));
            }
            else
            {
                CxFunc2D_2A1P func = (CxFunc2D_2A1P)(setm_tab.fn_2d[pix_size]);
                if( !func )
                    CX_ERROR( CX_StsUnsupportedFormat, "" );

                do
                {
                    func( iterator.ptr[0], CX_STUB_STEP,
                          iterator.ptr[1], CX_STUB_STEP,
                          iterator.size, buf );
                }
                while( cxNextNArraySlice( &iterator ));
            }
            EXIT;
        }    
        else
        {
            int coi = 0;
            CX_CALL( mat = cxGetMat( mat, &stub, &coi ));

            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "" );
        }
    }

    type = CX_MAT_TYPE( mat->type );
    pix_size = icxPixSize[type];
    size = cxGetMatSize( mat );
    mat_step = mat->step;

    if( !maskarr )
    {
        if( CX_IS_MAT_CONT( mat->type ))
        {
            size.width *= size.height;
        
            if( size.width <= (int)(CX_MAX_INLINE_MAT_OP_SIZE*sizeof(double)))
            {
                if( type == CX_32FC1 )
                {
                    int* dstdata = (int*)(mat->data.ptr);
                    float val = (float)value.val[0];
                    int ival = (int&)val;

                    do
                    {
                        dstdata[size.width-1] = ival;
                    }
                    while( --size.width );

                    EXIT;
                }

                if( type == CX_64FC1 )
                {
                    int64* dstdata = (int64*)(mat->data.ptr);
                    int64 ival = (int64&)(value.val[0]);

                    do
                    {
                        dstdata[size.width-1] = ival;
                    }
                    while( --size.width );

                    EXIT;
                }
            }

            mat_step = CX_STUB_STEP;
            size.height = 1;
        }
        
        size.width *= pix_size;
        CX_CALL( cxScalarToRawData( &value, buf, type, 1 ));

        IPPI_CALL( icxSet_8u_C1R( mat->data.ptr, mat_step, size, buf,
                                  icxPixSize[type & ~CX_MAT_CN_MASK]));
    }
    else
    {
        CxFunc2D_2A1P func;
        CxMat maskstub, *mask = (CxMat*)maskarr;
        int mask_step;

        CX_CALL( mask = cxGetMat( mask, &maskstub ));

        if( !CX_IS_MASK_ARR( mask ))
            CX_ERROR( CX_StsBadMask, "" );

        if( !inittab )
        {
            icxInitSetMRTable( &setm_tab );
            inittab = 1;
        }

        if( !CX_ARE_SIZES_EQ( mat, mask ))
            CX_ERROR( CX_StsUnmatchedSizes, "" );

        mask_step = mask->step;

        if( CX_IS_MAT_CONT( mat->type & mask->type ))
        {
            size.width *= size.height;
            mat_step = mask_step = CX_STUB_STEP;
            size.height = 1;
        }

        func = (CxFunc2D_2A1P)(setm_tab.fn_2d[pix_size]);
        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        CX_CALL( cxScalarToRawData( &value, buf, type, 0 ));

        IPPI_CALL( func( mat->data.ptr, mat_step, mask->data.ptr,
                         mask_step, size, buf ));
    }

    __END__;
}


/****************************************************************************************\
*                                          Clearing                                      *
\****************************************************************************************/

IPCXAPI_IMPL( CxStatus,
icxSetZero_8u_C1R, ( uchar* dst, int dststep, CxSize size ))
{
    for( ; size.height--; dst += dststep )
    {
        memset( dst, 0, size.width );
    }

    return CX_OK;
}

CX_IMPL void
cxSetZero( CxArr* arr )
{
    CX_FUNCNAME( "cxSetZero" );
    
    __BEGIN__;

    CxMat stub, *mat = (CxMat*)arr;
    CxSize size;
    int mat_step;

    if( !CX_IS_MAT( mat ))
    {
        if( CX_IS_MATND(mat))
        {
            CxMatND stub;
            CxNArrayIterator iterator;
            
            CX_CALL( cxInitNArrayIterator( 1, &arr, 0, &stub, &iterator ));
            iterator.size.width *= icxPixSize[CX_MAT_TYPE(iterator.hdr[0]->type)];

            if( iterator.size.width <= CX_MAX_INLINE_MAT_OP_SIZE*(int)sizeof(double) )
            {
                do
                {
                    memset( iterator.ptr[0], 0, iterator.size.width );
                }
                while( cxNextNArraySlice( &iterator ));
            }
            else
            {
                do
                {
                    icxSetZero_8u_C1R( iterator.ptr[0], CX_STUB_STEP, iterator.size );
                }
                while( cxNextNArraySlice( &iterator ));
            }
            EXIT;
        }    
        else if( CX_IS_SPARSE_MAT(mat))
        {
            CxSparseMat* mat1 = (CxSparseMat*)mat;
            cxClearSet( mat1->heap );
            if( mat1->hashtable )
                memset( mat1->hashtable, 0, mat1->hashsize*sizeof(mat1->hashtable[0]));
            mat1->total = 0;
            EXIT;
        }
        else
        {
            int coi = 0;
            CX_CALL( mat = cxGetMat( mat, &stub, &coi ));
            if( coi != 0 )
                CX_ERROR( CX_BadCOI, "coi is not supported" );
        }
    }

    size = cxGetMatSize( mat );
    size.width *= icxPixSize[CX_MAT_TYPE(mat->type)];
    mat_step = mat->step;

    if( CX_IS_MAT_CONT( mat->type ))
    {
        size.width *= size.height;

        if( size.width <= CX_MAX_INLINE_MAT_OP_SIZE*(int)sizeof(double) )
        {
            memset( mat->data.ptr, 0, size.width );
            EXIT;
        }

        mat_step = CX_STUB_STEP;
        size.height = 1;
    }

    IPPI_CALL( icxSetZero_8u_C1R( mat->data.ptr, mat_step, size ));

    __END__;
}


/****************************************************************************************\
*                                          Flipping                                      *
\****************************************************************************************/

#define ICX_DEF_FLIP_HZ_CASE_C1( arrtype, src, dst, len )           \
    for( i = 0; i < (len)/2; i++ )                                  \
    {                                                               \
        arrtype t0 = (src)[i];                                      \
        arrtype t1 = (src)[(len) - i - 1];                          \
        (dst)[i] = t1;                                              \
        (dst)[(len) - i - 1] = t0;                                  \
    }


#define ICX_DEF_FLIP_HZ_CASE_C3( arrtype, src, dst, len )           \
    for( i = 0; i < (len)/2; i++ )                                  \
    {                                                               \
        arrtype t0 = (src)[i*3];                                    \
        arrtype t1 = (src)[((len) - i)*3 - 3];                      \
        (dst)[i*3] = t1;                                            \
        (dst)[((len) - i)*3 - 3] = t0;                              \
        t0 = (src)[i*3 + 1];                                        \
        t1 = (src)[((len) - i)*3 - 2];                              \
        (dst)[i*3 + 1] = t1;                                        \
        (dst)[((len) - i)*3 - 2] = t0;                              \
        t0 = (src)[i*3 + 2];                                        \
        t1 = (src)[((len) - i)*3 - 1];                              \
        (dst)[i*3 + 2] = t1;                                        \
        (dst)[((len) - i)*3 - 1] = t0;                              \
    }


#define ICX_DEF_FLIP_HZ_CASE_C4( arrtype, src, dst, len )           \
    for( i = 0; i < (len)/2; i++ )                                  \
    {                                                               \
        arrtype t0 = (src)[i*4];                                    \
        arrtype t1 = (src)[((len) - i)*4 - 3];                      \
        (dst)[i*4] = t1;                                            \
        (dst)[((len) - i)*4 - 3] = t0;                              \
        t0 = (src)[i*4];                                            \
        t1 = (src)[((len) - i)*4 - 3];                              \
        (dst)[i*4] = t1;                                            \
        (dst)[((len) - i)*4 - 3] = t0;                              \
        t0 = (src)[i*4 + 1];                                        \
        t1 = (src)[((len) - i)*4 - 2];                              \
        (dst)[i*4 + 1] = t1;                                        \
        (dst)[((len) - i)*4 - 2] = t0;                              \
        t0 = (src)[i*4 + 2];                                        \
        t1 = (src)[((len) - i)*4 - 1];                              \
        (dst)[i*4 + 2] = t1;                                        \
        (dst)[((len) - i)*4 - 1] = t0;                              \
    }


#define ICX_DEF_FLIP_HZ_FUNC( flavor, arrtype, cn )                 \
static CxStatus CX_STDCALL                                          \
icxFlipHorz_##flavor( const arrtype* src, int srcstep,              \
                      arrtype* dst, int dststep, CxSize size )      \
{                                                                   \
    int y, i;                                                       \
    for( y = 0; y < size.height; y++, (char*&)src += srcstep,       \
                                      (char*&)dst += dststep )      \
    {                                                               \
        ICX_DEF_FLIP_HZ_CASE_C##cn( arrtype, src, dst, size.width ) \
    }                                                               \
                                                                    \
    return CX_OK;                                                   \
}


ICX_DEF_FLIP_HZ_FUNC( 8u_C1R, uchar, 1 )
ICX_DEF_FLIP_HZ_FUNC( 8u_C2R, ushort, 1 )
ICX_DEF_FLIP_HZ_FUNC( 8u_C3R, uchar, 3 )
ICX_DEF_FLIP_HZ_FUNC( 16u_C2R, int, 1 )
ICX_DEF_FLIP_HZ_FUNC( 16u_C3R, ushort, 3 )
ICX_DEF_FLIP_HZ_FUNC( 32s_C2R, int64, 1 )
ICX_DEF_FLIP_HZ_FUNC( 32s_C3R, int, 3 )
ICX_DEF_FLIP_HZ_FUNC( 64s_C2R, int, 4 )
ICX_DEF_FLIP_HZ_FUNC( 64s_C3R, int64, 3 )
ICX_DEF_FLIP_HZ_FUNC( 64s_C4R, int64, 4 )

CX_DEF_INIT_PIXSIZE_TAB_2D( FlipHorz, R )


static CxStatus
icxFlipVert_8u_C1R( const uchar* src, int srcstep,
                    uchar* dst, int dststep, CxSize size )
{
    int y, i;
    const uchar* src1 = src + (size.height - 1)*srcstep;
    uchar* dst1 = dst + (size.height - 1)*dststep;

    for( y = 0; y < (size.height + 1)/2; y++, src += srcstep, src1 -= srcstep,
                                              dst += dststep, dst1 -= dststep )
    {
        for( i = 0; i <= size.width - 16; i += 16 )
        {
            int t0 = ((int*)(src + i))[0];
            int t1 = ((int*)(src1 + i))[0];

            ((int*)(dst + i))[0] = t1;
            ((int*)(dst1 + i))[0] = t0;

            t0 = ((int*)(src + i))[1];
            t1 = ((int*)(src1 + i))[1];

            ((int*)(dst + i))[1] = t1;
            ((int*)(dst1 + i))[1] = t0;

            t0 = ((int*)(src + i))[2];
            t1 = ((int*)(src1 + i))[2];

            ((int*)(dst + i))[2] = t1;
            ((int*)(dst1 + i))[2] = t0;

            t0 = ((int*)(src + i))[3];
            t1 = ((int*)(src1 + i))[3];

            ((int*)(dst + i))[3] = t1;
            ((int*)(dst1 + i))[3] = t0;
        }

        for( ; i <= size.width - 4; i += 4 )
        {
            int t0 = ((int*)(src + i))[0];
            int t1 = ((int*)(src1 + i))[0];

            ((int*)(dst + i))[0] = t1;
            ((int*)(dst1 + i))[0] = t0;
        }

        for( ; i < size.width; i++ )
        {
            uchar t0 = src[i];
            uchar t1 = src1[i];

            dst[i] = t1;
            dst1[i] = t0;
        }
    }

    return CX_OK;
}


CX_IMPL void
cxFlip( const CxArr* srcarr, CxArr* dstarr, int flip_mode )
{
    static CxBtFuncTable tab;
    static int inittab = 0;
    
    CX_FUNCNAME( "cxFlip" );
    
    __BEGIN__;

    CxMat sstub, *src = (CxMat*)srcarr;
    CxMat dstub, *dst = (CxMat*)dstarr;
    CxSize size;
    CxFunc2D_2A func = 0;
    int pix_size;

    if( !inittab )
    {
        icxInitFlipHorzRTable( &tab );
        inittab = 1;
    }

    if( !CX_IS_MAT( src ))
    {
        int coi = 0;
        CX_CALL( src = cxGetMat( src, &sstub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "coi is not supported" );
    }

    if( !dst )
        dst = src;
    else if( !CX_IS_MAT( dst ))
    {
        int coi = 0;
        CX_CALL( dst = cxGetMat( dst, &dstub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "coi is not supported" );
    }

    if( !CX_ARE_TYPES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    if( !CX_ARE_SIZES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedSizes, "" );

    size = cxGetMatSize( src );
    pix_size = icxPixSize[CX_MAT_TYPE( src->type )];

    if( flip_mode == 0 )
    {
        size.width *= pix_size;
        
        IPPI_CALL( icxFlipVert_8u_C1R( src->data.ptr, src->step,
                                       dst->data.ptr, dst->step, size ));
    }
    else
    {
        int inplace = src->data.ptr == dst->data.ptr;
        uchar* dst_data = dst->data.ptr;
        int dst_step = dst->step;

        func = (CxFunc2D_2A)(tab.fn_2d[pix_size]);

        if( !func )
            CX_ERROR( CX_StsUnsupportedFormat, "" );

        if( flip_mode < 0 && !inplace )
        {
            dst_data += dst_step * (dst->height - 1);
            dst_step = -dst_step;
        }

        IPPI_CALL( func( src->data.ptr, src->step, dst_data, dst_step, size ));
        
        if( flip_mode < 0 && inplace )
        {
            size.width *= pix_size;
            IPPI_CALL( icxFlipVert_8u_C1R( dst->data.ptr, dst->step,
                                           dst->data.ptr, dst->step, size ));
        }
    }

    __END__;
}


/* cxRepeat */
CX_IMPL void
cxRepeat( const CxArr* srcarr, CxArr* dstarr )
{
    CX_FUNCNAME( "cxRepeat" );
    
    __BEGIN__;

    CxMat sstub, *src = (CxMat*)srcarr;
    CxMat dstub, *dst = (CxMat*)dstarr;
    CxSize srcsize, dstsize;
    int pix_size;
    int x, y, k, l;

    if( !CX_IS_MAT( src ))
    {
        int coi = 0;
        CX_CALL( src = cxGetMat( src, &sstub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "coi is not supported" );
    }

    if( !CX_IS_MAT( dst ))
    {
        int coi = 0;
        CX_CALL( dst = cxGetMat( dst, &dstub, &coi ));
        if( coi != 0 )
            CX_ERROR( CX_BadCOI, "coi is not supported" );
    }

    if( !CX_ARE_TYPES_EQ( src, dst ))
        CX_ERROR( CX_StsUnmatchedFormats, "" );

    srcsize = cxGetMatSize( src );
    dstsize = cxGetMatSize( dst );
    pix_size = icxPixSize[CX_MAT_TYPE( src->type )];

    for( y = 0, k = 0; y < dstsize.height; y++ )
    {
        for( x = 0; x < dstsize.width; x += srcsize.width )
        {
            l = srcsize.width;
            if( l > dstsize.width - x )
                l = dstsize.width - x;
            memcpy( dst->data.ptr + y*dst->step + x*pix_size,
                    src->data.ptr + k*src->step + x*pix_size, l*pix_size );
        }
        if( ++k == srcsize.height )
            k = 0;
    }

    __END__;
}

/* End of file. */

