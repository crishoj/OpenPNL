/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlBKnferenceEngine.cpp                                     //
//                                                                         //
//  Purpose:   Implementation of the BK inference algorithm                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlBKInferenceEngine.hpp"
#include "pnlDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlScalarPotential.hpp"

PNL_USING

CBKInfEngine::
CBKInfEngine(const CDynamicGraphicalModel *pGrModel, intVecVector clusters):
C1_5SliceInfEngine( pGrModel ), 

m_clustersPrSl(clusters),
m_clusters1_5Sl(0),

m_clqs1_5Sl(0),
m_clqsPrSl(0),

m_pPriorSliceJtreeInf(NULL),
m_p1_5SliceJtreeInf(NULL),


m_nVlsForIntNds(0),

m_QuerryJTree(NULL),

m_isExact(false)

{
    Initialization();
}



bool CBKInfEngine::
CheckClustersValidity( intVecVector& clusters, intVector& interfNds )
{
    if( clusters.empty() )
    {
	return false;
    }
    
    
    intVector::iterator interfNdsLit = interfNds.begin();
    intVector::iterator interfNdsRit = interfNds.end();
    
    int i;
    for( i = 0; i < clusters.size(); i++ )
    {
	if( clusters[i].empty() )
	{
	    return false;
	}
	else
	{
	    int j;
	    for( j = 0; j < clusters[i].size(); j++ )
	    {
		if( std::find(interfNdsLit, interfNdsRit, clusters[i][j]) == interfNdsRit )
		{
		    return false;
		}
	    }
	}
    }
    return true;
}


CBKInfEngine* CBKInfEngine::
Create(const CDynamicGraphicalModel *pGrModel, intVecVector& clusters)
{
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    std::string description;

    if( !pGrModel->IsValid( &description ))
    {
	PNL_THROW( CBadArg, description );
    }
    if( pGrModel->GetModelType() != mtDBN )
    {
	PNL_THROW( CInconsistentType, " input model is invalid " );
    }
    
    intVector interfNds;
    pGrModel->GetInterfaceNodes( &interfNds );
    
    if( !CheckClustersValidity(clusters, interfNds) )
    {
	PNL_THROW( CBadArg, " clusters are invalid " );
    } 
    
    return new CBKInfEngine(pGrModel, clusters);
    
}


CBKInfEngine* CBKInfEngine::
Create(const CDynamicGraphicalModel *pGrModel, bool isFF )
{
    intVector interfNds;
    pGrModel->GetInterfaceNodes( &interfNds );
    
    intVecVector clusters;
    if( !isFF )
    {
	clusters.resize(1);
	clusters[0].assign( interfNds.begin(), interfNds.end() );
    }
    else
    {
	int nIntNds = interfNds.size();
	clusters.resize( nIntNds );
	int i;
	for( i = 0; i < nIntNds; i++ )
	{
	    clusters[i].push_back(interfNds[i]);
	}
	
    }
    
    return new CBKInfEngine(pGrModel, clusters);
}

CBKInfEngine::~CBKInfEngine()
{
    DestroyData();
    
    //CJtreeInfEngine::Release(&m_pPriorSliceJtreeInf);
    delete m_pPriorSliceJtreeInf;

    //CJtreeInfEngine::Release(&m_p1_5SliceJtreeInf);
    delete m_p1_5SliceJtreeInf;
}

void CBKInfEngine::
Get1_5Clusters(int nnodesPerSlice, intVector& interfNds, intVecVector& clusters, 
	       intVecVector* clusters1_5Sl) const
{
    
    int nInterfNds = interfNds.size();
    
    clusters1_5Sl->assign( clusters.begin(), clusters.end() );
    clusters1_5Sl->insert( clusters1_5Sl->end(), clusters.begin(), clusters.end() );
    
    int nClusters = clusters.size();
    int i,j;
    for( i = 0; i < nClusters; i++ )
    {
	int nnodesPerClust = clusters[i].size();
	
	for( j = 0; j < nnodesPerClust; j++ )
	{
	    intVector::iterator loc = std::find( interfNds.begin(),
			    interfNds.end(), clusters[i][j] );
	    
	    (*clusters1_5Sl)[i][j] = loc - interfNds.begin();
	    (*clusters1_5Sl)[i + nClusters][j] += nInterfNds;
	}
	
    }
    
}

