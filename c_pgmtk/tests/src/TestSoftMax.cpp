/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      TestSoftMax.cpp                                             //
//                                                                         //
//  Purpose:   Test for SoftMax distribution type (with EMLearning)        //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Vinogradov                                         //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "SamplesOfSMNet.h"

// ============================================================================
// macros creation
#undef CREATE_SOFTMAX_BY_FUNCTION
#undef LOAD_SOFTMAX_BY_XML

#define CREATE_SOFTMAX_BY_FUNCTION

#ifndef CREATE_SOFTMAX_BY_FUNCTION
  #define LOAD_SOFTMAX_BY_XML
#endif

#define IS_DUMP

// end of macros creation
// ============================================================================

using namespace pnl;

// ============================================================================
int main(int argc, char* argv[])
{
  CBNet* pBNet = NULL;
  CEMLearningEngine *pLearnEng = NULL;

  #ifdef CREATE_SOFTMAX_BY_FUNCTION
//    pBNet = CreateTwoNodeEx();
    pBNet = CreateSixNodeEx();
  #endif
  #ifdef LOAD_LIMID_BY_XML
    //pBNet = Load...
  #endif

  pLearnEng = CEMLearningEngine::Create(pBNet);

//  SetTwoNodeExEvidences(pBNet, pLearnEng);
  SetRandomEvidences(pBNet, pLearnEng, -1.0, 1.0, 5);

  pLearnEng->SetMaximizingMethod(mmGradient);
//  pLearnEng->SetMaximizingMethod(mmConjGradient);

  pLearnEng->Learn();

  #ifdef IS_DUMP
    switch(pLearnEng->GetMaximizingMethod())
    {
      case mmGradient:
      {
        printf("\nMaximizeMethod - GRADIENT\n");
        break;
      }
      case mmConjGradient:
      {
        printf("\nMaximizeMethod - CONJ GRADIENT\n");
        break;
      }
    }

    printf("\nResults\n========\n");
    int numOfNdsTmp = pBNet->GetNumberOfNodes();
    for (int i = 0; i < numOfNdsTmp; i++)
    {
      pBNet->GetFactor(i)->GetDistribFun()->Dump();
    }
  #endif

  delete pLearnEng;
  delete pBNet;

  return 0;
}
// end of file ----------------------------------------------------------------
