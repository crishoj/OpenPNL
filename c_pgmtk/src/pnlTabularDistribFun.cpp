/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlTabularDistribFun.cpp                                    //
//                                                                         //
//  Purpose:   CTabularDistribFun class member functions implementation    //                                                                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sstream>

#include "pnlEvidence.hpp"
#include "pnlException.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlLog.hpp"
#include "pnlRng.hpp"

PNL_USING
using namespace std;

CTabularDistribFun* CTabularDistribFun::
Create( int NodeNumber, const CNodeType *const* NodeTypes, const float *data,
       int allocTheMatrices, int asDense)
{
    //need to check only node types - data can be missed
    PNL_CHECK_IS_NULL_POINTER( NodeTypes );
    PNL_CHECK_LEFT_BORDER( NodeNumber,1 );
    PNL_CHECK_RANGES( allocTheMatrices, 0, 1 );

    if( !data && allocTheMatrices )
    {
        PNL_CHECK_RANGES( asDense, 0, 1 );
    }
    
    int allNodesDiscrete = 1;
    for( int i=0; i<NodeNumber; i++ )
    {
        if(!(( NodeTypes[i]->IsDiscrete())||
            ( !NodeTypes[i]->IsDiscrete() )&&(NodeTypes[i]->GetNodeSize() == 0)))
        {
            allNodesDiscrete = 0;
            break;
        }
    }
    if( !allNodesDiscrete )
    {
        PNL_THROW( CInconsistentType,
            "Tabular - not all nodes discrete" );
        //some nodes are tabular - it cannot be Tabular distribution
    }
    else
    {
        CTabularDistribFun *CreatedTabData = new CTabularDistribFun(NodeNumber, 
            NodeTypes, data );
        return CreatedTabData;
    }
}

CTabularDistribFun* CTabularDistribFun::
CreateUnitFunctionDistribution( int NumberOfNodes,
                                const CNodeType *const*nodeTypes,
                                int asDense )
{
    PNL_CHECK_LEFT_BORDER( NumberOfNodes, 1 );
    PNL_CHECK_IS_NULL_POINTER( nodeTypes );
    
    int allNodesCorrect = 1;
    for( int i=0; i<NumberOfNodes; i++ )
    {
        if(!(( nodeTypes[i]->IsDiscrete())||
            ( !nodeTypes[i]->IsDiscrete() )&&(nodeTypes[i]->GetNodeSize() == 0)))
        {
            allNodesCorrect = 0;
            break;
        }
    }
    if( !allNodesCorrect )
    {
        PNL_THROW( CInconsistentType,
            "Tabular - not all nodes discrete" )
        //some nodes are tabular - it cannot be Tabular distribution
    }
    else
    {
        CTabularDistribFun *resData = new CTabularDistribFun( NumberOfNodes,
            nodeTypes, asDense );			
        return resData;
    }
}

CTabularDistribFun* CTabularDistribFun::
Copy( const CTabularDistribFun* pInpDistr )
{
    PNL_CHECK_IS_NULL_POINTER( pInpDistr );
    
    CTabularDistribFun *retDistr = new CTabularDistribFun( *pInpDistr );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retDistr );
    
    return retDistr;
}

CDistribFun* CTabularDistribFun::Clone() const
{
    if( !IsValid() )
    {
        PNL_THROW( CInvalidOperation, "can't clone invalid data" );
    }
    if( m_bUnitFunctionDistribution )
    {
        CDistribFun *retData = new CTabularDistribFun( m_NumberOfNodes, 
            &m_NodeTypes.front(), m_bDense, m_bCheckNegative);
        return retData;
    }
    else
    {
        CTabularDistribFun *retData =  new CTabularDistribFun(
            m_NumberOfNodes, &m_NodeTypes.front() );
        retData->m_bCheckNegative = m_bCheckNegative;
        retData->m_pMatrix = m_pMatrix->Clone();
        retData->m_pMatrix->AddRef(retData);
        retData->ChangeMatricesValidityFlag( GetMatricesValidityFlag() );
        return retData;
    }
}

CDistribFun* CTabularDistribFun::CloneWithSharedMatrices()
{
    if( !IsValid() )
    {
        PNL_THROW( CInvalidOperation, "can't clone invalid data" );
    }
    if( m_bUnitFunctionDistribution )
    {
        CDistribFun *retData = new CTabularDistribFun( m_NumberOfNodes, 
            &m_NodeTypes.front(), m_bDense, m_bCheckNegative);
        return retData;
    }
    else
    {
        CDistribFun *retData = (CDistribFun*) new CTabularDistribFun(
            m_NumberOfNodes, &m_NodeTypes.front() );
        ((CTabularDistribFun*)retData)->m_bCheckNegative = m_bCheckNegative;
        if( m_pMatrix )
        {
            retData->AttachMatrix( m_pMatrix, matTable );
        }
        return retData;
    }
}


CTabularDistribFun::CTabularDistribFun(int numOfNds,
  const CNodeType * const* nodeTypes, const float *data, int asDense,
  int allocTheMatrices, bool CheckNegative)
  :CDistribFun(dtTabular, numOfNds, nodeTypes, 0/*, allocTheMatrices*/)
{
    m_bCheckNegative = CheckNegative;
    if ((numOfNds)&&(nodeTypes))
    {
        intVector NodeSizes;
        NodeSizes.assign( numOfNds, 0 );
        int lineSize = 1;
        int i;
        for ( i = 0; i <numOfNds; i++ )
        {
            int size = (nodeTypes)[i]->GetNodeSize();
            NodeSizes[i] = size ? size : 1;
            lineSize *= NodeSizes[i];
        }
        if( data )
        {
            //check is the data nonnegative
            int nnodes = m_NodeTypes.size();
            if ((m_NodeTypes[nnodes - 1]->GetNodeState() != nsValue) &&
              (m_bCheckNegative))
            {
              for( i = 0; i < lineSize; i++ )
              {
                if( data[i] < 0 )
                {
                    PNL_THROW( CInconsistentType, "data for matrix must be nonnegative" );
                }
              }
            }
            m_pMatrix = CNumericDenseMatrix<float>::Create(numOfNds, &NodeSizes.front(),
                                                        data );
            PNL_CHECK_IF_MEMORY_ALLOCATED( m_pMatrix );
            void *pObj = this;
            m_pMatrix->AddRef(pObj);
            m_bDense = 1;
            ChangeMatricesValidityFlag(1);
            //AllocateMatrices();
        }
        else
        {
            if( allocTheMatrices )
            {
                if( asDense == 1 )
                {
                    m_pMatrix = CNumericDenseMatrix<float>::Create( numOfNds,
                        &NodeSizes.front(), PNL_FAKEPTR(float) );
                }
                else
                {
                    if( asDense == 0 )
                    {
                        m_pMatrix = CNumericSparseMatrix<float>::Create( numOfNds,
                        &NodeSizes.front() );
                    }
                    else
                    {
                        PNL_THROW( CInconsistentType,
                            "allocating matrix should be dense or sparse" );
                    }
                }
                //m_bMatricesAreAllocated = 1;
            }
            else
            {
                m_pMatrix = NULL;
                //m_bMatricesAreAllocated = 0;
            }
        }
    }
    else 
    {
        PNL_THROW( CInvalidOperation, "can't create distribFun without node types" );
    }
    m_bDense = 1;
    m_pLearnMatrix = NULL;
    m_pPseudoCounts = NULL;
}

CTabularDistribFun::CTabularDistribFun(int numOfNds, 
  const CNodeType *const* nodeTypes, int asDense, bool CheckNegative)
  :CDistribFun(dtTabular, numOfNds, nodeTypes, 1)
{
    m_bCheckNegative = CheckNegative;
    intVector nodeSizes;
    nodeSizes.assign(numOfNds, 0);
    for ( int i = 0; i <numOfNds; i++ )
    {
        int size = (nodeTypes)[i]->GetNodeSize();
        nodeSizes[i] = size ? size : 1;
    }
    if( asDense )
    {
        m_pMatrix = CNumericDenseMatrix<float>::Create( numOfNds,
            &nodeSizes.front(), PNL_FAKEPTR(float) );
    }
    else
    {
        m_pMatrix = CNumericSparseMatrix<float>::Create( numOfNds, &nodeSizes.front() );
    }
    void* pObj = this;
    m_pMatrix->AddRef(pObj);
    m_pLearnMatrix = NULL;
    m_bDense = asDense;    
    m_pPseudoCounts = NULL;
}

CTabularDistribFun::CTabularDistribFun(const CTabularDistribFun &inpDistr )
                                       :CDistribFun( dtTabular )
{
    m_NumberOfNodes = inpDistr.m_NumberOfNodes;

    m_NodeTypes.assign( inpDistr.m_NodeTypes.begin(),
        inpDistr.m_NodeTypes.end() );
    
    m_bUnitFunctionDistribution = inpDistr.m_bUnitFunctionDistribution;
    
    m_bDense = inpDistr.m_bDense;
    m_bCheckNegative = inpDistr.m_bCheckNegative;
    
    void *pObj = this;
    
    if( inpDistr.m_pMatrix )
    {
        m_pMatrix = inpDistr.m_pMatrix->Clone();
        m_pMatrix->AddRef(pObj);
        ChangeMatricesValidityFlag(inpDistr.GetMatricesValidityFlag());
        //AllocateMatrices();
    }
    m_pLearnMatrix = NULL;
    m_pPseudoCounts = NULL;
}

CTabularDistribFun::~CTabularDistribFun()
{
    void *pObj = this;
    if (m_pMatrix) m_pMatrix->Release(pObj); 
    if( m_pLearnMatrix ) delete(m_pLearnMatrix);
    if( m_pPseudoCounts ) m_pPseudoCounts->Release(pObj);
}

