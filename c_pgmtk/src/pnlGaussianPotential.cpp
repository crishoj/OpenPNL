/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGaussianPotential.cpp                                    //
//                                                                         //
//  Purpose:   CgaussianPotential class member functions implementation    //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlDistribFun.hpp"
#include "pnlInferenceEngine.hpp"
#include "pnlRng.hpp"
#include <math.h>

PNL_BEGIN

CGaussianPotential* 
CGaussianPotential::Create(CModelDomain* pMD, const intVector& domain,
			   int inMoment,
			   const C2DNumericDenseMatrix<float> *mean,
			   const C2DNumericDenseMatrix<float> *cov,
			   float normCoeff,
			   const intVector& obsIndices )
{
    int nel;
    const float *vec = NULL;
    const float *mat = NULL;
    if( mean )
    {
	mean->GetRawData(&nel, &vec);
    }
    if( cov )
    {
	cov->GetRawData(&nel, &mat);
    }
    return Create( &domain.front(), domain.size(), pMD, inMoment,
        vec, mat, normCoeff, obsIndices );
}

CGaussianPotential*
CGaussianPotential::Create( const intVector& domain,
			   CModelDomain* pMD,
			   int inMoment,
			   const floatVector& vec,
			   const floatVector& mat,
			   float normCoeff,
			   const intVector& obsIndices )
{
    const float* vecData = NULL;
    if( vec.size())
    {
	vecData = &vec.front();
    }
    const float* matData = NULL;
    if( mat.size())
    {
	matData = &mat.front();
    }
    return Create( &domain.front(), domain.size(), pMD, inMoment,
        vecData, matData, normCoeff, obsIndices );
}

CGaussianPotential*
CGaussianPotential::CreateDeltaFunction(const intVector& domain,
					CModelDomain* pMD,
					const floatVector& mean ,
					int isInMoment,
					const intVector& obsIndices )
{
    return CreateDeltaFunction( &domain.front(), domain.size(), pMD, 
        &mean.front(), isInMoment, obsIndices );
}

CGaussianPotential* 
CGaussianPotential::CreateDeltaFunction( const intVector& domain,
                                       CModelDomain* pMD,
                                       const C2DNumericDenseMatrix<float>* mean,
				       int isInMoment,
                                       const intVector& obsIndices)
{
    int nel;
    const float * data;
    mean->GetRawData(&nel, &data);
    return CreateDeltaFunction( &domain.front(), domain.size(), pMD, 
        data, isInMoment, obsIndices );
}

CGaussianPotential*
CGaussianPotential::CreateUnitFunctionDistribution( 
    const intVector& domain, CModelDomain* pMD,
    int isInCanonical, const intVector& obsIndices )
{
    return CreateUnitFunctionDistribution( &domain.front(), domain.size(), pMD,
        isInCanonical );
}

CGaussianPotential*
CGaussianPotential::Copy(const CGaussianPotential *pGauPot)
{
    PNL_CHECK_IS_NULL_POINTER( pGauPot );
    
    CGaussianPotential *retPot = new CGaussianPotential( *pGauPot );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retPot );
    return retPot;
}

CGaussianPotential*
CGaussianPotential::Create( const int *domain, int nNodes,
			   CModelDomain* pMD,
			   int isMoment, float const* pVec,
			   float const* pMat, float normCoeff,
			   const intVector& obsIndices )
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    
    CGaussianPotential *pNewParam = new CGaussianPotential(domain, nNodes, 
        pMD, obsIndices);
    PNL_CHECK_IF_MEMORY_ALLOCATED( pNewParam );
    if( isMoment == 1 )
    {
        pNewParam->SetCoefficient( normCoeff, 1-isMoment );
        if( pVec )
        {
            pNewParam->AllocMatrix( pVec, matMean );
        }
        if( pMat )
        {
            pNewParam->AllocMatrix( pMat, matCovariance );
        }
        
    }
    else if( isMoment == 0 )
    {
        pNewParam->SetCoefficient( normCoeff, 1-isMoment );
        if( pVec )
        {
            pNewParam->AllocMatrix( pVec, matH );
        }
        if( pMat )
        {
            pNewParam->AllocMatrix( pMat, matK );
        }
        
    }
    return pNewParam;
}

