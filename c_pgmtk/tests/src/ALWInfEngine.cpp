/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ALWInfEngine.cpp                                            //
//                                                                         //
//  Purpose:   Test on LW sampling inference for BN                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <algorithm>
#include "tCreateKjaerulffDBN.hpp"
#include "tCreateIncineratorBNet.hpp"

// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testLWInference";

static char* test_desc = "Provide Likelyhood-Sampling Inference";

static char* test_class = "Algorithm";

int LWMPEforScalarGaussianBNet(int seed, float eps);
int LWForInceneratorBNet(int seed, float eps);
int LWForGaussianBNet(int seed, float eps);
int LWForAsiaBNet(int seed, float eps);
int LWForSimplestGaussianBNet(int seed, float eps);
int LWForScalarGaussianBNet(int seed, float eps);
int LWForMixtureBNet(int seed, float eps);
int LWForSingleGaussian(int seed, float eps);
int LWForWaterSprinklerBNet(int seed, float eps);

int testLWInference()
{
    int ret = TRS_OK;
    int seed = pnlTestRandSeed();
	std::cout<<"seed"<<seed<<std::endl;
    srand( seed );
 /////////////////////////////////////////////////////////////////////////////

    float eps = -1.0f;
    while( eps <= 0 )
    {
		trssRead( &eps, "1.5e-1f", "accuracy in test" );
    }

	ret = ret && LWForWaterSprinklerBNet(seed,  eps);
    ret = ret && LWForSingleGaussian( seed,  eps);
	ret = ret && LWForAsiaBNet(seed, eps);
    ret = ret && LWForInceneratorBNet(seed, eps);
	ret = ret && LWMPEforScalarGaussianBNet(seed,  eps);
	ret = ret && LWForSimplestGaussianBNet( seed,  eps); 
   	
	//ret = ret && LWForGaussianBNet(seed, eps+0.7f);           
	//ret = ret && LWForMixtureBNet(seed, eps+0.7f);  
    //ret = ret && LWForScalarGaussianBNet( seed, eps+0.7f);    
    	
    return trsResult( ret, ret == TRS_OK ? "No errors" :
					"Bad test on LW Sampling Inference" );

}

int LWForAsiaBNet(int seed, float eps)
{
    ///////////////////////////////////////////////////////////////////////////////
    std::cout<<"Test on LW for asia bnet"<<std::endl;
    CBNet* pBnet = pnlExCreateAsiaBNet();
    int ret;

    pEvidencesVector evidences;
    pBnet->GenerateSamples( &evidences, 1 );
    const int ndsToToggle[] = { 1, 2, 5, 7 };
    evidences[0]->ToggleNodeState( 4, ndsToToggle );

    CLWSamplingInfEngine *pLWInf;
    pLWInf = CLWSamplingInfEngine::Create(pBnet);
    pLWInf->EnterEvidence( evidences[0] );

    CJtreeInfEngine *pJTreeInf = CJtreeInfEngine::Create(pBnet);
    pJTreeInf->EnterEvidence( evidences[0] );

    const int querySz = 2;
    const int query[] = {0, 2};

    pLWInf->MarginalNodes( query,querySz );
    pJTreeInf->MarginalNodes( query,querySz );

    const CPotential *pQueryPot1 = pLWInf->GetQueryJPD();
    const CPotential *pQueryPot2 = pJTreeInf->GetQueryJPD();

    ret = pQueryPot1-> IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    std::cout<<"result of LW"<<std::endl;
    pQueryPot1->Dump();
    std::cout<<std::endl<<"result of junction"<<std::endl;
    pQueryPot2->Dump();

    delete evidences[0];
    delete pJTreeInf;
    delete pLWInf;
    delete pBnet;

    return ret;
    ///////////////////////////////////////////////////////////////////////////////
}

