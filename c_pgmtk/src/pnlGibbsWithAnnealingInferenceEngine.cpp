/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGibbsWithAnnealingInferenceEngine.cpp                    //
//                                                                         //
//  Purpose:   CGibbsWithAnnealingInfEngine class member functions         //
//             implementation                                              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "pnlConfig.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlCondGaussianDistribFun.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "pnlMNet.hpp"
#include "pnlGibbsWithAnnealingInferenceEngine.hpp"
#include "pnlException.hpp"

PNL_USING

CGibbsWithAnnealingInfEngine * CGibbsWithAnnealingInfEngine::Create( 
								    const CStaticGraphicalModel *pGraphicalModel )
{
    if( !pGraphicalModel )
    {
	PNL_THROW( CNULLPointer, "graphical model" );
	return NULL;
    }
    else
    {
	CGibbsWithAnnealingInfEngine* newInf = new CGibbsWithAnnealingInfEngine( pGraphicalModel );
	return newInf;
    }
}


CGibbsWithAnnealingInfEngine::CGibbsWithAnnealingInfEngine( const CStaticGraphicalModel
							   *pGraphicalModel )
							   :CSamplingInfEngine( pGraphicalModel ),
							   m_pBestEvidence(NULL),
							   m_constC(1.0f),
							   m_constS(1.0f),
							   m_bAdapt(false),
							   m_counter(0),
							   m_currentTemp(1.0f)

							   
							   
							   
							   
							   
							   
{
    
}

CGibbsWithAnnealingInfEngine::~CGibbsWithAnnealingInfEngine()
{
    DestroyCurrentFactors();
    delete m_pEvidenceMPE;
    m_pEvidenceMPE = NULL;
    
}


void CGibbsWithAnnealingInfEngine::
EnterEvidence( const CEvidence *pEvidenceIn, int maximize, int sumOnMixtureNode )
{
    
    
    PNL_CHECK_IS_NULL_POINTER(pEvidenceIn);
    m_pEvidence = pEvidenceIn;
    DestroyCurrentFactors();
    DestroyCurrentEvidences();
    
    if(GetModel()->GetModelType() == mtBNet)
    {
        static_cast< const CBNet* >(m_pGraphicalModel)->
            GenerateSamples( GetCurrentEvidences(), GetNumStreams(), pEvidenceIn );
    }
    else
    {
        static_cast< const CMNet* >(m_pGraphicalModel)->
            GenerateSamples( GetCurrentEvidences(), GetNumStreams(), pEvidenceIn );
	
    }
    
    Initialization();
    boolVector sampleIsNeed;
    FindCurrentNdsForSampling( &sampleIsNeed );
    SetSamplingNdsFlags(sampleIsNeed);

    Sampling( 0, GetMaxTime() );
}

void CGibbsWithAnnealingInfEngine::DestroyCurrentFactors()
{
    
    pFactorVector * pFactors = GetCurrentFactors();
    int i;
    for( i = 0; i < pFactors->size(); i++ )
    {
        delete (*pFactors)[i];
    }
    pFactors->clear();

}

const CEvidence* CGibbsWithAnnealingInfEngine::GetMPE() const
{
    
    PNL_CHECK_IS_NULL_POINTER( m_pEvidenceMPE );
    
    return m_pEvidenceMPE;
}



void CGibbsWithAnnealingInfEngine::Initialization()
{
    
    int nFactors = m_pGraphicalModel->GetNumberOfFactors();
    pFactorVector* currentFactors=GetCurrentFactors();
    currentFactors->resize(nFactors);
    int i;
    for( i = 0; i < nFactors; i++ )
    {
        (*currentFactors)[i] = GetModel()->GetFactor(i)->Clone();
    }
}

