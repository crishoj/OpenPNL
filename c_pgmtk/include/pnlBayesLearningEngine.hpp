/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlBayesLearningEngine.hpp                                  //
//                                                                         //
//  Purpose:   Class definition for the factor learning algorithm for   //
//             BNet that uses prior distribution of factors             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLBAYESLEARNINGENGINE_HPP__
#define __PNLBAYESLEARNINGENGINE_HPP__

#include "pnlStaticLearningEngine.hpp" 

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN
                     
//currently CBayesLearningEngine can work only 
//with discrete multinomial distributions (they have tabular CPD)
class PNL_API CBayesLearningEngine : public CStaticLearningEngine  
{
public:
	~CBayesLearningEngine();
    
    //used instead of constructor
    static CBayesLearningEngine* Create(CStaticGraphicalModel *pGrModelIn);
    
#ifndef SWIG

    //Bayesian factor update allows for successive learning
    //so probably SetData is not necessary here
    virtual void SetData( int size, const CEvidence* const* pEvidencesIn );	
    virtual void AppendData( int size, const CEvidence* const* pEvidencesIn );
#endif

    //run learning engine
	void Learn();
    int GetCPDUpdatedStatus();
	
#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CBayesLearningEngine::m_TypeInfo;
  }
#endif
protected:
	int m_numberOfLearnedEvidences;
    int m_updateCPD;

    //hidden constructor
    CBayesLearningEngine( CStaticGraphicalModel *pGrModel );

#ifdef PNL_RTTI
  static const CPNLType m_TypeInfo;
#endif 
};

PNL_END

#endif //__PNLBAYESLEARNINGENGINE_HPP__
