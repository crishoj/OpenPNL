/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnl1_5SliceJtreeInferenceEngine.cpp                         //
//                                                                         //
//  Purpose:   Implementation of the  1.5 Slice Junction tree              //
//             inference algorithm                                         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnl1_5SliceJtreeInferenceEngine.hpp"
#include "pnlDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlScalarPotential.hpp"

PNL_USING

C1_5SliceJtreeInfEngine::
C1_5SliceJtreeInfEngine(const CDynamicGraphicalModel *pGraphicalModel):
    C1_5SliceInfEngine( pGraphicalModel ),
    
    m_QuerrySlice(0)
{
    //////////////////////////////////////////////////////////////////////////
    // define interface nodes for prior and regular slices
	/////////////////////////////////////////////////////////////////////////
	
	int nIntNds = GetNumIntNds();
	PNL_CHECK_LEFT_BORDER( nIntNds, 1 );

   
    //////////////////////////////////////////////////////////////////////////

    
    const int *subgrToConnect[] = { &GetIntNdsPriorSlice()->front() };
#ifdef PAR_OMP
    if( nIntNds > 1)
    {
        m_pPriorSliceJtreeInf = CParJtreeInfEngine::Create( GetPriorSliceBNet(),
        	1, &nIntNds, subgrToConnect );
    }
    else
    {
	    m_pPriorSliceJtreeInf = CParJtreeInfEngine::Create( GetPriorSliceBNet() );
    }
#else
    if( nIntNds > 1)
    {
	m_pPriorSliceJtreeInf = CJtreeInfEngine::Create( GetPriorSliceBNet(),
	1, &nIntNds, subgrToConnect );
    }
    else
    {
	m_pPriorSliceJtreeInf = CJtreeInfEngine::Create( GetPriorSliceBNet() );
    }
#endif

    int numOfCliques;
    const int* cliques;
    m_pPriorSliceJtreeInf->GetClqNumsContainingSubset( nIntNds,
	&GetIntNdsPriorSlice()->front(), &numOfCliques, &cliques );
    m_pPriorSliceJtreeInf->SetJTreeRootNode( cliques[0] );
    /////////////////////////////////////////////////////////////////////
	int numOfSubGrToConect = 2;
	pnlVector<const int *> SubGrToConnect;
    SubGrToConnect.resize( numOfSubGrToConect );
    SubGrToConnect[1] = &GetIntNdsISlice()->front();
    SubGrToConnect[0] = &GetRootNdsISlice()->front();

    intVector SubGrToConnectSizes;
    SubGrToConnectSizes.assign( numOfSubGrToConect, nIntNds );

#ifdef PAR_OMP    
    if( nIntNds > 1)
    {
	m_p1_5SliceJtreeInf = CParJtreeInfEngine::
	    Create( Get1_5SliceBNet(), numOfSubGrToConect,
	    &SubGrToConnectSizes.front(), &SubGrToConnect.front());
    }
    else
    {
	m_p1_5SliceJtreeInf = CParJtreeInfEngine::
	    Create( Get1_5SliceBNet() );
    }
#else
    if( nIntNds > 1)
    {
	m_p1_5SliceJtreeInf = CJtreeInfEngine::
	    Create( Get1_5SliceBNet(), numOfSubGrToConect,
	    &SubGrToConnectSizes.front(), &SubGrToConnect.front());
    }
    else
    {
	m_p1_5SliceJtreeInf = CJtreeInfEngine::
	    Create( Get1_5SliceBNet() );
    }
#endif

    PNL_CHECK_IF_MEMORY_ALLOCATED(m_p1_5SliceJtreeInf);

    m_p1_5SliceJtreeInf->GetClqNumsContainingSubset( nIntNds,
	&GetRootNdsISlice()->front(), &numOfCliques, &cliques );

    m_p1_5SliceJtreeInf->SetJTreeRootNode( cliques[0] );


    m_p1_5SliceJtreeInf->GetClqNumsContainingSubset( nIntNds,
	&GetIntNdsISlice()->front(), &numOfCliques, &cliques );
    m_interfaceClique = cliques[0];
}

