/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlAllocator.hpp                                            //
//                                                                         //
//  Purpose:   Fused allocator                                             //
//             polymorphic object that can allocate memory different ways  //
//                                                                         //
//  Author(s): Denis Lagno                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlAllocator.hpp"

PNL_USING

CStdAllocKludge pnl::global_std_alloc_kludge;

CStdAllocKludge::CStdAllocKludge()
{
    request_mem = &CStdAllocKludge::RequestMem;
    release_mem = &CStdAllocKludge::ReleaseMem;
    release_kludge = &CStdAllocKludge::ReleaseKludge;
    clone = &CStdAllocKludge::Clone;
}

CStdAllocKludge *CStdAllocKludge::Create()
{
    return &global_std_alloc_kludge;
}

void CStdAllocKludge::ReleaseKludge( CAllocKludge *kl )
{}

CAllocKludge *CStdAllocKludge::Clone( CAllocKludge *kl )
{
    return &global_std_alloc_kludge;
}

void *CStdAllocKludge::RequestMem( CAllocKludge *, int n )
{
    return operator new( n );
}

void CStdAllocKludge::ReleaseMem( CAllocKludge *, void *p, int )
{
    operator delete( p );
}

#ifdef PNL_BROKEN_TEMPLATE
CStackAllocKludge::CStackAllocKludge( CStackMemStor<> *stor ): stor( stor )
{
    request_mem = &CStackAllocKludge::RequestMem;
    release_mem = &CStackAllocKludge::ReleaseMem;
    release_kludge = &CStackAllocKludge::ReleaseKludge;
    clone = &CStackAllocKludge::Clone;
}

CStackAllocKludge *CStackAllocKludge::Create( CStackMemStor<> *stor )
{
#if 1
    return new( CStackAllocKludge )( stor );
#else // following is flawed
    CStackAllocKludge *kl;
    kl = (CStackAllocKludge *)stor->RequestMem( sizeof( CStackAllocKludge ) );
    return new( kl )( CStackAllocKludge )( stor );
#endif
}

void CStackAllocKludge::ReleaseKludge( CAllocKludge *kl )
{
#if 1
    delete( kl );
#endif
}

CAllocKludge *CStackAllocKludge::Clone( CAllocKludge *kl )
{
    return Create( ((CStackAllocKludge *)kl)->stor );
}

void *CStackAllocKludge::RequestMem( CAllocKludge *kl, int n )
{
    return ((CStackAllocKludge *)kl)->stor->RequestMem( n );
}

void CStackAllocKludge::ReleaseMem( CAllocKludge *, void *, int )
{}
#endif

/* end of file */
