/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGibbsSamplingInferenceEngine.cpp                         //
//                                                                         //
//  Purpose:   CGibbsSamplingInfEngine class member functions              //
//             implementation                                              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "pnlConfig.hpp"
#include "pnlNaiveInferenceEngine.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "pnlMNet.hpp"
#include "pnlGibbsSamplingInferenceEngine.hpp"
#include "pnlException.hpp"
#include <time.h>

PNL_USING

CGibbsSamplingInfEngine * CGibbsSamplingInfEngine::Create( 
							  const CStaticGraphicalModel *pGraphicalModel )
{
    if( !pGraphicalModel )
    {
	PNL_THROW( CNULLPointer, "graphical model" );
	return NULL;
    }
    else
    {
	CGibbsSamplingInfEngine* newInf = new CGibbsSamplingInfEngine( pGraphicalModel );
	return newInf;
    }
}


CGibbsSamplingInfEngine::CGibbsSamplingInfEngine( const CStaticGraphicalModel
						 *pGraphicalModel )
						 :CSamplingInfEngine( pGraphicalModel ),
						 m_queryNodes( 0 ),
						 m_queryes(0),
						 m_queryFactors(0),
						 m_pPotMPE(NULL),
						 m_bUsingDSep(false)
						 
						 
{
    m_bMaximize = 0;
    Initialization();
}

CGibbsSamplingInfEngine::~CGibbsSamplingInfEngine()
{
    DestroyQueryFactors();
    delete m_pQueryJPD;
    m_pQueryJPD = NULL;
    delete m_pPotMPE;
    m_pPotMPE = NULL;
}


void CGibbsSamplingInfEngine::UseDSeparation(bool isUsing)
{
    if( GetModel()->GetModelType() != mtBNet )
    {
	PNL_THROW(CNotImplemented, "d-separation have been realized for DAG" );
    }
   m_bUsingDSep = true;
}

void CGibbsSamplingInfEngine::Initialization()
{
    
    int nFactors = GetModel()->GetNumberOfFactors();
    pFactorVector *currentFactors = GetCurrentFactors();
    currentFactors->resize(nFactors);
    int i;
    for( i = 0; i < nFactors; i++ )
    {
        (*currentFactors)[i] = GetModel()->GetFactor(i);
    }
    
}

void CGibbsSamplingInfEngine::SetQueries(intVecVector &queryes)
{
    int nQueryes = queryes.size();
    PNL_CHECK_LEFT_BORDER( nQueryes, 1 );
    
    int i;
    for( i = 0; i < m_queryFactors.size(); i++ )
    {
        delete m_queryFactors[i];
    }
    
    intVector tmp;
    for( i = 0; i < nQueryes; i++ )
    {
        PNL_CHECK_RANGES( queryes[i].size(), 1, m_pGraphicalModel->GetNumberOfNodes() );
        tmp = queryes[i];
        std::sort( tmp.begin(), tmp.end() );
        intVector::iterator it = std::unique( tmp.begin(), tmp.end() );
        tmp.erase( it, tmp.end() );
        if( tmp.size() != queryes[i].size() )
        {
            PNL_THROW(CAlgorithmicException, "equal nodes in qurey");
        }
        tmp.clear();
    }
    m_queryes = queryes;    
    
    
}

void CGibbsSamplingInfEngine::DestroyQueryFactors()
{
    pFactorVector *queryFactors = GetQueryFactors();
    int i;
    for( i = 0; i < queryFactors->size(); i++ )
    {
        delete (*queryFactors)[i];
    }
    queryFactors->clear();
}

void CGibbsSamplingInfEngine::
EnterEvidence( const CEvidence *pEvidenceIn, int maximize, int sumOnMixtureNode )
{
    if( !m_queryes.size() )
    {
        PNL_THROW( CAlgorithmicException, "Possible queryes must be defined");
    }
        
    PNL_CHECK_IS_NULL_POINTER(pEvidenceIn);
    m_pEvidence = pEvidenceIn;
    m_bMaximize = maximize;
    
    DestroyCurrentEvidences();
    DestroyQueryFactors();
   
    
    if(GetModel()->GetModelType() == mtBNet)
    {
        static_cast< const CBNet* >(GetModel())->
            GenerateSamples( GetCurrentEvidences(), GetNumStreams(), pEvidenceIn );
    }
    else
    {
        static_cast< const CMNet* >(GetModel())->
            GenerateSamples( GetCurrentEvidences(), GetNumStreams(), pEvidenceIn );
	
    }
    
    CreateQueryFactors();
    
    boolVector sampleIsNeed;
    if( m_bUsingDSep )
    {
	ConsDSep( m_queryes, &sampleIsNeed, m_pEvidence );
    }
    else
    {
	FindCurrentNdsForSampling( &sampleIsNeed );
    }
    SetSamplingNdsFlags(sampleIsNeed);
    
    Sampling( 0, GetMaxTime() );
}

