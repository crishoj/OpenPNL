/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDynamicLearningEngine.cpp                                //
//                                                                         //
//  Purpose:   Implementation of the base class for all learning           //
//             algorithms for dynamic graphical models                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlDynamicLearningEngine.hpp"
#include "pnlDistribFun.hpp"

PNL_USING

CDynamicLearningEngine::CDynamicLearningEngine(CDynamicGraphicalModel *pGrModel,
                                               ELearningTypes learnType):
CLearningEngine( pGrModel, learnType ),
m_numberOfAllEvISlice(0),
m_numberOfAllEv0Slice(0),
m_FlagsIsAllObserved(0),
m_VecPVecPEvidences(0)

{
			
}

CDynamicLearningEngine::~CDynamicLearningEngine()
{
		
}

void CDynamicLearningEngine::
SetData( int NumberOfTimeSeries, int *NumberOfSlices,
        const CEvidence* const* const* pEvidences )
{
    m_numberOfAllEvISlice = 0;
    m_numberOfAllEv0Slice = 0;
    m_FlagsIsAllObserved.assign(NumberOfTimeSeries, 1);
    m_VecPVecPEvidences.resize(NumberOfTimeSeries);
    const CEvidence *pEv;
    
    for(int i = 0; i < NumberOfTimeSeries; i++)
    {
	
	PNL_CHECK_IS_NULL_POINTER( pEvidences[i]);
	pConstEvidenceVector *pVecPEv = new pConstEvidenceVector(NumberOfSlices[i]);
	PNL_CHECK_IF_MEMORY_ALLOCATED(pVecPEv);
	
	for(int j = 0; j < NumberOfSlices[i]; j++)
	{
	    pEv = (pEvidences[i])[j];
	    PNL_CHECK_IS_NULL_POINTER(pEv);
	    if(IsInfNeed(pEv))
	    {
		m_FlagsIsAllObserved[i] = 0;
	    }
	    (*pVecPEv)[j] = pEv;
	    
	}
	m_VecPVecPEvidences[i] = pVecPEv;
	m_numberOfAllEvISlice += (pVecPEv->size()-1);
    }
    
    
    m_numberOfAllEv0Slice += NumberOfTimeSeries;
    
}

void CDynamicLearningEngine::
SetData(const pEvidencesVecVector& evidences )
{
    int numOfTimeSeries = evidences.size();
    
    intVector numberOfSlices(numOfTimeSeries);
    
    int nSlices; 
    int series;
    const CEvidence ***evFinal;
    evFinal = new const CEvidence ** [numOfTimeSeries];
    
    for( series = 0; series < numOfTimeSeries; series++ ) 
    {
        nSlices = evidences[series].size();
        numberOfSlices[series] = nSlices;
        evFinal[series] = new const CEvidence * [nSlices];
        
        int slice;
        
        for( slice = 0; slice < numberOfSlices[series]; slice++ )
        {
            evFinal[series][slice] = evidences[series][slice];
        }
    }
   
    SetData( numOfTimeSeries, &numberOfSlices.front(), evFinal );
    
    for( series = 0; series < numOfTimeSeries; series++ ) 
    {
            delete []evFinal[series];
    }
    delete []evFinal;
}

#ifdef PNL_RTTI
const CPNLType CDynamicLearningEngine::m_TypeInfo = CPNLType("CDynamicLearningEngine", &(CLearningEngine::m_TypeInfo));

#endif