/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlTabularPotential.cpp                                     //
//                                                                         //
//  Purpose:   CTabularPotential class member functions implementation     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <math.h>

#include "pnlConfig.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlInferenceEngine.hpp"
#include "pnlRng.hpp"

PNL_USING

CTabularPotential* CTabularPotential::Create( CModelDomain* pMD, const intVector& domain,
					     CMatrix<float>* matIn )
{
    CTabularPotential *pPot =  CTabularPotential::Create( &domain.front(), domain.size(), pMD );
    if( matIn )
    {
	pPot->AttachMatrix(matIn, matTable);
    }
    return pPot;
}


CTabularPotential* CTabularPotential::Create( const intVector& domain,
                                             CModelDomain* pMD,
                                             const float* data,
                                             const intVector& obsIndices )
{
    return CTabularPotential::Create( &domain.front(), domain.size(), pMD,
        data, obsIndices );
}
CTabularPotential* CTabularPotential::CreateUnitFunctionDistribution( 
                           const intVector& domain, CModelDomain* pMD,
                           int asDense, const intVector& obsIndices )
{
    return CTabularPotential::CreateUnitFunctionDistribution( &domain.front(),
        domain.size(), pMD, asDense, obsIndices );
}


CTabularPotential* CTabularPotential::CreateUnitFunctionDistribution( 
                         const int *domain, int nNodes, CModelDomain* pMD,
                         int asDense, const intVector& obsIndices )
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );
    
    CTabularPotential* resPot = CTabularPotential::Create( domain, nNodes,
        pMD, NULL, obsIndices );
    const pConstNodeTypeVector* ntVec = resPot->GetArgType();
    CTabularDistribFun* UniData = 
        CTabularDistribFun::CreateUnitFunctionDistribution( nNodes,
        &ntVec->front(), asDense);
    delete resPot->m_CorrespDistribFun;
    resPot->m_CorrespDistribFun = UniData;
    return resPot;
}

CTabularPotential* CTabularPotential::Copy( 
                                           const CTabularPotential *pTabPotential )
{
    PNL_CHECK_IS_NULL_POINTER( pTabPotential );
    
    CTabularPotential *retPot = new CTabularPotential( *pTabPotential );
    PNL_CHECK_IF_MEMORY_ALLOCATED( retPot );
    return retPot;
}


CTabularPotential* CTabularPotential::Create( const int *domain, int nNodes,
                                             CModelDomain* pMD,
                                             const float* data,
                                             const intVector& obsIndices )
{
    PNL_CHECK_IS_NULL_POINTER(domain);
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );

    CTabularPotential *pNewParam = new CTabularPotential( domain, nNodes,
        pMD, obsIndices );
    PNL_CHECK_IF_MEMORY_ALLOCATED( pNewParam );
    if( data )
    {
        pNewParam->AllocMatrix( data, matTable );
    }
    return pNewParam;
}

CFactor* CTabularPotential::CloneWithSharedMatrices() 
{
    CTabularPotential* resPot = new CTabularPotential(this);
    PNL_CHECK_IF_MEMORY_ALLOCATED(resPot);

    return resPot;
}

CFactor* CTabularPotential::Clone() const
{
    const CTabularPotential* self = this;
    CTabularPotential* res = CTabularPotential::Copy(self);
    return res;
}

CTabularPotential::CTabularPotential( const CTabularPotential &pTabPotential )
:CPotential( dtTabular, ftPotential, &pTabPotential.m_Domain.front(), 
            pTabPotential.m_Domain.size(), pTabPotential.GetModelDomain(),
            pTabPotential.m_obsPositions )
{
    //m_CorrespDistribFun = pTabPotential.m_CorrespDistribFun->CloneDistribFun();
	delete m_CorrespDistribFun;
    m_CorrespDistribFun = CTabularDistribFun::Copy(
        static_cast<CTabularDistribFun*>(pTabPotential.m_CorrespDistribFun));
}

CTabularPotential:: CTabularPotential(const int *domain, int nNodes,
                                      CModelDomain* pMD,
                                      const intVector& obsIndices):
