/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlJtreeInferenceEngine.cpp                                 //
//                                                                         //
//  Purpose:   CJTreeInfEngine class member functions implementation       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlJtreeInferenceEngine.hpp"
#include "pnlDistribFun.hpp"

PNL_USING

CJtreeInfEngine* CJtreeInfEngine::Create( const CStaticGraphicalModel 
                                         *pGraphicalModel,
                                         int numOfSubGrToConnect,
                                         const int *SubGrToConnectSizes,
                                         const int **SubgrToConnect )
{
    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(pGraphicalModel);
    /* bad-args check end */
    
    CJtreeInfEngine *pJTreeInfEngine = new CJtreeInfEngine( pGraphicalModel,
        numOfSubGrToConnect, SubGrToConnectSizes, SubgrToConnect );
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pJTreeInfEngine);
    
    return pJTreeInfEngine;
}

CJtreeInfEngine* CJtreeInfEngine::Create( const CStaticGraphicalModel 
                                          *pGraphicalModel,
                                          const intVecVector& SubgrToConnect )
{
    if( SubgrToConnect.size() == 0 )
    {
        return Create( pGraphicalModel, 0, NULL, NULL );
    }
    else
    {
        int i;
        
        int numOfSubGrToConnect = SubgrToConnect.size();
        
        intVector SubgrToConnectSizes(numOfSubGrToConnect);
        
        pnlVector< const int * > pSubgrToConnect(numOfSubGrToConnect);
        
        
        for( i = 0; i < numOfSubGrToConnect; ++i )
        {
            SubgrToConnectSizes[i] = SubgrToConnect[i].size();
            
            pSubgrToConnect[i] = &SubgrToConnect[i].front();
        }
        
        return Create( pGraphicalModel, numOfSubGrToConnect,
            &SubgrToConnectSizes.front(), &pSubgrToConnect.front() );
    }
}

CJtreeInfEngine* CJtreeInfEngine::Create( const CStaticGraphicalModel
                                          *pGraphicalModel, 
                                          CJunctionTree *pJTree)
{
    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(pGraphicalModel);
    PNL_CHECK_IS_NULL_POINTER(pJTree);
    /* bad-args check end */
    
    CJtreeInfEngine *pJTreeInfEngine = new CJtreeInfEngine( pGraphicalModel,
        pJTree );
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pJTreeInfEngine);
    
    return pJTreeInfEngine;
}

CJtreeInfEngine* CJtreeInfEngine::Copy(const CJtreeInfEngine *pJTreeInfEng)
{
    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(pJTreeInfEng);
    /* bad-args check end */
    
    CJtreeInfEngine *pJTreeInfEngineCopy = new CJtreeInfEngine(*pJTreeInfEng);
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pJTreeInfEngineCopy);
    
    return pJTreeInfEngineCopy;
}

CJtreeInfEngine::CJtreeInfEngine( const CStaticGraphicalModel 
                                  *pGraphicalModel, int numOfSubGrToConnect,
                                  const int *SubGrToConnectSizes, 
                                  const int **SubgrToConnect )
                                  : CInfEngine( itJtree, pGraphicalModel ),
                                  m_pOriginalJTree(CJunctionTree::Create(
                                  pGraphicalModel, numOfSubGrToConnect, 
                                  SubGrToConnectSizes, SubgrToConnect )),
                                  m_pJTree(NULL),// m_pEvidence(NULL),
                                  m_collectSequence(0), m_actuallyObsNodes(0),
                                  m_lastOpDone(opsNotStarted),
                                  m_JTreeRootNode(-1), m_pPotMPE(NULL), m_norm(-1.0f)
{
    // have to call SetRootNode() to receive some default collect sequence
    SetJTreeRootNode(0);
}

CJtreeInfEngine::CJtreeInfEngine( const CStaticGraphicalModel 
                                  *pGraphicalModel, CJunctionTree *pJTree )
                                  : CInfEngine( itJtree, pGraphicalModel ),
                                  m_pOriginalJTree(pJTree), m_pJTree(NULL),
                                  //m_pEvidence(NULL),
                                  m_collectSequence(0),
                                  m_actuallyObsNodes(0),
                                  m_lastOpDone(opsNotStarted),
                                  m_JTreeRootNode(-1), m_pPotMPE(NULL), m_norm(-1.0f)
{
    // have to call SetRootNode() to receive some default collect sequence
    SetJTreeRootNode(0);
}

