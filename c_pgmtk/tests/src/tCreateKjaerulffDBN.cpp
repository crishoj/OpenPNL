/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      tCreateKjaerulffDBN.cpp                                     //
//                                                                         //
//  Purpose:   Implementation of the Kjaerulff DBN                         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "tCreateKjaerulffDBN.hpp"

PNL_USING

CDBN * tCreateKjaerulffDBN()
{
	
/*/////////////////////////////////////////////////////////////////////
An example of Dynamic Bayesian Network
Kjaerulff "dHugin: A computational system for dynamic time-sliced Bayesian 
networks",  Intl. J. Forecasting 11:89-111, 1995.

  The intra structure is (all arcs point downwards)
  
	0 ->1
	\ /
	2
	|
	3
    / \
	4   5
	\ /
	6
	|
	7
	
	  The inter structure is 0->0, 3->3, 7->7
	  
	*//////////////////////////////////////////////////////////////////////	
	
	int i;
	const int nnodes = 16;//Number of nodes
	int numNt = 1;//number of Node Types
	CNodeType *nodeTypes = new CNodeType [numNt];
	for( i=0; i < numNt; i++ )
	{
		nodeTypes[i] = CNodeType(1,2);//all nodes are discrete and binary
	}
	int nodeAssociation[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int numOfNeigh[] = { 3, 2, 3, 4, 2, 2, 3, 2, 3, 2, 3, 4, 2, 2, 3, 2};
    int neigh0[] = { 1, 2, 8 };
	int neigh1[] = { 0, 2 };
	int neigh2[] = { 0, 1, 3 };
	int neigh3[] = { 2, 4, 5, 11 };
	int neigh4[] = { 3, 6 };
	int neigh5[] = { 3, 6 };
	int neigh6[] = { 4, 5, 7 };
	int neigh7[] = { 6, 15 };
	int neigh8[] =  { 0, 9, 10 };
	int neigh9[] =  { 8, 10};
	int neigh10[] = { 8, 9, 11 };
	int neigh11[] = { 3, 10, 12, 13 };
	int neigh12[] = { 11, 14 };
	int neigh13[] = { 11, 14 };
	int neigh14[] = { 12, 13, 15 };
	int neigh15[] = { 7, 14 };
	
	ENeighborType orient0[] = { ntChild, ntChild, ntChild };
	ENeighborType orient1[] = { ntParent, ntChild };
	ENeighborType orient2[] = { ntParent, ntParent, ntChild };
	ENeighborType orient3[] = { ntParent, ntChild, ntChild, ntChild };
	ENeighborType orient4[] = { ntParent, ntChild };
	ENeighborType orient5[] = { ntParent, ntChild };
	ENeighborType orient6[] = { ntParent, ntParent, ntChild };
	ENeighborType orient7[] = { ntParent, ntChild };
	
	ENeighborType orient8[] = {  ntParent, ntChild, ntChild };
	ENeighborType orient9[] = {  ntParent, ntChild };
	ENeighborType orient10[] = { ntParent, ntParent, ntChild };
	ENeighborType orient11[] = { ntParent, ntParent, ntChild, ntChild };
	ENeighborType orient12[] = { ntParent, ntChild };
	ENeighborType orient13[] = { ntParent, ntChild };
	ENeighborType orient14[] = { ntParent, ntParent, ntChild };
	ENeighborType orient15[] = { ntParent, ntParent };
	
	int *neigh[] = { neigh0, neigh1, neigh2, neigh3, neigh4, neigh5, neigh6, 
		neigh7,	neigh8, neigh9, neigh10, neigh11, neigh12, neigh13, neigh14, 
		neigh15};
	ENeighborType *orient[] = { orient0, orient1, orient2, orient3, orient4, 
		orient5, orient6, orient7, orient8, orient9, orient10, orient11, 
		orient12, orient13, orient14, orient15};
	
	/////////////////////////////////////////////////////////////////////////////	
	CGraph* Graph = CGraph::Create( nnodes, numOfNeigh, neigh,
		orient);
	
	CBNet *myBNet = CBNet::Create(nnodes, numNt, nodeTypes, 
		nodeAssociation, Graph );
	/////////////////////////////////////////////////////////////////////////////
	myBNet->AllocFactors();
	int node;
	for (  node = 0; node < nnodes; node++ )
	{
		myBNet->AllocFactor( node );
		CFactor *factor = myBNet->GetFactor( node );
		int domSize;
		const int *domain;
		factor->GetDomain(&domSize, &domain);
		int prodDomSize = 1;
		for( i = 0; i< domSize; i++)
		{
			prodDomSize*=myBNet->GetNodeType(domain[i])->GetNodeSize();
		}
		floatVector prior;
		prior.resize(prodDomSize);
		
		
		for( i = 0; i < prodDomSize; i++ )
		{
			prior[i] = (float)(rand()%(10)) + 1.0f;
			
		}
		factor->AllocMatrix( &prior.front(), matTable );
		((CTabularCPD*)factor)->NormalizeCPD();
	}	
	/////////////////////////////////////////////////////////////////////////////
	//Create DBN 	
	/////////////////////////////////////////////////////////////////////////////
	CDBN *pDBN = CDBN::Create( myBNet );
        delete []nodeTypes;
   
	return pDBN;
		
}

