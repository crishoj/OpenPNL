/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlIDPotential.hpp                                          //
//                                                                         //
//  Purpose:   CIDPotential class definition                               //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLIDPOTENTIAL_HPP__
#define __PNLIDPOTENTIAL_HPP__

#include "pnlParConfig.hpp"
#include "pnlConfig.hpp"
#include "pnlFactor.hpp"
#include "pnlLog.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API CIDPotential : public CFactor
{
public:
  virtual CFactor* Clone() const = 0;

  virtual CFactor* CloneWithSharedMatrices() = 0;

  // This function has different implementation for TabularFactor,
  // GaussianFactor etc

  void AllocProbMatrix(const float*data, EMatrixType mType,
    int numberOfMatrix = -1, const int *discrParentValuesIndices = NULL);

  void AllocUtilityMatrix(const float*data, EMatrixType mType,
    int numberOfMatrix = -1, const int *discrParentValuesIndices = NULL);

  void AttachProbMatrix(CMatrix<float> *matrix, EMatrixType mType,
    int numberOfMatrix = -1, const int* discrParentValuesIndices = NULL);

  void AttachUtilityMatrix(CMatrix<float> *matrix, EMatrixType mType,
    int numberOfMatrix = -1, const int* discrParentValuesIndices = NULL);

  virtual CIDPotential* Marginalize(const intVector& smallDomainIn,
    int maximize = 0, CIDPotential* output = NULL) const;

#ifdef PNL_OBSOLETE
  CIDPotential *Marginalize(const int *pSmallDom, int domSize,
    int maximize = 0, CIDPotential* output = NULL) const;
#endif

  virtual CIDPotential* Combine(const CIDPotential *pOtherFactor,
    CIDPotential* output = NULL) const;

  virtual CDenseMatrix<float>* Contraction(CDenseMatrix<float>* output = NULL)
    const;

  virtual void DivideInSelf(const CCPD *pOtherFactor) const;

  virtual void InsertPoliticsInSelf(const CCPD *pOtherFactor) const;

  virtual void Dump() const = 0;

#ifndef SWIG
  static Log& dump();
#endif

  virtual ~CIDPotential();

  // from CFactor:

  // methods that are necessary to us for work with Model Domain:
  inline CDistribFun* GetProbDistribFun() const;

  inline CDistribFun* GetUtilityDistribFun() const;

  inline EFactorType GetFactorType() const;

  inline EDistributionType GetDistributionType() const;

  inline CModelDomain* GetModelDomain() const;

  inline int GetDomainSize() const;

  void GetDomain(int *DomainSizeOut, const int **domainOut) const;

  int GetNumInHeap() const;

  void SetProbDistribFun(const CDistribFun* data);

  void SetUtilityDistribFun(const CDistribFun* data);

#ifdef PAR_PNL
  virtual void UpdateStatisticsML(CFactor *pPot);
#endif

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CIDPotential::m_TypeInfo;
  }
#endif
protected:
  CIDPotential(EDistributionType dt, EFactorType pt, const int *domain,
    int nNodes, CModelDomain* pMD);

  CIDPotential(EDistributionType dt, EFactorType ft, CModelDomain* pMD);

  CIDPotential(const CIDPotential* potential);

  // methods that was abstract in base class and we have to do corks for them
  virtual void GenerateSample(CEvidence* evidencesIO, int maximize = 0) const;

  virtual CPotential* ConvertStatisticToPot(int numOfSamples) const;

  virtual float ProcessingStatisticalData(int numberOfEvidences);

  virtual void UpdateStatisticsEM(const CPotential *pMargPot,
    const CEvidence *pEvidence = NULL);

  virtual float GetLogLik(const CEvidence* pEv,
    const CPotential* pShrInfRes = NULL ) const;

  virtual void UpdateStatisticsML(const CEvidence* const* pEvidencesIn,
    int EvidenceNumber);

  //change model domain and corresponding node types in factor
  void SetModelDomain(CModelDomain* pMD, bool checkNodeTypesinMD = 1);

  CDistribFun *m_UtilityDistribFun; // utility distribution;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
};

inline EFactorType CIDPotential::GetFactorType() const
{
  return CFactor::GetFactorType();
}

inline EDistributionType CIDPotential::GetDistributionType() const
{
  return CFactor::GetDistributionType();
}

inline CDistribFun* CIDPotential::GetProbDistribFun()const
{
  return CFactor::GetDistribFun();
}

inline CDistribFun* CIDPotential::GetUtilityDistribFun()const
{
  return m_UtilityDistribFun;
}

inline CModelDomain* CIDPotential::GetModelDomain() const
{
  return CFactor::GetModelDomain();
}

inline int CIDPotential::GetDomainSize() const
{
  return CFactor::GetDomainSize();
}

PNL_END

#endif //__PNLIDPOTENTIAL_H__