CJtreeInfEngine::CJtreeInfEngine(const CJtreeInfEngine& rJTreeInfEngine)
                                 : CInfEngine( itJtree,
                                 rJTreeInfEngine.m_pGraphicalModel ),
                                 m_pOriginalJTree(CJunctionTree::Copy(
                                 rJTreeInfEngine.m_pOriginalJTree)),
                                 m_pJTree( rJTreeInfEngine.m_pJTree ?
				 ( rJTreeInfEngine.m_pOriginalJTree == rJTreeInfEngine.m_pJTree ?
				 m_pJTree = m_pOriginalJTree : 
				 CJunctionTree::Copy(rJTreeInfEngine.m_pJTree)) : NULL),
                                 //m_pEvidence(rJTreeInfEngine.m_pEvidence),
                                 m_JTreeRootNode(rJTreeInfEngine
                                 .m_JTreeRootNode), m_collectSequence(
                                 rJTreeInfEngine.m_collectSequence.begin(),
                                 rJTreeInfEngine.m_collectSequence.end() ),
                                 m_actuallyObsNodes(
                                 rJTreeInfEngine.m_actuallyObsNodes.begin(),
                                 rJTreeInfEngine.m_actuallyObsNodes.end() ),
                                 m_lastOpDone(rJTreeInfEngine.m_lastOpDone),
                                 m_pPotMPE(NULL),
				 m_norm(rJTreeInfEngine.m_norm)
{
    m_bMaximize = rJTreeInfEngine.m_bMaximize;
    m_pEvidence = rJTreeInfEngine.m_pEvidence;
}

/*void CJtreeInfEngine::Release(CJtreeInfEngine** pJTreeInfEng)
{
    delete *pJTreeInfEng;

    *pJTreeInfEng = NULL;
}
*/
CJtreeInfEngine::~CJtreeInfEngine()
{
    // when JTreeInfEngine is deleted it destroyes both Junction trees 
    // attached to it, cause they have some internal use semantics only
    //CJunctionTree::Release(&m_pOriginalJTree);
    delete m_pOriginalJTree;
    delete m_pPotMPE;
    //delete m_pJTree;
}

void CJtreeInfEngine::EnterEvidence( const CEvidence *pEvidence,
                                     int maximize, int sumOnMixtureNode )
{
   
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    PNL_CHECK_RANGES( maximize, 0, 2 );

    if( pEvidence->GetModelDomain() != m_pGraphicalModel->GetModelDomain() )
    {
        PNL_THROW( CInvalidOperation, 
            "evidence and the Graphical Model must be on one Model Domain" );
    }
    // bad-args check end
    ShrinkObserved( pEvidence, maximize, sumOnMixtureNode );
    CollectEvidence();
    DistributeEvidence();
}

void CJtreeInfEngine::ShrinkObserved( const CEvidence *pEvidence, 
                                      int maximize, int sumOnMixtureNode,
                                      bool bRebuildJTree )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    PNL_CHECK_RANGES( maximize, 0, 2 );
    // bad-args check end
    
    m_norm = -1;
    m_pEvidence = pEvidence;
    m_bMaximize = maximize;
    
    // deletes an old internal junction tree to start computations anew
    m_pOriginalJTree->ClearCharge();
    m_pOriginalJTree->InitCharge( m_pGraphicalModel, pEvidence,
        sumOnMixtureNode );

    m_pJTree = m_pOriginalJTree;

    pConstValueVector allOffsets;
    
    m_pEvidence->GetObsNodesWithValues( &m_actuallyObsNodes, &allOffsets );

