/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlmixturegaussiancpd.hpp                                   //
//                                                                         //
//  Purpose:   CMixtureGaussianCPD class definition                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//MixtureGaussianCPD.hpp
///////////////////////////////////////////////////////////////////////////////
#ifndef __PNLMIXTUREGAUSSIANCPD__HPP__
#define __PNLMIXTUREGAUSSIANCPD__HPP__

#include "pnlParConfig.hpp"
#include "pnlCPD.hpp"
#include "pnlTypeDefs.hpp"
//#include "pnlCondGaussianDistribFun.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class CCondGaussianDistribFun;

class PNL_API CMixtureGaussianCPD : public CCPD
{
public:
    //mixture CPD have one special discrete node - mixture node, 
    //it is the last node in discrete part of domain (if it isn't so exception throws),
    //the Gaussian distributions corresponding to its values sum up
    //and give resulting continuous child distribution
    static CMixtureGaussianCPD* Create( const intVector& domainIn,
                CModelDomain* pMD, const floatVector& probabilitiesIn );
    static CMixtureGaussianCPD* Copy( const CMixtureGaussianCPD* pMGauCPD );
    virtual CFactor* Clone() const;
    virtual CFactor* CloneWithSharedMatrices();
#ifndef SWIG
    void AllocDistributionVec(const floatVector& meanIn, const floatVector& covIn,
                            float normCoeff, const floatVecVector& weightsIn,
                            const intVector& discrComb );
#endif
    void AllocDistributionVec( const C2DNumericDenseMatrix<float>* meanMat,
		            const C2DNumericDenseMatrix<float>* covMat, float normCoeff,
		            const p2DDenseMatrixVector& weightsMat,
		            const intVector& discrComb);
    void SetCoefficientVec( float coeff, const intVector& discrComb );
    float GetCoefficientVec( const intVector& discrComb );
#ifdef PNL_OBSOLETE
	static CMixtureGaussianCPD* Create( const int *domain, int nNodes,
                                 CModelDomain* pMD, const float* probabilities );
    void AllocDistribution( const float* pMean, const float* pCov,
        float normCoeff, const float* const* pWeightsIn,
        const int* discrParentComb );
    void SetCoefficient( float coeff, const int* discrComb  );
    float GetCoefficient( const int* discrComb );
    int GetNumberOfMixtureNode() const;
#endif //PNL_OBSOLETE
    virtual ~CMixtureGaussianCPD(){}
    virtual CPotential *ConvertToPotential() const;
    virtual CPotential* ConvertWithEvidenceToPotential( 
        const CEvidence* pEvidence, int flagSumOnMixtureNode = 1 )const;
    virtual void NormalizeCPD();
    virtual float ProcessingStatisticalData( int numberOfEvidences);
    virtual void UpdateStatisticsEM(const CPotential *pMargPot, 
        const CEvidence *pEvidence = NULL );
    virtual void GenerateSample( CEvidence* evidencesIO, int maximize = 0 ) const;
    virtual CPotential* ConvertStatisticToPot(int numOfSamples) const;
    inline void GetProbabilities( floatVector *proprobabilities ) const;
    virtual float GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes = NULL ) const;
    
    
#ifdef PNL_OBSOLETE
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidencesIn, int EvidenceNumber);
#endif

#ifdef PAR_PNL
    virtual void UpdateStatisticsML(CFactor *pPot);
#endif

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CMixtureGaussianCPD::m_TypeInfo;
    }
#endif
protected:
    CMixtureGaussianCPD( const int *domain, int nNodes, CModelDomain* pMD,
	const float* probabilities );
    CMixtureGaussianCPD( const CMixtureGaussianCPD& GauCPD );
    CMixtureGaussianCPD( const CMixtureGaussianCPD* pGauCPD );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    floatVector m_probabilities;
    floatVector m_learnProbabilities;
};

inline void CMixtureGaussianCPD::GetProbabilities( floatVector *proprobabilities ) const
{
    PNL_CHECK_IS_NULL_POINTER(proprobabilities);
    proprobabilities->clear();
    (*proprobabilities).assign(m_probabilities.begin(), m_probabilities.end());
}

PNL_END

#endif// __PNLMIXTUREGAUSSIANCPD__HPP__
