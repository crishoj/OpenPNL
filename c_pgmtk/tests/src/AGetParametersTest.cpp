/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGetParametersTest.cpp                                      //
//                                                                         //
//  Purpose:   Test for GetFactors member functions for models of all types//
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"

PNL_USING

static char  func_nameBNet[] = "testGetFactorsBNet";
static char  func_nameMNet[] = "testGetFactorsMNet";
static char  func_nameMRF2[] = "testGetFactorsMRF2";
static char* test_desc = "Checks if the params returned are on the subdomain";
static char* test_class = "Algorithm";

int NodesInParamDomain( int numberOfNodes, const int *nodes,
					    pFactorVector *params )
{
	int i, j;
	int node;

	const int *location;

	int domainSize;
	const int *domain;

	for( i = 0; i < numberOfNodes; i++ )
	{
		node = nodes[i];

		for( j = 0; j < params->size(); j++ )
		{
			(*params)[j]->GetDomain( &domainSize, &domain );

			location = std::find( domain, domain + domainSize, node );

			if( location == (domain + domainSize) )
			{
				return 0;
			}
		}
	}

	return 1;
}

int testGetFactorsBNet()
{
	int i;

	int ret = TRS_OK;

	const int nnodes = 7;
	const int numnt = 2;

	int *nodeAssociation;
	
    int numOfNeigh[] = { 2, 2, 2, 5, 3, 3, 3 };
	
	int neigh0[] = { 3, 6 };
    int neigh1[] = { 3, 4 };
	int neigh2[] = { 3, 4 };
	int neigh3[] = { 0, 1, 2, 5, 6 };
	int neigh4[] = { 1, 2, 5 };
	int neigh5[] = { 3, 4, 6 };
	int neigh6[] = { 0, 3, 5 };
	
	ENeighborType orient0[] = { ntChild, ntChild };
	ENeighborType orient1[] = { ntChild, ntChild };
	ENeighborType orient2[] = { ntChild, ntChild };
	ENeighborType orient3[] = { ntParent, ntParent, ntParent, ntChild, ntChild };
	ENeighborType orient4[] = { ntParent, ntParent, ntChild };
	ENeighborType orient5[] = { ntParent, ntParent, ntChild };
	ENeighborType orient6[] = { ntParent, ntParent, ntParent };
	
	int *neigh[] = { neigh0, neigh1, neigh2, neigh3, neigh4, neigh5, neigh6	};
	ENeighborType *orient[] = { orient0, orient1, orient2, orient3, orient4, orient5,
		orient6 };

	CGraph *graph;
	CBNet *bnet;
	CNodeType *nodeTypes;

	nodeTypes = new CNodeType [numnt];
	nodeAssociation = new int [nnodes];

	nodeTypes[0].SetType(1, 3);
	nodeTypes[1].SetType(1, 4);

	for( i = 0; i < nnodes; i++ )
	{
		nodeAssociation[i] = i%2;
	}

	graph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );
	

    bnet = CBNet::Create( nnodes, numnt, nodeTypes, nodeAssociation, graph );

	bnet->AllocFactors();

	for( i = 0; i < nnodes; i++ )
	{
		bnet->AllocFactor(i);
	}

	int numOfQueries = 28;
	int queryLength[] = { 1, 1, 1, 1, 1, 1,	1, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4 };
	int queries[28][4] =
	{
		{ 0 },
		{ 1 },
		{ 2 },
		{ 3 },
		{ 4 },
		{ 5 },
		{ 6 },
		{ 0, 1 },
		{ 0, 3 },
		{ 1, 3 },
		{ 1, 2 },
		{ 2, 3 },
		{ 4, 1 },
		{ 2, 4 },
		{ 3, 4 },
		{ 3, 5 },
		{ 5, 4 },
		{ 0, 6 },
		{ 6, 5 },
		{ 6, 3 },
		{ 0, 2, 3 },
		{ 3, 2, 1 },
		{ 5, 3, 4 },
		{ 6, 3, 0 },
		{ 1, 2, 3 },
		{ 3, 2, 1 },
		{ 6, 3, 5, 0 },
		{ 3, 1, 2, 0 }
	};

	pFactorVector params;

	for( i = 0; i < numOfQueries; i++ )
	{
		bnet->GetFactors( queryLength[i], queries[i], &params );
		
		/* check if the query nodes are all in the factor domain */
		if( !NodesInParamDomain( queryLength[i], queries[i], &params ) )
		{
			ret = TRS_FAIL;
			break;
		}

		params.clear();
	}
	
	delete [] nodeAssociation;
	delete [] nodeTypes;
	delete(bnet);

    return trsResult( ret, ret == TRS_OK ?
		"No errors" : "GetFactors FAILED");
}

