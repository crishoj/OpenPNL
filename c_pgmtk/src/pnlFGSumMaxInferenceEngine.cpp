/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlFactorGraph.cpp                                          //
//                                                                         //
//  Purpose:   CFactorGraph class member functions implementation          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlFGSumMaxInferenceEngine.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlScalarPotential.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlEvidence.hpp"

//observed node have stupid belief, we work with shrinked model,
//at the end of method Marginal Nodes it expand if its requiered 

PNL_USING

CFGSumMaxInfEngine*
CFGSumMaxInfEngine::Create(const CStaticGraphicalModel *pGraphicalModel)
{
    EModelTypes mt = pGraphicalModel->GetModelType();
    if( mt != mtFactorGraph )
    {
        PNL_THROW( CInconsistentType,
            "this type of inference is for factor graphs only" );
    }
    CFGSumMaxInfEngine* resInf =  new CFGSumMaxInfEngine( 
        static_cast<const CFactorGraph*>(pGraphicalModel) ); 
    return resInf;
}

void CFGSumMaxInfEngine::EnterEvidence( const CEvidence *evidence,
                                       int maximize, int sumOnMixtureNode )
{
    if( !evidence )
    {
        PNL_THROW( CNULLPointer, "evidence" )
    }
    if( m_pShrinkedFG )
    {
        delete m_pShrinkedFG;
        m_pShrinkedFG = NULL;
    }
    m_pShrinkedFG = static_cast<const CFactorGraph*>(
        m_pGraphicalModel)->Shrink( evidence );
    m_bMaximize = maximize ? 1:0 ;
    m_pEvidence = evidence;
    //we need to clear all previous Data 
    DestroyAllMessages();
    // and create new arrays
    //init all protected fields
    InitEngine();
    //initialize messages
    InitMessages();
    //start inference
    ParallelProtocol();
}

void CFGSumMaxInfEngine::SetTolerance( float tolerance)
{
    if(( tolerance < FLT_MIN )||( tolerance >= FLT_MAX ))
    {
        PNL_THROW( COutOfRange, "tolerance" )
    }
    m_tolerance = tolerance;
}

void CFGSumMaxInfEngine::SetMaxNumberOfIterations(int number)
{
    if(number>=0)
    {
        m_maxNumberOfIterations = number;
    }
}



const CEvidence * CFGSumMaxInfEngine::GetMPE() const
{
    if( !m_bMaximize )
    {
        PNL_THROW( CInvalidOperation, "MPE - only for case of maximization")
    }
    return m_pEvidenceMPE;
}

const CPotential* CFGSumMaxInfEngine::GetQueryJPD() const
{
    if( m_bMaximize )
    {
        PNL_THROW( CInvalidOperation, "JPD - only for case of sum")
    }
    return m_pQueryJPD;
}

void CFGSumMaxInfEngine::MarginalNodes(const int *query, int querySize, int notExpandJPD )
{
    if( querySize <= 0 )
    {
        PNL_THROW( COutOfRange, "querySize" )
    }
    if (m_pQueryJPD)
    {
        delete m_pQueryJPD;
    }
    if (m_pEvidenceMPE )
    {
        delete m_pEvidenceMPE;
    }
    int numObsNdsInQuery = 0;
    int i;
    for( i = 0; i < querySize; i++ )
    {
        if( m_pEvidence->IsNodeObserved(query[i]) )
        {
            numObsNdsInQuery++;
        }
    }
    //handle frequent case
    if( querySize == 1 )
    {
        m_pQueryJPD = static_cast<CPotential*>(
            m_beliefs[query[0]]->GetNormalized());
    }
    else
    {
        //need to multply all potentials in one and marginalize it to the query
        //without expanding
        pFactorVector factors;
        m_pShrinkedFG->GetFactors(querySize, query, &factors);
        if( !factors.empty() )
        {
            m_pQueryJPD = static_cast<CPotential*>(factors[0])->Marginalize( query,
                querySize, m_bMaximize );
            for( i = 0; i < querySize; i++ )
            {
                (*m_pQueryJPD) *= (*m_beliefs[query[i]] );
                //this doesn't support multiplication with maximization!
            }
            m_pQueryJPD->Normalize();
        }
        else
        {
            PNL_THROW( CInconsistentType,
                "all nodes from query must be from one factor" );
        }
    }
    if(( numObsNdsInQuery && !notExpandJPD )||( m_bMaximize ))
    {
        CPotential* pot = m_pQueryJPD->ExpandObservedNodes( m_pEvidence );
        if( m_bMaximize )
        {
            m_pEvidenceMPE = pot->GetMPE();
            delete m_pQueryJPD;
            m_pQueryJPD = NULL;
            delete pot;
        }
        else
        {
            delete m_pQueryJPD;
            m_pQueryJPD = pot;
            if( m_pQueryJPD->GetDistributionType() == dtGaussian )
            {
                static_cast<CGaussianDistribFun*>(
                    m_pQueryJPD->GetDistribFun())->UpdateMomentForm();
            }
        }
    }
}

