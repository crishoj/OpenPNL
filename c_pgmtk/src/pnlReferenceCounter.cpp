/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlReferenceCounter.cpp                                     //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlReferenceCounter.hpp"


PNL_USING

#ifndef _DEBUG

CReferenceCounter::CReferenceCounter() : m_refCounter(0)
{
}

#else // NDEBUG

inline int CReferenceCounter::GetNumOfReferences() const
{
    return m_refList.size();
}
//////////////////////////////////////////////////////////////////////////

inline void CReferenceCounter::AddRef(void* pObject)
{
    PNL_CHECK_IS_NULL_POINTER(pObject);
    
    m_refList.push_back(pObject);
}
//////////////////////////////////////////////////////////////////////////

inline void CReferenceCounter::Release(void* pObject)
{
    PNL_CHECK_IS_NULL_POINTER(pObject);
    
    std::list<void*>::iterator location = std::find( m_refList.begin(),
        m_refList.end(), pObject );
    
    assert( location != m_refList.end() );
    
    while( location != m_refList.end() )
    {
        location = std::find( m_refList.erase(location), m_refList.end(),
            pObject );
    }
    
    if( m_refList.empty() )
    {
        delete this;
    }
}
//////////////////////////////////////////////////////////////////////////

CReferenceCounter::CReferenceCounter()
{
}

#endif // NDEBUG

CReferenceCounter::~CReferenceCounter()
{
}
