/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMNet.cpp                                                 //
//                                                                         //
//  Purpose:   CMNet class member functions implementation                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlMNet.hpp"
#include "pnlEvidence.hpp"
#include "pnlRng.hpp"
#include <sstream>

PNL_USING

CMNet* CMNet::Create( int numberOfCliques, const int *cliqueSizes,
		     const int **cliques, CModelDomain* pMD )
{
    /* bad-args check */
    PNL_CHECK_LEFT_BORDER( numberOfCliques, 1 );
    PNL_CHECK_IS_NULL_POINTER(cliqueSizes);
    PNL_CHECK_IS_NULL_POINTER(cliques);
    PNL_CHECK_IS_NULL_POINTER( pMD );
    /* bad-args check end */

    /* creating the model */
    CMNet *pMNet = new CMNet( numberOfCliques, cliqueSizes, cliques, pMD);

    PNL_CHECK_IF_MEMORY_ALLOCATED(pMNet);

    return pMNet;
}

CMNet* CMNet::Create( const intVecVector& clqs, CModelDomain* pMD )
{
    PNL_CHECK_IS_NULL_POINTER( pMD );

    int i;

    int numOfClqs = clqs.size();

    intVector clqsSizes(numOfClqs);

    pConstIntVector pClqs(numOfClqs);


    for( i = 0; i < numOfClqs; ++i )
    {
	clqsSizes[i] = clqs[i].size();

	pClqs[i] = &clqs[i].front();
    }

    return Create( numOfClqs, &clqsSizes.front(), &pClqs.front(), pMD );
}

CMNet* CMNet::Create( int numberOfNodes, int numberOfNodeTypes,
		     const CNodeType *nodeTypes, const int *nodeAssociation,
		     int numberOfCliques, const int *cliqueSizes,
		     const int **cliques )
{
    /* bad-args check */
    PNL_CHECK_LEFT_BORDER( numberOfNodes, 1 );
    PNL_CHECK_RANGES( numberOfNodeTypes, 1, numberOfNodes );
    PNL_CHECK_IS_NULL_POINTER(nodeTypes);
    PNL_CHECK_IS_NULL_POINTER(nodeAssociation);
    PNL_CHECK_LEFT_BORDER( numberOfCliques, 1 );
    PNL_CHECK_IS_NULL_POINTER(cliqueSizes);
    PNL_CHECK_IS_NULL_POINTER(cliques);
    /* bad-args check end */

    /* creating the model */
    CMNet *pMNet = new CMNet( numberOfNodes, numberOfNodeTypes,
	nodeTypes, nodeAssociation, numberOfCliques, cliqueSizes, cliques );

    PNL_CHECK_IF_MEMORY_ALLOCATED(pMNet);

    return pMNet;
}

CMNet* CMNet::Create( int numberOfNodes, const nodeTypeVector& nodeTypes,
		     const intVector& nodeAssociation,
		     const intVecVector& clqs )
{
    int i;

    int numOfClqs = clqs.size();

    intVector clqsSizes(numOfClqs);

    pConstIntVector pClqs(numOfClqs);


    for( i = 0; i < numOfClqs; ++i )
    {
	clqsSizes[i] = clqs[i].size();

	pClqs[i] = &clqs[i].front();
    }

    return Create( numberOfNodes, nodeTypes.size(), &nodeTypes.front(),
	&nodeAssociation.front(), numOfClqs, &clqsSizes.front(),
	&pClqs.front() );
}
CMNet* CMNet::CreateWithRandomMatrices( const intVecVector& clqs,
                                            CModelDomain* pMD)
{
    CMNet* pMNet = CMNet::Create( clqs, pMD );

    pMNet->AllocFactors();
    int numFactors = pMNet->GetNumberOfFactors();
    int i;
    for( i = 0; i < numFactors; i++ )
    {
        pMNet->AllocFactor( i );
        CFactor* ft = pMNet->GetFactor(i);
        ft->CreateAllNecessaryMatrices();
    }
    
    return pMNet;
}
//create mnet with random matrices
CMNet* CMNet::CreateWithRandomMatrices( int numberOfCliques,
                                        const int *cliqueSizes,
                                        const int **cliques,
                                        CModelDomain* pMD)
{
    CMNet* pMNet = CMNet::Create( numberOfCliques, cliqueSizes, cliques, pMD );
    
    pMNet->AllocFactors();
    int numFactors = pMNet->GetNumberOfFactors();
    int i;
    for( i = 0; i < numFactors; i++ )
    {
        pMNet->AllocFactor( i );
        CFactor* ft = pMNet->GetFactor(i);
        ft->CreateAllNecessaryMatrices();
    }
    
    return pMNet;
}


