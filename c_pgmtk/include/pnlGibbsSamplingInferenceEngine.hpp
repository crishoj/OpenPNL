/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGibbsSamplingInferenceEngine.hpp                         //
//                                                                         //
//  Purpose:   CGibbsSamplingInfEngine class definition                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLGIBBSSAMPLINGINFERENCEENGINE_HPP__
#define __PNLGIBBSSAMPLINGINFERENCEENGINE_HPP__

#include "pnlSamplingInferenceEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CGibbsSamplingInfEngine : public CSamplingInfEngine
{
public:
    
    
    static CGibbsSamplingInfEngine* Create( const CStaticGraphicalModel 
        *pGraphicalModelIn );
    
    virtual ~CGibbsSamplingInfEngine();
    
    // main public member-functions, which carry out the engine work
    virtual void EnterEvidence( const CEvidence *pEvidenceIn, int maximize = 0,
        int sumOnMixtureNode = 1 );
#ifdef PNL_OBSOLETE
    virtual void              MarginalNodes( const int *query, int querySize,
        int notExpandJPD = 0 );
#endif
    
    virtual void MarginalNodes( const intVector& queryNdsIn, int notExpandJPD = 0 );
    
    
    void SetQueries(intVecVector &queryes);
    const CPotential* GetQueryJPD() const;
    virtual const CEvidence* GetMPE() const;
    // destructor
    void UseDSeparation( bool isUsing);

    inline void SetNSamplesForSoftMax(int nSamples);
    inline int GetNSamplesForSoftMax() {return m_NSamplesForSoftMax;};

    inline void SetMaxNSamplesForSoftMax(int mNSamples);
    inline int GetMaxNSamplesForSoftMax() {return m_MaxNSamplesForSoftMax;};
    
#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CGibbsSamplingInfEngine::m_TypeInfo;
    }
#endif

protected:
    
    CGibbsSamplingInfEngine( const CStaticGraphicalModel *pGraphicalModel );
    
    
    
    virtual void Sampling(int t1, int t2);
    void CreateQueryFactors();
    virtual void Initialization();
    void GetQueryFactors( pFactorVector *pFactors );
    pFactorVector * GetQueryFactors( );
    void DestroyQueryFactors();
    void ConsDSep(intVecVector &queryes, boolVector *resultNds, const CEvidence *pEv ) const;

    inline intVecVector& GetQueryes();

    inline bool GetSignOfUsingDSeparation() const;

    bool ConvertingFamilyToPot( int node, const CEvidence* pEv );
   
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
    
    CPotential              *m_pPotMPE;
    intVector               m_queryNodes;
    intVecVector            m_queryes;
    pFactorVector           m_queryFactors;
    bool                    m_bUsingDSep;

    //
    int m_NSamplesForSoftMax;

    //
    int m_MaxNSamplesForSoftMax;

    pFactorVector m_SoftMaxGaussianFactors;
    intVecVector m_environment;       
};

inline void CGibbsSamplingInfEngine::GetQueryFactors( pFactorVector *pFactors )
{
    pFactors->assign(m_queryFactors.begin(), m_queryFactors.end());
}

inline pFactorVector * CGibbsSamplingInfEngine::GetQueryFactors( )
{
    return &m_queryFactors;
}

inline intVecVector& CGibbsSamplingInfEngine::GetQueryes()
{
    return m_queryes;
}

inline bool CGibbsSamplingInfEngine::GetSignOfUsingDSeparation() const
{
    return m_bUsingDSep;
}

inline void CGibbsSamplingInfEngine::SetNSamplesForSoftMax(int nSamples) 
{
  PNL_CHECK_LEFT_BORDER(nSamples, 1);

  m_NSamplesForSoftMax = nSamples;
}

inline void CGibbsSamplingInfEngine::SetMaxNSamplesForSoftMax(int mNSamples) 
{
  PNL_CHECK_LEFT_BORDER(mNSamples, 1);

  m_MaxNSamplesForSoftMax = mNSamples;
}

PNL_END

#endif //__PNLGIBBSSAMPLINGINFERENCEENGINE_HPP__
