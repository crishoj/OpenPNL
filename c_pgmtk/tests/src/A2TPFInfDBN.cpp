/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      A2TPFInfDBN.cpp                                             //
//                                                                         //
//  Purpose:   Test on 2T Slice Particle filtering inference for DBN       //
//             with conditional gaussian distribution                      //
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
#include <assert.h>
#include "tCreateArHMMwithGaussObs.hpp"

// this file must be last included file
#include "tUtil.h"
PNL_USING

static char func_name[] = "test2TPFInfDBN";

static char* test_desc = "Provide 2T Slice Inf";

static char* test_class = "Algorithm";

CDBN* tCreateHMM();

void PFCreateEvidencesHMM(CDBN *pDBN, int nTimeSlices,
			  pEvidencesVector *EvidencesForDBN );
CEvidence * PFCreateEvidenceForUnrolledHMM(CBNet *pUnrolledDBN, int nTimeSlices,
					   pEvidencesVector &evidencesForDBN );
void PFDefineQueryHMM(CDBN *pDBN, int nTimeSlice,
		      intVector * queryForDBNPrior,
		      intVector * queryForDBN,
		      intVecVector *queryForUnrollBnet);
int PFCompareHMM(const CPotential *pot1, const CPotential *pot2, float eps);

int PFCompareFilteringHMM(CDBN* pDBN, int nTimeSlice, float eps);


int test2TPFInfDBN()
{
    int ret = TRS_OK;
    int seed = pnlTestRandSeed();

    std::cout<<"seed"<<seed<<std::endl;
    srand( seed );
	
    //CDBN *pDBN = tCreateArHMMwithGaussObs();
	CDBN *pDBN = tCreateHMM();

    int nTimeSlice = -1;
    while(nTimeSlice <= 0)
    {
	trsiRead (&nTimeSlice, "4", "Number of slices");
    }

    float eps = -1.0f;
    while( eps <= 0 )
    {
	trssRead( &eps, "1.5e-1f", "accuracy in test");
    }


    int result = 1;
   
    result = PFCompareFilteringHMM( pDBN, nTimeSlice,  eps );

    if( !result )
    {
	ret = TRS_FAIL;
    }


    //////////////////////////////////////////////////////////////////////////
    delete pDBN;
    return trsResult( ret, ret == TRS_OK ? "No errors"
	: "Bad test on 2T PaticleFiltering Inference of DBN");
}

void initAtest2TPFInfDBN()
{
    trsReg(func_name, test_desc, test_class, test2TPFInfDBN);
}

CDBN* tCreateHMM()
{
    
/*
Make an HMM with autoregressive gaussian observations
X0 ->X1
|     | 
v     v
Y0   Y1 
    */
    
    //Create static model
    const int nnodes = 4;//Number of nodes    
	int numNt = 1;//number of Node types (all nodes are discrete)
    CNodeType *nodeTypes = new CNodeType [numNt];
    nodeTypes[0] = CNodeType( 1,2 );
    int nodeAssociation[] = { 0, 0, 0, 0 };
    
    //Create a dag
    int numOfNeigh[] = { 2, 1, 2, 1 };
    int neigh0[] = { 1, 2 };
    int neigh1[] = { 0 };
    int neigh2[] = { 0, 3 };
    int neigh3[] = { 2 };
    
    ENeighborType orient0[] = { ntChild, ntChild };
    ENeighborType orient1[] = { ntParent };
    ENeighborType orient2[] = { ntParent, ntChild };
    ENeighborType orient3[] = { ntParent };

    
    int *neigh[] = { neigh0, neigh1, neigh2, neigh3 };
    ENeighborType *orient[] = { orient0, orient1, orient2, orient3 };
    
    CGraph* pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );
    
    //Create static BNet
    CBNet *pBNet = 
        CBNet::Create( nnodes, numNt, nodeTypes, nodeAssociation, pGraph );
    pBNet->AllocFactors();
    
    //Let arbitrary distribution is
    float tableNode0[] = { 0.95f, 0.05f };
    float tableNode1[] = { 0.80f, 0.20f, 0.1f,0.9f };
    float tableNode2[] = { 0.1f, 0.9f, 0.8f, 0.2f };
    float tableNode3[] = { 0.80f, 0.20f, 0.1f,0.9f };
    
    pBNet->AllocFactor( 0 ); 
    pBNet->GetFactor(0)->AllocMatrix( tableNode0, matTable );
    
	pBNet->AllocFactor( 1 );
    pBNet->GetFactor(1)->AllocMatrix( tableNode1, matTable );
    
	pBNet->AllocFactor( 2 );
    pBNet->GetFactor(2)->AllocMatrix( tableNode2, matTable );
    
	pBNet->AllocFactor( 3 );
    pBNet->GetFactor(3)->AllocMatrix( tableNode3, matTable );
   
    //Create DBN usin BNet	
    CDBN *pHMM = CDBN::Create( pBNet );
   
    return pHMM;
}

