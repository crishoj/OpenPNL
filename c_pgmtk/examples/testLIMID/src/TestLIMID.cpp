/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      TestLIMID.cpp                                               //
//                                                                         //
//  Purpose:   Test for Influence Diagram                                  //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "LoadIDNet.h"
#include "SamplesOfIDNet.h"
#include "pnlLIMIDInfEngine.hpp"

// ============================================================================
// macros creation
#undef CREATE_LIMID_BY_FUNCTION
#undef LOAD_LIMID_BY_XML

#define CREATE_LIMID_BY_FUNCTION

#ifndef CREATE_LIMID_BY_FUNCTION
  #define LOAD_LIMID_BY_XML
#endif

#define IS_DUMP

// end of macros creation
// ============================================================================

using namespace pnl;

// ============================================================================
int main(int argc, char* argv[])
{
  CIDNet* pIDNet;

  #ifdef CREATE_LIMID_BY_FUNCTION
    pIDNet = CreatePigsLIMID();
//    pIDNet = CreateAppleJackLIMID();
//    pIDNet = CreateOilLIMID();
//    pIDNet = CreateLIMIDWith2DecInClick();
  #endif
  #ifdef LOAD_LIMID_BY_XML
    pIDNet = LoadIDNetFromXML(argv[1]);
  #endif

  #ifdef IS_DUMP
    pIDNet->GetGraph()->Dump();
  #endif

  CLIMIDInfEngine *pInfEng = NULL;
  pInfEng = CLIMIDInfEngine::Create(pIDNet);

  pInfEng->DoInference();

  #ifdef IS_DUMP
    pFactorVector *Vec = pInfEng->GetPolitics();
    printf("\n=====================\nPolitics are:\n");
    for (int i = 0; i < Vec->size(); i++)
    {
      (*Vec)[i]->GetDistribFun()->Dump();
    }
    float res = pInfEng->GetExpectation();
    printf("\nNumber of iterations is %d", pInfEng->GetIterNum());
    printf("\nExpectation is %.3f", res);
  #endif

  CLIMIDInfEngine::Release(&pInfEng);
  delete pIDNet;

  return 0;
}
// end of file ----------------------------------------------------------------
