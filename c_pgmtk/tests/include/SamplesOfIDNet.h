/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      SamplesOfIDNet.h                                            //
//                                                                         //
//  Purpose:   Influence Diagram net examples                              //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef SAMPLESOFIDNET_H
#define SAMPLESOFIDNET_H

#include "pnl_dll.hpp"
#include "pnlIDNet.hpp"

PNL_USING

CIDNet* CreateRandomIDNet(int num_nodes, int num_indep_nodes,
  int max_size_family, int num_decision_nodes, 
  int max_num_states_chance_nodes = 2, int max_num_states_decision_nodes = 2,
  int min_utility = -100, int max_utility = 100,
  bool is_uniform_start_policy = true);

CIDNet* CreatePigsLIMID();

CIDNet* CreateAppleJackLIMID();

CIDNet* CreateOilLIMID();

CIDNet* CreateLIMIDWith2DecInClick();

#endif
