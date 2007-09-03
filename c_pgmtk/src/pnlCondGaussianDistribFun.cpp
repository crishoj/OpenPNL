/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlCondGaussianDistribFun.cpp                               //
//                                                                         //
//  Purpose:   CCondGaussianDistribFun class member functions implementation//
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlMatrix_impl.hpp" // since we'll use CMatrix<CGaussianDistribFun*>
#include "pnlCondGaussianDistribFun.hpp"
#include "pnlSparseMatrix.hpp"
#include "pnlDenseMatrix.hpp"
#include "pnlNumericDenseMatrix.hpp"
#include "pnlLog.hpp"
#include <sstream>

PNL_USING

CCondGaussianDistribFun* CCondGaussianDistribFun::Create( int isFactor,
                                                         int nNodes, const CNodeType* const* nodeTypes,
                                                         int asDenseMatrix ,
                                                         CGaussianDistribFun* const pDefaultDistr)
{
    PNL_CHECK_IS_NULL_POINTER( nodeTypes );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );
    PNL_CHECK_RANGES( isFactor, 0, 1 );
    if( !asDenseMatrix )
    {
        PNL_CHECK_IS_NULL_POINTER(pDefaultDistr);
        if( !pDefaultDistr->IsValid() )
        {
            PNL_THROW( CInconsistentType, "default distribution must be valid" );
        }
    }
    
    if( nodeTypes[nNodes - 1]->IsDiscrete() )
    {
        PNL_THROW( CInvalidOperation,
            "conditional Gaussian must have last node continuous" )
    }
    
    CCondGaussianDistribFun *resDistr = new CCondGaussianDistribFun( isFactor,
        nNodes, nodeTypes, asDenseMatrix, pDefaultDistr  );
    PNL_CHECK_IF_MEMORY_ALLOCATED(resDistr);
    
    return resDistr;
}

CCondGaussianDistribFun*
CCondGaussianDistribFun::Copy(const CCondGaussianDistribFun* pInputDistr )
{
    PNL_CHECK_IS_NULL_POINTER( pInputDistr );
    
    CCondGaussianDistribFun* resDistr = new CCondGaussianDistribFun( *pInputDistr );
    PNL_CHECK_IF_MEMORY_ALLOCATED( resDistr );
    
    return resDistr;
}

//typeOfMatrices == 1 - randomly created matrices
//creates Gaussian distribution in moment form
void CCondGaussianDistribFun::CreateDefaultMatrices( int typeOfMatrices )
{
    PNL_CHECK_RANGES( typeOfMatrices, 1, 1 );
    //we have only one type of matrices now
    if( m_bUnitFunctionDistribution )
    {
        PNL_THROW( CInconsistentType,
            "uniform distribution can't have any matrices with data" );
    }
    //m_distribution - already exists, need to fill it
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
    
    pConstNodeTypeVector contParentsTypes;
    int numContParents = m_contParentsIndex.size();
    contParentsTypes.resize( numContParents + 1 );
    for( i = 0; i < numContParents; i++ )
    {
        contParentsTypes[i] = m_NodeTypes[m_contParentsIndex[i]];
    }
    contParentsTypes[numContParents] = m_NodeTypes[m_NumberOfNodes - 1];
    
    div_t result;
    intVector index;
    index.resize( numRanges );
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
        CGaussianDistribFun* theDistr = m_distribution->GetElementByIndexes(&index.front());
        theDistr = CGaussianDistribFun::CreateInMomentForm(
            m_bPotential, numContParents + 1, &contParentsTypes.front(),
            NULL, NULL, NULL );
        theDistr->CreateDefaultMatrices(typeOfMatrices);
        m_distribution->SetElementByIndexes( theDistr, &index.front() );
    }
    
}


CDistribFun& CCondGaussianDistribFun::operator=(const CDistribFun& pInputDistr)
{
    if( &pInputDistr == this )
    {
        return *this;
    }
    if( pInputDistr.GetDistributionType() != dtCondGaussian )
    {
        PNL_THROW( CInvalidOperation,
            "input distribution must be Conditional Gaussian " );
    }
    const CCondGaussianDistribFun &pGInputDistr =
        static_cast<const CCondGaussianDistribFun&>(pInputDistr);
    int i;
    int isTheSame = 1;
    if( pGInputDistr.m_numberOfDims != m_numberOfDims )
    {
        isTheSame = 0;
    }
    if( m_NumberOfNodes != pGInputDistr.m_NumberOfNodes )
    {
        isTheSame = 0;
    }
    else
    {
        const pConstNodeTypeVector* ntInput = pInputDistr.GetNodeTypesVector();
        for( i = 0; i < m_NumberOfNodes; i++ )
        {
            if( (*ntInput)[i] != m_NodeTypes[i] )
            {
                isTheSame = 0;
                break;
            }
        }
        if( m_bPotential != pGInputDistr.m_bPotential )
        {
            isTheSame = 0;
        }
    }
    if( m_distribution->GetMatrixClass() != pGInputDistr.m_distribution->GetMatrixClass() )
    {
        isTheSame = 0;
    }
    if( !IsValid() || !pGInputDistr.IsValid())
    {
        isTheSame = 0;
    }
    if( isTheSame )
    {
        CMatrixIterator<CGaussianDistribFun*>* iterChanging =
                m_distribution->InitIterator();
        for( iterChanging; m_distribution->IsValueHere( iterChanging );
                m_distribution->Next(iterChanging) )
        {
            CGaussianDistribFun* changingData =
                *(m_distribution->Value( iterChanging ));
            intVector index;
            m_distribution->Index( iterChanging, &index );
            CGaussianDistribFun* formingData =
                pGInputDistr.m_distribution->GetElementByIndexes(
                &index.front() );
            *(CDistribFun*)changingData = *(CDistribFun*)formingData;
        }
        delete iterChanging;
        if( m_distribution->GetMatrixClass() == mcSparse )
        {
            //set default distribution
            CGaussianDistribFun* pDefault = 
                static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
                m_distribution)->GetDefaultValue();
            const CGaussianDistribFun* pFormingDefault = 
                static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
                pGInputDistr.m_distribution)->GetDefaultValue();
            *(CDistribFun*)pDefault = *(CDistribFun*)pFormingDefault;
        }
    }
    else
    {
        PNL_THROW( CInvalidOperation,
            "distributions must be valid distributions of the same form & sizes" );
    }
    return *this;
}

CDistribFun* CCondGaussianDistribFun::Clone() const
{
    if( !IsValid() )
    {
        PNL_THROW( CInconsistentState, "can't clone invalid data" );
    }
    //call copy method
    CCondGaussianDistribFun* resData = CCondGaussianDistribFun::Copy( this );
    return resData;
    
}

CDistribFun* CCondGaussianDistribFun::CloneWithSharedMatrices()
{
    if( !IsValid() )
    {
        PNL_THROW( CInconsistentState, "can't clone invalid data" );
    }
    CCondGaussianDistribFun *resData = NULL;
    if( m_distribution->GetMatrixClass() == mcDense )
    {
        resData = CCondGaussianDistribFun::Create(
            m_bPotential, m_NumberOfNodes, &m_NodeTypes.front() );
    }
    else
    {
        //if( m_distribution->GetMatrixClass() == mcSparse )
        CGaussianDistribFun* pDefault = 
            static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
            m_distribution)->GetDefaultValue();
        resData = CCondGaussianDistribFun::Create( 
            m_bPotential, m_NumberOfNodes, &m_NodeTypes.front(), 0, pDefault);
    }
    CMatrixIterator<CGaussianDistribFun*>* iterForming =
        m_distribution->InitIterator();
    for( iterForming; m_distribution->IsValueHere( iterForming );
        m_distribution->Next(iterForming) )
    {
        CGaussianDistribFun* formingData =
            *(m_distribution->Value( iterForming ));
        CGaussianDistribFun* clonedWithSharedData =
            static_cast<CGaussianDistribFun*>(
            formingData->CloneWithSharedMatrices());
        intVector index;
        m_distribution->Index( iterForming, &index );
        resData->m_distribution->SetElementByIndexes( clonedWithSharedData,
            &index.front() );
    }
    delete iterForming;
    //(resData->m_distribution)->AddRef(resData);
    return resData;
}

