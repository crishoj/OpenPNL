/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnl1_5SliceInferenceEngine.cpp                              //
//                                                                         //
//  Purpose:                                                               //
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
#include "pnlMixtureGaussianCPD.hpp"

PNL_USING

C1_5SliceInfEngine::
C1_5SliceInfEngine( const CDynamicGraphicalModel *pDBN):
C2TBNInfEngine( pDBN ),
m_queryNodes(0),
m_VectorIntNodesPriorSlice(0),
m_VectorIntNodesISlice(0),
m_VectorRootNodesISlice(0),
m_nIntNodes(0)

{
    Initialization( pDBN );
}

C1_5SliceInfEngine::~C1_5SliceInfEngine()
{
    delete m_p1_5SliceBNet;
    m_p1_5SliceBNet = NULL;

    delete m_pPriorSliceBNet;
    m_pPriorSliceBNet = NULL;
}

//////////////////////////////////////////////////////////////////////////

void C1_5SliceInfEngine::Initialization(  const CDynamicGraphicalModel *pDBN )
{
    m_p1_5SliceBNet = Create1_5SliceBNet();
    PNL_CHECK_IF_MEMORY_ALLOCATED( m_p1_5SliceBNet );
    std::string description;
    if(! m_p1_5SliceBNet->IsValid( &description) )
    {
	PNL_THROW(CAlgorithmicException, description )
    }
    
    m_pPriorSliceBNet = static_cast<CBNet *>(pDBN->CreatePriorSliceGrModel());
    if(! m_p1_5SliceBNet->IsValid( &description) )
    {
	PNL_THROW(CAlgorithmicException, description)
    }
    PNL_CHECK_IF_MEMORY_ALLOCATED( m_pPriorSliceBNet );
    
    
    intVector interfNds;
    GrModel()->GetInterfaceNodes( &interfNds );
    int numInterfNds = interfNds.size();
    m_nIntNodes = numInterfNds;
    m_VectorIntNodesPriorSlice.assign(interfNds.begin(), interfNds.end());
    m_VectorIntNodesISlice.resize(numInterfNds);
    m_VectorRootNodesISlice.assign(interfNds.begin(), interfNds.end());
    
    int i;
    for( i = 0; i < numInterfNds; i++ )
    {
	m_VectorIntNodesISlice[i] = i;
	m_VectorRootNodesISlice[i] += numInterfNds;
    }
    
}