CFGSumMaxInfEngine::~CFGSumMaxInfEngine()
{
    DestroyAllMessages();
    delete m_pShrinkedFG;
    m_pShrinkedFG = NULL;
}

void CFGSumMaxInfEngine::InitEngine()
{
    int i, j, k;
    int numVariables = m_pShrinkedFG->GetNumberOfNodes();
    int numFactors = m_pShrinkedFG->GetNumFactorsAllocated();
    m_beliefs.assign( numVariables, (fgMessage)0 );
    m_oldBeliefs.assign(numVariables, (fgMessage)0 );
    m_curFGMessagesToVars.resize( numVariables );
    m_curFGMessagesToFactors.resize(numFactors);
    m_newFGMessagesToVars.resize( numVariables );
    m_newFGMessagesToFactors.resize(numFactors);
    //need to determine indices for multiplying every message 
    m_indicesForMultVars.resize(numVariables);
    m_indicesForMultFactors.resize(numFactors);
    for( i = 0; i < numVariables; i++ )
    {
        int num = m_pShrinkedFG->GetNumNbrFactors(i);
        m_curFGMessagesToVars[i].resize(num);
        m_newFGMessagesToVars[i].resize(num);
        m_indicesForMultVars[i].resize(num);
        //compute indices
        for( j = 0; j < num; j++ )
        {
            m_indicesForMultVars[i][j].resize(num - 1);
            for( k = 0; k < j; ++k)
            {
                m_indicesForMultVars[i][j][k] = k;
            }
            for(; k < num - 1; k++ )
            {
                m_indicesForMultVars[i][j][k] = k + 1;
            }
        }
    }
    for( i = 0; i < numFactors; i++ )
    {
        const CFactor* factor = m_pShrinkedFG->GetFactor(i);
        intVector domain;
        factor->GetDomain(&domain);
        int domSize = domain.size();
        m_curFGMessagesToFactors[i].resize(domSize);
        m_newFGMessagesToFactors[i].resize(domSize);
        m_indicesForMultFactors[i].resize(domSize);
        //compute indices
        for( j = 0; j < domSize; j++ )
        {
            m_indicesForMultFactors[i][j].resize(domSize - 1);
            for( k = 0; k < j; ++k)
            {
                m_indicesForMultFactors[i][j][k] = k;
            }
	    for( ; k < domSize - 1; ++k)
	    {
                m_indicesForMultFactors[i][j][k] = k + 1;
	    }
        }
    }
    intVector allNodes;
    allNodes.resize(numVariables);
    for( i = 0; i < numVariables; allNodes[i] = i, i++ );
    m_modelDt = pnlDetermineDistributionType( m_pGraphicalModel->GetModelDomain(),
        numVariables, &allNodes.front(), m_pEvidence  );
    m_areReallyObserved.reserve(numVariables);
    for( i = 0; i < numVariables; i++ )
    {
        if( m_pEvidence->IsNodeObserved(i) )
        {
            m_areReallyObserved.push_back(i);
        }
    }
}

