/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ALearningCondGaussDBN.cpp                                   //
//                                                                         //
//  Purpose:   Test on factor estimation of DBN using EM algorithm         //
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
static char func_name[] = "testLearningCondGaussDBN";

static char* test_desc = "Provide Learning CondGaussian DBN";
void cmp();

static char* test_class = "Algorithm";

int testLearningCondGaussDBN()
{
    int ret = TRS_OK;
    float eps = 0.1f;

    CBNet *pBNet; //= pnlExCreateCondGaussArBNet();
    
    
    CBNet *pTestBNet = pnlExCreateCondGaussArBNet();
    CDBN *pTestDBN = CDBN::Create(pTestBNet);
    
    CGraph *pGraph = CGraph::Copy(pTestBNet->GetGraph());
    CModelDomain *pMD = pTestBNet->GetModelDomain();

    pBNet = CBNet::CreateWithRandomMatrices(pGraph, pMD);
    CDBN *pDBN = CDBN::Create(CBNet::Copy(pBNet));
    
    
    int nSamples = 5000;
    intVector nSlices( nSamples, 2);
    pEvidencesVecVector evidences;
    pTestDBN->GenerateSamples( &evidences, nSlices );
    
    pEvidencesVector evidBnet;
    pBNet->GenerateSamples(&evidBnet, nSamples);

    valueVector vls1, vls2, newData;
    
    int i;
    for( i = 0; i < nSamples; i++ )
    {
	vls1.clear();
	vls2.clear();
	
	(evidences[i])[0]->GetRawData(&vls1);
	(evidences[i])[1]->GetRawData(&vls2);
	newData.resize(vls1.size()+vls2.size());
	
	memcpy(&newData.front(), &vls1.front(), vls1.size()*sizeof(Value));
	memcpy(&newData[vls1.size()], &vls2.front(), vls2.size()*sizeof(Value));
	
	evidBnet[i]->SetData(newData);
	
	//(evidences[i])[0]->MakeNodeHiddenBySerialNum(0);
	//(evidences[i])[1]->MakeNodeHiddenBySerialNum(0);

	
	//evidBnet[i]->MakeNodeHiddenBySerialNum(0);
	//evidBnet[i]->MakeNodeHiddenBySerialNum(2);

    }

   
    

    CEMLearningEngine *pLearnBNet = CEMLearningEngine::Create(pBNet);
    int parentIndices[] = {0};
    pBNet->GetFactor(1)->GetDistribFun()->GetMatrix(matMean, -1, parentIndices)->SetClamp(1);
    
    pBNet->GetFactor(3)->GetDistribFun()->GetMatrix(matCovariance, -1, parentIndices)->SetClamp(1);
    pBNet->GetFactor(3)->GetDistribFun()->GetMatrix(matCovariance, 0, parentIndices)->SetClamp(1);


    CEMLearningEngineDBN *pLearnDBN = CEMLearningEngineDBN::Create(pDBN);

    pLearnBNet->SetData( nSamples, &evidBnet.front() );
    pLearnBNet->SetMaxIterEM(10);
    pLearnDBN->SetData(evidences);
    pLearnDBN->SetMaxIterEM(10);


    pLearnBNet->Learn();
    std::cout<<"++++++++++++++++++++++++++++++++++++++++++++++++ \n";
    pLearnDBN->Learn();
    const CMatrix<float>* mat;
    mat = pBNet->GetFactor(0)->GetDistribFun()->GetStatisticalMatrix(stMatTable);
    mat = pDBN->GetFactor(0)->GetDistribFun()->GetStatisticalMatrix(stMatTable);
    
    for( i = 0; i < 4; i++ )
    {
	
	std::cout<<"\n ___ node "<<i<<"_________________________\n";
	std::cout<<"\n____ BNet_________________________________\n";
	pBNet->GetFactor(i)->GetDistribFun()->Dump();
	
	if(!pTestDBN->GetFactor(i)->IsFactorsDistribFunEqual(pDBN->GetFactor(i), eps, 0))
	{
	    std::cout<<"\n____ DBN__________________________________\n";
	    pDBN->GetFactor(i)->GetDistribFun()->Dump();
	    std::cout<<"\n____ Initial DBN__________________________\n";
	    pTestBNet->GetFactor(i)->GetDistribFun()->Dump();
	    std::cout<<"\n___________________________________________\n";
	    ret = TRS_FAIL;
	}
    }
    delete pLearnDBN;
    delete pLearnBNet;
    
    for( i = 0; i < nSamples; i++ )
    {
	int j;
	for(j = 0; j < evidences[i].size(); j++ )
	{
	    delete (evidences[i])[j];
	}
	delete evidBnet[i];
    }
    delete pBNet;
    delete pDBN;
    delete pTestDBN;

    
    
   
   
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on Unroll DBN");
    
    
}

void cmp()
{ 
    
    
    
}

void initALearningCondGaussDBN()
{
    trsReg(func_name, test_desc, test_class, testLearningCondGaussDBN);
}