/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMixtureGaussainModel.cpp                                   //
//                                                                         //
//  Purpose:   Test on inference for Mixture Gaussian Model                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include "tCreateIncineratorBNet.hpp"

PNL_USING

static char func_name[] = "testMixtureGaussian";

static char* test_desc = "Provide inference and learning on Mixture Gaussian Model";

static char* test_class = "Algorithm";


int testMixtureGaussian()
{
    int ret = TRS_OK;
    float eps = 1.5e-1f;
    
    CBNet* pMixBNet = pnlExCreateSimpleGauMix();
    //create evidence on all Gaussian nodes
    //int nnodes = pMixBNet->GetNumberOfNodes();
    pMixBNet->GetGraph()->Dump();
    
    //create evidence for inference
    int nObsNds = 3;
    int obsNds[] = { 2, 3, 4 };
    valueVector obsVals;
    obsVals.assign( 3, (Value)0 );
    obsVals[0].SetFlt(-.5f);
    obsVals[1].SetFlt(2.0f);
    obsVals[2].SetFlt(1.0f);
    CEvidence* evid = CEvidence::Create( pMixBNet, nObsNds, obsNds, obsVals );
    
    //create inference
    CNaiveInfEngine* inf = CNaiveInfEngine::Create( pMixBNet );
    CJtreeInfEngine* infJ = CJtreeInfEngine::Create( pMixBNet );
    
    //start inference with maximization
    int maxFlag = 1;
    inf->EnterEvidence( evid, maxFlag );
    int queryNode = 0;
    inf->MarginalNodes( &queryNode, 1 );
    const CEvidence* mpeEv = inf->GetMPE();
    int mpe = mpeEv->GetValue( queryNode )->GetInt();

    infJ->EnterEvidence( evid, maxFlag );
    infJ->MarginalNodes( &queryNode, 1 );
    const CEvidence* mpeEvJ = infJ->GetMPE();
    int mpeJ = mpeEvJ->GetValue( queryNode )->GetInt();
    if( mpe != mpeJ )
    {
        ret = TRS_FAIL;
    }

    /*queryNode = 0;
    inf->MarginalNodes( &queryNode, 1 );
    mpeEv = inf->GetMPE();
    mpe = mpeEv->GetValue( queryNode )->GetInt();

    int queryNds[] = { 0, 1 };
    inf->MarginalNodes( queryNds, 2);
    mpeEv = inf->GetMPE();
    int mpe0 = mpeEv->GetValue( queryNds[0] )->GetInt();
    int mpe1 = mpeEv->GetValue( queryNds[1] )->GetInt();*/

    //start inference without maximization
    maxFlag = 0;
    queryNode = 0;
    //naive inference
    inf->EnterEvidence( evid, maxFlag );
    inf->MarginalNodes( &queryNode, 1 );
    const CPotential* resMarg = inf->GetQueryJPD();
    //resMarg->Dump();
    
    //jtree inference
    infJ->EnterEvidence( evid, maxFlag );
    infJ->MarginalNodes( &queryNode, 1 );
    const CPotential* resMargJ = infJ->GetQueryJPD();
    //resMargJ->Dump();

    if( !resMargJ->IsFactorsDistribFunEqual(resMarg, eps) )
    {
        ret = TRS_FAIL;
    }

    //naive inference
/*    queryNode = 1;
    inf->MarginalNodes( &queryNode, 1 );
    resMarg = inf->GetQueryJPD();
    resMarg->Dump();
    //jtree inference
    infJ->MarginalNodes( &queryNode, 1 );
    resMargJ = infJ->GetQueryJPD();
    resMargJ->Dump();

    //naive inference
    inf->MarginalNodes( queryNds, 2);
    resMarg = inf->GetQueryJPD();
    resMarg->Dump();

    //jtree inference
    infJ->MarginalNodes( queryNds, 2);
    resMargJ = infJ->GetQueryJPD();
    resMargJ->Dump();*/
    
   
    delete evid;
    delete inf;
    //CJtreeInfEngine::Release(&infJ);
    delete infJ;
   
    delete pMixBNet;
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
		"Bad test on Mixture Gaussian");
}

void initAMixtureGaussian()
{
    trsReg(func_name, test_desc, test_class, testMixtureGaussian);
}

