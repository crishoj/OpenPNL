/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      APerformanceJTreeInf.cpp                                   //
//                                                                         //
//  Purpose:   Jnction tree inference performance test                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include "pnlExampleModels.hpp"
// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testPerformanceJTreeInf";

static char* test_desc = "Jnction tree inference performance test";

static char* test_class = "Performance";


int testPerformanceJTreeInf()
{

    int seed = pnlTestRandSeed();
    srand( seed );

    CBNet *pBNet = pnlExCreateAsiaBNet();

    double freqCPU = trsClocksPerSec();

    trsTimerStart(0);
    CJtreeInfEngine *pInfEng = CJtreeInfEngine::Create( pBNet );
    trsTimerStop(0);

    double timeOfInfCreation = trsTimerSec(0);

    int nLoops = 10;
    int nObsNds = 5;
    double timeOfInfDoing = 0;
    int loop;
    for ( loop = 0; loop < nLoops; loop ++ )
    {

	intVector obsNds;
	valueVector obsVals( nObsNds );

	int i = 0;
	do
	{
	    int value;
	    do
	    {
		value = rand()%(pBNet->GetNumberOfNodes()-1);

	    }while(std::find(obsNds.begin(), obsNds.end(), value) != obsNds.end());

	    obsNds.push_back(value);
	    obsVals[ i++ ].SetInt(rand()%(pBNet->GetNodeType(value)->GetNodeSize()));

	}while(obsNds.size() != nObsNds);


	CEvidence *pEv = CEvidence::Create( pBNet->GetModelDomain(), obsNds, obsVals );


	trsTimerStart(0);
	pInfEng->EnterEvidence(pEv);
	trsTimerStop(0);
	timeOfInfDoing += trsTimerSec(0);

	delete pEv;
    }

    double  everageTime = timeOfInfDoing/nLoops;

    static char* comments1 ="\n time of inference procedure creation";
    static char* comments2 ="\n everage time of inference doing";
    static char* comments3 ="\n CPU Freq";

    trsCSVString8("d", func_name, trsDouble(timeOfInfCreation), trsDouble(everageTime),
	trsDouble(freqCPU), comments1, comments2, comments3);

    trsWrite(TW_RUN | TW_CON, "%s performance at %lg MHz\n", func_name, freqCPU/10000.0);
    static char* comments = "time of inference procedure creation %s\n  everage time of inference doing  %s \n";
    trsWrite(TW_RUN | TW_CON, comments , trsDouble(timeOfInfCreation), trsDouble(everageTime));

    //CJtreeInfEngine::Release(&pInfEng);
    delete pInfEng;
    delete pBNet;

    return TRS_OK;
}

void initAPerformanceJTreeInf()
{
    trsReg(func_name, test_desc, test_class, testPerformanceJTreeInf);
}

