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
#include "pnlPearlInferenceEngine.hpp"
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

CPearlInfEngine* CPearlInfEngine::
Create(const CStaticGraphicalModel* pGrModel)
{
    PNL_CHECK_IS_NULL_POINTER(pGrModel);

    if( !IsInputModelValid(pGrModel) )
    {
        PNL_THROW( CInconsistentType, " input model is invalid " );
    }

    CPearlInfEngine* pPearlInfEng = new CPearlInfEngine(pGrModel);
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pPearlInfEng);
    
    return pPearlInfEng;
}
/////////////////////////////////////////////////////////////////////////////

/*void CPearlInfEngine::Release(CPearlInfEngine** PearlInfEngine)
{
    delete *PearlInfEngine;
    
    *PearlInfEngine = NULL;
}*/
/////////////////////////////////////////////////////////////////////////////

bool CPearlInfEngine::IsInputModelValid(const CStaticGraphicalModel* pGrModel)
{
    EModelTypes modelType = pGrModel->GetModelType();
    
    if( ( modelType != mtBNet ) && ( modelType != mtMRF2 ) )
    {
        return false;
    }
    
    return true;
}
/////////////////////////////////////////////////////////////////////////////

CPearlInfEngine::
CPearlInfEngine(const CStaticGraphicalModel* pGrModel)
                : CInfEngine( itPearl, pGrModel ),
                m_maxNumberOfIterations(0),
                m_IterationCounter(0),
                m_numOfNdsInModel(m_pGraphicalModel->GetNumberOfNodes()),
                m_pModelDomain(m_pGraphicalModel->GetModelDomain()),
                m_pModelGraph(m_pGraphicalModel->GetGraph()),
                m_beliefs(m_numOfNdsInModel),
                m_selfMessages(m_numOfNdsInModel),
                m_messagesFromNeighbors(m_numOfNdsInModel),
                m_tolerance(1e-6f)
{
    CreateMessagesStorage();
}
/////////////////////////////////////////////////////////////////////////////

CPearlInfEngine::~CPearlInfEngine()
{
    ClearMessagesStorage();
}
/////////////////////////////////////////////////////////////////////////////