int LWForGaussianBNet(int seed, float eps)
{
    std::cout<<std::endl<<"test on LW for gaussian BNet (5 nodes)"<< std::endl;
    CBNet *pBnet;
    pEvidencesVector evidences;

    CLWSamplingInfEngine *pLWInf;
    const CPotential *pQueryPot1, *pQueryPot2;
    int i, ret;

    ////////////////////////////////////////////////////////////////////////
    //Do the example from Satnam Alag's PhD thesis, UCB ME dept 1996 p46
    //Make the following polytree, where all arcs point down
    //
    // 0   1
    //  \ /
    //   2
    //  / \
    // 3   4
    //
    //////////////////////////////////////////////////////////////////////

    int nnodes = 5;
    int numnt = 2;
    CNodeType *nodeTypes = new CNodeType[numnt];
    nodeTypes[0] = CNodeType(0,2);
    nodeTypes[1] = CNodeType(0,1);
    intVector nodeAssociation = intVector(nnodes,0);
    nodeAssociation[1] = 1;
    nodeAssociation[3] = 1;
    int nbs0[] = { 2 };
    int nbs1[] = { 2 };
    int nbs2[] = { 0, 1, 3, 4 };
    int nbs3[] = { 2 };
    int nbs4[] = { 2 };
    ENeighborType ori0[] = { ntChild };
    ENeighborType ori1[] = { ntChild };
    ENeighborType ori2[] = { ntParent, ntParent, ntChild, ntChild };
    ENeighborType ori3[] = { ntParent };
    ENeighborType ori4[] = { ntParent };
    int *nbrs[] = { nbs0, nbs1, nbs2, nbs3, nbs4 };
    ENeighborType *orient[] = { ori0, ori1, ori2, ori3, ori4 };
    intVector numNeighb = intVector(5,1);
    numNeighb[2] = 4;
    CGraph *graph;
    graph = CGraph::Create(nnodes, &numNeighb.front(), nbrs, orient);

    pBnet = CBNet::Create( nnodes, numnt, nodeTypes, &nodeAssociation.front(),graph );
    pBnet->AllocParameters();

    for( i = 0; i < nnodes; i++ )
    {
	pBnet->AllocParameter(i);
    }
    //now we need to create data for parameters - we'll create matrices
    floatVector smData = floatVector(2,0.0f);
    floatVector bigData = floatVector(4,1.0f);
    intVector ranges = intVector(2, 1);
    ranges[0] = 2;
    smData[0] = 1.0f;
    CNumericDenseMatrix<float> *mean0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &smData.front());
    bigData[0] = 4.0f;
    bigData[3] = 4.0f;

    bigData[1] = 0.30f;
    bigData[2] = 0.3f;
    ranges[1] = 2;
    CNumericDenseMatrix<float> *cov0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(0)->AttachMatrix(mean0, matMean);
    pBnet->GetFactor(0)->AttachMatrix(cov0, matCovariance);
    ranges[0] = 1;
    ranges[1] = 1;
    float val = 1.0f;
    
    CNumericDenseMatrix<float> *mean1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    CNumericDenseMatrix<float> *cov1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    pBnet->GetFactor(1)->AttachMatrix(mean1, matMean);
    pBnet->GetFactor(1)->AttachMatrix(cov1, matCovariance);
    smData[0] = 0.0f;
    smData[1] = 0.0f;
 
    ranges[0] = 2;
    CNumericDenseMatrix<float> *mean2 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    smData[0] = 2.0f;
    smData[1] = 1.0f;
    CNumericDenseMatrix<float> *w21 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    bigData[0] = 2.0f;
    bigData[1] = 0.0f;
    bigData[2] = 0.0f;
    bigData[3] = 1.0f;
    ranges[1] = 2;
    CNumericDenseMatrix<float> *cov2 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[0] = 2.0f;
    bigData[1] = 1.0f;
    bigData[2] = 1.0f;
    bigData[3] = 3.0f;
    CNumericDenseMatrix<float> *w20 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(2)->AttachMatrix( mean2, matMean );
    pBnet->GetFactor(2)->AttachMatrix( cov2, matCovariance );
    pBnet->GetFactor(2)->AttachMatrix( w20, matWeights,0 );
    pBnet->GetFactor(2)->AttachMatrix( w21, matWeights,1 );

    val = 0.0f;
    ranges[0] = 1;
    ranges[1] = 1;
    CNumericDenseMatrix<float> *mean3 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);
    val = 1.0f;
    CNumericDenseMatrix<float> *cov3 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);
    ranges[1] = 2;
    smData[0] = 1.0f;
    smData[1] = 1.0f;
    CNumericDenseMatrix<float> *w30 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    pBnet->GetFactor(3)->AttachMatrix( mean3, matMean );
    pBnet->GetFactor(3)->AttachMatrix( cov3, matCovariance );
    pBnet->GetFactor(3)->AttachMatrix( w30, matWeights,0 );

    ranges[0] = 2;
    ranges[1] = 1;
    smData[0] = 0.0f;
    smData[1] = 0.0f;
    CNumericDenseMatrix<float> *mean4 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    ranges[1] = 2;
    bigData[0] = 1.0f;
    bigData[1] = 0.5f;
    bigData[2] = 0.5f;
    bigData[3] = 1.0f;
    CNumericDenseMatrix<float> *cov4 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[2] = 1.0f;
    CNumericDenseMatrix<float> *w40 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(4)->AttachMatrix( mean4, matMean );
    pBnet->GetFactor(4)->AttachMatrix( cov4, matCovariance );
    pBnet->GetFactor(4)->AttachMatrix( w40, matWeights,0 );

    evidences.clear();
    pBnet->GenerateSamples( &evidences, 1 );

    const int ndsToToggle2[] = { 0, 1, 2 };
    evidences[0]->ToggleNodeState( 3, ndsToToggle2 );
    const int *flags1 = evidences[0]->GetObsNodesFlags();
    std::cout<<"observed nodes"<<std::endl;
    for( i = 0; i < pBnet->GetNumberOfNodes(); i++ )
    {
	if ( flags1[i] )
	{
	    std::cout<<"node "<<i<<"; ";
	}
    }
    std::cout<<std::endl<<std::endl;

    const int querySz2 = 1;
    const int query2[] = { 2, 0 };

    CNaiveInfEngine *pNaiveInf = CNaiveInfEngine::Create(pBnet);
    pNaiveInf->EnterEvidence( evidences[0] );
    pNaiveInf->MarginalNodes( query2,querySz2 );

    pLWInf = CLWSamplingInfEngine::Create( pBnet, 10000 );
	
    pLWInf->EnterEvidence( evidences[0] );
    pLWInf->MarginalNodes( query2, querySz2 );

    pQueryPot1 = pLWInf->GetQueryJPD();
    pQueryPot2 = pNaiveInf->GetQueryJPD();
    std::cout<<"result of LW"<<std::endl<<std::endl;
    pQueryPot1->Dump();
    std::cout<<"result of naive"<<std::endl;
    pQueryPot2->Dump();

    ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
    delete pNaiveInf;
    delete pLWInf;
    delete pBnet;

    return ret;

    ////////////////////////////////////////////////////////////////////////////////////////
}

