/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGaussianCPD.cpp                                          //
//                                                                         //
//  Purpose:   CGaussianCPD class member functions implementation          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//GaussianCPD.cpp
///////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlCondGaussianDistribFun.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlTabularPotential.hpp"
#include <float.h>
#include <math.h>
#include "pnlInferenceEngine.hpp"
#include "pnlScalarPotential.hpp"

PNL_USING

CGaussianCPD* CGaussianCPD::Create( const intVector& domain, CModelDomain* pMD )
{
    return CGaussianCPD::Create( &domain.front(), domain.size(), pMD );
}
CGaussianCPD* CGaussianCPD::CreateUnitFunctionCPD( const intVector& domain,
		                                  CModelDomain* pMD)
{
    return CGaussianCPD::CreateUnitFunctionCPD( &domain.front(), domain.size(),
		                                    pMD);
}

void CGaussianCPD::AllocDistribution( const C2DNumericDenseMatrix<float>* meanMat,
		            const C2DNumericDenseMatrix<float>* covMat, float normCoeff,
		            const p2DDenseMatrixVector& weightsMat,
		            const intVector& parentCombination  )
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

    AllocDistribution( mean, cov, normCoeff, weights, parentCombination );

}
void CGaussianCPD::AllocDistribution( const floatVector& mean,
		            const floatVector& cov, float normCoeff,
		            const floatVecVector& weights,
		            const intVector& parentCombination  )
{
    if( weights.size() )
    {
	pnlVector<const float*> pWeights;
	int numWeights = weights.size();
	pWeights.resize( numWeights );
	for( int i = 0; i < numWeights; i++ )
	{
	    pWeights[i] = &weights[i].front();
	}
	AllocDistribution( &mean.front(), &cov.front(), normCoeff,
	    &pWeights.front(), &parentCombination.front() );
    }
    else
    {
	AllocDistribution( &mean.front(), &cov.front(), normCoeff, NULL,
	    &parentCombination.front() );
    }
}
void CGaussianCPD::SetCoefficientVec( float coeff,
		                     const intVector& parentCombination )
{
    SetCoefficient( coeff, &parentCombination.front() );
}
float CGaussianCPD::GetCoefficientVec( const intVector& parentCombination )
{
    return GetCoefficient( &parentCombination.front() );
}

CGaussianCPD* CGaussianCPD::Create( const int *domain, int nNodes,
		                   CModelDomain* pMD )
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );

    CGaussianCPD *pNewParam = new CGaussianCPD( domain, nNodes, pMD);
    PNL_CHECK_IF_MEMORY_ALLOCATED( pNewParam );
    return pNewParam;
}

CGaussianCPD* CGaussianCPD::Copy( const CGaussianCPD* pGauCPD )
{
    PNL_CHECK_IS_NULL_POINTER( pGauCPD );

    CGaussianCPD *retCPD = new CGaussianCPD( *pGauCPD );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retCPD );
    return retCPD;
}

CGaussianCPD*
CGaussianCPD::CreateUnitFunctionCPD(const int *domain, int nNodes, CModelDomain* pMD)
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );

    CGaussianCPD* resCPD = new CGaussianCPD( domain, nNodes, pMD );
    intVector dom = intVector( domain, domain + nNodes );
    pConstNodeTypeVector ntVec;
    pMD->GetVariableTypes( dom, &ntVec );
    CGaussianDistribFun* UniData =
	CGaussianDistribFun::CreateUnitFunctionDistribution( nNodes,
	&ntVec.front(), 0, 0);
    delete (resCPD->m_CorrespDistribFun);
    resCPD->m_CorrespDistribFun = UniData;
    return resCPD;
}

CFactor* CGaussianCPD::CloneWithSharedMatrices()
{
    CGaussianCPD* resCPD = new CGaussianCPD(this);
    PNL_CHECK_IF_MEMORY_ALLOCATED(resCPD);

    return resCPD;
}

CFactor* CGaussianCPD::Clone() const
{
    const CGaussianCPD* self = this;
    CGaussianCPD* res = CGaussianCPD::Copy(self);
    return res;
}

