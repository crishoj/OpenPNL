/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:     pnlCondSoftMaxDistribFun.hpp                                 //
//                                                                         //
//  Purpose:  CCondSoftMaxDistribFun class member functions implementation //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Vinogradov                                         //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlMatrix_impl.hpp" // since we'll use CMatrix<CSoftMaxDistribFun*>
#include "pnlCondSoftMaxDistribFun.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlLog.hpp"
#include <sstream>
// ----------------------------------------------------------------------------

PNL_USING

CDistribFun* CCondSoftMaxDistribFun::Clone() const
{
  if (!IsValid())
  {
    PNL_THROW(CInconsistentState, "can't clone invalid data");
  }
  //call copy method
  CCondSoftMaxDistribFun* resData = CCondSoftMaxDistribFun::Copy(this);

  return resData;
}
// ----------------------------------------------------------------------------

CDistribFun* CCondSoftMaxDistribFun::CloneWithSharedMatrices()
{
  if (!IsValid())
  {
    PNL_THROW(CInconsistentState, "can't clone invalid data");
  }

  CCondSoftMaxDistribFun *resData = CCondSoftMaxDistribFun::Create(
    m_NumberOfNodes, &m_NodeTypes.front());
  CMatrixIterator<CSoftMaxDistribFun*>* iterForming =
    m_distribution->InitIterator();

  for (iterForming; m_distribution->IsValueHere(iterForming);
    m_distribution->Next(iterForming))
  {
    CSoftMaxDistribFun* formingData =
      *(m_distribution->Value(iterForming));
    CSoftMaxDistribFun* clonedWithSharedData =
      static_cast<CSoftMaxDistribFun*>(formingData->CloneWithSharedMatrices());
    intVector index;
    m_distribution->Index(iterForming, &index);
    resData->m_distribution->SetElementByIndexes(clonedWithSharedData,
      &index.front());
  }

  delete iterForming;
  resData->m_distribution->AddRef(resData);

  return resData;
}
// ----------------------------------------------------------------------------

CCondSoftMaxDistribFun* CCondSoftMaxDistribFun::Create(int nNodes,
  const CNodeType *const* nodeTypes, CSoftMaxDistribFun* const pDefaultDistr)
{
  PNL_CHECK_IS_NULL_POINTER(nodeTypes);
  PNL_CHECK_LEFT_BORDER(nNodes, 1);

  if (!(nodeTypes[nNodes-1]->IsDiscrete()))
  {
    PNL_THROW(CInconsistentType, "childNode must be discrete");
  }

  CCondSoftMaxDistribFun *resDistr = new CCondSoftMaxDistribFun(nNodes, 
    nodeTypes, pDefaultDistr);
  PNL_CHECK_IF_MEMORY_ALLOCATED(resDistr);

  return resDistr;
}
// ----------------------------------------------------------------------------

CCondSoftMaxDistribFun* CCondSoftMaxDistribFun::Copy(
  const CCondSoftMaxDistribFun* pInpDistr)
{
  PNL_CHECK_IS_NULL_POINTER(pInpDistr);

  CCondSoftMaxDistribFun* resDistr = new CCondSoftMaxDistribFun(*pInpDistr);
  PNL_CHECK_IF_MEMORY_ALLOCATED(resDistr);
  
  return resDistr;
}
// ----------------------------------------------------------------------------

