/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlScalarDistribFun.cpp                                    //
//                                                                         //
//  Purpose:   CScalarDistribFun class member functions implementation    //                                                                    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlEvidence.hpp"
#include "pnlScalarDistribFun.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlLog.hpp"
//#include "pnlNumericDenseMatrix.hpp"
//#include "pnlNumericSparseMatrix.hpp"

PNL_USING

CScalarDistribFun*
CScalarDistribFun::Create(int NodeNumber, const CNodeType *const* NodeTypes,
                          int asDense )
{
    PNL_CHECK_IS_NULL_POINTER( NodeTypes );
    PNL_CHECK_RANGES( asDense, 0, 1 );
    
    CScalarDistribFun* resDistr = new CScalarDistribFun( NodeNumber,
        NodeTypes, asDense );
    PNL_CHECK_IF_MEMORY_ALLOCATED(resDistr);
    return resDistr;
}

CScalarDistribFun* CScalarDistribFun::Copy(const CScalarDistribFun *pInpDistr)
{
    PNL_CHECK_IS_NULL_POINTER(pInpDistr);
    
    CScalarDistribFun* resDistr = new CScalarDistribFun(*pInpDistr);
    PNL_CHECK_IF_MEMORY_ALLOCATED(resDistr);
    return resDistr;
    
}

CDistribFun& CScalarDistribFun::operator=(const CDistribFun& pInputDistr)
{
    //check node types and do nothing
    if( this == &pInputDistr  )
    {
        return *this;
    }
    if( pInputDistr.GetDistributionType() != dtScalar )
    {
        PNL_THROW( CInvalidOperation,
            "input distribution must be Scalar if the left operand is Scalar" )
    }
    int i;
    int isTheSame = 1;
    const CScalarDistribFun& pSInputDistr = static_cast<const CScalarDistribFun&>(pInputDistr);
    if( m_bUnitFunctionDistribution != pSInputDistr.m_bUnitFunctionDistribution )
    {
        isTheSame = 0;
    }
    else
    {
        if( m_NumberOfNodes != pSInputDistr.m_NumberOfNodes )
        {
            isTheSame = 0;
        }
        else
        {
            const pConstNodeTypeVector* ntInput = pSInputDistr.GetNodeTypesVector();
            for( i = 0; i < m_NumberOfNodes; i++ )
            {
                if( (*m_NodeTypes[i]) != (*(*ntInput)[i]) )
                {
                    isTheSame = 0;
                    break;
                }
            }
        }
    }
    if( isTheSame )
    {
        return *this;
    }
    else
    {
        PNL_THROW( CInvalidOperation, 
            "input distribution must be of the same types as right hand distribution" )
    }
}

CDistribFun* CScalarDistribFun::Clone() const
{
    if( !IsValid() )
    {
        PNL_THROW( CInvalidOperation, "can't clone invalid data" );
    }
    CDistribFun *retData = new CScalarDistribFun( m_NumberOfNodes, 
        &m_NodeTypes.front() );
    return retData;
}

CDistribFun* CScalarDistribFun::CloneWithSharedMatrices()
{
    if( !IsValid() )
    {
        PNL_THROW( CInvalidOperation, "can't clone invalid data" );
    }
    CDistribFun *retData = new CScalarDistribFun( m_NumberOfNodes, 
        &m_NodeTypes.front() );
    return retData;
}

void CScalarDistribFun::CreateDefaultMatrices( int /*typeOfMatrices*/  )
{
    //do nothing
}


