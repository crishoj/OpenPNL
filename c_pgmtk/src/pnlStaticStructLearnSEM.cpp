/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlStaticStructLearnSEM.cpp                                 //
//                                                                         //
//  Purpose:   Implementation of the CStaticStructLearnSEM class           //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlCondGaussianDistribFun.hpp"
#include "pnlEmLearningEngine.hpp"
#include "pnlStaticStructLearnSEM.hpp"
#include "pnlExInferenceEngine.hpp"


PNL_USING


CStaticStructLearnSEM::CStaticStructLearnSEM( CBNet* pBNet, 
						  ELearningTypes LearnType, 
						  int nMaxFanIn,
						  intVector* vAncestor,
						  intVector* vDescent ):
						  CStaticLearningEngine( pBNet, LearnType )
{
	PNL_CHECK_IS_NULL_POINTER(pBNet);
	int i;
	m_nMaxFanIn = nMaxFanIn;
	m_nNodes = pBNet->GetNumberOfNodes();
	if(vAncestor)
		m_vAncestor.assign(vAncestor->begin(), vAncestor->end());
	if(vDescent)
		m_vDescent.assign(vDescent->begin(), vDescent->end());
	m_pCurrBNet = pBNet->Copy(pBNet);
	m_vGlobalRenaming.assign(m_nNodes, 0);
	for(i=0; i<m_nNodes; i++)
	{
		m_vGlobalRenaming[i] = i;
	}
	m_IterEM = 8;
	m_maxLoop = 50;
	m_minProgress = (float)1e-3;
}

CStaticStructLearnSEM* CStaticStructLearnSEM::Create( CBNet* pBNet, 
						  ELearningTypes LearnType, 
						  int nMaxFanIn,
						  intVector* vAncestor,
						  intVector* vDescent )
{
	CStaticStructLearnSEM* ret = new CStaticStructLearnSEM( pBNet, LearnType, 
												   nMaxFanIn, vAncestor,
												   vDescent );
	return ret;
}

void CStaticStructLearnSEM::SetIterOfEM(int iter)
{
	m_IterEM = iter;
}

void CStaticStructLearnSEM::SetMinProgress(float minProgress)
{
	m_minProgress = minProgress;
}

void CStaticStructLearnSEM::SetMaxLoop(int maxLoop)
{
	m_maxLoop = maxLoop;
}

CStaticStructLearnSEM::~CStaticStructLearnSEM()
{
    if(m_pCurrBNet)
		delete m_pCurrBNet;
}

void CStaticStructLearnSEM::ConvertToCurrEvidences(CBNet* pBNet)
{
	if(!pBNet)
		PNL_THROW(CInvalidOperation, "need to create a new BNet first");
	int i,oldNode,newNode,m;
	intVector ObsNodes;
	if( m_vCurrEvidences.size() != 0 )
	{
		for(i=0; i<m_numberOfAllEvidences; i++)
			delete m_vCurrEvidences[i];
		m_vCurrEvidences.clear();
	}
	CModelDomain* pMD = pBNet->GetModelDomain();
	CModelDomain* pGrMD = this->GetStaticModel()->GetModelDomain();
	const CNodeType* nt;
	const Value* value;
	valueVector vValues;
	for(i=0; i<m_numberOfAllEvidences; i++)
	{
		const CEvidence* pEvidence = m_Vector_pEvidences[i];
		for(newNode=0; newNode<m_nNodes; newNode++)
		{
			oldNode = m_vGlobalRenaming[newNode];
			if( pEvidence->IsNodeObserved(oldNode) )
			{
				ObsNodes.push_back(newNode);
				value = pEvidence->GetValue(oldNode);
				nt = pGrMD->GetVariableType(oldNode);
				if(nt->IsDiscrete())
				{
					vValues.push_back(*value);
				}
				else
				{
					for(m=0; m<nt->GetNodeSize(); m++)
						vValues.push_back(*(value+m));
				}
			}
		}
		CEvidence* pEv = CEvidence::Create(pMD, ObsNodes, vValues);
		m_vCurrEvidences.push_back(pEv);
		vValues.clear();
		ObsNodes.clear();
	}
}

void CStaticStructLearnSEM::Learn()
{
	int loop = 0;
	bool progress = true;
	while( (loop<m_maxLoop) && progress )
	{
		progress = LearnOneStep();
		loop++;
	}
}

