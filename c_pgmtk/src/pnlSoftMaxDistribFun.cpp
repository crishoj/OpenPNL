/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlSoftMaxDistribFun.cpp                                   //
//                                                                         //
//  Purpose:   CSoftMaxDistribFun class member functions implementation    //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Vinogradov                                         //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlLog.hpp"
#include <sstream>
#include "pnlRng.hpp"
// ----------------------------------------------------------------------------

#ifdef _DEBUG
  #define SM_TEST
#endif

#if defined(_MSC_VER)
  #pragma warning(disable : 4244) 
  // conversion from 'double' to 'float', possible loss of data
#endif

PNL_USING

CDistribFun* CSoftMaxDistribFun::Clone() const
{
  if (!IsValid())
  {
    PNL_THROW(CInconsistentState, "can't clone invalid data");
  }

  CSoftMaxDistribFun* resData = CSoftMaxDistribFun::Create(m_NumberOfNodes, 
    &m_NodeTypes.front(), NULL, NULL);
  resData->m_bUnitFunctionDistribution = m_bUnitFunctionDistribution;

  if (m_bUnitFunctionDistribution)
  {
    return resData;
  }

  if (m_pMatrixWeight)
  {
    resData->AttachMatrix(m_pMatrixWeight->Clone(), matWeights);
  }
  resData->m_VectorOffset = m_VectorOffset;
  resData->m_MaximizingMethod = m_MaximizingMethod;

  return resData;
}
// ----------------------------------------------------------------------------

CDistribFun* CSoftMaxDistribFun::CloneWithSharedMatrices()
{
  if (!IsValid())
  {
    PNL_THROW(CInconsistentState, "can't clone invalid data");
  }
  CSoftMaxDistribFun* resData = CSoftMaxDistribFun::Create(m_NumberOfNodes, 
    &m_NodeTypes.front(), NULL, NULL);
  resData->m_bUnitFunctionDistribution = m_bUnitFunctionDistribution;

  if (m_bUnitFunctionDistribution)
  {
    return resData;
  }

  if (m_pMatrixWeight)
  {
    resData->AttachMatrix(m_pMatrixWeight, matWeights);
  }
  resData->m_VectorOffset = m_VectorOffset;
  resData->m_MaximizingMethod = m_MaximizingMethod;

  return resData;
}
// ----------------------------------------------------------------------------

CSoftMaxDistribFun* CSoftMaxDistribFun::Create(int NumberOfNodes, 
  const CNodeType *const* NodeTypes, const float *dataWeight, 
  const float *dataOffset)
{
  PNL_CHECK_IS_NULL_POINTER(NodeTypes);

  if (!(NodeTypes[NumberOfNodes-1]->IsDiscrete()))
  {
    PNL_THROW(CInconsistentType, "childNode must be discrete");
  }

  int allParentsContinious = 1;
  for(int i = 0; i<NumberOfNodes - 1; i++)
  {
    if (NodeTypes[i]->IsDiscrete())
    {
      allParentsContinious = 0;
      break;
    }
  }
  if (!allParentsContinious)
  {
    PNL_THROW(CInconsistentType, "not all parents continious")
      /*some parents are tabular - it is may be CCondSoftMax*/
  }

  CSoftMaxDistribFun *newData = new CSoftMaxDistribFun(NumberOfNodes, 
    NodeTypes, dataWeight, dataOffset);
  PNL_CHECK_IF_MEMORY_ALLOCATED(newData);

  return newData;
}
// ----------------------------------------------------------------------------

CSoftMaxDistribFun* CSoftMaxDistribFun::CreateUnitFunctionDistribution(
  int NumberOfNodes, const CNodeType *const* NodeTypes)
{
  PNL_CHECK_IS_NULL_POINTER(NodeTypes);

  if (!(NodeTypes[NumberOfNodes-1]->IsDiscrete()))
  {
    PNL_THROW(CInconsistentType, "childNode must be discrete");
  }
  int allParentsContinious = 1;
  for(int i = 0; i<NumberOfNodes - 1; i++)
  {
    if (NodeTypes[i]->IsDiscrete())
    {
      allParentsContinious = 0;
      break;
    }
  }
  if (!allParentsContinious)
  {
    PNL_THROW(CInconsistentType, "not all parents continious")
    // some parents are tabular - it is may be CCondSoftMax
  }

  CSoftMaxDistribFun *newData = new CSoftMaxDistribFun(NumberOfNodes, 
    NodeTypes);
  PNL_CHECK_IF_MEMORY_ALLOCATED(newData);

  return newData;
}
// ----------------------------------------------------------------------------

CSoftMaxDistribFun* CSoftMaxDistribFun::Copy(
  const CSoftMaxDistribFun* pInpDistr)
{
  PNL_CHECK_IS_NULL_POINTER(pInpDistr);

  CSoftMaxDistribFun *retDistr = new CSoftMaxDistribFun(*pInpDistr);
  PNL_CHECK_IF_MEMORY_ALLOCATED(retDistr);

  return retDistr;
}
// ----------------------------------------------------------------------------

bool CSoftMaxDistribFun::IsValid(std::string* description) const
{
  if (!m_bUnitFunctionDistribution)
  {
    return 1;
  }
  else
  {
    if (description)
    {
      std::stringstream st;
      st << "SoftMax distribution function haven't valid form." << std::endl;
      st << "For valid form should be matrix Weight and vector Offset." << std::endl;
      std::string s = st.str();
      description->insert(description->begin(), s.begin(), s.end());
    }
    return 0;
  }
}
// ----------------------------------------------------------------------------

int CSoftMaxDistribFun::GetMultipliedDelta(const int **positions, 
  const float **values, const int **offsets) const
{
  *positions = NULL;
  *values = NULL;
  *offsets = NULL;
  return 0;
}
// ----------------------------------------------------------------------------

//typeOfMatrices == 1 - randomly created matrices
void CSoftMaxDistribFun::CreateDefaultMatrices(int typeOfMatrices)
{
  PNL_CHECK_RANGES (typeOfMatrices, 1, 1);
  //we have only one type of matrices now
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW (CInconsistentType,
      "uniform distribution can't have any matrices with data");
  }
  void* pObj = this;
  
  // create matrix of weights
  if (m_pMatrixWeight)
  {
    PNL_THROW(CInvalidOperation,
      "can't create default matrix, the distribution already has it");
  }
  intVector dims;
  dims.resize(2);
  int chldNodeSize = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  dims[0] = m_NumberOfNodes - 1;
  dims[1] = chldNodeSize;


  floatVector weight;
  int weightSize = dims[0] * dims[1];
  weight.assign(weightSize, 0.0f);
  pnlRand(weightSize, &weight.front(), -5.0f, 5.0f);
  m_pMatrixWeight = C2DNumericDenseMatrix<float>::Create(&dims.front(),
    &weight.front());
  m_pMatrixWeight->AddRef(pObj);
  
  // create offsets vector
  PNL_CHECK_FOR_NON_ZERO(m_VectorOffset.size());
  m_VectorOffset.assign(chldNodeSize, 0.0f);
  pnlRand(chldNodeSize, &m_VectorOffset.front(), -5.0f, 5.0f);
}
// ----------------------------------------------------------------------------

//constructor for softmax data
//number of nodes - number of nodes in domain
CSoftMaxDistribFun::CSoftMaxDistribFun(int NumberOfNodes, 
  const CNodeType *const* nodeTypes, const float *dataWeight, 
  const float *dataOffset):
  CDistribFun(dtSoftMax, NumberOfNodes, nodeTypes, 0)
{
  PNL_CHECK_LEFT_BORDER(NumberOfNodes, 1);
  PNL_CHECK_IS_NULL_POINTER(nodeTypes);

  if (!(nodeTypes[NumberOfNodes - 1]->IsDiscrete()))
  {
    PNL_THROW(CInconsistentType, "childNode must be discrete");
  }
  
  m_pLearnMatrixWeight = NULL;
  m_pMatrixWeight = NULL;
  m_LearnVectorOffset.resize(0);
  m_VectorOffset.resize(0);
  m_hessian = NULL;

  void *pObj = this;

  int childNodeSize = nodeTypes[NumberOfNodes - 1]->GetNodeSize();
  
  if (dataWeight)
  {
    int *ranges = new int [2];
    PNL_CHECK_IF_MEMORY_ALLOCATED(ranges);
    ranges[1] = childNodeSize;
    ranges[0] = NumberOfNodes - 1;

    m_pMatrixWeight = C2DNumericDenseMatrix<float>::Create(ranges, dataWeight);
    static_cast<CMatrix<float>*>(m_pMatrixWeight)->AddRef(pObj);
    delete [] ranges;
  }

  if (dataWeight)
  {
    m_VectorOffset.assign(dataOffset, dataOffset + childNodeSize);
  }
  m_MaximizingMethod = mmGradient;
	m_pMatrixH = NULL;
	m_pMatrixK = NULL;
}
// ----------------------------------------------------------------------------

//constructor for unit function distribution
CSoftMaxDistribFun::CSoftMaxDistribFun(int NumberOfNodes,
  const CNodeType *const*nodeTypes):
  CDistribFun(dtSoftMax, NumberOfNodes, nodeTypes, 1)
{
  PNL_CHECK_LEFT_BORDER(NumberOfNodes, 1);
  PNL_CHECK_IS_NULL_POINTER(nodeTypes);

  if (!(nodeTypes[NumberOfNodes - 1]->IsDiscrete()))
  {
    PNL_THROW(CInconsistentType, "childNode must be discrete");
  }

  m_pLearnMatrixWeight = NULL;
  m_pMatrixWeight = NULL;
  m_LearnVectorOffset.resize(0);
  m_VectorOffset.resize(0);
  m_hessian = NULL;
  m_MaximizingMethod = mmGradient;
	m_pMatrixH = NULL;
	m_pMatrixK = NULL;
}
// ----------------------------------------------------------------------------