int LWForInceneratorBNet(int seed, float eps)
{

    std::cout<<std::endl<<"LW for Incenerator BNet"<< std::endl;

    CBNet *pBnet;
    pEvidencesVector evidences;
    CJtreeInfEngine *pJTreeInf;
    CLWSamplingInfEngine *pLWInf;
    const CPotential *pQueryPot1, *pQueryPot2;
    int i, ret;

    pBnet = tCreateIncineratorBNet();

    evidences.clear();
    pBnet->GenerateSamples( &evidences, 1 );


    const int ndsToToggle1[] = { 0, 1, 3 };
    evidences[0]->ToggleNodeState( 3, ndsToToggle1 );
    const int *flags = evidences[0]->GetObsNodesFlags();
    std::cout<<"observed nodes"<<std::endl;
    for( i = 0; i < pBnet->GetNumberOfNodes(); i++ )
    {
	if ( flags[i] )
	{
	    std::cout<<"node "<<i<<"; ";
	}
    }
    std::cout<<std::endl<<std::endl;

    const int querySz1 = 2;
     int query1[] = { 0, 1 };

    pLWInf = CLWSamplingInfEngine::Create( pBnet );
    pLWInf->EnterEvidence( evidences[0] );
    pLWInf->MarginalNodes( query1, querySz1 );

    pJTreeInf = CJtreeInfEngine::Create(pBnet);
    pJTreeInf->EnterEvidence( evidences[0] );
    pJTreeInf->MarginalNodes( query1,querySz1 );

    pQueryPot1 = pLWInf->GetQueryJPD();
    pQueryPot2 = pJTreeInf->GetQueryJPD();
		
    std::cout<<"result of LW"<<std::endl<<std::endl;
    pQueryPot1->Dump();
    std::cout<<"result of junction"<<std::endl;
    pQueryPot2->Dump();

    ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
	delete pJTreeInf;
    delete pLWInf;
    delete pBnet;

    return ret;

    ///////////////////////////////////////////////////////////////////////////////

}

