/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDistribFun.hpp                                           //
//                                                                         //
//  Purpose:   CDistribFun class definition                                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// DistribFun.hpp

#ifndef __PNLDISTRIBFUN_HPP__
#define __PNLDISTRIBFUN_HPP__

#include "pnlParConfig.hpp"
#include "pnlObject.hpp"
#include "pnlNodeType.hpp"
#include "pnlEvidence.hpp"
#include "pnl2DNumericDenseMatrix.hpp"
//fixme - enum use for Matrix - should it be here?

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API CDistribFun : public CPNLBase
{
public:
    virtual CDistribFun& operator=( const CDistribFun&){return (*this);}
    virtual void CreateDefaultMatrices( int typeOfMatrices = 1 ) = 0;
    //typeOfMatrices == 1 - randomly created matrices
    //for Gaussian distribution it creates in moment form
    virtual CDistribFun* Clone() const = 0;
    virtual CDistribFun* CloneWithSharedMatrices() = 0;
    inline const pConstNodeTypeVector *GetNodeTypesVector() const;
    inline void SetVariableType( int position, const CNodeType* varType );
    virtual bool IsValid(std::string* descriptionOut = NULL) const = 0;
    virtual void AllocMatrix( const float *data, EMatrixType mType, 
        int numberOfWeightMatrix = -1,
        const int *parentIndices = NULL) = 0;
    virtual void AttachMatrix( CMatrix<float> *pMatrix,
				EMatrixType mType,
                                int numberOfWeightMatrix = -1,
                                const int *parentIndices = NULL,
                                bool isMultipliedByDelta = 0) = 0;
    virtual CMatrix<float> *GetMatrix( EMatrixType mType, 
        int numWeightMat = -1,
        const int *parentIndices = NULL ) const = 0;
    inline int GetNumberOfNodes() const;
    virtual int IsDistributionSpecific() const = 0;
    virtual CMatrix<float>* GetStatisticalMatrix( EStatisticalMatrix mType,
        int *parentIndices = NULL ) const = 0;
    //void SetClamped(int Mask=0);
    virtual void MarginalizeData( const CDistribFun *pOldData,
        const int *DimsOfKeep, int NumDimsOfKeep,
        int maximize ) = 0;
        /*	virtual void MultiplyData( const int *pBigDomain, const int *pSmallDomain,
        const CDistribFun *pBigData,
    const CDistribFun *pSmallData) = 0;*/
    virtual void MultiplyInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData ) = 0;
    virtual void SumInSelfData(const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData) = 0;
    virtual void DivideInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData ) = 0;
        /*	virtual void DivideData( const int *pBigDomain, const int *pSmallDomain,
        const CDistribFun *pBigData,
    const CDistribFun *pSmallData) = 0;*/
    virtual void ShrinkObservedNodes( const CDistribFun* pOldData, 
        const int *pDimsOfObserved, 
        const Value* const* pObsValues, int numObsDim,
        const CNodeType* pObsTabNT, const CNodeType* pObsGauNT ) = 0;
    virtual void ExpandData( const int* pDimsToExtend, 
        int numDimsToExpand, const Value* const* valuesArray, 
        const CNodeType* const *allFullNodeTypes, int UpdateCanonical = 1 ) = 0;
    virtual void ClearStatisticalData() = 0;
    virtual void SetStatistics( const CMatrix<float> *pMat, 
	EStatisticalMatrix matrix, const int* parentsComb = NULL ) = 0;
    virtual void UpdateStatisticsEM( const CDistribFun* infData, 
        const CEvidence *pEvidence = NULL, float weightingCoeff = 1.0f,
        const int* domain = NULL ) = 0;
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidences, int EvidenceNumber,
        const int *domain, float weightingCoeff = 1.0f ) = 0;
    virtual float ProcessingStatisticalData( float numEvidences ) = 0;
    
    virtual int GetMultipliedDelta( const int **positions, const float **values, 
        const int **offsets ) const = 0;	
    
    virtual CDistribFun *ConvertCPDDistribFunToPot()const = 0;
    virtual CDistribFun *CPD_to_pi( CDistribFun *const* allPiMessages, 
        int *multParentIndices, int numMultNodes, int posOfExceptParent, 
        int maximizeFlag = 0 )const = 0;
    virtual CDistribFun *CPD_to_lambda( const CDistribFun *lambda, 
        CDistribFun *const* allPiMessages, int *multParentIndices,
        int numMultNodes, int posOfExceptNode, int maximizeFlag = 0 )const = 0;	
    
    virtual CDistribFun* GetNormalized()const = 0;
    virtual void Normalize() = 0;
    inline EDistributionType GetDistributionType() const;
    //the method also return value of max difference between the objects,
    //if there is the difference in node types the value of maxDifference is 0
    virtual int IsEqual( const CDistribFun *dataToCompare, float epsilon, 
        int withCoeff = 1, float* maxDifferenceOut = NULL) const = 0;
    virtual CNodeValues* GetMPE() = 0;
    //methods to convert distribution with dense matrices to distribution with sparse
    //if its already sparse - return copy
    virtual CDistribFun* ConvertToSparse() const = 0;
    
    //methods to convert distribution with sparse matrices to distribution with dense
    //if its already dense - return copy
    virtual CDistribFun* ConvertToDense() const = 0;
    
    virtual int IsSparse() const = 0;
    virtual int IsDense() const = 0;
    virtual void ResetNodeTypes( pConstNodeTypeVector &nodeTypes );
    virtual void Dump() const = 0;
    inline void SetUnitValue(int unitValue);
    inline bool GetMatricesValidityFlag() const;

	virtual int GetNumberOfFreeParameters() const = 0;

    ~CDistribFun(){}