bool CCondSoftMaxDistribFun::IsValid(std::string* description) const
{
  bool ret = 1;
  if (m_discrParentsIndex.size() != m_distribution->GetNumberDims())
  {
    if (description)
    {
      (*description) = "Conditional Gaussain distribution have inconsistent number of Gaussian distributions. It's number should be the same as number of different combination of values of discrete parents. ";
    }
    ret = 0;

    return ret;
  }
  CMatrixIterator<CSoftMaxDistribFun*>* iter = m_distribution->InitIterator();

  for (iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter))
  {
    CSoftMaxDistribFun* val = *(m_distribution->Value(iter));
    if (!val)
    {
      ret = 0;
      if (description)
      {
        intVector index;
        m_distribution->Index(iter, &index);
        int indexSize = index.size();
        int i;
        std::stringstream st;
        st << "Conditional SoftMax distribution haven't SoftMax ";
        st << "distribution for this combination of discrete parents: \n";
        for (i = 0; i < indexSize; i++)
        {
          st << index[i];
        }
        st << std::endl;
        std::string s = st.str();
        description->insert(description->begin(), s.begin(), s.end());
      }
      break;
    }
    else
    {
      if (!val->IsValid(description))
      {
        ret = 0;
        if (description)
        {
          intVector index;
          m_distribution->Index(iter, &index);
          int indexSize = index.size();
          int i;
          std::stringstream st;
          st << "Conditional SoftMax distribution have invalid SoftMax ";
          st << "distribution for this combination of discrete parents: ";
          for (i = 0; i < indexSize; i++)
          {
            st << index[i];
          }
          st << std::endl;
          std::string s = st.str();
          description->insert(description->begin(), s.begin(), s.end());
        }
        break;
      }
    }
  }

  delete iter;

  return ret;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::InitLearnData()
{
  CMatrixIterator<CSoftMaxDistribFun*>* iterForming =
    m_distribution->InitIterator();

  for (iterForming; m_distribution->IsValueHere(iterForming);
    m_distribution->Next(iterForming))
  {
    CSoftMaxDistribFun* formingData =
      *(m_distribution->Value(iterForming));
    formingData->InitLearnData();
  }

  delete iterForming;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::CopyLearnDataToDistrib()
{
  CMatrixIterator<CSoftMaxDistribFun*>* iterForming =
    m_distribution->InitIterator();

  for (iterForming; m_distribution->IsValueHere(iterForming);
    m_distribution->Next(iterForming))
  {
    CSoftMaxDistribFun* formingData =
      *(m_distribution->Value(iterForming));
    formingData->CopyLearnDataToDistrib();
  }

  delete iterForming;
}
// ----------------------------------------------------------------------------

int CCondSoftMaxDistribFun::GetMultipliedDelta(const int **positions, 
  const float **values, const int **offsets) const
{
  return 0;
}
// ----------------------------------------------------------------------------

//typeOfMatrices == 1 - randomly created matrices
void CCondSoftMaxDistribFun::CreateDefaultMatrices(int typeOfMatrices)
{
  PNL_CHECK_RANGES(typeOfMatrices, 1, 1);
  //we have only one type of matrices now
  if (m_bUnitFunctionDistribution)
  {
    PNL_THROW(CInconsistentType,
      "uniform distribution can't have any matrices with data");
  }
  //m_distribution - already exists, need to fill it
  int numRanges;
  const int* ranges;
  m_distribution->GetRanges(&numRanges, &ranges);
  int lineSize = 1;
  int i;
  for (i = 0; i < numRanges; i++)
  {
    lineSize *= ranges[i];
  }

  intVector pConvInd;
  pConvInd.resize(numRanges);
  int nline = 1;
  for (i = numRanges - 1; i >= 0; i--)
  {
    pConvInd[i] = nline;
    int nodeSize = ranges[i];
    nline *= nodeSize;
  }
    
  pConstNodeTypeVector contParentsTypes;
  int numContParents = m_contParentsIndex.size();
  contParentsTypes.resize(numContParents + 1);
  for (i = 0; i < numContParents; i++)
  {
    contParentsTypes[i] = m_NodeTypes[m_contParentsIndex[i]];
  }
  contParentsTypes[numContParents] = m_NodeTypes[m_NumberOfNodes - 1];
  
  div_t result;
  intVector index;
  index.resize(numRanges);
  for (i = 0; i < lineSize; i++)
  {
    int hres = i;
    for (int k = 0; k < numRanges; k++)
    {
      int pInd = pConvInd[k];
      result = div(hres, pInd);
      index[k] = result.quot;
      hres = result.rem;
    }
    CSoftMaxDistribFun* theDistr = 
      m_distribution->GetElementByIndexes(&index.front());
    theDistr = CSoftMaxDistribFun::Create(numContParents + 1, 
      &contParentsTypes.front(), NULL, NULL);
    theDistr->CreateDefaultMatrices(typeOfMatrices);
    m_distribution->SetElementByIndexes(theDistr, &index.front());
  }
}
// ----------------------------------------------------------------------------

CCondSoftMaxDistribFun::CCondSoftMaxDistribFun(int nNodes,
  const CNodeType *const* nodeTypes, CSoftMaxDistribFun* const pDefaultDistr):
  CDistribFun(dtCondSoftMax, nNodes, nodeTypes, 0)
{
  int i;
  for (i = 0; i < nNodes - 1; i++)
  {
    if (nodeTypes[i]->IsDiscrete())
    {
      m_discrParentsIndex.push_back(i);
    }
    else
    {
      m_contParentsIndex.push_back(i);
    }
  }
  int numDiscrParents = m_discrParentsIndex.size();
  intVector discrParentsSizes;
  discrParentsSizes.assign(numDiscrParents, 0);
  int dataSize = 1;
  for (i = 0; i < numDiscrParents; i++)
  {
    int parentSize = nodeTypes[m_discrParentsIndex[i]]->GetNodeSize();
    discrParentsSizes[i] = parentSize;
    dataSize *= parentSize;
  }

  m_numOfDiscreteParConfs = dataSize;
  pnlVector<CSoftMaxDistribFun*> data;
  data.assign(dataSize, (CSoftMaxDistribFun* const)NULL);
  m_distribution = CDenseMatrix<CSoftMaxDistribFun*>::Create(numDiscrParents,
    &discrParentsSizes.front(), &data.front());
  
  void *pObj = this;
  m_distribution->AddRef(pObj);
  m_MaximizingMethod = mmGradient;
}
// ----------------------------------------------------------------------------

//constructor for unit function distribution
CCondSoftMaxDistribFun::CCondSoftMaxDistribFun(int nNodes,
  const CNodeType* const* pNodeTypes, 
  CMatrix<CSoftMaxDistribFun*> const* pDistribMatrix):
  CDistribFun(dtCondSoftMax, nNodes, pNodeTypes, 0)
{
  int i;
  for (i = 0; i < nNodes - 1; i++)
  {
    if (pNodeTypes[i]->IsDiscrete())
    {
      m_discrParentsIndex.push_back(i);
    }
    else
    {
      m_contParentsIndex.push_back(i);
    }
  }
  int numDiscrParents = m_discrParentsIndex.size();

  // can not be all parents are discrete - it is tabular case
  PNL_CHECK_FOR_ZERO (numDiscrParents - (nNodes - 1))
  // can not be all parents are continious - it is SoftMaxDistribFun case
  PNL_CHECK_FOR_ZERO (m_contParentsIndex.size() - (nNodes - 1))

  intVector discrParentsSizes;
  discrParentsSizes.assign(numDiscrParents, 0);
  int dataSize = 1;
  for (i = 0; i < numDiscrParents; i++)
  {
    int parentSize = pNodeTypes[m_discrParentsIndex[i]]->GetNodeSize();
    discrParentsSizes[i] = parentSize;
    dataSize *= parentSize;
  }
  m_numOfDiscreteParConfs = dataSize;

  int numRanges;
  const int* ranges;
  pDistribMatrix->GetRanges(&numRanges, &ranges);
  int areTheSame = 1;
  for (i = 0; i < numDiscrParents; i++)
  {
    if (ranges[i] != discrParentsSizes[i])
    {
      areTheSame = 0;
      break;
    }
  }
  if (areTheSame)
  {
    //need to copy distributions for every element
    //of matrix to prevent destruction of data
    pnlVector<CSoftMaxDistribFun*> copyData;
    copyData.resize(dataSize);
    for (i = 0; i < dataSize; i++)
    {
      copyData[i] = pnl::CSoftMaxDistribFun::Copy(
        static_cast<const CDenseMatrix<CSoftMaxDistribFun*>*>(
        pDistribMatrix)->GetElementByOffset(i));
    }
    m_distribution = CDenseMatrix<CSoftMaxDistribFun*>::Create(
      numDiscrParents, &discrParentsSizes.front(), &copyData.front());
    void *pObj = this;
    m_distribution->AddRef(pObj);
  }
  else
  {
    PNL_THROW(CInvalidOperation,
      "distribution matrix should have the same sizes as discrete parents");
  }
  m_MaximizingMethod = mmGradient;
}
// ----------------------------------------------------------------------------

//copy constructor
CCondSoftMaxDistribFun::CCondSoftMaxDistribFun(
  const CCondSoftMaxDistribFun & inpDistr):CDistribFun(dtCondSoftMax)
{
  m_bUnitFunctionDistribution = inpDistr.m_bUnitFunctionDistribution;
  m_NodeTypes = pConstNodeTypeVector(inpDistr.m_NodeTypes.begin(),
    inpDistr.m_NodeTypes.end());// may be optimized
  m_NumberOfNodes = inpDistr.m_NumberOfNodes;
  m_contParentsIndex.assign(inpDistr.m_contParentsIndex.begin(),
    inpDistr.m_contParentsIndex.end());
  m_discrParentsIndex.assign(inpDistr.m_discrParentsIndex.begin(),
    inpDistr.m_discrParentsIndex.end());
  m_numOfDiscreteParConfs = inpDistr.m_numOfDiscreteParConfs;
  //need to copy conditional gaussian via copying every Gaussain
  //not only via copying matrices !!!
  m_distribution = inpDistr.m_distribution->Clone() ;
  CMatrixIterator<CSoftMaxDistribFun*>* iter =
    inpDistr.m_distribution->InitIterator();
  for (iter; inpDistr.m_distribution->IsValueHere(iter);
    inpDistr.m_distribution->Next(iter))
  {
    CSoftMaxDistribFun* tempDistr = *(inpDistr.m_distribution->Value(iter));
    intVector index;
    inpDistr.m_distribution->Index(iter, &index);
    CSoftMaxDistribFun* newDistr = CSoftMaxDistribFun::Copy(tempDistr);
    m_distribution->SetElementByIndexes(newDistr, &index.front());
  }

  delete iter;
  void* pObj = this;
  m_distribution->AddRef(pObj);
  m_MaximizingMethod = inpDistr.m_MaximizingMethod;
}
// ----------------------------------------------------------------------------

int CCondSoftMaxDistribFun::GetNumberOfFreeParameters() const
{
   if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInconsistentType,
            "uniform distribution can't have any matrices with data" );
    }
    int numRanges;
    const int* ranges;
    m_distribution->GetRanges(&numRanges, &ranges);
    int lineSize = 1;
    int i;
    for( i = 0; i < numRanges; i++ )
    {
        lineSize *= ranges[i];
    }
    
    intVector pConvInd;
    pConvInd.resize( numRanges );
    int nline = 1;
    for( i = numRanges - 1; i >= 0; i--)
    {
        pConvInd[i] = nline;
        int nodeSize = ranges[i];
        nline *= nodeSize;
    }
    
    div_t result;
    intVector index;
    index.resize( numRanges );
	int nparams = 0;
    for( i = 0; i < lineSize; i++ )
    {
        int hres = i;
        for( int k = 0; k < numRanges; k++ )
        {
            int pInd = pConvInd[k];
            result = div( hres, pInd );
            index[k] = result.quot;
            hres = result.rem;
        }
        CSoftMaxDistribFun* theDistr = m_distribution->GetElementByIndexes(&index.front());
        nparams += theDistr->GetNumberOfFreeParameters();
    }
	return nparams;
}
// ----------------------------------------------------------------------------