CDistribFun& CTabularDistribFun::operator=(const CDistribFun &pInputDistr)
{
    if( this == &pInputDistr  )
    {
        return *this;
    }
    if( pInputDistr.GetDistributionType() != dtTabular )
    {
        PNL_THROW( CInvalidOperation,
            "input distribution must be Tabular if the left operand is Tabular" )
    }
    int i;
    int isTheSame = 1;
    const CTabularDistribFun& pTInputDistr = 
        static_cast<const CTabularDistribFun&>( pInputDistr );
    if( m_NumberOfNodes != pTInputDistr.m_NumberOfNodes )
    {
        isTheSame = 0;
    }
    else
    {
        const pConstNodeTypeVector* ntInput = pTInputDistr.GetNodeTypesVector();
        for( i = 0; i < m_NumberOfNodes; i++ )
        {
            if( (*m_NodeTypes[i]) != (*(*ntInput)[i]) )
            {
                isTheSame = 0;
                break;
            }
        }
    }
    if( isTheSame )
    {
        if( m_bUnitFunctionDistribution == pTInputDistr.m_bUnitFunctionDistribution )
        {
                int mClassSelf = m_pMatrix->GetMatrixClass();
                int mClassOther = pTInputDistr.m_pMatrix->GetMatrixClass();
                if( mClassSelf != mClassOther )
                {
                    PNL_THROW( CInvalidOperation,
                        "distributions has different types of matrices - dense/sparse" );
                }
                if( isTheSame )
                {
                    m_pMatrix->SetDataFromOtherMatrix(pTInputDistr.m_pMatrix);
                }
        }
        else
        {
            //one of distributions is unit, other - not -> need to add matrices
            if( m_bUnitFunctionDistribution )
            {
                m_bUnitFunctionDistribution = 0;
                m_pMatrix->SetDataFromOtherMatrix(pTInputDistr.m_pMatrix);
            }
            else
            {
                m_bUnitFunctionDistribution = 1;
            }
        }
        ChangeMatricesValidityFlag( pInputDistr.GetMatricesValidityFlag() );
        m_bCheckNegative = pTInputDistr.m_bCheckNegative;

        return *this;
    }
    else
    {
        PNL_THROW( CInvalidOperation, 
            "input distribution must be of the same type as right hand distribution" )
    }
}
//typeOfMatrices == 1 - randomly created matrices
void CTabularDistribFun::CreateDefaultMatrices( int typeOfMatrices )
{
    PNL_CHECK_RANGES( typeOfMatrices, 1, 1 );
    //we have only one type of matrices now
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInconsistentType,
            "uniform distribution can't have any matrices with data" );
    }
    if( m_pMatrix )
    {
        PNL_THROW( CInvalidOperation,
            "the Distribution Function already have matrices" );
    }
    
    int lineSize = 1;
    int i;
    intVector dims;
    dims.assign( m_NumberOfNodes, 1 );
    for( i = 0; i < m_NumberOfNodes; i++ )
    {
        int nodeSize = m_NodeTypes[i]->GetNodeSize();
        dims[i] = (nodeSize > 0)? nodeSize : 1;
        lineSize *= dims[i];
    }
    floatVector data;
    data.assign(lineSize, 0.0f);
    pnlRand( lineSize, &data.front(), 0.0f, 1.0f );
    if( m_bDense )
    {
        m_pMatrix = CNumericDenseMatrix<float>::Create( m_NumberOfNodes,
            &dims.front(), &data.front() );
    }
    else
    {
        CNumericDenseMatrix<float>* pMatrix = 
            CNumericDenseMatrix<float>::Create( m_NumberOfNodes, &dims.front(),
            &data.front());
        m_pMatrix = pMatrix->ConvertToDense();
        delete pMatrix;
    }
    void* pObj = this;
    m_pMatrix->AddRef(pObj);
    ChangeMatricesValidityFlag(1);
}



void CTabularDistribFun::AttachMatrix( CMatrix<float>* pMatrix,
                                        EMatrixType mType, 
                                        int numberOfWeightMatrix,
                                        const int* parentIndices,
                                        bool)
{
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInvalidOperation,
            " matrices can't be attached to uniform distribution function " );
    }
    
    void* pObj = this;
    
    if( ( mType != matTable ) && ( mType != matDirichlet ) )
    {
        PNL_THROW( CInconsistentType,
            " matrix type should be either matTable or matDirichlet " );
    }

    //check ranges of matrix
    int        numRanges;
    const int* ranges;

    pMatrix->GetRanges( &numRanges, &ranges );
    
    if( numRanges != m_NumberOfNodes )
    {
        PNL_THROW( CInconsistentSize,
            " number of ranges of matrix should coincide"
            " with the number of nodes in domain " );
    }

    int i = 0;
    
    pConstNodeTypeVector::const_iterator ntIt = m_NodeTypes.begin();
    
    for( ; i < numRanges; ++i, ++ntIt )
    {
        bool IsDiscrete = (*ntIt)->IsDiscrete();
        
        int  nodeSize   = (*ntIt)->GetNodeSize();
        
        if( ( IsDiscrete && ( nodeSize != ranges[i] ) )
            || ( !IsDiscrete && ( ranges[i] != 1 ) ) )
        {
            PNL_THROW( CInconsistentSize,
                " input matrix sizes should coincide with node sizes of"
                " nodes in domain " );
        }
    }
    //check data of matrix - is it nonnegative
    float value;

  int nnodes = m_NodeTypes.size();
  if ((m_NodeTypes[nnodes - 1]->GetNodeState() != nsValue) &&
    (m_bCheckNegative))
  {
    CMatrixIterator<float>* iter = pMatrix->InitIterator();
    
    intVector index;
    for( iter; pMatrix->IsValueHere( iter ); pMatrix->Next(iter) )
    {
        value = (*pMatrix->Value(iter));
        if( value < 0 )
        {
            PNL_THROW( CInconsistentType, "data for matrix must be nonnegative" );
        }
    }
    delete iter;
  }
    if( pMatrix->GetMatrixClass() == mcNumericSparse )
    {
        value = static_cast<CNumericSparseMatrix<float>*>(pMatrix)->GetDefaultValue();
        if( value < 0 )
        {
            PNL_THROW( CInconsistentType, "data for matrix must be nonnegative" );
        }
    }
    
    if( mType == matTable )
    {
        if( m_pMatrix )
        {
            m_pMatrix->Release(pObj); 
        }
        // m_pMatrix = const_cast<CMatrix<float>*>(pMatrix);//fixme - about attaching matrix
        m_pMatrix = pMatrix;

        m_pMatrix->AddRef(pObj);

        ChangeMatricesValidityFlag(1);
        
        EMatrixClass mClass = m_pMatrix->GetMatrixClass();

        m_bDense = ( ( mClass == mcSparse ) || ( mClass == mcNumericSparse ) )
            ? 0 : 1;
    }
    else if( mType == matDirichlet )
    {
        if( m_pPseudoCounts )
        {
            m_pPseudoCounts->Release(pObj); 
        }

        m_pPseudoCounts = pMatrix;

        m_pPseudoCounts->AddRef(pObj);         
    }                                  
}

void CTabularDistribFun::AllocMatrix(const float *data, EMatrixType mType, 
  int numberOfWeightMatrix, const int *parentIndices)
{
  if( m_bUnitFunctionDistribution )
  {
    PNL_THROW( CInvalidOperation, "unit function doesn't need any matrices" )
  }
  
  if (mType != matTable && mType != matDirichlet)
  {
    PNL_THROW( CInconsistentType, "matrix type isn't matTable or matDirichlet" )
  }
  intVector NodeSizes;
  NodeSizes.resize( m_NumberOfNodes );
  int lineSize = 1;
  int i;
  for( i = 0; i < m_NumberOfNodes; i++ )
  {
    int NodeSize = m_NodeTypes[i]->GetNodeSize();
    NodeSizes[i] = NodeSize ? NodeSize : 1;
    lineSize *= NodeSizes[i];
  }
  //need to check is data positive
  int nnodes = m_NodeTypes.size();
  if ((m_NodeTypes[nnodes - 1]->GetNodeState() != nsValue) &&
    (m_bCheckNegative))
  {
    for( i = 0; i < lineSize; i++ )
    {
      if( data[i] < 0 )
      {
        PNL_THROW( CInconsistentType, "data for matrix must be nonnegative" );
      }
    }
  }

  void *pObj = this;
  if( mType == matTable )
  {
    if( m_pMatrix )
    {
      if( m_pMatrix->GetMatrixClass() != mcNumericSparse )
      {
        static_cast<CNumericDenseMatrix<float>*>( m_pMatrix )->SetData(data);
      }
      else
      {
        PNL_THROW( CInvalidOperation, "the matrix type is Sparse, it can't set the array to sparse matrix" );
      }
    }
    else
    {
      m_pMatrix = CNumericDenseMatrix<float>::Create( m_NumberOfNodes,
        &NodeSizes.front(), data);
      (m_pMatrix)->AddRef(pObj);
    }
    ChangeMatricesValidityFlag(1);
    m_bDense = 1;
  } 
  else if( mType == matDirichlet )
  {
    m_pPseudoCounts = CNumericDenseMatrix<float>::Create( m_NumberOfNodes,
      &NodeSizes.front(), data);
    m_pPseudoCounts->AddRef(pObj);
  }                                  
}

bool CTabularDistribFun::IsValid(std::string* description) const
{
    if( (!m_NodeTypes.empty())&&(m_DistributionType == dtTabular) )
    {
        if( m_bUnitFunctionDistribution )
        {
            return 1;
        }
        if( m_pMatrix )
        {
            if( GetMatricesValidityFlag() )
            {
                return 1;
            }
            else
            {
                //need to check matrix validity - 
                //is all the data present and it's nonnegative
                bool allDataValid = 1;

              int nnodes = m_NodeTypes.size();
              if ((m_NodeTypes[nnodes - 1]->GetNodeState() != nsValue) &&
                (m_bCheckNegative))
              {
                CMatrixIterator<float>* iter = m_pMatrix->InitIterator();
                for( iter; m_pMatrix->IsValueHere( iter ); m_pMatrix->Next(iter) )
                {
                    float val = *(m_pMatrix->Value(iter));
                    if( val < 0 )
                    {
                        allDataValid = 0;
                        break;
                    }
                }
                delete iter;
              }
                if( m_pMatrix->GetMatrixClass() == mcNumericSparse )
                {
                    const float val = static_cast<CNumericSparseMatrix<float>*>(
                        m_pMatrix)->GetDefaultValue();
                    if( val < 0 )
                    {
                        allDataValid = 0;
                    }
                }
                if( allDataValid )
                {
                    ChangeMatricesValidityFlag(1);
                    return 1;
                }
                else
                {
                    if( description )
                    {
                        std::stringstream st;
                        st<<"Tabular distribution have invalid matrix."<<std::endl;
                        st<<"The matrix should have all its values assigned";
                        st<<"(based on default value for sparse matrices) and nonnegative."<<std::endl;
                        std::string s = st.str();
	                description->insert( description->begin(), s.begin(), s.end() );
                    }
                    return 0;
                }
            }
        }
        else
        {
            if( description )
            {
                std::stringstream st;
                st<<"Tabular distribution function haven't matrix."<<std::endl;
                std::string s = st.str();
	        description->insert( description->begin(), s.begin(), s.end() );
            }
            return 0;
        }
    }
    else
    {
        if(description)
        {
            std::stringstream st;
            st<<"Tabular distribution function haven't node types."<<std::endl;
            description->insert(description->begin(), st.str().begin(),
                st.str().end());
        }
        return 0;
    }
}

CMatrix<float> *CTabularDistribFun::GetMatrix(EMatrixType mType, 
                                               int numWeightMat,
                                               const int *parentIndices)const
{
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInvalidOperation, "Unit Function haven't any matrices" );
    }

    switch(mType)
    {
    case matTable:  
        if( !m_pMatrix )
        {
            PNL_THROW( CInvalidOperation, "we have no matrix yet" );
        }
        else
        {
            ChangeMatricesValidityFlag(0);
            return m_pMatrix;
        }
        break;
    case matDirichlet:
        if( !m_pPseudoCounts )
        {
            PNL_THROW( CInvalidOperation, "we have no Dirichlet matrix yet" );
        }
        else
        {
            return m_pPseudoCounts;
        }
        break;
    default:
        PNL_THROW( CInconsistentType, "matrix type isn't matTable or matDirichlet" );
    }
}

