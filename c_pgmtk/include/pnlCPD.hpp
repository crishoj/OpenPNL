/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlCPD.hpp                                                  //
//                                                                         //
//  Purpose:   CCPD class definition                                       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// CPD.hpp: interface for the CCPD class.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __PNLCPD_H__
#define __PNLCPD_H__

#include "pnlPotential.hpp"
#include "pnlFactor.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API CCPD : public CFactor  
{
public:
    virtual CPotential *ConvertToPotential() const = 0;
    virtual CFactor* Clone() const = 0;
    virtual CFactor* CloneWithSharedMatrices() = 0;
    virtual void NormalizeCPD() = 0;
    virtual CPotential* ConvertWithEvidenceToPotential(
        const CEvidence* pEvidence,
        int flagSumOnMixtureNode = 1)const; 
    virtual ~CCPD(){}
    virtual float ProcessingStatisticalData(int numberOfEvidences ) = 0;
    virtual void  GenerateSample( CEvidence* evidencesIO, int maximize = 0 ) const = 0;
    virtual void UpdateStatisticsEM(const CPotential *pMargPot,
        const CEvidence *pEvidence = NULL ) = 0;
    virtual float GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes = NULL ) const = 0;
    
#ifdef PNL_OBSOLETE	
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidencesIn, 
        int EvidenceNumber) = 0;
#endif

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CCPD::m_TypeInfo;
    }
#endif
protected:
    CCPD( EDistributionType dt,
        EFactorType pt, const int *domain, int nNodes, CModelDomain* pMD );
    CCPD( EDistributionType dt, EFactorType ft, CModelDomain* pMD );
    CCPD( const CCPD* cpd );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
};

PNL_END

#endif // __PNLCPD_H__

