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

#define ICX_FREE_PTR(storage)  \
    ((char*)(storage)->top + (storage)->block_size - (storage)->free_space)

#define ICX_ALIGNED_SEQ_BLOCK_SIZE  \
    (((int)sizeof(CxSeqBlock) + CX_STRUCT_ALIGN - 1) & -CX_STRUCT_ALIGN)

#define CX_GET_LAST_ELEM( seq, block ) \
    ((block)->data + ((block)->count - 1)*((seq)->elem_size))

#define CX_SWAP_ELEMS(a,b)            \
{                                     \
    int k;                            \
    for( k = 0; k < elem_size; k++ )  \
    {                                 \
        char t0 = (a)[k];             \
        char t1 = (b)[k];             \
        (a)[k] = t1;                  \
        (b)[k] = t0;                  \
    }                                 \
}

/****************************************************************************************\
*            Functions for manipulating memory storage - list of memory blocks           *
\****************************************************************************************/

/* initializes allocated storage */
void
icxInitMemStorage( CxMemStorage* storage, int block_size )
{
    CX_FUNCNAME( "icxInitMemStorage " );
    
    __BEGIN__;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "" );

    if( block_size <= 0 )
        block_size = CX_STORAGE_BLOCK_SIZE;

    block_size = cxAlign( block_size, CX_STRUCT_ALIGN );
    assert( sizeof(CxMemBlock) % CX_STRUCT_ALIGN == 0 );

    memset( storage, 0, sizeof( *storage ));
    storage->signature = CX_STORAGE_MAGIC_VAL;
    storage->block_size = block_size;

    __END__;
}

/* initializes child memory storage */
void
icxInitChildMemStorage( CxMemStorage* parent, CxMemStorage* storage )
{
    CX_FUNCNAME( "icxInitChildMemStorage" );

    __BEGIN__;

    if( !storage || !parent )
        CX_ERROR( CX_StsNullPtr, "" );

    CX_CALL( icxInitMemStorage( storage, parent->block_size ));
    storage->parent = parent;

    __END__;
}


/* creates root memory storage */
CX_IMPL CxMemStorage*
cxCreateMemStorage( int block_size )
{
    CxMemStorage *storage = 0;

    CX_FUNCNAME( "cxCreateMemStorage" );

    __BEGIN__;

    CX_CALL( storage = (CxMemStorage *)cxAlloc( sizeof( CxMemStorage )));
    CX_CALL( icxInitMemStorage( storage, block_size ));

    __END__;

    if( cxGetErrStatus() < 0 )
        cxFree( (void**)&storage );

    return storage;
}


/* creates child memory storage */
CX_IMPL CxMemStorage *
cxCreateChildMemStorage( CxMemStorage * parent )
{
    CxMemStorage *storage = 0;
    CX_FUNCNAME( "cxCreateChildMemStorage" );

    __BEGIN__;

    if( !parent )
        CX_ERROR( CX_StsNullPtr, "" );

    CX_CALL( storage = cxCreateMemStorage(parent->block_size));
    storage->parent = parent;

    __END__;

    if( cxGetErrStatus() < 0 )
        cxFree( (void**)&storage );

    return storage;
}


/* releases all blocks of the storage (or returns them to parent if any) */
void
icxDestroyMemStorage( CxMemStorage* storage )
{
    CX_FUNCNAME( "icxDestroyMemStorage" );

    __BEGIN__;

    int k = 0;

    CxMemBlock *block;
    CxMemBlock *dst_top = 0;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "" );

    if( storage->parent )
        dst_top = storage->parent->top;

    for( block = storage->bottom; block != 0; k++ )
    {
        CxMemBlock *temp = block;

        block = block->next;
        if( storage->parent )
        {
            if( dst_top )
            {
                temp->prev = dst_top;
                temp->next = dst_top->next;
                if( temp->next )
                    temp->next->prev = temp;
                dst_top = dst_top->next = temp;
            }
            else
            {
                dst_top = storage->parent->bottom = storage->parent->top = temp;
                temp->prev = temp->next = 0;
                storage->free_space = storage->block_size - sizeof( *temp );
            }
        }
        else
        {
            cxFree( (void**)&temp );
        }
    }

    storage->top = storage->bottom = 0;
    storage->free_space = 0;

    __END__;
}


/* releases memory storage */
CX_IMPL void
cxReleaseMemStorage( CxMemStorage** storage )
{
    CxMemStorage *st;
    CX_FUNCNAME( "cxReleaseMemStorage" );

    __BEGIN__;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "" );

    st = *storage;
    *storage = 0;

    if( st )
    {
        CX_CALL( icxDestroyMemStorage( st ));
        cxFree( (void**)&st );
    }

    __END__;
}


/* clears memory storage (returns blocks to the parent if any) */
CX_IMPL void
cxClearMemStorage( CxMemStorage * storage )
{
    CX_FUNCNAME( "cxClearMemStorage" );

    __BEGIN__;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "" );

    if( storage->parent )
    {
        icxDestroyMemStorage( storage );
    }
    else
    {
        storage->top = storage->bottom;
        storage->free_space = storage->bottom ? storage->block_size - sizeof(CxMemBlock) : 0;
    }

    __END__;
}


/* moves stack pointer to next block.
   If no blocks, allocate new one and link it to the storage */
static void
icxGoNextMemBlock( CxMemStorage * storage )
{
    CX_FUNCNAME( "icxGoNextMemBlock" );
    
    __BEGIN__;
    
    if( !storage )
        CX_ERROR( CX_StsNullPtr, "" );

    if( !storage->top || !storage->top->next )
    {
        CxMemBlock *block;

        if( !(storage->parent) )
        {
            CX_CALL( block = (CxMemBlock *)cxAlloc( storage->block_size ));
        }
        else
        {
            CxMemStorage *parent = storage->parent;
            CxMemStoragePos parent_pos;

            cxSaveMemStoragePos( parent, &parent_pos );
            CX_CALL( icxGoNextMemBlock( parent ));

            block = parent->top;
            cxRestoreMemStoragePos( parent, &parent_pos );

            if( block == parent->top )  /* the single allocated block */
            {
                assert( parent->bottom == block );
                parent->top = parent->bottom = 0;
                parent->free_space = 0;
            }
            else
            {
                /* cut the block from the parent's list of blocks */
                parent->top->next = block->next;
                if( block->next )
                    block->next->prev = parent->top;
            }
        }

        /* link block */
        block->next = 0;
        block->prev = storage->top;

        if( storage->top )
            storage->top->next = block;
        else
            storage->top = storage->bottom = block;
    }

    if( storage->top->next )
        storage->top = storage->top->next;
    storage->free_space = storage->block_size - sizeof(CxMemBlock);
    assert( storage->free_space % CX_STRUCT_ALIGN == 0 );

    __END__;
}


/* remembers memory storage position */
CX_IMPL void
cxSaveMemStoragePos( const CxMemStorage * storage, CxMemStoragePos * pos )
{
    CX_FUNCNAME( "cxSaveMemStoragePos" );

    __BEGIN__;

    if( !storage || !pos )
        CX_ERROR( CX_StsNullPtr, "" );

    pos->top = storage->top;
    pos->free_space = storage->free_space;

    __END__;
}


/* restores memory storage position */
CX_IMPL void
cxRestoreMemStoragePos( CxMemStorage * storage, CxMemStoragePos * pos )
{
    CX_FUNCNAME( "cxRestoreMemStoragePos" );

    __BEGIN__;

    if( !storage || !pos )
        CX_ERROR( CX_StsNullPtr, "" );
    if( pos->free_space > storage->block_size )
        CX_ERROR_FROM_STATUS( CX_BADSIZE_ERR );

    storage->top = pos->top;
    storage->free_space = pos->free_space;

    if( !storage->top )
    {
        storage->top = storage->bottom;
        storage->free_space = storage->top ? storage->block_size - sizeof(CxMemBlock) : 0;
    }

    __END__;
}


/* Allocates continuous buffer of the specified size in the storage */
CX_IMPL  void*
cxMemStorageAlloc( CxMemStorage* storage, int size )
{
    char *ptr = 0;
    
    CX_FUNCNAME( "cxMemStorageAlloc" );

    __BEGIN__;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "NULL storage pointer" );

    assert( storage->free_space % CX_STRUCT_ALIGN == 0 );

    if( (unsigned)storage->free_space < (unsigned)size )
    {
        int max_free_space =
            (storage->block_size - (int)sizeof(CxMemBlock)) & -CX_STRUCT_ALIGN;

        if( (unsigned)max_free_space < (unsigned)size )
            CX_ERROR( CX_StsOutOfRange, "requested size is negative or too big" );

        CX_CALL( icxGoNextMemBlock( storage ));
    }

    ptr = ICX_FREE_PTR(storage);
    assert( (long)ptr % CX_STRUCT_ALIGN == 0 );
    storage->free_space = (storage->free_space - size) & -CX_STRUCT_ALIGN;

    __END__;

    return ptr;
}


/****************************************************************************************\
*                               Sequence implementation                                  *
\****************************************************************************************/

/* creates empty sequence */
CX_IMPL CxSeq *
cxCreateSeq( int seq_flags, int header_size, int elem_size, CxMemStorage * storage )
{
    CxSeq *seq = 0;

    CX_FUNCNAME( "cxCreateSeq" );

    __BEGIN__;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "" );
    if( header_size < (int)sizeof( CxSeq ) || elem_size <= 0 )
        CX_ERROR( CX_StsBadSize, "" );

    /* allocate sequence header */
    CX_CALL( seq = (CxSeq*)cxMemStorageAlloc( storage, header_size ));
    memset( seq, 0, header_size );

    seq->header_size = header_size;
    seq->flags = (seq_flags & ~CX_MAGIC_MASK) | CX_SEQ_MAGIC_VAL;
    {
        int elemtype = CX_MAT_TYPE(seq_flags);
        int typesize = icxPixSize[elemtype];

        if( elemtype != CX_SEQ_ELTYPE_GENERIC &&
            typesize != 0 && typesize != elem_size )
            CX_ERROR( CX_StsBadSize,
            "Element size doesn't match to the size of predefined element type "
            "(try to use 0 for sequence element type)" );
    }
    seq->elem_size = elem_size;
    seq->storage = storage;

    CX_CALL( cxSetSeqBlockSize( seq, (1 << 10)/elem_size ));

    __END__;

    return seq;
}


/* adjusts <delta_elems> field of sequence. It determines how much the sequence
   grows if there are no free space inside the sequence buffers */
CX_IMPL void
cxSetSeqBlockSize( CxSeq *seq, int delta_elements )
{
    int elem_size;
    int useful_block_size;

    CX_FUNCNAME( "cxSetSeqBlockSize" );

    __BEGIN__;

    if( !seq || !seq->storage )
        CX_ERROR( CX_StsNullPtr, "" );
    if( delta_elements < 0 )
        CX_ERROR( CX_StsOutOfRange, "" );

    useful_block_size = (seq->storage->block_size - sizeof(CxMemBlock) -
                        sizeof(CxSeqBlock)) & -CX_STRUCT_ALIGN;
    elem_size = seq->elem_size;

    if( delta_elements == 0 )
        delta_elements = (1 << 10) / elem_size;
    if( delta_elements * elem_size > useful_block_size )
    {
        delta_elements = useful_block_size / elem_size;
        if( delta_elements == 0 )
            CX_ERROR( CX_StsOutOfRange, "Storage block size is too small "
                                        "to fit the sequence elements" );
    }

    seq->delta_elems = delta_elements;

    __END__;
}


