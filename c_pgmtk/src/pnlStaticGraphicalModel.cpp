/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlStaticGraphicalModel.cpp                                 //
//                                                                         //
//  Purpose:   CStaticGraphicalModel class member functions implementation //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlStaticGraphicalModel.hpp"
#include "pnlSoftMaxCPD.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlInferenceEngine.hpp"
#include "pnlTreeCPD.hpp"
#include <sstream>


PNL_USING


CStaticGraphicalModel::
CStaticGraphicalModel( CGraph *pGraph, CModelDomain* pMD ) : m_pParams(NULL),
                       m_paramInds(0), m_numberOfNodes(pMD->GetNumberVariables()),
                       m_pGraph( pGraph == NULL ?
					   CGraph::Create( m_numberOfNodes, NULL, NULL, NULL )
                       : pGraph ), CGraphicalModel(pMD)
{
	/* all the bad-args check is done on the upper level in the 
	pnlCreateXXX functions, so need to do some validity check only */
}

CStaticGraphicalModel::CStaticGraphicalModel(CModelDomain* pMD): 
                       CGraphicalModel(pMD),
                       m_pParams(NULL), m_pGraph(NULL), 
                       m_numberOfNodes(pMD->GetNumberVariables()),
                       m_paramInds(0), m_paramsForNodes(0)
{
}

CStaticGraphicalModel::CStaticGraphicalModel(CGraph* pGraph, int numberOfNodes,
                int numberOfNodeTypes, const CNodeType *nodeTypes,
                const int *nodeAssociation):CGraphicalModel( numberOfNodes,
                numberOfNodeTypes, nodeTypes, nodeAssociation ),
                m_pGraph( pGraph == NULL ? CGraph::Create( numberOfNodes, 
                NULL, NULL, NULL ) : pGraph ), m_pParams(NULL), 
                m_numberOfNodes(numberOfNodes),
                m_paramInds(0), m_paramsForNodes(0)
{
    
}

CStaticGraphicalModel::~CStaticGraphicalModel()
{
	delete m_pParams;
	delete m_pGraph;
}

CFactors* 
CStaticGraphicalModel::AttachFactors(CFactors *params)
{
	/* attaches the factors created beforehand with the call of function 
	pnlCreateFactors and returns the previous set of factors that were 
	attached to the model (if there were any) for the user to destroy it.
	If there were factors attached it returns NULL */

	/* bad-args check */
	PNL_CHECK_IS_NULL_POINTER(params);
	/* bad-args check end */

	CFactors *tmpParams = m_pParams;

	m_pParams = params;

	return tmpParams;
}

CFactor* CStaticGraphicalModel::GetFactor(int domainNum) const
{
	/* bad-args check */
	PNL_CHECK_RANGES( domainNum, 0, m_paramInds.size() - 1 );
	/* bad-args check end */

	/* find the appropriate number of the factor with the use of the
	domain number and return a pointer to the factor with this number
	from the factors vector */

	return m_pParams->GetFactor( m_paramInds[domainNum] );
}