CBNet *C1_5SliceInfEngine::Create1_5SliceBNet()
{
    CGraph *p1_5SliceGraph = Create1_5SliceGraph();
    PNL_CHECK_IF_MEMORY_ALLOCATED(p1_5SliceGraph);
    intVecVector comp;
    p1_5SliceGraph->GetConnectivityComponents(&comp);
    PNL_CHECK_FOR_NON_ZERO(comp.size() -1 );
    nodeTypeVector nodeTypes;
    int nnodes = p1_5SliceGraph->GetNumberOfNodes();
    GrModel()->GetModelDomain()->GetVariableTypes(&nodeTypes);
    const int *nodeAssociatons = GrModel()->GetNodeAssociations();
    intVector FinalNodeAssociations;
    FinalNodeAssociations.resize(nnodes);
    int numberOfInterfaceNodes;
    const int *interfaceNodes;
    GrModel()->GetInterfaceNodes(&numberOfInterfaceNodes, &interfaceNodes);
    int nnodesPerSlice = GrModel()->GetNumberOfNodes();
    int node;
    for( node = 0; node < numberOfInterfaceNodes; node++ )
    {
	FinalNodeAssociations[node]= nodeAssociatons[interfaceNodes[node]];
    }

    for ( node = numberOfInterfaceNodes; node < nnodes; node++ )
    {
	FinalNodeAssociations[node]=
	    nodeAssociatons[nnodesPerSlice - numberOfInterfaceNodes + node];
    }

    CBNet *p1_5SliceGrModel = CBNet::Create( nnodes,	nodeTypes.size(),
	&nodeTypes.front(), &FinalNodeAssociations.front(), p1_5SliceGraph );

    p1_5SliceGrModel->AllocFactors();


    CFactor *pFactor;
    intVector domain(1);
    CFactor *pUnitFactor;

    for ( node = 0; node < numberOfInterfaceNodes; node++ )
    {
	domain[0] = node;
        if( GrModel()->GetNodeType(interfaceNodes[node])->IsDiscrete() )
	{

	    pUnitFactor =
                CTabularCPD::CreateUnitFunctionCPD( domain,
		p1_5SliceGrModel->GetModelDomain());
	}
	else
	{
            pUnitFactor =
                CGaussianCPD::CreateUnitFunctionCPD( domain,
		p1_5SliceGrModel->GetModelDomain());
	}
	PNL_CHECK_IF_MEMORY_ALLOCATED( pUnitFactor );

        p1_5SliceGrModel->AttachFactor( pUnitFactor );

    }

    for ( node = numberOfInterfaceNodes; node < nnodes; node++ )
    {
	domain.clear();
        p1_5SliceGraph->GetParents(node, &domain);
        domain.push_back(node);

        int num = nnodesPerSlice - numberOfInterfaceNodes + node;
       /*
        pFactor = CFactor::
       	    CopyWithNewDomain(GrModel()->GetFactor( num ), domain, p1_5SliceGrModel->GetModelDomain());
       	p1_5SliceGrModel->AttachFactor(pFactor );
       */
	pFactor = GrModel()->GetFactor(num);
	if( pFactor->GetDistributionType() == dtMixGaussian )
	{
	    floatVector prob;
	    static_cast<CMixtureGaussianCPD *>(pFactor)->GetProbabilities(&prob);
	    CMixtureGaussianCPD *pCPD = CMixtureGaussianCPD::Create(domain, p1_5SliceGrModel->GetModelDomain(), prob );
	    pCPD->TieDistribFun(pFactor);
	    p1_5SliceGrModel->AttachFactor(pCPD);

	}
	else
	{
		p1_5SliceGrModel->AllocFactor(node);
		p1_5SliceGrModel->GetFactor(node)->TieDistribFun(GrModel()->GetFactor(num));
	}
       
    }

    return p1_5SliceGrModel;
}


CGraph *C1_5SliceInfEngine::Create1_5SliceGraph()
{

    int node, i, j;

    CGraph *graph = GrModel()->GetGraph();

    int nnodesInDBN = graph->GetNumberOfNodes();

    int numberOfInterfaceNodes;
    const int *interfaceNodes;
    GrModel()->GetInterfaceNodes(&numberOfInterfaceNodes, &interfaceNodes);

    int nnodes = nnodesInDBN/2 + numberOfInterfaceNodes;
    CGraph *pFinalGraph = CGraph::Create( nnodes, NULL, NULL, NULL );
    PNL_CHECK_IF_MEMORY_ALLOCATED( pFinalGraph );

    int            numberOfNeighbors;
    const int             *neighbors;
    const ENeighborType *orientation;
    int newNumber;
    intVector                    FinalNeighbors;
    pnlVector<ENeighborType> FinalOrientation;

    intVector newIntNodes( numberOfInterfaceNodes );
    int numberOfNonIntNodes = nnodesInDBN/2 - numberOfInterfaceNodes;

    for ( node = 0; node < numberOfInterfaceNodes; node++ )
    {
	newIntNodes[node] = interfaceNodes[node] - numberOfNonIntNodes;
    }

    for( i = nnodesInDBN/2; i < nnodesInDBN; i++ )
    {
	graph->GetNeighbors(i, &numberOfNeighbors, &neighbors, &orientation);

	FinalNeighbors.resize(numberOfNeighbors);

	for ( j = 0; j < numberOfNeighbors; j++ )
	{
	    newNumber = neighbors[j] - numberOfNonIntNodes;
	    FinalNeighbors[j] = ( newNumber < numberOfInterfaceNodes ) ?
		( std::find( newIntNodes.begin(), newIntNodes.end(),
		newNumber) - newIntNodes.begin() ) : newNumber;
	}

	pFinalGraph->SetNeighbors( i - numberOfNonIntNodes, numberOfNeighbors,
	    &(FinalNeighbors.front()), orientation );
    }

    return pFinalGraph;
}

#ifdef PNL_RTTI
const CPNLType C1_5SliceInfEngine::m_TypeInfo = CPNLType("C1_5SliceInfEngine", &(C2TBNInfEngine::m_TypeInfo));

#endif