int LWForSimplestGaussianBNet(int seed, float eps)
{
    std::cout<<std::endl<<"LW for simplest gaussian BNet (3 nodes) "<<std::endl;

    int nnodes = 3;
    int numnt = 2;
    CNodeType *nodeTypes = new CNodeType[numnt];
    nodeTypes[0] = CNodeType(0,1);
    nodeTypes[1] = CNodeType(0,2);
    intVector nodeAssociation = intVector(nnodes,1);
    nodeAssociation[0] = 0;
    int nbs0[] = { 1 };
    int nbs1[] = { 0, 2 };
    int nbs2[] = { 1 };
    ENeighborType ori0[] = { ntChild };
    ENeighborType ori1[] = { ntParent, ntChild  };
    ENeighborType ori2[] = { ntParent };
    int *nbrs[] = { nbs0, nbs1, nbs2 };
    ENeighborType *orient[] = { ori0, ori1, ori2 };

    intVector numNeighb = intVector(3);
    numNeighb[0] = 1;
    numNeighb[1] = 2;
    numNeighb[2] = 1;

    CGraph *graph;
    graph = CGraph::Create(nnodes, &numNeighb.front(), nbrs, orient);

    CBNet *pBnet = CBNet::Create( nnodes, numnt, nodeTypes,
	&nodeAssociation.front(),graph );
    pBnet->AllocParameters();

    for(int i = 0; i < nnodes; i++ )
    {
	pBnet->AllocParameter(i);

    }

    floatVector data(1,0.0f);
    intVector ranges(2,1);

    ///////////////////////////////////////////////////////////////////
    CNumericDenseMatrix<float> *mean0 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    data[0] = 0.3f;
    CNumericDenseMatrix<float> *cov0 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    pBnet->GetFactor(0)->AttachMatrix( mean0, matMean );
    pBnet->GetFactor(0)->AttachMatrix( cov0, matCovariance );
    /////////////////////////////////////////////////////////////////////

    ranges[0] = 2;
    data.resize(2);
    data[0] = -1.0f;
    data[1] = 0.0f;
    CNumericDenseMatrix<float> *mean1 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    ranges[1] = 2;
    data.resize(4);
    data[0] = 1.0f;
    data[1] = 0.1f;
    data[3] = 3.0f;
    data[2] = 0.1f;
    CNumericDenseMatrix<float> *cov1 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    ranges[1] =1;
    data.resize(2);
    data[0] = 1.0f;
    data[1] = 0.5f;
    CNumericDenseMatrix<float> *weight1 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());


    pBnet->GetFactor(1)->AttachMatrix( mean1, matMean );
    pBnet->GetFactor(1)->AttachMatrix( cov1, matCovariance );
    pBnet->GetFactor(1)->AttachMatrix( weight1, matWeights,0 );
    ///////////////////////////////////////////////////////////////////////////


    ranges[0] = 2;
    data.resize(2);
    data[0] = 1.0f;
    data[1] = 20.5f;
    CNumericDenseMatrix<float> *mean2 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    ranges[1] = 2;
    data.resize(4);
    data[0] = 1.0f;
    data[1] = 0.0f;
    data[3] = 9.0f;
    data[2] = 0.0f;
    CNumericDenseMatrix<float> *cov2 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    data.resize(2);
    data[0] = 1.0f;
    data[1] = 3.5f;
    data[2] = 1.0f;
    data[3] = 0.5f;
    CNumericDenseMatrix<float> *weight2 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    pBnet->GetFactor(2)->AttachMatrix( mean2, matMean );
    pBnet->GetFactor(2)->AttachMatrix( cov2, matCovariance );
    pBnet->GetFactor(2)->AttachMatrix( weight2, matWeights,0 );
    ///////////////////////////////////////////////////////////////////////////

    pEvidencesVector evidences;

    pBnet->GenerateSamples( &evidences, 1 );

	const int ndsToToggle[] = { 0 };
    evidences[0]->ToggleNodeState( 1, ndsToToggle );
	
	//const int ndsToToggle[] = { 0, 1 };
    //evidences[0]->ToggleNodeState( 2, ndsToToggle );

    //intVector query(1,1);
	intVector query(1,0);

    CLWSamplingInfEngine *pLWInf = CLWSamplingInfEngine::Create( pBnet );
    pLWInf->EnterEvidence( evidences[0] );
    pLWInf->MarginalNodes( &query.front(),query.size() );
	
    CNaiveInfEngine *pNaiveInf = CNaiveInfEngine::Create(pBnet);
    pNaiveInf->EnterEvidence( evidences[0] );
    pNaiveInf->MarginalNodes( &query.front(),query.size() );

	const CPotential *pQueryPot1 = pLWInf->GetQueryJPD();
    const CPotential *pQueryPot2 = pNaiveInf->GetQueryJPD();
    std::cout<<"result of LW"<<std::endl<<std::endl;
    pQueryPot1->Dump();
    std::cout<<"result of naive"<<std::endl;
    pQueryPot2->Dump();

    int ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
    delete pNaiveInf;
    delete pLWInf;
    delete pBnet;

	return ret;
}

