/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlTreeCPD.hpp                                              //
//                                                                         //
//  Purpose:   CTreeCPD class definition                                   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//GaussianCPD.hpp
///////////////////////////////////////////////////////////////////////////////
#ifndef __PNLTREECPD__HPP__
#define __PNLTREECPD__HPP__

#include "pnlParConfig.hpp"
#include "pnlCPD.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlGraph.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API CTreeCPD : public CCPD
{
public:
    static  CTreeCPD* Create( const intVector& domainIn, CModelDomain* pMD );
    static  CTreeCPD* Create( const int *domain, int nNodes, CModelDomain* pMD );
    static  CTreeCPD* Copy( const CTreeCPD* pTreeCPD );
	void UpdateTree(const CGraph *pGraphTree, TreeNodeFields *fields );
    virtual CFactor* Clone() const;
    virtual CFactor* CloneWithSharedMatrices();
    virtual ~CTreeCPD(){}
    virtual CPotential* ConvertToPotential() const;
	virtual CPotential* ConvertWithEvidenceToPotential(
        const CEvidence* pEvidence,
        int flagSumOnMixtureNode = 1)const; 
    virtual void NormalizeCPD();
    
    virtual float ProcessingStatisticalData( int numberOfEvidences);
    virtual void UpdateStatisticsEM(const CPotential *pMargPot, 
	const CEvidence *pEvidence = NULL);
    virtual void GenerateSample( CEvidence* evidenceIO, int maximize = 0 ) const;
    virtual CPotential* ConvertStatisticToPot(int numOfSamples) const;
    virtual float GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes = NULL ) const;
#ifdef PNL_OBSOLETE
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidencesIn,
	int EvidenceNumber);
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
      return CTreeCPD::m_TypeInfo;
    }
#endif

protected:
    CTreeCPD( const int *domain, int nNodes, CModelDomain* pMD );
    CTreeCPD( const CTreeCPD& TreeCPD );
    CTreeCPD( const CTreeCPD* pTreeCPD );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    
};

PNL_END

#endif// __PNLTREECPD__HPP__
