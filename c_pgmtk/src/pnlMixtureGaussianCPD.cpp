/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMixtureGaussianCPD.cpp                                   //
//                                                                         //
//  Purpose:   CMixtureGaussianCPD class member functions implementation   //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//MixtureGaussianCPD.cpp
///////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlCondGaussianDistribFun.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlInferenceEngine.hpp"
#include "pnliNumericDenseMatrix.hpp"
#include "pnlRng.hpp"
#include <float.h>
#include <math.h>

#include "cxcore.h"

PNL_USING

CMixtureGaussianCPD*
CMixtureGaussianCPD::Create(const intVector& domain, CModelDomain* pMD,
			    const floatVector& probabilities)
{
    return CMixtureGaussianCPD::Create( &domain.front(), domain.size(),
	pMD, &probabilities.front() );
}

void CMixtureGaussianCPD::AllocDistributionVec( const C2DNumericDenseMatrix<float>* meanMat,
		            const C2DNumericDenseMatrix<float>* covMat, float normCoeff,
		            const p2DDenseMatrixVector& weightsMat,
		            const intVector& discrComb  )
{
    int nel;
    const float * vls;

    floatVector mean;
    meanMat->GetRawData(&nel, &vls);
    mean.assign(vls, vls + nel);
    
    floatVector cov;
    covMat->GetRawData(&nel, &vls);
    cov.assign(vls, vls + nel);

    floatVecVector weights;
    int nmat = weightsMat.size();
    weights.resize(nmat);

    int i;
    for( i = 0; i < nmat; i++ )
    {
	weightsMat[i]->GetRawData(&nel, &vls);
	weights[i].assign(vls, vls + nel);
    }

    AllocDistributionVec( mean, cov, normCoeff, weights, discrComb );

}

void CMixtureGaussianCPD::AllocDistributionVec( const floatVector& mean,
                            const floatVector& cov, float normCoeff,
                            const floatVecVector& weights,
                            const intVector& discrComb  )
{
    if( weights.size() )
    {
        //allocate distribution with weights matrices
        pnlVector<const float*> pWeights;
        int numWeights = weights.size();
        pWeights.resize( numWeights );
        for( int i = 0; i < numWeights; i++ )
        {
            pWeights[i] = &weights[i].front();
        }
        AllocDistribution( &mean.front(), &cov.front(), normCoeff,
             &pWeights.front(), &discrComb.front() );
    }
    else
    {
        AllocDistribution( &mean.front(), &cov.front(), normCoeff, NULL,
            &discrComb.front() );
    }
}


CMixtureGaussianCPD* CMixtureGaussianCPD::Create( const int *domain, int nNodes,
                                                  CModelDomain* pMD,
                                                  const float* probabilities )
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );
    PNL_CHECK_IS_NULL_POINTER( probabilities );

    intVector dom = intVector( domain, domain + nNodes );
    pConstNodeTypeVector ntVec;
    pMD->GetVariableTypes( dom, &ntVec );

    if( ntVec[nNodes - 1]->IsDiscrete() )
    {
        PNL_THROW( CInvalidOperation,
            "GaussianCPD must have continuous node as child" );
    }
    else
    {
        //need to check discrete node in domain corresponds mixture node
        int isDiscrNode = 0;
        for( int i = 0; i < nNodes; i++ )
        {
            if( ntVec[i]->IsDiscrete() )
            {
                isDiscrNode = 1;
                break;
            }
        }
        if( !isDiscrNode )
        {
            PNL_THROW( CInconsistentType,
                "there must be at least one discrete node in domain - mixture node!" );
        }
	CMixtureGaussianCPD *pNewParam = new CMixtureGaussianCPD( domain,
	    nNodes, pMD, probabilities);
        PNL_CHECK_IF_MEMORY_ALLOCATED( pNewParam );
	return pNewParam;
    }
}

CMixtureGaussianCPD* CMixtureGaussianCPD::Copy( const CMixtureGaussianCPD* pGauCPD )
{
    PNL_CHECK_IS_NULL_POINTER( pGauCPD );

    CMixtureGaussianCPD *retCPD = new CMixtureGaussianCPD( *pGauCPD );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retCPD );
    return retCPD;
}

CFactor* CMixtureGaussianCPD::CloneWithSharedMatrices()
{
    CMixtureGaussianCPD* resCPD = new CMixtureGaussianCPD(this);
    PNL_CHECK_IF_MEMORY_ALLOCATED(resCPD);

    return resCPD;
}