int LWForScalarGaussianBNet(int seed, float eps)
{
    std::cout<<std::endl<<" Scalar gaussian BNet (5 nodes)"<< std::endl;
    CBNet *pBnet;
    pEvidencesVector evidences;
   
    const CPotential *pQueryPot1, *pQueryPot2;
    int i, ret;

    ////////////////////////////////////////////////////////////////////////
    //Do the example from Satnam Alag's PhD thesis, UCB ME dept 1996 p46
    //Make the following polytree, where all arcs point down
    //
    // 0   1
    //  \ /
    //   2
    //  / \
    // 3   4
    //
    //////////////////////////////////////////////////////////////////////

    int nnodes = 5;
    int numnt = 1;
    CNodeType *nodeTypes = new CNodeType[numnt];
    nodeTypes[0] = CNodeType(0,1);

    intVector nodeAssociation = intVector(nnodes,0);

    int nbs0[] = { 2 };
    int nbs1[] = { 2 };
    int nbs2[] = { 0, 1, 3, 4 };
    int nbs3[] = { 2 };
    int nbs4[] = { 2 };
    ENeighborType ori0[] = { ntChild };
    ENeighborType ori1[] = { ntChild };
    ENeighborType ori2[] = { ntParent, ntParent, ntChild, ntChild };
    ENeighborType ori3[] = { ntParent };
    ENeighborType ori4[] = { ntParent };
    int *nbrs[] = { nbs0, nbs1, nbs2, nbs3, nbs4 };
    ENeighborType *orient[] = { ori0, ori1, ori2, ori3, ori4 };
    intVector numNeighb = intVector(5,1);
    numNeighb[2] = 4;
    CGraph *graph;
    graph = CGraph::Create(nnodes, &numNeighb.front(), nbrs, orient);

    pBnet = CBNet::Create( nnodes, numnt, nodeTypes, &nodeAssociation.front(),graph );
    pBnet->AllocParameters();

    for( i = 0; i < nnodes; i++ )
    {
	pBnet->AllocParameter(i);
    }
    //now we need to create data for parameters - we'll create matrices
    floatVector smData = floatVector(1,0.0f);
    floatVector bigData = floatVector(1,1.0f);
    intVector ranges = intVector(2, 1);
    ranges[0] = 1;
    smData[0] = 1.0f;
    CNumericDenseMatrix<float> *mean0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &smData.front());
    bigData[0] = 4.0f;

    CNumericDenseMatrix<float> *cov0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(0)->AttachMatrix(mean0, matMean);
    pBnet->GetFactor(0)->AttachMatrix(cov0, matCovariance);

    float val = 1.0f;

    CNumericDenseMatrix<float> *mean1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    CNumericDenseMatrix<float> *cov1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    pBnet->GetFactor(1)->AttachMatrix(mean1, matMean);
    pBnet->GetFactor(1)->AttachMatrix(cov1, matCovariance);
    smData[0] = 0.0f;

    CNumericDenseMatrix<float> *mean2 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    smData[0] = 2.0f;

    CNumericDenseMatrix<float> *w21 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    bigData[0] = 2.0f;

    CNumericDenseMatrix<float> *cov2 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[0] = 1.0f;

    CNumericDenseMatrix<float> *w20 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(2)->AttachMatrix( mean2, matMean );
    pBnet->GetFactor(2)->AttachMatrix( cov2, matCovariance );
    pBnet->GetFactor(2)->AttachMatrix( w20, matWeights,0 );
    pBnet->GetFactor(2)->AttachMatrix( w21, matWeights,1 );

    val = 0.0f;

    CNumericDenseMatrix<float> *mean3 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);
    val = 4.0f;
    CNumericDenseMatrix<float> *cov3 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);

    smData[0] = 1.1f;

    CNumericDenseMatrix<float> *w30 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    pBnet->GetFactor(3)->AttachMatrix( mean3, matMean );
    pBnet->GetFactor(3)->AttachMatrix( cov3, matCovariance );
    pBnet->GetFactor(3)->AttachMatrix( w30, matWeights,0 );


    smData[0] = -0.8f;

    CNumericDenseMatrix<float> *mean4 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());

    bigData[0] = 1.2f;

    CNumericDenseMatrix<float> *cov4 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[0] = 2.0f;

    CNumericDenseMatrix<float> *w40 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(4)->AttachMatrix( mean4, matMean );
    pBnet->GetFactor(4)->AttachMatrix( cov4, matCovariance );
    pBnet->GetFactor(4)->AttachMatrix( w40, matWeights,0 );

    evidences.clear();
    pBnet->GenerateSamples( &evidences, 1 );

    const int ndsToToggle2[] = { 0, 1, 2 };
    evidences[0]->ToggleNodeState( 3, ndsToToggle2 );
    const int *flags1 = evidences[0]->GetObsNodesFlags();
    std::cout<<"observed nodes"<<std::endl;
    for( i = 0; i < pBnet->GetNumberOfNodes(); i++ )
    {
	if ( flags1[i] )
	{
	    std::cout<<"node "<<i<<"; ";
	}
    }
    std::cout<<std::endl<<std::endl;

    const int querySz2 = 1;
    const int query2[] = { 0 };

	CLWSamplingInfEngine *pLWInf = CLWSamplingInfEngine::Create( pBnet );
    pLWInf->EnterEvidence( evidences[0] );
    pLWInf->MarginalNodes( query2, querySz2 );

	CNaiveInfEngine *pNaiveInf = CNaiveInfEngine::Create(pBnet);
    pNaiveInf->EnterEvidence( evidences[0] );
    pNaiveInf->MarginalNodes( query2,querySz2 );

    pQueryPot1 = pLWInf->GetQueryJPD();
    pQueryPot2 = pNaiveInf->GetQueryJPD();
	
    std::cout<<"result of LW"<<std::endl<<std::endl;
    pQueryPot1->Dump();
    std::cout<<"result of naive"<<std::endl;
    pQueryPot2->Dump();

    ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
    delete pNaiveInf;
	delete pLWInf;
    delete pBnet;

    return ret;

    ////////////////////////////////////////////////////////////////////////////////////////
}