void CGibbsWithAnnealingInfEngine::
Sampling( int startTime, int endTime )
{
        
    intVector ndsForSampling;
    GetNdsForSampling( &ndsForSampling );

    
    boolVector sampleIsNeed;
    GetSamplingNdsFlags( &sampleIsNeed );
    
    pEvidencesVector currentEvidences;
    GetCurrentEvidences( &currentEvidences );
    
    
    pFactorVector pFactors;
    GetCurrentFactors( &pFactors );
   

    float logLik = 0.0f;
    if( IsAdapt())
    {
	
	logLik = GetLikelihood(currentEvidences[0]);
	if( !startTime )
	{
	
	const CEvidence *ev = currentEvidences[0];
	m_pBestEvidence = CEvidence::Create( ev, ev->GetNumberObsNodes(), ev->GetAllObsNodes(),  
	    ev->GetModelDomain());
	}
    } 
 
    valueVector vlsOld;
    CEvidence * pCurrentEvidence = NULL;
    
    int t;
    for( t = startTime; t < endTime; t++ )
    {
        float dT = 0.0f;
	
	floatVector llVecCurrent;
	llVecCurrent.assign(pFactors.size(), 0.0f);
	pnlVector<char> doAnnealing(pFactors.size(), false);
	
	int series;
        for( series = 0; series < GetNumStreams(); series++ )
        {
            pCurrentEvidence = currentEvidences[series];
	    
	    if( IsAdapt() )
	    {
		vlsOld.clear();
		pCurrentEvidence->GetRawData(&vlsOld);
	    }
            
            int i = 0;
            for( i = 0; i < ndsForSampling.size(); i++ )
            { 
		if( sampleIsNeed[i])
		{
		    pCurrentEvidence->ToggleNodeStateBySerialNumber(1, &i);
		    bool canBeSample = ConvertingFamilyToPot( ndsForSampling[i], pCurrentEvidence );
		    if(canBeSample)
		    {
			GetPotToSampling(ndsForSampling[i])->GenerateSample( pCurrentEvidence );
		    }
		    else
		    {
			pCurrentEvidence->ToggleNodeStateBySerialNumber(1, &i);
		    }
		}
            }	
	    
        }
        if( t > GetBurnIn() )
	{
	    if( IsAdapt() )
	    {
		float ll = GetLikelihood(pCurrentEvidence);
		
		if( ll > logLik )
		{
		   
		    if( -ll < FLT_EPSILON )
		    {
			dT = FLT_EPSILON;
			doAnnealing.assign(doAnnealing.size(), true);
		    }
		    else
		    {
			if(-logLik < FLT_EPSILON )
			{
			    dT = -FLT_EPSILON;
			    doAnnealing.assign(doAnnealing.size(), true);
			}
			else
			{
			    dT = (ll/logLik);
			}
		    }
		    
		    AnnealingModel( dT, pCurrentEvidence, &doAnnealing.front());
		    
		    valueVector rawVls;
		    pCurrentEvidence->GetRawData(&rawVls);
		    m_pBestEvidence->SetData(rawVls);
		    logLik = ll;
		}
		
	    }
	    else
	    {
		float T1 = Temperature(m_constC, t*m_constS );
		float T0 = GetCurrentTemp();
		float dT = T1/T0;
		SetCurrentTemp(T1);
		
		AnnealingModel( dT );
		
	    }
	    
	}
    }
    
    if( !IsAdapt())
    {
	m_pBestEvidence = BestEvidence( currentEvidences );
    }
    
    
}

