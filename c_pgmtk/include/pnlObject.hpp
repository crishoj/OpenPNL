/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlObject.hpp                                               //
//                                                                         //
//  Purpose:   Base class for most of library classes                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLOBJECT_HPP__
#define __PNLOBJECT_HPP__

#ifndef __PNLCONFIG_HPP__
#include "pnlConfig.hpp"
#endif

// FORWARDS
class CPNLType;

PNL_BEGIN

/* base class for all Tk objects */
class PNL_API CPNLBase 
{
protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif

    CPNLBase() {};
public:
    virtual ~CPNLBase() {};

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
	return GetStaticTypeInfo();
    }

    static const CPNLType &GetStaticTypeInfo()
    {
	return m_TypeInfo;
    }
#endif
};

PNL_END

#endif // __PNLOBJECT_HPP__
/* End of file */