/* finds sequence element by its index */
CX_IMPL char*
cxGetSeqElem( CxSeq *seq, int index, CxSeqBlock **_block )
{
    CxSeqBlock *block;
    char *elem = 0;
    int count, total;

    CX_FUNCNAME( "cxGetSeqElem" );

    __BEGIN__;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "" );
    total = seq->total;
    index += index < 0 ? total : 0;
    index -= index >= total ? total : 0;

    if( (unsigned)index < (unsigned)total )
    {
        block = seq->first;
        while( index >= (count = block->count) )
        {
            index -= count;
            block = block->next;
        }

        if( _block )
            *_block = block;
        elem = block->data + index * seq->elem_size;
    }

    __END__;

    return elem;
}


/* calculates index of sequence element */
CX_IMPL int
cxSeqElemIdx( const CxSeq* seq, const void* _element, CxSeqBlock** _block )
{
    const char *element = (const char *)_element;
    int elem_size;
    int id = -1;
    CxSeqBlock *first_block;
    CxSeqBlock *block;

    CX_FUNCNAME( "cxSeqElemIdx" );

    __BEGIN__;

    if( !seq || !element )
        CX_ERROR( CX_StsNullPtr, "" );

    block = first_block = seq->first;
    elem_size = seq->elem_size;

    for( ;; )
    {
        if( (unsigned)(element - block->data) < (unsigned) (block->count * elem_size) )
        {
            if( _block )
                *_block = block;
            id = (element - block->data) / elem_size +
                block->start_index - seq->first->start_index;
            break;
        }
        block = block->next;
        if( block == first_block )
            break;
    }

    __END__;

    return id;
}


CX_IMPL int
cxSliceLength( CxSlice slice, CxSeq* seq )
{
    int total = seq->total;
    int length;
    if( slice.startIndex < 0 )
        slice.startIndex += total;
    if( slice.endIndex <= 0 )
        slice.endIndex += total;
    length = slice.endIndex - slice.startIndex;
    if( length < 0 )
    {
        length += total;
        /*if( length < 0 )
            length += total;*/
    }
    else if( length > total )
        length = total;

    return length;
};


/* copies all the sequence elements into single continuous array */
CX_IMPL void*
cxCxtSeqToArray( CxSeq *seq, void *array, CxSlice slice )
{
    int elem_size, total;
    CxSeqBlock *block;
    char *dstPtr = (char*)array;
    char *ptr = 0;

    CX_FUNCNAME( "cxCxtSeqToArray" );

    __BEGIN__;

    if( !seq || !array )
        CX_ERROR( CX_StsNullPtr, "" );

    elem_size = seq->elem_size;
    total = cxSliceLength( slice, seq )*elem_size;
    ptr = cxGetSeqElem( seq, slice.startIndex, &block );

    if( !ptr )
        CX_ERROR_FROM_STATUS( CX_BADRANGE_ERR );

    while( total > 0 )
    {
        int count = block->data + block->count*elem_size - ptr;
        if( count > total )
            count = total;

        memcpy( dstPtr, ptr, count );
        dstPtr += count;
        total -= count;
        block = block->next;
        ptr = block->data;
    }

    __END__;

    return array;
}


/* constructs sequence from array without copying any data.
   the resultant sequence can't grow above its initial size */
CX_IMPL CxSeq*
cxMakeSeqHeaderForArray( int seq_flags, int header_size, int elem_size,
                         void *array, int total, CxSeq *seq, CxSeqBlock * block )
{
    CxSeq* result = 0;
    
    CX_FUNCNAME( "cxMakeSeqHeaderForArray" );

    __BEGIN__;

    if( elem_size <= 0 || header_size < (int)sizeof( CxSeq ) || total < 0 )
        CX_ERROR_FROM_STATUS( CX_BADSIZE_ERR );

    if( !seq || ((!array || !block) && total > 0) )
        CX_ERROR( CX_StsNullPtr, "" );

    memset( seq, 0, header_size );

    seq->header_size = header_size;
    seq->flags = (seq_flags & ~CX_MAGIC_MASK) | CX_SEQ_MAGIC_VAL;
    {
        int elemtype = CX_MAT_TYPE(seq_flags);
        int typesize = icxPixSize[elemtype];

        if( elemtype != CX_SEQ_ELTYPE_GENERIC &&
            typesize != 0 && typesize != elem_size )
            CX_ERROR( CX_StsBadSize,
            "Element size doesn't match to the size of predefined element type "
            "(try to use 0 for sequence element type)" );
    }
    seq->elem_size = elem_size;
    seq->total = total;
    seq->block_max = seq->ptr = (char *) array + total * elem_size;

    if( total > 0 )
    {
        seq->first = block;
        block->prev = block->next = block;
        block->start_index = 0;
        block->count = total;
        block->data = (char *) array;
    }

    result = seq;

    __END__;

    return result;
}


/* tries to allocate space for at least single sequence element.
   if the sequence has released blocks (seq->free_blocks != 0),
   they are used, else additional space is allocated in the storage */
static void
icxGrowSeq( CxSeq *seq, int in_front_of )
{
    CX_FUNCNAME( "icxGrowSeq" );

    __BEGIN__;

    CxSeqBlock *free_blocks;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "" );
    free_blocks = seq->free_blocks;

    if( !free_blocks )
    {
        int elem_size = seq->elem_size;
        int delta_elems = seq->delta_elems;
        CxMemStorage *storage = seq->storage;

        if( !storage )
            CX_ERROR( CX_StsNullPtr, "The sequence has NULL storage pointer" );

        /* if there is a free space just after last allocated block
           and it's big enough then enlarge the last block
           (this can happen only if the new block is added to the end of sequence */
        if( (unsigned)(ICX_FREE_PTR(storage) - seq->block_max) < CX_STRUCT_ALIGN &&
            storage->free_space >= seq->elem_size && !in_front_of )
        {
            int delta = storage->free_space / elem_size;

            delta = MIN( delta, delta_elems ) * elem_size;
            seq->block_max += delta;
            storage->free_space = (((char*)storage->top + storage->block_size) -
                                   seq->block_max) & -CX_STRUCT_ALIGN;
            EXIT;
        }
        else
        {
            int delta = elem_size * delta_elems + ICX_ALIGNED_SEQ_BLOCK_SIZE;

            /* try to allocate <delta_elements> elements */
            if( storage->free_space < delta )
            {
                int small_block_size = MAX(1, delta_elems/3)*elem_size +
                                       ICX_ALIGNED_SEQ_BLOCK_SIZE;
                /* try to allocate smaller part */
                if( storage->free_space >= small_block_size + CX_STRUCT_ALIGN )
                {
                    delta = ((storage->free_space - ICX_ALIGNED_SEQ_BLOCK_SIZE)/
                            seq->elem_size)*seq->elem_size + ICX_ALIGNED_SEQ_BLOCK_SIZE;
                }
                else
                {
                    CX_CALL( icxGoNextMemBlock( storage ));
                    assert( storage->free_space >= delta );
                }
            }

            CX_CALL( free_blocks = (CxSeqBlock*)cxMemStorageAlloc( storage, delta ));
            free_blocks->data = (char*)cxAlignPtr( free_blocks + 1, CX_STRUCT_ALIGN );
            free_blocks->count = delta - ICX_ALIGNED_SEQ_BLOCK_SIZE;
            free_blocks->prev = free_blocks->next = 0;
        }
    }
    else
    {
        seq->free_blocks = free_blocks->next;
    }

    if( !(seq->first) )
    {
        seq->first = free_blocks;
        free_blocks->prev = free_blocks->next = free_blocks;
    }
    else
    {
        free_blocks->prev = seq->first->prev;
        free_blocks->next = seq->first;
        free_blocks->prev->next = free_blocks->next->prev = free_blocks;
    }

    /* for free blocks the <count> field means total number of bytes in the block.
       And for used blocks it means a current number of sequence
       elements in the block */
    assert( free_blocks->count % seq->elem_size == 0 && free_blocks->count > 0 );

    if( !in_front_of )
    {
        seq->ptr = free_blocks->data;
        seq->block_max = free_blocks->data + free_blocks->count;
        free_blocks->start_index = free_blocks == free_blocks->prev ? 0 :
            free_blocks->prev->start_index + free_blocks->prev->count;
    }
    else
    {
        int delta = free_blocks->count / seq->elem_size;
        free_blocks->data += free_blocks->count;

        if( free_blocks != free_blocks->prev )
        {
            assert( seq->first->start_index == 0 );
            seq->first = free_blocks;
        }
        else
        {
            seq->block_max = seq->ptr = free_blocks->data;
        }

        free_blocks->start_index = 0;

        for( ;; )
        {
            free_blocks->start_index += delta;
            free_blocks = free_blocks->next;
            if( free_blocks == seq->first )
                break;
        }
    }

    free_blocks->count = 0;

    __END__;
}

/* recycles a sequence block for the further use */
static void
icxFreeSeqBlock( CxSeq *seq, int in_front_of )
{
    /*CX_FUNCNAME( "icxFreeSeqBlock" );*/

    __BEGIN__;

    CxSeqBlock *block = seq->first;

    assert( (in_front_of ? block : block->prev)->count == 0 );

    if( block == block->prev )  /* single block case */
    {
        block->count = (seq->block_max - block->data) + block->start_index * seq->elem_size;
        block->data = seq->block_max - block->count;
        seq->first = 0;
        seq->ptr = seq->block_max = 0;
        seq->total = 0;
    }
    else
    {
        if( !in_front_of )
        {
            block = block->prev;
            assert( seq->ptr == block->data );

            block->count = seq->block_max - seq->ptr;
            seq->block_max = seq->ptr = block->prev->data +
                block->prev->count * seq->elem_size;
        }
        else
        {
            int delta = block->start_index;

            block->count = delta * seq->elem_size;
            block->data -= block->count;

            /* update start indices of sequence blocks */
            for( ;; )
            {
                block->start_index -= delta;
                block = block->next;
                if( block == seq->first )
                    break;
            }

            seq->first = block->next;
        }

        block->prev->next = block->next;
        block->next->prev = block->prev;
    }

    assert( block->count > 0 && block->count % seq->elem_size == 0 );
    block->next = seq->free_blocks;
    seq->free_blocks = block;

    __END__;
}


/****************************************************************************************\
*                             Sequence Writer implementation                             *
\****************************************************************************************/

/* initializes sequence writer */
CX_IMPL void
cxStartAppendToSeq( CxSeq *seq, CxSeqWriter * writer )
{
    CX_FUNCNAME( "cxStartAppendToSeq" );

    __BEGIN__;

    if( !seq || !writer )
        CX_ERROR( CX_StsNullPtr, "" );

    memset( writer, 0, sizeof( *writer ));
    writer->header_size = sizeof( CxSeqWriter );

    writer->seq = seq;
    writer->block = seq->first ? seq->first->prev : 0;
    writer->ptr = seq->ptr;
    writer->block_max = seq->block_max;

    __END__;
}


/* initializes sequence writer */
CX_IMPL void
cxStartWriteSeq( int seq_flags, int header_size,
                 int elem_size, CxMemStorage * storage, CxSeqWriter * writer )
{
    CxSeq *seq = 0;

    CX_FUNCNAME( "cxStartWriteSeq" );

    __BEGIN__;

    if( !storage || !writer )
        CX_ERROR( CX_StsNullPtr, "" );

    CX_CALL( seq = cxCreateSeq( seq_flags, header_size, elem_size, storage ));
    cxStartAppendToSeq( seq, writer );

    __END__;
}


/* updates sequence header */
CX_IMPL void
cxFlushSeqWriter( CxSeqWriter * writer )
{
    CxSeq *seq = 0;

    CX_FUNCNAME( "cxFlushSeqWriter" );

    __BEGIN__;

    if( !writer )
        CX_ERROR( CX_StsNullPtr, "" );

    seq = writer->seq;
    seq->ptr = writer->ptr;

    if( writer->block )
    {
        int total = 0;
        CxSeqBlock *first_block = writer->seq->first;
        CxSeqBlock *block = first_block;

        writer->block->count = (writer->ptr - writer->block->data) / seq->elem_size;
        assert( writer->block->count > 0 );

        do
        {
            total += block->count;
            block = block->next;
        }
        while( block != first_block );

        writer->seq->total = total;
    }

    __END__;
}


