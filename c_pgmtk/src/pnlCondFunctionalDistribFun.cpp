/**
 * @file pnlCondFunctionalDistribFun.cpp
 * @brief Implementation of the CCondFunctionalDistribFun class.
 * 
 * This header file implements the CCondFunctionalDistribFun class.
 * 
 * Derived from pnlCondGaussianDistribFun.cpp.
 * 
 * See also: pnlCondFunctionalDistribFun.hpp
 * 
 * TODO: Full conversion
 */

#include <sstream>

// PNL
#include "pnlConfig.hpp"
#include "pnlSparseMatrix.hpp"
#include "pnlDenseMatrix.hpp"
#include "pnlNumericDenseMatrix.hpp"
#include "pnlLog.hpp"

// PNL+
#include "pnlMatrix_impl.hpp"  // since we'll use CMatrix<CFunctionalDistribFun*>
#include "pnlCondFunctionalDistribFun.hpp"

PNL_USING

CCondFunctionalDistribFun* CCondFunctionalDistribFun::Create(int isFactor,
    int nNodes, const CNodeType* const* nodeTypes, int asDenseMatrix,
    CFunctionalDistribFun* const pDefaultDistr)
{
    PNL_CHECK_IS_NULL_POINTER( nodeTypes );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );
    PNL_CHECK_RANGES( isFactor, 0, 1 );
    if ( !asDenseMatrix)
    {
        PNL_CHECK_IS_NULL_POINTER(pDefaultDistr);
        if ( !pDefaultDistr->IsValid() )
        {
            PNL_THROW( CInconsistentType, "default distribution must be valid" );
        }
    }
    
    if (nodeTypes[nNodes - 1]->IsDiscrete() )
    {
        PNL_THROW( CInvalidOperation,
                "conditional Gaussian must have last node continuous" )
    }
    
    CCondFunctionalDistribFun *resDistr = new CCondFunctionalDistribFun( isFactor,
            nNodes, nodeTypes, asDenseMatrix, pDefaultDistr );
    PNL_CHECK_IF_MEMORY_ALLOCATED(resDistr);
    
    return resDistr;
}

CCondFunctionalDistribFun* CCondFunctionalDistribFun::Copy(
    const CCondFunctionalDistribFun* pInputDistr)
{
    PNL_CHECK_IS_NULL_POINTER( pInputDistr );
    
    CCondFunctionalDistribFun* resDistr = new CCondFunctionalDistribFun( *pInputDistr );
    PNL_CHECK_IF_MEMORY_ALLOCATED( resDistr );
    
    return resDistr;
}

//typeOfMatrices == 1 - randomly created matrices
//creates Gaussian distribution in moment form
void CCondFunctionalDistribFun::CreateDefaultMatrices(int typeOfMatrices)
{
    PNL_CHECK_RANGES( typeOfMatrices, 1, 1 );
    //we have only one type of matrices now
    if (m_bUnitFunctionDistribution)
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
        CFunctionalDistribFun* theDistr =
                m_distribution->GetElementByIndexes(&index.front());
        theDistr
                = CFunctionalDistribFun::CreateInMomentForm(
                                                            m_bPotential,
                                                            numContParents + 1,
                                                             &contParentsTypes.front(), 
                                                            NULL, NULL, NULL);
        theDistr->CreateDefaultMatrices(typeOfMatrices);
        m_distribution->SetElementByIndexes(theDistr, &index.front() );
    }
    
}

CDistribFun& CCondFunctionalDistribFun::operator=(const CDistribFun& pInputDistr)
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
    const CCondFunctionalDistribFun &pGInputDistr =
    static_cast<const CCondFunctionalDistribFun&>(pInputDistr);
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
        CMatrixIterator<CFunctionalDistribFun*>* iterChanging =
        m_distribution->InitIterator();
        for( iterChanging; m_distribution->IsValueHere( iterChanging );
                m_distribution->Next(iterChanging) )
        {
            CFunctionalDistribFun* changingData =
            *(m_distribution->Value( iterChanging ));
            intVector index;
            m_distribution->Index( iterChanging, &index );
            CFunctionalDistribFun* formingData =
            pGInputDistr.m_distribution->GetElementByIndexes(
                    &index.front() );
            *(CDistribFun*)changingData = *(CDistribFun*)formingData;
        }
        delete iterChanging;
        if( m_distribution->GetMatrixClass() == mcSparse )
        {
            //set default distribution
            CFunctionalDistribFun* pDefault =
            static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                    m_distribution)->GetDefaultValue();
            const CFunctionalDistribFun* pFormingDefault =
            static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
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

CDistribFun* CCondFunctionalDistribFun::Clone() const
{
    if ( !IsValid() )
    {
        PNL_THROW( CInconsistentState, "can't clone invalid data" );
    }
    //call copy method
    CCondFunctionalDistribFun* resData = CCondFunctionalDistribFun::Copy( this);
    return resData;
    
}

CDistribFun* CCondFunctionalDistribFun::CloneWithSharedMatrices()
{
    if ( !IsValid() )
    {
        PNL_THROW( CInconsistentState, "can't clone invalid data" );
    }
    CCondFunctionalDistribFun *resData= NULL;
    if (m_distribution->GetMatrixClass() == mcDense)
    {
        resData = CCondFunctionalDistribFun::Create(m_bPotential,
                                                    m_NumberOfNodes,
                                                     &m_NodeTypes.front() );
    } else
    {
        //if( m_distribution->GetMatrixClass() == mcSparse )
        CFunctionalDistribFun* pDefault =
                static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                        m_distribution)->GetDefaultValue();
        resData = CCondFunctionalDistribFun::Create(m_bPotential,
                                                    m_NumberOfNodes,
                                                     &m_NodeTypes.front(), 0,
                                                    pDefault);
    }
    CMatrixIterator<CFunctionalDistribFun*>* iterForming =
            m_distribution->InitIterator();
    for (iterForming; m_distribution->IsValueHere(iterForming); m_distribution->Next(iterForming) )
    {
        CFunctionalDistribFun* formingData =
                *(m_distribution->Value(iterForming));
        CFunctionalDistribFun
                * clonedWithSharedData =
                        static_cast<CFunctionalDistribFun*>(formingData->CloneWithSharedMatrices());
        intVector index;
        m_distribution->Index(iterForming, &index);
        resData->m_distribution->SetElementByIndexes(clonedWithSharedData,
                                                      &index.front() );
    }
    delete iterForming;
    //(resData->m_distribution)->AddRef(resData);
    return resData;
}

