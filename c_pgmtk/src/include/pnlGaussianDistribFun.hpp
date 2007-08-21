/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGaussianDistribFun.hpp                                   //
//                                                                         //
//  Purpose:   CGaussianDistribFun class definition                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// GaussianData.hpp: interface for the GaussianData class.
//
/////////////////////////////////////////////////////////////////////////////// 
#ifndef __PNLGAUSSIANDISTRIBFUN_HPP__
#define __PNLGAUSSIANDISTRIBFUN_HPP__

#include "pnlParConfig.hpp"
#include "pnlDistribFun.hpp"
//#include "pnl2DNumericDenseMatrix.hpp"
//#include "pnl2DMatrix.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

template <class Type> class C2DNumericDenseMatrix;

class PNL_API CGaussianDistribFun : public CDistribFun
{
public:
    static CGaussianDistribFun* CreateInMomentForm(int isPotential,
        int NumberOfNodes,
        const CNodeType *const* NodeTypes,
        const float *dataMean,
        const float *dataCov, 
        const float **dataWeight = NULL);
    static CGaussianDistribFun* CreateInCanonicalForm(int NumberOfNodes,
        const CNodeType *const* NodeTypes,
        const float *dataH,
        const float *dataK, float g = 0.0f);
    static CGaussianDistribFun* CreateDeltaDistribution(int NumberOfNodes,
        const CNodeType *const* nodeTypes,
        const float *dataMean, int isMoment = 1, int isPotential = 1);
    static CGaussianDistribFun* CreateUnitFunctionDistribution( int NumberOfNodes,
        const CNodeType *const*nodeTypes, int isPotential = 1,
        int isCanonical = 1);
    static CGaussianDistribFun* Copy( const CGaussianDistribFun* pInpDistr );
    
    virtual void CreateDefaultMatrices( int typeOfMatrices = 1 );
    //typeOfMatrices == 1 - randomly created matrices
    //creates Gaussian distribution in moment form
    
    CDistribFun& operator=( const CDistribFun& pInputDistr );
    
    virtual CDistribFun* Clone() const;
    virtual CDistribFun* CloneWithSharedMatrices();
    virtual void AllocMatrix( const float *data, EMatrixType mType, 
	int numberOfWeightMatrix = -1,
	const int *parentIndices = NULL);
    virtual void AttachMatrix( CMatrix<float> *pMatrix,
	EMatrixType mType,
	int numberOfWeightMatrix = -1,
	const int *parentIndices = NULL,
	bool isMultipliedByDelta = 0);
    virtual bool IsValid(std::string* descriptionOut = NULL) const;
    int CheckMomentFormValidity();
    int CheckCanonialFormValidity();
    /*This function add ReferenceCounter at this MultiDMatrix*/
    inline int GetCanonicalFormFlag() const;
    inline int GetMomentFormFlag() const;
    void SetCoefficient( float  coeff, int isG = 1 );
    void UpdateCanonicalCoefficient();
    
