/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDBN.cpp                                                  //
//                                                                         //
//  Purpose:   Implementation of the Dynamic Bayesian Network              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include <algorithm>
#include "pnlDBN.hpp"
#include "pnlBNet.hpp"
#include "pnlDistribFun.hpp"
#include "pnlMixtureGaussianCPD.hpp"

PNL_USING

CDBN::CDBN( EModelTypes modelType, CStaticGraphicalModel *pGrModel):
CDynamicGraphicalModel( modelType, pGrModel )
{
    if( ( pGrModel->GetModelType() ) != mtBNet)
    {
	PNL_THROW(CBadArg, "model must be BNet")
    }
}


CDBN * CDBN::Create(CStaticGraphicalModel *pGrModel)
{

    if(!pGrModel)
    {
	PNL_THROW(CNULLPointer, "Null pointer to static graphical model")
    }
    std::string description;
    if( !pGrModel->IsValidAsBaseForDynamicModel(&description) )
    {
	PNL_THROW( CBadArg, description );
    }
    CDBN *pDBN = new CDBN(mtDBN, pGrModel);
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    return pDBN;
}

CStaticGraphicalModel* CDBN::UnrollDynamicModel(int numberOfSlices) const
{
    if( numberOfSlices < 1  )
    {
	PNL_THROW( COutOfRange, " numberOfSlices < 1 " );

    }
    // bad-args check end

    CGraph *pGraph = GrModel()->GetGraph();
    int nnodesPerSlice = ( pGraph->GetNumberOfNodes() )/2;
    int nnodes = nnodesPerSlice*numberOfSlices;
    // nnodes is number of nodes in BNet created by unrolled
    // DBN for N=time slices

    CGraph *pFinalGraph;
    if( numberOfSlices == 1)
    {
	pFinalGraph = CreatePriorSliceGraph();
    }
    else
    {
	int i, j;
	/* creating an empty graph*/
	pFinalGraph = CGraph::Create( nnodes, NULL, NULL, NULL );

	/*finding list of neighbous for every node from prior slice*/
	int                    numberOfNeighbors;
	const int              *neighbors;
	const ENeighborType *orientation;

	for( i = 0; i < nnodesPerSlice; i++ )
	{
	    pGraph->GetNeighbors(i, &numberOfNeighbors, &neighbors,
		&orientation);

	    pFinalGraph->SetNeighbors(i, numberOfNeighbors, neighbors,
		orientation);
	}

	intVector FinalNeighbors;
	for( i = nnodesPerSlice; i < 2*nnodesPerSlice; i++ )
	{
	    pGraph->GetNeighbors(i, &numberOfNeighbors, &neighbors,
		&orientation);
	    FinalNeighbors.assign( neighbors, neighbors + numberOfNeighbors );

	    for( j = 0; j < numberOfSlices - 1 ; j++ )
	    {
		pFinalGraph->SetNeighbors( i + nnodesPerSlice*j,
		    numberOfNeighbors, &(FinalNeighbors.front()), orientation);

		for( int k = 0; k < numberOfNeighbors; k++ )
		{
		    FinalNeighbors[k] += nnodesPerSlice;
		}
	    }
	}
    }


    const CBNet *pBnet = static_cast<const CBNet *>(GrModel());

    nodeTypeVector nodeTypes;
    pBnet->GetModelDomain()->GetVariableTypes(&nodeTypes);

    const int *nodeAssociation = pBnet->GetNodeAssociations();
    int *FinalNodeAssociation = new int[nnodes];
    memcpy(FinalNodeAssociation, nodeAssociation, nnodesPerSlice*sizeof(int));

    int slice;
    for( slice = 1; slice < numberOfSlices; slice++)
    {
	memcpy( FinalNodeAssociation + nnodesPerSlice*slice,
	    nodeAssociation + nnodesPerSlice,
	    nnodesPerSlice*sizeof(int) );
    }

    CBNet *pFinalBnet = CBNet::Create( nnodes, nodeTypes.size(),
        &nodeTypes.front(), FinalNodeAssociation, pFinalGraph );
    pFinalBnet->AllocFactors();
    
    CFactor *pFactor;
    intVector domain;
    
    int node = 0;
    for ( ; node < nnodesPerSlice; node++ )
    {
	domain.clear();
	pFinalGraph->GetParents(node, &domain);
	domain.push_back(node);
	
	pFactor = CFactor::
	    CopyWithNewDomain( GrModel()->GetFactor(node), domain, pFinalBnet->GetModelDomain() );
	
	pFinalBnet->AttachFactor( pFactor );
    }
    
    for ( node = nnodesPerSlice; node < 2*nnodesPerSlice; node++ )
    {
	
	for( int j = 0; j < numberOfSlices - 1; j++)
	{
	    int number = node + j*nnodesPerSlice;
	    domain.clear();
	    pFinalGraph->GetParents(number, &domain);
	    domain.push_back(number);
	    
	    pFactor = CFactor::
		CopyWithNewDomain( GrModel()->GetFactor(node), domain, pFinalBnet->GetModelDomain() );
	    pFinalBnet->AttachFactor( pFactor );
	    
	}
	
    }
    
    delete []FinalNodeAssociation;
    return pFinalBnet;
}


