/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNodeType.cpp                                             //
//                                                                         //
//  Purpose:   CNodeType class member functions implementation             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlNodeType.hpp"

PNL_USING

#ifdef PNL_RTTI
const CPNLType CNodeType::m_TypeInfo = CPNLType("CNodeType", &(CPNLBase::m_TypeInfo));
#endif