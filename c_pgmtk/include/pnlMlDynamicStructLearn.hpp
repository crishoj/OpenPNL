/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMlDynamicStructLearn.hpp                                 //
//                                                                         //
//  Purpose:   CMlDynamicStructLearn class definition                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLMLDYNAMICSTRUCTLEARN_HPP__
#define __PNLMLDYNAMICSTRUCTLEARN_HPP__

#include "pnlMlStaticStructLearn.hpp"
#include "pnlMlStaticStructLearnHC.hpp"
#include "pnlDynamicLearningEngine.hpp"
#include "pnlDBN.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CMlDynamicStructLearn : public CDynamicLearningEngine
{
public:
	static CMlDynamicStructLearn* Create( CDBN* pGrModel, 
						  ELearningTypes LearnType, 
						  EOptimizeTypes AlgorithmType, 
						  EScoreFunTypes ScoreType, 
						  int nMaxFanIn,
						  int nRestarts,
						  int nMaxIters );

	~CMlDynamicStructLearn();
	void Learn();
	const CDAG* GetResultDag()const;
	void SetLearnPriorSlice(bool learnPriroSlice);
	void SetMinProgress(float minProgress);

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CMlDynamicStructLearn::m_TypeInfo;
  }
#endif
protected:
	CMlDynamicStructLearn( CDBN* pGrModel, 
						  ELearningTypes LearnType, 
						  EOptimizeTypes AlgorithmType, 
						  EScoreFunTypes ScoreType, 
						  int nMaxFanIn,
						  int nRestarts,
						  int nMaxIters );

	int		m_nSeries;
	int		m_nRestarts;
	int		m_nMaxFanIn;
	int		m_nMaxIters;
	CDAG*	m_pResultDag;
	CDBN*	m_pGrModel;
	bool	m_LearnPriorSlice;
	float	m_minProgress;
	ELearningTypes m_LearnType;
	EOptimizeTypes m_AlgorithmType; 
	EScoreFunTypes m_ScoreType;
	pEvidencesVector	m_vEvidences;
	pEvidencesVector	m_vEvidence0;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
	void RearrangeEvidences();
};

PNL_END

#endif //__PNLMLDYNAMICSTRUCTLEARN_HPP__
