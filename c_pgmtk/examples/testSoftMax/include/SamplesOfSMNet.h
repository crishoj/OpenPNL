/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      SamplesOfSMNet.h                                            //
//                                                                         //
//  Purpose:   BNet examples with SoftMax distribution type nodes          //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Vinogradov                                         //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef SAMPLESOFSMNET_H
#define SAMPLESOFSMNET_H

#include "pnl_dll.hpp"

PNL_USING

void SetTwoNodeExEvidences(CBNet* pBNet, CEMLearningEngine *pLearnEng);
CBNet* CreateTwoNodeEx(void);

CBNet* CreateSixNodeEx(void);

CBNet* CreateSevenNodeEx(void);

CBNet* CreateGaussianExample(void);

void GenerateSoftMaxEvidence(CStaticGraphicalModel *pBNet, float StartVal,
  float FinishVal, valueVector &vls);

void SetRandomEvidences(CBNet* pBNet, CEMLearningEngine *pLearnEng, 
  float StartVal, float FinishVal, int NumOfEvidences);

void SetRandEvidences(CBNet* pBNet, CEMLearningEngine *pLearnEng, 
  float StartVal, float FinishVal, int NumOfEvidences, int nObsNds, int* obsNds);

void GenerateEvidence(CStaticGraphicalModel *pBNet, float StartVal,
  float FinishVal, valueVector &vls, int nObsNds, int* obsNds);

#endif