CEvidence* PFCreateEvidenceForUnrolledHMM(CBNet *pUnrolledDBN, int nTimeSlices,
					  pEvidencesVector &evidencesForDBN )
{

    intVector obsNodesInUnrolledDBN;
    obsNodesInUnrolledDBN.resize( nTimeSlices );
    valueVector valuesForUnrolledDBN;
    valuesForUnrolledDBN.resize( nTimeSlices );

    int nnodesInDBN = 2;
    int slice;
    for( slice = 0; slice < nTimeSlices; slice++)
    {
	obsNodesInUnrolledDBN[slice] = 1 + nnodesInDBN*slice;
	valuesForUnrolledDBN[slice] = *(evidencesForDBN[slice]->GetValueBySerialNumber(0));
    }
    return CEvidence::Create( pUnrolledDBN->GetModelDomain(),
	obsNodesInUnrolledDBN, valuesForUnrolledDBN );
}

void PFCreateEvidencesHMM( CDBN *pDBN, int nTimeSlices,
			  pEvidencesVector *myEvidencesForDBN )
{

    if( myEvidencesForDBN->size() )
    {
        assert( 0 );
    }
    /////////////////////////////////////////////////////////////////////////////
    // Create values for evidence in every slice from t=0 to t=nTimeSlice
    /////////////////////////////////////////////////////////////////////////////

    intVector obsNodes(1, 1);
    int slice;
    valueVecVector valuesPerSlice;
	valueVector tmpVal(1,0);
	
	tmpVal[0].SetInt( rand()%2 );

    for( slice = 0; slice < nTimeSlices; slice++ )
    {
	//tmpVal[0].SetInt( rand()%2 );
	tmpVal[0].SetInt( (tmpVal[0].GetInt()+1)%2 );
	
	valuesPerSlice.push_back( tmpVal );
    }


    for( slice = 0; slice < nTimeSlices; slice++ )
    {
	myEvidencesForDBN->push_back( CEvidence::Create( pDBN->GetModelDomain(),
	    obsNodes, valuesPerSlice[slice] ) );
    }
}

