/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlIDPotential.cpp                                          //
//                                                                         //
//  Purpose:   CIDPotential class member functions implementation          //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlIDPotential.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlIDTabularPotential.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlLogUsing.hpp"
// ----------------------------------------------------------------------------

PNL_USING

CIDPotential::CIDPotential(EDistributionType dt, EFactorType ft, 
  CModelDomain* pMD) :CFactor(dt, ft, pMD)
{
  m_UtilityDistribFun = NULL;
}
// ----------------------------------------------------------------------------

CIDPotential::CIDPotential(EDistributionType dt, EFactorType pt, 
  const int *domain, int nNodes, CModelDomain* pMD):
  CFactor(dt, pt, domain, nNodes, pMD)
{
//  int i;
  pConstNodeTypeVector nt;
  intVector dom = intVector( domain, domain+nNodes );
  pMD->GetVariableTypes( dom, &nt );
  
  switch (dt)
  {
    case dtTabular:
    {
      m_UtilityDistribFun = CTabularDistribFun::Create(nNodes,
        &nt.front(), NULL);
      ((CTabularDistribFun*)m_UtilityDistribFun)->SetCheckNegative(false);
      ((CTabularDistribFun*)m_CorrespDistribFun)->SetCheckNegative(false);
      break;
    }
    default:
    {
      PNL_THROW(CNotImplemented, "only tabular now");
    }
  }
}
// ----------------------------------------------------------------------------

CIDPotential::CIDPotential(const CIDPotential* potential):
  CFactor(potential)
{
  delete m_CorrespDistribFun;
  m_CorrespDistribFun = potential->GetProbDistribFun()->Clone();
  m_UtilityDistribFun = potential->GetUtilityDistribFun()->Clone();
}
// ----------------------------------------------------------------------------

void CIDPotential::AllocProbMatrix(const float*data, EMatrixType mType, 
  int numberOfMatrix, const int *discrParentValuesIndices)
{
  CFactor::AllocMatrix(data, mType, numberOfMatrix, discrParentValuesIndices);
}
// ----------------------------------------------------------------------------

void CIDPotential::AllocUtilityMatrix(const float*data, EMatrixType mType, 
  int numberOfMatrix, const int *discrParentValuesIndices)
{
  PNL_CHECK_IS_NULL_POINTER(data);
  
  m_UtilityDistribFun->AllocMatrix(data, mType, numberOfMatrix,
    discrParentValuesIndices);
}
// ----------------------------------------------------------------------------

void CIDPotential::AttachProbMatrix(CMatrix<float> *matrix, EMatrixType mType,
  int numberOfMatrix, const int* discrParentValuesIndices)
{
  CFactor::AttachMatrix(matrix, mType, numberOfMatrix, 
    discrParentValuesIndices);
}
// ----------------------------------------------------------------------------

void CIDPotential::AttachUtilityMatrix(CMatrix<float> *matrix, 
  EMatrixType mType, int numberOfMatrix, const int* discrParentValuesIndices)
{
  PNL_CHECK_IS_NULL_POINTER(matrix);
  
  m_UtilityDistribFun->AttachMatrix(matrix, mType, numberOfMatrix,
    discrParentValuesIndices);
}
// ----------------------------------------------------------------------------

CIDPotential* CIDPotential::Marginalize(const intVector& smallDomain, 
  int maximize, CIDPotential* output) const
{
  int smallDomSize = smallDomain.size();
  const int* pSmallDom = &smallDomain.front();
  return Marginalize(pSmallDom, smallDomSize, maximize, output);
}
// ----------------------------------------------------------------------------

