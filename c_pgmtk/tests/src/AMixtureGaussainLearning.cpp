/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMixtureGaussainLearning.cpp                                //
//                                                                         //
//  Purpose:   Test on learning for Mixture Gaussian Model                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"

// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testMixtureGaussianLearning";

static char* test_desc = "Provide learning on Mixture Gaussian Model";

static char* test_class = "Algorithm";


int testMixtureGaussianLearning()
{
    int seed = time(0);
    std::cout<<"seed = "<< seed<<std::endl;
    pnlSeed(seed);
    int ret = TRS_OK;
    float eps = 1e-1f;
    
    CBNet *pMixBNet = pnlExCreateSingleGauMix();
    //create data for learning
    pEvidencesVector evidencesMix;
    int nEvidencesForMixBNet = 100;
    pMixBNet->GenerateSamples( &evidencesMix, nEvidencesForMixBNet );
        
    CBNet *pMixBNetToLearn = CBNet::Copy(pMixBNet);
    
    CEMLearningEngine *pMixLearn;
    pMixLearn = CEMLearningEngine::Create(pMixBNetToLearn);
    
    pMixLearn->SetData( nEvidencesForMixBNet, &evidencesMix.front() );
    pMixLearn->Learn();
    
    int nnodesInMixBNet = pMixBNet->GetNumberOfNodes();
       
    int i;
    for( i = 0; i < nnodesInMixBNet; i++ )
    {
        
        if( !pMixBNet->GetFactor( i )->
            IsFactorsDistribFunEqual( pMixBNetToLearn->GetFactor( i ), eps, 0 ) )
        {
            pMixBNet->GetFactor( i )->GetDistribFun()->Dump();
	    pMixBNetToLearn->GetFactor( i )->GetDistribFun()->Dump();
	    ret = TRS_FAIL;
        }
    }
    
    for( i = 0; i < nEvidencesForMixBNet; i ++ )
    {
        delete evidencesMix[i];
    }
    evidencesMix.clear();
    
    delete pMixLearn;
    delete pMixBNetToLearn;
    delete pMixBNet;
    
    
    
    
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on Mixture Gaussian");
    
}

void initAMixtureGaussianLearning()
{
    trsReg(func_name, test_desc, test_class, testMixtureGaussianLearning);
}

