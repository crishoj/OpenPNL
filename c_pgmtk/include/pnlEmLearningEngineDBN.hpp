/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlEmLearningEngineDBN.hpp                                  //
//                                                                         //
//  Purpose:   Class definition for the EM factor learning algorithm    //
//             for Dynamic Bayesian Network                                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLEMLEARNINGENGINEDBN_HPP__
#define __PNLEMLEARNINGENGINEDBN_HPP__

#include "pnlDynamicLearningEngine.hpp"
#include "pnlDynamicInferenceEngine.hpp"
#include "pnlDBN.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CEMLearningEngineDBN : public CDynamicLearningEngine
{
public:
    ~CEMLearningEngineDBN();
    static CEMLearningEngineDBN * Create(CDBN* pDBNIn, CDynamicInfEngine *pInfEngine = NULL );
    void Learn();
    void SetMaxIterEM(int Niter = 30);
    void SetTerminationToleranceEM(float precision = 0.001f);

    //inline const CInfEngine *GetInfEngine(){ return m_pInfEngine;};

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CEMLearningEngineDBN::m_TypeInfo;
  }
#endif
protected:
    CEMLearningEngineDBN(CDynamicGraphicalModel *pGrModel, ELearningTypes LearnType, 
	CDynamicInfEngine *pInfEngine );
    CEvidence* CreateEvIfDomainObserved(int nnodes,
	const int* domain, const pConstEvidenceVector* pEvidences,
	int slice, CDynamicGraphicalModel* pGrModel);
    bool IsDomainObserved(int nnodes, const int* domain,
	const pConstEvidenceVector* pEvidences,	int nnodesPerSlice, int slice);

    bool IsInfNeed(const pConstEvidenceVector* pEvidences, int nnodesPerSlice);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    CDynamicInfEngine *m_pInfEngine;

    int m_maxIter;
    float m_precisionEM;
    bool m_bDelInf;

};


PNL_END

#endif //__PNLEMLEARNINGENGINEDBN_HPP__