//copy constructor
CSoftMaxDistribFun::CSoftMaxDistribFun(const CSoftMaxDistribFun & inpDistr)
  :CDistribFun(dtSoftMax)
{
  m_NumberOfNodes = inpDistr.m_NumberOfNodes;
  m_NodeTypes.assign(inpDistr.m_NodeTypes.begin(), inpDistr.m_NodeTypes.end());
  m_bUnitFunctionDistribution = inpDistr.m_bUnitFunctionDistribution;

  m_MaximizingMethod = inpDistr.m_MaximizingMethod;
  m_pLearnMatrixWeight = NULL;
  m_pMatrixWeight = NULL;
  m_LearnVectorOffset.resize(0);
  m_VectorOffset.resize(0);
  m_hessian = NULL;

  if (m_bUnitFunctionDistribution)
  {
    return;
  }
  void *pObj = this;
  
  if (inpDistr.m_pMatrixWeight)
  {
    m_pMatrixWeight = 
      C2DNumericDenseMatrix<float>::Copy(inpDistr.m_pMatrixWeight);
    static_cast<CMatrix<float>*>(m_pMatrixWeight)->AddRef(pObj);
  }

  m_VectorOffset = inpDistr.m_VectorOffset;
	m_pMatrixH = NULL;
	m_pMatrixK = NULL;
}
// ----------------------------------------------------------------------------

int CSoftMaxDistribFun::GetNumberOfFreeParameters() const
{
	int i; 
    intVector discrPar(0);
    intVector contPar(0);
    int numConf = 1;
    int nDiscr = 0;
    int nCont = 0;
    int nW = 0;
	
    for (i = 0; i < m_NumberOfNodes - 1; i++ )
    {
        if (m_NodeTypes[i]->IsDiscrete())
        {
            discrPar.push_back(i);
            numConf *= m_NodeTypes[i]->GetNodeSize();
        }
        else
            contPar.push_back(i);
    }
	
    for (i = 0; i < contPar.size(); i++ )
    {
        int ns = m_NodeTypes[contPar[i]]->GetNodeSize();
        nW += ns * m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
    }
	
    nCont = contPar.size() * (contPar.size() - 1) / 2; 
	
    //return nDiscr + nCont + nW + contPar.size() +1;
	return nDiscr + nCont + nW;

}
// ----------------------------------------------------------------------------

CSoftMaxDistribFun::~CSoftMaxDistribFun()
{
  void* pObj = this;
  if (m_pMatrixWeight)
  {
    static_cast<CMatrix<float>*>(m_pMatrixWeight)->Release(pObj);
    m_pMatrixWeight = NULL;
  }
  if (m_pLearnMatrixWeight)
  {
    static_cast<CMatrix<float>*>(m_pLearnMatrixWeight)->Release(pObj);
    m_pLearnMatrixWeight = NULL;
  }
  if (m_hessian)
  {
    static_cast<CMatrix<float>*>(m_hessian)->Release(pObj);
    m_hessian = NULL;
  }

	if (m_pMatrixH) 
	{
  	static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
    m_pMatrixH = NULL;
	}
		
	if (m_pMatrixK) 
	{
    static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
    m_pMatrixK = NULL;
	}
}
// ----------------------------------------------------------------------------

CDistribFun& CSoftMaxDistribFun::operator =(const CDistribFun& pInputDistr)
{
  if (this == &pInputDistr)
  {
    return *this;
  }
  if (pInputDistr.GetDistributionType() != dtSoftMax)
  {
    PNL_THROW(CInvalidOperation, 
      "input distribution must be SoftMax ");
  }
  //both distributions must be on the same node types
  const CSoftMaxDistribFun &pSMInputDistr = 
    static_cast<const CSoftMaxDistribFun&>(pInputDistr);
  int isTheSame = 1;
  if (m_NumberOfNodes != pSMInputDistr.m_NumberOfNodes)
  {
    isTheSame = 0;
  }
  int i;
  const pConstNodeTypeVector* ntInput = pInputDistr.GetNodeTypesVector();
  for(i = 0; i < m_NumberOfNodes; i++)
  {
    if ((*ntInput)[i] != m_NodeTypes[i])
    {
      isTheSame = 0;
      break;
    }
  }
  if (!isTheSame)
  {
    PNL_THROW(CInvalidOperation,
      "both distributions must be on the same nodes");
  }
  int sameUnit = (m_bUnitFunctionDistribution == 
    pSMInputDistr.m_bUnitFunctionDistribution)? 1:0;
  
  if (sameUnit)
  {
    if (m_bUnitFunctionDistribution) // both distribs are unit
    {
      return *this;
    }
    else // both distribs are not unit
    {
      (*m_pMatrixWeight) = *(pSMInputDistr.m_pMatrixWeight);
      m_VectorOffset = pSMInputDistr.m_VectorOffset;
      return *this;
    }
  }
  else
  {
    void *pObj = this;
    if (m_bUnitFunctionDistribution) //this distribution is unit, other - not
    {
      m_bUnitFunctionDistribution = 0;
      m_pMatrixWeight = static_cast<C2DNumericDenseMatrix<
        float>*>(pSMInputDistr.m_pMatrixWeight->Clone());
      m_pMatrixWeight->AddRef(pObj);
      m_VectorOffset = pSMInputDistr.m_VectorOffset;
    }
    else //the other distribution is unit, this - not
    {
      m_bUnitFunctionDistribution = 1;
      if (m_pMatrixWeight)
      {
        m_pMatrixWeight->Release(pObj);
        m_pMatrixWeight = NULL;
      }
      m_VectorOffset.clear();
    }
  }
  return *this;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::AllocMatrix(const float *data, EMatrixType mType, 
  int numberOfWeightMatrix, const int *parentIndices)
{
  PNL_CHECK_IS_NULL_POINTER(data);
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW(CInvalidOperation, 
      "uniform distribution have no matrices")
  }

  void *pObj = this;
  
  if (mType == matWeights)
  {
    //check number of matrix with node sizes and node types
    if (m_pMatrixWeight)
    {
      m_pMatrixWeight->Release(pObj);
      m_pMatrixWeight = NULL;
    }
    int* dims = new int[2];
    PNL_CHECK_IF_MEMORY_ALLOCATED(dims);

    dims[1] = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
    dims[0] = m_NumberOfNodes - 1;
    m_pMatrixWeight = C2DNumericDenseMatrix<float>::Create(dims, data);
    static_cast<CMatrix<float>*>(m_pMatrixWeight)->AddRef(pObj);
    delete [] dims;    
  }
  else
  {
    PNL_THROW(CInconsistentType, "matrix type");
  }
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::AttachMatrix(CMatrix<float>* pMatrix,
  EMatrixType mType, int numberOfWeightMatrix, const int *parentIndices,
  bool isMultipliedByDelta)
{
  PNL_CHECK_IS_NULL_POINTER (pMatrix);
  
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW(CInvalidOperation, 
      "uniform distribution have no matrices")
  }
  int numDims;
  const int *ranges;
  pMatrix->GetRanges(&numDims, &ranges);
  int childNodeSize = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();

  PNL_CHECK_FOR_NON_ZERO (numDims - 2);
  PNL_CHECK_FOR_NON_ZERO (ranges[1] - childNodeSize);
  PNL_CHECK_FOR_NON_ZERO (ranges[0] - (m_NumberOfNodes - 1));

  if (mType == matWeights)
  {
    void *pObj = this;
    if (m_pMatrixWeight)
    {
      static_cast<CMatrix<float>*>(m_pMatrixWeight)->Release(pObj);
      m_pMatrixWeight = NULL;
    }
    m_pMatrixWeight = static_cast<C2DNumericDenseMatrix<float>*>(pMatrix);
    static_cast<CMatrix<float>*>(m_pMatrixWeight)->AddRef(pObj);
  }
  else
  {
    PNL_THROW(CInconsistentType, "matrix type");
  }
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::AllocOffsetVector(const float *data)
{
  PNL_CHECK_IS_NULL_POINTER (data);
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW(CInvalidOperation, 
      "uniform distribution have no vectors")
  }

  int childNodeSize = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();

  m_VectorOffset.assign(data, data + childNodeSize);
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::AttachOffsetVector(const floatVector *pVector)
{
  PNL_CHECK_IS_NULL_POINTER (pVector);
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW(CInvalidOperation, 
      "uniform distribution have no vectors")
  }
  
  int childNodeSize = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  PNL_CHECK_FOR_NON_ZERO (pVector->size() - childNodeSize);

  m_VectorOffset = *pVector;
}
// ----------------------------------------------------------------------------

CNodeValues *CSoftMaxDistribFun::GetMPE()
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
  return NULL;
}
// ----------------------------------------------------------------------------

