/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      LoadIDNet.h                                                 //
//                                                                         //
//  Purpose:   Loading Influence Diagram net                               //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef LOADIDNET_H
#define LOADIDNET_H

#include "pnl_dll.hpp"
#include "pnlIDNet.hpp"

PNL_USING

using namespace std;

CIDNet* LoadIDNetFromXML(const string fname);

int SaveIDNetAsXML(CIDNet* pIDNet, const string fname);

#endif