CGaussianCPD::CGaussianCPD( const CGaussianCPD& GauCPD )
:CCPD( dtGaussian, ftCPD, GauCPD.GetModelDomain() )
{
    //m_CorrespDistribFun = GauCPD.m_CorrespDistribFun->CloneDistribFun();
    if( GauCPD.m_CorrespDistribFun->GetDistributionType() == dtGaussian )
    {
	delete m_CorrespDistribFun;
	m_CorrespDistribFun = CGaussianDistribFun::Copy(
	    static_cast<CGaussianDistribFun*>(GauCPD.m_CorrespDistribFun ));
    }
    else
    {
	if( GauCPD.m_CorrespDistribFun->GetDistributionType() == dtCondGaussian )
	{
	    delete m_CorrespDistribFun;
	    m_CorrespDistribFun = CCondGaussianDistribFun::Copy(
		static_cast<CCondGaussianDistribFun*>(GauCPD.m_CorrespDistribFun));
	}
	else
	{
	    PNL_THROW( CInconsistentType,
		"distribution must be gaussian or conditional gaussian" )
	}
    }
    m_Domain = intVector( GauCPD.m_Domain );
}

CGaussianCPD::CGaussianCPD( const int *domain, int nNodes, CModelDomain* pMD )
:CCPD( dtGaussian, ftCPD, domain, nNodes, pMD )
{
}

CGaussianCPD::CGaussianCPD( const CGaussianCPD* pGauCPD ):CCPD(pGauCPD)
{
}

void
CGaussianCPD::AllocDistribution(const float* pMean, const float* pCov,
				float normCoeff, const float* const* pWeights,
				const int* parentCombination )
{
    PNL_CHECK_IS_NULL_POINTER( pMean );
    PNL_CHECK_IS_NULL_POINTER( pCov );
    if( m_CorrespDistribFun->GetDistributionType() == dtGaussian )
    {
        SetCoefficient( normCoeff, 0 );
	    AllocMatrix( pMean, matMean );
	    AllocMatrix( pCov, matCovariance );
	    if( pWeights )
	    {
	        int numParents = m_Domain.size() - 1;
	        for( int i = 0; i < numParents; i++ )
	        {
		        if( pWeights[i] )
		        {
		            m_CorrespDistribFun->AllocMatrix( pWeights[i],
		                matWeights, i );
		        }
	        }
	    }
    }
    else
    {
	    PNL_CHECK_IS_NULL_POINTER( parentCombination );
        SetCoefficient( normCoeff, parentCombination );
	    AllocMatrix( pMean, matMean, -1, parentCombination );
	    AllocMatrix( pCov, matCovariance,  -1, parentCombination );
	    if( pWeights )
	    {
	        intVector contParentsIndex;
	        static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun)->
		    GetContinuousParentsIndices(&contParentsIndex);
	        int numParents = contParentsIndex.size();
	        for( int i = 0; i < numParents; i++ )
	        {
		        if( pWeights[i] )
		        {
		            m_CorrespDistribFun->AllocMatrix( pWeights[i],
		                matWeights, i, parentCombination );
		        }
	        }
	    }
    }
}

void CGaussianCPD::NormalizeCPD()
{
    CDistribFun *normData = m_CorrespDistribFun->GetNormalized();
    SetDistribFun( normData );
}

CPotential *CGaussianCPD::ConvertToPotential() const
{
    CDistribFun *gauFactData = (m_CorrespDistribFun)->ConvertCPDDistribFunToPot();
    CGaussianPotential *resFactor = CGaussianPotential::Create(	&m_Domain.front(), m_Domain.size(), GetModelDomain() );
    resFactor->SetDistribFun( gauFactData );
    delete gauFactData;
    return resFactor;
}