C1_5SliceJtreeInfEngine::~C1_5SliceJtreeInfEngine()
{
    /////////////////////////////////////////////////////////////////////////
    //destructor
    /////////////////////////////////////////////////////////////////////////

    if( m_CRingJtreeInf.size() )
    {
#ifdef PAR_OMP 
	CRing<CParJtreeInfEngine *>::iterator beginIter = m_CRingJtreeInf.begin();
#else
	CRing<CJtreeInfEngine *>::iterator beginIter = m_CRingJtreeInf.begin();
#endif

	m_JTreeInfIter = beginIter;

	do
	{
	    if( *m_JTreeInfIter )
	    {
		//CJtreeInfEngine::Release(&(*m_JTreeInfIter));
                delete (*m_JTreeInfIter);
	    }

	    m_JTreeInfIter++;

	}while( m_JTreeInfIter!=beginIter );
    }

    //CJtreeInfEngine::Release(&m_pPriorSliceJtreeInf);
    delete m_pPriorSliceJtreeInf;

    //CJtreeInfEngine::Release(&m_p1_5SliceJtreeInf);
    delete m_p1_5SliceJtreeInf;

    if( m_CRingDistrOnSep.size() )
    {
	CRing<CPotential *>::iterator dequeIter = m_CRingDistrOnSep.begin();
	do
	{
	    if( *dequeIter != NULL )
	    {
		delete( *dequeIter );
	    }
	    dequeIter++;
	}while( dequeIter != m_CRingDistrOnSep.begin() );
    }
}

C1_5SliceJtreeInfEngine *
C1_5SliceJtreeInfEngine::Create( const CDynamicGraphicalModel *pGrModel )
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
    return new C1_5SliceJtreeInfEngine( pGrModel );
}


void C1_5SliceJtreeInfEngine::
DefineProcedure( EProcedureTypes procedureType, int lag )
{
    /////////////////////////////////////////////////////////////////////////
    // Selection procedure (smoothing, filtering, ...
    /////////////////////////////////////////////////////////////////////////

    // delete prevous potentials on separators if they exist
    CRing<CPotential *>::iterator dequeIter = m_CRingDistrOnSep.begin();
    if( m_CRingDistrOnSep.size() )
    {
	do
	{
	    if( *dequeIter )
	    {
		delete ( *dequeIter );
	    }
	    dequeIter++;
	}while( dequeIter != m_CRingDistrOnSep.begin() );
    }
    m_CRingDistrOnSep.clear();

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
    // initialization procedure (i.e. lag, ring of jtree inferences)

    switch( procedureType )
    {
    case ptFiltering:
	{
	    PNL_CHECK_FOR_NON_ZERO( lag );
	    m_Lag = 0;
	    m_CRingJtreeInf.assign( 1, NULL );
	    m_CRingDistrOnSep.assign( 1, NULL );
	    m_ProcedureType = ptFiltering;
	    break;
	}

	case ptPrediction:
	{
	    PNL_CHECK_FOR_NON_ZERO( lag );
	    m_Lag = 0;
	    m_ProcedureType = ptPrediction;
	    break;
	}

    case ptFixLagSmoothing:
	{
	    PNL_CHECK_LEFT_BORDER( lag, 0 );
	    m_Lag = lag;
	    m_CRingJtreeInf.assign( m_Lag + 1, NULL );
	    m_CRingDistrOnSep.assign( m_Lag + 1, NULL );
	    m_ProcedureType = procedureType;
	    break;
	}

    case ptSmoothing:
    case ptViterbi:
	{
	    PNL_CHECK_LEFT_BORDER( lag, 1 );
	    m_Lag = lag;
	    m_CRingJtreeInf.assign( m_Lag, NULL );
	    m_CRingDistrOnSep.assign( m_Lag + 1, NULL );
	    m_ProcedureType = procedureType;
	    break;
	}


    default:
	{
	    PNL_THROW( CInconsistentType, "incorrect type of inference procedure" );
	}
    }

    // initialize iterator for the jtree inferences sequence
    m_JTreeInfIter = m_CRingJtreeInf.begin();
    m_DistrOnSepIter = m_CRingDistrOnSep.begin();
    m_CurrentTime = 0;


}


