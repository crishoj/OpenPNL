/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlReferenceCounter.hpp                                     //
//                                                                         //
//  Purpose:   CReferenceCounter class definition                          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// ReferenceCounter.hpp: interface for the ReferenceCounter class.

// It is a list or references of all objects which are linked with corresponding matrix

#ifndef __PNLREFERENCECOUNTER_HPP__
#define __PNLREFERENCECOUNTER_HPP__

#include "pnlObject.hpp"
#include "pnlException.hpp"
#include <assert.h>

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

#include "pnlParConfig.hpp"

#ifdef PAR_OMP
#include "omp.h"
#endif

PNL_BEGIN

class PNL_API CReferenceCounter : public CPNLBase 
{
    
public:
    
    inline void AddRef(void* pObjectIn);

    inline void Release(void* pObjectIn);

    inline int  GetNumOfReferences() const;
    
#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CReferenceCounter::m_TypeInfo;
  }
#endif
protected:

    CReferenceCounter();  
    
    ~CReferenceCounter(); 

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:

    // no need to copy CRefrenceCounter.
    // Anyway this operator should be reviewed
    CReferenceCounter& operator=(const CReferenceCounter& ref)
    { return *this; }

#ifdef _DEBUG

// has been commented - it is transfered to "config.h"
//#ifdef _MSC_VER
//#pragma warning(disable : 4284)
// return type for 'const_iterator::operator ->' is 'void *const * '
// (ie; not a UDT or reference to a UDT.
// Will produce errors if applied using infix notation)
//#endif

    std::list<void *> m_refList;

#ifdef PAR_OMP
	//This lock is used only in degug version of the project 
	omp_lock_t m_release_lock;
#endif

#else

    int               m_refCounter;

#endif

};
//////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG

inline int CReferenceCounter::GetNumOfReferences() const
{
    return m_refCounter;
}
//////////////////////////////////////////////////////////////////////////

inline void CReferenceCounter::AddRef(void* pObject)
{
    ++m_refCounter; 
}
//////////////////////////////////////////////////////////////////////////

inline void CReferenceCounter::Release(void* pObject)
{
    if( --m_refCounter == 0 ) 
    {
        delete this;
    }
}
//////////////////////////////////////////////////////////////////////////

#endif // _DEBUG


PNL_END

#endif //__PNLREFERENCECOUNTER_HPP__