CPotential*
CGaussianCPD::ConvertWithEvidenceToPotential(const CEvidence* pEvidence,
					     int flagSumOnMixtureNode )const
{
    if( m_CorrespDistribFun->GetDistributionType() == dtGaussian )
    {
	    //need to convert to potential and after that add evidence
	    CPotential* potWithoutEv = ConvertToPotential();
	    CPotential* potWithEvid = potWithoutEv->ShrinkObservedNodes(pEvidence);
	    delete potWithoutEv;
	    return potWithEvid;
    }
    else //it means m_CorrespDistribFun->GetDistributionType == dtCondGaussian
    {
        //need to enter discrete & continuous evidence separetly
        //before it create node types - if all nodes discrete
        //or there are only nodes of size 0 from continuous -
        // - result distribution type is Tabular
        
        //collect information for enter discrete evidence & continuous
        int domSize = m_Domain.size();
        intVector obsDiscreteIndex;
        obsDiscreteIndex.reserve( domSize );
        //observed discrete values put into int vector
        intVector obsDiscrVals;
        obsDiscrVals.reserve( domSize );
        //collect information about Gaussian observed indices
        intVector obsGauIndex;
        obsGauIndex.reserve( domSize );
        //continuous observed values into vector of matrices
        pnlVector<C2DNumericDenseMatrix<float>*> obsGauVals;
        obsGauVals.reserve( domSize );
        //create matrix to store observed value of node
        C2DNumericDenseMatrix<float>* obsSelfVal = NULL;
        //create vectors for storage temporary objects
        int i;
        int isTab;
        for( i = 0; i < domSize; i++ )
        {
            int curNum = m_Domain[i];
            if( pEvidence->IsNodeObserved(curNum) )
            {
                const CNodeType* nt = GetModelDomain()->GetVariableType( curNum );
                isTab = nt->IsDiscrete();
                if( isTab )
                {
                    obsDiscreteIndex.push_back( i );
                    obsDiscrVals.push_back( pEvidence->GetValue(curNum)->GetInt() );
                }
                else
                {
                    int contSize = nt->GetNodeSize();
                    //create matrices to call Enter continuous evidence
                    floatVector val;
                    val.resize(contSize);
                    const Value* vFromEv = pEvidence->GetValue(curNum);
                    for( int j = 0; j < contSize; j++ )
                    {
                        val[j] = vFromEv[j].GetFlt();
                    }
                    intVector dims;
                    dims.assign( 2, 1 );
                    dims[0] = contSize;
                    if( i == domSize - 1 )
                    {
                        obsSelfVal = C2DNumericDenseMatrix<float>::Create(
                            &dims.front(), &val.front());
                    }
                    else
                    {
                        //store only parent indices
                        obsGauIndex.push_back( i );
                        C2DNumericDenseMatrix<float>* obsGauVal =
                            C2DNumericDenseMatrix<float>::Create(
                            &dims.front(), &val.front() );
                        obsGauVals.push_back( obsGauVal );
                    }
                }
            }
        } //        for( i = 0; i < domSize; i++ )

        CModelDomain* pMD = GetModelDomain();
        
        CPotential* resPot = NULL;
        int isLastNodeObs = pEvidence->IsNodeObserved(m_Domain[m_Domain.size()-1]); 
        if( (obsDiscreteIndex.size() + obsGauIndex.size() == m_Domain.size()-1) && isLastNodeObs)
        {
            //result distribution is scalar
            obsDiscreteIndex.insert(obsDiscreteIndex.end(), obsGauIndex.begin(),
                obsGauIndex.end());
            //child node is observed
            obsDiscreteIndex.insert(obsDiscreteIndex.end(), domSize-1);  
            resPot = CScalarPotential::Create( m_Domain, GetModelDomain(), obsDiscreteIndex );
        }

        else
        {
            const CNodeType* nt;
            //if all discrete nodes are observed then distribution will be Gaussian (Bader - comment)
            int allDiscrObs = 1;
            int allContObs = 1;
            int i;
            int isTab;
            int isCon;
            for( i = 0; i < domSize; i++ )
            {
                int curNum = m_Domain[i];
                nt = GetModelDomain()->GetVariableType( curNum );
                isTab = nt->IsDiscrete();
                isCon = !(isTab);
                if( isTab )
                    if( !(pEvidence->IsNodeObserved(curNum)) )
                        allDiscrObs = 0;
                    if( isCon )
                        if( !(pEvidence->IsNodeObserved(curNum)) )
                            allContObs = 0;
            }
            if (allContObs && (!allDiscrObs) )
            {
                CCondGaussianDistribFun* withDiscrEv =
                    (static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun))->
                    EnterDiscreteEvidence(obsDiscreteIndex.size(),
                    &obsDiscreteIndex.front(), &obsDiscrVals.front(),
                    pMD->GetObsTabVarType() );
                
                CTabularDistribFun* resDistr = withDiscrEv->
                    EnterFullContinuousEvidence( obsGauIndex.size(),
                    &obsGauIndex.front(), obsSelfVal, &obsGauVals.front(),
                    pMD->GetObsGauVarType() );

                //need to unite gaussian and tabular observed index
                obsDiscreteIndex.insert(obsDiscreteIndex.end(), obsGauIndex.begin(),
                    obsGauIndex.end());
                //child node is observed
                obsDiscreteIndex.insert(obsDiscreteIndex.end(), domSize-1);
                resPot = CTabularPotential::Create(
                    &m_Domain.front(), m_Domain.size(), GetModelDomain(), NULL,
                    obsDiscreteIndex );
                resPot->SetDistribFun( resDistr );
                delete withDiscrEv;
                delete resDistr;
            }
            else
            {
                if (allDiscrObs && !allContObs)
                {
                    intVector discParents;
                    
                    for ( i = 0; i < m_Domain.size(); i++)
                    {
                        nt = GetModelDomain()->GetVariableType( m_Domain[i] );
                        if (nt->IsDiscrete())
                            discParents.push_back(m_Domain[i]);
                    }
                    
                    int *parentComb = new int [discParents.size()];
                    
                    intVector pObsNodes;
                    pConstValueVector pObsValues;
                    pConstNodeTypeVector pNodeTypes;
                    pEvidence->GetObsNodesWithValues(&pObsNodes, &pObsValues, &pNodeTypes);
                    
                    int j;
                    int location;
                    for ( j = 0; j < discParents.size(); j++)
                    {
                        location = 
                            std::find(pObsNodes.begin(), pObsNodes.end(), discParents[j]) 
                            - pObsNodes.begin();
                        parentComb[j] = pObsValues[location]->GetInt();
                    }
                    
                    const CGaussianDistribFun* resDistr = 
                        static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun)->GetDistribution(parentComb);
                    
                    CDistribFun* newResDistr = resDistr->ConvertCPDDistribFunToPot();

                    obsGauIndex.insert(obsGauIndex.end(), obsDiscreteIndex.begin(),
                        obsDiscreteIndex.end());
                    intVector gauSubDomain;
                    for( j = 0; j < m_Domain.size(); j++)
                    {
                        nt = GetModelDomain()->GetVariableType( m_Domain[j] );
                        if(!(nt->IsDiscrete()))
                            gauSubDomain.push_back(m_Domain[j]);
                    }
                    resPot = CGaussianPotential::Create( &gauSubDomain.front(), 
                        gauSubDomain.size(), GetModelDomain());
                    resPot->SetDistribFun( newResDistr );
                    delete newResDistr;
                    delete [] parentComb;
                }
                else
                {
                   //can enter discrete evidence first if all continuous nodes observed
                   //need to check if all them observed!
                    CCondGaussianDistribFun* withDiscrEv =
                        (static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun))->
                        EnterDiscreteEvidence(obsDiscreteIndex.size(),
                        &obsDiscreteIndex.front(), &obsDiscrVals.front(),
                        pMD->GetObsTabVarType() );
                    //need to enter continuous evidence
                    CTabularDistribFun* resDistr = withDiscrEv->
                        EnterFullContinuousEvidence( obsGauIndex.size(),
                        &obsGauIndex.front(), obsSelfVal, &obsGauVals.front(),
                        pMD->GetObsGauVarType() );
                    //need to unite gaussian and tabular observed index
                    obsDiscreteIndex.insert(obsDiscreteIndex.end(), obsGauIndex.begin(),
                        obsGauIndex.end());
                    //child node is observed
                    obsDiscreteIndex.insert(obsDiscreteIndex.end(), domSize-1);
                    resPot = CTabularPotential::Create(
                        &m_Domain.front(), m_Domain.size(), GetModelDomain(), NULL,
                        obsDiscreteIndex );
                    resPot->SetDistribFun( resDistr );
                    delete withDiscrEv;
                    delete resDistr;
                }
            }
        }
        delete obsSelfVal;
        for( i = 0; i < obsGauVals.size(); i++ )
        {
            delete obsGauVals[i];
        }
        return resPot;
    }
}

