/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlIDTabularPotential.cpp                                   //
//                                                                         //
//  Purpose:   CIDTabularPotential class member functions implementation   //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlIDTabularPotential.hpp"
#include "pnlTabularDistribFun.hpp"
// ----------------------------------------------------------------------------

PNL_USING

CIDTabularPotential* CIDTabularPotential::Create(const intVector& domain,
  CModelDomain* pMD, const float* probData, const float* utilityData)
{
  return CIDTabularPotential::Create(&domain.front(), domain.size(), pMD,
    probData, utilityData);
}
// ----------------------------------------------------------------------------

CIDTabularPotential* CIDTabularPotential::Create(const int *domain,
  int nNodes, CModelDomain* pMD, const float* probData,
  const float* utilityData)
{
  PNL_CHECK_IS_NULL_POINTER(domain);
  PNL_CHECK_IS_NULL_POINTER( pMD );
  PNL_CHECK_LEFT_BORDER( nNodes, 1 );
  
  CIDTabularPotential *pNewParam = new CIDTabularPotential(domain, nNodes, 
    pMD);
  PNL_CHECK_IF_MEMORY_ALLOCATED(pNewParam);
  if (probData)
  {
    pNewParam->AllocProbMatrix(probData, matTable);
  }
  if (utilityData)
  {
    pNewParam->AllocUtilityMatrix(utilityData, matTable);
  }
  return pNewParam;
}
// ----------------------------------------------------------------------------

CIDTabularPotential::CIDTabularPotential(const int *domain, int nNodes, 
  CModelDomain* pMD): CIDPotential(dtTabular, ftIDPotential, domain, nNodes, 
  pMD)
{
}
// ----------------------------------------------------------------------------

CIDTabularPotential::CIDTabularPotential(const CIDTabularPotential* potential):
CIDPotential(potential)
{
}
// ----------------------------------------------------------------------------

CIDTabularPotential* CIDTabularPotential::Copy(
  const CIDTabularPotential *pIDTabPotential)
{
  PNL_CHECK_IS_NULL_POINTER(pIDTabPotential);
  
  CIDTabularPotential *retPot = new CIDTabularPotential(pIDTabPotential);
  PNL_CHECK_IF_MEMORY_ALLOCATED(retPot);

  return retPot;
}
// ----------------------------------------------------------------------------

CFactor* CIDTabularPotential::Clone() const
{
  const CIDTabularPotential* self = this;
  CIDTabularPotential* res = CIDTabularPotential::Copy(self);

  return res;
}
// ----------------------------------------------------------------------------

CFactor* CIDTabularPotential::CloneWithSharedMatrices() 
{
  CIDTabularPotential* resPot = new CIDTabularPotential(this);
  PNL_CHECK_IF_MEMORY_ALLOCATED(resPot);

  return resPot;
}
// ----------------------------------------------------------------------------

void CIDTabularPotential::Dump() const
{
  int i;
  dump()<<"I'm a factor of "<<m_Domain.size()
      <<" nodes. My Distribution type is Tabular.\n";
  dump()<<"My domain is\n";
  for (i = 0; i < m_Domain.size(); i++)
  {
    dump()<<m_Domain[i]<<" ";
  }
  dump()<<"\n";
  GetProbDistribFun()->Dump();
  GetUtilityDistribFun()->Dump();
  dump().flush();
}
// ----------------------------------------------------------------------------

#ifdef PNL_RTTI
const CPNLType CIDTabularPotential::m_TypeInfo = CPNLType("CIDTabularPotential", &(CIDPotential::m_TypeInfo));

#endif
// end of file ----------------------------------------------------------------
