/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDynamicInferenceEngine.cpp                               //
//                                                                         //
//  Purpose:   Implementation of the base class for all inference          //
//             algorithms for dynamic graphical models                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlDynamicInferenceEngine.hpp"

PNL_USING

CDynamicInfEngine::CDynamicInfEngine( const CDynamicGraphicalModel *pGM ) : 
					
						m_CurrentTime(0),
						m_pGrModel(NULL),
                        m_pEvidenceMPE(NULL),
                        m_pQueryPot(NULL)
{
	PNL_CHECK_IS_NULL_POINTER(pGM);
	m_pGrModel = pGM;
}           

CDynamicInfEngine::~CDynamicInfEngine()
{
    delete m_pEvidenceMPE;
    m_pEvidenceMPE = NULL;
    
    delete m_pQueryPot;
    m_pQueryPot = NULL;
    
    if(m_CRingpEv.size())
    {
        CRing<CEvidence *>::iterator evidIter, evidIterBegin;
        evidIterBegin= m_CRingpEv.begin();
        evidIter = evidIterBegin;
        
        do
        {
            if( *evidIter )
            {
                delete ( *evidIter );
            }
            evidIter++;
        }while( evidIter != evidIterBegin );
    }
}

void CDynamicInfEngine::EnterEvidence(const pConstEvidenceVector& evidences)
{
    EnterEvidence( &evidences.front(), evidences.size() );
}

void CDynamicInfEngine::MarginalNodes(const intVector& query, int time, int notExpanded)
{
    MarginalNodes( &query.front(), query.size(), time, notExpanded );
}

#ifdef PNL_RTTI
const CPNLType CDynamicInfEngine::m_TypeInfo = CPNLType("CDynamicInfEngine", &(CPNLBase::m_TypeInfo));

#endif