void CGaussianCPD::SetCoefficient( float coeff,
		                   const int* pParentCombination )
{
    EDistributionType dt = m_CorrespDistribFun->GetDistributionType();
    int isForCanonical = 0;
    if( dt == dtTabular )
    {
	PNL_THROW( CInvalidOperation,
	    "normal coefficient is only for non Tabular" )
    }
    else
    {
	if( dt == dtGaussian )
	{
	    static_cast<CGaussianDistribFun*>(m_CorrespDistribFun)->
		SetCoefficient( coeff, isForCanonical );
	}
	else
	{
	    if( dt == dtCondGaussian )
	    {
		static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun)
		    ->SetCoefficient( coeff, isForCanonical, pParentCombination );
	    }
	}
    }

}

float CGaussianCPD::GetCoefficient( const int* parentCombination  )
{
    EDistributionType dt = m_CorrespDistribFun->GetDistributionType();
    int forCanonical = 0;
    float val = FLT_MAX;
    if( dt == dtGaussian )
    {
	val = static_cast<CGaussianDistribFun*>(m_CorrespDistribFun)->
	    GetCoefficient( forCanonical );
    }
    else
    {
	if( dt == dtCondGaussian )
	{
	     val = static_cast<CCondGaussianDistribFun*>(m_CorrespDistribFun)
		 ->GetCoefficient( forCanonical, parentCombination );
	}
    }
    return val;
}

