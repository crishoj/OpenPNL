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
#include "pnlFactorGraph.hpp"
#include "pnlEvidence.hpp"
#include "pnlPotential.hpp"
#include "pnlBNet.hpp"
#include "pnlMNet.hpp"
#include "pnlCPD.hpp"
#include <sstream>

PNL_USING

CFactorGraph* CFactorGraph::Create( CModelDomain* pMD, const CFactors* pFactors )
{
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_IS_NULL_POINTER(pFactors);
    int numFactors = pFactors->GetNumberOfFactors();
    //factor graph must be connected
    PNL_CHECK_LEFT_BORDER( numFactors, 1 );
    //check is all factors here are potentials
    int i;
    for( i = 0; i < numFactors; i++ )
    {
        if(( pFactors->GetFactor(i))->GetFactorType() != ftPotential )
        {
            PNL_THROW( CInconsistentType,
                "all factors for factor graph must be potentials" );
        }
    }

    CFactors* pFactorsHere = CFactors::Copy(pFactors);
    CFactorGraph* pResFG = new CFactorGraph( pMD, pFactorsHere );

    PNL_CHECK_IF_MEMORY_ALLOCATED( pResFG );
    return pResFG;
}

CFactorGraph* CFactorGraph::Create( CModelDomain* pMD, int numFactors )
{
    PNL_CHECK_IS_NULL_POINTER( pMD );
    //factor graph must be connected
    PNL_CHECK_LEFT_BORDER( numFactors, 1 );

    CFactorGraph* pResFG = new CFactorGraph( pMD, numFactors );
    PNL_CHECK_IF_MEMORY_ALLOCATED( pResFG );
    return pResFG;
}

CFactorGraph* CFactorGraph::Copy( const CFactorGraph* pFG)
{
    PNL_CHECK_IS_NULL_POINTER( pFG );

    CFactorGraph* resFG = new CFactorGraph( *pFG );
    PNL_CHECK_IF_MEMORY_ALLOCATED( pFG );
    return resFG;
}

CFactorGraph* CFactorGraph::ConvertFromBNet( const CBNet* pBNet)
{
    const int numOfFactors = pBNet->GetNumberOfFactors();

    CFactorGraph* pFactorGraph = CFactorGraph::Create( pBNet->GetModelDomain(),
        numOfFactors );

    int i = 0;

    for( ; i < numOfFactors; ++i )
    {
        pFactorGraph->AttachFactor((static_cast<CCPD*>(
            pBNet->GetFactor(i)))->ConvertToPotential());
    }

    return pFactorGraph;
}

CFactorGraph* CFactorGraph::ConvertFromMNet( const CMNet* pMNet )
{
    const int numOfFactors = pMNet->GetNumberOfFactors();

    CFactorGraph* pFactorGraph = CFactorGraph::Create( pMNet->GetModelDomain(),
        numOfFactors );

    int i = 0;

    for( ; i < numOfFactors; ++i )
    {
        pFactorGraph->AttachFactor(static_cast<CPotential*>(
            pMNet->GetFactor(i))->Clone());
    }

    return pFactorGraph;
}

int CFactorGraph::IsDomainFromMD( int numNodesInDomain, const int* domain) const
{
    int ret = 1;
    int numNodesInMD = GetModelDomain()->GetNumberVariables();
    for( int i = 0; i < numNodesInDomain; i++ )
    {
        if( domain[i] >= numNodesInMD )
        {
            ret = 0;
            break;
        }
    }
    return ret;
}

void CFactorGraph::AllocFactors()
{
    //do nothing - they are allocated in constructor
}

void CFactorGraph::AllocFactor( int numberOfNodesInDomain,
                                  const int *domain )
{
    if( numberOfNodesInDomain <= 0 )
    {
	    PNL_THROW( COutOfRange, " numberOfNodesInDomain > 0 " );
    }

    if( !domain )
    {
	    PNL_THROW( CNULLPointer, " domain == NULL " );
    }
    if( !IsDomainFromMD(numberOfNodesInDomain, domain) )
    {
        PNL_THROW( CInconsistentSize, "nodes not from model domain" );
    }
    _AllocFactor( m_numFactorsAllocated,
        numberOfNodesInDomain,	domain, ftPotential );
    //need to set information to list of neighbors
    for( int i = 0; i < numberOfNodesInDomain; i++ )
    {
        m_nbrFactors[domain[i]].push_back(m_numFactorsAllocated);
    }
    m_numFactorsAllocated++;
}
void CFactorGraph::AllocFactor(int number)
{
    PNL_THROW( CInvalidOperation,
        "can't allocate potential for factor without domain" );
}
void CFactorGraph::AttachFactor(CFactor *param)
{
    PNL_CHECK_IS_NULL_POINTER( param );
    if( param->GetFactorType() != ftPotential )
    {
        PNL_THROW( CInconsistentType,
            "all factors for factor graph must be potentials"  );
    }

    int numberOfNodesInDomain;
    const int *domain;

    param->GetDomain( &numberOfNodesInDomain, &domain );

    if(	!IsDomainFromMD( numberOfNodesInDomain, domain) )
    {
	PNL_THROW( CInvalidOperation,
	    " parameter's domain doesn't exist in the model " );
    }

    m_paramInds[m_numFactorsAllocated] = m_pParams->AddFactor(param);
    for( int i = 0; i < numberOfNodesInDomain; i++ )
    {
        m_nbrFactors[domain[i]].push_back(m_numFactorsAllocated);
    }
    m_numFactorsAllocated++;
}