CFactor* CMixtureGaussianCPD::Clone() const
{
    const CMixtureGaussianCPD* self = this;
    CMixtureGaussianCPD* res = CMixtureGaussianCPD::Copy(self);
    return res;
}


CMixtureGaussianCPD::CMixtureGaussianCPD( const CMixtureGaussianCPD& GauCPD )
:CCPD( dtMixGaussian, ftCPD, GauCPD.GetModelDomain() )
{
    //m_CorrespDistribFun = GauCPD.m_CorrespDistribFun->CloneDistribFun();
    delete m_CorrespDistribFun;
    m_CorrespDistribFun = CCondGaussianDistribFun::Copy(
            static_cast<CCondGaussianDistribFun*>(GauCPD.m_CorrespDistribFun));
    m_Domain = intVector( GauCPD.m_Domain );
    m_probabilities = floatVector( GauCPD.m_probabilities.begin(),
        GauCPD.m_probabilities.end() );
    int probSize = m_probabilities.size();
    m_learnProbabilities.assign( probSize, 0.0f );
}

CMixtureGaussianCPD::CMixtureGaussianCPD(const int *domain, int nNodes,
					 CModelDomain* pMD, const float *probabilities)
					 :CCPD( dtMixGaussian, ftCPD, domain, nNodes, pMD )
{
    intVector discrParentIndices;
    static_cast< CCondGaussianDistribFun *>(m_CorrespDistribFun)->
            GetDiscreteParentsIndices( &discrParentIndices );
    const pConstNodeTypeVector* nt = m_CorrespDistribFun->GetNodeTypesVector();
    int discrSize = (*nt)[discrParentIndices[discrParentIndices.size() - 1]]->GetNodeSize();
    m_probabilities.assign( probabilities, probabilities + discrSize );
    m_learnProbabilities.assign( discrSize, 0.0f );
}

CMixtureGaussianCPD::CMixtureGaussianCPD(const CMixtureGaussianCPD* pMixCPD):CCPD(pMixCPD)
{
}

void CMixtureGaussianCPD::AllocDistribution( const float* pMean, const float* pCov,
                         float normCoeff, const float* const* pWeights,
                         const int* discrComb )
{
    PNL_CHECK_IS_NULL_POINTER( pMean );
    PNL_CHECK_IS_NULL_POINTER( pCov );
    if( m_CorrespDistribFun->GetDistributionType() == dtGaussian )
    {
        PNL_THROW( CInvalidOperation,
            "we create mixture Gaussian as conditional Gaussian only" )
    }
    else
    {
        SetCoefficient( normCoeff, discrComb );
        AllocMatrix( pMean, matMean, -1, discrComb );
        AllocMatrix( pCov, matCovariance,  -1, discrComb );
        if( pWeights )
        {
            intVector contDomainPartIndex;
            static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun)->
                GetContinuousParentsIndices(&contDomainPartIndex);
            int numContParents = contDomainPartIndex.size();
            //all cont nodes in domain include last node - child
            for( int i = 0; i < numContParents; i++ )
            {
                if( pWeights[i] )
                {
                    m_CorrespDistribFun->AllocMatrix( pWeights[i],
                        matWeights, i, discrComb );
                }
            }
        }
    }
}

void CMixtureGaussianCPD :: NormalizeCPD()
{
    CDistribFun *normData = m_CorrespDistribFun->GetNormalized();
    SetDistribFun( normData );
    float precision = 1e-5f;
    //need to normalize m_probabilities vector
    float sum = 0.0f;
    int sizeProbHere = m_probabilities.size();
    for( int i = 0; i < sizeProbHere; i++ )
    {
        sum += m_probabilities[i];
    }
    if( fabs(sum - 1.0f) > precision )
    {
        for( int i = 0; i < sizeProbHere; i++ )
        {
            m_probabilities[i] /= sum;
        }
    }
}

CPotential *CMixtureGaussianCPD::ConvertToPotential() const
{
    PNL_THROW( CNotImplemented,
        "cant't convert to potential becouse of absense of MOG potetnial class" );
    return NULL;
}

