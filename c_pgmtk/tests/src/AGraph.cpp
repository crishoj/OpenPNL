/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGraph.cpp                                                  //
//                                                                         //
//  Purpose:   TRS test of graph creation                                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include "AGraph.hpp"

PNL_USING

static char  func_name[] = "testGraph";
static char* test_desc = "Initialize a graph";
static char* test_class = "Algorithm";

int testGraph()
{
    int i, j;

    int ret = TRS_OK;

    const int nnodes = 7;

    int numOfNeigh[] = { 3, 3, 4, 5, 2, 3, 2 };

    int neigh1[] = { 3, 1, 2 };
    int neigh2[] = { 2, 0, 3 };
    int neigh3[] = { 0, 1, 3, 6 };
    int neigh4[] = { 0, 1, 2, 4, 5 };
    int neigh5[] = { 3, 5 };
    int neigh6[] = { 3, 4, 6 };
    int neigh7[] = { 5, 2 };

    ENeighborType orient1[] = { ntChild, ntChild, ntChild };
    ENeighborType orient2[] = { ntChild, ntParent, ntChild };
    ENeighborType orient3[] = { ntParent, ntParent, ntChild, ntChild };
    ENeighborType orient4[] = { ntParent, ntParent, ntParent, ntNeighbor, ntNeighbor };
    ENeighborType orient5[] = { ntNeighbor, ntNeighbor };
    ENeighborType orient6[] = { ntNeighbor, ntNeighbor, ntChild };
    ENeighborType orient7[] = { ntParent, ntParent };

    int *neigh[] = { neigh1, neigh2, neigh3, neigh4, neigh5, neigh6,
	neigh7 };
    ENeighborType *orient[] = { orient1, orient2, orient3, orient4, orient5,
	orient6, orient7 };

    const int *neighbors;
    const ENeighborType *orientation;

    double time1;

    TestGraph *graph;

    /* The graph I used to have a test is described in a document named
    graph_structure.doc which is stored in VSS base in docs folder */

    /* to test graph the graph creation you can either: */

    /* 1. create it from the list of neighbors */

    trsTimerStart(0);
    graph = new TestGraph(nnodes, numOfNeigh, neigh, orient);
    graph->Dump();

    /* look at the graph */

    for( i = 0; i < nnodes; i++ )
    {
	graph->GetNeighbors( i, &numOfNeigh[i], &neighbors, &orientation );

	intVector neighToComp( neigh[i], neigh[i] + numOfNeigh[i] );
	std::sort( neighToComp.begin(), neighToComp.end() );

	for( j = 0; j < numOfNeigh[i]; j++ )
	{
	    if(neighbors[j] != neighToComp[j])
	    {
		ret = TRS_FAIL;
	    }
	}
    }

    delete(graph);

    time1 = trsTimerClock(0);
    std::cout<<"timing of graph creation with all neighbors preset "
	<<time1<<std::endl;

        /* 2. create a graph with a blank list of neighbors
    and then set neighbors */

    trsTimerStart(0);
    graph = new TestGraph(nnodes, NULL, NULL, NULL);

    graph->SetNeighbors( 0, numOfNeigh[0], neigh1, orient1 );
    graph->SetNeighbors( 1, numOfNeigh[1], neigh2, orient2 );
    graph->SetNeighbors( 2, numOfNeigh[2], neigh3, orient3 );
    graph->SetNeighbors( 3, numOfNeigh[3], neigh4, orient4 );
    graph->SetNeighbors( 4, numOfNeigh[4], neigh5, orient5 );
    graph->SetNeighbors( 5, numOfNeigh[5], neigh6, orient6 );
    graph->SetNeighbors( 6, numOfNeigh[6], neigh7, orient7 );
    graph->Dump();

    for( i = 0; i < nnodes; i++ )
    {
	graph->GetNeighbors( i, &numOfNeigh[i], &neighbors, &orientation );

	intVector neighToComp( neigh[i], neigh[i] + numOfNeigh[i] );
	std::sort( neighToComp.begin(), neighToComp.end() );

	for( j = 0; j < numOfNeigh[i]; j++ )
	{
	    if(neighbors[j] != neighToComp[j])
	    {
		ret = TRS_FAIL;
	    }
	}
    }

    delete(graph);

    time1 = trsTimerClock(0);
    std::cout<<"graph creation with setting the each neighbor separatly "
	<<time1<<std::endl;

        /* 3. create a blank graph and add edges to it. take the edges from the
    list of neighbors above */

    trsTimerStart(0);
    graph = new TestGraph(nnodes, NULL, NULL, NULL);

    graph->AddEdge( 0, 1, 1 );
    graph->AddEdge( 0, 2, 1 );
    graph->AddEdge( 0, 3, 1 );
    graph->AddEdge( 1, 2, 1 );
    graph->AddEdge( 1, 3, 1 );
    graph->AddEdge( 2, 3, 1 );
    graph->AddEdge( 2, 6, 1 );
    graph->AddEdge( 3, 4, 0 );
    graph->AddEdge( 3, 5, 0 );
    graph->AddEdge( 4, 5, 0 );
    graph->AddEdge( 5, 6, 1 );
    graph->Dump();

    for( i = 0; i < nnodes; i++ )
    {
	graph->GetNeighbors( i, &numOfNeigh[i], &neighbors, &orientation );
	intVector neighToComp( neigh[i], neigh[i] + numOfNeigh[i] );
	std::sort( neighToComp.begin(), neighToComp.end() );

	for( j = 0; j < numOfNeigh[i]; j++ )
	{
	    if(neighbors[j] != neighToComp[j])
	    {
		ret = TRS_FAIL;
	    }
	}
    }

    delete(graph);

    time1 = trsTimerClock(0);
    std::cout<<"timing graph creation with edges added "<<time1<<std::endl;

    /* all three graphs that are shown, should look the same */

    return trsResult( ret, ret == TRS_OK ? "No errors" : "Graph FAILED");
}

void initAGraph()
{
    trsReg( func_name, test_desc, test_class, testGraph );
}