int LWForMixtureBNet(int seed, float eps)
{
    
   std::cout<<std::endl<<"LW for mixture gaussian BNet"<<std::endl;

    CBNet* pMixBNet = pnlExCreateSimpleGauMix();

    //create evidence for inference
    int nObsNds = 3;
    int obsNds[] = { 2, 3, 4 };
    valueVector obsVals;

    obsVals.assign( 3, (Value)0 );
    obsVals[0].SetFlt(-.5f);
    obsVals[1].SetFlt(2.0f);
    obsVals[2].SetFlt(1.0f);

    CEvidence* evid = CEvidence::Create( pMixBNet, nObsNds, obsNds, obsVals );
    intVector queryNode(1, 0);
   
	//LW inference
	CLWSamplingInfEngine* infLW = CLWSamplingInfEngine::Create( pMixBNet );
	infLW->EnterEvidence( evid);
    infLW->MarginalNodes( &queryNode.front(), 1 );
    const CPotential* pQueryPot1 = infLW->GetQueryJPD();
   
    //naive inference
	CNaiveInfEngine* infNaive = CNaiveInfEngine::Create( pMixBNet );
    infNaive->EnterEvidence( evid);
    infNaive->MarginalNodes( &queryNode.front(), 1 );
    const CPotential* pQueryPot2 = infNaive->GetQueryJPD();

    std::cout<<"Results of LW"<<std::endl;
    pQueryPot1->Dump();
    std::cout<<"Results of Naive"<<std::endl;
    pQueryPot2->Dump();

    int ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evid;
    delete infNaive;
    delete infLW;
    delete pMixBNet;

    return ret;
}

