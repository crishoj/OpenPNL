/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlScalarPotential.hpp                                     //
//                                                                         //
//  Purpose:   CScalarPotential class definition                          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// ScalarPotential.hpp: interface for the ScalarFactor class.

///////////////////////////////////////////////////////////////////////////////

#ifndef __PNLSCALARPOTENTIAL_HPP__
#define __PNLSCALARPOTENTIAL_HPP__

#include "pnlParConfig.hpp"
#include "pnlPotential.hpp"
//#include "ScalarData.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CScalarPotential : public CPotential  
{
public:
    static CScalarPotential* Create( const intVector& domain,
                        CModelDomain* pMD,
                        const intVector& obsIndicesIn = intVector() );
#ifdef PNL_OBSOLETE
    static CScalarPotential* Create( const int *domain, int nNodes,
                         CModelDomain* pMD,
                         const intVector& obsIndicesIn = intVector());
#endif //PNL_OBSOLETE
	static CScalarPotential* Copy( const CScalarPotential *pScalarPotential );
    virtual CFactor* Clone() const;
    CFactor* CloneWithSharedMatrices();
	float ProcessingStatisticalData(int numberOfEvidences);
	void UpdateStatisticsEM(const CPotential *pMargJPD, 
        const CEvidence *pEvidence = NULL );
    void UpdateStatisticsML( 
        const pConstEvidenceVector& evidencesIn );
#ifdef PNL_OBSOLETE
	void UpdateStatisticsML(const CEvidence* const* pEvidencesIn,
        int EvidenceNumber);
#endif //PNL_OBSOLETE
	virtual void Dump()const;
    virtual void GenerateSample( CEvidence* evidenceIO, int maximize = 0 ) const;
    virtual CPotential* ConvertStatisticToPot(int numOfSamples) const;
    virtual float GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes = NULL ) const;
    

	~CScalarPotential(){}

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
      return CScalarPotential::m_TypeInfo;
    }
#endif
protected:
	CScalarPotential( const int *domain, int nNodes, CModelDomain* pMD,
        const intVector& obsIndicesIn = intVector());
	CScalarPotential( const CScalarPotential &pScalarPotential );
    CScalarPotential( const CScalarPotential* pScalarPotential );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
};

PNL_END

#endif // __PNLSCALARPOTENTIAL_HPP__
