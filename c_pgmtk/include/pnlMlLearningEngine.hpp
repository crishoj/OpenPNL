/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMlLearningEngine.hpp                                     //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLMLLEARNINGENGINE_HPP__
#define __PNLMLLEARNINGENGINE_HPP__

#include "pnlStaticLearningEngine.hpp" 

PNL_BEGIN

class PNL_API CMlLearningEngine : public CStaticLearningEngine
{
public:

    ~CMlLearningEngine();
#ifdef PNL_OBSOLETE
    virtual void SetData(int size, CEvidence * const* pEvidences);
#endif
    void Learn();

	
protected:
	CMlLearningEngine(CStaticGraphicalModel *pGrModel,
		ELearningTypes LearnType);
	float _LearnPotentials();

private:
	
};

PNL_END

#endif //__PNLMlLLEARNINGENGINE_HPP__
