/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlSparseMatrix.cpp                                         //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlSparseMatrix.hpp"

PNL_USING

#if 0
inline int CSparseMatrix<float>::ConvertToIndex() const
{
    return CX_32F;
}

inline int CSparseMatrix<double>::ConvertToIndex()const
{
    return CX_64F;
}
#endif

#ifdef PNL_RTTI
template<>
const CPNLType &CSparseMatrix< int >::GetStaticTypeInfo()
{
  return CSparseMatrix< int >::m_TypeInfo;
}
#endif