void C1_5SliceJtreeInfEngine::ForwardFirst(const CEvidence *pEvidence, int maximize )
{
    //////////////////////////////////////////////////////////////////////////
    // forward step at time = 0
    //////////////////////////////////////////////////////////////////////////

    if( m_CurrentTime ||
	m_JTreeInfIter != m_CRingJtreeInf.begin() ||
	m_CRingpEv.size() != m_CRingJtreeInf.size() )
    {
	PNL_THROW(CAlgorithmicException, "it must be initial step")
    }

#ifdef PAR_OMP
    (*m_JTreeInfIter) = CParJtreeInfEngine::Copy(m_pPriorSliceJtreeInf);
    (*m_JTreeInfIter)->ShrinkObservedOMP( pEvidence, maximize );
    (*m_JTreeInfIter)->CollectEvidenceOMP();
#else
    (*m_JTreeInfIter) = CJtreeInfEngine::Copy(m_pPriorSliceJtreeInf);
    (*m_JTreeInfIter)->ShrinkObserved( pEvidence, maximize );
    (*m_JTreeInfIter)->CollectEvidence();
#endif

    ///////////////////////////////////////////////////////////////////////

    CPotential *pPotentialOnSep = NULL;

    intVector nonObsSubset;
    intVector indices;
    FindNonObsSubset(*GetIntNdsPriorSlice(), pEvidence, &nonObsSubset, &indices);
    if( nonObsSubset.size() )
    {
	(*m_JTreeInfIter)->
	    MarginalNodes( &nonObsSubset.front(), nonObsSubset.size(), 1 );


	const CPotential *queryPot;

	if( maximize )
	{
	    queryPot = (*m_JTreeInfIter)->GetQueryMPE();

	}
	else
	{
	    queryPot = (*m_JTreeInfIter)->GetQueryJPD();
	}

	int i;
	for( i = 0; i < nonObsSubset.size(); i++)
	{
	    nonObsSubset[i] = (*GetIntNdsISlice())[indices[i]];
	}

	pPotentialOnSep = static_cast<CPotential*> (CFactor::
	    CopyWithNewDomain(queryPot, nonObsSubset,
	    Get1_5SliceBNet()->GetModelDomain() ) );

    }

    if( *m_DistrOnSepIter )
    {
	delete( *m_DistrOnSepIter );

    }
    *m_DistrOnSepIter = pPotentialOnSep;

    m_CurrentTime++;
    //////////////////////////////////////////////////////////////////////////
}


