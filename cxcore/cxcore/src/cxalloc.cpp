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

// default <malloc>
CX_IMPL void* icxDefaultAlloc( size_t size, const char*, int)
{
    char *ptr, *ptr0 = (char*)malloc(
        (size_t)(size + CX_MALLOC_ALIGN*((size >= 4096) + 1) + sizeof(char*)*2));

    if( !ptr0 )
        return 0;

    // align the pointer
    ptr = ptr0 + sizeof(char*)*2 + 1;
    ptr = (char*)cxAlignPtr(ptr, CX_MALLOC_ALIGN);
    *(char**)(ptr - sizeof(char*)) = ptr0;

    return ptr;
}


// default <free>
CX_IMPL CXStatus icxDefaultFree( void** pptr, const char*, int )
{
    /* Checking for bad pointer */
    if( pptr && *pptr )
    {
        char* ptr = (char*)*pptr;

        /* Pointer must be align by CX_MALLOC_ALIGN */
        if( ((size_t)ptr & (CX_MALLOC_ALIGN-1)) != 0 )
            return CX_BADARG_ERR;

        *pptr = 0;
        free( *(char**)(ptr - sizeof(char*)) );
    }

    return CX_OK;
}


// pointers to allocation functions, initially set to default
static CxAllocFunc p_cxAlloc = icxDefaultAlloc;
static CxFreeFunc p_cxFree = icxDefaultFree;

CX_IMPL void cxSetMemoryManager( CxAllocFunc alloc_func, CxFreeFunc free_func )
{
    CX_FUNCNAME( "cxSetMemoryManager" );

    __BEGIN__;
    
    if( (alloc_func == 0) ^ (free_func == 0) )
        CX_ERROR( CX_StsNullPtr, "Either both pointers should be NULL or none of them");

    p_cxAlloc = alloc_func ? alloc_func : icxDefaultAlloc;
    p_cxFree = free_func ? free_func : icxDefaultFree;

    __END__;
}


CX_IMPL  void*  cxAlloc( size_t size )
{
    void* ptr = 0;
    
    CX_FUNCNAME( "cxAlloc" );

    __BEGIN__;

    if( (unsigned)size > CX_MAX_ALLOC_SIZE )
        CX_ERROR( CX_StsOutOfRange,
                  "Negative or too large argument of cxAlloc function" );

    ptr = p_cxAlloc( size, "", 0 );
    if( !ptr )
        CX_ERROR( CX_StsNoMem, "Out of memory" );

    __END__;

    return ptr;
}


CX_IMPL  void  cxFree( void** ptr )
{
    CX_FUNCNAME( "cxFree" );

    __BEGIN__;

    CXStatus status = p_cxFree( ptr, "", 0 );
    if( status < 0 )
        CX_ERROR( status, "" );

    __END__;
}

/* End of file. */
