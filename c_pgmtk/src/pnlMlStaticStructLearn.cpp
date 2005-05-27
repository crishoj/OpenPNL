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
#include "pnlTabularDistribFun.hpp"

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
	m_ScoreMethod = MaxLh;
	m_priorType = Dirichlet;
	m_K2alfa = 0;
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
    if (m_ScoreMethod == MarLh)
    {
        score = 1.0f;
    }
    for(int i=0; i<m_nNodes; i++)
    {
        vFamily.clear();
        pDAG->GetParents(i, &vFamily);
        vFamily.push_back(i);
        fscore = ScoreFamily(vFamily);
        familyScore->push_back(fscore);
        if (m_ScoreMethod == MarLh)
        {
            score *= fscore;
        }
        else
        {
            score += fscore;
        }
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
	CTabularDistribFun *pDistribFun;
	int ncases = m_Vector_pEvidences.size();
	const CEvidence * pEv;
	float score;
	float pred = 0;
	EDistributionType NodeType;
    switch (m_ScoreMethod)
	{
	case MaxLh :
		if ( !((iCPD->GetDistribFun()->GetDistributionType() == dtSoftMax) 
			|| (iCPD->GetDistribFun()->GetDistributionType() == dtCondSoftMax)))
		{
			iCPD->UpdateStatisticsML( &m_Vector_pEvidences.front(), ncases );
			score = iCPD->ProcessingStatisticalData(ncases);
		}
		else
		{
			float **evid = NULL;
			float **full_evid = NULL;	
			BuildFullEvidenceMatrix(&full_evid);	
			CSoftMaxCPD* SoftMaxFactor = static_cast<CSoftMaxCPD*>(iCPD);
			SoftMaxFactor->BuildCurrentEvidenceMatrix(&full_evid, &evid,
				  vFamily,m_Vector_pEvidences.size());
			SoftMaxFactor->InitLearnData(); 
			SoftMaxFactor->SetMaximizingMethod(mmGradient);
			SoftMaxFactor->MaximumLikelihood(evid, m_Vector_pEvidences.size(),
				0.00001f, 0.01f);
			SoftMaxFactor->CopyLearnDataToDistrib();
			if (SoftMaxFactor->GetDistribFun()->GetDistributionType() == dtSoftMax)
				
			{
				score = ((CSoftMaxDistribFun*)SoftMaxFactor->GetDistribFun())->CalculateLikelihood(evid,ncases);
			}
			else
			{
				score = ((CCondSoftMaxDistribFun*)SoftMaxFactor->GetDistribFun())->CalculateLikelihood(evid,ncases);  
			};
			for (int k = 0; k < SoftMaxFactor->GetDomainSize(); k++)
			{
				delete [] evid[k];
			}
			delete [] evid;
			int i;
			intVector obsNodes;
			(m_Vector_pEvidences[0])->GetAllObsNodes(&obsNodes);
			for (i=0; i<obsNodes.size(); i++)
			{
				delete [] full_evid[i];
			}
			delete [] full_evid;
		};
		break;
	case PreAs :
		int i;
		NodeType = iCPD->GetDistributionType();
		switch (NodeType)
		{
		case dtTabular : 
			for(i = 0; i < ncases; i++)
			{
				
				pConstEvidenceVector tempEv(0);
				tempEv.push_back(m_Vector_pEvidences[i]);
				iCPD->UpdateStatisticsML(&tempEv.front(), tempEv.size());
				iCPD->ProcessingStatisticalData(tempEv.size());
				pred += log(((CTabularCPD*)iCPD)->GetMatrixValue(m_Vector_pEvidences[i]));
			}
			break;
		case dtGaussian :
			for(i = 0; i < ncases; i += 1 )
			{
				
				pConstEvidenceVector tempEv(0);
				tempEv.push_back(m_Vector_pEvidences[i]);
				
				iCPD->UpdateStatisticsML(&tempEv.front(), tempEv.size());
				float tmp = 0;
				if (i != 0)
				{
					tmp =iCPD->ProcessingStatisticalData(1);
					pred +=tmp;
					
				}
				
			}
			break;
		case dtSoftMax: 
			PNL_THROW(CNotImplemented, 
					 "This type score method has not been implemented yet");
			break;
		default: PNL_THROW(CNotImplemented, 
					 "This type score method has not been implemented yet");
			break;
		};
				
		score = pred;
		break;
	case MarLh : 
        {
                //проверка того, что потенциал дискретный
        if (iCPD->GetDistributionType() != dtTabular)
        {
            PNL_THROW(CNotImplemented, 
			    "This type of score method has been implemented only for discrete nets");
        }
        
        int DomainSize;
        const int * domain;
		switch(m_priorType)
		{
		case Dirichlet: 
			iCPD->GetDomain(&DomainSize, &domain);
			
			pDistribFun = static_cast<CTabularDistribFun *>(iCPD->GetDistribFun());
			
			pDistribFun->InitPseudoCounts();
			
			for (i=0; i<ncases; i++)
			{
				pEv = m_Vector_pEvidences[i];
				const CEvidence *pEvidences[] = { pEv };
				pDistribFun->BayesUpdateFactor(pEvidences, 1, domain);
			}
			score = pDistribFun->CalculateBayesianScore();
			break;
		case K2:
			iCPD->GetDomain(&DomainSize, &domain);
			
			pDistribFun = static_cast<CTabularDistribFun *>(iCPD->GetDistribFun());
			
			pDistribFun->InitPseudoCounts(m_K2alfa);
			
			for (i=0; i<ncases; i++)
			{
				pEv = m_Vector_pEvidences[i];
				const CEvidence *pEvidences[] = { pEv };
				pDistribFun->BayesUpdateFactor(pEvidences, 1, domain);
			}
			score = pDistribFun->CalculateBayesianScore();
			break;
		case BDeu:
			iCPD->GetDomain(&DomainSize, &domain);
			
			pDistribFun = static_cast<CTabularDistribFun *>(iCPD->GetDistribFun());
			
			pDistribFun->InitPseudoCounts();
			
			for (i=0; i<ncases; i++)
			{
				pEv = m_Vector_pEvidences[i];
				const CEvidence *pEvidences[] = { pEv };
				pDistribFun->BayesUpdateFactor(pEvidences, 1, domain);
			}
			score = pDistribFun->CalculateBayesianScore() / iCPD->GetNumberOfFreeParameters();
			break;
		default:
			PNL_THROW(CNotImplemented, 
				"This type of prior has not been implemented yet");
			break;
		}
       

		break;
        }
	default : PNL_THROW(CNotImplemented, 
				  "This type score method has not been implemented yet");
		break;
	}
	

		int dim = iCPD->GetNumberOfFreeParameters();
		switch (m_ScoreType)
		{
		case BIC :
			score -= 0.5f * float(dim) * float(log(float(ncases)));
			break;
		case AIC :
			score -= 0.5f * float(dim);
			break;
		case WithoutFine:
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
	int DomainSize;
    const int * domain;
	const CEvidence * pEv;
	int i;
	CTabularDistribFun *pDistribFun;
	CCPD* iCPD = this->CreateRandomCPD(nFamily, 
		&vFamily.front(), pGrModel);
    int ncases = m_Vector_pEvidences.size();
    if ( !(iCPD->GetDistributionType() == dtSoftMax))
	{
		if (m_ScoreMethod != MarLh)
		{
			iCPD->UpdateStatisticsML( pEvidences, ncases );
			iCPD->ProcessingStatisticalData(ncases);
		}
		else
		{
			iCPD->GetDomain(&DomainSize, &domain);
			
			pDistribFun = static_cast<CTabularDistribFun *>(iCPD->GetDistribFun());
			
			pDistribFun->InitPseudoCounts(m_K2alfa);
			
			for (i=0; i<ncases; i++)
			{
				pEv = m_Vector_pEvidences[i];
				const CEvidence *pEvidences[] = { pEv };
				pDistribFun->BayesUpdateFactor(pEvidences, 1, domain);
			}
			pDistribFun->PriorToCPD();
		}
    }
	else
	{
		float **evid = NULL;
		float **full_evid = NULL;	
		BuildFullEvidenceMatrix(&full_evid);
		CSoftMaxCPD* SoftMaxFactor = (CSoftMaxCPD*)iCPD;
		SoftMaxFactor->BuildCurrentEvidenceMatrix(&full_evid, &evid,vFamily,
			m_Vector_pEvidences.size());
		SoftMaxFactor->InitLearnData();
		SoftMaxFactor->SetMaximizingMethod(mmGradient);
		SoftMaxFactor->MaximumLikelihood(evid, m_Vector_pEvidences.size(),
			0.00001f, 0.01f);
		SoftMaxFactor->CopyLearnDataToDistrib();
		for (int k = 0; k < SoftMaxFactor->GetDomainSize(); k++)
		{
			delete [] evid[k];
		}
		delete [] evid;
		int i;
		intVector obsNodes;
		(m_Vector_pEvidences[0])->GetAllObsNodes(&obsNodes);
		for (i=0; i<obsNodes.size(); i++)
        {
            delete [] full_evid[i];
        }
        delete [] full_evid;
	};
    return iCPD;
}

CCPD* 
CMlStaticStructLearn::CreateRandomCPD(int nfamily, const int* family, CGraphicalModel* pGrModel)
{
	CModelDomain* pMD = pGrModel->GetModelDomain();
	EDistributionType dt = pnlDetermineDistributionType( pMD, nfamily, family, NULL);
                 
    CCPD* pCPD;
   
	int i;
	int j;
    // checking SoftMax distribution 
	 for( i = 0; i < nfamily; i++ )
    {
        if( (!pMD->GetVariableType(family[i])->IsDiscrete()) 
			&& pMD->GetVariableType(family[nfamily-1])->IsDiscrete() )
        {
        for( j = 0; j < nfamily-1; j++ ) 
			if(pMD->GetVariableType(family[j])->IsDiscrete())
			{
		     dt = dtCondSoftMax;
			 break;
			}; 
		dt =  dtSoftMax;
		break;
        }
    }
  // end of checking
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
	case dtSoftMax:
    case dtCondSoftMax:
        pCPD = CSoftMaxCPD::Create(family, nfamily, pMD);  
        pCPD->CreateAllNecessaryMatrices(1); 
		break;
    default:
		PNL_THROW(CNotImplemented, "This type of distribution has not been implemented yet");
		break;
    }
	return pCPD;
}

EScoreMethodTypes CMlStaticStructLearn::GetScoreMethod()
{
 return  m_ScoreMethod;
}

void CMlStaticStructLearn::SetScoreMethod(EScoreMethodTypes Type)
{
	m_ScoreMethod = Type;
}

EPriorTypes CMlStaticStructLearn::GetPriorType()
{
	return m_priorType;
}

void CMlStaticStructLearn::SetPriorType(EPriorTypes ptype)
{
	m_priorType = ptype;
}

void CMlStaticStructLearn::SetScoreFunction(EScoreFunTypes ftype)
{
	m_ScoreType = ftype;
}

EScoreFunTypes CMlStaticStructLearn::GetScoreFunction()
{
 return m_ScoreType;
}

int CMlStaticStructLearn::GetK2PriorParam()
{
    return m_K2alfa;
}

void CMlStaticStructLearn::SetK2PriorParam(int alfa)
{
	m_K2alfa = alfa;
}

#ifdef PNL_RTTI
const CPNLType CMlStaticStructLearn::m_TypeInfo = CPNLType("CMlStaticStructLearn", &(CStaticLearningEngine::m_TypeInfo));

#endif