CMNet* CMNet::ConvertFromBNet(const CBNet* pBNet)
{
    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(pBNet);
    /* bad-args check end */

    CMNet *pMNet = new CMNet(pBNet);

    PNL_CHECK_IF_MEMORY_ALLOCATED(pMNet);

    return pMNet;
}

CMNet* CMNet::ConvertFromBNetUsingEvidence( const CBNet *pBNet,
					   const CEvidence *pEvidence )
{
    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(pBNet);
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    /* bad-args check end */

    CMNet *pMNet = new CMNet( pBNet, pEvidence );

    PNL_CHECK_IF_MEMORY_ALLOCATED(pMNet);

    return pMNet;
}


CMNet* CMNet::Copy(const CMNet* pMNet)
{
    CMNet *pCopyMNet = new CMNet(*pMNet);

    PNL_CHECK_IF_MEMORY_ALLOCATED(pCopyMNet);

    return pCopyMNet;
}

CMNet::CMNet( int numberOfCliques, const int *cliqueSizes,
	     const int **cliques, CModelDomain* pMD )
	     : CStaticGraphicalModel( NULL, pMD ), m_cliques(numberOfCliques),
	     m_clqsNumsForNds(m_numberOfNodes)
{
    m_modelType = mtMNet;

    // cliques initialization
    int i = 0;
    for( ; i < numberOfCliques; ++i )
    {
	intVector tmp_vec_clique( cliques[i], cliques[i] + cliqueSizes[i] );

	std::sort( tmp_vec_clique.begin(), tmp_vec_clique.end() );

	m_cliques[i] = tmp_vec_clique;
    }
    // cliques initialization end

    // graph construction from cliques
    ConstructGraphFromInternalCliques();

    // need to prohibit the graph changing, cause there is no
    // synchronization	implemented of the graph and cliques change
    m_pGraph->ProhibitChange();

    FillInClqsNumsForNdsFromInternalCliques();

    //m_pFactors.assign(numberOfCliques, 0);
}

CMNet::CMNet( int numberOfNodes, int numberOfNodeTypes,
	     const CNodeType *nodeTypes, const int *nodeAssociation,
	     int numberOfCliques, const int *cliqueSizes,
	     const int **cliques )
	     : CStaticGraphicalModel( NULL, numberOfNodes, numberOfNodeTypes,
	     nodeTypes, nodeAssociation), m_cliques(numberOfCliques),
	     m_clqsNumsForNds(numberOfNodes)
{
    m_modelType = mtMNet;

    /*
    CGraphicalModel* pObj = this;
    nodeTypeVector nt = nodeTypeVector( nodeTypes, nodeTypes + numberOfNodeTypes );
    intVector nAssociation = intVector( nodeAssociation,
    nodeAssociation + numberOfNodes );
    m_pMD = CModelDomain::Create( nt, nAssociation, pObj );
    */

    // cliques initialization
    int i = 0;
    for( ; i < numberOfCliques; ++i )
    {
	intVector tmp_vec_clique( cliques[i], cliques[i] + cliqueSizes[i] );

	std::sort( tmp_vec_clique.begin(), tmp_vec_clique.end() );

	m_cliques[i] = tmp_vec_clique;
    }
    // cliques initialization end

    // graph construction from cliques
    ConstructGraphFromInternalCliques();

    // need to prohibit the graph changing, cause there is no
    // synchronization	implemented of the graph and cliques change
    m_pGraph->ProhibitChange();

    FillInClqsNumsForNdsFromInternalCliques();
}