int LWForSingleGaussian(int seed, float eps)
{
    std::cout<<std::endl<<"Using LW for testing samples from gaussian"<<std::endl;

    int nnodes = 1;
    int numnt = 1;
    CNodeType *nodeTypes = new CNodeType[numnt];
    nodeTypes[0] = CNodeType(0,2);
   
    intVector nodeAssociation = intVector(nnodes,0);
   
   
    CGraph *graph;
    graph = CGraph::Create(nnodes, 0, NULL, NULL);

    CBNet *pBnet = CBNet::Create( nnodes, numnt, nodeTypes,
	&nodeAssociation.front(),graph );
    pBnet->AllocParameters();
	pBnet->AllocParameter(0);


    float mean[2] = {0.0f, 0.0f};
    intVector ranges(2,1);
    ranges[0] = 2;

    ///////////////////////////////////////////////////////////////////
    CNumericDenseMatrix<float> *mean0 =	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), mean);

    ranges[1] = 2;
    float cov[4] = {1.0f, 0.3f, 0.3f, 1.0f};
    CNumericDenseMatrix<float> *cov0 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), cov);

    pBnet->GetFactor(0)->AttachMatrix( mean0, matMean );
    pBnet->GetFactor(0)->AttachMatrix( cov0, matCovariance );
    /////////////////////////////////////////////////////////////////////
       
    pEvidencesVector evidences;
    pBnet->GenerateSamples(&evidences, 1 );
    
    const int ndsToToggle[] = { 0 };
    evidences[0]->ToggleNodeState( 1, ndsToToggle );
    
    intVector query(1,0);
    
	// LW inference
	CLWSamplingInfEngine *pLWInf = CLWSamplingInfEngine::Create( pBnet );
    pLWInf->EnterEvidence( evidences[0] );
    pLWInf->MarginalNodes( &query.front(),query.size() );
    const CPotential *pQueryPot1 = pLWInf->GetQueryJPD();
  
	//naive inference
	CNaiveInfEngine* infNaive = CNaiveInfEngine::Create( pBnet );
    infNaive->EnterEvidence( evidences[0]);
    infNaive->MarginalNodes( &query.front(),query.size() );
    const CPotential* pQueryPot2 = infNaive->GetQueryJPD();

    std::cout<<"result of LW"<<std::endl<<std::endl;
    pQueryPot1->Dump();
	std::cout<<"result of naive"<<std::endl<<std::endl;
    pQueryPot2->Dump();

    int ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
    delete pLWInf;
	delete infNaive;
    delete pBnet;

    return ret;

}