bool CStaticStructLearnSEM::LearnOneStep()
{
	intVecVector decompsition;
	CGraph* graph = m_pCurrBNet->GetGraph();
	graph->GetConnectivityComponents( &decompsition );
	CEMLearningEngine* pEMLearn;
	if(decompsition.size() > 1)
	{
		CExInfEngine< CJtreeInfEngine, CBNet, PNL_EXINFENGINEFLAVOUR_DISCONNECTED > *pInf = 
                	CExInfEngine< CJtreeInfEngine, CBNet, PNL_EXINFENGINEFLAVOUR_DISCONNECTED >::
       		Create( m_pCurrBNet  );
		pEMLearn = CEMLearningEngine::Create(m_pCurrBNet, pInf);
	}
	else
    {
        CJtreeInfEngine *pInf = CJtreeInfEngine::Create(m_pCurrBNet);
        pEMLearn = CEMLearningEngine::Create(m_pCurrBNet, pInf);
    }

	int i;
	for(i=0; i<decompsition.size(); i++)
		decompsition[i].clear();
	decompsition.clear();

	ConvertToCurrEvidences(m_pCurrBNet);
	pEMLearn->SetData(m_numberOfAllEvidences, &m_vCurrEvidences.front());
	
	pEMLearn->SetMaxIterEM(m_IterEM);

//	pEMLearn->ClearStatisticData();
	pCPDVector vNeighborCPDs;
	floatVector vNeighborLLs;
	EDGEOPVECTOR vValidMoves;
	intVector vRevCorrespDel;
	CreateNeighborCPDs(m_pCurrBNet, &vNeighborCPDs, &vValidMoves, &vRevCorrespDel);

	pEMLearn->LearnExtraCPDs(m_nMaxFanIn+1, &vNeighborCPDs, &vNeighborLLs);

//	m_pCurrBNet = static_cast<CBNet*>(pEMLearn->GetStaticModel());
	const float* familyLL = pEMLearn->GetFamilyLogLik();
	floatVector familyScores(m_nNodes,0);
	int j, freeparams;
	float logebase = (float)log(float(m_numberOfAllEvidences));
	float total_score = 0.0f;
	CFactor* pCPD;
	for(i=0; i<m_nNodes; i++)
	{
		pCPD = m_pCurrBNet->GetFactor(i);
		freeparams = pCPD->GetNumberOfFreeParameters();
		familyScores[i] = familyLL[i] - 0.5f * float(freeparams) * logebase;
		total_score += familyScores[i];
	}
	int nMoves = vValidMoves.size();
	floatVector neighborScores(nMoves, 0);
	for(i=0; i<nMoves; i++)
	{
		pCPD = static_cast<CFactor*>(vNeighborCPDs[i]);
		freeparams = pCPD->GetNumberOfFreeParameters();
		neighborScores[i] = vNeighborLLs[i] - 0.5f * float(freeparams) * logebase;
	}

	int start, end, max_position=0;
	float tmp_score, best_score = -1e37f; 
	EDGEOP move;
	for(i=0; i<nMoves; i++)
	{
		move = vValidMoves[i];
		switch (move.DAGChangeType)
		{
		case DAG_DEL : 
			end = move.originalEdge.endNode;
			tmp_score = neighborScores[i] - familyScores[end];
			if( best_score<tmp_score )
			{
				best_score = tmp_score;
				max_position = i;
			}
			break;

		case DAG_ADD :
			end = move.originalEdge.endNode;
			tmp_score = neighborScores[i] - familyScores[end];
			if( best_score<tmp_score )
			{
				best_score = tmp_score;
				max_position = i;
			}
			break;

		case DAG_REV :
			end = move.originalEdge.startNode;
			tmp_score = neighborScores[i] - familyScores[end];
			
			end = move.originalEdge.endNode;
			tmp_score += neighborScores[vRevCorrespDel[i]] - familyScores[end];
			if( best_score<tmp_score )
			{
				best_score = tmp_score;
				max_position = i;
			}
			break;
		}
	}

	move = vValidMoves[max_position];
	start = move.originalEdge.startNode;
	end = move.originalEdge.endNode;
	EDAGChangeType changeType = move.DAGChangeType;
	CCPD *addCPD=0, *delCPD=0;
	switch (changeType)
	{
	case DAG_DEL : 
		delCPD = static_cast<CCPD*>((vNeighborCPDs[max_position])->Clone());
		break;

	case DAG_ADD :
		addCPD = static_cast<CCPD*>((vNeighborCPDs[max_position])->Clone());
		break;

	case DAG_REV :
		addCPD = static_cast<CCPD*>((vNeighborCPDs[max_position])->Clone());
		delCPD = static_cast<CCPD*>((vNeighborCPDs[vRevCorrespDel[max_position]])->Clone());			
		break;
	}

	delete pEMLearn;
	for(i=0; i<vNeighborCPDs.size(); i++)
	{
		delete vNeighborCPDs[i];
	}
	vNeighborCPDs.clear();
	for(i=0; i<m_numberOfAllEvidences; i++)
	{
		delete m_vCurrEvidences[i];
	}
	m_vCurrEvidences.clear();
	vValidMoves.clear();
	float score_gate = (float)fabs(m_minProgress * total_score);
	if(best_score <= score_gate)
	{
		if(changeType == DAG_REV)
		{
			delete addCPD;
			delete delCPD;
		}
		if(changeType == DAG_ADD)delete addCPD;
		if(changeType == DAG_DEL)delete delCPD;
		return false;
	}

	total_score += best_score;
	CDAG* pDAG = CDAG::Create(*(m_pCurrBNet->GetGraph()));
	int node, node1, newnode;
	if(!(pDAG->DoMove(start, end, changeType)))
	{
		PNL_THROW(CInternalError, "There are some internal errors");
	}

	intVector vRenaming, Old2New;
	CDAG* iDAG;
	int TopologicSorted = pDAG->IsTopologicallySorted();
	if( TopologicSorted )
	{
		iDAG = pDAG->Clone();
		for(i=0; i<m_nNodes; i++) vRenaming.push_back(i);
	}
	else
		iDAG = pDAG->TopologicalCreateDAG(vRenaming);
	pDAG->Dump();
	intVector gRename;
	for(i=0; i<m_nNodes; i++)
	{
		node = vRenaming[i];
		node1 = m_vGlobalRenaming[node];
		gRename.push_back(node1);
	}
	m_vGlobalRenaming.assign(gRename.begin(), gRename.end());

	int pos;
	for(i=0; i<m_nNodes; i++)
	{
		pos = std::find(vRenaming.begin(), vRenaming.end(), i) - vRenaming.begin();
		Old2New.push_back(pos);
	}

	const int* oldNodeAsso = m_pCurrBNet->GetNodeAssociations();
	intVector newNodeAsso(m_nNodes,0);
	for(i=0; i<m_nNodes; i++)
	{
		newNodeAsso[i] = oldNodeAsso[vRenaming[i]];
	}
	nodeTypeVector vpnt;
	m_pCurrBNet->GetNodeTypes(&vpnt);
	CBNet* pBNet = CBNet::Create(m_nNodes, vpnt.size(), &vpnt.front(), 
		           &newNodeAsso.front(), static_cast<CGraph*>(iDAG));
	CModelDomain* pMDnew = pBNet->GetModelDomain();
	pBNet->AllocFactors();
	intVector domainNew, domainOld;
	const CFactor* factor=0;
	CFactor* curFactor;
	for(i=0; i<m_nNodes; i++)
	{
		domainNew.clear();
		newnode = Old2New[i];
		if( (i != start) && (i != end) )
		{
			factor = m_pCurrBNet->GetFactor(i);
		}
		else
		{
			if(changeType == DAG_REV)
			{
				if(i == start)
					factor = addCPD->Clone();
				if(i == end)
					factor = delCPD->Clone();
			}
			if(changeType == DAG_DEL)
			{
				if(i == start)
					factor = m_pCurrBNet->GetFactor(i);
				if(i == end)
					factor = delCPD->Clone();
			}
			if(changeType == DAG_ADD)
			{
				if(i == start)
					factor = m_pCurrBNet->GetFactor(i);
				if(i == end)
					factor = addCPD->Clone();
			}
		}
		factor->GetDomain(&domainOld);
		for(j=0; j<domainOld.size(); j++)
		{
			domainNew.push_back(Old2New[domainOld[j]]);
		}
		curFactor = CFactor::CopyWithNewDomain(factor, domainNew, pMDnew);
		pBNet->AttachFactor(curFactor);
	}

	if(changeType == DAG_REV)
	{
		delete addCPD;
		delete delCPD;
	}
	if(changeType == DAG_ADD)delete addCPD;
	if(changeType == DAG_DEL)delete delCPD;

	delete m_pCurrBNet;
	delete pDAG;
	m_pCurrBNet = pBNet;
	m_critValue.push_back(total_score);
	return true;
}