CGraph *CDBN::CreatePriorSliceGraph() const
{
    /* creating graph corresponding to first slice of 2TBN */

    CGraph *graph = GrModel()->GetGraph();
    int nnodesInDBN = graph->GetNumberOfNodes();
    int nnodes = nnodesInDBN/2;

    CGraph *pFinalGraph = CGraph::Create( nnodes, NULL, NULL, NULL );

    int                    numberOfNeighbors;
    const int              *neighbors;
    const ENeighborType *orientation;

    intVector                     FinalNeighbors;
    neighborTypeVector FinalOrientation;

    int i = 0, j;

    for( ; i < nnodes; i++)
    {
	graph->GetNeighbors(i, &numberOfNeighbors, &neighbors, &orientation);

	for( j = 0; j < numberOfNeighbors; j++ )
	{
	    if( neighbors[j] < nnodes )
	    {
		FinalNeighbors.push_back(neighbors[j]);
		FinalOrientation.push_back(orientation[j]);
	    }
	}

	pFinalGraph->SetNeighbors(i, FinalNeighbors.size(),
	    &FinalNeighbors.front(), &FinalOrientation.front());

	FinalNeighbors.clear();
	FinalOrientation.clear();
    }

    return pFinalGraph;
}


/*
CStaticGraphicalModel *CDBN::CreatePriorSliceGrModel() const
{
	CGraph *pPriorSliceGraph = CreatePriorSliceGraph();
	int numberOfNodeTypes;
	const CNodeType *nodeTypes;
	int nnodes = pPriorSliceGraph->GetNumberOfNodes();
	GrModel()->GetNodeTypes(&numberOfNodeTypes, &nodeTypes);
	const int *nodeAssociatons = GrModel()->GetNodeAssociations();
	int *FinalNodeAssociations = new int[nnodes];

	for(int node = 0; node < m_nnodesPerSlice; node++ )
	{
		FinalNodeAssociations[node]= nodeAssociatons[node];
	}

	CBNet *pPriorSliceGrModel = CBNet::Create( nnodes, numberOfNodeTypes,
		nodeTypes, FinalNodeAssociations, pPriorSliceGraph );
	pPriorSliceGrModel->AllocFactors();
	CDistribFun *data;
	for ( node = 0; node < m_nnodesPerSlice; node++ )
	{
		data = GrModel()->GetFactor(node)->GetDistribFun();
		pPriorSliceGrModel->AllocFactor( node );
		(pPriorSliceGrModel->GetFactor( node )) ->SetDistribFun(data);
	}
	delete []FinalNodeAssociations;
	return (pPriorSliceGrModel);
}
*/
CStaticGraphicalModel *CDBN::CreatePriorSliceGrModel() const
{
    CGraph *pPriorSliceGraph = CreatePriorSliceGraph();

    nodeTypeVector nodeTypes;
    int nnodes = pPriorSliceGraph->GetNumberOfNodes();
    GrModel()->GetModelDomain()->GetVariableTypes(&nodeTypes);

    const int *nodeAssociatons = GrModel()->GetNodeAssociations();
    int *FinalNodeAssociations = new int[nnodes];


    memcpy( FinalNodeAssociations, nodeAssociatons, nnodes*sizeof(int));

    CBNet *pPriorSliceGrModel = CBNet::Create( nnodes, nodeTypes.size(),
        &nodeTypes.front(), FinalNodeAssociations, pPriorSliceGraph );

    pPriorSliceGrModel->AllocFactors();
    CFactor *pFactor;
    intVector domain;
    int node;
    for ( node = 0; node < m_nnodesPerSlice; node++ )
    {
        pFactor = GrModel()->GetFactor(node);
	if( pFactor->GetDistributionType() == dtMixGaussian )
	{
	    domain.clear();
	    pPriorSliceGraph->GetParents(node, &domain);
	    domain.push_back(node);
	    floatVector prob;
	    static_cast<CMixtureGaussianCPD *>(pFactor)->GetProbabilities(&prob);
	    CMixtureGaussianCPD *pCPD = CMixtureGaussianCPD::Create(domain, pPriorSliceGrModel->GetModelDomain(), prob );
	    pCPD->TieDistribFun(pFactor);
	    pPriorSliceGrModel->AttachFactor(pCPD);
	    
	}
	else
	{
	    
	/*
        domain.clear();
	pPriorSliceGraph->GetParents(node, &domain);
	domain.push_back(node);
        
	  pFactor = CFactor::
	  CopyWithNewDomain( GrModel()->GetFactor(node), domain, pPriorSliceGrModel->GetModelDomain() );
	  
	    pPriorSliceGrModel->AttachFactor( pFactor );
	    */
	    pPriorSliceGrModel->AllocFactor(node);
	    pPriorSliceGrModel->GetFactor(node)->TieDistribFun(GrModel()->GetFactor(node));
	    
	}
    }
    delete []FinalNodeAssociations;
    return (pPriorSliceGrModel);
}

