/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlBNet.cpp                                                 //
//                                                                         //
//  Purpose:   CBNet class member functions implementation                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlBNet.hpp"
#include "pnlEvidence.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlRng.hpp"
#include <sstream>
#include <string>

PNL_USING

CBNet* CBNet::Create( int numberOfNodes, int numberOfNodeTypes,
		     const CNodeType *nodeTypes, const int *nodeAssociation,
		     CGraph *pGraph )
{
    /* bad-args check */
    PNL_CHECK_LEFT_BORDER( numberOfNodes, 1 );
#if 1
    PNL_CHECK_RANGES( numberOfNodeTypes, 1, numberOfNodes );
#else
    PNL_CHECK_LEFT_BORDER( numberOfNodeTypes, 1 );
#endif
    PNL_CHECK_IS_NULL_POINTER(nodeTypes);
    PNL_CHECK_IS_NULL_POINTER(nodeAssociation);
    /* bad-args check end */

    /* creating the model */
    CBNet *pBNet = new CBNet( numberOfNodes, numberOfNodeTypes,
	nodeTypes, nodeAssociation, pGraph );

    PNL_CHECK_IF_MEMORY_ALLOCATED(pBNet);

    /* graph validity check */
    if( !pBNet->GetGraph()->IsDAG() )
    {
	PNL_THROW( CInconsistentType, " the graph should be a DAG " );
    }

    if( !pBNet->GetGraph()->IsTopologicallySorted() )
    {
	PNL_THROW( CInconsistentType,
	    " the graph should be sorted topologically " );
    }
    /* graph validity check end */

    return pBNet;
}

CBNet* CBNet::Create( int numberOfNodes,
		     const nodeTypeVector& nodeTypes,
		     const intVector& nodeAssociation, CGraph *pGraph )
{
    return Create( numberOfNodes, nodeTypes.size(), &nodeTypes.front(),
	&nodeAssociation.front(), pGraph );
}


CBNet* CBNet::Create( CGraph *pGraph, CModelDomain* pMD )
{
    PNL_CHECK_IS_NULL_POINTER( pGraph );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    CBNet* pBNet = new CBNet( pGraph, pMD );

    PNL_CHECK_IF_MEMORY_ALLOCATED(pBNet);

    /* graph validity check */
    if( !pBNet->GetGraph()->IsDAG() )
    {
	PNL_THROW( CInconsistentType, " the graph should be a DAG " );
    }

    if( !pBNet->GetGraph()->IsTopologicallySorted() )
    {
	PNL_THROW( CInconsistentType,
	    " the graph should be sorted topologically " );
    }
    /* graph validity check end */

    return pBNet;
}

CBNet* CBNet::CreateWithRandomMatrices( CGraph* pGraph, CModelDomain* pMD )
{
    CBNet* pBNet = CBNet::Create( pGraph, pMD );
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pBNet);
    
    /* graph validity check */
    if( !pBNet->GetGraph()->IsDAG() )
    {
        PNL_THROW( CInconsistentType, " the graph should be a DAG " );
    }
    
    if( !pBNet->GetGraph()->IsTopologicallySorted() )
    {
        PNL_THROW( CInconsistentType,
            " the graph should be sorted topologically " );
    }
    /* graph validity check end */

    pBNet->AllocFactors();
    int numNodes = pBNet->GetNumberOfNodes();
    int i;
    for( i = 0; i < numNodes; i++ )
    {
        pBNet->AllocFactor( i );
        CFactor* ft = pBNet->GetFactor(i);
        ft->CreateAllNecessaryMatrices();
    }
    
    return pBNet;
}


CBNet* CBNet::Copy(const CBNet* pBNet)
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pBNet);
    // bad-args check end

    // creating the model
    CBNet *pCopyBNet = new CBNet(*pBNet);

    PNL_CHECK_IF_MEMORY_ALLOCATED(pCopyBNet);

    return pCopyBNet;
}

