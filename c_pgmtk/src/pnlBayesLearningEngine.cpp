/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlBayesLearningEngine.cpp                                  //
//                                                                         //
//  Purpose:   Implementation of the factor learning algorithm for      //
//             BNet that uses prior distribution of factors             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"    
#include "pnlTabularDistribFun.hpp"
/*#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "math.h"
#include "pnlEmLearningEngine.hpp"
#include "pnlLearningEngine.hpp"
#include "pnlNaiveInferenceEngine.hpp"
#include <math.h>
#include "pnlNumericDenseMatrix.hpp"
*/
#include "pnlBayesLearningEngine.hpp"

PNL_USING

CBayesLearningEngine::CBayesLearningEngine(CStaticGraphicalModel *pGrModel):
                         CStaticLearningEngine( pGrModel, itParamLearnBayes )
{
    m_numberOfLearnedEvidences = 0;
}

CBayesLearningEngine::~CBayesLearningEngine()
{  
}

CBayesLearningEngine* 
CBayesLearningEngine::Create(CStaticGraphicalModel *pGrModelIn)
{
    if(!pGrModelIn)
	{
		PNL_THROW(CNULLPointer, "Graphical Model");
	}

    //analyze type of graphical model
    if( pGrModelIn->GetModelType() != mtBNet)
    {
        PNL_THROW(CNotImplemented, 
            "currently CBayesLearningEngine supports only BNets");
    }
    
    CBayesLearningEngine *pNewEngine = new CBayesLearningEngine(pGrModelIn);
	return pNewEngine; //should we throw out of memory exception 
                       //if pNewEngine==0 ?
}

void CBayesLearningEngine::SetData( int size, 
                                    const CEvidence* const* pEvidencesIn )
{
    //clear previously set data. Clear learning data
    m_Vector_pEvidences.clear();
    AppendData(size, pEvidencesIn);
	ClearStatisticData();
	m_numberOfAllEvidences = m_Vector_pEvidences.size();
    m_numberOfLearnedEvidences = 0;
}

void CBayesLearningEngine::AppendData( int dim, 
                                       const CEvidence* const* pEvidencesIn )
{
  	//create vector consists of points to evidences
    PNL_CHECK_LEFT_BORDER(dim, 1);
    int i = 0;
    for( i = 0; i < dim; i++ )
    {
        if (!pEvidencesIn[i])
        {
            PNL_THROW(CNULLPointer,"evidence")
        }
        if( IsInfNeed(pEvidencesIn[i]) )
        {
            PNL_THROW(CNotImplemented,"all nodes should be observed")
        }
        m_Vector_pEvidences.push_back( pEvidencesIn[i] );
    }
    m_numberOfAllEvidences = m_Vector_pEvidences.size();
}

void CBayesLearningEngine::Learn()
{
    if(!m_pGrModel)
    {
	PNL_THROW( CNULLPointer, "no graphical model")
    }
    CStaticGraphicalModel *grmodel = this->GetStaticModel();
    CFactor *factor = NULL;
    int numberOfFactors = grmodel->GetNumberOfFactors();
    int domainNodes;
    
    if(m_numberOfLearnedEvidences == m_numberOfAllEvidences)
    {
	PNL_THROW(COutOfRange, "number of unlearned evidences must be positive")
    }
    int currentEvidNumber;
    const CEvidence* pCurrentEvid;
    
    //below code is intended to work on tabular CPD
    //later we will generalize it for other distribution types
    for( int ev = m_numberOfLearnedEvidences; ev < m_numberOfAllEvidences; ev++)
    {
        currentEvidNumber = ev;
        pCurrentEvid = m_Vector_pEvidences[currentEvidNumber];
        
        if( !pCurrentEvid)
        {
            PNL_THROW(CNULLPointer, "evidence")
        }
        
        for( domainNodes = 0; domainNodes < numberOfFactors; domainNodes++ )
        {
            factor = grmodel->GetFactor( domainNodes );
            int DomainSize;
	    const int *domain;
	    factor->GetDomain( &DomainSize, &domain );
            const CEvidence *pEvidences[] = { pCurrentEvid };
            CTabularDistribFun* pDistribFun = (CTabularDistribFun*)(factor->GetDistribFun());
            pDistribFun->BayesUpdateFactor(pEvidences, 1, domain);
        }
    }       		
    switch (grmodel->GetModelType())
    {
    case mtBNet:
        {
            for( domainNodes = 0; domainNodes < numberOfFactors; domainNodes++ )
            {
                factor = grmodel -> GetFactor( domainNodes );
                CTabularDistribFun* pDistribFun = (CTabularDistribFun*)(factor->GetDistribFun());
                pDistribFun->PriorToCPD();
            }
            break;
        }
    default:
        {
            PNL_THROW(CBadConst, "model type" )
                break;
        }
    }                                                  
    //update number of learned evidences
    m_numberOfLearnedEvidences = m_numberOfAllEvidences;                    
}

#ifdef PNL_RTTI
const CPNLType CBayesLearningEngine::m_TypeInfo = CPNLType("CBayesLearningEngine", &(CStaticLearningEngine::m_TypeInfo));

#endif