void CCondGaussianDistribFun::AllocDistribFun(
                                              const int* discreteParentCombination, int isMoment ,
                                              int isDelta , int isUniform )
{
    PNL_CHECK_IS_NULL_POINTER( discreteParentCombination );
    if( isDelta && isUniform )
    {
        PNL_THROW( CInvalidOperation,
            "distribution can't be both Delta & Uniform" );
    }
    PNL_CHECK_RANGES( isMoment, 0, 1 );
    if( m_bPotential && !isMoment)
    {
        PNL_THROW( CInconsistentType, "CPD can be only in moment form" );
    }
    
    //find corresponding node types
    pConstNodeTypeVector contParentsTypes;
    int numContParents = m_contParentsIndex.size();
    contParentsTypes.resize( numContParents + 1 );
    for( int i = 0; i < numContParents; i++ )
    {
        contParentsTypes[i] = m_NodeTypes[m_contParentsIndex[i]];
    }
    contParentsTypes[numContParents] = m_NodeTypes[m_NumberOfNodes - 1];
    CGaussianDistribFun* theDistr =  m_distribution->GetElementByIndexes(
            discreteParentCombination );
    int isExist = 1;
    if( m_distribution->GetMatrixClass() == mcSparse )
    {
        isExist = static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
            m_distribution)->IsExistingElement(discreteParentCombination);
    }
    else
    {
        if( !theDistr )
        {
            isExist = 0;
        }
    }
    if( isExist )
    {
        delete theDistr;
    }
    //detect which of distributions should be allocated
    if( isDelta )
    {
        theDistr = CGaussianDistribFun::CreateDeltaDistribution(
            numContParents + 1, &contParentsTypes.front(), NULL,
            isMoment, m_bPotential );
    }
    else
    {
        if( isUniform )
        {
            theDistr = CGaussianDistribFun::CreateUnitFunctionDistribution(
                numContParents + 1 , &contParentsTypes.front(),
                m_bPotential, 1-isMoment );
        }
        else
        {
            if( isMoment )
            {
                theDistr = CGaussianDistribFun::CreateInMomentForm(
                    m_bPotential, numContParents + 1,
                    &contParentsTypes.front(), NULL, NULL, NULL );
            }
            else
            {
                theDistr = CGaussianDistribFun::CreateInCanonicalForm( numContParents + 1,
                                                                       &contParentsTypes.front(),
                                                                       0, 0, 0.f );
            }
        }
    }
    m_distribution->SetElementByIndexes( theDistr, discreteParentCombination );
}

const CGaussianDistribFun*
CCondGaussianDistribFun::GetDistribution(const int* discrParentIndex ) const
{
    PNL_CHECK_IS_NULL_POINTER( discrParentIndex );
    
    const CGaussianDistribFun* distr = m_distribution->GetElementByIndexes(
        discrParentIndex );
    return distr;
}

void CCondGaussianDistribFun::SetDistribFun(CGaussianDistribFun* const inputDistr,
                                            const int *discreteParentCombination)
{
    PNL_CHECK_IS_NULL_POINTER( inputDistr );
    PNL_CHECK_IS_NULL_POINTER( discreteParentCombination );
    
    //check ranges for parents
    int numDims;
    const int* ranges;
    m_distribution->GetRanges( &numDims, &ranges);
    for( int i = 0; i < numDims; i++ )
    {
        PNL_CHECK_RANGES( discreteParentCombination[i], 0, ranges[i] - 1 );
    }
    
    //check validity of inputDistr
    if( inputDistr->GetDistributionType() != dtGaussian )
    {
        PNL_THROW( CInconsistentType, "we can set only Gaussian" );
    }
    if( inputDistr->GetFactorFlag() != m_bPotential )
    {
        PNL_THROW( CInconsistentType,
            "we can set only data in the same form as we are");
    }
    if( !inputDistr->GetMomentFormFlag() )
    {
        PNL_THROW( CInconsistentType, "we can set only data in moment form" );
    }
    int isExist = 1;
    if( m_distribution->GetMatrixClass() == mcSparse )
    {
        isExist = static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
            m_distribution)->IsExistingElement(discreteParentCombination);
    }
    CGaussianDistribFun* distr = m_distribution->GetElementByIndexes(
        discreteParentCombination);
    if( isExist )
    {
        delete distr;
    }
    m_distribution->SetElementByIndexes(
        CGaussianDistribFun::Copy( inputDistr ), discreteParentCombination );
}

bool CCondGaussianDistribFun::IsValid(std::string* description) const
{
    bool ret = 1;
    if( m_discrParentsIndex.size() != size_t(m_distribution->GetNumberDims()))
    {
        if( description )
        {
            (*description) = "Conditional Gaussain distribution have inconsistent number of Gaussian distributions. It's number should be the same as number of different combination of values of discrete parents. ";
        }
        ret = 0;
        return ret;
    }
    CMatrixIterator<CGaussianDistribFun*>* iter = m_distribution->InitIterator();
    for( iter; m_distribution->IsValueHere( iter ); m_distribution->Next(iter))
    {
        CGaussianDistribFun* val = *(m_distribution->Value( iter ));
        if( !val )
        {
            ret = 0;
            if( description )
            {
                intVector index;
                m_distribution->Index(iter, &index);
                int indexSize = index.size();
                int i;
                std::stringstream st;
                st<<"Conditional Gaussian distribution haven't Gaussain ";
                st<<"distribution for this combination of discrete parents: \n";
                for( i = 0; i < indexSize; i++ )
                {
                    st<< index[i];
                }
                st<<std::endl;
                std::string s = st.str();
                description->insert( description->begin(), s.begin(), s.end() );
            }
            break;
        }
        else
        {
            if(!val->IsValid(description))
            {
                ret = 0;
                if( description )
                {
                    intVector index;
                    m_distribution->Index(iter, &index);
                    int indexSize = index.size();
                    int i;
                    std::stringstream st;
                    st<<"Conditional Gaussian distribution have invalid Gaussain ";
                    st<<"distribution for this combination of discrete parents: ";
                    for( i = 0; i < indexSize; i++ )
                    {
                        st<< index[i];
                    }
                    st<<std::endl;
                    std::string s = st.str();
                    description->insert( description->begin(), s.begin(), s.end() );
                }
                break;
            }
        }
    }
    delete iter;
    return ret;
    if( m_distribution->GetMatrixClass() == mcSparse )
    {
        if( ! static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
            m_distribution)->GetDefaultValue()->IsValid() )
        {
            std::stringstream st;
            st<<"Conditional Gaussian distribution have invalid default distribution"<<std::endl;
            std::string s = st.str();
            description->insert(description->begin(), s.begin(), s.end());
            ret = 0;
        }
    }
    return ret;
}

void
CCondGaussianDistribFun::AllocMatrix(const float *data, EMatrixType mType,
                                     int numberOfWeightMatrix,
                                     const int *parentIndices)
{
    PNL_CHECK_IS_NULL_POINTER( data );
    PNL_CHECK_IS_NULL_POINTER( parentIndices );
    
    if( mType == matTable )
    {
        PNL_THROW( CInconsistentType,
            "can't alloc Tabular matrix to Conditional Gaussian" );
    }
    CGaussianDistribFun* thisDistr = m_distribution->GetElementByIndexes(
            parentIndices );
    int isExist = 1;
    if( m_distribution->GetMatrixClass() == mcSparse )
    {
        isExist = static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
            m_distribution)->IsExistingElement(parentIndices);
    }
    else
    {
        if( !thisDistr )
        {
            isExist = 0;
        }
    }
    if( !isExist )
    {
        //alloc distribution function for corresponding indices
        AllocDistribFun( parentIndices );
        thisDistr = m_distribution->GetElementByIndexes( parentIndices );
    }
    thisDistr->AllocMatrix(data, mType, numberOfWeightMatrix);
}