CPotential(dtTabular, ftPotential, domain, nNodes, pMD, obsIndices)
{
}

CTabularPotential::CTabularPotential(const CTabularPotential* pTabPotential )
                    :CPotential(pTabPotential)
{
}

float CTabularPotential::ProcessingStatisticalData(int numberOfEvidences)
{
    if( !m_CorrespDistribFun )
    {
        PNL_THROW( CNULLPointer, "no corresponding data" )//no corresp Data
    }
    
    CNumericDenseMatrix<float> *m_pMatrix = static_cast<
        CNumericDenseMatrix<float>*>(m_CorrespDistribFun->GetMatrix(matTable));
    CNumericDenseMatrix<float> *m_pLearnMatrix = static_cast<
        CNumericDenseMatrix<float>*>(m_CorrespDistribFun->GetStatisticalMatrix(stMatTable));
    if( !m_pMatrix )
    {
        PNL_THROW( CNULLPointer, "no corresponding matrix" )
    }
    if( !m_pLearnMatrix )
    {
        PNL_THROW( CNULLPointer, "no corresponding learn matrix" )
    }
    float loglik = 0.0f;
    int DomainSize;
    const int *NodeSizes;
    m_pMatrix->GetRanges(&DomainSize,&NodeSizes);
    
    int prodRanges = 1;
    int i;

    for( i = 0; i < DomainSize; i++)
    {
        prodRanges *= NodeSizes[i];
    }
    
    float value = 0.0f;
				
    for( i = 0; i < prodRanges; i++ )
    {
        
        value = static_cast<CNumericDenseMatrix<float>*>(m_pLearnMatrix)->
            GetElementByOffset(i) ;
        
        loglik += value * 
            float( ( value < FLT_EPSILON ) ? -FLT_MAX : log( value ) );
    }
    
    return loglik;
}