CIDPotential* CIDPotential::Marginalize(const int *pSmallDom, int domSize, 
  int maximize, CIDPotential* output) const
{
  if (domSize > m_Domain.size())
  {
    // bad argument check : Small domain should be subset of the Factor domain
    PNL_THROW(CInconsistentSize, 
      "small domain size more than this domain size" )
  }
  CIDPotential *pNewParam = NULL;
  if (output)
  {
    if (domSize != output->GetDomainSize())
    {
      PNL_THROW(CInconsistentSize, "Marg: output domain does not fit");
    }
    pNewParam = output;
  }
  intVector::const_iterator pEquivPos;

  int *pEqPositions = new int[domSize];
  PNL_CHECK_IF_MEMORY_ALLOCATED( pEqPositions )

  int i;
//  int find;
  for (i = 0; i < domSize; i++)
  {
    pEquivPos = std::find(m_Domain.begin(),m_Domain.end(), pSmallDom[i]);
    if (pEquivPos != m_Domain.end())
    {
      pEqPositions[i] = (pEquivPos - m_Domain.begin());
    }
    else
    {
      PNL_THROW(CInconsistentSize, "small domain isn't subset of domain")
      return NULL;
    }
  }
  EDistributionType dt = m_DistributionType;
  switch (dt)
  {
    case dtTabular:
    {
      CDistribFun *tempDistr = CTabularDistribFun::Copy(
        (CTabularDistribFun* const)(m_UtilityDistribFun));
      tempDistr->MultiplyInSelfData(&m_Domain.front(), &m_Domain.front(),
        m_CorrespDistribFun);
      if (!output)
      {
        pNewParam = CIDTabularPotential::Create(pSmallDom, domSize,
          GetModelDomain());
        pNewParam->GetProbDistribFun()->MarginalizeData(m_CorrespDistribFun, 
          pEqPositions, domSize, maximize);
        pNewParam->GetUtilityDistribFun()->MarginalizeData(tempDistr, 
          pEqPositions, domSize, maximize);
      }
      else
      {
        static_cast<CTabularDistribFun*>(pNewParam->GetProbDistribFun())->
          Marginalize(m_CorrespDistribFun, pEqPositions, domSize, maximize);
        static_cast<CTabularDistribFun*>(pNewParam->GetUtilityDistribFun())->
          Marginalize(tempDistr, pEqPositions, domSize, maximize);
      }
      delete tempDistr;

      CDenseMatrix<float> *probMat = static_cast<CDenseMatrix<float>*>(
        pNewParam->GetProbDistribFun()->GetMatrix(matTable));
      CDenseMatrix<float> *utilityMat = static_cast<CDenseMatrix<float>*>(
        pNewParam->GetUtilityDistribFun()->GetMatrix(matTable));

      const float *probData;
      float *utilityData;
      int DataLength = 0;

      probMat->GetRawData(&DataLength, &probData);
      utilityMat->GetRawData(&DataLength, (const float**)&utilityData);

      for (i = 0; i<DataLength; i++)
      {
//        utilityData[i] = (probData[i]) ? (utilityData[i] / probData[i]) : 0;
        if (probData[i])
        {
          utilityData[i] /= probData[i];
        }
      }
      break;
    }
    default:
    {
      PNL_THROW(CNotImplemented, "we have only Tabular now");
    }
  } // end of switch

  delete [] pEqPositions;

  return pNewParam;
}
// ----------------------------------------------------------------------------

