/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      learn_param.cpp                                             //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnl_dll.hpp"
#include "pnlLearningEngine.hpp"
#include <ctype.h>
#include <math.h>
#include "time.h"
#include <iostream>
#include <stdio.h>
#include <fstream>

int main()
{
    PNL_USING
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


	*/
	int i;
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
	int *nbrs[] = { nbs0, nbs1, nbs2, nbs3, nbs4 };
	int numNeighb[] = {1, 1, 4, 1, 1};

	
	ENeighborType ori0[] = { ntChild };
	ENeighborType ori1[] = { ntChild };
	ENeighborType ori2[] = { ntParent, ntParent, ntChild, ntChild };
	ENeighborType ori3[] = { ntParent };
	ENeighborType ori4[] = { ntParent };
	ENeighborType *orient[] = { ori0, ori1, ori2, ori3, ori4 }; 
	
	
	CGraph *pGraph;
	pGraph = CGraph::Create(nnodes, numNeighb, nbrs, orient);
	
	CBNet *pBNet;
	
	pBNet = CBNet::Create( nnodes, numnt, nodeTypes, &nodeAssociation.front(), pGraph );
	//Allocation space for all factors of the model
	pBNet->AllocFactors();
	
	for( i = 0; i < nnodes; i++ )
	{
	    //Allocation space for all matrices of CPD
	    pBNet->AllocFactor(i);
	}
	
	//now we need to create data for CPDs - we'll create matrices
	CFactor *pCPD;
	floatVector smData = floatVector(2,0.0f);
	floatVector bigData = floatVector(4,1.0f);
	intVector ranges = intVector(2, 1);
	ranges[0] = 2;
	smData[0] = 1.0f;
	CNumericDenseMatrix<float> *mean0 = CNumericDenseMatrix<float>::
        Create( 2, &ranges.front(), &smData.front());
	bigData[0] = 4.0f;
	bigData[3] = 4.0f;
	ranges[1] = 2;
	CNumericDenseMatrix<float> *cov0 = CNumericDenseMatrix<float>::
        Create( 2, &ranges.front(), &bigData.front());
	pCPD = pBNet->GetFactor(0);
	pCPD->AttachMatrix(mean0, matMean);
	pCPD->AttachMatrix(cov0, matCovariance);
	ranges[0] = 1;
	ranges[1] = 1;
	float val = 1.0f;
	CNumericDenseMatrix<float> *mean1 = CNumericDenseMatrix<float>::
        Create( 2, &ranges.front(), &val );
	CNumericDenseMatrix<float> *cov1 = CNumericDenseMatrix<float>::
        Create( 2, &ranges.front(), &val );
	pCPD = pBNet->GetFactor(1);
	pCPD->AttachMatrix(mean1, matMean);
	pCPD->AttachMatrix(cov1, matCovariance);
	smData[0] = 0.0f;
	smData[1] = 0.0f;
	ranges[0] = 2;
	CNumericDenseMatrix<float> *mean2 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &smData.front());
	smData[0] = 2.0f;
	smData[1] = 1.0f;
	CNumericDenseMatrix<float> *w21 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &smData.front());
	bigData[0] = 2.0f;
	bigData[1] = 1.0f;
	bigData[2] = 1.0f;
	bigData[3] = 1.0f;
	ranges[1] = 2;
	CNumericDenseMatrix<float> *cov2 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &bigData.front());
	bigData[0] = 1.0f;
	bigData[1] = 2.0f;
	bigData[2] = 1.0f;
	bigData[3] = 0.0f;
	CNumericDenseMatrix<float> *w20 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &bigData.front());
	pCPD = pBNet->GetFactor(2);
	pCPD->AttachMatrix( mean2, matMean );
	pCPD->AttachMatrix( cov2, matCovariance );
	pCPD->AttachMatrix( w20, matWeights,0 );
	pCPD->AttachMatrix( w21, matWeights,1 );
	
	val = 0.0f;
	ranges[0] = 1;
	ranges[1] = 1;
	CNumericDenseMatrix<float> *mean3 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &val);
	val = 1.0f;
	CNumericDenseMatrix<float> *cov3 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &val);
	ranges[1] = 2;
	smData[0] = 1.0f;
	smData[1] = 1.0f;
	CNumericDenseMatrix<float> *w30 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &smData.front());
	pCPD = pBNet->GetFactor(3);
	pCPD->AttachMatrix( mean3, matMean );
	pCPD->AttachMatrix( cov3, matCovariance );
	pCPD->AttachMatrix( w30, matWeights,0 );

	ranges[0] = 2; 
	ranges[1] = 1;
	smData[0] = 0.0f;
	smData[1] = 0.0f;
	CNumericDenseMatrix<float> *mean4 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &smData.front());
	ranges[1] = 2;
	bigData[0] = 1.0f;
	bigData[1] = 0.0f;
	bigData[2] = 0.0f;
	bigData[3] = 1.0f;
	CNumericDenseMatrix<float> *cov4 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &bigData.front());
	bigData[2] = 1.0f;
	CNumericDenseMatrix<float> *w40 = CNumericDenseMatrix<float>::
        Create(2, &ranges.front(), &bigData.front());
	pCPD = pBNet->GetFactor(4);
	pCPD->AttachMatrix( mean4, matMean );
	pCPD->AttachMatrix( cov4, matCovariance );
	pCPD->AttachMatrix( w40, matWeights,0 );

	//Generate random evidences for the modes
	int nEv = 1000;
	pEvidencesVector evid;
	pBNet->GenerateSamples( &evid, nEv );
	/////////////////////////////////////////////////////////////////////
		
	//Create copy of initial model with random matrices 
	CGraph *pGraphCopy = CGraph::Copy(pGraph); 
	CBNet *pLearnBNet = CBNet::CreateWithRandomMatrices(pGraphCopy, pBNet->GetModelDomain() );
	
	// Creating learning process	
	CEMLearningEngine *pLearn = CEMLearningEngine::Create(pLearnBNet);

	pLearn->SetData(nEv, &evid.front());
	pLearn->Learn();
	CNumericDenseMatrix<float> *pMatrix;
	int length = 0;
	const float *output;
	
	///////////////////////////////////////////////////////////////////////
	std::cout<<" results of learning (number of evidences = "<<nEv<<std::endl;
	for (i = 0; i < nnodes; i++ )
	{
	    int j;
	    std::cout<<"\n matrix mean for node "<<i;
	    std::cout<<"\n initial BNet \n";
	    pMatrix = static_cast<CNumericDenseMatrix<float>*>
		(pBNet->GetFactor(i)->GetMatrix(matMean));
	    pMatrix->GetRawData(&length, &output);
	    for ( j = 0; j < length; j++ )
	    {
		std::cout<<" "<<output[j];
	    }
	    std::cout<<"\n BNet with random matrices after learning \n ";
	    pMatrix = static_cast<CNumericDenseMatrix<float>*>
		(pLearnBNet->GetFactor(i)->GetMatrix(matMean));
	    pMatrix->GetRawData(&length, &output);
	    for ( j = 0; j < length; j++)
	    {
		std::cout<<" "<<output[j];
	    }
	    
    	    std::cout<<"\n \n matrix covariance for node "<<i<<'\n';
	    std::cout<<"\n initial BNet \n";

	    pMatrix = static_cast<CNumericDenseMatrix<float>*>
		(pBNet->GetFactor(i)->GetMatrix(matCovariance));
	    pMatrix->GetRawData(&length, &output);
	    for (j = 0; j < length; j++ )
	    {
		std::cout<<" "<<output[j];
	    }
    	    std::cout<<"\n BNet with random matrices after learning \n ";
	    pMatrix = static_cast<CNumericDenseMatrix<float>*>
		(pLearnBNet->GetFactor(i)->GetMatrix(matCovariance));
	    pMatrix->GetRawData(&length, &output);
	    for ( j = 0; j < length; j++ )
	    {
		std::cout<<" "<<output[j];
	    }

	    std::cout<<"\n ___________________________\n";
	    
	}
	
	
	for( i = 0; i < nEv; i++)
	{
	    delete evid[i];
	}
	delete pLearn;
	delete pLearnBNet;
	delete pBNet;
	
	

return 1;
}
