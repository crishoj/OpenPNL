/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlParGibbsSamplingInferenceEngine.hpp                      //
//                                                                         //
//  Purpose:   CParGibbsSamplingInfEngine class definition                 //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLPARGIBBSSAMPLINGINFERENCEENGINE_HPP__
#define __PNLPARGIBBSSAMPLINGINFERENCEENGINE_HPP__
 
#include "pnlParConfig.hpp"

//#undef PAR_PNL

#ifdef PAR_PNL

#include "pnlGibbsSamplingInferenceEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif
PNL_BEGIN

class PNL_API CParGibbsSamplingInfEngine : public CGibbsSamplingInfEngine
{
public:

    static CParGibbsSamplingInfEngine* Create(const CStaticGraphicalModel 
        *pGraphicalModelIn);

    virtual void MarginalNodes(const int *queryIn, int querySz, 
        int notExpandJPD = 0);
    // it's absolutely necessary to call this method from each MPI process

    void EnterEvidence( const CEvidence *pEvidenceIn, int maximize = 0, 
        int sumOnMixtureNode = 1 );

    virtual ~CParGibbsSamplingInfEngine();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CParGibbsSamplingInfEngine::m_TypeInfo;
  }
#endif
protected:

    CParGibbsSamplingInfEngine(const CStaticGraphicalModel 
        *pGraphicalModel);

    void Sampling( int statTime, int endTime );

    void CreateSamplingPotentials( potsPVector* potsToSampling );

    bool ConvertingFamilyToPot( int node, const CEvidence* pEv );

    void CreateQueryFactors();

    virtual void Initialization();
    
    //Each cont. parent of tabular child has got an element in m_SoftMaxGaussianFactorsForOMP
    //It contains a potential for statistic inference (see ConvertingFamilyToPot function)
    pFactorVector m_SoftMaxGaussianFactorsForOMP;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:

    int m_MyRank;                       // process number
    int m_NumberOfProcesses;            // max amount of processes

    int m_NumberOfThreads;
    int m_NumberOfNodesForSample;

    int m_currentTime;

    intVecVector m_environment;       

    // init consts from mpi framework (in case MPI) or default (without MPI)
    void InitIPCConsts();

    // send my potential to the process 0
    void SendMyPotentialViaMpi();

    // recv potentials from other processes 
    void RecvPotentialsViaMpi(intVector *pNumberOfSamples,
        floatVecVector *pProbDistrib);

    int GetFactorNumber(intVector queryIn);

    void SendMyGaussianStatisticsViaMpi(int NumberOfFactor);

    void RecvGaussianStatisticsViaMpi(intVector *pNumberOfSamples, 
        floatVecVector *pLearnMean, floatVecVector *pLearnCov, 
        int NumberOfFactor);



};

PNL_END

#endif // PAR_PNL

#endif // __PNLPARGIBBSSAMPLINGINFERENCEENGINE_HPP__
