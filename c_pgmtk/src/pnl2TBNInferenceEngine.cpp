/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnl2TBNInferenceEngine.cpp                                  //
//                                                                         //
//  Purpose:   Implementation of the inference algorithms that use         //
//             forward-backward operations                                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnl2TBNInferenceEngine.hpp"

PNL_USING

C2TBNInfEngine::
C2TBNInfEngine( const CDynamicGraphicalModel *pGraphicalModel):
CDynamicInfEngine(  pGraphicalModel )

{
	
}


C2TBNInfEngine::~C2TBNInfEngine()
{
}

void C2TBNInfEngine::Filtering(int time)
{
	
	PNL_CHECK_FOR_NON_ZERO(time-m_CurrentTime);
	PNL_CHECK_IS_NULL_POINTER(m_CRingpEv[0]);
	if(time == 0)
	{
		ForwardFirst(m_CRingpEv[0]);
	}
	else
	{
		Forward(m_CRingpEv[0]);
	}
	BackwardT();
}

void C2TBNInfEngine::Prediction(int time)
{
	if (time > 0)
	{
		const intVector n(0);
		const valueVector v(0);
		int i;
		int startTime = m_CurrentTime;
		for(i = startTime + 1; i <= startTime + time; i++)
		{
			CEvidence *clEvid = CEvidence::Create(GrModel()->GetModelDomain(),n,v);
			if(time == 0)
			{
				ForwardFirst(clEvid);
			}
			else
			{
				Forward(clEvid);
			}
			BackwardT();
			delete clEvid;
		}
	}
	else
	{
		PNL_THROW
            (COutOfRange, 
            "prediction time must be more than 0")	
	}
}

void C2TBNInfEngine::Smoothing()
{
	CRing<CEvidence *>::iterator evIter = m_CRingpEv.begin();
	PNL_CHECK_IS_NULL_POINTER(*evIter);
	ForwardFirst(*evIter);
	int time = 0;
	for(int i = 1 ; i < m_Lag; i++ )
	{
		evIter++;
		time++;
		PNL_CHECK_IS_NULL_POINTER(*evIter);
		Forward(*evIter);
	}
	BackwardT();
	while(time--)
	{
		Backward();
	}
}

void C2TBNInfEngine::FindMPE()
{
    int maximize = 1;
    CRing<CEvidence *>::iterator evIter = m_CRingpEv.begin();
    PNL_CHECK_IS_NULL_POINTER(*evIter);
    ForwardFirst(*evIter, maximize);
    int time = 0;
    for(int i = 1 ; i < m_Lag; i++ )
    {
        evIter++;
        time++;
        PNL_CHECK_IS_NULL_POINTER(*evIter);
        Forward(*evIter, maximize);
    }
    BackwardT();
    while(time--)
    {
        Backward( maximize );
    }
}

void C2TBNInfEngine::FixLagSmoothing(int time)
{
    PNL_CHECK_LEFT_BORDER(time, GetLag());
    if( m_CRingpEv.size() != GetLag() + 1)
    {
        PNL_THROW
            (COutOfRange, 
            "number of received evidences must be equal to lag-1")
    }
    
    
    if(!GetTime())
        
    {
        ForwardFirst(m_CRingpEv[GetTime()]);
        for( int i = 1 ; i < GetLag() + 1; i++  )
        {
            Forward(m_CRingpEv[GetTime()]);
            
        }
        BackwardFixLag();
        
    }
    else
    {      
        Forward(m_CRingpEv[GetTime()]);
        BackwardFixLag();
    }
    
}

#ifdef PNL_RTTI
const CPNLType C2TBNInfEngine::m_TypeInfo = CPNLType("C2TBNInfEngine", &(CDynamicInfEngine::m_TypeInfo));

#endif
