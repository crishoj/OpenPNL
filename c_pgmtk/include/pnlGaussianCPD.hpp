/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGaussianCPD.hpp                                          //
//                                                                         //
//  Purpose:   CGaussianCPD class definition                               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//GaussianCPD.hpp
///////////////////////////////////////////////////////////////////////////////
#ifndef __PNLGAUSSIANCPD__HPP__
#define __PNLGAUSSIANCPD__HPP__

#include "pnlParConfig.hpp"
#include "pnlCPD.hpp"
#include "pnlTypeDefs.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN
#ifdef SWIG
%rename(CreateUnitF) CGaussianCPD::CreateUnitFunctionCPD( const intVector& domainIn, CModelDomain* pMD);
#endif


class PNL_API CGaussianCPD : public CCPD
{
public:
    static CGaussianCPD* Create( const intVector& domainIn, CModelDomain* pMD );
    static CGaussianCPD* Copy( const CGaussianCPD* pGauCPD );
    static CGaussianCPD* CreateUnitFunctionCPD( const intVector& domainIn,
	CModelDomain* pMD);
    virtual CFactor* Clone() const;
    virtual CFactor* CloneWithSharedMatrices();
#ifndef SWIG
    void AllocDistribution( const floatVector& meanIn, const floatVector& covIn,
	float normCoeff,
	const floatVecVector& weightsIn,
	const intVector& parentCombination = intVector());
#endif
    void AllocDistribution( const C2DNumericDenseMatrix<float>* meanMat,
		            const C2DNumericDenseMatrix<float>* covMat, float normCoeff,
		            const p2DDenseMatrixVector& weightsMat,
		            const intVector& parentCombination  = intVector());
    void SetCoefficientVec( float coeff, const intVector& parentCombinationIn 
	= intVector() );
    float GetCoefficientVec( const intVector& parentCombinationIn = intVector() );
#ifdef PNL_OBSOLETE
    static CGaussianCPD* Create( const int *domain, int nNodes,
	CModelDomain* pMD );
    static CGaussianCPD* CreateUnitFunctionCPD( const int *domain, int nNodes,
	CModelDomain* pMD );
    void AllocDistribution( const float* pMean, const float* pCov,
        float normCoeff, const float* const* pWeightsIn,
        const int* parentCombination = NULL );
    void SetCoefficient( float coeff, const int* parentCombination = NULL );
    float GetCoefficient( const int* parentCombination = NULL  );
#endif //PNL_OBSOLETE
    virtual ~CGaussianCPD(){}
    virtual CPotential *ConvertToPotential() const;
    virtual CPotential* ConvertWithEvidenceToPotential( const CEvidence* pEvidence,
        int flagSumOnMixtureNode = 1)const;
    virtual void NormalizeCPD();
    virtual float ProcessingStatisticalData( int numberOfEvidences);
    virtual void UpdateStatisticsEM(const CPotential *pMargPot, 
        const CEvidence *pEvidence = NULL);
    virtual void GenerateSample( CEvidence* evidenceIO, int maximize = 0 ) const;
    virtual CPotential* ConvertStatisticToPot(int numOfSamples) const;
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
      return CGaussianCPD::m_TypeInfo;
    }
#endif

protected:
    CGaussianCPD( const int *domain, int nNodes, CModelDomain* pMD );
    CGaussianCPD( const CGaussianCPD& GauCPD );
    CGaussianCPD( const CGaussianCPD* pGauCPD );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    
};

PNL_END

#endif// __PNLGAUSSIANCPD__HPP__
