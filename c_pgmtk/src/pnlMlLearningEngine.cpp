/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMlLearningEngine.cpp                                     //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <math.h>

#include "pnlConfig.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "pnlMlLearningEngine.hpp"
#include "pnlLearningEngine.hpp"
#include "pnlNaiveInferenceEngine.hpp"
#include "pnlNumericDenseMatrix.hpp"
#include "pnl2DNumericDenseMatrix.hpp"
//#include "pnlNumericSparseMAtrix.hpp"

PNL_USING


CMlLearningEngine::CMlLearningEngine(CStaticGraphicalModel *pGrModel,
				     ELearningTypes LearnType):
				     CStaticLearningEngine(pGrModel, LearnType)
{

}

CMlLearningEngine::~CMlLearningEngine()
{

}



void CMlLearningEngine::SetData(int dim, CEvidence *const *pEvidences)
{
    
    //create vector consists of points to evidences
    
    int i;
    int numberOfparameters = m_pGrModel->GetNumberOfFactors();
    m_Vector_pEvidences.clear();
    for( i = 0; i < dim; i++ )
    {
        if (!pEvidences[i])
        {
            PNL_THROW(CNULLPointer,"evidence")
        }
        if((pEvidences[i]->GetNumberObsNodes()) < numberOfparameters)
        {
            PNL_THROW(CBadArg,"number of observed nodes")
        }
        else
        {
            
            const int *obsNodesFlags = pEvidences[i]->GetObsNodesFlags();
            const int *location;
            location = std::find( obsNodesFlags, obsNodesFlags + numberOfparameters,  0);
            if( (location - (obsNodesFlags+numberOfparameters) ) )
            {
                PNL_THROW(CBadArg,"number of observed nodes")
            }
            
        }
        m_Vector_pEvidences.push_back( pEvidences[i] );
    }
    m_numberOfAllEvidences = m_Vector_pEvidences.size();
    
}




void CMlLearningEngine::Learn()
{
/*
function takes an information from m_pEvidences and learns factors
of graphical model using prior probabilities or not
    */
    float logLikTmp = 0;
    if(!m_pGrModel)
    {
        PNL_THROW( CNULLPointer, "no graphical model")
    }
    CStaticGraphicalModel *grmodel = this->GetStaticModel();
    CFactor *parameter = NULL;
    int numberOfDomains = grmodel -> GetNumberOfFactors();
    
    for( int domainNodes = 0; domainNodes < numberOfDomains; domainNodes++ )
    {
        factor = grmodel->GetFactor( domainNodes );
        factor ->UpdateStatisticsML( &m_Vector_pEvidences.front(), 
            m_Vector_pEvidences.size() );
        PNL_CHECK_LEFT_BORDER(m_numberOfAllEvidences, 1);
        logLikTmp += parameter->ProcessingStatisticalData(m_numberOfAllEvidences);
    }
    switch( grmodel -> GetModelType() )
    {
    case mtBNet:
        {
            break;
        }
    case mtMRF2:
    case mtMNet:
        {
            logLikTmp = _LearnPotentials();
            break;
        }
    default:
        {
            PNL_THROW(CBadConst, "model type" )
                break;
        }
    }
    m_critValue.push_back(logLikTmp);
}

float CMlLearningEngine::_LearnPotentials()
{
    int iteration = 1;
    float log_lik = 0.0f;
    CStaticGraphicalModel *grmodel = this->GetStaticModel();
    CFactor *parameter = NULL;
    
    float epsilon = m_precisionIPF;
    const CPotential *joint_prob = NULL;
    CPotential *clique_jpd = NULL;
    
    CMatrix<float> *itogMatrix;
    
    CInfEngine *m_pInfEngine = 
        CNaiveInfEngine::Create(grmodel);
    intVector obsNodes(0);
    valueVector obsValues(0);
    CEvidence *emptyEvidence = CEvidence::Create(grmodel->GetModelDomain(), obsNodes, obsValues);
    m_pInfEngine -> EnterEvidence( emptyEvidence );
    int querySize = grmodel->GetNumberOfNodes();
    int *query;
    query = new int [querySize];

    int i;
    for( i = 0; i < querySize; i++ )
    {
        query[i] = i;
    }
    m_pInfEngine -> MarginalNodes( query, querySize );
    joint_prob = m_pInfEngine->GetQueryJPD();
    CPotential *itog_joint_prob = 
        static_cast<CPotential *>(joint_prob ->Marginalize(query, querySize));
    
    int DomainSize;
    const int *domain;
    
    potsPVector learn_factors;
    CPotential *tmp_factor;
    
    for (i = 0; i <  grmodel -> GetNumberOfFactors(); i++)
    {
        factor = grmodel -> GetFactor(i);
        factor -> GetDomain( &DomainSize, &domain );
        CDistribFun *correspData= factor -> GetDistribFun();
        
        CMatrix<float> *learnMatrix = correspData ->
            GetStatisticalMatrix(stMatTable);
        
        CPotential *factor = CTabularPotential::Create(	domain, DomainSize,
            parameter->GetModelDomain());
        
        learn_factors.push_back(factor);
        learn_factors[i] -> AttachMatrix(learnMatrix->NormalizeAll(), matTable);
    }
    
    int data_length;
    float *old_itog_data = NULL;
    const float *itog_data;
    
    delete [] query;
    int convergence = 0;	
    while( !convergence && (iteration <= m_maxIterIPF))
    {
        iteration++;
        itogMatrix = (itog_joint_prob->GetDistribFun())
            -> GetMatrix(matTable);
        static_cast<CNumericDenseMatrix<float>*>(itogMatrix)->
            GetRawData(&data_length, &itog_data);
        old_itog_data = new float[data_length];
        for( i = 0; i < data_length; i++)
        {
            old_itog_data[i] = itog_data[i];
        }
        for( int clique = 0; clique < grmodel->GetNumberOfFactors(); clique++)
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
        
        static_cast<CNumericDenseMatrix<float>*>(itogMatrix)->
            GetRawData(&data_length, &itog_data);
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
    if(iteration > m_maxIterIPF)
    {
        PNL_THROW(CNotConverged, 
            "maximum number of iterations for IPF procedure")
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
        
        matrix = itog_joint_prob->Marginalize( domain, DomainSize )
            ->GetDistribFun()-> GetMatrix( matTable );
        static_cast<CNumericDenseMatrix<float>*>(matrix)->GetRawData(&data_length, &data);
        CNumericDenseMatrix<float>* matLearn = 
            static_cast<CNumericDenseMatrix<float>*>(
            parameter->GetDistribFun()->GetStatisticalMatrix(stMatTable));
        for(int offset = 0; offset < data_length; offset++)
        {
            float prob = float( ( data[offset] < FLT_EPSILON ) ? -FLT_MAX : log( data[offset] ) );
            
            log_lik += matLearn->GetElementByOffset(offset)*prob - 
                m_Vector_pEvidences.size();
        }
        factor ->AttachMatrix(matrix, matTable);
        delete (learn_factors[clique]);
    }
    delete (itog_joint_prob);
    learn_factors.clear();
    
    return log_lik;
}