CGaussianPotential*
CGaussianPotential::CreateDeltaFunction( const int *domain,
					int nNodes, CModelDomain* pMD,
					const float *pMean , int isInMoment,
					const intVector& obsIndices )
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_IS_NULL_POINTER( pMean );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );
    
    CGaussianPotential *resPot = CGaussianPotential::Create( domain, nNodes,
        pMD, isInMoment, NULL, NULL, 0.0f, obsIndices );
    const pConstNodeTypeVector* ntVec = resPot->GetArgType();
    CGaussianDistribFun *DeltaData = 
        CGaussianDistribFun::CreateDeltaDistribution( nNodes, &ntVec->front(),
        pMean, isInMoment );
    delete (resPot->m_CorrespDistribFun);
    resPot->m_CorrespDistribFun = DeltaData;
    return resPot;
}

CGaussianPotential* CGaussianPotential::CreateUnitFunctionDistribution( 
                                                                       const int *domain, int nNodes,
                                                                       CModelDomain* pMD, int isInCanonical,
                                                                       const intVector& obsIndices )
{
   	PNL_CHECK_IS_NULL_POINTER( domain );
        PNL_CHECK_IS_NULL_POINTER( pMD );
        PNL_CHECK_LEFT_BORDER( nNodes, 1 );
        PNL_CHECK_RANGES( isInCanonical, 0, 1 );
        
        CGaussianPotential* resPot = CGaussianPotential::Create( domain, nNodes,
            pMD, 1-isInCanonical, NULL, NULL, 0.0f, obsIndices );
        const pConstNodeTypeVector* ntVec = resPot->GetArgType();
        CGaussianDistribFun* UniData = 
            CGaussianDistribFun::CreateUnitFunctionDistribution( nNodes,
            &ntVec->front(), 1, isInCanonical);
        delete (resPot->m_CorrespDistribFun);
        resPot->m_CorrespDistribFun = UniData;
        return resPot;
}

CFactor* CGaussianPotential::CloneWithSharedMatrices()
{
    CGaussianPotential* resPot = new CGaussianPotential(this);
    PNL_CHECK_IF_MEMORY_ALLOCATED(resPot);
    
    return resPot;  
}

CFactor* CGaussianPotential::Clone() const
{
    const CGaussianPotential* self = this;
    CGaussianPotential* res = CGaussianPotential::Copy(self);
    return res;
}

CGaussianPotential::CGaussianPotential( const CGaussianPotential &pGauPot )
:CPotential( dtGaussian, ftPotential, &pGauPot.m_Domain.front(),
            pGauPot.m_Domain.size(), pGauPot.GetModelDomain(),
            pGauPot.m_obsPositions )
{
    delete m_CorrespDistribFun;
    m_CorrespDistribFun = CGaussianDistribFun::Copy(
        static_cast<CGaussianDistribFun*>(pGauPot.m_CorrespDistribFun));
    m_Domain = intVector( pGauPot.m_Domain );	
}


CGaussianPotential:: CGaussianPotential(const int *domain, int nNodes, 
                                        CModelDomain* pMD,
                                        const intVector& obsIndices ):
CPotential( dtGaussian, ftPotential, domain, nNodes, pMD, obsIndices )
{
    
}

CGaussianPotential::CGaussianPotential(const CGaussianPotential* pGauPot):CPotential(pGauPot)
{
}

float CGaussianPotential::ProcessingStatisticalData( int numberOfEvidences) 
{
    return 1.0f;
}

