/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGibbsWithAnnealingInferenceEngine.hpp                    //
//                                                                         //
//  Purpose:   CGibbsWithAnnealingInfEngine class definition               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLGIBBSWITHANNEALINGINFERENCEENGINE_HPP__
#define __PNLGIBBSWITHANNEALINGINFERENCEENGINE_HPP__

#include "pnlSamplingInferenceEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API CGibbsWithAnnealingInfEngine : public CSamplingInfEngine
{
public:
    
    
    static CGibbsWithAnnealingInfEngine* Create( const CStaticGraphicalModel 
        *pGraphicalModelIn );    
    virtual ~CGibbsWithAnnealingInfEngine();
    
    // main public member-functions, which carry out the engine work
    virtual void EnterEvidence( const CEvidence *pEvidenceIn, int maximize = 0,
        int sumOnMixtureNode = 1 );
    virtual const CEvidence* GetMPE() const;
    void SetAnnealingCoefficientC(float val);
    void SetAnnealingCoefficientS(float val);
    void UseAdaptation(bool isUse );
    
#ifdef PNL_OBSOLETE
    virtual void MarginalNodes( const int *query, int querySize, int notExpandJPD = 0 );
#endif
    virtual void MarginalNodes( const intVector& queryNdsIn, int notExpandJPD = 0 );

   
    float GetCurrentTemp() const;

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CGibbsWithAnnealingInfEngine::m_TypeInfo;
  }
#endif
protected:
    
    CGibbsWithAnnealingInfEngine( const CStaticGraphicalModel *pGraphicalModel );
    
    void SetCurrentTemp( float T);
    
    virtual void Initialization();
    
    virtual void Sampling(int t1, int t2);
    
    void Annealing(CFactor *pFactor, float T, const CEvidence *pEv = NULL );  
    
    void AnnealingModel( float T, const CEvidence *pEv = NULL, const char* doAnneling = NULL);
    
    virtual void DestroyInf();
    
    virtual void CreateSamplingPotentials( potsPVector* potsToSampling );
    
    float Temperature( float C, float S ) const;
    
    
    CEvidence * BestEvidence( pEvidencesVector &evidences);
    
    bool IsEvidencesEqual(const CEvidence* pEv1, const CEvidence* pEv2, float eps = 0.01f) const;
    bool IsVlsEqual( valueVector &vls11, valueVector &vls2, float eps = 0.01f ) const;
    
    void FillVlsVector(pEvidencesVector &pEv);
    void DestroyCurrentFactors();
    bool IsAdapt() const;
    float GetLikelihood(const CEvidence* pEv);
    
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    
    CEvidence *m_pBestEvidence;
    
    float m_constC;
    float m_constS;

    bool m_bAdapt;
    int m_counter;

    float m_currentTemp;
    
    valueVecVector m_oldVls;
};



PNL_END

#endif 
