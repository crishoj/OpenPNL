/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMlStaticStructLearn.cpp                                  //
//                                                                         //
//  Purpose:   CMlStaticStructLearn class member functions implementation  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlMlStaticStructLearn.hpp"

PNL_USING
const CBNet* CMlStaticStructLearn::GetResultBNet() const
{
    return m_pResultBNet;
}

const CDAG*  CMlStaticStructLearn::GetResultDAG() const
{
    return m_pResultDAG;
}

const int* CMlStaticStructLearn::GetResultRenaming()const
{
    return &m_vResultRenaming.front();
}

void CMlStaticStructLearn::SetMinProgress(float minProgress)
{
	m_minProgress = minProgress;
}

CMlStaticStructLearn::CMlStaticStructLearn(CStaticGraphicalModel *pGrModel,
					   ELearningTypes LearnType, 
					   EOptimizeTypes AlgorithmType,
					   EScoreFunTypes ScoreType, int nMaxFanIn,
					   intVector& vAncestor, intVector& vDescent)
					   :CStaticLearningEngine(pGrModel, LearnType),
					   m_pResultBNet(NULL), m_pResultDAG(NULL)
{ 
    int nnodes = pGrModel->GetNumberOfNodes();
    m_nNodes = nnodes;
    int i;
    for (i = 0; i<nnodes; i++) m_vResultRenaming.push_back(i);

    m_ScoreType = ScoreType;
    m_Algorithm = AlgorithmType;
    m_nMaxFanIn = nMaxFanIn;
    m_vAncestor.assign(vAncestor.begin(), vAncestor.end());
    m_vDescent.assign(vDescent.begin(),vDescent.end());
    PNL_CHECK_RANGES(nMaxFanIn, 1, nnodes);
    intVector ranges(nMaxFanIn);
    for(i=0; i<nMaxFanIn; i++) ranges[i] = nnodes+1;
    float max = 0.0f;
	m_pNodeScoreCache =	(CSparseMatrix<float>**)malloc(nnodes * sizeof(CSparseMatrix<float>*));
	for(i=0; i<nnodes; i++)
	{
		m_pNodeScoreCache[i] = CSparseMatrix<float>::Create(nMaxFanIn, 
		                &ranges.front(), max, 0);
	}
}

CMlStaticStructLearn::~CMlStaticStructLearn()
{
	int i;
	for(i=0; i<m_nNodes; i++)
		delete m_pNodeScoreCache[i];
	delete m_pNodeScoreCache;
    if(m_pResultBNet) delete m_pResultBNet;
    if(m_pResultDAG) delete m_pResultDAG;
}

void CMlStaticStructLearn::SetInitGraphicalModel(CGraphicalModel* pGrModel)
{
    if(m_pGrModel)
        delete m_pGrModel;
    m_pGrModel = pGrModel;
}

float CMlStaticStructLearn::ScoreDAG(CDAG* pDAG, floatVector* familyScore)
{
    familyScore->clear();
    intVector vFamily;
    float fscore, score = 0.0f;
    for(int i=0; i<m_nNodes; i++)
    {
        vFamily.clear();
        pDAG->GetParents(i, &vFamily);
        vFamily.push_back(i);
        fscore = ScoreFamily(vFamily);
        familyScore->push_back(fscore);
        score += fscore;
    }
    return score;
}

float CMlStaticStructLearn::ScoreFamily(intVector vFamily)
{
    int nParents = vFamily.size() - 1;
    PNL_CHECK_RANGES(nParents, 0, m_nMaxFanIn);
    intVector indexes(m_nMaxFanIn,0);
    int i;
    for(i=0; i<nParents; i++)
    {
        indexes[i] = vFamily[i]+1;
    }
	int node = vFamily[nParents];
    float score;
	float defval = m_pNodeScoreCache[node]->GetDefaultValue();
    score = m_pNodeScoreCache[node]->GetElementByIndexes(&indexes.front());
	if(score == defval)
    {
        score = ComputeFamilyScore(vFamily);
        m_pNodeScoreCache[node]->SetElementByIndexes(score, &indexes.front());
    }
    return score;
}


