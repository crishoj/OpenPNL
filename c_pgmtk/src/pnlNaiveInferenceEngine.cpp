/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNaiveInferenceEngine.cpp                                 //
//                                                                         //
//  Purpose:   CnaiveInferenceEngine class member functions implementation //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlNaiveInferenceEngine.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlScalarPotential.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "pnlMNet.hpp"
#include "pnlException.hpp"


PNL_USING

CNaiveInfEngine *
CNaiveInfEngine::Create(const CStaticGraphicalModel *pGraphicalModel)
{
    if( !pGraphicalModel )
    {
        PNL_THROW( CNULLPointer, "graphical model" );
        return NULL;
    }
    else
    {
        CNaiveInfEngine* newInf = new CNaiveInfEngine( pGraphicalModel );
        return newInf;
    }
}


CNaiveInfEngine::CNaiveInfEngine(const CStaticGraphicalModel *pGraphicalModel)
                                 :CInfEngine( itNaive, pGraphicalModel ),
				 m_pJPD(NULL)
{
    /*here we specify factors; we creates them later*/
    //m_pEvidence = NULL;
}

CNaiveInfEngine::~CNaiveInfEngine()
{
    if( m_pJPD ) 
    {
        delete( m_pJPD );
    }
    m_excludeNodes.clear();
}

void CNaiveInfEngine::SetExclude( const intVector& excludeNodes )
{
    SetExclude( excludeNodes.size(), &excludeNodes.front() );
}