    int GetMultipliedDelta( const int **positions, const float **values, 
        const int **offsets ) const;
    void UpdateMomentForm();
    void UpdateCanonicalForm();
    CDistribFun *ConvertCPDDistribFunToPot()const ;
    CDistribFun *CPD_to_pi( CDistribFun *const* allPiMessages, 
        int *multParentIndices, int numMultNodes, int posOfExceptParent = -1, 
        int maximizeFlag = 0 )const ;
    CDistribFun *CPD_to_lambda( const CDistribFun *lambda, 
        CDistribFun *const* allPiMessages, int *multParentIndices,
        int numMultNodes, int posOfExceptNode = -1, int maximizeFlag = 0 )const ;	
    //multParentIndices[i] = j <=> corresponds j's parent in domain for CPD 
    //for which we need to multiply
    //allPiMessages[i] corresponds pi message from i's parent in domain for CPD
    //lambda is message from last node in domain for CPD - child node
    inline int GetFactorFlag() const;
    virtual inline int IsDistributionSpecific() const;
    //if 0 - full distribution (non delta, non uniform, non mixed, can haven't valid form - use GetFlagMoment, GetFlagCanonical)
    //if 1 - uniform distribution - have no matrices, only size
    //if 2 - delta function - have only Mean matrix
    //if 3 - mixed distribution - result of multiplication of some full distribution
    //by delta function on some of its dimensions
    virtual void  MarginalizeData( const CDistribFun* pOldData, 
        const int *DimOfKeep, 
        int NumDimsOfKeep, int maximize );
    void ShrinkObservedNodes( const CDistribFun* pOldData,
        const int *pVarsObserved, 
        const Value* const* pObsValues, int numObsVars,
        const CNodeType* pObsTabNT, const CNodeType* pObsGauNT ) ;
    void ExpandData( const int* pDimsToExtend, 
        int numDimsToExpand, const Value* const* valuesArray, 
        const CNodeType* const*allFullNodeTypes, int UpdateCanonical = 1 );
        /*	virtual void MultiplyData( const int *pBigDomain, const int *pSmallDomain,
								const CDistribFun *pBigData,
    const CDistribFun *pSmallData) ;*/
    virtual void MultiplyInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData );
    virtual void SumInSelfData(const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData);
    virtual void DivideInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData ) ;
        /*	virtual void DivideData( const int *pDomain1, const int *pDomain2,
								const CDistribFun *pData1,
    const CDistribFun *pData2);*/
    virtual CMatrix<float> *GetMatrix( EMatrixType mType, 
        int numWeightMat = -1,
        const int *parentIndices = NULL) const;
    inline float GetCoefficient( int isCanonical ) const;
    virtual CMatrix<float> *GetStatisticalMatrix( EStatisticalMatrix mType, 
        int *parentIndices = NULL ) const;
    virtual void SetStatistics( const CMatrix<float> *pMat, 
	EStatisticalMatrix matrix, const int* parentsComb = NULL );
    virtual void UpdateStatisticsEM( const CDistribFun* infData, 
        const CEvidence *pEvidence = NULL, float weightingCoeff = 1.0f,
        const int* domain = NULL);
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidences, int EvidenceNumber, 
								const int *domain, float weightingCoeff = 1.0f );
    virtual void ClearStatisticalData();
    float ProcessingStatisticalData( float numEvidences );

    void BayesUpdateFactor( const CEvidence* const* pEvidences,
        int EvidenceNumber, const int *domain );
    void PriorToCPD( floatVecVector &parentPriors ); //convert pseudocounts to probability
	void SetFreedomDegrees(int , int ); 

    
    virtual int IsEqual( const CDistribFun *dataToCompare, float epsilon,
        int withCoeff = 1, float* maxDifferenceOut = NULL ) const;
    virtual CDistribFun* GetNormalized()const ;
    virtual void Normalize();
    virtual CNodeValues* GetMPE() ;
    //methods to convert distribution with dense matrices to distribution with sparse
    //if its already sparse - return copy
    virtual CDistribFun* ConvertToSparse() const;
    
    //methods to convert distribution with sparse matrices to distribution with dense
    //if its already dense - return copy
    virtual CDistribFun* ConvertToDense() const;
    // virtual void GenerateSample(pValueVector &obsVls ) const;
    
    virtual int IsSparse() const;
    virtual int IsDense() const;

    virtual void Dump() const;
    
    ~CGaussianDistribFun();
    
    //compute probability for given variable vector
    double ComputeProbability( const C2DNumericDenseMatrix<float>* pMatVariable , int asLog = 1,
        int numObsParents = 0, const int* obsParentsIndices = NULL,
        C2DNumericDenseMatrix<float>*const * pObsParentsMats = NULL ) const;
    //asLog = 1 - it means we return only exponential order
    //asLog = 0 - we return the value of exponent with coefficient
    //asLog = 2 - return exponential order without processing of coefficient

	virtual int GetNumberOfFreeParameters() const;

#ifdef PAR_PNL
    void UpdateStatisticsML(CDistribFun *pDF);
#endif

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CGaussianDistribFun::m_TypeInfo;
  }
#endif