void CGibbsSamplingInfEngine::
Sampling( int statTime, int endTime )
{
    
    intVector ndsForSampling;
    GetNdsForSampling( &ndsForSampling );
    boolVector sampleIsNeed;
    GetSamplingNdsFlags( &sampleIsNeed );
    
    int numNdsForSampling = ndsForSampling.size();
    
    pEvidencesVector currentEvidences;
    GetCurrentEvidences( &currentEvidences );
    CEvidence * pCurrentEvidence;
    
    int t; 
    int i;
    for( t = statTime; t < endTime; t++ )
    {
        int series;
        for( series = 0; series < GetNumStreams(); series++ )
        {
            pCurrentEvidence = currentEvidences[series];
            
	    
            for( i = 0; i < numNdsForSampling; i++ )
            {
		if( sampleIsNeed[i] )
		{
		    pCurrentEvidence->ToggleNodeStateBySerialNumber(1, &i);
		    bool canBeSample = ConvertingFamilyToPot( ndsForSampling[i], pCurrentEvidence );
		    if(canBeSample)
		    {
			
			GetPotToSampling(ndsForSampling[i])->GenerateSample( pCurrentEvidence, m_bMaximize );
		    }
		    else
		    {
			pCurrentEvidence->ToggleNodeStateBySerialNumber(1, &i);
		    }
		}
		
            }	
	    
        }
        if( t > GetBurnIn())
	{
	    pFactorVector queryFactors;
	    GetQueryFactors( &queryFactors );
	    int i;
	    for( i = 0; i < queryFactors.size(); i++ )
            {
                queryFactors[i]->UpdateStatisticsML( &(GetCurrentEvidences()->front()), GetNumStreams() );
            }  
	    
	}
    }
    
}


void CGibbsSamplingInfEngine::
MarginalNodes( const intVector& queryNdsIn, int notExpandJPD  )
{
    MarginalNodes( &queryNdsIn.front(), queryNdsIn.size(), notExpandJPD );
}

void CGibbsSamplingInfEngine::
MarginalNodes( const int *queryIn, int querySz, int notExpandJPD )
{
    delete m_pQueryJPD;
    m_pQueryJPD = NULL;
    
    delete m_pPotMPE;
    m_pPotMPE = NULL;
    
    delete m_pEvidenceMPE;
    m_pEvidenceMPE = NULL;
    
    const CFactor *pFactor;
    CPotential *pPot =  NULL;
    int *begin1;
    int *end1;
    int *begin2;
    int *end2;
    
    intVector domainVec;
    intVector queryVec;
    intVector obsQueryVec;
    queryVec.reserve(querySz);
    obsQueryVec.reserve(querySz);
    int i;
    for( i = 0; i < querySz; i++ )
    {
        m_pEvidence->IsNodeObserved(queryIn[i]) ? 
            obsQueryVec.push_back(queryIn[i]):
        queryVec.push_back(queryIn[i]);
    }
    
    
    CPotential *tmpPot = NULL;
    
    if( queryVec.size() )
    {
        for( i = 0; i < m_queryFactors.size(); i++)     
        {
            
            domainVec.clear();
            pFactor = m_queryFactors[i];
            pFactor->GetDomain(&domainVec);
            begin1 = &domainVec.front();
            end1 = &domainVec.back() + 1;
            std::sort(begin1, end1);

            begin2 = &queryVec.front();
            end2 = &queryVec.back() + 1;
            std::sort(begin2, end2);

            if( std::includes(begin1, end1, begin2, end2) )
            {
                pPot = pFactor->ConvertStatisticToPot( (GetMaxTime()-GetBurnIn())*GetNumStreams() );
                tmpPot = pPot->Marginalize( queryVec );
                delete pPot;
                break;
	    }
            
        }
        if( !tmpPot )
        {
            PNL_THROW(CInvalidOperation, "Invalid query");
        }
    }
    delete m_pQueryJPD; 
    
    if( obsQueryVec.size() )
    {
        	
	EDistributionType paramDistrType = 
	    pnlDetermineDistributionType( GetModel()->GetModelDomain(), querySz, queryIn, m_pEvidence);

        
        CPotential *pQueryPot;
        switch( paramDistrType )
        {
        case dtTabular:
            {
                pQueryPot = CTabularPotential::CreateUnitFunctionDistribution(
                    queryIn, querySz, m_pGraphicalModel->GetModelDomain() );
                break;
            }
            
        case dtGaussian:
            {
                pQueryPot = CGaussianPotential::CreateUnitFunctionDistribution(
		    queryIn, querySz, m_pGraphicalModel->GetModelDomain()  );
                break;
            }
        case dtCondGaussian:
            {
                PNL_THROW( CNotImplemented, "conditional gaussian factors" )
                    break;
            }
        default:
            {
                PNL_THROW( CInconsistentType, "distribution type" )
            }
        }
        
        if( tmpPot)
        {
            (*pQueryPot) *= (*tmpPot);
            delete tmpPot;
        }
        
        if( m_bMaximize )
        {
            m_pPotMPE   = static_cast<CPotential*>
                ( pQueryPot->ExpandObservedNodes( m_pEvidence, 0) );
            
            m_pEvidenceMPE = m_pPotMPE->GetMPE();
        }
        else
        {
            m_pQueryJPD = static_cast<CPotential*>( pQueryPot->ExpandObservedNodes( m_pEvidence, 0) );
        }
        
        delete pQueryPot;
    }
    else
    {
        if( m_bMaximize )
        {
            m_pPotMPE = tmpPot;
            m_pEvidenceMPE = m_pPotMPE->GetMPE();
        }
        else
        {
            m_pQueryJPD = tmpPot;
            
        }
    }
    
    
}