float CMlStaticStructLearn::ComputeFamilyScore(intVector vFamily)
{
    int nFamily = vFamily.size();
	CCPD* iCPD = this->CreateRandomCPD(nFamily, &vFamily.front(), m_pGrModel);
    int ncases = m_Vector_pEvidences.size();
    iCPD->UpdateStatisticsML( &m_Vector_pEvidences.front(), ncases );
    float score = iCPD->ProcessingStatisticalData(ncases);
    int dim = iCPD->GetNumberOfFreeParameters();
    switch (m_ScoreType)
    {
    case BIC :
		score -= 0.5f * float(dim) * float(log(float(ncases)));
		break;
    case AIC :
		score -= 0.5f * float(dim);
		break;
    case VAR :
		PNL_THROW(CNotImplemented, 
			"This type score function has not been implemented yet");
		break;
    default:
		PNL_THROW(CNotImplemented, 
			"This type score function has not been implemented yet");
		break;
    }
    delete iCPD;
    return score;
}

void  CMlStaticStructLearn::CreateResultBNet(CDAG* pDAG)
{
    int i, j, k, ns;
    int nnodes = m_nNodes;
    CDAG* iDAG = pDAG->TopologicalCreateDAG(m_vResultRenaming);
    nodeTypeVector vnt;
    m_pGrModel->GetNodeTypes(&vnt);
    intVector na(nnodes);
    const int* nas = m_pGrModel->GetNodeAssociations();
    for(i=0; i<nnodes; i++) na[i] = nas[m_vResultRenaming[i]];
    m_pResultBNet = CBNet::Create(nnodes, vnt.size(), &vnt.front(), 
		                    &na.front(), static_cast<CGraph*>(iDAG));
    const CNodeType* nt;

    int nEv = m_Vector_pEvidences.size();
    CEvidence** pEv = new CEvidence*[nEv];
    intVector obsnodes(nnodes);
    for(i=0; i<nnodes; i++) obsnodes[i] = i;
    valueVector new_data;
    const Value* val;
    for(i = 0 ; i < nEv; i++)
    {
	for(j=0; j<nnodes; j++)
	{
	    val = m_Vector_pEvidences[i]->GetValue(m_vResultRenaming[j]);
		nt = m_pResultBNet->GetNodeType(j);
	    if(nt->IsDiscrete())
	    {
		    new_data.push_back(*val);
	    }
	    else
	    {
		    ns = nt->GetNodeSize();
		    for(k=0; k<ns; k++)
		        new_data.push_back(*(val+k));
	    }
	}			
	pEv[i] = CEvidence::Create(m_pResultBNet, nnodes, &obsnodes.front(), new_data);
	new_data.clear();
    }
    vnt.clear();
    
    intVector vFamily;
    m_pResultBNet->AllocFactors();
    for(i=0; i<nnodes; i++)
    {
	    vFamily.clear();
	    iDAG->GetParents(i, &vFamily);
	    vFamily.push_back(i);
	    CCPD* iCPD = ComputeFactor(vFamily, m_pResultBNet, pEv);
	    m_pResultBNet->AttachFactor(iCPD);
    }
    
    for(i=0; i<nEv; i++)delete pEv[i];
    delete[] pEv;
}

CCPD*
CMlStaticStructLearn::ComputeFactor(intVector vFamily, CGraphicalModel* pGrModel, CEvidence** pEvidences)
{
    int nFamily = vFamily.size();
	CCPD* iCPD = this->CreateRandomCPD(nFamily, 
		             &vFamily.front(), pGrModel);
    int ncases = m_Vector_pEvidences.size();
    iCPD->UpdateStatisticsML( pEvidences, ncases );
    iCPD->ProcessingStatisticalData(ncases);
    return iCPD;
}

CCPD* 
CMlStaticStructLearn::CreateRandomCPD(int nfamily, const int* family, CGraphicalModel* pGrModel)
{
	CModelDomain* pMD = pGrModel->GetModelDomain();
	EDistributionType dt = pnlDetermineDistributionType( pMD, nfamily, family, NULL);
                 
    CCPD* pCPD;

	switch (dt)
    {
    case dtTabular :
		pCPD = CTabularCPD::Create(family, nfamily, pMD);
		pCPD->CreateAllNecessaryMatrices(1);
		break;
    
	case dtTree :
		pCPD = CTreeCPD::Create(family, nfamily, pMD);
		break;

    case dtGaussian :
	case dtCondGaussian :
		pCPD = CGaussianCPD::Create(family, nfamily, pMD);
		pCPD->CreateAllNecessaryMatrices(1);
		break;
    default:
		PNL_THROW(CNotImplemented, "This type of distribution has not been implemented yet");
		break;
    }
	return pCPD;
}