void CBKInfEngine::
FindingMinimalClqs(CJtreeInfEngine *pInf, intVecVector &clusters, intVector *cliques)
{
    
    cliques->resize(clusters.size());
    
    intVecVector::iterator clusterIt = clusters.begin();
    intVector::iterator cliquesIt = cliques->begin();
    for( ; clusterIt != clusters.end(); clusterIt++, cliquesIt++ )
    {
	intVector currentCliques;
	pInf->GetClqNumsContainingSubset( *clusterIt, &currentCliques);
	
	intVector::iterator clqIt = currentCliques.begin();
	intVector::iterator minClqIt = clqIt;
	
	intVector content;
	pInf->GetJTreeNodeContent( *clqIt, &content );
	int minNdSz = content.size();
	++clqIt;
	
	for( ; clqIt != currentCliques.end(); clqIt++)
	{ 
	    pInf->GetJTreeNodeContent( *clqIt, &content );
	    int ndSz = content.size();
	    if( ndSz < minNdSz )
	    {
		minClqIt = clqIt;
		minNdSz = ndSz;
	    }            
	}
	(*cliquesIt) = *minClqIt;
    }
}

void CBKInfEngine::
Initialization()
{
    
    int nIntNodes = GetNumIntNds();
    m_nVlsForIntNds.assign(nIntNodes,1 );
    int i;
    for( i = 0; i < nIntNodes; i++ )
    {
	const CNodeType* nt = GrModel()->GetNodeType((*GetIntNdsPriorSlice())[i]);
	if( !nt->IsDiscrete() )
	{
	    m_nVlsForIntNds[i] = nt->GetNodeSize();
	}
    }
    
   
    intVector interfNds(GetIntNdsPriorSlice()->begin(), GetIntNdsPriorSlice()->end());
    Get1_5Clusters(GrModel()->GetNumberOfNodes(), interfNds, m_clustersPrSl, &m_clusters1_5Sl);
    
    intVecVector clustersIn;
    RemoveUnitClusters( m_clustersPrSl, &clustersIn );
    m_pPriorSliceJtreeInf = CJtreeInfEngine::Create( GetPriorSliceBNet(), clustersIn );
    
    clustersIn.clear();
    RemoveUnitClusters( m_clusters1_5Sl, &clustersIn );
    m_p1_5SliceJtreeInf   = CJtreeInfEngine::Create( Get1_5SliceBNet(), clustersIn);
    
    FindingMinimalClqs(m_pPriorSliceJtreeInf, m_clustersPrSl, &m_clqsPrSl);
    FindingMinimalClqs(m_p1_5SliceJtreeInf, m_clusters1_5Sl, &m_clqs1_5Sl);
    
    if( m_clustersPrSl.size() == 1 )
    {
	m_pPriorSliceJtreeInf->SetJTreeRootNode( m_clqsPrSl[0] );
	m_p1_5SliceJtreeInf->SetJTreeRootNode( m_clqs1_5Sl[1] );
	m_isExact = true;
    }
    
}

void CBKInfEngine::DestroyData()
{
    
    int rg;
    for( rg = 0; rg < m_CRingpEv.size(); rg++ )
    {
	delete m_CRingpEv[rg];
	m_CRingpEv[rg] = NULL;
    }
    
    m_CRingpEv.clear();
    
    for( rg = 0; rg < m_CRingJtreeInf.size(); rg++ )
    {
        //CJtreeInfEngine::Release(&m_CRingJtreeInf[rg]);
        delete m_CRingJtreeInf[rg];
    }
    
    m_CRingJtreeInf.clear();
    
    for( rg = 0; rg < m_CRingDistrOnSep.size(); rg++ )
    {
        int i;
        for( i = 0; i < m_CRingDistrOnSep[rg].size(); i++ )
        {
            delete m_CRingDistrOnSep[rg][i];
        }
        m_CRingDistrOnSep[rg].clear();
    }
    
    m_CRingDistrOnSep.clear();
    
    delete GetQueryPot();
    SetQueryPot( NULL );
    
    delete GetEvidenceMPE();
    SetEvidenceMPE( NULL );
    
    m_QuerryJTree = NULL;
    
    m_CurrentTime = 0;
    
}