void CStaticStructLearnSEM::CreateNeighborCPDs(CBNet* pBNet, 
	pCPDVector* vNeighborCPDs, EDGEOPVECTOR* vValidMoves, intVector* RevCorrespDel)
{
	CGraph* pGraph = pBNet->GetGraph();
	CDAG* pDAG = CDAG::Create(*pGraph);
	CModelDomain* pMD = pBNet->GetModelDomain();
	intVector vDiscrete, vContinuous;
	intVector vAncestor, vDescent;
	intVector vMixture, vMix;
	const CNodeType* nt;
	CFactor* factor;

	int i, j, position;
	vAncestor.assign(m_vAncestor.begin(), m_vAncestor.end());
	vDescent.assign(m_vDescent.begin(), m_vDescent.end());

	pBNet->FindMixtureNodes(&vMix);
	for(i=0; i<vMix.size(); i++)
	{
		factor = pBNet->GetFactor(vMix[i]);
		j = static_cast<CMixtureGaussianCPD*>(factor) -> GetNumberOfMixtureNode();
		vMixture.push_back(j);
	}

	for(i=0; i<m_nNodes; i++)
	{
		nt = pMD->GetVariableType(i);
		if( nt->IsDiscrete() )
		{
			vDiscrete.push_back(i);
		}
		else
			vContinuous.push_back(i);
	}
	
	vValidMoves->clear();
	vNeighborCPDs->clear();
	RevCorrespDel->clear();
	pDAG->GetAllValidMove(vValidMoves, &vMixture.front(), vMixture.size(), m_nMaxFanIn, 
		                  &vDiscrete, &vContinuous, &vDescent, &vAncestor );
	int nMoves = vValidMoves->size();
	intVector domain;
	EDGEOP curMove;

	int start, end;
	for(i=0; i<nMoves; i++)
	{
		domain.clear();
		curMove = (*vValidMoves)[i];
		switch (curMove.DAGChangeType)
		{
		case DAG_DEL : 
			start = curMove.originalEdge.startNode;
			end = curMove.originalEdge.endNode;
			factor = pBNet->GetFactor(end);
			factor->GetDomain(&domain);
			position = std::find(domain.begin(), domain.end(), start)
					       - domain.begin();
			domain.erase(domain.begin()+position);
			vNeighborCPDs->push_back(CreateRandomCPD(domain.size(), &domain.front(), pBNet));
			break;

		case DAG_ADD :
			start = curMove.originalEdge.startNode;
			end = curMove.originalEdge.endNode;
			factor = pBNet->GetFactor(end);
			factor->GetDomain(&domain);
			domain.insert(domain.begin(), start);
			vNeighborCPDs->push_back(CreateRandomCPD(domain.size(), &domain.front(), pBNet));
			break;

		case DAG_REV :
			end = curMove.originalEdge.startNode;
			start = curMove.originalEdge.endNode;
			factor = pBNet->GetFactor(end);
			factor->GetDomain(&domain);
			domain.insert(domain.begin(), start);
			vNeighborCPDs->push_back(CreateRandomCPD(domain.size(), &domain.front(), pBNet));
			break;
		}
	}

	RevCorrespDel->assign(nMoves, -1);
	EDGEOP pre_move;
	for(i=0; i<nMoves; i++)
	{
		curMove = (*vValidMoves)[i];
		if(curMove.DAGChangeType == DAG_REV)
		{
			start = curMove.originalEdge.startNode;
			end = curMove.originalEdge.endNode;
			for(j=0; j<nMoves; j++)
			{
				pre_move = (*vValidMoves)[j];
				if( (start == pre_move.originalEdge.startNode) &&
					(end == pre_move.originalEdge.endNode) &&
					(pre_move.DAGChangeType == DAG_DEL) )
				{
					(*RevCorrespDel)[i] = j;
					break;
				}
			}
		}
	}
}

