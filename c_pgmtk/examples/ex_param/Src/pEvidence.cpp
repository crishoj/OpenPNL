/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pEvidence.cpp                                               //
//                                                                         //
//  Purpose:   Showing of CEvidence class functionality                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"

//#include "pnlInfEngine.hpp"


int main()
{
    PNL_USING
    int i;
	int nnodes = 7;
	int numnt = 2;
	int *nodeAssociation;
	int numOfNeigh[] = { 3, 3, 4, 5, 2, 3, 2 };
	
    int neigh1[] = { 1, 2, 3 };
	int neigh2[] = { 0, 2, 3 };
	int neigh3[] = { 0, 1, 3, 6 };
	int neigh4[] = { 0, 1, 2, 4, 5 };
	int neigh5[] = { 3, 5 };
	int neigh6[] = { 3, 4, 6 };
	int neigh7[] = { 2, 5 };
	
	ENeighborType orient1[] = { ntChild, ntChild, ntChild };
	ENeighborType orient2[] = { ntParent, ntChild, ntChild };
	ENeighborType orient3[] = { ntParent, ntParent, ntChild, ntChild };
	ENeighborType orient4[] = { ntParent, ntParent, ntParent, ntChild, ntChild };
	ENeighborType orient5[] = { ntParent, ntChild };
	ENeighborType orient6[] = { ntParent, ntParent, ntChild };
	ENeighborType orient7[] = { ntParent, ntParent };
	
	const int *neigh[] = { neigh1, neigh2, neigh3, neigh4, neigh5, neigh6, neigh7 };
	const ENeighborType *orient[] = { orient1, orient2, orient3, orient4, orient5, orient6, orient7 };
	
	EModelTypes type;
	CBNet *mn;
	
	CNodeType *nodeTypes = new CNodeType [numnt];
//	CNodeType *nodeTypes_tmp = new CNodeType [numnt];
	nodeAssociation = new int [nnodes];
	
	nodeTypes[0].SetType( 1, 3 );
	nodeTypes[1].SetType( 0, 3 );
	
	nodeAssociation[0] = 0;
	nodeAssociation[1] = 1;
	nodeAssociation[2] = 0;
	nodeAssociation[3] = 0;
	nodeAssociation[4] = 0;
	nodeAssociation[5] = 1;
	nodeAssociation[6] = 0;
	
	type = mtBNet;

    CGraph* graph = CGraph::Create( nnodes, NULL );
    graph->AddEdge( 2, 6, 1 );
	graph->AddEdge( 5, 6, 1 );
	for( i = 0; i < nnodes - 1; i++ )
	{
		graph->SetNeighbors( i, numOfNeigh[i], neigh[i], orient[i] );
	}
	graph->Dump();
	
	mn = CBNet::Create( nnodes, numnt, nodeTypes, nodeAssociation, NULL );
		
//	mn->SayIt();
	
	graph->AddEdge( 2, 6, 1 );
	graph->AddEdge( 5, 6, 1 );
	for( i = 0; i < nnodes - 1; i++ )
	{
		graph->SetNeighbors( i, numOfNeigh[i], neigh[i], orient[i] );
	}
	mn->GetGraph()->Dump();
	//getchar();
	
	/*add evidence to this model*/
	int *obsNodes = new int[3];
	obsNodes[0] = 0;
	obsNodes[1] = 1;
	obsNodes[2] = 5;	
	valueVector myEvid;
    myEvid.resize( 7 );
	myEvid[0].SetInt(1);
	myEvid[1].SetFlt(1.0f);
	myEvid[2].SetFlt(0.3f);
	myEvid[3].SetFlt(0.7f);
	myEvid[4].SetFlt(1.0f);
	myEvid[5].SetFlt(2.1f);
	myEvid[6].SetFlt(3.2f);
    CModelDomain* pMD = mn->GetModelDomain();
	CEvidence *pMyEvid = CEvidence::Create( pMD, 3, obsNodes, myEvid );
	
	/*get some information from evidence object*/
	int nObsNodes = pMyEvid->GetNumberObsNodes();
	const int *pObsNodesNow = pMyEvid->GetObsNodesFlags();
	const int *myOffset = pMyEvid->GetOffset();
	const int *myNumAllObsNodes = pMyEvid->GetAllObsNodes();
    valueVector ev;
	pMyEvid->GetRawData(&ev);
	/*some information for determineDistributionType*/
	intVector numbersOfReallyObsNodes;
	int numReallyObsNodes=0;
	printf( "My Evidence is:\n" );
	printf( "Nodes			Offset\n" );
	for ( i=0; i<nObsNodes; i++ )
	{
		if ( pObsNodesNow[i] )
		{
			numbersOfReallyObsNodes.push_back( myNumAllObsNodes[i] );
			numReallyObsNodes++;
			printf( "%d			%d	\n",myNumAllObsNodes[i], myOffset[i] );
		}
	}
	getchar();
	const CNodeType ** AllNodeTypesFromModel= new const CNodeType*[nnodes];
	for ( i=0; i<nnodes; i++ )
	{
		AllNodeTypesFromModel[i] = mn->GetNodeType( i );
	}
	
	//print values
	printf("\nMy Values are:\n\n");
	for (i=0; i<nObsNodes; i++)
	{
		const CNodeType nt = *(mn->GetNodeType(myNumAllObsNodes[i]));
		int IsDiscreteNode = nt.IsDiscrete();
		if(IsDiscreteNode)
		{
			const int val = (ev[myOffset[i]].GetInt());
			printf("%d \n", val);
		}
		else
		{
			for (int j=0; j<3; j++)	printf("%f ", ev[myOffset[i]+j].GetFlt());
			printf("\n");
		}
		printf("\n");
	}
	getchar();

/*Now we can determine distribution type of all model taking into account observed nodes*/
	char *mDt = NULL;
	EDistributionType myDt;
	myDt = pnlDetermineDistributionType(nnodes, numReallyObsNodes,
					&numbersOfReallyObsNodes.front(), AllNodeTypesFromModel);
	switch (myDt)
	{
	case dtTabular: 
		{mDt = "dtTabular"; break;}
	case dtGaussian: 
		{mDt = "dtGaussian"; break;}
	case dtCondGaussian: 
		{mDt = "dtCondGaussian";break;}
	}
	printf("Distribution Type is: %s (%d)\n\n", mDt, myDt);
	getchar();
	numbersOfReallyObsNodes.erase(numbersOfReallyObsNodes.begin(), 
		numbersOfReallyObsNodes.end());

	//Toggle Node 1
	int ToggleNodeNums[1] ;
	ToggleNodeNums[0]= 1;
	pMyEvid->ToggleNodeState(1,ToggleNodeNums);
	pObsNodesNow = pMyEvid->GetObsNodesFlags();
//	intVector numbersOfReallyObsNodes;
	numReallyObsNodes=0;		
	
	printf("We toggle node 1. Our observed nodes are: \n");
	printf("Nodes			Offset\n");
	for ( i=0; i<nObsNodes; i++)
	{
		if (pObsNodesNow[i])
		{
			numbersOfReallyObsNodes.push_back(myNumAllObsNodes[i]);
			numReallyObsNodes++;
			printf("%d			%d\n",myNumAllObsNodes[i], myOffset[i]);
		}
	}
	printf("\n");
	getchar();
	myDt = pnlDetermineDistributionType(nnodes, numReallyObsNodes,
					&numbersOfReallyObsNodes.front(), AllNodeTypesFromModel);
/*Now we can determine distribution type of all model taking into account observed nodes - 
	we toggle node 1 so it is changes*/
//	char *mDt = NULL;
//	EDistributionType myDt;
	myDt = pnlDetermineDistributionType(nnodes, numReallyObsNodes,
				&numbersOfReallyObsNodes.front(), AllNodeTypesFromModel);
	switch (myDt)
	{
	case dtTabular: 
		{mDt = "dtTabular"; break;}
	case dtGaussian: 
		{mDt = "dtGaussian"; break;}
	case dtCondGaussian: 
		{mDt = "dtCondGaussian";break;}
	}
	printf("Distribution Type is: %s (%d)\n\n", mDt, myDt);
	
	int dim = 2;
	intVector ranges(2,2);
    ranges[1] = 1;
	floatVector data(2, 1.0f );
	CNumericDenseMatrix<float> *myMat = CNumericDenseMatrix<float>::Create( dim, &ranges.front(),
		&data.front() );
	float val = 3.0f;
	intVector indices( 2, 0 );
	myMat->SetElementByIndexes( val, &indices.front());
	//int dim0 = 0;
	//int keepPos = 1;
	//int sizeNew = 4;
	//CMatrix<float> *expMat = myMat->ExpandDims( &dim0, &keepPos, &sizeNew, 1 );

	delete myMat;
	getchar();
	//some parts of inference for this model
//	CInfEngine *infEng = pnlCreateInfEngine(itNaive, mn);
	//infEng->EnterEvidence(pMyEvid);
	/*we don't enter evidence so Distribution type is 
	the same as distribution type of factor 
	specified for all model's nodes without evidence*/
	delete(mn);
	delete (pMyEvid);
	delete []nodeTypes;
	delete []nodeAssociation;
	delete []obsNodes;
	delete []AllNodeTypesFromModel;

	return 0;
}		
