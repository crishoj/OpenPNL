/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMatrix.cpp                                               //
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
#include "pnlMatrix.hpp"
#include "pnlMatrix_impl.hpp"

PNL_USING

// Explicit instantiation of commonly used CMatrix classes.  These lines 
// ensure that the code in pnlMatrix_impl.hpp is compiled for these classes, 
// eliminating the need to #include pnlMatrix_impl.hpp in every file where 
// these classes are used.  
template class CMatrix<int>;
template class CMatrix<float>;

#ifdef PNL_RTTI
template<>
const CPNLType &CMatrix< int >::GetStaticTypeInfo()
{
  return CMatrix< int >::m_TypeInfo;
}
#endif
