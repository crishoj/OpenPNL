/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPearlInferenceEngine.cpp                                 //
//                                                                         //
//  Purpose:   CPearlInferenceEngine class member functions implementation //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//pgmPearlInfEngine.cpp
#include "pnlConfig.hpp"
#include "pnlSpecPearlInferenceEngine.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "pnlMNet.hpp"
#include <float.h>
#include "pnlException.hpp"

PNL_USING

/////////////////////////////////////////////////////////////////////////////

CSpecPearlInfEngine* CSpecPearlInfEngine::Create(const CStaticGraphicalModel* pGrModel)
{
    PNL_CHECK_IS_NULL_POINTER(pGrModel);

    if( !IsInputModelValid(pGrModel) )
    {
        PNL_THROW( CInconsistentType, " input model is invalid " );
    }

    CSpecPearlInfEngine* pPearlInfEng = new CSpecPearlInfEngine(pGrModel);
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pPearlInfEng);
    
    return pPearlInfEng;
}
/////////////////////////////////////////////////////////////////////////////

/*void CSpecPearlInfEngine::Release(CSpecPearlInfEngine** PearlInfEngine)
{
    delete *PearlInfEngine;
    
    *PearlInfEngine = NULL;
}*/
/////////////////////////////////////////////////////////////////////////////