//after entering evidence mixture discrete node shrinks to size 1(if corresponding flag == 1),
//but it didn't really takes any value - the node always hidden
CPotential*
CMixtureGaussianCPD::ConvertWithEvidenceToPotential(const CEvidence* pEvidence,
						    int flagSumOnMixtureNode) const
{
#if 0
    const intVector& allObsNodes,
	const pnlVector<const unsigned char*>& pObsValues,
	const CNodeType* pObsTabNodeType,
	const CNodeType* pObsGauNodeType,
	int flagSumOnMixtureNode
#endif

    CCondGaussianDistribFun* selfDistr = static_cast<CCondGaussianDistribFun*>
                                (m_CorrespDistribFun);
    intVector discrIndices;
    selfDistr->GetDiscreteParentsIndices( &discrIndices );
    intVector contIndices;
    selfDistr->GetContinuousParentsIndices( &contIndices );
    int i;
    int domSize = m_Domain.size();
    intVector obsIndices;
    obsIndices.reserve( domSize );
    intVector obsPosInDom;
    obsPosInDom.reserve( domSize );
    CModelDomain* pMD = GetModelDomain();
    const CNodeType* pObsTabNodeType = pMD->GetObsTabVarType();
    const CNodeType* pObsGauNodeType = pMD->GetObsGauVarType();
    intVector allObsNodes;
    pConstValueVector pObsValues;
    pEvidence->GetObsNodesWithValues( &allObsNodes, &pObsValues );
    intVector::const_iterator itObsNodesBeg = allObsNodes.begin();
    intVector::const_iterator itObsNodesEnd = allObsNodes.end();
    intVector::const_iterator location;

    for( i = 0; i < domSize; i++ )
    {
        if( ( location = std::find( itObsNodesBeg, itObsNodesEnd,
            m_Domain[i] ) ) != itObsNodesEnd )
        {
            obsIndices.push_back(location - itObsNodesBeg);
            obsPosInDom.push_back(i);
            if( i == discrIndices[discrIndices.size() - 1] )
            {
                PNL_THROW( CInvalidOperation, "mixture node is always hidden" )
            }
        }
    }
    int numObsInThis = obsIndices.size();
    pConstNodeTypeVector nTypes;
    const pConstNodeTypeVector* ntFromCPD = m_CorrespDistribFun->GetNodeTypesVector();
    nTypes.assign( ntFromCPD->begin(), ntFromCPD->end() );
    //need to enter discrete & continuous evidence separetly
    //before it create node types - if all nodes discrete
    //or there are only nodes of size 0 from continuous -
    // - result distribution type is Tabular

    //collect information for enter discrete evidence & continuous
    intVector obsDiscreteIndex;
    obsDiscreteIndex.reserve( numObsInThis );
    //observed discrete values put into int vector
    intVector obsDiscrVals;
    obsDiscrVals.reserve( numObsInThis );
    //collect information about Gaussian observed indices
    intVector obsGauIndex;
    obsGauIndex.reserve( numObsInThis );
    //continuous observed values into vector of matrices
    pnlVector<C2DNumericDenseMatrix<float>*> obsGauVals;
    obsGauVals.reserve( numObsInThis );
    //create matrix to store observed value of node
    C2DNumericDenseMatrix<float>* obsSelfVal = NULL;
    //create vectors for storage temporary objects
    int isTab;
    for( i = 0; i < numObsInThis; i++ )
    {
        int indexInVals = obsIndices[i];
        int indexInDom = obsPosInDom[i];
        isTab = nTypes[indexInDom]->IsDiscrete();
        if( isTab )
        {
            nTypes[indexInDom] = pObsTabNodeType;
            obsDiscreteIndex.push_back( indexInDom );
            obsDiscrVals.push_back( pObsValues[indexInVals]->GetInt() );
        }
        else
        {
            nTypes[indexInDom] = pObsGauNodeType;
            //create matrices to call Enter continuous evidence
            intVector dims;
            dims.assign( 2, 1 );
            dims[0] = (*ntFromCPD)[indexInDom]->GetNodeSize();
            //need to convert from values to float vector of required length
            floatVector valFl;
            valFl.assign( dims[0], 0.f );
            int j;
            for( j = 0; j < dims[0]; j++ )
            {
                valFl[j] = pObsValues[indexInVals][j].GetFlt();
            }
            if( indexInDom == domSize - 1 )
            {
               obsSelfVal = C2DNumericDenseMatrix<float>::Create( &dims.front(),
                   &valFl.front());
            }
            else
            {
                //store only parent indices
                obsGauIndex.push_back( indexInDom );
                C2DNumericDenseMatrix<float>* obsGauVal =
                    C2DNumericDenseMatrix<float>::Create( &dims.front(),
                    &valFl.front() );
                obsGauVals.push_back( obsGauVal );
            }
        }
    }
    //can enter discrete evidence first if all continuous nodes observed
    //need to check if all them observed!
    CCondGaussianDistribFun* withDiscrEv =
        (static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun))->
        EnterDiscreteEvidence(obsDiscreteIndex.size(),
        &obsDiscreteIndex.front(), &obsDiscrVals.front(), pObsTabNodeType );
    //need to enter continuous evidence
    CTabularDistribFun* resDistr = withDiscrEv->
            EnterFullContinuousEvidence( obsGauIndex.size(),
            &obsGauIndex.front(), obsSelfVal, &obsGauVals.front(),
            pObsGauNodeType );
    delete obsSelfVal;
    for( i = 0; i < obsGauVals.size(); i++ )
    {
        delete obsGauVals[i];
    }
    CTabularPotential* resPot = NULL;
    //find observed positions
    obsGauIndex.insert( obsGauIndex.end(),
                    obsDiscreteIndex.begin(), obsDiscreteIndex.end() );
    obsGauIndex.push_back( domSize - 1 );
    if( flagSumOnMixtureNode )
    {
        //get result tabular matrix and shrink it with summarize flag
        const CMatrix<float>* tabMat = resDistr->GetMatrix( matTable );
        intVector restDims;
        restDims.assign( m_Domain.size(), 0 );
        for( i = 0; i < m_Domain.size(); i++ )
        {
            restDims[i] = i;
        }
        restDims.erase( restDims.begin() + discrIndices[discrIndices.size() - 1] );
        CMatrix<float>* tabShrMat = tabMat->ReduceOp( &restDims.front(),
                      restDims.size(), 0 );
        CDenseMatrix<float>* denseShrMat = tabShrMat->ConvertToDense();
        const floatVector* vecData = denseShrMat->GetVector();
        //add to observed values mixture node
        obsGauIndex.push_back( discrIndices[discrIndices.size() - 1] );
        //need to convert from tabular without sum to sum product
        intVector obsIndices;
        resPot = CTabularPotential::Create( &m_Domain.front(), m_Domain.size(),
            GetModelDomain(), NULL, obsGauIndex );
        resPot->AllocMatrix( &vecData->front(), matTable );
        delete denseShrMat;
        delete tabShrMat;
    }
    else
    {
        resPot = CTabularPotential::Create( &m_Domain.front(), m_Domain.size(),
                                          GetModelDomain(),NULL, obsGauIndex );
        resPot->SetDistribFun( resDistr );
    }
    delete withDiscrEv;
    delete resDistr;
    return resPot;
     //otherwise enter continuous evidence first!
}

