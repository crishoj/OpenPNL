/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMlDynamicStructLearn.cpp                                 //
//                                                                         //
//  Purpose:   Implementation of the CMlDynamicStructLearn class           //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlMlDynamicStructLearn.hpp"


PNL_USING

const CDAG* CMlDynamicStructLearn::GetResultDag() const
{
    return m_pResultDag;
}

void CMlDynamicStructLearn::SetLearnPriorSlice(bool learnPriorSlice)
{
    m_LearnPriorSlice = learnPriorSlice;
}

void CMlDynamicStructLearn::SetMinProgress(float minProgress)
{
    m_minProgress = minProgress;
}

CMlDynamicStructLearn:: CMlDynamicStructLearn(CDBN* pGrModel,
					      ELearningTypes LearnType,
					      EOptimizeTypes AlgorithmType,
					      EScoreFunTypes ScoreType,
					      int nMaxFanIn,
					      int nRestarts,
					      int nMaxIters ) :
CDynamicLearningEngine(pGrModel, LearnType)
{
    m_ScoreType = ScoreType;
    m_AlgorithmType = AlgorithmType;
    m_LearnType = LearnType;
    m_nMaxFanIn = nMaxFanIn;
    m_nRestarts = nRestarts;
    m_nMaxIters = nMaxIters;
    m_LearnPriorSlice = false;
    m_minProgress = (float)1e-4;
    m_pGrModel = pGrModel;
    m_pResultDag = NULL;
    m_vEvidences.clear();
    m_vEvidence0.clear();
}

CMlDynamicStructLearn* CMlDynamicStructLearn::Create(CDBN* pGrModel,
						     ELearningTypes LearnType,
						     EOptimizeTypes AlgorithmType,
						     EScoreFunTypes ScoreType,
						     int nMaxFanIn,
						     int nRestarts,
						     int nMaxIters )
{
    CMlDynamicStructLearn* ret = new CMlDynamicStructLearn( pGrModel, LearnType,
	AlgorithmType, ScoreType, nMaxFanIn, nRestarts, nMaxIters );
    PNL_CHECK_IF_MEMORY_ALLOCATED(ret);
    return ret;
}

CMlDynamicStructLearn::~CMlDynamicStructLearn()
{
//	if(m_pGrModel) delete m_pGrModel;
    if(m_pResultDag) delete m_pResultDag;
    m_vEvidences.clear();
    m_vEvidence0.clear();
}

void CMlDynamicStructLearn::RearrangeEvidences()
{
    int i, j, k, m, n, ns;
    m_nSeries = m_VecPVecPEvidences.size();
    if(m_nSeries == 0)
	PNL_THROW(CInconsistentState, "need to set training data first");
    int nSeries = m_nSeries;
    int nNodesSlice = m_pGrModel->GetNumberOfNodes();
    CStaticGraphicalModel* pSGrModel = m_pGrModel->GetStaticModel();
    int nNodes = nNodesSlice + nNodesSlice;
    int* obsNodes = new int[nNodes];
    for(i=0; i<nNodes; i++) obsNodes[i] = i;
    boolVector vIsDiscrete(nNodes);
    intVector  vNodeSizes(nNodes);
    for(i=0; i<nNodes; i++)
    {
	if(pSGrModel->GetNodeType(i)->IsDiscrete())
	{
	    vIsDiscrete[i] = true;
	}
	else
	{
	    vIsDiscrete[i] = false;
	}
	vNodeSizes[i] = pSGrModel->GetNodeType(i)->GetNodeSize();
    }
    intVector vSeriesLen(nSeries);
    int nEvidences = 0;
    for(i=0; i<nSeries; i++)
    {
	vSeriesLen[i] = m_VecPVecPEvidences[i]->size();
	nEvidences += vSeriesLen[i] - 1;
    }
    m_vEvidences.assign( nEvidences, (CEvidence *)0 );
    valueVector vValues;
    const Value* value;

    n = 0;
    for(i=0; i<nSeries; i++)
    {
	for(j=0; j<vSeriesLen[i]-1; j++)
	{
	    for(k=0; k<nNodesSlice; k++)
	    {
		value = (*(m_VecPVecPEvidences[i]))[j] -> GetValue(k);
		if(vIsDiscrete[k])
		{
		    vValues.push_back(*value);
		}
		else
		{
		    ns = vNodeSizes[k];
		    for(m=0; m<ns; m++)
			vValues.push_back(*(value+m));
		}

	    }
	    for(k=0; k<nNodesSlice; k++)
	    {
		value = (*(m_VecPVecPEvidences[i]))[j+1] -> GetValue(k);
		if(vIsDiscrete[k])
		{
		    vValues.push_back(*value);
		}
		else
		{
		    ns = vNodeSizes[k];
		    for(m=0; m<ns; m++)
			vValues.push_back(*(value+m));
		}

	    }
	    m_vEvidences[n] = CEvidence::Create(pSGrModel, nNodes, obsNodes, vValues);
	    vValues.clear();
	    n++;
	}
    }
    vValues.clear();
    if(!m_LearnPriorSlice)
    {
	delete[] obsNodes;
	return;
    }
    m_vEvidence0.assign( nSeries, (CEvidence *)0 );
    CStaticGraphicalModel* pGrModel0 = m_pGrModel->CreatePriorSliceGrModel();
    n = 0;
    for(i=0; i<nSeries; i++)
    {
	for(k=0; k<nNodesSlice; k++)
	{
	    value = (*(m_VecPVecPEvidences[i]))[0] -> GetValue(k);
	    if(vIsDiscrete[k])
	    {
		vValues.push_back(*value);
	    }
	    else
	    {
		ns = vNodeSizes[k];
		for(m=0; m<ns; m++)
		    vValues.push_back(*(value+m));
	    }
	}
	m_vEvidence0[n] = CEvidence::Create(pGrModel0, nNodesSlice, obsNodes, vValues);
	vValues.clear();
	n++;
    }
    vValues.clear();
    delete[] obsNodes;
    delete pGrModel0;
}