/*    
    const int numOfNodesInGraph = m_pGraphicalModel->GetNumberOfNodes();
    const int numOfClqs         = m_pJTree->GetNumberOfNodes();
    
    const int numOfObsNodes          = m_pEvidence->GetNumberObsNodes();
    const int *obsNodes              = m_pEvidence->GetAllObsNodes();
    const int *actuallyObsNodesFlags = m_pEvidence->GetObsNodesFlags();
    const int *pOffset               = m_pEvidence->GetOffset();
    const unsigned char *evidData    = m_pEvidence->GetRawData();
    
    pnlVector< const unsigned char* > allOffsets( numOfNodesInGraph, NULL );
    
    int i;
    
    for( i = 0; i < numOfObsNodes; ++i )
    {
        if( actuallyObsNodesFlags[i] )
        {
            m_actuallyObsNodes.push_back(obsNodes[i]);
            allOffsets[obsNodes[i]] = evidData + pOffset[i];
        }
    }
    
    const int *clqIt, *clq_end;
    const int *nbrIt, *nbrs_end;
    const int *sepDomIt, *sepDom_end;
    
    intVector obsNodesInClq;
    intVector obsNodesInSep;
    
    obsNodesInClq.reserve(numOfObsNodes);
    obsNodesInSep.reserve(numOfObsNodes);
    
    // shrink node pots and separator pots based on the observed info
    for( i = 0; i < numOfClqs; ++i )
    {
        int       numOfNdsInClq;
        const int *clique;
        
        m_pJTree->GetNodeContent( i, &numOfNdsInClq, &clique );
        
        for( clqIt = clique, clq_end = clique + numOfNdsInClq;
        clqIt != clq_end; ++clqIt )
        {
            if( std::find( m_actuallyObsNodes.begin(),
                m_actuallyObsNodes.end(), *clqIt )
                != m_actuallyObsNodes.end() )
            {
                obsNodesInClq.push_back(*clqIt);
            }
        }
        
        if( !obsNodesInClq.empty() )
        {
            // here is a line, why it all should not work for a cond gaussian
            const CNodeType *pObsNodeNT = 
                m_pJTree->GetNodeType(i)->IsDiscrete()
                ? &m_ObsNodeType : &m_ObsGaussType;
            
            const CPotential *pShrPot = m_pJTree->GetNodePotential(i)
                ->ShrinkObservedNodes( obsNodesInClq.size(),
                obsNodesInClq.begin(), allOffsets.begin(), pObsNodeNT );
            
            // this should be changed to perform a safe operation
            m_pJTree->GetNodePotential(i)->SetDistribFun(
                pShrPot->GetDistribFun());
            
            delete pShrPot;
            
            // when done shrinking node pots, we go on with separator pots
            int                 numOfNbrs;
            const int           *nbrs;
            const ENeighborType *nbrsTypes;
            
            m_pJTree->GetGraph()->GetNeighbors( i, &numOfNbrs, &nbrs,
                &nbrsTypes );
            
            for( nbrIt = nbrs, nbrs_end = nbrs + numOfNbrs; nbrIt != nbrs_end;
            ++nbrIt )
            {
                if( *nbrIt > i )
                {
                    int       sepDomSz;
                    const int *sepDomain;
                    
                    m_pJTree->GetSeparatorDomain( i, *nbrIt, &sepDomSz,
                        &sepDomain );
                    
                    for( sepDomIt = sepDomain, sepDom_end = sepDomain
                        + sepDomSz; sepDomIt != sepDom_end; ++sepDomIt )
                    {
                        if( std::find( obsNodesInClq.begin(),
                            obsNodesInClq.end(), *sepDomIt )
                            != obsNodesInClq.end() )
                        {
                            obsNodesInSep.push_back(*sepDomIt);
                        }
                    }
                    
                    if( !obsNodesInSep.empty() )
                    {
                        pShrPot = m_pJTree->GetSeparatorPotential(i, *nbrIt)
                            ->ShrinkObservedNodes( obsNodesInSep.size(),
                            obsNodesInSep.begin(), allOffsets.begin(),
                            pObsNodeNT );
                        
                        m_pJTree->GetSeparatorPotential(i, *nbrIt)
                            ->SetDistribFun(pShrPot->GetDistribFun());
                        
                        delete pShrPot;
                    }
                    
                    obsNodesInSep.clear();
                }
            }
            
            obsNodesInClq.clear();
        }
    }
    */
    m_lastOpDone = opsShrinkEv;
}

