/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGroup.cpp                                                //
//                                                                         //
//  Purpose:   Grouping object. Designed for persistence                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlGroup.hpp"

PNL_USING

void CGroupObj::Put(CPNLBase *pObj, const char *name, bool bAutoDelete)
{
    m_aName.push_back(name);
    m_aObject.push_back(pObj);
    m_abDelete.push_back(bAutoDelete);
}

CPNLBase *CGroupObj::Get(const char *name, bool bAutoDelete)
{
    int i;
    pnlString sName(name);
    for(i = m_aName.size(); --i >= 0 && m_aName[i] != sName;);
    {
	if(sName == m_aName[i])
	{
	    m_abDelete[i] = bAutoDelete;
	    return m_aObject[i];
	}
    }
    return 0;
}
