/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      tCreateArHMMwithGaussObs.cpp                                //
//                                                                         //
//  Purpose:   Implementation of the ArHMM with Gaussian Observed nodes    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "tCreateArHMMwithGaussObs.hpp"
PNL_USING
CDBN* tCreateArHMMwithGaussObs()
{
    
/*
Make an HMM with autoregressive gaussian observations
X0 ->X1
|     | 
v     v
Y0 ->Y1 
    */
    
    
    //Create static model
    const int nnodes = 4;//Number of nodes    
    int numNt = 2;//number of Node types (all nodes are discrete)
    CNodeType *nodeTypes = new CNodeType [numNt];
    nodeTypes[0] = CNodeType( 1,2 );
    nodeTypes[1] = CNodeType( 0,1 );
    int nodeAssociation[] = { 0, 1, 0, 1 };
    
    //Create a dag
    int numOfNeigh[] = { 2, 2, 2, 2 };
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
    
    CGraph* pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );
    
    //Create static BNet
    CBNet *pBNet = 
        CBNet::Create( nnodes, numNt, nodeTypes, nodeAssociation, pGraph );
    pBNet->AllocFactors();
    
    //Let arbitrary distribution is
    float tableNode0[] = { 0.95f, 0.05f };
    float tableNode2[] = { 0.1f, 0.9f, 0.5f, 0.5f };
    
    float mean1w0 = -3.2f; float cov1w0 = 0.00002f; 
    float mean1w1 = -0.5f; float cov1w1 = 0.0001f;

    float mean3w0 = 6.5f; float cov3w0 = 0.03f; float weight3w0 = 1.0f;
    float mean3w1 = 7.5f; float cov3w1 = 0.04f; float weight3w1 = 0.5f;
    
    
    pBNet->AllocFactor( 0 );
    pBNet->GetFactor(0)->AllocMatrix( tableNode0, matTable );
    
    pBNet->AllocFactor( 2 );
    pBNet->GetFactor(2)->AllocMatrix( tableNode2, matTable );
    
    int parent[] = { 0 };
    
    pBNet->AllocFactor( 1 );
    pBNet->GetFactor(1)->AllocMatrix( &mean1w0, matMean, -1, parent );
    pBNet->GetFactor(1)->AllocMatrix( &cov1w0, matCovariance, -1, parent );

    parent[0] = 1;
    
    pBNet->GetFactor(1)->AllocMatrix( &mean1w1, matMean, -1, parent );
    pBNet->GetFactor(1)->AllocMatrix( &cov1w1, matCovariance, -1, parent );
    
    parent[0] = 0;
    pBNet->AllocFactor(3);
    pBNet->GetFactor(3)->AllocMatrix( &mean3w0, matMean, -1, parent );
    pBNet->GetFactor(3)->AllocMatrix( &cov3w0, matCovariance, -1, parent );
    pBNet->GetFactor(3)->AllocMatrix( &weight3w0, matWeights, 0, parent );

    parent[0] = 1;
   
    pBNet->GetFactor(3)->AllocMatrix( &mean3w1, matMean,-1, parent );
    pBNet->GetFactor(3)->AllocMatrix( &cov3w1, matCovariance,-1, parent );
    pBNet->GetFactor(3)->AllocMatrix( &weight3w1, matWeights, 0, parent );
    
    
    //Create DBN usin BNet	
    CDBN *pArHMM = CDBN::Create( pBNet );
   
    return pArHMM;
}