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
#include "pnlRng.hpp"

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

void CEMLearningEngine::Cast(const CPotential * pot, int node, int ev, float *** full_evid)
{
//    EDistributionType dt = pot->GetDistributionType();
  //  if (dt == dtTabular) printf("Tabular distribFun\n");
    
    int dims;
    dims = pot->GetDistribFun()->GetMatrix(matTable)->GetNumberDims();
    const int * ranges;
    pot->GetDistribFun()->GetMatrix(matTable)->GetRanges(&dims, &ranges);

    float * segment = new float [ranges[dims-1]+1];
    segment[0] = 0;

    int i, j;

    if (dims == 1)  //discrete node has no discrete parents
    {
        int * multiindex = new int [1];

        for (i=1; i <= ranges[0]; i++)
        {
            multiindex[0] = i-1;
            segment[i] = segment[i-1] + pot->GetDistribFun()->
                GetMatrix(matTable)->GetElementByIndexes(multiindex); 
        }
        segment[0] = -0.001f;

        delete [] multiindex;
    }

    else //discrete node has discrete parents
    {
        int   data_length; 
        const float * data;
        (static_cast <CDenseMatrix <float> * >(pot->GetDistribFun()->GetMatrix(matTable)))->GetRawData (&data_length, &data);
        
        float  * probability = new float [ranges[dims-1]]; 
        for (i=0; i< ranges[dims-1]; i++)
        {
            probability[i] = 0;
        }

        for (i=0; i< ranges[dims-1]; i++)
        {
            for (j=0; (j*ranges[dims-1]+i) < data_length; j++)
            {
                probability[i] += data[ j*ranges[dims-1] + i ];
            }

        }

        for (i=1; i <= ranges[dims-1]; i++)
        {
            segment[i] = segment[i-1] + probability[i-1];
        }

        delete [] probability;
    }

    segment[0] = -0.001f;
    float my_val = pnlRand(0.0f, 1.0f);
    for (i=1; i<=ranges[dims-1]; i++)
    {
        if ((my_val > segment[i-1]) && (my_val <= segment[i]))
        {
            (*full_evid)[node][ev] = i-1;
        }
    }
    delete [] segment;
    
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

    bool IsCastNeed = false;
    int i;
    for( i = 0; i < nFactors; i++ )
    {
        pFactor = pGrModel->GetFactor(i);
        EDistributionType dt = pFactor->GetDistributionType();
        if ( dt == dtSoftMax ) IsCastNeed = true;
    }

    float ** full_evid = NULL;
    if (IsCastNeed)
    {
        BuildFullEvidenceMatrix(&full_evid);
    }

    
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
                
                intVector family;
				family.resize(0);
                pGrModel->GetGraph()->GetParents(i, &family);
                family.push_back(i);
                CSoftMaxCPD* SoftMaxFactor = static_cast<CSoftMaxCPD*>(factor);
                SoftMaxFactor->BuildCurrentEvidenceMatrix(&full_evid, 
					&evid,family,m_Vector_pEvidences.size());
				SoftMaxFactor->InitLearnData();
                SoftMaxFactor->SetMaximizingMethod(m_MaximizingMethod);
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
        const CPotential * pot;
        
/*        bool IsCastNeed = false;
        int i;
        for( i = 0; i < nFactors; i++ )
        {
            pFactor = pGrModel->GetFactor(i);
            EDistributionType dt = pFactor->GetDistributionType();
            if ( dt == dtSoftMax ) IsCastNeed = true;
        }

        float ** full_evid;
        if (IsCastNeed)
        {
            BuildFullEvidenceMatrix(full_evid);
        }*/
        
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
                        pot = pInfEng->GetQueryJPD(); 
                        if ( (!(m_Vector_pEvidences[ev])->IsNodeObserved(i)) && (IsCastNeed) )
                        {
                            Cast(pot, i, ev, &full_evid);
                        }
                        EDistributionType dt;
                        dt = pFactor->GetDistributionType();
                        if ( !(dt == dtSoftMax) )
                            pFactor->UpdateStatisticsEM( /*pInfEng->GetQueryJPD */ pot, pEv );
                    }
                    else
                    {
                        if ((pFactor->GetDistributionType()) != dtSoftMax)
                            pFactor->UpdateStatisticsML( &pEv, 1 );
                    }
                }
            }
            
            int i;