void CMlDynamicStructLearn::Learn()
{
    RearrangeEvidences();
    int nNodesSlice = m_pGrModel->GetNumberOfNodes();
    intVector vAncestor, vDescent;
    for(int i=0; i<nNodesSlice; i++)
    {
	vAncestor.push_back(i);
	vDescent.push_back(i+nNodesSlice);
    }
    //currently only the hill climbing algorithm available
    CMlStaticStructLearn* pSSL = CMlStaticStructLearnHC::Create(m_pGrModel->GetStaticModel(), itStructLearnML,
	StructLearnHC, BIC, m_nMaxFanIn,
	vAncestor, vDescent, m_nRestarts);
    pSSL->SetData(m_vEvidences.size(), &m_vEvidences.front());
    pSSL->SetMaxIterIPF(m_nMaxIters);
    static_cast<CMlStaticStructLearnHC*>(pSSL) ->SetMinProgress(m_minProgress);
    pSSL->Learn();
    const CDAG* p2SDAG = pSSL->GetResultDAG();
    //	p2SDAG->Dump();
    if(!m_LearnPriorSlice)
    {
	m_pResultDag = const_cast<CDAG*>(p2SDAG)->Clone();
	delete pSSL;
	return;
    }
    intVector vA, vD;
    CStaticGraphicalModel* pGrModel0 = m_pGrModel->CreatePriorSliceGrModel();
    CMlStaticStructLearn* pSSL0 = CMlStaticStructLearnHC::Create(pGrModel0, itStructLearnML,
	StructLearnHC, BIC, m_nMaxFanIn,
	vA, vD, m_nRestarts);
    pSSL0->SetData(m_vEvidence0.size(), &m_vEvidence0.front());
    pSSL0->SetMaxIterIPF(m_nMaxIters / 2);
    static_cast<CMlStaticStructLearnHC*>(pSSL0) ->SetMinProgress(m_minProgress);

    pSSL0->Learn();
    const CDAG* p0SDAG = pSSL0->GetResultDAG();
    //	p0SDAG->Dump();
    CDAG* pDAG = const_cast<CDAG*>(p2SDAG)->Clone();
    if(pDAG->SetSubDag(vAncestor, const_cast<CDAG*>(p0SDAG)))
    {
	m_pResultDag = pDAG->Clone();
	delete pDAG;
    }
    else
	PNL_THROW(CInternalError, "InternalError, can not generate a DAG");

    delete pSSL;
    delete pSSL0;
}

#ifdef PNL_RTTI
const CPNLType CMlDynamicStructLearn::m_TypeInfo = CPNLType("CMlDynamicStructLearn", &(CDynamicLearningEngine::m_TypeInfo));

#endif