/* calls icxFlushSeqWriter and finishes writing process */
CX_IMPL CxSeq *
cxEndWriteSeq( CxSeqWriter * writer )
{
    CxSeq *seq = 0;

    CX_FUNCNAME( "cxEndWriteSeq" );

    __BEGIN__;

    if( !writer )
        CX_ERROR( CX_StsNullPtr, "" );

    CX_CALL( cxFlushSeqWriter( writer ));

    /* truncate the last block */
    if( writer->block && writer->seq->storage )
    {
        CxSeq *seq = writer->seq;
        CxMemStorage *storage = seq->storage;
        char *storage_block_max = (char *) storage->top + storage->block_size;

        assert( writer->block->count > 0 );

        if( (unsigned)((storage_block_max - storage->free_space)
            - seq->block_max) < CX_STRUCT_ALIGN )
        {
            storage->free_space = (storage_block_max - seq->ptr) & -CX_STRUCT_ALIGN;
            seq->block_max = seq->ptr;
        }
    }

    seq = writer->seq;

    /*writer->seq = 0; */
    writer->ptr = 0;

    __END__;

    return seq;
}


/* creates new sequence block */
CX_IMPL void
cxCreateSeqBlock( CxSeqWriter * writer )
{
    CX_FUNCNAME( "cxCreateSeqBlock" );

    __BEGIN__;

    CxSeq *seq;

    if( !writer || !writer->seq )
        CX_ERROR( CX_StsNullPtr, "" );

    seq = writer->seq;

    cxFlushSeqWriter( writer );

    CX_CALL( icxGrowSeq( seq, 0 ));

    writer->block = seq->first->prev;
    writer->ptr = seq->ptr;
    writer->block_max = seq->block_max;

    __END__;
}


/****************************************************************************************\
*                               Sequence Reader implementation                           *
\****************************************************************************************/

/* initializes sequence reader */
CX_IMPL void
cxStartReadSeq( const CxSeq *seq, CxSeqReader * reader, int reverse )
{
    CxSeqBlock *first_block;
    CxSeqBlock *last_block;

    CX_FUNCNAME( "cxStartReadSeq" );

    __BEGIN__;

    if( !seq || !reader )
        CX_ERROR( CX_StsNullPtr, "" );

    reader->header_size = sizeof( CxSeqReader );
    reader->seq = (CxSeq*)seq;

    first_block = seq->first;

    if( first_block )
    {
        last_block = first_block->prev;
        reader->ptr = first_block->data;
        reader->prev_elem = CX_GET_LAST_ELEM( seq, last_block );
        reader->delta_index = seq->first->start_index;

        if( reverse )
        {
            char *temp = reader->ptr;

            reader->ptr = reader->prev_elem;
            reader->prev_elem = temp;

            reader->block = last_block;
        }
        else
        {
            reader->block = first_block;
        }

        reader->block_min = reader->block->data;
        reader->block_max = reader->block_min + reader->block->count * seq->elem_size;
    }
    else
    {
        reader->delta_index = 0;
        reader->block = 0;

        reader->ptr = reader->prev_elem = reader->block_min = reader->block_max = 0;
    }

    __END__;
}


/* changes the current reading block to the previous or to the next */
CX_IMPL void
cxChangeSeqBlock( CxSeqReader * reader, int direction )
{
    CX_FUNCNAME( "cxChangeSeqBlock" );

    __BEGIN__;
    
    if( !reader )
        CX_ERROR( CX_StsNullPtr, "" );

    if( direction > 0 )
    {
        reader->block = reader->block->next;
        reader->ptr = reader->block->data;
    }
    else
    {
        reader->block = reader->block->prev;
        reader->ptr = CX_GET_LAST_ELEM( reader->seq, reader->block );
    }
    reader->block_min = reader->block->data;
    reader->block_max = reader->block_min + reader->block->count * reader->seq->elem_size;

    __END__;
}


/* returns the current reader position */
CX_IMPL int
cxGetSeqReaderPos( CxSeqReader * reader )
{
    int elem_size;
    int index = -1;

    CX_FUNCNAME( "cxGetSeqReaderPos" );

    __BEGIN__;

    if( !reader || !reader->ptr )
        CX_ERROR( CX_StsNullPtr, "" );

    elem_size = reader->seq->elem_size;
    if( elem_size == 8 )
    {
        index = (reader->ptr - reader->block_min) >> 3;
    }
    else if( elem_size == 4 )
    {
        index = (reader->ptr - reader->block_min) >> 2;
    }
    else if( elem_size == 1 )
    {
        index = reader->ptr - reader->block_min;
    }
    else
    {
        index = (reader->ptr - reader->block_min) / elem_size;
    }

    index += reader->block->start_index - reader->delta_index;

    __END__;

    return index;
}


/* sets reader position to given absolute or relative
   (relatively to the current one) position */
CX_IMPL void
cxSetSeqReaderPos( CxSeqReader * reader, int index, int is_relative )
{
    int total;
    CxSeqBlock *block;
    int idx, elem_size;

    CX_FUNCNAME( "cxSetSeqReaderPos" );

    __BEGIN__;

    if( !reader )
        CX_ERROR( CX_StsNullPtr, "" );

    total = reader->seq->total;

    if( is_relative )
        index += cxGetSeqReaderPos( reader );

    if( index < 0 )
        index += total;
    if( index >= total )
        index -= total;
    if( (unsigned) index >= (unsigned) total )
        CX_ERROR_FROM_STATUS( CX_BADRANGE_ERR );

    elem_size = reader->seq->elem_size;

    block = reader->block;
    idx = index - block->start_index + reader->delta_index;

    if( (unsigned) idx < (unsigned) block->count )
    {
        reader->ptr = block->data + idx * elem_size;
    }
    else
    {
        reader->ptr = cxGetSeqElem( reader->seq, index, &block );
        assert( reader->ptr && block );

        reader->block = block;
        reader->block_min = block->data;
        reader->block_max = CX_GET_LAST_ELEM( reader->seq, block ) + elem_size;
    }

    __END__;
}


/* pushes element to the sequence */
CX_IMPL char*
cxSeqPush( CxSeq *seq, void *element )
{
    char *ptr = 0;
    int elem_size;

    CX_FUNCNAME( "cxSeqPush" );

    __BEGIN__;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "" );

    elem_size = seq->elem_size;
    ptr = seq->ptr;

    if( ptr >= seq->block_max )
    {
        CX_CALL( icxGrowSeq( seq, 0 ));

        ptr = seq->ptr;
        assert( ptr + elem_size <= seq->block_max /*&& ptr == seq->block_min */  );
    }

    if( element )
        memcpy( ptr, element, elem_size );
    seq->first->prev->count++;
    seq->total++;
    seq->ptr = ptr + elem_size;

    __END__;

    return ptr;
}


/* pops the last element out of the sequence */
CX_IMPL void
cxSeqPop( CxSeq *seq, void *element )
{
    char *ptr;
    int elem_size;

    CX_FUNCNAME( "cxSeqPop" );

    __BEGIN__;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "" );
    if( seq->total <= 0 )
        CX_ERROR_FROM_STATUS( CX_BADSIZE_ERR );

    elem_size = seq->elem_size;
    seq->ptr = ptr = seq->ptr - elem_size;

    if( element )
        memcpy( element, ptr, elem_size );
    seq->ptr = ptr;
    seq->total--;

    if( --(seq->first->prev->count) == 0 )
    {
        icxFreeSeqBlock( seq, 0 );
        assert( seq->ptr == seq->block_max );
    }

    __END__;
}


/* pushes element to the front of the sequence */
CX_IMPL char*
cxSeqPushFront( CxSeq *seq, void *element )
{
    char* ptr = 0;
    int elem_size;
    CxSeqBlock *block;

    CX_FUNCNAME( "cxSeqPushFront" );

    __BEGIN__;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "" );

    elem_size = seq->elem_size;
    block = seq->first;

    if( !block || block->start_index == 0 )
    {
        CX_CALL( icxGrowSeq( seq, 1 ));

        block = seq->first;
        assert( block->start_index > 0 );
    }

    ptr = block->data -= elem_size;

    if( element )
        memcpy( ptr, element, elem_size );
    block->count++;
    block->start_index--;
    seq->total++;

    __END__;

    return ptr;
}


/* pulls out the first element of the sequence */
CX_IMPL void
cxSeqPopFront( CxSeq *seq, void *element )
{
    int elem_size;
    CxSeqBlock *block;

    CX_FUNCNAME( "cxSeqPopFront" );

    __BEGIN__;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "" );
    if( seq->total <= 0 )
        CX_ERROR_FROM_STATUS( CX_BADSIZE_ERR );

    elem_size = seq->elem_size;
    block = seq->first;

    if( element )
        memcpy( element, block->data, elem_size );
    block->data += elem_size;
    block->start_index++;
    seq->total--;

    if( --(block->count) == 0 )
    {
        icxFreeSeqBlock( seq, 1 );
    }

    __END__;
}

/* inserts new element in the middle of the sequence */
CX_IMPL char*
cxSeqInsert( CxSeq *seq, int before_index, void *element )
{
    int elem_size;
    int block_size;
    CxSeqBlock *block;
    int delta_index;
    int total;
    char* ret_ptr = 0;

    CX_FUNCNAME( "cxSeqInsert" );

    __BEGIN__;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "" );

    total = seq->total;
    before_index += before_index < 0 ? total : 0;
    before_index -= before_index > total ? total : 0;

    if( (unsigned)before_index > (unsigned)total )
        CX_ERROR_FROM_STATUS( CX_BADRANGE_ERR );

    if( before_index == total )
    {
        CX_CALL( ret_ptr = cxSeqPush( seq, element ));
    }
    else if( before_index == 0 )
    {
        CX_CALL( ret_ptr = cxSeqPushFront( seq, element ));
    }
    else
    {
        elem_size = seq->elem_size;

        if( before_index >= total >> 1 )
        {
            char *ptr = seq->ptr + elem_size;

            if( ptr > seq->block_max )
            {
                CX_CALL( icxGrowSeq( seq, 0 ));

                ptr = seq->ptr + elem_size;
                assert( ptr <= seq->block_max );
            }

            delta_index = seq->first->start_index;
            block = seq->first->prev;
            block->count++;
            block_size = ptr - block->data;

            while( before_index < block->start_index - delta_index )
            {
                CxSeqBlock *prev_block = block->prev;

                memmove( block->data + elem_size, block->data, block_size - elem_size );
                block_size = prev_block->count * elem_size;
                memcpy( block->data, prev_block->data + block_size - elem_size, elem_size );
                block = prev_block;

                /* check that we don't fall in the infinite loop */
                assert( block != seq->first->prev );
            }

            before_index = (before_index - block->start_index + delta_index) * elem_size;
            memmove( block->data + before_index + elem_size, block->data + before_index,
                     block_size - before_index - elem_size );

            ret_ptr = block->data + before_index;

            if( element )
                memcpy( ret_ptr, element, elem_size );
            seq->ptr = ptr;
        }
        else
        {
            block = seq->first;

            if( block->start_index == 0 )
            {
                CX_CALL( icxGrowSeq( seq, 1 ));

                block = seq->first;
            }

            delta_index = block->start_index;
            block->count++;
            block->start_index--;
            block->data -= elem_size;

            while( before_index > block->start_index - delta_index + block->count )
            {
                CxSeqBlock *next_block = block->next;

                block_size = block->count * elem_size;
                memmove( block->data, block->data + elem_size, block_size - elem_size );
                memcpy( block->data + block_size - elem_size, next_block->data, elem_size );
                block = next_block;
                /* check that we don't fall in the infinite loop */
                assert( block != seq->first );
            }

            before_index = (before_index - block->start_index + delta_index) * elem_size;
            memmove( block->data, block->data + elem_size, before_index - elem_size );

            ret_ptr = block->data + before_index - elem_size;

            if( element )
                memcpy( ret_ptr, element, elem_size );
        }

        seq->total = total + 1;
    }

    __END__;

    return ret_ptr;
}


