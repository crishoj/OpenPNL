/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlcondgaussiandistribfun.hpp                               //
//                                                                         //
//  Purpose:   CCondGaussianDistrib class definition                       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// CondGaussianData.hpp: interface for the CondGaussianData class.
//
/////////////////////////////////////////////////////////////////////////////// 
#ifndef __PNLCONDGAUSSIANDISTRIBFUN_HPP__
#define __PNLCONDGAUSSIANDISTRIBFUN_HPP__

#include "pnlParConfig.hpp"
#include "pnlDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlDenseMatrix.hpp"
#include "pnlNumericDenseMatrix.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN


class PNL_API CCondGaussianDistribFun : public CDistribFun
{
public:
    static CCondGaussianDistribFun* Create( int isFactor, int nNodes,
        const CNodeType *const* nodeTypes,
        int asDenseMatrix = 1,
        CGaussianDistribFun* const pDefaultDistr = NULL );
    static CCondGaussianDistribFun* Copy( 
        const CCondGaussianDistribFun* pInputDistr );
    //change only data for two distributions of the same size & type
    virtual CDistribFun& operator=( const CDistribFun& pInputDistr );
    virtual CDistribFun* Clone() const;
    //make copy by attaching matrices
    //instead of copy them as in copy constructor
    virtual CDistribFun* CloneWithSharedMatrices();
    //set Gaussain distribution for indicated discrete parents combination
    virtual void CreateDefaultMatrices( int typeOfMatrices = 1 );
    //typeOfMatrices == 1 - randomly created matrices
    //creates Gaussian distribution in moment form
    //create dense matrix with Gaussian distributions for conditional Gaussian
    void SetDistribFun( CGaussianDistribFun* const inputDistr, 
        const int* discreteParentCombination );
    //we can create distribution functions without asking about their contens
    //and alloc or attach matrices after that
    void AllocDistribFun( const int* discreteParentCombination, 
        int isMoment = 1, int isDelta = 0, int isUniform = 0 );
    const CGaussianDistribFun* GetDistribution( const int* discrParentIndex )const;
    //check validity of distribution - is all Gaussians here
    virtual bool IsValid(std::string* descriptionOut = NULL) const;
    //allocate specified type of matrix to indicated parents combination
    virtual void AllocMatrix( const float *data, EMatrixType mType, 
        int numberOfWeightMatrix = -1,
        const int *parentIndices = NULL);
    //attach specified type of matrix to indicated parents combination
    virtual void AttachMatrix( CMatrix<float> *pMatrix,
				EMatrixType mType,
                                int numberOfWeightMatrix = -1,
                                const int *parentIndices = NULL,
                                bool isMultipliedByDelta = 0);
    //set coeeficient for Gaussian distribution for input parent combination
    void SetCoefficient( float coeff, int isCanonical,
        const int* pParentCombination );
    //get the value of normalization constant
    float GetCoefficient( int isCanonical, const int* pParentCombination );
    //return pointer to specified type of matrix 
    //from indicated discrete parents combination
    virtual CMatrix<float>* GetMatrix( EMatrixType mType, 
        int numWeightMat = -1,
        const int *parentIndices = NULL ) const;
    //check is the distribution delta or Unit function
    virtual inline int IsDistributionSpecific() const;
    