void CFGSumMaxInfEngine::InitMessages()
{
    //its easy - create with evidence
    int numVariables = m_pShrinkedFG->GetNumberOfNodes();
    int numFactors = m_pShrinkedFG->GetNumberOfFactors();
    int i,j;
    intVector obsIndex;
    obsIndex.resize(1);
    obsIndex[0] = 0;
    CModelDomain* pMD = m_pShrinkedFG->GetModelDomain();
    switch(m_modelDt) 
    {
    case dtTabular:
        {
            for( i = 0; i < numVariables; i++ )
            {
                int numNbrFactors = m_pShrinkedFG->GetNumNbrFactors(i);
                if( m_pEvidence->IsNodeObserved( i ) )
                {
                    m_beliefs[i] = 
                        CScalarPotential::Create(
                        &i, 1, pMD, obsIndex );
                    for( j = 0; j < numNbrFactors; j++ )
                    {
                        m_curFGMessagesToVars[i][j] = 
                            CScalarPotential::Create(
                            &i, 1, pMD, obsIndex );
                        m_newFGMessagesToVars[i][j] = 
                            CScalarPotential::Create(
                            &i, 1, pMD, obsIndex );
                    }
                }
                else
                {
                    m_beliefs[i] = InitUnitMessage(i);
                    for( j = 0; j < numNbrFactors; j++ )
                    {
                        m_curFGMessagesToVars[i][j] = InitUnitMessage(i);
                        m_newFGMessagesToVars[i][j] = InitUnitMessage(i);
                    }
                }
                m_oldBeliefs[i] = static_cast<CPotential*>(m_beliefs[i]->Clone());
            }
            for( i = 0; i < numFactors; i++ )
            {
                int domSize;
                const int* dom;
                m_pShrinkedFG->GetFactor(i)->GetDomain(&domSize, &dom);
                for( j = 0; j < domSize; j++ )
                {
                    if( m_pEvidence->IsNodeObserved(dom[j]) )
                    {
                        m_curFGMessagesToFactors[i][j] = 
                            CScalarPotential::Create(
                            &dom[j], 1, pMD, obsIndex );
                    }
                    else
                    {
                        m_curFGMessagesToFactors[i][j] = InitUnitMessage(dom[j]);
                    }
                    m_newFGMessagesToFactors[i][j] = static_cast<CPotential*>(
                        m_curFGMessagesToFactors[i][j]->Clone());
                }
            }
            break;
        }
    case dtGaussian:
        {
            for( i = 0; i < numVariables; i++ )
            {
                int numNbrFactors = m_pShrinkedFG->GetNumNbrFactors(i);
                if( m_pEvidence->IsNodeObserved( i ) )
                {
                    m_beliefs[i] = 
                        CScalarPotential::Create(
                        &i, 1, pMD, obsIndex );
                    for( j = 0; j < numNbrFactors; j++ )
                    {
                        m_curFGMessagesToVars[i][j] = 
                            CScalarPotential::Create(
                            &i, 1, pMD, obsIndex );
                        m_newFGMessagesToVars[i][j] = 
                            CScalarPotential::Create(
                            &i, 1, pMD, obsIndex );
                    }
                }
                else
                {
                    m_beliefs[i] = InitUnitMessage(i);
                    for( j = 0; j < numNbrFactors; j++ )
                    {
                        m_curFGMessagesToVars[i][j] = InitUnitMessage(i);
                        m_newFGMessagesToVars[i][j] = InitUnitMessage(i);
                    }
                }
                m_oldBeliefs[i] = static_cast<CPotential*>(m_beliefs[i]->Clone());
            }
            for( i = 0; i < numFactors; i++ )
            {
                int domSize;
                const int* dom;
                m_pShrinkedFG->GetFactor(i)->GetDomain(&domSize, &dom);
                for( j = 0; j < domSize; j++ )
                {
                    if( m_pEvidence->IsNodeObserved(dom[j]) )
                    {
                        m_curFGMessagesToFactors[i][j] = 
                            CScalarPotential::Create(
                            &dom[j], 1, pMD, obsIndex );
                    }
                    else
                    {
                        m_curFGMessagesToFactors[i][j] = InitUnitMessage(dom[j]);
                    }
                    m_newFGMessagesToFactors[i][j] = static_cast<CPotential*>(
                        m_curFGMessagesToFactors[i][j]->Clone());
                }
            }
            break;
        }
    default:
        {
            PNL_THROW( CNotImplemented, "only tabualar and Gaussian" )
        }
    }    
}


