/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      tCreateIncineratorBNet.cpp                                  //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "tCreateIncineratorBNet.hpp"

PNL_USING

CBNet* tCreateIncineratorBNet()
{
    // for details go to Cowell p. 145. model name "waste"
	int i;

    const int nnodes = 9;
	
    int numOfNeigh[] = { 1, 3, 3, 2, 1, 5, 2, 2, 1 };
	
    int neigh0[] = { 2 };
    int neigh1[] = { 2, 5, 6 };
    int neigh2[] = { 0, 1, 5  };
	int neigh3[] = { 4, 5 };
	int neigh4[] = { 3 };
	int neigh5[] = { 1, 2, 3, 7, 8 };
	int neigh6[] = { 1, 7 };
	int neigh7[] = { 5, 6 };
    int neigh8[] = { 5 };
	
    ENeighborType orient0[] = { ntChild };
	ENeighborType orient1[] = { ntChild, ntChild, ntChild };
    ENeighborType orient2[] = { ntParent, ntParent, ntChild };
	ENeighborType orient3[] = { ntChild, ntChild };
	ENeighborType orient4[] = { ntParent };
	ENeighborType orient5[] = { ntParent, ntParent, ntParent, ntChild, ntChild };
	ENeighborType orient6[] = { ntParent, ntChild };
	ENeighborType orient7[] = { ntParent, ntParent };
    ENeighborType orient8[] = { ntParent };

	int *neigh[] = { neigh0, neigh1, neigh2, neigh3, neigh4, neigh5, neigh6,
		neigh7, neigh8 };
    ENeighborType *orient[] = { orient0, orient1, orient2, orient3, orient4,
        orient5, orient6, orient7, orient8 };

    CGraph* pWasteGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );

    int numNodeTypes = 2;
    CNodeType* ndTypes = new CNodeType[numNodeTypes];
    ndTypes[0].SetType( 1, 2 );
    ndTypes[1].SetType( 0, 1 );

    intVector nodeAssociation;
    nodeAssociation.assign( nnodes, 1 );
    nodeAssociation[0] = 0;
    nodeAssociation[1] = 0;
    nodeAssociation[3] = 0;

    float table0[] = { 0.95f, 0.05f };// intact - defective
    float table1[] = { 0.28571428571428571428571428571429f, 0.71428571428571428571428571428571f };
    // industrial - household
    float table3[] = { 0.85f, 0.15f };// stable - unstable

    float mean2w01 = -3.2f; float cov2w01 = 0.00002f; 
    float mean2w11 = -0.5f; float cov2w11 = 0.0001f;
    float mean2w00 = -3.9f; float cov2w00 = 0.00002f;
    float mean2w10 = -0.4f; float cov2w10 = 0.0001f;

    float mean4w0 = -2.0f; float cov4w0 = 0.1f; 
    float mean4w1 = -1.0f; float cov4w1 = 0.3f;
    
    float mean5w00 = 6.5f; float cov5w00 = 0.03f; float weight5 = 1.0f;
    float mean5w01 = 7.5f; float cov5w01 = 0.1f;
    float mean5w10 = 6.0f; float cov5w10 = 0.04f;
    float mean5w11 = 7.0f; float cov5w11 = 0.1f;

    float mean6w0 = 0.5f; float cov6w0 = 0.01f; 
    float mean6w1 = -0.5f; float cov6w1 = 0.005f;

    float mean7 = 0.0f; float cov7 = 0.002f; 
                                float weight7p5 = 1.0f; float weight7p6 = 1.0f;

    float mean8 = 3.0f; float cov8 = 0.25f; float weight8 = -0.5f;

    CBNet* pWasteBNet = CBNet::Create( nnodes, numNodeTypes, ndTypes,
        &nodeAssociation.front(), pWasteGraph );

    CModelDomain* pMD = pWasteBNet->GetModelDomain();
    
	pWasteBNet->AllocFactors();
    
    CCPD** params = new CCPD *[nnodes];
	
	pConstNodeTypeVector ntDomain;
    ntDomain.reserve(nnodes);
    
	intVector domain;
    domain.reserve(nnodes);
    
	for( i = 0; i < nnodes; ++i )
    {
        pWasteGraph->GetParents( i, &domain );

        domain.push_back( i );

        if( i == 0 || i == 1 || i == 3 )
        {
            params[i] = CTabularCPD::Create( &domain.front(), domain.size(), pMD );
        }
        else
        {
            params[i] = CGaussianCPD::Create( &domain.front(), domain.size(), pMD );
        }

        domain.clear();
        ntDomain.clear();
    }
    
    //allocate tabular matrix for tabular nodes 0, 1
    params[0]->AllocMatrix( table0, matTable );
    params[1]->AllocMatrix( table1, matTable );
    //need to allocate conditional Gaussian distribution for node 2 & 5 
    intVector discrParentVals;
    discrParentVals.assign(2, 0);
    params[2]->AllocMatrix( &mean2w00, matMean, -1, &discrParentVals.front() );
    params[2]->AllocMatrix( &cov2w00, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &mean5w00, matMean, -1, &discrParentVals.front() );
    params[5]->AllocMatrix( &cov5w00, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &weight5, matWeights, 0, &discrParentVals.front() );
    discrParentVals[1] = 1;
    params[2]->AllocMatrix( &mean2w01, matMean, -1, &discrParentVals.front() );
    params[2]->AllocMatrix( &cov2w01, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &mean5w01, matMean, -1, &discrParentVals.front() );
    params[5]->AllocMatrix( &cov5w01, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &weight5, matWeights, 0, &discrParentVals.front() );
    discrParentVals[0] = 1;
    params[2]->AllocMatrix( &mean2w11, matMean, -1, &discrParentVals.front() );
    params[2]->AllocMatrix( &cov2w11, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &mean5w11, matMean, -1, &discrParentVals.front() );
    params[5]->AllocMatrix( &cov5w11, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &weight5, matWeights, 0, &discrParentVals.front() );
    discrParentVals[1] = 0;
    params[2]->AllocMatrix( &mean2w10, matMean, -1, &discrParentVals.front() );
    params[2]->AllocMatrix( &cov2w10, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &mean5w10, matMean, -1, &discrParentVals.front() );
    params[5]->AllocMatrix( &cov5w10, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &weight5, matWeights, 0, &discrParentVals.front() );
    //tabular matrix for node 3
    params[3]->AllocMatrix( table3, matTable );
    //conditional Gaussian distribution for node 4 & 6
    int parentVal = 0;
    params[4]->AllocMatrix( &mean4w0, matMean, -1, &parentVal );
    params[4]->AllocMatrix( &cov4w0, matCovariance, -1, &parentVal );
    params[6]->AllocMatrix( &mean6w0, matMean, -1, &parentVal );
    params[6]->AllocMatrix( &cov6w0, matCovariance, -1, &parentVal );
    parentVal = 1;
    params[4]->AllocMatrix( &mean4w1, matMean, -1, &parentVal );
    params[4]->AllocMatrix( &cov4w1, matCovariance, -1, &parentVal );
    params[6]->AllocMatrix( &mean6w1, matMean, -1, &parentVal );
    params[6]->AllocMatrix( &cov6w1, matCovariance, -1, &parentVal );
    //Gaussian distribution for node 7
    params[7]->AllocMatrix( &mean7, matMean );
    params[7]->AllocMatrix( &cov7, matCovariance);
    params[7]->AllocMatrix( &weight7p5, matWeights, 0 );
    params[7]->AllocMatrix( &weight7p6, matWeights, 1 );
    //Gaussian distribution for node 8
    params[8]->AllocMatrix( &mean8, matMean );
    params[8]->AllocMatrix( &cov8, matCovariance );
    params[8]->AllocMatrix( &weight8, matWeights, 0 );

    for( i = 0; i < nnodes; ++i )
    {
        pWasteBNet->AttachFactor(params[i]);
    }

    delete [] ndTypes;
    delete []params;
        
    return pWasteBNet;

};