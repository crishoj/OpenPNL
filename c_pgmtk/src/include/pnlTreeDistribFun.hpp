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
//  Purpose:   CTreeDistribFun class definition                        //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// GaussianData.hpp: interface for the GaussianData class.
//
/////////////////////////////////////////////////////////////////////////////// 
#ifndef __PNLTREEDISTRIBFUN_HPP__
#define __PNLTREEDISTRIBFUN_HPP__

#include "pnlParConfig.hpp"
#include "cart/cvcart.h"
#include "pnlDistribFun.hpp"

typedef struct _SCARTParams
{
    bool is_cross_val;
    int cross_val_folds;
    float learn_sample_part;
    CxMat* priors;
} SCARTParams;

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

template <class Type> class C2DNumericDenseMatrix;

class PNL_API CTreeDistribFun : public CDistribFun
{
public:
    virtual ~CTreeDistribFun();
    static CTreeDistribFun* Create( int NodeNumber, const CNodeType *const* NodeTypes, const SCARTParams* params = 0);
    static  CTreeDistribFun* Copy( const CTreeDistribFun* pInpDistr );
    virtual CDistribFun& operator =( const CDistribFun& pInputDistr );
    virtual CDistribFun* Clone() const;
    virtual void CreateDefaultMatrices( int typeOfMatrices = 1 );

    virtual CDistribFun* CloneWithSharedMatrices();

    virtual void AllocMatrix( const float *data, EMatrixType mType, 
							 int numberOfWeightMatrix = -1,
                                                         const int *parentIndices = NULL);
    virtual void AttachMatrix( CMatrix<float> *pMatrix,
				EMatrixType mType,
                                int numberOfWeightMatrix = -1,
                                const int *parentIndices = NULL,
                                bool isMultipliedByDelta = 0);
	void UpdateTree(const CGraph* pGraphTree, TreeNodeFields *fields );
	void SetDomain(intVector domain );
    virtual bool IsValid(std::string* descriptionOut = NULL) const;
    CDistribFun *ConvertCPDDistribFunToPot()const ;
    CDistribFun *CPD_to_pi( CDistribFun *const* allPiMessages, 
        int *multParentIndices, int numMultNodes, int posOfExceptParent = -1, 
        int maximizeFlag = 0 )const ;
    CDistribFun *CPD_to_lambda( const CDistribFun *lambda, 
        CDistribFun *const* allPiMessages, int *multParentIndices,
        int numMultNodes, int posOfExceptNode = -1, int maximizeFlag = 0 )const ;	
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
    virtual void MultiplyInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData );
    virtual void SumInSelfData(const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData);
    virtual void DivideInSelfData( const int *pBigDomain, 
        const int *pSmallDomain, const CDistribFun *pOtherData ) ;
    virtual CMatrix<float> *GetMatrix( EMatrixType mType, 
        int numWeightMat = -1,
        const int *parentIndices = NULL) const;
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
    
    virtual int GetMultipliedDelta( const int **positions, const float **values, 
        const int **offsets ) const;	
    
    virtual int IsEqual( const CDistribFun *dataToCompare, float epsilon,
        int withCoeff = 1, float* maxDifferenceOut = NULL) const;
    virtual CDistribFun* GetNormalized()const ;
    virtual void Normalize();
    virtual CNodeValues* GetMPE() ;
    //methods to convert distribution with dense matrices to distribution with sparse
    //if its already sparse - return copy
    virtual CDistribFun* ConvertToSparse() const;
    
    //methods to convert distribution with sparse matrices to distribution with dense
    //if its already dense - return copy
    virtual CDistribFun* ConvertToDense() const;
    
    virtual int IsSparse() const;
    virtual int IsDense() const;
    void Dump() const;
    
	void ConvertGraphToTree(CxCART *pCart,CxCARTNode *node,const CGraph *pGraph, int nodeNum) const;
    void FillTree(CxCART *pCart,CxCARTNode *node,const CGraph *pGraph, TreeNodeFields *filds, int nodeNum) const;

    void GenerateSample( CEvidence* evidence, int maximize ) const;
    
    virtual inline int IsDistributionSpecific()const;
    inline bool IsRegression() const;
    inline float GetNodeValue( const CEvidence* pEv, int i ) const;
    float GetLogLik( const CEvidence* pEv ) const;
	floatVector GetProbability( const CEvidence* pEv ) const;
    int GetAdjectives( const CEvidence* pEv, float &expect, float &varian) const;
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
    return CTreeDistribFun::m_TypeInfo;
  }
#endif

protected:
    CTreeDistribFun(int NodeNumber, 
        const CNodeType *const* NodeTypes,
        const SCARTParams* params = 0);
    CTreeDistribFun( const CTreeDistribFun & inpDistr );

#ifdef PNL_RTTI
  static const CPNLType m_TypeInfo;
#endif
private:
    void Clear();
    void _CreateCART();
    
private:
    std::vector <const CEvidence*> m_vecAllEvidences;
    SCARTParams m_Params;
    float m_fLogLik;
    CxCART* m_pCART;
    CxClassifierSample* m_pSample;
    CxPruningData* m_pOptimalData;
    CxMat* m_terms_mat;
    CxMat* m_missed_mask;
    CxMat* m_response_mat;
    const int* m_domain;
};

inline int CTreeDistribFun::IsDistributionSpecific()const
{
    return m_bUnitFunctionDistribution;
    //fixme - we can have other special cases of tabular distributions
}

inline bool CTreeDistribFun::IsRegression() const
{
    return !(m_NodeTypes.back()->IsDiscrete());
}

inline float CTreeDistribFun::GetNodeValue( const CEvidence* pEv, int i ) const
{
    assert(m_domain);
    int node =  m_domain[i];
    assert(node >= 0);
    const Value* val = pEv->GetValueBySerialNumber(node);
    return m_NodeTypes[i]->IsDiscrete() ? (float)val->GetInt() : val->GetFlt();
    
}

PNL_END

#endif //__PNLTREEDISTRIBFUN_HPP__
