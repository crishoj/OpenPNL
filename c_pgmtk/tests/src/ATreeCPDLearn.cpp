/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AFactorLearning.cpp                                      //
//                                                                         //
//  Purpose:   Test on factor learning                                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include "pnlTreeCPD.hpp"

PNL_USING

static char func_name[] = "testTreeCPDLearn";
static char* test_desc = "Tree CPD Learning";
static char* test_class = "Algorithm";

CEvidence**
CreateDataForLearning( const CBNet* pBnet, int numberOfCases = 1000 )
{
    int numberOfEvidences = 0;

    if( numberOfCases < 1000 )
    {
        numberOfEvidences = numberOfCases;
    }
    else
    {
        numberOfEvidences = 1000;
    }

    int nnodes = pBnet->GetNumberOfNodes();
    CEvidence **pEvidences = new CEvidence *[numberOfEvidences];
    int dataSize = 4;//summ all sizes

    int obs_nodes[] = {0, 1, 2, 3};

    //read data from file cases_gaussian.txt to evidences
    // char buf[_MAX_PATH];
    // _getcwd(buf, _MAX_PATH);

    FILE* file = fopen("../../examples/Data/casesTab", "rt");
    if( !file )
    {
	if((file = fopen("../c_pgmtk/examples/Data/casesTab", "rt")) == NULL)
	{
	    return NULL;
	}
    }

    valueVector input_data;
    input_data.assign( dataSize, (Value)0 );

    for(int i = 0 ; i < numberOfEvidences; i++)
    {
        for (int j = 0; j < dataSize ; j++)
        {
            int val;
            fscanf(file , "%d", &val);
	    assert((long)val < 2);
	    if (j%2 == 1)
		input_data[j].SetInt(val);
	    else
		input_data[j].SetFlt((float)val);
        }
        pEvidences[i] = CEvidence::Create(pBnet, nnodes, obs_nodes, input_data);
    }

    fclose(file);
    return pEvidences;
}

int testTreeCPDLearn()
{
	int ret = TRS_OK;

    const int nnodes = 4; //Number of nodes
    const int numNt =  2; //number of Node types (all nodes are discrete)
    CNodeType* nodeTypes = new CNodeType [numNt];
    nodeTypes[0] = CNodeType( 1,2 );
    nodeTypes[1] = CNodeType( 0,1 );
    int nodeAssociation[] = { 0, 1, 0, 1 };
    //Create a dag
    int numOfNeigh[] = {2, 2, 2, 2};
    int neigh0[] = {1, 2};
    int neigh1[] = {0, 3};
    int neigh2[] = {0, 3};
    int neigh3[] = {1, 2};

    ENeighborType orient0[] = { ntChild, ntChild };
    ENeighborType orient1[] = { ntParent, ntChild };
    ENeighborType orient2[] = { ntParent, ntChild };
    ENeighborType orient3[] = { ntParent, ntParent };

    int *neigh[] = { neigh0, neigh1, neigh2, neigh3 };
    ENeighborType *orient[] = { orient0, orient1, orient2, orient3 };

    CGraph* pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient);

    //Create static BNet
    CBNet* pBNet = CBNet::Create( nnodes, numNt, nodeTypes, nodeAssociation, pGraph );
    pBNet->AllocFactors();

    int* domain = new int[nnodes];
    CTreeCPD** ppCPD = new CTreeCPD*[4];
    for ( int node = 0; node < nnodes; node++ )
    {
	//        pBNet->AllocFactor( node );
	int                    numOfNbrs;
	const int              *nbrs;
	const ENeighborType *nbrsTypes;

	pGraph->GetNeighbors( node, &numOfNbrs, &nbrs, &nbrsTypes );
	int* _domain = domain;
	for(int i = 0; i < numOfNbrs; i++ )
	{
	    if( nbrsTypes[i] == ntParent )
		*(_domain++) = nbrs[i];
	}
	*(_domain++) = node;

	CTreeCPD* myCPD = ppCPD[node] =
	    CTreeCPD::Create( domain, _domain - domain, pBNet->GetModelDomain() );
	pBNet->AttachFactor( myCPD );
    }
    delete[] domain;
    int nCases = 1000;
    CEvidence** ppEv = CreateDataForLearning(pBNet, nCases);
    if ( ppEv == NULL )
	ret = TRS_FAIL;
    else
    {
	CEMLearningEngine* pEng = CEMLearningEngine::Create( pBNet );

	pEng->SetData( nCases, ppEv );
	pEng->SetTerminationToleranceEM( 0.01f );
	pEng->SetMaxIterEM( 5 );

	// Start learning
	try
	{
	    trsTimerStart(0);
	    pEng->Learn();
	    double time = trsTimerClock(0);
	    printf( "timing of Tree CPD learning procedure %fn", time);
	}
	catch(CAlgorithmicException except)
	{
	    std::cout << except.GetMessage() << std::endl;
	    ret = TRS_FAIL;
	}
	catch( ... )
	{
	    ret = TRS_FAIL;
	}
	for(int i = 0 ; i < nCases; i++)
	{
	    delete ppEv[i];
	}
	delete[] ppEv;
	delete pEng;
    }
    delete pBNet;
    delete[] nodeTypes;
    delete[] ppCPD;
    return trsResult( ret, ret == TRS_OK ? "No errors" :
	"Bad test on TreeCPDLearn");
}

void initATreeCPDLearn()
{
    trsReg(func_name, test_desc, test_class, testTreeCPDLearn);
}