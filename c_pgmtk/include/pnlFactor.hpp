/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlFactor.hpp                                               //
//                                                                         //
//  Purpose:   CFactor class definition                                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// Factor.hpp: interface for the Factor class.

#ifndef __PNLFACTOR_HPP__
#define __PNLFACTOR_HPP__

#include "pnlParConfig.hpp"
#include "pnlObject.hpp"
#include "pnlNodeType.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlNumericDenseMatrix.hpp"
#include "pnlNumericSparseMatrix.hpp"
//#include "pnlModelDomain.hpp"

#include <float.h>

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

//fixme - enum use for Param&Diatribution - should they be here?
typedef PNL_API enum
{
    dtTabular,
        dtGaussian,
        dtCondGaussian,
        dtUnitFunction,
        dtMixGaussian,
        dtScalar,
        dtTree,
        dtInvalid,
        dtSoftMax,
        dtCondSoftMax
} EDistributionType;

typedef PNL_API enum
{
    itUsing,
        itNotUsing
} EUsingPrior;

typedef PNL_API enum
{   ftPotential
,   ftCPD
,   ftIDPotential
}  EFactorType;

bool PNL_API pnlIsSubset(int smaSize, int* smaDomain, int bigSize, int* bigDomain);
intVector PNL_API pnlIntersect(int size1, int* Domain1, int size2, int* Domain2);
intVector PNL_API pnlSetUnion(int size1, int* Domain1, int size2, int* Domain2);
bool PNL_API pnlIsIdentical(int size1, int* Domain1, int size2, int* Domain2);

class CDistribFun;
class CTabularDistribFun ;
class CPotential;
class CEvidence;
class CInfEngine;
class CModelDomain;

class PNL_API CFactor : public CPNLBase
{
public:
    //create new factor in other domain and model domain if node types corresponds
    static CFactor* CopyWithNewDomain(const CFactor *factor, intVector &domain,
        CModelDomain *pModelDomain,
        const intVector& obsIndices = intVector());
    virtual CFactor* Clone() const = 0;

    virtual CFactor* CloneWithSharedMatrices() = 0;

    virtual void CreateAllNecessaryMatrices(int typeOfMatrices = 1);
    //typeOfMatrices = 1 - all matrices are random
    //only Gaussian covariance matrix is matrix unit
    //for ConditionalGaussianDistribution
    //the matrix of Gaussian distribution functions is dense

    //methods for work with Model Domain
    //return factor number in factor heap
    int GetNumInHeap() const;
    //release model domain from this factor
    void ChangeOwnerToGraphicalModel() const;
    //check if the Model is owned by Model Domain
    bool IsOwnedByModelDomain() const;
    //return pointer to Model Domain
    inline CModelDomain* GetModelDomain() const;

    CFactor & operator =( const CFactor & pInputFactor );
    inline EFactorType GetFactorType() const;

    inline EDistributionType GetDistributionType() const;

#ifndef SWIG
    const pConstNodeTypeVector *GetArgType() const;
#endif

    CMatrix<float> *GetMatrix( EMatrixType mType,
        int numberOfMatrix = -1, const int* discrParentValuesIndices = NULL ) const;

    //methods to convert factor with dense matrices to factor with sparse
    //if its already sparse - return copy
    void ConvertToSparse();

    //methods to convert factor with sparse matrices to factor with dense
    //if its already dense - return copy
    void ConvertToDense();

    int IsDense() const;
    int IsSparse() const;

    inline size_t GetDomainSize() const;

    inline void GetObsPositions( intVector* obsPosOut ) const;

    inline CDistribFun* GetDistribFun() const;

    virtual bool IsValid(std::string* descriptionOut = NULL) const;

    int IsFactorsDistribFunEqual(const CFactor *param, float eps,
        int withCoeff = 1, float* maxDifferenceOut = NULL) const;
    void GetDomain( intVector* domainOut ) const;
#ifdef PNL_OBSOLETE
    void GetDomain(int *DomainSizeOut, const int **domainOut ) const;
#endif
    //  void Clamp(int clumpingType);