void CMixtureGaussianCPD::SetCoefficientVec(float coeff,
					    const intVector& parentCombination)
{
    SetCoefficient( coeff, &parentCombination.front() );
}

float CMixtureGaussianCPD::GetCoefficientVec( const intVector& parentCombination )
{
    return GetCoefficient( &parentCombination.front() );
}

void CMixtureGaussianCPD::SetCoefficient( float coeff, const int* discrComb )
{
    int isForCanonical = 0;
    static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun)
                    ->SetCoefficient( coeff, isForCanonical, discrComb );
}

float CMixtureGaussianCPD::GetCoefficient( const int* discrComb  )
{
    int forCanonical = 0;
    return static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun)
                 ->GetCoefficient( forCanonical, discrComb );
}

#if 0
void CMixtureGaussianCPD::UpdateStatisticsEM( const CPotential *pMargPot,
                                            const CEvidence *pEvidence )
{
    PNL_CHECK_IS_NULL_POINTER(pMargPot);
    const CDistribFun *pMargData = pMargPot->GetDistribFun();
    if( !pMargData )
    {
	PNL_THROW( CNULLPointer, "pMargData" );
    }
    //we can work up tabular data with all Gaussian nodes observed
    EDistributionType dt = pMargData->GetDistributionType();
    if( dt != dtTabular )
    {
        PNL_THROW( CNotImplemented,
            "we can learn only data with all Gaussian nodes observed" );
    }
    const CMatrix<float>* mat = pMargData->GetMatrix(matTable);
    intVector discrIndices;
    intVector contIndices;
    CCondGaussianDistribFun* correspDistr =
        static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun);
    correspDistr->GetDiscreteParentsIndices(&discrIndices);
    correspDistr->GetContinuousParentsIndices(&contIndices);
    int discSize = discrIndices.size();
    int mixNodePos = discSize - 1;
    CMatrix<float>* shrMat = mat->ReduceOp( &discrIndices[mixNodePos],
        1, 0 );
    CDenseMatrix<float>* denseMat = shrMat->ConvertToDense();
    const floatVector* vec = denseMat->GetVector();
    int sizeLearned = vec->size();
    int sizeProbHere = m_learnProbabilities.size();
    if( sizeLearned != sizeProbHere )
    {
        PNL_THROW( CInconsistentSize, "sizes of learned matrices should corresponds" )
    }

    int i;
    for( i = 0; i < sizeProbHere; i++ )
    {
        m_learnProbabilities[i] += (*vec)[i];
    }
    //find corresponding Gaussian distribution to add information from this evidence
    intVector contDomain;
    int contSize = contIndices.size();
    contDomain.resize( contSize + 1 );
    for( i = 0; i < contSize; i++ )
    {
        contDomain[i] = m_Domain[contIndices[i]];
    }
    contDomain[contSize] = m_Domain[m_Domain.size() - 1];
    intVector index;
    intVector tabIndex;
    tabIndex.assign( discSize, 0 );
    //find values of all other discrete nodes to set them
    for( i = 0; i < mixNodePos; i++ )
    {
        tabIndex[i] = pEvidence->GetValue(m_Domain[discrIndices[i]])->GetInt();
    }
    //as all tabular nodes (except mixture) are observed -
    //need to set indices from evidence only!
    CMatrix<CGaussianDistribFun*>* distribution =
                correspDistr->GetMatrixWithDistribution();
    CDenseMatrix<float>* numEvidencesLearned =
                correspDistr->GetMatrixNumEvidences();
    CMatrixIterator<float>* iter = shrMat->InitIterator();
    for( iter; shrMat->IsValueHere( iter ); shrMat->Next(iter) )
    {
        shrMat->Index( iter, &index );
        tabIndex[mixNodePos] = index[0];
        float valAdd = *(shrMat->Value( iter ));
        //learn corresp Gaussian distribution with weight coefficient
        CGaussianDistribFun* curLearn = distribution->GetElementByIndexes(
            &tabIndex.front() );
        curLearn->UpdateStatisticsML( &pEvidence, 1, &contDomain.front(), valAdd );
        //add weight coefficient to general number of evidences learned
        float valLearned = numEvidencesLearned->GetElementByIndexes(
            &tabIndex.front() );
        valLearned += valAdd;
        numEvidencesLearned->SetElementByIndexes( valLearned, &tabIndex.front() );
    }
    delete shrMat;
    delete denseMat;
    delete iter;
}
#endif

