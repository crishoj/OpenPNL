/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGaussianInference.cpp                                      //
//                                                                         //
//  Purpose:   Test on inference for Gaussian BNet model                   //
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

static char func_name[] = "testGaussianInf";

static char* test_desc = "Provide inference(all existing types) for Gaussian BNet model ";

static char* test_class = "Algorithm";

int testGaussianInf()
{

    int ret = TRS_OK;
    float eps = 1e-2f;
    int i;

    //create very simple model
    //0->1->2
    //n = 3
    int nSimNodes = 3;
    int numAdjMatDims = 2;
    int rangesS[] = {nSimNodes, nSimNodes};
    intVector matrixData;
    matrixData.assign( nSimNodes*nSimNodes, 0 );
    CDenseMatrix<int>* adjMat = CDenseMatrix<int>::Create( numAdjMatDims,
        rangesS, &matrixData.front() );
    int indices[] = { 0, 1 };
	adjMat->SetElementByIndexes(1, indices);
    indices[0] = 1;
    indices[1] = 2;
    adjMat->SetElementByIndexes(1, indices);

    // this is a creation of directed graph for the BNet model based on adjacency matrix
    CGraph* pSimGraph = CGraph::Create( adjMat );

    //create Model Domain
    CNodeType simNT = CNodeType(0, 2);
    CModelDomain* pSimDomain = CModelDomain::Create( nSimNodes, simNT );

    CBNet* pSimBNet = CBNet::Create( pSimGraph, pSimDomain );
    //need to alloc matrices
    float meanS0[] = { 1.0f, 2.0f} ; 
    float covS0[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float meanS1[] = { -0.1f, -.2f };
    float covS1[] = { 1.0f, 0.5f, 0.5f, 1.0f };
    float weightS01[] = { 3.0f, -2.0f, 1.0f, -1.0f };
    float meanS2[] = { 1.5f, -2.0f };
    float covS2[] = { 2.0f, 1.0f, 1.0f, 2.0f };
    float weightS12[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float* meansS[] = { meanS0, meanS1, meanS2 };
    float* covsS[] = { covS0, covS1, covS2 };
    float* weightsS[] = { weightS01, weightS12 };
    pSimBNet->AllocFactors();
    for( i = 0; i < nSimNodes; i++ )
    {
        pSimBNet->AllocFactor(i);
        CGaussianCPD* curCPD = static_cast<CGaussianCPD*>(pSimBNet->GetFactor(i));
        curCPD->AllocMatrix( meansS[i], matMean );
        curCPD->AllocMatrix( covsS[i], matCovariance );
        //add some code coverage
        std::string descr;
        curCPD->IsValid(&descr);
#if 1
        std::cout<<descr<<std::endl;
#endif
        if( i > 0 )
        {
            curCPD->AllocMatrix( weightsS[i-1], matWeights, 0 );
        }
    }
    //create evidence
    CEvidence* pSimEmptyEv = CEvidence::Create( pSimDomain, 0, NULL, valueVector() );
    CInfEngine* pSimInf = CNaiveInfEngine::Create(pSimBNet);
    pSimInf->EnterEvidence( pSimEmptyEv );
    int queryS = 0;
    pSimInf->MarginalNodes( &queryS, 1 );
#if 0
    const CPotential* pPotS = pSimInf->GetQueryJPD();
    pPotS->Dump();
#endif
    //start the inference for MPE
    pSimInf->EnterEvidence(pSimEmptyEv, 1);
    pSimInf->MarginalNodes( &queryS, 1 );
#if 1   
    const CEvidence* pSimMPE = pSimInf->GetMPE();
    pSimMPE->Dump();
#endif
    delete pSimEmptyEv;
    delete pSimInf;
    delete pSimBNet;
    delete pSimDomain;
    delete adjMat;
    //we create very small model to start inference on it
    // the model is from Kevin Murphy's BNT\examples\static\belprop_polytree_gaussain
    /*
    Do the example from Satnam Alag's PhD thesis, UCB ME dept 1996 p46
    Make the following polytree, where all arcs point down
    
     0   1
      \ /
       2
      / \
     3   4

    N = 5;
    dag = zeros(N,N);
    dag(1,3) = 1;
    dag(2,3) = 1;
    dag(3, [4 5]) = 1;
    ns = [2 1 2 1 2];
    bnet = mk_bnet(dag, ns, 'discrete', []);
    */
    //create this model
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
    bnet = CBNet::Create(nnodes, numnt, nodeTypes, &nodeAssociation.front(), graph );
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
    //bnet.CPD{0} = gaussianCPD(bnet, 1, 'mean', [1 0]', 'cov', [4 1; 1 4]);
    smData[0] = 1.0f;
    CNumericDenseMatrix<float> *mean0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &smData.front());
    bigData[0] = 4.0f;
    bigData[3] = 4.0f;
    ranges[1] = 2;
    CNumericDenseMatrix<float> *cov0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &bigData.front());
    bnet->GetFactor(0)->AttachMatrix(mean0, matMean);
    bnet->GetFactor(0)->AttachMatrix(cov0, matCovariance);
    //bnet.CPD{1} = gaussianCPD(bnet, 2, 'mean', 1, 'cov', 1);
    ranges[0] = 1;
    ranges[1] = 1;
    float val = 1.0f;
    CNumericDenseMatrix<float> *mean1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    CNumericDenseMatrix<float> *cov1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    bnet->GetFactor(1)->AttachMatrix(mean1, matMean);
    bnet->GetFactor(1)->AttachMatrix(cov1, matCovariance);
    //B1 = [1 2; 1 0]; B2 = [2 1]';
    //bnet.CPD{2} = gaussianCPD(bnet, 3, 'mean', [0 0]', 'cov', [2 1; 1 1], ...
    //		   'weights', [B1 B2]);
    smData[0] = 0.0f;
    smData[1] = 0.0f;
    ranges[0] = 2;
    CNumericDenseMatrix<float> *mean2 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
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
    //	H1 = [1 1];
    //bnet.CPD{3} = gaussianCPD(bnet, 4, 'mean', 0, 'cov', 1, 'weights', H1);
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
    //	H2 = [1 0; 1 1];
    //	bnet.CPD{4} = gaussianCPD(bnet, 5, 'mean', [0 0]', 'cov', eye(2), 'weights', H2);
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
    
    //we start inference without evidence
    CEvidence *emptyEv = CEvidence::Create(bnet, 0, NULL, valueVector());
    //start NaiveInf
    CInfEngine *infer = CNaiveInfEngine::Create(bnet );
    infer->EnterEvidence(emptyEv);
    intVector query = intVector(2,2);
    const floatVector *res = NULL;
    infer->MarginalNodes( &query.front(), 1 );
    //m = marginal_nodes(engine{e}, 3, add_ev);
    //assert(approxeq(m.mu, [3 2]'))
    //assert(approxeq(m.Sigma, [30 9; 9 6]))
    const CPotential *jpd2 = infer->GetQueryJPD();
    CNumericDenseMatrix<float> *meanRes2 = static_cast<CNumericDenseMatrix<
        float>*>(jpd2->GetMatrix(matMean));
    res = meanRes2->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-3.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-2.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    CNumericDenseMatrix<float> *covRes2 = static_cast<CNumericDenseMatrix<
        float>*>(jpd2->GetMatrix(matCovariance));
    res = covRes2->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-30.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-9.0f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]-9.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-6.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    //  m = marginal_nodes(engine{e}, 4, add_ev);
    //  assert(approxeq(m.mu, 5))
    // assert(approxeq(m.Sigma, 55))
    query[0] = 3;
    infer->MarginalNodes( &query.front(), 1 );
    const CPotential* jpd3 = infer->GetQueryJPD();
    CNumericDenseMatrix<float> *meanRes3 = static_cast<CNumericDenseMatrix<
        float>*>(jpd3->GetMatrix(matMean));
    res = meanRes3->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-5.0f))>eps )
    {
        ret = TRS_FAIL;
    }	
    CNumericDenseMatrix<float> *covRes3 = static_cast<CNumericDenseMatrix<
        float>*>(jpd3->GetMatrix(matCovariance));
    res = covRes3->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-55.0f))>eps )
    {
        ret = TRS_FAIL;
    }	
    // m = marginal_nodes(engine{e}, 5, add_ev);
    //assert(approxeq(m.mu, [3 5]'))
    //assert(approxeq(m.Sigma, [31 39; 39 55]))
    query[0] = 4;
    infer->MarginalNodes( &query.front(), 1 );
    const CPotential* jpd4 = infer->GetQueryJPD();
    CNumericDenseMatrix<float> *meanRes4 = static_cast<CNumericDenseMatrix<
        float>*>(jpd4->GetMatrix(matMean));
    res = meanRes4->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-3.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-5.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    CNumericDenseMatrix<float> *covRes4 = static_cast<CNumericDenseMatrix<
        float>*>(jpd4->GetMatrix(matCovariance));
    res = covRes4->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-31.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-39.0f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]-39.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-55.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    
    //add evidence to the model
    valueVector obsValues;
    obsValues.assign( 2, (Value)0 );
    obsValues[0].SetFlt(5.0f);
    obsValues[1].SetFlt(5.0f);
    int obsNum = 4;
    CEvidence *Ev4 = CEvidence::Create(bnet, 1, &obsNum, obsValues);
    CInfEngine *inferE4 = CNaiveInfEngine::Create( bnet );
    query[0] = 2;
    inferE4->EnterEvidence(Ev4);
    inferE4->MarginalNodes( &query.front(), 1 );
    //m = marginal_nodes(engine{e}, 3, add_ev);
    //assert(approxeq(m.mu, [4.4022 1.0217]'))
    //assert(approxeq(m.Sigma, [0.7011 -0.4891; -0.4891 1.1087]))
    jpd2 = inferE4->GetQueryJPD();
    meanRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matMean));
    res = meanRes2->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-4.4022f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-1.0217f))>eps )
    {
        ret = TRS_FAIL;
    }
    covRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matCovariance));
    res = covRes2->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-0.7011f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]+0.4891f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]+0.4891f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-1.1087f))>eps )
    {
        ret = TRS_FAIL;
    }
    //m = marginal_nodes(engine{e}, 4, add_ev);
    //assert(approxeq(m.mu, 5.4239))
    //assert(approxeq(m.Sigma, 1.8315))
    query[0] = 3;
    inferE4->MarginalNodes( &query.front(), 1 );
    jpd3 = inferE4->GetQueryJPD();
    meanRes3 = static_cast<CNumericDenseMatrix<float>*>(jpd3->GetMatrix(matMean));
    res = meanRes3->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-5.4239f))>eps )
    {
        ret = TRS_FAIL;
    }	
    covRes3 = static_cast<CNumericDenseMatrix<float>*>(jpd3->GetMatrix(matCovariance));
    res = covRes3->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-1.8315f))>eps )
    {
        ret = TRS_FAIL;
    }	
    //m = marginal_nodes(engine{e}, 5, add_ev);
    //assert(approxeq(m.mu, evidence{5}))
    //assert(approxeq(m.Sigma, zeros(2)))
    query[0] = 4;
    inferE4->MarginalNodes( &query.front(), 1 );
    jpd4 = inferE4->GetQueryJPD();
    meanRes4 = static_cast<CNumericDenseMatrix<float>*>(jpd4->GetMatrix(matMean));
    res = meanRes4->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-5.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-5.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( jpd4->IsDistributionSpecific() != 2 )
    {
        covRes4 = static_cast<CNumericDenseMatrix<float>*>(jpd4->GetMatrix(matCovariance));
        res = covRes4->GetVector();
        if( res->size() != 4)
        {
            ret = TRS_FAIL;
        }
        if( (fabs((*res)[0]-0.0f))>eps )
        {	
            ret = TRS_FAIL;
        }
        if( (fabs((*res)[1]-0.0f))>eps )
        {
            ret = TRS_FAIL;
        }	
        if( (fabs((*res)[2]-0.0f))>eps )
        {
            ret = TRS_FAIL;
        }
        
        if( (fabs((*res)[3]-0.0f))>eps )
        {
            ret = TRS_FAIL;
        }
    }
    //m = marginal_nodes(engine{e}, 1, add_ev);
    //assert(approxeq(m.mu, [0.3478 1.1413]'))
    //assert(approxeq(m.Sigma, [1.8261 -0.1957; -0.1957 1.0924]))
    query[0] = 0;
    inferE4->MarginalNodes( &query.front(), 1 );
    const CPotential* jpd0 = inferE4->GetQueryJPD();
    CNumericDenseMatrix<float> *meanRes0 = static_cast<CNumericDenseMatrix<
        float>*>(jpd0->GetMatrix(matMean));
    res = meanRes0->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-0.3478f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-1.1413f))>eps )
    {
        ret = TRS_FAIL;
    }
    CNumericDenseMatrix<float>* covRes0 = static_cast<CNumericDenseMatrix<
        float>*>(jpd0->GetMatrix(matCovariance));
    res = covRes0->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-1.8261f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]+0.1957f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]+0.1957f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-1.0924f))>eps )
    {
        ret = TRS_FAIL;
    }
    //m = marginal_nodes(engine{e}, 2, add_ev);
    //assert(approxeq(m.mu, 0.9239))
    //assert(approxeq(m.Sigma, 0.8315))
    query[0] = 1;
    inferE4->MarginalNodes( &query.front(), 1 );
    const CPotential *jpd1 = inferE4->GetQueryJPD();
    CNumericDenseMatrix<float> *meanRes1 = static_cast<CNumericDenseMatrix<
        float>*>(jpd1->GetMatrix(matMean));
    res = meanRes1->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-0.9239f))>eps )
    {
        ret = TRS_FAIL;
    }	
    CNumericDenseMatrix<float> *covRes1 = static_cast<CNumericDenseMatrix<
        float>*>(jpd1->GetMatrix(matCovariance));
	res = covRes1->GetVector();
	if( res->size() != 1 )
	{
		ret = TRS_FAIL;
	}
	if( (fabs((*res)[0]-0.8315f))>eps )
	{
		ret = TRS_FAIL;
	}	
	//add evidence on 3 nodes
	//% evidence on leaves 4,5 and root 2
	//evidence = cell(1,N);
	//evidence{2} = 0;
	//evidence{4} = 10;
	//evidence{5} = [5 5]';
	//for e=1:E
	//[engine{e}, ll(e)] = enter_evidence(engine{e}, evidence);
	//add_ev = 1;

    query.resize( 5, 0 );
    query[1] = 1;
    query[2] = 2;
    query[3] = 3;
    query[4] = 4;
    //inferE4->MarginalNodes( &query.front(), query.size() );
    //jpd0 = inferE4->GetQueryJPD();


    intVector obsNodes = intVector(3);
    obsNodes[0] = 1;
    obsNodes[1] = 3;
    obsNodes[2] = 4;
    valueVector pEvid;
    pEvid.assign( 4, (Value)0 );
    pEvid[0].SetFlt(0.0f);
    pEvid[1].SetFlt(10.0f);
    pEvid[2].SetFlt(5.0f);
    pEvid[3].SetFlt(5.0f);
    CEvidence *EvNodes = CEvidence::Create( bnet, 3, &obsNodes.front(), pEvid );
    //start Naive inference with such evidence
    CInfEngine *inferENodes = CNaiveInfEngine::Create( bnet );
    query[0] = 2;
    inferENodes->EnterEvidence(EvNodes);
    inferENodes->MarginalNodes( &query.front(), 1 );
    //m = marginal_nodes(engine{e}, 3, add_ev);
    //assert(approxeq(m.mu, [4.9964 2.4444]'));
    //assert(approxeq(m.Sigma, [0.6738 -0.5556; -0.5556 0.8889]));
    jpd2 = inferENodes->GetQueryJPD();
    meanRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matMean));
    res = meanRes2->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-4.9964f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-2.4444f))>eps )
    {
        ret = TRS_FAIL;
    }
    covRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matCovariance));
    res = covRes2->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-0.6738f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]+0.5556f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]+0.5556f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-0.8889f))>eps )
    {
        ret = TRS_FAIL;
    }	
    //  m = marginal_nodes(engine{e}, 1, add_ev);
    //assert(approxeq(m.mu, [2.2043 1.2151]'));
    //assert(approxeq(m.Sigma, [1.2903 -0.4839; -0.4839 0.8065]));
    query[0] = 0;
    inferENodes->MarginalNodes( &query.front(), 1 );
    jpd0 = inferENodes->GetQueryJPD();
    meanRes0 = static_cast<CNumericDenseMatrix<float>*>(jpd0->GetMatrix(matMean));
    res = meanRes0->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-2.2043f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-1.2151f))>eps )
    {
        ret = TRS_FAIL;
    }
    covRes0 = static_cast<CNumericDenseMatrix<float>*>(jpd0->GetMatrix(matCovariance));
    res = covRes0->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-1.2903f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]+0.4839f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]+0.4839f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-0.8056f))>eps )
    {
        ret = TRS_FAIL;
    }
    
    query.resize( 5, 0 );
    query[1] = 1;
    query[2] = 2;
    query[3] = 3;
    query[4] = 4;
    //inferENodes->MarginalNodes( &query.front(), query.size() );
    //jpd0 = inferENodes->GetQueryJPD();
    //meanRes0 = static_cast<CNumericDenseMatrix<float>*>(jpd0->GetMatrix(matMean));
    //res = meanRes0->GetVector();
    
    //start PearlInf
    CInfEngine *infer2 = CPearlInfEngine::Create( bnet );
    infer2->EnterEvidence(emptyEv);
    query = intVector(2,2);
    res = NULL;
    infer2->MarginalNodes( &query.front(), 1 );
    //m = marginal_nodes(engine{e}, 3, add_ev);
    //assert(approxeq(m.mu, [3 2]'))
    //assert(approxeq(m.Sigma, [30 9; 9 6]))
    jpd2 = infer2->GetQueryJPD();
    meanRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matMean));
    res = meanRes2->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-3.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-2.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    covRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matCovariance));
    res = covRes2->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-30.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-9.0f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]-9.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-6.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    //  m = marginal_nodes(engine{e}, 4, add_ev);
    //  assert(approxeq(m.mu, 5))
    // assert(approxeq(m.Sigma, 55))
    query[0] = 3;
    infer2->MarginalNodes( &query.front(), 1 );
    jpd3 = infer2->GetQueryJPD();
    meanRes3 = static_cast<CNumericDenseMatrix<float>*>(jpd3->GetMatrix(matMean));
    res = meanRes3->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-5.0f))>eps )
    {
        ret = TRS_FAIL;
    }	
    covRes3 = static_cast<CNumericDenseMatrix<float>*>(jpd3->GetMatrix(matCovariance));
    res = covRes3->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-55.0f))>eps )
    {
        ret = TRS_FAIL;
    }	
    // m = marginal_nodes(engine{e}, 5, add_ev);
    //assert(approxeq(m.mu, [3 5]'))
    //assert(approxeq(m.Sigma, [31 39; 39 55]))
    query[0] = 4;
    infer2->MarginalNodes( &query.front(), 1 );
    jpd4 = infer2->GetQueryJPD();
    meanRes4 = static_cast<CNumericDenseMatrix<float>*>(jpd4->GetMatrix(matMean));
    res = meanRes4->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-3.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-5.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    covRes4 = static_cast<CNumericDenseMatrix<float>*>(jpd4->GetMatrix(matCovariance));
    res = covRes4->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-31.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-39.0f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]-39.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-55.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    //add evidence to the model
    CInfEngine *infer2E4 = CPearlInfEngine::Create( bnet );
    query[0] = 2;
    infer2E4->EnterEvidence(Ev4);
    infer2E4->MarginalNodes(&query.front(), 1);
    //m = marginal_nodes(engine{e}, 3, add_ev);
    //assert(approxeq(m.mu, [4.4022 1.0217]'))
    //assert(approxeq(m.Sigma, [0.7011 -0.4891; -0.4891 1.1087]))
    jpd2 = infer2E4->GetQueryJPD();
    meanRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matMean));
    res = meanRes2->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-4.4022f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-1.0217f))>eps )
    {
        ret = TRS_FAIL;
    }
    covRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matCovariance));
    res = covRes2->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-0.7011f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]+0.4891f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]+0.4891f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-1.1087f))>eps )
    {
        ret = TRS_FAIL;
    }
    //m = marginal_nodes(engine{e}, 4, add_ev);
    //assert(approxeq(m.mu, 5.4239))
    //assert(approxeq(m.Sigma, 1.8315))
    query[0] = 3;
    infer2E4->MarginalNodes(&query.front(), 1);
    jpd3 = infer2E4->GetQueryJPD();
    meanRes3 = static_cast<CNumericDenseMatrix<float>*>(jpd3->GetMatrix(matMean));
    res = meanRes3->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-5.4239f))>eps )
    {
        ret = TRS_FAIL;
    }	
    covRes3 = static_cast<CNumericDenseMatrix<float>*>(jpd3->GetMatrix(matCovariance));
    res = covRes3->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-1.8315f))>eps )
    {
        ret = TRS_FAIL;
    }	
    //m = marginal_nodes(engine{e}, 5, add_ev);
    //assert(approxeq(m.mu, evidence{5}))
    //assert(approxeq(m.Sigma, zeros(2)))
    query[0] = 4;
    infer2E4->MarginalNodes(&query.front(), 1);
    jpd4 = infer2E4->GetQueryJPD();
    meanRes4 = static_cast<CNumericDenseMatrix<float>*>(jpd4->GetMatrix(matMean));
    res = meanRes4->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-5.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-5.0f))>eps )
    {
        ret = TRS_FAIL;
    }
    //check for delta distribution - it haven't covariance
    if( jpd4->IsDistributionSpecific() != 2 )
    {
        covRes4 = static_cast<CNumericDenseMatrix<float>*>(jpd4->GetMatrix(matCovariance));
        res = covRes4->GetVector();
        if( res->size() != 4)
        {
            ret = TRS_FAIL;
        }
        if( (fabs((*res)[0]-0.0f))>eps )
        {
            ret = TRS_FAIL;
        }
        if( (fabs((*res)[1]-0.0f))>eps )
        {
            ret = TRS_FAIL;
        }	
        if( (fabs((*res)[2]-0.0f))>eps )
        {
            ret = TRS_FAIL;
        }
        if( (fabs((*res)[3]-0.0f))>eps )
        {
            ret = TRS_FAIL;
        }
    }
    //m = marginal_nodes(engine{e}, 1, add_ev);
    //assert(approxeq(m.mu, [0.3478 1.1413]'))
    //assert(approxeq(m.Sigma, [1.8261 -0.1957; -0.1957 1.0924]))
    query[0] = 0;
    infer2E4->MarginalNodes(&query.front(), 1);
    jpd0 = infer2E4->GetQueryJPD();
    meanRes0 = static_cast<CNumericDenseMatrix<float>*>(jpd0->GetMatrix(matMean));
    res = meanRes0->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-0.3478f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-1.1413f))>eps )
    {
        ret = TRS_FAIL;
    }
    covRes0 = static_cast<CNumericDenseMatrix<float>*>(jpd0->GetMatrix(matCovariance));
    res = covRes0->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-1.8261f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]+0.1957f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]+0.1957f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-1.0924f))>eps )
    {
        ret = TRS_FAIL;
    }
    //m = marginal_nodes(engine{e}, 2, add_ev);
    //assert(approxeq(m.mu, 0.9239))
    //assert(approxeq(m.Sigma, 0.8315))
    query[0] = 1;
    infer2E4->MarginalNodes(&query.front(), 1);
    jpd1 = infer2E4->GetQueryJPD();
    meanRes1 = static_cast<CNumericDenseMatrix<float>*>(jpd1->GetMatrix(matMean));
    res = meanRes1->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-0.9239f))>eps )
    {
        ret = TRS_FAIL;
    }	
    covRes1 = static_cast<CNumericDenseMatrix<float>*>(jpd1->GetMatrix(matCovariance));
    res = covRes1->GetVector();
    if( res->size() != 1 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-0.8315f))>eps )
    {
        ret = TRS_FAIL;
    }
    
    //start Pearl inference with such evidence
    CInfEngine *infer2ENodes = CPearlInfEngine::Create( bnet );
    query[0] = 2;
    infer2ENodes->EnterEvidence(EvNodes);
    infer2ENodes->MarginalNodes(&query.front(), 1);
    //m = marginal_nodes(engine{e}, 3, add_ev);
    //assert(approxeq(m.mu, [4.9964 2.4444]'));
    //assert(approxeq(m.Sigma, [0.6738 -0.5556; -0.5556 0.8889]));
    jpd2 = infer2ENodes->GetQueryJPD();
    meanRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matMean));
    res = meanRes2->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-4.9964f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-2.4444f))>eps )
    {
        ret = TRS_FAIL;
    }
    covRes2 = static_cast<CNumericDenseMatrix<float>*>(jpd2->GetMatrix(matCovariance));
    res = covRes2->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-0.6738f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]+0.5556f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]+0.5556f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-0.8889f))>eps )
    {
        ret = TRS_FAIL;
    }	
    //  m = marginal_nodes(engine{e}, 1, add_ev);
    //assert(approxeq(m.mu, [2.2043 1.2151]'));
    //assert(approxeq(m.Sigma, [1.2903 -0.4839; -0.4839 0.8065]));
    query[0] = 0;
    infer2ENodes->MarginalNodes(&query.front(), 1);
    jpd0 = infer2ENodes->GetQueryJPD();
    meanRes0 = static_cast<CNumericDenseMatrix<float>*>(jpd0->GetMatrix(matMean));
    res = meanRes0->GetVector();
    if( res->size() != 2 )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-2.2043f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]-1.2151f))>eps )
    {
        ret = TRS_FAIL;
    }
    covRes0 = static_cast<CNumericDenseMatrix<float>*>(jpd0->GetMatrix(matCovariance));
    res = covRes0->GetVector();
    if( res->size() != 4)
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[0]-1.2903f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[1]+0.4839f))>eps )
    {
        ret = TRS_FAIL;
    }	
    if( (fabs((*res)[2]+0.4839f))>eps )
    {
        ret = TRS_FAIL;
    }
    if( (fabs((*res)[3]-0.8056f))>eps )
    {
        ret = TRS_FAIL;
    }
    delete EvNodes;
    delete infer2ENodes;
    delete infer2E4;	
    delete emptyEv;
    delete infer;
    delete infer2;
    delete inferE4;
    delete Ev4;
    delete inferENodes;
    delete bnet;
    delete []nodeTypes;
    
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on GaussianInf");
}

void initAGaussianInf()
{
    trsReg(func_name, test_desc, test_class, testGaussianInf);
}
