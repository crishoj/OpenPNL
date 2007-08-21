/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlTabularDistribFun.hpp                                    //
//                                                                         //
//  Purpose:   CTabularDistribFun class definition                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// TabularDistribFun.hpp: interface for the TabularData class.
//
/////////////////////////////////////////////////////////////////////////////// 
#ifndef __PNLTABULARDISTRIBFUN_HPP__
#define __PNLTABULARDISTRIBFUN_HPP__

#include "pnlParConfig.hpp"
#include "pnlDistribFun.hpp"
#include "pnlNumericDenseMatrix.hpp"
#include "pnlNumericSparseMatrix.hpp"
//#include "Evidence.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API CTabularDistribFun: public CDistribFun
{
public:
    static CTabularDistribFun* Create( int numOfNds,
        const CNodeType *const* NodeTypes, const float *data,
        int allocTheMatrices = 0, int asDense = -1);
    static CTabularDistribFun* CreateUnitFunctionDistribution( int NumberOfNodes,
        const CNodeType *const*nodeTypes, int asDense = 1 );
    static CTabularDistribFun* Copy( const CTabularDistribFun *pInpDistr );
    virtual CDistribFun& operator=( const CDistribFun& pInputDistr );
    virtual CDistribFun* Clone() const;

    virtual void CreateDefaultMatrices( int typeOfMatrices = 1 );
    //typeOfMatrices == 1 - randomly created matrices

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
    /*This function add ReferenceCounter at this MultiDMatrix*/
    virtual CMatrix<float> *GetMatrix( EMatrixType mType, 
        int numWeightMat = -1,
        const int *parentIndices = NULL) const;
    virtual inline int IsDistributionSpecific()const;
    virtual CMatrix<float> *GetStatisticalMatrix( EStatisticalMatrix mType, 
        int *parentIndices = NULL ) const;
    virtual void  MarginalizeData( const CDistribFun* pOldData, 
        const int *DimOfKeep, 
        int NumDimsOfKeep, int maximize );
    //the function that provide marginalization in-place
    void Marginalize( const CDistribFun* pOldData, const int* DimsOfKeep,
        int NumDimsOfKeep, int maximize );
    void ShrinkObservedNodes( const CDistribFun* pOldData,
        const int *pVarsObserved, 
        const Value* const* pObsValues, int numObsVars,
        const CNodeType* pObsTabNT, const CNodeType* pObsGauNT ) ;
    void ExpandData( const int* pDimsToExpand, 
        int numDimsToExpand, const Value* const* valuesArray, 
        const CNodeType* const*allFullNodeTypes, int UpdateCanonical = 1 );
        /*      virtual void MultiplyData( const int *pBigDomain, const int *pSmallDomain,
                                                                const CDistribFun *pBigData,
    const CDistribFun *pSmallData) ;*/
    virtual void MultiplyInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData );
    virtual void SumInSelfData(const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData);
    virtual void DivideInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData );
        /*      virtual void DivideData( const int *pBigDomain, const int *pSmallDomain,
                                                                const CDistribFun *pBigData,
    const CDistribFun *pSmallData) ;*/
    virtual void SetStatistics( const CMatrix<float> *pMat, 
        EStatisticalMatrix matrix, const int* parentsComb = NULL );

    virtual void UpdateStatisticsEM( const CDistribFun* infData,
        const CEvidence *pEvidence = NULL, float weightingCoeff = 1.0f,
        const int* domain = NULL );
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidences, int EvidenceNumber, 
                                                                const int *domain, float weightingCoeff = 1.0f );
    virtual float ProcessingStatisticalData( float numEvidences ) ;
    
    void BayesUpdateFactor(const CEvidence* const* pEvidences,
        int EvidenceNumber, const int *domain );
    void PriorToCPD(); //convert pseudocounts to probability
    
    
    int GetMultipliedDelta( const int **positions, const float **values, 
        const int **offsets ) const;
    virtual void ClearStatisticalData();
    virtual int IsEqual( const CDistribFun *dataToCompare, float epsilon, 
        int withCoeff = 1, float* maxDifferenceOut = NULL ) const ;
    virtual CDistribFun *ConvertCPDDistribFunToPot()const; 