/* removes element from the sequence */
CX_IMPL void
cxSeqRemove( CxSeq *seq, int index )
{
    char *ptr;
    int elem_size;
    int block_size;
    CxSeqBlock *block;
    int delta_index;
    int total, front = 0;

    CX_FUNCNAME( "cxSeqRemove" );

    __BEGIN__;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "" );

    total = seq->total;

    index += index < 0 ? total : 0;
    index -= index >= total ? total : 0;

    if( (unsigned) index >= (unsigned) total )
        CX_ERROR( CX_StsOutOfRange, "Invalid index" );

    if( index == total - 1 )
    {
        cxSeqPop( seq, 0 );
    }
    else if( index == 0 )
    {
        cxSeqPopFront( seq, 0 );
    }
    else
    {
        block = seq->first;
        elem_size = seq->elem_size;
        delta_index = block->start_index;
        while( block->start_index - delta_index + block->count <= index )
            block = block->next;

        ptr = block->data + (index - block->start_index + delta_index) * elem_size;

        front = index < total >> 1;
        if( !front )
        {
            block_size = block->count * elem_size - (ptr - block->data);

            while( block != seq->first->prev )  /* while not the last block */
            {
                CxSeqBlock *next_block = block->next;

                memmove( ptr, ptr + elem_size, block_size - elem_size );
                memcpy( ptr + block_size - elem_size, next_block->data, elem_size );
                block = next_block;
                ptr = block->data;
                block_size = block->count * elem_size;
            }

            memmove( ptr, ptr + elem_size, block_size - elem_size );
            seq->ptr -= elem_size;
        }
        else
        {
            ptr += elem_size;
            block_size = ptr - block->data;

            while( block != seq->first )
            {
                CxSeqBlock *prev_block = block->prev;

                memmove( block->data + elem_size, block->data, block_size - elem_size );
                block_size = prev_block->count * elem_size;
                memcpy( block->data, prev_block->data + block_size - elem_size, elem_size );
                block = prev_block;
            }

            memmove( block->data + elem_size, block->data, block_size - elem_size );
            block->data += elem_size;
            block->start_index++;
        }

        seq->total = total - 1;
        if( --block->count == 0 )
            icxFreeSeqBlock( seq, front );
    }

    __END__;
}


/* adds several elements to the end or in the beginning of sequence */
CX_IMPL void
cxSeqPushMulti( CxSeq *seq, void *_elements, int count, int front )
{
    char *elements = (char *) _elements;

    CX_FUNCNAME( "cxSeqPushMulti" );

    __BEGIN__;
    int elem_size;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "NULL sequence pointer" );
    if( count < 0 )
        CX_ERROR( CX_StsBadSize, "number of removed elements is negative" );

    elem_size = seq->elem_size;

    if( !front )
    {
        while( count > 0 )
        {
            int delta = (seq->block_max - seq->ptr) / elem_size;

            delta = MIN( delta, count );
            if( delta > 0 )
            {
                seq->first->prev->count += delta;
                seq->total += delta;
                count -= delta;
                delta *= elem_size;
                if( elements )
                {
                    memcpy( seq->ptr, elements, delta );
                    elements += delta;
                }
                seq->ptr += delta;
            }

            if( count > 0 )
                CX_CALL( icxGrowSeq( seq, 0 ));
        }
    }
    else
    {
        CxSeqBlock* block = seq->first;
        
        while( count > 0 )
        {
            int delta;
            
            if( !block || block->start_index == 0 )
            {
                CX_CALL( icxGrowSeq( seq, 1 ));

                block = seq->first;
                assert( block->start_index > 0 );
            }

            delta = MIN( block->start_index, count );
            count -= delta;
            block->start_index -= delta;
            block->count += delta;
            seq->total += delta;
            delta *= elem_size;
            block->data -= delta;

            if( elements )
                memcpy( block->data, elements + count*elem_size, delta );
        }
    }

    __END__;
}


/* removes several elements from the end of sequence */
CX_IMPL void
cxSeqPopMulti( CxSeq *seq, void *_elements, int count, int front )
{
    char *elements = (char *) _elements;

    CX_FUNCNAME( "cxSeqPopMulti" );

    __BEGIN__;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "NULL sequence pointer" );
    if( count < 0 )
        CX_ERROR( CX_StsBadSize, "number of removed elements is negative" );

    count = MIN( count, seq->total );

    if( !front )
    {
        if( elements )
            elements += count * seq->elem_size;

        while( count > 0 )
        {
            int delta = seq->first->prev->count;

            delta = MIN( delta, count );
            assert( delta > 0 );

            seq->first->prev->count -= delta;
            seq->total -= delta;
            count -= delta;
            delta *= seq->elem_size;
            seq->ptr -= delta;

            if( elements )
            {
                elements -= delta;
                memcpy( elements, seq->ptr, delta );
            }

            if( seq->first->prev->count == 0 )
                icxFreeSeqBlock( seq, 0 );
        }
    }
    else
    {
        while( count > 0 )
        {
            int delta = seq->first->count;

            delta = MIN( delta, count );
            assert( delta > 0 );

            seq->first->count -= delta;
            seq->total -= delta;
            count -= delta;
            seq->first->start_index += delta;
            delta *= seq->elem_size;
            seq->first->data += delta;

            if( elements )
            {
                memcpy( elements, seq->first->data, delta );
                elements += delta;
            }

            if( seq->first->count == 0 )
                icxFreeSeqBlock( seq, 1 );
        }
    }

    __END__;
}


/* removes all elements from the sequence */
CX_IMPL void
cxClearSeq( CxSeq *seq )
{
    CX_FUNCNAME( "cxClearSeq" );

    __BEGIN__;

    if( !seq )
        CX_ERROR( CX_StsNullPtr, "" );
    cxSeqPopMulti( seq, 0, seq->total );

    __END__;
}


CX_IMPL CxSeq*
cxSeqSlice( CxSeq* seq, CxSlice slice, CxMemStorage* storage, int copy_data )
{
    CxSeq* subseq = 0;
    
    CX_FUNCNAME("cxSeqSlice");

    __BEGIN__;
    int elem_size, length; 
    
    if( !CX_IS_SEQ(seq) )
        CX_ERROR( CX_StsBadArg, "Invalid sequence header" );

    if( !storage )
    {
        storage = seq->storage;
        if( !storage )
            CX_ERROR( CX_StsNullPtr, "NULL storage pointer" );
    }

    elem_size = seq->elem_size;
    length = cxSliceLength( slice, seq );
    if( slice.startIndex < 0 )
        slice.startIndex += seq->total;
    else if( slice.startIndex >= seq->total )
        slice.startIndex -= seq->total;
    if( (unsigned)length > (unsigned)seq->total ||
        ((unsigned)slice.startIndex >= (unsigned)seq->total && length != 0) )
        CX_ERROR( CX_StsOutOfRange, "Bad sequence slice" );

    if( !copy_data )
    {
        int header_size = cxAlign( seq->header_size, CX_STRUCT_ALIGN );
        int size, cnt;
        CxSeqBlock* start_block = 0, *block, *prev_block;
        char* data = 0;
        
        size = header_size + (length > 0 ? sizeof(CxSeqBlock) : 0);

        CX_CALL( data = cxGetSeqElem( seq, slice.startIndex, &start_block ));
        block = start_block;

        cnt = start_block->start_index - seq->first->start_index +
              start_block->count - slice.startIndex;

        while( cnt < length )
        {
            block = block->next;
            cnt += block->count;
            size += sizeof(CxSeqBlock);
        }

        // allocate sequence header and memory for all sequence blocks
        CX_CALL( subseq = cxCreateSeq( seq->flags, size, elem_size, storage ));
        
        if( length > 0 )
        {
            subseq->total = length;
            subseq->header_size = seq->header_size;
            subseq->first = prev_block = block =
                (CxSeqBlock*)((char*)subseq + header_size );

            cnt = start_block->start_index - seq->first->start_index +
                  start_block->count - slice.startIndex;

            prev_block->start_index = prev_block->count = 0;

            do
            {
                cnt = MIN( cnt, length );
                length -= cnt;
                block->prev = prev_block;
                prev_block->next = block;
                block->start_index = prev_block->start_index + prev_block->count;
                block->count = cnt;
                block->data = data;
                prev_block = block;
                block++;
                start_block = start_block->next;
                cnt = start_block->count;
                data = start_block->data;
            }
            while( length > 0 );

            --block;
            subseq->ptr = subseq->block_max = block->data + block->count*subseq->elem_size;
            block->next = subseq->first;
            subseq->first->prev = block;
        }
    }
    else
    {
        CX_CALL( subseq = cxCreateSeq( seq->flags, seq->header_size,
                                       seq->elem_size, storage ));

        if( length > 0 )
        {
            CxSeqBlock* block = 0;
            char* data = 0;
 
            CX_CALL( data = cxGetSeqElem( seq, slice.startIndex, &block ));

            int cnt = block->start_index - seq->first->start_index +
                      block->count - slice.startIndex;

            do
            {
                cnt = MIN( cnt, length );
                length -= cnt;

                cxSeqPushMulti( subseq, data, cnt );

                block = block->next;
                cnt = block->count;
                data = block->data;
            }
            while( length > 0 );
        }
    }
    
    __END__;

    return subseq;
}


// Remove slice from the middle of the sequence
// !!! TODO !!! Implement more efficient algorithm
CX_IMPL void
cxSeqRemoveSlice( CxSeq* seq, CxSlice slice )
{
    CX_FUNCNAME("cxSeqRemoveSlice");

    __BEGIN__;

    int total, length;

    if( !CX_IS_SEQ(seq) )
        CX_ERROR( CX_StsBadArg, "Invalid sequence header" );

    length = cxSliceLength( slice, seq );
    total = seq->total;

    if( slice.startIndex < 0 )
        slice.startIndex += total;
    else if( slice.startIndex >= total )
        slice.startIndex -= total;

    if( (unsigned)slice.startIndex >= (unsigned)total )
        CX_ERROR( CX_StsOutOfRange, "start slice index is out of range" );

    slice.endIndex = slice.startIndex + length;

    if( slice.endIndex < total )
    {
        CxSeqReader reader_to, reader_from;
        int elem_size = seq->elem_size;

        cxStartReadSeq( seq, &reader_to );
        cxStartReadSeq( seq, &reader_from );

        if( slice.startIndex > total - slice.endIndex )
        {
            int i, count = seq->total - slice.endIndex;
            cxSetSeqReaderPos( &reader_to, slice.startIndex );
            cxSetSeqReaderPos( &reader_from, slice.endIndex );

            for( i = 0; i < count; i++ )
            {
                memcpy( reader_to.ptr, reader_from.ptr, elem_size );
                CX_NEXT_SEQ_ELEM( elem_size, reader_to );
                CX_NEXT_SEQ_ELEM( elem_size, reader_from );
            }

            cxSeqPopMulti( seq, 0, slice.endIndex - slice.startIndex );
        }
        else
        {
            int i, count = slice.startIndex;
            cxSetSeqReaderPos( &reader_to, slice.endIndex );
            cxSetSeqReaderPos( &reader_from, slice.startIndex );

            for( i = 0; i < count; i++ )
            {
                CX_PREV_SEQ_ELEM( elem_size, reader_to );
                CX_PREV_SEQ_ELEM( elem_size, reader_from );

                memcpy( reader_to.ptr, reader_from.ptr, elem_size );
            }

            cxSeqPopMulti( seq, 0, slice.endIndex - slice.startIndex, 1 );
        }
    }
    else
    {
        cxSeqPopMulti( seq, 0, total - slice.startIndex );
        cxSeqPopMulti( seq, 0, slice.endIndex - total, 1 );
    }

    __END__;
}


