/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ASetStatistics.cpp                                          //
//                                                                         //
//  Purpose:   Test on factor estimation using safficient statistics       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>

#include <float.h>
#include <assert.h>

#include "tUtil.h"

PNL_USING
static char func_name[] = "testSetStatistics";

static char* test_desc = "Provide Learning using safficient statistics";

static char* test_class = "Algorithm";

int testSetStatistics()
{
    int ret = TRS_OK;
    float eps = 0.1f;
    
    int seed = pnlTestRandSeed();
    pnlSeed( seed );   
            
    CBNet *pBNet = pnlExCreateCondGaussArBNet();
    CModelDomain *pMD = pBNet->GetModelDomain();

    
    CGraph *pGraph = CGraph::Copy(pBNet->GetGraph());
    
    CBNet *pBNet1 = CBNet::CreateWithRandomMatrices( pGraph, pMD );

    pEvidencesVector evidences;
    int nEvidences = pnlRand( 3000, 4000);
    
    pBNet->GenerateSamples( &evidences, nEvidences );
   
    
    int i;
    for( i = 0; i < nEvidences; i++)
    {
	
	//evidences[i]->MakeNodeHiddenBySerialNum(0);
    }
    

    CEMLearningEngine *pLearn = CEMLearningEngine::Create(pBNet1);
    pLearn->SetData( nEvidences, &evidences.front() );
    pLearn->SetMaxIterEM();
    pLearn->Learn();

    for( i = 0; i < pBNet->GetNumberOfFactors(); i++ )
    {
	if( ! pBNet->GetFactor(i)->IsFactorsDistribFunEqual(pBNet1->GetFactor(i), eps))
	{
	    ret = TRS_FAIL;
	    pBNet->GetFactor(i)->GetDistribFun()->Dump();
	    pBNet1->GetFactor(i)->GetDistribFun()->Dump();

	}
    }
    
    CDistribFun *pDistr;
    const CMatrix<float>* pMat;
    CFactor *pCPD;
    
    pDistr = pBNet1->GetFactor(0)->GetDistribFun();
    pMat = pDistr->GetStatisticalMatrix(stMatTable);
    
    pCPD = pBNet->GetFactor(0);
    pCPD->SetStatistics(pMat, stMatTable);
    pCPD->ProcessingStatisticalData(nEvidences);
    if( ! pCPD->IsFactorsDistribFunEqual(pBNet1->GetFactor(0), 0.0001f) )
    {
	ret = TRS_FAIL;
    }
    

    pDistr = pBNet1->GetFactor(1)->GetDistribFun();
    
    int parentVal;
    pCPD = pBNet->GetFactor(1);
    
    parentVal = 0;

    pCPD->SetStatistics(pMat, stMatCoeff);

    pMat = pDistr->GetStatisticalMatrix(stMatMu, &parentVal);
    pCPD->SetStatistics(pMat, stMatMu, &parentVal);
    
    
    pMat = pDistr->GetStatisticalMatrix(stMatSigma, &parentVal);
    pCPD->SetStatistics(pMat, stMatSigma, &parentVal);
    
    parentVal = 1;
    
    pMat = pDistr->GetStatisticalMatrix(stMatMu, &parentVal);
    pCPD->SetStatistics(pMat, stMatMu, &parentVal);
    
    
    pMat = pDistr->GetStatisticalMatrix(stMatSigma, &parentVal);
    pCPD->SetStatistics(pMat, stMatSigma, &parentVal);

    pCPD->ProcessingStatisticalData(nEvidences);
    
    if( ! pCPD->IsFactorsDistribFunEqual(pBNet1->GetFactor(1), eps) )
    {
	ret = TRS_FAIL;
    }
    
    
    for( i = 0; i < nEvidences; i++)
    {
	delete evidences[i];
    }
    delete pLearn;
    delete pBNet1;
    delete pBNet;

    
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on SetStatistics");
    
    
}

void initASetStatistics()
{
    trsReg(func_name, test_desc, test_class, testSetStatistics);
}