CBNet* CBNet::ConvertToSparse() const
{
    const CBNet* selfBNet = this;
    CBNet* pBnet = Copy( selfBNet );
    int numParams =  pBnet->m_pParams->GetNumberOfFactors();
    CFactor* pFactor;
    for( int i = 0; i < numParams; i++ )
    {
        pFactor = pBnet->GetFactor(i);
        pFactor->ConvertToSparse();
    }
    return pBnet;
}

CBNet* CBNet::ConvertToDense() const
{
    const CBNet* selfBNet = this;
    CBNet* pBnet = Copy( selfBNet );
    int numParams =  pBnet->m_pParams->GetNumberOfFactors();
    CFactor* pFactor;
    for( int i = 0; i < numParams; i++ )
    {
        pFactor = pBnet->GetFactor(i);
        pFactor->ConvertToDense();
    }
    return pBnet;
}

bool CBNet::IsValid( std::string* description ) const
{
    bool ret = 1;
    if( !GetGraph()->IsDAG() || !GetGraph()->IsTopologicallySorted()
        || ( GetGraph()->NumberOfConnectivityComponents() > 1 ) )
    {
        if( description )
        {
            std::stringstream st;
            st<<"Bayesian Network have invalid graph. The graph should be DAG,";
            st<<"should be topologically sorted,";
            st<<"and have one connectivity component."<<std::endl;
            std::string s = st.str();
            description->insert( description->begin(), s.begin(), s.end() );
        }
        ret = 0;
        return ret;
    }
    //if we haven't allocated factors - should type about it
    if( !m_pParams )
    {
        if(description)
        {
            std::stringstream st;
            st<< "The BNet haven't the storage allocated for factors and haven't any factors."<<std::endl;
            std::string s = st.str();
            description->insert(description->begin(), s.begin(), s.end());
        }
        ret = 0;
        return ret;
    }
    int numFactors = GetNumberOfFactors();
    int i;
    for( i = 0; i < numFactors; ++i )
    {
        const CFactor* pFact = GetFactor(i);
        
        if( !pFact )
        {
            if( description )
            {
                std::stringstream st;
                st<<"The Bayesian Network haven't factor for node "<<i<<"."<<std::endl;
                std::string s = st.str();
                description->insert( description->begin(), s.begin(), s.end() );
            }
            ret = 0;
            break;
        }
        else
        {
            if( !pFact->IsValid(description) )
            {
                if( description )
                {
                    std::stringstream st;
                    st<<"The Bayesian Network have invalid factor for node "<<i<<"."<<std::endl;
                    std::string s = st.str();
                    description->insert( description->begin(), s.begin(),
                        s.end() );
                }
                ret = 0;
                break;
            }
        }
    }
    if( GetNumberOfNodes() != numFactors)
    {
        if( description )
        {
            description->insert( 0,
                "The Bayesain Network haven't factors for every node.\n", 55 );
        }
        ret = 0;
    }
    return ret;
}

CBNet::CBNet( int numberOfNodes, int numberOfNodeTypes,
	     const CNodeType *nodeTypes, const int *nodeAssociation,
	     CGraph *pGraph ) : CStaticGraphicalModel( pGraph, numberOfNodes,
	     numberOfNodeTypes, nodeTypes, nodeAssociation )
{
    m_modelType = mtBNet;
}

CBNet::CBNet( CGraph *pGraph, CModelDomain* pMD )
:CStaticGraphicalModel( pGraph, pMD )
{
    m_modelType = mtBNet;
}

CBNet::CBNet(const CBNet& rBNet)
:CStaticGraphicalModel( CGraph::Copy(rBNet.m_pGraph),
		       rBNet.GetModelDomain() )
{
    m_modelType = mtBNet;

    if( rBNet.m_pParams )
    {
	m_pParams = CFactors::Copy(rBNet.m_pParams);

	m_paramInds.assign( rBNet.m_paramInds.begin(),
	    rBNet.m_paramInds.end() );

	m_pGraph->ProhibitChange();
    }
}