void CCondFunctionalDistribFun::AllocDistribFun(
    const int* discreteParentCombination, int isMoment, int isDelta,
    int isUniform)
{
    PNL_CHECK_IS_NULL_POINTER( discreteParentCombination );
    if (isDelta && isUniform)
    {
        PNL_THROW( CInvalidOperation,
                "distribution can't be both Delta & Uniform" );
    }
    PNL_CHECK_RANGES( isMoment, 0, 1 );
    if (m_bPotential && !isMoment)
    {
        PNL_THROW( CInconsistentType, "CPD can be only in moment form" );
    }
    
    //find corresponding node types
    pConstNodeTypeVector contParentsTypes;
    int numContParents = m_contParentsIndex.size();
    contParentsTypes.resize(numContParents + 1);
    for (int i = 0; i < numContParents; i++)
    {
        contParentsTypes[i] = m_NodeTypes[m_contParentsIndex[i]];
    }
    contParentsTypes[numContParents] = m_NodeTypes[m_NumberOfNodes - 1];
    CFunctionalDistribFun* theDistr =
            m_distribution->GetElementByIndexes(discreteParentCombination);
    int isExist = 1;
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        isExist = static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                m_distribution)->IsExistingElement(discreteParentCombination);
    } else
    {
        if ( !theDistr)
        {
            isExist = 0;
        }
    }
    if (isExist)
    {
        delete theDistr;
    }
    //detect which of distributions should be allocated
    if (isDelta)
    {
        theDistr
                = CFunctionalDistribFun::CreateDeltaDistribution(numContParents
                        + 1, &contParentsTypes.front(), NULL, isMoment,
                                                                 m_bPotential);
    } else
    {
        if (isUniform)
        {
            theDistr
                    = CFunctionalDistribFun::CreateUnitFunctionDistribution(
                                                                            numContParents
                                                                                    + 1,
                                                                             &contParentsTypes.front(),
                                                                            m_bPotential,
                                                                             1
                                                                                    -isMoment);
        } else
        {
            if (isMoment)
            {
                theDistr
                        = CFunctionalDistribFun::CreateInMomentForm(
                                                                    m_bPotential,
                                                                    numContParents
                                                                            + 1,
                                                                     &contParentsTypes.front(), 
                                                                    NULL, NULL, 
                                                                    NULL);
            } else
            {
                theDistr
                        = CFunctionalDistribFun::CreateInCanonicalForm(
                                                                       numContParents
                                                                               + 1,
                                                                        &contParentsTypes.front(),
                                                                        0, 0,
                                                                        0.f);
            }
        }
    }
    m_distribution->SetElementByIndexes(theDistr, discreteParentCombination);
}

const CFunctionalDistribFun* CCondFunctionalDistribFun::GetDistribution(
    const int* discrParentIndex) const
{
    PNL_CHECK_IS_NULL_POINTER( discrParentIndex );
    
    const CFunctionalDistribFun* distr =
            m_distribution->GetElementByIndexes(discrParentIndex);
    return distr;
}

void CCondFunctionalDistribFun::SetDistribFun(
    CFunctionalDistribFun* const inputDistr,
    const int *discreteParentCombination)
{
    PNL_CHECK_IS_NULL_POINTER( inputDistr );
    PNL_CHECK_IS_NULL_POINTER( discreteParentCombination );
    
    //check ranges for parents
    int numDims;
    const int* ranges;
    m_distribution->GetRanges( &numDims, &ranges);
    for (int i = 0; i < numDims; i++)
    {
        PNL_CHECK_RANGES( discreteParentCombination[i], 0, ranges[i] - 1 );
    }
    
    //check validity of inputDistr
    if (inputDistr->GetDistributionType() != dtGaussian)
    {
        PNL_THROW( CInconsistentType, "we can set only Gaussian" );
    }
    if (inputDistr->GetFactorFlag() != m_bPotential)
    {
        PNL_THROW( CInconsistentType,
                "we can set only data in the same form as we are");
    }
    if ( !inputDistr->GetMomentFormFlag() )
    {
        PNL_THROW( CInconsistentType, "we can set only data in moment form" );
    }
    int isExist = 1;
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        isExist = static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                m_distribution)->IsExistingElement(discreteParentCombination);
    }
    CFunctionalDistribFun* distr =
            m_distribution->GetElementByIndexes(discreteParentCombination);
    if (isExist)
    {
        delete distr;
    }
    m_distribution->SetElementByIndexes(
                                        CFunctionalDistribFun::Copy(inputDistr),
                                        discreteParentCombination);
}

bool CCondFunctionalDistribFun::IsValid(std::string* description) const
{
    bool ret = 1;
    if (m_discrParentsIndex.size() != size_t(m_distribution->GetNumberDims()))
    {
        if (description)
        {
            (*description)
                    = "Conditional Gaussain distribution have inconsistent number of Gaussian distributions. It's number should be the same as number of different combination of values of discrete parents. ";
        }
        ret = 0;
        return ret;
    }
    CMatrixIterator<CFunctionalDistribFun*>* iter =
            m_distribution->InitIterator();
    for (iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter))
    {
        CFunctionalDistribFun* val = *(m_distribution->Value(iter));
        if ( !val)
        {
            ret = 0;
            if (description)
            {
                intVector index;
                m_distribution->Index(iter, &index);
                int indexSize = index.size();
                int i;
                std::stringstream st;
                st<<"Conditional Gaussian distribution haven't Gaussain ";
                st<<"distribution for this combination of discrete parents: \n";
                for (i = 0; i < indexSize; i++)
                {
                    st<< index[i];
                }
                st<<std::endl;
                std::string s = st.str();
                description->insert(description->begin(), s.begin(), s.end() );
            }
            break;
        } else
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
                    st
                            <<"Conditional Gaussian distribution have invalid Gaussain ";
                    st
                            <<"distribution for this combination of discrete parents: ";
                    for (i = 0; i < indexSize; i++)
                    {
                        st<< index[i];
                    }
                    st<<std::endl;
                    std::string s = st.str();
                    description->insert(description->begin(), s.begin(),
                                        s.end() );
                }
                break;
            }
        }
    }
    delete iter;
    return ret;
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        if ( !static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                m_distribution)->GetDefaultValue()->IsValid() )
        {
            std::stringstream st;
            st
                    <<"Conditional Gaussian distribution have invalid default distribution"
                    <<std::endl;
            std::string s = st.str();
            description->insert(description->begin(), s.begin(), s.end());
            ret = 0;
        }
    }
    return ret;
}

void CCondFunctionalDistribFun::AllocMatrix(const float *data,
    EMatrixType mType, int numberOfWeightMatrix, const int *parentIndices)
{
    PNL_CHECK_IS_NULL_POINTER( data );
    PNL_CHECK_IS_NULL_POINTER( parentIndices );
    
    if (mType == matTable)
    {
        PNL_THROW( CInconsistentType,
                "can't alloc Tabular matrix to Conditional Gaussian" );
    }
    CFunctionalDistribFun* thisDistr =
            m_distribution->GetElementByIndexes(parentIndices);
    int isExist = 1;
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        isExist = static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                m_distribution)->IsExistingElement(parentIndices);
    } else
    {
        if ( !thisDistr)
        {
            isExist = 0;
        }
    }
    if ( !isExist)
    {
        //alloc distribution function for corresponding indices
        AllocDistribFun(parentIndices);
        thisDistr = m_distribution->GetElementByIndexes(parentIndices);
    }
    thisDistr->AllocMatrix(data, mType, numberOfWeightMatrix);
}

