/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AConditionalGaussianLearning.cpp                            //
//                                                                         //
//  Purpose:   Test on learning for conditional Gaussian model             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "tCreateIncineratorBNet.hpp"
#include "tCreateArHMMwithGaussObs.hpp"
#include "tConfig.h"

// this file must be last included file
#include "tUtil.h"

PNL_USING


static char func_name[] = "testConditionalGaussianLearning";

static char* test_desc = "Provide learning on conditional Gaussian Model";

static char* test_class = "Algorithm";


int testConditionalGaussianLearning()
{
    
    int ret = TRS_OK;
    int seed = pnlTestRandSeed();
    seed = 1078223234;
    pnlSeed( seed );
    std::cout<<"seed"<<seed<<std::endl;
    float eps = 1e-1f;
    
    
    int i;
    CBNet* pWasteBNet = pnlExCreateWasteBNet();
    int nnodes = pWasteBNet->GetNumberOfNodes();
    intVector toggleNodes;
    toggleNodes.assign( 3, 0 );
    toggleNodes[1] = 1;
    toggleNodes[2] = 3;
    
    pEvidencesVector evidData;
    int nEv = 20000;
    pWasteBNet->GenerateSamples( &evidData, nEv );
    
    for( i = 0; i < nEv; i++ )
    {
        evidData[i]->ToggleNodeState( 3, &toggleNodes.front() );
    }
    std::cout<<"evidences was generated \n";
    
    //now we can start learning
    CBNet* pLearningBNet = CBNet::Copy( pWasteBNet );
    CEMLearningEngine* pLearnEng = CEMLearningEngine::Create(pLearningBNet);
    pLearnEng ->SetData( nEv, &evidData.front() );
    pLearnEng->Learn();
    
     std::cout<<"learning was done \n";
    const CCPD* realCPD;
    const CCPD* learnedCPD;
    for( i = 0; i < nnodes; i++ )
    {
        realCPD = static_cast<CCPD*>(pWasteBNet->GetFactor( i ));
        learnedCPD = static_cast<CCPD*>(pLearningBNet->GetFactor(i));
        if(!( realCPD->IsFactorsDistribFunEqual(learnedCPD, eps, 0 ) ))
        {
            realCPD->GetDistribFun()->Dump();
	    learnedCPD->GetDistribFun()->Dump();
	    ret = TRS_FAIL;
        }
    }
    for( i = 0; i < nEv; i++)
    {
        delete evidData[i];
    }
    delete pLearnEng;
    delete pLearningBNet;
    delete pWasteBNet;
    
    

    //////////////////////////////////////////////////////////////////
    //simple model of 4 nodes - base for Ar HMM
    
    CBNet *pArBnet = pnlExCreateCondGaussArBNet();
    int nArNodes = pArBnet->GetNumberOfNodes();
    
    pEvidencesVector evidArData;
    int nArEv = 40000;
    pArBnet->GenerateSamples( &evidArData, nArEv );
     std::cout<<"evidences was generated \n";
    
    //create copy of BNet
    CBNet* pLearnArBnet = CBNet::Copy( pArBnet );
   
    
    CEMLearningEngine* pLearnArEng = CEMLearningEngine::Create(pLearnArBnet);
    pLearnArEng ->SetData( nArEv, &evidArData.front() );
    pLearnArEng->Learn();
     std::cout<<"learning was done \n";
    
    const CCPD* realArCPD;
    const CCPD* learnedArCPD;
    
    for( i = 0; i < nArNodes; i++ )
    {
        realArCPD = static_cast<CCPD*>(pArBnet->GetFactor( i ));
        
        learnedArCPD = static_cast<CCPD*>(pLearnArBnet->GetFactor(i));
        
        if(!( realArCPD->IsFactorsDistribFunEqual(learnedArCPD, eps, 0 ) ))
        {
            ret = TRS_FAIL;
        }
    }
    
    for( i = 0; i < nArEv; i++)
    {
        delete evidArData[i];
    }
    
    delete pLearnArEng;
    delete pLearnArBnet;
    delete pArBnet;
    
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on Conditional Gaussian learning");
}

void initAConditionalGaussianLearning()
{
    trsReg(func_name, test_desc, test_class, testConditionalGaussianLearning);
}