void C1_5SliceJtreeInfEngine::Forward(const CEvidence *pEvidence, int maximize )
{
    //////////////////////////////////////////////////////////////////////////
    // forward step fot time > 0
    //////////////////////////////////////////////////////////////////////////
    if( !m_CurrentTime )
    {
	PNL_THROW(CAlgorithmicException, "it must be prior step");
    }
    if( m_CRingpEv.size() != m_CRingJtreeInf.size() )
    {
	PNL_THROW( CAlgorithmicException, "number of evidences" );
    }

    m_JTreeInfIter++;

    //////////////////////////////////////////////////////////////////////////
    //create jtree inference using obtained jtree and do collect
    //////////////////////////////////////////////////////////////////////////

#ifdef PAR_OMP 
    if( *m_JTreeInfIter )
    {
	if( (*m_JTreeInfIter)->GetModel()->GetNumberOfNodes() == GetPriorSliceBNet()->GetNumberOfNodes() )
	{
        //CJtreeInfEngine::Release(&(*m_JTreeInfIter));
        delete (*m_JTreeInfIter);
        (*m_JTreeInfIter) = NULL;
        (*m_JTreeInfIter) = CParJtreeInfEngine::Copy(m_p1_5SliceJtreeInf);
        (*m_JTreeInfIter)->ShrinkObservedOMP( pEvidence, maximize );
	}
	else
	{
        (*m_JTreeInfIter)->ShrinkObservedOMP( pEvidence, maximize );
	}
    }
    else
    {
	    (*m_JTreeInfIter) = CParJtreeInfEngine::Copy(m_p1_5SliceJtreeInf);
	    (*m_JTreeInfIter)->ShrinkObservedOMP( pEvidence, maximize );
    }
#else
    if( *m_JTreeInfIter )
    {
    	if( (*m_JTreeInfIter)->GetModel()->GetNumberOfNodes() == GetPriorSliceBNet()->GetNumberOfNodes() )
	    {
	        //CJtreeInfEngine::Release(&(*m_JTreeInfIter));
             delete (*m_JTreeInfIter);
	        (*m_JTreeInfIter) = NULL;
	        (*m_JTreeInfIter) = CJtreeInfEngine::Copy(m_p1_5SliceJtreeInf);
	        (*m_JTreeInfIter)->ShrinkObserved( pEvidence, maximize );
	    }
	    else
	    {

	        (*m_JTreeInfIter)->ShrinkObserved( pEvidence, maximize );
	    }
    }
    else
    {
	    (*m_JTreeInfIter) = CJtreeInfEngine::Copy(m_p1_5SliceJtreeInf);
	    (*m_JTreeInfIter)->ShrinkObserved( pEvidence, maximize );
    }
#endif


    if( *m_DistrOnSepIter )
    {
	intVector domain;
	(*m_DistrOnSepIter)->GetDomain(&domain);
	(*m_JTreeInfIter)->MultJTreeNodePotByDistribFun( m_interfaceClique,
	    &domain.front(), (*m_DistrOnSepIter)->GetDistribFun() );
    }
#ifdef PAR_OMP
    (*m_JTreeInfIter)->CollectEvidenceOMP();
#else
    (*m_JTreeInfIter)->CollectEvidence();
#endif

    m_CurrentTime++;

    //////////////////////////////////////////////////////////////////////////////
    CPotential *pPotentialOnSep = NULL;

    intVector nonObsSubset;
    intVector indices;

    FindNonObsSubset(*GetRootNdsISlice(), pEvidence, &nonObsSubset, &indices);

    if( nonObsSubset.size())
    {
	const CPotential *queryPot;
	(*m_JTreeInfIter)->MarginalNodes( &nonObsSubset.front(), nonObsSubset.size(), 1 );

	if( maximize )
	{
	    queryPot = (*m_JTreeInfIter)->GetQueryMPE();

	}
	else
	{
	    queryPot = (*m_JTreeInfIter)->GetQueryJPD();
	}

	///////////////////////////////////////////////////////////////////////

	pPotentialOnSep  = static_cast<CPotential*> (CFactor::
	    CopyWithNewDomain(queryPot, indices,
	    Get1_5SliceBNet()->GetModelDomain() ) );
    }

    m_DistrOnSepIter++;

    if( *m_DistrOnSepIter )
    {
	delete *m_DistrOnSepIter;

    }
    (*m_DistrOnSepIter) = pPotentialOnSep;

}

void C1_5SliceJtreeInfEngine::BackwardT()
{
    PNL_CHECK_IS_NULL_POINTER(*m_JTreeInfIter);
    PNL_CHECK_LEFT_BORDER(m_CurrentTime, 1);
    //////////////////////////////////////////////////////////////////////////
    // first backward step after last forward step
    //////////////////////////////////////////////////////////////////////////
#ifdef PAR_OMP
    (*m_JTreeInfIter)->DistributeEvidenceOMP();
#else
    (*m_JTreeInfIter)->DistributeEvidence();
#endif
}