CCPD* CStaticStructLearnSEM::CreateRandomCPD(int nfamily, const int* family, CBNet* pBNet)
{
	int child = family[nfamily-1];
	CModelDomain* pMD = pBNet->GetModelDomain();
	CFactor* factor = pBNet->GetFactor(child);
	EDistributionType dt = factor->GetDistributionType();
	CCPD* pCPD;

	if( dt == dtTabular )
	{
		pCPD = CTabularCPD::Create(family, nfamily, pMD);
		pCPD->CreateAllNecessaryMatrices(1);
		return pCPD;
	}
	else
	{
		if( dt == dtMixGaussian )
		{
			floatVector data;
			static_cast<CMixtureGaussianCPD*>(factor)->GetProbabilities(&data);
			pCPD = CMixtureGaussianCPD::Create(family, nfamily, pMD, &data.front());
			static_cast<CCondGaussianDistribFun*>(pCPD->GetDistribFun()) -> CreateDefaultMatrices(1);
			return pCPD;
		}
		else
		{
			if( (dt == dtGaussian) || (dt == dtCondGaussian) )
			{
				pCPD = CGaussianCPD::Create(family, nfamily, pMD);
				pCPD->CreateAllNecessaryMatrices(1);
				return pCPD;
			}
			else
				PNL_THROW(CNotImplemented, "this type of distribution is not supported yet");
		}				
	}
}

#ifdef PNL_RTTI
const CPNLType CStaticStructLearnSEM::m_TypeInfo = CPNLType("CStaticStructLearnSEM", &(CStaticLearningEngine::m_TypeInfo));

#endif