CEvidence * CGibbsWithAnnealingInfEngine::
BestEvidence( pEvidencesVector &evidences)
{
    int best = 0;
    
    if( GetModel()->GetModelType() == mtBNet )
    {
        
        float ll = 0.0f;
        const CBNet *pGrModel =
            static_cast<const CBNet*>(GetModel());
        
        const CEvidence *pEv = evidences.front();
        ll = pGrModel->ComputeLogLik(pEv);
       	

        int i;
        for( i = 1; i < evidences.size(); i++ )
        {
            {
                float tmp = pGrModel->ComputeLogLik( evidences[i] );
                if( tmp > ll )
                {
                    best = i;
                    ll = tmp;
                    pEv = evidences[i];
                }
            }
        }
    }
    else
    {
        float ll = 0.0f;
        const CMNet *pGrModel =
            static_cast<const CMNet*>(GetModel());
        
        const CEvidence *pEv = evidences.front();
        ll = pGrModel->ComputeLogLik(pEv);
        int i;
        for( i = 1; i < evidences.size(); i++ )
        {
            if( ! IsEvidencesEqual( pEv, evidences[i] ) )
            {
                float tmp = pGrModel->ComputeLogLik( evidences[i] );
                if( tmp > ll )
                {
                    best = i;
                    ll = tmp;
                    pEv = evidences[i];
                }
            }
        }
        
    }
    
    
    return evidences[best];
}


void CGibbsWithAnnealingInfEngine::
AnnealingModel( float T, const CEvidence *pEv, const char * doAnnealing )
{
    pFactorVector pFactors;
    GetCurrentFactors( &pFactors );
    int i;
    for( i = 0; i < pFactors.size(); i++ )
    {
	
	Annealing( pFactors[i], T, pEv );
    }
}


void CGibbsWithAnnealingInfEngine::
Annealing(CFactor *pFactor, float T, const CEvidence *pEv)
{
    PNL_CHECK_FOR_ZERO(T);
    
    CDistribFun *pDistribFun = pFactor->GetDistribFun();
    CMatrix<float>* pMat;
    CMatrixIterator<float>* iter = NULL;
    switch(pDistribFun->GetDistributionType()) 
    {
    case dtTabular:
        {
            
	    pMat = pDistribFun->GetMatrix(matTable);
	    if ( !pEv)
	    {
		iter = pMat->InitIterator();
		float koeff = 0.0f;
		for( iter; pMat->IsValueHere( iter ); pMat->Next(iter) )
		{ 
		    float val = (float) pow(*(pMat->Value( iter )), 1.0f/T);
		    
		    koeff += val; 
		    * const_cast<float*>(pMat->Value( iter )) = val;
		}
		delete iter;
		
		iter = pMat->InitIterator();
		for( iter; pMat->IsValueHere( iter ); pMat->Next(iter) )
		{
		    float val = (*(pMat->Value( iter )))/koeff;
		    
		    * const_cast<float*>(pMat->Value( iter )) = val;
		    
		}
		delete iter;
	    }
	    else
	    {
		const int* domain;
		int nnodes;
		
		pFactor->GetDomain(&nnodes, &domain);
		
		intVector indices;
		indices.resize(nnodes);
		int i;
		for( i = 0; i < nnodes; i++ )
		{
		    indices[i] = pEv->GetValue(domain[i])->GetInt();
		}
		
		float val = (float) pow((pMat->GetElementByIndexes(&indices.front())), 1.0f/T);
		pMat->SetElementByIndexes(val, &indices.front());
		pDistribFun->Normalize();
	    }
	    
        }
        break;
    case dtGaussian:
        {
            float T1 = 1/T;
            pMat = pDistribFun->GetMatrix(matCovariance);
            iter = pMat->InitIterator();
            float val;
	    for( iter; pMat->IsValueHere( iter ); pMat->Next(iter) )
            {
                val = *(pMat->Value( iter ))*T1;
		* const_cast<float*>(pMat->Value( iter )) = val;
		
            }
            delete iter;
        }
        break;
    case dtCondGaussian:
        {
            float T1 = 1/T;
            
            CCondGaussianDistribFun *pTmpDistr = 
                static_cast<CCondGaussianDistribFun *>(pDistribFun);
            
            CMatrix<CGaussianDistribFun*>* pMatDistr = 
                pTmpDistr->GetMatrixWithDistribution();
            
            CMatrixIterator<CGaussianDistribFun*>* iterDistr = 
                pMatDistr->InitIterator();
	    
            for( iterDistr; pMatDistr->IsValueHere( iterDistr ); pMatDistr->Next(iterDistr) )
            {
                CGaussianDistribFun *pGausDistr = *(pMatDistr->Value(iterDistr));
                pMat = pGausDistr->GetMatrix(matCovariance);
                iter = pMat->InitIterator();
                for( iter; pMat->IsValueHere( iter ); pMat->Next(iter) )
                {
                    * const_cast<float*>(pMat->Value( iter )) = *(pMat->Value( iter ))*T1;
                }
                delete iter;
            }
            delete iterDistr;
        }
        break;
    default:
        {
            PNL_THROW( CNotImplemented, "Not implemented" );
        }
    }
    
}