#ifndef SWIG
    void ConvertCPDDistribFunToPot(CDistribFun** pDistrInOut) const;
#endif
    CDistribFun *CPD_to_pi( CDistribFun *const* allPiMessages, 
        int *multParentIndices, int numMultNodes, int posOfExceptParent = -1, 
        int maximizeFlag = 0 )const ;
    //doesn't allocate new memory - put result to 
    void CPDToPi( CDistribFun *const* allPiMessages, 
        int *multParentIndices, int numMultNodes, CDistribFun** piMesInOut,
        CDistribFun* cpdForMultIn, int posOfExceptParent = -1, 
        int maximizeFlag = 0 )const ;
    CDistribFun *CPD_to_lambda( const CDistribFun *lambda, 
        CDistribFun *const* allPiMessages, int *multParentIndices,
        int numMultNodes, int posOfExceptNode = -1, int maximizeFlag = 0 )const ;
    void CPDToLambda( const CDistribFun *lambda, 
        CDistribFun *const* allPiMessages, int *multParentIndices,
        int numMultNodes, CDistribFun** lambdaMesInOut, CDistribFun* cpdForMultIn,
        int posOfExceptNode = -1, int maximizeFlag = 0 )const ;
    //multParentIndices[i] = j <=> corresponds j's parent in domain for CPD 
    //for which we need to multiply
    //allPiMessages[i] corresponds pi message from i's parent in domain for CPD
    //lambda is message from last node in domain for CPD - child node
    virtual CDistribFun* GetNormalized()const ;
    virtual void Normalize();
    virtual CNodeValues* GetMPE();
    //methods to convert distribution with dense matrices to distribution with sparse
    //if its already sparse - return copy
    virtual CDistribFun* ConvertToSparse() const;
    
    //methods to convert distribution with sparse matrices to distribution with dense
    //if its already dense - return copy
    virtual CDistribFun* ConvertToDense() const;
    
    virtual int IsSparse() const;
    virtual int IsDense() const;
    virtual void Dump() const;

    //for domain with value node as last function allways return true 
    bool IsMatrixNormalizedForCPD( float eps = 1e-5f) const;
    
    ~CTabularDistribFun();

    virtual int GetNumberOfFreeParameters() const;

    void SetCheckNegative(bool val);

    void InitPseudoCounts(int val = 0);

    float CalculateBayesianScore(); 

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
    return CTabularDistribFun::m_TypeInfo;
  }
#endif

protected:
    CTabularDistribFun(int NodeNumber, const CNodeType *const* nodeTypes,
        const float *data = NULL, int asDense = 1, int allocTheMatrices = 0,
        bool CheckNegative = true);

    CTabularDistribFun(int NodeNumber, const CNodeType *const* nodeTypes,
        int asDense, bool CheckNegative = true);

    CTabularDistribFun( const CTabularDistribFun &inpDistr );
    CMatrix<float>* CreateUnitFunctionMatrix( int isDense = 1, int withoutData = 1) const;
    inline void CreateUnitFunctionData();

#ifdef PNL_RTTI
  static const CPNLType m_TypeInfo;
#endif

  double Gamma (int arg);

private:
    int m_bDense;
    bool m_bCheckNegative;
    CMatrix<float> *m_pMatrix;
    //the matrix is always presented here,
    //in case of unit distribution it contain stupid data

    CMatrix<float> *m_pLearnMatrix;
    
    //Prior/posterior factor distribution function
    //Modelled as Dirichlet distribution with "alphas" 
    //(see any reference on Dirichlet)
    //these "alphas" has the meaning of pseudocounts
    //i.e. imaginary number of observed cases
    //When learning these pseudocounts are updated(added)
    //with real counts. Final target distribution is result of 
    //pseudocounts normalization
    CMatrix<float> *m_pPseudoCounts;
};

inline int CTabularDistribFun::IsDistributionSpecific()const
{
    return m_bUnitFunctionDistribution;
    //fixme - we can have other special cases of tabular distributions
}

inline void CTabularDistribFun::CreateUnitFunctionData() 
{
    //call corresponding method of matrix
}

PNL_END

#endif //__PNLTABULARDISTRIBFUN_HPP__
