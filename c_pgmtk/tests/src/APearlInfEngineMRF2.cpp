/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      APearlInfEngineMRF2.cpp                                     //
//                                                                         //
//  Purpose:   Test on Pearl inference algorithm on MRF2 model             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <math.h>
#include <float.h>

PNL_USING

static char func_name[] = "testPearlInfEngineMRF2";

static char* test_desc = "Provide Pearl inference for MRF2 and compare with results from Matlab";

static char* test_class = "Algorithm";

PNL_USING

int testPearlInfEngineMRF2()
{
    int ret = TRS_OK;
    int i=0, j=0;
    int nnodes = 11;//4;//7;
    int numClqs =10;//3;// 6;
    float eps = 1e-5f;

    nodeTypeVector nodeTypes;
    nodeTypes.assign( 1, CNodeType() );

    nodeTypes[0] = CNodeType(1,2);

    intVector nodeAssociation;
    nodeAssociation.assign(nnodes, 0);

    CModelDomain* pMD = CModelDomain::Create( nodeTypes, nodeAssociation );

    //create graphical model by clqs;
    int *clqSizes  = new int[numClqs];
    for ( i = 0; i < numClqs; i++ )
    {
	clqSizes[i] = 2;
    }

    int clqs0[] = {0,1};
    int clqs1[] = {1,2};
    int clqs2[] = {1,3};
    int clqs3[] = {0,4};
    int clqs4[] = {4,5};
    int clqs5[] = {4,6};
    int clqs6[] = {6,7};
    int clqs7[] = {6,8};
    int clqs8[] = {6,9};
    int clqs9[] = {9,10};/**/
    const int *clqs[] = { clqs0, clqs1, clqs2, clqs3, clqs4, clqs5, clqs6,
	clqs7, clqs8, clqs9/**/};

    CMRF2 *myModel = CMRF2::Create( numClqs, clqSizes, clqs, pMD );


    myModel->GetGraph()->Dump();
    //we creates every factor - it is factor

    //number of factors is the same as number of cliques - one per clique
    CFactor **myParams = new CFactor*[numClqs];


    //to create factors we need to create their tables

    //create container for Factors
    myModel->AllocFactors();
    //create array of data for every parameter
    float Data0[] = { 0.6f, 0.4f, 0.8f, 0.2f };
    float Data1[] = { 0.5f, 0.5f, 0.7f, 0.3f };
    float Data2[] = { 0.1f, 0.9f, 0.3f, 0.7f };
    float Data3[] = { 0.1f, 0.9f, 0.2f, 0.8f };//{ 0.01f, 0.99f, 0.02f, 0.98f };
    float Data4[] = { 0.2f, 0.8f, 0.3f, 0.7f };
    float Data5[] = { 0.4f, 0.6f, 0.6f, 0.4f };
    float Data6[] = { 0.8f, 0.2f, 0.9f, 0.1f };
    float Data7[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float Data8[] = { 0.5f, 0.5f, 0.5f, 0.5f };
    float Data9[] = { 0.1f, 0.9f, 0.2f, 0.8f };/**/
    float *Data[] = { Data0, Data1, Data2, Data3, Data4, Data5, Data6,
	Data7, Data8, Data9 /**/};
    for(i=0; i<numClqs; i++)
    {

	myParams[i] = CTabularPotential::Create( clqs[i], 2, pMD);

	myParams[i]->AllocMatrix(Data[i], matTable);

	myModel->AttachFactor((CTabularPotential*)myParams[i]);
    }
    //now we can start inference
    //first we start inference without evidence
    //create evidence
    CEvidence *myEvid = CEvidence::Create(myModel, 0, NULL, valueVector());

    /////////////////////////////////////////////////////////////////
    //now we can compare Naive Inf results with Pearl Inf results
    //pearl algorithm
    CPearlInfEngine *myInfP = CPearlInfEngine::Create(myModel);
    myInfP->EnterEvidence(myEvid);

    CNaiveInfEngine *myInfN = CNaiveInfEngine::Create(myModel);
    myInfN->EnterEvidence(myEvid);
    
    /*CJtreeInfEngine* myInfJ = CJtreeInfEngine::Create(myModel);
    myInfJ->EnterEvidence(myEvid);*/
    /*
    
      //data from Matlab
      float marginals0[] = { 0.5f, 0.5f };
      float marginals1[] = { 0.7f, 0.3f };
      float marginals2[] = { 0.56f, 0.44f };
      float marginals3[] = { 0.16f, 0.84f };	//{ 0.16f, 0.84f };
      float marginals4[] = { 0.15f, 0.85f };		//{ 0.015f, 0.985f };
      float marginals5[] = { 0.285f, 0.715f };	//{ 0.2985f, 0.7015f };
      float marginals6[] = { 0.57f, 0.43f };		//{ 0.597f, 0.403f };
      float marginals7[] = { 0.843f, 0.157f };	//{ 0.8403f, 0.1597f };
      float marginals8[] = { 0.57f, 0.43f };		//{ 0.597f, 0.403f };
      float marginals9[] = { 0.5f, 0.5f };
      float marginals10[] = { 0.15f, 0.85f };
      
	float *marginals[] ={ marginals0, marginals1, marginals2, marginals3,
	marginals4, marginals5, marginals6, marginals7,
	marginals8, marginals9, marginals10 };
    */

	
	int querySize = 1;
	int *query = (int *)trsGuardcAlloc(querySize, sizeof(int));
	const CPotential* myQueryJPDP;
	const CPotential* myQueryJPDN;
	//const CPotential* myQueryJPDJ;
	//CNumericDenseMatrix<float>* myMatrP = NULL;
	//CNumericDenseMatrix<float>* myMatrN = NULL;
	//CNumericDenseMatrix<float>* myMatrJ = NULL;
	//int matPSize;
	//int matNSize;
	//int matJSize;
	for( i = 0; i < nnodes; i++ )
	{
	    query[0] = i;
	    myInfP->MarginalNodes( query, querySize );
	    myInfN->MarginalNodes( query, querySize );
   	    //myInfJ->MarginalNodes( query, querySize );
	    myQueryJPDP = myInfP->GetQueryJPD();
	    myQueryJPDN = myInfN->GetQueryJPD();
            if( !myQueryJPDP->IsFactorsDistribFunEqual( myQueryJPDN, eps, 0 ) )
            {
                ret = TRS_FAIL;
            }

    }


    //we can add evidence and compare results with results from NaiveInfrenceEngine
    int numObsNodes = 0;
    while((numObsNodes<1)||(numObsNodes>nnodes))
    {
	trsiRead( &numObsNodes, "1", "number of observed nodes");
    }
    int seed1 =  1021450643;//pnlTestRandSeed();
    //create string to display the value
    char *value = new char[20];
#if 0
    _itoa(seed1, value, 10);
#else
    sprintf( value, "%d", seed1 );
#endif
    trsiRead(&seed1, value, "Seed for srand to define NodeTypes etc.");
    delete []value;
    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);
    srand(seed1);

    int *ObsNodes = (int *)trsGuardcAlloc(numObsNodes, sizeof(int));
    valueVector ObsValues;
    ObsValues.assign( numObsNodes, (Value)0 );
    intVector residuaryNodesFor ;
    for(i=0; i<nnodes; i++)
    {
	residuaryNodesFor.push_back(i);
    }
    for(i=0; i<numObsNodes; i++)
    {
	j = rand()%(nnodes-i);
	ObsNodes[i] = residuaryNodesFor[j];
	residuaryNodesFor.erase(residuaryNodesFor.begin()+j);
	ObsValues[i].SetInt(rand()%(myModel->GetNodeType(ObsNodes[i])->GetNodeSize()));
    }

    residuaryNodesFor.clear();
    CEvidence *myEvidWithObs = CEvidence::Create(myModel, numObsNodes,
	ObsNodes, ObsValues);
    myInfP->EnterEvidence(myEvidWithObs);
    //	NumIters = myInfP->GetNumberOfProvideIterations();
    myInfN->EnterEvidence(myEvidWithObs);
    for( i = 0; i < nnodes; i++ )
    {
	query[0] = i;
	myInfP->MarginalNodes( query, querySize );
	myInfN->MarginalNodes( query, querySize );
	myQueryJPDP = myInfP->GetQueryJPD();
	myQueryJPDN = myInfN->GetQueryJPD();
        if( !myQueryJPDP->IsFactorsDistribFunEqual( myQueryJPDN, eps, 0 ) )
        {
            ret = TRS_FAIL;
        }
    }
    //check all and free memory

    //CPearlInfEngine::Release(&myInfP);
    delete (myInfP);
    delete (myInfN);
    //delete (myInfJ);
    delete (myEvidWithObs);
    delete (myEvid);
    delete (myModel);
    delete (myParams);
    delete []clqSizes;
    delete pMD;
    if(trsGuardCheck(query))
    {
	    return trsResult( TRS_FAIL, "Dirty memory");
    }
	return trsResult( ret, ret == TRS_OK ? "No errors" : 
	"Bad test on PearlInfEngineMRF2");
}

void initAPearlInfEngineMRF2()
{
    trsReg(func_name, test_desc, test_class, testPearlInfEngineMRF2);
}
