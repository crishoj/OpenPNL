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
#include "pnlGaussianPotential.hpp"
#include "pnlFactor.hpp"
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
    int i;
	int NumContPar = 0;
	for(i = 0; i<nNodes; i++)
	{
		if (!pMD->GetVariableType(domain[i])->IsDiscrete()) 
		{
			NumContPar++;
		}
	}
	if(NumContPar == 0 )
	{
		PNL_THROW(CInconsistentType,
			"SoftMax node does not have continuous parents");          
	}
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
    
////////////////////////////////////////////////
    const CNodeType *nt;
    nt = GetModelDomain()->GetVariableType( m_Domain[m_Domain.size()-1] );
    int SoftMaxSize = nt->GetNodeSize();

    if (SoftMaxSize == 2)
    {
      int matSize = 0;
      int i;
      for (i = 0; i < m_Domain.size(); i++)
      {
         nt = GetModelDomain()->GetVariableType( m_Domain[i] );
         if(!(nt->IsDiscrete()))
         {
            matSize ++;
         }
      }
      matSize = matSize;

      for (i = 0; i < 2*matSize-1; i+=2)
      {
        if (pWeights[i] - pWeights[i+1] == 0)
          PNL_THROW(CNotImplemented, "sigmoid must have distinct weights");
      }
    }
////////////////////////////////////////////////

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
  //need to check
  //is all parents observed
  int NNodes = m_Domain.size();
  bool isObserved = true;

  for (int node = 0; node < NNodes - 1; node++) 
    if (!evidence->IsNodeObserved(m_Domain[node]))
      isObserved = false;
     
  CPotential *pTabPot;

  if (!isObserved) {
		PNL_THROW(CAlgorithmicException, "all parents must be observed");
	}

  pTabPot = ConvertWithEvidenceToTabularPotential(evidence);

  pTabPot->GenerateSample(evidence);

  delete pTabPot;
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
    const CEvidence* pEvidence, int flagSumOnMixtureNode ) const
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

    discrDomSize = discriteNodesPosInDom.size();
    
    //fill parents indexes vector
    parentIndexes = new int[discrDomSize-1];
    for( i = 0; i < discrDomSize-1; i++ )
    {
        parentIndexes[i] = discriteNodesPosInDom[i];
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
    {
        obsNodes[i] = pObsNodes[i];
    }

    CEvidence *pCopyEvidence;
    valueVector cpyValVect(0);
    for(i = 0; i < obsNodesSize; i++)
    {
        cpyValVect.push_back(*(pObsValues[i]));
    }
    
    pCopyEvidence = CEvidence::Create(pEvidence->GetModelDomain(), obsNodesSize, 
        obsNodes,(const valueVector&)cpyValVect);

    for(i = 0; i < pObsNodes.size(); i++)
    {
        if((std::find(m_Domain.begin(),m_Domain.end(), pObsNodes[i])) == m_Domain.end())
        {
            pCopyEvidence->MakeNodeHidden(pObsNodes[i]);
        }
    };
    
    //creating tabular potential 
    CTabularPotential *resFactor = CTabularPotential::Create(
        GetModelDomain(),discriteNodesPosInDom);

    if( m_DistributionType == dtSoftMax)
    {
        resFactor->AttachMatrix(((CSoftMaxDistribFun*)m_CorrespDistribFun)->
        GetProbMatrix(pCopyEvidence),matTable);
    }
    else
    {
        if(m_DistributionType == dtCondSoftMax)
        {
            resFactor->AttachMatrix(((CCondSoftMaxDistribFun*)m_CorrespDistribFun)->
            GetProbMatrix(pCopyEvidence),matTable);
        }
        else
        {
            PNL_THROW( CInconsistentType,
                "distribution must be SoftMax or conditional SoftMax" )
        }
    }

    delete [] parentIndexes;
    delete [] obsNodes;
    delete pCopyEvidence;
    return resFactor;
}