void CJtreeInfEngine::CollectEvidence()
{
    // this is a phase of evidence propagation, when leaves send 
    // messages flow up to the root of the junction tree level by level
    
    // operation validity check
    /*	if( m_lastOpDone != opsShrinkEv )
    {
    PNL_THROW( CInvalidOperation,
    " cannot perform collect, JTreeInfEngine is inconsistent " );
    }
    */	// operation validity check end
    
    m_norm = 1.0f;
        
    const int                    *nbr, *nbrs_end;
    intVector::const_iterator    sourceIt, source_end;
    intVecVector::const_iterator layerIt     = m_collectSequence.begin(),
        collSeq_end = m_collectSequence.end();
    
    const CGraph *pGraph = m_pJTree->GetGraph();
    
    boolVector nodesSentMessages( m_pJTree->GetNumberOfNodes(), false );
    
    // at each step the propagation is from m_collectSequence i-th layer's
    // node to it's neighbors which has not sent the message yet
    for( ; layerIt != collSeq_end; ++layerIt )
    {
        for( sourceIt = layerIt->begin(), source_end = layerIt->end();
        sourceIt != source_end; ++sourceIt )
        {
            int                 numOfNbrs;
            const int           *nbrs;
            const ENeighborType *nbrsTypes;
            
            pGraph->GetNeighbors( *sourceIt, &numOfNbrs, &nbrs, &nbrsTypes );
            
            for( nbr = nbrs, nbrs_end = nbrs + numOfNbrs; nbr != nbrs_end;
            ++nbr )
            {
                if( !nodesSentMessages[*nbr] )
                {
                    PropagateBetweenClqs( *sourceIt, *nbr, true );
                    break;
                }
            }
            
            nodesSentMessages[*sourceIt] = true;
        }
    }
    
    m_lastOpDone = opsCollect;
}

void CJtreeInfEngine::DistributeEvidence()
{
    // this is a phase of evidence propagation, when the root node sends 
    // message flow down to it's neighbors untill the flow reaches leaves
    
    // operation validity check
    /*	if( m_lastOpDone != opsCollect )
    {
    PNL_THROW( CInvalidOperation,
    " distribute operation impossible. JTreeInfEngine incosistent " );
    }
    */	// operation validity check end
    
    const CGraph *pGraph   = m_pJTree->GetGraph();
    const int    numOfClqs = m_pJTree->GetNumberOfNodes();
    
    const int *nbr, *nbrs_end;
    
    boolVector nodesSentMessages( numOfClqs, false );
    
    intQueue source;
    
    source.push(m_JTreeRootNode);
    
    while( !source.empty() )
    {
        int sender = source.front();
        
        int                 numOfNbrs;
        const int           *nbrs;
        const ENeighborType *nbrsTypes;
        
        pGraph->GetNeighbors( sender, &numOfNbrs, &nbrs, &nbrsTypes );
        
        for( nbr = nbrs, nbrs_end = nbrs + numOfNbrs; nbr != nbrs_end;
        ++nbr )
        {
            if( !nodesSentMessages[*nbr] )
            {
                PropagateBetweenClqs( sender, *nbr, false );
                
                // need to save to the source queue, if its not a leaf node
                if( pGraph->GetNumberOfNeighbors(*nbr) > 1 )
                {
                    source.push(*nbr);
                }
            }
        }
        
        nodesSentMessages[sender] = true;
        source.pop();
    }
    
    m_lastOpDone = opsDistribute;
}

void CJtreeInfEngine::MarginalNodes( const int *query, int querySz, int notExpandJPD )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(query);
    PNL_CHECK_RANGES( querySz, 1, m_pGraphicalModel->GetNumberOfNodes() );
    // bad-args check end