CMNet::CMNet(const CBNet *pBNet) : CStaticGraphicalModel(pBNet->GetModelDomain())
{
    m_modelType = mtMNet;

    // need to delete the old all-empty graph and attach a new-made one
    m_pGraph = CGraph::MoralizeGraph(pBNet->GetGraph());

    // find all the maximal cliques to set factors on to
    ConstructCliquesFromInternalGraph();

    // then should be factors allocation done, but since maximal cliques
    // finding is not implemented can finish la

}

CMNet::CMNet( const CBNet *pBNet, const CEvidence *pEvidence )
: CStaticGraphicalModel(pBNet->GetModelDomain())
{
    m_modelType = mtMNet;

    // need to delete the old all-empty graph and attach a new-made one
    m_pGraph = CGraph::MoralizeGraph(pBNet->GetGraph());

    ConstructCliquesFromInternalGraph();
}

CMNet::CMNet(const CMNet& rMNet):CStaticGraphicalModel( rMNet.GetModelDomain() )
{
    // need to copy Graph and factors
}

bool CMNet::IsValid(std::string* description)const
{
    int i;
    int numFactors = GetNumberOfFactors();
    
    bool ret = 1;
    for( i = 0; i < numFactors; ++i )
    {
        const CFactor* pFact = GetFactor(i);
        
        if( !pFact )
        {
            if( description )
            {
                std::stringstream st;
                st<<"The Markov Network haven't factor for group of nodes "<<i<<"."<<std::endl;
                std::string s = st.str();
                description->insert( description->begin(), s.begin(), s.end() );
            }
            ret = 0;
        }
        else
        {
            if( !pFact->IsValid(description) )
            {
                if( description )
                {
                    std::stringstream st;
                    st<<"The Markov Network have invalid potential for nodes:";
                    int domSize;
                    const int* dom;
                    pFact->GetDomain(&domSize, &dom);
                    int j;
                    for( j = 0; j < domSize; j++ )
                    {
                        st<< dom[j];
                    }
                    st<<std::endl;
                    std::string s = st.str();
                    description->insert( description->begin(),
                        s.begin(), s.end() );
                }
                ret = 0;
            }
        }
    }
    
    if(GetNumberOfCliques() != numFactors )
    {
        description->insert( 0,
                "The Markov Network haven't factors for every clique.\n", 55 );
        ret = 0;
    }
    return ret;
}

void CMNet::AllocFactor( int numberOfNodesInDomain, const int *domain )
{
    /* bad-args check */
    if( numberOfNodesInDomain <= 0 )
    {
	PNL_THROW( COutOfRange, " numberOfNodesInDomain <= 0 " );
    }

    if( !domain )
    {
	PNL_THROW( CNULLPointer, " domain == NULL " );
    }
    /* bad-args check end */

    int cliqueNumber;

    cliqueNumber = FindCliqueNumberByDomain( numberOfNodesInDomain, domain );

    if ( cliqueNumber == -1 )
    {
	PNL_THROW( CInvalidOperation, " no clique matching domain " );
	/* there is no clique to match the domain */
    }

    _AllocFactor( cliqueNumber, m_cliques[cliqueNumber].size(),
	&m_cliques[cliqueNumber].front(), ftPotential );
}

void CMNet::AllocFactor(int cliqueNumber)
{
    /* bad-args check */
    if( ( cliqueNumber < 0 ) || ( cliqueNumber >= m_cliques.size() ) )
    {
	PNL_THROW( COutOfRange,
	    " cliqueNumber < 0 or >= numberOfCliques " );
    }
    /* bad-args check end */

    _AllocFactor( cliqueNumber, m_cliques[cliqueNumber].size(),
	&m_cliques[cliqueNumber].front(), ftPotential );

}