void CScalarDistribFun::AllocMatrix( const float *data, EMatrixType mType, 
                                     int numberOfWeightMatrix ,
                                     const int *parentIndices )
{
    PNL_THROW( CInvalidOperation, "scalar distribution naven't any matrices" );
}
void CScalarDistribFun::AttachMatrix( CMatrix<float> *pMatrix,
                                      EMatrixType mType,
                                      int numberOfWeightMatrix ,
                                      const int *parentIndices,
                                      bool isMultipliedByDelta )
{
    PNL_THROW( CInvalidOperation, "scalar distribution naven't any matrices" );
}
bool CScalarDistribFun::IsValid(std::string* description) const
{
    return 1;
}
CMatrix<float>* CScalarDistribFun::GetMatrix( EMatrixType mType, 
                                              int numWeightMat,
                                              const int *parentIndices ) const
{
    PNL_THROW( CInvalidOperation, "scalar distribution haven't any matrices" );
    
    return NULL;
}
CMatrix<float>* CScalarDistribFun::GetStatisticalMatrix( EStatisticalMatrix mType, 
                                                   int *parentIndices ) const
{
    PNL_THROW( CInvalidOperation, "scalar distribution naven't any matrices" );
    
    return NULL;
}
void CScalarDistribFun::MarginalizeData( const CDistribFun* pOldData, 
                                         const int *DimOfKeep, 
                                         int NumDimsOfKeep, int maximize )
{
    
    //need only to extract node types
    m_NumberOfNodes = NumDimsOfKeep;
    //we need to choose corresponding pointers to node types
    m_NodeTypes.resize( NumDimsOfKeep );
    const pConstNodeTypeVector* nt = pOldData->GetNodeTypesVector();
    int i;
    for( i = 0; i < NumDimsOfKeep; i++ )
    {
        m_NodeTypes[i] = (*nt)[DimOfKeep[i]];
    }
    if( pOldData->GetDistributionType() == dtScalar )
    {
        m_bUnitFunctionDistribution = 0;
        return;
    }
    else
    {
        //need to check - if all keeped dims shrink - its really scalar
        for( i = 0; i < NumDimsOfKeep; i++ )
        {
            int isTab = m_NodeTypes[i]->IsDiscrete();
            int size = m_NodeTypes[i]->GetNodeSize();
            if(!(( isTab && (size == 1) )||(!isTab &&(size == 0))))
            {
                PNL_THROW( CInvalidOperation,
                    "result is scalar iff all nodes in it observed" );
            }
        }
    }
    
}
void CScalarDistribFun::ShrinkObservedNodes( const CDistribFun* pOldData,
                                             const int *pVarsObserved, 
                                             const Value* const* pObsValues, int numObsVars,
                                             const CNodeType* pObsTabNT, const CNodeType* pObsGauNT )
{
    //need to change node types & check all nodes become observed if it wasn't scalar
    
}
void CScalarDistribFun::ExpandData( const int* pDimsToExpand, 
                                    int numDimsToExpand, const Value* const* valuesArray, 
                                    const CNodeType* const*allFullNodeTypes, int UpdateCanonical )
{
    //need to change node types
}

CDistribFun* CScalarDistribFun::ExpandScalarData( EDistributionType dtRequested, 
                                                 const int* pDimsToExpand, 
                                                 int numDimsToExpand, const Value* const* valuesArray, 
                                                 const CNodeType* const*allFullNodeTypes, int UpdateCanonical )const
{
    PNL_CHECK_IS_NULL_POINTER( pDimsToExpand );
    PNL_CHECK_IS_NULL_POINTER( valuesArray );
    PNL_CHECK_IS_NULL_POINTER( allFullNodeTypes );
    if((( dtRequested == dtTabular )||( dtRequested == dtGaussian ))
        &&(numDimsToExpand != GetNumberOfNodes()))
    {
        PNL_THROW( CInconsistentType, "can create potential of requested type" )
    }
    //needn't check correspondence between node types and requested distribution type
    int i;
    CDistribFun* resDistr = NULL;
    if( dtRequested == dtTabular )
    {
        resDistr = CTabularDistribFun::Create( numDimsToExpand, allFullNodeTypes, NULL );
        //create matrix
        int dataSize = 1;
        intVector dims;
        dims.resize(numDimsToExpand);
        intVector observedIndices;
        observedIndices.resize(numDimsToExpand);
        for( i = 0; i < numDimsToExpand; i++ )
        {
            int size = allFullNodeTypes[i]->GetNodeSize();
            dims[i] = size;
            dataSize*= size;
            observedIndices[i] = valuesArray[i]->GetInt();
        }
        CMatrix<float>* mat = NULL;
        if( m_bDense )
        {
            floatVector data;
            data.assign(dataSize, 0.0f);
            mat = CNumericDenseMatrix<float>::Create( numDimsToExpand,
                &dims.front(), &data.front() );
        }
        else
        {
            mat = CNumericSparseMatrix<float>::Create( numDimsToExpand,
                &dims.front() );
        }
        mat->SetElementByIndexes( 1.0f, &observedIndices.front() );
        resDistr->AttachMatrix( mat, matTable );
    }
    if( dtRequested == dtGaussian )
    {
        int allLength = 0;
        intVector floatSizes;
        floatSizes.resize(numDimsToExpand);
        for( i = 0; i < numDimsToExpand; i++ )
        {
            floatSizes[i] = allFullNodeTypes[i]->GetNodeSize();
            allLength += floatSizes[i];
        }
        floatVector meanData;
        meanData.assign(allLength, 0.0);
        int offset = 0;
        for( i = 0; i < numDimsToExpand; i++ )
        {
            const Value* val = valuesArray[i];
            int numCopy = floatSizes[i];
            for( int j = 0; j < numCopy; j++ )
            {
                meanData[offset+j] = val[j].GetFlt();
            }
            offset += numCopy;
        }
        resDistr = CGaussianDistribFun::CreateDeltaDistribution( 
            numDimsToExpand, allFullNodeTypes, &meanData.front() );
        if(UpdateCanonical)
        {
            static_cast<CGaussianDistribFun*>(resDistr)->UpdateCanonicalForm();
        }
    }
    return resDistr;
}

