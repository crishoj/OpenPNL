/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlEmLearningEngineDBN.cpp                                  //
//                                                                         //
//  Purpose:   Implementation of the EM factor learning algorithm for   //
//             Dynamic Bayesian Network                                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include <math.h>
#include "pnlTabularDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "pnlEmLearningEngineDBN.hpp"
#include "pnlDynamicLearningEngine.hpp"
#include "pnlBKInferenceEngine.hpp"
#include "pnlTabularPotential.hpp"
#include "pnl1_5SliceJtreeInferenceEngine.hpp"


PNL_USING

CEMLearningEngineDBN::CEMLearningEngineDBN(CDynamicGraphicalModel *pGrModel,
					   ELearningTypes LearnType, CDynamicInfEngine *pInfEng):

CDynamicLearningEngine( pGrModel, LearnType ),
m_maxIter(10),
m_precisionEM(0.01f),
m_pInfEngine(pInfEng),
m_bDelInf(false)

{
    PNL_CHECK_IS_NULL_POINTER(m_pGrModel);
    
}

CEMLearningEngineDBN::~CEMLearningEngineDBN()
{
    pnlVector< pConstEvidenceVector * >::iterator iter = m_VecPVecPEvidences.begin();
    
    for(; m_VecPVecPEvidences.end() - iter; iter++)
    {
	delete *iter;
    }
    if(m_bDelInf)
    {
	delete m_pInfEngine;
    }
}

CEMLearningEngineDBN * CEMLearningEngineDBN::
Create(CDBN *pDBN, CDynamicInfEngine *pInfEng)
{
    if(!pDBN)
    {
	PNL_THROW(CNULLPointer, "dynamic graphical model")
    }
    
    if( pInfEng )
    {
	if( pInfEng->GetDynamicModel() != pDBN )
	{
	    PNL_THROW(CBadArg, "inference engine");
	}
    }
    
    
    return (new CEMLearningEngineDBN( pDBN, itDBNParamLearnEm, pInfEng ));
    
}



void CEMLearningEngineDBN::SetMaxIterEM(int Niter )
{
    /*define maximum number of iterations*/
    if( Niter <= 0 )
    {
	PNL_THROW(COutOfRange, "number of iteration must be positively" )
    }
    
    m_maxIter = Niter;
}

void  CEMLearningEngineDBN::SetTerminationToleranceEM(float precision)
{
    if( precision <= FLT_MIN )
    {
	PNL_THROW(CBadConst, "precision be positively" )
    }
    m_precisionEM = precision;
    
}

void CEMLearningEngineDBN::Learn()
{
    int iteration = 0;
    int domainNodes = 0;
    CBNet *pStaticGrModel = static_cast<CBNet *>(this->GetDynamicModel()->GetStaticModel());
    
    int nParamPerSlice = m_pGrModel->GetNumberOfNodes();
    intVector *pFlagsIsAllObserved = GetObservationsFlags();
    bool IsTableObserved = true;
    
    const intVector::iterator loc = std::find((*pFlagsIsAllObserved).begin(),
        (*pFlagsIsAllObserved).end(), 0);
    if(loc != (*pFlagsIsAllObserved).end())
    {
        IsTableObserved = 0;
	if( !m_pInfEngine )
	{
	    m_pInfEngine = CBKInfEngine::Create( GetDynamicModel(), 1);
	    m_bDelInf = true;
	}
    }
    
    CFactor *factor = NULL;
    pnlVector< pConstEvidenceVector * >::iterator iterColumn;
    int series = 0;
    
    
    int domainSize;
    const int *domain;
    const CPotential *pMargJPD;
    float loglik;
    bool bContinue;
    
    do
    {
	/*
	if(m_pInfEngine)
		{
		    delete (m_pInfEngine);
		}
	*/
	
	if(iteration)
	{
	    for(int node = 0; node < 2*nParamPerSlice; node++)
	    {
		pStaticGrModel->GetFactor(node)->GetDistribFun()->ClearStatisticalData();
	    }
	    
	}
	iteration++;
	iterColumn = m_VecPVecPEvidences.begin();
	for(series = 0; m_VecPVecPEvidences.end() - iterColumn; iterColumn++, series++)
	{
	    
	    int nTimeSlices = (*iterColumn)->size();
            const CEvidence** pEv= &(*iterColumn)->front();
            if(!(*pFlagsIsAllObserved)[series])
            {
		
                m_pInfEngine->DefineProcedure(ptSmoothing, nTimeSlices);
                m_pInfEngine->EnterEvidence(pEv, nTimeSlices);
                m_pInfEngine->Smoothing();
            }
            for( domainNodes = 0; domainNodes < nParamPerSlice;
            domainNodes++ )
            {
                /*compute statistics for prior slice */
                factor = m_pGrModel->GetFactor( domainNodes );
		
                factor->GetDomain( &domainSize, &domain );
		
                if(IsDomainObserved(domainSize, domain, *iterColumn, nParamPerSlice, 0))
                {
                    factor->UpdateStatisticsML(pEv, 1);
                }
                else
                {
                    m_pInfEngine->MarginalNodes( domain, domainSize, 0, 1);
                    pMargJPD = m_pInfEngine->GetQueryJPD();
		    
                    factor->UpdateStatisticsEM( pMargJPD, *pEv );
                }
            }
            for( domainNodes = nParamPerSlice;
	    domainNodes < 2*nParamPerSlice; domainNodes++ )
	    {
		/*compute statistics for 1_5 slice*/
		factor = m_pGrModel->GetFactor( domainNodes );
		factor->GetDomain( &domainSize, &domain );
		
		
                for(int slice = 1 ; slice < nTimeSlices; slice++)
                {
                    CEvidence* pTmpEv = CreateEvIfDomainObserved(domainSize, domain,
                        *iterColumn, slice, this->GetDynamicModel());
		    
                    if( pTmpEv && pTmpEv->GetNumberObsNodes() == domainSize )
                    {
                        factor->UpdateStatisticsML(&pTmpEv, 1);
			
                    }
                    else
                    {
                        m_pInfEngine->MarginalNodes( domain, domainSize, slice, 1);
                        pMargJPD = m_pInfEngine->GetQueryJPD();
                        factor->UpdateStatisticsEM( pMargJPD, pTmpEv );
                    }
                    delete pTmpEv;
                }
            }
	    
	}
	
	loglik = 0.0f;
	for( domainNodes = 0; domainNodes < nParamPerSlice;
	domainNodes++ )
	{
	    
	    factor = m_pGrModel->GetFactor( domainNodes );
	    
	    loglik += factor->ProcessingStatisticalData(m_numberOfAllEv0Slice);
	}
	
	for( domainNodes = nParamPerSlice; domainNodes < 2*nParamPerSlice;
	domainNodes++ )
	{
	    factor = m_pGrModel->GetFactor( domainNodes );
	    loglik += factor->ProcessingStatisticalData(m_numberOfAllEvISlice);
	}
	
	if( IsTableObserved || m_maxIter == 1)
	{ 
	    bContinue = false;
	}
	else
	{
	    bool flag = iteration == 1 ? true : 
	    (fabs(2*(m_critValue.back()-loglik)/(m_critValue.back() + loglik)) > m_precisionEM );
	    
	    bContinue = m_maxIter > iteration && flag;
	    
	}	
	m_critValue.push_back(loglik);
	
	
	}while(bContinue);
	
}