void CGibbsWithAnnealingInfEngine::
MarginalNodes( const int *query, int querySize, int notExpandJPD  )
{
    PNL_CHECK_IS_NULL_POINTER(m_pBestEvidence);
    const CModelDomain *pMD = GetModel()->GetModelDomain();
    int nnodes = GetModel()->GetNumberOfNodes();
    intVector ndsForSampling;
    GetNdsForSampling(&ndsForSampling);
    intVector nVls;
    nVls.resize(querySize);
    
    int snVls = 0;
    int i;
    for( i = 0; i < querySize; i++ )
    {
	if( nnodes != int(ndsForSampling.size()))
	{
	    if( std::find(ndsForSampling.begin(), ndsForSampling.end(), query[i]) == ndsForSampling.end() )
	    {
		PNL_THROW(CAlgorithmicException, "check query")
	    }
	}
	int n = pMD->GetNumVlsForNode(query[i]);
	nVls[i] = n;
	snVls += n;
	
    }
    valueVector vls;
    vls.resize(snVls);
    valueVector::iterator it = vls.begin();
    for( i = 0; i < querySize; i++ )
    {
	const Value* val;
	val = m_pBestEvidence->GetValue( query[i] );
	
	int j;
	for( j = 0; j < nVls[i]; j++, it++)
	{
	    *it = val[j];
	}
    }
    delete m_pEvidenceMPE;
    m_pEvidenceMPE = CEvidence::Create(GetModel()->GetModelDomain(), querySize, query, vls );
    
}

void CGibbsWithAnnealingInfEngine::
MarginalNodes( const intVector& queryNdsIn, int notExpandJPD  )
{
    MarginalNodes( &queryNdsIn.front(), queryNdsIn.size() );
}

bool CGibbsWithAnnealingInfEngine::
IsVlsEqual(valueVector &vls1, valueVector &vls2, float eps) const
{
    
    valueVector::iterator it1 = vls1.begin();
    valueVector::iterator it2 = vls2.begin();
    for( ; it1 != vls1.end(); it1++, it2++ )
    {
	if( it1->IsDiscrete())
	{
	    if( it1->GetInt() != it2->GetInt() )
		return false;
	}
	else
	{
	    if( fabs(it1->GetFlt() - it2->GetFlt()) > eps )
	    {
		return false;
	    }
	}
    }
    
    return  true; 
}
bool CGibbsWithAnnealingInfEngine::
IsEvidencesEqual(const CEvidence* pEv1, const CEvidence* pEv2, float eps) const
{
    
    valueVector vls1, vls2;
    pEv1->GetRawData(&vls1);
    pEv2->GetRawData(&vls2);
    
    return IsVlsEqual(vls1, vls2, eps);
    
}