bool CSpecPearlInfEngine::IsInputModelValid(const CStaticGraphicalModel* pGrModel)
{
    EModelTypes modelType = pGrModel->GetModelType();
    
    //if(  modelType == mtBNet  ) 
    //{
    //    return true;
    //}
    if( modelType == mtMRF2 )   
    {
        return true;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////

CSpecPearlInfEngine::
CSpecPearlInfEngine(const CStaticGraphicalModel* pGrModel)
                : CInfEngine( itPearl, pGrModel ),
                m_modelType( pGrModel->GetModelType() ),
                m_maxNumberOfIterations(0),
                m_IterationCounter(0),
                m_numOfNdsInModel(m_pGraphicalModel->GetNumberOfNodes()),
                m_pModelDomain(m_pGraphicalModel->GetModelDomain()),
                m_pModelGraph(m_pGraphicalModel->GetGraph()),
                m_selfMessages(m_numOfNdsInModel),
                m_beliefs(2),
                m_curMessages(2),
                m_nbrDistributions(m_numOfNdsInModel),
                m_familyDistributions(m_numOfNdsInModel),
                m_tolerance(1e-7f),
                m_bDense(1)
{
    m_beliefs[0].resize( m_numOfNdsInModel );
    m_beliefs[1].resize( m_numOfNdsInModel );
    m_curMessages[0].resize(m_numOfNdsInModel);
    m_curMessages[1].resize(m_numOfNdsInModel);
    CreateMessagesStorage();
}
/////////////////////////////////////////////////////////////////////////////

CSpecPearlInfEngine::~CSpecPearlInfEngine()
{
    ClearMessagesStorage();
}
/////////////////////////////////////////////////////////////////////////////

void CSpecPearlInfEngine::EnterEvidence( const CEvidence* evidence,
                                     int maximize, int sumOnMixtureNode )
{
    PNL_CHECK_IS_NULL_POINTER(evidence);

    m_bMaximize = maximize ? 1 : 0;

    // clear all the previous data
    ClearMessagesStorage();
    
    // init new data
    InitEngine(evidence);

    // initialize messages
    InitMessages(evidence);

    // start inference
    ParallelProtocol();
}
/////////////////////////////////////////////////////////////////////////////

void CSpecPearlInfEngine::MarginalNodes( const int* query, int querySize,
                                     int notExpandJPD )
{
    if( notExpandJPD == 1 )
    {
        PNL_THROW( CInconsistentType, "pearl inference work with expanded distributions only" );
    }

    PNL_CHECK_LEFT_BORDER(querySize, 1);
    
    if( m_pQueryJPD )
    {
        delete m_pQueryJPD;
    }

    if( m_pEvidenceMPE )
    {
        delete m_pEvidenceMPE;
    }
    
    if( querySize == 1 )
    {
        if( m_bMaximize )
        {
            //compute MPE
            m_pEvidenceMPE = m_beliefs[m_curState][query[0]]->GetMPE();
        }
        else
        {
            // get marginal for one node - cretae parameter on existing data - m_beliefs[query[0]];
            m_pQueryJPD = m_beliefs[m_curState][query[0]]->GetNormalized();
        }
    }
    else
    {
        int numParams;
        CFactor ** params; 
        m_pGraphicalModel->GetFactors( querySize, query, &numParams ,&params );
        if ( !numParams )
        {
            PNL_THROW( CBadArg, "only members of one family can be in query instead of one node" ) 
        }
        if( numParams != 1 )
        {
            PNL_THROW( CBadArg, "add more nodes to specify which of jpd you want to know")
        }
        int i;
        //get informatiom from parametr on these nodes to crate new parameter
        //with updated Data
        CPotential* allPot;
        if( m_modelType == mtMRF2 )
        {
            //just multiply and marginalize
            allPot = static_cast<CPotential*>(params[0]->Clone());
        }
        else
        {
            //m_modelType == mtBNet
            //need to convert to potential withiut evidence and multiply
            //and marginalize after that
            allPot = static_cast<CCPD*>(params[0])->ConvertToPotential();
        }
        //get neighbors of last node in domain (child for CPD) 
        //to compute JPD for his family
        int domSize;
        const int* dom;
        params[0]->GetDomain(&domSize, &dom);
        //start multiply to add information after inference
        for( i = 0; i < domSize; i++ )
        {
            (*allPot) *= (*m_beliefs[m_curState][dom[i]]) ;
        }
        m_pQueryJPD = allPot->Marginalize( query, querySize, m_bMaximize );
        //fixme - can replace by normalize in self
        m_pQueryJPD->Normalize();
        if( m_bMaximize )
        {
            //compute MPE
            m_pEvidenceMPE = m_pQueryJPD->GetMPE();
            delete m_pQueryJPD;
            m_pQueryJPD = NULL;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////

void CSpecPearlInfEngine::InitEngine(const CEvidence* pEvidence)
{
    //determine distribution type for messages
//    intVector         obsNds;
//    pConstValueVector obsNdsVals;

    const int nObsNodes = pEvidence->GetNumberObsNodes();
    
    const int *ObsNodes = pEvidence->GetAllObsNodes();
    
    const int *ReallyObs = pEvidence->GetObsNodesFlags();
    
    intVector ReallyObsNodes;

    int i = 0;

    for ( ; i < nObsNodes; i++ )
    {
        if( ReallyObs[i] )
        {
            ReallyObsNodes.push_back(ObsNodes[i]);
        }
    }
    
    int NumReallyObs = ReallyObsNodes.size();

    pConstNodeTypeVector nodeTypes(m_numOfNdsInModel);

    for ( i = 0; i < m_numOfNdsInModel; i++ )
    {
        nodeTypes[i] = m_pGraphicalModel->GetNodeType(i);
    }

    m_modelDt = pnlDetermineDistributionType( m_numOfNdsInModel, NumReallyObs,
                                              &ReallyObsNodes.front(), &nodeTypes.front() );
    

    EDistributionType dtWithoutEv = pnlDetermineDistributionType( m_numOfNdsInModel, 0,
                                                                  &ReallyObsNodes.front(), &nodeTypes.front() );

    switch (dtWithoutEv)
    {
        //create vector of connected nodes
        case dtTabular: case dtGaussian:
        {
            m_connNodes = intVector(m_numOfNdsInModel);

            for( i = 0; i < m_numOfNdsInModel; ++i )
            {
                m_connNodes[i] = i;
            }

            break;
        }
        case dtCondGaussian:
        {	
            int loc;
            
            for( i = 0; i < m_numOfNdsInModel; i++ )
            {
                loc = std::find( ReallyObsNodes.begin(), ReallyObsNodes.end(),
                    i ) - ReallyObsNodes.begin();
            
                if(( loc < ReallyObsNodes.size() )&&
                    ( nodeTypes[ReallyObsNodes[loc]]->IsDiscrete() ))
                {
                    m_connNodes.push_back(i);
                }
            }
            
            break;
        }
        default:
        {
            PNL_THROW( CInconsistentType, "only for fully tabular or fully gaussian models" )
                break;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////

void CSpecPearlInfEngine::InitMessages( const CEvidence* evidence )
{
    int i,j;

    const int  nObsNodes = evidence->GetNumberObsNodes();

    const int* ObsNodes = evidence->GetAllObsNodes();
    
    const int* ReallyObs = evidence->GetObsNodesFlags();
    
    m_areReallyObserved = intVector( m_numOfNdsInModel, 0) ;
    
    for ( i =0; i < nObsNodes; i++ )
    {
        if( ReallyObs[i] )
        {
            m_areReallyObserved[ObsNodes[i]] = 1;
        }
    }
    
    //int numConnNodes = m_connNodes.size();
    intVector obsNds = intVector( 1, 0 );
    int                  numOfNeighb;
    const int*           neighbors;
    const ENeighborType* orientation;
    intVector dom;
    dom.assign(1,0);
    
    //init messages to nbrs
    if( m_modelDt == dtTabular )
    {
        for( i = 0; i < m_numOfNdsInModel; i++ )
        {
            dom[0] = i;
            if( m_areReallyObserved[i] )
            {
                const Value* valP = evidence->GetValue(i);
                int val = valP->GetInt();
                int nodeSize = m_pModelDomain->GetVariableType(i)->GetNodeSize();
                floatVector prob;
                prob.assign( nodeSize, 0 );
                prob[val] = 1.0f;
                
                m_selfMessages[i] = CTabularPotential::Create( dom,
                    m_pModelDomain, &prob.front() );
            }
            else
            {
                m_selfMessages[i] = 
                    CTabularPotential::CreateUnitFunctionDistribution( dom,
                    m_pModelDomain, m_bDense );   
            }
            m_beliefs[0][i] = static_cast<CPotential*>(m_selfMessages[i]->Clone());
            m_beliefs[1][i] = static_cast<CPotential*>(m_selfMessages[i]->Clone());


            m_pModelGraph->GetNeighbors( i,  &numOfNeighb, &neighbors, &orientation );

            for( j = 0; j < numOfNeighb; j++ )
            {
                m_curMessages[0][i][j] = 
                    CTabularPotential::CreateUnitFunctionDistribution( dom,
                    m_pModelDomain, m_bDense );
                m_curMessages[1][i][j] = static_cast<CPotential*>(m_curMessages[0][i][j]->Clone());
            }
        }
    }
    else //m_modelDt == dtGaussian
    {
        for( i = 0; i < m_numOfNdsInModel; i++ )
        {
            dom[0] = i;
            if( m_areReallyObserved[i] )
            {
                //in canonical form
                const Value* valP = evidence->GetValue(i);
                int nodeSize = m_pModelDomain->GetVariableType(i)->GetNodeSize();
                floatVector mean;
                mean.resize(nodeSize);
                for( int j = 0; j < nodeSize; j++ )
                {
                    mean[i] = (valP[i]).GetFlt();
                }
                m_selfMessages[i] = CGaussianPotential::CreateDeltaFunction( dom,
                    m_pModelDomain, mean, 0 );
            }
            else
            {
                //in canonical form
                m_selfMessages[i] = 
                    CGaussianPotential::CreateUnitFunctionDistribution( dom,
                    m_pModelDomain, 1 );   
            }
            m_beliefs[0][i] = static_cast<CPotential*>(m_selfMessages[i]->Clone());
            m_beliefs[1][i] = static_cast<CPotential*>(m_selfMessages[i]->Clone());
            
            m_pModelGraph->GetNeighbors( i,  &numOfNeighb, &neighbors, &orientation );
            
            for( j = 0; j < numOfNeighb; j++ )
            {
                if( orientation[j] == ntParent )//message from parent (pi) - it has another type
                {
                    //in moment form for Gaussian
                    m_curMessages[0][i][j] = 
                        CGaussianPotential::CreateUnitFunctionDistribution( dom, 
                        m_pModelDomain, 0 );
                }
                else
                {
                    //in Canonical form for Gaussian
                    m_curMessages[0][i][j] = CGaussianPotential::CreateUnitFunctionDistribution( dom,
                        m_pModelDomain, 1 );
                }
                m_curMessages[1][i][j] = static_cast<CPotential*>(m_curMessages[0][i][j]->Clone());
            }
        }
    }
    //init distribution functions on families (if model type is BNet)
    if( m_modelType == mtBNet )
    {
        for( i = 0; i < m_numOfNdsInModel; i++ )
        {
            m_familyDistributions[i] = 
                m_pGraphicalModel->GetFactor(i)->GetDistribFun()->Clone();   
        }
    }
    //init potentials on edges (if model type is MNet)
    if( m_modelType == mtMRF2 )
    {
        int numOfParams; CFactor**params;
        for( i = 0; i < m_numOfNdsInModel; i++ )
        {
            m_pGraphicalModel->GetFactors(1, &i, &numOfParams, &params );
            for( j = 0; j < numOfParams; j++ )
            {
                intVector domain;
                params[j]->GetDomain(&domain);
                //here should be only two nodes in domain - the model is MRF2
                if( domain.size() != 2 )
                {
                    PNL_THROW( CInconsistentSize, "all factors shold be of size 2" );
                }
                m_nbrDistributions[domain[0]][domain[1]] = 
                    static_cast<CPotential*>(params[j]->Clone());
                m_nbrDistributions[domain[1]][domain[0]] = 
                    static_cast<CPotential*>(params[j]->Clone());
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////

void CSpecPearlInfEngine::ComputeMessage( int destination, int source, 
                                         int orientFlag, fgMessage& mes ) const
{
    switch( orientFlag )
    {
    case 0://destination is the child of source - 
        //source send this message to destination as message to child (pi)(X = source)
        //result message must be in moment form (the same as pi)
        {
            /*message piProduct = */ComputeProductPi( source, mes );
            /*message lambdaProduct = */ProductLambdaMsgs( source, mes, destination );
            //int dom = 0;//nbrs[source];
            //(*piProduct) *= (*lambdaProduct);
            /*message allProd = */mes->Normalize();
            //delete piProduct;
            //delete lambdaProduct;					
            return;// allProd;
        }
    case 1://destination is the parent of source - 
        // source send message to destination as message to parent (lambda) (X = source)
        //result message must be in canonica form (the same as lambda)
        {
            //compute lambda
            /*fgMessage lambdaProduct = */ProductLambdaMsgs( source, mes );
            //it is the data of length nodeSize(destination), domain : [source]
            /*fgMessage res = */ComputeProductLambda( source, mes, mes,
                destination );
            mes->Normalize();
            //fgMessage resNorm = res->_NormalizeData();
            //delete lambdaProduct;
            //delete res;
            //return resNorm;			
            return;// res;
        }
    case 2://destination & source are the neighbors
        {
            //we need to compute message which node 'destination' recieve 
            //from node 'source'
            int i;
            int numNeighb;
            const int *nbrs;
            const ENeighborType *orient;
            m_pGraphicalModel->GetGraph()->GetNeighbors( source, &numNeighb, 
                &nbrs, &orient );
            //multiply all messages
            fgMessage commonPot = m_nbrDistributions[source][destination];
            (*commonPot) *= (*m_selfMessages[source]);
            for ( i = 0; i < numNeighb; i++ )
            {
                if( nbrs[i] != destination )
                {
                    (*commonPot) *= (*m_curMessages[m_curState][source][i]) ;
                }
            }
            //mes - is message for destination node
            //we can compute the positions to marginalize and give it to the marginalization
            const int* corrPos = NULL;
            mes->MarginalizeInPlace( commonPot, corrPos, m_bMaximize );
            //set previous data to the potential for the next multiplication
            mes->Normalize();
            (*m_nbrDistributions[source][destination] ) = 
                (*m_nbrDistributions[destination][source]);
            return;
        }
    default:
        {
            PNL_THROW( CInvalidOperation, "no such type of orientation" )
                //return NULL;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////
                                         
void CSpecPearlInfEngine::ComputeProductPi(int nodeNumber, fgMessage& resMes,
                                       int except, int multFlag) const
{
    int i;
    CGraph *theGraph = m_pGraphicalModel->GetGraph();
    
    int                    numNeighb;
    const int              *neighbors;
    const ENeighborType *orientations;
    
    theGraph->GetNeighbors( nodeNumber, &numNeighb, &neighbors,
        &orientations );
    
    if( orientations[0] == ntNeighbor )
    {
        if( !multFlag )
        {
            resMes->MakeUnitFunction();
        }
        return;
    }
    //getting parameter from the model
    int nodes = nodeNumber;
    int numOfParams;
    CFactor ** param;
    m_pGraphicalModel->GetFactors(1, &nodes, &numOfParams, &param );
    
    if( !numOfParams )
    {
        PNL_THROW( CInvalidOperation, "no parameter" )
    }
    
    //we want to find parameter in which nodeNumber is a child - last in domain
    const CFactor *parameter = param[0];
    
    //to check last node in domain
    int bigDomSize;
    const int *BigDomain; 
    
    //flag to check - if there isn't such domain
    int isSuchDom = 0;
    
    for ( i = 0; i < numOfParams; i++ )
    {
        param[i]->GetDomain( &bigDomSize, &BigDomain );
        if( BigDomain[bigDomSize-1] == nodeNumber )
        {
            parameter = param[i];
            isSuchDom = 1;
            break;
        }
    }
    
    if( !isSuchDom )
    {
        //we need to produce message without any interesting information - only ones
        if( !multFlag )
        {
            resMes->MakeUnitFunction();
        }
        return;
    }
    //getting data from parameter
    /*const CPotential* paramData = static_cast<const CPotential*>(parameter);//->_GetDistribFun();
    
    //getting domain for marginalize and multiply
    
    parameter->GetDomain( &bigDomSize, &BigDomain );
    
    //we need to use serial numbers
    intVector domNumbers( bigDomSize );
    for ( i = 0; i < bigDomSize; i++ )
    {
        domNumbers[i] = i;
    }
    //we need to find all parents, collect their messages
    // put the pointers to them into array, call CPD_to_pi()
    int location;
    intVector parentIndices;
    //we create vector of messages in the same order as in domain - they are first in domain
    std::vector<fgMessage> parentMessages = std::vector<fgMessage>(bigDomSize - 1);
    int keepNode = -1;
    for( i = 0; i < numNeighb; i++ )
    {
        location = std::find( BigDomain, BigDomain + bigDomSize,
            neighbors[i] ) - BigDomain;
        if( ( orientations[i] == ntParent ) && ( location < bigDomSize ) )
        {
            parentMessages[location] = m_curMessages[m_curState][nodeNumber][i];
            if(  neighbors[i] != except  )
            {
                parentIndices.push_back(location);
            }
            if (neighbors[i] == except)
            {
                keepNode =  location;
            }
        }
    }
    //fgMessage resMes1 = paramData->GetDistribFun()->CPD_to_pi(parentMessages.begin(), 
    //    parentIndices.begin(), parentIndices.size(), keepNode, m_bMaximize);
    //return resMes;*/
}
/////////////////////////////////////////////////////////////////////////////
void CSpecPearlInfEngine::ComputeProductLambda( int nodeNumber, fgMessage lambda,
                                               fgMessage& resMes,
                                               int except ) const
{
    int i;
    CGraph *theGraph = m_pGraphicalModel->GetGraph();
    
    int                    numNeighb;
    const int              *neighbors;
    const ENeighborType *orientations;
    
    theGraph->GetNeighbors( nodeNumber, &numNeighb, &neighbors, &orientations );
    
    if( orientations[0] == ntNeighbor )
    {
        return;// InitMessage( nodeNumber, ( m_pGraphicalModel->GetNodeType(nodeNumber) ) );
    }
    //getting parameter from the model
    
    int nodes = nodeNumber;
    int numOfParams;
    CFactor ** param;
    m_pGraphicalModel->GetFactors(1, &nodes, &numOfParams, &param );
    
    if( !numOfParams )
    {
        PNL_THROW( CInvalidOperation, "no parameter" )
    }
    
    //we want to find parameter in which nodeNumber is a child - last in domain
    const CFactor *parameter = param[0];
    
    //to check last node in domain
    const int *BigDomain; int bigDomSize;
    
    //flag to check - if there isn't such domain
    int bSuchDom = 0;
    
    for ( i = 0; i < numOfParams; i++ )
    {
        param[i]->GetDomain( &bigDomSize, &BigDomain );
        if( BigDomain[bigDomSize-1] == nodeNumber )
        {
            parameter = param[i];
            bSuchDom = 1;
            break;
        }
    }
    
    if( !bSuchDom )
    {
        //we need to produce message without any interesting information - only ones
        //fgMessage allData = InitMessage( nodeNumber, m_pGraphicalModel->GetNodeType( nodeNumber ) );
        return;// allData;
    }
    //getting data from parameter
    /*fgMessage paramData = static_cast<CPotential*>(parameter->Clone());//->_GetDistribFun();
    
    //getting domain for marginalize and multiply
    
    parameter->GetDomain( &bigDomSize, &BigDomain );
    
    //we need to use serial numbers
    intVector domNumbers( bigDomSize );
    for ( i = 0; i < bigDomSize; i++ )
    {
        domNumbers[i] = i;
    }
    //we need to find all parents, collect their messages
    // put the pointers to them into array, call CPD_to_pi()
    int location;
    intVector parentIndices;
    //we create vector of messages in the same order as in domain - they are first in domain
    std::vector<fgMessage> parentMessages = std::vector<fgMessage>(bigDomSize - 1);
    int keepNode = -1;
    for( i = 0; i < numNeighb; i++ )
    {
        location = std::find( BigDomain, BigDomain + bigDomSize,
            neighbors[i] ) - BigDomain;
        if( ( orientations[i] == ntParent ) && ( location < bigDomSize ) )
        {
            parentMessages[location] = m_curMessages[m_curState][nodeNumber][i];
            if(  neighbors[i] != except  )
            {
                parentIndices.push_back(location);
            }
            if (neighbors[i] == except)
            {
                keepNode =  location;
            }
        }
    }
    //fgMessage resMes1 = paramData->_GetDistribFun()->_CPD_to_lambda(lambda, parentMessages.begin(), 
    //    parentIndices.begin(), parentIndices.size(), keepNode, m_bMaximize);
    //return resMes1;*/
}
/////////////////////////////////////////////////////////////////////////////

void CSpecPearlInfEngine::ProductLambdaMsgs( int nodeNumber, fgMessage& resMes,
                                           int except) const
{
    //resMes is on the NodeNumber domain
    int i;
    
    int                  numNeighb;
    const int*           nbrs;
    const ENeighborType* orient;
    
    m_pGraphicalModel->GetGraph()->GetNeighbors( nodeNumber, &numNeighb,
        &nbrs, &orient );
    

    (*resMes) = (*m_selfMessages[nodeNumber]);

    for( i = 0 ; i <numNeighb; i++)
    {
        if( ( orient[i] != ntParent ) && ( nbrs[i] != except) )
        {
            (*resMes) *= (*m_curMessages[m_curState][nodeNumber][i]);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////

void CSpecPearlInfEngine::ComputeBelief( int nodeNumber )
{
    //we compute it after all message passing and m_curState has been changed
    //all references to information is to m_curState
    //the result is placed to the m_beliefs[m_curState][nodeNumber]
    
    ProductLambdaMsgs(nodeNumber, m_beliefs[m_curState][nodeNumber]);
    //need to multiply by pi for CPD

    ComputeProductPi(nodeNumber, m_beliefs[m_curState][nodeNumber], -1, 1 );
    
    //we delete belief in m_oldBeliefs - we copy pointers!
    //delete m_beliefs[nodeNumber];
    m_beliefs[m_curState][nodeNumber]->Normalize();
}
/////////////////////////////////////////////////////////////////////////////

void CSpecPearlInfEngine::ParallelProtocol()
{
    if( m_maxNumberOfIterations == 0 )
    {
        SetMaxNumberOfIterations(m_numOfNdsInModel);
    }
    int i, j;
    int converged = 0;
    int changed = 0;
    int iter = 0;
    const CGraph *pGraph = m_pGraphicalModel->GetGraph();
    int nNodes = m_connNodes.size();

    //set current state
    m_curState = 0;
    m_nextState = 1;
    while ((!converged)&&( iter<m_maxNumberOfIterations ))
    {
        //work with new data
        int                    numOfNeighb;
        const int              *neighbors;
        const ENeighborType *orientation;
        
        for( i = 0; i < nNodes; i++ )
        {
            pGraph->GetNeighbors( m_connNodes[i], &numOfNeighb, 
                &neighbors, &orientation );
            for( j = 0; j < numOfNeighb; j++)
            {
                //delete m_newMessages[m_connNodes[i]][j];
                ComputeMessage( m_connNodes[i], neighbors[j], orientation[j],
                    m_curMessages[m_nextState][m_connNodes[i]][j] );
            }
        }
        //need to replace all old messages with new
        m_curState = m_nextState;
        m_nextState = 1 - m_curState;
        //compute beliefs
        changed = 0;
        for( i = 0; i < nNodes; i++ )
        {
            if( !m_areReallyObserved[m_connNodes[i]])
            {
                ComputeBelief( m_connNodes[i] );
                changed += !m_beliefs[m_curState][m_connNodes[i]]->IsFactorsDistribFunEqual(
                    m_beliefs[m_nextState][m_connNodes[i]], m_tolerance);
            }
        }
        converged = !(changed);
        iter++;
    }//while ((!converged)&&(iter<m_numberOfIterations))
    m_IterationCounter = iter;
    //need to set both states to valid state
    m_curState = m_nextState;
}
/////////////////////////////////////////////////////////////////////////////

void CSpecPearlInfEngine::TreeProtocol()
{
}
/////////////////////////////////////////////////////////////////////////////

#ifdef PNL_RTTI
const CPNLType CSpecPearlInfEngine::m_TypeInfo = CPNLType("CSpecPearlInfEngine", &(CInfEngine::m_TypeInfo));

#endif
/////////////////////////////////////////////////////////////////////////////