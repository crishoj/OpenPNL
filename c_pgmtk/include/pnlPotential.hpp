/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPotential.hpp                                            //
//                                                                         //
//  Purpose:   CPotential class definition                                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// Potential.hpp: interface for the Factor class.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __PNLPOTENTIAL_HPP__
#define __PNLPOTENTIAL_HPP__

#include "pnlFactor.hpp"
#include "pnlLog.hpp"

//#include "pnlJPD.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

PNL_API CPotential* pnlMultiply(CPotential* Pot1, CPotential* Pot2, CModelDomain* pMD);

class PNL_API CPotential : public CFactor  
{
public:
    virtual CPotential * Multiply( const CPotential *pOtherFactor ) const;
    virtual CFactor* Clone() const = 0;
    CPotential& operator *=( const CPotential &pSmallPotential );
    CPotential& operator /=( const CPotential &pSmallPotential );
    virtual CFactor* CloneWithSharedMatrices() = 0;
    /*This function has different implementation for TabularFactor, 
    GaussianFactor etc*/
    //normalize & return new normalized potential
    virtual CPotential* GetNormalized() const;
    //normalize self
    void Normalize();
    /*This function has its own version for every type of factor*/
    //	virtual CFactor *Marginalize(int *pSmallDom,
    //		int domSize, int maximize)=0;
    /*This function has implementation for TabularFactor,
    GaussianFactor etc*/
    CPotential* Marginalize( const intVector& smallDomainIn,
        int maximize = 0 ) const;
    void MarginalizeInPlace( const CPotential* pOldPot,
        const int* corrPositions = NULL,  int maximize = 0);
    CPotential* ShrinkObservedNodes( const CEvidence* pEvidence ) const;
    CEvidence* GetMPE() const;
    
#ifdef PNL_OBSOLETE
    CPotential *Marginalize( const int *pSmallDom, int domSize, 
								int maximize = 0 ) const ;
#endif
    CPotential *ExpandObservedNodes ( const CEvidence *evid,
        int updateCanonical = 1) const;
    void GetMultipliedDelta( intVector* positionsOut, floatVector* valuesOut,
        intVector* offsetsOut ) const;
#ifdef PNL_OBSOLETE
    int GetMultiplyedDelta( const int **positionsOut, const float **valuesOut, 
        const int **offsetsOut ) const;
#endif
    virtual CPotential * Divide(const CPotential *pOtherFactor ) const;
    virtual void GenerateSample( CEvidence* evidencesIO, int maximize = 0  ) const = 0;
    virtual float ProcessingStatisticalData( int nnumberOfEvidences) = 0;
    virtual void UpdateStatisticsEM(const CPotential *pMargPot, 
        const CEvidence *pEvidence = NULL)=0;
#ifdef PNL_OBSOLETE
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidencesIn, int EvidenceNumber) = 0;
#endif
    virtual void Dump()const = 0;
    virtual float GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes = NULL ) const = 0;
    
    
    virtual ~CPotential(){}
#ifndef SWIG
    static Log& dump();
#endif
    static void SetDump(const char *fileName);
    
#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CPotential::m_TypeInfo;
    }
#endif
protected:
    CPotential( EDistributionType dt,
        EFactorType pt,	const int *domain, int nNodes, CModelDomain* pMD,
        const intVector& obsIndicesIn = intVector() );	
    CPotential( EDistributionType dt, EFactorType ft, CModelDomain* pMD );
    CPotential( const CPotential* potential );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    
};

PNL_END

#endif //__PNLPOTENTIAL_H__