void CGaussianCPD::UpdateStatisticsEM( const CPotential *pMargPot,
		                     const CEvidence *pEvidence )
{
    if( !pMargPot )
    {
	PNL_THROW( CNULLPointer, "evidences" )//no corresp evidences
    }

    intVector obsPos;
    pMargPot->GetObsPositions(&obsPos);

    if( obsPos.size() && (this->GetDistribFun()->GetDistributionType() != dtCondGaussian) )
    {
	PNL_CHECK_IS_NULL_POINTER(pEvidence);
	CPotential *pExpandPot = pMargPot->ExpandObservedNodes(pEvidence, 0);
	m_CorrespDistribFun->UpdateStatisticsEM(pExpandPot->GetDistribFun(), pEvidence, 1.0f,
	    &m_Domain.front());
	delete pExpandPot;
    }
    else
    {
	m_CorrespDistribFun->UpdateStatisticsEM( pMargPot->GetDistribFun(), pEvidence, 1.0f,
	    &m_Domain.front() );
    }

}
float CGaussianCPD::ProcessingStatisticalData( int numberOfEvidences)
{
    return m_CorrespDistribFun->ProcessingStatisticalData(static_cast<float>(numberOfEvidences));
}


void CGaussianCPD::UpdateStatisticsML(const CEvidence* const* pEvidences,
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
}


void CGaussianCPD::
GenerateSample( CEvidence* evidence, int maximize ) const
{
    int nnodes;
    const int *domain;
    this->GetDomain( &nnodes, &domain );
    int i;
    for( i = 0; i < nnodes - 1; i++ )
    {
	if( ! evidence->IsNodeObserved( domain[i]) )
	{
	    PNL_THROW(CAlgorithmicException, "all parents must be observed");
	}
    }
    if( evidence->IsNodeObserved( domain[nnodes-1] ) )
    {
	PNL_THROW(CAlgorithmicException, "node is already observed");
    }

    CPotential *resultPot;
    if( this->GetDistribFun()->GetDistributionType() == dtCondGaussian )
    {
	CCondGaussianDistribFun *tmpDistr = (static_cast<CCondGaussianDistribFun*>(this->GetDistribFun()));
	intVector discrParentIndices;
	tmpDistr->GetDiscreteParentsIndices( &discrParentIndices );
	intVector contParentIndices;
	tmpDistr->GetContinuousParentsIndices( &contParentIndices );

	intVector discrParentVls(discrParentIndices.size());

	for ( i = 0; i < discrParentIndices.size(); i++ )
	{
	    int node = m_Domain[discrParentIndices[i]];
	    if( !evidence->IsNodeObserved( node ) )
	    {
		PNL_THROW(CAlgorithmicException, "all parents must be observed")
	    }
	    discrParentVls[i] = evidence->GetValue( node )->GetInt();
	}


	const CGaussianDistribFun *currentDistrib = tmpDistr->GetDistribution(&discrParentVls.front());
	intVector domain(contParentIndices.size() + 1);
	for( i = 0; i < contParentIndices.size(); i++ )
	{
	    domain[i] = m_Domain[contParentIndices[i]];
	}
	domain[i] = m_Domain.back();
	CGaussianCPD *tmpCPD = CGaussianCPD::Create(domain, this->GetModelDomain());

	//currentDistrib->UpdateMomentForm();
	tmpCPD->SetDistribFun( currentDistrib );
	if( contParentIndices.size() )
	{

	   resultPot = tmpCPD->ConvertWithEvidenceToPotential( evidence );

	}
	else
	{
	    resultPot = tmpCPD->ConvertToPotential();

	}
	delete tmpCPD;
    }
    else
    {
	if( nnodes > 1)
	{
	    resultPot = this->ConvertWithEvidenceToPotential( evidence );
	}
	else
	{
	    resultPot = this->ConvertToPotential();
	}
    }

    resultPot->GenerateSample( evidence, maximize );
    delete resultPot;
}


