/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGroup.hpp                                                //
//                                                                         //
//  Purpose:   Grouping object. Designed for persistence                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLGROUP_HPP__
#define __PNLGROUP_HPP__

#include "pnlString.hpp"

PNL_BEGIN

// grouping class
class PNL_API CGroupObj: public CPNLBase
{
public:
    CGroupObj()  { m_aName.reserve(4); m_aObject.reserve(4); m_abDelete.reserve(4); }
    ~CGroupObj() { for(int i = m_abDelete.size(); --i >= 0;) if(m_abDelete[i]) delete m_aObject[i]; }
    void Put(CPNLBase *pObj, const char *name, bool bAutoDelete = true);
    CPNLBase *Get(const char *name, bool bAutoDelete = true);
    void GetChildrenNames(pnlVector<pnlString> *paChild)
    {
	*paChild = m_aName;
    }

private:
    pnlVector<pnlString> m_aName;
    pnlVector<CPNLBase*> m_aObject;
    pnlVector<char>	 m_abDelete;
};

PNL_END

#endif // include guard