// Inserts a new sequence into the middle of another sequence
// !!! TODO !!! Implement more efficient algorithm
CX_IMPL void
cxSeqInsertSlice( CxSeq* seq, int index, const CxArr* from_arr )
{
    CxSeqReader reader_to, reader_from;
    int i, elem_size, total, from_total;
    
    CX_FUNCNAME("cxSeqInsertSlice");

    __BEGIN__;

    CxSeq from_header, *from = (CxSeq*)from_arr;
    CxSeqBlock block;

    if( !CX_IS_SEQ(seq) )
        CX_ERROR( CX_StsBadArg, "Invalid destination sequence header" );

    if( !CX_IS_SEQ(from))
    {
        CxMat* mat = (CxMat*)from;
        if( !CX_IS_MAT(mat))
            CX_ERROR( CX_StsBadArg, "Source is not a sequence nor matrix" );

        if( !CX_IS_MAT_CONT(mat->type) || mat->rows != 1 || mat->cols != 1 )
            CX_ERROR( CX_StsBadArg, "The source array must be 1d coninuous vector" );

        CX_CALL( from = cxMakeSeqHeaderForArray( CX_SEQ_KIND_GENERIC, sizeof(from_header),
                                                 icxPixSize[CX_MAT_TYPE(mat->type)],
                                                 mat->data.ptr, mat->cols + mat->rows - 1,
                                                 &from_header, &block ));
    }

    if( seq->elem_size != from->elem_size )
        CX_ERROR( CX_StsUnmatchedSizes,
        "Sizes of source and destination sequences' elements are different" );

    from_total = from->total;

    if( from_total == 0 )
        EXIT;

    total = seq->total;
    index += index < 0 ? total : 0;
    index -= index > total ? total : 0;

    if( (unsigned)index > (unsigned)total )
        CX_ERROR_FROM_STATUS( CX_BADRANGE_ERR );

    elem_size = seq->elem_size;

    if( index < (total >> 1) )
    {
        cxSeqPushMulti( seq, 0, from_total, 1 );

        cxStartReadSeq( seq, &reader_to );
        cxStartReadSeq( seq, &reader_from );
        cxSetSeqReaderPos( &reader_from, from_total );

        for( i = 0; i < index; i++ )
        {
            memcpy( reader_to.ptr, reader_from.ptr, elem_size );
            CX_NEXT_SEQ_ELEM( elem_size, reader_to );
            CX_NEXT_SEQ_ELEM( elem_size, reader_from );
        }
    }
    else
    {
        cxSeqPushMulti( seq, 0, from_total );

        cxStartReadSeq( seq, &reader_to );
        cxStartReadSeq( seq, &reader_from );
        cxSetSeqReaderPos( &reader_from, total );
        cxSetSeqReaderPos( &reader_to, seq->total );

        for( i = 0; i < total - index; i++ )
        {
            CX_PREV_SEQ_ELEM( elem_size, reader_to );
            CX_PREV_SEQ_ELEM( elem_size, reader_from );
            memcpy( reader_to.ptr, reader_from.ptr, elem_size );
        }
    }

    cxStartReadSeq( from, &reader_from );
    cxSetSeqReaderPos( &reader_to, index );

    for( i = 0; i < from_total; i++ )
    {
        memcpy( reader_to.ptr, reader_from.ptr, elem_size );
        CX_NEXT_SEQ_ELEM( elem_size, reader_to );
        CX_NEXT_SEQ_ELEM( elem_size, reader_from );
    }

    __END__;
}


// Sort the sequence using user-specified comparison function.
// Semantics is the same as in qsort function
CX_IMPL void
cxSeqSort( CxSeq* seq, CxCmpFunc cmp_func, void* userdata )
{
    const int bubble_level = 16;

    struct
    {
        int lb, ub;
    }
    stack[48];

    int sp = 0;

    int elem_size;
    CxSeqReader left_reader, right_reader;

    CX_FUNCNAME("cxSeqSort");

    __BEGIN__;

    if( !seq || !cmp_func )
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );

    if( seq->total <= 1 )
        EXIT;

    stack[0].lb = 0;
    stack[0].ub = seq->total - 1;

    elem_size = seq->elem_size;

    CX_CALL( cxStartReadSeq( seq, &left_reader ));
    CX_CALL( cxStartReadSeq( seq, &right_reader ));

    while( sp >= 0 )
    {
        int lb = stack[sp].lb;
        int ub = stack[sp--].ub;

        for(;;)
        {
            int diff = ub - lb;
            if( diff < bubble_level )
            {
                int i, j;
                cxSetSeqReaderPos( &left_reader, lb );

                for( i = diff; i > 0; i-- )
                {
                    int f = 0;

                    right_reader.block = left_reader.block;
                    right_reader.ptr = left_reader.ptr;
                    right_reader.block_min = left_reader.block_min;
                    right_reader.block_max = left_reader.block_max;
                    
                    for( j = 0; j < i; j++ )
                    {
                        char* ptr = right_reader.ptr;
                        CX_NEXT_SEQ_ELEM( elem_size, right_reader );
                        if( cmp_func( ptr, right_reader.ptr, userdata ) > 0 )
                        {
                            CX_SWAP_ELEMS( ptr, right_reader.ptr );
                            f = 1;
                        }
                    }
                    if( !f ) break;
                }
                break;
            }
            else
            {
                /* select pivot and exchange with 1st element */
                int  m = lb + (diff >> 1);
                int  i = lb, j = ub + 1;
                char* pivot_ptr;

                cxSetSeqReaderPos( &left_reader, lb );
                cxSetSeqReaderPos( &right_reader, m );
                pivot_ptr = right_reader.ptr;
                cxSetSeqReaderPos( &right_reader, ub );

                /* choose median among seq[lb], seq[m], seq[ub] */
                int a = cmp_func( pivot_ptr, left_reader.ptr, userdata ) < 0;
                int b = cmp_func( pivot_ptr, right_reader.ptr, userdata ) < 0;

                if( a == b )
                {
                    b = cmp_func( left_reader.ptr, right_reader.ptr, userdata ) < 0;
                    pivot_ptr = a == b ? left_reader.ptr : right_reader.ptr;
                }

                if( pivot_ptr != left_reader.ptr )
                {
                    CX_SWAP_ELEMS( left_reader.ptr, pivot_ptr );
                    pivot_ptr = left_reader.ptr;
                }
                    
                CX_NEXT_SEQ_ELEM( elem_size, left_reader );

                /* partition into two segments */
                for(;;)
                {
                    for( ; ++i < j && cmp_func( left_reader.ptr, pivot_ptr, userdata ) <= 0; )
                    {
                        CX_NEXT_SEQ_ELEM( elem_size, left_reader );
                    }

                    for( ; --j >= i && cmp_func( pivot_ptr, right_reader.ptr, userdata ) <= 0; )
                    {
                        CX_PREV_SEQ_ELEM( elem_size, right_reader );
                    }

                    if( i >= j ) break;
                    CX_SWAP_ELEMS( left_reader.ptr, right_reader.ptr );
                    CX_NEXT_SEQ_ELEM( elem_size, left_reader );
                    CX_PREV_SEQ_ELEM( elem_size, right_reader );
                }

                /* pivot belongs in A[j] */
                CX_SWAP_ELEMS( right_reader.ptr, pivot_ptr );

                /* keep processing smallest segment, and stack largest*/
                if( j - lb <= ub - j )
                {
                    if( j + 1 < ub )
                    {
                        stack[++sp].lb   = j + 1;
                        stack[sp].ub = ub;
                    }
                    ub = j - 1;
                }
                else
                {
                    if( j - 1 > lb)
                    {
                        stack[++sp].lb = lb;
                        stack[sp].ub = j - 1;
                    }
                    lb = j + 1;
                }
            }
        }
    }

    __END__;
}


CX_IMPL void
cxSeqInvert( CxSeq* seq )
{
    CX_FUNCNAME( "cxSeqInvert" );

    __BEGIN__;

    CxSeqReader left_reader, right_reader;
    int elem_size = seq->elem_size;
    int i, count;

    CX_CALL( cxStartReadSeq( seq, &left_reader, 0 ));
    CX_CALL( cxStartReadSeq( seq, &right_reader, 1 ));
    count = seq->total >> 1;

    for( i = 0; i < count; i++ )
    {
        CX_SWAP_ELEMS( left_reader.ptr, right_reader.ptr );
        CX_NEXT_SEQ_ELEM( elem_size, left_reader );
        CX_PREV_SEQ_ELEM( elem_size, right_reader );
    }

    __END__;
}


typedef struct CxPTreeNode
{
    struct CxPTreeNode* parent;
    char* element;
    int rank;
}
CxPTreeNode;


// split the input seq/set into one or more connected components.
// is_equal returns 1 if two elements belong to the same component
// the function returns sequence of integers - 0-based class indices for
// each element.  
CX_IMPL  int
cxPartitionSeq( CxSeq* seq, CxMemStorage* storage, CxSeq** comps,
                CxCmpFunc is_equal, void* userdata, int is_set )
{
    CxSeq* result = 0;
    CxMemStorage* temp_storage = 0;
    int class_idx = 0;
    
    CX_FUNCNAME( "icxSeqPartition" );

    __BEGIN__;

    CxSeqWriter writer;
    CxSeqReader reader;
    CxSeq* nodes;
    int i, j;

    if( !comps )
        CX_ERROR( CX_StsNullPtr, "" );

    if( !seq || !is_equal )
        CX_ERROR( CX_StsNullPtr, "" );

    if( !storage )
        storage = seq->storage;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "" );

    temp_storage = cxCreateChildMemStorage( storage );

    nodes = cxCreateSeq( 0, sizeof(CxSeq), sizeof(CxPTreeNode), temp_storage );

    cxStartReadSeq( seq, &reader );
    memset( &writer, 0, sizeof(writer));
    cxStartAppendToSeq( nodes, &writer ); 

    // Initial O(N) pass. Make a forest of single-vertex trees.
    for( i = 0; i < seq->total; i++ )
    {
        CxPTreeNode node = { 0, 0, 0 };
        if( !is_set || CX_IS_SET_ELEM( reader.ptr ))
            node.element = reader.ptr;
        CX_WRITE_SEQ_ELEM( node, writer );
        CX_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }

    cxEndWriteSeq( &writer );

    // because every time we made a full cycle through the node sequence,
    // we do not need to initialize reader every time
    cxStartReadSeq( nodes, &reader );

    // The main O(N^2) pass. Merge connected components.
    for( i = 0; i < nodes->total; i++ )
    {
        CxPTreeNode* node = (CxPTreeNode*)cxGetSeqElem( nodes, i );
        CxPTreeNode* root = node;

        if( !node->element )
            continue;

        // find root
        while( root->parent )
            root = root->parent;

        for( j = 0; j < nodes->total; j++ )
        {
            CxPTreeNode* node2 = (CxPTreeNode*)reader.ptr;
            
            if( node2->element && node2 != node &&
                is_equal( node->element, node2->element, userdata ))
            {
                CxPTreeNode* root2 = node2;
                
                // unite both trees
                while( root2->parent )
                    root2 = root2->parent;

                if( root2 != root )
                {
                    if( root->rank > root2->rank )
                        root2->parent = root;
                    else
                    {
                        root->parent = root2;
                        root2->rank += root->rank == root2->rank;
                        root = root2;
                    }
                    assert( root->parent == 0 );

                    // compress path from node2 to the root
                    while( node2->parent )
                    {
                        CxPTreeNode* temp = node2;
                        node2 = node2->parent;
                        temp->parent = root;
                    }

                    // compress path from node to the root
                    node2 = node;
                    while( node2->parent )
                    {
                        CxPTreeNode* temp = node2;
                        node2 = node2->parent;
                        temp->parent = root;
                    }
                }
            }

            CX_NEXT_SEQ_ELEM( sizeof(*node), reader );
        }
    }

    // Final O(N) pass (Enumerate classes)
    // Reuse reader one more time
    result = cxCreateSeq( 0, sizeof(CxSeq), sizeof(int), storage );
    cxStartAppendToSeq( result, &writer );

    for( i = 0; i < nodes->total; i++ )
    {
        CxPTreeNode* node = (CxPTreeNode*)reader.ptr;
        int idx = -1;
        
        if( node->element )
        {
            while( node->parent )
                node = node->parent;
            if( node->rank >= 0 )
                node->rank = ~class_idx++;
            idx = ~node->rank;
        }

        CX_NEXT_SEQ_ELEM( sizeof(*node), reader );
        CX_WRITE_SEQ_ELEM( idx, writer );
    }

    cxEndWriteSeq( &writer );

    __END__;

    if( comps )
        *comps = result;

    cxReleaseMemStorage( &temp_storage );
    return class_idx;
}


