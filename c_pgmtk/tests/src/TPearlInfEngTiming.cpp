/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      TPearlInfEngTiming.cpp                                      //
//                                                                         //
//  Purpose:   Timing test for Junction Tree inference engine              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"


#define MAX_NUM_OF_ITERS 100


static char func_name[]  = "RunPearlInfEngineTiming";

static char test_desc[]  = "Run timing test for Pearl inference engine";

static char test_class[] = "Timing";


PNL_USING

CStaticGraphicalModel* ChooseModelFromExamles(int numOfModel);

void SetRndObsNdsAndVals( const CModelDomain* pModelDomain, intVector* obsNds,
                          valueVector* obsNdsVals );


int RunPearlInfEngineTiming()
{
    trsWrite( TW_RUN | TW_CON,
        "Please choose one of the following models to run inference for:\n"
        "WaterSprinklerBNet - 1\n"
        "AsiaBNet           - 2\n"
        "KjaerulfsBNet      - 3\n"
        "BNetFormSMILE      - 4\n"
        "BigMRF2            - 5\n"  );
    
    int numOfModel = 0;
    
    while( ( numOfModel < 1 ) || ( numOfModel > 5 ) )
    {
        trsiRead( &numOfModel, "4",
            "Number of a model to run inference for" );
    }

    int maxNumOfIters = 0;
    
    while( ( maxNumOfIters < 1 ) || ( maxNumOfIters > MAX_NUM_OF_ITERS ) )
    {
        trsiRead( &maxNumOfIters, "40",
            "Number of iterations Pearl Inference engine will run for" );
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

    CPearlInfEngine* pPearlEng = CPearlInfEngine::Create(pModelToWorkWith);
    
    pPearlEng->SetMaxNumberOfIterations(maxNumOfIters);

    trsTimerStart(0);

    pPearlEng->EnterEvidence(pEvidence);
    
    trsTimerStop(0);
    
    double timeOfEnterEvidenceForPearl = trsTimerSec(0);
    
    delete pEvidence;
    
    delete pPearlEng;

    delete pModelToWorkWith;

    
    trsWrite( TW_RUN | TW_CON,
        " %s performance measurement:\n\n", func_name );
    
    trsWrite( TW_RUN | TW_CON,
        " Pearl Inference Entering Evidence took %g seconds\n",
        timeOfEnterEvidenceForPearl );

    return TRS_OK;
}
//////////////////////////////////////////////////////////////////////////

CStaticGraphicalModel* ChooseModelFromExamles(int numOfModel)
{
    assert( ( numOfModel > 0 ) && ( numOfModel < 6 ) );

//   "WaterSprinklerBNet - 1\n"
//   "AsiaBNet           - 2\n"
//   "KjaerulfsBNet      - 3\n"
//   "BNetFormSMILE      - 4\n"
//   "BigMRF2            - 5\n"

    switch( numOfModel )
    {
    case 1:
        return pnlExCreateWaterSprinklerBNet();

    case 2:
        return pnlExCreateAsiaBNet();

    case 3:
        return pnlExCreateKjaerulfsBNet();

    case 4:
        return pnlExCreateBNetFormSMILE();

    case 5:
        return pnlExCreateBigMRF2( 0, 15, 15, 5, 2.5f, 2.5f );

    default:
        return 0;
    }       
}
//////////////////////////////////////////////////////////////////////////

void initTPearlTiming()
{
    trsReg( func_name, test_desc, test_class, RunPearlInfEngineTiming );
}
//////////////////////////////////////////////////////////////////////////