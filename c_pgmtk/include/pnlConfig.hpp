/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlConfig.hpp                                               //
//                                                                         //
//  Purpose:   Config file with library settings and macros                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLCONFIG_HPP__
#define __PNLCONFIG_HPP__

#include "pnlParConfig.hpp"
#include "pnlConfig.h"

#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif


#pragma warning (push, 3 )
#include <map>
#include <list>
#include <vector>
#include <queue>
#include <iostream>
#include <algorithm>
#pragma warning(pop)


#define PNL_BEGIN namespace pnl {
#define PNL_END   }
#define PNL_USING using namespace pnl;

#define PNL_OBSOLETE

#ifdef PNL_I_WANT_CODE_COVERAGE
#include "pnlCoverage.hpp"
#endif

#include "pnlNumericDenseMatrix.hpp"
#include "pnlNumericSparseMatrix.hpp"
#include "pnl2DNumericDenseMatrix.hpp"


#define C_TRY()\
		try {\
			while(0) 

#define C_CATCH()\
		return pgmOK;\
	}\
	catch (CException e)\
	{\
		return e.GetCode();\
	}\
	catch(...)\
	{\
		return pgmInternalError;\
	}\

#endif //__PNLCONFIG_HPP__