bool CFactorGraph::IsValid(std::string* description) const
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
                st<<"The Factor Graph haven't factor for group of nodes "<<i<<"."<<std::endl;
                std::string s = st.str();
                description->insert( description->begin(), s.begin(), s.end() );
            }
            ret = 0;
        }
        else
        {
            if( !pFact->IsValid(description) )
            {
                std::stringstream st;
                st<<"The Factor Graph have invalid potential for nodes:";
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
                description->insert( description->begin(), s.begin(), s.end() );
                ret = 0;
            }
        }
    }
    //here should be the check of connectivity of model
    //check is there nbrs for every node
    int numNodes = GetNumberOfNodes();
    for( i = 0; i < numNodes; i++ )
    {
        if( GetNumNbrFactors(i) == 0 )
        {
            ret = 0;
            std::stringstream st;
            st<<"Node "<<i <<"is not in the domain of any potential."<<std::endl;
            std::string s = st.str();
            description->insert( description->begin(), s.begin(), s.end() );
            break;
        }
    }
    //need to check is there nbrs for every factor or group of factors
    return ret;
}

void
CFactorGraph::GetFactors(int numberOfNodes, const int* nodes,
			 int *numberOfFactors, CFactor ***params ) const
{
    PNL_CHECK_LEFT_BORDER( numberOfNodes, 1 );
    PNL_CHECK_IS_NULL_POINTER( nodes );
    PNL_CHECK_IS_NULL_POINTER( numberOfFactors ) ;
    PNL_CHECK_IS_NULL_POINTER( params );

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

int CFactorGraph::GetFactors( int numberOfNodes, const int* nodes,
			     pFactorVector *paramVec ) const
{
    PNL_CHECK_IS_NULL_POINTER( nodes );
    int numNodesInMD = GetModelDomain()->GetNumberVariables();
    PNL_CHECK_RANGES( numberOfNodes, 0, numNodesInMD );
    PNL_CHECK_IS_NULL_POINTER( paramVec );

    if( numberOfNodes == 1 )
    {
        int node = nodes[0];
        int numFactors = m_nbrFactors[node].size();
        paramVec->reserve(numFactors);
        for( int i = 0; i < numFactors; i++ )
        {
            paramVec->push_back(GetFactor( m_nbrFactors[node][i] ));
        }
        if( numFactors )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        assert( m_nbrFactors[nodes[0]].size()> 0 );

        intVector factsNums( m_nbrFactors[nodes[0]].begin(),
            m_nbrFactors[nodes[0]].end() );

        const int *ndsIt   = nodes + 1,
                  *nds_end = nodes + numberOfNodes;

        for( ; ndsIt != nds_end; ++ndsIt )
        {

            intVector::iterator       factsNumsIt = factsNums.end() - 1;
            intVector::const_iterator factsNums_before_begin
                = factsNums.begin() - 1;

            for( ; factsNumsIt != factsNums_before_begin; --factsNumsIt )
            {
                if( std::find( m_nbrFactors[*ndsIt].begin(),
                    m_nbrFactors[*ndsIt].end(), *factsNumsIt )
                    == m_nbrFactors[*ndsIt].end() )
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
	    paramVec->push_back(GetFactor(*factsNumsIt));
	}

        return 1;
    }

}

CFactorGraph* CFactorGraph::Shrink( const CEvidence* pEvidence)const
{
    //this method can be rewritten in more optimized form
    PNL_CHECK_IS_NULL_POINTER( pEvidence );

    CFactorGraph* resFG = CFactorGraph::Create( GetModelDomain(),
        m_pParams->GetNumberOfFactors() );    
    //go through the observed nodes and shrink all assigned factors
    int i, j;
    intVector obsNodes;
    pConstValueVector obsVals;
    pEvidence->GetObsNodesWithValues( &obsNodes, &obsVals );
    intVector numsOfShrinkedFactors;
    int numAllFactors = GetNumberOfFactors();
    numsOfShrinkedFactors.reserve(numAllFactors + 1);
    int numObsNds = obsNodes.size();
    intVector::iterator locInShrinked = intVector::iterator(0);
    for( i = 0; i < numObsNds; i++ )
    {
        int nodeNum = obsNodes[i];
        int numFactorsAssociatedWithIt = m_nbrFactors[nodeNum].size();
        for( j = 0; j < numFactorsAssociatedWithIt; j++ )
        {
            int factNum = m_nbrFactors[nodeNum][j];
            locInShrinked = std::find( numsOfShrinkedFactors.begin(),
                numsOfShrinkedFactors.end(), factNum ) ;
            if( locInShrinked == numsOfShrinkedFactors.end() )
            {
                const CFactor* curPot = GetFactor(factNum);
                CPotential* shrFactor = static_cast<const CPotential*>(curPot)->
                        ShrinkObservedNodes(pEvidence);
                numsOfShrinkedFactors.push_back(factNum);                
                resFG->AttachFactor(shrFactor);
            }
        }
    }
    if( int(numsOfShrinkedFactors.size()) == numAllFactors )
    {
        return resFG;
    }
    std::sort(numsOfShrinkedFactors.begin(), numsOfShrinkedFactors.end() );
    //add the last number 
    numsOfShrinkedFactors.push_back(numAllFactors);
    //need to attach all other factors (that wasn't shrinked)
    int t = 0;
    for( i = 0; i < numAllFactors; i++ )
    {
        int smallestNumAttached = numsOfShrinkedFactors[t];
        if( i < smallestNumAttached )                                                  
        {
            CFactor* curPot = GetFactor(i);
            resFG->AttachFactor((static_cast<CPotential*>(curPot))->Clone());
        }
        else
        {
            if( i == smallestNumAttached )
            {
                t++;
            }
            else
            {
                if( i > smallestNumAttached )
                {
                    CFactor* curPot = GetFactor(i);
                    resFG->AttachFactor((static_cast<CPotential*>(curPot))->Clone());
                }
            }
        }
    } 
    return resFG;
}

CFactorGraph::CFactorGraph(CModelDomain* pMD, CFactors* pFactors ):
                        CStaticGraphicalModel( pMD ),
                        m_nbrFactors( pMD->GetNumberVariables() ),
                        m_numFactorsAllocated( pFactors->GetNumberOfFactors() )
{
    m_modelType = mtFactorGraph;
    int i, j;
    AttachFactors( pFactors );
    int numFactors = pFactors->GetNumberOfFactors();
    //need to fill factor indices
    //and change the factor owner
    //fixme!
    m_paramInds.assign( numFactors, 0 );
    for( i = 0; i < numFactors; i++ )
    {
        m_paramInds[i] = i;
        pFactors->GetFactor(i)->ChangeOwnerToGraphicalModel();
    }
    //find indices to factors for every node
    int numNodes = pMD->GetNumberVariables();
    for( i = 0; i < numNodes; i++ )
    {
        m_nbrFactors[i].reserve( numNodes );
    }
    int domainSize;
    const int* domain;
    for( i = 0; i < m_numFactorsAllocated; i++ )
    {
        CFactor* pFactor = pFactors->GetFactor(i);
        pFactor->GetDomain( &domainSize, &domain );
        for( j = 0; j < domainSize; j++ )
        {
            m_nbrFactors[domain[j]].push_back(i);
        }
    }
}

CFactorGraph::CFactorGraph( CModelDomain* pMD, int numFactors ):
        CStaticGraphicalModel(pMD), m_nbrFactors(pMD->GetNumberVariables()),
        m_numFactorsAllocated(0)
{
    m_pParams = CFactors::Create(numFactors);
    m_paramInds.assign(numFactors, 0);
    m_modelType = mtFactorGraph;
}

CFactorGraph::CFactorGraph( const CFactorGraph& rFG)
: CStaticGraphicalModel( rFG.GetModelDomain() ),
  m_nbrFactors( rFG.GetNumberOfNodes() ),
  m_numFactorsAllocated(rFG.GetNumFactorsAllocated())
{
    m_modelType = mtFactorGraph;
    m_nbrFactors = rFG.m_nbrFactors;
    if( rFG.m_pParams )
    {
	m_pParams = CFactors::Copy(rFG.m_pParams);

	m_paramInds.assign( rFG.m_paramInds.begin(),
	    rFG.m_paramInds.end() );
    }
}

#ifdef PNL_RTTI
const CPNLType CFactorGraph::m_TypeInfo = CPNLType("CFactorGraph", &(CStaticGraphicalModel::m_TypeInfo));

#endif