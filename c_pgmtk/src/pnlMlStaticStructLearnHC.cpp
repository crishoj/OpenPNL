/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMlStaticStructLearnHC.cpp                                //
//                                                                         //
//  Purpose:   Implementation of the CMlStaticStructLearnHC class          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlMlStaticStructLearnHC.hpp"

PNL_USING

CMlStaticStructLearnHC::CMlStaticStructLearnHC( CStaticGraphicalModel* pGrModel, 
						  ELearningTypes LearnType, 
						  EOptimizeTypes AlgorithmType, 
						  EScoreFunTypes ScoreType, 
						  int nMaxFanIn,
						  intVector& vAncestor,
						  intVector& vDescent,
						  int nRestarts ):
							CMlStaticStructLearn( pGrModel, LearnType, 
								                   AlgorithmType,ScoreType,
												   nMaxFanIn,vAncestor,vDescent )
{
	m_nRestarts = nRestarts;
	m_minProgress = (float)1e-4;
	m_nSteps = 100;
}

CMlStaticStructLearnHC* CMlStaticStructLearnHC::Create( CStaticGraphicalModel* pGrModel, 
						  ELearningTypes LearnType, 
						  EOptimizeTypes AlgorithmType, 
						  EScoreFunTypes ScoreType, 
						  int nMaxFanIn,
						  intVector& vAncestor,
						  intVector& vDescent,
						  int nRestarts )
{
	CMlStaticStructLearnHC* ret = new CMlStaticStructLearnHC( pGrModel, LearnType, 
								                   AlgorithmType,ScoreType,
												   nMaxFanIn, vAncestor,
												   vDescent, nRestarts );
	return ret;
}

void CMlStaticStructLearnHC::SetMaxSteps(int steps)
{
	m_nSteps = steps;
}

void CMlStaticStructLearnHC::Learn()
{
	if(m_Vector_pEvidences.size() == 0)
		PNL_THROW(CInconsistentState, "should set the data first");
	
	CGraph* iGraph = m_pGrModel->GetGraph();
	CDAG* iDAG = CDAG::Create(*iGraph);
	CDAG* pDAG;
	CDAG* pBestDAG = NULL;
	float BestScore = (float)-1e37;
	int irestarts = m_nRestarts;
	int i, istart;
	float score;
	for(istart=0; istart<irestarts; istart++)
	{
		if(istart>0)
		{
			delete iDAG;
			intVector	vDiscrete, vContinuous;
			const CNodeType* nt;
			for(i=0; i<m_nNodes; i++)
			{
				nt = m_pGrModel->GetNodeType(i);
				if( nt->IsDiscrete() )
					vDiscrete.push_back(i);
				else
					vContinuous.push_back(i);
			}
			iDAG = CDAG::RandomCreateADAG(m_nNodes, m_nMaxFanIn, vDiscrete, vContinuous);
		}
		LearnInOneStart(iDAG, &pDAG, &score);
		if(score > BestScore)
		{
			delete pBestDAG;
			pBestDAG = pDAG->Clone();
			BestScore = score;
		}
		delete pDAG;
	}
	delete iDAG;
	m_pResultDAG = pBestDAG->Clone();
	m_critValue.push_back(BestScore);
	delete pBestDAG;
}