int CSoftMaxDistribFun::IsEqual(const CDistribFun *dataToCompare, 
  float epsilon, int withCoeff, float* maxDifference) const
{
  if (!dataToCompare)
  {
    PNL_THROW(CNULLPointer, "dataToCompare");
    return 0;
  }
  if (maxDifference)
  {
    *maxDifference = 0.0f;
  }
  if (dataToCompare->GetDistributionType() != dtSoftMax)
  {
    PNL_THROW(CInconsistentType, 
      "we can compare datas of the same type");
    return 0;
  }
  int i;
  const CSoftMaxDistribFun *data = (const CSoftMaxDistribFun*)dataToCompare;
  
  //we need to check node types
  if (m_NumberOfNodes != data->m_NumberOfNodes)
  {
    return 0;
  }
  const pConstNodeTypeVector* ntIn = dataToCompare->GetNodeTypesVector();
  for (i = 0; i < m_NumberOfNodes; i++)
  {
    if((*m_NodeTypes[i]) != (*(*ntIn)[i]))
    {
      return 0;
    }
  }
  
  int ret = 0;
  if (m_bUnitFunctionDistribution == 1)
  {
    if (dataToCompare->IsDistributionSpecific() == 1)
    {
      ret = 1;
    }
    else
    {
      //can check the matrix
      float wSum = data->m_pMatrixWeight->SumAll(1);
      float oSum = 0.0f;
      for (i = 0; i < data->m_VectorOffset.size(); i++)
      {
        oSum += data->m_VectorOffset[i];
      }
      ret = ((wSum < epsilon) && (oSum < epsilon)) ? 1 : 0;
      if(maxDifference && !ret)
      {
        *maxDifference = (wSum > oSum) ? wSum : oSum;
      }
    }
    return ret;
  }
  // now we can check if data is in unitFunction form & 
  // "this" usn't in this form
  if (data->m_bUnitFunctionDistribution)
  {
    if (!IsDistributionSpecific())
    {
      float wSum = m_pMatrixWeight->SumAll(1);
      float oSum = 0.0f;
      for (i = 0; i < m_VectorOffset.size(); i++)
      {
        oSum += m_VectorOffset[i];
      }
      ret = ((wSum < epsilon) && (oSum < epsilon)) ? 1 : 0;
      if(maxDifference && !ret)
      {
        *maxDifference = (wSum > oSum) ? wSum : oSum;
      }
    }
    else
    {
      ret = 1;
    }
    return ret;
  }
  
  C2DNumericDenseMatrix<float> *matDiff;
  matDiff= static_cast<C2DNumericDenseMatrix<float>*>
    (pnlCombineNumericMatrices(data->m_pMatrixWeight, m_pMatrixWeight, 0));
  
  int nDims;
  const int *ranges;
  matDiff->GetRanges(&nDims, &ranges);
  float diffWeight = matDiff->SumAll(1) / (ranges[0] * ranges[1]);
  delete matDiff;
  
  const floatVector::const_iterator it1 = (data->m_VectorOffset).begin();
  floatVector diffVec = m_VectorOffset;
  SumVector<float>(diffVec.begin(), it1, diffVec.size(), -1);
  float diffOffset = 0.0f;
  for (i = 0; i < diffVec.size(); i++)
  {
    diffOffset += diffVec[i];
  }
  diffOffset /= diffVec.size();
  
  ret = ((diffWeight < epsilon) && (diffOffset < epsilon)) ? 1:0;
  if (maxDifference && !ret)
  {
    *maxDifference = (diffWeight > diffOffset) ? diffWeight : diffOffset;
  }
  return ret;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::SumInSelfData(const int *pBigDomain,
  const int *pSmallDomain, const CDistribFun *pOtherData)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::MultiplyInSelfData(const int *pBigDomain, 
  const int *pSmallDomain, const CDistribFun *pOtherData)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::DivideInSelfData(const int *pBigDomain, 
  const int *pSmallDomain, const CDistribFun *pOtherData)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
}
// ----------------------------------------------------------------------------

CDistribFun* CSoftMaxDistribFun::GetNormalized() const
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
  return NULL;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::Normalize()
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
}
// ----------------------------------------------------------------------------

CMatrix<float>* CSoftMaxDistribFun::GetMatrix(EMatrixType mType, 
  int numWeightMat, const int *parentIndices) const
{
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW(CInvalidOperation, 
      "we haven't any matrices for uniform distribution - it is special case ");
  }
  switch (mType)
  {
  case matWeights:
    {
      return m_pMatrixWeight;
    }
  default:
    {
      PNL_THROW(CBadConst, "no competent type");
    }
  } 
}
// ----------------------------------------------------------------------------

floatVector* CSoftMaxDistribFun::GetOffsetVector()
{
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW(CInvalidOperation, 
      "we haven't offset vector for uniform distribution - it is special case ");
  }
  return &m_VectorOffset;
}
// ----------------------------------------------------------------------------

CMatrix<float> *CSoftMaxDistribFun::GetStatisticalMatrix(
  EStatisticalMatrix mType, int *parentIndices) const
{
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW(CInvalidOperation, 
      "we haven't any matrices for uniform distribution - it is special case ");
  }
  switch (mType)
  {
    case matWeights:
    {
      return m_pLearnMatrixWeight;
    }
    default:
    {
      PNL_THROW(CBadConst, "no competent type");
    }
  } 
}
// ----------------------------------------------------------------------------

floatVector* CSoftMaxDistribFun::GetStatisticalOffsetVector()
{
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW(CInvalidOperation, 
      "we haven't offset vector for uniform distribution - it is special case ");
  }
  return &m_LearnVectorOffset;
}
// ----------------------------------------------------------------------------

CDistribFun* CSoftMaxDistribFun::ConvertToSparse() const
{
  const CSoftMaxDistribFun* self = this;
  CSoftMaxDistribFun* resDistrib = Copy(self);

  return resDistrib;
}
// ----------------------------------------------------------------------------