int LWForWaterSprinklerBNet(int seed, float eps)
{

	std::cout<<std::endl<<"LW for Water Sprinkler BNet"<<std::endl;

    int ret =1;
    CBNet *pBnet = pnlExCreateWaterSprinklerBNet();
        
	intVector obsNodes(1, 3);
	valueVector obsVals( 1, 0 );
	obsVals[0].SetInt(1);
	CEvidence* pEVid = CEvidence::Create( pBnet, obsNodes, obsVals);
	
	intVector query(1,2);

	// LW inference
	CLWSamplingInfEngine *pLWInf = CLWSamplingInfEngine::Create( pBnet );
    pLWInf->EnterEvidence( pEVid);
    pLWInf->MarginalNodes( &query.front(),query.size() );
    const CPotential *pQueryPot1 = pLWInf->GetQueryJPD();

	// JTree inference
    CJtreeInfEngine *pJTreeInf = CJtreeInfEngine::Create(pBnet);
    pJTreeInf->EnterEvidence(pEVid);
	pJTreeInf->MarginalNodes(&query.front(), query.size());
    const CPotential *pQueryPot2 = pJTreeInf->GetQueryJPD();
   
    std::cout<<"result of LW"<<std::endl<<std::endl;
    pQueryPot1->Dump();
	std::cout<<"result of JTree"<<std::endl<<std::endl;
    pQueryPot2->Dump();

    ret = pQueryPot1-> IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete pEVid;
    delete pLWInf;
    delete pJTreeInf;
    delete pBnet;
	
    return ret;
}


int LWMPEforScalarGaussianBNet(int seed, float eps)
{
    std::cout<<std::endl<<"LW MPE for scalar gaussian BNet"<<std::endl;

    int ret =1;
    CBNet *pBnet = pnlExCreateScalarGaussianBNet();
        
    pEvidencesVector evidences;
    pBnet->GenerateSamples(&evidences, 1 );
    
    const int ndsToToggle[] = { 0, 3 };
	evidences[0]->ToggleNodeState( 2, ndsToToggle );
    intVector query(1, 0);
	
	// LW inference
	CLWSamplingInfEngine *pLWInf = CLWSamplingInfEngine::Create( pBnet );
    pLWInf->EnterEvidence( evidences[0], 1 );
    pLWInf->MarginalNodes( &query.front(),query.size() );
	const CEvidence *pEvLW = pLWInf->GetMPE();

	//Jtree inference
    CJtreeInfEngine *pJTreeInf = CJtreeInfEngine::Create(pBnet);
    pJTreeInf->EnterEvidence(evidences[0], 1);
    pJTreeInf->MarginalNodes(&query.front(), query.size());
    const CEvidence* pEvJTree = pJTreeInf->GetMPE();
    
    std::cout<<"result of LW"<<std::endl<<std::endl;
    pEvLW->Dump();
	std::cout<<"result of JTree"<<std::endl<<std::endl;
    pEvJTree->Dump();
    
    delete evidences[0];
   
    delete pLWInf;
    delete pJTreeInf;
    delete pBnet;
	
    return ret;
}

void initALWInference()
{
    trsReg(func_name, test_desc, test_class, testLWInference);
}





















