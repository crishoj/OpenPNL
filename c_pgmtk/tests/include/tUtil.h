/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      tUtil.h                                                     //
//                                                                         //
//  Purpose:   Utilities for tests                                         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

// Function to get random seed
// Designed for debugging purposes:
//   - when _DEBUG macro is defined it returns one number (for exmaple 121) on
//     every call
int pnlTestRandSeed();