void C1_5SliceJtreeInfEngine::BackwardFixLag()
{
    //////////////////////////////////////////////////////////////////////////
    // Backward step for fixed-lag smoothing procedure
    //////////////////////////////////////////////////////////////////////////
    PNL_CHECK_LEFT_BORDER(m_CurrentTime, m_Lag);

    if( m_Lag )
    {
	int currentTimeTmp = m_CurrentTime;

#ifdef PAR_OMP
    CRing<CParJtreeInfEngine *>::iterator m_JTreeInfIterTmp = m_JTreeInfIter;
	CRing<CPotential *>::iterator m_DistrOnSepIterTmp = m_DistrOnSepIter;

	CRing<CParJtreeInfEngine*> pCRingJtreeInfTmp;
	pCRingJtreeInfTmp.assign( 2,NULL );
	CRing<CParJtreeInfEngine *>::iterator JtreeInfTmpIter = pCRingJtreeInfTmp.begin();
	*JtreeInfTmpIter = CParJtreeInfEngine::Copy( *m_JTreeInfIter );
#else
	CRing<CJtreeInfEngine *>::iterator m_JTreeInfIterTmp = m_JTreeInfIter;
	CRing<CPotential *>::iterator m_DistrOnSepIterTmp = m_DistrOnSepIter;

	CRing<CJtreeInfEngine*> pCRingJtreeInfTmp;
	pCRingJtreeInfTmp.assign( 2,NULL );
	CRing<CJtreeInfEngine *>::iterator JtreeInfTmpIter = pCRingJtreeInfTmp.begin();
	*JtreeInfTmpIter = CJtreeInfEngine::Copy( *m_JTreeInfIter );
#endif

	CPotential *pPotTmp = NULL;
	BackwardT();
	int t = m_Lag;
	for( t = 0; t < m_Lag; t++ )
	{
	    if( t < m_Lag - 1 )
	    {
		JtreeInfTmpIter++;
#ifdef PAR_OMP
        *JtreeInfTmpIter = CParJtreeInfEngine::Copy(*(m_JTreeInfIter-1));
#else
        *JtreeInfTmpIter = CJtreeInfEngine::Copy(*(m_JTreeInfIter-1));
#endif
		JtreeInfTmpIter++;
	    }

	    ///////////////////////////
	    switch( (*(m_DistrOnSepIter-1))->GetDistributionType())
	    {
	    case dtTabular:
		{
		    pPotTmp = CTabularPotential::
			Copy(static_cast<CTabularPotential *>((*(m_DistrOnSepIter-1))));
		    break;
		}
	    case dtGaussian:
		{
		    pPotTmp = CGaussianPotential::
			Copy(static_cast<CGaussianPotential *>((*(m_DistrOnSepIter-1))));
		    break;
		}
	    case dtScalar:
		{
		    pPotTmp = CScalarPotential::
			Copy(static_cast<CScalarPotential *>((*(m_DistrOnSepIter-1))));
		    break;
		}
	    default:
		{
		    PNL_THROW( CInconsistentType, "distribution type" );
		}
	    }

	    /////////////////////////////////////
	    Backward();

	    if( t < m_Lag - 1 )
	    {
		//CJtreeInfEngine::Release(&(*(m_JTreeInfIter + 1)));
                delete (*(m_JTreeInfIter+1));

		*(m_JTreeInfIter+1) = *JtreeInfTmpIter;

		JtreeInfTmpIter++;
	    }

	    delete (*m_DistrOnSepIter);

	    (*m_DistrOnSepIter) = pPotTmp;
	}

	//CJtreeInfEngine::Release(&(*(m_JTreeInfIter+1)));
        delete (*(m_JTreeInfIter+1));

	*(m_JTreeInfIter+1) = *JtreeInfTmpIter;

	m_CurrentTime = currentTimeTmp;
	m_JTreeInfIter = m_JTreeInfIterTmp;
	m_DistrOnSepIter = m_DistrOnSepIterTmp;
    }
    else
    {
	BackwardT();
    }
}