void CGaussianPotential::UpdateStatisticsEM( const CPotential *pMargPot, 
                                           const CEvidence *pEvidence )
{
    intVector obsPos;
    pMargPot->GetObsPositions(&obsPos);
    
    if( obsPos.size() )
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

void CGaussianPotential::UpdateStatisticsML(const pConstEvidenceVector& evidencesIn)
{
    UpdateStatisticsML( &evidencesIn.front(), evidencesIn.size() );
}

void CGaussianPotential::UpdateStatisticsML(const CEvidence* const* pEvidences,
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
    int DomainSize;
    const int *domain;
    GetDomain(&DomainSize, &domain);
    m_CorrespDistribFun->UpdateStatisticsML( pEvidences, EvidenceNumber, domain );
    
}

void CGaussianPotential::Dump() const
{
    int i;
    dump()<<"I'm a factor of "<< m_Domain.size() <<" nodes. My Distribution type is Gaussian.\n";
    dump()<<"My domain is\n";
    for( i = 0; i < m_Domain.size(); i++ )
    {
        dump()<<m_Domain[i]<<" ";
    }
    dump()<<"\n";
    m_CorrespDistribFun->Dump();
/*    int isSpecific = m_CorrespDistribFun->IsDistributionSpecific();
    if( isSpecific == 1 )
    {
        dump()<<"I'm a Uniform distribution, haven't any matrices.\n";
        return;
    }
    if( isSpecific == 2 )
    {
        dump()<<"I'm a delta distribution - have only mean matrix, covariance is zero matrix\n";
        dump()<<"My matrix Mean is:\n";
        CMatrix<float> *matrMean = m_CorrespDistribFun->GetMatrix( matMean );
        const floatVector *myVector = static_cast<CNumericDenseMatrix<float>*>(
            matrMean)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump()<<(*myVector)[i]<<" ";
        }
        dump()<<"\n";
        return;
    }
    if( isSpecific == 3 ) 
    {
        dump()<<"I'm a mixed distribution, result of multiplication by Delta distributions in some dimensions.";
        dump()<<" To see me in valid form you need to marginalize.\n";
        return;
    }
    int isFactor = ((CGaussianDistribFun*)m_CorrespDistribFun)->GetFactorFlag();
    int canonicalFormFlag = ((CGaussianDistribFun*)m_CorrespDistribFun)->GetCanonicalFormFlag();
    int momentFormFlag = ((CGaussianDistribFun*)m_CorrespDistribFun)->GetMomentFormFlag();
    if( momentFormFlag )
    {
        dump()<<"I have valid moment form\n";
        float norm_coeff = ((CGaussianDistribFun*)m_CorrespDistribFun)->GetCoefficient(0);
        dump()<<"My normal coefficient is" << norm_coeff<<"\n";
        CMatrix<float> *matrMean = m_CorrespDistribFun->GetMatrix( matMean );
        dump()<<"My matrix Mean is:\n";
        const floatVector *myVector = static_cast<CNumericDenseMatrix<float>*>(
            matrMean)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump()<<(*myVector)[i]<<" ";
        }
        dump()<<"\n";
        CMatrix<float> *matrCov = m_CorrespDistribFun->GetMatrix(matCovariance);
        dump()<<"My matrix Covarience is:\n";
        myVector = static_cast<CNumericDenseMatrix<float>*>(matrCov)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump()<<(*myVector)[i]<<" ";
        }
        dump()<<"\n";
        if( !isFactor )
        {
            int numParents = m_Domain.size() - 1;
            for( int j = 0; j < numParents; j++ )
            {
                CMatrix<float> *matrW = m_CorrespDistribFun->GetMatrix(matWeights, j);
                dump()<<"My matrix Weights from %d parent is:"<<j<<"\n";
                myVector = static_cast<CNumericDenseMatrix<float>*>(matrW)->GetVector();
                for( i = 0; i < myVector->size(); i++ )
                {
                    dump()<<(*myVector)[i]<<" ";
                }
                dump()<<"\n";
            }
        }
    }
    if( canonicalFormFlag )
    {
        dump()<<"I have valid canonical form\n";
        float norm_coeff = ((CGaussianDistribFun*)m_CorrespDistribFun)->GetCoefficient(1);
        dump()<<"My normal coefficient is"<<norm_coeff<<"\n";
        CMatrix<float> *matrH = m_CorrespDistribFun->GetMatrix( matH );
        dump()<<"My matrix H is:\n";
        const floatVector *myVector = static_cast<
            CNumericDenseMatrix<float>*>(matrH)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump()<<(*myVector)[i]<<" ";
        }
        dump()<<"\n";
        CMatrix<float> *matrK = m_CorrespDistribFun->GetMatrix(matK);
        dump()<<"My matrix Covarience is:\n";
        myVector = static_cast<CNumericDenseMatrix<float>*>(matrK)->GetVector();
        for( i = 0; i < myVector->size(); i++ )
        {
            dump()<<(*myVector)[i]<<" ";
        }
        dump()<<"\n";
    }
    if(!(momentFormFlag||canonicalFormFlag) )        
    {
        dump()<<"I haven't any valid form\n";
    }
*/       
    dump().flush();
}

void CGaussianPotential::SetCoefficient( float coeff, int isForCanonical )
{
    EDistributionType dt = m_CorrespDistribFun->GetDistributionType();
    if( dt == dtTabular )
    {
        PNL_THROW( CInvalidOperation, 
            "normal coefficient is only for non Tabular" )
    }
    else
    {
        static_cast<CGaussianDistribFun*>(m_CorrespDistribFun)->SetCoefficient( coeff, 
            isForCanonical );
    }
}

float CGaussianPotential::GetCoefficient( int forCanonical )
{
    if( m_CorrespDistribFun->GetDistributionType() != dtGaussian )
    {
        PNL_THROW( CInvalidOperation, 
            "normal coefficient is only for non Tabular" )
    }
    else
    {
        return static_cast<CGaussianDistribFun*>(m_CorrespDistribFun)->GetCoefficient(
            forCanonical );
    }
}

void CGaussianPotential::
GenerateSample( CEvidence* evidence, int maximize ) const
{
    
    PNL_CHECK_IS_NULL_POINTER( evidence );
	
    int typeSpec = IsDistributionSpecific();
    bool dDeltaFunction = false;
    switch(typeSpec)
    {
    case 2:
		dDeltaFunction = true;
		break;
    case 0:
		break;
    default:
		PNL_THROW(CAlgorithmicException, "bad distribution for sampling")
    }
    
    
    
    int nnodes;
    const int *domain;
    this->GetDomain( &nnodes, &domain );
    
    intVector nonObsNodes;
    intVector nodeSz;
    
    intVector obsPos;
    this->GetObsPositions(&obsPos);
    
    intVector obsFlags(nnodes, 0);
    
    int i;
    for( i = 0; i < obsPos.size(); i++ )
    {
        obsFlags[obsPos[i]] = 1;
    }
    bool shrinkIsNeed = false;
    int sz = 0;
    
    
    for( i = 0; i < nnodes; i++ )
    {
        if( !obsFlags[i] )
        {
            if( evidence->IsNodeObserved( domain[i] )  )
            {
                shrinkIsNeed = true;
            }
            else
            {
                nonObsNodes.push_back( domain[i] );
                nodeSz.push_back( this->GetModelDomain()->GetVariableType(domain[i])->GetNodeSize() );
                sz += nodeSz.back();
            }
        }
    }
    if( nonObsNodes.size() )
    {
        // PNL_THROW(CAlgorithmicException, "all nodes in domain is already observed");
        
        floatVector covData;
        floatVector meanData;
        
        if(int(nonObsNodes.size()) != nnodes)
        {
            CPotential *pMargPot;
            
            if( shrinkIsNeed )
            {
                CPotential *pShrPot = this->ShrinkObservedNodes( evidence );
                pMargPot = pShrPot->Marginalize( nonObsNodes, maximize );
                delete pShrPot;
            }
            else
            {
                pMargPot = this->Marginalize( nonObsNodes, maximize );
            }
            
            static_cast<CGaussianDistribFun*>(pMargPot->GetDistribFun())->UpdateMomentForm();
            
            covData = *( static_cast<C2DNumericDenseMatrix<float>*>
                ( pMargPot->GetDistribFun()->GetMatrix( matCovariance ) ) )->GetVector();
            
            meanData = *( static_cast<C2DNumericDenseMatrix<float>*>
                (pMargPot->GetDistribFun()->GetMatrix( matMean ) ) )->GetVector();
            
            delete pMargPot;
        }
        else
        {
            static_cast<CGaussianDistribFun*>(this->GetDistribFun())->UpdateMomentForm();
            
            covData = *( static_cast<C2DNumericDenseMatrix<float>*>
                ( this->GetDistribFun()->GetMatrix( matCovariance ) ) )
                ->GetVector();
            
            meanData = *( static_cast<C2DNumericDenseMatrix<float>*>
                (this->GetDistribFun()->GetMatrix( matMean ) ) )
                ->GetVector();
        }
        
        if(!maximize && !dDeltaFunction )
        {
            
            if( sz > 1)
            {
                doubleVector::iterator dataIt;
				doubleVector doubleMean;
				doubleMean.resize(sz);
				doubleVector doubleCov;
				doubleCov.resize(sz*sz);
				
				for( i = 0; i < sz; i++ )
				{
					doubleMean[i] = meanData[i];
				}
				for( i = 0; i < sz*sz; i++ )
				{
					doubleCov[i] = covData[i];
				}
				
				doubleVector rndVls;
                pnlRandNormal( &rndVls, doubleMean, doubleCov );
                
                dataIt = rndVls.begin();
                int j = 0;
                for( i = 0; i < nonObsNodes.size(); i++ )
                { 
                    for( j = 0; j < nodeSz[i]; j++, dataIt++ )
                    {
                        evidence->GetValue( nonObsNodes[i] )[j].SetFlt((float)*dataIt);
                    }
                    evidence->MakeNodeObserved( nonObsNodes[i] );
                }
                
            }
            else
            {
                float val = pnlRandNormal( meanData[0], covData[0] );
                evidence->GetValue(nonObsNodes[0])->SetFlt(val);
                evidence->MakeNodeObserved( nonObsNodes[0] );
            }
        }
        else
        {
            int i,j;
            float *dataIt = &meanData.front();
            for( i = 0; i < nonObsNodes.size(); i++ )
            { 
                for( j = 0; j < nodeSz[i]; j++, dataIt++ )
                {
                    evidence->GetValue( nonObsNodes[i] )[j].SetFlt(*dataIt);
                }
                
            }
            evidence->ToggleNodeState(nonObsNodes);
            
        }
    }
    
}



CPotential* CGaussianPotential::ConvertStatisticToPot(int numOfSamples) const
{
    
    PNL_CHECK_LEFT_BORDER( numOfSamples, 0);
    
    C2DNumericDenseMatrix<float>* pLearnMatrixMean = static_cast<C2DNumericDenseMatrix<float> *>
        (this->GetDistribFun()->GetStatisticalMatrix(stMatMu));
    C2DNumericDenseMatrix<float>* pLearnMatrixCov = static_cast<C2DNumericDenseMatrix<float> *>
        (this->GetDistribFun()->GetStatisticalMatrix(stMatSigma));
    PNL_CHECK_IS_NULL_POINTER( pLearnMatrixMean );
    PNL_CHECK_IS_NULL_POINTER( pLearnMatrixCov );
    
    
    intVector domain;
    this->GetDomain( &domain);
    CGaussianPotential *pPot = CGaussianPotential::
        Create( &domain.front(), domain.size(), GetModelDomain() );
    
    
    C2DNumericDenseMatrix<float> *pMatrixMean = 
        static_cast< C2DNumericDenseMatrix<float>* >( pLearnMatrixMean->Clone() );
    C2DNumericDenseMatrix<float> *pMatrixCov = 
        static_cast< C2DNumericDenseMatrix<float>* >( pLearnMatrixCov->Clone() );
    
    
    int numOfVls = pLearnMatrixMean->GetRawDataLength();
    int indexes[] ={ 0, 0 };
    
    float value;
    
    indexes[1] = 0;
    for( indexes[0] = 0; indexes[0] < numOfVls; indexes[0]++)
    {
        value = pMatrixMean->GetElementByIndexes(indexes)/numOfSamples;
        pMatrixMean->SetElementByIndexes(value, indexes);
        
    }
    pPot->AttachMatrix( pMatrixMean, matMean );      
    
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
            indexes[0]^=indexes[1]^=indexes[0]^=indexes[1];
            pMatrixCov->SetElementByIndexes(value, indexes);
        }
    }
    
    pPot->AttachMatrix( pMatrixCov, matCovariance );
    
    delete matrixMeanTr;
    delete meanSquare;
    return pPot;
    
}

