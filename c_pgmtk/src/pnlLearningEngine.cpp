/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLearningEngine.cpp                                       //
//                                                                         //
//  Purpose:   Implementation of the base class for all learning           //
//             algorithms                                                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlLearningEngine.hpp"
#include "pnlEvidence.hpp"
#include "pnlDistribFun.hpp"
#include "pnlString.hpp"

PNL_USING

CLearningEngine::CLearningEngine( CGraphicalModel *pGrModel, ELearningTypes learnType)
{
    
    CheckModelValidity(pGrModel);
    m_pGrModel = pGrModel;
    m_LearnType = learnType;
}

CLearningEngine::~CLearningEngine()
{
}

bool CLearningEngine::IsInfNeed(const CEvidence * pEvidences)
{
    int nnodesPerSlice = m_pGrModel->GetNumberOfNodes();
    if( pEvidences->GetNumberObsNodes() != nnodesPerSlice)
    {
        return true;
    }
    else
    {
        const int* flags = pEvidences->GetObsNodesFlags();
        if(std::find(flags, flags + nnodesPerSlice, 0) != flags + nnodesPerSlice)
        {
            return true;
        }
        
    }
    
    return false;
}


void CLearningEngine::CheckModelValidity(const CGraphicalModel *pGrModel)
{
    if(!pGrModel)
    {
	PNL_THROW(CNULLPointer, "no graphical model")
    }

    int i;
    int numFactors = pGrModel->GetNumberOfFactors();
    
    for( i = 0; i < numFactors; ++i )
    {
        const CFactor* pFact = pGrModel->GetFactor(i);
	pnlString str;
        
        if( !pFact )
        {
	    str << "learning on invalid model: factor #" << i << " is absent";
            PNL_THROW( CInvalidOperation, str.c_str());
        }
        else
        {
	    std::string mesg;
            if( !pFact->IsValid(&mesg) )
            {
		str << "learning on invalid model: factor #" << i << ": " << mesg;
                PNL_THROW( CInvalidOperation, str.c_str());
            }
        }
    }
}


void CLearningEngine::GetCriterionValue(int *nsteps, const float **score) const
{
    (*nsteps) = m_critValue.size();
    (*score) = &m_critValue.front();
}

void CLearningEngine::GetCriterionValue( floatVector *critVls ) const
{
    PNL_CHECK_IS_NULL_POINTER(critVls);
    (*critVls).assign(m_critValue.begin(), m_critValue.end());
}

void CLearningEngine::ClearStatisticData()
{
    
    
    if(m_pGrModel)
    {
	
	int numberOfFactors = m_pGrModel -> GetNumberOfFactors();
	for( int paramNumber = 0; paramNumber < numberOfFactors; paramNumber++ )
	{
	    m_pGrModel->GetFactor(paramNumber)->GetDistribFun()->ClearStatisticalData();
	}
    }
    else
    {
	PNL_THROW( CNULLPointer, "no graphical model")
    }
}

#ifdef PNL_RTTI
const CPNLType CLearningEngine::m_TypeInfo = CPNLType("CLearningEngine", &(CPNLBase::m_TypeInfo));

#endif