/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AJTreeInfDBN.cpp                                            //
//                                                                         //
//  Purpose:   Test on 1.5 Slice Junction tree inference for DBN           //
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


static char func_name[] = "testBKInfDBN";

static char* test_desc = "Provide BK Inf";

static char* test_class = "Algorithm";

int testBKInfDBN()
{
    int ret = TRS_OK;
    int seed = pnlTestRandSeed();
    
    std::cout<<"seed"<<seed<<std::endl;
    
   
    
    int nTimeSlices = -1;
    while(nTimeSlices <= 4)
    {
        trsiRead (&nTimeSlices, "10", "Number of slices");
    }
    
    float eps = -1.0f;
    while( eps <= 0 )
    {
        trssRead( &eps, "1.5e-2f", "accuracy in test");
    }
    
    
    
    CDBN *pDBN = tCreateKjaerulffDBN();
    
    intVector nSlices( 1, nTimeSlices );
    pEvidencesVecVector pEvid;
    
    pDBN->GenerateSamples( &pEvid, nSlices);
    
    intVector ndsToToggle(5);
    ndsToToggle[0] = 0;
    ndsToToggle[1] = 2;
    ndsToToggle[2] = 3;
    ndsToToggle[3] = 4;
    ndsToToggle[4] = 7;

    int slice=0;
    pEvid[0][slice]->ToggleNodeState(ndsToToggle.size(), &ndsToToggle.front());
    slice++;
    
    for( ; slice < nTimeSlices; slice++ )
    {
        pEvid[0][slice]->ToggleNodeState(ndsToToggle);
    }
    //////////////////////////////////////////////
    
    CBKInfEngine *pBKInf = CBKInfEngine::Create(pDBN, true);
    pBKInf->DefineProcedure(ptSmoothing, nTimeSlices);
    
    C1_5SliceJtreeInfEngine *pJTreeInf = C1_5SliceJtreeInfEngine::Create(pDBN);
    pJTreeInf->DefineProcedure(ptSmoothing, nTimeSlices);
    
    slice = pnlRand(0, nTimeSlices -1 );
    intVector querySmoothing;
    int nnodesPerSlice = pDBN->GetNumberOfNodes();
    int end = slice ?  2*nnodesPerSlice-1 : nnodesPerSlice - 1;
    int start = slice  ? nnodesPerSlice : 0;
    int node = pnlRand(start, end);
    pDBN->GetFactor(node)->GetDomain(&querySmoothing);
    
    const CPotential* queryPotBK;
    const CPotential* queryPotJTree;
    
    
    pBKInf->EnterEvidence(&pEvid[0].front(), nTimeSlices);
    pBKInf->Smoothing();
    pBKInf->MarginalNodes(&querySmoothing.front(), querySmoothing.size(), slice);
    queryPotBK = pBKInf->GetQueryJPD();
    std::cout<<"BK inference"<<std::endl;
    queryPotBK->Dump();
    
    pJTreeInf->EnterEvidence(&pEvid[0].front(), nTimeSlices);
    pJTreeInf->Smoothing();
    pJTreeInf->MarginalNodes(&querySmoothing.front(), querySmoothing.size(), slice);
    queryPotJTree = pJTreeInf->GetQueryJPD();
    std::cout<<"1_5 slice inference"<<std::endl; 
    queryPotJTree->Dump();
    
    if( !queryPotJTree->IsFactorsDistribFunEqual(queryPotBK, eps) )
    {
	ret = TRS_FAIL;
	return trsResult( ret, ret == TRS_OK ? "No errors" : 
	"Bad test on Unroll DBN");
    }
    
    //////////////////////////////////////////////////////////////////////////
    
    int lag = 3;
    pBKInf->DefineProcedure(ptFixLagSmoothing, lag);
    
    //delete pJTreeInf;
    //pJTreeInf = C1_5SliceJtreeInfEngine::Create(pDBN);
    pJTreeInf->DefineProcedure(ptFixLagSmoothing, lag);
    for( slice = 0; slice < lag; slice++ )
    {
        pBKInf->EnterEvidence(&pEvid[0][slice], 1);
        pJTreeInf->EnterEvidence(&pEvid[0][slice], 1);
    }

    for( ; slice < nTimeSlices; slice++ )
    {
        pBKInf->EnterEvidence(&pEvid[0][slice], 1);
        pJTreeInf->EnterEvidence(&pEvid[0][slice], 1);
        
        pJTreeInf->FixLagSmoothing(slice);
        pBKInf->FixLagSmoothing(slice);
        
        int nnodesPerSlice = pDBN->GetNumberOfNodes();
	int end = slice - lag ?  2*nnodesPerSlice - 1 : nnodesPerSlice - 1;
	int start = slice - lag ? nnodesPerSlice : 0;
	int node = pnlRand(start, end);
	intVector query;
	pDBN->GetFactor(node)->GetDomain(&query);
        pBKInf->MarginalNodes( &query.front(), query.size() );
        queryPotBK = pBKInf->GetQueryJPD();
        std::cout<<"BK inference"<<std::endl;
        queryPotBK->Dump();
        
        pJTreeInf->MarginalNodes( &query.front(), query.size() );
        queryPotJTree = pJTreeInf->GetQueryJPD();
        std::cout<<"1_5 slice inference"<<std::endl;
        queryPotJTree->Dump();

        if( !queryPotJTree->IsFactorsDistribFunEqual(queryPotBK, eps) )
        {
            ret = TRS_FAIL;
            break;
        }
        
    }
    
     //////////////////////////////////////////////////////////////////////////
    
    
    pBKInf->DefineProcedure(ptFiltering);
    
    delete pJTreeInf;
    pJTreeInf = C1_5SliceJtreeInfEngine::Create(pDBN);
    pJTreeInf->DefineProcedure(ptFiltering);
   

    
    for( slice = 0; slice < nTimeSlices; slice++ )
    {
        pBKInf->EnterEvidence(&pEvid[0][slice], 1);
        pJTreeInf->EnterEvidence(&pEvid[0][slice], 1);
        
        pJTreeInf->Filtering(slice);
        pBKInf->Filtering(slice);
        
        int nnodesPerSlice = pDBN->GetNumberOfNodes();
	int end =   slice  ? 2*nnodesPerSlice - 1 : nnodesPerSlice - 1;
	int start = slice  ? nnodesPerSlice : 0;
	int node = pnlRand(start, end);
	intVector query;
	pDBN->GetFactor(node)->GetDomain(&query);
        pBKInf->MarginalNodes( &query.front(), query.size() );
        queryPotBK = pBKInf->GetQueryJPD();
        std::cout<<"BK inference"<<std::endl;
        queryPotBK->Dump();
        
        pJTreeInf->MarginalNodes( &query.front(), query.size() );
        queryPotJTree = pJTreeInf->GetQueryJPD();
        std::cout<<"1_5 slice inference"<<std::endl;
        queryPotJTree->Dump();

        if( !queryPotJTree->IsFactorsDistribFunEqual(queryPotBK, eps) )
        {
            ret = TRS_FAIL;
            break;
        }
        
    }
    



    delete pJTreeInf;
    delete pBKInf;
    for( slice = 0; slice < nTimeSlices; slice++ )
    {
        delete pEvid[0][slice];
    }
    
    delete pDBN;
    
    
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on Unroll DBN");
    
    
}

void initABKInfDBN()
{
    trsReg(func_name, test_desc, test_class, testBKInfDBN);
}