#if 1
void CMixtureGaussianCPD::UpdateStatisticsEM( const CPotential *pMargPot,
                                            const CEvidence *pEvidence )
{
    PNL_CHECK_IS_NULL_POINTER(pMargPot);
    const CDistribFun *pMargData = pMargPot->GetDistribFun();
    if( !pMargData )
    {
	PNL_THROW( CNULLPointer, "pMargData" );
    }
    //we can work up tabular data with all Gaussian nodes observed
    EDistributionType dt = pMargData->GetDistributionType();
    if( dt != dtTabular )
    {
        PNL_THROW( CNotImplemented,
            "we can learn only data with all Gaussian nodes observed" );
    }
    const CMatrix<float>* mat = pMargData->GetMatrix(matTable);
    intVector discrIndices;
    intVector contIndices;
    CCondGaussianDistribFun* correspDistr =
        static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun);
    correspDistr->GetDiscreteParentsIndices(&discrIndices);
    correspDistr->GetContinuousParentsIndices(&contIndices);
    int discSize = discrIndices.size();
    int mixNodePos = discSize - 1;
    CMatrix<float>* shrMat = mat->ReduceOp( &discrIndices[mixNodePos],
        1, 0 );
    
    int mClass = shrMat->GetMatrixClass();
    if(!(( mClass == mcSparse )||(mClass == mcNumericSparse)))
    {
        CDenseMatrix<float>* denseMat = static_cast<CDenseMatrix<float>*>(shrMat);
	const floatVector* vec = denseMat->GetVector();
	int sizeLearned = vec->size();
	int sizeProbHere = m_learnProbabilities.size();
	if( sizeLearned != sizeProbHere )
	{
	    PNL_THROW( CInconsistentSize, "sizes of learned matrices should corresponds" )
	}
	
	int i;
	for( i = 0; i < sizeProbHere; i++ )
	{
	    m_learnProbabilities[i] += (*vec)[i];
	}
    }
    else
    {
	int dim;
	const int* ranges;
	shrMat->GetRanges( &dim, &ranges );
	
	CMatrixIterator<float>* iter = shrMat->InitIterator();
	
	intVector index;
	int offset;
	for( iter; shrMat->IsValueHere( iter ); shrMat->Next(iter) )
	{
	    
	    index.clear();
	    shrMat->Index( iter, &index ); 
	    float valAdd = *(shrMat->Value( iter ));
	    offset = 0;
	    int i;
	    for( i = 0; i < dim; i++)
	    {
		offset = offset * ranges[i] + index[i];
	    }
	    m_learnProbabilities[offset] += valAdd;
	}
	
	
    }
    
   
    //////////////////////////////////////////////////////////////////////////
    
	m_CorrespDistribFun->UpdateStatisticsEM( pMargPot->GetDistribFun(), pEvidence, 1.0f,
	    &m_Domain.front() );
    
    //////////////////////////////////////////////////////////////////////////
    
    
    delete shrMat;
}