void CBKInfEngine::
DefineProcedure( EProcedureTypes procedureType, int lag )
{
    /////////////////////////////////////////////////////////////////////////
    // Selection procedure (smoothing, filtering, ...
    /////////////////////////////////////////////////////////////////////////
    
    DestroyData();
    
    switch( procedureType )
    {
    case ptFiltering:
        {
            PNL_CHECK_FOR_NON_ZERO( lag );
            m_Lag = 0;
            m_CRingJtreeInf.resize( 2 );
            m_CRingJtreeInf[0]= CJtreeInfEngine::Copy(m_pPriorSliceJtreeInf);
            m_CRingJtreeInf[1]= CJtreeInfEngine::Copy(m_p1_5SliceJtreeInf) ;
            m_ProcedureType = ptFiltering;
            break;
        }
        
    case ptFixLagSmoothing:
        {
            PNL_CHECK_LEFT_BORDER( lag, 0 );
            m_Lag = lag;
            m_CRingJtreeInf.resize( m_Lag+1 );
            m_CRingJtreeInf[0] = CJtreeInfEngine::Copy(m_pPriorSliceJtreeInf);
            
            int t;
            for( t = 1; t < lag + 1; t++ )
            {
                m_CRingJtreeInf[t] = CJtreeInfEngine::Copy(m_p1_5SliceJtreeInf);
            }
            
            m_CRingDistrOnSep.resize(lag);
            m_ProcedureType = procedureType;
            break;
        }
        
    case ptSmoothing:
    case ptViterbi:
        {
            PNL_CHECK_LEFT_BORDER( lag, 1 );
            m_Lag = lag;
            
            m_CRingJtreeInf.resize(lag);
            m_CRingJtreeInf[0] = CJtreeInfEngine::Copy(m_pPriorSliceJtreeInf);
            
            int t;
            for( t = 1; t < lag; t++ )
            {
                m_CRingJtreeInf[t] = CJtreeInfEngine::Copy(m_p1_5SliceJtreeInf);
            }
            m_CRingDistrOnSep.resize(lag-1);
            
            m_ProcedureType = procedureType;
            break;
        }
        
        
    default:
        {
            PNL_THROW( CInconsistentType, "incorrect type of inference procedure" );
        }
    }
    
    int i;
    for( i = 0; i < m_CRingDistrOnSep.size(); i++ )
    {
        m_CRingDistrOnSep[i].resize(GetNumOfClusters());
        
    }
    // initialize iterator for the jtree inferences sequence
    m_JTreeInfIter = m_CRingJtreeInf.begin();
    m_CDistrOnSepIter = m_CRingDistrOnSep.begin();
    m_CurrentTime = 0;
    
}

void CBKInfEngine::ForwardFirst(const CEvidence *pEvidence, int maximize )
{
    
    PNL_CHECK_FOR_NON_ZERO(GetTime());
    m_JTreeInfIter = m_CRingJtreeInf.begin();
    
    PNL_CHECK_IS_NULL_POINTER(*m_JTreeInfIter);
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    
    (*m_JTreeInfIter)->ShrinkObserved( pEvidence, maximize );
    (*m_JTreeInfIter)->CollectEvidence();
    
    m_CurrentTime++;
    
}