void CMlStaticStructLearnHC::LearnInOneStart(CDAG* InitDag, 
					CDAG** LearnedDag, float* LearnedScore)
{
	int		i, j, step=0;
	bool	progress = true;
	CDAG*	iDAG = InitDag->Clone();
	floatVector FamilyScore;
	float init_score = ScoreDAG(iDAG, &FamilyScore);
	int			nValidMoves;
	EDGEOPVECTOR vValidMoves;
	EDGEOP      move;
	intVector   newFamily;
	intVector	vAncestor, vDescent;
	intVector	vDiscrete, vContinuous;
	int			start, end, position;
	const CNodeType* nt;
	for(i=0; i<m_nNodes; i++)
	{
		nt = m_pGrModel->GetNodeType(i);
		if( nt->IsDiscrete() )
			vDiscrete.push_back(i);
		else
			vContinuous.push_back(i);
	}
	vAncestor.assign(m_vAncestor.begin(), m_vAncestor.end());
	vDescent.assign(m_vDescent.begin(), m_vDescent.end());
	
	while ( step<m_nSteps && progress )
	{
		iDAG->GetAllValidMove(&vValidMoves,&vDiscrete, &vContinuous, &vDescent, &vAncestor);
		nValidMoves = vValidMoves.size();
		float tmp_score, max_score = 0.0f;
		float tmp_start, max_start = 0.0f;
		float tmp_end, max_end = 0.0f;
		int   max_index = 0;
		for(i=0; i<nValidMoves; i++) 
		{
			newFamily.clear();
			move = vValidMoves[i];
			switch (move.DAGChangeType)
			{
			case DAG_DEL : 
				start = move.originalEdge.startNode;
				end = move.originalEdge.endNode;
				iDAG->GetParents(end, &newFamily);
				newFamily.push_back(end);
				position = std::find(newFamily.begin(), newFamily.end(), start)
					       - newFamily.begin();
				newFamily.erase(newFamily.begin()+position);

				tmp_score = ScoreFamily(newFamily) - FamilyScore[end];
				if(tmp_score > max_score)
				{
					max_score = tmp_score;
					max_index = i;
				}
				break;

			case DAG_ADD :
				start = move.originalEdge.startNode;
				end = move.originalEdge.endNode;
				iDAG->GetParents(end, &newFamily);
				position = newFamily.size();
				for(j=0; j<newFamily.size(); j++)
				{
					if(start<newFamily[j])
					{
						position = j;
						break;
					}
				}
				if(position == int(newFamily.size()))
					newFamily.push_back(start);
				else
					newFamily.insert(newFamily.begin()+position, start);		
				newFamily.push_back(end);
				if(newFamily.size() > (m_nMaxFanIn+1))
					break;
				tmp_score = ScoreFamily(newFamily) - FamilyScore[end];
				if(tmp_score > max_score)
				{
					max_score = tmp_score;
					max_index = i;
				}
				break;

			case DAG_REV :
				start = move.originalEdge.startNode;
				end = move.originalEdge.endNode;
				iDAG->GetParents(start, &newFamily); //add an edge
				position = newFamily.size();
				for(j=0; j<newFamily.size(); j++)
				{
					if(end<newFamily[j])
					{
						position = j;
						break;
					}
				}
				if(position == int(newFamily.size()))
					newFamily.push_back(end);
				else
					newFamily.insert(newFamily.begin()+position, end);		
				newFamily.push_back(start);
				if(newFamily.size() > (m_nMaxFanIn+1))
					break;
				tmp_score = ScoreFamily(newFamily) - FamilyScore[start];
				tmp_start = tmp_score;

				start = move.originalEdge.startNode;
				end = move.originalEdge.endNode;
				iDAG->GetParents(end, &newFamily);
				newFamily.push_back(end);
				position = std::find(newFamily.begin(), newFamily.end(), start)
					       - newFamily.begin();
				newFamily.erase(newFamily.begin()+position);

				tmp_score = ScoreFamily(newFamily) - FamilyScore[end];
				tmp_end = tmp_score;
				tmp_score = tmp_start + tmp_end;
				if(tmp_score > max_score)
				{
					max_score = tmp_score;
					max_start = tmp_start;
					max_end   = tmp_end;
					max_index = i;
				}
				break;
			}
		}

		float score_gate = (float)fabs(m_minProgress * init_score);
		if(max_score <= score_gate)
		{
			vValidMoves.clear();
			progress = false;
			break;
		}

		move = vValidMoves[max_index];
		start = move.originalEdge.startNode;
		end = move.originalEdge.endNode;
		switch (move.DAGChangeType)
		{
		case DAG_DEL :
			if(iDAG->DoMove(start, end, DAG_DEL))
			{ 
				init_score += max_score;
				FamilyScore[end] += max_score;
			}
			break;

		case DAG_ADD :
			if(iDAG->DoMove(start, end, DAG_ADD))
			{
				init_score += max_score;
				FamilyScore[end] += max_score;
			}
			break;

		case DAG_REV :
			if(iDAG->DoMove(start, end, DAG_REV))
			{
				init_score += max_score;
				FamilyScore[start] += max_start;
				FamilyScore[end] += max_end;
			}
			break;
		}
		vValidMoves.clear();
		step++;
	}

	*LearnedScore = this->ScoreDAG(iDAG, &FamilyScore);
	*LearnedDag = iDAG->Clone();
	delete iDAG;
}

#ifdef PNL_RTTI
const CPNLType CMlStaticStructLearnHC::m_TypeInfo = CPNLType("CMlStaticStructLearnHC", &(CMlStaticStructLearn::m_TypeInfo));

#endif