void CBNet::AllocFactor( int numberOfNodesInDomain, const int *domain )
{
    /* bad-args check */
    if( numberOfNodesInDomain <= 0 )
    {
	PNL_THROW( COutOfRange, " numberOfNodesInDomain > 0 " );
    }

    if( !domain )
    {
	PNL_THROW( CNULLPointer, " domain == NULL " );
    }

    if(	!IfDomainExists( numberOfNodesInDomain, domain) )
    {
	PNL_THROW( CInvalidOperation, " domain doesnt exist " );
    }

    _AllocFactor( domain[numberOfNodesInDomain - 1], numberOfNodesInDomain,
	domain, ftCPD );
}

void CBNet::AllocFactor(int childNodeNumber)
{
    /* bad-args check */
    PNL_CHECK_RANGES( childNodeNumber, 0, m_numberOfNodes - 1 );
    /* bad-args check end */

    int i;

    int                    numOfNbrs;
    const int              *nbrs;
    const ENeighborType *nbrsTypes;

    intVector domain;

    m_pGraph->GetNeighbors( childNodeNumber, &numOfNbrs, &nbrs, &nbrsTypes );

    for( i = 0; i < numOfNbrs; i++ )
    {
	if( nbrsTypes[i] == ntParent )
	{
	    domain.push_back(nbrs[i]);
	}
    }

    domain.push_back(childNodeNumber);

    _AllocFactor( domain[domain.size() - 1], domain.size() ,
	&domain.front(), ftCPD );

}

void CBNet::CreateTabularCPD( int childNodeNumber, const floatVector& matrixData )
{
    AllocFactor( childNodeNumber );

    CFactor* factor = GetFactor( childNodeNumber );
    factor->AllocMatrix( &matrixData.front(), matTable );

}

void CBNet::AttachFactor(CFactor *param)
{
/* this function is used for attaching factors that have been created
    already with the call of function for some type of factor Create() */

    /* bad-args check */
    if( !param )
    {
	PNL_THROW( CNULLPointer, " param == NULL " );
    }
    
    if( param->GetFactorType() != ftCPD )
    {
        PNL_THROW( CInconsistentType, "can attach only CPD to BNet");
    }

    int numberOfNodesInDomain;
    const int *domain;

    param->GetDomain( &numberOfNodesInDomain, &domain );

    if(	!IfDomainExists( numberOfNodesInDomain, domain) )
    {
	PNL_THROW( CInvalidOperation,
	    " parameter's domain doesn't exist in the model " );
    }
    
    //make calling AllocFactors() optional
    if( !m_pParams )
    {
        AllocFactors();
    }
    

    m_paramInds[domain[numberOfNodesInDomain - 1]] =
	m_pParams->AddFactor(param);
}

void CBNet::GetFactors( int numberOfNodes, const int *nodes,
		       int *numberOfFactors, CFactor ***params ) const
{
    /* bad-args check */
    if( numberOfNodes <= 0 )
    {
	PNL_THROW( COutOfRange, " numberOfNodes <= 0 " );
    }

    if( ( !nodes ) || ( !numberOfNodes ) || ( !params ) )
    {
	PNL_THROW( CNULLPointer, " nodes == NULL " );
	PNL_THROW( CNULLPointer, " numberOfNodes == NULL " );
	PNL_THROW( CNULLPointer, " params == NULL " );
    }
    /* bad-args check end */

    m_paramsForNodes.clear();

    if( GetFactors( numberOfNodes, nodes, &m_paramsForNodes ) )
    {
	(*numberOfFactors) = m_paramsForNodes.size();
	(*params) = &m_paramsForNodes.front();
	return;
    }

    (*numberOfFactors) = 0;
    (*params) = NULL;
}

