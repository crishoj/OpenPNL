/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlSoftMaxDistribFun.hpp                                    //
//                                                                         //
//  Purpose:   CSoftMaxDistribFun class definition                         //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Vinogradov                                         //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLSOFTMAXDISTRIBFUN_HPP__
#define __PNLSOFTMAXDISTRIBFUN_HPP__

#include "pnlParConfig.hpp"
#include "pnlDistribFun.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

template <class Type> class C2DNumericDenseMatrix;

class PNL_API CSoftMaxDistribFun : public CDistribFun
{
public:
  static CSoftMaxDistribFun* Create(int NodeNumber,
    const CNodeType *const* nodeTypes, const float *dataWeight,
    const float *dataOffset);

  static CSoftMaxDistribFun* CreateUnitFunctionDistribution(int NodeNumber,
    const CNodeType *const* nodeTypes);

  virtual void CreateDefaultMatrices(int typeOfMatrices = 1);

  CDistribFun& operator=(const CDistribFun& pInputDistr);

  virtual CDistribFun* Clone() const;

  virtual CDistribFun* CloneWithSharedMatrices();

  virtual void AllocMatrix(const float *data, EMatrixType mType,
    int numberOfWeightMatrix = -1, const int *parentIndices = NULL);

  virtual void AllocOffsetVector(const float *data);

  virtual void AttachMatrix(CMatrix<float> *pMatrix, EMatrixType mType,
    int numberOfWeightMatrix = -1, const int *parentIndices = NULL,
    bool isMultipliedByDelta = 0);

  virtual void AttachOffsetVector(const floatVector *pVector);

  static CSoftMaxDistribFun* Copy(const CSoftMaxDistribFun* pInpDistr);

  virtual bool IsValid(std::string* descriptionOut = NULL) const;

  inline int IsDistributionSpecific() const;
  // if 0 - full distribution (non delta, non uniform, non mixed,
  //   can haven't valid form - use GetFlagMoment, GetFlagCanonical)
  // if 1 - uniform distribution - have no matrices, only size

  int GetMultipliedDelta(const int **positions, const float **values,
    const int **offsets) const;

  CDistribFun *ConvertCPDDistribFunToPot() const ;
  
  CDistribFun *ConvertCPDDistribFunToPotential(floatVector MeanContParents, 
      C2DNumericDenseMatrix<float>* CovContParents, int r);

  CDistribFun *CPD_to_pi(CDistribFun *const* allPiMessages,
    int *multParentIndices, int numMultNodes, int posOfExceptParent = -1,
    int maximizeFlag = 0) const;

  CDistribFun *CPD_to_lambda(const CDistribFun *lambda,
    CDistribFun *const* allPiMessages, int *multParentIndices,
    int numMultNodes, int posOfExceptNode = -1, int maximizeFlag = 0) const;

  virtual void MarginalizeData(const CDistribFun* pOldData,
    const int *DimOfKeep, int NumDimsOfKeep, int maximize);

  void ShrinkObservedNodes(const CDistribFun* pOldData,
    const int *pVarsObserved, const Value* const* pObsValues, int numObsVars,
    const CNodeType* pObsTabNT, const CNodeType* pObsGauNT);

  void ExpandData(const int* pDimsToExtend,
    int numDimsToExpand, const Value* const* valuesArray,
    const CNodeType* const*allFullNodeTypes, int UpdateCanonical = 1);

  virtual void MultiplyInSelfData(const int *pBigDomain,
    const int *pSmallDomain, const CDistribFun *pOtherData);

  virtual void SumInSelfData(const int *pBigDomain,
    const int *pSmallDomain, const CDistribFun *pOtherData);

  virtual void DivideInSelfData(const int *pBigDomain,
    const int *pSmallDomain, const CDistribFun *pOtherData) ;

  virtual CMatrix<float> *GetMatrix(EMatrixType mType,
    int numWeightMat = -1, const int *parentIndices = NULL) const;

  floatVector *GetOffsetVector();

  void InitLearnData();

  void CopyLearnDataToDistrib();

  virtual CMatrix<float> *GetStatisticalMatrix(EStatisticalMatrix mType,
    int *parentIndices = NULL) const;

  floatVector *GetStatisticalOffsetVector();

  virtual void SetStatistics(const CMatrix<float> *pMat,
    EStatisticalMatrix matrix, const int* parentsComb = NULL);