//-----------------------------------------------------------------------------
CPotential* CSoftMaxCPD::ConvertWithEvidenceToGaussianPotential(
    const CEvidence* pEvidence,
    floatVector MeanContParents, 
    C2DNumericDenseMatrix<float>* CovContParents,
    const int *parentIndices,
    int flagSumOnMixtureNode ) const
{
    int SoftMaxSize = GetSoftMaxSize();
    if (SoftMaxSize != 2)
    {
        PNL_THROW(CNotImplemented, "It is not sigmoid");
    }
    else
    {
        if (m_CorrespDistribFun->GetDistributionType() == dtSoftMax)
        {
            CPotential* pot = ConvertToGaussianPotential(pEvidence, 
                m_CorrespDistribFun, MeanContParents, CovContParents);

            CPotential *pot2 = NULL;

            int domSize = pot->GetDomainSize();
            bool IsAllContUnobserved = true;
            const pConstNodeTypeVector* ntVec = pot->GetDistribFun()->GetNodeTypesVector();
            for( int i = 0; i < domSize-1; i++  )    
            {
              intVector Domain;
              pot->GetDomain(&Domain);
              int curNode =  Domain[i];
              if( (pEvidence->IsNodeObserved(curNode)))
              {
                if( !(*ntVec)[i]->IsDiscrete() )
                {
                  IsAllContUnobserved = false;
                }
              }
            }

            if ((pot->GetDomainSize() >= 3)&&(!IsAllContUnobserved))
            {
              pot2 = pot->ShrinkObservedNodes(pEvidence);
            }
            else
            {
              intVector Domain;
              pot->GetDomain(&Domain);
              pot2 = pot->Marginalize(&(Domain[0]), 1);
            }
            delete pot;
            return pot2;
        }
        else //it means m_CorrespDistribFun->GetDistributionType == dtCondSoftMax
        {
            int i;
            const CSoftMaxDistribFun* dtSM;

            dtSM = 
                static_cast<CCondSoftMaxDistribFun*>(m_CorrespDistribFun)->
                GetDistribution(parentIndices);
            
            intVector pObsNodes;
            pConstValueVector pObsValues;
            pConstNodeTypeVector pNodeTypes;
            pEvidence->GetObsNodesWithValues(&pObsNodes, &pObsValues, &pNodeTypes);
            
            int r = -1;
            for (i = 0; i < pObsNodes.size(); i++)
            {
                if (m_Domain[m_Domain.size()-1] == pObsNodes[i])
                {
                    r = pObsValues[i]->GetInt();
                    break;
                }
            }
            if (r == -1)
            {
                PNL_THROW(CNotImplemented, "Not exist evidence");
            }
            
            CDistribFun *gauFactData = const_cast<CSoftMaxDistribFun*>(dtSM)->
                ConvertCPDDistribFunToPotential(MeanContParents, CovContParents, r);
            
            intVector gauSubDomain;
            const CNodeType *nt;
            for(i = 0; i < m_Domain.size(); i++)
            {
                nt = GetModelDomain()->GetVariableType( m_Domain[i] );
                if(!(nt->IsDiscrete()))
                {
                    gauSubDomain.push_back(m_Domain[i]);
                }
            }
            
            intVector obsIndex;
            for( i = 0; i < gauSubDomain.size(); i++ )
            {
                if( pEvidence->IsNodeObserved(gauSubDomain[i]) )
                {
                    obsIndex.push_back( i );
                }
            }
            
            CGaussianPotential *resFactor = CGaussianPotential::Create(&gauSubDomain.front(), 
                gauSubDomain.size(), GetModelDomain());
            
            resFactor->SetDistribFun( gauFactData );


            CPotential *pot = NULL;

            int domSize = resFactor->GetDomainSize();
            bool IsAllContUnobserved = true;
            const pConstNodeTypeVector* ntVec = resFactor->GetDistribFun()->GetNodeTypesVector();
            for( i = 0; i < domSize-1; i++  )    
            {
              intVector Domain;
              resFactor->GetDomain(&Domain);
              int curNode =  Domain[i];
              if( (pEvidence->IsNodeObserved(curNode)))
              {
                if( !(*ntVec)[i]->IsDiscrete() )
                {
                  IsAllContUnobserved = false;
                }
              }
            }
            if ((resFactor->GetDomainSize() >= 3)&&(!IsAllContUnobserved))
            {
              pot = resFactor->ShrinkObservedNodes(pEvidence);
            }
            else
            {
              intVector Domain;
              resFactor->GetDomain(&Domain);
              pot = resFactor->Marginalize(&(Domain[0]), 1);
            }
            delete resFactor;
            
            delete gauFactData;
            return pot;
            
        }
    }
}
//-----------------------------------------------------------------------------
CPotential *CSoftMaxCPD::ConvertToGaussianPotential(const CEvidence* pEvidence,
    CDistribFun *df,
    floatVector MeanContParents, 
    C2DNumericDenseMatrix<float>* CovContParents) const
{
    int i; 
    
    intVector pObsNodes;
    pConstValueVector pObsValues;
    pConstNodeTypeVector pNodeTypes;
    pEvidence->GetObsNodesWithValues(&pObsNodes, &pObsValues, &pNodeTypes);
    
    int r = -1;
    for (i = 0; i < pObsNodes.size(); i++)
    {
        if (m_Domain[m_Domain.size()-1] == pObsNodes[i])
        {
            r = pObsValues[i]->GetInt();
            break;
        }
    }
    if (r == -1)
    {
        PNL_THROW(CNotImplemented, "Not exist evidence");
    }
        
    CDistribFun *gauFactData = static_cast<CSoftMaxDistribFun*>(m_CorrespDistribFun)->
        ConvertCPDDistribFunToPotential(MeanContParents, CovContParents, r);
    
    CGaussianPotential *resFactor = CGaussianPotential::Create(	&m_Domain.front(), 
        m_Domain.size(), GetModelDomain());
    
    resFactor->SetDistribFun( gauFactData );
    //resFactor->Dump();
    //resFactor = resFactor->ShrinkObservedNodes(pEvidence);
    //resFactor->Dump();
    delete gauFactData;
    return resFactor;
}
//-----------------------------------------------------------------------------