/*

void CGaussianCPD::
GenerateSample( CEvidence* evidence, unsigned int seed ) const
{

    //generate samples for node using iformation about its parens
    const int *pDomain;
    int nnodes;
    this->GetDomain( &nnodes, &pDomain );

    int chldSz = (evidence->GetNodeTypes())[pDomain[nnodes-1]]->GetNodeSize();

    int i;

    intVector discrParents;
    intVector contParents;
    intVector discrParVls;

    if( m_CorrespDistribFun->GetDistributionType() == dtCondGaussian )
    {
	static_cast< CCondGaussianDistribFun *> (m_CorrespDistribFun)->
	    GetDiscreteParentsIndices( &discrParents );
	static_cast< CCondGaussianDistribFun *> (m_CorrespDistribFun)->
	    GetContinuousParentsIndices( &contParents );

	discrParVls.resize( discrParents.size() );
    }
    else
    {
	contParents.resize( nnodes - 1 );
	for( i = 0; i < nnodes - 1; i++)
	{
	    contParents[i] = i;
	}
    }

    C2DNumericDenseMatrix<float> *prntValue;
    C2DNumericDenseMatrix<float> *tmpMatrix = NULL;
    C2DNumericDenseMatrix<float> *matrixMean;

    const C2DNumericDenseMatrix<float> *matrixCov;
    const C2DNumericDenseMatrix<float> *matrixWeight;

    floatVector covData( chldSz*chldSz );
    floatVector evecData( chldSz*chldSz );
    floatVector evalData( chldSz );
    floatVector rnd( chldSz );

    CvMat covMat = cvMat( chldSz, chldSz, CV_32F, &covData.front() );
    CvMat evecMat = cvMat( chldSz, chldSz, CV_32F, &evecData.front() );
    CvMat evalMat = cvMat( chldSz, 1, CV_32F, &evalData.front() );
    CvMat normalValueMat = cvMat( chldSz, 1, CV_32F, &rnd.front() );

    int sizes[] = { 1, 1 };

    CvRandState rng_state;
    cvRandInit( &rng_state, 1.0f, 0.0f, seed, CV_RAND_NORMAL );


	PNL_CHECK_IS_NULL_POINTER(evidence);
	if( ! evidence->IsNodeObserved( pDomain[nnodes-1] ) )
	{
	    evidence->MakeNodeObserved( pDomain[nnodes-1] );
	}

	for ( i = 0; i < discrParents.size(); i++ )
	{
	    if( !evidence->IsNodeObserved( pDomain[discrParents[i]] ) )
	    {
		PNL_THROW(CAlgorithmicException, "all parents must be observed")
	    }
	    discrParVls[i] = evidence->GetValue( pDomain[discrParents[i]] )->GetInt();
	}
	matrixCov =  static_cast<C2DNumericDenseMatrix<float>*>
	    ( m_CorrespDistribFun->GetMatrix( matCovariance, -1,
	    &discrParVls.front() ) );

	matrixMean = C2DNumericDenseMatrix<float>::Copy(
	    static_cast<C2DNumericDenseMatrix<float>*>
	    (m_CorrespDistribFun->GetMatrix(matMean, -1,
	    &discrParVls.front() ) ) );

	for( i = 0; i < contParents.size(); i++ )
	{
	    matrixWeight = static_cast<C2DNumericDenseMatrix<float>*>
		(m_CorrespDistribFun->GetMatrix( matWeights, i,
		&discrParVls.front() ));


	    if( ! evidence->IsNodeObserved( pDomain[contParents[i]] ) )
	    {
		PNL_THROW(CAlgorithmicException, "all parents must be observed")
	    }

	    int ndims;
	    const int *ranges;
	    matrixWeight->GetRanges( &ndims, &ranges );
	    sizes[0] = ranges[1];
	    !! wrong - prntValue = C2DNumericDenseMatrix<float>::Create( sizes,
		(const float*)( evidence->GetValue( pDomain[contParents[i]] ) ) );

	    tmpMatrix = pnlMultiply( matrixWeight, prntValue, 0 );
	    matrixMean->CombineInSelf( tmpMatrix );
	    delete tmpMatrix;
	    delete prntValue;

	}

	int covlength;
	const float *covdata;
	matrixCov->GetRawData( &covlength, &covdata );

	if( chldSz > 1)
	{
	    memcpy( &covData.front(), covdata, chldSz*chldSz*sizeof(float) );
	    cvEigenVV( &covMat, &evecMat, &evalMat, FLT_EPSILON );
	}
	else
	{
	    evecData[0] = 1.0f;
	    evalData[0] = covdata[0];
	}

	cvRand( &rng_state, &normalValueMat );

	for( i = 0; i < chldSz; i++)
	{
	    evalData[i] = float( sqrt( evalData[i] ) )*rnd[i];
	}

	floatVector prodData( chldSz );
	CvMat prodMat = cvMat( chldSz, 1, CV_32F, &prodData.front() );
	cvMatMulAdd( &evecMat, &evalMat, NULL, &prodMat );

	sizes[0] = chldSz;
	tmpMatrix = C2DNumericDenseMatrix<float>::Create(sizes, &prodData.front());
	tmpMatrix->CombineInSelf( matrixMean );

	int length;
	const float *data;
	tmpMatrix->GetRawData( &length, &data );
	if( ! evidence->IsNodeObserved( pDomain[nnodes-1] ) )
	{
	    evidence->MakeNodeObserved( pDomain[nnodes-1] );
	}
	! wrong memcpy( evidence->GetValue( pDomain[nnodes - 1] ), data,
	    sizeof(float)/sizeof(unsigned char)*length );

	delete tmpMatrix;
	delete matrixMean;


}

*/


