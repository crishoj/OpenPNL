/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlSoftMaxCPD.cpp                                           //
//                                                                         //
//  Purpose:   CSoftMaxCPD class member functions implementation           //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Vinogradov                                         //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlSoftMaxCPD.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlCondSoftMaxDistribFun.hpp"
#include "pnlTabularPotential.hpp"
// ----------------------------------------------------------------------------

PNL_USING

CSoftMaxCPD* CSoftMaxCPD::Create(const intVector& domain, CModelDomain* pMD)
{
  return CSoftMaxCPD::Create(&domain.front(), domain.size(), pMD);
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::AllocDistribution(const floatVector& weights,
  const floatVector& offsets, const intVector& parentCombination)
{
  AllocDistribution(&weights.front(), &offsets.front(), 
    &parentCombination.front());
}
// ----------------------------------------------------------------------------

CSoftMaxCPD* CSoftMaxCPD::Create(const int *domain, int nNodes,
  CModelDomain* pMD)
{
  PNL_CHECK_IS_NULL_POINTER(domain);
  PNL_CHECK_IS_NULL_POINTER(pMD);
  PNL_CHECK_LEFT_BORDER(nNodes, 1);
  
  CSoftMaxCPD *pNewParam = new CSoftMaxCPD(domain, nNodes, pMD);
  PNL_CHECK_IF_MEMORY_ALLOCATED(pNewParam);

  return pNewParam;
}
// ----------------------------------------------------------------------------

CSoftMaxCPD* CSoftMaxCPD::Copy(const CSoftMaxCPD* pSMCPD)
{
  PNL_CHECK_IS_NULL_POINTER(pSMCPD);
  
  CSoftMaxCPD *retCPD = new CSoftMaxCPD(*pSMCPD);
  PNL_CHECK_IF_MEMORY_ALLOCATED(retCPD);

  return retCPD;
}
// ----------------------------------------------------------------------------

CFactor* CSoftMaxCPD::CloneWithSharedMatrices()
{
  CSoftMaxCPD* resCPD = new CSoftMaxCPD(this);
  PNL_CHECK_IF_MEMORY_ALLOCATED(resCPD);

  return resCPD;
}
// ----------------------------------------------------------------------------

CFactor* CSoftMaxCPD::Clone() const
{
  const CSoftMaxCPD* self = this;
  CSoftMaxCPD* res = CSoftMaxCPD::Copy(self);

  return res;
}
// ----------------------------------------------------------------------------

CSoftMaxCPD::CSoftMaxCPD(const CSoftMaxCPD& SMCPD):
  CCPD(dtSoftMax, ftCPD, SMCPD.GetModelDomain())
{
  if (SMCPD.m_CorrespDistribFun->GetDistributionType() == dtSoftMax)
  {
    delete m_CorrespDistribFun;
    m_CorrespDistribFun = CSoftMaxDistribFun::Copy(
      static_cast<CSoftMaxDistribFun*>(SMCPD.m_CorrespDistribFun));
  }
  else
  {
    if (SMCPD.m_CorrespDistribFun->GetDistributionType() == dtCondSoftMax)
    {
      delete m_CorrespDistribFun;
      m_CorrespDistribFun = CCondSoftMaxDistribFun::Copy(
        static_cast<CCondSoftMaxDistribFun*>(SMCPD.m_CorrespDistribFun));
    }
    else
    {
      PNL_THROW(CInconsistentType,
        "distribution must be SoftMax or conditional SoftMax")
    }
  }
  m_Domain = intVector(SMCPD.m_Domain);
  m_MaximizingMethod = SMCPD.m_MaximizingMethod;
}
// ----------------------------------------------------------------------------

CSoftMaxCPD::CSoftMaxCPD(const int *domain, int nNodes, CModelDomain* pMD):
  CCPD(dtSoftMax, ftCPD, domain, nNodes, pMD)
{
  m_MaximizingMethod = mmGradient;
}
// ----------------------------------------------------------------------------

CSoftMaxCPD::CSoftMaxCPD(const CSoftMaxCPD* pSMCPD):CCPD(pSMCPD)
{
  m_MaximizingMethod = mmGradient;
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::AllocDistribution(const float* pWeights, 
  const float* pOffsets, const int* parentCombination)
{
  PNL_CHECK_IS_NULL_POINTER(pWeights);
  PNL_CHECK_IS_NULL_POINTER(pOffsets);

  if (m_CorrespDistribFun->GetDistributionType() == dtSoftMax)
  {
    AllocMatrix(pWeights, matWeights);
    static_cast<CSoftMaxDistribFun*>(m_CorrespDistribFun)->
      AllocOffsetVector(pOffsets);
  }
  else
  {
    PNL_CHECK_IS_NULL_POINTER(parentCombination);
    AllocMatrix(pWeights, matWeights, -1, parentCombination);
    static_cast<CCondSoftMaxDistribFun*>(m_CorrespDistribFun)->
      AllocOffsetVector(pOffsets, parentCombination);
  }
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::NormalizeCPD()
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxCPD now ");
}
// ----------------------------------------------------------------------------

CPotential *CSoftMaxCPD::ConvertToPotential() const
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxCPD now ");
  return NULL;
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::InitLearnData()
{
  if (m_CorrespDistribFun->GetDistributionType() == dtSoftMax)
  {
    static_cast<CSoftMaxDistribFun*>(m_CorrespDistribFun)->InitLearnData();
  }
  else
  {
    static_cast<CCondSoftMaxDistribFun*>(m_CorrespDistribFun)->
      InitLearnData();
  }
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::CopyLearnDataToDistrib()
{
  if (m_CorrespDistribFun->GetDistributionType() == dtSoftMax)
  {
    static_cast<CSoftMaxDistribFun*>(m_CorrespDistribFun)->
      CopyLearnDataToDistrib();
  }
  else
  {
    static_cast<CCondSoftMaxDistribFun*>(m_CorrespDistribFun)->
      CopyLearnDataToDistrib();
  }
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::UpdateStatisticsEM(const CPotential *pMargPot,
  const CEvidence *pEvidence)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxCPD now ");
}
// ----------------------------------------------------------------------------

float CSoftMaxCPD::ProcessingStatisticalData(int numberOfEvidences)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxCPD now ");
  return 0;
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::UpdateStatisticsML(const CEvidence* const* pEvidences,
  int EvidenceNumber)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxCPD now ");
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::GenerateSample(CEvidence* evidence, int maximize) const
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxCPD now ");
}
// ----------------------------------------------------------------------------

CPotential* CSoftMaxCPD::ConvertStatisticToPot(int numOfSamples) const
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxCPD now ");

  return NULL;
}
// ----------------------------------------------------------------------------

float CSoftMaxCPD::GetLogLik(const CEvidence* pEv, 
  const CPotential* pShrInfRes) const
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxCPD now ");

  return 0;
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::SetMaximizingMethod(EMaximizingMethod met)
{
  m_MaximizingMethod = met;
  if (m_CorrespDistribFun->GetDistributionType() == dtSoftMax)
  {
    static_cast<CSoftMaxDistribFun*>(m_CorrespDistribFun)->
      SetMaximizingMethod(met);
  }
  else
  {
    static_cast<CCondSoftMaxDistribFun*>(m_CorrespDistribFun)->
      SetMaximizingMethod(met);
  }
}
// ----------------------------------------------------------------------------

EMaximizingMethod CSoftMaxCPD::GetMaximizingMethod()
{
  return m_MaximizingMethod;
}
// ----------------------------------------------------------------------------

void CSoftMaxCPD::MaximumLikelihood(float **Observation,
  int NumberOfObservations, float Accuracy, float step)
{
  if (m_CorrespDistribFun->GetDistributionType() == dtSoftMax)
  {
    static_cast<CSoftMaxDistribFun*>(m_CorrespDistribFun)->
      MaximumLikelihood(Observation, NumberOfObservations, Accuracy);
  }
  else
  {
    static_cast<CCondSoftMaxDistribFun*>(m_CorrespDistribFun)->
      MaximumLikelihood(Observation, NumberOfObservations, Accuracy);
  }
}
//------------------------------------------------------------------------------
CPotential* CSoftMaxCPD::ConvertWithEvidenceToTabularPotential(
        const CEvidence* pEvidence,
        int flagSumOnMixtureNode ) const
{
    
        //need to convert to potential and after that add evidence
        CPotential* potWithoutEv = ConvertToTabularPotential(pEvidence);
        CPotential* potWithEvid = potWithoutEv->ShrinkObservedNodes(pEvidence);
        delete potWithoutEv;
        return potWithEvid;
    
}
//-----------------------------------------------------------------------------
CPotential *CSoftMaxCPD::ConvertToTabularPotential(const CEvidence* pEvidence) const
{

    //searching discrite nodes in domain
    intVector discriteNodesPosInDom;
    int domSize = m_Domain.size();
    int numSoftMaxNode;
    int discrDomSize = 0;
    int *parentIndexes;
    int SoftMaxSize;
    const pConstNodeTypeVector* ntVec = GetDistribFun()->GetNodeTypesVector();
    int i;
    for (i = 0; i < domSize; i++)
    {
        if ((*ntVec)[i]->IsDiscrete())
        {
            discriteNodesPosInDom.push_back(m_Domain[i]);
            SoftMaxSize = (*ntVec)[i]->GetNodeSize();
            numSoftMaxNode = i;
        }
    };
    discrDomSize=discriteNodesPosInDom.size();
    
    //fill parents indexes vector
    parentIndexes = new int[discrDomSize-1];
    for( i = 0; i < discrDomSize-1; i++ )
    {
        parentIndexes[i]=discriteNodesPosInDom[i];
    }    
 
    // creating new evidece that contain all observed nodes in this domain
    intVector pObsNodes;
    pConstValueVector pObsValues;
    pConstNodeTypeVector pNodeTypes;
    pEvidence->GetObsNodesWithValues(&pObsNodes,&pObsValues,&pNodeTypes);
    int *obsNodes;
    int obsNodesSize;
    obsNodesSize=pObsNodes.size();
    obsNodes = new int[obsNodesSize];
    for(i = 0;i < obsNodesSize; i++)
        obsNodes[i] = pObsNodes[i];
    CEvidence *pCopyEvidence;
    valueVector cpyValVect(0);
    for(i = 0; i < obsNodesSize; i++)
        cpyValVect.push_back(*(pObsValues[i]));
    
    pCopyEvidence = CEvidence::Create(pEvidence->GetModelDomain(), obsNodesSize, 
        obsNodes,(const valueVector&)cpyValVect);
    for(i = 0; i < pObsNodes.size(); i++)
    {
        if((std::find(m_Domain.begin(),m_Domain.end(), pObsNodes[i])) == m_Domain.end())
            pCopyEvidence->MakeNodeHidden(pObsNodes[i]);
    };
    
    //creating tabular potential 
    CTabularPotential *resFactor = CTabularPotential::Create(
        GetModelDomain(),discriteNodesPosInDom);

    if( m_DistributionType == dtSoftMax)
        resFactor->AttachMatrix(((CSoftMaxDistribFun*)m_CorrespDistribFun)->GetProbMatrix(pCopyEvidence),matTable);
    else
    {
        if(m_DistributionType == dtCondSoftMax)
            resFactor->AttachMatrix(((CCondSoftMaxDistribFun*)m_CorrespDistribFun)->GetProbMatrix(pCopyEvidence),matTable);
        
        else
        {
            PNL_THROW( CInconsistentType,
                "distribution must be SoftMax or conditional SoftMax" )
        }
    }
    delete []parentIndexes;
    delete []obsNodes;
    return resFactor;

}
// end of file ----------------------------------------------------------------