#endif
float CMixtureGaussianCPD::ProcessingStatisticalData( int numberOfEvidences)
{
    //need to normalize it
    int probLength = m_learnProbabilities.size();
    m_probabilities.assign( m_learnProbabilities.begin(),
        m_learnProbabilities.end() );
    float sum = 0.0f;
    int i;
    for( i = 0; i < probLength; i++ )
    {
        sum+= m_learnProbabilities[i];
    }
    if( sum > FLT_MIN*10.f )
    {
        if( fabs(sum - 1) > 0.00001f )
	{
	    float reciprocalSum = 1/sum;
	    floatVector::iterator it = m_probabilities.begin();
	    floatVector::iterator itE = m_probabilities.end();
	    for ( ; it != itE; it++ )
	    {
		(*it) *= reciprocalSum;
	    }
	}
    }
    else
    {
	sum = (float)probLength;
	float reciprocalSum = 1/sum;
        floatVector::iterator it = m_probabilities.begin();
        floatVector::iterator itE = m_probabilities.end();
	for ( ; it != itE; it++ )
	{
	    (*it) = reciprocalSum;
	}
    }
    return m_CorrespDistribFun->ProcessingStatisticalData((float)numberOfEvidences);
}


void CMixtureGaussianCPD::UpdateStatisticsML(const CEvidence* const* pEvidences,
                                     int EvidenceNumber)
{
    if( !pEvidences )
    {
	PNL_THROW( CNULLPointer, "evidences" )//no corresp evidences
    }
    if( EvidenceNumber <= 0 )
    {
	PNL_THROW(COutOfRange, "number of evidences must be positively")
    }
    m_CorrespDistribFun->UpdateStatisticsML( pEvidences, EvidenceNumber,
            &m_Domain.front() );

    //learn Tabular Node - do it itself
    //find the number of mixture node to learn it
    int i;
    intVector discrParIndex;
    static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun)->
                                GetDiscreteParentsIndices( &discrParIndex );
    int learningNode = m_Domain[discrParIndex[discrParIndex.size() - 1]];
    for( i = 0; i < EvidenceNumber; i++ )
    {
        int counter = pEvidences[i]->GetValue(learningNode)->GetInt();
        m_learnProbabilities[counter]++;
    }
}

int CMixtureGaussianCPD::GetNumberOfMixtureNode() const
{
    intVector discreteParentsIndex;
    static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun)->
        GetDiscreteParentsIndices( &discreteParentsIndex );
    return m_Domain[discreteParentsIndex[discreteParentsIndex.size()-1]];
}