CPotential* CGaussianCPD::ConvertStatisticToPot(int numOfSamples) const
{

    PNL_CHECK_LEFT_BORDER( numOfSamples, 0);

    C2DNumericDenseMatrix<float>* pLearnMatrixMean = static_cast<C2DNumericDenseMatrix<float> *>
	(this->GetDistribFun()->GetStatisticalMatrix(stMatMu));
    C2DNumericDenseMatrix<float>* pLearnMatrixCov = static_cast<C2DNumericDenseMatrix<float> *>
	(this->GetDistribFun()->GetStatisticalMatrix(stMatSigma));
    PNL_CHECK_IS_NULL_POINTER( pLearnMatrixMean );
    PNL_CHECK_IS_NULL_POINTER( pLearnMatrixCov );
    CPotential *pPot = this->ConvertToPotential();

    C2DNumericDenseMatrix<float> *pMatrixMean = static_cast<C2DNumericDenseMatrix<float> *>
	(this->GetDistribFun()->GetMatrix(matMean));
    C2DNumericDenseMatrix<float> *pMatrixCov = static_cast<C2DNumericDenseMatrix<float> *>
	(this->GetDistribFun()->GetMatrix(matCovariance));


    int numOfVls = pLearnMatrixMean->GetRawDataLength();
    int indexes[] ={ 0, 0 };

    float value;

    indexes[1] = 0;
    for( indexes[0] = 0; indexes[0] < numOfVls; indexes[0]++)
    {
	value = pLearnMatrixMean->GetElementByIndexes(indexes)/numOfSamples;
	pMatrixMean->SetElementByIndexes(value, indexes);
    }

    C2DNumericDenseMatrix<float>* matrixMeanTr = pMatrixMean->Transpose();
    C2DNumericDenseMatrix<float>* meanSquare = pnlMultiply( pMatrixMean, matrixMeanTr, 0 );

    int i, j;
    for( i = 0 ; i < numOfVls; i++ )
    {
	for(j = i; j < numOfVls; j++)
	{
	    indexes[0] = i;
	    indexes[1] = j;
	    value = pLearnMatrixCov->GetElementByIndexes(indexes)/numOfSamples;
	    value -= meanSquare->GetElementByIndexes(indexes);
	    pMatrixCov->SetElementByIndexes(value, indexes);

	    indexes[0] = j;
	    indexes[1] = i;
	    pMatrixCov->SetElementByIndexes(value, indexes);
	}
    }

    delete matrixMeanTr;
    delete meanSquare;
    return pPot;
}

