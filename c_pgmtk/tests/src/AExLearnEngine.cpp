/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AExLearnEngine.cpp                                          //
//                                                                         //
//  Purpose:   Test provide learning on disconnected networks              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include "pnlExInferenceEngine.hpp"

#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "tUtil.h"

PNL_USING

static char func_name[] = "testExLearnEngine";

static char* test_desc = "Provide learning on disconnected networks";

static char* test_class = "Algorithm";

int testExLearnEngine()
{
    int ret = TRS_OK;
   //pnlSeed(1088368891);
    int seed = pnlTestRandSeed();
       
       std::cout<<"seed"<<seed<<std::endl;
       
      
       
       int nSamples = -1;
       while(nSamples < 400)
       {
           trsiRead (&nSamples, "400", "Number of slices");
       }
       
       float eps = -1.0f;
       while( eps <= 0 )
       {
           trssRead( &eps, "1.e-1f", "accuracy in test");
       }
       
       int nnodes = 10;
       int nedges = 2;
       int ndSz = 2;
       float edgeprob = 0.4f;
   
       CBNet* pBNet = pnlExCreateRandomBNet( nnodes, nedges, 0, ndSz, 1, edgeprob, true );
       
       
       CModelDomain *pMD = pBNet->GetModelDomain();
       CGraph *pGraph =pBNet->GetGraph();

       CBNet* pBNetToLearn = CBNet::CreateWithRandomMatrices(CGraph::Copy(pGraph), pMD);
       std::cout<<"bnet"<<std::endl;
       
       
       
       CExInfEngine< CJtreeInfEngine, CBNet, PNL_EXINFENGINEFLAVOUR_DISCONNECTED > *pInf = 
                	CExInfEngine< CJtreeInfEngine, CBNet, PNL_EXINFENGINEFLAVOUR_DISCONNECTED >::
       		Create( pBNetToLearn  );
       
       
       CEMLearningEngine *pLearn = CEMLearningEngine::Create(pBNetToLearn, pInf);
   
       pEvidencesVector pEv;
       pBNet->GenerateSamples( &pEv, nSamples );
       
       int sn, i;
       for( i = 0; i < nSamples; i++ )
       {
	   sn = (int)pnlRand(0, nnodes-1);
	   pEv[i]->MakeNodeHiddenBySerialNum(sn);
       }
       
        std::cout<<"samples"<<std::endl;

       pLearn->SetData(nSamples, &pEv.front() );
       pLearn->Learn();
       std::cout<<"learn"<<std::endl;


       for( i = 0; i < pBNet->GetNumberOfNodes(); i++ )
       {
	   if(  ! pBNetToLearn->GetFactor(i)->IsFactorsDistribFunEqual(pBNet->GetFactor(i), eps))
	   {
	       ret = TRS_FAIL;
	       pBNetToLearn->GetFactor(i)->GetDistribFun()->Dump();
	       pBNet->GetFactor(i)->GetDistribFun()->Dump();
		       
	       break;
	   }
       }

       for( i = 0; i < pEv.size(); i++ )
       {
	   delete pEv[i];
       }
       
       delete pInf;
       delete pBNet;
       delete pBNetToLearn;
      
      
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test of learning disconnected network");
    
    
}

void initAExLearnEngine()
{
    trsReg(func_name, test_desc, test_class, testExLearnEngine);
}