void CTabularDistribFun::Marginalize( const CDistribFun* pOldData,
                      const int* DimsOfKeep, int NumDimsOfKeep, int maximize)
{
    if( !m_pMatrix )
    {
        PNL_THROW( CInvalidOperation, "we can't provide in place marginalization" );
    }
    if( pOldData->GetDistributionType() != dtTabular )
    {
        PNL_THROW( CInconsistentType, "pOldData isn't tabular" )
    }
    if( !pOldData->IsValid() )
    {
        PNL_THROW( CInconsistentType, "pOldData is invalid" )
    }
    //we didn't check node sizes and so on - it should be the same here
    
    //if the distribution is unit - we do nothing, just set the flag
    if( pOldData->IsDistributionSpecific() == 1 )
    {
        m_bUnitFunctionDistribution = 1;
        return;
    }
    m_pMatrix = pOldData->GetMatrix(matTable)->ReduceOp( DimsOfKeep,
        NumDimsOfKeep, maximize, NULL, m_pMatrix );
    m_bUnitFunctionDistribution = 0;
}

void CTabularDistribFun :: MarginalizeData( const CDistribFun *pOldData, 
                                            const int *DimOfKeep,
                                            int NumDimsOfKeep, int maximize) 
{
    if( pOldData->GetDistributionType() != dtTabular )
    {
        PNL_THROW( CInconsistentType, "pOldData isn't tabular" )
    }
    if( !pOldData->IsValid() )
    {
        PNL_THROW( CInconsistentType, "pOldData is invalid" )
    }
    m_NumberOfNodes = NumDimsOfKeep;
    //we need to choose corresponding pointers to node types
    m_NodeTypes.resize( NumDimsOfKeep );
    const pConstNodeTypeVector* nt = pOldData->GetNodeTypesVector();
    int i;
    for( i = 0; i < NumDimsOfKeep; i++ )
    {
        m_NodeTypes[i] = (*nt)[DimOfKeep[i]];
    }
    //check if Old distribution is Unit Function - do nothing
    if( pOldData->IsDistributionSpecific() == 1 )
    {
        void *pObj = this;
        if(m_pMatrix)
        {
            m_pMatrix->Release( pObj );
            m_pMatrix = NULL;
        }
        if( m_pLearnMatrix )
        {
            m_pLearnMatrix->Release( pObj );
            m_pLearnMatrix = NULL;
        }
        //need to create new matrix
        intVector nodeSizes;
        nodeSizes.assign( NumDimsOfKeep, 0 );
        for( i = 0; i < NumDimsOfKeep; i++ )
        {
            nodeSizes[i] = m_NodeTypes[i]->GetNodeSize();
        }
        if( pOldData->IsDense() )
        {
            m_pMatrix = CNumericDenseMatrix<float>::Create( NumDimsOfKeep,
                &nodeSizes.front(), PNL_FAKEPTR(float) );
        }
        if( pOldData->IsSparse() )
        {
            m_pMatrix = CNumericSparseMatrix<float>::Create( NumDimsOfKeep, 
                &nodeSizes.front() );
        }
        m_pMatrix->AddRef(pObj);
        m_bUnitFunctionDistribution = 1;
        return;
    }
    CMatrix<float>* pOldMatrix = pOldData->GetMatrix(matTable);
    CMatrix<float>* matRes = NULL;
    int fl = pOldMatrix->GetMatrixClass();
    if((fl == mcNumericDense)||(fl == mc2DNumericDense))
    {
        matRes = static_cast<CNumericDenseMatrix<float>*>(pOldMatrix)->
            ReduceOp(DimOfKeep, NumDimsOfKeep, maximize);
    }
    else
    {
        if( (fl == mcNumericSparse)||( fl == mc2DNumericSparse ) )
        {
            matRes = static_cast<CNumericSparseMatrix<float>*>(pOldMatrix)->
                ReduceOp(DimOfKeep, NumDimsOfKeep, maximize);
        }
        else
        {
            matRes = pOldMatrix->ReduceOp(DimOfKeep, NumDimsOfKeep, maximize);
        }
    }
    AttachMatrix(matRes, matTable);
}

void CTabularDistribFun::SumInSelfData(const int *pBigDomain,
  const int *pSmallDomain, const CDistribFun *pOtherData)
{
  PNL_CHECK_IS_NULL_POINTER(pBigDomain);
  PNL_CHECK_IS_NULL_POINTER(pSmallDomain);
  PNL_CHECK_IS_NULL_POINTER(pOtherData);
  
  if (!IsValid() || !pOtherData->IsValid())
  {
    PNL_THROW(CInvalidOperation, "multiply invalid data");
  }
  EDistributionType dtOther = pOtherData->GetDistributionType();
  if ((dtOther != dtTabular) && (dtOther != dtScalar))
  {
    PNL_THROW(CInvalidOperation, "we can multiply only tabulars")
  }
  
  int smIsUnit = pOtherData->IsDistributionSpecific();
  //check if small distribution is Unit function or scalar - do nothing
  if ((dtOther == dtScalar) || (smIsUnit == 1))
  {
    return;
  }

  int smallNumNodes = pOtherData->GetNumberOfNodes();
  
  int location;
  int uniFlag = 0;
  float uniVal = 0.0f;
  if (m_bUnitFunctionDistribution)
  {
    int smallNumNodes = pOtherData->GetNumberOfNodes();
    if (m_NumberOfNodes == smallNumNodes)
    {
      // we need to copy small distribFun in self
      // check the order of nodes in domain before and reduce if its need
      // find order of small in big and rearrange dims
      intVector orderOfSmallInBig;
      orderOfSmallInBig.assign(smallNumNodes, 0);
      for (int i = 0; i < smallNumNodes; i++)
      {
        location = std::find(pSmallDomain, 
          pSmallDomain + m_NumberOfNodes, pBigDomain[i]) - pSmallDomain;
        if (location < m_NumberOfNodes)
        {
          orderOfSmallInBig[i] = location;
        }
      }
      m_bUnitFunctionDistribution = 0;
      m_pMatrix = pOtherData->GetMatrix(matTable)->ReduceOp(
        &orderOfSmallInBig.front(), m_NumberOfNodes, 0, NULL, m_pMatrix);

      return;
    }
    else
    {
      m_bUnitFunctionDistribution = 0;
      m_pMatrix->SetUnitData();
      uniFlag = 1;
      uniVal = 1.0f;
    }
  }
  intVector orderOfBigInSmall;
  orderOfBigInSmall.assign( smallNumNodes, 0);
  if ((smallNumNodes == 1) && (m_NumberOfNodes == 2))
  {
    if (pBigDomain[0] == pSmallDomain[0])
    {
      orderOfBigInSmall[0] = 0;
    }
    else
    {
      //we suppose the domain are corresponds - the check is at CPotential level
      orderOfBigInSmall[0] = 1;
    }
  }
  for (int i = 0; i < smallNumNodes; i++)
  {
    location = std::find(pBigDomain, 
      pBigDomain + m_NumberOfNodes, pSmallDomain[i] ) - pBigDomain;
    if (location < m_NumberOfNodes)
    {
      orderOfBigInSmall[i] = location;
    }
  }
  CMatrix<float> *pSmallMatrix = pOtherData->GetMatrix(matTable);
  m_pMatrix->SumInSelf(pSmallMatrix, orderOfBigInSmall.size(), 
    &orderOfBigInSmall.front(), uniFlag, uniVal);
}

void CTabularDistribFun::MultiplyInSelfData( const int *pBigDomain, 
                                             const int *pSmallDomain, const CDistribFun *pOtherData )
{
    PNL_CHECK_IS_NULL_POINTER( pBigDomain );
    PNL_CHECK_IS_NULL_POINTER( pSmallDomain );
    PNL_CHECK_IS_NULL_POINTER( pOtherData );
    
    if( !IsValid() || !pOtherData->IsValid() )
    {
        PNL_THROW( CInvalidOperation, "multiply invalid data" );
    }
    EDistributionType dtOther = pOtherData->GetDistributionType();
    if(( dtOther != dtTabular )&&( dtOther != dtScalar ))
    {
        PNL_THROW( CInvalidOperation, "we can multiply only tabulars" )
    }
    
    int smIsUnit = pOtherData->IsDistributionSpecific();
    //check if small distribution is Unit function or scalar - do nothing
    if(( dtOther == dtScalar )||( smIsUnit == 1 ))
    {
        return;
    }
    int smallNumNodes = pOtherData->GetNumberOfNodes();
    
    int location;
    int uniFlag = 0;
    float uniVal = 0.0f;
    if( m_bUnitFunctionDistribution )
    {
        int smallNumNodes = pOtherData->GetNumberOfNodes();
        if( m_NumberOfNodes == smallNumNodes )
        {
            //we need to copy small distribFun in self
            //check the order of nodes in domain before and reduce if its need
            //find order of small in big and rearrange dims
            intVector orderOfSmallInBig;
            orderOfSmallInBig.assign( m_NumberOfNodes, 0);
            for( int i = 0; i < smallNumNodes; i++ )
            {
                location = std::find( pSmallDomain, 
                    pSmallDomain + m_NumberOfNodes, pBigDomain[i] ) - pSmallDomain;
                if( location < m_NumberOfNodes )
                {
                    orderOfSmallInBig[i] = location;
                }
            }
            m_bUnitFunctionDistribution = 0;
            m_pMatrix = pOtherData->GetMatrix(matTable)->ReduceOp( 
                &orderOfSmallInBig.front(), m_NumberOfNodes, 0, NULL, m_pMatrix);
            return;
        }
        else
        {
            m_bUnitFunctionDistribution = 0;
            m_pMatrix->SetUnitData();
            uniFlag = 1;
            uniVal = 1.0f;
        }
    }
    intVector orderOfBigInSmall;
    orderOfBigInSmall.assign( smallNumNodes, 0);
    if(( smallNumNodes == 1 )&&( m_NumberOfNodes == 2 ))
    {
        if( pBigDomain[0] == pSmallDomain[0] )
        {
            orderOfBigInSmall[0] = 0;
        }
        else
        {
            //we suppose the domain are corresponds - the check is at CPotential level
            orderOfBigInSmall[0] = 1;
        }
    }
    for( int i = 0; i < smallNumNodes; i++ )
    {
        location = std::find( pBigDomain, 
            pBigDomain + m_NumberOfNodes, pSmallDomain[i] ) - pBigDomain;
        if( location < m_NumberOfNodes )
        {
            orderOfBigInSmall[i] = location;
        }
    }
    CMatrix<float> *pSmallMatrix = pOtherData->GetMatrix( matTable );
    //CMatrix<float>* pNewMatrix = m_pMatrix->Clone();
    /*pNewMatrix*/
    m_pMatrix->MultiplyInSelf( pSmallMatrix, orderOfBigInSmall.size(), 
        &orderOfBigInSmall.front(), uniFlag, uniVal);
    //AttachMatrix( pNewMatrix, matTable );
    //fixme - we have problems in inference when try to change data in matrix!!!
    //m_pMatrix->SetData( pResultData ); 
}