float CGaussianCPD::GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes  ) const
{
    int asLog = 1;

    const CModelDomain *pMd = GetModelDomain();
    intVector domain;
    GetDomain(&domain);

    intVector contDomain;

    if(!pShrInfRes )
    {
	PNL_CHECK_IS_NULL_POINTER(pEv);

	if( pMd != pEv->GetModelDomain() )
	{
	    PNL_THROW(CBadArg, "model domains");
	}
	const CGaussianDistribFun *currentDistrib = NULL;
	if( this->GetDistribFun()->GetDistributionType() == dtCondGaussian )
	{
	    const CCondGaussianDistribFun *tmpDistr = (static_cast<CCondGaussianDistribFun*>
		(this->GetDistribFun()));

	    intVector discrParentIndices;
	    tmpDistr->GetDiscreteParentsIndices( &discrParentIndices );

	    intVector contParentIndices;
	    tmpDistr->GetContinuousParentsIndices( &contParentIndices );

	    intVector discrParentVls(discrParentIndices.size());

	    int i;
	    for ( i = 0; i < discrParentIndices.size(); i++ )
	    {
		int node = m_Domain[discrParentIndices[i]];
		if( !pEv->IsNodeObserved( node ) )
		{
		    PNL_THROW(CAlgorithmicException, "all parents must be observed")
		}
		discrParentVls[i] = pEv->GetValue( node )->GetInt();
	    }

	    currentDistrib = tmpDistr->GetDistribution(&discrParentVls.front());

	    contDomain.resize(contParentIndices.size()+1);
	    for ( i = 0; i < contParentIndices.size(); i++ )
	    {
		contDomain[i] = m_Domain[contParentIndices[i]];
	    }
	    contDomain[i] = domain.back();
	}
	else
	{
	    currentDistrib = static_cast<const CGaussianDistribFun*>(GetDistribFun());
	    contDomain = domain;
	}

	intVector ndsSz(contDomain.size());

	std::vector<C2DNumericDenseMatrix<float> *> pValMat(contDomain.size(), NULL);
	std::vector<C2DNumericDenseMatrix<float>*>::iterator pMatIt= pValMat.begin();

	floatVector data;
	intVector contDomInd(contDomain.size());

        intVector::iterator it = contDomain.begin();
	int j, i = 0;
	for( ; it != contDomain.end(); it++, pMatIt++, i++ )
	{
	    contDomInd[i] = i;
	    int ndSz = pMd->GetNumVlsForNode(*it);
	    data.resize(ndSz);
	    if( pEv->IsNodeObserved(*it) )
	    {
		for(j = ndSz; --j >= 0;)
		{
		    data[j] = pEv->GetValue(*it)[j].GetFlt();
		}

		int sizes[] ={ndSz, 1};
		*pMatIt = C2DNumericDenseMatrix<float>::Create( sizes, &data.front());
	    }
	    else
	    {
		PNL_THROW(CAlgorithmicException, "inference results");
	    }
	}

	float logLik = static_cast<float>(currentDistrib->
	    ComputeProbability( pValMat.back(), asLog, contDomain.size()-1,
	    &contDomInd.front(), &pValMat.front() ));
	for( pMatIt = pValMat.begin(); pMatIt != pValMat.end(); pMatIt++)
	{
	    delete (*pMatIt);
	}

	return logLik;

    }
    else
    {
	PNL_THROW(CNotImplemented, "not implemented");
	return 0.f;
    }
}

#ifdef PAR_PNL
void CGaussianCPD::UpdateStatisticsML(CFactor *pPot)
{
    PNL_THROW(CNotImplemented, 
        "UpdateStatisticsML for CGaussianCPD not implemented yet");
};
#endif // PAR_OMP

#ifdef PNL_RTTI
const CPNLType CGaussianCPD::m_TypeInfo = CPNLType("CGaussianCPD", &(CCPD::m_TypeInfo));

#endif