void CFGSumMaxInfEngine::ParallelProtocol()
{
    //start sending messages 
    int converged = 0;
    int changed = 0;
    int iter = 0;
    int i, j;
    int numVariables = m_pShrinkedFG->GetNumberOfNodes();
    int numFactors = m_pShrinkedFG->GetNumFactorsAllocated();
    while( (!converged)&&(iter<m_maxNumberOfIterations) )
    {
        changed = 0;
        for( i = 0; i < numFactors; i++ )
        {
            int domSize;
            const int* dom;
            m_pShrinkedFG->GetFactor(i)->GetDomain(&domSize, &dom);
            intVector nbrFactors;
            for( j = 0; j < domSize; j++ )
            {
                m_pShrinkedFG->GetNbrFactors(dom[j], &nbrFactors);
                int posOfFact = std::find(nbrFactors.begin(),
                    nbrFactors.end(), i) - nbrFactors.begin();
                (*(m_newFGMessagesToVars[dom[j]][posOfFact])) = 
                    (*(CompMessageFromFactor( j, i, dom[j])));
                
                (*(m_newFGMessagesToFactors[i][j])) = 
                    (*(CompMessageToFactor( dom[j], posOfFact )));

                changed += !m_newFGMessagesToVars[dom[j]][posOfFact]->
                    IsFactorsDistribFunEqual(m_curFGMessagesToVars[dom[j]][posOfFact],
                    m_tolerance);
                changed += !m_newFGMessagesToFactors[i][j]->
                    IsFactorsDistribFunEqual(m_curFGMessagesToFactors[i][j],
                    m_tolerance);
            }
        }
        
        //compute beliefs
        for( i = 0; i < numVariables; i++ )
        {
            int numMes = m_newFGMessagesToVars[i].size();
            if( numMes > 1 )
            {
                fgMessage bel = m_newFGMessagesToVars[i][0]->Multiply( m_newFGMessagesToVars[i][1] );
                for( j = 2; j < numMes; j++ )
                {
                    (*bel) *= *m_newFGMessagesToVars[i][j];
                }
                (*m_beliefs[i]) = (*bel);
            }
            else
            {
                (*m_beliefs[i]) = (*m_newFGMessagesToVars[i][0]);
            }
            //changed = !m_oldBeliefs[i]->IsFactorsDistribFunEqual(m_beliefs[i], m_tolerance);
        }
        converged = !(changed);
        //need to replace old messages by new
        for( i = 0; i < numVariables; i++ )
        {
            int numMes = m_newFGMessagesToVars[i].size();
            for( j = 0; j < numMes; j++ )
            {
                (*m_curFGMessagesToVars[i][j]) = (*(m_newFGMessagesToVars[i][j]));
            }
            (*m_oldBeliefs[i]) = (*m_beliefs[i]);
        }
        for( i = 0; i < numFactors; i++ )
        {
            int numMes = m_newFGMessagesToFactors[i].size();
            for( j = 0; j < numMes; j++ )
            {
                (*m_curFGMessagesToFactors[i][j]) = (*(m_newFGMessagesToFactors[i][j]));
            }
        }
        iter++;
    }
    
    //compute beliefs - to have something to compare
    m_IterationCounter = iter;
}