void
CCondGaussianDistribFun::AttachMatrix(CMatrix<float> *pMatrix, EMatrixType mType,
                                      int numberOfWeightMatrix,
                                      const int *parentIndices,
                                      bool)
{
    PNL_CHECK_IS_NULL_POINTER( pMatrix );
    PNL_CHECK_IS_NULL_POINTER( parentIndices );

    if(!(( mType == matMean )||(mType == matCovariance)||( mType == matH )||
        (mType == matK)||( mType == matWeights)||(mType == matWishartMean)||
	(mType == matWishartCov) ) )
    {
        PNL_THROW( CInconsistentType,
            "can't attach Tabular matrix to Conditional Gaussian" );
    }
    CGaussianDistribFun* thisDistr = m_distribution->GetElementByIndexes(
        parentIndices );
    int isExist = 1;
    if( m_distribution->GetMatrixClass() == mcSparse )
    {
        isExist = static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
            m_distribution)->IsExistingElement(parentIndices);
    }
    else
    {
        if( !thisDistr )
        {
            isExist = 0;
        }
    }
    if( !isExist )
    {
        AllocDistribFun( parentIndices );
        thisDistr = m_distribution->GetElementByIndexes( parentIndices );
    }
    thisDistr->AttachMatrix( pMatrix, mType, numberOfWeightMatrix );
}

void CCondGaussianDistribFun::SetCoefficient(float coeff, int isCanonical,
                                             const int* pParentCombination )
{
    PNL_CHECK_IS_NULL_POINTER( pParentCombination );
    CGaussianDistribFun* thisDistr =
        m_distribution->GetElementByIndexes(pParentCombination);
    int isExist = 1;
    if( m_distribution->GetMatrixClass() == mcSparse )
    {
        isExist = static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
            m_distribution)->IsExistingElement(pParentCombination);
    }
    else
    {
        if( !thisDistr )
        {
            isExist = 0;
        }
    }
    if( !isExist )
    {
        AllocDistribFun( pParentCombination );
        thisDistr = m_distribution->GetElementByIndexes( pParentCombination );
    }
    thisDistr->SetCoefficient( coeff, isCanonical );
}

float CCondGaussianDistribFun::GetCoefficient( int isCanonical,
                                              const int* pParentCombination )
{
    PNL_CHECK_IS_NULL_POINTER( pParentCombination );
    CGaussianDistribFun* pDistr =
        m_distribution->GetElementByIndexes(pParentCombination);
    float val = FLT_MAX;
    if( !pDistr )
    {
        PNL_THROW( CInvalidOperation,
            "can't set coefficient to non allocated Gaussian distrbution" );
    }
    else
    {
        val = pDistr->GetCoefficient( isCanonical );
    }
    return val;
}

CMatrix<float>*
CCondGaussianDistribFun::GetMatrix(EMatrixType mType, int numWeightMat,
                                   const int *parentIndices) const
{
    PNL_CHECK_IS_NULL_POINTER( parentIndices );
    if(!(( mType == matMean )||(mType == matCovariance)||( mType == matH )||
        (mType == matK)||( mType == matWeights) ))
    {
        PNL_THROW( CInconsistentType,
            "Conditional Gaussian haven't Tabular matrix" );
    }
    
    CGaussianDistribFun* thisDistr = m_distribution->GetElementByIndexes(
        parentIndices );
    if( thisDistr )
    {
        return thisDistr->GetMatrix( mType, numWeightMat );
    }
    else
    {
        PNL_THROW( CInvalidOperation,
            "distribution function must be sit before getting matrix from it" );
    }
}

CMatrix<float>*
CCondGaussianDistribFun::GetStatisticalMatrix(EStatisticalMatrix mType, int *parentIndices) const
{
    PNL_CHECK_IS_NULL_POINTER( parentIndices );
    
    CGaussianDistribFun* thisDistr = m_distribution->GetElementByIndexes(
        parentIndices );
    if( thisDistr )
    {
        return thisDistr->GetStatisticalMatrix( mType );
    }
    else
    {
        PNL_THROW( CInvalidOperation,
            "distribution function must be sit before getting matrix from it" );
    }
}


void
CCondGaussianDistribFun::MarginalizeData(const CDistribFun *pOldData,
                                         const int *DimsOfKeep, int NumDimsOfKeep,
                                         int maximize )
{
}
/*void CCondGaussianDistribFun::MultiplyData( const int *pBigDomain,
const int *pSmallDomain,
const CDistribFun *pBigData,
const CDistribFun *pSmallData)
{

}*/
void CCondGaussianDistribFun::MultiplyInSelfData( const int *pBigDomain,
                                                 const int *pSmallDomain, const CDistribFun *pOtherData )
{
    PNL_THROW( CNotImplemented, "haven't conditional Gaussian potentials" );
}

void CCondGaussianDistribFun::SumInSelfData(const int *pBigDomain,
  const int *pSmallDomain, const CDistribFun *pOtherData)
{
  PNL_THROW(CNotImplemented, "haven't conditional Gaussian potentials");
}

void CCondGaussianDistribFun::DivideInSelfData( const int *pBigDomain,
                                               const int *pSmallDomain, const CDistribFun *pOtherData )
{
    PNL_THROW( CNotImplemented, "haven't conditional Gaussian potentials" );
}

void
CCondGaussianDistribFun::ShrinkObservedNodes(const CDistribFun* pOldData,
                                             const int *pDimsOfObserved,
                                             const Value* const* pObsValues,
                                             int numObsDim,
                                             const CNodeType* pObsTabNT,
                                             const CNodeType* pObsGauNT)
{
    PNL_THROW( CNotImplemented, "haven't conditional Gaussian potentials" );
}

void
CCondGaussianDistribFun::ExpandData( const int* pDimsToExtend,
                                    int numDimsToExpand,
                                    const Value* const* valuesArray,
                                    const CNodeType* const *allFullNodeTypes,
                                    int UpdateCanonical )
{
    PNL_THROW( CNotImplemented, "haven't conditional Gaussian potentials" );
}

void CCondGaussianDistribFun::ClearStatisticalData()
{
    CMatrixIterator<CGaussianDistribFun*>* iter = m_distribution->InitIterator();
    for( iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter) )
    {
        CGaussianDistribFun* dis = *(m_distribution->Value( iter ));
        dis->ClearStatisticalData();
    }
    delete iter;
    m_numEvidencesLearned->ClearData();
}

/*
void
CCondGaussianDistribFun::UpdateStatisticsEM(const CDistribFun* infData,
                                            const CEvidence *pEvidence,
                                            float weightingCoeff, const int* domain)
{
    //need to add processing tabular inference data,
    //it can be only tabular by now (all gaussian nodes in condGau CPD observed
    if( infData->GetDistributionType() != dtTabular )
    {
        PNL_THROW( CNotImplemented,
            "now we can learn only all gaussian observed at mixed distribution" );
    }
    int i;
    const CTabularDistribFun* tabDistr = static_cast<
        const CTabularDistribFun*>(infData);
    //create domain corresponding the continuous part of domain
    intVector contDomain;
    int contSize = m_contParentsIndex.size();
    contDomain.resize( contSize + 1 );
    for( i = 0; i < contSize; i++ )
    {
        contDomain[i] = domain[m_contParentsIndex[i]];
    }
    contDomain[contSize] = domain[m_NumberOfNodes - 1];
    //values this from matrix are weight coefficients
    CMatrix<float>*tabMat = tabDistr->GetMatrix( matTable );
    int nDimsTabMat;
    const int* tabDims;
    tabMat->GetRanges( &nDimsTabMat, &tabDims );
    if( nDimsTabMat != m_NumberOfNodes )
    {
        PNL_THROW( CInconsistentSize,
            "num dims in matrix must corresponds number of nodes in domain" );
    }
    //check tabular part of matrix - it should be the same
    int discSize  = m_discrParentsIndex.size();
    for( i = 0; i < discSize; i++ )
    {
        int discrPos = m_discrParentsIndex[i];
        if( tabDims[discrPos] != m_NodeTypes[discrPos]->GetNodeSize() )
        {
            if( ! (tabDims[discrPos] == 1 && pEvidence->IsNodeObserved(domain[discrPos]) ) )
	    {
		PNL_THROW( CInconsistentSize,
                "ranges in input distributon must corresponds discrete part of domain" );
	    }
        }
    }
    CMatrixIterator<float>* iter = tabMat->InitIterator();
    intVector index;
    intVector tabIndex;
    tabIndex.resize( discSize );
    for( iter; tabMat->IsValueHere( iter ); tabMat->Next(iter) )
    {
        tabMat->Index( iter, &index );
        float valAdd = *(tabMat->Value( iter ));
        //create index of discrete parents indices
        for( i = 0; i < discSize; i++ )
        {
            tabIndex[i] = index[m_discrParentsIndex[i]];
        }
        //learn corresp Gaussian distribution with weight coefficient
        CGaussianDistribFun* curLearn = m_distribution->GetElementByIndexes(
            &tabIndex.front() );
        curLearn->UpdateStatisticsML( &pEvidence, 1, &contDomain.front(), valAdd );
        //add weight coefficient to general number of evidences learned
        float valLearned = m_numEvidencesLearned->GetElementByIndexes(
            &tabIndex.front() );
        valLearned += valAdd;
        m_numEvidencesLearned->SetElementByIndexes( valLearned, &tabIndex.front() );
    }
    
    delete iter;
}*/