void CTabularPotential::UpdateStatisticsEM(const CPotential *pMargPot, const CEvidence *pEvidence)
{
    
    PNL_CHECK_IS_NULL_POINTER(pMargPot);
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

void CTabularPotential::UpdateStatisticsML(const pConstEvidenceVector& evidencesIn)
{
    UpdateStatisticsML( &evidencesIn.front(), evidencesIn.size() );
}

void CTabularPotential::UpdateStatisticsML(const CEvidence* const* pEvidences,
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

void CTabularPotential::Dump() const
{
    int i;
    dump()<<"I'm a factor of "<<m_Domain.size()
        <<" nodes. My Distribution type is Tabular.\n";
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
        dump()<<"I'm a uniform distribution.\n";
        return;
    }
    CMatrix<float>* mat = m_CorrespDistribFun->GetMatrix(matTable);
    CNumericDenseMatrix<float>* myMatrix = static_cast<
        CNumericDenseMatrix<float>*>(mat->ConvertToDense());
    dump()<<"My matrix is:\n";
    const floatVector *myVector = (myMatrix)->GetVector();
    for( i = 0; i < myVector->size(); i++ )
    {
        dump()<<(*myVector)[i]<<" ";
    }
    dump()<<"\n";
    delete myMatrix;*/
    dump().flush();
    
}

/*
void CTabularPotential::
GenerateSample( CEvidence* evidence, unsigned int seed, int maximize ) const
{
    
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
                
            }
        }
    }
    
    if( nonObsNodes.size() )
    {
        CPotential *pMargPot = NULL;
        const CMatrix<float> *pShrMatrix;  
        if( nonObsNodes.size() != nnodes)
        {
            
            
            if( shrinkIsNeed )
            {
                CPotential *pShrPot = this->ShrinkObservedNodes( evidence );
                pMargPot = pShrPot->Marginalize( nonObsNodes );
                delete pShrPot;
            }
            else
            {
                pMargPot = this->Marginalize( nonObsNodes );
            }
            
            pShrMatrix = pMargPot->GetMatrix(matTable);
            
        }
        else
        {
            pShrMatrix = this->GetMatrix(matTable);
        }
        
        
        float rnd;
        CvMat uniValueMat = cvMat( 1, 1, CV_32F, &rnd );
        
        
        CMatrixIterator<float>* matrixIter = pShrMatrix->InitIterator();
        CvRandState rng_state;
        cvRandInit( &rng_state, 0.0f, pShrMatrix->SumAll(0), seed, CV_RAND_UNI );
        cvRand( &rng_state, &uniValueMat );     
        float val = 0.0f;
        for( ; pShrMatrix->IsValueHere( matrixIter ); pShrMatrix->Next( matrixIter ) )
        {   
            val += *pShrMatrix->Value( matrixIter );
            if( rnd <= val)
            {
                break;
            }
        } 
        
        if( !val )
        {
            PNL_THROW(CAlgorithmicException, "bad distribution function" );
        }
        intVector indices;
        
        pShrMatrix->Index( matrixIter, &indices );
        
        for( i = 0; i < nonObsNodes.size(); i++ )
        {
            
            if(  !evidence->IsNodeObserved( nonObsNodes[i] ) )
            {
                int nodeValue = indices[i];

                evidence->MakeNodeObserved( nonObsNodes[i] );
                evidence->GetValue(nonObsNodes[i])->SetInt(nodeValue);
            }
        }
        
        delete matrixIter;
        delete pMargPot;
    }
}*/


void CTabularPotential::
GenerateSample( CEvidence* evidence, int maximize ) const
{
    
    
    CPotential *pShrPot = ShrinkObservedNodes(evidence);
    
    if( pShrPot->GetDistributionType() != dtScalar )
    {
        intVector obsPos;
        pShrPot->GetObsPositions( &obsPos );
        
        intVector domain;
        GetDomain( &domain );
        
        intVector nonObsDom;
        
        if(obsPos.empty())
        {
            nonObsDom.assign( domain.begin(), domain.end() );
        }
        else
        {
            
	    int i;
            for( i = 0; i < obsPos.size(); i++ )
            {
                domain[obsPos[i]] = -1;
            }

            nonObsDom.reserve(domain.size());
	    intVector::iterator it = domain.begin();
            for( ; it != domain.end(); it++ )
            {
                if( *it >= 0 )
                {
                    nonObsDom.push_back(*it);
                }
            }
        }
        if( maximize )
        {
            CEvidence* pEvMPE;
            if( obsPos.size() == 0 )
            {
                pEvMPE = pShrPot->GetMPE();
                
            }
            else
            {
                CPotential *pMargPot = pShrPot->Marginalize(nonObsDom, 1);
                pEvMPE = pMargPot->GetMPE();
                
                delete pMargPot;
            }
            
            int i;
            for( i = 0; i < nonObsDom.size(); i++ )
            {
                *(evidence->GetValue( nonObsDom[i] )) = *(pEvMPE->GetValueBySerialNumber(i));
            } 
            
            evidence->ToggleNodeState( nonObsDom );
            
            delete pEvMPE;
            
        }
        else
        {         
            const CMatrix<float> *pShrMatrix;          
            
            CPotential *pMargPot = NULL;
            
            if( obsPos.size() == 0 )
            {
                //pShrPot->Normalize();
                pShrMatrix = pShrPot->GetDistribFun()->GetMatrix(matTable);
                
            }
            else
            {
                pMargPot = pShrPot->Marginalize(nonObsDom);
                //pMargPot->Normalize();
                pShrMatrix = pMargPot->GetDistribFun()->GetMatrix(matTable);
                
            }
            float sum = pShrMatrix->SumAll(0);
            intVector indices;
	    
	    if( sum != 0.0f )
	    {
		float rnd = pnlRand(0.0f, sum);                  
		float val = 0.0f;
		
		CMatrixIterator<float>* matrixIter = pShrMatrix->InitIterator();
		for( ; pShrMatrix->IsValueHere( matrixIter ); pShrMatrix->Next( matrixIter ) )
		{   
		    val += *pShrMatrix->Value( matrixIter );
		    if( rnd < val)
		    {
			break;
		    }
		} 
		pShrMatrix->Index( matrixIter, &indices );
		delete matrixIter;
	    }
	    else
	    {
		PNL_THROW(CAlgorithmicException, "bad distribution function" );
	   
	    }
	    int i;
	    for( i = 0; i < nonObsDom.size(); i++ )
	    {
		evidence->GetValue(nonObsDom[i])->SetInt(indices[i]);
	    }
	    evidence->ToggleNodeState( nonObsDom );
	            
	    delete pMargPot;
            
            
        }
        delete pShrPot;
        
  }  
}

CPotential* CTabularPotential::ConvertStatisticToPot(int numOfSamples) const
{
    PNL_CHECK_LEFT_BORDER( numOfSamples, 0);
    CMatrix<float>* pLearnMatrix = this->GetDistribFun()->GetStatisticalMatrix(stMatTable);
    /*
    CTabularPotential *pPot = CTabularPotential::Copy(this);
    pPot->AttachMatrix(pLearnMatrix, matTable);
    pPot->Normalize();
    return pPot;
    */
    intVector domain;
    this->GetDomain( &domain);
    CTabularPotential *pPot = CTabularPotential::Create( &domain.front(),
        domain.size(), GetModelDomain() );
    pPot->AttachMatrix(pLearnMatrix->Clone(), matTable);
    pPot->Normalize();
    return pPot;
}

float CTabularPotential::
GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes ) const
{
    PNL_CHECK_IS_NULL_POINTER( pEv || pShrInfRes );
    intVector domain;
    GetDomain(&domain);
    intVector::iterator it = domain.begin();
    if(!pShrInfRes )
    {
        PNL_CHECK_IS_NULL_POINTER(pEv);
        intVector values(domain.size());
        intVector::iterator valIt = values.begin();
        for( ; it != domain.end(); it++, valIt++ )
        {
            if( pEv->IsNodeObserved(*it) )
            {
                *valIt = pEv->GetValue(*it)->GetInt();
            }
            else
            {
                PNL_THROW(CAlgorithmicException, "inference results");
            }
            
        }
        
        float ll = GetMatrix(matTable)->GetElementByIndexes(&values.front());
        //PNL_CHECK_LEFT_BORDER(ll, FLT_EPSILON);
	if(ll < FLT_EPSILON) ll+=FLT_EPSILON;
        return float(log(ll));
    }
    else
    {
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
        CMatrix<float> *pInfMat;
        if( obsPos.size() )
        {
            PNL_CHECK_IS_NULL_POINTER(pEv);
            pExpInfRes = pShrInfRes->ExpandObservedNodes(pEv);
            pInfMat = pExpInfRes->GetMatrix(matTable);
        }
        else
        {
            pInfMat = pShrInfRes->GetMatrix(matTable);
        }
        
        CMatrix<float> *pProbMat = GetMatrix(matTable);
              
        CMatrixIterator<float>* probIter = pProbMat->InitIterator();
        CMatrixIterator<float>* infIter = pInfMat->InitIterator();
        
        
        float logLik = 0.0f;
        for( ; pProbMat->IsValueHere( probIter ); pProbMat->Next(probIter), 
            pInfMat->Next(infIter) )
        {
            float infVal = *( pInfMat->Value( infIter ) );
            if( infVal > FLT_EPSILON )
            {
                float probVal = *( pProbMat->Value( probIter ) );
                PNL_CHECK_LEFT_BORDER(probVal, FLT_EPSILON);
                logLik +=  infVal* float(log(probVal));
            }
        }
        
        delete probIter;
        delete infIter;
        delete pExpInfRes;
        
        return logLik;
    }
}

#ifdef PAR_PNL
void CTabularPotential::UpdateStatisticsML(CFactor *pPot)
{
    //Is pPot correct?
    if (pPot->GetDistributionType() != dtTabular)
        PNL_THROW(CInconsistentType, 
        "Can not use function CTabularPotential::UpdateStatisticsML with wrong distribution type");

    CDistribFun *pDF = pPot->GetDistribFun();
    m_CorrespDistribFun->UpdateStatisticsML( pDF );
};
#endif // PAR_OMP

#ifdef PNL_RTTI
const CPNLType CTabularPotential::m_TypeInfo = CPNLType("CTabularPotential", &(CPotential::m_TypeInfo));

#endif