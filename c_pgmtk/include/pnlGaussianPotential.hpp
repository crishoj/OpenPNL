/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGaussianPotential.hpp                                    //
//                                                                         //
//  Purpose:   CGaussianPotential class definition                         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// GaussianPotential.hpp: interface for the GaussianFactor class.

///////////////////////////////////////////////////////////////////////////////

#ifndef __PNLGAUSSIANPOTENTIAL_HPP__
#define __PNLGAUSSIANPOTENTIAL_HPP__

#include "pnlParConfig.hpp"
#include "pnlPotential.hpp"
//#include "GaussianData.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN
#ifdef SWIG
%rename(CreateUnitF) CGaussianPotential::CreateUnitFunctionDistribution(const intVector&, CModelDomain*,int, const intVector& ); 
%rename(CreateDeltaF) CGaussianPotential::CreateDeltaFunction( const intVector&, CModelDomain*, const C2DNumericDenseMatrix<float>*, int, const intVector&);
#endif


class PNL_API CGaussianPotential : public CPotential  
{
public:
#ifndef SWIG
    static CGaussianPotential* Create( const intVector& domainIn,
                                       CModelDomain* pMD,
                                       int inMoment = -1,
                                       const floatVector& VecIn = floatVector(),
                                       const floatVector& MatIn = floatVector(),
                                       float normCoeff = 0.0f,
                                       const intVector& obsIndicesIn = intVector());
#endif
    static CGaussianPotential* Create( CModelDomain* pMD,const intVector& domainIn,
	int inMoment = -1,
	const C2DNumericDenseMatrix<float> *meanIn = NULL,
	const C2DNumericDenseMatrix<float> *covIn = NULL,
	float normCoeff = 0.0f,
	const intVector& obsIndicesIn = intVector());
    
    
#ifdef PNL_OBSOLETE
	static CGaussianPotential* Create( const int *domain, int nNodes,
                                    CModelDomain* pMD,
                                    int inMoment = -1, float const* pVec = NULL,
                                    float const* pMat = NULL, 
                                    float normCoeff = 0.0f,
                                    const intVector& obsIndicesIn = intVector());
#endif
	static CGaussianPotential* Copy( const CGaussianPotential *pGauPot );
#ifndef SWIG
	static CGaussianPotential* CreateDeltaFunction( const intVector& domainIn,
                                       CModelDomain* pMD,
                                       const floatVector& meanIn ,
				       int isInMoment = 1,
                                       const intVector& obsIndicesIn = intVector());
#endif
	static CGaussianPotential* CreateDeltaFunction( const intVector& domainIn,
                                       CModelDomain* pMD,
                                       const C2DNumericDenseMatrix<float>* meanIn,
				       int isInMoment = 1,
                                       const intVector& obsIndicesIn = intVector());
	static CGaussianPotential* CreateUnitFunctionDistribution( 
                                const intVector& domainIn, 
                                CModelDomain* pMD,
                                int isInCanonical = 1,
                                const intVector& obsIndicesIn = intVector()); 
    virtual CFactor* Clone() const;
    virtual CFactor* CloneWithSharedMatrices();
#ifdef PNL_OBSOLETE
	static CGaussianPotential* CreateDeltaFunction( const int *domain,
                    int nNodes, CModelDomain* pMD, const float *pMean,
                    int isInMoment = 1,
                    const intVector& obsIndicesIn = intVector());
	static CGaussianPotential* CreateUnitFunctionDistribution( 
                    const int *domain, int nNodes,CModelDomain* pMD,
                    int isInCanonical = 1,
                    const intVector& obsIndicesIn = intVector());
#endif    
    virtual float ProcessingStatisticalData( int nnumberOfEvidences);
	virtual void UpdateStatisticsEM(const CPotential *pMargPot, 
        const CEvidence *pEvidence = NULL);
    virtual void UpdateStatisticsML( const pConstEvidenceVector& evidencesIn );
#ifdef PNL_OBSOLETE
	virtual void UpdateStatisticsML(const CEvidence* const* pEvidencesIn, 
        int EvidenceNumber);
#endif
	virtual void Dump()const;
    void SetCoefficient( float coeff, int isForCanonical );
	float GetCoefficient( int forCanonical );
    virtual void GenerateSample( CEvidence* evidenceIO, int maximize = 0) const;
    virtual CPotential* ConvertStatisticToPot(int numOfSamples) const;
    virtual float GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes = NULL ) const;
    
    ~CGaussianPotential(){}

#ifdef PAR_PNL
    void UpdateStatisticsML(CFactor *pPot);
#endif

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CGaussianPotential::m_TypeInfo;
    }
#endif
protected:
	CGaussianPotential( const int *domain, int nNodes, CModelDomain* pMD,
        const intVector& obsIndicesIn = intVector() );
	CGaussianPotential( const CGaussianPotential &pGauPot );
    CGaussianPotential( const CGaussianPotential* pGauPot );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
};

PNL_END

#endif // __PNLGAUSSIANPOTENTIAL_HPP__
