/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLearningEngine.hpp                                       //
//                                                                         //
//  Purpose:   The base class for all learning algorithms                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLLEARNINGENGINE_HPP__
#define __PNLLEARNINGENGINE_HPP__

#include "pnlObject.hpp"
#include "pnlGraphicalModel.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

typedef PNL_API enum
{
    itParamLearnML,
    itParamLearnEM,
    itParamLearnBayes,
    itStructLearnBIC,
    itDBNParamLearnEm,
    itStructLearnML,
    itDBNStructLearnML,
	itStructLearnSEM
} ELearningTypes;

class PNL_API CLearningEngine : public CPNLBase
{
public:
    virtual ~CLearningEngine();
    virtual void Learn()=0;
    
#ifdef PNL_OBSOLETE
    virtual void GetCriterionValue( int *nstepsOut, const float **scoreOut ) const;
#endif
    void ClearStatisticData();
    virtual void GetCriterionValue( floatVector *critValuesOut ) const;

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CLearningEngine::m_TypeInfo;
  }
#endif
protected:
    CLearningEngine( CGraphicalModel *pGrModel, ELearningTypes learnType );
    bool IsInfNeed( const CEvidence * pEvidences );
    static void CheckModelValidity(const CGraphicalModel *pGrModel);
    
    CGraphicalModel *m_pGrModel;
    ELearningTypes m_LearnType;
    floatVector m_critValue;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};



PNL_END

#endif // __PNLLEARNINGENGINE_HPP__
