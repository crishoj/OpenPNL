/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlEmLearningEngine.cpp                                     //
//                                                                         //
//  Purpose:   Implementation of the EM factor learning algorithm       //
//             for BNet                                                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include <math.h>
#include "pnlTabularPotential.hpp"
#include "pnlEmLearningEngine.hpp"
#include "pnlNaiveInferenceEngine.hpp"
#include "pnlJtreeInferenceEngine.hpp"
#include "pnlExInferenceEngine.hpp"
#include "pnlSoftMaxCPD.hpp"

PNL_USING

CEMLearningEngine::CEMLearningEngine(CStaticGraphicalModel *pGrModel,
  CInfEngine *pInfEng, ELearningTypes LearnType):
  CStaticLearningEngine( pGrModel, LearnType ),
  m_maxIterEM(5), m_precisionEM(0.01f), m_pInfEngine(pInfEng), m_obsFlags(0),
  m_bAllObserved(true), m_numProcEv(0), m_bDelInf( pInfEng == NULL)
{
}

CEMLearningEngine::~CEMLearningEngine()
{
  if( m_bDelInf )
  {
    delete m_pInfEngine;
  }
}

CEMLearningEngine * CEMLearningEngine::
Create( CStaticGraphicalModel *pGrModel, CInfEngine *pInfEng )
{
    if(!pGrModel)
    {
	PNL_THROW(CNULLPointer, "Graphical Model");
    }

    if( pInfEng )
    {
	if( pInfEng->GetModel() != pGrModel )
	{
	    PNL_THROW(CBadArg, "inference engine");
	}
    }
    
    CEMLearningEngine *pNewEngine =
	new CEMLearningEngine( pGrModel, pInfEng, itParamLearnEM );
    return pNewEngine;

}


void CEMLearningEngine::SetMaxIterEM(int nIter )
{
    /*define maximum number of iterations*/
    if( nIter <= 0 )
    {
	PNL_THROW(COutOfRange, "number of iteration must be positively" )
    }

    m_maxIterEM = nIter;
}

void  CEMLearningEngine::SetTerminationToleranceEM(float precision)
{
    if( precision <= FLT_MIN )
    {
	PNL_THROW(CBadConst, "precision be positively" )
    }
    m_precisionEM = precision;

}

void CEMLearningEngine::BuildEvidenceMatrix(int Node, float ***evid)
{
  int i, j;
  const CEvidence* pCurrentEvid;
  CStaticGraphicalModel *grmodel = GetStaticModel();

  intVector parents;
  parents.resize(0);
  grmodel->GetGraph()->GetParents(Node, &parents);
  parents.push_back(Node);

  int NumOfNodes = grmodel->GetNumberOfNodes();
  *evid = new float* [parents.size()];
  for (i = 0; i < parents.size(); i++)
  {
    (*evid)[i] = new float [m_Vector_pEvidences.size()];
  }

  valueVector *values = new valueVector();
  values->resize(NumOfNodes);

  for (i = 0; i < m_Vector_pEvidences.size(); i++)
  {
    pCurrentEvid = m_Vector_pEvidences[i];
    pCurrentEvid->GetRawData(values);
    for (j = 0; j < parents.size(); j++)
    {
      (*evid)[j][i] = (*values)[parents[j]].GetFlt();
    }
  }
}
// ----------------------------------------------------------------------------

void CEMLearningEngine::SetMaximizingMethod(EMaximizingMethod met)
{
  m_MaximizingMethod = met;
}
// ----------------------------------------------------------------------------

EMaximizingMethod CEMLearningEngine::GetMaximizingMethod()
{
  return m_MaximizingMethod;
}
// ----------------------------------------------------------------------------

