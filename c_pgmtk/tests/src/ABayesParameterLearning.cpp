/////////////////////////////////////////////////////////////////////////////
//									   //
//		INTEL CORPORATION PROPRIETARY INFORMATION		   //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.	   //
//									   //
//  File:      ABayesParameterLearning.cpp				   //
//									   //
//  Purpose:   Test on bayesian factor update/learning		   //
//									   //
//  Author(s):								   //
//									   //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <float.h>
PNL_USING

static char func_name[] = "testBayesLearningEngine";

static char* test_desc = "Bayesian factor update";

static char* test_class = "Algorithm";

int testBayesLearningEngine()
{
    
    int ret = TRS_OK;
    int i, j;
    const int nnodes = 4;//Number of nodes
    int numNt = 1;//number of Node Types

    float eps = -1.0f;
    while( eps <= 0)
    {
	trssRead( &eps, "0.01f", "accuracy in test");
    }
    CNodeType *nodeTypes = new CNodeType [numNt];
    for( i=0; i < numNt; i++ )
    {
	nodeTypes[i] = CNodeType(1,2);//all nodes are discrete and binary
    }

    int nodeAssociation[] = {0, 0, 0, 0};
    int obs_nodes[] = { 0, 1, 2, 3 };
    int numOfNeigh[] = { 2, 2, 2, 2};

    int neigh0[] = { 1, 2 };
    int neigh1[] = { 0, 3 };
    int neigh2[] = { 0, 3 };
    int neigh3[] = { 1, 2 };

    ENeighborType orient0[] = { ntChild, ntChild };
    ENeighborType orient1[] = { ntParent, ntChild };
    ENeighborType orient2[] = { ntParent, ntChild };
    ENeighborType orient3[] = { ntParent, ntParent };

    int *neigh[] = { neigh0, neigh1, neigh2, neigh3 };
    ENeighborType *orient[] = { orient0, orient1, orient2, orient3 };

    float prior0[] = { 1.f, 1.f };
    float prior1[] = { 1.f, 1.f, 1.f, 1.f };
    float prior2[] = { 1.f, 1.f, 1.f, 1.f };
    float prior3[] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };

    float* priors[] = { prior0,prior1,prior2,prior3 };

    float zero_array[] = { 1,1,1,1, 1,1,1,1 };

    float test_data0[] = { 0.636364f, 0.363636f };
    float test_data1[] = { 0.6f, 0.4f, 0.777778f, 0.222222f };
    float test_data2[] = { 0.866667f, 0.133333f, 0.111111f, 0.888889f };
    float test_data3[] = { 0.888889f, 0.111111f, 0.111111f, 0.888889f,
	0.142857f, 0.857143f, 0.333333f, 0.666667f };
    float* test_data_first[] = { test_data0, test_data1, test_data2, test_data3 };

    float test_data4[] = { 0.519231f, 0.480769f };
    float test_data5[] = { 0.571429f, 0.428571f, 0.884615f, 0.115385f };
    float test_data6[] = { 0.857143f, 0.142857f, 0.0769231f, 0.923077f };
    float test_data7[] = { 0.937500f, 0.0625000f, 0.12f, 0.88f,
	0.166667f, 0.833333f, 0.2f, 0.8f };
    float* test_data_second[] = { test_data4, test_data5, test_data6, test_data7 };

    CGraph* Graph = CGraph::Create( nnodes, numOfNeigh, neigh,
	orient);

    CBNet *myBNet = CBNet::Create(nnodes, numNt, nodeTypes,
	nodeAssociation, Graph );

    myBNet->AllocFactors();
    for ( int node = 0; node < nnodes; node++ )
    {
	myBNet->AllocFactor( node );
	//allocate empty CPT matrix, it needs to be allocated even if
	//we are going to learn train it.
	(myBNet->GetFactor( node ))->AllocMatrix( zero_array, matTable );
	//allocate prior matrices
	(myBNet->GetFactor( node ))->AllocMatrix( priors[node], matDirichlet );
	static_cast<CCPD *>(myBNet->GetFactor( node ))->NormalizeCPD();
    }

    ///////////////////////////////////////////////////////////////
    //Reading cases from disk

    FILE *fp;
    const int nEv = 50;
    CEvidence **m_pEv;
    m_pEv = new CEvidence *[nEv];
    std::vector<valueVector> Evidence(nEv);
    for( int ev = 0; ev < nEv; ev++)
    {
	Evidence[ev].resize(nnodes);
    }

    int simbol;
    char *argv = "../c_pgmtk/tests/testdata/cases1";
    fp = fopen(argv, "r");
    if(!fp)
    {
        argv = "../testdata/cases1";
        if ((fp = fopen(argv, "r")) == NULL)
	{
            printf( "can't open file %s\n", argv );
	    ret = TRS_FAIL;
	    return trsResult( ret, ret == TRS_OK ? "No errors" :
	        "Bad test: no file with cases");
	}
    }
    if (fp)
    {
	i = 0;
	j = 0;
	while( i < nEv)
	{
	    simbol = getc( fp );
	    if( isdigit( simbol ) )
	    {
		(Evidence[i])[j].SetInt(simbol - '0');
		j++;
		if( ( j - nnodes ) == 0 )
		{
		    i++;
		    j = 0;
		}
	    }
	}
    }

    for( i = 0; i < nEv; i++ )
    {
	m_pEv[i] = CEvidence::Create(myBNet->GetModelDomain(),
	    nnodes, obs_nodes, Evidence[i]);
    }

    //create bayesin learning engine
    CBayesLearningEngine *pLearn = CBayesLearningEngine::Create(myBNet);

    //Learn only portion of evidences
    pLearn->SetData(20, m_pEv);
    pLearn ->Learn();

    ///////////////////////////////////////////////////////////////////////////////
    CNumericDenseMatrix<float> *pMatrix;
    int length = 0;
    const float *output;

    for ( i = 0; i < nnodes; i++)
    {
	pMatrix = static_cast<CNumericDenseMatrix<float>*>(myBNet->
	    GetFactor(i)->GetMatrix(matTable));
	pMatrix->GetRawData(&length, &output);
	for (j = 0; j < length; j++)
	{
	    if( fabs(output[j] - test_data_first[i][j] ) > eps )
	    {
		ret = TRS_FAIL;
		return trsResult( ret, ret == TRS_OK ? "No errors" :
		"Bad test on BayesLearningEngine");
		break;
	    }

	}
    }

    //Learn second portion of evidences
    pLearn->AppendData(20, m_pEv+20);
    pLearn->AppendData(10, m_pEv+40);
    pLearn ->Learn();

    //check second portion
    for ( i = 0; i < nnodes; i++)
    {
	pMatrix = static_cast<CNumericDenseMatrix<float>*>(myBNet->
	    GetFactor(i)->GetMatrix(matTable));
	pMatrix->GetRawData(&length, &output);
	for (j = 0; j < length; j++)
	{
	    if( fabs(output[j] - test_data_second[i][j] ) > eps )
	    {
		ret = TRS_FAIL;
		return trsResult( ret, ret == TRS_OK ? "No errors" :
		"Bad test on BayesLearningEngine");
		break;
	    }

	}
    }

    for( i = 0; i < nEv; i++ )
    {
	delete (m_pEv[i]);

    }
    delete []m_pEv;
    delete (pLearn);
    delete (myBNet);
    delete [](nodeTypes);
    fclose(fp);
    return trsResult( ret, ret == TRS_OK ? "No errors"
	: "Bad test on BayesLearningEngine");
}

void initABayesLearningEngine()
{
    trsReg(func_name, test_desc, test_class, testBayesLearningEngine);
}