/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDynamicLearningEngine.hpp                                //
//                                                                         //
//  Purpose:   Implementation of the base class for all learning           //
//             algorithms for dynamic graphical models                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// DynamicLearningEngine.hpp: interface for the CDynamicLearningEngine class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __PNLDYNAMICLEARNINGENGINE_HPP__
#define __PNLDYNAMICLEARNINGENGINE_HPP__

#include "pnlLearningEngine.hpp"
#include "pnlDynamicGraphicalModel.hpp"
#include "pnlEvidence.hpp"
#include "pnlInferenceEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CDynamicLearningEngine : public CLearningEngine
{
public:
    virtual ~CDynamicLearningEngine();
    virtual inline CDynamicGraphicalModel* GetDynamicModel() const;
#ifdef PNL_OBSOLETE
    virtual void SetData( int NumberOfTimeSeries, int *NumberOfSlicesIn, 
        const CEvidence* const* const* pEvidencesIn );
#endif
    virtual void SetData(const pEvidencesVecVector& evidencesIn );
    virtual void Learn()=0;

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CDynamicLearningEngine::m_TypeInfo;
  }
#endif
protected:
    CDynamicLearningEngine(CDynamicGraphicalModel *pGrModel, ELearningTypes LearnType);
    intVector* GetObservationsFlags(){return &m_FlagsIsAllObserved;}
    int m_numberOfAllEv0Slice;
    int m_numberOfAllEvISlice;
    pnlVector< pConstEvidenceVector * > m_VecPVecPEvidences;
    
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    ELearningTypes m_LearnType;
    intVector m_FlagsIsAllObserved;
};

#ifndef SWIG
inline  CDynamicGraphicalModel* CDynamicLearningEngine::GetDynamicModel() const
{
	return static_cast<CDynamicGraphicalModel*> (m_pGrModel);
}
#endif


PNL_END

#endif // __PNLDynamicLEARNINGENGINE_HPP__