fgMessage CFGSumMaxInfEngine::CompMessageFromFactor(int posOfDestNode, 
                                                    int numFactor, int destNode)
{
    int i;
    fgMessage res = NULL;
    int numMultMessages = m_indicesForMultFactors[numFactor][posOfDestNode].size();
    if( numMultMessages > 0 )
    {
        fgMessage multProd  = static_cast<CPotential*>(m_pShrinkedFG->GetFactor(numFactor))->
            Multiply(m_curFGMessagesToFactors[numFactor]
            [m_indicesForMultFactors[numFactor][posOfDestNode][0]]);
        for( i = 1; i < numMultMessages; i++ )
        {
            *multProd *= *(m_curFGMessagesToFactors[numFactor][
                m_indicesForMultFactors[numFactor][posOfDestNode][i]]);
        }
        res = multProd->Marginalize(&destNode, 1, m_bMaximize);
        delete multProd;
    }
    else
    {
        if( m_pEvidence->IsNodeObserved(destNode) )
        {
            intVector obsInd;
            obsInd.push_back(0);
            res = CScalarPotential::Create(&destNode, 1,
                m_pShrinkedFG->GetModelDomain(), obsInd);
        }
        else
        {
            //need to create unit function of requered type
            res = InitUnitMessage(destNode);
        }
    }
    return res;
}

fgMessage CFGSumMaxInfEngine::CompMessageToFactor(int sourseNode, int numFactor)
{
    int i;
    fgMessage res = NULL;
    int numMultMessages = m_indicesForMultVars[sourseNode][numFactor].size();
    if( numMultMessages > 0 )
    {
        if( numMultMessages == 1 )
        {
            fgMessage copyMes = m_curFGMessagesToVars[sourseNode][
                m_indicesForMultVars[sourseNode][numFactor][0]];
            res = static_cast<CPotential*>(copyMes->Clone());
        }
        else
        {
            res = m_curFGMessagesToVars[sourseNode][
                m_indicesForMultVars[sourseNode][numFactor][0]]->Multiply(
                m_curFGMessagesToVars[sourseNode][
                m_indicesForMultVars[sourseNode][numFactor][1]]);
            if( numMultMessages > 2 )
            {
                for( i = 2; i < numMultMessages; i++ )
                {
                    (*res) *= *(m_curFGMessagesToVars[sourseNode][
                        m_indicesForMultVars[sourseNode][numFactor][i]]);
                }
            }
        }
    }
    else
    {
        //need to create unit function of requered type 
        if( m_pEvidence->IsNodeObserved(sourseNode) )
        {
            intVector obsInd;
            obsInd.push_back(0);
            res = CScalarPotential::Create(&sourseNode, 1,
                m_pShrinkedFG->GetModelDomain(), obsInd);
        }
        else
        {
            res = InitUnitMessage(sourseNode);
        }
    }
    return res;
}

void CFGSumMaxInfEngine::DestroyAllMessages()
{
    int i,j;
    m_tolerance = 1e-7f;
    int numBeliefs = m_beliefs.size();
    for ( i = 0; i < numBeliefs; i++)
    {
        if( m_beliefs[i] )
        {
            delete m_beliefs[i];
        }
    }
    m_beliefs.clear();
    int numMes = m_curFGMessagesToVars.size();
    for( i = 0; i < numMes; i++ )
    {
        int numMesHere = m_curFGMessagesToVars[i].size();
        for ( j = 0; j <numMesHere; j++ )
        {
            delete m_curFGMessagesToVars[i][j];
            delete m_newFGMessagesToVars[i][j];
        }
        m_curFGMessagesToVars[i].clear();
        m_newFGMessagesToVars[i].clear();
    }
    m_curFGMessagesToVars.clear();
    m_newFGMessagesToVars.clear();
    m_areReallyObserved.clear();
}

CFGSumMaxInfEngine::CFGSumMaxInfEngine( const CFactorGraph *pFactorGraph ):
CInfEngine( itFGSumMaxProduct, pFactorGraph )
{
    m_bDense = 1;
    m_maxNumberOfIterations = pFactorGraph->GetNumberOfNodes();
    m_pShrinkedFG = NULL;   
}

#ifdef PNL_RTTI
const CPNLType CFGSumMaxInfEngine::m_TypeInfo = CPNLType("CFGSumMaxInfEngine", &(CInfEngine::m_TypeInfo));

#endif