int testGetFactorsMNet()
{
	int i;

	int ret = TRS_OK;

	const int nnodes = 7;
	const int numnt = 2;

	int *nodeAssociation;

	const int numberOfCliques = 4;
	int cliqueSizes[] = { 3, 3, 3, 4 };

	int clique0[] = { 1, 2, 3 };
	int clique1[] = { 0, 1, 3 };
	int clique2[] = { 4, 5, 6 };
	int clique3[] = { 0, 3, 4, 5 };
	
	const int *cliques[] = { clique0, clique1, clique2, clique3 };

	CMNet *mnet;
	CNodeType *nodeTypes;

	nodeTypes = new CNodeType [numnt];
	nodeAssociation = new int [nnodes];

	nodeTypes[0].SetType(1, 3);
	nodeTypes[1].SetType(1, 4);

	for( i = 0; i < nnodes; i++ )
	{
		nodeAssociation[i] = i%2;
	}


    mnet = CMNet::Create( nnodes, numnt,	nodeTypes, nodeAssociation,
		numberOfCliques, cliqueSizes, cliques );

	mnet->AllocFactors();

	for( i = 0; i < numberOfCliques; i++ )
	{
		mnet->AllocFactor(i);
	}

	int numOfQueries = 27;
	int queryLength[] = { 1, 1, 1, 1, 1, 1,	1, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4 };
	int queries[28][4] =
	{
		{ 0 },
		{ 1 },
		{ 2 },
		{ 3 },
		{ 4 },
		{ 5 },
		{ 6 },
		{ 0, 1 },
		{ 0, 3 },
		{ 1, 3 },
		{ 1, 2 },
		{ 2, 3 },
		{ 4, 3 },
		{ 0, 4 },
		{ 5, 4 },
		{ 3, 5 },
		{ 6, 4 },
		{ 0, 6 },
		{ 6, 5 },
		{ 0, 5 },
		{ 0, 1, 3 },
		{ 3, 2, 1 },
		{ 5, 3, 4 },
		{ 6, 4, 5 },
		{ 0, 4, 3 },
		{ 0, 4, 5 },
		{ 4, 3, 5, 0 },
	};

	pFactorVector params;

	for( i = 0; i < numOfQueries; i++ )
	{
		mnet->GetFactors( queryLength[i], queries[i], &params );

		/* check if the query nodes are all in the factor domain */
		if( !NodesInParamDomain( queryLength[i], queries[i], &params ) )
		{
			ret = TRS_FAIL;
			break;
		}

		params.clear();
	}
	
	delete [] nodeAssociation;
	delete [] nodeTypes;
	delete(mnet);

    return trsResult( ret, ret == TRS_OK ?
		"No errors" : "GetFactors FAILED");
}

int testGetFactorsMRF2()
{
	int i;

	int ret = TRS_OK;

	const int nnodes = 7;
	const int numnt = 2;

	int *nodeAssociation;

	const int numberOfCliques = 6;
	int cliqueSizes[] = { 2, 2, 2, 2, 2, 2 };

	int clique0[] = { 0, 1 };
	int clique1[] = { 1, 2 };
	int clique2[] = { 1, 3 };
	int clique3[] = { 2, 4 };
	int clique4[] = { 2, 5 };
	int clique5[] = { 3, 6 };
	
	const int *cliques[] = { clique0, clique1, clique2, clique3, clique4,
		clique5 };


	CMRF2 *mrf2;
	CNodeType *nodeTypes;

	nodeTypes = new CNodeType [numnt];
	nodeAssociation = new int [nnodes];

	nodeTypes[0].SetType(1, 3);
	nodeTypes[1].SetType(1, 4);

	for( i = 0; i < nnodes; i++ )
	{
		nodeAssociation[i] = i%2;
	}

    mrf2 = CMRF2::Create( nnodes, numnt, nodeTypes, nodeAssociation, 
		numberOfCliques, cliqueSizes, cliques );

	mrf2->AllocFactors();

	for( i = 0; i < numberOfCliques; i++ )
	{
		mrf2->AllocFactor(i);
	}

	int numOfQueries = 13;
	int queryLength[] = { 1, 1, 1, 1, 1, 1,	1, 2, 2, 2, 2, 2, 2 };
	int queries[13][2] =
	{
		{ 0 },
		{ 1 },
		{ 2 },
		{ 3 },
		{ 4 },
		{ 5 },
		{ 6 },
		{ 0, 1 },
		{ 1, 3 },
		{ 1, 2 },
		{ 4, 2 },
		{ 2, 5 },
		{ 6, 3 }
	};

	pFactorVector params;

	for( i = 0; i < numOfQueries; i++ )
	{
		mrf2->GetFactors( queryLength[i], queries[i], &params );

		/* check if the query nodes are all in the factor domain */
		if( !NodesInParamDomain( queryLength[i], queries[i], &params ) )
		{
			ret = TRS_FAIL;
			break;
		}

		params.clear();
	}
	
	delete [] nodeAssociation;
	delete [] nodeTypes;
	delete(mrf2);

    return trsResult( ret, ret == TRS_OK ?
		"No errors" : "GetFactors FAILED");
}

void initATestGetFactorsBNet()
{
    trsReg( func_nameBNet, test_desc, test_class, testGetFactorsBNet );
}

void initATestGetFactorsMNet()
{
    trsReg( func_nameMNet, test_desc, test_class, testGetFactorsMNet );
}

void initATestGetFactorsMRF2()
{
    trsReg( func_nameMRF2, test_desc, test_class, testGetFactorsMRF2 );
}