CEvidence* CEMLearningEngineDBN::CreateEvIfDomainObserved(int nnodes, const int* domain,
							  const pConstEvidenceVector* pEvidences, 
							  int slice,
							  CDynamicGraphicalModel* pGrModel)
{
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    PNL_CHECK_RANGES(slice, 1, (*pEvidences).size()-1);
    int nnodesPerSlice = pGrModel->GetNumberOfNodes();
    
    const CModelDomain *pMD = pGrModel->GetModelDomain();
    const CNodeType *nt;
    valueVector obsVals;
    intVector obsNodes;
    int i;
    for( i =0; i < nnodes; i++ )
    {
	int nodeNumber = domain[i];
	
        if( nodeNumber < nnodesPerSlice )
        {
            if((*pEvidences)[slice - 1]->IsNodeObserved(nodeNumber))
            {
                obsNodes.push_back(nodeNumber);
                nt =  pMD->GetVariableType(nodeNumber);
                int nObsVals = nt->IsDiscrete() ? 1: nt->GetNodeSize();
                int num = nObsVals*sizeof(Value);
                obsVals.resize( obsVals.size() + nObsVals );
                memcpy( &obsVals.back()-nObsVals+1, 
		    (*pEvidences)[slice - 1]->GetValue(nodeNumber), num);
		
            }
	    
        }
        else
        {
	    
            if( (*pEvidences)[slice]->IsNodeObserved(nodeNumber-nnodesPerSlice) )
            {
		
                obsNodes.push_back(nodeNumber);
                nt =  pMD->GetVariableType(nodeNumber-nnodesPerSlice);
                int nObsVals = nt->IsDiscrete() ? 1 : nt->GetNodeSize();
                int num = nObsVals*sizeof(Value);
                obsVals.resize( obsVals.size() + nObsVals );
                
		memcpy( &obsVals.back()-nObsVals+1, 
		    (*pEvidences)[slice]->GetValue(nodeNumber-nnodesPerSlice), 
		    num);
            }
	    
        }
	
    }
    
    if(obsNodes.size())
    {
        return CEvidence::Create(pGrModel->GetStaticModel()->GetModelDomain(), 
	    obsNodes.size(), &obsNodes.front(), obsVals );
    }
    else
    {
        return NULL;
    }
    
}



bool CEMLearningEngineDBN::IsDomainObserved(int nnodes, const int* domain,
                                            const pConstEvidenceVector* pEvidences,
                                            int nnodesPerSlice, int slice)
{
    PNL_CHECK_RANGES(slice, 0, (*pEvidences).size()-1);
    int i;
    for( i = 0; i < nnodes; i++ )
    {
	int nodeNumber = domain[i];
	
        int isObserved = nodeNumber < nnodesPerSlice ?
            (*pEvidences)[slice ? slice - 1 : 0]->IsNodeObserved(nodeNumber):
        (*pEvidences)[slice]->IsNodeObserved(nodeNumber - nnodesPerSlice);
	
        if(!isObserved)
        {
            return false;
        }
	
    }
    return true;
    
}


#ifdef PNL_RTTI
const CPNLType CEMLearningEngineDBN::m_TypeInfo = CPNLType("CEMLearningEngineDBN", &(CDynamicLearningEngine::m_TypeInfo));

#endif