void CCondFunctionalDistribFun::AttachMatrix(CMatrix<float> *pMatrix,
    EMatrixType mType, int numberOfWeightMatrix, const int *parentIndices, bool)
{
    PNL_CHECK_IS_NULL_POINTER( pMatrix );
    PNL_CHECK_IS_NULL_POINTER( parentIndices );
    
    if (!((mType == matMean )||(mType == matCovariance)||(mType == matH )
            || (mType == matK)||(mType == matWeights) ||(mType
            == matWishartMean)|| (mType == matWishartCov) ))
    {
        PNL_THROW( CInconsistentType,
                "can't attach Tabular matrix to Conditional Gaussian" );
    }
    CFunctionalDistribFun* thisDistr =
            m_distribution->GetElementByIndexes(parentIndices);
    int isExist = 1;
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        isExist = static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                m_distribution)->IsExistingElement(parentIndices);
    } else
    {
        if ( !thisDistr)
        {
            isExist = 0;
        }
    }
    if ( !isExist)
    {
        AllocDistribFun(parentIndices);
        thisDistr = m_distribution->GetElementByIndexes(parentIndices);
    }
    thisDistr->AttachMatrix(pMatrix, mType, numberOfWeightMatrix);
}

void CCondFunctionalDistribFun::SetCoefficient(float coeff, int isCanonical,
    const int* pParentCombination)
{
    PNL_CHECK_IS_NULL_POINTER( pParentCombination );
    CFunctionalDistribFun* thisDistr =
            m_distribution->GetElementByIndexes(pParentCombination);
    int isExist = 1;
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        isExist = static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                m_distribution)->IsExistingElement(pParentCombination);
    } else
    {
        if ( !thisDistr)
        {
            isExist = 0;
        }
    }
    if ( !isExist)
    {
        AllocDistribFun(pParentCombination);
        thisDistr = m_distribution->GetElementByIndexes(pParentCombination);
    }
    thisDistr->SetCoefficient(coeff, isCanonical);
}

float CCondFunctionalDistribFun::GetCoefficient(int isCanonical,
    const int* pParentCombination)
{
    PNL_CHECK_IS_NULL_POINTER( pParentCombination );
    CFunctionalDistribFun* pDistr =
            m_distribution->GetElementByIndexes(pParentCombination);
    float val= FLT_MAX;
    if ( !pDistr)
    {
        PNL_THROW( CInvalidOperation,
                "can't set coefficient to non allocated Gaussian distrbution" );
    } else
    {
        val = pDistr->GetCoefficient(isCanonical);
    }
    return val;
}

CMatrix<float>* CCondFunctionalDistribFun::GetMatrix(EMatrixType mType,
    int numWeightMat, const int *parentIndices) const
{
    PNL_CHECK_IS_NULL_POINTER( parentIndices );
    if (!((mType == matMean )||(mType == matCovariance)||(mType == matH )
            || (mType == matK)||(mType == matWeights) ))
    {
        PNL_THROW( CInconsistentType,
                "Conditional Gaussian haven't Tabular matrix" );
    }
    
    CFunctionalDistribFun* thisDistr =
            m_distribution->GetElementByIndexes(parentIndices);
    if (thisDistr)
    {
        return thisDistr->GetMatrix(mType, numWeightMat);
    } else
    {
        PNL_THROW( CInvalidOperation,
                "distribution function must be sit before getting matrix from it" );
    }
}

CMatrix<float>* CCondFunctionalDistribFun::GetStatisticalMatrix(
    EStatisticalMatrix mType, int *parentIndices) const
{
    PNL_CHECK_IS_NULL_POINTER( parentIndices );
    
    CFunctionalDistribFun* thisDistr =
            m_distribution->GetElementByIndexes(parentIndices);
    if (thisDistr)
    {
        return thisDistr->GetStatisticalMatrix(mType);
    } else
    {
        PNL_THROW( CInvalidOperation,
                "distribution function must be sit before getting matrix from it" );
    }
}

void CCondFunctionalDistribFun::MarginalizeData(const CDistribFun *pOldData,
    const int *DimsOfKeep, int NumDimsOfKeep, int maximize)
{
}
/*void CCondFunctionalDistribFun::MultiplyData( const int *pBigDomain,
 const int *pSmallDomain,
 const CDistribFun *pBigData,
 const CDistribFun *pSmallData)
 {

 }*/
void CCondFunctionalDistribFun::MultiplyInSelfData(const int *pBigDomain,
    const int *pSmallDomain, const CDistribFun *pOtherData)
{
    PNL_THROW( CNotImplemented, "haven't conditional Gaussian potentials" );
}

void CCondFunctionalDistribFun::SumInSelfData(const int *pBigDomain,
    const int *pSmallDomain, const CDistribFun *pOtherData)
{
    PNL_THROW(CNotImplemented, "haven't conditional Gaussian potentials");
}

void CCondFunctionalDistribFun::DivideInSelfData(const int *pBigDomain,
    const int *pSmallDomain, const CDistribFun *pOtherData)
{
    PNL_THROW( CNotImplemented, "haven't conditional Gaussian potentials" );
}

void CCondFunctionalDistribFun::ShrinkObservedNodes(
    const CDistribFun* pOldData, const int *pDimsOfObserved,
    const Value* const* pObsValues, int numObsDim, const CNodeType* pObsTabNT,
    const CNodeType* pObsGauNT)
{
    PNL_THROW( CNotImplemented, "haven't conditional Gaussian potentials" );
}

void CCondFunctionalDistribFun::ExpandData(const int* pDimsToExtend,
    int numDimsToExpand, const Value* const* valuesArray,
    const CNodeType* const *allFullNodeTypes, int UpdateCanonical)
{
    PNL_THROW( CNotImplemented, "haven't conditional Gaussian potentials" );
}

void CCondFunctionalDistribFun::ClearStatisticalData()
{
    CMatrixIterator<CFunctionalDistribFun*>* iter =
            m_distribution->InitIterator();
    for (iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter) )
    {
        CFunctionalDistribFun* dis = *(m_distribution->Value(iter));
        dis->ClearStatisticalData();
    }
    delete iter;
    m_numEvidencesLearned->ClearData();
}