#ifdef PAR_PNL
    virtual void UpdateStatisticsML(CDistribFun *pDF) = 0;
#endif

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CDistribFun::m_TypeInfo;
    }
#endif

protected:
    CDistribFun(EDistributionType dt, int numNodes,
		const CNodeType* const* nodeTypes,
                int isUnitFunction = 0, bool allocTheMatrices = 0);
    CDistribFun( EDistributionType dt );
    
    EDistributionType m_DistributionType;

//    inline void AllocateMatrices();
    
    inline void ChangeMatricesValidityFlag( bool flag ) const; 

    int m_NumberOfNodes;
    int m_bUnitFunctionDistribution;

    
    pConstNodeTypeVector m_NodeTypes;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif

private:
    //	int m_bClamping;
    //flag if the matrices are allocated - for both unit function or not
//    bool m_bMatricesAreAllocated;

    //the flag is set to 1 if the matrices are valid and to 0 if it should be checked
    mutable bool m_bAllMatricesAreValid;

};

inline void CDistribFun::SetUnitValue(int unitValue)
{
    m_bUnitFunctionDistribution = unitValue;
}

inline const
pConstNodeTypeVector *CDistribFun::GetNodeTypesVector() const
{
    return &m_NodeTypes;
}

inline int CDistribFun::GetNumberOfNodes() const
{
    return m_NumberOfNodes;
}

inline EDistributionType CDistribFun::GetDistributionType() const
{
    return m_DistributionType;
}

inline void CDistribFun::SetVariableType( int position,
                                         const CNodeType* varType )
{
    int isNowDiscrete = m_NodeTypes[position]->IsDiscrete();
    int comeDiscrete = varType->IsDiscrete();
    if( isNowDiscrete != comeDiscrete )
    {
        PNL_THROW( CInvalidOperation, "can't change type of node, can only change size" )
    }
    m_NodeTypes[position] = varType;
}

inline bool CDistribFun::GetMatricesValidityFlag() const
{
    return m_bAllMatricesAreValid;
}

inline void CDistribFun::ChangeMatricesValidityFlag(bool flag) const
{
    m_bAllMatricesAreValid = flag;
}

/*inline void CDistribFun::AllocateMatrices()
{
    m_bMatricesAreAllocated = 1;
}*/

PNL_END

#endif //__PNLDISTRIBFUN_HPP__

