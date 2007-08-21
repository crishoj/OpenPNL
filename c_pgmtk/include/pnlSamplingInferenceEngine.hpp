/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlSamplingInferenceEngine.hpp                              //
//                                                                         //
//  Purpose:   CSamplingInfEngine class definition                         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#ifndef __PNLSAMPLINGINFERENCEENGINE_HPP__
#define __PNLSAMPLINGINFERENCEENGINE_HPP__

#include "pnlInferenceEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API CSamplingInfEngine : public CInfEngine
{
public:
    
    
    //void SetParemeter( eGibbsFactors param, int x);
    
    // main public member-functions, which carry out the engine work
    virtual void EnterEvidence( const CEvidence *pEvidenceIn, int maximize = 0,
        int sumOnMixtureNode = 1 ) = 0;
#ifdef PNL_OBSOLETE
    virtual void              MarginalNodes( const int *query, int querySize,
        int notExpandJPD = 0 ) = 0;
#endif
    
    virtual void MarginalNodes( const intVector& queryNdsIn, int notExpandJPD = 0 ) = 0;
    
    
    
    
    inline void SetMaxTime(int time);
    inline void SetBurnIn( int time);
    inline void SetNumStreams( int nStreams );
    
    inline int GetMaxTime() const;
    inline int GetBurnIn() const;
    inline int GetNumStreams() const;
    
    virtual ~CSamplingInfEngine();
    virtual void Continue( int dt );

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CSamplingInfEngine::m_TypeInfo;
  }
#endif
protected:
    
    CSamplingInfEngine( const CStaticGraphicalModel *pGraphicalModel );
    
    
    virtual void Initialization() = 0;
    virtual void Sampling( int t1, int t2 ) = 0;
    
    
    void DestroyInf();
    void Normalization(CPotential *pot);
    
    void CreateSamplingPotentials( potsPVector* potsToSampling );
    void FindEnvironment(intVecVector *chlds);
    bool ConvertingFamilyToPot( int node, const CEvidence* pEv );
    
    void FindNdsForSampling( intVector* nds, intVector* mixtureNds );
    void GetNdsForSampling( intVector* nds ) const;
    void GetCurrentFactors( pFactorVector* pFactors);
    inline pFactorVector* GetCurrentFactors();
    void GetCurrentEvidences( pEvidencesVector* pEv );
    
    pEvidencesVector* GetCurrentEvidences();
    void GetPotsToSampling( potsPVector* pPots );
    
    int GetNumStreams();
    void GetObsDimsWithVls(intVector &domain, int nonObsNode, const CEvidence* pEv,
        intVector *dims, intVector *vls) const;
    bool CheckIsAllNdsTab() const;
    bool IsAllNdsTab() const;
    void FindCurrentNdsForSampling(boolVector *flags) const;
    inline CPotential* GetPotToSampling(int i) const;
    inline potsPVector* GetPotsToSampling();
    void DestroyCurrentEvidences();
    void SetSamplingNdsFlags(boolVector &flags);
    void GetSamplingNdsFlags(boolVector* flags) const;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif     
private:
    
    intVector m_ndsForSampling;
    intVector m_mixtureNds;
    potsPVector m_potsToSampling;
    pFactorVector m_currentFactors;
    pEvidencesVector m_pEvidences;
    
    intVecVector m_environment;
    
    
    int m_maxTime;
    int m_barnin;
    int m_nStreams;
    
    bool m_allNdsIsTab;
    boolVector m_sampleIsNeed;

    //float m_constC;
    
    valueVecVector m_oldVls;
};

#ifndef SWIG
inline void CSamplingInfEngine::GetSamplingNdsFlags( boolVector* flags ) const
{
    flags->assign( m_sampleIsNeed.begin(), m_sampleIsNeed.end() );
}
inline void CSamplingInfEngine::SetSamplingNdsFlags( boolVector& flags )
{
    m_sampleIsNeed.assign( flags.begin(), flags.end() );
}
inline CPotential* CSamplingInfEngine::GetPotToSampling(int i) const
{
    return m_potsToSampling[i];
}
inline int CSamplingInfEngine::GetMaxTime() const
{ 
    return m_maxTime;
}
inline void CSamplingInfEngine::SetMaxTime( int time )
{
    PNL_CHECK_LEFT_BORDER( time, GetBurnIn() );
    m_maxTime = time;
}
inline int CSamplingInfEngine::GetBurnIn() const
{ 
    return m_barnin;
}
inline void CSamplingInfEngine::SetBurnIn( int time )
{
   PNL_CHECK_RANGES( time, 0, GetMaxTime() );
    m_barnin = time;
}
inline void CSamplingInfEngine::SetNumStreams( int nStreams)
{
    PNL_CHECK_LEFT_BORDER( nStreams, 0 );
    m_nStreams = nStreams;
}
inline int CSamplingInfEngine::GetNumStreams() const
{ 
    return m_nStreams;
}
inline pFactorVector* CSamplingInfEngine::GetCurrentFactors()
{
    return &m_currentFactors;
}
inline potsPVector* CSamplingInfEngine::GetPotsToSampling()
{
    return &m_potsToSampling;
}

inline void CSamplingInfEngine::GetNdsForSampling( intVector* nds ) const
{
    nds->assign( m_ndsForSampling.begin(), m_ndsForSampling.end() );
}

inline void CSamplingInfEngine::
GetCurrentEvidences( pEvidencesVector* pEv )
{
    pEv->assign(m_pEvidences.begin(), m_pEvidences.end() );
}

inline void CSamplingInfEngine::
GetCurrentFactors( pFactorVector* pFactors)
{
    pFactors->assign( m_currentFactors.begin(), m_currentFactors.end() );
}

inline int CSamplingInfEngine::GetNumStreams()
{
    return m_nStreams;
}

inline bool CSamplingInfEngine::IsAllNdsTab() const
{
    return m_allNdsIsTab;
}

inline pEvidencesVector * CSamplingInfEngine::GetCurrentEvidences() 
{
    return &m_pEvidences;
}


#endif // SWIG

PNL_END

#endif // include guard 