/*
    // the following should be working differently for the case of doing the 
    // whole EnterEvidence procedure or just CollectEvidence for the root node
    if( ( m_lastOpDone != opsDistribute )
        && ( m_lastOpDone != opsMargNodes ) )
    {
        if( m_lastOpDone != opsCollect )
        {
            PNL_THROW( CInvalidOperation,
                " cannot perform marginalization, infEngine inconsistent " );
        }
        
        int       numOfClqsContQuery;
        const int *clqsContQuery;
        
        m_pOriginalJTree->GetClqNumsContainingSubset( querySz, query,
            &numOfClqsContQuery, &clqsContQuery );
        
        PNL_CHECK_FOR_ZERO(numOfClqsContQuery);
        
        if( std::find( clqsContQuery, clqsContQuery + numOfClqsContQuery,
            m_JTreeRootNode ) == clqsContQuery + numOfClqsContQuery )
        {
            PNL_THROW( CInvalidOperation,
                " cannot marginalize to the non-root-clq nodes set " );
        }
        
        //////// this is to debug
        for( int i = 0; i < numOfClqsContQuery; ++i )
        {
            CPotential *pJPot = m_pJTree->GetNodePotential(clqsContQuery[i])
                ->Marginalize( query, querySz );
            
            CPotential *pJPot1 = pJPot->GetNormalized();
            
            pJPot1->Dump();
            
            delete pJPot;
            delete pJPot1;
        }
        ///////////////////////////////////////////////////////
        
        MarginalizeCliqueToQuery( m_JTreeRootNode, querySz, query );
        
        m_lastOpDone = opsMargNodes;
    }
    else
    {
*/
    int numOfClqsContQuery;
    const int *clqsContQuery;
    
    m_pJTree->GetClqNumsContainingSubset( querySz, query,
        &numOfClqsContQuery, &clqsContQuery );
    
    PNL_CHECK_FOR_ZERO(numOfClqsContQuery);
    
    if( std::find( clqsContQuery, clqsContQuery + numOfClqsContQuery,
        m_JTreeRootNode ) != ( clqsContQuery + numOfClqsContQuery ) )
    {
        MarginalizeCliqueToQuery( m_JTreeRootNode, querySz, query, notExpandJPD );
    }
    else
    {
        MarginalizeCliqueToQuery( *clqsContQuery, querySz, query, notExpandJPD );
    }
    
    /*		m_lastOpDone = opsMargNodes;
    }
    */
}

const CPotential* CJtreeInfEngine::GetQueryJPD() const
{
    /* operation validity check */
    /*	if( m_lastOpDone != opsMargNodes )
    {
    PNL_THROW( CInvalidOperation,
    " cannot return query jpd, marginalization not done " );
    }
    */	/* operation validity check end */
    
    if( !m_pQueryJPD )
    {
        PNL_THROW( CInvalidOperation,
            " can't call GetQueryJPD() before calling MarginalNodes() " );
    }
    
    return m_pQueryJPD;
}

const CPotential* CJtreeInfEngine::GetQueryMPE() const
{
    if( !m_pPotMPE )
    {
        PNL_THROW( CInvalidOperation,
            " can't call GetQueryMPE() before calling MarginalNodes() " );
    }
    
    return m_pPotMPE;
}

const CEvidence* CJtreeInfEngine::GetMPE() const
{
    if( !m_bMaximize )
    {
        PNL_THROW( CInvalidOperation,
            " you have not been computing the MPE ");
    }
    
    if( !m_pEvidenceMPE )
    {
        PNL_THROW( CInvalidOperation,
            " can't call GetMPE() before calling MarginalNodes() " );
    }
    
    return m_pEvidenceMPE;
}

void CJtreeInfEngine::PropagateBetweenClqs( int source, int sink, bool isCollect )
{
    // bad-args check
    PNL_CHECK_RANGES( source, 0, m_pJTree->GetNumberOfNodes() - 1 );
    PNL_CHECK_RANGES( sink,   0, m_pJTree->GetNumberOfNodes() - 1 );
    // bad-args check end
    
    // operation validity check
    if( source == sink )
    {
        PNL_THROW( CInvalidOperation, " source and sink should differ " );
    }
    
    if( !m_pJTree->GetGraph()->IsExistingEdge( source, sink ) )
    {
        PNL_THROW( CInvalidOperation,
            " there is no edge between source and sink " );
    }
    /* operation validity check end */
    
    int       numNdsInSepDom;
    const int *sepDom;
    
    CPotential *potSource = m_pJTree->GetNodePotential(source),
        *potSink   = m_pJTree->GetNodePotential(sink);
    
    CPotential *potSep    = m_pJTree->GetSeparatorPotential( source, sink );
    
    CPotential *newPotSep, *updateRatio;
    
    potSep->GetDomain( &numNdsInSepDom, &sepDom );

    newPotSep = potSource->Marginalize( sepDom, numNdsInSepDom, m_bMaximize );
    
    updateRatio = newPotSep->Divide(potSep);
    
    *potSink *= *updateRatio;
    
    MyNormalize(potSink, isCollect);
    //potSink->Normalize();
    
    potSep->SetDistribFun(newPotSep->GetDistribFun());
    
    delete newPotSep;
    delete updateRatio;
}

