/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDenseMatrix.cpp                                               //
//                                                                         //
//  Purpose:                                                               //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Labutina, Senin,    //
//             Sidorov, Sysoyev, Vinogradov                                //
//                                                                         //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlDenseMatrix.hpp"

PNL_USING

#ifdef PNL_RTTI
template<>
const CPNLType &CDenseMatrix< int >::GetStaticTypeInfo()
{
  return CDenseMatrix< int >::m_TypeInfo;
}
#endif