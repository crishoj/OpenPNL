/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AParameterLearning.cpp                                      //
//                                                                         //
//  Purpose:   Test on factor learning                                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <float.h>

PNL_USING

static char func_name[] = "testFactorLearningEngine";

static char* test_desc = "Provide ML and EM Factor Learning";

static char* test_class = "Algorithm";

int testFactorLearningEngine()
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

    int obs_nodes[]={ 0, 1, 2, 3 };

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


    float data0[]={ 0.5f, 0.5f };
    float data1[]={ 0.5f, 0.5f, 0.5f, 0.5f };
    float data2[]={ 0.5f, 0.5f, 0.5f, 0.5f };
    float data3[]={ 1.0f, 0.0f, 0.087f, 0.913f, 0.1f, 0.9f, 0.0f, 1.0f };

    pnlVector <float*> prior;

    prior.push_back( data0 );
    prior.push_back( data1 );
    prior.push_back( data2 );
    prior.push_back( data3 );

    float test_data0[]={ 0.520f, 0.480f };
    float test_data1[]={ 0.5769f, 0.4231f, 0.9167f, 0.0833f };
    float test_data2[]={ 0.8846f, 0.1154f, 0.0417f, 0.9583f };
    float test_data3[]={ 1.0f, 0.0f, 0.087f, 0.913f, 0.1f, 0.9f, 0.0f, 1.0f };

    //values to compare
    pnlVector <float*> test_data;

    test_data.push_back( test_data0 );
    test_data.push_back( test_data1 );
    test_data.push_back( test_data2 );
    test_data.push_back( test_data3 );

    CGraph* Graph = CGraph::Create( nnodes, numOfNeigh, neigh,
	orient);

    CBNet *myBNet = CBNet::Create(nnodes, numNt, nodeTypes,
	nodeAssociation, Graph );

    myBNet->AllocFactors();
    for ( int node = 0; node < nnodes; node++ )
    {
	myBNet->AllocFactor( node );
	(myBNet->GetFactor( node )) ->AllocMatrix( prior[node], matTable );
    }


    ///////////////////////////////////////////////////////////////

    char *argv = "../c_pgmtk/tests/testdata/cases1";
    FILE *fp;
    const int nEv = 50;
    CEvidence **m_pEv;
    m_pEv = new CEvidence *[nEv];
    //int Evidence[nEv][nnodes];
    valueVecVector Evidence(nEv);
    for( int ev = 0; ev < nEv; ev++)
    {
        Evidence[ev].resize(nnodes);
    }


    int simbol;
    if( (fp = fopen( argv, "r" ) ) == NULL)
    {
        argv = "../testdata/cases1";
	if ( (fp = fopen( argv, "r" ) ) == NULL )
	{
	    printf( "can't open file %s\n", argv );
	    ret = TRS_FAIL;
	    return trsResult( ret, ret == TRS_OK ? "No errors" :
	    "Bad test: not file with cases");
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

    CEMLearningEngine *pLearn = CEMLearningEngine::Create(myBNet);

    pLearn -> SetData(50, m_pEv);
    trsTimerStart(0);
    pLearn ->Learn();
    double time = trsTimerClock(0);
    printf( "timing of ML learning procedure %f \n", time);
    int nsteps;
    const float* ll;
    pLearn->GetCriterionValue(&nsteps, &ll);
    printf( "likelihood %f \n", ll[nsteps-1] );

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
	    if( fabs(output[j] - test_data[i][j] ) > eps )
	    {
		ret = TRS_FAIL;
		break;
	    }

	}
    }
    delete (pLearn);
    ///////////////////////////////////////////////////////////////////
    int nodeNumbers[2] = {0,1};
    for (i=0;i<nEv;i++)
    {
	m_pEv[i] -> ToggleNodeState( 2, nodeNumbers );

    }
    int MaxIter = 0;
    while(MaxIter<=0)
    {
	trsiRead( &MaxIter, "10", "maximum number of iterations");
    }
    trsTimerStart(0);

    CEMLearningEngine *pLearnEm = CEMLearningEngine::Create(myBNet);

    pLearnEm -> SetData(nEv, m_pEv);
    pLearnEm -> SetMaxIterEM(int(MaxIter));
    pLearnEm ->Learn();
    time = trsTimerClock(0);
    printf( "timing of EMlearning procedure %fn", time);


    for ( i = 0; i < nnodes; i++)
    {

	pMatrix = static_cast<CNumericDenseMatrix<float>*>(myBNet->
            GetFactor(i)->GetMatrix(matTable));
	pMatrix->GetRawData(&length, &output);
	for (j = 0; j < length; j++)
	{
	    if( fabs(output[j] - test_data[i][j] ) > eps )
	    {
		ret = TRS_FAIL;
		
		break;
	    }

	}
    }
    for( i = 0; i < nEv; i++ )
    {
	delete (m_pEv[i]);

    }
    delete []m_pEv;
    delete (pLearnEm);
    delete (myBNet);
    delete [](nodeTypes);

    fclose(fp);
    return trsResult( ret, ret == TRS_OK ? "No errors" :
    "Bad test on ParamLearningEngine");
}

void initAFactorLearningEngine()
{
    trsReg(func_name, test_desc, test_class, testFactorLearningEngine);
}