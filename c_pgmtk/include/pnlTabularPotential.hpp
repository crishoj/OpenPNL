/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlTabularPotential.hpp                                     //
//                                                                         //
//  Purpose:   CTabularPotential class definition                          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// TabularPotential.hpp: interface for the TabularFactor class.

///////////////////////////////////////////////////////////////////////////////

#ifndef __PNLTABULARPOTENTIAL_HPP__
#define __PNLTABULARPOTENTIAL_HPP__

#include "pnlParConfig.hpp"
#include "pnlPotential.hpp"
//#include "TabularData.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN
#ifdef SWIG
%rename(CreateUnitF) CTabularPotential::CreateUnitFunctionDistribution( const intVector& , CModelDomain* , int , const intVector& ); 
#endif


class PNL_API CTabularPotential : public CPotential  
{
public:
#ifndef SWIG
    static CTabularPotential* Create( const intVector& domain,
                                      CModelDomain* pMD,
                                      const float* data,
                                 const intVector& obsIndicesIn = intVector() );
#endif
    static CTabularPotential* Create( CModelDomain* pMD, const intVector& domain,
                                      CMatrix<float>* data = NULL );


    static CTabularPotential* CreateUnitFunctionDistribution( 
                                const intVector& domain, CModelDomain* pMD,
                                int asDense = 1,
                                const intVector& obsIndicesIn = intVector() ); 
#ifdef PNL_OBSOLETE
    static CTabularPotential* Create( const int *domain, int nNodes,
                                      CModelDomain* pMD,
                                      const float* data = NULL,
                                 const intVector& obsIndicesIn = intVector() );
    static CTabularPotential* CreateUnitFunctionDistribution( const int *domain,
                                      int nNodes, CModelDomain* pMD,
                                      int asDense = 1,
                                 const intVector& obsIndicesIn = intVector() );
#endif //PNL_OBSOLETE
	static CTabularPotential* Copy( const CTabularPotential *pTabPotential );
    virtual CFactor* Clone() const;
    virtual CFactor* CloneWithSharedMatrices();
	float ProcessingStatisticalData(int numberOfEvidences);
	void UpdateStatisticsEM(const CPotential *pInfData, 
        const CEvidence *pEvidence = NULL );
    void UpdateStatisticsML(const pConstEvidenceVector& evidencesIn );
#ifdef PNL_OBSOLETE
	void UpdateStatisticsML(const CEvidence* const* pEvidencesIn,
        int EvidenceNumber);
#endif //PNL_OBSOLETE
	virtual void Dump()const;
    virtual void GenerateSample( CEvidence* evidenceIO, int maximize = 0 ) const;
    virtual CPotential* ConvertStatisticToPot(int numOfSamples) const;
    virtual float GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes = NULL ) const;
    

	~CTabularPotential(){}

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
      return CTabularPotential::m_TypeInfo;
    }
#endif
protected:
	CTabularPotential( const int *domain, int nNodes, CModelDomain* pMD,
        const intVector& obsIndicesIn = intVector() );
	CTabularPotential( const CTabularPotential &pTabPotential );
    CTabularPotential( const CTabularPotential* pTabPot );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
};

PNL_END

#endif // __PNLTABULARPOTENTIAL_HPP__
