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
   
    int seed = pnlTestRandSeed();
       
       std::cout<<"seed"<<seed<<std::endl;
       
      
       
       int nSamples = -1;
       while(nSamples < 100)
       {
           trsiRead (&nSamples, "100", "Number of slices");
       }
       
       float eps = -1.0f;
       while( eps <= 0 )
       {
           trssRead( &eps, "1.5e-2f", "accuracy in test");
       }
       
       int nnodes = 10;
       int nedges = 1;
       int ndSz = 2;
       float edgeprob = 0.4f;
   
       CBNet* pBNet = pnlExCreateRandomBNet( nnodes, nedges, 0, ndSz, 1, edgeprob, true );
       CModelDomain *pMD = pBNet->GetModelDomain();
       CGraph *pGraph =pBNet->GetGraph();

       CBNet* pBNetToLearn = CBNet::CreateWithRandomMatrices(CGraph::Copy(pGraph), pMD);
       
       
       
       CExInfEngine< CNaiveInfEngine, CBNet, PNL_EXINFENGINEFLAVOUR_DISCONNECTED > *pInf = 
                	CExInfEngine< CNaiveInfEngine, CBNet, PNL_EXINFENGINEFLAVOUR_DISCONNECTED >::
       		Create( pBNetToLearn  );
       
       //CJtreeInfEngine *pInf = CJtreeInfEngine::Create(pBNetToLearn);
       
       CEMLearningEngine *pLearn = CEMLearningEngine::Create(pBNetToLearn, pInf);
   
       pEvidencesVector pEv;
       pBNet->GenerateSamples( &pEv, nSamples );
   
       int sn, i;
       for( i = 0; i < nSamples; i++ )
       {
   	sn = (int)pnlRand(0, nnodes);
   	pEv[i]->MakeNodeHiddenBySerialNum(sn);
       }
   
       pLearn->SetData(nSamples, &pEv.front() );
       pLearn->Learn();

       for( i = 0; i < pBNet->GetNumberOfNodes(); i++ )
       {
	   if(  ! pBNetToLearn->GetFactor(i)->IsFactorsDistribFunEqual(pBNet->GetFactor(i), eps))
	   {
	       ret = TRS_FAIL;
	       break;
	   }
       }

       for( i = 0; i < pEv.size(); i++ )
       {
	   if( i == 17 )
	   {
	      int ttt = 1;
	   }
	   delete pEv[i];
	   std::cout<<i<<std::endl;
       }

       delete pBNet;
       delete pBNetToLearn;
      
      
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on learning disconnected network");
    
    
}

void initAExLearnEngine()
{
    trsReg(func_name, test_desc, test_class, testExLearnEngine);
}