int CBNet::GetFactors( int numberOfNodes, const int *nodes,
		      pFactorVector *params) const
{
/* the funtion is to find all the factors which are set on the domains
    which contain "nodes" as a subset */

    /* bad-args check */
    if( numberOfNodes <= 0 )
    {
	PNL_THROW( COutOfRange, " numberOfNodes <= 0 " );
    }

    if( ( !nodes ) || ( !params ) )
    {
	PNL_THROW( CNULLPointer, " nodes == NULL " );
	PNL_THROW( CNULLPointer, " params == NULL " );
    }
    /* bad-args check end */

    int i;
    CFactor *pParam;

    int                    numOfNbrs;
    const int              *nbrs;
    const ENeighborType *nbrsTypes;

    if(	numberOfNodes == 1 )
    {
	bool bFound;
	int node = (*nodes);
	int numberOfParents = 0;

	m_pGraph->GetNeighbors( node, &numOfNbrs, &nbrs, &nbrsTypes );

	if( numOfNbrs == 0 )
	{
	/* the node cant be isolated, therefore the number of its
	    neighbors should be positive */
	    PNL_THROW( COutOfRange, " numberOfNeighbors == 0 " );
	}

	bFound = false;

	for( i = 0; i < numOfNbrs; i++ )
	{
	    if( nbrsTypes[i] == ntNeighbor )
	    {
	    /* the type of the neighbor can be either ntParent or
		ntChild, since it is a Bayesian Network */
		PNL_THROW( CInconsistentType,
		    " nbrsTypes[i] should not be ntNeighbor " );
	    }

	    if( ( nbrsTypes[i] == ntParent ) && ( !bFound ) )
	    {
		bFound = true;
		numberOfParents++;

		pParam = m_pParams->GetFactor(m_paramInds[node]);
		params->push_back(pParam);
	    }
	    else if( nbrsTypes[i] == ntChild )
	    {
		/* means that the neighbor found is a child */
		pParam = m_pParams->GetFactor(m_paramInds[nbrs[i]]);
		params->push_back(pParam);
	    }
	}

	if( !numberOfParents )
	{
	    pParam = m_pParams->GetFactor(m_paramInds[node]);
	    params->push_back(pParam);
	}
    }
    else
    {
	int j;
	int counter;
	const int *location;
	intVector children;

	/* first find out if there is a domain with one of the nodes as
	a child and all the other nodes as parents */

	for( i = 0; i < numberOfNodes; i++ )
	{
	    m_pGraph->GetNeighbors( nodes[i], &numOfNbrs, &nbrs, &nbrsTypes );

	    if( numOfNbrs == 0 )
	    {
	    /* the node cant be isolated, therefore the number of its
		neighbors should be positive */
		PNL_THROW( COutOfRange, " numberOfNeighbors == 0 " );
	    }

	    counter = 0;

	    for( j = 0; j < numberOfNodes; j++ )
	    {
		if( j != i )
		{
		    location = std::find( nbrs,	nbrs + numOfNbrs, nodes[j] );

		    if( ( location != nbrs + numOfNbrs )
			&& ( nbrsTypes[location - nbrs] == ntParent ) )
		    {
			counter++;
		    }
		}
	    }

	    if( counter == numberOfNodes - 1 )
	    {
		pParam = m_pParams->GetFactor(m_paramInds[nodes[i]]);
		params->push_back(pParam);

		break;
	    }
	}

	/* now find all the domains in which all the nodes from the
	subset are parents of one node (meaning that all the nodes in
	the given subset are found in domain as parents	only)*/

	m_pGraph->GetNeighbors( nodes[0], &numOfNbrs, &nbrs, &nbrsTypes );

	for( i = 0; i < numOfNbrs; i++ )
	{
	    if( nbrsTypes[i] == ntChild )
	    {
		children.push_back(nbrs[i]);
	    }
	}

	for( i = 1; ( i < numberOfNodes ) && ( !children.empty() ); i++ )
	{
	    m_pGraph->GetNeighbors( nodes[i], &numOfNbrs, &nbrs, &nbrsTypes );

	    j = 0;

	    while( ( j < children.size() ) && ( !children.empty() ) )
	    {
		location = std::find( nbrs,	nbrs + numOfNbrs, children[j] );

		if( ( location == nbrs + numOfNbrs )
		    || ( ( location != nbrs + numOfNbrs )
		    && ( nbrsTypes[location - nbrs] != ntChild ) ) )
		{
		    children.erase(children.begin() + j);
		    --j;
		}

		++j;
	    }
	}

	for( i = 0; i < children.size(); i++ )
	{
	    pParam = m_pParams->GetFactor(m_paramInds[children[i]]);
	    params->push_back(pParam);
	}
    }

    if( params->empty() )
    {
	return 0;
    }

    return 1;
}