protected:
    void ReleaseAllMatrices();
    void ShrinkToMatrices( const int* keepPositions, int numKeepPositions,
        C2DNumericDenseMatrix<float>* obsMatrix, 
        C2DNumericDenseMatrix<float>** resVecMatOut,
        C2DNumericDenseMatrix<float>** resSquareMatOut,
        float *coeffOut )const;
    //the method provide marginalization in conditions of no positions of Delta multiply
    void MarginalizeOnMatrices( const int* keepVariables, int numKeepVariables,
        const int* allVariableSizes, 
        int numVariables,
        C2DNumericDenseMatrix<float>* vecMat,
        C2DNumericDenseMatrix<float>* squareMat,
        float coeff, int isInMoment,
        C2DNumericDenseMatrix<float>** resVecMatOut,
        C2DNumericDenseMatrix<float>** resSquareMatOut,
        float *resCoeffOut,
        int maximize )const;
    CGaussianDistribFun( int isPotential, int NodeNumber,
        const CNodeType *const* nodeTypes,
        const float *dataMean,
        const float *dataCov, 
        const float **dataWeight = NULL);
    CGaussianDistribFun( int NumberOfNodes, 
        const CNodeType *const* nodeTypes,
        const float *dataH,
        const float *dataK,
        float g = 0.0f);
    CGaussianDistribFun( int NumberOfNodes, const CNodeType *const* nodeTypes,
        const float *dataMean, int isMoment, int isPotential);
    CGaussianDistribFun ( int NumberOfNodes, const CNodeType *const*nodeTypes,
        int isPotential, int isCanonical);
    CGaussianDistribFun( const CGaussianDistribFun & inpDistr );
    C2DNumericDenseMatrix<float> * GetBlock( intVector &ind1, intVector &ind2,
	intVector &ns,
	const C2DNumericDenseMatrix<float> *pMat,
	const C2DNumericDenseMatrix<float> *pMat1 = NULL);
    C2DNumericDenseMatrix<float> * FormXX( intVector& unclumpedDims, intVector& nsVec, 
	bool bMeanCl, float nEv);
    C2DNumericDenseMatrix<float> * FormMuX(  intVector& unclumpedDims, intVector& nsVec );
    C2DNumericDenseMatrix<float> * FormCov(intVector& nsVec, float nEv );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    int m_numberOfDims;
    
    int m_bPotential;
    int m_bMoment;
    int m_bCanonical;
    int m_bDeltaFunction;
    
    C2DNumericDenseMatrix<float> *m_pMatrixMean;
    C2DNumericDenseMatrix<float> *m_pMatrixCov;
    C2DNumericDenseMatrix<float> **m_pMatricesWeight;
    float m_normCoeff;
    
    float m_g;
    C2DNumericDenseMatrix<float> *m_pMatrixH;
    C2DNumericDenseMatrix<float> *m_pMatrixK;
    
    
    C2DNumericDenseMatrix<float> *m_pLearnMatrixMean;
    C2DNumericDenseMatrix<float> *m_pLearnMatrixCov;
    
    //we need to keep delta functions which were multiplied by this
    //we'll keep positions in which we have new direct mean and these means
    intVector m_posOfDeltaMultiply;
    floatVector m_meanValuesOfMult;
    intVector m_offsetToNextMean;

    //Wishart priors params - only for moment form
    int m_freedomDegreeMean;
    int m_freedomDegreeCov;
    C2DNumericDenseMatrix<float> *m_pPseudoCountsMean;
    C2DNumericDenseMatrix<float> *m_pPseudoCountsCov;
    
};

inline int CGaussianDistribFun::GetFactorFlag()const
{
    return m_bPotential;
}

inline float CGaussianDistribFun::GetCoefficient( int isCanonical ) const
{
    if(( isCanonical )&&( m_bCanonical ))
    {
        return m_g;
    }
    else 
    {
        if(( !isCanonical )&&( m_bMoment ))
        {
            return m_normCoeff;
        }
        else
        {
            PNL_THROW( CInvalidOperation, "no valid coefficient for asked form" )
                return 0;
        }
    }
}

inline int CGaussianDistribFun::GetCanonicalFormFlag() const
{
    return m_bCanonical;		
}

inline int CGaussianDistribFun::GetMomentFormFlag() const
{
    return m_bMoment;		
}

inline int CGaussianDistribFun::IsDistributionSpecific() const
{
    if( m_bUnitFunctionDistribution )
    {
        return 1;
    }
    if( m_bDeltaFunction )
    {
        return 2;
    }
    if( m_posOfDeltaMultiply.size() == 0 )
    {
        return 0;
    }
    else 
    {
        return 3;
    }
}

PNL_END

#endif //__PNLGAUSSIANDISTRIBFUN_HPP__