void PFDefineQueryHMM( CDBN *pDBN, int nTimeSlice,
		      intVector * queryForDBNPrior,
		      intVector * queryForDBN,
		      intVecVector *queryForUnrollBnet )
{
    ///////////////////////////////////////////////////////////////////////////
    //Define query for unrolled DBN and DBN (it is parents of some random node)
    /////////////////////////////////////////////////////////////////////////////
    int numOfNodesInDBN=pDBN->GetNumberOfNodes();
    int numOfInterfaceNodes;
    const int * interfaceNodes;
    pDBN->GetInterfaceNodes( &numOfInterfaceNodes, &interfaceNodes );
    /////////////////////////////////////////////////////////////////////////////
    int node = ( int )( rand()%( numOfNodesInDBN - 1 ) ) ;
    /////////////////////////////////////////////////////////////////////////////
    (*queryForUnrollBnet).resize( nTimeSlice );

    /////////////////////////////////////////////////////////////////////////////
    int numOfNeighbors;
    const int * neighbors;
    const ENeighborType * types;
    /////////////////////////////////////////////////////////////////////////////

    CGraph *pPriorGraph = pDBN->CreatePriorSliceGraph();
    pPriorGraph->GetNeighbors( node, &numOfNeighbors, &neighbors, &types );
    int i;
    for( i = 0; i < numOfNeighbors; i++ )
    {
	if( types[i] == ntParent )
	{
	    (*queryForDBNPrior).push_back( neighbors[i] );
	    (*queryForUnrollBnet)[0].push_back( neighbors[i] );
	}

    }
    (*queryForDBNPrior).push_back( node );
    (*queryForUnrollBnet)[0].push_back( node );
    delete pPriorGraph;

    //////////////////////////////////////////////////////////////////////////////
    node +=  numOfNodesInDBN;
    pDBN->GetGraph()->GetNeighbors( node, &numOfNeighbors, &neighbors, &types );


    int slice;

    for( i = 0; i < numOfNeighbors; i++ )
    {
	if( types[i] == ntParent )
	{
	    (*queryForDBN).push_back( neighbors[i] );
	    for( slice = 0; slice < nTimeSlice - 1; slice++ )
	    {
		(*queryForUnrollBnet)[slice+1].push_back( neighbors[i] +
		    slice*numOfNodesInDBN );
	    }
	}

    }
    (*queryForDBN).push_back( node );


    for( slice = 0; slice < nTimeSlice - 1; slice++ )
    {
	(*queryForUnrollBnet)[slice+1].push_back( node + slice*numOfNodesInDBN );
    }

}

int PFCompareHMM( const CPotential *pot1, const CPotential *pot2, float eps )
{
    const float *data1;
    int data_length;
    const float *data2;
    int data_length1;
    int stop;
    CNumericDenseMatrix<float>* mat = static_cast<CNumericDenseMatrix<float>*>(
        pot1->GetMatrix(matTable));
    mat->GetRawData( &data_length, &data1 );
    mat = static_cast<CNumericDenseMatrix<float>*>(pot2->GetMatrix(matTable));
    mat->GetRawData( &data_length1, &data2 );
    int result = pot1-> IsFactorsDistribFunEqual( pot2, eps );
    if(  !result )
    {
	stop = 1;
    }
    return result;

}