/*
            printf ("\n My Full Evidence Matrix");
            for (i=0; i<nFactors; i++)
            {
                for (j=0; j<GetNumEv(); j++)
                {
                    printf ("%f   ", full_evid[i][j]);
                }
                printf("\n");
            } 
*/            
            float **evid = NULL;
            EDistributionType dt;
            CFactor *factor = NULL;
            // int i;
            for (i = 0; i < nFactors; i++)
            {
                factor = pGrModel->GetFactor(i);
                dt = factor->GetDistributionType();
                if (dt == dtSoftMax)
                {
					intVector family;
				    family.resize(0);
                    pGrModel->GetGraph()->GetParents(i, &family);
                    family.push_back(i);
                    CSoftMaxCPD* SoftMaxFactor = static_cast<CSoftMaxCPD*>(factor);
					SoftMaxFactor->BuildCurrentEvidenceMatrix(&full_evid, 
						&evid,family,m_Vector_pEvidences.size());
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
   
    if (IsCastNeed)
    {
        int NumOfNodes = pGrModel->GetGraph()->GetNumberOfNodes();
        for (i=0; i<NumOfNodes; i++)
        {
            delete [] full_evid[i];
        }
        delete [] full_evid;
    }

}

void CEMLearningEngine::LearnExtraCPDs(int nMaxFamily, pCPDVector* additionalCPDs, floatVector* additionalLLs)
{

    CStaticGraphicalModel *pGrModel =  this->GetStaticModel();
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    PNL_CHECK_LEFT_BORDER(GetNumEv(), 1);
    
    int numberOfFactors = pGrModel->GetNumberOfFactors();
    int numberOfAddFactors = additionalCPDs->size();
    
    additionalLLs->resize(numberOfAddFactors);
    additionalLLs->clear();
    
    m_vFamilyLogLik.resize(numberOfFactors);
    float	loglik = 0.0f, ll;
    int		i, ev;
    int iteration = 0;
    const CEvidence* pEv;
    
    CFactor *factor = NULL;
    int nnodes;
    const int * domain;
    
    bool bInfIsNeed;
    CInfEngine *pInfEng = m_pInfEngine;
    
    if (IsAllObserved())
    {
        for (i = 0; i < numberOfFactors; i++)
        {
            factor = pGrModel->GetFactor(i);
            factor->UpdateStatisticsML(&m_Vector_pEvidences[GetNumberProcEv()], 
                GetNumEv() - GetNumberProcEv());
        }
        
        for( ev = 0; ev < GetNumEv() ; ev++)
        {
            pEv = m_Vector_pEvidences[ev];
            for( i = 0; i < numberOfAddFactors; i++ )
            {
                factor = static_cast<CFactor*>((*additionalCPDs)[i]);
                factor->UpdateStatisticsML( &pEv, 1 );
            }
        }
        
        switch (pGrModel->GetModelType())
        {
        case mtBNet:
            {
                for( i = 0; i<numberOfFactors; i++ )
                {
                    factor = pGrModel->GetFactor(i);
                    ll = factor->ProcessingStatisticalData( GetNumEv());
                    m_vFamilyLogLik[i] = ll;
                    loglik += ll;
                }
                
                for( i = 0; i < numberOfAddFactors; i++ )
                {
                    factor = static_cast<CFactor*>((*additionalCPDs)[i]);
                    ll = factor->ProcessingStatisticalData( GetNumEv());
                    (*additionalLLs)[i] = ll;
                }
                break;
            }
        case mtMRF2:
        case mtMNet:
            {	
                break;
            }
        default:
            {
                PNL_THROW(CBadConst, "model type" )
                    break;
            }
        }
        m_critValue.push_back(loglik);    
        
    }
    else
    {
        bool isFirst = false;

        bool bContinue;
        do
        {
            ClearStatisticData();
            
            iteration++;
            for( ev = 0; ev < GetNumEv() ; ev++)
            {
                bInfIsNeed = !GetObsFlags(ev)->empty(); 
                pEv = m_Vector_pEvidences[ev];
                if( bInfIsNeed )
                {
                    pInfEng->EnterEvidence(pEv, 0, 0);
                }
                
                for (i = 0; i < numberOfFactors; i++)
                {
                    factor = pGrModel->GetFactor(i);
                    factor->GetDomain( &nnodes, &domain );
                    if( bInfIsNeed && !IsDomainObserved(nnodes, domain, ev ) )
                    {
                        pInfEng->MarginalNodes( domain, nnodes, 1 );
                        factor->UpdateStatisticsEM( pInfEng->GetQueryJPD(), pEv );
                    }
                    else
                    {
                        factor->UpdateStatisticsML( &pEv, 1 );
                    }
                }
                if (!isFirst)
                {
                    for( i = 0; i < numberOfAddFactors; i++ )
                    {
                        factor = static_cast<CFactor*>((*additionalCPDs)[i]);
                        factor->GetDomain( &nnodes, &domain );
                        if( bInfIsNeed && !IsDomainObserved(nnodes, domain, ev ) )
                        {
                            pInfEng->MarginalNodes( domain, nnodes, 1 );
                            factor->UpdateStatisticsEM( pInfEng->GetQueryJPD(), pEv );
                        }
                        else
                        {
                            factor->UpdateStatisticsML( &pEv, 1 );
                        }
                    }
                }
            } // for( ev = 0;  

            loglik = 0.0f;
            switch (pGrModel->GetModelType())
            {
            case mtBNet:
                {
                    for( i = 0; i<numberOfFactors; i++ )
                    {
                        factor = pGrModel->GetFactor(i);
                        ll = factor->ProcessingStatisticalData( GetNumEv());
                        m_vFamilyLogLik[i] = ll;
                        loglik += ll;
                    }
                    if (!isFirst)                
                    {
                        for( i = 0; i < numberOfAddFactors; i++ )
                        {
                            factor = static_cast<CFactor*>((*additionalCPDs)[i]);
                            ll = factor->ProcessingStatisticalData( GetNumEv());
                            (*additionalLLs)[i] = ll;
                        }
                        isFirst = true;
                    }
                    break;
                }
            case mtMRF2:
            case mtMNet:
                {	
                    break;
                }
            default:
                {
                    PNL_THROW(CBadConst, "model type" )
                        break;
                }
            }
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
        
        } while(bContinue);
    }
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

#if 1
    CInfEngine *pJPDInfEng = CNaiveInfEngine::Create(grmodel);
#else
    CInfEngine *pJPDInfEng = CExInfEngine< CJtreeInfEngine >::Create(grmodel);
#endif
    intVector obsNodes(0);
    valueVector obsVals(0);
    CEvidence *emptyEvidence = CEvidence::Create(grmodel->GetModelDomain(), obsNodes, obsVals);
    pJPDInfEng -> EnterEvidence( emptyEvidence );
    int querySize = grmodel->GetNumberOfNodes();
    int *query;
    int i;

    query = new int [querySize];
    for(i = 0; i < querySize; i++ )
    {
	query[i] = i;
    }
    pJPDInfEng->MarginalNodes( query, querySize );
    joint_prob = pJPDInfEng->GetQueryJPD();
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

const float* CEMLearningEngine::GetFamilyLogLik() const
{
    return &m_vFamilyLogLik.front();
}

#ifdef PNL_RTTI
const CPNLType CEMLearningEngine::m_TypeInfo = CPNLType("CEMLearningEngine", &(CStaticLearningEngine::m_TypeInfo));

#endif