CDistribFun* CSoftMaxDistribFun::ConvertToDense() const
{
  const CSoftMaxDistribFun* self = this;
  CSoftMaxDistribFun* resDistrib = Copy(self);

  return resDistrib;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::InitLearnData()
{
  void *pObj = this;
  if (m_pLearnMatrixWeight)
  {
    static_cast<CMatrix<float>*>(m_pLearnMatrixWeight)->Release(pObj);
    m_pLearnMatrixWeight = NULL;
  }
  m_pLearnMatrixWeight = C2DNumericDenseMatrix<float>::Copy(m_pMatrixWeight);
  static_cast<CMatrix<float>*>(m_pLearnMatrixWeight)->AddRef(pObj);

  m_LearnVectorOffset = m_VectorOffset;

  if (m_hessian)
  {
    static_cast<CMatrix<float>*>(m_hessian)->Release(pObj);
    m_hessian = NULL;
  }
  int childNodeSize = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  int range;
  
  int *ranges = new int [2];
  PNL_CHECK_IF_MEMORY_ALLOCATED(ranges);

  range = (childNodeSize - 1) * (m_NumberOfNodes);
  float *dataHes = new float [range * range];
  PNL_CHECK_IF_MEMORY_ALLOCATED(dataHes);

  ranges[0] = range;
  ranges[1] = range;
  m_hessian = C2DNumericDenseMatrix<float>::Create(ranges, dataHes);
  static_cast<CMatrix<float>*>(m_hessian)->AddRef(pObj);

  delete [] ranges;
  delete [] dataHes;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::CopyLearnDataToDistrib()
{
  void *pObj = this;
  if (m_pMatrixWeight)
  {
    static_cast<CMatrix<float>*>(m_pMatrixWeight)->Release(pObj);
    m_pMatrixWeight = NULL;
  }
  m_pMatrixWeight = C2DNumericDenseMatrix<float>::Copy(m_pLearnMatrixWeight);
  static_cast<CMatrix<float>*>(m_pMatrixWeight)->AddRef(pObj);

  m_VectorOffset = m_LearnVectorOffset;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::Dump() const
{
  Log dump("", eLOG_RESULT, eLOGSRV_PNL_POTENTIAL);
  
  dump << "I'm a SoftMax distribution function of " << m_NumberOfNodes << " nodes.\n";

  if (IsDistributionSpecific())
  {
    dump << "I'm a Uniform distribution, haven't any matrices.\n";
    return;
  }
  int i;
  dump << "My matrix Weight is:\n";
  const floatVector *myVector = static_cast<CNumericDenseMatrix<float>*>(
    m_pMatrixWeight)->GetVector();
  for(i = 0; i < myVector->size(); i++)
  {
    dump << (*myVector)[i] << " ";
  }
  dump << "\n";
  dump << "My vector offset is:\n";
  for(i = 0; i < m_VectorOffset.size(); i++)
  {
    dump << m_VectorOffset[i] << " ";
  }
  dump << "\n";
}
//-----------------------------------------------------------------------------
void CSoftMaxDistribFun::DumpMatrix(const CEvidence *pEvidence) 
{
    //It is very importaint, that pEvidence contains only observations 
    // in this domain
    CNumericDenseMatrix<float> *curMatrix = GetProbMatrix(pEvidence);
    const float *data;
    int size;
    curMatrix->GetRawData(&size,&data);
    int i;
    printf("\nProbMatrix values are:\n");
    for( i = 0; i < size; i++)
        printf(" %f ",data[i]);

}
// ----------------------------------------------------------------------------

int CSoftMaxDistribFun::IsSparse() const
{
  return 0;
}

// ----------------------------------------------------------------------------
int CSoftMaxDistribFun::IsDense() const
{
  return 1;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::MarginalizeData(const CDistribFun *pOldData,
  const int *VarsOfKeep, int NumVarsOfKeep, int maximize)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::ShrinkObservedNodes(const CDistribFun* pOldData, 
  const int *pVarsObserved, const Value* const* pObsValues,
  int numObsVars, const CNodeType* pObsTabNT, const CNodeType* pObsGauNT)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::ExpandData(const int* pDimsToExtend, 
  int numDimsToExpand, const Value* const* valuesArray, 
  const CNodeType* const *allFullNodeTypes, int UpdateCanonical)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::ClearStatisticalData()
{
  if (m_pLearnMatrixWeight)
  {
    m_pLearnMatrixWeight->ClearData();
  }
  
  if (m_LearnVectorOffset.size() > 0)
  {
    m_LearnVectorOffset.clear();
  }
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::UpdateStatisticsEM(const CDistribFun* infData,
  const CEvidence *pEvidence, float weightingCoeff, const int* domain)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::SetStatistics(const CMatrix<float> *pMat, 
  EStatisticalMatrix matrix, const int* parentsComb)
{
  PNL_CHECK_IS_NULL_POINTER(pMat);
  if (pMat->GetMatrixClass() == mc2DNumericDense )
  {
    PNL_THROW(CInconsistentType, "matrix should be 2D numeric dense");
  }
  switch(matrix) 
  {
    case stMatWeight:
    {
      if( !m_pLearnMatrixWeight )
      {
        int childNodeSize = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
        int ranges[2] = { childNodeSize, m_NumberOfNodes - 1 };
        int numEl;
        const float* data;
        static_cast<const C2DNumericDenseMatrix<float >*>(pMat)->
          GetRawData(&numEl, &data);
        PNL_CHECK_FOR_NON_ZERO(numEl - childNodeSize * (m_NumberOfNodes - 1));
        m_pLearnMatrixWeight = 
          C2DNumericDenseMatrix<float>::Create(ranges, data);
      }
      else
      {
        m_pLearnMatrixWeight->SetDataFromOtherMatrix(pMat);
      }
      
      break;
    }
    default: 
    {
      PNL_THROW(CBadArg, "number of matrix");
    }
  }
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::SetOffsetStatistics(const floatVector *pVec)
{
  PNL_CHECK_IS_NULL_POINTER(pVec);
  PNL_CHECK_FOR_NON_ZERO(pVec->size() - m_NodeTypes[m_NumberOfNodes - 1]->
    GetNodeSize());
  m_LearnVectorOffset = *pVec;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::UpdateStatisticsML(const CEvidence* const* pEvidences,
  int EvidenceNumber, const int *domain, float weightingCoeff)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");
}
// ----------------------------------------------------------------------------

float CSoftMaxDistribFun::ProcessingStatisticalData(float numEvidences)
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");

  return 0;
}
// ----------------------------------------------------------------------------

CDistribFun *CSoftMaxDistribFun::ConvertCPDDistribFunToPot() const
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");

  return NULL;
}
// ----------------------------------------------------------------------------

CDistribFun *CSoftMaxDistribFun::ConvertCPDDistribFunToPotential(floatVector MeanContParents, 
  C2DNumericDenseMatrix<float>* CovContParents, int r) 
{
  CGaussianDistribFun* factData = NULL;
  
  int i, j;
  int *multiindex = new int [2];
  
  int NumAllInweights = 0;
  for( i = 0; i < m_NumberOfNodes - 1; i++ )
  {
    NumAllInweights += m_NodeTypes[i]->GetNodeSize();
  }
  
  // ksi^2 = w'(sigma + mu*mu')w +2bw'mu + b^2
  
  float NewKsi = 0.0f;

  NewKsi = CalculateKsi( MeanContParents, CovContParents);
  float OldKsi = 0.0f;
  int iternum = 0;

  do
  {
    floatVector MeanVector;
    C2DNumericDenseMatrix<float> *CovMatrix = NULL;
    
    OldKsi = NewKsi;
    
    CalculateMeanAndCovariance(OldKsi, r, MeanVector, &CovMatrix);
    NewKsi = CalculateKsi( MeanVector, CovMatrix);
    iternum ++;

    delete CovMatrix;
  }
  while((fabs(NewKsi - OldKsi) > 0.001)&&(iternum < 10));
  
  float ksi = NewKsi;
  
  // canonical characteristics
  // g = log(sigma) + (1/2)(2r - 1)b - (1/2)ksi + lambda(b^2 - ksi^2);
  // H = (1/2)(2r-1)w + 2lambda*b*w
  // K = -2*lambda*w*w'
  
  float b = 0.0f;
  b = m_VectorOffset[1]-m_VectorOffset[0];
  
  float sigma = 1 / ( 1 + exp(-ksi));
  
  float lambda = ((0.5f) - sigma)/(2*ksi);
  m_g = log(sigma) + (0.5f)*(2*r - 1)*b - (0.5f)*ksi + lambda*(b*b - ksi*ksi);
  
  intVector ranges = intVector(2, NumAllInweights);
  ranges[1] = 1;
  floatVector ZeroData = floatVector( NumAllInweights, 0);
  
  C2DNumericDenseMatrix<float> *newMatWeights = 
    C2DNumericDenseMatrix<float>::Create( &ranges.front(), &ZeroData.front());
  float weight0, weight1;
  for (i = 0; i < NumAllInweights; i++ )
  {
    multiindex[0] = i;
    multiindex[1] = 0;
    weight0 = m_pMatrixWeight->GetElementByIndexes(multiindex);
    
    multiindex[1] = 1;
    weight1 = m_pMatrixWeight->GetElementByIndexes(multiindex);
    
    multiindex[1] = 0;
    newMatWeights->SetElementByIndexes(weight1-weight0, multiindex);
  }
  
	void *pObj = this;

	if (m_pMatrixH) 
	{
    static_cast<CMatrix<float>*>(m_pMatrixH)->Release(pObj);
    m_pMatrixH = NULL;
	}

  m_pMatrixH = 
    C2DNumericDenseMatrix<float>::Create(&ranges.front(), &ZeroData.front() );
  static_cast<CMatrix<float>*>(m_pMatrixH)->AddRef(pObj);

  float value;
  for (i = 0; i < ranges[0]; i++)
  {
    for (j = 0; j < ranges[1]; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      value = ((0.5f)*(2*r - 1) + 2*lambda*b)*
        (newMatWeights->GetElementByIndexes(multiindex));
      m_pMatrixH->SetElementByIndexes( value, multiindex);
    }
  }

  const floatVector *hvec = NULL;
  hvec = m_pMatrixH->GetVector();
  floatVector vecH = floatVector((*hvec).size());
  memcpy(&vecH[0], &(*hvec)[0], (*hvec).size()*sizeof(float));
  
  ranges[0] = NumAllInweights;
  ranges[1] = NumAllInweights;
  
	pObj = this;
	if (m_pMatrixK) 
	{
    static_cast<CMatrix<float>*>(m_pMatrixK)->Release(pObj);
    m_pMatrixK = NULL;
	}
  
  C2DNumericDenseMatrix<float> *newMatWeightsTrans = newMatWeights->Transpose();
  m_pMatrixK = pnlMultiply( newMatWeights, newMatWeightsTrans, 0 );
  static_cast<CMatrix<float>*>(m_pMatrixK)->AddRef(pObj);
  
  for (i = 0; i < ranges[0]; i++)
  {
    for (j = 0; j < ranges[1]; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      value = m_pMatrixK->GetElementByIndexes(multiindex);
      m_pMatrixK->SetElementByIndexes( -2*lambda*value, multiindex);
    }
  }

  const floatVector *kvec = NULL;
  kvec = m_pMatrixK->GetVector();
  floatVector vecK = floatVector((*kvec).size());
  memcpy(&vecK[0], &(*kvec)[0], (*kvec).size()*sizeof(float));

  pConstNodeTypeVector *NodeTypes = new pConstNodeTypeVector();
  for (i = 0; i < m_NumberOfNodes - 1; i++)
  {   
    const CNodeType *newType = m_NodeTypes[i];
    NodeTypes->push_back(newType);
  }
  const CNodeType *newType = &m_ZeroNT;
  NodeTypes->push_back(newType);
  
  factData = CGaussianDistribFun::CreateInCanonicalForm(m_NumberOfNodes, 
    &NodeTypes->front(), &vecH.front(), &vecK.front(), m_g );
  
  delete [] multiindex;
  delete newMatWeights;
  delete newMatWeightsTrans;
  delete NodeTypes;
  return factData;
}
// ----------------------------------------------------------------------------

CDistribFun* CSoftMaxDistribFun::CPD_to_pi(CDistribFun *const*allPiMessages, 
  int *multParentIndices, int numMultNodes, int posOfExceptParent, 
  int maximizeFlag) const
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");

  return NULL;
}
// ----------------------------------------------------------------------------

CDistribFun* CSoftMaxDistribFun::CPD_to_lambda(const CDistribFun *lambda, 
  CDistribFun *const* allPiMessages, int *multParentIndices, 
  int numNodes, int posOfExceptNode, int maximizeFlag)const
{
  PNL_THROW(CNotImplemented, "haven't for CSoftMaxDistribFun now");

  return NULL;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::SetMaximizingMethod(EMaximizingMethod met)
{
  m_MaximizingMethod = met;
}
// ----------------------------------------------------------------------------

EMaximizingMethod CSoftMaxDistribFun::GetMaximizingMethod()
{
  return m_MaximizingMethod;
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::CalculateHessian(float ** pContVectorEvidence, 
  int NumberOfObservations)
{
  int i, j, e, k, l, w, z;
  int NumOfStates = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  int NumOfContinousParents = m_NumberOfNodes-1;
  int * multiindex =  new int [2];
  int * mindex     =  new int [2];
  for ( k = 0; k < NumOfStates - 1; k++ )
  {
    for ( l = 0; l < NumOfStates - 1; l++ )
    {
      for ( i = 0; i < NumOfContinousParents+1; i++ )
      {
        for ( j = 0; j < NumOfContinousParents+1; j++ )
        {
          multiindex[0] = k * (NumOfContinousParents +1) + i;
          multiindex[1] = l * (NumOfContinousParents +1) + j;
          float tempSum = 0;
          float temp = 0;
          float globSum = 0;
          float globTempSum = 0;
          for ( e = 0; e < NumberOfObservations; e++ )
          {
            tempSum = 0;
            for ( z = 0; z < NumOfStates - 1; z++ )
            {
              temp = 0;
              for ( w = 0; w < NumOfContinousParents; w++ )
              {
                mindex[0] = w;
                mindex[1] = z;
                temp += m_pMatrixWeight->GetElementByIndexes(mindex)
                  *(pContVectorEvidence[w][e]);
              }
              temp += m_VectorOffset[z];
              tempSum += exp(temp);
            }
            tempSum += 1;
            temp = 0;
            for (z = 0; z < NumOfContinousParents; z++)
            {
              mindex[0] = z;
              mindex[1] = k;
              temp += m_pMatrixWeight->GetElementByIndexes(mindex)
                *(pContVectorEvidence[z][e]);
            }
            temp += m_VectorOffset[k];
            globSum = (exp(temp)) / tempSum;
            temp = 0;
            for (z = 0; z < NumOfContinousParents; z++)
            {
              mindex[0] = z;
              mindex[1] = l;
              temp += m_pMatrixWeight->GetElementByIndexes(mindex)*
                (pContVectorEvidence[z][e]);
            }
            temp += m_VectorOffset[l];
            if (k == l)
              globSum *= (1.0 - (exp(temp)) / tempSum);
            else 
              globSum *= -(exp(temp))/tempSum;
            if ((i == (NumOfContinousParents)) && 
              (j == (NumOfContinousParents)))
            {
              globTempSum += -globSum;
            }
            else
              if (i == (NumOfContinousParents))
              {
                globTempSum += -globSum*(pContVectorEvidence[j][e]);
              }
              else
                if (j == (NumOfContinousParents))
                {
                  globTempSum += -globSum*(pContVectorEvidence[i][e]);
                }
                else
                  globTempSum += -globSum*(pContVectorEvidence[j][e]) *
                    (pContVectorEvidence[i][e]);

          }
          m_hessian->SetElementByIndexes(globTempSum, multiindex);
        }
      }
    }
  }
  m_hessian->Inverse();
  delete [] mindex,multiindex;
}
//-----------------------------------------------------------------------------

float CSoftMaxDistribFun::CalculateLikelihood(float **Observations, 
  int NumOfObservations)
{
  // quantity of SoftMax node states
  int NumOfStates = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  int NumOfContinousParents = m_NumberOfNodes - 1;

  float res = 0;
  float temp = 0;
  float tempSum;
  int ind;
  int *multiindex = new int [2];
  for (int i = 0; i < NumOfObservations; i++)
  {
    // numerator
    ind = int(Observations[NumOfContinousParents][i]);
    temp = m_LearnVectorOffset[ind];
    for (int j = 0; j < NumOfContinousParents; j++)
    {
      multiindex[0] = j;
      multiindex[1] = ind;
      temp += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
        * Observations[j][i];
    }
    
    // denominator
    tempSum = 1;
    float t;
    for (int l = 0; l < NumOfStates - 1; l++)
    {
      t = m_LearnVectorOffset[l];
      for (int j = 0; j < NumOfContinousParents; j++)
      {
        multiindex[0] = j;
        multiindex[1] = l;
        t += m_pLearnMatrixWeight->GetElementByIndexes(multiindex) * 
          Observations[j][i];
      }
      tempSum += exp(t);
    }
    res += temp - log(double(tempSum));
  }
  delete [] multiindex;
  return res;
}
//-----------------------------------------------------------------------------

void CSoftMaxDistribFun::MaximumLikelihood(float **Observation,
  int NumberOfObservations, float Accuracy, float step)
{
  switch (m_MaximizingMethod)
  {
    case mmGradient:
    {
      MaximumLikelihoodGradient(Observation, NumberOfObservations, Accuracy,
        step);
      break;
    }
    case mmHessian:
    {
      PNL_THROW(CNotImplemented, "Hessian is not available in this version");
//      MaximumLikelihoodHessian(Observation, NumberOfObservations, Accuracy,
//        step);
      break;
    }
    case mmConjGradient:
    {
      MaximumLikelihoodConjugateGradient(Observation, NumberOfObservations, 
        Accuracy, step);
      break;
    }
    default:
    {
      MaximumLikelihoodGradient(Observation, NumberOfObservations, Accuracy,
        step);
    }
  }
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::MaximumLikelihoodGradient(float **Observations,
  int NumOfObservations, float Accuracy, float step)
{
  float start_step = step;
  int iternum = -1;
  float temp;
  float tempSum;
  int a, b, i, j, l, w;
  float temp_1;
  float OldLikelihood; // old likelihood
  float NewLikelihood; // new likelihood
  int *multiindex = new int [2];
  float t;

  int NumOfStates = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  // quantity of SoftMax node states
  int NumOfContinousParents = m_NumberOfNodes - 1;
  
#ifdef SM_TEST
  printf("\nMatrix of Weights\n");
  for (i = 0; i< NumOfContinousParents; i++)
  {
    for (j = 0; j < NumOfStates; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      printf("%f   ", m_pLearnMatrixWeight->GetElementByIndexes(multiindex));
    }
    printf("\n");
  }
  printf ("Vector Offset:\n");
  for (i = 0; i < NumOfStates; i++)
    printf("%f   ", m_LearnVectorOffset[i]);
  printf("\n");
#endif
  
  for (i = 0; i < NumOfStates - 1; i++)
  {
    m_LearnVectorOffset[i] -= m_LearnVectorOffset[NumOfStates - 1];
    for (j = 0; j < NumOfContinousParents; j++)
    {
      multiindex[0] = j;
      multiindex[1] = i;
      t = m_pLearnMatrixWeight->GetElementByIndexes(multiindex);
      multiindex[1] = NumOfStates - 1;
      t -= m_pLearnMatrixWeight->GetElementByIndexes(multiindex);
      multiindex[1] = i;
      m_pLearnMatrixWeight->SetElementByIndexes(t, multiindex);
    }
  }
  m_LearnVectorOffset[NumOfStates - 1] = 0;
  for (j = 0; j < NumOfContinousParents; j++)
  {
    multiindex[0] = j;
    multiindex[1] = NumOfStates - 1;
    m_pLearnMatrixWeight->SetElementByIndexes(0, multiindex);
  }

#ifdef SM_TEST
  printf("\nMatrix of Weights\n");
  for (i = 0; i < NumOfContinousParents; i++)
  {
    for (j = 0; j < NumOfStates; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      printf("%f   ", m_pLearnMatrixWeight->GetElementByIndexes(multiindex));
    }
    printf("\n");
  }
  printf ("Vector Offset:\n");
  for (i = 0; i < NumOfStates; i++)
    printf("%f   ", m_LearnVectorOffset[i]);
  printf("\n");

/*  printf("\nMatrix of Observations\n");
  for (i = 0; i < NumOfContinousParents+1; i++)
  {
    for (j = 0; j < NumOfObservations; j++)
    {
      printf("%f   ", Observations[i][j]);
    }
    printf("\n");
  }*/
#endif

  float *grad_offset = new float [NumOfStates - 1];
  float **grad_weight = new float* [NumOfContinousParents];
  for (i = 0; i < NumOfContinousParents; i++)
  {
    grad_weight[i] = new float [NumOfStates - 1];
  }
  
  //(NumberOfObservations) X (NumberOfNodes) - size of matrix of Observations
  
  NewLikelihood = CalculateLikelihood(Observations, NumOfObservations);

#ifdef SM_TEST
  printf("OLD lIKELIHOOD %g\n", exp(NewLikelihood));
#endif

  do
  {
    iternum++;
    OldLikelihood = NewLikelihood;
    
    // Offset vector updating
    for (w = 0; w < NumOfStates - 1; w++)
    {
      grad_offset[w] = 0;
      for (i = 0; i < NumOfObservations; i++)
      {
        if (int(Observations[NumOfContinousParents][i]) == w)
          grad_offset[w]++;
        temp = m_LearnVectorOffset[w];
        for (j = 0; j < NumOfContinousParents; j++)
        {
          multiindex[0] = j;
          multiindex[1] = w;
          temp += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
            * Observations[j][i];
        }
        tempSum = 1;
        for (l = 0; l < NumOfStates - 1; l++)
        {
          t = m_LearnVectorOffset[l];
          for (j = 0; j < NumOfContinousParents; j++)
          {
            multiindex[0] = j;
            multiindex[1] = l;
            t += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
              * Observations[j][i];
          }
          tempSum += exp(t);
        }
        grad_offset[w] -= exp(temp) / tempSum;
      }
    }
    
    // Weight matrix updating
    for (a = 0; a < NumOfContinousParents; a++)
    {
      for (b = 0; b < (NumOfStates - 1); b++)
      {
        grad_weight[a][b] = 0;
        for (i = 0; i < NumOfObservations; i++)
        {
          if (int(Observations[NumOfContinousParents][i]) == b) 
            grad_weight[a][b] += Observations[a][i];
        }
        for (i = 0; i < NumOfObservations; i++)
        {
          temp = m_LearnVectorOffset[b];
          for (j = 0; j < NumOfContinousParents; j++)
          {
            multiindex[0] = j;
            multiindex[1] = b;
            temp += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
              * Observations[j][i];
          }
          tempSum = 1;
          for (l = 0; l < NumOfStates - 1; l++)
          {
            t = m_LearnVectorOffset[l];
            for (j = 0; j < NumOfContinousParents; j++)
            {
              multiindex[0] = j;
              multiindex[1] = l;
              t += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
                * Observations[j][i];
            }
            tempSum += exp(t);
          }
          grad_weight[a][b] -= Observations[a][i] * exp(temp) / tempSum;
        }
      }
    }

    while (fabs(step) > 0.0000001)
    {
      for (i = 0; i < NumOfContinousParents; i++)
      {
        for (j = 0; j < NumOfStates - 1; j++)
        {
          multiindex[0] = i;
          multiindex[1] = j;
          temp_1 = m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
            + step * grad_weight[i][j];
          m_pLearnMatrixWeight->SetElementByIndexes(temp_1, multiindex);
        }
      }
      for (i = 0; i < NumOfStates - 1; i++)
      {
        m_LearnVectorOffset[i] += step * grad_offset[i];
      }
      
      NewLikelihood = CalculateLikelihood(Observations, NumOfObservations);
      
      if (NewLikelihood < OldLikelihood)
      {
        if (step > 0)
          step = -step / 2;
        else
          step = step / 2;
      }
      else
      {
        if (step < 0)
          step = start_step;
        break;
      }
    }
  }
  while (((fabs((OldLikelihood - NewLikelihood) * 2 / 
    (OldLikelihood + NewLikelihood)) > Accuracy)) && (iternum < 1000));

  delete [] grad_offset;
  for (i = 0; i < NumOfContinousParents; i++)
  {
    delete [] grad_weight[i];
  }
  delete [] grad_weight;
  delete [] multiindex;

#ifdef SM_TEST
  printf("NEW LIKELIHOOD %g\n", exp(NewLikelihood));
  printf("\nNum of iterations is %d\n", iternum+1);
#endif
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::MaximumLikelihoodHessian(float ** Observations, 
  int NumberOfObservations, float Accuracy, float step)
{
  int iternum = -1;
  float temp;
  float tempSum;
  int a, b, i, j, l, w;
  float OldLikelihood; // old likelihood
  float NewLikelihood; // new likelihood
  int *multiindex = new int [2];
  float t;

  int NumOfStates = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  // quantity of SoftMax node states
  int NumOfContinousParents = m_NumberOfNodes - 1;
  
#ifdef SM_TEST
  printf("\nMatrix of Weights\n");
  for (i = 0; i< NumOfContinousParents; i++)
  {
    for (j = 0; j < NumOfStates; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      printf("%f   ", m_pLearnMatrixWeight->GetElementByIndexes(multiindex));
    }
    printf("\n");
  }
  printf ("Vector Offset:\n");
  for (i = 0; i < NumOfStates; i++)
    printf("%f   ", m_LearnVectorOffset[i]);
  printf("\n");
#endif
  
  for (i = 0; i < NumOfStates - 1; i++)
  {
    m_LearnVectorOffset[i] -= m_LearnVectorOffset[NumOfStates - 1];
    for (j = 0; j < NumOfContinousParents; j++)
    {
      multiindex[0] = j;
      multiindex[1] = i;
      t = m_pLearnMatrixWeight->GetElementByIndexes(multiindex);
      multiindex[1] = NumOfStates - 1;
      t -= m_pLearnMatrixWeight->GetElementByIndexes(multiindex);
      multiindex[1] = i;
      m_pLearnMatrixWeight->SetElementByIndexes(t, multiindex);
    }
  }
  m_LearnVectorOffset[NumOfStates - 1] = 0;
  for (j = 0; j < NumOfContinousParents; j++)
  {
    multiindex[0] = j;
    multiindex[1] = NumOfStates - 1;
    m_pLearnMatrixWeight->SetElementByIndexes(0, multiindex);
  }

#ifdef SM_TEST
  printf("\nMatrix of Weights\n");
  for (i = 0; i < NumOfContinousParents; i++)
  {
    for (j = 0; j < NumOfStates; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      printf("%f   ", m_pLearnMatrixWeight->GetElementByIndexes(multiindex));
    }
    printf("\n");
  }
  printf("Vector Offset:\n");
  for (i = 0; i < NumOfStates; i++)
  {
    printf("%f   ", m_LearnVectorOffset[i]);
  }
  printf("\n");
#endif

  float *grad_offset = new float [NumOfStates - 1];
  float **grad_weight = new float* [NumOfContinousParents];
 
  for (i = 0; i < NumOfContinousParents; i++)
  {
    grad_weight[i] = new float [NumOfStates - 1];
  }

  float ** full_grad = new float * [NumOfContinousParents + 1];
  for (i=0; i<NumOfContinousParents+1; i++)
  {
    full_grad[i] = new float [NumOfStates - 1];
  }

  float ** full_matrix = new float * [NumOfContinousParents + 1];
  for (i = 0; i < NumOfContinousParents + 1; i++)
  {
    full_matrix[i] = new float [NumOfStates - 1];
  }
    

  //(NumberOfObservations) X (NumberOfNodes) - size of matrix of Observations
  
  NewLikelihood = CalculateLikelihood(Observations, NumberOfObservations);
  printf("OLD lIKELIHOOD %g\n", exp(NewLikelihood));

  do
  {
    iternum++;
    OldLikelihood = NewLikelihood;
    
    // Offset vector updating
    for (w = 0; w < NumOfStates - 1; w++)
    {
      grad_offset[w] = 0;
      for (i = 0; i < NumberOfObservations; i++)
      {
        if (int(Observations[NumOfContinousParents][i]) == w)
          grad_offset[w]++;
        temp = m_LearnVectorOffset[w];
        for (j = 0; j < NumOfContinousParents; j++)
        {
          multiindex[0] = j;
          multiindex[1] = w;
          temp += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
            * Observations[j][i];
        }
        tempSum = 1;
        for (l = 0; l < NumOfStates - 1; l++)
        {
          t = m_LearnVectorOffset[l];
          for (j = 0; j < NumOfContinousParents; j++)
          {
            multiindex[0] = j;
            multiindex[1] = l;
            t += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
              * Observations[j][i];
          }
          tempSum += exp(t);
        }
        grad_offset[w] -= exp(temp) / tempSum;
      }
    }
    
    // Weight matrix updating
    for (a = 0; a < NumOfContinousParents; a++)
    {
      for (b = 0; b < (NumOfStates - 1); b++)
      {
        grad_weight[a][b] = 0;
        for (i = 0; i < NumberOfObservations; i++)
        {
          if (int(Observations[NumOfContinousParents][i]) == b) 
            grad_weight[a][b] += Observations[a][i];
        }
        for (i = 0; i < NumberOfObservations; i++)
        {
          temp = m_LearnVectorOffset[b];
          for (j = 0; j < NumOfContinousParents; j++)
          {
            multiindex[0] = j;
            multiindex[1] = b;
            temp += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
              * Observations[j][i];
          }
          tempSum = 1;
          for (l = 0; l < NumOfStates - 1; l++)
          {
            t = m_LearnVectorOffset[l];
            for (j = 0; j < NumOfContinousParents; j++)
            {
              multiindex[0] = j;
              multiindex[1] = l;
              t += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
                * Observations[j][i];
            }
            tempSum += exp(t);
          }
          grad_weight[a][b] -= Observations[a][i] * exp(temp) / tempSum;
        }
      }
    }

    CalculateHessian(Observations, NumberOfObservations);
    
    for (i = 0; i<NumOfContinousParents; i++)
    {
      for (j = 0; j<NumOfStates-1; j++)
      {
        full_grad[i][j] = grad_weight[i][j];
      }
    }
    for (i = 0; i < NumOfStates-1; i++)
    {
      full_grad[NumOfContinousParents][i] = grad_offset[i];
    }

    for (i = 0; i < NumOfContinousParents; i++)
    {
      for (j = 0; j < NumOfStates-1; j++)
      {
        multiindex[0] = i;
        multiindex[1] = j;
        full_matrix[i][j] = 
          m_pLearnMatrixWeight->GetElementByIndexes(multiindex);
      }
    }
    for (i = 0; i < NumOfStates-1; i++)
    {
      full_matrix[NumOfContinousParents][i] = m_LearnVectorOffset[i];
    }

    for (i = 0; i < NumOfContinousParents +1; i++)
      {
        for (j = 0; j < NumOfStates - 1; j++)
        {
          for (int a = 0; a < (NumOfContinousParents + 1) * (NumOfStates - 1);
            a++)
          {
            multiindex[0] = j*(NumOfContinousParents+1) + i;
            multiindex[1] = a;
            full_matrix[i][j] -= step * 
              m_hessian->GetElementByIndexes(multiindex) * 
              full_grad[a % (NumOfContinousParents + 1)][
              int(a / (NumOfContinousParents + 1))];
          }
        }
      }

    for (i = 0; i < NumOfContinousParents; i++)
    {
      for (j = 0; j < NumOfStates - 1; j++)
      {
        multiindex[0] = i;
        multiindex[1] = j;
        m_pLearnMatrixWeight->SetElementByIndexes(full_matrix[i][j],
          multiindex);
      }
    }
    for (i = 0; i < NumOfStates - 1; i++)
    {
      m_LearnVectorOffset[i] = full_matrix[NumOfContinousParents][i];
    }

    NewLikelihood = CalculateLikelihood(Observations, NumberOfObservations);
  }
  while (((fabs((OldLikelihood - NewLikelihood) * 2 / 
    (OldLikelihood + NewLikelihood)) > Accuracy)) && (iternum < 10000));

  delete [] grad_offset;
  for (i = 0; i < NumOfContinousParents; i++)
  {
    delete [] grad_weight[i];
  }
  for (i=0; i<NumOfContinousParents+1; i++)
  {
    delete [] full_grad[i];
  }
  delete [] full_grad;
  for (i=0; i<NumOfContinousParents+1; i++)
  {
    delete [] full_matrix[i];
  }
  delete [] full_matrix;

  delete [] grad_weight;
  delete [] multiindex;

#ifdef SM_TEST
  printf("NEW LIKELIHOOD %g\n", exp(NewLikelihood));
  printf("\nNum of iterations is %d\n", iternum + 1);
#endif
}
// ----------------------------------------------------------------------------

void CSoftMaxDistribFun::MaximumLikelihoodConjugateGradient(
  float **Observations, int NumOfObservations, float Accuracy, float step)
{
  float start_step = step;
  int iternum = -1;
  float temp;
  float tempSum;
  int a, b, i, j, l, w;
  float temp_1;
  float OldLikelihood; // old likelihood
  float NewLikelihood; // new likelihood
  int *multiindex = new int [2];
  float t;

  int NumOfStates = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  // quantity of SoftMax node states
  int NumOfContinousParents = m_NumberOfNodes - 1;
  
#ifdef SM_TEST
  printf("\nMatrix of Weights\n");
  for (i = 0; i< NumOfContinousParents; i++)
  {
    for (j = 0; j < NumOfStates; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      printf("%f   ", m_pLearnMatrixWeight->GetElementByIndexes(multiindex));
    }
    printf("\n");
  }
  printf ("Vector Offset:\n");
  for (i = 0; i < NumOfStates; i++)
    printf("%f   ", m_LearnVectorOffset[i]);
  printf("\n");
#endif
  
  for (i = 0; i < NumOfStates - 1; i++)
  {
    m_LearnVectorOffset[i] -= m_LearnVectorOffset[NumOfStates - 1];
    for (j = 0; j < NumOfContinousParents; j++)
    {
      multiindex[0] = j;
      multiindex[1] = i;
      t = m_pLearnMatrixWeight->GetElementByIndexes(multiindex);
      multiindex[1] = NumOfStates - 1;
      t -= m_pLearnMatrixWeight->GetElementByIndexes(multiindex);
      multiindex[1] = i;
      m_pLearnMatrixWeight->SetElementByIndexes(t, multiindex);
    }
  }
  m_LearnVectorOffset[NumOfStates - 1] = 0;
  for (j = 0; j < NumOfContinousParents; j++)
  {
    multiindex[0] = j;
    multiindex[1] = NumOfStates - 1;
    m_pLearnMatrixWeight->SetElementByIndexes(0, multiindex);
  }

#ifdef SM_TEST
  printf("\nMatrix of Weights\n");
  for (i = 0; i < NumOfContinousParents; i++)
  {
    for (j = 0; j < NumOfStates; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      printf("%f   ", m_pLearnMatrixWeight->GetElementByIndexes(multiindex));
    }
    printf("\n");
  }
  printf ("Vector Offset:\n");
  for (i = 0; i < NumOfStates; i++)
    printf("%f   ", m_LearnVectorOffset[i]);
  printf("\n");

/*  printf("\nMatrix of Observations\n");
  for (i = 0; i < NumOfContinousParents+1; i++)
  {
    for (j = 0; j < NumOfObservations; j++)
    {
      printf("%f   ", Observations[i][j]);
    }
    printf("\n");
  }*/
#endif

  float *grad_offset = new float [NumOfStates - 1];
  float **grad_weight = new float* [NumOfContinousParents];
  for (i = 0; i < NumOfContinousParents; i++)
  {
    grad_weight[i] = new float [NumOfStates - 1];
  }
  
  float *OldGradOffset = new float [NumOfStates - 1];
  float **OldGradWeight = new float* [NumOfContinousParents];
  for (i = 0; i < NumOfContinousParents; i++)
  {
    OldGradWeight[i] = new float [NumOfStates - 1];
  }

  //(NumberOfObservations) X (NumberOfNodes) - size of matrix of Observations
  
  NewLikelihood = CalculateLikelihood(Observations, NumOfObservations);
  printf("OLD lIKELIHOOD %g\n", exp(NewLikelihood));

  do
  {
    iternum++;
    OldLikelihood = NewLikelihood;
    
    // Offset vector updating
    for (w = 0; w < NumOfStates - 1; w++)
    {
      grad_offset[w] = 0;
      for (i = 0; i < NumOfObservations; i++)
      {
        if (int(Observations[NumOfContinousParents][i]) == w)
          grad_offset[w]++;
        temp = m_LearnVectorOffset[w];
        for (j = 0; j < NumOfContinousParents; j++)
        {
          multiindex[0] = j;
          multiindex[1] = w;
          temp += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
            * Observations[j][i];
        }
        tempSum = 1;
        for (l = 0; l < NumOfStates - 1; l++)
        {
          t = m_LearnVectorOffset[l];
          for (j = 0; j < NumOfContinousParents; j++)
          {
            multiindex[0] = j;
            multiindex[1] = l;
            t += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
              * Observations[j][i];
          }
          tempSum += exp(t);
        }
        grad_offset[w] -= exp(temp) / tempSum;
      }
    }
    
    // Weight matrix updating
    for (a = 0; a < NumOfContinousParents; a++)
    {
      for (b = 0; b < (NumOfStates - 1); b++)
      {
        grad_weight[a][b] = 0;
        for (i = 0; i < NumOfObservations; i++)
        {
          if (int(Observations[NumOfContinousParents][i]) == b) 
            grad_weight[a][b] += Observations[a][i];
        }
        for (i = 0; i < NumOfObservations; i++)
        {
          temp = m_LearnVectorOffset[b];
          for (j = 0; j < NumOfContinousParents; j++)
          {
            multiindex[0] = j;
            multiindex[1] = b;
            temp += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
              * Observations[j][i];
          }
          tempSum = 1;
          for (l = 0; l < NumOfStates - 1; l++)
          {
            t = m_LearnVectorOffset[l];
            for (j = 0; j < NumOfContinousParents; j++)
            {
              multiindex[0] = j;
              multiindex[1] = l;
              t += m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
                * Observations[j][i];
            }
            tempSum += exp(t);
          }
          grad_weight[a][b] -= Observations[a][i] * exp(temp) / tempSum;
        }
      }
    }

    while (fabs(step) > 0.0000001)
    {
      if (iternum == 0)
      {
        for (i = 0; i < NumOfContinousParents; i++)
        {
          for (j = 0; j < NumOfStates - 1; j++)
          {
            multiindex[0] = i;
            multiindex[1] = j;
            temp_1 = m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
              + step * grad_weight[i][j];
            m_pLearnMatrixWeight->SetElementByIndexes(temp_1, multiindex);
          }
        }
        for (i = 0; i < NumOfStates - 1; i++)
        {
          m_LearnVectorOffset[i] += step * grad_offset[i];
        }
      }
      else
      {
        float res1 = CalculateNorm(grad_weight, grad_offset);
        float res2 = CalculateNorm(OldGradWeight, OldGradOffset);
        float beta = -(res1/res2);
        for (i = 0; i < NumOfContinousParents; i++)
        {
          for (j = 0; j < NumOfStates-1; j++)
          {
            grad_weight[i][j] -= beta * OldGradWeight[i][j];
          }
        }
        for (i = 0; i < NumOfStates - 1; i++)
        {
          grad_offset[i] -= beta * OldGradOffset[i];
        }

        for (i = 0; i < NumOfContinousParents; i++)
        {
          for (j = 0; j < NumOfStates - 1; j++)
          {
            multiindex[0] = i;
            multiindex[1] = j;
            temp_1 = m_pLearnMatrixWeight->GetElementByIndexes(multiindex)
              + step * grad_weight[i][j];
            m_pLearnMatrixWeight->SetElementByIndexes(temp_1, multiindex);
          }
        }
        for (i = 0; i < NumOfStates - 1; i++)
        {
          m_LearnVectorOffset[i] += step * grad_offset[i];
        }
      }
      
      NewLikelihood = CalculateLikelihood(Observations, NumOfObservations);
      
      if (NewLikelihood < OldLikelihood)
      {
        if (step > 0)
          step = -step / 2;
        else
          step = step / 2;
      }
      else
      {
        if (step < 0)
          step = start_step;
        break;
      }
    }
  
    for (i = 0; i < NumOfStates-1; i++)
    {
      OldGradOffset[i] = grad_offset[i];
      for (j = 0; j < NumOfContinousParents; j++)
      {
        OldGradWeight[j][i] = grad_weight[j][i];
      }
    }

  }
  while (((fabs((OldLikelihood - NewLikelihood) * 2 / 
    (OldLikelihood + NewLikelihood)) > Accuracy)) && (iternum < 100000));

  delete [] grad_offset;
  for (i = 0; i < NumOfContinousParents; i++)
  {
    delete [] grad_weight[i];
  }
  delete [] grad_weight;

  delete [] OldGradOffset;
  for (i = 0; i < NumOfContinousParents; i++)
  {
    delete [] OldGradWeight[i];
  }
  delete [] OldGradWeight;

  delete [] multiindex;

#ifdef SM_TEST
  printf("NEW LIKELIHOOD %g\n", exp(NewLikelihood));
  printf("\nNum of iterations is %d\n", iternum + 1);
#endif
}
// ----------------------------------------------------------------------------

float CSoftMaxDistribFun::CalculateNorm(float ** grad_weights, 
  float * grad_offset)
{
  int i;
  float result = 0;
  int NumOfStates = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  // quantity of SoftMax node states
  int NumOfContinousParents = m_NumberOfNodes - 1;

  for ( i = 0; i < NumOfContinousParents; i++ )
  {
    for (int j = 0; j < NumOfStates - 1; j++)
    {
      result += pow(grad_weights[i][j], 2);
    }
  }
  for ( i = 0; i < NumOfStates - 1; i++ )
  {
    result += pow(grad_offset[i], 2);
  }
  return result;
}
//-----------------------------------------------------------------------------
CNumericDenseMatrix<float>* CSoftMaxDistribFun:: GetProbMatrix(const CEvidence *pEvidence)
{
  int NumOfStates = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
  int numOfNodes;
  int *mIOfMatrWeight;
  float norm = 0;
  float scal = 0;
  float curScal = 0;
  float *probValues;
  intVector pObsNodes;
  pConstValueVector pObsValues;
  pConstNodeTypeVector pNodeTypes;
  pEvidence->GetObsNodesWithValues(&pObsNodes,&pObsValues,&pNodeTypes);
  intVector SetOfContNodes;
  floatVector SetofContNodesValues;
  int i;
  int j;
  for(i = 0;i < pObsNodes.size(); i++)
  {
    if( !((pNodeTypes[i])->IsDiscrete()) )
    {
      SetOfContNodes.push_back(pObsNodes[i]);
      SetofContNodesValues.push_back(pObsValues[i]->GetFlt());
    };
  };
  numOfNodes = SetOfContNodes.size();
  mIOfMatrWeight = new int[2];
  probValues = new float[NumOfStates];
  float temp;
  for(i = 0; i < NumOfStates; i++)
  {   
    scal = 0; 
    for(j = 0; j < numOfNodes; j++)
    {
      mIOfMatrWeight[1] = i;
      mIOfMatrWeight[0] = j;
      temp = (m_pMatrixWeight->GetElementByIndexes(mIOfMatrWeight))*SetofContNodesValues[j];
      scal += temp;
    };
    norm += exp(scal+m_VectorOffset[i]);
  }
  for(i = 0; i < NumOfStates; i++)
  {
    curScal = 0;    
    for(j = 0; j < numOfNodes; j++)
    {
      mIOfMatrWeight[1] = i;
      mIOfMatrWeight[0] = j;
      curScal+=m_pMatrixWeight->GetElementByIndexes(mIOfMatrWeight)*SetofContNodesValues[j];
    };
    probValues[i] = exp(curScal + m_VectorOffset[i])/norm;
  };
  CNumericDenseMatrix<float> *NewMatrix = CNumericDenseMatrix<float>::Create(1,&NumOfStates,probValues);
  delete []mIOfMatrWeight;
  delete []probValues;
  return NewMatrix;
};
//-----------------------------------------------------------------------------
#ifdef PAR_PNL
void CSoftMaxDistribFun::UpdateStatisticsML(CDistribFun *pPot)
{
    PNL_THROW(CNotImplemented,
        "UpdateStatisticsML for CSoftMaxDistribFun not implemented yet");
};
#endif // PAR_OMP
// ----------------------------------------------------------------------------
float CSoftMaxDistribFun::CalculateKsi(floatVector MeanContParents, 
  C2DNumericDenseMatrix<float>* CovContParents)
{
  int i, j;
  int *multiindex = new int [2];
  
  int NumAllInweights = 0;
  for( i = 0; i < m_NumberOfNodes - 1; i++ )
  {
    NumAllInweights += m_NodeTypes[i]->GetNodeSize();
  }
  
  // ksi^2 = w'(sigma + mu*mu')w +2bw'mu + b^2
  float SqKsi = 0.0f;
  
  // the first component
  intVector ranges = intVector(2, NumAllInweights);
  floatVector ZeroData = floatVector( NumAllInweights*NumAllInweights, 0);
  
  C2DNumericDenseMatrix<float> *ProdMu = 
    C2DNumericDenseMatrix<float>::Create( &ranges.front(), &ZeroData.front());
  
  for (i = 0; i < NumAllInweights; i++ )
  {
    for (j = 0; j < NumAllInweights; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      ProdMu->SetElementByIndexes(MeanContParents[i]*MeanContParents[j], multiindex);
    }
  }
    
  C2DNumericDenseMatrix<float> *SumMatr = static_cast<C2DNumericDenseMatrix<float>*>(
    pnlCombineNumericMatrices( CovContParents, ProdMu, 1 ));
    
  ranges[1] = 1;
  floatVector Zero = floatVector( NumAllInweights, 0);
    
  C2DNumericDenseMatrix<float> *newMatWeights = 
    C2DNumericDenseMatrix<float>::Create( &ranges.front(), &Zero.front());
    
  float weight0, weight1;
  for (i = 0; i < NumAllInweights; i++ )
  {
    multiindex[0] = i;
    multiindex[1] = 0;
    weight0 = m_pMatrixWeight->GetElementByIndexes(multiindex);
    
    multiindex[1] = 1;
    weight1 = m_pMatrixWeight->GetElementByIndexes(multiindex);
    
    multiindex[0] = i;
    multiindex[1] = 0;
    newMatWeights->SetElementByIndexes(weight1-weight0, multiindex);
  }
  
  C2DNumericDenseMatrix<float> *newMatWeightsTransp = newMatWeights->Transpose();
  C2DNumericDenseMatrix<float> *matrixProd = pnlMultiply( newMatWeightsTransp, 
    SumMatr, 0 );
  
  float prod1 = 1;
  for (i = 0; i < NumAllInweights; i++ )
  {
    multiindex[0] = i;     
    multiindex[1] = 0;
    float val2 = newMatWeights->GetElementByIndexes(multiindex);
    
    multiindex[0] = 0;    
    multiindex[1] = i;
    float val1 = matrixProd->GetElementByIndexes(multiindex);
    prod1 *= (val1)*(val2);
  }
  
  // the second component
  float prod = 1; 
  for (i = 0; i < NumAllInweights; i++)
  {
    multiindex[0] = i;
    multiindex[1] = 0;
    prod *= (newMatWeights->GetElementByIndexes(multiindex))*MeanContParents[i];
  }
  prod *= 2*m_VectorOffset[0]; 
  
  // the third component
  float SqB = 0.0f;
  SqB += (m_VectorOffset[1]-m_VectorOffset[0])*(m_VectorOffset[1]-m_VectorOffset[0]);
  
  SqKsi = prod1 + prod + SqB;
  
  float ksi = sqrt(fabs(SqKsi));
  
  delete [] multiindex;
  delete ProdMu; 
  delete SumMatr;
  delete newMatWeights; 
  delete newMatWeightsTransp;
  delete matrixProd;
  
  return ksi;
}

// ----------------------------------------------------------------------------
void CSoftMaxDistribFun::CalculateMeanAndCovariance(float OldKsi, float r, 
  floatVector &MeanVector, C2DNumericDenseMatrix<float> **CovMatrix)
{
  
  int i, j;
  int *multiindex = new int [2];
  
  int dims;
  const int *rangs;
  m_pMatrixWeight->GetRanges( &dims, &rangs );
  
  int NumAllInweights = 0;
  for(i = 0; i < m_NumberOfNodes - 1; i++)
  {
    NumAllInweights += m_NodeTypes[i]->GetNodeSize();
  }
  
  float sigma = 1 / ( 1 + exp(-OldKsi));
  float lambda = ((0.5f) - sigma)/(2*OldKsi);
  
  intVector ranges = intVector(2);
  ranges[0] = NumAllInweights;
  ranges[1] = 1;
  floatVector ZeroData = floatVector( NumAllInweights*NumAllInweights, 0.0f);
  
  // new covariance
  C2DNumericDenseMatrix<float> *newMatWeights = 
    C2DNumericDenseMatrix<float>::Create( &ranges.front(), &ZeroData.front());
  
  float weight0, weight1;
  for (i = 0; i < NumAllInweights; i++ )
  {
    multiindex[0] = i;
    multiindex[1] = 0;
    weight0 = m_pMatrixWeight->GetElementByIndexes(multiindex);
    
    multiindex[1] = 1;
    weight1 = m_pMatrixWeight->GetElementByIndexes(multiindex);
    
    multiindex[1] = 0;
    newMatWeights->SetElementByIndexes(weight0-weight1, multiindex);
  }
  
  C2DNumericDenseMatrix<float> *newMatWeightsTransp = newMatWeights->Transpose();
  C2DNumericDenseMatrix<float> *matrixProd 
    = pnlMultiply( newMatWeights, newMatWeightsTransp, 0 );
  
  float val;
  for (i = 0; i < NumAllInweights; i++ )
  {
    for (j = 0; j < NumAllInweights; j++)
    {
      multiindex[0] = i;
      multiindex[1] = j;
      val = matrixProd->GetElementByIndexes(multiindex);
      val *= (-2*lambda);
      matrixProd->SetElementByIndexes(val, multiindex);
    }
  }
    
  ranges[1] = NumAllInweights;
  C2DNumericDenseMatrix<float> *UnitaryMatrix = 
    C2DNumericDenseMatrix<float>::Create( &ranges.front(), &ZeroData.front());
  for (i = 0; i < NumAllInweights; i++ )
  {
    multiindex[0] = i;
    multiindex[1] = i;
    UnitaryMatrix->SetElementByIndexes(1.0f, multiindex);
  }
  C2DNumericDenseMatrix<float> *SumMatrix = static_cast<C2DNumericDenseMatrix<float>*>(
    pnlCombineNumericMatrices( UnitaryMatrix, matrixProd, 0 ));
  
  *CovMatrix = SumMatrix->Inverse();
  
  // new mean
  float Koeff = r - 0.5f + 2*lambda*m_VectorOffset[0];
  for (i = 0; i < NumAllInweights; i++)
  {
    multiindex[0] = i;
    multiindex[1] = 0;
    val = newMatWeights->GetElementByIndexes(multiindex);
    newMatWeights->SetElementByIndexes(Koeff*val, multiindex);
  }
  
  C2DNumericDenseMatrix<float> *NewMeanMatrix = 
    pnlMultiply( *CovMatrix, newMatWeights, 0 );
  
  const floatVector *kvec = NULL;
  kvec = NewMeanMatrix->GetVector();
  MeanVector.resize((*kvec).size());
  memcpy(&MeanVector[0], &(*kvec)[0], (*kvec).size()*sizeof(float));
  
  delete [] multiindex;
  delete newMatWeights;
  delete newMatWeightsTransp;
  delete matrixProd;
  delete UnitaryMatrix;
  delete SumMatrix;
  delete NewMeanMatrix;

}

// ----------------------------------------------------------------------------
const CNodeType CSoftMaxDistribFun::m_ZeroNT = CNodeType(0,0,nsChance);

// ----------------------------------------------------------------------------
#ifdef PNL_RTTI
const CPNLType CSoftMaxDistribFun::m_TypeInfo = CPNLType("CSoftMaxDistribFun", &(CDistribFun::m_TypeInfo));

#endif

// end of file ----------------------------------------------------------------