int CBNet::IfDomainExists( int numberOfNodesInDomain,
			   const int *domain) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( numberOfNodesInDomain, 1, m_numberOfNodes );
    PNL_CHECK_IS_NULL_POINTER(domain);
    /* bad-args check end */

    int i;

    int numOfParents = 0;

    int                    numOfNbrs;
    const int              *nbrs;
    const ENeighborType *nbrsTypes;

    /* the check consists of the following: find the neighbors for the
    childnode in the domain, create a domain for it using neighbors and
    check if the domain user inputs is the same as the one from graph */

    intVector tmp_domain( domain, domain + numberOfNodesInDomain );
    std::sort( tmp_domain.begin(), tmp_domain.end() - 1 );
    /* creating the pnlVector for domain storing and sorting it ascending
    makes it look like domain */

    m_pGraph->GetNeighbors( tmp_domain[numberOfNodesInDomain - 1],
	&numOfNbrs, &nbrs, &nbrsTypes );
    /* get neighbors for the child node in the user-set domain */

    for( i = 0; i < numOfNbrs; i++ )
    {
    /* look in the neighbors array to find a parent for the child
	node of the user-set domain */
	if( nbrsTypes[i] == ntParent )
	{
	    numOfParents++;

	    if( numOfParents > numberOfNodesInDomain - 1 )
	    {
		/* the real domain is larger than the user-set one */
		return 0;
	    }

	    if( nbrs[i] != tmp_domain[numOfParents - 1] )
	    {
		/* nodes in real domain and in the user-set one dont match */
		return 0;
	    }
	}
    }

    if( numOfParents < numberOfNodesInDomain - 1 )
    {
	/* the real domain is smaller than the user-set one */
	return 0;
    }

    /* if the "1" is returned then the domains exactly match */
    return 1;
}

/*
void CBNet::GenerateSamples( pEvidencesVector* evidences,
                            int nSamples, unsigned int seed ) const
{
    // generate random samples for BNet
    // evidences - vector of poimter to evidences which is generated
    // nSamples - number of samples
    srand(seed);
    if( nSamples <= 0)
    {
        PNL_THROW(CBadArg, "number of evidences must be positively")
    }
    (*evidences).assign(nSamples, NULL);

    const CBNet* selfBNet = this;
    int nnodes = selfBNet->GetNumberOfNodes();
    intVector obsNodes;

    const CNodeType * nt;
    valueVector obsVals;
    int numVals = 0;


    intVector mixtureNodes;
    const CFactor *pCPD;
    int i;
    for( i = 0; i < nnodes; i++)
    {
        pCPD = selfBNet->GetFactor(i);
        if( pCPD->GetDistributionType() == dtMixGaussian )
        {
            mixtureNodes.push_back(static_cast< const CMixtureGaussianCPD* >
                (pCPD)->GetNumberOfMixtureNode());
        }
    }


    int *leftIt  = mixtureNodes.begin();
    int *rightIt = mixtureNodes.end();
    int *location;
    for( i = 0; i < nnodes; i++)
    {
        location = std::find( leftIt, rightIt, i);
        if( location == rightIt )
        {
            obsNodes.push_back( i );
            nt = this->GetNodeType(i);
            if( nt->IsDiscrete() )
            {
                numVals++;
                obsVals.resize(numVals);
                obsVals[numVals-1].SetInt(0);
            }
            else
            {
                int sz = nt->GetNodeSize();
                obsVals.resize(numVals + sz);
                for( int j = 0; j < sz; j++ )
                {
                    obsVals[numVals + j].GetFlt() = 0.0f;
                }
                numVals += sz;
            }

        }
    }


    pEvidencesVector::iterator evidIter = evidences->begin();
    for( ; evidIter != evidences->end(); evidIter++ )
    {
        (*evidIter) = CEvidence::Create(selfBNet->GetModelDomain(), obsNodes, obsVals);
        (*evidIter)->ToggleNodeState( obsNodes.size(), &obsNodes.front() );
        for ( i = 0; i < obsNodes.size(); i++ )
        {
            int rnd = rand();
            GetFactor( obsNodes[i] )->GenerateSample( (*evidIter), rnd );
        }
    }
}*/


