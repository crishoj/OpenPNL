/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMlStaticStructLearnHC.hpp                                //
//                                                                         //
//  Purpose:   CMlStaticStructLearnHC class definition                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLMLSTATICSTRUCTLEARNHC_HPP__
#define __PNLMLSTATICSTRUCTLEARNHC_HPP__

#include "pnlMlStaticStructLearn.hpp"

PNL_BEGIN


class PNL_API CMlStaticStructLearnHC : public CMlStaticStructLearn
{
public:
	~CMlStaticStructLearnHC(){};
	virtual void Learn();
	void SetMaxSteps(int steps);
	static CMlStaticStructLearnHC* Create(CStaticGraphicalModel* pGrModel, 
						  ELearningTypes LearnType, 
						  EOptimizeTypes AlgorithmType, 
						  EScoreFunTypes ScoreType, 
						  int nMaxFanIn,
						  intVector& vAncestor,
						  intVector& vDescent,
						  int nRestarts=1);

protected:
	CMlStaticStructLearnHC(CStaticGraphicalModel* pGrModel, 
						  ELearningTypes LearnType, 
						  EOptimizeTypes AlgorithmType, 
						  EScoreFunTypes ScoreType, 
						  int nMaxFanIn,
						  intVector& vAncestor,
						  intVector& vDescent,
						  int nRestarts=1);

	int					  m_nRestarts;
	int					  m_nSteps;

private:
	void LearnInOneStart(CDAG* InitDag, CDAG** LearnedDag, float* LearndScore);
};
PNL_END

#endif //__PNLKBNSTRUCTURELEARNING_HPP__