/////////////////////////////////////////////////////////////////////////////
int PFCompareFilteringHMM(CDBN* pDBN, int nTimeSlice, float eps)
{
	std::cout<<std::endl<<"2T PF for Filtering HMM "<<std::endl;

	//Create 2TPFInference engine
    C2TPFInfEngine *pDynamicPF;
    pDynamicPF = C2TPFInfEngine::Create(pDBN);
    pDynamicPF->DefineProcedure(ptFiltering);
    
	//Create Evidence and query
    CEvidence *myEvidenceForUnrolledDBN;
    pEvidencesVector myEvidencesForDBN;
    PFCreateEvidencesHMM(pDBN, nTimeSlice, &myEvidencesForDBN);
    intVector queryForDBN, queryForDBNPrior;
    intVecVector queryForUnrollBnet;
    PFDefineQueryHMM(pDBN, nTimeSlice,
	&queryForDBNPrior,	&queryForDBN, &queryForUnrollBnet);

	//Test 
	int result = 1;
    int slice;

    for( slice = 0; slice < nTimeSlice; slice++ )
    {

	CBNet *	pUnrolledOnISlice =
	    static_cast<CBNet *>(pDBN->UnrollDynamicModel(slice+1));
	myEvidenceForUnrolledDBN =
	    PFCreateEvidenceForUnrolledHMM(pUnrolledOnISlice , slice+1, myEvidencesForDBN);
	CJtreeInfEngine *pUnrolJTree =
	    CJtreeInfEngine::Create(pUnrolledOnISlice);

	pUnrolJTree->EnterEvidence(myEvidenceForUnrolledDBN);
	pDynamicPF->EnterEvidence(&(myEvidencesForDBN[slice]), 1);

	pUnrolJTree->MarginalNodes(&(queryForUnrollBnet[slice]).front(),
	    (queryForUnrollBnet[slice]).size());

	if(slice)// slice 1,2...
	{
	    pDynamicPF->MarginalNodes(&queryForDBN.front(),
		queryForDBN.size());
	}
	else     //slice 0
	{
	    pDynamicPF->MarginalNodes(&queryForDBNPrior.front(),
		queryForDBNPrior.size());
	}

	const CPotential* qJPD = pUnrolJTree->GetQueryJPD();
	//const CEvidence*  mJPD = pUnrolJTree->GetMPE();

	const CPotential* qPF = pDynamicPF->GetQueryJPD();
	const CEvidence*  mPF = pDynamicPF->GetMPE();
	
	std::cout<<"result of 2TPF"<<std::endl<<std::endl;
	qPF->Dump();
	mPF->Dump();
	std::cout<<"result of UnrollJTree"<<std::endl<<std::endl;
	qJPD->Dump();
	//mJPD->Dump();
	
	std::cout<<std::endl;
	
	int res = PFCompareHMM( qPF, qJPD, eps);
	result  = result && res;
	
	delete myEvidenceForUnrolledDBN;
	delete pUnrolJTree;
	delete pUnrolledOnISlice;
    }

	// Free myEvidencesForDBN
    for( slice = 0; slice < (myEvidencesForDBN).size(); slice++ )
    {
	delete myEvidencesForDBN[slice];
    }

	////////////////////////////////////////////////////////////////////////
	//Test by changing parameters of 2TPFInference Engine 
	////////////////////////////////////////////////////////////////////////
	std::cout<<"Test by changing parameters of 2TPFInference Engine "<<std::endl<<std::endl;
	
	//set 2TPFInf as 1000 particles with minium effective particle number is 50
	pDynamicPF->SetParemeter(1000, 50); 

	std::cout<<"Re-init slice0"<<std::endl<<std::endl;
	intVector   InitNodes(2,0);
	InitNodes[0] = 0;  InitNodes[1] = 1;
	
	valueVector InitValueForDBN(2,0);
	InitValueForDBN[0].SetInt(0); InitValueForDBN[1].SetInt(1);
	
	CEvidence * pInitEvd = CEvidence::Create( pDBN->GetModelDomain(), InitNodes, InitValueForDBN);

	//init slice0 by pInitEvd
	pDynamicPF->InitSlice0Particles(pInitEvd);
	pInitEvd->Dump();
	delete pInitEvd;
	
	std::cout<<"LW sampling slice t+1 from slice t"<<std::endl<<std::endl;
	// LW Sampling slice t+1 from slice t
	pDynamicPF->LWSampling();

	//Get the pointer of particle values
	pEvidencesVector* pSamples = pDynamicPF->GetCurSamples();

	//Get the pointer of corresponding particle weights
	floatVector* pSampleWeights = pDynamicPF->GetParticleWeights();
	
	//User can calculate and set weights by himself ...
	
	std::cout<<"Input pSampleWeights"<<std::endl<<std::endl;
	//Input pSampleWeights as new particle weights
	pDynamicPF->EnterEvidenceProbability(pSampleWeights);
		
	std::cout<<"Estimate parameters by GetMPE()"<<std::endl<<std::endl;
	pDynamicPF->MarginalNodes(&queryForDBN.front(), queryForDBN.size());
	const CEvidence*  mPF = pDynamicPF->GetMPE();
	mPF->Dump();

	//Estimate possible value ( is similar to mPF)
	valueVector queryValueForDBN(queryForDBN.size(),0);
	CEvidence * pEst = CEvidence::Create( pDBN->GetModelDomain(), queryForDBN, queryValueForDBN);
	
	std::cout<<"Estimate parameters directly by Estimate()"<<std::endl<<std::endl;
	pDynamicPF->Estimate(pEst);
	pEst->Dump();
	
	std::cout<<"Free evidence"<<std::endl<<std::endl;
 	delete pEst;

	//Free 2TPFInf
	std::cout<<"Free 2tPFInfEngine"<<std::endl<<std::endl;
	delete pDynamicPF;

    return result;
}