CIDPotential* CIDPotential::Combine(const CIDPotential *pOtherFactor, 
  CIDPotential* output) const
{
  if (GetModelDomain() != pOtherFactor->GetModelDomain())
  {
    PNL_THROW(CInconsistentType,
      "potentials based on the same ModelDomain can be combined")
  }
  EDistributionType my_dt = m_DistributionType;
  EDistributionType other_dt = pOtherFactor->GetDistributionType();
  CIDPotential *resFactor = NULL;
  if (output)
  {
    if (GetModelDomain() != output->GetModelDomain())
    {
      PNL_THROW(CInconsistentType,
        "output potential must base on the same ModelDomain as source potentials")
    }
    EDistributionType out_dt = output->GetDistributionType();
    if (my_dt != out_dt)
    {
      PNL_THROW(CInconsistentType,
        "output and input potentials can't be different types of factors")
    }
    resFactor = output;
  }
  int i;
  if (((my_dt == other_dt) && ((my_dt == dtTabular) || (my_dt == dtGaussian))))
  {
    int s1 = GetDomainSize();
    int s2 = pOtherFactor->GetDomainSize();
    const CIDPotential *bigFactor = NULL;
    const CIDPotential *smallFactor = NULL;
    if (s1 > s2)
    {
      bigFactor = this;
      smallFactor = pOtherFactor;
    }
    else
    {
      bigFactor = pOtherFactor;
      smallFactor = this;
    }
    //check inclusion of small domain in big
    int bigDomSize; 
    const int *bigDomain;
    bigFactor->GetDomain(&bigDomSize, &bigDomain);
    intVector bigDomCheck(bigDomain, bigDomain+bigDomSize);
    int smDomSize;
    const int *smDomain;
    smallFactor->GetDomain(&smDomSize, &smDomain);
    if ((bigDomSize == 2) && (smDomSize == 1))
    {
      if (!((smDomain[0] == bigDomain[0])||(smDomain[0] == bigDomain[1])))
      {
        PNL_THROW(COutOfRange, "small domain isn't subset of big");
      }
    }
    else
    {
      int loc;
      for (i = 0; i < smDomSize; i++)
      {
        loc = std::find(bigDomCheck.begin(), bigDomCheck.end(), 
          smDomain[i] )- bigDomCheck.begin();
        if (loc >= bigDomCheck.size())
        {
          PNL_THROW(COutOfRange, "small domain isn't subset of big");
        }
        bigDomCheck.erase(bigDomCheck.begin()+loc);
      }
    }
    //we create new factor of the same node types as in big domain
    if (!output)
    {
      switch (my_dt)
      {
        case dtTabular:
        {
          resFactor = CIDTabularPotential::Create(bigDomain, bigDomSize,
            GetModelDomain());
          break;
        }
        default:
        {
          PNL_THROW(CNotImplemented, "we have only Tabular now");
        }
      }
    }
    CDistribFun* bigDistr = bigFactor->GetProbDistribFun();
    CDistribFun* smallDistr = smallFactor->GetProbDistribFun();

    resFactor->SetProbDistribFun(bigDistr);
    resFactor->GetProbDistribFun()->MultiplyInSelfData(bigDomain,
      smDomain, smallDistr);

    bigDistr = bigFactor->GetUtilityDistribFun();
    smallDistr = smallFactor->GetUtilityDistribFun();

    resFactor->SetUtilityDistribFun(bigDistr);
    resFactor->GetUtilityDistribFun()->SumInSelfData(bigDomain,
      smDomain, smallDistr);

    return resFactor;
  }
  else
  {
    PNL_THROW(CNotImplemented, "multiply of different types of factors");
  }
  return NULL;
}
// ----------------------------------------------------------------------------

CDenseMatrix<float>* CIDPotential::Contraction(CDenseMatrix<float>* output)
  const
{
  int res_numOfdims;
  const int *res_ranges;
  CDenseMatrix<float>* resMatrix = NULL;
  switch (m_DistributionType)
  {
    case dtTabular:
    {
      CDenseMatrix<float>* probMatrix = static_cast<CDenseMatrix<float>*>(
        GetProbDistribFun()->GetMatrix(matTable));
      probMatrix->GetRanges(&res_numOfdims, &res_ranges);
      if (output)
      {
        if (output->GetMatrixClass() != mcNumericDense)
        {
          PNL_THROW( CInconsistentType, "Incorrect output matrix type");
        }
        int out_numOfdims;
        const int *out_ranges;
        output->GetRanges(&out_numOfdims, &out_ranges);
        if (out_numOfdims != res_numOfdims)
        {
            PNL_THROW(CInconsistentSize, "Output matrix does not fit");
        }
        resMatrix = output;
      }
      else
      {
        resMatrix = CNumericDenseMatrix<float>::Create(res_numOfdims, 
          res_ranges, PNL_FAKEPTR(float));
      }
      resMatrix->SetDataFromOtherMatrix(probMatrix);

      int *dims_to_mul = new int [res_numOfdims];
      for (int i = 0; i < res_numOfdims; i++)
      {
        dims_to_mul[i] = i;
      }
      resMatrix->MultiplyInSelf(GetUtilityDistribFun()->GetMatrix(matTable), 
        res_numOfdims, dims_to_mul);
      delete [] dims_to_mul;
      break;
    }
    default:
    {
      PNL_THROW(CNotImplemented, "we have only Tabular now");
    }
  }
  //??? resMatrix->AddRef must be done
  return resMatrix;
}
// ----------------------------------------------------------------------------