void CBNet::GenerateSamples( pEvidencesVector* evidences,
                            int nSamples, const CEvidence *pEvIn ) const
{
    // generate random samples for BNet
    // evidences - vector of poimter to evidences which is generated
    // nSamples - number of samples

    if( nSamples <= 0)
    {
        PNL_THROW(CBadArg, "number of evidences must be positively")
    }
    (*evidences).assign(nSamples, (CEvidence *const)NULL);

    const CBNet* selfBNet = this;
    int nnodes = selfBNet->GetNumberOfNodes();

    const CNodeType * nt;

    intVector mixtureNodes;
    const CFactor *pCPD;
    int i;
    for( i = 0; i < nnodes; i++)
    {
        pCPD = selfBNet->GetFactor(i);
        if( pCPD->GetDistributionType() == dtMixGaussian )
        {
            mixtureNodes.push_back(static_cast< const CMixtureGaussianCPD* >
                (pCPD)->GetNumberOfMixtureNode());
        }
    }



    intVector obsNodes(nnodes - mixtureNodes.size());
    intVector numVlsForNode(obsNodes.size());
    intVector obsPos;
    int numVls = 0;

    intVector::iterator leftIt  = mixtureNodes.begin();
    intVector::iterator rightIt = mixtureNodes.end();
    intVector::iterator location;
    int offset = 0;
    for ( i = 0; i < nnodes; i ++ )
    {
        location = std::find( leftIt, rightIt, i);
        if( location == rightIt )
        {
            obsNodes[offset] = i;
            nt = this->GetNodeType(i);
            if( nt->IsDiscrete() )
            {
                numVls++;
                numVlsForNode[offset] = 1;
            }
            else
            {
                int sz = nt->GetNodeSize();
                numVlsForNode[offset] = sz;
                numVls += sz;

            }
           offset++;
        }
    }

    valueVector obsVals( numVls, (Value)0 );
    valueVector::iterator vlsIter = obsVals.begin();

    intVector nodesForSampling;
    nodesForSampling.reserve(obsNodes.size());
    intVector serialNums;
    serialNums.reserve(obsNodes.size());

    if(pEvIn)
    {
        for ( i = 0; i < obsNodes.size(); i++  )
        {
            if( pEvIn->IsNodeObserved( obsNodes[i] ) )
            {
                memcpy( &*vlsIter, pEvIn->GetValue( obsNodes[i] ),
		    sizeof(Value)*numVlsForNode[i]);
		
            }
            else
            {
                nodesForSampling.push_back(obsNodes[i]);
		serialNums.push_back(i);
            }

            vlsIter += numVlsForNode[i];
        }
    }
    else
    {
        for ( i = 0; i < obsNodes.size(); i++  )
        {
	    serialNums.push_back(i);
	}
	nodesForSampling = obsNodes;
    }


    pEvidencesVector::iterator evidIter = evidences->begin();
    for( ; evidIter != evidences->end(); evidIter++ )
    {
        (*evidIter) = CEvidence::Create(selfBNet->GetModelDomain(), obsNodes, obsVals);

        (*evidIter)->ToggleNodeStateBySerialNumber( serialNums.size(), &serialNums.front() );

        for ( i = 0; i < nodesForSampling.size(); i++ )
        {

            GetFactor( nodesForSampling[i] )->GenerateSample( (*evidIter) );
        }

    }
}