void CNaiveInfEngine::SetExclude( int nNodesExclude, const int*excludeNodes )
{
    int i;
    int nNodesInGraph = m_pGraphicalModel->GetNumberOfNodes();
    if ( nNodesExclude >= nNodesInGraph )
    {
        PNL_THROW( CInconsistentSize, " number of nodes to exclude more than all number of nodes " ) ; 
        /*error if we want to create JPD for one node*/
    }
    else
    {
        for( i = 0; i<nNodesExclude; i++ )
        {
            if( ( excludeNodes[i] > nNodesInGraph )||( excludeNodes[i] < 0 ) )
            {
                PNL_THROW( CInconsistentSize, "number of node to exclude isn't number of node in model" )
                    /*error - there is excluded node which is not in Graph*/
            }
        }
    }
    m_excludeNodes = intVector( excludeNodes, excludeNodes+nNodesExclude );
}
//add evidence to the model and providing inference
void CNaiveInfEngine::EnterEvidence( const CEvidence *evidence,
                                    int maximize, int sumOnMixtureNode )
{
    if( !evidence )
    {
        PNL_THROW( CNULLPointer, "evidence" )//no Evidence - NULL pointer
    }
    if( evidence->GetModelDomain() != m_pGraphicalModel->GetModelDomain() )
    {
        PNL_THROW( CInvalidOperation, 
            "evidence and the Graphical Model must be on one Model Domain" );
    }
    m_pEvidence = evidence;
    m_bMaximize = maximize;

    /*We create joint pot here for all nodes in graph
    We don't implement exclude nodes now - it could come later*/
    int i;
    
    intVector         ReallyObsNodes;
    pConstValueVector ObsValues;
    evidence->GetObsNodesWithValues( &ReallyObsNodes, &ObsValues );
    
    int NumReallyObs = ReallyObsNodes.size();
    int nNodesInGraph = m_pGraphicalModel->GetNumberOfNodes();

    //create JPD node Types
    pConstNodeTypeVector pJPDNodeType;
    pJPDNodeType.resize( nNodesInGraph );
    
    /*all nodes in Graphical Model except exclude*/
    intVector pJPDdomain;
    pJPDdomain.resize( nNodesInGraph );
    for ( i = 0; i < nNodesInGraph; i++ )
    {
        pJPDNodeType[i] = m_pGraphicalModel->GetNodeType(i);
        pJPDdomain[i] = i;
    }
    
    //create vector for evidence
    /* pValueVector obsVals(nNodesInGraph, NULL);

    for( i = 0; i < NumReallyObs; i++ )
    {
        obsVals[ReallyObsNodes[i]] = ObsValues[i];
    }
    */
    /*If node is observed now we need to set it's NodeType in discrete
    with 1 value*/
    
    int numMixNodes = 0;
    
    intVector mixtureNodes;
    
    if( sumOnMixtureNode )
    {
        if( m_pGraphicalModel->GetModelType() == mtBNet )
        {
            //also need to change node types for all mixture node if there are they
            
            const CBNet* pBnet = static_cast<const CBNet*>(m_pGraphicalModel);
            for( i = 0; i <nNodesInGraph; i++ )
            {
                const CFactor* pCpd = pBnet->GetFactor(i);
                if( pCpd->GetDistributionType() == dtMixGaussian )
                {
                    mixtureNodes.push_back( 
                        (static_cast<const CMixtureGaussianCPD*>(pCpd))->
                        GetNumberOfMixtureNode() );
                }
            }
            numMixNodes = mixtureNodes.size();
        }
    }
    /*we need to determine distribution type according to NodeTypes 
    of all nodes which are included in JPD*/
    EDistributionType paramDistrType = 
        pnlDetermineDistributionType( nNodesInGraph, NumReallyObs,
        &ReallyObsNodes.front(), &pJPDNodeType.front() );
        /*now we can create Joint Factor for our Graphical Model and 
    convert it to JPD after Multiplying*/
    CPotential *pJointFactor = NULL;
    /*We create new JPD but it doesn't contain any data
    We need to AllocMatrix (or AttachMatrix) to it.
    First we will alloc matrix of ones(for dtTabular),...*/
    CModelDomain* pMD = m_pGraphicalModel->GetModelDomain();
    switch( paramDistrType )
    {
    case dtTabular:
        {
            pJointFactor = CTabularPotential::CreateUnitFunctionDistribution(
                &pJPDdomain.front(), nNodesInGraph, pMD, 1, ReallyObsNodes );
            if( numMixNodes > 0 )
            {
                //need to shrink mixture nodes to size 1
                valueVector mixVals;
                mixVals.resize(numMixNodes);
                for( i = 0; i < numMixNodes; mixVals[i].SetInt(0),i++ );
                CEvidence* evidForMixed = CEvidence::Create( m_pGraphicalModel,
                    numMixNodes, &mixtureNodes.front(), mixVals );
                CPotential* pOtherFactor = pJointFactor->ShrinkObservedNodes( 
                    evidForMixed );
                delete evidForMixed;
                delete pJointFactor;
                pJointFactor = pOtherFactor;
            }
            break;
        }
        /*we implement it later*/
    case dtGaussian:
        {
            pJointFactor = CGaussianPotential::CreateUnitFunctionDistribution(
                &pJPDdomain.front(), nNodesInGraph, pMD, 1, ReallyObsNodes );
            if( numMixNodes > 0 )
            {
                //need to shrink mixture nodes to size 1
                valueVector mixVals;
                mixVals.resize(numMixNodes);
                for( i = 0; i < numMixNodes; mixVals[i].SetInt(0),i++ );
                CEvidence* evidForMixed = CEvidence::Create( m_pGraphicalModel,
                    numMixNodes, &mixtureNodes.front(), mixVals );
                CPotential* pOtherFactor = pJointFactor->ShrinkObservedNodes( 
                    evidForMixed );
                delete evidForMixed;
                delete pJointFactor;
                pJointFactor = pOtherFactor;
            }
            
            break;
        }
    case dtScalar:
        {
            pJointFactor = CScalarPotential::Create( &pJPDdomain.front(),
                nNodesInGraph, pMD, ReallyObsNodes );
            break;
        }
    case dtCondGaussian:
        {
            PNL_THROW( CNotImplemented, "conditional gaussian factors" )
                break;
        }
    default:
        {
            PNL_THROW( CBadConst, "distribution type" )
        }
    }
    /*we need to extract the factors of all nodes from Graphical Model
    and Multiply their Matrices - there are different ways to extract
    factor for MNet and BNet*/
    switch ( m_pGraphicalModel->GetModelType() )
    {
    case mtBNet:
        {
            for( i = 0; i < nNodesInGraph; i++ )
            {
                /*we get factor of this domain and convert it to factor*/
                const CCPD *pTempCPD = static_cast<CCPD*>
                    ( m_pGraphicalModel->GetFactor(i) );
                CPotential *pPotentialWithObserved = 
                    pTempCPD->ConvertWithEvidenceToPotential(
                    m_pEvidence, sumOnMixtureNode );
                (*pJointFactor) *= (*pPotentialWithObserved);
                pJointFactor->Normalize();
                delete pPotentialWithObserved;
            }
            break;
        }//case mtBNet
    case mtMRF2: case mtMNet:
        {
            const CMNet * myMnet = static_cast<const CMNet*>(m_pGraphicalModel);
            int NumCliques = myMnet->GetNumberOfCliques();
            //const int* ClSizes = m_pGraphicalModel->GetCliquesSizes();
            /*we can extract all cliques one by one and Multiply them*/
            for( i = 0; i < NumCliques; i++ )
            {
                int cliqueSize;
                const int *clique  = NULL;
                myMnet->GetClique( i, &cliqueSize, &clique );
                const CPotential *pTempFactor = static_cast<CPotential*>
                    ( m_pGraphicalModel->GetFactor(i) );
                CPotential *pFactorWithObserved = 
                    pTempFactor->ShrinkObservedNodes( m_pEvidence );
                (*pJointFactor) *= (*pFactorWithObserved );
                delete pFactorWithObserved;
            }
            pJointFactor->Normalize();
            break;
        }//case mtMNet
    }//switch
    ReallyObsNodes.clear();
    /*convert Joint Factor to our resulting JPD and free mJPD*/
    if( m_pJPD )
    {
        delete ( m_pJPD );
        m_pJPD = NULL;
    }
    m_pJPD = pJointFactor;    
}
void CNaiveInfEngine::MarginalNodes( const int *query, int querySize, int notExpandJPD )
{
    //to compute common JPD for nodes from Query*/
    int i,j;
    int numAllNodes = m_pGraphicalModel->GetNumberOfNodes();
    if( querySize > numAllNodes )
    {
        PNL_THROW( CInconsistentSize, "query is larger than all JPD" )
            /*query must be subset of domain of JPD - it's size must be less*/
    }
    else
    {
        if( m_pQueryJPD )
        {
            delete m_pQueryJPD;
            m_pQueryJPD = NULL;
        }
        if( m_pEvidenceMPE )
        {
            delete m_pEvidenceMPE;
            m_pEvidenceMPE = NULL;
        }
        //we need to check if where is an observed node in query or not 
        //if there is such node we need to expand result of marginalization 
        //for query to initial size of nodes
        const int *AllObsNodes = m_pEvidence->GetAllObsNodes();
        const int *pObservationFlags = m_pEvidence->GetObsNodesFlags();
        int numObsNodes = 0;
        for( i = 0; i < querySize; i++)
        {
            for( j = 0; j < m_pEvidence->GetNumberObsNodes(); j++ )
            {
                if( ( query[i] == AllObsNodes[j] )&&( pObservationFlags[j] ) )
                {
                    numObsNodes++;
                    break;
                }
                if( numObsNodes )
                {
                    break;
                }
            }
        }
        if( !numObsNodes )
        {
            CPotential *pQueryJPD =static_cast<CPotential*>
                ( m_pJPD->Marginalize( query, querySize, m_bMaximize) );
            if( m_bMaximize )
            {
                m_pEvidenceMPE = pQueryJPD->GetMPE();
                delete pQueryJPD;
                return;
            }
            else
            {
                m_pQueryJPD = pQueryJPD->GetNormalized();
            }
            delete pQueryJPD;
        }
        else
        {
            if(( !m_bMaximize )&&( notExpandJPD ))
            {
                m_pQueryJPD = m_pJPD->Marginalize( query, querySize, m_bMaximize);
            }
            else
            {
                CPotential *ExpandJPD = static_cast<CPotential*>
                    (m_pJPD->ExpandObservedNodes( m_pEvidence, 0));
                CPotential *pMargJPD = static_cast<CPotential*>
                    ( ExpandJPD->Marginalize( query, querySize, m_bMaximize) );
                if( m_bMaximize )
                {
                    m_pEvidenceMPE = pMargJPD->GetMPE();
                    delete ExpandJPD;
                    delete pMargJPD;
                    return;
                }
                else
                {
                    m_pQueryJPD = pMargJPD->GetNormalized();
                }
                delete ExpandJPD;
                delete pMargJPD;
            }
        }
    }
    if( m_pQueryJPD->GetDistributionType() == dtGaussian )
    {
        //we need to update moment form
        static_cast<CGaussianDistribFun*>(m_pQueryJPD->GetDistribFun()
            )->UpdateMomentForm();
    }
}

const CPotential* CNaiveInfEngine::GetQueryJPD() const
{
    if( !m_bMaximize )
    {
        if( m_pQueryJPD )
        {   
            return m_pQueryJPD;
        }
        else
        {
            PNL_THROW( CInvalidOperation, "getting JPD before calling MarginalNodes" )
        }
    }
    else
    {
        PNL_THROW( CInvalidOperation, "JPD - only without maximization" );
    }
}

const CEvidence* CNaiveInfEngine::GetMPE() const
{
    if( !m_bMaximize )
    {
        PNL_THROW( CInvalidOperation, "MPE - only for case of maximization")
    }
    if( m_pEvidenceMPE )
    {   
        return m_pEvidenceMPE;
    }
    else
    {
        PNL_THROW( CInvalidOperation, "getting MPE before calling MarginalNodes" )
    }
}

#ifdef PNL_RTTI
const CPNLType CNaiveInfEngine::m_TypeInfo = CPNLType("CNaiveInfEngine", &(CInfEngine::m_TypeInfo));

#endif