void CDBN::GenerateSamples( pEvidencesVecVector* evidences,
			   const intVector& nSlices ) const
{
    PNL_CHECK_LEFT_BORDER(nSlices.size(), 1);

    evidences->resize( nSlices.size() );

    const CDBN *selfDBN = this;
    int nnodesInDBN = selfDBN->GetNumberOfNodes();

    const CStaticGraphicalModel *pBNet = selfDBN->GetStaticModel();
    intVector sliceObsNodes;
    valueVector sliceObsVals;
    int numVals = 0;
    const CNodeType *nt;

    intVector mixtureNodes;
    const CFactor *pCPD;
    int i;
    for( i = 0; i < nnodesInDBN; i++)
    {
        pCPD = selfDBN->GetFactor(i);
        if( pCPD->GetDistributionType() == dtMixGaussian )
        {
            mixtureNodes.push_back(static_cast< const CMixtureGaussianCPD* >
                (pCPD)->GetNumberOfMixtureNode());
        }
    }

    intVector::iterator leftIt  = mixtureNodes.begin();
    intVector::iterator rightIt = mixtureNodes.end();
    intVector::iterator location;

    for( i = 0; i < nnodesInDBN; i++)
    {
        location = std::find( leftIt, rightIt, i);
        if( location == rightIt )
        {
            sliceObsNodes.push_back(i);
            nt = selfDBN->GetNodeType(i);
            if( nt->IsDiscrete() )
            {
                numVals++;
                sliceObsVals.resize(numVals);
                sliceObsVals[numVals-1].SetInt(0);
            }
            else
            {
                int sz = nt->GetNodeSize();
                sliceObsVals.resize(numVals + sz);
                for( int j = 0; j < sz; j++ )
                {
                    sliceObsVals[numVals + j].SetFlt(0.0f);
                }
                numVals += sz;
            }
        }
    }

    valueVector obsValsForBNet( 2*sliceObsVals.size() );
    memcpy(&obsValsForBNet.front(), &sliceObsVals.front(), sliceObsVals.size()*sizeof(Value));
    memcpy(&obsValsForBNet.front() + sliceObsVals.size(), &sliceObsVals.front(),
        sliceObsVals.size()*sizeof(Value));

    int numObsNodesPerSlice = sliceObsNodes.size();
    intVector obsNodesForBNet( 2*numObsNodesPerSlice );
    memcpy(&obsNodesForBNet.front(), &sliceObsNodes.front(),numObsNodesPerSlice*sizeof(int));
    for( i = 0; i < numObsNodesPerSlice; i++ )
    {
        obsNodesForBNet[i + numObsNodesPerSlice] = sliceObsNodes[i] + nnodesInDBN;
    }

    CEvidence *pEv = CEvidence::Create( pBNet->GetModelDomain(), obsNodesForBNet, obsValsForBNet );
    pEv->ToggleNodeStateBySerialNumber(sliceObsNodes.size(), &obsNodesForBNet[numObsNodesPerSlice]);

    pEvidencesVecVector::iterator seriesIter = evidences->begin();
    intVector::const_iterator nslicesIter = nSlices.begin();
    pEvidencesVector::iterator sliceIter;

    for( ; seriesIter != evidences->end(); seriesIter++, nslicesIter++ )
    {
        seriesIter->assign( 1,
            CEvidence::Create( selfDBN->GetModelDomain(), sliceObsNodes, sliceObsVals ) );

        (*seriesIter)[0]->ToggleNodeStateBySerialNumber(sliceObsNodes);

        for ( i = 0; i < numObsNodesPerSlice; i++ )
        {
            selfDBN->GetFactor( sliceObsNodes[i] )->GenerateSample( (*seriesIter)[0] );
        }

        memcpy(pEv->GetValue(sliceObsNodes[0]),(*seriesIter)[0]->GetValue(sliceObsNodes[0]),
            numObsNodesPerSlice*sizeof(Value));

        if( *nslicesIter <= 0)
        {
            PNL_THROW(CBadArg, "number of slices must be not less then 1");
        }
        seriesIter->resize(*nslicesIter);
        sliceIter = (*seriesIter).begin() + 1;
        for( ; sliceIter != (*seriesIter).end(); sliceIter++ )
        {
            for ( i = 0; i < numObsNodesPerSlice; i++ )
            {
                pBNet->GetFactor( sliceObsNodes[i] + nnodesInDBN  )->GenerateSample( pEv );
            }
            (*sliceIter) = CEvidence::Create( selfDBN->GetModelDomain(),
                sliceObsNodes, sliceObsVals );

            memcpy((*sliceIter )->GetValue(sliceObsNodes[0]),
                pEv->GetValue(sliceObsNodes[0] + nnodesInDBN),
                numObsNodesPerSlice*sizeof(Value));

            memcpy(pEv->GetValue(sliceObsNodes[0]),pEv->GetValue(sliceObsNodes[0]+nnodesInDBN),
                numObsNodesPerSlice*sizeof(Value));

            pEv->ToggleNodeStateBySerialNumber(sliceObsNodes.size(), &obsNodesForBNet[numObsNodesPerSlice]);

        }
    }
    delete pEv;
}

#ifdef PNL_RTTI
const CPNLType CDBN::m_TypeInfo = CPNLType("CDBN", &(CDynamicGraphicalModel::m_TypeInfo));

#endif
