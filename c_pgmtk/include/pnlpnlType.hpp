/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlpnlType.hpp                                              //
//                                                                         //
//  Purpose:   Class definition for main class for RTTI in PNL             //
//                                                                         //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Labutina, Senin,    //
//             Sidorov, Sysoyev, Vinogradov                                //
//                                                                         //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLPNLTYPE_HPP__
#define __PNLPNLTYPE_HPP__

#include <string>

// This is the main class for RTTI in PNL
// It does not support multiple inheritance
class PNL_API CPNLType
{
public:
    CPNLType(const std::string &TypeName, const CPNLType *const Parent = NULL):
	m_TypeName(TypeName),
	m_pParent(Parent)
    {}

    bool operator==(const CPNLType &OtherType) const
    {
	return (&OtherType == this);
    }

    bool IsDerived(const CPNLType &ParentType) const
    {
	if (ParentType == *this)
	{
	    return true;
	}

	if (m_pParent != NULL)
	{
	    return m_pParent->IsDerived(ParentType);
	}

	return false;
    }

    const pnlString &GetTypeName() const
    {
	return m_TypeName;
    }

private:
    const pnlString m_TypeName;
    const CPNLType *const m_pParent;
};

#endif