void CBKInfEngine::Forward(const CEvidence *pEvidence, int maximize )
{
    
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    PNL_CHECK_IS_NULL_POINTER(*m_JTreeInfIter);
    
    
    CJtreeInfEngine *pastEng; 
    
    if( !m_isExact )
    {
	if(m_CRingDistrOnSep.size())
	{
	    pastEng = CJtreeInfEngine::Copy(*m_JTreeInfIter);
	}
	else
	{
	    pastEng = *m_JTreeInfIter;
	}
	pastEng->DistributeEvidence();
    }
    else
    {
	pastEng = *m_JTreeInfIter;
    }
    
    m_JTreeInfIter++;
    PNL_CHECK_IS_NULL_POINTER(*m_JTreeInfIter);
    
    if( (*m_JTreeInfIter)->GetModel()->GetNumberOfNodes() == GetPriorSliceBNet()->GetNumberOfNodes() )
    {
    	//CJtreeInfEngine::Release(&(*m_JTreeInfIter));
        delete (*m_JTreeInfIter);

    	(*m_JTreeInfIter) = CJtreeInfEngine::Copy(m_p1_5SliceJtreeInf);
    }
    
    (*m_JTreeInfIter)->ShrinkObserved( pEvidence, maximize );
    
    intVecVector::iterator clustersItFrom, clustersItEnd;
    
    if( GetTime() == 1 )
    {
	clustersItFrom = m_clustersPrSl.begin();
	clustersItEnd = m_clustersPrSl.end();
	
    }
    else
    {
	clustersItFrom = m_clusters1_5Sl.begin() + m_clustersPrSl.size();
	clustersItEnd = m_clusters1_5Sl.end();
    }
    
    intVecVector::iterator clustersItTo = m_clusters1_5Sl.begin();
    intVector::iterator JTreeNdsIt = m_clqs1_5Sl.begin();
    
    distrPVector::iterator sepIt = distrPVector::iterator(0);
    if(m_CRingDistrOnSep.size())
    {
	sepIt = (*m_CDistrOnSepIter).begin();
    }
    const CPotential *queryPot;
    
    for( ; clustersItFrom != clustersItEnd; clustersItFrom++, clustersItTo++, JTreeNdsIt++)
    {
	pastEng->MarginalNodes( &(clustersItFrom->front()), clustersItFrom->size(), 1 );
	
	if( maximize )
	{
	    queryPot = pastEng->GetQueryMPE();
	}
	else
	{
	    queryPot = pastEng->GetQueryJPD();
	    
	}
	
	(*m_JTreeInfIter)->MultJTreeNodePotByDistribFun(*JTreeNdsIt, &(clustersItTo->front()), 
	    queryPot->GetDistribFun());
	
	if(m_CRingDistrOnSep.size())
	{
	    delete (*sepIt);
	    *sepIt = queryPot->GetDistribFun()->CloneWithSharedMatrices();
	    sepIt++;
	}
    }
    (*m_JTreeInfIter)->CollectEvidence();
    
    if( !m_isExact && m_CRingDistrOnSep.size())
    {
    	//CJtreeInfEngine::Release(&pastEng);
        delete pastEng;
	pastEng = NULL;
    }
    
    m_CurrentTime++;
    if(m_CRingDistrOnSep.size())
    {
	m_CDistrOnSepIter++;
    }
    
    
}

void CBKInfEngine::BackwardT()
{
    PNL_CHECK_IS_NULL_POINTER(*m_JTreeInfIter);
    
    (*m_JTreeInfIter)->DistributeEvidence();
    
    if(m_CRingDistrOnSep.size())
    {
	m_CDistrOnSepIter--;
    }
}


void CBKInfEngine::BackwardFixLag()
{
    //////////////////////////////////////////////////////////////////////////
    // Backward step for fixed-lag smoothing procedure
    //////////////////////////////////////////////////////////////////////////
    PNL_CHECK_LEFT_BORDER(m_CurrentTime, m_Lag);
    
    if( m_Lag )
    {
	int currentTimeTmp = GetTime();
	CRing<CJtreeInfEngine *>::iterator tmpInfIter = m_JTreeInfIter;
	CRing< distrPVector >::iterator tmpDistrIter =  m_CDistrOnSepIter;
	
	CRing<CJtreeInfEngine*> ringEng;
	ringEng.assign( 2 , NULL ); 
	CRing<CJtreeInfEngine *>::iterator ringEngIter = ringEng.begin();
	
	*ringEngIter = CJtreeInfEngine::Copy(*m_JTreeInfIter);
	ringEngIter++;
	
	BackwardT();
	
	distrPVector tmpDistr(GetNumOfClusters(), (CDistribFun* const)NULL);
	int i;
	for( i = 0; i < GetLag(); i++ )
	{
	    if( i < GetLag() - 1 )
	    {
		*ringEngIter = CJtreeInfEngine::Copy(*(m_JTreeInfIter-1));
		int j;
		for( j = 0; j < GetNumOfClusters(); j++ )
		{
		    tmpDistr[j] = (*m_CDistrOnSepIter)[j]->Clone();
		}
	    }
	    Backward();
	    
	    //CJtreeInfEngine::Release(&(*(m_JTreeInfIter + 1)));
            delete (*(m_JTreeInfIter + 1));
	    
	    ringEngIter++;
	    
	    *(m_JTreeInfIter + 1) = *ringEngIter;
	    
	    if( i < GetLag() - 1 )
	    { 
		int j;
		for( j = 0; j < GetNumOfClusters(); j++ )
		{
		    delete (*(m_CDistrOnSepIter+1))[j];
		    (*(m_CDistrOnSepIter+1))[j]=tmpDistr[j];
		}
	    }
	} 
	
	
	m_CDistrOnSepIter = tmpDistrIter;
	m_JTreeInfIter = tmpInfIter;
	m_CurrentTime = currentTimeTmp;
	
    }
    else
    {
	BackwardT();
    }
    
}