void CPearlInfEngine::EnterEvidence( const CEvidence* evidence,
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

void CPearlInfEngine::MarginalNodes( const int* query, int querySize,
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
        m_pQueryJPD = NULL;
    }

    if( m_pEvidenceMPE )
    {
        delete m_pEvidenceMPE;
        m_pEvidenceMPE = NULL;
    }
    
    CModelDomain* pMD = m_pGraphicalModel->GetModelDomain();
    if( querySize == 1 )
    {
        if( m_bMaximize )
        {
            //compute MPE
            CNodeValues * values = m_beliefs[query[0]]->GetMPE();
            CEvidence *evid = CEvidence::Create( values, querySize, query,
                m_pGraphicalModel->GetModelDomain() );
            m_pEvidenceMPE = evid;
            delete values;
        }
        else
        {
            // get marginal for one node - cretae factor on existing data - m_beliefs[query[0]];
            switch (m_modelDt)
            {
                case dtTabular:
                {
                    m_pQueryJPD = CTabularPotential::Create( query, querySize,
                        pMD);
                    break;
                }
                case dtGaussian:
                {
                    m_pQueryJPD = CGaussianPotential::Create( query, querySize,
                        pMD);
                    break;
                }
                default:
                {
                    PNL_THROW( CNotImplemented, "only tabular & Gaussian now");
                }
            }
            //we need to add our beliefs here
            if( m_beliefs[query[0]]->GetDistributionType() == dtGaussian )
            {
                static_cast<CGaussianDistribFun*>( m_beliefs[query[0]] )->UpdateMomentForm();
            }
            CDistribFun* pNormalized =  m_beliefs[query[0]]->GetNormalized();
            m_pQueryJPD->SetDistribFun(pNormalized);
            delete pNormalized;
        }
    }
    else
    {
        int numParams;
        CFactor ** params; 
        m_pGraphicalModel->GetFactors( querySize, query,
            &numParams ,&params );
        if ( !numParams )
        {
            PNL_THROW( CBadArg, "only members of one family can be in query instead of one node" ) 
        }
        /*if( numParams != 1 )
        {
            PNL_THROW( CBadArg, "add more nodes to specify which of jpd you want to know")
        }*/
        int i;
        //get informatiom from parametr on these nodes to crate new parameter
        //with updated Data
        EDistributionType dt = params[0]->GetDistributionType();
        message allProduct;
		CPotential *potParam = NULL;
        switch (dt)
        {
        case dtTabular:
            {
				allProduct = params[0]->GetDistribFun()->ConvertCPDDistribFunToPot();
                break;
            }
        case dtGaussian:
            {
				potParam = static_cast<CCPD*>(params[0])->ConvertWithEvidenceToPotential(m_pEvidence);
				allProduct = potParam->GetDistribFun()->Clone();
                break;
            }
        default:
            {
                PNL_THROW( CNotImplemented, "only tabular & Gaussian now");
            }
        }
        int domLength;
        const int *domain;
        params[0]->GetDomain( &domLength, &domain );
        CGraph *graph = m_pGraphicalModel->GetGraph();
        int lastNodeInDomain = domain[domLength-1];
        //get neighbors of last node in domain (child for CPD) 
        //to compute JPD for his family
        const int *nbrs;
        const ENeighborType *orient;
        int numNbrs;
        graph->GetNeighbors( lastNodeInDomain, &numNbrs, &nbrs, &orient );
        int smallDom;
        int *domPos = new int [domLength];
        PNL_CHECK_IF_MEMORY_ALLOCATED( domPos );
        for( i = 0; i < domLength; i++ )
        {
            domPos[i] = i;
        }
        int location;
        //start multiply to add information after inference
        for( i = 0; i < numNbrs; i++ )
        {
            location = std::find(domain, domain + domLength, nbrs[i]) - domain;
            if( location >= domLength )
            {
                break;
            }
            smallDom = location;
            allProduct->MultiplyInSelfData( domPos, &smallDom, 
                m_messagesFromNeighbors[domain[domLength-1]][i] );
        }
        //add self-information
        smallDom = domLength - 1;
        allProduct->MultiplyInSelfData( domPos, &smallDom, 
            m_selfMessages[lastNodeInDomain]);
        CDistribFun* normalizedData = allProduct->GetNormalized();
        if( normalizedData->GetDistributionType() == dtGaussian )
        {
            static_cast<CGaussianDistribFun*>(normalizedData)->UpdateMomentForm();
        }
        //create factor on this nodes
        CPotential* pQueryJPD = NULL;
        switch (dt)
        {
        case dtTabular:
            {
                pQueryJPD = CTabularPotential::Create( domain, domLength, pMD );
                break;
            }
        case dtGaussian:
            {
                pQueryJPD = CGaussianPotential::Create( domain, domLength, pMD );
                break;
            }
        default:
            {
                PNL_THROW( CNotImplemented, "only tabular & Gaussian now");
            }
        }
        // we need to add data to this Factor
        pQueryJPD->SetDistribFun(normalizedData);
        m_pQueryJPD = pQueryJPD->Marginalize( query, querySize, m_bMaximize );
        delete pQueryJPD;
        if( m_bMaximize )
        {
            //compute MPE
            CNodeValues * values = m_pQueryJPD->GetDistribFun()->GetMPE();
            CEvidence *evid = CEvidence::Create( values, querySize, query,
                m_pGraphicalModel->GetModelDomain() );
            m_pEvidenceMPE = evid;
            delete values;
            delete m_pQueryJPD;
            m_pQueryJPD = NULL;
        }
        delete normalizedData;
		delete potParam;
        delete allProduct;
        delete []domPos;
    }
}
/////////////////////////////////////////////////////////////////////////////