void C1_5SliceJtreeInfEngine::Backward( int maximize )
{
    /////////////////////////////////////////////////////////////////////////
    // Take potential on interface clique contained interface nodes from
    // slice t+1, marginalize it, divide on potential on separator and
    // multiply it on the potential of root  clique from slice t.
    // Then do distribute evidence.
    /////////////////////////////////////////////////////////////////////////

    PNL_CHECK_LEFT_BORDER(m_CurrentTime, 1);

    m_DistrOnSepIter--;
    m_CurrentTime--;
    if( *m_DistrOnSepIter )
    {
	const CEvidence *pEvidence = (*m_JTreeInfIter)->GetEvidence();
	intVector nonObsSubset;
	intVector indices;
	FindNonObsSubset(*GetIntNdsISlice(), pEvidence, &nonObsSubset, &indices);


	(*m_JTreeInfIter)->
	    MarginalNodes( &nonObsSubset.front(), nonObsSubset.size(), 1 );

	/////////////////////////////////////////////////////////////
	const CPotential *queryPot;

	if( maximize )
	{
	    queryPot = (*m_JTreeInfIter)->GetQueryMPE();

	}
	else
	{
	    queryPot = (*m_JTreeInfIter)->GetQueryJPD();
	}



	CPotential *tmpPot = queryPot->Divide(*m_DistrOnSepIter);

	m_JTreeInfIter--;

	pEvidence = (*m_JTreeInfIter)->GetEvidence();
	const int *connectedNodes;
	if( m_CurrentTime - 1 )
	{
	    connectedNodes = &GetRootNdsISlice()->front();
	}
	else
	{
	    connectedNodes = &GetIntNdsPriorSlice()->front();

	}
	for( int i = 0; i < indices.size(); i++)
	{
	    nonObsSubset[i] = connectedNodes[indices[i]];
	}
	//final junction tree is

	int rootNode = (*m_JTreeInfIter)->GetJTreeRootNode();
	(*m_JTreeInfIter)->MultJTreeNodePotByDistribFun( rootNode, &nonObsSubset.front(),
	    tmpPot->GetDistribFun() );

	delete tmpPot;
    }
    else
    {
	m_JTreeInfIter--;
    }
    /////////////////////////////////////////////////////////////////////////
    // do distribute evidence for current jtree
#ifdef PAR_OMP
    (*m_JTreeInfIter)->DistributeEvidenceOMP();
#else
    (*m_JTreeInfIter)->DistributeEvidence();
#endif
}


const CPotential* C1_5SliceJtreeInfEngine::GetQueryJPD()
{
    if( m_ProcedureType != ptFiltering &&
	m_ProcedureType != ptSmoothing &&
	m_ProcedureType != ptFixLagSmoothing &&
	m_ProcedureType != ptPrediction )
    {
	PNL_THROW( CInvalidOperation,
	    " you have not been computing the Query JPD ");
    }
    if( !GetQueryPot() )
    {
	const CPotential* pQueryPot = m_CRingJtreeInf[m_QuerrySlice]->GetQueryJPD();
	intVector obsPos;
	pQueryPot->GetObsPositions(&obsPos);

	intVector queryNodes;
	GetQueryNodes(&queryNodes);
	SetQueryPot(static_cast<CPotential*>( CFactor::
	    CopyWithNewDomain(pQueryPot, queryNodes, GrModel()->GetModelDomain(), obsPos)));

    }
    return GetQueryPot();
}

const CPotential* C1_5SliceJtreeInfEngine::GetQueryMPE()
{
    if( m_ProcedureType != ptViterbi )
    {
	PNL_THROW( CInvalidOperation,
	    " you have not been computing the MPE ");
    }
    const CPotential* pQueryPot = m_CRingJtreeInf[m_QuerrySlice]->GetQueryMPE();
    intVector obsPos;
    pQueryPot->GetObsPositions(&obsPos);
    
    intVector queryNodes;
    GetQueryNodes(&queryNodes);
    SetQueryPot(static_cast<CPotential*>( CFactor::
	CopyWithNewDomain(pQueryPot, queryNodes, GrModel()->GetModelDomain(), obsPos)));
    return GetQueryPot();
    
}


const CEvidence* C1_5SliceJtreeInfEngine::GetMPE()
{
    if( m_ProcedureType != ptViterbi )
    {
	PNL_THROW( CInvalidOperation,
	    " you have not been computing the MPE ");
    }

    const CEvidence *pEv = m_CRingJtreeInf[m_QuerrySlice]->GetMPE();
    intVector queryNodes;
    GetQueryNodes(&queryNodes);
    SetEvidenceMPE( CEvidence::
	Create(pEv, queryNodes.size(), &queryNodes.front(), GrModel()->GetModelDomain()));
    return GetEvidenceMPE();
}