void CBKInfEngine::Backward( int maximize )
{
    m_CurrentTime--;
    intVecVector::iterator clustersItFrom, clustersItEnd;
    
    
    clustersItFrom = m_clusters1_5Sl.begin();
    clustersItEnd = clustersItFrom + GetNumOfClusters();
    
    CJtreeInfEngine *pFutureInf = *m_JTreeInfIter;
    
    m_JTreeInfIter--;
    CJtreeInfEngine *pCurrentInf = *m_JTreeInfIter;
    
    
    intVector::iterator JTreeNdsIt = m_clqs1_5Sl.begin();
    
    intVecVector::iterator clustersItTo;
    if( pCurrentInf ->GetModel()->GetNumberOfNodes() == GetPriorSliceBNet()->GetNumberOfNodes() )
    {
	clustersItTo = m_clustersPrSl.begin();
	JTreeNdsIt = m_clqsPrSl.begin();
	
    }
    else
    {
	clustersItTo = m_clusters1_5Sl.begin()+ m_clustersPrSl.size();
	JTreeNdsIt = m_clqs1_5Sl.begin() + m_clustersPrSl.size();
    }
    
    distrPVector::iterator sepIt = (*m_CDistrOnSepIter).begin();
    
    const CPotential * queryPotF;
    
    for( ; clustersItFrom != clustersItEnd; clustersItFrom++, clustersItTo++, JTreeNdsIt++, sepIt++ )
    {
	pFutureInf->MarginalNodes( &(clustersItFrom->front()), clustersItFrom->size(), 1 );
	
	if( maximize )
	{
	    queryPotF = pFutureInf->GetQueryMPE();
	    
	}
	else
	{
	    queryPotF = pFutureInf->GetQueryJPD();
	}
	
	
	//hack
	queryPotF->GetDistribFun()->
	    DivideInSelfData(&(clustersItTo->front()),&(clustersItTo->front()),
	    *sepIt);
	
	pCurrentInf->MultJTreeNodePotByDistribFun(*JTreeNdsIt, &(clustersItTo->front()), 
	    queryPotF->GetDistribFun());
    }
    pCurrentInf->DistributeEvidence();
    m_CDistrOnSepIter--;
    
}

const CPotential* CBKInfEngine::GetQueryJPD() 
{
    if( m_ProcedureType != ptFiltering && 
	m_ProcedureType != ptSmoothing &&
	m_ProcedureType != ptFixLagSmoothing )
    {
	PNL_THROW( CInvalidOperation,
	    " you have not been computing the Query JPD ");
    }
    if( !GetQueryPot() )
    {
        const CPotential* pQueryPot = m_QuerryJTree->GetQueryJPD();
        /*
        const CDistribFun *pQueryDistribFun = pQueryPot->GetDistribFun();
        
          
            switch( pQueryDistribFun->GetDistributionType() )
            {
            case dtTabular:
            SetQueryPot( CTabularPotential::Create( m_queryNodes,
            GrModel()->GetModelDomain(), NULL) );
            break;
            case dtGaussian:
            SetQueryPot( CGaussianPotential::Create( m_queryNodes,
            GrModel()->GetModelDomain(), NULL) );
            break;
            case dtScalar:
            SetQueryPot( CScalarPotential::Create( m_queryNodes,
            GrModel()->GetModelDomain() ) );
            break;
            default:
            PNL_THROW(CNotImplemented, "type of potential");
            }
            
              GetQueryPot()->SetDistribFun( pQueryDistribFun );
        */
        intVector obsPos; 
        pQueryPot->GetObsPositions(&obsPos);
        intVector queryNodes;
	GetQueryNodes(&queryNodes);
        SetQueryPot(static_cast<CPotential*>( CFactor::
            CopyWithNewDomain(pQueryPot, queryNodes, GrModel()->GetModelDomain(), obsPos)));
    }
    
    return GetQueryPot();
}