void CPearlInfEngine::InitEngine(const CEvidence* pEvidence)
{
    //determine distribution type for messages
//    intVector         obsNds;
//    pConstValueVector obsNdsVals;
    m_pEvidence = pEvidence;

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

    m_modelDt = pnlDetermineDistributionType( m_numOfNdsInModel, 
        NumReallyObs, &ReallyObsNodes.front(), &nodeTypes.front() );
    if( m_modelDt == dtScalar )
    {
        PNL_THROW( CInconsistentType, "the model is fully observed" );
    }
    
    //initialize beliefs
    for( i=0; i < m_numOfNdsInModel; i++)
    {
        message temp = InitMessage( i, nodeTypes[i] );
        m_beliefs[i] = temp->GetNormalized();
        delete temp;
    }

    EDistributionType dtWithoutEv = pnlDetermineDistributionType(
        m_numOfNdsInModel, 0, &ReallyObsNodes.front(), &nodeTypes.front() );

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

void CPearlInfEngine::InitMessages( const CEvidence* evidence )
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
    
    pConstNodeTypeVector nodeTypes(m_numOfNdsInModel);
    
    for( i = 0; i < m_numOfNdsInModel; i++ )
    {
        nodeTypes[i] = m_pGraphicalModel->GetNodeType(i);
    }
    
    int                  numOfNeighb;
    const int*           neighbors;
    const ENeighborType* orientation;
    
    int mesSize = m_messagesFromNeighbors.size();
    
    for( i = 0; i < mesSize/*m_numOfNdsInModel*/; i++)
    {
        m_pModelGraph->GetNeighbors( i,  &numOfNeighb, &neighbors, &orientation );
        
        /*initialize messages from neighbors*/
        for( j = 0; j < numOfNeighb; ++j )
        {
            if( orientation[j] == ntParent )//message from parent (pi) - it has another type
            {
                //in moment form for Gaussian
                m_messagesFromNeighbors[i][j] = InitMessage( i,
                    nodeTypes[neighbors[j]], 1 );
            }
            else
            {
                //in Canonical form for Gaussian
                m_messagesFromNeighbors[i][j] = InitMessage( i,
                    nodeTypes[i], 0 );
            }
        }
        /*initialize messages from self*/
        if( !m_areReallyObserved[i] )
        {
            m_selfMessages[i] = InitMessage( i, nodeTypes[i] );
        }
        else
        {
            const Value *vall = evidence->GetValue(i);
            if( nodeTypes[i]->IsDiscrete() )
            {
                int val = vall->GetInt();
                m_selfMessages[i] = InitMessageWithEvidence( i, nodeTypes[i], val);
                if( m_beliefs[i] )
                {
                    delete m_beliefs[i];
                }
                m_beliefs[i] = InitMessageWithEvidence( i, nodeTypes[i], val);
            }
            else
            {
                int size = nodeTypes[i]->GetNodeSize();
                floatVector val;
                val.resize(size);
                for( j = 0; j < size; j++ )
                {
                    val[j] = vall[j].GetFlt();
                }
                if( m_beliefs[i] )
                {
                    delete m_beliefs[i];
                }
                m_beliefs[i] = InitMessageWithEvidence( i, nodeTypes[i], &val.front() );
                m_selfMessages[i] = InitMessageWithEvidence( i, nodeTypes[i], &val.front() );
                //fixme - we need to set beliefs
                //PNL_THROW(CNotImplemented, "gaussian")
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////

message CPearlInfEngine::
InitMessageWithEvidence( int node, const CNodeType* type, int val )
{
    message retMes = NULL;
    
    switch ( m_modelDt )
    {
        case dtTabular:
        {
            int size = type->GetNodeSize();
            
            if( val >= size )
            {
                PNL_THROW( CInconsistentSize, "value of discrete node is more than range" )
                    return NULL;
            }
            
            floatVector dataVec = floatVector( size, 0.0f );
            
            dataVec[val] = 1.0f;
            
            retMes = CTabularDistribFun::Create( 1, &type, &dataVec.front());
            
            break;
        }
        default:
        {
            PNL_THROW(CBadConst, "no distribution type")
                //evidence for tabular is only int!
        }
    }

    return retMes;
}
/////////////////////////////////////////////////////////////////////////////

message CPearlInfEngine::
InitMessageWithEvidence( int node, const CNodeType* type, float* val )
{
    message retMes = NULL;

    switch( m_modelDt )
    {
        case  dtGaussian:
        {
            //we create all messages in moment form, send and multiply all messages in this form!
            //all messages except observed send uniform distribution in canonical form
            //observed nodes send delta function in moment form
            //beliefs update moment form!
            retMes = CGaussianDistribFun::CreateDeltaDistribution( 1, &type,
                val, 0) ;
            break;			
        }
        default:
        {
            PNL_THROW(CBadConst, "no distribution type")
                //evidence for Gaussian is only float!
        }
    }
    
    return retMes;
}

/////////////////////////////////////////////////////////////////////////////

message CPearlInfEngine::ComputeMessage( int destination, int source, 
                                         int orientFlag ) const
{
    switch( orientFlag )
    {
    case 0://destination is the child of source - 
        //source send this message to destination as message to child (pi)(X = source)
        //result message must be in moment form (the same as pi)
        {
            message piProduct = ComputeProductPi( source );
            message lambdaProduct = ProductLambdaMsgs( source, destination );
            int dom = 0;//nbrs[source];
            piProduct->MultiplyInSelfData(&dom, &dom, lambdaProduct);
            message allProd = piProduct->GetNormalized();
            delete piProduct;
            delete lambdaProduct;					
            return allProd;
        }
    case 1://destination is the parent of source - 
        // source send message to destination as message to parent (lambda) (X = source)
        //result message must be in canonica form (the same as lambda)
        {
            //compute lambda
            message lambdaProduct = ProductLambdaMsgs( source );
            //it is the data of length nodeSize(destination), domain : [source]
            message res = ComputeProductLambda( source, lambdaProduct,
                destination );
            message resNorm = res->GetNormalized();
            delete lambdaProduct;
            delete res;
            return resNorm;			
            //return res;
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
            int domain = 0;//we need to find position of source in this domain
            //const CNodeType* nt = m_pGraphicalModel->GetNodeType( destination );
            //we need to multiply on self message for node source
            message productNbrs = m_selfMessages[source]->Clone();
            //multiply all messages
            for ( i = 0; i < numNeighb; i++ )
            {
                if( nbrs[i] != destination )
                {
                    productNbrs->MultiplyInSelfData( &domain, &domain,
                        m_messagesFromNeighbors[source][i] );
                }
            }
            // multiply product of nbrs on matrix of parameter
            int nodes[] = { source, destination };
            int numOfParams; CFactor**params;
            m_pGraphicalModel->GetFactors(2, nodes, &numOfParams, &params );
            message paramMes = params[0]->GetDistribFun();
            const int *ParamDomain; int domSize;
            params[0]->GetDomain( &domSize, &ParamDomain );
            int MarginalizeIndex, MultiplyIndex ; 
            //we need to create array of positions in domain
            int *posDom = new int [domSize];
            PNL_CHECK_IF_MEMORY_ALLOCATED( posDom );
            for ( i = 0; i <domSize; i++ )
            {
                posDom[i] = i;
                if( ParamDomain[i] == destination )
                {
                    //we need to marginalize for to (to = dims of keep )
                    MarginalizeIndex = posDom[i];
                }
                else
                {
                    //we need to multiply for source 
                    MultiplyIndex = posDom[i];
                }
            }
            message mesCopy = paramMes->Clone();
            mesCopy->MultiplyInSelfData( posDom, &MultiplyIndex, productNbrs );
            //we need to marginalize (to = dims of keep)
            productNbrs->MarginalizeData( mesCopy, &MarginalizeIndex, 
                1, m_bMaximize );
            
            message resMessage = productNbrs->GetNormalized();
            delete productNbrs;
            delete mesCopy;
            delete []posDom;

            return resMessage;
        }
    default:
        {
            PNL_THROW( CInvalidOperation, "no such type of orientation" )
                return NULL;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////
                                         
message CPearlInfEngine::ComputeProductPi(int nodeNumber, int except) const
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
        return InitMessage( nodeNumber,
            ( m_pGraphicalModel->GetNodeType(nodeNumber) ) );
    }
    //getting factor from the model
    
    int nodes = nodeNumber;
    //int numOfParams;
    //CFactor ** param;
    //m_pGraphicalModel->GetFactors(1, &nodes, &numOfParams, &param );
    pFactorVector param;
	m_pGraphicalModel->GetFactors(1, &nodes, &param);
	int numOfParams = param.size();
    
    if( !numOfParams )
    {
        PNL_THROW( CInvalidOperation, "no parameter" )
    }
    
    //we want to find factor in which nodeNumber is a child - last in domain
    const CFactor *factor = param[0];
    
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
            factor = param[i];
            isSuchDom = 1;
            break;
        }
    }
    
    if( !isSuchDom )
    {
        //we need to produce message without any interesting information - only ones
        message allData = InitMessage( nodeNumber, m_pGraphicalModel->GetNodeType( nodeNumber ) );
        return allData;
    }
    //getting data from parameter
    message paramData = factor->GetDistribFun();
    
    //getting domain for marginalize and multiply
    
    factor->GetDomain( &bigDomSize, &BigDomain );
    
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
    std::vector<message> parentMessages = std::vector<message>(bigDomSize - 1);
    int keepNode = -1;
    for( i = 0; i < numNeighb; i++ )
    {
        location = std::find( BigDomain, BigDomain + bigDomSize,
            neighbors[i] ) - BigDomain;
        if( ( orientations[i] == ntParent ) && ( location < bigDomSize ) )
        {
            parentMessages[location] = m_messagesFromNeighbors[nodeNumber][i];
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
    message resMes = paramData->CPD_to_pi(&parentMessages.front(), 
        &parentIndices.front(), parentIndices.size(), keepNode, m_bMaximize);
    return resMes;
}
/////////////////////////////////////////////////////////////////////////////

message CPearlInfEngine::ComputeProductLambda( int nodeNumber, message lambda,
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
        return InitMessage( nodeNumber, ( m_pGraphicalModel->GetNodeType(nodeNumber) ) );
    }
    //getting factor from the model
    
    int nodes = nodeNumber;
    //int numOfParams;
    //CFactor ** param;
    //m_pGraphicalModel->GetFactors(1, &nodes, &numOfParams, &param );
    pFactorVector param;
	m_pGraphicalModel->GetFactors(1, &nodes, &param);
	int numOfParams = param.size();
    
    if( !numOfParams )
    {
        PNL_THROW( CInvalidOperation, "no parameter" )
    }
    
    //we want to find factor in which nodeNumber is a child - last in domain
    const CFactor *factor = param[0];
    
    //to check last node in domain
    const int *BigDomain; int bigDomSize;
    
    //flag to check - if there isn't such domain
    int bSuchDom = 0;
    
    for ( i = 0; i < numOfParams; i++ )
    {
        param[i]->GetDomain( &bigDomSize, &BigDomain );
        if( BigDomain[bigDomSize-1] == nodeNumber )
        {
            factor = param[i];
            bSuchDom = 1;
            break;
        }
    }
    
    if( !bSuchDom )
    {
        //we need to produce message without any interesting information - only ones
        message allData = InitMessage( nodeNumber, m_pGraphicalModel->GetNodeType( nodeNumber ) );
        return allData;
    }
    //getting data from parameter
    message paramData = factor->GetDistribFun();
    
    //getting domain for marginalize and multiply
    
    factor->GetDomain( &bigDomSize, &BigDomain );
    
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
    std::vector<message> parentMessages = std::vector<message>(bigDomSize - 1);
    int keepNode = -1;
    for( i = 0; i < numNeighb; i++ )
    {
        location = std::find( BigDomain, BigDomain + bigDomSize,
            neighbors[i] ) - BigDomain;
        if( ( orientations[i] == ntParent ) && ( location < bigDomSize ) )
        {
            parentMessages[location] = m_messagesFromNeighbors[nodeNumber][i];
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
    if( keepNode == -1 )
    {
        keepNode = -1;
    }
    message resMes = paramData->CPD_to_lambda(lambda, &parentMessages.front(), 
        &parentIndices.front(), parentIndices.size(), keepNode, m_bMaximize);
    return resMes;
}
/////////////////////////////////////////////////////////////////////////////

message CPearlInfEngine::ProductLambdaMsgs( int nodeNumber, int except) const
{
    int i;
    
    int                  numNeighb;
    const int*           nbrs;
    const ENeighborType* orient;
    
    m_pGraphicalModel->GetGraph()->GetNeighbors( nodeNumber, &numNeighb,
        &nbrs, &orient );
    
    int smallDom = 0;

    message lambda = NULL;

    if( m_selfMessages[nodeNumber]->GetDistributionType() == dtTabular )
    {
        lambda = CTabularDistribFun::Copy( (CTabularDistribFun* const)
            (m_selfMessages[nodeNumber]) );
    }
    else
    {
        lambda = CGaussianDistribFun::Copy( (CGaussianDistribFun* const)
            (m_selfMessages[nodeNumber]) );
    }
    for( i = 0 ; i <numNeighb; i++)
    {
        if( ( orient[i] != ntParent ) && ( nbrs[i] != except) )
        {
            lambda->MultiplyInSelfData( &smallDom, &smallDom, 
                m_messagesFromNeighbors[nodeNumber][i]);
        }
    }
    return lambda;
}
/////////////////////////////////////////////////////////////////////////////

void CPearlInfEngine::ComputeBelief( int nodeNumber )
{
    int dom = 0;

    message piProd = ComputeProductPi(nodeNumber);
    
    message lambdaProd = ProductLambdaMsgs(nodeNumber);
    
    piProd->MultiplyInSelfData(&dom, &dom, lambdaProd);
    //we delete belief in m_oldBeliefs - we copy pointers!
    //delete m_beliefs[nodeNumber];
    
    m_beliefs[nodeNumber] = piProd->GetNormalized();
    
    delete piProd;
    
    delete lambdaProd;
}
/////////////////////////////////////////////////////////////////////////////

void CPearlInfEngine::ParallelProtocol()
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
    int nAllMes = m_messagesFromNeighbors.size();
#if 0
    int mes_size;
#endif
    messageVecVector newMessages(m_messagesFromNeighbors);

    //create array of old messages
#if 0
    for( i = 0; i < nAllMes; i++)
    {
        mes_size = m_messagesFromNeighbors[i].size();

        for( j = 0; j <mes_size; j++)
        {
            newMessages[i][j] = m_messagesFromNeighbors[i][j];
        }
    }
#endif
    while ((!converged)&&(iter<m_maxNumberOfIterations))
    {
        //delete all old data
        //work with new data
        if (iter > 0)
        {
#ifdef _OPENMP
//#pragma omp parallel for
#endif
            for( i = 0; i < nAllMes	; i++)
            {
                for( j = 0; j <m_messagesFromNeighbors[i].size(); j++)
                {
                    delete m_messagesFromNeighbors[i][j];
                    m_messagesFromNeighbors[i][j] = newMessages[i][j];
                }
            }

            //compute beliefs
            changed = 0;
#ifdef _OPENMP
//#pragma omp parallel for schedule(dynamic) reduction(+:changed)
#endif
            for( i=0; i < nNodes; i++ )
            {
                if( !m_areReallyObserved[m_connNodes[i]])
                {
                    message tempBel = m_beliefs[m_connNodes[i]];
                    ComputeBelief( m_connNodes[i] );
                    changed += !tempBel->IsEqual( m_beliefs[m_connNodes[i]],
                        m_tolerance);
                    delete tempBel;
                }
            }
            converged = !(changed);
        }
        
        int                    numOfNeighb;
        const int              *neighbors;
        const ENeighborType *orientation;
        
#ifdef _OPENMP
//#pragma omp parallel for schedule(dynamic)
#endif
        for( i = 0; i < nNodes; i++ )
        {
            pGraph->GetNeighbors( m_connNodes[i], &numOfNeighb, 
                &neighbors, &orientation );
            for( j = 0; j < numOfNeighb; j++)
            {
                newMessages[m_connNodes[i]][j] = ComputeMessage( m_connNodes[i],
                    neighbors[j], orientation[j] );
            }
        }
        iter++;
    }//while ((!converged)&&(iter<m_numberOfIterations))
    //compute beliefs after last passing messages

    for( i = 0; i < nAllMes	; i++)
    {
        for( j = 0; j <m_messagesFromNeighbors[i].size(); j++)
        {
            delete m_messagesFromNeighbors[i][j];
            m_messagesFromNeighbors[i][j] = newMessages[i][j];
        }
    }
#ifdef _OPENMP
//#pragma omp parallel for schedule(dynamic)
#endif
    for( i=0; i < nNodes; i++ )
    {
        if( !m_areReallyObserved[m_connNodes[i]])
        {
            message tempBel = m_beliefs[m_connNodes[i]];
            ComputeBelief( m_connNodes[i] );
            delete tempBel;
        }
    }
    m_IterationCounter = iter;
}
/////////////////////////////////////////////////////////////////////////////

void CPearlInfEngine::TreeProtocol()
{
}
/////////////////////////////////////////////////////////////////////////////

messageVector& CPearlInfEngine::GetCurBeliefs()
{
    return m_beliefs;
}
/////////////////////////////////////////////////////////////////////////////

bool CPearlInfEngine::AllContinuousNodes( const CStaticGraphicalModel *pGrModel) const
{
    PNL_CHECK_IS_NULL_POINTER( pGrModel )
    int nnodes = GetNumberOfNodesInModel();
    int i;
    for (i = 0; i < nnodes; ++i)
    {
        PNL_CHECK_IS_NULL_POINTER( pGrModel->GetFactor(i) )
    }
    
    bool res = true;
    for (i = 0; i < nnodes; ++i)
    {
        if (pGrModel->GetFactor(i)->GetDistributionType() != dtGaussian)
            res = false;
    }
    
    return res;
}
/////////////////////////////////////////////////////////////////////////////

#ifdef PAR_RESULTS_RELIABILITY
bool IsEqualNumbers(float Num1, float Num2, float Diff)
{
    int exponent1 = 0;
    int sign1 = 1, sign2 = 1;
    if (Num1<0.0f)
        sign1 = 0;
    
    if (Num2<0.0f)
        sign2 = 0;

    if (sign1 != sign2)
        return false;

    if ((fabs(Num1)<1.0f)&&(fabs(Num2)<1.0f)) // numbers < 1
        if ( fabs(Num1-Num2) > Diff)
            return false;
        else
            return true;

    // number > 1
    while (fabs(Num1)>1.0f)
    {
        Num1 = Num1/10.0f;
        exponent1 ++;
    }

    int exponent2 = 0;
    while (fabs(Num2)>1.0f)
    {
        Num2 = Num2/10.0f;
        exponent2 ++;
    }
    if (exponent1 != exponent2)
        return false;
    else
        if ( fabs(Num1-Num2) > Diff)
            return false;
        else
            return true;
}

bool pnl::EqualResults(CPearlInfEngine& eng1, CPearlInfEngine& eng2,
    float epsilon)
{
    int i, j;
    bool res = true;
    messageVector& firstBels = eng1.GetCurBeliefs();
    
    if (eng1.AllContinuousNodes(eng2.GetModel()))
    {
        messageVector& secBels = eng2.CPearlInfEngine::GetCurBeliefs();
        if (firstBels.size() != secBels.size())
            res = false;
        else
        {
            CGaussianDistribFun* d1;
            CGaussianDistribFun* d2;
            for (j = 0; j < firstBels.size(); j++)
            {
                d1 = static_cast<CGaussianDistribFun*>(firstBels[j]);
                d2 = static_cast<CGaussianDistribFun*>(secBels[j]);
                if (!d1->IsEqual(d2, epsilon))
                {
                    const float *dataH1, *dataK1;
                    int sizeH1, sizeK1;
                    
                    const float *dataH2, *dataK2;
                    int sizeH2, sizeK2;
                    
                    const float *dataM1, *dataC1;
                    int sizeM1, sizeC1;
                    
                    const float *dataM2, *dataC2;
                    int sizeM2, sizeC2;
                    
                    int flag1 = d1->IsDistributionSpecific();
                    int flag2 = d2->IsDistributionSpecific();
                    if (flag1 != flag2)
                    {
                        res = false;
                        break;
                    }

                    if (flag1 == 1) // uniform
                    {
                        continue;
                    }
                    
                    if (flag1 == 2) // delta 
                    {
                        static_cast<CDenseMatrix<float>*>(d1->GetMatrix(matMean))->
                            GetRawData(&sizeM1, &dataM1);
                        
                        static_cast<CDenseMatrix<float>*>(d2->GetMatrix(matMean))->
                            GetRawData(&sizeM2, &dataM2);
                        
                        if (sizeM1 != sizeM2)
                        {
                            res = false;
                            break;
                        }
                        else
                            for (i = 0; i < sizeM1; i++)
                            {
                                printf("\n%.3f\t%.3f",dataM1[i],dataM2[i]);
                                if (!IsEqualNumbers(dataM1[i], dataM2[i], epsilon))
                                {
                                    res = false;
                                    break;
                                }
                            }
                        if (!res)
                            break;
                        else
                            continue;
                        
                    }

                    if (d1->GetCanonicalFormFlag())
                    {
                        static_cast<CDenseMatrix<float>*>(d1->GetMatrix(matH))->
                            GetRawData(&sizeH1, &dataH1);
                        static_cast<CDenseMatrix<float>*>(d1->GetMatrix(matK))->
                            GetRawData(&sizeK1, &dataK1);
                        
                        static_cast<CDenseMatrix<float>*>(d2->GetMatrix(matH))->
                            GetRawData(&sizeH2, &dataH2);
                        static_cast<CDenseMatrix<float>*>(d2->GetMatrix(matK))->
                            GetRawData(&sizeK2, &dataK2);
                        
                        if (sizeH1 != sizeH2)
                        {
                            res = false;
                            break;
                        }
                        else
                            for (i = 0; i < sizeH1; i++)
                                if (!IsEqualNumbers(dataH1[i], dataH2[i], epsilon))
                                {
                                    res = false;
                                    break;
                                }
                        if (!res)
                            break;
                            
                        if (sizeK1 != sizeK2)
                        {
                            res = false;
                            break;
                        }
                        else
                            for (i = 0; i < sizeK1; i++)
                                if (!IsEqualNumbers(dataK1[i], dataK2[i], epsilon))
                                {
                                    res = false;
                                    break;
                                }
                        if (!res)
                            break;
                    }
                    if (d1->GetMomentFormFlag())
                    {
                        static_cast<CDenseMatrix<float>*>(d1->GetMatrix(matMean))->
                            GetRawData(&sizeM1, &dataM1);
                        static_cast<CDenseMatrix<float>*>(d1->GetMatrix(matCovariance))->
                            GetRawData(&sizeC1, &dataC1);
                        
                        static_cast<CDenseMatrix<float>*>(d2->GetMatrix(matMean))->
                            GetRawData(&sizeM2, &dataM2);
                        static_cast<CDenseMatrix<float>*>(d2->GetMatrix(matCovariance))->
                            GetRawData(&sizeC2, &dataC2);
                        
                        if (sizeM1 != sizeM2)
                        {
                            res = false;
                            break;
                        }
                        else
                            for (i = 0; i < sizeM1; i++)
                                if (!IsEqualNumbers(dataM1[i], dataM2[i], epsilon))
                                {
                                    res = false;
                                    break;
                                }
                                
                        if (sizeC1 != sizeC2)
                        {
                            res = false;
                            break;
                        }
                        else
                            for (i = 0; i < sizeC1; i++)
                                if (!IsEqualNumbers(dataC1[i], dataC2[i], epsilon))
                                {
                                    res = false;
                                    break;
                                }
                    }
                }
            }
        }
    }
    else
    {
        messageVector& secBels = eng2.GetCurBeliefs();
        if (firstBels.size() != secBels.size())
            res = false;
        else
        {
            for (int i = 0; i < firstBels.size(); i++)
                if (!firstBels[i]->IsEqual(secBels[i], epsilon))
                {
                    res = false;
                }
        }
    }

    return res;
}
#endif // PAR_RESULTS_RELIABILITY
/////////////////////////////////////////////////////////////////////////////

#ifdef PNL_RTTI
const CPNLType CPearlInfEngine::m_TypeInfo = CPNLType("CPearlInfEngine", &(CInfEngine::m_TypeInfo));

#endif

/////////////////////////////////////////////////////////////////////////////