float CGaussianPotential::GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes  ) const
{
    
    const C2DNumericDenseMatrix<float>* pMatVariable;
    int asLog = 1;
    
    PNL_CHECK_IS_NULL_POINTER( pShrInfRes );
    PNL_CHECK_IS_NULL_POINTER( pEv);
    
    const CModelDomain *pMd = GetModelDomain();
    intVector domain;
    GetDomain(&domain);
    
    intVector::iterator it = domain.begin();
    if(!pShrInfRes )
    {
        PNL_CHECK_IS_NULL_POINTER(pEv);
        
        if( pMd != pEv->GetModelDomain() )
        {
            PNL_THROW(CBadArg, "model domains");
        }
        
        intVector ndsSz(domain.size());
        int nVls = 0;
        
        intVector::iterator ndsSzIt = ndsSz.begin();
        for( ; it != domain.end(); it++, ndsSzIt++ )
        {
            *ndsSzIt = pMd->GetNumVlsForNode(*it);
            nVls += *ndsSzIt;
        }
        
        floatVector data(nVls);
        
        float* dataIt = &data.front();
        ndsSzIt = ndsSz.begin();
        for( ; it != domain.end(); it++, ndsSzIt++ )
        {
            if( pEv->IsNodeObserved(*it) )
            {
                for(int j = *ndsSzIt; --j >= 0;)
                {
                    dataIt[j] = pEv->GetValue(*it)[j].GetFlt();
                }
                dataIt += *ndsSzIt;
            }
            else
            {
                PNL_THROW(CAlgorithmicException, "inference results");
            }
            
        } 
        int sizes[] ={1, nVls};
        pMatVariable = C2DNumericDenseMatrix<float>::Create( sizes, &data.front());
        float logLik = float(  static_cast<CGaussianDistribFun*>(GetDistribFun())->
            ComputeProbability( pMatVariable, asLog, 0, NULL, NULL ));
        return logLik;
        
    }
    else
    {
        PNL_THROW(CNotImplemented,"not emplemented")
            intVector infDom;
        pShrInfRes->GetDomain( &infDom );
        if( !std::equal( domain.begin(), domain.end(), infDom.begin() ) )
        {
            PNL_THROW( CBadArg, "domains must be equal" );
        }
        if( GetModelDomain() != pShrInfRes->GetModelDomain() )
        {
            PNL_THROW(CBadArg, "model domains");
        }
        
        intVector obsPos;
        pShrInfRes->GetObsPositions(&obsPos);
        
        CPotential* pExpInfRes = NULL;
        const C2DNumericDenseMatrix<float> *pInfMatMean = NULL;
        const C2DNumericDenseMatrix<float> *pInfMatCov = NULL;
        
        if( obsPos.size() )
        {
            PNL_CHECK_IS_NULL_POINTER(pEv);
            pExpInfRes = pShrInfRes->ExpandObservedNodes(pEv);
            
            pInfMatMean = static_cast< const C2DNumericDenseMatrix<float> *>
                (pExpInfRes->GetMatrix(matMean));
            pInfMatCov = static_cast< const C2DNumericDenseMatrix<float> *>
                (pExpInfRes->GetMatrix(matCovariance));
        }
        else
        {
            pInfMatMean = static_cast< const C2DNumericDenseMatrix<float> *>
                (pShrInfRes->GetMatrix(matMean));
            
            pInfMatCov = static_cast< const C2DNumericDenseMatrix<float> *>
                (pShrInfRes->GetMatrix(matCovariance));
        }
        
        float logLik = float( static_cast<CGaussianDistribFun*>(GetDistribFun())->
            ComputeProbability( pMatVariable, asLog, 0, NULL, NULL ));
        
        delete pExpInfRes;
        
        return logLik;
    }
}

#ifdef PAR_PNL
void CGaussianPotential::UpdateStatisticsML(CFactor *pPot)
{
    //Is pPot correct?
    if (pPot->GetDistributionType() != dtGaussian)
        PNL_THROW(CInconsistentType, 
        "Can not use function CGaussianPotential::UpdateStatisticsML with wrong distribution type");

    CDistribFun *pDF = pPot->GetDistribFun();
    m_CorrespDistribFun->UpdateStatisticsML( pDF );
}
#endif // PAR_OMP

#ifdef PNL_RTTI
const CPNLType CGaussianPotential::m_TypeInfo = CPNLType("CGaussianPotential", &(CPotential::m_TypeInfo));

#endif

PNL_END