void CMNet::AttachFactor(CFactor *param)
{
/* this function is used for attaching factors that have been created
    already with the call of function Create() for some type of fator */

    /* bad-args check */
    if( ! param )
    {
	PNL_THROW( CNULLPointer, " param == NULL " );
    }

    if( param->GetFactorType() != ftPotential )
    {
        PNL_THROW( CInconsistentType, "can attach only potential to MNet" );
    }

    int cliqueNumber;
    int numberOfNodesInDomain;
    const int *domain;

    param->GetDomain( &numberOfNodesInDomain, &domain );

    cliqueNumber = FindCliqueNumberByDomain( numberOfNodesInDomain, domain );

    if ( cliqueNumber == -1 )
    {
	PNL_THROW( CInvalidOperation, " no clique matching domain " );
	/* there is no clique to match the domain */
    }

    //make calling AllocFactors() optional
    if( !m_pParams )
    {
        AllocFactors();
    }

    m_paramInds[cliqueNumber] = m_pParams->AddFactor(param);

}

void CMNet::CreateTabularPotential( const intVector& domain,
        const floatVector& data )
{
    AllocFactor( domain.size(), &domain.front() );

    pFactorVector factors;
    int numFactors = GetFactors( domain.size(), &domain.front(), &factors );
    if( numFactors != 1 )
    {
        PNL_THROW( CInconsistentSize,
            "domain must be the same as corresponding domain size got from graph" );
    }
    factors[0]->AllocMatrix( &data.front(), matTable );
}

void CMNet::GetFactors( int numberOfNodes, const int *nodes,
					    int *numberOfFactors, CFactor ***params ) const
{
	// bad-args check
    PNL_CHECK_LEFT_BORDER( numberOfNodes, 1 );
    PNL_CHECK_IS_NULL_POINTER(nodes);
    PNL_CHECK_IS_NULL_POINTER(numberOfFactors);
    PNL_CHECK_IS_NULL_POINTER(params);
	// bad-args check end

	if( GetFactors( numberOfNodes, nodes, &m_paramsForNodes ) )
	{
		*numberOfFactors = m_paramsForNodes.size();
        *params             = &m_paramsForNodes.front();
	}
    else
    {
        *numberOfFactors = 0;
        *params             = NULL;
    }
}

int CMNet::GetFactors( int numberOfNodes, const int *nodes,
		               pFactorVector *params ) const
{
    // the function is to find all the factors which are set on
	// the domains	which contain "nodes" as a subset

	// bad-args check
    PNL_CHECK_LEFT_BORDER( numberOfNodes, 1 );
    PNL_CHECK_IS_NULL_POINTER(nodes);
    PNL_CHECK_IS_NULL_POINTER(params);
	// bad-args check end

    params->clear();


    int       numOfClqs;
    const int *clqsNums;

    GetClqsNumsForNode( *nodes, &numOfClqs, &clqsNums );

    assert( numOfClqs > 0 );

    intVector factsNums( clqsNums, clqsNums + numOfClqs );

    const int *ndsIt   = nodes + 1,
              *nds_end = nodes + numberOfNodes;

    for( ; ndsIt != nds_end; ++ndsIt )
    {
        GetClqsNumsForNode( *ndsIt, &numOfClqs, &clqsNums );

        intVector::iterator       factsNumsIt = factsNums.end() - 1;
        intVector::const_iterator factsNums_before_begin
            = factsNums.begin() - 1;

        for( ; factsNumsIt != factsNums_before_begin; --factsNumsIt )
        {
            if( std::find( clqsNums, clqsNums + numOfClqs, *factsNumsIt )
                == clqsNums + numOfClqs )
            {
                factsNums.erase(factsNumsIt);
            }

            if( factsNums.empty() )
            {
                return 0;
            }
        }
    }

    intVector::const_iterator factsNumsIt   = factsNums.begin(),
	factsNums_end = factsNums.end();

    for( ; factsNumsIt != factsNums_end; ++factsNumsIt )
    {
	params->push_back(GetFactor(*factsNumsIt));
    }

    return 1;
}