void CIDPotential::DivideInSelf(const CCPD *pOtherFactor) const
{
  if (GetModelDomain() != pOtherFactor->GetModelDomain())
  {
    PNL_THROW(CInconsistentType,
      "potentials based on the same ModelDomain can be divided")
  }

  EDistributionType my_dt = GetDistributionType();
  EDistributionType other_dt = pOtherFactor->GetDistributionType();

  if ((my_dt == other_dt) && ((my_dt == dtTabular) || (my_dt == dtGaussian)))
  {
    int i;
    int s1 = GetDomainSize();
    int s2 = pOtherFactor->GetDomainSize();
    const CIDPotential *bigFactor = NULL;
    const CPotential *smallFactor = NULL;

    if (s1 >= s2)
    {
      bigFactor = this;
      smallFactor = (CPotential*)pOtherFactor;
    }
    else
    {
      PNL_THROW(CInvalidOperation, "we can divide only bigger on smaller")
    }

    //check inclusion of small domain in big
    int bigDomSize;
    const int *bigDomain;
    bigFactor->GetDomain(&bigDomSize, &bigDomain);
    intVector bigDomCheck(bigDomain, bigDomain + bigDomSize);
    int smDomSize;
    const int *smDomain;
    smallFactor->GetDomain(&smDomSize, &smDomain);
    int loc;

    for (i = 0; i < smDomSize; i++)
    {
      loc = std::find(bigDomCheck.begin(), bigDomCheck.end(), smDomain[i]) - 
        bigDomCheck.begin();
      if (loc >= bigDomCheck.size())
      {
        PNL_THROW(COutOfRange, "small domain isn't subset of big");
      }
      bigDomCheck.erase(bigDomCheck.begin() +  loc);
    }

    CDistribFun* smallDistr = smallFactor->GetDistribFun();
    bigFactor->GetProbDistribFun()->DivideInSelfData(bigDomain,
      smDomain, smallDistr);
  }
  else
  {
    PNL_THROW(CNotImplemented, "multiply of different types of factors");
  }
}
// ----------------------------------------------------------------------------

void CIDPotential::InsertPoliticsInSelf(const CCPD *pOtherFactor) const
{
  if (GetModelDomain() != pOtherFactor->GetModelDomain())
  {
    PNL_THROW(CInconsistentType,
      "potentials based on the same ModelDomain can be divided")
  }

  EDistributionType my_dt=GetDistributionType();
  EDistributionType other_dt=pOtherFactor->GetDistributionType();

  if ((my_dt == other_dt) && ((my_dt == dtTabular) || (my_dt == dtGaussian)))
  {
    int i;
    int s1 = GetDomainSize();
    int s2 = pOtherFactor->GetDomainSize();
    const CIDPotential *bigFactor = NULL;
    const CPotential *smallFactor = NULL;

    if (s1 >= s2)
    {
      bigFactor = this;
      smallFactor = (CPotential*)pOtherFactor;
    }
    else
    {
      PNL_THROW(CInvalidOperation, "we can divide only bigger on smaller")
    }

    //check inclusion of small domain in big
    int bigDomSize;
    const int *bigDomain;
    bigFactor->GetDomain(&bigDomSize, &bigDomain);
    intVector bigDomCheck(bigDomain, bigDomain+bigDomSize);
    int smDomSize; 
    const int *smDomain;
    smallFactor->GetDomain(&smDomSize, &smDomain);
    int loc;

    for (i = 0; i < smDomSize; i++)
    {
      loc = std::find(bigDomCheck.begin(), bigDomCheck.end(), 
        smDomain[i]) - bigDomCheck.begin();
      if (loc >= bigDomCheck.size())
      {
        PNL_THROW(COutOfRange, "small domain isn't subset of big");
      }
      bigDomCheck.erase(bigDomCheck.begin() +  loc);
    }

    CDistribFun* smallDistr = smallFactor->GetDistribFun();
    bigFactor->GetProbDistribFun()->MultiplyInSelfData( bigDomain,
      smDomain, smallDistr);
  }
  else
  {
    PNL_THROW(CNotImplemented, "multiply of different types of factors");
  }
}
// ----------------------------------------------------------------------------

void CIDPotential::GetDomain(int *DomainSizeOut, const int **domainOut) const
{
  CFactor::GetDomain(DomainSizeOut, domainOut);
}
// ----------------------------------------------------------------------------

