/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlTabularCPD.hpp                                           //
//                                                                         //
//  Purpose:   CTabularCPD class definition                                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//TabularCPD.hpp
///////////////////////////////////////////////////////////////////////////////
#ifndef __PNLTABULARCPD__HPP__
#define __PNLTABULARCPD__HPP__

#include "pnlParConfig.hpp"
#include "pnlCPD.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

#ifdef SWIG
%rename(CreateUnitF) CTabularCPD::CreateUnitFunctionCPD( const intVector& domainIn, CModelDomain* pMD);
#endif

class PNL_API CTabularCPD : public CCPD
{
public:
    static CTabularCPD* Copy( const CTabularCPD* pTabCPD );
#ifndef SWIG
    static CTabularCPD* Create( const intVector& domainIn, CModelDomain* pMD,
	const floatVector& dataIn = floatVector() );
#endif
    static CTabularCPD* Create( CModelDomain* pMD, const intVector& domainIn, 
	CMatrix<float>* dataIn = NULL);
    

    static CTabularCPD* CreateUnitFunctionCPD( const intVector& domainIn,
	CModelDomain* pMD);

    float GetMatrixValue(const CEvidence *pEv);

    virtual void CreateAllNecessaryMatrices(int typeOfMatrices = 1);
    //typeOfMatrices = 1 - all matrices are random
    //only Gaussian covariance matrix is matrix unit
    //for ConditionalGaussianDistribution 
    //the matrix of Gaussian distribution functions is dense

#ifdef PNL_OBSOLETE
    static CTabularCPD* Create( const int *domain, int nNodes,
	CModelDomain* pMD, const float *data = NULL );
    static CTabularCPD* CreateUnitFunctionCPD( const int *domain, int nNodes,
	CModelDomain* pMD );
#endif
    virtual CFactor* Clone() const;
    virtual CFactor* CloneWithSharedMatrices();
    virtual CPotential *ConvertToPotential() const;
    virtual ~CTabularCPD(){}
    virtual void NormalizeCPD();
    virtual float ProcessingStatisticalData(int numberOfEvidences );
    virtual void UpdateStatisticsEM(const CPotential *pMargPot, 
        const CEvidence *pEvidence = NULL );
    virtual void GenerateSample( CEvidence* evidenceIO, int maximize = 0  ) const;
    virtual CPotential* ConvertStatisticToPot(int numOfSamples) const;
#ifdef PNL_OBSOLETE
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidencesIn,
        int EvidenceNumber);
#endif
    virtual float GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes = NULL ) const;
    
    bool IsValid(std::string* descriptionOut = NULL) const;

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
      return CTabularCPD::m_TypeInfo;
    }
#endif
protected:
    CTabularCPD( const int *domain, int nNodes, CModelDomain* pMD );
    CTabularCPD( const CTabularCPD& TabCPD );
    CTabularCPD( const CTabularCPD* pTabCPD );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
};

PNL_END

#endif// __PNLTABULARCPD__HPP__