void CTabularDistribFun::DivideInSelfData( const int *pBigDomain, 
                                           const int *pSmallDomain, const CDistribFun *pOtherData )
{
    PNL_CHECK_IS_NULL_POINTER( pBigDomain );
    PNL_CHECK_IS_NULL_POINTER( pSmallDomain );
    PNL_CHECK_IS_NULL_POINTER( pOtherData );
    
    if( !IsValid() || !pOtherData->IsValid() )
    {
        PNL_THROW( CInvalidOperation, "divide invalid data" );
    }
    EDistributionType dtOther = pOtherData->GetDistributionType();
    if(( dtOther != dtTabular )&&(dtOther != dtScalar))
    {
        PNL_THROW( CInvalidOperation, "we can divide only tabulars" )
    }
    int smIsUnit = pOtherData->IsDistributionSpecific();
    //if divisor is Unit Function or scalar - we can do nothing
    if(( smIsUnit == 1 )||( dtOther == dtScalar ))
    {
        return;
    }
    //create matrix of ones to compare with UnitDistribution matrix
    int i;
    //int flag = static_cast<const CTabularDistribFun*>(pOtherData)->IsDense();
    //CMatrix<float> *onesMatrix = CreateUnitFunctionMatrix(flag);
    
    //check form of self distribution & determine matrix for work with
    CMatrix<float> *pBigMatrix = NULL;
    if( m_bUnitFunctionDistribution == 1 )
    {
        m_pMatrix->SetUnitData();
        m_bUnitFunctionDistribution = 0;
    }
    pBigMatrix = m_pMatrix;
    
    CMatrix<float> *pSmallMatrix = pOtherData->GetMatrix( matTable );
    
    int smallNumNodes = pOtherData->GetNumberOfNodes();
    
    intVector orderOfBigInSmall;
    orderOfBigInSmall.assign( smallNumNodes, 0 );
    int location;
    for( i = 0; i < smallNumNodes; i++ )
    {
        location = std::find( pBigDomain, 
            pBigDomain + m_NumberOfNodes, pSmallDomain[i] ) - pBigDomain;
        if( location < m_NumberOfNodes )
        {
            orderOfBigInSmall[i] = location;
        }
    }
    
    CMatrix<float>* pNewMatrix = pBigMatrix->Clone();
    pNewMatrix->DivideInSelf( pSmallMatrix, orderOfBigInSmall.size(),
        &orderOfBigInSmall.front());
    
        /*	CMatrix<float> *pDiffMatrix = pnlCombineNumericMatrices( pNewMatrix, onesMatrix, 0 );
        float sum = pDiffMatrix->SumAll(1);
        if( fabs(sum) < 0.001f )
        {
        m_bUnitFunctionDistribution = 1;
        delete pNewMatrix;
        }
        else
        {
        m_bUnitFunctionDistribution = 0;	
        AttachMatrix( pNewMatrix, matTable);
        }
    */
    AttachMatrix( pNewMatrix, matTable );
}

void CTabularDistribFun::ShrinkObservedNodes( const CDistribFun* pOldData, 
                                              const int *pVarsOfObserved, 
                                              const Value* const* pObsValues,
                                              int numObsVar,
                                              const CNodeType* pObsTabNT,
                                              const CNodeType* pObsGauNT )
{
    PNL_CHECK_IS_NULL_POINTER( pOldData );
    PNL_CHECK_LEFT_BORDER( numObsVar, 0 );
    if( numObsVar > 0 )
    {
        PNL_CHECK_IS_NULL_POINTER( pObsValues );
        PNL_CHECK_IS_NULL_POINTER( pVarsOfObserved );
        PNL_CHECK_IS_NULL_POINTER( pObsTabNT );
        PNL_CHECK_IS_NULL_POINTER( pObsGauNT );
    }
    if( !pOldData->IsValid() )
    {
        PNL_THROW( CInvalidOperation, "shrink invalid data" );
    }
    //check for special cases - we needn't to shrink matrices -
    //they have the same values of ones
    //only need to change node types
    int i;
    const pConstNodeTypeVector* nt = pOldData->GetNodeTypesVector();
    m_NodeTypes.assign( nt->begin(), nt->end() );
    for( i = 0; i < numObsVar; i++ )
    {
        int IsDiscrete = m_NodeTypes[pVarsOfObserved[i]]->IsDiscrete();
        if( IsDiscrete )
        {
            m_NodeTypes[pVarsOfObserved[i]] = pObsTabNT;
        }
        else
        {
            m_NodeTypes[pVarsOfObserved[i]] = pObsGauNT;
        }
        
    }

    if( pOldData->GetDistributionType() == dtTabular )
    {    
        m_bDense = static_cast<const CTabularDistribFun*>(pOldData)->IsDense();
        if( pOldData->IsDistributionSpecific() == 1 )
        {
            m_bUnitFunctionDistribution = 1;
            //need to change matrix
            void* pObj = this;
            if(m_pMatrix)
            {
                m_pMatrix->Release(pObj);
            }
            m_pMatrix = CreateUnitFunctionMatrix( m_bDense, 1 );
            m_pMatrix->AddRef(pObj);
            return;
        }
        CMatrix<float>* pOldMatrix = pOldData->GetMatrix(matTable);
        CMatrix<float> *pNewMatrix = NULL;
        if( numObsVar )
        {
            intVector ObsValues;
            ObsValues.resize(numObsVar);
            for( i = 0; i < numObsVar; i++)
            {
                int val = pObsValues[i]->GetInt();
                ObsValues[i] = val;
            }
            pNewMatrix = pOldMatrix->ReduceOp( pVarsOfObserved, numObsVar, 2,
                &ObsValues.front());
        }    
        else
        {
            pNewMatrix = pOldMatrix;
        }
        m_NumberOfNodes = pOldData->GetNumberOfNodes();
        AttachMatrix(pNewMatrix, matTable);
        
    }
    else
    {
        //if all Gaussian nodes observed - we have Tabular
        if( pOldData->GetDistributionType() == dtGaussian )
        {
            //the case is valid iff all nodes observed
            if( m_NumberOfNodes != numObsVar )
            {
                PNL_THROW( CInvalidOperation,
                    "can't create Tabular from Gaussian without full observability" )
            }
            else
            {
                if( m_pMatrix )
                {
                    void* pObj = this;
                    m_pMatrix->Release(pObj);
                    m_pMatrix = NULL;
                    m_bDense = 1;
                    //need to create unit matrix
                    m_pMatrix = CreateUnitFunctionMatrix( m_bDense ,1 );
                    m_pMatrix->AddRef(pObj);
                }
                ChangeMatricesValidityFlag(0);
                m_bUnitFunctionDistribution = 1;
            }
        }
    }
}

void CTabularDistribFun::ExpandData(const int * pDimsToExpand, 
                                     int numDimsToExpand,
                                     const Value* const* valuesArray, 
                                     const CNodeType* const* allFullNodeTypes,
                                     int UpdateCanonical)
{
    if(( !allFullNodeTypes )||( !valuesArray )||( !pDimsToExpand ))
    {
        PNL_THROW( CNULLPointer, "input arrays" )
    }
    if( numDimsToExpand < 0 )
    {
        PNL_THROW( COutOfRange, "number of dims to Expand is negative" )
    }
    if(( numDimsToExpand )&&( numDimsToExpand <= m_NumberOfNodes ))
    {
        int i;
        m_NodeTypes = pConstNodeTypeVector(allFullNodeTypes,
            allFullNodeTypes + m_NumberOfNodes);// may be optimized
        
        int *valuesOfDims = new int[numDimsToExpand];
        PNL_CHECK_IF_MEMORY_ALLOCATED( valuesOfDims );
        int *sizesOfExpandDims = new int[numDimsToExpand];
        PNL_CHECK_IF_MEMORY_ALLOCATED( sizesOfExpandDims );
        for( i = 0; i < numDimsToExpand; i++ )
        {
            const int val =	valuesArray[i]->GetInt();
            valuesOfDims[i] = val;
            sizesOfExpandDims[i] =
                allFullNodeTypes[pDimsToExpand[i]]->GetNodeSize();
            if( val >= sizesOfExpandDims[i] )
            {
                PNL_THROW( COutOfRange, 
                    "Values of nodes is more than range" )
            }
        }
        if( m_bUnitFunctionDistribution == 1 )
        {
            //we need to create matrix of ones and add evidences to it!
            m_bUnitFunctionDistribution = 0;
            //fixme - if we need to expand and result matrix should be sparse?
            m_pMatrix->SetUnitData();
        }
        CMatrix<float> *ExpandedMatrix = m_pMatrix->ExpandDims( pDimsToExpand, 
            valuesOfDims, sizesOfExpandDims, numDimsToExpand);
        AttachMatrix( ExpandedMatrix, matTable );
        delete []sizesOfExpandDims;
        delete []valuesOfDims;
    }
}

CMatrix<float> *CTabularDistribFun::GetStatisticalMatrix(EStatisticalMatrix mType,
                                                    int *parentIndices )const
{
    if (mType!= stMatTable) 
    { 
        PNL_THROW( CInconsistentType, "matrix type isn't matTable" )
    }
    if (!m_pLearnMatrix) 
    { 
        PNL_THROW( CNULLPointer, "Learn Matrix" )
    }
    else return m_pLearnMatrix;
    
}
void CTabularDistribFun::ClearStatisticalData()
{
    if (m_pLearnMatrix)
    {
        m_pLearnMatrix->ClearData();
    }
}


#if 0
void CTabularDistribFun::UpdateStatisticsEM(const CDistribFun* pInfData,
                                      const CEvidence *pEvidence, 
                                      float weightingCoeff,
                                      const int* domain )
{
    
    if( !pInfData )
    {
        PNL_THROW( CNULLPointer, "No inference data" )//no Inf  - NULL pointer
    }
    CMatrix<float> *jpdMatrix = pInfData ->GetMatrix(matTable);
    
    int DomainSize; 
    const int *NodeSizes;
    m_pMatrix->GetRanges( &DomainSize, &NodeSizes );
    if (!m_pLearnMatrix)
    {
        m_pLearnMatrix = m_pMatrix->CreateEmptyMatrix(DomainSize,
            NodeSizes, 0);
    }
    int nlineBig = 1;
    float value;
    intVector pconvIndBig;
    pconvIndBig.resize( DomainSize );
    for( int i1 = DomainSize-1; i1 >= 0; i1--)
    {	
        pconvIndBig[i1] = nlineBig;
        int bigNodeSize = NodeSizes[i1];
        nlineBig = nlineBig*bigNodeSize;
    }
    //need to set element by indices!
    intVector indices;
    indices.resize(DomainSize);
    for( int i=0; i < nlineBig; i++)
    {
        div_t result;
        int hres = i;
        for( int k = 0; k < DomainSize; k++ )
        {
            result = div( hres, pconvIndBig[k] );
            indices[k] = result.quot;
            hres = result.rem;
        }
        value = m_pLearnMatrix->GetElementByIndexes(&indices.front());
        value += jpdMatrix->GetElementByIndexes(&indices.front());
        m_pLearnMatrix->SetElementByIndexes( value, &indices.front() );
    }
}


#endif


