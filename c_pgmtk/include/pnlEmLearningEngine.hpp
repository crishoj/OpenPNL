/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlEmLearningEngine.hpp                                     //
//                                                                         //
//  Purpose:   Class definition for EM factor learning algorithm        //
//             for BNet                                                    //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLEMLEARNINGENGINE_HPP__
#define __PNLEMLEARNINGENGINE_HPP__

#include "pnlStaticLearningEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CEMLearningEngine : public CStaticLearningEngine
{
public:
    static CEMLearningEngine* Create(CStaticGraphicalModel *pGrModelIn, CInfEngine *pInfEng = NULL );

    virtual ~CEMLearningEngine();

    virtual void Learn();

    void SetMaxIterEM(int Niter = 30);
    inline int GetMaxIterEM() const;

    void SetTerminationToleranceEM(float precision = 0.001f);
    inline float GetPrecisionEM() const;

#ifndef SWIG
    virtual void AppendData(int dim, const CEvidence* const* pEvidences);
#endif

    void SetMaximizingMethod(EMaximizingMethod met);
    void Cast(const CPotential * pot, int node, int ev, float *** full_evid);
    EMaximizingMethod GetMaximizingMethod();

	const float* GetFamilyLogLik()const;  
	void LearnExtraCPDs(int nMaxFamily, pCPDVector* additionalCPDs, floatVector* additionalLLs);

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CEMLearningEngine::m_TypeInfo;
  }
#endif
protected:
    CEMLearningEngine(CStaticGraphicalModel *pGrModel, CInfEngine *pInfEng,
  ELearningTypes LearnType );
    float _LearnPotentials();
    bool IsDomainObserved( int nnodes, const int* domain, int evNumber );
    inline const boolVector * GetObsFlags( int evNumber ) const;
    inline int GetNumberProcEv() const;
    void SetNumProcEv( int numEv );
    inline bool IsAllObserved() const;
    float UpdateModel();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    CInfEngine *m_pInfEngine;
    bool m_bDelInf;

    boolVecVector m_obsFlags;
    bool m_bAllObserved;
    int m_numProcEv;

    int m_maxIterEM;
    float m_precisionEM;
    EMaximizingMethod m_MaximizingMethod;

	floatVector m_vFamilyLogLik; 

};

#ifndef SWIG

inline bool CEMLearningEngine::IsAllObserved() const
{
    return m_bAllObserved;
}

inline int CEMLearningEngine::GetMaxIterEM() const
{
    return m_maxIterEM;
}

inline float CEMLearningEngine::GetPrecisionEM() const
{
    return m_precisionEM;

}

inline const boolVector * CEMLearningEngine::GetObsFlags( int evNumber ) const
{
    return &(m_obsFlags[evNumber]);
}

inline int CEMLearningEngine::GetNumberProcEv() const
{
    return m_numProcEv;
}

#endif

PNL_END

#endif //__PNLEMLEARNINGENGINE_HPP__