/****************************************************************************************\
*                                      Set implementation                                *
\****************************************************************************************/

/* creates empty set */
CX_IMPL CxSet*
cxCreateSet( int set_flags, int header_size, int elem_size, CxMemStorage * storage )
{
    CxSet *set = 0;

    CX_FUNCNAME( "cxCreateSet" );

    __BEGIN__;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "" );
    if( header_size < (int)sizeof( CxSet ) ||
        elem_size < (int)sizeof(void*)*2 ||
        (elem_size & (sizeof(void*)-1)) != 0 )
        CX_ERROR_FROM_STATUS( CX_BADSIZE_ERR );

    set = (CxSet*) cxCreateSeq( set_flags, header_size, elem_size, storage );
    set->flags = (set->flags & ~CX_MAGIC_MASK) | CX_SET_MAGIC_VAL;

    __END__;

    return set;
}


/* adds new element to the set */
CX_IMPL int
cxSetAdd( CxSet* set, CxSetElem* element, CxSetElem** inserted_element )
{
    int id = -1;

    CX_FUNCNAME( "cxSetAdd" );

    __BEGIN__;

    CxSetElem *free_elem;

    if( !set )
        CX_ERROR( CX_StsNullPtr, "" );

    if( !(set->free_elems) )
    {
        int count = set->total;
        int elem_size = set->elem_size;
        char *ptr;
        CX_CALL( icxGrowSeq( (CxSeq *) set, 0 ));

        set->free_elems = (CxSetElem*) (ptr = set->ptr);
        for( ; ptr + elem_size <= set->block_max; ptr += elem_size, count++ )
        {
            ((CxSetElem*)ptr)->flags = count | CX_SET_ELEM_FREE_FLAG;
            ((CxSetElem*)ptr)->next_free = (CxSetElem*)(ptr + elem_size);
        }
        assert( count <= CX_SET_ELEM_IDX_MASK+1 );
        ((CxSetElem*)(ptr - elem_size))->next_free = 0;
        set->first->prev->count += count - set->total;
        set->total = count;
        set->ptr = set->block_max;
    }

    free_elem = set->free_elems;
    set->free_elems = free_elem->next_free;

    id = free_elem->flags & CX_SET_ELEM_IDX_MASK;
    if( element )
        memcpy( free_elem, element, set->elem_size );

    free_elem->flags = id;

    if( inserted_element )
        *inserted_element = free_elem;

    __END__;

    return id;
}


/* get the set element given its index */
CX_IMPL CxSetElem *
cxGetSetElem( CxSet * set, int index )
{
    CxSetElem *elem = 0;

    /*CX_FUNCNAME( "cxGetSetElem" );*/

    __BEGIN__;

    elem = (CxSetElem*)cxGetSeqElem( (CxSeq*)set, index, 0 );

    if( elem && !CX_IS_SET_ELEM( elem ))
        elem = 0;

    __END__;

    return elem;
}


/* removes element from the set given its index */
CX_IMPL void
cxSetRemove( CxSet* set, int index )
{
    CxSetElem *elem;

    CX_FUNCNAME( "cxSetRemove" );

    __BEGIN__;

    elem = cxGetSetElem( set, index );

    if( elem )
    {
        cxSetRemoveByPtr( set, elem );
    }
    else if( !set )
    {
        CX_ERROR( CX_StsNullPtr, "" );
    }

    __END__;
}


/* removes all elements from the set */
CX_IMPL void
cxClearSet( CxSet* set )
{
    CX_FUNCNAME( "cxClearSet" );

    __BEGIN__;

    CX_CALL( cxClearSeq( (CxSeq*)set ));
    set->free_elems = 0;

    __END__;
}


/****************************************************************************************\
*                                 Graph  implementation                                  *
\****************************************************************************************/

/* creates new graph */
CX_IMPL CxGraph *
cxCreateGraph( int graph_type, int header_size,
               int vtx_size, int edge_size, CxMemStorage * storage )
{
    CxGraph *graph = 0;
    CxSet *temp_graph = 0;
    CxSet *edges = 0;

    CX_FUNCNAME( "cxCleateGraph" );

    __BEGIN__;

    if( header_size < (int)sizeof( CxGraph ) ||
        edge_size < (int)sizeof( CxGraphEdge ) ||
        vtx_size < (int)sizeof( CxGraphVtx ))
        CX_ERROR_FROM_STATUS( CX_BADSIZE_ERR );

    CX_CALL( temp_graph = cxCreateSet( graph_type, header_size, vtx_size, storage ));

    CX_CALL( edges = cxCreateSet( CX_SEQ_KIND_GENERIC | CX_SEQ_ELTYPE_GRAPH_EDGE,
                                  sizeof( CxSet ), edge_size, storage ));

    graph = (CxGraph*)temp_graph;
    graph->edges = edges;

    __END__;

    return graph;
}


/* Removes all the vertices and edges from the graph */
CX_IMPL void
cxClearGraph( CxGraph * graph )
{
    CX_FUNCNAME( "cxClearGraph" );

    __BEGIN__;

    if( !graph )
        CX_ERROR( CX_StsNullPtr, "" );

    cxClearSet( graph->edges );
    cxClearSet( (CxSet *) graph );

    __END__;
}


/* Adds vertex to the graph */
CX_IMPL int
cxGraphAddVtx( CxGraph * graph, CxGraphVtx * _vertex, CxGraphVtx ** _inserted_vertex )
{
    CxGraphVtx *vertex = 0;
    int index = -1;

    CX_FUNCNAME( "cxGraphAddVtx" );

    __BEGIN__;

    if( !graph )
        CX_ERROR( CX_StsNullPtr, "" );

    CX_CALL( index = cxSetAdd((CxSet*)graph, (CxSetElem*)_vertex, (CxSetElem**)&vertex));
    if( _inserted_vertex )
        *_inserted_vertex = vertex;
    vertex->first = 0;

    __END__;

    return index;
}


/* Removes vertex from the graph given its index together with incident edges */
CX_IMPL int
cxGraphRemoveVtxByPtr( CxGraph* graph, CxGraphVtx* vtx )
{
    int count = -1;
    
    CX_FUNCNAME( "cxGraphRemoveVtxByPtr" );

    __BEGIN__;

    if( !graph || !vtx )
        CX_ERROR( CX_StsNullPtr, "" );

    if( !CX_IS_SET_ELEM(vtx))
        CX_ERROR( CX_StsBadArg, "The vertex does not belong to the graph" );

    for( ;; )
    {
        CxGraphEdge *edge = vtx->first;
        count++;

        if( !edge )
            break;
        cxGraphRemoveEdgeByPtr( graph, edge->vtx[0], edge->vtx[1] );
    }
    cxSetRemoveByPtr( (CxSet*)graph, vtx );

    __END__;

    return count;
}


/* removes vertex from the graph */
CX_IMPL int
cxGraphRemoveVtx( CxGraph* graph, int index )
{
    int count = -1;
    CxGraphVtx *vtx = 0;

    CX_FUNCNAME( "cxGraphRemoveVtx" );

    __BEGIN__;

    if( !graph )
        CX_ERROR( CX_StsNullPtr, "" );

    vtx = cxGetGraphVtx( graph, index );
    if( !vtx )
        CX_ERROR( CX_StsBadArg, "The vertex is not found" );

    for( ;; )
    {
        CxGraphEdge *edge = vtx->first;
        count++;

        if( !edge )
            break;
        cxGraphRemoveEdgeByPtr( graph, edge->vtx[0], edge->vtx[1] );
    }
    cxSetRemoveByPtr( (CxSet*)graph, vtx );

    __END__;

    return count;
}


/* finds graph edge given pointers to the ending vertices */
CX_IMPL CxGraphEdge*
cxFindGraphEdgeByPtr( CxGraph * graph, CxGraphVtx* start_vtx, CxGraphVtx* end_vtx )
{
    CxGraphEdge *edge = 0;

    CX_FUNCNAME( "cxFindGraphEdgeByPtr" );

    __BEGIN__;

    if( !graph || !start_vtx || !end_vtx )
        CX_ERROR( CX_StsNullPtr, "" );

    if( start_vtx != end_vtx )
    {
        int ofs = 0;

        edge = start_vtx->first;

        if( CX_IS_GRAPH_ORIENTED( graph ))
        {
            for( ; edge; edge = edge->next[ofs] )
            {
                ofs = start_vtx == edge->vtx[1];
                assert( ofs == 1 || start_vtx == edge->vtx[0] );
                if( ofs == 0 && edge->vtx[1] == end_vtx )
                    break;
            }
        }
        else
        {
            for( ; edge; edge = edge->next[ofs] )
            {
                ofs = start_vtx == edge->vtx[1];
                assert( ofs == 1 || start_vtx == edge->vtx[0] );
                if( edge->vtx[ofs ^ 1] == end_vtx )
                    break;
            }
        }
    }

    __END__;

    return edge;
}


/* finds edge in the graph given indices of the ending vertices */
CX_IMPL CxGraphEdge *
cxFindGraphEdge( CxGraph * graph, int start_idx, int end_idx )
{
    CxGraphEdge *edge = 0;
    CxGraphVtx *start_vtx;
    CxGraphVtx *end_vtx;

    CX_FUNCNAME( "cxFindGraphEdge" );

    __BEGIN__;

    if( !graph )
        CX_ERROR( CX_StsNullPtr, "" );

    start_vtx = cxGetGraphVtx( graph, start_idx );
    end_vtx = cxGetGraphVtx( graph, end_idx );

    edge = cxFindGraphEdgeByPtr( graph, start_vtx, end_vtx );

    __END__;

    return edge;
}


/* Adds new edge connecting two given vertices
   to the graph or returns already existing edge */
CX_IMPL int
cxGraphAddEdgeByPtr( CxGraph * graph,
                     CxGraphVtx * start_vtx, CxGraphVtx * end_vtx,
                     CxGraphEdge * _edge, CxGraphEdge ** _inserted_edge )
{
    CxGraphVtx *vtx[2];
    CxGraphEdge *edge;
    int result = -1;
    int i;

    CX_FUNCNAME( "cxGraphAddEdgeByPtr" );

    __BEGIN__;

    vtx[0] = start_vtx;
    vtx[1] = end_vtx;

    CX_CALL( edge = cxFindGraphEdgeByPtr( graph, vtx[0], vtx[1] ));

    if( edge )
    {
        result = 0;
        EXIT;
    }

    if( start_vtx == end_vtx )
        CX_ERROR( start_vtx ? CX_StsBadArg : CX_StsNullPtr, "" );

    CX_CALL( edge = (CxGraphEdge*)cxSetNew( (CxSet*)(graph->edges) ));
    assert( edge->flags >= 0 );

    for( i = 0; i < 2; i++ )
    {
        edge->vtx[i] = vtx[i];
        edge->next[i] = vtx[i]->first;
        vtx[i]->first = edge;
    }

    if( _edge )
    {
        memcpy( edge + 1, _edge + 1, graph->edges->elem_size - sizeof( *edge ));
        edge->weight = _edge->weight;
    }

    result = 1;

    __END__;

    if( _inserted_edge )
        *_inserted_edge = edge;

    return result;
}

