/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlScalarDistribFun.hpp                                    //
//                                                                         //
//  Purpose:   CScalarDistribFun class definition                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// ScalarDistribFun.hpp: interface for the ScalarData class.
//
/////////////////////////////////////////////////////////////////////////////// 
#ifndef __PNLSCALARDISTRIBFUN_HPP__
#define __PNLSCALARDISTRIBFUN_HPP__

#include "pnlParConfig.hpp"
#include "pnlDistribFun.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API CScalarDistribFun: public CDistribFun
{
public:
    static CScalarDistribFun* Create( int NodeNumber,
        const CNodeType *const* NodeTypes, int asDense = 1 );
    static CScalarDistribFun* Copy( const CScalarDistribFun *pInpDistr );
    virtual CDistribFun& operator=( const CDistribFun& pInputDistr );
    virtual CDistribFun* Clone() const;
    virtual void CreateDefaultMatrices( int typeOfMatrices = 1 );
    //typeOfMatrices == 1 - randomly created matrices
    //creates Gaussian distribution in moment form

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
    void ShrinkObservedNodes( const CDistribFun* pOldData,
        const int *pVarsObserved, 
        const Value* const* pObsValues, int numObsVars,
        const CNodeType* pObsTabNT, const CNodeType* pObsGauNT ) ;
    void ExpandData( const int* pDimsToExpand, 
        int numDimsToExpand, const Value* const* valuesArray, 
        const CNodeType* const*allFullNodeTypes, int UpdateCanonical = 1 );
    CDistribFun* ExpandScalarData( EDistributionType dtRequested,
        const int* pDimsToExpand, int numDimsToExpand,
        const Value* const* valuesArray, 
        const CNodeType* const*allFullNodeTypes,
        int UpdateCanonical = 1 ) const;
    virtual void MultiplyInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData );
    virtual void SumInSelfData(const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData);
    virtual void DivideInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData );
    virtual void SetStatistics( const CMatrix<float> *pMat, 
	EStatisticalMatrix matrix, const int* parentsComb = NULL );
    virtual void UpdateStatisticsEM( const CDistribFun* infData,
        const CEvidence *pEvidence = NULL, float weightingCoeff = 1.0f,
        const int* domain = NULL );
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidences, int EvidenceNumber, 
								const int *domain, float weightingCoeff = 1.0f );
    virtual float ProcessingStatisticalData( float numEvidences ) ;
    
    int GetMultipliedDelta( const int **positions, const float **values, 
        const int **offsets ) const;
    virtual void ClearStatisticalData();
    virtual int IsEqual( const CDistribFun *dataToCompare, float epsilon, 
        int withCoeff = 1, float* maxDifferenceOut = NULL) const ;
    virtual CDistribFun *ConvertCPDDistribFunToPot()const; 
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
    void Dump() const;
    
	virtual int GetNumberOfFreeParameters()const;      

    ~CScalarDistribFun(){}

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
      return CScalarDistribFun::m_TypeInfo;
    }
#endif

protected:
    CScalarDistribFun( int NodeNumber, const CNodeType *const* nodeTypes,
        int asDense = 1);
    CScalarDistribFun( const CScalarDistribFun &inpDistr );
   
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    int m_bDense;
};

inline int CScalarDistribFun::IsDistributionSpecific()const
{
    return 0;
    //fixme - we can have other special cases of tabular distributions
}

PNL_END

#endif //__PNLSCALARDISTRIBFUN_HPP__
