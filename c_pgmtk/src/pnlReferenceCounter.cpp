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
#ifdef PAR_OMP
#include <omp.h>
#endif

PNL_USING

#ifndef _DEBUG

CReferenceCounter::CReferenceCounter() : m_refCounter(0)
{
}

CReferenceCounter::~CReferenceCounter()
{
}

#else // NDEBUG
#ifdef PAR_OMP
inline int CReferenceCounter::GetNumOfReferences() const
{

	omp_set_lock(&m_release_lock);
	int size = m_refList.size();
	omp_unset_lock(&m_release_lock);

    return size;

}
//////////////////////////////////////////////////////////////////////////

inline void CReferenceCounter::AddRef(void* pObject)
{
    PNL_CHECK_IS_NULL_POINTER(pObject);
    
    omp_set_lock(&m_release_lock);
    m_refList.push_back(pObject);
    omp_unset_lock(&m_release_lock);
}
//////////////////////////////////////////////////////////////////////////

inline void CReferenceCounter::Release(void* pObject)
{
    PNL_CHECK_IS_NULL_POINTER(pObject);
    
    omp_set_lock(&m_release_lock);
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
	omp_unset_lock(&m_release_lock);
        delete this;
    }
    else
    {
        omp_unset_lock(&m_release_lock);
    };
}
//////////////////////////////////////////////////////////////////////////

CReferenceCounter::CReferenceCounter()
{
    omp_init_lock(&m_release_lock);
}

//////////////////////////////////////////////////////////////////////////

CReferenceCounter::~CReferenceCounter()
{
    omp_destroy_lock(&m_release_lock);
}

#else  // PAR_OMP

inline int CReferenceCounter::GetNumOfReferences() const
{
	int size = m_refList.size();

	return size;
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
    };
}
//////////////////////////////////////////////////////////////////////////

CReferenceCounter::CReferenceCounter()
{
}

//////////////////////////////////////////////////////////////////////////

CReferenceCounter::~CReferenceCounter()
{
}
#endif // PAR_OMP

#endif // NDEBUG

#ifdef PNL_RTTI
const CPNLType CReferenceCounter::m_TypeInfo = CPNLType("CReferenceCounter", &(CPNLBase::m_TypeInfo));

#endif