void CScalarDistribFun::SumInSelfData(const int *pBigDomain,
  const int *pSmallDomain, const CDistribFun *pOtherData)
{
  PNL_THROW(CNotImplemented, "haven't for CScalarDistribFun now");
}

void CScalarDistribFun::MultiplyInSelfData( const int *pBigDomain, 
                                            const int *pSmallDomain, const CDistribFun *pOtherData )
{
    //can multiply iff both scalars of same types
    //(other - need to multiply in self for second distribution functions)
    if( pOtherData->GetDistributionType() != dtScalar )
    {
        PNL_THROW( CInconsistentType, 
            "multiplying of different types doesn't support - see potential level" );
    }
    return;
}

void CScalarDistribFun::DivideInSelfData( const int *pBigDomain, 
                                          const int *pSmallDomain, const CDistribFun *pOtherData )
{
    //can multiply iff both scalars of same types
    //(other - need to multiply in self for second distribution functions)
    if( pOtherData->GetDistributionType() != dtScalar )
    {
        PNL_THROW( CInconsistentType, 
            "multiplying of different types doesn't support - see potential level" );
    }
    return;
}

void CScalarDistribFun::UpdateStatisticsEM( const CDistribFun* infData,
                                     const CEvidence *pEvidence, float weightingCoeff,
                                     const int* domain )
{
}
void CScalarDistribFun::UpdateStatisticsML(const CEvidence* const* pEvidences, int EvidenceNumber, 
                                     const int *domain, float weightingCoeff )
{
}
float CScalarDistribFun::ProcessingStatisticalData( float numEvidences )
{
    return 0.0f;
}

int CScalarDistribFun::GetMultipliedDelta( const int **positions, const float **values, 
                                           const int **offsets ) const
{
    return 0;
}
void CScalarDistribFun::ClearStatisticalData()
{
}
int CScalarDistribFun::IsEqual( const CDistribFun *dataToCompare, float epsilon, 
                                int withCoeff, float* maxDifference ) const
{
    //can compare only node types
    int isTheSame = 1;
    if( m_NumberOfNodes != dataToCompare->GetNumberOfNodes() )
    {
        isTheSame = 0;
    }
    else
    {
        const pConstNodeTypeVector* ntInput = dataToCompare->GetNodeTypesVector();
        for( int i = 0; i < m_NumberOfNodes; i++ )
        {
            if( (*m_NodeTypes[i]) != (*(*ntInput)[i]) )
            {
                isTheSame = 0;
                break;
            }
        }
    }
    if( maxDifference )
    {
        *maxDifference = 0.0f;
    }
    return isTheSame;
}
CDistribFun* CScalarDistribFun::ConvertCPDDistribFunToPot()const
{
    //do nothing - it is only potential
    return NULL;
}
CDistribFun* CScalarDistribFun::CPD_to_pi( CDistribFun *const* allPiMessages, 
                                           int *multParentIndices, int numMultNodes, int posOfExceptParent, 
                                           int maximizeFlag )const
{
    //do nothing - it is only potential
    return NULL;
}