void CGibbsWithAnnealingInfEngine::
CreateSamplingPotentials( potsPVector* potsToSampling )
{
    
    CModelDomain *pMD = GetModel()->GetModelDomain();
    
    intVector ndsForSampling;
    GetNdsForSampling( &ndsForSampling );
    potsToSampling->resize( ndsForSampling.size() );
    
    CPotential *tmpPot;
    
    int i;
    for( i = 0; i < ndsForSampling.size(); i++ )
    {
        const CNodeType* nt = pMD->GetVariableType( ndsForSampling[i]);
        if( nt->IsDiscrete() )
        {
            tmpPot = CTabularPotential::
                Create( &ndsForSampling[i], 1, pMD, &floatVector(nt->GetNodeSize(), 1.0f).front() );
        }
        else
        {
            tmpPot = CGaussianPotential::
                CreateUnitFunctionDistribution( &ndsForSampling[i], 1, pMD );
        }
        (*potsToSampling)[i] = tmpPot;
    }
}


float CGibbsWithAnnealingInfEngine::
Temperature( float c, float s ) const
{
    
    return  float(c/log( s + 1.f));
}



void CGibbsWithAnnealingInfEngine::DestroyInf()
{
    pEvidencesVector*  pEv = GetCurrentEvidences();
    int i;
    for( i = 0; i < pEv->size(); i++ )
    {
        delete (*pEv)[i];
    }
    potsPVector* potsToSampling= GetPotsToSampling();
    for( i = 0; i < potsToSampling->size(); i++ )
    {
        delete (*potsToSampling)[i];
    }
    
    pFactorVector* currentFactors = GetCurrentFactors();
    for( i = 0; i < currentFactors->size(); i++ )
    {
        delete (*currentFactors)[i];
    }
    
    delete m_pBestEvidence;
}



void CGibbsWithAnnealingInfEngine::FillVlsVector( pEvidencesVector &pEv )
{
    pEvidencesVector::iterator it = pEv.begin();
    valueVecVector::iterator itVls = m_oldVls.begin();
    for( ; it != pEv.end(); it++, itVls++ )
    {
	itVls->clear();
	(*it)->GetRawData( &*itVls );
    }
}

void CGibbsWithAnnealingInfEngine::SetAnnealingCoefficientC(float val)
{
    PNL_CHECK_LEFT_BORDER(val, 0.0f);
    m_constC = val;
    
}

void CGibbsWithAnnealingInfEngine::SetAnnealingCoefficientS(float val)
{
    PNL_CHECK_LEFT_BORDER(val, 0.0f);
    m_constS = val;
    
}

void CGibbsWithAnnealingInfEngine::UseAdaptation(bool isUse )
{
    if( isUse )
    {
	if( GetNumStreams() != 1)
	{
	    PNL_THROW(CAlgorithmicException, "number of streams in sampling");
	}
	m_bAdapt = true;
    }
    else
    {
	isUse = false;
    }
}

bool CGibbsWithAnnealingInfEngine::IsAdapt() const
{
    return m_bAdapt && (GetNumStreams() == 1);
}

void CGibbsWithAnnealingInfEngine::SetCurrentTemp( float T)
{
    m_currentTemp = T;
}

float CGibbsWithAnnealingInfEngine::GetCurrentTemp( ) const
{
    return m_currentTemp;
}

float CGibbsWithAnnealingInfEngine::GetLikelihood(const CEvidence *pEv)
{	
    float logLik = 0.0f;
    if(0) 
    {
	pFactorVector pFactors;
	GetCurrentFactors( &pFactors );
	
	float val;
	int s;
	for( s = 0; s < pFactors.size(); s++)
	{
	    val = pFactors[s]->GetLogLik(pEv);
	    
	    logLik += val;
	    
	}
    }
    else
    {
	int nFactors = m_pGraphicalModel->GetNumberOfFactors();
	int i;
	for( i = 0; i < nFactors; i++ )
	{
	    logLik += m_pGraphicalModel->GetFactor(i)->GetLogLik(pEv);
	}
    }
    return logLik;
}

#ifdef PNL_RTTI
const CPNLType CGibbsWithAnnealingInfEngine::m_TypeInfo = CPNLType("CGibbsWithAnnealingInfEngine", &(CSamplingInfEngine::m_TypeInfo));

#endif