  void SetOffsetStatistics(const floatVector *pVec);

  virtual void UpdateStatisticsEM(const CDistribFun* infData,
    const CEvidence *pEvidence = NULL, float weightingCoeff = 1.0f,
    const int* domain = NULL);

  virtual void UpdateStatisticsML(const CEvidence* const* pEvidences,
    int EvidenceNumber, const int *domain, float weightingCoeff = 1.0f);

  virtual void ClearStatisticalData();

  virtual CNumericDenseMatrix<float>* GetProbMatrix(const CEvidence *pEvidence);

  float ProcessingStatisticalData(float numEvidences);

  virtual int IsEqual(const CDistribFun *dataToCompare, float epsilon,
    int withCoeff = 1, float* maxDifferenceOut = NULL) const;

  virtual CDistribFun* GetNormalized() const;

  virtual void Normalize();

  virtual CNodeValues* GetMPE();

  virtual CDistribFun* ConvertToSparse() const;

  virtual CDistribFun* ConvertToDense() const;

  virtual int IsSparse() const;

  virtual int IsDense() const;

  virtual void Dump() const;

  virtual void DumpMatrix(const CEvidence *pEvidence);

  virtual int GetNumberOfFreeParameters() const;

  virtual ~CSoftMaxDistribFun();

  void MaximumLikelihood(float **Observation,
    int NumberOfObservations, float Accuracy, float step = 0.1);

  void SetMaximizingMethod(EMaximizingMethod met);

  float CalculateLikelihood(float **Observation, int NumberOfObservations);

  EMaximizingMethod GetMaximizingMethod();

#ifdef PAR_PNL
  void UpdateStatisticsML(CDistribFun *pDF);
#endif

  inline int GetSoftMaxSize() const;

  float CalculateKsi(floatVector MeanContParents, 
      C2DNumericDenseMatrix<float>* CovContParents);
  
  void CalculateMeanAndCovariance(float OldKsi, float r, 
      floatVector &MeanVector, 
      C2DNumericDenseMatrix<float> **CovMatrix);

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CSoftMaxDistribFun::m_TypeInfo;
  }
#endif

protected:
  CSoftMaxDistribFun(int NodeNumber, const CNodeType *const* nodeTypes,
    const float *dataWeight, const float *dataOffset);

  CSoftMaxDistribFun(int NumberOfNodes, const CNodeType *const* nodeTypes);

  CSoftMaxDistribFun(const CSoftMaxDistribFun &inpDistr);

  void CalculateHessianForOffset(float ** pContVectorEvidence,
    int NumberOfObservations);

  void CalculateHessianForWeights(float ** pContVectorEvidence,
    int NumberOfObservations);

  float CalculateNorm(float ** grad_weights, float * grad_offset);

  void CalculateHessian(float ** pContVectorEvidence,
    int NumberOfObservations);

  void MaximumLikelihoodGradient(float **Observation,
    int NumberOfObservations, float Accuracy, float step = 0.1);

  void MaximumLikelihoodConjugateGradient(float **Observation,
    int NumberOfObservations, float Accuracy, float step = 0.1);

  void MaximumLikelihoodHessian(float **Observations,
    int NumberOfObservations, float Accuracy, float step = 0.1);

#ifdef PNL_RTTI
  static const CPNLType m_TypeInfo;
#endif

private:
  C2DNumericDenseMatrix<float> *m_pMatrixWeight;
  floatVector m_VectorOffset;

  C2DNumericDenseMatrix<float> *m_pLearnMatrixWeight;
  floatVector m_LearnVectorOffset;

  C2DNumericDenseMatrix<float> *m_hessian;

  EMaximizingMethod m_MaximizingMethod;
  // defines method, which is used for Likelihood Maximizing

  //canonocal characteristics
  float m_g;
  C2DNumericDenseMatrix<float> *m_pMatrixH;
  C2DNumericDenseMatrix<float> *m_pMatrixK;

  const static CNodeType m_ZeroNT;
};

inline int CSoftMaxDistribFun::IsDistributionSpecific() const
{
  if (m_bUnitFunctionDistribution)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

//----------------------------------------------------------------------
inline int CSoftMaxDistribFun::GetSoftMaxSize() const
{
    return m_VectorOffset.size();
}
//----------------------------------------------------------------------
PNL_END

#endif //__PNLGAUSSIANDISTRIBFUN_HPP__
