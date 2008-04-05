/**
 * @file pnlpFunctionalPotential.hpp
 * @brief Declaration of the CFunctionalPotential class.
 * 
 * This header file declares the CFunctionalPotential class.
 * 
 * Derived from pnlGaussianPotential.hpp.
 * 
 * See also: pnlpFunctionalPotential.cpp
 * 
 * TODO: Full conversion
 */

#ifndef __PNLPFUNCTIONALPOTENTIAL_HPP__
#define __PNLPFUNCTIONALPOTENTIAL_HPP__

// PNL
#include "pnlParConfig.hpp"
#include "pnlPotential.hpp"

// PNL+
#include "pnlpDistributionType.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

#ifdef SWIG
%rename(CreateUnitF) CGaussianPotential::CreateUnitFunctionDistribution(const intVector&, CModelDomain*,int, const intVector& );
%rename(CreateDeltaF) CGaussianPotential::CreateDeltaFunction( const intVector&, CModelDomain*, const C2DNumericDenseMatrix<float>*, int, const intVector&);
#endif

/**
 * @brief CFunctionalPotential class
 * 
 */
class PNL_API CFunctionalPotential : public CPotential
{
public:
#ifndef SWIG
    static CFunctionalPotential* Create( const intVector& domainIn,
            CModelDomain* pMD,
            int inMoment = -1,
            const floatVector& VecIn = floatVector(),
            const floatVector& MatIn = floatVector(),
            float normCoeff = 0.0f,
            const intVector& obsIndicesIn = intVector());
#endif
    static CFunctionalPotential* Create( CModelDomain* pMD,const intVector& domainIn,
            int inMoment = -1,
            const C2DNumericDenseMatrix<float> *meanIn = NULL,
            const C2DNumericDenseMatrix<float> *covIn = NULL,
            float normCoeff = 0.0f,
            const intVector& obsIndicesIn = intVector());

#ifdef PNL_OBSOLETE
    static CFunctionalPotential* Create( const int *domain, int nNodes,
            CModelDomain* pMD,
            int inMoment = -1, float const* pVec = NULL,
            float const* pMat = NULL,
            float normCoeff = 0.0f,
            const intVector& obsIndicesIn = intVector());
#endif
    static CFunctionalPotential* Copy( const CFunctionalPotential *pGauPot );
#ifndef SWIG
    static CFunctionalPotential* CreateDeltaFunction( const intVector& domainIn,
            CModelDomain* pMD,
            const floatVector& meanIn ,
            int isInMoment = 1,
            const intVector& obsIndicesIn = intVector());
#endif
    static CFunctionalPotential* CreateDeltaFunction( const intVector& domainIn,
            CModelDomain* pMD,
            const C2DNumericDenseMatrix<float>* meanIn,
            int isInMoment = 1,
            const intVector& obsIndicesIn = intVector());
    static CFunctionalPotential* CreateUnitFunctionDistribution(
            const intVector& domainIn,
            CModelDomain* pMD,
            int isInCanonical = 1,
            const intVector& obsIndicesIn = intVector());
    virtual CFactor* Clone() const;
    virtual CFactor* CloneWithSharedMatrices();
#ifdef PNL_OBSOLETE
    static CFunctionalPotential* CreateDeltaFunction( const int *domain,
            int nNodes, CModelDomain* pMD, const float *pMean,
            int isInMoment = 1,
            const intVector& obsIndicesIn = intVector());
    static CFunctionalPotential* CreateUnitFunctionDistribution(
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

    ~CFunctionalPotential()
    {}

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
        return CFunctionalPotential::m_TypeInfo;
    }
#endif
protected:
    CFunctionalPotential( const int *domain, int nNodes, CModelDomain* pMD,
            const intVector& obsIndicesIn = intVector() );
    CFunctionalPotential( const CFunctionalPotential &pGauPot );
    CFunctionalPotential( const CFunctionalPotential* pGauPot );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
};

PNL_END

#endif // __PNLPFUNCTIONALPOTENTIAL_HPP__