float CBNet::ComputeLogLik(const CEvidence *pEv ) const
{
    if( GetModelDomain() != pEv->GetModelDomain() )
    {
        PNL_THROW(CBadArg, "different model domain")
    }
    int nnodes = GetNumberOfNodes();
    int nObsNodes = pEv->GetNumberObsNodes();
    //special case - summarize gaussian model
    intVector summarizeNodes;
    FindSummarizeNodes(&summarizeNodes);
    int numMixNodes = summarizeNodes.size();
    if( nObsNodes != nnodes - numMixNodes )
    {
        PNL_THROW( CNotImplemented, "all nodes must be observed" );
    }
    float ll = 0.0f;
    int i;
    for( i = 0; i < nnodes; i++ )
    {
        if( std::find( summarizeNodes.begin(), summarizeNodes.end(), i ) 
            == summarizeNodes.end())
        {
            if( !pEv->IsNodeObserved(i) )
            {
                PNL_THROW( CNotImplemented, "all nodes must be observed" );
            }
            else
            {
                ll += GetFactor( i )->GetLogLik( pEv );
            }
        }
    }
    return ll;
}

void CBNet::FindMixtureNodes(intVector* mixtureNds) const
{
    //finding mixture nodes
    PNL_CHECK_IS_NULL_POINTER( mixtureNds );
    int nnodes = GetNumberOfFactors();
    mixtureNds->reserve(nnodes);
    int i;
    for( i = 0; i < nnodes; i++)
    {
        if( GetFactor(i)->GetDistributionType() == dtMixGaussian )
        {
            mixtureNds->push_back(i);
        }
    }
}

void CBNet::FindSummarizeNodes(intVector* summarizeNodes) const
{
    PNL_CHECK_IS_NULL_POINTER(summarizeNodes);
    int nnodes = GetNumberOfFactors();
    summarizeNodes->reserve(nnodes);
    int i;
    CFactor* curFactor = NULL;
    for( i = 0; i < nnodes; i++)
    {
        curFactor = GetFactor(i); 
        if( curFactor->GetDistributionType() == dtMixGaussian )
        {
            summarizeNodes->push_back(static_cast<CMixtureGaussianCPD*>(
                curFactor)->GetNumberOfMixtureNode());
        }
    }
}

void CBNet::GetDiscreteParents(int nodeNum, intVector *discParents) const
{
    intVector parentOut;
    GetGraph()->GetParents(nodeNum, &parentOut);
    int i;
    const CNodeType *nodetype;
    for (i = 0; i < parentOut.size(); i++)
    {
        nodetype = GetModelDomain()->GetVariableType(parentOut[i]);
        if ( nodetype->IsDiscrete() )
            discParents->push_back(parentOut[i]);        
     }
}


void CBNet::GetContinuousParents(int nodeNum, intVector *contParents) const
{
    intVector parentOut;
    GetGraph()->GetParents(nodeNum, &parentOut);
    int i;
    const CNodeType *nodetype;
    for (i = 0; i < parentOut.size(); i++)
    {
        nodetype = GetModelDomain()->GetVariableType(parentOut[i]);
        if ( !(nodetype->IsDiscrete()))
            contParents->push_back(parentOut[i]);        
     }

}

#ifdef PNL_RTTI
const CPNLType CBNet::m_TypeInfo = CPNLType("CBNet", &(CStaticGraphicalModel::m_TypeInfo));

#endif