int CMNet::FindCliqueNumberByDomain( int numberOfNodesInDomain,
				    const int *domain ) const
{
    // looks in the clique vector to find the clique to match the input
    // domain exactly

    /* bad-args check */
    if( numberOfNodesInDomain <= 0 )
    {
	PNL_THROW( COutOfRange, " numberOfNodesInDomain <= 0 " );
    }

    if( !domain )
    {
	PNL_THROW( CNULLPointer, " domain == NULL " );
    }
    /* bad-args check end */

    int i, j;
    intVector tmp_domain(domain, domain + numberOfNodesInDomain);

    // the cliques are sorted ascending in the Graphical model,
    // so the domain should be sorted too to optimize the search
    std::sort( tmp_domain.begin(), tmp_domain.end() );

    for( i = 0; i < m_cliques.size(); i++ )
    {
	if((numberOfNodesInDomain == int(m_cliques[i].size()))
	    && ( tmp_domain[0] == m_cliques[i][0] )
	    && ( tmp_domain[numberOfNodesInDomain - 1]
	    == m_cliques[i][numberOfNodesInDomain - 1] ) )
	{
	    for( j = 1; ( j < numberOfNodesInDomain - 1 )
		&& ( tmp_domain[j] == m_cliques[i][j] ); j++ );

	    if( j == numberOfNodesInDomain - 1 )
	    {
		return i;
	    }
	}
    }

    return -1;
}

void CMNet::ConstructGraphFromInternalCliques()
{
    /* bad-args check */
    if( m_cliques.size() == 0 )
    {
	PNL_THROW( CInconsistentState,
	    " no cliques to construct the graph from " );
    }
    /* bad-args check end */

    intVecVector::const_iterator iter;
    intVector::const_iterator    iter1, iter2;

    /* constructing m_pGraph from the input set of cliques */
    for( iter = m_cliques.begin(); m_cliques.end() != iter; iter++ )
    {
	for( iter1 = iter->begin(); iter->end() - iter1 - 1; iter1++ )
	{
	    for( iter2 = iter1 + 1; iter->end() - iter2; iter2++ )
	    {
		m_pGraph->AddEdge( *iter1, *iter2, 0 );
	    }
	}
    }
}

void CMNet::ConstructCliquesFromInternalGraph()
{
    PNL_THROW( CNotImplemented, " and dont know if we need it at all " );
}

void CMNet::ConstructClqsFromBNetsFamiliesAndInternalGraph(const CBNet *pBNet)
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pBNet);
    // bad-args check end

}
void CMNet::GenerateSamples( pEvidencesVector* evidences, int nSamples, const CEvidence *pEvIn ) const
{
    // generate random samples for MNet
    // evidences - vector of poimter to evidences which is generated
    // nSamples - number of samples

    if( nSamples <= 0)
    {
        PNL_THROW(CBadArg, "number of evidences must be positively")
    }
    (*evidences).assign( nSamples, (CEvidence *)0 );

    const CMNet* selfMNet = this;
    int ncliques = selfMNet->GetNumberOfCliques();
    int nnodes = selfMNet->GetNumberOfNodes();
    intVector obsNodes(nnodes);
    intVector obsNdsSz(nnodes);
    CModelDomain *pMD = GetModelDomain();

    int nVls = 0;
    int i;
    for( i = 0; i < nnodes; i++ )
    {
        obsNodes[i] = i;
        obsNdsSz[i] = pMD->GetNumVlsForNode(i);
        nVls += obsNdsSz[i];

    }

    valueVector obsVls(nVls);
    valueVector::iterator iter = obsVls.begin();

    intVector ndsToToggle;
    ndsToToggle.reserve(nnodes);


    for( i = 0; i < nnodes; i++ )
    {
        int j;
        if( pEvIn && pEvIn->IsNodeObserved(i) )
        {

            const Value* pVal = pEvIn->GetValue(i);
            for( j = 0; j < obsNdsSz[i]; j++, iter++ )
            {
                *iter = pVal[j];
            }
        }
        else
        {
            ndsToToggle.push_back(i);
            bool isDiscr = pMD->GetVariableType(i)->IsDiscrete();
            if( isDiscr )
            {
                iter->SetInt(0);
                iter++;
            }
            else
            {
                for( j = 0; j < obsNdsSz[i]; j++, iter++ )
                {
                    iter->SetFlt(0.0f);
                }
            }
        }
    }


    pEvidencesVector::iterator evidIter = evidences->begin();
    for( ; evidIter != evidences->end(); evidIter++ )
    {
        (*evidIter) = CEvidence::Create(selfMNet->GetModelDomain(), obsNodes, obsVls);
        (*evidIter)->ToggleNodeState( ndsToToggle.size(), &ndsToToggle.front() );

        for ( i = 0; i < ncliques; i++ )
        {
            m_pParams->GetFactor( i )->GenerateSample( *evidIter );
        }
    }
}

