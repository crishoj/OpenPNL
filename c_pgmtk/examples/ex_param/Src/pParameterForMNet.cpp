/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pParameterForMNet.cpp                                       //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"

PNL_USING

int main()
{
    int i;
	const int nnodes = 6;
	const int numnt = 2;
	int *nodeAssociation;
	/*	int numOfNeigh[] = { 2, 5, 4, 2, 4, 3};
	
	  int neigh1[] = { 1, 2 };
	  int neigh2[] = { 0, 2, 3, 4,5 };
	  int neigh3[] = { 0, 1, 4, 5 };
	  int neigh4[] = { 1,4 };
	  int neigh5[] = { 1, 2, 3, 5};
	  int neigh6[] = { 1, 2, 4 };
	*/
	int numOfCliques = 3;
	int *cliqueSize = new int [numOfCliques];
	cliqueSize[0] = 3; cliqueSize[1] = 3; cliqueSize[2]=4;
	int clique0[] = {0,1,2};
	int clique1[] = {1,3,4};
	int clique2[] = {1,2,4,5};
	const int *cliques[] = {clique0, clique1, clique2};
	
	//	const int *neigh[] = { neigh1, neigh2, neigh3, neigh4, neigh5, neigh6, neigh7 };
	//	const int *orient[] = { orient1, orient2, orient3, orient4, orient5, orient6, orient7 };
	
	EModelTypes type;
	CStaticGraphicalModel *mn;
	
	CNodeType *nodeTypes = new CNodeType [numnt];
	nodeAssociation = new int [nnodes];
	
	nodeTypes[0].SetType(1, 2);
	nodeTypes[1].SetType(0, 3);
	
	nodeAssociation[0] = 1;
	nodeAssociation[1] = 0;
	nodeAssociation[2] = 0;
	nodeAssociation[3] = 1;
	nodeAssociation[4] = 0;
	nodeAssociation[5] = 0;
	
	type = mtMNet;
	
	mn = pnlCreateStaticModel(type, nnodes, numnt, nodeTypes, nodeAssociation, numOfCliques, 
		cliqueSize, cliques);
	
	//CGraph *graph = mn->GetGraph();
	
	//mn->SayIt();
	
	//	graph->pgmAddEdge( 2, 6, 1 );
	//	mn->pgmShowGraph();
	//	getchar();
	
	//	graph->pgmAddEdge( 5, 6, 1 );
	//	mn->pgmShowGraph();
	//	getchar();
	
	//	for( i = 0; i < nnodes - 1; i++ )
	//	{
	//		graph->pgmSetNeighbors(i, numOfNeigh[i], neigh[i], orient[i]);
	//		mn->pgmShowGraph();
	//		getchar();
	//	}
	int *obsNodes = new int[4];
	obsNodes[0] = 0;
	obsNodes[1] = 1;
	obsNodes[2] = 3;
	obsNodes[3] = 5;
	struct
	{
		float f1[3];
		int f2;
		float f3[3];
		int f4;
	} myEvid;
	myEvid.f1[0] = 1.0f;
	myEvid.f1[1] = 0.3f;
	myEvid.f1[2] = 0.7f;
	myEvid.f2 = 1;
	myEvid.f3[0] = 1.0f;
	myEvid.f3[1] = 2.1f;
	myEvid.f3[2] = 3.2f;
	myEvid.f4 = 0;
	CEvidence *pMyEvid = CEvidence::Create(mn, 4, obsNodes, &myEvid)	;
	//	void ToggleNodeState(int nNodes, int *nodeIndices);
	//	int GetNumberObsNodes() const;
	//	const int *GetObsNodesFlags()const;
	//	const int *GetOffset()const;
	int nObsNodes = pMyEvid->GetNumberObsNodes();
	const int *pObsNodesNow = pMyEvid->GetObsNodesFlags();
	const int *myOffset = pMyEvid->GetOffset();
	const int *myNumAllObsNodes = pMyEvid->GetAllObsNodes();
	const unsigned char *ev = pMyEvid->GetRawData();
//	const float *valf = (const float*)(ev+myOffset[0]);
//	const int val = *(const int*)(ev+myOffset[1]);
	
	printf("My Evidence is:\n");
	printf("Nodes				Offset	\n");
	intVector ReallyObsNodes;
	int numReallyObs = 0;
	pnlVector<const unsigned char *> AllOffsets( nnodes, NULL );
	for ( i=0; i<nObsNodes; i++)
	{
	//	AllOffsets.push_back(0);
		if (pObsNodesNow[i])
		{
			printf("%d			%d	\n",myNumAllObsNodes[i], myOffset[i]);
			if((myNumAllObsNodes[i]==1))
			{
				AllOffsets[1]=ev+myOffset[i];
				ReallyObsNodes.push_back(pObsNodesNow[i]);
				numReallyObs++;
			}
			if(myNumAllObsNodes[i]==5)
			{
				AllOffsets[5]=ev+myOffset[i];
				ReallyObsNodes.push_back(pObsNodesNow[i]);
				numReallyObs++;
			}
		}
	}
	getchar();
	
	/*Check the method ShrinkObservedNodes for the Factor*/
	printf("\nWe test method ShrinkObservedNodes for Factor.\n");
	printf("We create Factor on Nodes 1, 2, 4, 5 (nodes 1, 5 - observed)\n");
	int nNodes = 4;
	int domain[] = {1,2,4,5};
	const CNodeType* nt[] = {
		mn->GetNodeType(1),
		mn->GetNodeType(2),
		mn->GetNodeType(4),
		mn->GetNodeType(5)
	};
	CTabularPotential *pMyFactor = CTabularPotential::Create( &nt.front(), 
		domain, nNodes);
	float data[] = {0.0f,0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f,
		0.8f,0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f};
	pMyFactor->AllocMatrix(data, matTable);
	int pObsNodes[] = {1,5}; 
	CNodeType ObsNodeType = CNodeType(1,1);
	CTabularPotential *pSmallPotential = static_cast<CTabularPotential*>
		(pMyFactor->ShrinkObservedNodes(numReallyObs, pObsNodes,
						&AllOffsets.front(), &ObsNodeType));
	const CMatrix<float> *myNewMatrix = pSmallPotential->GetMatrix(matTable);
	const floatVector* myNewVector = myNewMatrix->GetVector();
	printf("\n");
	printf("My New Data is: \n");
	for (i=0; i<myNewVector->size();i++)
	{
		printf("%f ",(*myNewVector)[i]);
	}	
	int MyNewNumDims;	// = myNewMatrix->GetNumberDims();
	const int *MyNewRanges ;
	myNewMatrix->GetRanges(&MyNewNumDims, &MyNewRanges);
	printf("\nMy New Ranges are\n");
	for (i=0; i<MyNewNumDims; i++)	printf("%d ", MyNewRanges[i]);
	printf("\n");
	
	//Toggle Node 1
	printf("\nToggleNodeState for node  0");
	int ToggleNodeNums[1] ;
	ToggleNodeNums[0]= 0;
	pMyEvid->ToggleNodeState(1,ToggleNodeNums);
	pObsNodesNow = pMyEvid->GetObsNodesFlags();
	//CInfEngine *infEng = pnlNaiveInfEngine::Create(mn);
	//infEng->EnterEvidence(pMyEvid);
	printf("\nMy Evidence is \n");
	printf("Nodes				Offset	\n");
	for ( i=0; i<nObsNodes; i++)
	{
		if (pObsNodesNow[i]){
			printf("%d			%d\n",myNumAllObsNodes[i], myOffset[i]);
		}
	}
	getchar();
	//	printf("Distribution Type is: %d\n", myDt);
	getchar();
	
	delete(mn);
	delete (pMyEvid);
	delete []cliqueSize;
	delete []nodeTypes;
	delete []nodeAssociation;
	delete []obsNodes;

	return 0;
}		
