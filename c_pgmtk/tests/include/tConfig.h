/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      tConfig.h                                                   //
//                                                                         //
//  Purpose:   General configuring test and include common files           //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#pragma warning(disable : 4786) // The identifier string exceeded the maximum allowable length and was truncated
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4514) // unreferenced inline function has been removed
#endif

#pragma warning (push, 3)

#include <vector>
#include <fstream>
#include <list>
#include <queue>
#pragma warning (pop)