/* Adds new edge connecting two given vertices
   to the graph or returns already existing edge */
CX_IMPL int
cxGraphAddEdge( CxGraph * graph,
                int start_idx, int end_idx,
                CxGraphEdge * _edge, CxGraphEdge ** _inserted_edge )
{
    CxGraphVtx *start_vtx;
    CxGraphVtx *end_vtx;
    int result = -1;

    CX_FUNCNAME( "cxGraphAddEdge" );

    __BEGIN__;

    if( !graph )
        CX_ERROR( CX_StsNullPtr, "" );

    start_vtx = cxGetGraphVtx( graph, start_idx );
    end_vtx = cxGetGraphVtx( graph, end_idx );

    result = cxGraphAddEdgeByPtr( graph, start_vtx, end_vtx, _edge, _inserted_edge );

    __END__;

    return result;
}


/* removes graph edge given pointers to the ending vertices */
CX_IMPL void
cxGraphRemoveEdgeByPtr( CxGraph* graph, CxGraphVtx* start_vtx, CxGraphVtx* end_vtx )
{
    int i;
    CxGraphVtx *vtx[2];
    CxGraphEdge *prev = 0;
    CxGraphEdge *edge = 0;

    CX_FUNCNAME( "cxGraphRemoveEdgeByPtr" );

    __BEGIN__;

    if( !graph || !start_vtx || !end_vtx )
        CX_ERROR( CX_StsNullPtr, "" );

    if( start_vtx != end_vtx )
    {
        vtx[0] = start_vtx;
        vtx[1] = end_vtx;

        if( CX_IS_GRAPH_ORIENTED( graph ))
        {
            for( i = 0; i < 2; i++ )
            {
                int ofs = 0, prev_ofs = 0;

                for( prev = 0, edge = vtx[i]->first; edge; prev_ofs = ofs, prev = edge,
                     edge = edge->next[ofs] )
                {
                    ofs = vtx[i] == edge->vtx[1];
                    assert( ofs == 1 || vtx[i] == edge->vtx[0] );
                    if( ofs == i && edge->vtx[i ^ 1] == vtx[i ^ 1] )
                        break;
                }

                if( edge )
                {
                    if( prev )
                        prev->next[prev_ofs] = edge->next[ofs];
                    else
                        vtx[i]->first = edge->next[ofs];
                }
            }
        }
        else
        {
            for( i = 0; i < 2; i++ )
            {
                int ofs = 0, prev_ofs = 0;

                for( prev = 0, edge = vtx[i]->first; edge; prev_ofs = ofs, prev = edge,
                     edge = edge->next[ofs] )
                {
                    ofs = vtx[i] == edge->vtx[1];
                    assert( ofs == 1 || vtx[i] == edge->vtx[0] );
                    if( edge->vtx[ofs ^ 1] == vtx[i ^ 1] )
                        break;
                }

                if( edge )
                {
                    if( prev )
                        prev->next[prev_ofs] = edge->next[ofs];
                    else
                        vtx[i]->first = edge->next[ofs];
                }
            }
        }

        if( edge )
            cxSetRemoveByPtr( graph->edges, edge );
    }

    __END__;
}


/* Removes edge from the graph given indices of the ending vertices */
CX_IMPL void
cxGraphRemoveEdge( CxGraph * graph, int start_idx, int end_idx )
{
    CxGraphVtx *start_vtx;
    CxGraphVtx *end_vtx;

    CX_FUNCNAME( "cxGraphRemoveEdge" );

    __BEGIN__;

    if( !graph )
        CX_ERROR( CX_StsNullPtr, "" );

    start_vtx = cxGetGraphVtx( graph, start_idx );
    end_vtx = cxGetGraphVtx( graph, end_idx );

    cxGraphRemoveEdgeByPtr( graph, start_vtx, end_vtx );

    __END__;
}


/* counts number of edges incident to the vertex */
CX_IMPL int
cxGraphVtxDegreeByPtr( CxGraph * graph, CxGraphVtx * vertex )
{
    CxGraphEdge *edge;
    int count = -1;

    CX_FUNCNAME( "cxGraphVtxDegreeByPtr" );

    __BEGIN__;

    if( !graph || !vertex )
        CX_ERROR( CX_StsNullPtr, "" );

    for( edge = vertex->first, count = 0; edge; )
    {
        count++;
        edge = CX_NEXT_GRAPH_EDGE( edge, vertex );
    }

    __END__;

    return count;
}


/* counts number of edges incident to the vertex */
CX_IMPL int
cxGraphVtxDegree( CxGraph * graph, int vtx_idx )
{
    CxGraphVtx *vertex;
    CxGraphEdge *edge;
    int count = -1;

    CX_FUNCNAME( "cxGraphVtxDegree" );

    __BEGIN__;

    if( !graph )
        CX_ERROR( CX_StsNullPtr, "" );

    vertex = cxGetGraphVtx( graph, vtx_idx );
    if( !vertex )
        CX_ERROR_FROM_STATUS( CX_NOTFOUND_ERR );

    for( edge = vertex->first, count = 0; edge; )
    {
        count++;
        edge = CX_NEXT_GRAPH_EDGE( edge, vertex );
    }

    __END__;

    return count;
}


typedef struct CxGraphItem
{
    CxGraphVtx* vtx;
    CxGraphEdge* edge;
}
CxGraphItem;


static  void
icxSeqElemsClearFlags( CxSeq* seq, int offset, int clear_mask )
{
    CX_FUNCNAME("icxStartScanGraph");

    __BEGIN__;
    
    CxSeqReader reader;
    int i, total, elem_size;

    if( !seq )
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );

    elem_size = seq->elem_size;
    total = seq->total;

    if( (unsigned)offset > (unsigned)elem_size )
        CX_ERROR_FROM_STATUS( CX_BADARG_ERR );

    CX_CALL( cxStartReadSeq( seq, &reader ));

    for( i = 0; i < total; i++ )
    {
        int* flag_ptr = (int*)(reader.ptr + offset);
        *flag_ptr &= ~clear_mask;

        CX_NEXT_SEQ_ELEM( elem_size, reader );
    }

    __END__;
}


static  char*
icxSeqFindNextElem( CxSeq* seq, int offset, int mask,
                    int value, int* start_index )
{
    char* elem_ptr = 0;
    
    CX_FUNCNAME("icxStartScanGraph");

    __BEGIN__;
    
    CxSeqReader reader;
    int total, elem_size, index;

    if( !seq || !start_index )
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );

    elem_size = seq->elem_size;
    total = seq->total;
    index = *start_index;

    if( (unsigned)offset > (unsigned)elem_size )
        CX_ERROR_FROM_STATUS( CX_BADARG_ERR );

    if( total == 0 )
        EXIT;

    if( (unsigned)index >= (unsigned)total )
    {
        index %= total;
        index += index < 0 ? total : 0;
    }

    CX_CALL( cxStartReadSeq( seq, &reader ));

    if( index != 0 )
    {
        CX_CALL( cxSetSeqReaderPos( &reader, index ));
    }

    for( index = 0; index < total; index++ )
    {
        int* flag_ptr = (int*)(reader.ptr + offset);
        if( (*flag_ptr & mask) == value )
            break;

        CX_NEXT_SEQ_ELEM( elem_size, reader );
    }

    if( index < total )
    {
        elem_ptr = reader.ptr;
        *start_index = index;
    }

    __END__;

    return  elem_ptr;
}

#define CX_FIELD_OFFSET( field, structtype ) ((int)(long)&((structtype*)0)->field)

CX_IMPL void
cxStartScanGraph( CxGraph* graph, CxGraphScanner* scanner,
                  CxGraphVtx* vtx, int mask )
{
    CxMemStorage* child_storage = 0;

    CX_FUNCNAME("cxStartScanGraph");

    __BEGIN__;

    if( !graph || !scanner )
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );

    if( !(graph->storage ))
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );

    memset( scanner, 0, sizeof(*scanner));

    scanner->graph = graph;
    scanner->mask = mask;
    scanner->vtx = vtx;
    scanner->index = vtx == 0 ? 0 : -1;

    CX_CALL( child_storage = cxCreateChildMemStorage( graph->storage ));

    CX_CALL( scanner->stack = cxCreateSeq( 0, sizeof(CxSet),
                       sizeof(CxGraphItem), child_storage ));

    CX_CALL( icxSeqElemsClearFlags( (CxSeq*)graph,
                                    CX_FIELD_OFFSET( flags, CxGraphVtx),
                                    CX_GRAPH_ITEM_VISITED_FLAG|
                                    CX_GRAPH_SEARCH_TREE_NODE_FLAG ));

    CX_CALL( icxSeqElemsClearFlags( (CxSeq*)(graph->edges),
                                    CX_FIELD_OFFSET( flags, CxGraphEdge),
                                    CX_GRAPH_ITEM_VISITED_FLAG ));

    __END__;

    if( cxGetErrStatus() < 0 )
        cxReleaseMemStorage( &child_storage );
}


CX_IMPL void
cxEndScanGraph( CxGraphScanner* scanner )
{
    CX_FUNCNAME("cxEndScanGraph");

    __BEGIN__;

    if( !scanner )
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );

    if( scanner->stack )
        CX_CALL( cxReleaseMemStorage( &(scanner->stack->storage)));

    __END__;
}


