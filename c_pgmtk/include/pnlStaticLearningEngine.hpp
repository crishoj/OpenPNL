/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlStaticLearningEngine.hpp                                 //
//                                                                         //
//  Purpose:   The base class for all learning                             //
//             algorithms for static graphical models                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLSTATICLEARNINGENGINE_HPP__
#define __PNLSTATICLEARNINGENGINE_HPP__

#include "pnlLearningEngine.hpp"
#include "pnlStaticGraphicalModel.hpp"
#include "pnlEvidence.hpp"
#include "pnlInferenceEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CStaticLearningEngine : public CLearningEngine
{
public:
    
    virtual ~CStaticLearningEngine();
    
#ifdef PNL_OBSOLETE
    virtual void SetData( int size, const CEvidence* const* pEvidencesIn );	
    virtual void AppendData( int size, const CEvidence* const* pEvidencesIn );
#endif
    
    virtual void SetData( const pConstEvidenceVector& evidencesIn );
    virtual void AppendData( const pConstEvidenceVector& evidencesIn );
    //virtual void ClearStatisticData();
    virtual void Learn()=0;
    void SetMaxIterIPF(int max_iter = 10);
    void SetTerminationToleranceIPF(float precision = 0.01);
    virtual inline CStaticGraphicalModel* GetStaticModel() const;
    inline int GetNumEv() const;

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CStaticLearningEngine::m_TypeInfo;
  }
#endif
protected:
    CStaticLearningEngine(CStaticGraphicalModel *pGrModel, ELearningTypes LearnType);
    
    pConstEvidenceVector m_Vector_pEvidences;
    int m_numberOfAllEvidences;
    
    int m_maxIterIPF;
    float m_precisionIPF;
	
	void BuildFullEvidenceMatrix(float ***full_evid);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
};


#ifndef SWIG
inline CStaticGraphicalModel* CStaticLearningEngine::GetStaticModel() const
{
	return static_cast<CStaticGraphicalModel*> (m_pGrModel);
}

inline int CStaticLearningEngine::GetNumEv() const
{
    return m_Vector_pEvidences.size();
}
#endif

PNL_END

#endif // __PNLSTATICLEARNINGENGINE_HPP__
