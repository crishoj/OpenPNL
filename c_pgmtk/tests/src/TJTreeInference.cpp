/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      TJTreeInference.cpp                                         //
//                                                                         //
//  Purpose:   Timing Test of Junction Tree inference engine               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "DSLPNLConverter.h"

#define DSL_NETWORKS_FOLDER         "../c_pgmtk/tests/smile/models/"
#define DSL_NETWORK_NAME            "A_diagnose.dsl"
#define NUM_OF_OBS_NDS              4
#define TIMES_TO_RUN_ENTER_EVIDENCE 10

PNL_USING

static char func_name[]  = "timeJTreeInfEngine";

static char test_desc[]  = "Run timing test for Junction Tree inference engine";

static char test_class[] = "Timing";


void SetRndObsNdsAndVals( const CModelDomain* pModelDomain, intVector* obsNds,
                          valueVector* obsNdsVals );

static inline std::string dslFilename(const std::string& file)
{
    std::stringstream filename;

    filename << DSL_NETWORKS_FOLDER << file.c_str();

    return filename.str();
}

CBNet* ConvertFromDSLNet(const std::string& filename)
{
    DSL_network dslNet;

    if( dslNet.ReadFile(const_cast<char*>(dslFilename(filename).c_str()))
        != DSL_OKAY )
    {
        std::cout << " can't read DSL model file named "
            << dslFilename(filename).c_str() << std::endl;

        return NULL;
    }

    // Convert DSL to PNL
    DSLPNLConverter DPConverter;

    CBNet* pnlNet;

    pnlNet = DPConverter.CreateBNet(dslNet);

    if( pnlNet == NULL )
    {
        std::cout << " Conversion DSL->PNL failed. " << std::endl;

        return NULL;
    }

    return pnlNet;
}

int timeJTreeInfEngine()
{
    int ret = TRS_OK;

    char filename[120];

    trstRead(filename, sizeof(filename), DSL_NETWORK_NAME, "Model name");

    trsTimerStart(0);

    CBNet* pBNet = ConvertFromDSLNet(filename);

    trsTimerStop(0);

    double timeOfDSL2PNLConversion = trsTimerSec(0);

    if( pBNet == NULL )
    {
        ret = TRS_FAIL;

        return trsResult( ret, ret == TRS_OK ? "No errors" : "JTree timing FAILED");
    }

    const CModelDomain* pModelDomain = pBNet->GetModelDomain();

    const int numOfNds               = pBNet->GetNumberOfNodes();

    const int numOfObsNds            = NUM_OF_OBS_NDS;

    assert( numOfObsNds <= numOfNds );


    intVector   obsNds(numOfObsNds);

    valueVector obsNdsVals(numOfObsNds);

    SetRndObsNdsAndVals( pModelDomain, &obsNds, &obsNdsVals );

    CEvidence* pEvidence = CEvidence::Create( pModelDomain, obsNds,
        obsNdsVals );

    trsTimerStart(1);

    CJtreeInfEngine* pJTreeInfEngine = CJtreeInfEngine::Create(pBNet);

    trsTimerStop(1);

    double timeOfInfCreation = trsTimerSec(1);


    const int numOfEnterEvidenceLoops = TIMES_TO_RUN_ENTER_EVIDENCE;

    assert( numOfEnterEvidenceLoops > 0 );

    trsTimerStart(2);

    int i = 0;

    for( ; i < numOfEnterEvidenceLoops; ++i )
    {
        pJTreeInfEngine->EnterEvidence(pEvidence);
    }

    trsTimerStop(2);

    double timeOfEnterEvidence = trsTimerSec(2);

    double averageTimeOfEnterEvidence = timeOfEnterEvidence
        /numOfEnterEvidenceLoops;

    double freqCPU = trsClocksPerSec();

    trsCSVString8( "d", func_name,
        trsDouble(timeOfInfCreation),
        trsDouble(averageTimeOfEnterEvidence),
        trsDouble(freqCPU),
        "\n JTree inference creation ",
        "\n average time for entering evidence ",
        "\n CPU frequency " );

    trsWrite( TW_RUN | TW_CON,
        " %s performance measurement:\n\n", func_name );

    trsWrite( TW_RUN | TW_CON,
        " Conversion from DSL to PNL network took    %g seconds\n"
        " JTree inference engine creation took       %g seconds\n"
        " Average entering evidence time is          %g seconds\n",
        timeOfDSL2PNLConversion,
        timeOfInfCreation,
        averageTimeOfEnterEvidence );

    delete pEvidence;

    //CJtreeInfEngine::Release(&pJTreeInfEngine);
    delete pJTreeInfEngine;

    delete pBNet;

    return trsResult( ret, ret == TRS_OK ? "No errors" : "JTree timing FAILED");
}

void initTJTreeInfEngine()
{
    trsReg( func_name, test_desc, test_class, timeJTreeInfEngine );
}