#if 1
void CTabularDistribFun::UpdateStatisticsEM(const CDistribFun* pInfData,
                                      const CEvidence *pEvidence, 
                                      float weightingCoeff,
                                      const int* domain )
{
    
    if( !pInfData )
    {
        PNL_THROW( CNULLPointer, "No inference data" )//no Inf  - NULL pointer
    }
    
    if (!m_pLearnMatrix)
    { 
        int DomainSize; 
        const int *NodeSizes;
        m_pMatrix->GetRanges( &DomainSize, &NodeSizes );
        m_pLearnMatrix = m_pMatrix->CreateEmptyMatrix(DomainSize,
            NodeSizes, 0);
    }
   
    CMatrix<float> *jpdMatrix = pInfData ->GetMatrix(matTable);
    CMatrixIterator<float>* iter = jpdMatrix->InitIterator();
    
    float value;
    intVector index;
    for( iter; jpdMatrix->IsValueHere( iter ); jpdMatrix->Next(iter) )
    {
        jpdMatrix->Index( iter, &index );
        value = m_pLearnMatrix->GetElementByIndexes(&index.front());
        value += *(jpdMatrix->Value( iter ));
        m_pLearnMatrix->SetElementByIndexes( value, &index.front() );
        index.clear();
    }
    delete iter;
    
}
#endif

void CTabularDistribFun::SetStatistics( const CMatrix<float> *pMat, 
	EStatisticalMatrix matrix, const int* parentsComb )
{
    if( matrix != stMatTable )
    {
	PNL_THROW( CBadArg, "type of matrix" );
    }
    PNL_CHECK_IS_NULL_POINTER( pMat );
    if (!m_pLearnMatrix)
    { 
        int DomainSize; 
        const int *NodeSizes;
        m_pMatrix->GetRanges( &DomainSize, &NodeSizes );
        m_pLearnMatrix = m_pMatrix->CreateEmptyMatrix(DomainSize,
            NodeSizes, 0);
    }
    m_pLearnMatrix->SetDataFromOtherMatrix( pMat);
}

void CTabularDistribFun::UpdateStatisticsML(const CEvidence* const* pEvidences,
                                      int EvidenceNumber, const int *domain, 
                                      float weightingCoeff)
{
    if( !pEvidences )
    {
        PNL_THROW( CNULLPointer, "evidences" )//no Evidences - NULL pointer
    }
    /*
    if( !m_pMatrix )
    {
    PNL_THROW( CNULLPointer, "matrix" )//no Matrix - NULL pointer
    }
    
      int i;
      if (!m_pLearnMatrix)
      {
      m_pLearnMatrix = m_pMatrix->CreateEmptyMatrix(DomainSize,
      NodeSizes, 0);
      }//end if m_pLearnMatrix
    */
    
    int DomainSize;
    const int *NodeSizes; 
    int i;
    if( !m_pLearnMatrix)
    {
        int sz = 1;
        const pConstNodeTypeVector *nt = GetNodeTypesVector();
        intVector nodeSizes;
        nodeSizes.resize(nt->size());
        for( i = 0; i < nt->size(); i++ )
        {
            nodeSizes[i] = (*nt)[i]->GetNodeSize();
            sz *= nodeSizes[i];
        }
        if( m_pMatrix )
        {
            m_pLearnMatrix = m_pMatrix->CreateEmptyMatrix(nodeSizes.size(),
                &nodeSizes.front(), 0); 
        }
        else
        {
            floatVector data(sz, 0.0f);
            m_pLearnMatrix = CNumericDenseMatrix<float>::Create(nodeSizes.size(),
                &nodeSizes.front(), &data.front() );
        }
        
    }
    m_pLearnMatrix->GetRanges( &DomainSize, &NodeSizes );
    
    
    int *ranges = new int[DomainSize];
    int counter;
    int node;
    float element;
    
    for( int evNumber = 0; evNumber < EvidenceNumber; evNumber++ )
    {
    /* 
    find the meaning of every node from domain 
    and constract linear index of maltidmatrix for every evidence 
        */
        for( i = 0; i < DomainSize; i++ )
        {
            if( !pEvidences[evNumber] )
            {
                PNL_THROW( CNULLPointer, "evidence" )//no Evidence - NULL pointer
            }
            
            node = domain[i];
            counter = (pEvidences[evNumber]->GetValue(node))->GetInt();
            
            if( counter >= 0 && counter <  NodeSizes[i])
            {
                ranges[i] = counter;
            }
            else
            {
                
                PNL_THROW( CInconsistentSize, 
                    "observed value is larger then node size or negative") ; 
            }
        }
        /*
        finde an element in the matrix which corresponding to this 
        evidence for every node in domain and then inc it
        */
        element = m_pLearnMatrix->GetElementByIndexes( ranges );
        element++;
        m_pLearnMatrix -> SetElementByIndexes( element, ranges );
    }
    delete []ranges;
}

float CTabularDistribFun::ProcessingStatisticalData( float numEvidences )
{
    float loglik = 0.0f;
    bool notClamp = m_pMatrix->GetClampValue() ? false : true;
    
    float prob = 0.0f;
    float value = 0.0f;
    float denominator = 0.0f;
    int DomainSize;
    const int *NodeSizes;
    m_pMatrix->GetRanges( &DomainSize, &NodeSizes );
    
    int prodParentsRanges = 1;
    int i;
    
    for(i = 0; i < DomainSize - 1; i++ )
    {
        prodParentsRanges *= NodeSizes[i];
    }
    
    int childeSize = NodeSizes[DomainSize - 1];
    
    //need to work with sparse matrices also - 
    //convert from dense to sparse doesnt' need
    EMatrixClass matCl = m_pMatrix->GetMatrixClass();
    
    if( matCl ==  mcNumericDense || matCl == mc2DNumericDense )
    {
	for( i = 0; i < prodParentsRanges ; i++ )
	{
	    int jump = i * childeSize;
	    denominator = 0.0f;
	    int j;
	    if(notClamp)
	    {
		for( j = 0; j < childeSize ; j++ )
		{
		    denominator += ( static_cast<CNumericDenseMatrix<float>*>(
			m_pLearnMatrix)->GetElementByOffset( jump + j ) );
		}
	    }
	    
	    if( denominator < FLT_EPSILON  )
	    {
		if(notClamp)
		{
		    value = 1.0f / childeSize;
		    
		    for( j = 0; j < childeSize; j++ )
		    {
			static_cast<CNumericDenseMatrix<float>*>(
			    m_pMatrix)->SetElementByOffset( value, jump + j );
		    }
		}
	    }
	    else
	    {
		for( j = 0; j < childeSize; j++ )
		{
		    value = ( static_cast<CNumericDenseMatrix<float>*>(
			m_pLearnMatrix)->GetElementByOffset( jump + j ) );
		    if( notClamp )
		    {
			
			prob = value / denominator;
			loglik += value * 
			    float( ( prob < FLT_EPSILON ) ? -FLT_MAX : log( prob ) );
			static_cast<CNumericDenseMatrix<float>*>(
			    m_pMatrix)->SetElementByOffset( prob, jump + j );
		    }
		    else
		    {
			prob = static_cast<CNumericDenseMatrix<float>*>(
			    m_pMatrix)->GetElementByOffset( jump + j );
			loglik += value * 
			    float( ( prob < FLT_EPSILON ) ? -FLT_MAX : log( prob ) );
			static_cast<CNumericDenseMatrix<float>*>(
			    m_pMatrix)->SetElementByOffset( prob, jump + j );
		    }
		}
		
	    }
	    
	}
	//check the data in matrix
    }
    else
	{
		intVector indexes(DomainSize, 0);
		indexes[0] = -1;
		int ind;
		float sum;
		float koeff;
		float prob;
		
		if( DomainSize > 1)
		{
			for( ind = 0; ind < DomainSize -1 ; )
			{
				if( indexes[ind] == NodeSizes[ind] - 1 )
				{
					indexes[ind] = 0;
					ind++;
				}
				else
				{
					indexes[ind]++;
					ind = 0;
					for( i = 0, sum = 0.0f; i < childeSize; i++ )
					{
						indexes[DomainSize - 1] = i;
						sum += m_pLearnMatrix->GetElementByIndexes(&indexes.front());
					}
					koeff = sum > (1/FLT_MAX) ? 1/sum : 0.0f;
					for( i = 0; i < childeSize; i++ )
					{
						indexes[DomainSize - 1] = i;
						if( notClamp )
						{  
							value = m_pLearnMatrix->GetElementByIndexes(&indexes.front());
							prob = koeff > (1/FLT_MAX) ? value*koeff : 1/childeSize;  
							m_pMatrix->SetElementByIndexes(prob, &indexes.front());
						}
						else
						{
							prob = m_pMatrix->GetElementByIndexes(&indexes.front());
						}
						loglik += value*float( ( prob < (1/FLT_MAX) ) ? -FLT_MAX : log( prob ) );
					}
				}
			}
		}
		else
		{
			for( i = 0, sum = 0.0f; i < childeSize; i++ )
			{
				sum += m_pLearnMatrix->GetElementByIndexes(&i);
			}
			koeff = sum > (1/FLT_MAX) ? 1/sum : 0.0f;
			for( i = 0; i < childeSize; i++ )
			{
				if( notClamp )
				{  
					value = m_pLearnMatrix->GetElementByIndexes(&i);
					prob = koeff > (1/FLT_MAX) ? value*koeff : 1/childeSize;  
					m_pMatrix->SetElementByIndexes(prob, &i);
				}
				else
				{
					prob = m_pMatrix->GetElementByIndexes(&i);
				}
				loglik += value*float( ( prob < (1/FLT_MAX) ) ? -FLT_MAX : log( prob ) );
			}
			
		}
    }
    return loglik;
}
int CTabularDistribFun::IsEqual( const CDistribFun *dataToCompare,
                                 float epsilon, int withCoeff,
                                 float* maxDifference )const
{
    PNL_CHECK_IS_NULL_POINTER( dataToCompare );
    if( !IsValid() || !dataToCompare->IsValid() )
    {
        PNL_THROW( CInvalidOperation, "compare invalid data" );
    }
    if( maxDifference )
    {
        *maxDifference = 0.0f;
    }
    EDistributionType dt = dataToCompare->GetDistributionType();
    if( dt != dtTabular )
    {
        PNL_THROW(CInconsistentType, "distribution type isn't Tabular" )
            //we can compare only the same datas? (or return 0;)
    }
    float ZeroEpsilon = epsilon;
    //need to compare node types first
    int i;
    if( m_NumberOfNodes != dataToCompare->GetNumberOfNodes() )
    {
        return 0;
    }
    const pConstNodeTypeVector* ntIn = dataToCompare->GetNodeTypesVector();
    for( i = 0; i < m_NumberOfNodes; i++ )
    {
        
        for( i = 0; i < m_NumberOfNodes; i++ )
        {
            if( (*m_NodeTypes[i]) != (*(*ntIn)[i]) )
            {
                return 0;
            }
        }
    }
    if( m_bUnitFunctionDistribution == 1 )
    {
        if( dataToCompare->IsDistributionSpecific() == 1 )
        {
            return 1;
        }
        else
        {
            //can check the matrix
            float diff = static_cast<const CTabularDistribFun*>(dataToCompare)->m_pMatrix->SumAll(1);
            if( diff < ZeroEpsilon )
            {
                return 1;
            }
            if( maxDifference )
            {
                *maxDifference = diff;
            }
            return 0;
        }
    }
    if( dataToCompare->IsDistributionSpecific() == 1 )
    {
        float diff = m_pMatrix->SumAll(1);
        if( diff < ZeroEpsilon )
        {
            return 1;
        }
        if( maxDifference )
        {
            *maxDifference = diff;
        }
        return 0;
    }
    CMatrix<float>* matrixCompare = dataToCompare->GetMatrix(matTable);
    //compare nDims:
    int nDims = m_pMatrix->GetNumberDims();
    int nDimsCompare = matrixCompare->GetNumberDims();
    if( nDims != nDimsCompare )
    {
        return 0;
    }
    const int *ranges;
    m_pMatrix->GetRanges( &nDims, &ranges );
    const int *rangesCompare;
    matrixCompare->GetRanges( &nDimsCompare, &rangesCompare );
    int lineSize = 1;
    for( i = 0; i < nDims; i++ )
    {
        if( ranges[i] != rangesCompare[i] )
        {
            return 0;
        }
        lineSize *= ranges[i];
    }
    //need to compare matrices of the same type by direct combining
    //if matrices of different types - need to convert
    //check forms of matrices
    int flClassSelf = m_pMatrix->GetMatrixClass();
    int flClassCompare = matrixCompare->GetMatrixClass();
    int selfDense = ((flClassSelf == mcDense)||(flClassSelf == mcNumericDense)
        ||( flClassSelf == mc2DNumericDense ))? 1:0;
    int otherDense = ((flClassCompare == mcDense)||
        (flClassCompare == mcNumericDense)||
        (flClassCompare == mc2DNumericDense))? 1:0;
    float differAbs = FLT_MAX;
    if( selfDense == otherDense )
    {
        if( selfDense )
        {
            CNumericDenseMatrix<float>* matDiffer = static_cast<
                CNumericDenseMatrix<float>*>(m_pMatrix->Clone());
            matDiffer->CombineInSelf( static_cast<CNumericDenseMatrix<float>*>
                (matrixCompare), 0);
            differAbs = matDiffer->SumAll(1)/lineSize;
            delete matDiffer;
        }
        else
        {
            CNumericSparseMatrix<float>* matDiffer = static_cast<
                CNumericSparseMatrix<float>*>(m_pMatrix->Clone());
            matDiffer->CombineInSelf( static_cast<CNumericSparseMatrix<float>*>
                (matrixCompare), 0);
            differAbs = matDiffer->SumAll(1)/lineSize;
            delete matDiffer;
        }
    }
    else
    {
        //convert both to sparse and compare
        if( selfDense )
        {
            CNumericSparseMatrix<float>* matDiffer = static_cast<
                CNumericSparseMatrix<float>*>(m_pMatrix->ConvertToSparse());
            matDiffer->CombineInSelf( static_cast<CNumericSparseMatrix<float>*>
                (matrixCompare), 0);
            differAbs = matDiffer->SumAll(1)/lineSize;
            delete matDiffer;
        }
        else
        {
            CNumericSparseMatrix<float>* matDiffer = static_cast<
                CNumericSparseMatrix<float>*>(matrixCompare->ConvertToSparse());
            matDiffer->CombineInSelf( static_cast<CNumericSparseMatrix<float>*>
                (m_pMatrix), 0);
            differAbs = matDiffer->SumAll(1)/lineSize;
            delete matDiffer;
        }
    }
    if( differAbs < epsilon )
    {
        return 1;
    }
    else
    {
        if( maxDifference )
        {
            *maxDifference = differAbs;
        }
        return 0;
    }
}