int CSoftMaxCPD::GetSoftMaxSize() const
{
    if (m_CorrespDistribFun->GetDistributionType() == dtSoftMax)
        return static_cast<CSoftMaxDistribFun*>(m_CorrespDistribFun)->GetSoftMaxSize();
    else
        return static_cast<CCondSoftMaxDistribFun*>(m_CorrespDistribFun)->GetSoftMaxSize();
}
//-----------------------------------------------------------------------------

#ifdef PAR_PNL
void CSoftMaxCPD::UpdateStatisticsML(CFactor *pPot)
{
    PNL_THROW(CNotImplemented,
        "UpdateStatisticsML for CSoftMaxCPD not implemented yet");
};
#endif // PAR_OMP
//-----------------------------------------------------------------------------

void CSoftMaxCPD::CreateMeanAndCovMatrixForNode(int Node, const CEvidence* pEvidence, 
    const CBNet *pBNet, floatVector &Mean, 
    C2DNumericDenseMatrix<float>**CovContParents) const
{
    int i, j, k;
    int *multiindex = new int [2];
    intVector contParents;
    pBNet->GetContinuousParents(Node, &contParents);
                   
    intVector lineSizes;
    lineSizes.assign(2, contParents.size());
    floatVector zerodata;
    zerodata.assign(contParents.size()*contParents.size(), 0.0f);
    
    *CovContParents = 
        C2DNumericDenseMatrix<float>::Create( &lineSizes.front(), &zerodata.front() );
    for (i = 0; i < contParents.size(); i++ )    
    {
        for (j = 0; j < contParents.size(); j++ )
        {
            multiindex[0] = i;
            multiindex[1] = j;
            (*CovContParents)->SetElementByIndexes(0.0f, multiindex);
        }
    }
 
    intVector pObsNodes;
    pConstValueVector pObsValues;
    pConstNodeTypeVector pNodeTypes;
    pEvidence->GetObsNodesWithValues(&pObsNodes, &pObsValues, &pNodeTypes);

    Mean.resize(0);
    for ( i = 0; i < contParents.size(); i++)
    { 
       int CurNode = contParents[i];  
       CFactor *param;
       param = pBNet->GetFactor(CurNode);
       intVector parentOut;
       intVector DiscrParents;
       pBNet->GetDiscreteParents(CurNode, &DiscrParents);
          
       int *parentComb = new int [DiscrParents.size()];
    
       for ( j = 0; j < DiscrParents.size(); j++)
       {
            for ( k = 0; k < pObsNodes.size(); k++)
            {
                if (DiscrParents[j] == pObsNodes[k])
                {
                    parentComb[j] = pObsValues[k]->GetInt();
                    break;
                }
            }
       }
                    
       const float *data;
       int datasize;
                    
       static_cast<CDenseMatrix<float>*>(param->GetMatrix(matMean, -1, parentComb))->
           GetRawData(&datasize, &data);
       Mean.push_back(data[0]);
                    
       static_cast<CDenseMatrix<float>*>(param->GetMatrix(matCovariance, -1, parentComb))->
           GetRawData(&datasize, &data);
       multiindex[0] = i;
       multiindex[1] = i;
       (*CovContParents)->SetElementByIndexes(data[0], multiindex);
       delete [] parentComb;
    }
    delete [] multiindex;
}

//-----------------------------------------------------------------------------
void CSoftMaxCPD::CreateAllNecessaryMatrices( int typeOfMatrices )
{
    PNL_CHECK_RANGES( typeOfMatrices, 1, 1 );
    //we have only one type of matrices now
    if( m_CorrespDistribFun->IsDistributionSpecific() == 1 )
    {
        PNL_THROW( CInconsistentType,
            "uniform distribution can't have any matrices with data" );
    }
    m_CorrespDistribFun->CreateDefaultMatrices(typeOfMatrices);
}
//-------------------------------------------------------------------------------
 void CSoftMaxCPD::BuildCurrentEvidenceMatrix(float ***full_evid, float ***evid,intVector family,int numEv)
{
  int i, j;
  *evid = new float* [family.size()];
  for (i = 0; i < family.size(); i++)
  {
    (*evid)[i] = new float [numEv];
  }

  for (i = 0; i < numEv; i++)
  {
    for (j = 0; j < family.size(); j++)
    {
      (*evid)[j][i] = (*full_evid)[family[j]][i];
    }
  }
}
//--------------------------------------------------------------------------------

#ifdef PNL_RTTI
const CPNLType CSoftMaxCPD::m_TypeInfo = CPNLType("CSoftMaxCPD", &(CCPD::m_TypeInfo));

#endif

// end of file ----------------------------------------------------------------