const CPotential* CGibbsSamplingInfEngine::
GetQueryJPD() const
{
    if( !m_pQueryJPD )
    {
        PNL_THROW( CInvalidOperation,
            " can't call GetQueryJPD() before calling MarginalNodes() " );
    }
    
    return m_pQueryJPD;
}

const CEvidence* CGibbsSamplingInfEngine::GetMPE() const
{
    if( !m_bMaximize )
    {
        PNL_THROW( CInvalidOperation,
            " you have not been computing the MPE ");
    }
    
    if( !m_pEvidenceMPE )
    {
        PNL_THROW( CInvalidOperation,
            " can't call GetMPE() before calling MarginalNodes() " );
    }
    
    return m_pEvidenceMPE;
}



void CGibbsSamplingInfEngine::CreateQueryFactors()
{
    pConstNodeTypeVector ntVec;
    const CNodeType *nt;
    intVector query;
    for( int number = 0; number < m_queryes.size(); number++ )
    {
        for( int node = 0; node < m_queryes[number].size(); node++)
        {
            if( !m_pEvidence->IsNodeObserved(m_queryes[number][node]) )
            {
                query.push_back(m_queryes[number][node]);
                nt = m_pGraphicalModel->GetNodeType(m_queryes[number][node]);
                if ( ntVec.size() )
                {
                    if(  !( ( nt->IsDiscrete() && ntVec.back()->IsDiscrete() ) ||
                        ( !nt->IsDiscrete() && !ntVec.back()->IsDiscrete() ) ))
                        
                    {
                        PNL_THROW(CAlgorithmicException, "invalid query");
                    }
                }
                ntVec.push_back( nt );
            }
            
        }
        if( query.size() )
        {
            if( ntVec[0]->IsDiscrete() )
            {
                m_queryFactors.push_back( CTabularPotential::Create( query, 
                    m_pGraphicalModel->GetModelDomain(), NULL ) );
            }
            else
            {
                m_queryFactors.push_back( CGaussianPotential::Create( query, 
                    m_pGraphicalModel->GetModelDomain(), 0 ) );
            }
        }
        ntVec.clear();
        query.clear();
    }
}

void CGibbsSamplingInfEngine::
ConsDSep(intVecVector &allNds, boolVector *sampleIsNeed, const CEvidence *pEv ) const
{
    
    intVector ndsForSample;
    GetNdsForSampling(&ndsForSample);
    
    int nnodes = pEv->GetNumberObsNodes();
    const int* obsNds = pEv->GetAllObsNodes();
    const int* flags = pEv->GetObsNodesFlags();
    intVector separator;
    separator.reserve(nnodes);
    
    int i;
    for( i = 0; i < nnodes; i++ )
    {
	if( flags[i] )
	{
	    separator.push_back( obsNds[i] );
	}
    }
    

    const CGraph *pGraph = GetModel()->GetGraph();   
    boolVector flagsDSep( pGraph->GetNumberOfNodes() );

    intVector dsep;
    
    for( i = 0; i < allNds.size(); i++ )
    {
	int j;
	for( j = 0; j < allNds[i].size(); j++ )
	{
	    dsep.clear();
	    pGraph->GetDConnectionList( (allNds[i])[j], separator, &dsep );
	    
	    int k;
	    for( k = 0; k < dsep.size(); k++ )
	    {
		flagsDSep[dsep[k]] = true;
	    }
	}
    }
    
    FindCurrentNdsForSampling( sampleIsNeed );
    for( i = 0; i < ndsForSample.size(); i++ )
    {
	(*sampleIsNeed)[i] = (*sampleIsNeed)[i] && flags[ndsForSample[i]];
    }
     
}