void C1_5SliceJtreeInfEngine::
MarginalNodes( const int *query, int querySize, int slice, int notExpandJPD )
{
    /////////////////////////////////////////////////////////////////////////
    PNL_CHECK_LEFT_BORDER(m_CRingpEv.size() , m_CRingJtreeInf.size());
    int nnodesPerSlice = GrModel()->GetNumberOfNodes();
    PNL_CHECK_RANGES( querySize, 0, 2*nnodesPerSlice);
    /////////////////////////////////////////////////////////////////////////
    intVector finalQuery;
    
    if( GetEvidenceMPE() )
    {
      delete GetEvidenceMPE();
      SetEvidenceMPE (NULL);
    }
    if( GetQueryPot() )
    {
      delete GetQueryPot();
      SetQueryPot(NULL);
    }

    SetQueryNodes(querySize, query);
    intVector queryVec;
    queryVec.assign(query, query + querySize);
    
    switch( m_ProcedureType )
    {
    case ptFiltering:
	case ptPrediction:
	{
	    m_QuerrySlice = 0;
	    FindFinalQuery( queryVec, m_CurrentTime - 1, &finalQuery);

	    (*m_CRingJtreeInf.begin())->MarginalNodes( &finalQuery.front(), querySize, notExpandJPD  );
	    break;
	}
    case ptFixLagSmoothing:
	{
	    m_QuerrySlice = m_CurrentTime - m_Lag - 1;
	    FindFinalQuery( queryVec, m_QuerrySlice, &finalQuery);

	    m_CRingJtreeInf[m_CurrentTime - m_Lag - 1]->MarginalNodes( &finalQuery.front(), querySize, notExpandJPD );
	    break;
	}
    case ptSmoothing:
    case ptViterbi:
	{
	    PNL_CHECK_RANGES(slice, 0, m_Lag);
	    m_QuerrySlice = slice;
	    FindFinalQuery( queryVec, slice, &finalQuery);
	    m_CRingJtreeInf[slice]->MarginalNodes( &finalQuery.front(), querySize, notExpandJPD );
	    break;
	}
    }
}