void CEMLearningEngine::Learn()
{
  CStaticGraphicalModel *pGrModel =  this->GetStaticModel();
  PNL_CHECK_IS_NULL_POINTER(pGrModel);
  PNL_CHECK_LEFT_BORDER(GetNumEv() - GetNumberProcEv() , 1);
  
  CInfEngine *pInfEng = NULL;
  if (m_pInfEngine)
  {
    pInfEng = m_pInfEngine;
  }
  else
  {
    if (!m_bAllObserved)
    {
      pInfEng = CJtreeInfEngine::Create(pGrModel);
      m_pInfEngine = pInfEng;
    }
  }
  
  float loglik = 0.0f;
  
  int nFactors = pGrModel->GetNumberOfFactors();
  const CEvidence *pEv;
  CFactor *pFactor;
  
  int iteration = 0;
  int ev;
  if (IsAllObserved())
  {
    int i;
    float **evid = NULL;
    EDistributionType dt;
    CFactor *factor = NULL;
    for (i = 0; i < nFactors; i++)
    {
      factor = pGrModel->GetFactor(i);
      dt = factor->GetDistributionType();
      if (dt != dtSoftMax)
      {
        factor->UpdateStatisticsML(&m_Vector_pEvidences[GetNumberProcEv()], 
          GetNumEv() - GetNumberProcEv());
      }
      else
      {
        BuildEvidenceMatrix(i, &evid);
        CSoftMaxCPD* SoftMaxFactor = static_cast<CSoftMaxCPD*>(factor);
        SoftMaxFactor->InitLearnData();
        SoftMaxFactor->SetMaximizingMethod(m_MaximizingMethod);
//        SoftMaxFactor->MaximumLikelihood(evid, m_numberOfLastEvidences, 
        SoftMaxFactor->MaximumLikelihood(evid, m_Vector_pEvidences.size(),
          0.00001f, 0.01f);
        SoftMaxFactor->CopyLearnDataToDistrib();
        for (int k = 0; k < factor->GetDomainSize(); k++)
        {
          delete [] evid[k];
        }
        delete [] evid;
      }
    }
    m_critValue.push_back(UpdateModel());
  }
  else
  {
    bool bContinue;
    do
    {
      ClearStatisticData();
      iteration++;
      for( ev = GetNumberProcEv(); ev < GetNumEv() ; ev++ )
      {
        bool bInfIsNeed = !GetObsFlags(ev)->empty(); 
        pEv = m_Vector_pEvidences[ev];
        if( bInfIsNeed )
        {
          pInfEng->EnterEvidence(pEv, 0, 0);
        }
        int i;
        for( i = 0; i < nFactors; i++ )
        {
          pFactor = pGrModel->GetFactor(i);
          int nnodes;
          const int * domain;
          pFactor->GetDomain( &nnodes, &domain );
          if( bInfIsNeed && !IsDomainObserved(nnodes, domain, ev ) )
          {
            pInfEng->MarginalNodes( domain, nnodes, 1 );
            pFactor->UpdateStatisticsEM( pInfEng->GetQueryJPD(), pEv );
          }
          else
          {
            pFactor->UpdateStatisticsML( &pEv, 1 );
          }
        }
      }
      
      loglik = UpdateModel();
      
      if( GetMaxIterEM() != 1)
      {
        bool flag = iteration == 1 ? true : 
        (fabs(2*(m_critValue.back()-loglik)/(m_critValue.back() + loglik)) > GetPrecisionEM() );
        
        bContinue = GetMaxIterEM() > iteration && flag;
      }
      else
      {
        bContinue = false;
      }
      m_critValue.push_back(loglik);
      
    }while(bContinue);
  }
  SetNumProcEv( GetNumEv() );
  
}


float CEMLearningEngine::_LearnPotentials()
{
    float log_lik = 0.0f;
    CStaticGraphicalModel *grmodel =
        static_cast<CStaticGraphicalModel *>(m_pGrModel);
    CFactor *factor = NULL;

    float epsilon = m_precisionIPF;
    const CPotential *joint_prob = NULL;
    CPotential *clique_jpd = NULL;

    CMatrix<float> *itogMatrix;

#if 0
    CInfEngine *pJPDInfEng = CNaiveInfEngine::Create(grmodel);
#else
    CInfEngine *pJPDInfEng = CExInfEngine< CJtreeInfEngine >::Create(grmodel);
#endif
    intVector obsNodes(0);
    valueVector obsVals(0);
    CEvidence *emptyEvidence = CEvidence::Create(grmodel->GetModelDomain(), obsNodes, obsVals);
    m_pInfEngine -> EnterEvidence( emptyEvidence );
    int querySize = grmodel->GetNumberOfNodes();
    int *query;
    int i;

    query = new int [querySize];
    for(i = 0; i < querySize; i++ )
    {
	query[i] = i;
    }
    m_pInfEngine->MarginalNodes( query, querySize );
    joint_prob = m_pInfEngine->GetQueryJPD();
    CPotential *itog_joint_prob =
    static_cast<CPotential *>(joint_prob->Marginalize(query, querySize));
    delete (pJPDInfEng);
    int DomainSize;
    const int *domain;

    potsPVector learn_factors;
    CPotential *tmp_factor;

    for (i = 0; i <  grmodel->GetNumberOfFactors(); i++)
    {
	factor = grmodel -> GetFactor(i);
	factor->GetDomain( &DomainSize, &domain );
	CDistribFun *correspData= factor->GetDistribFun();

	CMatrix<float> *learnMatrix = correspData->GetStatisticalMatrix(stMatTable);

	learn_factors.push_back(CTabularPotential::Create(
	    domain, DomainSize, factor->GetModelDomain()));
	learn_factors[i]->AttachMatrix(learnMatrix->NormalizeAll(), matTable);
    }


    int data_length;
    float *old_itog_data = NULL;
    const float *itog_data;

    delete [] query;
    int convergence = 0;
    while( !convergence )
    {
	itogMatrix = (itog_joint_prob->GetDistribFun())
	    -> GetMatrix(matTable);
	static_cast<CNumericDenseMatrix<float>*>(itogMatrix)
	    ->GetRawData(&data_length, &itog_data);
	old_itog_data = new float[data_length];
	for( i = 0; i < data_length; i++)
	{
	    old_itog_data[i] = itog_data[i];
	}
	for( int clique = 0; clique < grmodel -> GetNumberOfFactors(); clique++)
	{
	    factor = grmodel -> GetFactor(clique);
	    factor -> GetDomain( &DomainSize, &domain );
	    clique_jpd = static_cast<CPotential *>
		(itog_joint_prob -> Marginalize( domain, DomainSize ));


	    tmp_factor = itog_joint_prob -> Multiply(learn_factors[clique]);
	    delete (itog_joint_prob);
	    itog_joint_prob = tmp_factor;
	    tmp_factor = itog_joint_prob -> Divide(clique_jpd);
	    delete (itog_joint_prob);
	    delete (clique_jpd);
	    itog_joint_prob = tmp_factor;

	}
	itogMatrix = (itog_joint_prob->GetDistribFun())
	    -> GetMatrix(matTable);


	static_cast<CNumericDenseMatrix<float>*>(itogMatrix)
	    ->GetRawData(&data_length, &itog_data);
	convergence = true;
	for (int j = 0; j < data_length; j++)
	{
	    if( fabs( itog_data[j] - old_itog_data[j] ) > epsilon)
	    {
		convergence = false;
		break;
	    }

	}
	delete []old_itog_data;

    }


    for(int  clique = 0; clique < grmodel -> GetNumberOfFactors(); clique++)
    {
	CMatrix<float> *matrix = NULL;
	factor = grmodel -> GetFactor(clique);
	int DomainSize;
	const int *domain;
	int data_length;
	const float *data;
	factor -> GetDomain( &DomainSize, &domain );

	matrix = (
	    ( itog_joint_prob->Marginalize( domain, DomainSize ) ) ->
	    GetDistribFun()
	    ) -> GetMatrix( matTable );
	static_cast<CNumericDenseMatrix<float>*>(matrix)
            ->GetRawData(&data_length, &data);
	for(int offset = 0; offset < data_length; offset++)
	{
	    float prob = float( ( data[offset] < FLT_EPSILON ) ? -FLT_MAX : log( data[offset] ) );
            CMatrix<float>* m1 = factor->GetDistribFun()->GetStatisticalMatrix(stMatTable);
	    log_lik +=
		(static_cast<CNumericDenseMatrix<float>*>(m1)->
		GetElementByOffset(offset)) * prob - m_Vector_pEvidences.size();

	}
	factor ->AttachMatrix(matrix, matTable);
	delete (learn_factors[clique]);
    }
    delete (itog_joint_prob);
    learn_factors.clear();

    return log_lik;
}