void
CCondGaussianDistribFun::UpdateStatisticsEM(const CDistribFun* infData,
                                            const CEvidence *pEvidence,
                                            float weightingCoeff, const int* domain)
{
    //need to add processing tabular inference data,
    //it can be only tabular by now (all gaussian nodes in condGau CPD observed
    if( infData->GetDistributionType() != dtTabular )
    {
        PNL_THROW( CNotImplemented,
            "now we can learn only all gaussian observed at mixed distribution" );
    }
    int i;
    const CTabularDistribFun* tabDistr = static_cast<
        const CTabularDistribFun*>(infData);
    //create domain corresponding the continuous part of domain
    intVector contDomain;
    int contSize = m_contParentsIndex.size();
    contDomain.resize( contSize + 1 );
    for( i = 0; i < contSize; i++ )
    {
        contDomain[i] = domain[m_contParentsIndex[i]];
    }
    contDomain[contSize] = domain[m_NumberOfNodes - 1];
 
    int discSize  = m_discrParentsIndex.size();
    intVector vls(0, 0);
    intVector obsInd(0, 0);
    intVector dnt(discSize, 0);
    for( i = 0; i < discSize; i++ )
    {
        
	if( pEvidence->IsNodeObserved(domain[m_discrParentsIndex[i]])  )
	{
	    vls.push_back(pEvidence->GetValue(domain[m_discrParentsIndex[i]])->GetInt());
	    obsInd.push_back(m_discrParentsIndex[i]);
	}
	dnt[i] = m_NodeTypes[m_discrParentsIndex[i]]->GetNodeSize();
    }
    

    
    CMatrix<float> *tabMat = NULL;
    if( obsInd.size() )
    {
        tabMat = tabDistr->GetMatrix(matTable)->ExpandDims( &obsInd.front(), 
	&vls.front(), &dnt.front(), obsInd.size());
    }
    else
    {
	tabMat = tabDistr->GetMatrix(matTable);
    }
    
    CMatrixIterator<float>* iter = tabMat->InitIterator();
    intVector index;
    intVector tabIndex;
    tabIndex.resize( discSize );
    for( iter; tabMat->IsValueHere( iter ); tabMat->Next(iter) )
    {
        tabMat->Index( iter, &index );
        float valAdd = *(tabMat->Value( iter ));
        //create index of discrete parents indices
        for( i = 0; i < discSize; i++ )
        {
            tabIndex[i] = index[m_discrParentsIndex[i]];
        }
        //learn corresp Gaussian distribution with weight coefficient
        CGaussianDistribFun* curLearn = m_distribution->GetElementByIndexes(
            &tabIndex.front() );
        curLearn->UpdateStatisticsML( &pEvidence, 1, &contDomain.front(), valAdd );
        //add weight coefficient to general number of evidences learned
        float valLearned = m_numEvidencesLearned->GetElementByIndexes(
            &tabIndex.front() );
        valLearned += valAdd;
        m_numEvidencesLearned->SetElementByIndexes( valLearned, &tabIndex.front() );
    }
    
    delete iter;
    if( obsInd.size() )
    {
	delete tabMat;
    }
}

void CCondGaussianDistribFun::SetStatistics( const CMatrix<float> *pMat, 
                                            EStatisticalMatrix matrix, const int* parentsComb )
{
    PNL_CHECK_IS_NULL_POINTER(pMat);
    
    switch(matrix) 
    {
    case stMatMu:
        
    case stMatSigma:
        { 
            CGaussianDistribFun* distr = m_distribution->GetElementByIndexes( parentsComb );
            distr->SetStatistics(pMat, matrix);
        }
        break;
    case stMatCoeff:
        {
            
            if( !(pMat->GetMatrixClass() == mcNumericDense ||  pMat->GetMatrixClass() == mc2DNumericDense) )
            {
                PNL_THROW(CInconsistentType, "inconsistent matrix type")
            }
            const pConstNodeTypeVector * pNodeTypes= GetNodeTypesVector();
            
            int numDims;
            const int* ranges;
            pMat->GetRanges( &numDims, &ranges );
            
            int numDiscrParents = m_discrParentsIndex.size();
            
            PNL_CHECK_FOR_NON_ZERO( numDims - numDiscrParents );
            
            intVector discrParentsSizes;
            discrParentsSizes.assign( numDiscrParents, 0 );
            int i;
            for( i = 0; i < numDiscrParents; i++ )
            {
                int parentSize = (*pNodeTypes)[m_discrParentsIndex[i]]->GetNodeSize();
                
                PNL_CHECK_FOR_NON_ZERO( parentSize - ranges[i] );
            }
            if( !m_numEvidencesLearned )
            {
                m_numEvidencesLearned = static_cast<CDenseMatrix<float>*>(pMat->Clone());
            }
            else
            {
                m_numEvidencesLearned->SetDataFromOtherMatrix(pMat);
            }
            
        }
        break;
    default:
        PNL_THROW(CInconsistentType, "inconsistent matrix type");
    }
}


