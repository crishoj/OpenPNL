/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ABKInfUsingClusters.cpp                                     //
//                                                                         //
//  Purpose:   Test on BK inference engine                                 //
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
#include "tCreateKjaerulffDBN.hpp"
#include "tUtil.h"


static char func_name[] = "testBKInfUsingClusters";

static char* test_desc = "Provide BK Inf using clusters";

static char* test_class = "Algorithm";

int testBKInfUsingClusters()
{
    
    
    int ret = TRS_OK;

    int seed = pnlTestRandSeed();
    pnlSeed( seed );
    std::cout<<"seed"<<seed<<std::endl;
    
    
    int nTimeSlices = -1;
    while(nTimeSlices <= 5)
    {
        trsiRead (&nTimeSlices, "10", "Number of slices");
    }
    
    float eps = -1.0f;
    while( eps <= 0 )
    {
        trssRead( &eps, "1.0e-1f", "accuracy in test");
    }
    
    
    CDBN *pDBN = CDBN::Create( pnlExCreateBatNetwork() );
    
    intVecVector clusters;
    intVector interfNds;
    pDBN->GetInterfaceNodes( &interfNds );
    int numIntNds = interfNds.size();
    int numOfClusters = pnlRand( 1, numIntNds );
    clusters.resize(numOfClusters);
    int i;
    for( i = 0; i < numIntNds; i++ )
    {
	( clusters[pnlRand( 0, numOfClusters-1 )] ).push_back( interfNds[i] );
    }

    intVecVector validClusters;
    validClusters.reserve(numOfClusters);
    for( i = 0; i < clusters.size(); i++ )
    {
	if(! clusters[i].empty())
	{
	    validClusters.push_back(clusters[i]);
	}
    }
        
    CBKInfEngine *pBKInf;
    pBKInf = CBKInfEngine::Create( pDBN, validClusters );

    C1_5SliceJtreeInfEngine *pJTreeInf;
    pJTreeInf = C1_5SliceJtreeInfEngine::Create( pDBN );

    intVector nSlices( 1, nTimeSlices );
    pEvidencesVecVector pEvid;
    
    pDBN->GenerateSamples( &pEvid, nSlices);
    int nnodesPerSlice = pDBN->GetNumberOfNodes();
    intVector nodes(nnodesPerSlice, 0);
    for( i = 0; i < nnodesPerSlice; i++ )
    {
	nodes[i] = i;
    }
    intVector ndsToToggle;
    for( i = 0; i < nTimeSlices; i++ )
    {
	std::random_shuffle( nodes.begin(), nodes.end() );
	ndsToToggle.resize( pnlRand(1, nnodesPerSlice) );
	int j;
	for( j = 0; j < ndsToToggle.size(); j++ )
	{
	    ndsToToggle[j] = nodes[j];
	}
	
	(pEvid[0])[i]->ToggleNodeState( ndsToToggle );
    }
    pBKInf->DefineProcedure( ptSmoothing, nTimeSlices );
    pBKInf->EnterEvidence( &(pEvid[0]).front(), nTimeSlices );
    pBKInf->Smoothing();

    pJTreeInf->DefineProcedure( ptSmoothing, nTimeSlices );
    pJTreeInf->EnterEvidence( &pEvid[0].front(), nTimeSlices );
    pJTreeInf->Smoothing();
    
    int querySlice = pnlRand( 0, nTimeSlices - 1 );
    int queryNode = pnlRand( 0, nnodesPerSlice - 1);
    queryNode += (querySlice ? nnodesPerSlice : 0);

    intVector query;
    pDBN->GetGraph()->GetParents( queryNode, &query );
    query.push_back( queryNode );
    std::random_shuffle( query.begin(), query.end() );
    query.resize( pnlRand(1, query.size()) );

    pBKInf->MarginalNodes(&query.front(), query.size(), querySlice);
    pJTreeInf->MarginalNodes(&query.front(), query.size(), querySlice);

    const CPotential *potBK = pBKInf->GetQueryJPD();
    const CPotential *potJTree = pJTreeInf->GetQueryJPD();
    if( !potBK->IsFactorsDistribFunEqual( potJTree , eps ) )
    {
	std::cout<<"BK query JPD \n";
	potBK->Dump();
	std::cout<<"JTree query JPD \n";
	potJTree->Dump();

	ret = TRS_FAIL;
    }
    for( i = 0; i < nTimeSlices; i++ )
    {
	delete (pEvid[0])[i];
    }
    
    delete pBKInf;
    delete pJTreeInf;
    delete pDBN;
 
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on BK Inference using clusters");
    
    
}

void initABKInfUsingClusters()
{
    trsReg(func_name, test_desc, test_class, testBKInfUsingClusters);
}