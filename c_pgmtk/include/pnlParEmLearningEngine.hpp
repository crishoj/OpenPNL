/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlParEmLearningEngine.hpp                                  //
//                                                                         //
//  Purpose:   Class definition for parallel version of EM parameter       //
//             learning algorithm for BNet                                 //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLPAREMLEARNINGENGINE_HPP__
#define __PNLPAREMLEARNINGENGINE_HPP__

#include "pnlParConfig.hpp"

#ifdef PAR_PNL

#include "pnlEmLearningEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

#ifdef PAR_RESULTS_RELIABILITY
bool PNL_API EqualResults(CEMLearningEngine& eng1, CEMLearningEngine& eng2,
    float epsilon = 1e-6, int doPrint = 0);
#endif

class PNL_API CParEMLearningEngine : public CEMLearningEngine
{
public:

#ifdef PAR_RESULTS_RELIABILITY
    int EqualResults(CEMLearningEngine&, CEMLearningEngine&, float, int);
#endif

    static CParEMLearningEngine* Create(CStaticGraphicalModel *pGrModelIn);

    virtual ~CParEMLearningEngine();

#ifdef PAR_MPI
    void Learn();
    void LearnContMPI();
#endif

#ifdef PAR_OMP
    void LearnOMP();
#endif

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CParEMLearningEngine::m_TypeInfo;
  }
#endif

protected:

    CParEMLearningEngine(CStaticGraphicalModel *pGrModel, CInfEngine *pInfEng,
        ELearningTypes LearnType);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};

PNL_END

#endif // PAR_PNL

#endif //__PNLPAREMLEARNINGENGINE_HPP__