    void SetDistribFun(const CDistribFun* data);

    void TieDistribFun( CFactor *parameter );

    void AllocMatrix( const float*data, EMatrixType mType,
        int numberOfMatrix = -1, const int *discrParentValuesIndices = NULL );

    void AttachMatrix( CMatrix<float> *matrix, EMatrixType mType,
        int numberOfMatrix = -1, const int* discrParentValuesIndices = NULL );

    int IsDistributionSpecific()const;
    //if 0 - full distribution (non delta, non uniform, non mixed, can haven't valid form - use GetFlagMoment, GetFlagCanonical)
    //if 1 - uniform distribution - have no matrices, only size
    //if 2 - delta function - have only Mean matrix
    //if 3 - mixed distribution - result of multiplication of some full distribution
    //by delta function on some of its dimensions
    void MakeUnitFunction();

    virtual void GenerateSample( CEvidence* evidencesIO, int maximize = 0  ) const = 0;
    virtual CPotential* ConvertStatisticToPot(int numOfSamples) const = 0;

    virtual void SetStatistics( const CMatrix<float> *pMat,
  EStatisticalMatrix matrix, const int* parentsComb = NULL );
    virtual float ProcessingStatisticalData(int numberOfEvidences) = 0;

    virtual void UpdateStatisticsEM(const CPotential *pMargPot,
           const CEvidence *pEvidence = NULL) = 0;
    virtual void UpdateStatisticsML( const pConstEvidenceVector& evidencesIn );

    virtual float GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes = NULL ) const = 0;

  int GetNumberOfFreeParameters()const;

#ifdef PNL_OBSOLETE
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidencesIn,
        int EvidenceNumber) = 0;
#endif
    inline int AreThereAnyObsPositions() const;
    ~CFactor();

#ifdef PAR_PNL
    virtual void UpdateStatisticsML(CFactor *pPot) = 0;
#endif

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CFactor::m_TypeInfo;
    }
#endif
protected:

    CFactor( EDistributionType dt, EFactorType pt,
        const int *domain, int nNodes, CModelDomain* pMD,
        const intVector& obsIndicesIn = intVector() );
    CFactor( EDistributionType dt, EFactorType ft, CModelDomain* pMD );
    //this means copy constructor with clone of matrices - doesn't copy them
    CFactor( const CFactor* factor );

    EFactorType m_FactorType;
    /*One of types Factor, CPD, JPD etc*/
    EDistributionType m_DistributionType;
    /*One of types dtNoisyOr,dtGaussian,dtTabular,dtUnif,dtRand,dtCG*/

    CDistribFun *m_CorrespDistribFun;

    intVector m_Domain;

    intVector m_obsPositions;

    //change model domain and corresponding node types in factor
    void SetModelDomain(CModelDomain* pMD, bool checkNodeTypesinMD = 1);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    // a pointer to a model domain
    CModelDomain *m_pMD;

    // a number of the factor in the heap
    int m_factNumInHeap;



};

#ifndef SWIG

inline EFactorType CFactor::GetFactorType()const
{
    return m_FactorType;
}
inline EDistributionType CFactor::GetDistributionType()const
{
    return m_DistributionType;
}

inline size_t CFactor::GetDomainSize() const
{
    return m_Domain.size();
}

inline CDistribFun* CFactor::GetDistribFun() const
{
    return m_CorrespDistribFun;
}

inline CModelDomain* CFactor::GetModelDomain() const
{
    return m_pMD;
}

inline int CFactor::AreThereAnyObsPositions() const
{
    return (!m_obsPositions.empty());
}

inline void CFactor::GetObsPositions( intVector* obsPosOut )const
{
    PNL_CHECK_IS_NULL_POINTER( obsPosOut );

    obsPosOut->assign(m_obsPositions.begin(), m_obsPositions.end());
}

#endif

PNL_END

#endif //__PNLFACTOR_HPP__