void CJtreeInfEngine::
MyNormalize(CPotential *pot, bool isCollect )
{
    CDistribFun *pDf= pot->GetDistribFun();
    EDistributionType dt = pDf->GetDistributionType();
    if(! pDf->IsDistributionSpecific() )
    {
	if( dt == dtTabular )
	{
	    if( isCollect )
	    {
		CMatrix<float>* pMat = pDf->GetMatrix(matTable);
		m_norm *= pMat->SumAll(0);
	    }
	    pDf->Normalize();	
	}
    }
    
    
}

void CJtreeInfEngine::
MultJTreeNodePotByDistribFun( int clqPotNum, const int *domain,
                              const CDistribFun *pDistrFun )
{
    // bad-args check
    PNL_CHECK_RANGES( clqPotNum, 0, m_pJTree->GetNumberOfNodes() - 1 );
    PNL_CHECK_IS_NULL_POINTER(domain);
    PNL_CHECK_IS_NULL_POINTER(pDistrFun);
    // bad-args check end
    
    CPotential *pNodePot = m_pJTree->GetNodePotential(clqPotNum);
    
    int       nodePotDomSz;
    const int *nodePotDomain;
    
    pNodePot->GetDomain( &nodePotDomSz, &nodePotDomain );
    
    pNodePot->GetDistribFun()->MultiplyInSelfData( nodePotDomain, domain,
        pDistrFun );
}

void CJtreeInfEngine::
DivideJTreeNodePotByDistribFun( int clqPotNum, const int *domain,
                                const CDistribFun *pDistrFun )
{
    // bad-args check
    PNL_CHECK_RANGES( clqPotNum, 0, m_pJTree->GetNumberOfNodes() - 1 );
    PNL_CHECK_IS_NULL_POINTER(domain);
    PNL_CHECK_IS_NULL_POINTER(pDistrFun);
    // bad-args check end
    
    CPotential *pNodePot = m_pJTree->GetNodePotential(clqPotNum);
    
    int       nodePotDomSz;
    const int *nodePotDomain;
    
    pNodePot->GetDomain( &nodePotDomSz, &nodePotDomain );
    
    pNodePot->GetDistribFun()->DivideInSelfData( nodePotDomain, domain,
        pDistrFun );
}