CDistribFun *CTabularDistribFun::ConvertCPDDistribFunToPot() const
{
    if( !IsValid() )
    {
        PNL_THROW( CInvalidOperation, "can't clone invalid data" );
    }
    if( m_bUnitFunctionDistribution )
    {
        CDistribFun *retData = new CTabularDistribFun( m_NumberOfNodes, 
            &m_NodeTypes.front(), m_bDense );
        return retData;
    }
    else
    {
        CTabularDistribFun *retData = new CTabularDistribFun(m_NumberOfNodes,
            &m_NodeTypes.front());
        delete retData->m_pMatrix;
        if( m_pMatrix )
        {
            retData->AttachMatrix( m_pMatrix->Clone(), matTable);
        }
        return retData;
    }
}

CDistribFun *CTabularDistribFun::CPD_to_pi(CDistribFun *const*allPiMessages, 
                                            int *multParentIndices, int numMultNodes,
                                            int posOfExceptNode, int maximizeFlag )const
{
    PNL_CHECK_LEFT_BORDER( numMultNodes, 0 );
    if( numMultNodes > 0 )
    {
	PNL_CHECK_IS_NULL_POINTER( allPiMessages );
        PNL_CHECK_IS_NULL_POINTER( multParentIndices );
    }
    //we need to multipl by all this messages, and marginalize to all other
    int i;
    //some parents are in nodeNumbers
    if( numMultNodes > m_NumberOfNodes - 1 )
    {
        PNL_THROW( CInconsistentSize, "num nodes must be less than number of parents" )
    }
    if(( posOfExceptNode >=0 )&&( numMultNodes + 2 != m_NumberOfNodes ))
    {
        PNL_THROW( CInconsistentSize, "numNodes should connect" )
    }
    for( i = 0; i < numMultNodes; i++ )
    {
        if(allPiMessages[i]->GetDistributionType() != dtTabular )
        {
            PNL_THROW( CInvalidOperation, "Gaussian pi messages can't produce tabular pi" )
        }
    }
    intVector domNumbers( m_NumberOfNodes );
    for ( i = 0; i < m_NumberOfNodes; i++ )
    {
        domNumbers[i] = i;
    }
    CDistribFun *multData = this->ConvertCPDDistribFunToPot();
    CDistribFun* tempData = this->ConvertCPDDistribFunToPot();
    for( i = 0; i < numMultNodes; i++ )
    {
        multData->MultiplyInSelfData( &domNumbers.front(), multParentIndices+i,
            allPiMessages[multParentIndices[i]] );
    }
    intVector keepedNodes = intVector(1, domNumbers[m_NumberOfNodes-1]);
    if( posOfExceptNode >= 0 )
    {
        keepedNodes.push_back(posOfExceptNode);
    }
    tempData->MarginalizeData( multData, &keepedNodes.front(), keepedNodes.size(), 
        maximizeFlag );
    delete multData;
    return tempData;
}

void CTabularDistribFun::CPDToPi( CDistribFun *const* allPiMessages, 
             int *multParentIndices, int numMultNodes, CDistribFun** piMes,
             CDistribFun* cpdForMult, int posOfExceptParent, 
             int maximizeFlag )const 
{
    //we doesn't check sizes of cpdForMult and piMes - think its OK here
    PNL_CHECK_IS_NULL_POINTER( allPiMessages );
    PNL_CHECK_LEFT_BORDER( numMultNodes, 0 );
    if( numMultNodes > 0 )
    {
        PNL_CHECK_IS_NULL_POINTER( multParentIndices );
    }
    if( (*piMes)->GetDistributionType() != dtTabular)
    {
        PNL_THROW( CInconsistentType, "the result can be only tabular" )
    }
    if( cpdForMult->GetDistributionType() != dtTabular )
    {
        PNL_THROW( CInconsistentType, 
            "the distribution for multiply must be tabular" );
    }
    //we need to multipl by all this messages, and marginalize to all other
    int i;
    //some parents are in nodeNumbers
    if( numMultNodes > m_NumberOfNodes - 1 )
    {
        PNL_THROW( CInconsistentSize, "num nodes must be less than number of parents" )
    }
    if(( posOfExceptParent >=0 )&&( numMultNodes + 2 != m_NumberOfNodes ))
    {
        PNL_THROW( CInconsistentSize, "numNodes should connect" )
    }
    //set data to the object for multiply
    cpdForMult->GetMatrix(matTable)->SetDataFromOtherMatrix( GetMatrix(matTable) );
    intVector domNumbers( m_NumberOfNodes );
    for ( i = 0; i < m_NumberOfNodes; i++ )
    {
        domNumbers[i] = i;
    }
    for( i = 0; i < numMultNodes; i++ )
    {
        cpdForMult->MultiplyInSelfData( &domNumbers.front(), multParentIndices+i,
            allPiMessages[multParentIndices[i]] );
    }
    intVector keepedNodes = intVector(1, domNumbers[m_NumberOfNodes-1]);
    if( posOfExceptParent >= 0 )
    {
        keepedNodes.push_back(posOfExceptParent);
    }
    static_cast<CTabularDistribFun*>(*piMes)->Marginalize( cpdForMult,
        &keepedNodes.front(), keepedNodes.size(), maximizeFlag );
    (*cpdForMult) = ( *this );
}

CDistribFun * CTabularDistribFun::CPD_to_lambda(const CDistribFun *lambda, 
             CDistribFun *const* allPiMessages, int *multParentIndices, 
             int numMultNodes, int posOfExceptNode, int maximizeFlag )const
{
    int i;
    //some parents are in nodeNumbers
    if( numMultNodes > m_NumberOfNodes - 2 )
    {
        PNL_THROW( CInconsistentSize, 
            "numNodes is number of parents except one" )
    }
    if(( posOfExceptNode >=0 )&&( numMultNodes + 2 != m_NumberOfNodes ))
    {
        PNL_THROW( CInconsistentSize, "numNodes should connect" )
    }
    if( lambda->GetDistributionType() != dtTabular )
    {
        PNL_THROW( CInvalidOperation,
            "Gaussian lambda can't produce tabular message to parent" )
    }
    for( i = 0; i < numMultNodes; i++ )
    {
        if(allPiMessages[multParentIndices[i]]->GetDistributionType() != dtTabular )
        {
            PNL_THROW( CInvalidOperation, 
                "Gaussian pi messages can't produce Tabular message to parent" )
        }
    }
    CTabularDistribFun *resData = static_cast<CTabularDistribFun*>(Clone());
    CTabularDistribFun* tData = static_cast<CTabularDistribFun*>(resData->Clone());
    intVector allDomain = intVector( m_NumberOfNodes );
    for( i= 0; i < m_NumberOfNodes; i++)
    {
        allDomain[i] = i;
    }
    for( i = 0; i < numMultNodes; i++  )
    {
        resData->MultiplyInSelfData( &allDomain.front(), &multParentIndices[i],
            allPiMessages[multParentIndices[i]] );
    }
    int numThis = m_NumberOfNodes - 1;
    resData->MultiplyInSelfData( &allDomain.front(), &numThis, lambda );
    tData->MarginalizeData( resData, &posOfExceptNode, 1, maximizeFlag );
    delete resData;
    return tData;
}
void CTabularDistribFun::CPDToLambda( const CDistribFun *lambda, 
                 CDistribFun *const* allPiMessages, int *multParentIndices,
                 int numMultNodes, CDistribFun** lambdaMes,
                 CDistribFun* cpdForMult, int posOfExceptNode,
                 int maximizeFlag )const
{
    int i;
    //some parents are in nodeNumbers
    if( numMultNodes > m_NumberOfNodes - 2 )
    {
        PNL_THROW( CInconsistentSize, 
            "numNodes is number of parents except one" )
    }
    if(( posOfExceptNode >=0 )&&( numMultNodes + 2 != m_NumberOfNodes ))
    {
        PNL_THROW( CInconsistentSize, "numNodes should connect" )
    }
    if( lambda->GetDistributionType() != dtTabular )
    {
        PNL_THROW( CInvalidOperation,
            "Gaussian lambda can't produce tabular message to parent" )
    }
    if( (*lambdaMes)->GetDistributionType() != dtTabular)
    {
        PNL_THROW( CInconsistentType, "the result can be only tabular" )
    }
    if( cpdForMult->GetDistributionType() != dtTabular )
    {
        PNL_THROW( CInconsistentType, 
            "the distribution for multiply must be tabular" );
    }
    for( i = 0; i < numMultNodes; i++ )
    {
        if(allPiMessages[multParentIndices[i]]->GetDistributionType() != dtTabular )
        {
            PNL_THROW( CInvalidOperation, 
                "Gaussian pi messages can't produce Tabular message to parent" )
        }
    }
    //set data to the object for multiply
    cpdForMult->GetMatrix(matTable)->SetDataFromOtherMatrix( GetMatrix(matTable) );
    intVector allDomain = intVector( m_NumberOfNodes );
    for( i= 0; i < m_NumberOfNodes; i++)
    {
        allDomain[i] = i;
    }
    for( i = 0; i < numMultNodes; i++  )
    {
        cpdForMult->MultiplyInSelfData( &allDomain.front(), &multParentIndices[i],
            allPiMessages[multParentIndices[i]] );
    }
    int numThis = m_NumberOfNodes - 1;
    cpdForMult->MultiplyInSelfData( &allDomain.front(), &numThis, lambda );
    static_cast<CTabularDistribFun*>(*lambdaMes)->Marginalize( cpdForMult,
        &posOfExceptNode, 1, maximizeFlag );
    (*cpdForMult) = ( *this );

}