/*
 void
 CCondFunctionalDistribFun::UpdateStatisticsEM(const CDistribFun* infData,
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
 CFunctionalDistribFun* curLearn = m_distribution->GetElementByIndexes(
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

void CCondFunctionalDistribFun::UpdateStatisticsEM(const CDistribFun* infData,
    const CEvidence *pEvidence, float weightingCoeff, const int* domain)
{
    //need to add processing tabular inference data,
    //it can be only tabular by now (all gaussian nodes in condGau CPD observed
    if (infData->GetDistributionType() != dtTabular)
    {
        PNL_THROW( CNotImplemented,
                "now we can learn only all gaussian observed at mixed distribution" );
    }
    int i;
    const CTabularDistribFun* tabDistr =
            static_cast< const CTabularDistribFun*>(infData);
    //create domain corresponding the continuous part of domain
    intVector contDomain;
    int contSize = m_contParentsIndex.size();
    contDomain.resize(contSize + 1);
    for (i = 0; i < contSize; i++)
    {
        contDomain[i] = domain[m_contParentsIndex[i]];
    }
    contDomain[contSize] = domain[m_NumberOfNodes - 1];
    
    int discSize = m_discrParentsIndex.size();
    intVector vls(0, 0);
    intVector obsInd(0, 0);
    intVector dnt(discSize, 0);
    for (i = 0; i < discSize; i++)
    {
        
        if (pEvidence->IsNodeObserved(domain[m_discrParentsIndex[i]]) )
        {
            vls.push_back(pEvidence->GetValue(domain[m_discrParentsIndex[i]])->GetInt());
            obsInd.push_back(m_discrParentsIndex[i]);
        }
        dnt[i] = m_NodeTypes[m_discrParentsIndex[i]]->GetNodeSize();
    }
    
    CMatrix<float> *tabMat= NULL;
    if (obsInd.size() )
    {
        tabMat = tabDistr->GetMatrix(matTable)->ExpandDims( &obsInd.front(), &vls.front(),
                                       &dnt.front(), obsInd.size());
    } else
    {
        tabMat = tabDistr->GetMatrix(matTable);
    }
    
    CMatrixIterator<float>* iter = tabMat->InitIterator();
    intVector index;
    intVector tabIndex;
    tabIndex.resize(discSize);
    for (iter; tabMat->IsValueHere(iter); tabMat->Next(iter) )
    {
        tabMat->Index(iter, &index);
        float valAdd = *(tabMat->Value(iter));
        //create index of discrete parents indices
        for (i = 0; i < discSize; i++)
        {
            tabIndex[i] = index[m_discrParentsIndex[i]];
        }
        //learn corresp Gaussian distribution with weight coefficient
        CFunctionalDistribFun* curLearn =
                m_distribution->GetElementByIndexes( &tabIndex.front() );
        curLearn->UpdateStatisticsML( &pEvidence, 1, &contDomain.front(),
                                     valAdd);
        //add weight coefficient to general number of evidences learned
        float valLearned =
                m_numEvidencesLearned->GetElementByIndexes( &tabIndex.front() );
        valLearned += valAdd;
        m_numEvidencesLearned->SetElementByIndexes(valLearned,
                                                    &tabIndex.front() );
    }
    
    delete iter;
    if (obsInd.size() )
    {
        delete tabMat;
    }
}

void CCondFunctionalDistribFun::SetStatistics(const CMatrix<float> *pMat,
    EStatisticalMatrix matrix, const int* parentsComb)
{
    PNL_CHECK_IS_NULL_POINTER(pMat);
    
    switch (matrix)
    {
        case stMatMu:

        case stMatSigma:
        {
            CFunctionalDistribFun* distr =
                    m_distribution->GetElementByIndexes(parentsComb);
            distr->SetStatistics(pMat, matrix);
        }
            break;
        case stMatCoeff:
        {
            
            if ( !(pMat->GetMatrixClass() == mcNumericDense
                    || pMat->GetMatrixClass() == mc2DNumericDense))
            {
                PNL_THROW(CInconsistentType, "inconsistent matrix type")
            }
            const pConstNodeTypeVector * pNodeTypes= GetNodeTypesVector();
            
            int numDims;
            const int* ranges;
            pMat->GetRanges( &numDims, &ranges);
            
            int numDiscrParents = m_discrParentsIndex.size();
            
            PNL_CHECK_FOR_NON_ZERO( numDims - numDiscrParents );
            
            intVector discrParentsSizes;
            discrParentsSizes.assign(numDiscrParents, 0);
            int i;
            for (i = 0; i < numDiscrParents; i++)
            {
                int parentSize = (*pNodeTypes)[m_discrParentsIndex[i]]->GetNodeSize();
                
                PNL_CHECK_FOR_NON_ZERO( parentSize - ranges[i] );
            }
            if ( !m_numEvidencesLearned)
            {
                m_numEvidencesLearned
                        = static_cast<CDenseMatrix<float>*>(pMat->Clone());
            } else
            {
                m_numEvidencesLearned->SetDataFromOtherMatrix(pMat);
            }
            
        }
            break;
        default:
            PNL_THROW(CInconsistentType, "inconsistent matrix type")
            ;
    }
}

void CCondFunctionalDistribFun::UpdateStatisticsML(
    const CEvidence* const* pEvidences, int EvidenceNumber, const int *domain,
    float weightingCoeff)
{
    //we just separate Evidences into parts corresponding every parent combination
    //and call learning for them only for GaussianPart of domain!
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pEvidences );
    PNL_CHECK_LEFT_BORDER( EvidenceNumber, 0 );
    int i;
    for (i = 0; i <EvidenceNumber; i++)
    {
        PNL_CHECK_IS_NULL_POINTER( pEvidences[i] );
    }
    
    //fill discrete & continuos parts of domain
    intVector discrDomain;
    int discrSize = m_discrParentsIndex.size();
    discrDomain.resize(discrSize);
    for (i = 0; i < discrSize; i++)
    {
        discrDomain[i] = domain[m_discrParentsIndex[i]];
    }
    intVector contDomain;
    int contSize = m_contParentsIndex.size();
    contDomain.resize(contSize + 1);
    for (i = 0; i < contSize; i++)
    {
        contDomain[i] = domain[m_contParentsIndex[i]];
    }
    contDomain[contSize] = domain[m_NumberOfNodes - 1];
    //collect Evidence corresponding every of discrete parent combination

    int numRanges;
    const int *ranges;
    m_distribution->GetRanges( &numRanges, &ranges);
    int lineSize = 1;
    for (i = 0; i < numRanges; i++)
    {
        lineSize *= ranges[i];
    }
    
    //separate evidences for every discrete parent combination
    pConstEvidencesVecVector correspEvid;
    correspEvid.assign(lineSize, pConstEvidenceVector() );
    
    //discrete parents combination for current evidence
    intVector currentComb;
    currentComb.resize(numRanges);
    
    //look through all evidence lines & find which of discrete combinations is here
    for (i = 0; i < EvidenceNumber; i++)
    {
        for (int j = 0; j < numRanges; j++)
        {
            int val = pEvidences[i]->GetValue(discrDomain[j])->GetInt();
            currentComb[j] = val;
        }
        int offset = 0;
        for (int i1 = 0; i1 < numRanges; i1++)
        {
            offset = offset*ranges[i1] + currentComb[i1];
        }
        correspEvid[offset].push_back(pEvidences[i]);
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
    div_t result;
    intVector index;
    index.resize(numRanges);
    for (i = 0; i < lineSize; i++)
    {
        int numCorrespEvid = correspEvid[i].size();
        if (numCorrespEvid)
        {
            //convert from line number to index
            int hres = i;
            for (int k = 0; k < numRanges; k++)
            {
                int pInd = pConvInd[k];
                result = div(hres, pInd);
                index[k] = result.quot;
                hres = result.rem;
            }
            CFunctionalDistribFun* currentLearn =
                    m_distribution->GetElementByIndexes(&index.front());
            currentLearn->UpdateStatisticsML( &correspEvid[i].front(),
                                             numCorrespEvid,
                                              &contDomain.front());
            float curNum = m_numEvidencesLearned->GetElementByOffset(i);
            curNum += numCorrespEvid;
            m_numEvidencesLearned->SetElementByOffset(curNum, i);
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
 CFunctionalDistribFun* currentLearn =
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

float CCondFunctionalDistribFun::ProcessingStatisticalData(float numEvidences)
{
    float numEvid = 0;
    CMatrixIterator<float>* iter = m_numEvidencesLearned->InitIterator();
    for (iter; m_numEvidencesLearned->IsValueHere(iter); m_numEvidencesLearned->Next(iter) )
    {
        float num = *(m_numEvidencesLearned->Value(iter));
        numEvid += num;
    }
    delete iter;
    float eps = 1e-3f;
    float sum = numEvid + numEvidences;
    if ( (fabs((numEvid - numEvidences)/sum)) > eps)
    {
        PNL_THROW( CInconsistentSize,
                "numEvidences learned should correspond numEvidences input" );
    }
    float loglik = 0.0f;
    CMatrixIterator<CFunctionalDistribFun*>* iterG =
            m_distribution->InitIterator();
    for (iterG; m_distribution->IsValueHere(iterG); m_distribution->Next(iterG) )
    {
        CFunctionalDistribFun* data = *(m_distribution->Value(iterG));
        intVector index;
        m_distribution->Index(iterG, &index);
        //we can do following trick - 
        //if the number of evidences match current parent combination is equal to 0
        //we make the distribution unifrom or left the distribution unchanged
        float numEvid =
                m_numEvidencesLearned->GetElementByIndexes(&index.front());
        if (numEvid > 0)
        {
            loglik += data->ProcessingStatisticalData(numEvid);
        } else
        {
            //we need to make the data uniform (or left unchanged) and doesn't change loglik
        }
    }
    delete iterG;
    return loglik;
}

int CCondFunctionalDistribFun::GetMultipliedDelta(const int **positions,
    const float **values, const int **offsets) const
{
    return 0;
}

CDistribFun *CCondFunctionalDistribFun::ConvertCPDDistribFunToPot() const
{
    return NULL;
}

CDistribFun* CCondFunctionalDistribFun::CPD_to_pi(
    CDistribFun *const* allPiMessages, int *multParentIndices,
    int numMultNodes, int posOfExceptParent, int maximizeFlag) const
{
    return NULL;
}

CDistribFun *CCondFunctionalDistribFun::CPD_to_lambda(
    const CDistribFun *lambda, CDistribFun *const* allPiMessages,
    int *multParentIndices, int numMultNodes, int posOfExceptNode,
    int maximizeFlag) const
{
    return NULL;
}

CDistribFun* CCondFunctionalDistribFun::GetNormalized() const
{
    CCondFunctionalDistribFun
            * retDisr =
                    static_cast<CCondFunctionalDistribFun*>(CCondFunctionalDistribFun::Copy(this));
    CMatrixIterator<CFunctionalDistribFun*>* iter =
            retDisr->m_distribution->InitIterator();
    for (iter; retDisr->m_distribution->IsValueHere(iter); retDisr->m_distribution->Next(iter) )
    {
        CFunctionalDistribFun* data = *(retDisr->m_distribution->Value(iter));
        data->Normalize();
    }
    delete iter;
    return retDisr;
}

void CCondFunctionalDistribFun::Normalize()
{
    CMatrixIterator<CFunctionalDistribFun*>* iter =
            m_distribution->InitIterator();
    for (iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter) )
    {
        CFunctionalDistribFun* data = *(m_distribution->Value(iter));
        data->Normalize();
    }
    delete iter;
}

int CCondFunctionalDistribFun::IsEqual(const CDistribFun *dataToCompare,
    float epsilon, int withCoeff, float* maxDifference) const
{
    PNL_CHECK_IS_NULL_POINTER( dataToCompare );
    PNL_CHECK_LEFT_BORDER( epsilon, 0.0f );
    
    if (dataToCompare->GetDistributionType() != dtCondGaussian)
    {
        PNL_THROW( CInconsistentType, "data to compare must be ConditionalGaussian" )
    }
    if (maxDifference)
    {
        *maxDifference = 0.0f;
    }
    const CCondFunctionalDistribFun* data =
            static_cast<const CCondFunctionalDistribFun*>(dataToCompare);
    if ( (!IsValid())||( !data->IsValid() ))
    {
        PNL_THROW( CInvalidOperation, "we can't compare invalid data" )
    }
    //we need to check node types
    int ret = 1;
    if (m_numberOfDims != data->m_numberOfDims)
    {
        return 0;
    }
    if (m_NumberOfNodes != data->m_NumberOfNodes)
    {
        return 0;
    }
    int class1 = m_distribution->GetMatrixClass();
    int class2 = m_distribution->GetMatrixClass();
    if (class1 != class2)
    {
        ret = 0;
    }
    if (class1 == mcSparse)
    {
        CFunctionalDistribFun* def =
                static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                        m_distribution)->GetDefaultValue();
        CFunctionalDistribFun* defCompare =
                static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                        data->m_distribution)->GetDefaultValue();
        if ( !def->IsEqual(defCompare, epsilon, withCoeff, maxDifference) )
        {
            ret = 0;
        }
    }
    int numNodes = 0;
    CMatrixIterator<CFunctionalDistribFun*>* iterThis =
            m_distribution->InitIterator();
    for (iterThis; m_distribution->IsValueHere(iterThis); m_distribution->Next(iterThis))
    {
        numNodes++;
        intVector index1;
        m_distribution->Index(iterThis, &index1);
        CFunctionalDistribFun* val = *(m_distribution->Value(iterThis));
        //now we can find corresponding element in compare matrix
        CFunctionalDistribFun* compVal = data->m_distribution->
        GetElementByIndexes(&index1.front());
        if ( !val->IsEqual(compVal, epsilon, withCoeff, maxDifference) )
        {
            ret = 0;
            break;
        }
    }
    delete iterThis;
    if (ret)
    {
        //we need to check it only in case of no breaks in previous loop
        int numOtherNodes = 0;
        CMatrixIterator<CFunctionalDistribFun*>* iterOther =
                data->m_distribution->InitIterator();
        for (iterOther; data->m_distribution->IsValueHere(iterOther); data->m_distribution->Next(iterOther))
        {
            numOtherNodes++;
        }
        delete iterOther;
        if (numOtherNodes != numNodes)
        {
            ret = 0;
        }
    }
    return ret;
}

CNodeValues* CCondFunctionalDistribFun::GetMPE()
{
    if ( !m_bPotential)
    {
        PNL_THROW( CInvalidOperation, "can't get MPE for Conditional distribution" );
    } else
    {
        PNL_THROW( CNotImplemented, "MPE for mixture of Gaussians" )
    }
    
    return NULL;
}

CDistribFun* CCondFunctionalDistribFun::ConvertToSparse() const
{
    const CCondFunctionalDistribFun* self = this;
    CCondFunctionalDistribFun* resDistrib = Copy(self);
    return resDistrib;
}

CDistribFun* CCondFunctionalDistribFun::ConvertToDense() const
{
    const CCondFunctionalDistribFun* self = this;
    CCondFunctionalDistribFun* resDistrib = Copy(self);
    return resDistrib;
}

void CCondFunctionalDistribFun::Dump() const
{
    Log dump("", eLOG_RESULT, eLOGSRV_PNL_POTENTIAL);
    
    int i;
    dump<<"I'm a Conditional Gaussian distribution function of "
            << m_NumberOfNodes
            <<" nodes.\nI have both discrete and continuous parents.\n";
    dump<<"The distributions for every discrete combination of parents are: \n";
    int numDiscrPs = m_discrParentsIndex.size();
    CMatrixIterator<CFunctionalDistribFun*>* iterThis =
            m_distribution->InitIterator();
    for (iterThis; m_distribution->IsValueHere(iterThis); m_distribution->Next(iterThis))
    {
        intVector index1;
        m_distribution->Index(iterThis, &index1);
        dump<<"Discrete parents combination is:";
        for (i = 0; i < numDiscrPs; i++)
        {
            dump<<index1[i]<<" ";
        }
        dump<<"\n";
        CFunctionalDistribFun* distr = *(m_distribution->Value(iterThis));
        distr->Dump();
    }
    delete iterThis;
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        dump<<"The default distribution is:\n";
        static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                m_distribution)->GetDefaultValue()->Dump();
    }
}

int CCondFunctionalDistribFun::IsSparse() const
{
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        return 1;
    } else
    {
        return 0;
    }
}
int CCondFunctionalDistribFun::IsDense() const
{
    if (m_distribution->GetMatrixClass() != mcSparse)
    {
        return 1;
    } else
    {
        return 0;
    }
}

CCondFunctionalDistribFun* CCondFunctionalDistribFun::EnterDiscreteEvidence(
    int nDiscrObsNodes, const int* discrObsNodes, const int *pDiscrValues,
    const CNodeType* pObsTabNodeType) const
{
    PNL_CHECK_RANGES( nDiscrObsNodes, 0, m_discrParentsIndex.size() );
    if (nDiscrObsNodes > 0)
    {
        PNL_CHECK_IS_NULL_POINTER( discrObsNodes );
        PNL_CHECK_IS_NULL_POINTER( pDiscrValues );
        PNL_CHECK_IS_NULL_POINTER( pObsTabNodeType );
        
        int i;
        if ( !pObsTabNodeType->IsDiscrete() || pObsTabNodeType->GetNodeSize()
                != 1)
        {
            PNL_THROW( CInconsistentType, "observed node type must be tabular" )
        }
        //find the position of discrete observed nodes in discrete part of domain
        int loc = 0;
        int discrSize = m_discrParentsIndex.size();
        intVector discrIndices;
        discrIndices.resize(nDiscrObsNodes);
        for (i = 0; i < nDiscrObsNodes; i++)
        {
            loc = std::find(m_discrParentsIndex.begin(),
                            m_discrParentsIndex.end(), discrObsNodes[i])
                    - m_discrParentsIndex.begin();
            if (loc < discrSize)
            {
                discrIndices[i] = loc;
            } else
            {
                PNL_THROW( CInconsistentSize, "discrete indices" );
            }
        }
        CMatrix<CFunctionalDistribFun*>
                * pSmallDistrib =
                        static_cast<CMatrix<CFunctionalDistribFun*>*>(m_distribution->ReduceOp(
                                                                                                &discrIndices.front(),
                                                                                               nDiscrObsNodes,
                                                                                                2,
                                                                                               pDiscrValues));
        pConstNodeTypeVector newNodeTypes;
        newNodeTypes.assign(m_NodeTypes.begin(), m_NodeTypes.end() );
        for (i = 0; i < nDiscrObsNodes; i++)
        {
            newNodeTypes[m_discrParentsIndex[discrIndices[i]]]
                    = pObsTabNodeType;
        }
        CCondFunctionalDistribFun* res = new CCondFunctionalDistribFun(
                m_bPotential, m_NumberOfNodes, &newNodeTypes.front(),
                pSmallDistrib );
        delete pSmallDistrib;
        return res;
    } else
    {
        return CCondFunctionalDistribFun::Copy( this);
    }
}

CTabularDistribFun* CCondFunctionalDistribFun::EnterFullContinuousEvidence(
    int nContObsParents, const int* contObsParentsIndices,
    const C2DNumericDenseMatrix<float>* obsChildValue,
    C2DNumericDenseMatrix<float>*const * obsValues,
    const CNodeType* pObsGauNodeType) const
{
    int numContParents = m_contParentsIndex.size();
    PNL_CHECK_RANGES( nContObsParents, 0, numContParents);
    if (nContObsParents)
    {
        PNL_CHECK_IS_NULL_POINTER( contObsParentsIndices );
        PNL_CHECK_IS_NULL_POINTER( obsValues );
    }
    PNL_CHECK_IS_NULL_POINTER( obsChildValue );
    PNL_CHECK_IS_NULL_POINTER( pObsGauNodeType );
    
    if (pObsGauNodeType->IsDiscrete() || pObsGauNodeType->GetNodeSize() )
    {
        PNL_THROW( CInconsistentType, "observed node type must be Gaussian" );
    }
    if (nContObsParents != numContParents)
    {
        PNL_THROW( CInvalidOperation, "all continuous nodes must be observed" );
    }
    //create mapping between observed nodes and indices
    //in corresponding gaussian distribution

    intVector indicesInGaussian;
    indicesInGaussian.resize(nContObsParents);
    int i;
    int location;
    for (i = 0; i < nContObsParents; i++)
    {
        location = std::find(contObsParentsIndices, contObsParentsIndices
                +nContObsParents, m_contParentsIndex[i])
                - contObsParentsIndices;
        if (location < numContParents)
        {
            indicesInGaussian[i] = location;
        } else
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
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        int dims;
        const int* ranges;
        
        m_distribution->GetRanges( &dims, &ranges);
        for (i = 0; i < dims; i++)
        {
            DiscrLength *= ranges[i];
        }
        //need to compute default probability
        double smallestProb= FLT_MAX;
        CFunctionalDistribFun* def = (static_cast<
                CSparseMatrix<CFunctionalDistribFun*>*>(m_distribution))->GetDefaultValue();
        double defProb = def->ComputeProbability(obsChildValue, 0,
                                                 nContObsParents,
                                                  &indicesInGaussian.front(),
                                                 obsValues);
        if ((fabs(defProb)<smallestProb))
        {
            smallestProb = fabs(defProb);
        }
        doubleVector resVect;
        resVect.assign(DiscrLength, defProb);
        CNumericDenseMatrix<double>* probMat =
                CNumericDenseMatrix<double>::Create(dims, ranges,
                                                     &resVect.front() );
        intVector index;
        index.assign(dims, 0);
        CMatrixIterator<CFunctionalDistribFun*>* iter =
                m_distribution->InitIterator();
        double someProb;
        for (iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter) )
        {
            CFunctionalDistribFun* current = *(m_distribution->Value(iter));
            m_distribution->Index(iter, &index);
            someProb = current->ComputeProbability(obsChildValue, asLog,
                                                   nContObsParents,
                                                    &indicesInGaussian.front(),
                                                   obsValues);
            probMat->SetElementByIndexes(someProb, &index.front() );
            if ((fabs(someProb) < smallestProb))
            {
                smallestProb = fabs(someProb);
            }
        }
        delete iter;
        const pnlVector<double>* vect = probMat->GetVector();
        int numProbs = vect->size();
        probability.resize(numProbs);
        for (i = 0; i < numProbs; i++)
        {
            probability[i] = exp((*vect)[i]+smallestProb);
            sum += probability[i];
        }
        //need to normalize data
        floatVector probs;
        probs.assign(probability.size(), 0.0f);
        if (sum)
        {
            for (i = 0; i < DiscrLength; i++)
            {
                probs[i] = (float)(probability[i]/sum);
            }
        }
        //create new node types
        pConstNodeTypeVector nTypes;
        nTypes.assign(m_NodeTypes.begin(), m_NodeTypes.end() );
        for (i = 0; i < nContObsParents; i++)
        {
            nTypes[contObsParentsIndices[i]] = pObsGauNodeType;
        }
        //make child node observed
        nTypes[m_NumberOfNodes - 1] = pObsGauNodeType;
        intVector nodeSizes;
        CTabularDistribFun* resData =
                CTabularDistribFun::Create(m_NumberOfNodes, &nTypes.front(),
                                            &probs.front() );
        return resData;
    } else
    {
        CFunctionalDistribFun* const* data;
        CDenseMatrix<CFunctionalDistribFun*>
                * matDistr =
                        static_cast< CDenseMatrix<CFunctionalDistribFun*>*>(m_distribution);
        matDistr->GetRawData( &DiscrLength, &data);
        doubleVector expOrders;
        expOrders.resize(DiscrLength);
        
        for (i = 0; i < DiscrLength; i++)
        {
            expOrders[i]
                    = data[i]->ComputeProbability(obsChildValue, asLog,
                                                  nContObsParents,
                                                   &indicesInGaussian.front(),
                                                  obsValues);
        }
        floatVector probs;
        GetNormExp(expOrders, &probs);
        
        //create new node types
        pConstNodeTypeVector nTypes;
        nTypes.assign(m_NodeTypes.begin(), m_NodeTypes.end() );
        for (i = 0; i < nContObsParents; i++)
        {
            nTypes[contObsParentsIndices[i]] = pObsGauNodeType;
        }
        //make child node observed
        nTypes[m_NumberOfNodes - 1] = pObsGauNodeType;
        intVector nodeSizes;
        CTabularDistribFun* resData =
                CTabularDistribFun::Create(m_NumberOfNodes, &nTypes.front(),
                                            &probs.front() );
        return resData;
    }
}

CCondFunctionalDistribFun::~CCondFunctionalDistribFun()
{
    if (m_distribution->GetNumOfReferences() == 1)
    {
        CMatrixIterator<CFunctionalDistribFun*>* iter =
                m_distribution->InitIterator();
        for (iter; m_distribution->IsValueHere(iter); m_distribution->Next(iter) )
        {
            CFunctionalDistribFun* current = *(m_distribution->Value(iter));
            delete current;
        }
        delete iter;
        if (m_distribution->GetMatrixClass() == mcSparse)
        {
            CFunctionalDistribFun* def = static_cast<CSparseMatrix<
            CFunctionalDistribFun*>*>(m_distribution)->GetDefaultValue();
            delete def;
        }
    }
    void* pObj = this;
    m_distribution->Release(pObj);
    delete m_numEvidencesLearned;
}

CCondFunctionalDistribFun::CCondFunctionalDistribFun(
    const CCondFunctionalDistribFun& inpDistr) :
    CDistribFun(dtCondGaussian)
{
    m_bUnitFunctionDistribution = inpDistr.m_bUnitFunctionDistribution;
    m_NodeTypes = pConstNodeTypeVector(inpDistr.m_NodeTypes.begin(),
                                       inpDistr.m_NodeTypes.end());// may be optimized
    m_NumberOfNodes = inpDistr.m_NumberOfNodes;
    m_bPotential = inpDistr.m_bPotential;
    m_numberOfDims = inpDistr.m_numberOfDims;
    m_contParentsIndex.assign(inpDistr.m_contParentsIndex.begin(),
                              inpDistr.m_contParentsIndex.end() );
    m_discrParentsIndex.assign(inpDistr.m_discrParentsIndex.begin(),
                               inpDistr.m_discrParentsIndex.end() );
    //need to copy conditional gaussian via copying every Gaussain
    //not only via copying matrices !!!
    m_distribution = inpDistr.m_distribution->Clone() ;
    CMatrixIterator<CFunctionalDistribFun*>* iter =
            inpDistr.m_distribution->InitIterator();
    for (iter; inpDistr.m_distribution->IsValueHere(iter); inpDistr.m_distribution->Next(iter) )
    {
        CFunctionalDistribFun* tempDistr =
                *(inpDistr.m_distribution->Value(iter));
        intVector index;
        inpDistr.m_distribution->Index(iter, &index);
        CFunctionalDistribFun* newDistr =
                CFunctionalDistribFun::Copy(tempDistr);
        m_distribution->SetElementByIndexes(newDistr, &index.front() );
    }
    delete iter;
    if (m_distribution->GetMatrixClass() == mcSparse)
    {
        CFunctionalDistribFun
                * copyDef =
                        CFunctionalDistribFun::Copy(static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                                inpDistr.m_distribution)->GetDefaultValue());
        //we can just set default value 
        //because the pointer to it was copied from input disrtribution
        static_cast<CSparseMatrix<CFunctionalDistribFun*>*>(
                m_distribution)->SetDefaultVal(copyDef);
    }
    void* pObj = this;
    m_distribution->AddRef(pObj);
    m_numEvidencesLearned
            = static_cast<CDenseMatrix<float>*>(inpDistr.m_numEvidencesLearned->Clone());
}

CCondFunctionalDistribFun::CCondFunctionalDistribFun(int isFactor, int nNodes,
    const CNodeType* const* nodeTypes, int asDenseMatrix,
    CFunctionalDistribFun* const pDefaultDistrib) :
    CDistribFun(dtCondGaussian, nNodes, nodeTypes, 0)
{
    m_numberOfDims = nodeTypes[nNodes - 1]->GetNodeSize();
    m_bPotential = isFactor;
    int i;
    for (i = 0; i < nNodes - 1; i++)
    {
        if (nodeTypes[i]->IsDiscrete() )
        {
            m_discrParentsIndex.push_back(i);
        } else
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
    if (asDenseMatrix)
    {
        pnlVector<CFunctionalDistribFun*> data;
        data.assign(dataSize, (CFunctionalDistribFun* const)NULL );
        m_distribution
                = CDenseMatrix<CFunctionalDistribFun*>::Create(
                                                               numDiscrParents,
                                                                &discrParentsSizes.front(),
                                                                &data.front());
    } else
    {
        CFunctionalDistribFun* pDefault =
                CFunctionalDistribFun::Copy(pDefaultDistrib);
        m_distribution
                = CSparseMatrix<CFunctionalDistribFun*>::Create(
                                                                numDiscrParents,
                                                                 &discrParentsSizes.front(),
                                                                pDefault);
    }
    void *pObj = this;
    m_distribution->AddRef(pObj);
    floatVector vectZeros = floatVector(dataSize, 0.0f);
    m_numEvidencesLearned
            = CDenseMatrix<float>::Create(numDiscrParents,
                                           &discrParentsSizes.front(),
                                           &vectZeros.front() );
}

CCondFunctionalDistribFun::CCondFunctionalDistribFun(int isFactor, int nNodes,
    const CNodeType* const* pNodeTypes,
    CMatrix<CFunctionalDistribFun*> const* pDistribMatrix) :
    CDistribFun(dtCondGaussian, nNodes, pNodeTypes, 0)
{
    m_numberOfDims = pNodeTypes[nNodes - 1]->GetNodeSize();
    m_bPotential = isFactor;
    int i;
    for (i = 0; i < nNodes - 1; i++)
    {
        if (pNodeTypes[i]->IsDiscrete() )
        {
            m_discrParentsIndex.push_back(i);
        } else
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
        int parentSize = pNodeTypes[m_discrParentsIndex[i]]->GetNodeSize();
        discrParentsSizes[i] = parentSize;
        dataSize *= parentSize;
    }
    int numRanges;
    const int* ranges;
    pDistribMatrix->GetRanges( &numRanges, &ranges);
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
        if (pDistribMatrix->GetMatrixClass() != mcSparse)
        {
            pnlVector<CFunctionalDistribFun*> copyData;
            copyData.resize(dataSize);
            for (i = 0; i < dataSize; i++)
            {
                copyData[i]
                        = pnl::CFunctionalDistribFun::Copy(static_cast<const CDenseMatrix<CFunctionalDistribFun*>*>(
                                pDistribMatrix)->GetElementByOffset(i));
            }
            m_distribution
                    = CDenseMatrix<CFunctionalDistribFun*>::Create(
                                                                   numDiscrParents,
                                                                    &discrParentsSizes.front(),
                                                                    &copyData.front());
        } else
        {
            CFunctionalDistribFun
                    * pDefault =
                            CFunctionalDistribFun::Copy(static_cast<const CSparseMatrix<CFunctionalDistribFun*>*>(
                                    pDistribMatrix)->GetDefaultValue());
            m_distribution
                    = pDistribMatrix->CreateEmptyMatrix(
                                                        numDiscrParents,
                                                         &discrParentsSizes.front(),
                                                         0, pDefault);
            //need to set elements one by one
            CMatrixIterator<CFunctionalDistribFun*>* iter =
                    pDistribMatrix->InitIterator();
            intVector index;
            for (iter; pDistribMatrix->IsValueHere(iter); pDistribMatrix->Next(iter) )
            {
                CFunctionalDistribFun* data = *(pDistribMatrix->Value(iter));
                CFunctionalDistribFun* cloneData =
                        CFunctionalDistribFun::Copy(data);
                pDistribMatrix->Index(iter, &index);
                m_distribution->SetElementByIndexes(cloneData, &index.front());
            }
            delete iter;
        }
        void *pObj = this;
        m_distribution->AddRef(pObj);
        floatVector vectZeros = floatVector(dataSize, 0.0f);
        m_numEvidencesLearned
                = CDenseMatrix<float>::Create(numDiscrParents,
                                               &discrParentsSizes.front(),
                                               &vectZeros.front() );
    } else
    {
        PNL_THROW( CInvalidOperation,
                "distribution matrix should have the same sizes as discrete parents" );
    }
}

int CCondFunctionalDistribFun::GetNumberOfFreeParameters() const
{
    if (m_bUnitFunctionDistribution)
    {
        PNL_THROW( CInconsistentType,
                "uniform distribution can't have any matrices with data" );
    }
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
    
    div_t result;
    intVector index;
    index.resize(numRanges);
    int nparams = 0;
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
        CFunctionalDistribFun* theDistr =
                m_distribution->GetElementByIndexes(&index.front());
        nparams += theDistr->GetNumberOfFreeParameters();
    }
    return nparams;
}

void CCondFunctionalDistribFun::GetNormExp(doubleVector &expOrders,
    floatVector *normExpData) const
{
    
    const double maxForExp = 200;
    const double minForExp = -36;
    double maxEl = expOrders[0];
    double minEl = maxEl;
    double sum = 0;
    
    int nEl = expOrders.size();
    normExpData->resize(nEl);
    
    int i;
    for (i = 0; i < nEl; i++)
    {
        
        if (expOrders[i] > maxEl)
        {
            maxEl = expOrders[i];
        } else
        {
            if (expOrders[i] < minEl)
            {
                minEl = expOrders[i];
            }
        }
    }
    if (maxEl > maxForExp)
    {
        double val = maxEl - maxForExp;
        for (i = 0; i < nEl; i++)
        {
            double pw = expOrders[i] - val;
            if (pw > minForExp)
            {
                expOrders[i] = exp(pw);
            } else
            {
                expOrders[i] = 0;
            }
            
            sum += expOrders[i];
        }
    } else
    {
        if (minEl < minForExp)
        {
            
            double val = minForExp - minEl;
            if (maxEl + val > maxForExp)
            {
                val = maxForExp - maxEl;
            }
            for (i = 0; i < nEl; i++)
            {
                if (expOrders[i] + val < minForExp)
                {
                    expOrders[i] = 0;
                } else
                {
                    expOrders[i] = exp(expOrders[i] + val);
                }
                sum += expOrders[i];
            }
        } else
        {
            
            for (i = 0; i < nEl; i++)
            {
                expOrders[i] = exp(expOrders[i]);
                sum += expOrders[i];
            }
        }
        
    }
    
    while (sum > DBL_MAX)
    {
        sum = 0;
        for (i = 0; i < nEl; i++)
        {
            expOrders[i] /= DBL_MAX/nEl;
            sum += expOrders[i];
        }
    }
    float s1 = 0.0f;
    for (i = 0; i < nEl; i++)
    {
        (*normExpData)[i] = (float)(expOrders[i] / sum);
        s1 += (*normExpData)[i];
    }
    
}

#ifdef PAR_PNL
void CCondFunctionalDistribFun::UpdateStatisticsML(CDistribFun *pPot)
{
    PNL_THROW(CNotImplemented,
            "UpdateStatisticsML for CCondFunctionalDistribFun not implemented yet");
};
#endif // PAR_OMP
void CCondFunctionalDistribFun::ResetNodeTypes(pConstNodeTypeVector &nodeTypes)
{
    CDistribFun::ResetNodeTypes(nodeTypes);
    
    int ncontPar = m_contParentsIndex.size();
    pConstNodeTypeVector ntCont(ncontPar + 1, (const CNodeType*)0);
    int i;
    for (i = 0; i < ncontPar; i++)
    {
        ntCont[i] = nodeTypes[m_contParentsIndex[i]];
    }
    ntCont.back() = nodeTypes.back();
    CMatrix<CFunctionalDistribFun*>* distribf =
            static_cast<CCondFunctionalDistribFun*>(this)
            ->GetMatrixWithDistribution();
    CMatrixIterator<CFunctionalDistribFun*>* iter = distribf->InitIterator();
    for (iter; distribf->IsValueHere(iter); distribf->Next(iter) )
    {
        CFunctionalDistribFun* tempDistr = *(distribf->Value(iter));
        tempDistr->ResetNodeTypes(ntCont);
    }
    delete iter;
}

#ifdef PNL_RTTI
const CPNLType CCondFunctionalDistribFun::m_TypeInfo = CPNLType("CCondFunctionalDistribFun", &(CDistribFun::m_TypeInfo));

#endif