const CEvidence* CBKInfEngine::GetMPE()
{
    if( m_ProcedureType != ptViterbi )
    {
	PNL_THROW( CInvalidOperation,
	    " you have not been computing the MPE ");
    }
    
    const CEvidence *pEv = m_QuerryJTree->GetMPE();
    intVector queryNodes;
    GetQueryNodes(&queryNodes);
    SetEvidenceMPE( CEvidence::
	Create(pEv, queryNodes.size(), &queryNodes.front(), GrModel()->GetModelDomain()));
    return GetEvidenceMPE();
}

void CBKInfEngine::
MarginalNodes( const int *query, int querySize, int slice, int notExpandJPD )
{
    /////////////////////////////////////////////////////////////////////////
    if( GetProcedureType() == ptFiltering )
    {
	PNL_CHECK_LEFT_BORDER(m_CRingpEv.size(), 1);
    }
    else
    {
	PNL_CHECK_LEFT_BORDER(m_CRingpEv.size() , m_CRingJtreeInf.size());
    }
   
    /////////////////////////////////////////////////////////////////////////
    
    if( GetEvidenceMPE() )
    {
	delete GetEvidenceMPE();
	SetEvidenceMPE(NULL);
    }
    if( GetQueryPot() )
    {
	delete GetQueryPot();
	SetQueryPot(NULL);
    }
    
   
    SetQueryNodes(querySize, query);
    intVector queryVec;
    queryVec.assign(query, query + querySize);
    
    intVector finalQuery;

    switch( m_ProcedureType )
    {
    case ptFiltering:
	{
	    FindFinalQuery( queryVec, m_CurrentTime  - 1, &finalQuery);
	    m_QuerryJTree  =  m_CRingJtreeInf[m_CurrentTime  - 1];
	    m_QuerryJTree->MarginalNodes( &finalQuery.front(), querySize, notExpandJPD );
	    break;
	}
    case ptFixLagSmoothing:
	{
	    FindFinalQuery( queryVec, m_CurrentTime - m_Lag - 1, &finalQuery);
	    m_QuerryJTree = m_CRingJtreeInf[m_CurrentTime - m_Lag - 1];
	    m_QuerryJTree->MarginalNodes( &finalQuery.front(), querySize, notExpandJPD );
	    break;
	}
    case ptSmoothing:
    case ptViterbi:
	{
	    PNL_CHECK_RANGES(slice, 0, m_Lag);
	    
	    FindFinalQuery( queryVec, slice, &finalQuery);
	    m_QuerryJTree = m_CRingJtreeInf[slice];
	    m_QuerryJTree->MarginalNodes( &finalQuery.front(), querySize, notExpandJPD );
	    break;
	}
    }
}