CDistribFun* CTabularDistribFun::GetNormalized() const
{
    if( m_bUnitFunctionDistribution )
    {
        CDistribFun* ret = CTabularDistribFun::CreateUnitFunctionDistribution(
            m_NumberOfNodes, &m_NodeTypes.front() );
        return ret;
    }
    if( !m_pMatrix )
    {
        PNL_THROW( CInvalidOperation, "we have no matrix yet" );
    }
    CDistribFun *retData = (CDistribFun*) new CTabularDistribFun(m_NumberOfNodes,
        &m_NodeTypes.front());
    CMatrix<float>* newMatr = m_pMatrix->NormalizeAll();
    retData->AttachMatrix( newMatr, matTable );
    return retData;
}

void CTabularDistribFun::Normalize() 
{
    if( m_bUnitFunctionDistribution )
    {
        return;
    }
    if( !m_pMatrix )
    {
        PNL_THROW( CInvalidOperation, "we have no matrix yet" );
    }
    else
    {
        if( GetMatricesValidityFlag() )
        {
            m_pMatrix->Normalize();
        }
        else
        {
            if(!IsValid())
            {
                PNL_THROW( CInvalidOperation, "we have no matrix valid matrix for normalization" );
            }
			else
			{
				m_pMatrix->Normalize();
			}
        }
    }
}

bool CTabularDistribFun::IsMatrixNormalizedForCPD(float eps) const
{
    int nnodes = m_NodeTypes.size();
    if( m_NodeTypes[nnodes - 1]->GetNodeState() == nsValue )
    {
        return 1;
    }
    if( !m_pMatrix )
    {
        return 0;
    }
    int i;
    bool ret = 1;
    //check it by summing by values for child
    int numRanges; 
    const int* ranges;
    m_pMatrix->GetRanges(&numRanges, &ranges);
    int childSize = ranges[numRanges - 1];
    if( m_pMatrix->GetMatrixClass() == mcNumericDense )
    {
        
        int rawDataSize;
        const float* rawData;
        (static_cast<CNumericDenseMatrix<float>*>(m_pMatrix))->GetRawData(
            &rawDataSize, &rawData );
        int t = 1;
        float sum = 0.0f;
        for( i = 0; i < rawDataSize; i++ )
        {
            if( i < t*childSize )
            {
                sum += rawData[i];
            }
            else
            {
                if( fabs(sum - 1.0f) > eps )
                {
                    ret = 0;
                    break;
                }
                sum = rawData[i];
                t++;
            }
        }
        if( fabs(sum - 1.0f) > eps )
        {
            ret = 0;
        }
    }
    else
    {
        if( m_pMatrix->GetMatrixClass() == mcNumericSparse )
        {
            //can convert to dense and do the same
            CMatrix<float>* pMatrix = m_pMatrix->ConvertToDense();
            int rawDataSize;
            const float* rawData;
            (static_cast<CNumericDenseMatrix<float>*>(pMatrix))->GetRawData(
                &rawDataSize, &rawData );
            int t = 1;
            float sum = 0.0f;
            for( i = 0; i < rawDataSize; i++ )
            {
                if( i < t*childSize )
                {
                    sum += rawData[i];
                }
                else
                {
                    if( fabs(sum - 1.0f) > eps )
                    {
                        ret = 0;
                        break;
                    }
                    sum = rawData[i];
                    t++;
                }
            }
            if( fabs(sum - 1.0f) > eps )
            {
                ret = 0;
            }
            delete pMatrix;
        }
    }
    return ret;
}

void CTabularDistribFun::InitPseudoCounts(int val)
{
    if (!m_pPseudoCounts)
    {
        int DomainSize; 
        const int *NodeSizes;
        m_pMatrix->GetRanges( &DomainSize, &NodeSizes );

        m_pPseudoCounts = m_pMatrix->CreateEmptyMatrix(DomainSize,
            NodeSizes, 0);
        void *pObj = this;
            m_pPseudoCounts->AddRef(pObj);
    }	
	m_pPseudoCounts->ClearData();
		
	if (val != 0)
	{
		CMatrixIterator<float>* iter = m_pPseudoCounts->InitIterator();
		for (iter; m_pPseudoCounts->IsValueHere(iter); m_pPseudoCounts->Next(iter))
		{
			intVector index;
			m_pPseudoCounts->Index(iter, &index);
			m_pPseudoCounts->SetElementByIndexes(1, &index.front());
		}
	}
}

double CTabularDistribFun::Gamma (int arg)
{
    double res = 1;
    for (int i = arg-1; i > 0; i--)
    {
        res = res*i;
    }
    return res;
}

float CTabularDistribFun::CalculateBayesianScore()
{
    const int * ranges;
    int dims;
    m_pPseudoCounts->GetRanges(&dims, &ranges);
        
    int StateNumber = ranges[0]; //number of node states

    int ConfigNum = 1; //number of discrete parents configurations
    
    int i, j;

    for (i = 1; i < dims; i++)
    {
        ConfigNum *= ranges[i];
    }
        
    int a = 0; 
    double b = 1;

    int * array;

    double score = 1.0f;

    double bmin = exp(50.0f);
    double bmax = exp(200.0f);
        
    const float * output;
    int length = 0;
    static_cast<CNumericDenseMatrix<float> *>(m_pPseudoCounts)->GetRawData(&length, &output);
        
    //calculating the product by the number of node states
    if (ConfigNum == 1)
    {
        score = 1; 
    }
    else
    {
    for (i=0; i < StateNumber; i++)
    {
        a = 0; b = 1;
        int counter;
        //calculating sum in denominator
        for (j=0, counter = i; j < ConfigNum; j++, counter += StateNumber)
        {
            a += int(output[counter]);
        }
        
        //forming the array to calculate 
        //product of factorials by the number of configurations
        array = new int [a];
        int k, curr = 0;
        for (j=0, counter = i; j < ConfigNum; j++, counter += StateNumber)
        {
            for (k=1; k <= int(output[counter]); k++)
            {
                array[curr] = k;
                curr++;
            }
        }

        a = a + ConfigNum;

        for (j = 0; j < curr; j++)
        {
            if ((b < bmax) || ((b > bmax) && (a == 0)))
            {
                b = b * array[j];
            }
            else
            {
                while ((b > bmin) && (a>0))
                {
                    b = b/a;
                    a--;
                }
            }
        }

  /*      for (j=0, counter = i; j < ConfigNum; j++, counter += StateNumber)
        {
            if ((b < bmax) || ((b > bmax) && (a == 0)))
            {
                b = b * Gamma(1 + int(output[counter]));
            }
            else
            {
                while ((b > bmin) && (a>0))
                {
                    b = b/a;
                    a--;
                }
            }
        }*/

        score = score * b * Gamma(ConfigNum) / Gamma(a);
        delete [] array;
    }
    }
    //score = log(score);
    return score;

}


CNodeValues *CTabularDistribFun::GetMPE() 
{
    if( m_bUnitFunctionDistribution )
    {
        m_pMatrix->SetUnitData();
    }
    if( !IsValid() )
    {
        PNL_THROW( CInvalidOperation, "we have no matrix yet or the matrix is invalid" );
    }
    int i;
    intVector obsGaussianPositions;
    obsGaussianPositions.reserve(m_NumberOfNodes);
    for( i = 0; i < m_NumberOfNodes; i++ )
    {
        if(( !m_NodeTypes[i]->IsDiscrete() )&&(m_NodeTypes[i]->GetNodeSize() == 0))
        {
            obsGaussianPositions.push_back(i);
        }
    }
    intVector indices;
    m_pMatrix->GetIndicesOfMaxValue(&indices);
    int numIndices = indices.size();
    valueVector maxValueIndex;
    maxValueIndex.resize(numIndices);
    for( i = 0; i < numIndices; i++ )
    {
        maxValueIndex[i].SetInt(indices[i]);
    }
    int numObsGauPos = obsGaussianPositions.size();
    for( i = numObsGauPos - 1; i >= 0; i-- )
    {
        maxValueIndex.erase( maxValueIndex.begin()+obsGaussianPositions[i] );
    }
    CNodeValues *resValues = CNodeValues::Create( m_NumberOfNodes,
        &m_NodeTypes.front(), maxValueIndex );
    return resValues;
}

CDistribFun* CTabularDistribFun::ConvertToSparse() const
{
    const CTabularDistribFun* self = this;
    CTabularDistribFun* resDistrib = Copy( self );
    resDistrib->m_bDense = 0;
    if( m_bUnitFunctionDistribution )
    {
        //need to create unit sparse matrix
        CMatrix<float>* matrix = CreateUnitFunctionMatrix(0, 1);
        void* pObj = resDistrib;
        if( resDistrib->m_pMatrix )
        {
            resDistrib->m_pMatrix->Release(pObj);
            resDistrib->m_pMatrix = matrix;
            matrix->AddRef(pObj);
        }
    }
    else
    {
        if( IsValid() )
        {
            resDistrib->AttachMatrix(m_pMatrix->ConvertToSparse(), matTable);
        }
        else
        {
            PNL_THROW( CInvalidOperation, "we have no matrix yet or the matrix is invalid" );
        }
    }
    if( m_pLearnMatrix )
    {
        delete resDistrib->m_pLearnMatrix;
        resDistrib->m_pLearnMatrix = m_pLearnMatrix->ConvertToSparse();
    }
    return resDistrib;
}

CDistribFun* CTabularDistribFun::ConvertToDense() const
{
    const CTabularDistribFun* self = this;
    CTabularDistribFun* resDistrib = Copy( self );
    resDistrib->m_bDense = 1;
    if( m_bUnitFunctionDistribution )
    {
        //need to create dense unit matrix
        CMatrix<float>* matrix = CreateUnitFunctionMatrix( 1, 1 );
        void* pObj = resDistrib;
        if( resDistrib->m_pMatrix )
        {
            resDistrib->m_pMatrix->Release(pObj);
            resDistrib->m_pMatrix = matrix;
            matrix->AddRef(pObj);
        }
    }
    else
    {
        if( IsValid() )
        {
            resDistrib->AttachMatrix(m_pMatrix->ConvertToDense(), matTable);
        }
        else
        {
            PNL_THROW( CInvalidOperation, "we have no matrix yet or the matrix is invalid" );
        }
    }
    if( m_pLearnMatrix )
    {
        delete resDistrib->m_pLearnMatrix;
        resDistrib->m_pLearnMatrix = m_pLearnMatrix->ConvertToDense();
    }
    return resDistrib;
}

