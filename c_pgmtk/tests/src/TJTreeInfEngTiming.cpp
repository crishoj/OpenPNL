/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      TJTreeInfEngTiming.cpp                                      //
//                                                                         //
//  Purpose:   Timing test for Junction Tree inference engine              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"

static char func_name[]  = "RunJTreeInfEngineTiming";

static char test_desc[]  = "Run timing test for Junction Tree inference engine";

static char test_class[] = "Timing";

PNL_USING

CStaticGraphicalModel* ChooseModelFromExamles(int numOfModel);

void SetRndObsNdsAndVals( const CModelDomain* pModelDomain, intVector* obsNds,
                          valueVector* obsNdsVals );


int RunJTreeInfEngineTiming()
{
    trsWrite( TW_RUN | TW_CON,
        "Please choose one of the following models to run inference for:\n"
        "WaterSprinklerBNet - 1\n"
        "AsiaBNet           - 2\n"
        "KjaerulfsBNet      - 3\n"
        "BNetFormSMILE      - 4\n" );
    
    int numOfModel = 0;
    
    while( ( numOfModel < 1 ) || ( numOfModel > 4 ) )
    {
        trsiRead( &numOfModel, "3",
            "Number of a model to run inference for" );
    }
    
   
    CStaticGraphicalModel* pModelToWorkWith = ChooseModelFromExamles(numOfModel);
    
    const int numOfObsNds = rand()
        % ( pModelToWorkWith->GetNumberOfNodes() - 2 );
    
    intVector   obsNds(numOfObsNds);
    
    valueVector obsNdsVals(numOfObsNds);
    
    SetRndObsNdsAndVals( pModelToWorkWith->GetModelDomain(), &obsNds,
        &obsNdsVals );
    
    CEvidence* pEvidence = CEvidence::Create( pModelToWorkWith,
        obsNds, obsNdsVals );
    
    CJtreeInfEngine* pJTreeEng = CJtreeInfEngine::Create(pModelToWorkWith);
    
    trsTimerStart(0);
    
    pJTreeEng->EnterEvidence(pEvidence);
    
    trsTimerStop(0);
    
    double timeOfEnterEvidenceForJTree = trsTimerSec(0);
    
    delete pEvidence;
    
    //CJtreeInfEngine::Release(&pJTreeEng);
    delete pJTreeEng;

    delete pModelToWorkWith;


    trsWrite( TW_RUN | TW_CON,
        " %s performance measurement:\n\n", func_name );
    
    trsWrite( TW_RUN | TW_CON,
        " JTree Inference Entering Evidence took %g seconds\n",
        timeOfEnterEvidenceForJTree );
    
    return TRS_OK;
}
//////////////////////////////////////////////////////////////////////////

void initTJTreeTiming()
{
    trsReg( func_name, test_desc, test_class, RunJTreeInfEngineTiming );
}
//////////////////////////////////////////////////////////////////////////