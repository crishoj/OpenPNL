/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlFakePtr.hpp                                              //
//                                                                         //
//  Purpose:   declaration of fake pointers and dummies                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLFAKEPTR_HPP__
#define __PNLFAKEPTR_HPP__

#include "pnlConfig.hpp"

PNL_BEGIN

#ifndef PNL_VC6
#define PNL_FAKEPTR( T ) (pnl::FakePtr< T >())
#else
#define PNL_FAKEPTR( T ) (pnl::FakePtr< T >((T *)0))
#endif

// remaining is internal stuff

// assuming double is properly aligned for any type.  Otherwise cast will have undefined behaviour.
extern PNL_API double global_fakedummy;

template< typename T >
#ifndef PNL_VC6
inline T *FakePtr()
#else
inline T *FakePtr( T * )
#endif
{
    return (T *)&pnl::global_fakedummy;
}

PNL_END

#endif