int CStaticGraphicalModel::_AllocFactor( int domainNodes,
                                        int numberOfNodesInDomain,
					const int *domain,
					EFactorType paramType )
{
	/* bad-args check */
    PNL_CHECK_LEFT_BORDER( numberOfNodesInDomain, 1 );
    PNL_CHECK_IS_NULL_POINTER(domain);
    /* bad-args check end */
    
    EDistributionType distribType;
    
    pConstNodeTypeVector domNdsTypes;
    
    int i = 0;
    
    for( ; i < numberOfNodesInDomain; ++i )
    {
        domNdsTypes.push_back(GetNodeType(*(domain + i)));
    }
    
    distribType = pnlDetermineDistributionType( numberOfNodesInDomain, 0,
        NULL, &domNdsTypes.front() );
    
    CFactor* pFactor;
    
    switch (paramType)
    {
    case ftCPD:
        {
            switch (distribType)
            {			
            case dtTabular:
                {
                    pFactor = CTabularCPD::Create( domain,
                        numberOfNodesInDomain, m_pMD );
                    break;
                }
            case dtGaussian: case dtCondGaussian:
                {
                    pFactor = CGaussianCPD::Create( domain,
                        numberOfNodesInDomain, m_pMD );
                    break;
                }
            case dtTree :
                {
                    pFactor = CTreeCPD::Create( domain,
                        numberOfNodesInDomain, m_pMD );
                    break;
                }
            case dtSoftMax : case dtCondSoftMax:
                {
                    pFactor = CSoftMaxCPD::Create( domain,
                        numberOfNodesInDomain, m_pMD );
                    break;
                }
            default:
                {
                    PNL_THROW( CInvalidOperation,
                        " invalid distribution for pot creation " );
                }
            }
            break;
        }
    case ftPotential:
        {
            switch (distribType)
            {			
            case dtTabular:
                {
                    pFactor = CTabularPotential::Create( domain,
                        numberOfNodesInDomain, m_pMD );
                    break;
                }
            case dtGaussian:
                {
                    pFactor = CGaussianPotential::Create( domain,
                        numberOfNodesInDomain, m_pMD );
                    break;
                }
            default:
                {
                    PNL_THROW( CInvalidOperation,
                        " invalid distribution for pot creation " );
                }
            }
            break;
        }
    default:
        {
            PNL_THROW( CInvalidOperation,
                " invalid factor type for factor creation " );
        }
    }

    //make calling AllocFactors() optional
    if( !m_pParams )
    {
        AllocFactors();
    }
    
    m_paramInds[domainNodes] = m_pParams->AddFactor(pFactor);
    
    /* the function returns the index of the factor in the params array */
    /* will return the error code or the success code afterall */
    return m_paramInds[domainNodes];
}

bool CStaticGraphicalModel::IsValidAsBaseForDynamicModel(
                                 std::string* description) const
{
    if( GetModelType() != mtBNet )
    {
	std::stringstream st;
        st<<" support only oriented Dynamic model "<<std::endl;
        std::string s = st.str();
	description->insert( description->begin(), s.begin(), s.end() );
	return false;//support only oriented Dynamic model 
    }
    if( !IsValid() )
    {
        std::stringstream st;
	st<<" Static Bayesian Network is invalid "<<std::endl;
        std::string s = st.str();
        description->insert( description->begin(), s.begin(), s.end() );
        return false;
    }
    int nnodes = GetNumberOfNodes();
    if( nnodes % 2 )
    {
	std::stringstream st;
	st<<"Number of nodes in graphical model must even "<<std::endl;
        std::string s = st.str();
        description->insert( description->begin(), s.begin(), s.end() );
	return false;
    }
    
    CGraph *graph = GetGraph();
    if( graph->NumberOfConnectivityComponents() > 1 )
    {
        PNL_THROW( CBadArg, "The graph must be connected" )
    }
    int i = 0;
    int nnodesPerSlice = nnodes/2;
    for( ; i < nnodesPerSlice; i++ )
    {
        
        if( GetNodeType(i) != GetNodeType(i + nnodesPerSlice) )
        {
	    std::stringstream st;
	    st<<"Node types must be equal"<<std::endl;
            std::string s = st.str();
	    description->insert( description->begin(), s.begin(), s.end() );
	    return false;
        }
    }
    
    int                    numberOfNeighbors;
    const int              *neighbors;
    const ENeighborType *orientation;
    
    for( i = 0; i < nnodesPerSlice; i++)
    {
        graph->GetNeighbors(i, &numberOfNeighbors, &neighbors, &orientation);
        int j = 0;
        for( ; j < numberOfNeighbors; j++)
        {
            if( neighbors[j] >= nnodesPerSlice )
            {
                if( orientation[j] != ntChild )
		{
		    std::stringstream st;
		    st<<"Static graphical model can't have arcs from slice t to slice t-1"<<std::endl;
                    std::string s = st.str();
		    description->insert( description->begin(), s.begin(), s.end() );
		    return false;
		}
            }
        }
    }
    return true;
}

#ifdef PNL_RTTI
const CPNLType CStaticGraphicalModel::m_TypeInfo = CPNLType("CStaticGraphicalModel", &(CGraphicalModel::m_TypeInfo));

#endif