void
CCondGaussianDistribFun::UpdateStatisticsML(const CEvidence* const* pEvidences,
                                            int EvidenceNumber, const int *domain,
                                            float weightingCoeff )
{
    //we just separate Evidences into parts corresponding every parent combination
    //and call learning for them only for GaussianPart of domain!
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pEvidences );
    PNL_CHECK_LEFT_BORDER( EvidenceNumber, 0 );
    int i;
    for( i = 0; i <EvidenceNumber; i++ )
    {
        PNL_CHECK_IS_NULL_POINTER( pEvidences[i] );
    }
    
    //fill discrete & continuos parts of domain
    intVector discrDomain;
    int discrSize = m_discrParentsIndex.size();
    discrDomain.resize( discrSize );
    for( i = 0; i < discrSize; i++ )
    {
        discrDomain[i] = domain[m_discrParentsIndex[i]];
    }
    intVector contDomain;
    int contSize = m_contParentsIndex.size();
    contDomain.resize( contSize + 1 );
    for( i = 0; i < contSize; i++ )
    {
        contDomain[i] = domain[m_contParentsIndex[i]];
    }
    contDomain[contSize] = domain[m_NumberOfNodes - 1];
    //collect Evidence corresponding every of discrete parent combination
    
    int numRanges;
    const int *ranges;
    m_distribution->GetRanges( &numRanges, &ranges );
    int lineSize = 1;
    for( i = 0; i < numRanges; i++ )
    {
        lineSize *= ranges[i];
    }
    
    //separate evidences for every discrete parent combination
    pConstEvidencesVecVector correspEvid;
    correspEvid.assign( lineSize, pConstEvidenceVector() );
    
    //discrete parents combination for current evidence
    intVector currentComb;
    currentComb.resize( numRanges );
    
    //look through all evidence lines & find which of discrete combinations is here
    for( i = 0; i < EvidenceNumber; i++ )
    {
        for( int j = 0; j < numRanges; j++ )
        {
            int val = pEvidences[i]->GetValue(discrDomain[j])->GetInt();
            currentComb[j] = val;
        }
        int offset = 0;
        for( int i1 = 0; i1 < numRanges; i1++ )
        {
            offset = offset*ranges[i1] + currentComb[i1];
        }
        correspEvid[offset].push_back( pEvidences[i] );
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
    for( i = 0; i < lineSize; i++ )
    {
        int numCorrespEvid = correspEvid[i].size();
        if( numCorrespEvid )
        {
            //convert from line number to index
            int hres = i;
            for( int k = 0; k < numRanges; k++ )
            {
                int pInd = pConvInd[k];
                result = div( hres, pInd );
                index[k] = result.quot;
                hres = result.rem;
            }
            CGaussianDistribFun* currentLearn =
                m_distribution->GetElementByIndexes(&index.front());
            currentLearn->UpdateStatisticsML( &correspEvid[i].front(), numCorrespEvid,
                &contDomain.front());
            float curNum = m_numEvidencesLearned->GetElementByOffset(i);
            curNum += numCorrespEvid;
            m_numEvidencesLearned->SetElementByOffset( curNum, i);
        }
    }
}
/*
//create auxillary array to convert from line index to multidimensional
intVector convertIndex;
convertIndex.resize( numRanges );
int line = 1;
for( i = numRanges - 1; i >= 0; i-- )
{
convertIndex[i] = line;
line = line * ranges[i];
}

  div_t res;

    //create vector to store current parent combination
    intVector parentComb;
    parentComb.resize( numRanges );
    //collect corresponding Evidences
    pConstEvidenceVector corrEvid;

      for( int lineNumber = 0; lineNumber < lineSize; lineNumber++ )
      {
      //need to convert line index to multidimentional & find such evidence
      int tempLine = lineNumber;
      for( i = 0; i < numRanges; i++ )
      {
      res = div( tempLine, convertIndex[i] );
      parentComb[i] = res.quot;
      tempLine = res.rem;
      }
      //collect Evidence with such discrete observed part
      for( i = 0; i < EvidenceNumber; i++ )
      {
      int isSuch = 1;
      for( int j = 0; j < numRanges; j++ )
      {
      int val = pEvidences[i]->GetValue(discrDomain[j])->GetInt();
      if( val != parentComb[i] )
      {
      isSuch = 0;
      break;
      }
      }
      if( isSuch )
      {
      corrEvid.push_back( pEvidences[i] );
      }
      }
      int corrEvidSize = corrEvid.size();
      if( corrEvidSize > 0 )
      {
      CGaussianDistribFun* currentLearn =
      m_distribution->GetElementByOffset(lineNumber);
      currentLearn->UpdateStatisticsML( &corrEvid.front(), corrEvid.size(),
      &contDomain.front());
      int numLearned = m_numEvidencesLearned->GetElementByOffset(lineNumber);
      numLearned += corrEvidSize;
      m_numEvidencesLearned->SetElementByOffset( numLearned, lineNumber );
      }
      corrEvid.clear();
      }
      */
                                                  
float CCondGaussianDistribFun::ProcessingStatisticalData( float numEvidences )
{
  float numEvid = 0;
  CMatrixIterator<float>* iter = m_numEvidencesLearned->InitIterator();
  for( iter; m_numEvidencesLearned->IsValueHere( iter );
  m_numEvidencesLearned->Next(iter) )
  {
      float num = *(m_numEvidencesLearned->Value( iter ));
      numEvid += num;
  }
  delete iter;
  float eps = 1e-3f;
  float sum = numEvid + numEvidences;
  if( (fabs((numEvid - numEvidences)/sum)) > eps )
  {
      PNL_THROW( CInconsistentSize,
          "numEvidences learned should correspond numEvidences input" );
  }
  float loglik = 0.0f;
  CMatrixIterator<CGaussianDistribFun*>* iterG = m_distribution->InitIterator();
  for( iterG; m_distribution->IsValueHere( iterG );
  m_distribution->Next(iterG) )
  {
      CGaussianDistribFun* data = *(m_distribution->Value( iterG ));
      intVector index;
      m_distribution->Index(iterG, &index);
	  //we can do following trick - 
	  //if the number of evidences match current parent combination is equal to 0
	  //we make the distribution unifrom or left the distribution unchanged
	  float numEvid =  m_numEvidencesLearned->GetElementByIndexes(&index.front());
	  if(numEvid > 0)
          {
              loglik += data->ProcessingStatisticalData(numEvid);
	  }
	  else
	  {
            //we need to make the data uniform (or left unchanged) and doesn't change loglik
	  }
  }
  delete iterG;
  return loglik;
}

int
  CCondGaussianDistribFun::GetMultipliedDelta(const int **positions,
  const float **values,
  const int **offsets ) const
{
  return 0;
}

CDistribFun *CCondGaussianDistribFun::ConvertCPDDistribFunToPot()const
{
  return NULL;
}

CDistribFun*
  CCondGaussianDistribFun::CPD_to_pi( CDistribFun *const* allPiMessages,
  int *multParentIndices, int numMultNodes,
  int posOfExceptParent, int maximizeFlag ) const
{
  return NULL;
}

CDistribFun *CCondGaussianDistribFun::CPD_to_lambda( const CDistribFun *lambda,
  CDistribFun *const* allPiMessages, int *multParentIndices,
  int numMultNodes, int posOfExceptNode, int maximizeFlag )const
{
  return NULL;
}

CDistribFun* CCondGaussianDistribFun::GetNormalized()const
{
  CCondGaussianDistribFun* retDisr = static_cast<CCondGaussianDistribFun*>(
      CCondGaussianDistribFun::Copy(this));
  CMatrixIterator<CGaussianDistribFun*>* iter = retDisr->m_distribution->InitIterator();
  for( iter; retDisr->m_distribution->IsValueHere(iter); retDisr->m_distribution->Next(iter) )
  {
      CGaussianDistribFun* data = *(retDisr->m_distribution->Value( iter ));
      data->Normalize();
  }
  delete iter;
  return retDisr;
}

void CCondGaussianDistribFun::Normalize()
{
  CMatrixIterator<CGaussianDistribFun*>* iter = m_distribution->InitIterator();
  for( iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter) )
  {
      CGaussianDistribFun* data = *(m_distribution->Value( iter ));
      data->Normalize();
  }
  delete iter;
}

int CCondGaussianDistribFun::IsEqual( const CDistribFun *dataToCompare,
  float epsilon, int withCoeff,
  float* maxDifference ) const
{
  PNL_CHECK_IS_NULL_POINTER( dataToCompare );
  PNL_CHECK_LEFT_BORDER( epsilon, 0.0f );
  
  if( dataToCompare->GetDistributionType() != dtCondGaussian )
  {
      PNL_THROW( CInconsistentType, "data to compare must be ConditionalGaussian" )
  }
  if( maxDifference )
  {
      *maxDifference = 0.0f;
  }
  const CCondGaussianDistribFun* data =
      static_cast<const CCondGaussianDistribFun*>(dataToCompare);
  if( (!IsValid())||( !data->IsValid() ) )
  {
      PNL_THROW( CInvalidOperation, "we can't compare invalid data" )
  }
  //we need to check node types
  int ret = 1;
  if( m_numberOfDims != data->m_numberOfDims )
  {
      return 0;
  }
  if( m_NumberOfNodes != data->m_NumberOfNodes )
  {
      return 0;
  }
  int class1 = m_distribution->GetMatrixClass();
  int class2 = m_distribution->GetMatrixClass();
  if( class1 != class2 )
  {
      ret = 0;
  }
  if( class1 == mcSparse )
  {
      CGaussianDistribFun* def = static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
          m_distribution)->GetDefaultValue();
      CGaussianDistribFun* defCompare = static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
          data->m_distribution)->GetDefaultValue();
      if( !def->IsEqual(defCompare, epsilon, withCoeff, maxDifference) )
      {
          ret = 0;
      }
  }
  int numNodes = 0;
  CMatrixIterator<CGaussianDistribFun*>* iterThis =
      m_distribution->InitIterator();
  for( iterThis; m_distribution->IsValueHere( iterThis );
            m_distribution->Next(iterThis))
  {
      numNodes++;
      intVector index1;
      m_distribution->Index( iterThis, &index1 );
      CGaussianDistribFun* val = *(m_distribution->Value( iterThis ));
      //now we can find corresponding element in compare matrix
      CGaussianDistribFun* compVal = data->m_distribution->
          GetElementByIndexes(&index1.front());
      if( !val->IsEqual( compVal, epsilon, withCoeff, maxDifference ) )
      {
          ret = 0;
          break;
      }
  }
  delete iterThis;
  if( ret )
  {
      //we need to check it only in case of no breaks in previous loop
      int numOtherNodes = 0;
      CMatrixIterator<CGaussianDistribFun*>* iterOther =
          data->m_distribution->InitIterator();
      for( iterOther; data->m_distribution->IsValueHere( iterOther );
            data->m_distribution->Next(iterOther))
      {
          numOtherNodes++;
      }
      delete iterOther;
      if( numOtherNodes != numNodes )
      {
          ret = 0;
      }
  }
  return ret;
}