int CIDPotential::GetNumInHeap() const
{
  return CFactor::GetNumInHeap();
}
// ----------------------------------------------------------------------------

void CIDPotential::SetModelDomain(CModelDomain* pMD, bool checkNodeTypesinMD)
{
  CFactor::SetModelDomain(pMD, checkNodeTypesinMD);
}
// ----------------------------------------------------------------------------

Log& CIDPotential::dump()
{
  return *LogPotential();
}
// ----------------------------------------------------------------------------

CIDPotential::~CIDPotential()
{
  if (m_UtilityDistribFun)
  {
    delete(m_UtilityDistribFun);
  }
}
// ----------------------------------------------------------------------------

void CIDPotential::SetProbDistribFun(const CDistribFun* data)
{
  CFactor::SetDistribFun(data);
  ((CTabularDistribFun*)m_CorrespDistribFun)->SetCheckNegative(false);
}
// ----------------------------------------------------------------------------

void CIDPotential::SetUtilityDistribFun(const CDistribFun* data)
{
  if (!data)
  {
    PNL_THROW(CNULLPointer, "data")
  }
  if (!m_UtilityDistribFun)
  {
    PNL_THROW(CInvalidOperation, 
      "no corresponding data - we can't set new data")
  }

  //we need to compare factors of data - they must be the same
  EDistributionType dtCorr = m_UtilityDistribFun->GetDistributionType();
  EDistributionType dtIn = data->GetDistributionType();

  const pConstNodeTypeVector *ntCorr = 
    m_UtilityDistribFun->GetNodeTypesVector();
  const pConstNodeTypeVector *ntIn   = data->GetNodeTypesVector();
  int sizeCorr = ntCorr->size();
  int sizeIn = ntIn->size();

  //fixme - is there any other possible combinations
  if (!((dtCorr == dtIn) || (dtCorr == dtScalar) || (dtIn == dtScalar)))
  {
    PNL_THROW(CInconsistentType, "distribution type")
  }
  if (sizeCorr != sizeIn)
  {
    PNL_THROW(CInconsistentSize, "size of data")
  }

  //now we check all and can replace correaponding data by input data
  delete m_UtilityDistribFun;
  CDistribFun *dat = data->Clone();
  m_UtilityDistribFun = dat;
  ((CTabularDistribFun*)m_UtilityDistribFun)->SetCheckNegative(false);
}
// ----------------------------------------------------------------------------

// methods that was abstract in base class and we have to do stubs for them
void CIDPotential::GenerateSample(CEvidence* evidencesIO, int maximize) const
{
  PNL_THROW(CNotImplemented, "this method is for only CPotential and CCPD");
}
// ----------------------------------------------------------------------------

CPotential* CIDPotential::ConvertStatisticToPot(int numOfSamples) const
{
  PNL_THROW( CNotImplemented, "this method is for only CPotential and CCPD");
}
// ----------------------------------------------------------------------------

float CIDPotential::ProcessingStatisticalData(int numberOfEvidences)
{
  PNL_THROW( CNotImplemented, "this method is for only CPotential and CCPD");
}
// ----------------------------------------------------------------------------

void CIDPotential::UpdateStatisticsEM(const CPotential *pMargPot, 
  const CEvidence *pEvidence)
{
  PNL_THROW( CNotImplemented, "this method is for only CPotential and CCPD");
}
// ----------------------------------------------------------------------------

float CIDPotential::GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes ) const
{
  PNL_THROW( CNotImplemented, "this method is for only CPotential and CCPD");
}
// ----------------------------------------------------------------------------

void CIDPotential::UpdateStatisticsML(const CEvidence* const* pEvidencesIn,
  int EvidenceNumber)
{
  PNL_THROW( CNotImplemented, "this method is for only CPotential and CCPD");
}
// ----------------------------------------------------------------------------

#ifdef PAR_PNL
void CIDPotential::UpdateStatisticsML(CFactor *pPot)
{
  PNL_THROW(CNotImplemented, 
      "UpdateStatisticsML for CIDPotential not implemented yet");
};
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PNL_RTTI
const CPNLType CIDPotential::m_TypeInfo = CPNLType("CIDPotential", &(CFactor::m_TypeInfo));

#endif
// end of file ----------------------------------------------------------------
