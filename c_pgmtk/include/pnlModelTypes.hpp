/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlModelTypes.hpp                                           //
//                                                                         //
//  Purpose:   EModelTypes enum definition                                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLMODELTYPES_HPP__
#define __PNLMODELTYPES_HPP__

PNL_BEGIN

typedef enum  /* enum of allowed model types */
{   mtMNet
,   mtMRF2
,   mtBNet
,   mtIDNet
,   mtJTree
,   mtDBN
,   mtFactorGraph
} EModelTypes;

PNL_END

#endif //ModelTypes.h