CNodeValues* CCondGaussianDistribFun::GetMPE()
{
  if( !m_bPotential )
  {
      PNL_THROW( CInvalidOperation, "can't get MPE for Conditional distribution" );
  }
  else
  {
      PNL_THROW( CNotImplemented, "MPE for mixture of Gaussians" )
  }

  return NULL;
}

CDistribFun* CCondGaussianDistribFun::ConvertToSparse() const
{
  const CCondGaussianDistribFun* self = this;
  CCondGaussianDistribFun* resDistrib = Copy( self );
  return resDistrib;
}

CDistribFun* CCondGaussianDistribFun::ConvertToDense() const
{
  const CCondGaussianDistribFun* self = this;
  CCondGaussianDistribFun* resDistrib = Copy( self );
  return resDistrib;
}

void CCondGaussianDistribFun::Dump() const
{
  Log dump("", eLOG_RESULT, eLOGSRV_PNL_POTENTIAL);
  
  int i;
  dump<<"I'm a Conditional Gaussian distribution function of "<< m_NumberOfNodes <<" nodes.\nI have both discrete and continuous parents.\n";
  dump<<"The distributions for every discrete combination of parents are: \n";
  int numDiscrPs = m_discrParentsIndex.size();
  CMatrixIterator<CGaussianDistribFun*>* iterThis =
      m_distribution->InitIterator();
  for( iterThis; m_distribution->IsValueHere( iterThis );
  m_distribution->Next(iterThis))
  {
      intVector index1;
      m_distribution->Index( iterThis, &index1 );
      dump<<"Discrete parents combination is:";
      for( i = 0; i < numDiscrPs; i++ )
      {
          dump<<index1[i]<<" ";
      }
      dump<<"\n";
      CGaussianDistribFun* distr = *(m_distribution->Value( iterThis ));
      distr->Dump();
  }
  delete iterThis;
  if( m_distribution->GetMatrixClass() == mcSparse )
  {
	  dump<<"The default distribution is:\n";
	  static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
		  m_distribution)->GetDefaultValue()->Dump();
  }
}

int CCondGaussianDistribFun::IsSparse() const
{
  if( m_distribution->GetMatrixClass() == mcSparse )
  {
      return 1;
  }
  else
  {
      return 0;
  }
}
int CCondGaussianDistribFun::IsDense() const
{
  if( m_distribution->GetMatrixClass() != mcSparse)
  {
      return 1;
  }
  else
  {
      return 0;
  }
}

CCondGaussianDistribFun* CCondGaussianDistribFun::EnterDiscreteEvidence(
  int nDiscrObsNodes, const int* discrObsNodes,
  const int *pDiscrValues, const CNodeType* pObsTabNodeType ) const
{
  PNL_CHECK_RANGES( nDiscrObsNodes, 0, m_discrParentsIndex.size() );
  if( nDiscrObsNodes > 0 )
  {
      PNL_CHECK_IS_NULL_POINTER( discrObsNodes );
      PNL_CHECK_IS_NULL_POINTER( pDiscrValues );
      PNL_CHECK_IS_NULL_POINTER( pObsTabNodeType );
      
      int i;
      if( !pObsTabNodeType->IsDiscrete() ||
          pObsTabNodeType->GetNodeSize() != 1 )
      {
          PNL_THROW( CInconsistentType, "observed node type must be tabular" )
      }
      //find the position of discrete observed nodes in discrete part of domain
      int loc = 0;
      int discrSize = m_discrParentsIndex.size();
      intVector discrIndices;
      discrIndices.resize( nDiscrObsNodes );
      for( i = 0; i < nDiscrObsNodes; i++ )
      {
          loc = std::find( m_discrParentsIndex.begin(),
              m_discrParentsIndex.end(), discrObsNodes[i] )
              - m_discrParentsIndex.begin();
          if( loc < discrSize )
          {
              discrIndices[i] = loc;
          }
          else
          {
              PNL_THROW( CInconsistentSize, "discrete indices" );
          }
      }
      CMatrix<CGaussianDistribFun*>* pSmallDistrib =
          static_cast<CMatrix<CGaussianDistribFun*>*>(
          m_distribution->ReduceOp( &discrIndices.front(),
          nDiscrObsNodes, 2, pDiscrValues ));
      pConstNodeTypeVector newNodeTypes;
      newNodeTypes.assign( m_NodeTypes.begin(), m_NodeTypes.end() );
      for( i = 0; i < nDiscrObsNodes; i++ )
      {
          newNodeTypes[m_discrParentsIndex[discrIndices[i]]] = pObsTabNodeType;
      }
      CCondGaussianDistribFun* res = new CCondGaussianDistribFun(
          m_bPotential, m_NumberOfNodes, &newNodeTypes.front(),
          pSmallDistrib );
      delete pSmallDistrib;
      return res;
  }
  else
  {
      return CCondGaussianDistribFun::Copy( this );
  }
}

