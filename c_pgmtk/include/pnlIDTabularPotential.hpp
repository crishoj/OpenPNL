/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlIDTabularPotential.hpp                                   //
//                                                                         //
//  Purpose:   CIDTabularPotential class definition                        //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLIDTABULARPOTENTIAL_HPP__
#define __PNLIDTABULARPOTENTIAL_HPP__

#include "pnlIDPotential.hpp"

PNL_BEGIN

class PNL_API CIDTabularPotential : public CIDPotential  
{
public:
  static CIDTabularPotential* Create(const intVector& domain,
    CModelDomain* pMD, const float* probData, const float* utilityData);

  static CIDTabularPotential* Create(const int *domain, int nNodes,
    CModelDomain* pMD, const float* probData = NULL, 
    const float* utilityData = NULL);

  static CIDTabularPotential* Copy(const CIDTabularPotential *pIDTabPotential);

  virtual CFactor* Clone() const;

  virtual CFactor* CloneWithSharedMatrices();
  
  virtual void Dump()const;
  
  virtual ~CIDTabularPotential(){}

protected:
  CIDTabularPotential(const int *domain, int nNodes, CModelDomain* pMD);

  CIDTabularPotential(const CIDTabularPotential* potential);
  
private:
};

PNL_END

#endif //__PNLIDTABULARPOTENTIAL_H__