    virtual CMatrix<float> *GetStatisticalMatrix( EStatisticalMatrix mType,
        int *parentIndices = NULL ) const;
    //arithmetic operations
    virtual void MarginalizeData( const CDistribFun *pOldData,
        const int *DimsOfKeep, int NumDimsOfKeep,
        int maximize );
        /*virtual void MultiplyData( const int *pBigDomain, const int *pSmallDomain,
        const CDistribFun *pBigData,
    const CDistribFun *pSmallData);*/
    virtual void MultiplyInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData );
    virtual void SumInSelfData(const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData);
    virtual void DivideInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData );
        /*	virtual void DivideData( const int *pBigDomain, const int *pSmallDomain,
        const CDistribFun *pBigData,
    const CDistribFun *pSmallData);*/
    //methods to reduce volume of computations
    virtual void ShrinkObservedNodes( const CDistribFun* pOldData, 
        const int *pDimsOfObserved, 
        const Value* const* pObsValues, int numObsDim,
        const CNodeType* pObsTabNT, const CNodeType* pObsGauNT);
    virtual void ExpandData( const int* pDimsToExtend, 
        int numDimsToExpand, const Value* const* valuesArray, 
        const CNodeType* const *allFullNodeTypes,
        int UpdateCanonical = 1 );
    //methods for learning
    virtual void ClearStatisticalData();
     virtual void SetStatistics( const CMatrix<float> *pMat, 
	EStatisticalMatrix matrix, const int* parentsComb = NULL );

    virtual void UpdateStatisticsEM( const CDistribFun* infData, 
        const CEvidence *pEvidence = NULL, float weightingCoeff = 1.0f, const int* domain = NULL );
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidences, int EvidenceNumber,
        const int *domain, float weightingCoeff = 1.0f );
    virtual float ProcessingStatisticalData( float numEvidences ) ;
    
    //return result of multiplication by special functions
    virtual int GetMultipliedDelta( const int **positions, const float **values, 
        const int **offsets ) const;	
    //method used for inference
    virtual CDistribFun *ConvertCPDDistribFunToPot()const;
    //methods used by Pearl inference
    virtual CDistribFun *CPD_to_pi( CDistribFun *const* allPiMessages, 
        int *multParentIndices, int numMultNodes, int posOfExceptParent, 
        int maximizeFlag = 0 ) const;
    virtual CDistribFun *CPD_to_lambda( const CDistribFun *lambda, 
        CDistribFun *const* allPiMessages, int *multParentIndices,
        int numMultNodes, int posOfExceptNode, int maximizeFlag = 0 )const;	
    //method to avoid numerical instability
    virtual CDistribFun* GetNormalized()const;
    virtual void Normalize();
    //compare DistribFun
    virtual int IsEqual( const CDistribFun *dataToCompare, float epsilon, 
        int withCoeff = 1, float* maxDifferenceOut = NULL ) const;
    //return most probable explanation
    virtual CNodeValues* GetMPE();
    
    //methods to convert distribution with dense matrices to distribution with sparse
    //if its already sparse - return copy
    virtual CDistribFun* ConvertToSparse() const;
    
    //methods to convert distribution with sparse matrices to distribution with dense
    //if its already dense - return copy
    virtual CDistribFun* ConvertToDense() const;
    
    virtual int IsSparse() const;
    virtual int IsDense() const;

    void Dump() const;
    
    //enter discrete evidence to the CPD
    CCondGaussianDistribFun* EnterDiscreteEvidence( int nDiscrObsNodes,
        const int* discrObsNodes, const int *discrValues,
        const CNodeType* pObsTabNodeType ) const;
    CTabularDistribFun* EnterFullContinuousEvidence( int nContObsParents, 
        const int* contObsParentsIndices,
        const C2DNumericDenseMatrix<float>* obsChildValue,
        C2DNumericDenseMatrix<float>* const* obsValues,
        const CNodeType* pObsGauNodeType ) const;
    inline void GetDiscreteParentsIndices( intVector* const discrParents ) const;
    inline void GetContinuousParentsIndices( intVector* const contParents ) const;
    inline CMatrix<CGaussianDistribFun*>* GetMatrixWithDistribution();
    inline CDenseMatrix<float>* GetMatrixNumEvidences();
    virtual ~CCondGaussianDistribFun();

    virtual int GetNumberOfFreeParameters() const;
    virtual void ResetNodeTypes(pConstNodeTypeVector &nodeTypes);


#ifdef PAR_PNL
    virtual void UpdateStatisticsML(CDistribFun *pPot);
#endif

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CCondGaussianDistribFun::m_TypeInfo;
    }
#endif

protected:
    CCondGaussianDistribFun( int isFactor, int nNodes,
        const CNodeType *const* nodeTypes, int asDenseMatrix,
        CGaussianDistribFun* const pDefaultDistr );
    CCondGaussianDistribFun( const CCondGaussianDistribFun & inpDistr );
    CCondGaussianDistribFun( int isFactor, int nNodes,
        const CNodeType* const* pNodeTypes,
        CMatrix<CGaussianDistribFun*> const* pDistribMatrix  );
    void GetNormExp(doubleVector &data, floatVector *normExpData) const;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif

private:
    int m_numberOfDims;
    
    int m_bPotential;
    
    //we divide parents to discrete & continuous 
    //- corresponding indices keep here
    intVector m_contParentsIndex;
    intVector m_discrParentsIndex;
    
    //this matrix contain pointers
    //to GaussianDistribFun on Gaussian child & Gaussian parents
    //which corresponds discrete parent combination
    //order of discrete parents in such matrix corresponds such order in nodeTypes
    //and domain (for corresponding factor)
    CMatrix<CGaussianDistribFun*>* m_distribution;
    
    //matrix of number of evidence lines corresponding discrete parent combination
    CDenseMatrix <float>* m_numEvidencesLearned;
    
};

inline int CCondGaussianDistribFun::IsDistributionSpecific() const
{
    return 0;
}

inline void CCondGaussianDistribFun::GetDiscreteParentsIndices( 
                                                                intVector* const discrParents ) const
{
    PNL_CHECK_IS_NULL_POINTER( discrParents );
    
    discrParents->assign( m_discrParentsIndex.begin(),
        m_discrParentsIndex.end());
}

inline void CCondGaussianDistribFun::GetContinuousParentsIndices( 
                                                                  intVector* const contParents ) const 
{
    PNL_CHECK_IS_NULL_POINTER( contParents );
    
    contParents->assign( m_contParentsIndex.begin(), m_contParentsIndex.end());
}

inline CMatrix<CGaussianDistribFun*>* CCondGaussianDistribFun::GetMatrixWithDistribution()
{
    return m_distribution;
}

inline CDenseMatrix<float>* CCondGaussianDistribFun::GetMatrixNumEvidences() 
{
    return m_numEvidencesLearned;
}

PNL_END

#endif //__PNLCONDGAUSSIANDISTRIBFUN_HPP__
