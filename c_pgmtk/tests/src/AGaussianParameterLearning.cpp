/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGaussianParameterLearning.cpp                              //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
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
#include "tConfig.h"

PNL_USING

static char func_name[] = "testGaussianFactorLearningEngine";

static char* test_desc = "Provide Factor Learning for Gaussian nodes";

static char* test_class = "Algorithm";

int testGaussianFactorLearningEngine()
{
    int ret = TRS_OK;
    float eps = -1.0f;
    float epsForComp = -1.0f;

    while( epsForComp <= 0)
    {
        trssRead( &epsForComp, "0.1f", "accuracy in test");
    }
    while( eps <= 0)
    {
        trssRead( &eps, "0.1f", "for stop expression");
    }

    int maxIter = -10;
    while( maxIter <= 0)
    {
        trsiRead( &maxIter, "10", "maximum number of iteration");
    }

    /*
    Do the example from Satnam Alag's PhD thesis, UCB ME dept 1996 p46
    Make the following polytree, where all arcs point down
    
    0   1
     \ /
      2
     / \
    3   4
      
    */
    int i;
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
    graph = CGraph::Create( nnodes, &numNeighb.front(), nbrs, orient );
    CBNet *bnet;
    
    bnet = CBNet::Create( nnodes, numnt, nodeTypes, &nodeAssociation.front(), graph );
    bnet->AllocFactors();
    for( i = 0; i < nnodes; i++ )
    {
        bnet->AllocFactor(i);
    }
    //now we need to create data for factors - we'll create matrices
    floatVector smData = floatVector(2,0.0f);
    floatVector bigData = floatVector(4,1.0f);
    intVector ranges = intVector(2, 1);
    ranges[0] = 2;
    smData[0] = 1.0f;
    CNumericDenseMatrix<float> *mean0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &smData.front());
    bigData[0] = 4.0f;
    bigData[3] = 4.0f;
    //bigData[0] = 2.0f;
    //bigData[3] = 1.0f;
    ranges[1] = 2;
    CNumericDenseMatrix<float> *cov0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &bigData.front() );
    bnet->GetFactor(0)->AttachMatrix(mean0, matMean);
    bnet->GetFactor(0)->AttachMatrix(cov0, matCovariance);
    ranges[0] = 1;
    ranges[1] = 1;
    float val = 1.0f;
    //float val = 2.0f;
    CNumericDenseMatrix<float> *mean1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    CNumericDenseMatrix<float> *cov1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    bnet->GetFactor(1)->AttachMatrix(mean1, matMean);
    bnet->GetFactor(1)->AttachMatrix(cov1, matCovariance);
    smData[0] = 0.0f;
    smData[1] = 0.0f;
    //smData[0] = 3.0f;
    //smData[1] = 4.0f;
    ranges[0] = 2;
    CNumericDenseMatrix<float> *mean2 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &smData.front() );
    smData[0] = 2.0f;
    smData[1] = 1.0f;
    CNumericDenseMatrix<float> *w21 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    bigData[0] = 2.0f;
    bigData[1] = 1.0f;
    bigData[2] = 1.0f;
    bigData[3] = 1.0f;
    ranges[1] = 2;
    CNumericDenseMatrix<float> *cov2 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[0] = 1.0f;
    bigData[1] = 2.0f;
    bigData[2] = 1.0f;
    bigData[3] = 0.0f;
    CNumericDenseMatrix<float> *w20 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bnet->GetFactor(2)->AttachMatrix( mean2, matMean );
    bnet->GetFactor(2)->AttachMatrix( cov2, matCovariance );
    bnet->GetFactor(2)->AttachMatrix( w20, matWeights,0 );
    bnet->GetFactor(2)->AttachMatrix( w21, matWeights,1 );
    
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
    bnet->GetFactor(3)->AttachMatrix( mean3, matMean );
    bnet->GetFactor(3)->AttachMatrix( cov3, matCovariance );
    bnet->GetFactor(3)->AttachMatrix( w30, matWeights,0 );
    
    ranges[0] = 2;
    ranges[1] = 1;
    smData[0] = 0.0f;
    smData[1] = 0.0f;
    CNumericDenseMatrix<float> *mean4 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    ranges[1] = 2;
    bigData[0] = 1.0f;
    bigData[1] = 0.0f;
    bigData[2] = 0.0f;
    bigData[3] = 1.0f;
    CNumericDenseMatrix<float> *cov4 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[2] = 1.0f;
    CNumericDenseMatrix<float> *w40 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bnet->GetFactor(4)->AttachMatrix( mean4, matMean );
    bnet->GetFactor(4)->AttachMatrix( cov4, matCovariance );
    bnet->GetFactor(4)->AttachMatrix( w40, matWeights,0 );
    CBNet *bnetToLearn = 
	CBNet::CreateWithRandomMatrices(CGraph::Copy(bnet->GetGraph()), bnet->GetModelDomain());
    
    ///////////////////////////////////////////////////////////////////////
    int nEv = 5000;
    
    
    CInfEngine *pInfEng =NULL;
    // pInfEng = CNaiveInfEngine::Create( bnetToLearn);
    CEMLearningEngine *pLearn = CEMLearningEngine::Create(bnetToLearn, pInfEng);
    pEvidencesVector pEvVec;
    bnet->GenerateSamples(&pEvVec, nEv);
    float llBest = 0.0f;
    for( i = 0; i < nEv; i++ )
    {
	llBest += bnet->ComputeLogLik(pEvVec[i]);
    }
    std::cout<<"\n likelihood for initial model (if all observed)= "<< llBest <<std::endl;
    
    
    int toggle[]={0, 1};
    for( i = 0; i < nEv; i++ )
    {
	pEvVec[i]->ToggleNodeState(2, toggle);
	
    }
    
    pLearn -> SetData(nEv, &pEvVec.front());
    pLearn->SetMaxIterEM(maxIter);
    pLearn->SetTerminationToleranceEM(eps);
    trsTimerStart(0);
    pLearn ->Learn();
    double time = trsTimerClock(0);
    printf( "timing of learning procedure %f \n", time);
    
    int nsteps;
    const float * score;
    pLearn->GetCriterionValue(&nsteps, &score);
    for(int step = 0; step < nsteps; step++)
    {
        std::cout<<"step"<<step<<" "<<score[step];
    }
    
    float llRes = score[nsteps - 1];
    if( llRes < llBest )
    {
	if( llRes - llBest >epsForComp )
	{
	    ret = TRS_FAIL;
	}
    }
	
    delete(pInfEng);
    for( i = 0; i < nEv; i++)
    {
        delete pEvVec[i];
    }
    delete(pLearn);
    delete bnetToLearn;
    delete(bnet);
    delete []nodeTypes;
    
    
    return trsResult( ret, ret == TRS_OK ? "No errors" :
    "Bad test on ParamLearningEngine");
}

void initAGaussianFactorLearningEngine()
{
    trsReg(func_name, test_desc, test_class, testGaussianFactorLearningEngine);
}