void CJtreeInfEngine::RebuildTreeFromRoot() const
{
    // find all the leaves of the JTree and push them to the first layer
    // of the collect evidence sequence
    const int numOfNds   = m_pOriginalJTree->GetNumberOfNodes();
    const CGraph* pGraph = m_pOriginalJTree->GetGraph();
    
    int                 numOfNbrs1, numOfNbrs2;
    const int           *nbrs1, *nbrs2;
    const ENeighborType *nbrsTypes1, *nbrsTypes2;
    
    intVector  collectSeqCurrLayer;
    boolVector checkedNodes( numOfNds, false );
    
    m_collectSequence.clear();
    m_collectSequence.reserve(numOfNds);
    collectSeqCurrLayer.reserve(numOfNds);
    
    int i;
    
    for( i = 0; i < numOfNds; i++ )
    {
        pGraph->GetNeighbors( i, &numOfNbrs1, &nbrs1, &nbrsTypes1 );
        
        /* i-th node is a leaf if has one neighbor and not a root node */
        if( ( numOfNbrs1 == 1 ) && ( i != m_JTreeRootNode ) )
        {
            collectSeqCurrLayer.push_back(i);
            checkedNodes[i] = true;
        }
    }
    
    m_collectSequence.push_back(collectSeqCurrLayer);
    collectSeqCurrLayer.clear();
    
    const int *nbr1, *nbr2, *nbrs_end1, *nbrs_end2;
    intVector::const_iterator nodeIt, layer_end;
    
    int numOfCheckedNbrs;
    
    for( intVecVector::const_iterator layerIt = m_collectSequence.begin();; )
    {
        for( nodeIt = layerIt->begin(), layer_end = layerIt->end();
        layer_end - nodeIt; ++nodeIt )
        {
            pGraph->GetNeighbors( *nodeIt, &numOfNbrs1, &nbrs1, &nbrsTypes1 );
            
            for( nbr1 = nbrs1, nbrs_end1 = nbrs1 + numOfNbrs1;
            nbrs_end1 - nbr1; ++nbr1 )
            {
                if( !checkedNodes[*nbr1] )
                {
                    pGraph->GetNeighbors( *nbr1, &numOfNbrs2, &nbrs2,
                        &nbrsTypes2 );
                    
                    numOfCheckedNbrs = 0;
                    
                    for( nbr2 = nbrs2, nbrs_end2 = nbrs2 + numOfNbrs2;
                    nbrs_end2 - nbr2; ++nbr2 )
                    {
                        if( checkedNodes[*nbr2] )
                        {
                            ++numOfCheckedNbrs;
                        }
                    }
                    
                    if( ( numOfCheckedNbrs == numOfNbrs2 - 1 ) &&
                        ( *nbr1 != m_JTreeRootNode ) )
                    {
                        collectSeqCurrLayer.push_back(*nbr1);
                        checkedNodes[*nbr1] = true;
                    }
                }
            }
        }
        
        /* if the only one left is the root node, then the search is over */
        if( collectSeqCurrLayer.empty() )
        {
            collectSeqCurrLayer.push_back(m_JTreeRootNode);
            m_collectSequence.push_back(collectSeqCurrLayer);
            break;
        }
        
        /* check all the nodes from the current layer */
        m_collectSequence.push_back(collectSeqCurrLayer);
        layerIt = m_collectSequence.end() - 1;
        collectSeqCurrLayer.clear();
    }
}

void CJtreeInfEngine::
MarginalizeCliqueToQuery( int clqNum, int querySz, const int *query,
                          int notExpandJPD )
{
    // bad-args check
    PNL_CHECK_RANGES( clqNum, 0, m_pJTree->GetNumberOfNodes() - 1 );
    PNL_CHECK_RANGES( querySz, 1, m_pGraphicalModel->GetNumberOfNodes() );
    PNL_CHECK_IS_NULL_POINTER(query);
    // bad-args check end
    
    // Note: cant call expand() for potentials, which contain continuous
    // observed nodes in domain, cause those are to be expanded to 
    // mixture of gaussians, which we dont support right now.
    
    delete m_pQueryJPD;
    m_pQueryJPD = NULL;

    delete m_pPotMPE;
    m_pPotMPE = NULL;
    
    delete m_pEvidenceMPE;
    m_pEvidenceMPE = NULL;
    

    bool bExpandAllowed = true;
    
    CPotential* clqPotWithQuery = m_pJTree->GetNodePotential(clqNum);
    EDistributionType dtClqWithQuery = clqPotWithQuery->GetDistributionType();

    if( std::find_first_of( query, query + querySz,
        m_actuallyObsNodes.begin(),	m_actuallyObsNodes.end() )
        != ( query + querySz ) )
    {
        const int *queryIt = query, *query_end = query + querySz;
        
        for( ; queryIt != query_end; ++queryIt )
        {
            if( std::find( m_actuallyObsNodes.begin(),
                m_actuallyObsNodes.end(), *queryIt )
                != m_actuallyObsNodes.end() )
            {
                int shrNodebDiscrete = 
                    m_pGraphicalModel->GetNodeType(*queryIt)->IsDiscrete();
                if(((dtClqWithQuery == dtTabular)&&( !shrNodebDiscrete ))
                    ||(( dtClqWithQuery == dtGaussian )&&( shrNodebDiscrete )))
                {
                    bExpandAllowed = false;
                    break;
                }
            }
        }
    }

    if( ( bExpandAllowed == false ) && ( notExpandJPD == false ) )
    {
        PNL_THROW( CAlgorithmicException,
            " JPD expansion not possible technically " );
    }

    bExpandAllowed = notExpandJPD ? false : bExpandAllowed;
    
    CPotential *pMargJPot = clqPotWithQuery->Marginalize( query, querySz,
                                    m_bMaximize );
    
    if( bExpandAllowed )
    {
        CPotential *pExpObsJPot = pMargJPot->ExpandObservedNodes(m_pEvidence);
        
        if( m_bMaximize )
        {
            if( pMargJPot->GetDistributionType() == dtScalar )
            {
                m_pPotMPE = pExpObsJPot->GetNormalized();
                m_pEvidenceMPE = m_pPotMPE->GetMPE();
            }
            else
            {
                m_pPotMPE      = pMargJPot->GetNormalized();
                m_pEvidenceMPE = m_pPotMPE->GetMPE();
            }
        }
        else
        {
            m_pQueryJPD = pExpObsJPot->GetNormalized();
        }
        
        delete pExpObsJPot;
    }
    else
    {
        if( m_bMaximize )
        {
            m_pPotMPE      = pMargJPot->GetNormalized();
            m_pEvidenceMPE = m_pPotMPE->GetMPE();
        }
        else
        {
            m_pQueryJPD = pMargJPot->GetNormalized();
        }
    }
    if((!m_bMaximize)&&(m_pQueryJPD->GetDistributionType() == dtGaussian))
    {
        static_cast<CGaussianDistribFun*>(
            m_pQueryJPD->GetDistribFun())->UpdateMomentForm();
    }
    if((m_bMaximize)&&(m_pPotMPE->GetDistributionType() == dtGaussian))
    {
        static_cast<CGaussianDistribFun*>(
            m_pPotMPE->GetDistribFun())->UpdateMomentForm();
    }
    delete pMargJPot;
}