CDistribFun* CScalarDistribFun::CPD_to_lambda( const CDistribFun *lambda, 
                                               CDistribFun *const* allPiMessages, int *multParentIndices,
                                               int numMultNodes, int posOfExceptNode, int maximizeFlag )const
{
    //do nothing - it is only potential
    return NULL;
}
CDistribFun* CScalarDistribFun::GetNormalized()const
{
    //create copy
    const CScalarDistribFun* self = this;
    CScalarDistribFun* resDistrib = CScalarDistribFun::Copy( self );
    return resDistrib;
}

void CScalarDistribFun::Normalize()
{
    //do nothing - its normalized;
}
CNodeValues* CScalarDistribFun::GetMPE()
{
    //haven't MPE
    return NULL;
}
//methods to convert distribution with dense matrices to distribution with sparse
//if its already sparse - return copy
CDistribFun* CScalarDistribFun::ConvertToSparse() const
{
    //create copy - it is dense and sparse 
    const CScalarDistribFun* self = this;
    CScalarDistribFun* resDistrib = CScalarDistribFun::Copy( self );
    return resDistrib;
}

//methods to convert distribution with sparse matrices to distribution with dense
//if its already dense - return copy
CDistribFun* CScalarDistribFun::ConvertToDense() const
{
    //create copy
    const CScalarDistribFun* self = this;
    CScalarDistribFun* resDistrib = CScalarDistribFun::Copy( self );
    return resDistrib;
}

void CScalarDistribFun::Dump() const
{
    Log dump("", eLOG_RESULT, eLOGSRV_PNL_POTENTIAL);

    dump<<"I'm a Scalar distribution function of "<< m_NumberOfNodes <<" nodes.";
    dump<<"I haven't any matrices. \n";
}

int CScalarDistribFun::IsSparse() const
{
    return 1-m_bDense;
}
int CScalarDistribFun::IsDense() const
{
    return m_bDense;
}

CScalarDistribFun::CScalarDistribFun( int NodeNumber,
                                     const CNodeType *const* nodeTypes,
                                     int asDense):
CDistribFun( dtScalar, NodeNumber, nodeTypes, 0 ), m_bDense(asDense)
{
}

CScalarDistribFun::CScalarDistribFun( const CScalarDistribFun &inpDistr ):
CDistribFun( dtScalar )
{
    m_NumberOfNodes = inpDistr.m_NumberOfNodes;
    m_NodeTypes.assign( inpDistr.m_NodeTypes.begin(),inpDistr.m_NodeTypes.end() );
    m_bUnitFunctionDistribution = 0;
    m_bDense = inpDistr.IsDense();
    //m_bUnitFunctionDistribution = 1;
}


void CScalarDistribFun::SetStatistics( const CMatrix<float> *pMat, 
        EStatisticalMatrix matrix, const int* parentsComb)
{
    PNL_THROW(CAlgorithmicException, "scalar distrib function couldn't have statistic")
}

int CScalarDistribFun::GetNumberOfFreeParameters()const
{
        PNL_THROW( CInvalidOperation, "ScalarDistribution does not has free parameters");
}

#ifdef PAR_PNL
void CScalarDistribFun::UpdateStatisticsML(CDistribFun *pPot)
{
    PNL_THROW(CNotImplemented, 
        "UpdateStatisticsML for CScalarDistribFun not implemented yet");
};
#endif // PAR_OMP

#ifdef PNL_RTTI
const CPNLType CScalarDistribFun::m_TypeInfo = CPNLType("CScalarDistribFun", &(CDistribFun::m_TypeInfo));

#endif