CX_IMPL int
cxNextGraphItem( CxGraphScanner* scanner )
{
    int code = -1;
    
    CX_FUNCNAME("cxNextGraphItem");

    __BEGIN__;

    CxGraphVtx* vtx;
    CxGraphVtx* dst;
    CxGraphEdge* edge;
    CxGraphItem item;

    if( !scanner || !(scanner->stack))
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );

    dst = scanner->dst;
    vtx = scanner->vtx;
    edge = scanner->edge;

    for(;;)
    {
        for(;;)
        {
            if( dst && !CX_IS_GRAPH_VERTEX_VISITED(dst) )
            {
                scanner->vtx = vtx = dst;
                edge = vtx->first;
                dst->flags |= CX_GRAPH_ITEM_VISITED_FLAG;

                if((scanner->mask & CX_GRAPH_VERTEX))
                {
                    scanner->vtx = vtx;
                    scanner->edge = vtx->first;
                    scanner->dst = 0;
                    code = CX_GRAPH_VERTEX;
                    EXIT;
                }
            }

            while( edge )
            {
                dst = edge->vtx[vtx == edge->vtx[0]];
                
                if( !CX_IS_GRAPH_EDGE_VISITED(edge) )
                {
                    // check that the edge is outcoming
                    if( !CX_IS_GRAPH_ORIENTED( scanner->graph ) || dst != edge->vtx[0] )
                    {
                        edge->flags |= CX_GRAPH_ITEM_VISITED_FLAG;
                        code = CX_GRAPH_BACK_EDGE;

                        if( !CX_IS_GRAPH_VERTEX_VISITED(dst) )
                        {
                            item.vtx = vtx;
                            item.edge = edge;

                            vtx->flags |= CX_GRAPH_SEARCH_TREE_NODE_FLAG;

                            cxSeqPush( scanner->stack, &item );
                            
                            if( scanner->mask & CX_GRAPH_TREE_EDGE )
                            {
                                code = CX_GRAPH_TREE_EDGE;
                                scanner->dst = dst;
                                scanner->edge = edge;
                                EXIT;
                            }
                            break;
                        }
                        else
                        {
                            if( scanner->mask & (CX_GRAPH_BACK_EDGE|
                                                 CX_GRAPH_CROSS_EDGE|
                                                 CX_GRAPH_FORWARD_EDGE) )
                            {
                                code = (dst->flags & CX_GRAPH_SEARCH_TREE_NODE_FLAG) ?
                                       CX_GRAPH_BACK_EDGE :
                                       (edge->flags & CX_GRAPH_FORWARD_EDGE_FLAG) ?
                                       CX_GRAPH_FORWARD_EDGE : CX_GRAPH_CROSS_EDGE;
                                edge->flags &= ~CX_GRAPH_FORWARD_EDGE_FLAG;
                                if( scanner->mask & code )
                                {
                                    scanner->vtx = vtx;
                                    scanner->dst = dst;
                                    scanner->edge = edge;
                                    EXIT;
                                }
                            }
                        }
                    }
                    else if( (dst->flags & (CX_GRAPH_ITEM_VISITED_FLAG|
                             CX_GRAPH_SEARCH_TREE_NODE_FLAG)) ==
                             (CX_GRAPH_ITEM_VISITED_FLAG|
                             CX_GRAPH_SEARCH_TREE_NODE_FLAG))
                    {
                        edge->flags |= CX_GRAPH_FORWARD_EDGE_FLAG;
                    }
                }

                edge = CX_NEXT_GRAPH_EDGE( edge, vtx );
            }

            if( !edge ) // need to backtrack
            {
                if( scanner->stack->total == 0 )
                {
                    if( scanner->index >= 0 )
                        vtx = 0;
                    else
                        scanner->index = 0;
                    break;
                }
                cxSeqPop( scanner->stack, &item );
                vtx = item.vtx;
                vtx->flags &= ~CX_GRAPH_SEARCH_TREE_NODE_FLAG;
                edge = item.edge;
                dst = 0;

                if( scanner->mask & CX_GRAPH_BACKTRACKING )
                {
                    scanner->vtx = vtx;
                    scanner->edge = edge;
                    scanner->dst = edge->vtx[vtx == edge->vtx[0]];
                    code = CX_GRAPH_BACKTRACKING;
                    EXIT;
                }
            }
        }

        if( !vtx )
        {
            vtx = (CxGraphVtx*)icxSeqFindNextElem( (CxSeq*)(scanner->graph),
                  CX_FIELD_OFFSET( flags, CxGraphVtx ), CX_GRAPH_ITEM_VISITED_FLAG|INT_MIN,
                  0, &(scanner->index) );

            if( !vtx )
            {
                code = CX_GRAPH_OVER;
                cxEndScanGraph( scanner );
                scanner->stack = 0;
                break;
            }
        }

        dst = vtx;
        if( scanner->mask & CX_GRAPH_NEW_TREE )
        {
            scanner->dst = dst;
            scanner->edge = 0;
            scanner->vtx = 0;
            code = CX_GRAPH_NEW_TREE;
            break;
        }
    }

    __END__;

    return code;
}


CX_IMPL CxGraph*
cxCloneGraph( const CxGraph* graph, CxMemStorage* storage )
{
    int* flag_buffer = 0;
    CxGraphVtx** ptr_buffer = 0;
    CxGraph* result = 0;
    
    CX_FUNCNAME( "cxCloneGraph" );

    __BEGIN__;

    int i, k;
    int vtx_size, edge_size;
    CxSeqReader reader;

    if( !CX_IS_GRAPH(graph))
        CX_ERROR( CX_StsBadArg, "Invalid graph pointer" );

    if( !storage )
        storage = graph->storage;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "NULL storage pointer" );

    vtx_size = graph->elem_size;
    edge_size = graph->edges->elem_size;

    CX_CALL( flag_buffer = (int*)cxAlloc( graph->total*sizeof(flag_buffer[0])));
    CX_CALL( ptr_buffer = (CxGraphVtx**)cxAlloc( graph->total*sizeof(ptr_buffer[0])));
    CX_CALL( result = cxCreateGraph( graph->flags, graph->header_size,
                                     vtx_size, edge_size, storage ));
    memcpy( result + sizeof(CxGraph), graph + sizeof(CxGraph),
            graph->header_size - sizeof(CxGraph));

    // pass 1. save flags, copy vertices
    cxStartReadSeq( (CxSeq*)graph, &reader );
    for( i = 0, k = 0; i < graph->total; i++ )
    {
        if( CX_IS_SET_ELEM( reader.ptr ))
        {
            CxGraphVtx* vtx = (CxGraphVtx*)reader.ptr;
            CxGraphVtx* dstvtx = 0;
            CX_CALL( cxGraphAddVtx( result, vtx, &dstvtx ));
            flag_buffer[k] = dstvtx->flags = vtx->flags;
            vtx->flags = k;
            ptr_buffer[k++] = dstvtx;
        }
        CX_NEXT_SEQ_ELEM( vtx_size, reader );
    }

    // pass 2. copy edges
    cxStartReadSeq( (CxSeq*)graph->edges, &reader );
    for( i = 0; i < graph->edges->total; i++ )
    {
        if( CX_IS_SET_ELEM( reader.ptr ))
        {
            CxGraphEdge* edge = (CxGraphEdge*)reader.ptr;
            CxGraphEdge* dstedge = 0;
            CxGraphVtx* new_org = ptr_buffer[edge->vtx[0]->flags];
            CxGraphVtx* new_dst = ptr_buffer[edge->vtx[1]->flags];
            CX_CALL( cxGraphAddEdgeByPtr( result, new_org, new_dst, edge, &dstedge ));
            dstedge->flags = edge->flags;
        }
        CX_NEXT_SEQ_ELEM( edge_size, reader );
    }

    // pass 3. restore flags
    cxStartReadSeq( (CxSeq*)graph, &reader );
    for( i = 0, k = 0; i < graph->edges->total; i++ )
    {
        if( CX_IS_SET_ELEM( reader.ptr ))
        {
            CxGraphVtx* vtx = (CxGraphVtx*)reader.ptr;
            vtx->flags = flag_buffer[k++];
        }
        CX_NEXT_SEQ_ELEM( vtx_size, reader );
    }

    __END__;

    cxFree( (void**)&flag_buffer );
    cxFree( (void**)&ptr_buffer );

    if( cxGetErrStatus() < 0 )
        result = 0;

    return result;
}


/****************************************************************************************\
*                                 Working with sequence tree                             *
\****************************************************************************************/

// Gathers pointers to all the sequences, accessible from the <first>, to the single sequence.
CX_IMPL CxSeq*
cxTreeToNodeSeq( const void* first, int header_size, CxMemStorage* storage )
{
    CxSeq* allseq = 0;

    CX_FUNCNAME("cxTreeToNodeSeq");

    __BEGIN__;

    CxTreeNodeIterator iterator;

    if( !storage )
        CX_ERROR( CX_StsNullPtr, "NULL storage pointer" );

    CX_CALL( allseq = cxCreateSeq( 0, header_size, sizeof(first), storage ));

    if( first )
    {
        CX_CALL( cxInitTreeNodeIterator( &iterator, first, INT_MAX ));

        for(;;)
        {
            void* node = cxNextTreeNode( &iterator );
            if( !node )
                break;
            cxSeqPush( allseq, &node );
        }
    }

    __END__;

    return allseq;
}


typedef struct CxTreeNode
{
    int       flags;         /* micsellaneous flags */         
    int       header_size;   /* size of sequence header */     
    struct    CxTreeNode* h_prev; /* previous sequence */      
    struct    CxTreeNode* h_next; /* next sequence */          
    struct    CxTreeNode* v_prev; /* 2nd previous sequence */  
    struct    CxTreeNode* v_next; /* 2nd next sequence */
}
CxTreeNode;



// Insert contour into tree given certain parent sequence.
// If parent is equal to frame (the most external contour),
// then added contour will have null pointer to parent.
CX_IMPL void
cxInsertNodeIntoTree( void* _node, void* _parent, void* _frame )
{
    CX_FUNCNAME( "cxInsertNodeIntoTree" );

    __BEGIN__;

    CxTreeNode* node = (CxTreeNode*)_node;
    CxTreeNode* parent = (CxTreeNode*)_parent;

    if( !node || !parent )
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );

    node->v_prev = _parent != _frame ? parent : 0;
    node->h_next = parent->v_next;

    assert( parent->v_next != node );

    if( parent->v_next )
        parent->v_next->h_prev = node;
    parent->v_next = node;

    __END__;
}


// Removes contour from tree (together with the contour children).
CX_IMPL void
cxRemoveNodeFromTree( void* _node, void* _frame )
{
    CX_FUNCNAME( "cxRemoveNodeFromTree" );

    __BEGIN__;

    CxTreeNode* node = (CxTreeNode*)_node;
    CxTreeNode* frame = (CxTreeNode*)_frame;

    if( !node )
        CX_ERROR_FROM_CODE( CX_StsNullPtr );

    if( node == frame )
        CX_ERROR( CX_StsBadArg, "frame node could not be deleted" );

    if( node->h_next )
        node->h_next->h_prev = node->h_prev;

    if( node->h_prev )
        node->h_prev->h_next = node->h_next;
    else
    {
        CxTreeNode* parent = node->v_prev;
        if( !parent )
            parent = frame;

        if( parent )
        {
            assert( parent->v_next == node );
            parent->v_next = node->h_next;
        }
    }

    __END__;
}


CX_IMPL void
cxInitTreeNodeIterator( CxTreeNodeIterator* treeIterator,
                        const void* first, int maxLevel )
{
    CX_FUNCNAME("icxInitTreeNodeIterator");

    __BEGIN__;
    
    if( !treeIterator || !first )
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );

    if( maxLevel < 0 )
        CX_ERROR_FROM_STATUS( CX_BADRANGE_ERR );

    treeIterator->node = (void*)first;
    treeIterator->level = 0;
    treeIterator->maxLevel = maxLevel;

    __END__;
}


CX_IMPL void*
cxNextTreeNode( CxTreeNodeIterator* treeIterator )
{
    CxTreeNode* prevNode = 0;
    
    CX_FUNCNAME("cxNextTreeNode");

    __BEGIN__;
    
    CxTreeNode* node;
    int level;

    if( !treeIterator )
        CX_ERROR( CX_StsNullPtr, "NULL iterator pointer" );

    prevNode = node = (CxTreeNode*)treeIterator->node;
    level = treeIterator->level;

    if( node )
    {
        if( node->v_next && level+1 < treeIterator->maxLevel )
        {
            node = node->v_next;
            level++;
        }
        else
        {
            while( node->h_next == 0 )
            {
                node = node->v_prev;
                if( --level < 0 )
                {
                    node = 0;
                    break;
                }
            }
            node = node && treeIterator->maxLevel != 0 ? node->h_next : 0;
        }
    }

    treeIterator->node = node;
    treeIterator->level = level;

    __END__;

    return prevNode;
}


CX_IMPL void*
cxPrevTreeNode( CxTreeNodeIterator* treeIterator )
{
    CxTreeNode* prevNode = 0;
    
    CX_FUNCNAME("cxPrevTreeNode");

    __BEGIN__;

    CxTreeNode* node;
    int level;

    if( !treeIterator )
        CX_ERROR_FROM_STATUS( CX_NULLPTR_ERR );    

    prevNode = node = (CxTreeNode*)treeIterator->node;
    level = treeIterator->level;
    
    if( node )
    {
        if( !node->h_prev )
        {
            node = node->v_prev;
            if( --level < 0 )
                node = 0;
        }
        else
        {
            node = node->h_prev;

            while( node->v_next && level < treeIterator->maxLevel )
            {
                node = node->v_next;
                level++;

                while( node->h_next )
                    node = node->h_next;
            }
        }
    }

    treeIterator->node = node;
    treeIterator->level = level;

    __END__;

    return prevNode;
}

/* End of file. */