float CJtreeInfEngine::GetLogLik() const
{
    if( m_norm == -1.0f )
    {
	PNL_THROW( CInvalidOperation, " can't call GetLogLik before calling inferences procedure collect" );
    }

    //////////////////////////////////////////////////////////////////////////
    float ll = 0.0f;

    intVector obsDomains;
    GetObservedDomains(m_pEvidence, &obsDomains);
    const CStaticGraphicalModel *pGrModel = GetModel();

    int i;
    for( i = 0; i < obsDomains.size(); i++ )
    {
	ll += pGrModel->GetFactor(obsDomains[i])->GetLogLik(m_pEvidence);
    }

    CPotential *pPot;
    int root = GetJTreeRootNode();
    pPot=m_pJTree->GetNodePotential(root);
    if( pPot->GetDistribFun()->GetDistributionType() == dtGaussian )
    {
	CGaussianDistribFun *pDistr = static_cast<CGaussianDistribFun *>(pPot->GetDistribFun());
	pDistr->UpdateMomentForm();
	float koeff = pDistr->GetCoefficient(0);
	ll += (float)log( koeff );
    }
    else
    {
	ll += m_norm < FLT_EPSILON ? (float)log( FLT_EPSILON ) : (float)log( m_norm );
    }

    return ll;
}

void CJtreeInfEngine::GetObservedDomains(const CEvidence *pEv, intVector* domains ) const
{
    const int *obsNds = pEv->GetAllObsNodes();
    const int *obsFlags = pEv->GetObsNodesFlags();
    int nObsNds = pEv->GetNumberObsNodes();
    
    
    const CStaticGraphicalModel *pGrModel = GetModel();
    int nFactors = pGrModel->GetNumberOfFactors();
    int nnodes = pGrModel->GetNumberOfNodes();

    boolVector flags(nnodes, false );
    
    int i;
    for( i = 0; i < nObsNds; i++ )
    {
	if( obsFlags[i])
	{
	    flags[obsNds[i]] = true;
	}
    }

    for( i = 0; i < nFactors; i++ )
    {
	const int *domain;
	pGrModel->GetFactor(i)->GetDomain( &nnodes, &domain );

	int j;
	bool allObs = true;
	for( j = 0; (j < nnodes) && allObs; j++ )
	{
	    allObs &= flags[domain[j]];	    
	}
	if(allObs)
	{
	    domains->push_back(i);
	}
    }
}