void CEMLearningEngine::AppendData(int dim, const CEvidence* const* pEvidences)
{ 
   
    int nnodes = GetStaticModel()->GetNumberOfNodes();
    bool flag = true;
    
    PNL_CHECK_LEFT_BORDER(dim, 1);
    m_obsFlags.resize( m_obsFlags.size() + dim, boolVector(nnodes, false) );
    
    int nObsNds;
    const int *obsNds;
    const int *flags;
    
    int nPrevEv = m_Vector_pEvidences.size();
    m_Vector_pEvidences.resize(nPrevEv + dim );
    memcpy(&m_Vector_pEvidences[nPrevEv], pEvidences, dim*sizeof(CEvidence *));
    int i;
    for( i = 0; i < dim; i++ )
    {
	PNL_CHECK_IS_NULL_POINTER(pEvidences[i]);
	
	flag = true;
	nObsNds = pEvidences[i]->GetNumberObsNodes();
	obsNds  = pEvidences[i]->GetAllObsNodes();
	flags   = pEvidences[i]->GetObsNodesFlags();
	if( nObsNds - nnodes )
	{
	    flag = false;
	}
	int j;
	for( j = 0; j < nObsNds; j++ )
	{
	    (m_obsFlags[i])[obsNds[j]] = flags[j] ? true : false;
	    flag = flag && flags[j];
	}
	if( flag )
	{
	    // all nodes are observed
	    (m_obsFlags[i]).clear();
	}
	else
	{
	    m_bAllObserved = false;
	    m_numProcEv = 0;
	}
	
    }
}

void CEMLearningEngine::SetNumProcEv( int num)
{
    m_numProcEv = num;
}

bool CEMLearningEngine::IsDomainObserved( int nnodes, const int* domain, int evNumber )
{
    const boolVector *flags = GetObsFlags(evNumber);
    if( flags->empty())
    {
	return true;
    }
    int i;
    for( i = 0; i < nnodes; i++ )
    {
	if(!(*flags)[domain[i]])
	{
	    return false;
	}
    }
    return true;
}

float CEMLearningEngine::UpdateModel()
{
  CStaticGraphicalModel *pGrModel = GetStaticModel();
  int nFactors = pGrModel->GetNumberOfFactors();
  float loglik = 0.0f;
  switch ( pGrModel->GetModelType())
  {
    case mtBNet:
    {
      
      int i;
      for( i = 0; i < nFactors; i++ )
      {
        if (pGrModel->GetFactor(i)->GetDistributionType() != dtSoftMax)
        {
          loglik += pGrModel->GetFactor(i)->
            ProcessingStatisticalData( GetNumEv() );
        }
      }
      break;
    }
    case mtMRF2:
    case mtMNet:
    {
      loglik = _LearnPotentials();
      break;
    }
    default:
    {
      PNL_THROW(CBadConst, "model type" )
        break;
    }
  }
  return loglik;
}