/*

CPotential*  C1_5SliceJtreeInfEngine::
ShrinkObserved( const CPotential* potToShrink, const CEvidence *evidence,
	       int isForvard, int maximize )
{
    intVector connectedNodesCurrent;
    CModelDomain *pMD;
    if( isForvard )
    {
	if( m_CurrentTime == 1 )
	{
	    connectedNodesCurrent = m_VectorIntNodesPriorSlice;
	    pMD = Get1_5SliceBNet()->GetModelDomain();
	}
	else
	{
	    connectedNodesCurrent = m_VectorIntNodesISlice;
	    pMD = Get1_5SliceBNet()->GetModelDomain();

	}

    }

    else
    {

	if( m_CurrentTime == 1 )
	{
	    connectedNodesCurrent = m_VectorIntNodesPriorSlice;
	    pMD = GetPriorSliceBNet()->GetModelDomain();
	}
	else
	{
	    connectedNodesCurrent = m_VectorRootNodesISlice;
	    pMD = Get1_5SliceBNet()->GetModelDomain();
	}

    }


    CPotential *tmpPot;
    tmpPot = static_cast<CPotential*>( CFactor::CopyWithNewDomain
	(potToShrink, connectedNodesCurrent, pMD));
    CPotential *pShrPot = tmpPot->ShrinkObservedNodes(evidence);
    delete tmpPot;
    return pShrPot;

}
*/
void C1_5SliceJtreeInfEngine::
EnterEvidence( const CEvidence * const* evidence, int nSlices )
{
    if(( m_ProcedureType == ptFiltering ||
	m_ProcedureType ==ptFixLagSmoothing ) &&
	nSlices != 1)
    {
	char* mess = "It is online procedure. Number of evidences must be equal to 1";
	PNL_THROW( COutOfRange, mess )
    }

    int size = m_CRingJtreeInf.size();
    CRing<CEvidence *>::iterator evidIter = m_CRingpEv.begin();
    evidIter += m_CurrentTime;
    int i;
    for( i= 0; i < nSlices; i++ )
    {
	CEvidence *pNewEvidence;
	const CEvidence *pCurrentEvidence = evidence[i];
	const int *oldObsNodes = pCurrentEvidence->GetAllObsNodes();
	int numObsNodes = pCurrentEvidence->GetNumberObsNodes();
	intVector newObsNodes(0);

	if( !m_CRingpEv.size() )
	{
	    pNewEvidence = CEvidence::Create( pCurrentEvidence,
		numObsNodes, oldObsNodes, GetPriorSliceBNet()->GetModelDomain() );
	    const int* flags = pCurrentEvidence->GetObsNodesFlags();
	    int node;
	    for( node = 0; node < numObsNodes; node++ )
	    {
		if( !flags[node] )
		{
		    pNewEvidence->MakeNodeHiddenBySerialNum( node );
		}
	    }

	}
	else
	{
	    valueVector itogObsValues;
	    const intVector* arrayOfNodes;
	    if( ( m_CRingpEv.size() == 1 &&
		(m_CurrentTime == 0 || m_CurrentTime == 1) ) ||
		( m_CRingpEv.size() > 1 && m_CurrentTime ==1 ) )
	    {
		arrayOfNodes = GetIntNdsPriorSlice();
	    }
	    else
	    {
		arrayOfNodes = GetRootNdsISlice();
	    }

		const CEvidence* ptmpEv;
		if( !m_CurrentTime  )
		{
		    ptmpEv = m_CRingpEv[m_CRingpEv.size()-1];
		}
		else
		{
		    ptmpEv = m_CRingpEv[m_CurrentTime-1];
		}
		const CModelDomain *ptmpMD = ptmpEv->GetModelDomain();
		int node;
		
		for( node = 0; node < GetNumIntNds(); node++ )
		{
		    int currentNode = (*arrayOfNodes)[node];
		    if( ptmpEv->IsNodeObserved( currentNode ) )
		    {
			newObsNodes.push_back( node );
			
			const Value* currentVal =
			    ptmpEv->GetValue( currentNode );
			int nvls = ptmpMD->GetVariableType(currentNode)->IsDiscrete() ?
			    1 : ptmpMD->GetVariableType(currentNode)->GetNodeSize();
			for( int ival = 0; ival < nvls; ival++ )
			{
			    itogObsValues.push_back( currentVal[ival] );
			}
			
		    }
		}
		ptmpEv = pCurrentEvidence;
		const int* ollObsNodes = ptmpEv->GetAllObsNodes();
		for( node = 0; node < ptmpEv->GetNumberObsNodes(); node++ )
		{
		    int currentNode = ollObsNodes[node];
		    if( ptmpEv->IsNodeObserved( currentNode ) )
		    {
			newObsNodes.push_back( currentNode + GetNumIntNds() );

			const Value* currentVal =
			    ptmpEv->GetValue( currentNode );
			int nvls = ptmpMD->GetVariableType(currentNode)->IsDiscrete() ?
			    1 : ptmpMD->GetVariableType(currentNode)->GetNodeSize();
			for( int ival = 0; ival < nvls; ival++ )
			{
			    itogObsValues.push_back( currentVal[ival] );
			}

		    }
		}

		pNewEvidence = CEvidence::Create(Get1_5SliceBNet()->GetModelDomain(),
		    newObsNodes, itogObsValues );
	
	}

	if( m_CRingpEv.size() < size )
	{
	    m_CRingpEv.push_back( pNewEvidence );
	    evidIter++;
	}
	else
	{
	    CEvidence *tmp = m_CRingpEv[m_CurrentTime];
	    if(tmp)
	    {
		delete tmp;
	    }
	    m_CRingpEv[m_CurrentTime] = pNewEvidence;
	}
    }
}

void C1_5SliceJtreeInfEngine::
FindNonObsSubset(const intVector &subset, const CEvidence *evidence,
		 intVector *nonObsSubset, intVector *indices)
{
    PNL_CHECK_IS_NULL_POINTER(nonObsSubset);
    PNL_CHECK_IS_NULL_POINTER(indices);
    nonObsSubset->clear();
    nonObsSubset->reserve(subset.size());
    indices->reserve(subset.size());

    int i=0;
    intVector::const_iterator it = subset.begin();
    intVector::const_iterator endIt = subset.end();
    for( ; it != endIt; it++, i++ )
    {
	if(!evidence->IsNodeObserved(*it))
	{
	    nonObsSubset->push_back(*it);
	    indices->push_back(i);
	}
    }
}

#ifdef PNL_RTTI
const CPNLType C1_5SliceJtreeInfEngine::m_TypeInfo = CPNLType("C1_5SliceJtreeInfEngine", &(C1_5SliceInfEngine::m_TypeInfo));

#endif
