/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNumericDenseMatrix.cpp                                  //
//                                                                         //
//  Purpose:   CNumericDenseMatrix class member functions implementation   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlNumericDenseMatrix.hpp"

PNL_USING

#ifdef PNL_RTTI
template<>
const CPNLType &CNumericDenseMatrix< int >::GetStaticTypeInfo()
{
  return CNumericDenseMatrix< int >::m_TypeInfo;
}

template <>
const CPNLType CNumericDenseMatrix< float >::m_TypeInfo = CPNLType("CNumericDenseMatrix", &(iCNumericDenseMatrix< Type >::m_TypeInfo));

#endif