void CBKInfEngine::
EnterEvidence( const CEvidence * const* evidence, int nSlices )
{
    if(( m_ProcedureType == ptFiltering || 
        m_ProcedureType ==ptFixLagSmoothing ) && 
        nSlices != 1)
    {
        char* mess = "It is online procedure. Number of evidences must be equal to 1";
        PNL_THROW( COutOfRange, mess )
    }
    
    switch( GetProcedureType() )
    {
    case ptSmoothing:
    case ptViterbi:
        {
            if( GetQueryPot() || GetEvidenceMPE() )
            {
                PNL_THROW(CAlgorithmicException, "need to define procedure");
            }
            int numEv = GetLag();
            PNL_CHECK_FOR_NON_ZERO( nSlices - numEv );
            if( m_CRingpEv.size() )
            {
                
                int rg;
                for( rg = 0; rg < m_CRingpEv.size(); rg++ )
                {
                    delete m_CRingpEv[rg];
                    m_CRingpEv[rg] = NULL;
                }
                for( rg = 0; rg < m_CRingDistrOnSep.size(); rg++ )
                {
                    int i;
                    for( i = 0; i < m_CRingDistrOnSep[rg].size(); i++ )
                    {
                        delete m_CRingDistrOnSep[rg][i];
                        m_CRingDistrOnSep[rg][i] = NULL;
                    }
                }
            }
            else
            {        
                m_CRingpEv.resize( numEv );
            }
            
            CRing<CEvidence *>::iterator evidIter = m_CRingpEv.begin();
            PNL_CHECK_IS_NULL_POINTER(evidence[0]);
            *evidIter = CreateCurrentEvidence(evidence[0], NULL ); 
            evidIter++;
            int i;
            for( i = 1; i < nSlices; i++, evidIter++ )
            {
                PNL_CHECK_IS_NULL_POINTER(evidence[i]);
                *evidIter = CreateCurrentEvidence( evidence[i], *(evidIter-1) );
            }
            break;
        }
    case ptFiltering:
    case ptFixLagSmoothing:
        {
            int cTime = GetTime();
            int nEv = m_CRingpEv.size();
            if( cTime && nEv == GetLag() + 1 )
            {
                PNL_CHECK_IS_NULL_POINTER(evidence[0]);
                PNL_CHECK_FOR_NON_ZERO( GetLag() + 1 - nEv);
                CEvidence* tmpEv = CreateCurrentEvidence( evidence[0], m_CRingpEv[cTime] );
                delete m_CRingpEv[cTime];
                m_CRingpEv[cTime] = tmpEv;
            }
            else
            {
                
                if( !nEv )
                {
                    PNL_CHECK_IS_NULL_POINTER(evidence[0]);
                    m_CRingpEv.push_back(CreateCurrentEvidence( evidence[0] ));
                }
                else
                {
                    
                    PNL_CHECK_RIGHT_BORDER(nEv, GetLag() + 1)
                        m_CRingpEv.push_back(CreateCurrentEvidence( evidence[0], m_CRingpEv[nEv-1]));
                }
                
            }
            break;
        }
    default:
        {
            PNL_THROW(CNotImplemented, "not implemented")
        }
    }
    
}

CEvidence* CBKInfEngine::
CreateCurrentEvidence(const CEvidence* pEvIn, const CEvidence* pEvPast )
{
    
    int nObsNdsIn = pEvIn->GetNumberObsNodes();
    intVector obsNds(nObsNdsIn);
    memcpy(&obsNds.front(), pEvIn->GetAllObsNodes(), nObsNdsIn*sizeof(int));
    
    valueVector obsVls;
    pEvIn->GetRawData(&obsVls);
    
    const CModelDomain* pMD;
    
    if( pEvPast)
    {
	bool pastIs1_5Slice = pEvPast->GetModelDomain()->GetNumberVariables() == 
	    Get1_5SliceBNet()->GetNumberOfNodes() ? true : false;
	const int* outgoingNds;
	
	outgoingNds = GetOutgoingNds(pastIs1_5Slice);
	
	pMD = Get1_5SliceBNet()->GetModelDomain();
	int i;
	for( i = 0; i < nObsNdsIn; i++ )
	{
	    obsNds[i] += GetNumIntNds();
	}
	
	
	for( i = 0; i < GetNumIntNds(); i++ )
	{
	    if( pEvPast->IsNodeObserved( outgoingNds[i]) )
	    {
		obsNds.push_back(i);
		
		const Value* val = pEvPast->GetValue(outgoingNds[i]);
		int nVls = GetNumVlsForIntNode(i);
		int j;
		for( j = 0; j < nVls; j++ )
		{
		    obsVls.push_back( val[j] );
		}
	    }
	}
    }
    else
    {
	pMD = GetPriorSliceBNet()->GetModelDomain();
	
    }
    CEvidence* pEvOut =  CEvidence::Create(pMD, obsNds, obsVls);
    const int* flags = pEvIn->GetObsNodesFlags();
    int i;
    for( i = 0; i < nObsNdsIn; i++ )
    {
	if(!flags[i])
	{
	    pEvOut->ToggleNodeStateBySerialNumber(1, &i);
	}
    }
    return pEvOut;
}

void CBKInfEngine::
RemoveUnitClusters(intVecVector &clustersIn, intVecVector *clustersOut)
{
    clustersOut->clear();
    intVecVector::iterator itIn = clustersIn.begin();
    clustersOut->reserve(clustersIn.size());
    for( ; itIn != clustersIn.end(); itIn++ )
    {
	if( itIn->size() > 1 )
	{
	    clustersOut->push_back(*itIn);
	}
    }
}

#ifdef PNL_RTTI
const CPNLType CBKInfEngine::m_TypeInfo = CPNLType("CBKInfEngine", &(C1_5SliceInfEngine::m_TypeInfo));

#endif
