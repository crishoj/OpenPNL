/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AStructureLearning.cpp                                      //
//                                                                         //
//  Purpose:   Test on structure learning of BNet                          //
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

PNL_USING

static char func_name[] = "testStructureLearningEngine";

static char* test_desc = "Provide structure Learning with BIC";

static char* test_class = "Algorithm";

int testStructureLearningEngine()
{
    int ret = TRS_OK;
    int i;
    
    int  nSamples = 0;
    while( nSamples < 50 )
    {
	trsiRead( &nSamples, "50", "number of evidences");
    }	
    float eps = -1.0f;
    while( eps <= 0 )
    {
        trssRead( &eps, "1.0e-2f", "accuracy in test");
    }
    
    

CBNet *pBNet = pnlExCreateWaterSprinklerBNet();
CModelDomain *pMD = pBNet->GetModelDomain();
pEvidencesVector ev;
pBNet->GenerateSamples(&ev, nSamples);
//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////


CGraph *pGraph= CGraph::Create(pBNet->GetNumberOfNodes(), NULL, NULL, NULL);
CBNet *pInputBNet = CBNet::CreateWithRandomMatrices(pGraph, pMD);
CBICLearningEngine *pLearn = CBICLearningEngine::Create(pInputBNet);
pLearn->SetData(ev.size(), &ev.front());
pLearn->Learn();

floatVector resultBIC;
pLearn->GetCriterionValue(&resultBIC);

float ll = 0.0f;
for( i = 0; i < nSamples; i++ )
{
    ll += pBNet->ComputeLogLik(ev[i]);
}

int dimOfModel = 0;
int dim;
for ( i = 0; i < pBNet->GetNumberOfNodes(); i++)
{
    dim = 1;
    int size;
    const int *ranges;
    static_cast<CNumericDenseMatrix<float>*>(pBNet->GetFactor(i)->GetMatrix(matTable))->
	GetRanges(&size, &ranges);
    for(int j=0; j < size - 1; j++)
    {
	dim *= ranges[j];
	
    }
    dim *= ranges[size-1]-1;
    dimOfModel += dim;
}		   
float InitialBIC = ll - 0.5f*float( dimOfModel*log(float(ev.size())) );


if( (InitialBIC - resultBIC.back()) > eps*ev.size() )
{
    ret = TRS_FAIL;
}


for( i = 0; i < nSamples; i++ )
{
    delete ev[i];
}

delete pBNet;
delete pInputBNet;
delete pLearn;
return trsResult( ret, ret == TRS_OK ? "No errors" : 
"Bad test on ParamLearningEngineML");
}

void initAStructureLearningEngine()
{
    trsReg(func_name, test_desc, test_class, testStructureLearningEngine);
}