CTabularDistribFun* CCondGaussianDistribFun::EnterFullContinuousEvidence(
									 int nContObsParents,
									 const int* contObsParentsIndices,
									 const C2DNumericDenseMatrix<float>* obsChildValue,
									 C2DNumericDenseMatrix<float>*const * obsValues,
									 const CNodeType* pObsGauNodeType ) const
{
    int numContParents = m_contParentsIndex.size();
    PNL_CHECK_RANGES( nContObsParents, 0, numContParents);
    if( nContObsParents)
    {
	PNL_CHECK_IS_NULL_POINTER( contObsParentsIndices );
	PNL_CHECK_IS_NULL_POINTER( obsValues );
    }
    PNL_CHECK_IS_NULL_POINTER( obsChildValue );
    PNL_CHECK_IS_NULL_POINTER( pObsGauNodeType );
    
    if( pObsGauNodeType->IsDiscrete() || pObsGauNodeType->GetNodeSize() )
    {
	PNL_THROW( CInconsistentType, "observed node type must be Gaussian" );
    }
    if( nContObsParents != numContParents  )
    {
	PNL_THROW( CInvalidOperation, "all continuous nodes must be observed" );
    }
    //create mapping between observed nodes and indices
    //in corresponding gaussian distribution
    
    intVector indicesInGaussian;
    indicesInGaussian.resize( nContObsParents );
    int i;
    int location;
    for( i = 0; i < nContObsParents; i++ )
    {
	location = std::find( contObsParentsIndices,
	    contObsParentsIndices+nContObsParents, m_contParentsIndex[i] )
	    - contObsParentsIndices;
	if( location < numContParents )
	{
	    indicesInGaussian[i] = location;
	}
	else
	{
	    PNL_THROW( CInvalidOperation,
		"continuous parents should be from this domain" );
	}
    }
    //need to put all computed probability into result vector
    double sum = 0.0f;
    doubleVector probability;
    int DiscrLength = 0;
    int asLog = 1;
    //we can compute probability in two stages - first compute logariphm
    //after that compute normalize constant and recompute real probabilities
    if( m_distribution->GetMatrixClass() == mcSparse )
    {
	int dims;
	const int* ranges;
	
	m_distribution->GetRanges( &dims, &ranges );
	for(i = 0; i < dims; i++ )
	{
	    DiscrLength *= ranges[i];
	}
	//need to compute default probability
	double smallestProb = FLT_MAX;
	CGaussianDistribFun* def = (static_cast<
	    CSparseMatrix<CGaussianDistribFun*>*>(m_distribution))->GetDefaultValue();
	double defProb = def->ComputeProbability( obsChildValue, 0,
	    nContObsParents, &indicesInGaussian.front(), obsValues );
	if((fabs(defProb)<smallestProb))
	{
	    smallestProb = fabs(defProb);
	}
	doubleVector resVect;
	resVect.assign( DiscrLength, defProb );
	CNumericDenseMatrix<double>* probMat = CNumericDenseMatrix<double>::
	    Create( dims, ranges, &resVect.front() );
	intVector index;
	index.assign(dims, 0);
	CMatrixIterator<CGaussianDistribFun*>* iter =
	    m_distribution->InitIterator();
	double someProb;
	for( iter; m_distribution->IsValueHere( iter ); m_distribution->Next(iter) )
	{
	    CGaussianDistribFun* current = *(m_distribution->Value(iter));
	    m_distribution->Index( iter, &index );
	    someProb = current->ComputeProbability( obsChildValue, asLog,
		nContObsParents, &indicesInGaussian.front(), obsValues );
	    probMat->SetElementByIndexes( someProb, &index.front() );
	    if((fabs(someProb) < smallestProb))
	    {
		smallestProb = fabs(someProb);
	    }
	}
	delete iter;
	const pnlVector<double>* vect = probMat->GetVector();
	int numProbs = vect->size();
	probability.resize(numProbs);
	for( i = 0; i < numProbs; i++ )
	{
	    probability[i] = exp( (*vect)[i]+smallestProb );
	    sum += probability[i];
	}
	//need to normalize data
	floatVector probs;
	probs.assign(probability.size(), 0.0f);
	if( sum )
	{
	    for( i = 0; i < DiscrLength; i++ )
	    {
		probs[i] = (float)(probability[i]/sum);
	    }
	}
	//create new node types
	pConstNodeTypeVector nTypes;
	nTypes.assign( m_NodeTypes.begin(), m_NodeTypes.end() );
	for( i = 0; i < nContObsParents; i++ )
	{
	    nTypes[contObsParentsIndices[i]] = pObsGauNodeType;
	}
	//make child node observed
	nTypes[m_NumberOfNodes - 1] = pObsGauNodeType;
	intVector nodeSizes;
	CTabularDistribFun* resData = CTabularDistribFun::Create( m_NumberOfNodes,
	    &nTypes.front(), &probs.front() );
	return resData;
    }
    else
    {
	CGaussianDistribFun* const* data;
	CDenseMatrix<CGaussianDistribFun*>* matDistr = static_cast<
	    CDenseMatrix<CGaussianDistribFun*>*>(m_distribution);
	matDistr->GetRawData( &DiscrLength, &data );
	doubleVector expOrders;
	expOrders.resize(DiscrLength);
	
	for( i = 0; i < DiscrLength; i++ )
	{
	    expOrders[i] = data[i]->ComputeProbability( obsChildValue, asLog,
		nContObsParents, &indicesInGaussian.front(), obsValues );
	}
	floatVector probs;
	GetNormExp(expOrders, &probs);
	
	//create new node types
	pConstNodeTypeVector nTypes;
	nTypes.assign( m_NodeTypes.begin(), m_NodeTypes.end() );
	for( i = 0; i < nContObsParents; i++ )
	{
	    nTypes[contObsParentsIndices[i]] = pObsGauNodeType;
	}
	//make child node observed
	nTypes[m_NumberOfNodes - 1] = pObsGauNodeType;
	intVector nodeSizes;
	CTabularDistribFun* resData = CTabularDistribFun::Create( m_NumberOfNodes,
	    &nTypes.front(), &probs.front() );
	return resData;
    }
  }
  
CCondGaussianDistribFun::~CCondGaussianDistribFun()
{
	if( m_distribution->GetNumOfReferences() == 1 )
	{
		CMatrixIterator<CGaussianDistribFun*>* iter =
		  m_distribution->InitIterator();
		for( iter; m_distribution->IsValueHere( iter ); m_distribution->Next(iter) )
		{
			 CGaussianDistribFun* current = *(m_distribution->Value(iter));
			 delete current;
		}
		delete iter;
		if( m_distribution->GetMatrixClass() == mcSparse )
		{
			CGaussianDistribFun* def = static_cast<CSparseMatrix<
				CGaussianDistribFun*>*>(m_distribution)->GetDefaultValue();
			delete def;
		}	
	}
	void* pObj = this;
	m_distribution->Release( pObj );
	delete m_numEvidencesLearned;
}


CCondGaussianDistribFun::CCondGaussianDistribFun(
                                                 const CCondGaussianDistribFun& inpDistr )
                                                 :CDistribFun( dtCondGaussian )
{
    m_bUnitFunctionDistribution = inpDistr.m_bUnitFunctionDistribution;
    m_NodeTypes = pConstNodeTypeVector(inpDistr.m_NodeTypes.begin(),
        inpDistr.m_NodeTypes.end());// may be optimized
    m_NumberOfNodes = inpDistr.m_NumberOfNodes;
    m_bPotential = inpDistr.m_bPotential;
    m_numberOfDims = inpDistr.m_numberOfDims;
    m_contParentsIndex.assign( inpDistr.m_contParentsIndex.begin(),
        inpDistr.m_contParentsIndex.end() );
    m_discrParentsIndex.assign( inpDistr.m_discrParentsIndex.begin(),
        inpDistr.m_discrParentsIndex.end() );
    //need to copy conditional gaussian via copying every Gaussain
    //not only via copying matrices !!!
    m_distribution = inpDistr.m_distribution->Clone() ;
    CMatrixIterator<CGaussianDistribFun*>* iter =
        inpDistr.m_distribution->InitIterator();
    for( iter; inpDistr.m_distribution->IsValueHere( iter );
    inpDistr.m_distribution->Next(iter) )
    {
        CGaussianDistribFun* tempDistr = *(inpDistr.m_distribution->Value( iter ));
        intVector index;
        inpDistr.m_distribution->Index( iter, &index );
        CGaussianDistribFun* newDistr = CGaussianDistribFun::Copy( tempDistr );
        m_distribution->SetElementByIndexes( newDistr, &index.front() );
    }
    delete iter;
    if( m_distribution->GetMatrixClass() == mcSparse )
    {
        CGaussianDistribFun* copyDef = 
			CGaussianDistribFun::Copy(static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
            inpDistr.m_distribution)->GetDefaultValue());
		//we can just set default value 
		//because the pointer to it was copied from input disrtribution
        static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
            m_distribution)->SetDefaultVal(copyDef);
    }
    void* pObj = this;
    m_distribution->AddRef(pObj);
    m_numEvidencesLearned = static_cast<CDenseMatrix<float>*>(
        inpDistr.m_numEvidencesLearned->Clone());
}