#if 0
void CMNet::GenerateSamples( pEvidencesVector* evidences, int nSamples ) const
{
    // generate random samples for MNet
    // evidences - vector of poimter to evidences which is generated
    // nSamples - number of samples

    if( nSamples <= 0)
    {
        PNL_THROW(CBadArg, "number of evidences must be positively")
    }
    (*evidences).assign(nSamples, NULL);

    const CMNet* selfMNet = this;
    int ncliques = selfMNet->GetNumberOfCliques();

    int nnodes = selfMNet->GetNumberOfNodes();
    intVector obsNodes(nnodes);

    const CNodeType * nt;
    int nVals = 0;
    int i;
    for( i = 0; i < nnodes; i++ )
    {
        obsNodes[i] = i;
        nt = this->GetNodeType(i);
        if( nt->IsDiscrete() )
        {
            nVals++;
        }
        else
        {
            nVals += nt->GetNodeSize();

        }
    }

    valueVector obsVals(nVals);

    valueVector::iterator iter = obsVals.begin();

    for( i = 0; i < nnodes; i++ )
    {

        nt = this->GetNodeType(i);
        if( nt->IsDiscrete() )
        {
            iter->SetInt(0);
            iter++;
        }
        else
        {
            int j;
            for( j = 0; j < nt->GetNodeSize(); j++, iter++ )
            {
                iter->SetFlt(0.0f);

            }

        }
    }


    pEvidencesVector::iterator evidIter = evidences->begin();
    for( ; evidIter != evidences->end(); evidIter++ )
    {
        (*evidIter) = CEvidence::Create(selfMNet->GetModelDomain(), obsNodes, obsVals);
        (*evidIter)->ToggleNodeState( nnodes, &obsNodes.front() );
        for ( i = 0; i < ncliques; i++ )
        {
            m_pParams->GetFactor( i )->GenerateSample( *evidIter );
        }
    }
}
#endif

float CMNet::ComputeLogLik( const CEvidence *pEv ) const
{

    if( GetModelDomain() != pEv->GetModelDomain() )
    {
        PNL_THROW(CBadArg, "different model domain")
    }
    int nnodes = GetGraph()->GetNumberOfNodes();
    int nObsNodes = pEv->GetNumberObsNodes();
    if( nObsNodes != nnodes )
    {
        PNL_THROW(CNotImplemented, "all nodes must be observed")
    }

    const int* flags = pEv->GetObsNodesFlags();
    if( std::find( flags, flags + nnodes, 0 ) != flags + nnodes )
    {
        PNL_THROW( CNotImplemented, "all nodes must be observed" )
    }

    float ll = 0.0f;
    int i;
    for( i = 0; i < GetNumberOfCliques(); i++ )
    {
        ll += GetFactor( i )->GetLogLik( pEv );
    }
    return ll;
}

#ifdef PNL_RTTI
const CPNLType CMNet::m_TypeInfo = CPNLType("CMNet", &(CStaticGraphicalModel::m_TypeInfo));

#endif