void CTabularDistribFun::Dump() const
{
    Log dump("", eLOG_RESULT, eLOGSRV_PNL_POTENTIAL);

    int isSpecific = IsDistributionSpecific();
    if( isSpecific == 1 )
    {
        dump<<"I'm a uniform distribution.\n";
        return;
    }
    if( !IsValid() )
    {
        dump<<"I'm invalid distribution. I haven't matrix or matrix data is invalid.\n";
        return;
    }
    int i;
    EMatrixClass mc = m_pMatrix->GetMatrixClass();
    if(( mc == mcDense )||( mc == mcNumericDense )||( mc == mc2DNumericDense ))
    {
        dump<<"I have dense matrix with data. My matrix is:\n";
        const floatVector *myVector = (static_cast<CNumericDenseMatrix<float>*>(
                m_pMatrix))->GetVector();
        int vecSize = myVector->size();
        for( i = 0; i < vecSize; i++ )
        {
            dump<<(*myVector)[i]<<" ";
        }
        dump<<"\n";
    }
    else
    {
        CNumericDenseMatrix<float>* myMatrix = static_cast<
                    CNumericDenseMatrix<float>*>(m_pMatrix->ConvertToDense());
        dump<<"I have sparse matrix with data. My matrix is:\n";
        const floatVector *myVector = (myMatrix)->GetVector();
        int vecSize = myVector->size();
        for( i = 0; i < vecSize; i++ )
        {
            dump<<(*myVector)[i]<<" ";
        }
        dump<<"\n";
        delete myMatrix;
    }
    

}

int CTabularDistribFun::IsDense() const
{
    if( m_bUnitFunctionDistribution )
    {
        return m_bDense;
    }
    int mClass = m_pMatrix->GetMatrixClass();
    if(!(( mClass == mcSparse )||(mClass == mcNumericSparse)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int CTabularDistribFun::IsSparse() const
{
    if( m_bUnitFunctionDistribution )
    {
        return (!m_bDense);
    }
    int mClass = m_pMatrix->GetMatrixClass();
    if( ( mClass == mcSparse )||(mClass == mcNumericSparse) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int CTabularDistribFun::GetMultipliedDelta( const int **positions, 
                                            const float **values, const int **offsets )const
{
    *positions = NULL;
    *values = NULL;
    *offsets = NULL;
    return 0;
}

CMatrix<float>* CTabularDistribFun::CreateUnitFunctionMatrix( int isDense,
                                                              int withoutData ) const
{
    int i;
    int size;
    int dataSize = 1;
    intVector dims;
    dims.resize( m_NumberOfNodes );
    for( i = 0; i < m_NumberOfNodes; i++ )
    {
        size = (m_NodeTypes[i])->GetNodeSize();
        if(( size == 0 )&&( !m_NodeTypes[i]->IsDiscrete() ))
        {
            dims[i] = 1;
        }
        else
        {
            dims[i] = size;
            dataSize *= size;
        }
    }
    if( isDense && m_bDense )
    {   
        CNumericDenseMatrix<float>* mat = NULL;
        if( withoutData )
        {
            mat = CNumericDenseMatrix<float>::Create( 
                m_NumberOfNodes, &dims.front(), PNL_FAKEPTR(float));
        }
        else
        {
            floatVector data;
            data.assign( dataSize, 1.0f );
            mat = CNumericDenseMatrix<float>::Create( 
            m_NumberOfNodes, &dims.front(), &data.front());
        }
        return mat;
    }
    else
    {
        CNumericSparseMatrix<float>* resMat = CNumericSparseMatrix<float>::
            Create( m_NumberOfNodes, &dims.front() );
            /*int nlineBig = 1;
            //filling vector to convert from line index to multidimentional
            intVector pconvIndBig;
            pconvIndBig.assign( m_NumberOfNodes, 0 );
            for( int i1 = m_NumberOfNodes-1; i1 >= 0; i1--)
            {	
            pconvIndBig[i1] = nlineBig;
            int bigNodeSize = dims[i1];
            nlineBig = nlineBig*bigNodeSize;
            }
            div_t result;
            intVector pInd;
            pInd.resize( m_NumberOfNodes );
            float val = 1.0f;
            for( int i = 0; i < dataSize; i++ )
            {
            int hres = i;
            for( int k = 0; k < m_NumberOfNodes; k++ )
            {
            result = div( hres, pconvIndBig[k] );
            pInd[k] = result.quot;
            hres = result.rem;
            }
            resMat->SetElementByIndexes( val, &pInd.front() );
    }*/
        return resMat;
    }
}

//update Dirichlet factors 
//add counted numbers to pseudocounts table
void CTabularDistribFun::BayesUpdateFactor(const CEvidence* const* pEvidences,
                                            int EvidenceNumber, const int *domain )
{
    if( !pEvidences )
    {
        PNL_THROW( CNULLPointer, "evidences" )//no Evidences - NULL pointer
    }
    
    int DomainSize;
    const int *NodeSizes; 
    int i;
    if( !m_pPseudoCounts)
    {
        PNL_THROW( CNULLPointer, "Prior distribution was not specified" )
    }
    m_pPseudoCounts->GetRanges( &DomainSize, &NodeSizes );
    
    int *ranges = new int[DomainSize];
    int counter;
    int node;
    float element;
    
    for( int evNumber = 0; evNumber < EvidenceNumber; evNumber++ )
    {
    /* 
    find the meaning of every node from domain 
    and constract linear index of maltidmatrix for every evidence 
        */
        for( i = 0; i < DomainSize; i++ )
        {
            if( !pEvidences[evNumber] )
            {
                PNL_THROW( CNULLPointer, "evidence" )//no Evidence - NULL pointer
            }
            
            node = domain[i];
            counter = (pEvidences[evNumber]->GetValue(node))->GetInt();
            
            if( counter >= 0 && counter <  NodeSizes[i])
            {
                ranges[i] = counter;
            }
            else
            {
                
                PNL_THROW( CInconsistentSize, 
                    "observed value is larger then node size or negative") ; 
            }
        }
        /*
        find an element in the matrix which corresponding to this 
        evidence for every node in domain and then inc it
        */
        element = m_pPseudoCounts->GetElementByIndexes( ranges );
        element++;
        m_pPseudoCounts->SetElementByIndexes( element, ranges );
    }
    delete []ranges;
}

//Find target distribution by integrating over prior factor distr.
//In case of TabularCPD this is just to normalize Dirichlet pseudocounts 
void CTabularDistribFun::PriorToCPD()
{
    if( m_pMatrix->GetClampValue() ) 
    {
        return;
        //PNL_THROW( CBadArg, 
        //			"can't update clamped matrix") ; 
    }
    int DomainSize; //number of dimensions
    const int *NodeSizes;  //size of dimensions
    
    m_pPseudoCounts->GetRanges( &DomainSize, &NodeSizes );
    
    int prodParentsRanges = 1; //product of all parent dimensions
    for( int i = 0; i < DomainSize - 1; i++ )
    {
        prodParentsRanges *= NodeSizes[i];
    }         		
    // child(last) dimension size
    int ChildSize = NodeSizes[DomainSize - 1];
    
    //get pointers to prior matrix and CPT data
    const float* count; int len;
    static_cast<CNumericDenseMatrix<float>*>(m_pPseudoCounts)->GetRawData(&len,&count);
    
    float* table; int len2;
    static_cast<CNumericDenseMatrix<float>*>(m_pMatrix)->GetRawData(&len2,(const float**)(&table));
    
    if( len != len2 )
    {
        PNL_THROW( CInconsistentSize, 
            "prior and CPT matrices have different size") ; 
    }   
    
    for (int i1 = 0; i1 < prodParentsRanges ; i1++)
    {
        int k = i1*ChildSize;
        float Sum = 0.0f;
	int i2;
        for (i2 = 0; i2 < ChildSize ; i2++)
        {
            Sum = Sum + count[k+i2];
        }
        if( fabs( Sum ) < (1/FLT_MAX) )
        {
            for (i2 = 0; i2 < ChildSize; i2++)
            {
                table[k+i2] = (count[k+i2]);//to avoid dividing by zero?
            }
        }
        else
        {   
            for (i2 = 0; i2 < ChildSize; i2++)
            {
                table[k+i2] = (count[k+i2])/Sum; //need to optimize this?
            }
        }
    } 
}

int CTabularDistribFun::GetNumberOfFreeParameters() const
{
	CMatrix<float>* pMatrix;
	int i, ns, nDims;
	const int* Dims;

	pMatrix = this->GetMatrix(matTable);
	if( pMatrix->GetClampValue() )
	{
		return 0;
	}
	else
	{
		pMatrix->GetRanges(&nDims, &Dims);
		ns = Dims[nDims-1] - 1;
		for(i=0; i<nDims-1; i++)
		{
			ns *= Dims[i];
		}
		return ns;
	}
}

void CTabularDistribFun::SetCheckNegative(bool val)
{
  m_bCheckNegative = val;
}

#ifdef PAR_PNL
void CTabularDistribFun::UpdateStatisticsML(CDistribFun *pDF)
{
    if (pDF->GetDistributionType()!=dtTabular)
        PNL_THROW(CInconsistentType, 
        "Can not use function CTabularDistribFun::UpdateStatisticsML with wrong distribution type");

    int NumDims;
    const int *ranges;

    if(!m_pLearnMatrix)
    {
        int sz = 1;
        int i;
        const pConstNodeTypeVector *nt = GetNodeTypesVector();
        intVector nodeSizes;
        nodeSizes.resize(nt->size());
        for( i = 0; i < nt->size(); i++ )
        {
            nodeSizes[i] = (*nt)[i]->GetNodeSize();
            sz *= nodeSizes[i];
        }
        if( m_pMatrix )
        {
            m_pLearnMatrix = m_pMatrix->CreateEmptyMatrix(nodeSizes.size(),
                &nodeSizes.front(), 0); 
        }
        else
        {
            floatVector data(sz, 0.0f);
            m_pLearnMatrix = CNumericDenseMatrix<float>::Create(nodeSizes.size(),
                &nodeSizes.front(), &data.front() );
        }        
    }

    m_pLearnMatrix->GetRanges(&NumDims, &ranges);

    //  int NDims = m_pLearnMatrix->GetNumberDims();
    int *array = new int[NumDims];

    for (int y = 0; y < NumDims; y++)
    {
        array[y] = y;
    };

    m_pLearnMatrix->SumInSelf(pDF->GetStatisticalMatrix(stMatTable),
        NumDims, array);

    delete array;
};
#endif // PAR_OMP

#ifdef PNL_RTTI
const CPNLType CTabularDistribFun::m_TypeInfo = CPNLType("CTabularDistribFun", &(CDistribFun::m_TypeInfo));

#endif