CCondSoftMaxDistribFun::~CCondSoftMaxDistribFun()
{
  if (m_distribution->GetNumOfReferences() == 1)
  {
    CMatrixIterator<CSoftMaxDistribFun*>* iter =
      m_distribution->InitIterator();
    for (iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter))
    {
      CSoftMaxDistribFun* current = *(m_distribution->Value(iter));
      delete current;
    }
    delete iter;
  }
  void* pObj = this;
  m_distribution->Release(pObj);
}
// ----------------------------------------------------------------------------

CDistribFun& CCondSoftMaxDistribFun::operator =(const CDistribFun& pInputDistr)
{
  if (&pInputDistr == this)
  {
    return *this;
  }
  if (this == &pInputDistr)
  {
    return *this;
  }
  if (pInputDistr.GetDistributionType() != dtCondSoftMax)
  {
    PNL_THROW(CInvalidOperation,
      "input distribution must be Conditional SoftNax ");
  }
  const CCondSoftMaxDistribFun &pSMInputDistr =
    static_cast<const CCondSoftMaxDistribFun&>(pInputDistr);
  int i;
  int isTheSame = 1;
  if (m_NumberOfNodes != pSMInputDistr.m_NumberOfNodes)
  {
    isTheSame = 0;
  }
  else
  {
    const pConstNodeTypeVector* ntInput = pInputDistr.GetNodeTypesVector();
    for (i = 0; i < m_NumberOfNodes; i++)
    {
      if ((*ntInput)[i] != m_NodeTypes[i])
      {
        isTheSame = 0;
        break;
      }
    }
  }
  if (!IsValid() || !pSMInputDistr.IsValid())
  {
    isTheSame = 0;
  }
  if (isTheSame)
  {
    CMatrixIterator<CSoftMaxDistribFun*>* iterChanging =
      m_distribution->InitIterator();
    for (iterChanging; m_distribution->IsValueHere(iterChanging);
    m_distribution->Next(iterChanging))
    {
      CSoftMaxDistribFun* changingData =
        *(m_distribution->Value(iterChanging));
      intVector index;
      m_distribution->Index(iterChanging, &index);
      CSoftMaxDistribFun* formingData =
        pSMInputDistr.m_distribution->GetElementByIndexes(
        &index.front());
      *(CDistribFun*)changingData = *(CDistribFun*)formingData;
    }
    delete iterChanging;
  }
  else
  {
    PNL_THROW(CInvalidOperation,
      "distributions must be valid distributions of the same form & sizes");
  }
  return *this;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::SetDistribFun(CSoftMaxDistribFun* inputDistr, 
  const int* discreteParentCombination)
{
  PNL_CHECK_IS_NULL_POINTER(inputDistr);
  PNL_CHECK_IS_NULL_POINTER(discreteParentCombination);

  //check ranges for parents
  int numDims;
  const int* ranges;
  m_distribution->GetRanges(&numDims, &ranges);
  for (int i = 0; i < numDims; i++)
  {
    PNL_CHECK_RANGES(discreteParentCombination[i], 0, ranges[i] - 1);
  }

  //check validity of inputDistr
  if (inputDistr->GetDistributionType() != dtSoftMax)
  {
    PNL_THROW(CInconsistentType, "we can set only SoftMax");
  }
  CSoftMaxDistribFun* distr = m_distribution->GetElementByIndexes(
    discreteParentCombination);

  delete distr;
  m_distribution->SetElementByIndexes(
    CSoftMaxDistribFun::Copy(inputDistr), discreteParentCombination);
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::AllocDistribFun(
  const int* discreteParentCombination, int isUniform)
{
  PNL_CHECK_IS_NULL_POINTER(discreteParentCombination);

  //find corresponding node types
  pConstNodeTypeVector contParentsTypes;
  int numContParents = m_contParentsIndex.size();
  contParentsTypes.resize(numContParents + 1);
  for (int i = 0; i < numContParents; i++)
  {
    contParentsTypes[i] = m_NodeTypes[m_contParentsIndex[i]];
  }
  contParentsTypes[numContParents] = m_NodeTypes[m_NumberOfNodes - 1];
  
  CSoftMaxDistribFun* theDistr =  m_distribution->GetElementByIndexes(
    discreteParentCombination);
  if (theDistr)
  {
    delete theDistr;
  }
  if (isUniform)
  {
    theDistr = CSoftMaxDistribFun::CreateUnitFunctionDistribution(
      numContParents + 1 , &contParentsTypes.front());
  }
  else
  {
    theDistr = CSoftMaxDistribFun::Create(numContParents + 1,
      &contParentsTypes.front(), NULL, NULL);
  }
  m_distribution->SetElementByIndexes(theDistr, discreteParentCombination);
}
// ----------------------------------------------------------------------------

const CSoftMaxDistribFun* CCondSoftMaxDistribFun::GetDistribution(
  const int* discrParentIndex)const
{
  PNL_CHECK_IS_NULL_POINTER(discrParentIndex);

  const CSoftMaxDistribFun* distr = m_distribution->GetElementByIndexes(
    discrParentIndex);

  return distr;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::AllocMatrix(const float *data, EMatrixType mType,
  int numberOfWeightMatrix, const int *parentIndices)
{
  PNL_CHECK_IS_NULL_POINTER(data);
  PNL_CHECK_IS_NULL_POINTER(parentIndices);

  if (mType != matWeights)
  {
    PNL_THROW(CInconsistentType,
      "SoftMax distribution have only Weight matrix");
  }

  //alloc matrix for corresponding SoftMaxDistribFun
  CSoftMaxDistribFun* thisDistr = m_distribution->GetElementByIndexes(
    parentIndices);

  if (thisDistr)
  {
    thisDistr->AllocMatrix(data, matWeights);
  }
  else
  {
    AllocDistribFun(parentIndices);
    thisDistr = m_distribution->GetElementByIndexes(parentIndices);
    thisDistr->AllocMatrix(data, matWeights);
  }
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::AttachMatrix(CMatrix<float>* pMatrix,
  EMatrixType mType, int numberOfWeightMatrix, const int *parentIndices,
  bool isMultipliedByDelta)
{
  PNL_CHECK_IS_NULL_POINTER(pMatrix);
  PNL_CHECK_IS_NULL_POINTER(parentIndices);

  if (mType != matWeights)
  {
    PNL_THROW(CInconsistentType,
      "SoftMax distribution have only Weight matrix");
  }

  CSoftMaxDistribFun* thisDistr = m_distribution->GetElementByIndexes(
    parentIndices);
  if (thisDistr)
  {
    thisDistr->AttachMatrix(pMatrix, mType);
  }
  else
  {
    PNL_THROW(CInvalidOperation,
      "distribution function must be sit before attaching matrix to it");
  }
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::AllocOffsetVector(const float *data,
  const int *parentIndices)
{
  PNL_CHECK_IS_NULL_POINTER(data);
  PNL_CHECK_IS_NULL_POINTER(parentIndices);

  //alloc vector for corresponding SoftMaxDistribFun
  CSoftMaxDistribFun* thisDistr = m_distribution->GetElementByIndexes(
    parentIndices);
  if (thisDistr)
  {
    thisDistr->AllocOffsetVector(data);
  }
  else
  {
    AllocDistribFun(parentIndices);
    thisDistr = m_distribution->GetElementByIndexes(parentIndices);
    thisDistr->AllocOffsetVector(data);
  }
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::AttachOffsetVector(const floatVector *pVector,
  const int *parentIndices)
{
  PNL_CHECK_IS_NULL_POINTER(pVector);
  PNL_CHECK_IS_NULL_POINTER(parentIndices);

  CSoftMaxDistribFun* thisDistr = m_distribution->GetElementByIndexes(
    parentIndices);
  if (thisDistr)
  {
    thisDistr->AttachOffsetVector(pVector);
  }
  else
  {
    PNL_THROW(CInvalidOperation,
      "distribution function must be sit before attaching vector to it");
  }
}
// ----------------------------------------------------------------------------

CNodeValues *CCondSoftMaxDistribFun::GetMPE()
{
  PNL_THROW(CNotImplemented, "MPE for mixture of SoftMax")
  return NULL;
}
// ----------------------------------------------------------------------------

int CCondSoftMaxDistribFun::IsEqual(const CDistribFun *dataToCompare, 
  float epsilon, int withCoeff, float* maxDifference) const
{
  PNL_CHECK_IS_NULL_POINTER(dataToCompare);
  PNL_CHECK_LEFT_BORDER(epsilon, 0.0f);
  
  if (dataToCompare->GetDistributionType() != dtCondSoftMax)
  {
    PNL_THROW(CInconsistentType, "data to compare must be ConditionalSoftMax");
    return 0;
  }
  if(maxDifference)
  {
    *maxDifference = 0.0f;
  }
  const CCondSoftMaxDistribFun* data =
    static_cast<const CCondSoftMaxDistribFun*>(dataToCompare);

  if ((!IsValid()) || (!data->IsValid()))
  {
    PNL_THROW(CInvalidOperation, "we can't compare invalid data")
  }

  int ret = 1;
  if (m_NumberOfNodes != data->m_NumberOfNodes)
  {
    return 0;
  }
  int numNodes = 0;
  CMatrixIterator<CSoftMaxDistribFun*>* iterThis =
    m_distribution->InitIterator();
  for (iterThis; m_distribution->IsValueHere(iterThis);
    m_distribution->Next(iterThis))
  {
    numNodes++;
    intVector index1;
    m_distribution->Index(iterThis, &index1);
    CSoftMaxDistribFun* val = *(m_distribution->Value(iterThis));
    //now we can find corresponding element in compare matrix
    CSoftMaxDistribFun* compVal = data->m_distribution->
      GetElementByIndexes(&index1.front());
    if (!val->IsEqual(compVal, epsilon, withCoeff, maxDifference))
    {
      ret = 0;
      break;
    }
  }
  delete iterThis;
  int numOtherNodes = 0;
  CMatrixIterator<CSoftMaxDistribFun*>* iterOther =
    data->m_distribution->InitIterator();
  for (iterOther; data->m_distribution->IsValueHere(iterOther);
  data->m_distribution->Next(iterOther))
  {
    numOtherNodes++;
  }
  delete iterOther;
  if (numOtherNodes != numNodes)
  {
    ret = 0;
  }
  return ret;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::SumInSelfData(const int *pBigDomain,
  const int *pSmallDomain, const CDistribFun *pOtherData)
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::MultiplyInSelfData(const int *pBigDomain, 
  const int *pSmallDomain, const CDistribFun *pOtherData)
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::DivideInSelfData(const int *pBigDomain, 
  const int *pSmallDomain, const CDistribFun *pOtherData) 
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
}
// ----------------------------------------------------------------------------

CDistribFun* CCondSoftMaxDistribFun::GetNormalized() const 
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
  return NULL;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::Normalize()
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
}
// ----------------------------------------------------------------------------

CMatrix<float>* CCondSoftMaxDistribFun::GetMatrix(EMatrixType mType, 
  int numWeightMat, const int *parentIndices) const
{
  PNL_CHECK_IS_NULL_POINTER(parentIndices);
  if (mType != matWeights)
  {
    PNL_THROW(CInconsistentType,
      "Conditional SoftMax can have only weights matrix");
  }
  
  CSoftMaxDistribFun* thisDistr = m_distribution->GetElementByIndexes(
    parentIndices);
  if (thisDistr)
  {
    return thisDistr->GetMatrix(mType, numWeightMat);
  }
  else
  {
    PNL_THROW(CInvalidOperation,
      "distribution function must be sit before getting matrix from it");
  }
}
// ----------------------------------------------------------------------------

floatVector* CCondSoftMaxDistribFun::GetOffsetVector(
  const int *parentIndices) const
{
  PNL_CHECK_IS_NULL_POINTER(parentIndices);
  
  CSoftMaxDistribFun* thisDistr = m_distribution->GetElementByIndexes(
    parentIndices);
  if (thisDistr)
  {
    return thisDistr->GetOffsetVector();
  }
  else
  {
    PNL_THROW(CInvalidOperation,
      "distribution function must be sit before getting matrix from it");
  }
}
// ----------------------------------------------------------------------------

CMatrix<float> *CCondSoftMaxDistribFun::GetStatisticalMatrix(
  EStatisticalMatrix mType, int *parentIndices) const
{
  PNL_CHECK_IS_NULL_POINTER(parentIndices);
  
  CSoftMaxDistribFun* thisDistr = m_distribution->GetElementByIndexes(
    parentIndices);
  if (thisDistr)
  {
    return thisDistr->GetStatisticalMatrix(mType);
  }
  else
  {
    PNL_THROW(CInvalidOperation,
      "distribution function must be sit before getting matrix from it");
  }
}
// ----------------------------------------------------------------------------

floatVector* CCondSoftMaxDistribFun::GetStatisticalOffsetVector(
  int *parentIndices) const
{
  PNL_CHECK_IS_NULL_POINTER(parentIndices);
  
  CSoftMaxDistribFun* thisDistr = m_distribution->GetElementByIndexes(
    parentIndices);
  if (thisDistr)
  {
    return thisDistr->GetStatisticalOffsetVector();
  }
  else
  {
    PNL_THROW(CInvalidOperation,
      "distribution function must be sit before getting vector from it");
  }
}
// ----------------------------------------------------------------------------

CDistribFun* CCondSoftMaxDistribFun::ConvertToSparse() const
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
  return NULL;
}
// ----------------------------------------------------------------------------

CDistribFun* CCondSoftMaxDistribFun::ConvertToDense() const
{
  const CCondSoftMaxDistribFun* self = this;
  CCondSoftMaxDistribFun* resDistrib = Copy(self);
  return resDistrib;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::Dump() const
{
  Log dump("", eLOG_RESULT, eLOGSRV_PNL_POTENTIAL);

  int i;
  dump << "I'm a Conditional SoftMax distribution function of " <<  m_NumberOfNodes <<" nodes.\n I have both discrete and continuous parents.\n";
  dump << "The distribution for every discrete combination of parents is: \n";
  int numDiscrPs = m_discrParentsIndex.size();
  CMatrixIterator<CSoftMaxDistribFun*>* iterThis =
    m_distribution->InitIterator();
  for (iterThis; m_distribution->IsValueHere(iterThis);
    m_distribution->Next(iterThis))
  {
    intVector index1;
    m_distribution->Index(iterThis, &index1);
    dump << "Discrete parents combination is:";
    for (i = 0; i < numDiscrPs; i++)
    {
      dump << index1[i]<<" ";
    }
    dump << "\n";
    CSoftMaxDistribFun* distr = *(m_distribution->Value(iterThis));
    distr->Dump();
  }
  delete iterThis;
}
//-----------------------------------------------------------------------------
void CCondSoftMaxDistribFun::DumpMatrix(const CEvidence *pEvidence) 
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

int CCondSoftMaxDistribFun::IsSparse() const 
{
  return 0;
}
// ----------------------------------------------------------------------------

int CCondSoftMaxDistribFun::IsDense() const 
{
  return 1;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::MarginalizeData(const CDistribFun *pOldData,
  const int *VarsOfKeep, int NumVarsOfKeep, int maximize)
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::ShrinkObservedNodes(const CDistribFun* pOldData, 
  const int *pVarsObserved, const Value* const* pObsValues, int numObsVars,
  const CNodeType* pObsTabNT, const CNodeType* pObsGauNT)
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::ExpandData(const int* pDimsToExtend, 
  int numDimsToExpand, const Value* const* valuesArray, 
  const CNodeType* const *allFullNodeTypes, int UpdateCanonical)
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::ClearStatisticalData()
{
  CMatrixIterator<CSoftMaxDistribFun*>* iter = m_distribution->InitIterator();
  for (iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter))
  {
    CSoftMaxDistribFun* dis = *(m_distribution->Value(iter));
    dis->ClearStatisticalData();
  }
  delete iter;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::UpdateStatisticsEM(const CDistribFun* infData,
  const CEvidence *pEvidence, float weightingCoeff, const int* domain)
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::SetStatistics(const CMatrix<float> *pMat, 
  EStatisticalMatrix matrix, const int* parentsComb)
{
  PNL_CHECK_IS_NULL_POINTER(pMat);
  PNL_CHECK_IS_NULL_POINTER(parentsComb);
  
  switch(matrix) 
  {
    case stMatWeight:
    {
      CSoftMaxDistribFun* distr = 
        m_distribution->GetElementByIndexes(parentsComb);
      distr->SetStatistics(pMat, matrix);
      break;
    }
    default:
    {
      PNL_THROW(CInconsistentType, "inconsistent matrix type");
    }
  }
}
// ----------------------------------------------------------------------------
void CCondSoftMaxDistribFun::SetOffsetStatistics(const floatVector *pVec, 
  const int* parentsComb)
{
  PNL_CHECK_IS_NULL_POINTER(pVec);
  PNL_CHECK_IS_NULL_POINTER(parentsComb);
  CSoftMaxDistribFun* distr = m_distribution->GetElementByIndexes(parentsComb);
  distr->SetOffsetStatistics(pVec);
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::UpdateStatisticsML(
  const CEvidence* const* pEvidences, int EvidenceNumber, const int *domain, 
  float weightingCoeff)
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
}
// ----------------------------------------------------------------------------

float CCondSoftMaxDistribFun::ProcessingStatisticalData(float numEvidences)
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
  return 0;
}
// ----------------------------------------------------------------------------

CDistribFun *CCondSoftMaxDistribFun::ConvertCPDDistribFunToPot()const
{
  PNL_THROW(CNotImplemented, "haven't for CCondSoftMaxDistribFun now ");
  return NULL;
}
// ----------------------------------------------------------------------------

CDistribFun* CCondSoftMaxDistribFun::CPD_to_pi(
  CDistribFun *const*allPiMessages, int *multParentIndices, int numMultNodes,
  int posOfExceptParent, int maximizeFlag) const
{
  return NULL;
}
// ----------------------------------------------------------------------------

CDistribFun* CCondSoftMaxDistribFun::CPD_to_lambda(const CDistribFun *lambda,
  CDistribFun *const* allPiMessages, int *multParentIndices, int numNodes,
  int posOfExceptNode, int maximizeFlag) const
{
  return NULL;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::SetMaximizingMethod(EMaximizingMethod met)
{
  m_MaximizingMethod = met;
}
// ----------------------------------------------------------------------------

EMaximizingMethod CCondSoftMaxDistribFun::GetMaximizingMethod()
{
  return m_MaximizingMethod;
}
// ----------------------------------------------------------------------------

void CCondSoftMaxDistribFun::MaximumLikelihood(float ** Observation, 
  int NumberOfObservations, float Accuracy, float step)
{
  float **obsForConf = new float* [m_contParentsIndex.size() + 1];
  int i, j;
  for (i = 0; i < m_contParentsIndex.size() + 1; i++)
  {
    obsForConf[i] = new float [NumberOfObservations];
  }
 
/*  printf("\nObservation\n");
  for (i =0; i < m_contParentsIndex.size() + m_discrParentsIndex.size() + 1;
    i++)
  {
    for (j = 0; j < NumberOfObservations; j++)
      printf("%f\t", Observation[i][j]);
    printf("\n");
  }
  printf("\nm_discrParentsIndex\t");
  for (i = 0; i < m_discrParentsIndex.size(); i++)
    printf("%d\t", m_discrParentsIndex[i]);*/
 
  int NumberOfObsForThisConf;
  CMatrixIterator<CSoftMaxDistribFun*>* iterChanging =
    m_distribution->InitIterator();
  for (iterChanging; m_distribution->IsValueHere(iterChanging); 
    m_distribution->Next(iterChanging))
  {
    NumberOfObsForThisConf = 0;
    CSoftMaxDistribFun* changingData =
      *(m_distribution->Value(iterChanging));
    intVector index;
    m_distribution->Index(iterChanging, &index);
    /*printf("\nindex\t");
    for (int k=0; k < index.size(); k++)
      printf("%d\t", index[k]);*/
    for (i = 0; i < NumberOfObservations; i++)
    {
      int isObsForThisConf = 1;
      for (j = 0; j < m_discrParentsIndex.size(); j++)
      {
        if (Observation[m_discrParentsIndex[j]][i] != index[j])
        {
          isObsForThisConf = 0;
          break;
        }
      }
      if (isObsForThisConf)
      {
        for (j = 0; j < m_contParentsIndex.size(); j++)
        {
          obsForConf[j][NumberOfObsForThisConf] = 
            Observation[m_contParentsIndex[j]][i];
        }
        obsForConf[m_contParentsIndex.size()][NumberOfObsForThisConf] = 
          Observation[m_contParentsIndex.size() + 
          m_discrParentsIndex.size()][i];
        NumberOfObsForThisConf++;
      }
    }
    if (NumberOfObsForThisConf != 0)
    {
      changingData->SetMaximizingMethod(m_MaximizingMethod);
      changingData->MaximumLikelihood(obsForConf, NumberOfObsForThisConf,
        Accuracy, step);
    }
  }

  delete iterChanging;
  for (i = 0; i < m_contParentsIndex.size() + 1; i++)
  {
    delete [] obsForConf[i];
  }
  delete [] obsForConf;
}
//-----------------------------------------------------------------------------
CNumericDenseMatrix<float>* CCondSoftMaxDistribFun:: GetProbMatrix(const CEvidence *pEvidence)
{
    int NumOfStates = m_NodeTypes[m_NumberOfNodes - 1]->GetNodeSize();
    int SizeOfProbMatrix = NumOfStates * m_numOfDiscreteParConfs;
    int IterationNum = 0;
    int i = 0;
    float *Values = new float[SizeOfProbMatrix];
    int *multInd;
    multInd = new int[1];
    const float *data;
    CNumericDenseMatrix<float> *currentMatrix;

    CMatrixIterator<CSoftMaxDistribFun*>* iterChanging =
        m_distribution->InitIterator();
    
    for (iterChanging; m_distribution->IsValueHere(iterChanging); 
    m_distribution->Next(iterChanging))
    {
         currentMatrix =  
             (*(m_distribution->Value(iterChanging)))->GetProbMatrix(pEvidence);
         currentMatrix->GetRawData(&NumOfStates,&data);
        for(i = 0; i < NumOfStates; i++)
        {
            Values[IterationNum*NumOfStates+i] = data[i];       
        };
        IterationNum++;
        delete currentMatrix;       
    };
    int dims;
	const int* ranges;
	
	m_distribution->GetRanges( &dims, &ranges );
   int *newRanges = new int[dims+1];
    for(i = 0; i < dims; i++)
        newRanges[i] = ranges[i];

     newRanges[dims] = NumOfStates; 
     CNumericDenseMatrix<float> *NewMatrix = CNumericDenseMatrix<float>::Create(dims+1,newRanges,Values);
     delete []newRanges;
     delete []multInd;
     delete []Values;
     delete iterChanging;
     return NewMatrix;
};
//-----------------------------------------------------------------------------
#ifdef PAR_PNL
void CCondSoftMaxDistribFun::UpdateStatisticsML(CDistribFun *pPot)
{
    PNL_THROW(CNotImplemented, 
        "UpdateStatisticsML for CCondSoftMaxDistribFun not implemented yet");
};
#endif // PAR_OMP
//-----------------------------------------------------------------------------
int CCondSoftMaxDistribFun::GetSoftMaxSize() const
{
    int *multiindex = new int[2];
    multiindex[0] = 0;
    multiindex[1] = 0;
    int res = m_distribution->GetElementByIndexes(multiindex)->GetSoftMaxSize();
    delete [] multiindex;
    return res;
}
//-----------------------------------------------------------------------------
#ifdef PNL_RTTI
const CPNLType CCondSoftMaxDistribFun::m_TypeInfo = CPNLType("CCondSoftMaxDistribFun", &(CDistribFun::m_TypeInfo));

#endif

//-----------------------------------------------------------------------------
float  CCondSoftMaxDistribFun::CalculateLikelihood(float **Observation, int NumberOfObservations) 
{
   /* float loglik = 0.0f;

    CMatrixIterator<CSoftMaxDistribFun*>* iterChanging =
        m_distribution->InitIterator();
    for (iterChanging; m_distribution->IsValueHere(iterChanging); 
        m_distribution->Next(iterChanging))
    {
        loglik += (*(m_distribution->Value(iterChanging)))->
            CalculateLikelihood(Observation, NumberOfObservations);
    }
    
   return loglik;*/
	float loglik = 0.0f;
    float ll;

    float **obsForConf = new float* [m_contParentsIndex.size() + 1];
    int i, j;
    for (i = 0; i < m_contParentsIndex.size() + 1; i++)
    {
        obsForConf[i] = new float [NumberOfObservations];
    }
    int NumberOfObsForThisConf;
    
    CMatrixIterator<CSoftMaxDistribFun*>* iterChanging =
        m_distribution->InitIterator();
    
    for (iterChanging; m_distribution->IsValueHere(iterChanging); 
    m_distribution->Next(iterChanging))
    {
        NumberOfObsForThisConf = 0;
        CSoftMaxDistribFun* changingData = *(m_distribution->Value(iterChanging));
        intVector index;
        m_distribution->Index(iterChanging, &index);
        for (i = 0; i < NumberOfObservations; i++)
        {
            int isObsForThisConf = 1;
            for (j = 0; j < m_discrParentsIndex.size(); j++)
            {
                if (Observation[m_discrParentsIndex[j]][i] != index[j])
                {
                    isObsForThisConf = 0;
                    break;
                }
            }
            if (isObsForThisConf)
            {
                for (j = 0; j < m_contParentsIndex.size(); j++)
                {
                    obsForConf[j][NumberOfObsForThisConf] = 
                        Observation[m_contParentsIndex[j]][i];
                }
                obsForConf[m_contParentsIndex.size()][NumberOfObsForThisConf] = 
                    Observation[m_contParentsIndex.size() + m_discrParentsIndex.size()][i];
                NumberOfObsForThisConf++;
            }
        }
        
        if (NumberOfObsForThisConf != 0)
        {
            ll = changingData->CalculateLikelihood(obsForConf, NumberOfObsForThisConf);
            loglik += ll;
        }
    }

    delete iterChanging;
    for (i = 0; i < m_contParentsIndex.size() + 1; i++)
    {
        delete [] obsForConf[i];
    }
    delete [] obsForConf;

   return loglik;

}
// end of file ----------------------------------------------------------------