CCondGaussianDistribFun::CCondGaussianDistribFun( int isFactor,
                                                 int nNodes, const CNodeType* const* nodeTypes,
                                                 int asDenseMatrix,
                                                 CGaussianDistribFun* const pDefaultDistrib )
                                                 :CDistribFun( dtCondGaussian, nNodes, nodeTypes, 0 )
{
    m_numberOfDims = nodeTypes[nNodes - 1]->GetNodeSize();
    m_bPotential = isFactor;
    int i;
    for( i = 0; i < nNodes - 1; i++ )
    {
        if( nodeTypes[i]->IsDiscrete() )
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
    discrParentsSizes.assign( numDiscrParents, 0 );
    int dataSize = 1;
    for( i = 0; i < numDiscrParents; i++ )
    {
        int parentSize = nodeTypes[m_discrParentsIndex[i]]->GetNodeSize();
        discrParentsSizes[i] = parentSize;
        dataSize *= parentSize;
    }
    if( asDenseMatrix )
    {
        pnlVector<CGaussianDistribFun*> data;
        data.assign( dataSize, (CGaussianDistribFun* const)NULL );
        m_distribution = CDenseMatrix<CGaussianDistribFun*>::Create( numDiscrParents,
            &discrParentsSizes.front(), &data.front());
    }
    else
    {
        CGaussianDistribFun* pDefault = CGaussianDistribFun::Copy(pDefaultDistrib);
        m_distribution = CSparseMatrix<CGaussianDistribFun*>::Create(
            numDiscrParents, &discrParentsSizes.front(), pDefault );
    }
    void *pObj = this;
    m_distribution->AddRef(pObj);
    floatVector vectZeros = floatVector( dataSize, 0.0f );
    m_numEvidencesLearned = CDenseMatrix<float>::Create( numDiscrParents,
        &discrParentsSizes.front(), &vectZeros.front() );
}

CCondGaussianDistribFun::CCondGaussianDistribFun( int isFactor, int nNodes,
                                                 const CNodeType* const* pNodeTypes,
                                                 CMatrix<CGaussianDistribFun*> const* pDistribMatrix )
                                                 :CDistribFun( dtCondGaussian, nNodes, pNodeTypes, 0 )
{
    m_numberOfDims = pNodeTypes[nNodes - 1]->GetNodeSize();
    m_bPotential = isFactor;
    int i;
    for( i = 0; i < nNodes - 1; i++ )
    {
        if( pNodeTypes[i]->IsDiscrete() )
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
    discrParentsSizes.assign( numDiscrParents, 0 );
    int dataSize = 1;
    for( i = 0; i < numDiscrParents; i++ )
    {
        int parentSize = pNodeTypes[m_discrParentsIndex[i]]->GetNodeSize();
        discrParentsSizes[i] = parentSize;
        dataSize *= parentSize;
    }
    int numRanges;
    const int* ranges;
    pDistribMatrix->GetRanges( &numRanges, &ranges );
    int areTheSame = 1;
    for( i = 0; i < numDiscrParents; i++ )
    {
        if( ranges[i] != discrParentsSizes[i] )
        {
            areTheSame = 0;
            break;
        }
    }
    if( areTheSame )
    {
        //need to copy distributions for every element
        //of matrix to prevent destruction of data
        if( pDistribMatrix->GetMatrixClass() != mcSparse )
        {
            pnlVector<CGaussianDistribFun*> copyData;
            copyData.resize( dataSize );
            for( i = 0; i < dataSize; i++ )
            {
                copyData[i] = pnl::CGaussianDistribFun::Copy(
                    static_cast<const CDenseMatrix<CGaussianDistribFun*>*>(
                    pDistribMatrix)->GetElementByOffset(i));
            }
            m_distribution = CDenseMatrix<CGaussianDistribFun*>::Create(
                numDiscrParents, &discrParentsSizes.front(), &copyData.front());
        }
        else
        {
            CGaussianDistribFun* pDefault = CGaussianDistribFun::Copy(
                static_cast<const CSparseMatrix<CGaussianDistribFun*>*>(
                pDistribMatrix)->GetDefaultValue());
            m_distribution = pDistribMatrix->CreateEmptyMatrix(
                numDiscrParents, &discrParentsSizes.front(), 0, pDefault);
            //need to set elements one by one
            CMatrixIterator<CGaussianDistribFun*>* iter =
                pDistribMatrix->InitIterator();
            intVector index;
            for( iter; pDistribMatrix->IsValueHere(iter);
            pDistribMatrix->Next(iter) )
            {
                CGaussianDistribFun* data = *(pDistribMatrix->Value( iter ));
                CGaussianDistribFun* cloneData = CGaussianDistribFun::Copy(data);
                pDistribMatrix->Index( iter, &index );
                m_distribution->SetElementByIndexes( cloneData, &index.front());
            }
            delete iter;
        }
        void *pObj = this;
        m_distribution->AddRef(pObj);
        floatVector vectZeros = floatVector( dataSize, 0.0f );
        m_numEvidencesLearned = CDenseMatrix<float>::Create( numDiscrParents,
            &discrParentsSizes.front(), &vectZeros.front() );
    }
    else
    {
        PNL_THROW( CInvalidOperation,
            "distribution matrix should have the same sizes as discrete parents" );
    }
}

int CCondGaussianDistribFun::GetNumberOfFreeParameters() const
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
        CGaussianDistribFun* theDistr = m_distribution->GetElementByIndexes(&index.front());
        nparams += theDistr->GetNumberOfFreeParameters();
    }
	return nparams;
}

void CCondGaussianDistribFun::GetNormExp(doubleVector &expOrders, floatVector *normExpData) const
{
    
    const double maxForExp = 200;
    const double minForExp = -36;
    double maxEl = expOrders[0];
    double minEl = maxEl;
    double sum = 0;

    int nEl = expOrders.size();
    normExpData->resize(nEl);

    int i;
    for( i = 0; i < nEl; i++ )
    {
	
	if( expOrders[i] > maxEl )
	{
	    maxEl = expOrders[i];
	}
	else
	{
	    if( expOrders[i] < minEl )
	    {
		minEl = expOrders[i];
	    }
	}
    }
    if( maxEl > maxForExp )
    {
	double val = maxEl - maxForExp;
	for( i = 0; i < nEl; i++ )
	{
	    double pw = expOrders[i] - val;
	    if( pw > minForExp )
	    {
		expOrders[i] = exp(pw);
	    }
	    else
	    {
		expOrders[i] = 0;
	    }
	    
	    sum += expOrders[i];
	}
    }
    else
    {
	if( minEl < minForExp )
	{
	    
	    double val = minForExp - minEl;
	    if( maxEl + val > maxForExp )
	    {
		val = maxForExp - maxEl;
	    }
	    for( i = 0; i < nEl; i++ )
	    {
		if( expOrders[i] + val < minForExp )
		{
		    expOrders[i] = 0;
		}
		else
		{
		    expOrders[i] = exp(expOrders[i] + val );
		}
		sum += expOrders[i];
	    }
	}
	else
	{
	    
	    for( i = 0; i < nEl; i++ )
	    {
		expOrders[i] = exp(expOrders[i]);
		sum += expOrders[i];
	    }
	}
	
    }
    
    while( sum > DBL_MAX )
    {
	sum = 0;
	for( i = 0; i < nEl; i++ )
	{
	    expOrders[i] /= DBL_MAX/nEl;
	    sum += expOrders[i];
	}
    }
    float s1 = 0.0f;
    for( i = 0; i < nEl; i++ )
    {
	(*normExpData)[i] = (float)(expOrders[i] / sum);
	s1 += (*normExpData)[i];
    }
    
        
}

#ifdef PAR_PNL
void CCondGaussianDistribFun::UpdateStatisticsML(CDistribFun *pPot)
{
    PNL_THROW(CNotImplemented, 
        "UpdateStatisticsML for CCondGaussianDistribFun not implemented yet");
};
#endif // PAR_OMP

void CCondGaussianDistribFun::ResetNodeTypes(pConstNodeTypeVector &nodeTypes)
{
        CDistribFun::ResetNodeTypes(nodeTypes);
	
	int ncontPar = m_contParentsIndex.size();
	pConstNodeTypeVector ntCont(ncontPar + 1, (const CNodeType*)0);
	int i;
	for( i = 0 ; i < ncontPar; i++ )
	{
	    ntCont[i] = nodeTypes[m_contParentsIndex[i]];
	}
	ntCont.back() = nodeTypes.back();
	CMatrix<CGaussianDistribFun*>* distribf = static_cast<CCondGaussianDistribFun*>(this)
	    ->GetMatrixWithDistribution();
	CMatrixIterator<CGaussianDistribFun*>* iter = distribf->InitIterator();
	for( iter; distribf->IsValueHere( iter ); distribf->Next(iter) )
	{
	    CGaussianDistribFun* tempDistr = *(distribf->Value( iter ));
	    tempDistr->ResetNodeTypes( ntCont );
	}
	delete iter;
}

#ifdef PNL_RTTI
const CPNLType CCondGaussianDistribFun::m_TypeInfo = CPNLType("CCondGaussianDistribFun", &(CDistribFun::m_TypeInfo));

#endif