void CMixtureGaussianCPD::
GenerateSample( CEvidence* evidence, int maximize ) const
{
    if(maximize)
    {
	PNL_THROW(CNotImplemented, "not implemented for mixture gaussin")
    }
    const CCondGaussianDistribFun *pDistrFun =
	static_cast<const CCondGaussianDistribFun*>(m_CorrespDistribFun);

    int chldNode = m_Domain.back();

    intVector discrParIndex;
    pDistrFun->GetDiscreteParentsIndices( &discrParIndex );
    int numDiscrParents = discrParIndex.size();

    intVector contParIndex;
    pDistrFun->GetContinuousParentsIndices( &contParIndex );
    int numContParents = contParIndex.size();
    intVector contNodes(numContParents + 1);
    int i;
    for( i = 0; i < numContParents; i++ )
    {
	contNodes[i] = m_Domain[contParIndex[i]];
    }
    contNodes[i] = chldNode;

    floatVector probabilities;
    GetProbabilities( &probabilities );

    intVector valuesVec(numDiscrParents);


    for( i = 0; i < numDiscrParents - 1; i++ )
    {
	valuesVec[i] = evidence->GetValue( m_Domain[discrParIndex[i]] )->GetInt();
    }

    float val = 0.0f;

    float rnd = pnlRand(0.0f, 1.0f);

    for( i = 0 ; i < probabilities.size(); i++ )
    {
	val += probabilities[i];
	if( rnd <= val)
	{
	    break;
	}
    }

    valuesVec[numDiscrParents-1] = i;

    const CGaussianDistribFun *pCurrentDistr;
    pCurrentDistr = pDistrFun->GetDistribution( &valuesVec.front() );

    CGaussianCPD *pGaussCPD;
    pGaussCPD = CGaussianCPD::Create( contNodes,  this->GetModelDomain() );
    pGaussCPD->SetDistribFun(pCurrentDistr);

    pGaussCPD->GenerateSample(evidence, maximize);
    delete pGaussCPD;
}

CPotential* CMixtureGaussianCPD::ConvertStatisticToPot(int numOfSamples) const
{
    return NULL;
}

float CMixtureGaussianCPD::GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes ) const
{
    intVector domain;

    GetDomain(&domain);

    CModelDomain* pMd = GetModelDomain();
    CGaussianCPD *tmpCPD = CGaussianCPD::Create(domain, pMd);
    const CCondGaussianDistribFun *pDistrFun =
        static_cast<const CCondGaussianDistribFun*>(GetDistribFun());

    tmpCPD->SetDistribFun(pDistrFun);

    floatVector probabilities;
    GetProbabilities( &probabilities );

    int mixNode = GetNumberOfMixtureNode();

    intVector ndsSz(domain.size());
    int nVls = 0;

    intVector::iterator ndSzIt = ndsSz.begin();
    intVector::iterator domIt = domain.begin();
    for( ; domIt != domain.end(); domIt++, ndSzIt++ )
    {
        *ndSzIt = pMd->GetNumVlsForNode(*domIt);
        nVls += *ndSzIt;
    }

    valueVector vlsVec;
    intVector obsNodes;

    vlsVec.reserve(nVls);
    obsNodes.reserve(domain.size());

    domIt = domain.begin();
    ndSzIt = ndsSz.begin();
    for( ; domIt != domain.end(); domIt++, ndSzIt++ )
    {
        if(pEv->IsNodeObserved(*domIt))
        {
            obsNodes.push_back(*domIt);
            int j;
            for( j = 0; j < *ndSzIt; j++ )
            {
                vlsVec.push_back(pEv->GetValue(*domIt)[j]);
            }
        }
    }
    obsNodes.push_back(mixNode);
    Value val;
    val.SetInt(0);
    vlsVec.push_back(val);

    CEvidence *tmpEv = CEvidence::Create(pMd, obsNodes, vlsVec);

    float logLik = 0.0f;

    int mixSz = pMd->GetVariableType(mixNode)->GetNodeSize();
    int i = 0;

    for( i; i < mixSz; i++ )
    {
        tmpEv->GetValue(mixNode)->SetInt(i);
        logLik += probabilities[i]*tmpCPD->GetLogLik(tmpEv, pShrInfRes );

    }

    delete tmpCPD;
    delete tmpEv;
    return logLik;
}

#ifdef PAR_PNL
void CMixtureGaussianCPD::UpdateStatisticsML(CFactor *pPot)
{
    PNL_THROW(CNotImplemented, 
        "UpdateStatisticsML for CMixtureGaussianCPD not implemented yet");
};
#endif // PAR_OMP

#ifdef PNL_RTTI
const CPNLType CMixtureGaussianCPD::m_TypeInfo = CPNLType("CMixtureGaussianCPD", &(CCPD::m_TypeInfo));

#endif

