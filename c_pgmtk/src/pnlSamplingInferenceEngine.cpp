/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlSamplingInferenceEngine.cpp                              //
//                                                                         //
//  Purpose:   CSamplingInfEngine class member functions                   //
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
#include "pnlSamplingInferenceEngine.hpp"
#include "pnlException.hpp"

PNL_USING



CSamplingInfEngine::CSamplingInfEngine( const CStaticGraphicalModel
				       *pGraphicalModel )
				       :CInfEngine( itGibbsSampling,
				       pGraphicalModel ),
				       
				       m_maxTime(600),
				       m_barnin(10),
				       m_nStreams(1),
				       m_pEvidences(0),
				       m_potsToSampling(0),
				       m_ndsForSampling(0),
				       m_mixtureNds(0),
				       m_allNdsIsTab(CheckIsAllNdsTab()),
				       m_sampleIsNeed(0)
				       
				       
				       
				       
				       
{
    EModelTypes mt;
    mt = GetModel()->GetModelType();
    
    if( mt != mtBNet && mt != mtMNet && mt != mtMRF2  )
    {
        PNL_THROW( CNotImplemented, " not emplemented for this model ")
    }
    
    
    FindNdsForSampling( &m_ndsForSampling, &m_mixtureNds );
    FindEnvironment(&m_environment);
    CreateSamplingPotentials( &m_potsToSampling );
    
}

CSamplingInfEngine::~CSamplingInfEngine()
{
    
    DestroyInf();
}




void CSamplingInfEngine::
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

void CSamplingInfEngine::
FindEnvironment( intVecVector *chlds )
{
    
    
    const CGraph *pGraph = m_pGraphicalModel->GetGraph();
    int i;
    if( GetModel()->GetModelType() != mtBNet )
    {
        int nnodes; 
        nnodes = GetModel()->GetNumberOfNodes();
        chlds->resize( nnodes );
        const CMNet* pMNet = static_cast<const CMNet*>( m_pGraphicalModel );
        
        for( i = 0; i < nnodes; i++ )
        {
            
            pMNet->GetClqsNumsForNode( i, &(*chlds)[i] );
        }
    }
    else
    {
        intVector ndsForSampling;
        GetNdsForSampling( &ndsForSampling );
        chlds->resize( ndsForSampling.size() );
        
        for( i = 0; i < ndsForSampling.size(); i++ )
        {
            pGraph->GetChildren( ndsForSampling[i], &(*chlds)[i] );
            (*chlds)[i].push_back( ndsForSampling[i]) ;
        }
        
    }
    
}


void CSamplingInfEngine::Normalization(CPotential *pot)
{
    
    if( pot->GetDistributionType() != dtTabular )
    {
	const pConstNodeTypeVector *nt = pot->GetDistribFun()->GetNodeTypesVector();
	CDistribFun *pUnitFun = CGaussianDistribFun::
	    CreateUnitFunctionDistribution(nt->size(), &nt->front());
	pot->SetDistribFun(pUnitFun);
	delete pUnitFun;
    }
    else
    {
	CMatrix< float > *pMatToSample;
	pMatToSample = static_cast<CTabularDistribFun*>(pot->GetDistribFun())
	    ->GetMatrix(matTable);
	EMatrixClass mc = pMatToSample->GetMatrixClass();
	if( mc != mcNumericDense || mc != mc2DNumericDense )
	{
	    
	    CMatrixIterator<float>* iter = pMatToSample->InitIterator();
	    for( iter; pMatToSample->IsValueHere( iter ); pMatToSample->Next(iter) )
	    {
		*const_cast<float*>(pMatToSample->Value( iter )) = 1.0f;;
		
	    }
	    delete iter;
	}
	else
	{
	    
	    floatVector *data = const_cast< floatVector *>(
		static_cast<CNumericDenseMatrix<float>*>(pMatToSample)->GetVector());
	    floatVector::iterator it1 = data->begin();
	    floatVector::iterator it2 = data->end();
	    for( ; it1 != it2; it1++ )
	    {
		*it1 = 1.0f;
	    }
	}
	
    }
    
}

bool CSamplingInfEngine::
ConvertingFamilyToPot( int node, const CEvidence* pEv )
{
    bool ret = false;
    CPotential* potToSample = m_potsToSampling[node];
    Normalization(potToSample);
    int i;
    if( !IsAllNdsTab() )
    {
	
	if( GetModel()->GetModelType() == mtBNet )
	{
	    
	    for( i = 0; i < m_environment[node].size(); i++ )
	    {            
		int num = m_environment[node][i];
		CPotential *pot1 = static_cast< CCPD* >( m_currentFactors[ num ] )
		    ->ConvertWithEvidenceToPotential(pEv);
		CPotential *pot2 = pot1->Marginalize(&node, 1);
		delete pot1;
		*potToSample *= *pot2;
		delete pot2;
	    }
	}
	
	else
	{
	    for( i = 0; i < m_environment[node].size(); i++ )
	    {
		int num = m_environment[node][i];
		CPotential *pot1 = static_cast< CPotential* >( m_currentFactors[ num ] )
		    ->ShrinkObservedNodes(pEv);
		CPotential *pot2 = pot1->Marginalize(&node, 1);
		delete pot1;
		*potToSample *= *pot2;
		delete pot2;
	    }
	}
	
    }
    else
    {
	
	CMatrix< float > *pMatToSample;
	pMatToSample = static_cast<CTabularDistribFun*>(potToSample->GetDistribFun())
	    ->GetMatrix(matTable);
	
	intVector dims;
	intVector vls;
	intVector domain;
	
	for( i = 0; i < m_environment[node].size(); i++ )
	{            
	    int num = m_environment[node][i];
	    m_currentFactors[ num ]->GetDomain(&domain);
	    GetObsDimsWithVls( domain, node, pEv, &dims, &vls); 
	    CMatrix< float > *pMat;
	    pMat = static_cast<CTabularDistribFun*>(m_currentFactors[ num ]->
		GetDistribFun())->GetMatrix(matTable);
	    pMat->ReduceOp( &dims.front(), dims.size(), 2, &vls.front(),
		pMatToSample, PNL_ACCUM_TYPE_MUL );
	    dims.clear();
	    vls.clear();
	    domain.clear();
	    
	}
    }
    
    //check for non zero elements
    CMatrix<float> *pMat;
    if( potToSample->GetDistributionType()==dtTabular )
    {	
	pMat = potToSample->GetDistribFun()->GetMatrix(matTable);
    }
    else
    {
	CGaussianDistribFun* pDistr = static_cast<CGaussianDistribFun*>(potToSample->GetDistribFun());
	if(pDistr->GetMomentFormFlag())
	{
	    pMat = pDistr->GetMatrix(matCovariance);
	}
	else
	{
	    pMat = pDistr->GetMatrix(matK);

	}
    }
    CMatrixIterator<float>* iter = pMat->InitIterator();
    for( iter; pMat->IsValueHere( iter ); pMat->Next(iter) )
    {
	
	if(*(pMat->Value( iter )) > FLT_EPSILON)
	{
	    ret = true;
	    break;
	}
    }
    delete iter;
    return ret;
}

void CSamplingInfEngine::
GetObsDimsWithVls(intVector &domain, int nonObsNode, const CEvidence* pEv, 
		  intVector *dims, intVector *vls) const
{
    int nnodes = domain.size();
    dims->resize(nnodes - 1);
    vls->resize(nnodes - 1);
    
    int* it = &domain.front();
    int* itDims = &dims->front();
    int* itVls = &vls->front();
    int i;
    for( i = 0; i < nnodes; i++, it++ )
    {		
	if( *it != nonObsNode )
	{
	    *itDims = i;
	    *itVls = pEv->GetValueBySerialNumber(*it)->GetInt();//only if all nds are tabular!!
	    
	    itDims++;
	    itVls++;
	}
    }
    
}

void CSamplingInfEngine::DestroyInf()
{
    DestroyCurrentEvidences();
    int i;
    for( i = 0; i < m_potsToSampling.size(); i++ )
    {
        delete m_potsToSampling[i];
    }
    m_potsToSampling.clear();
    
}

void CSamplingInfEngine::DestroyCurrentEvidences()
{
    int i;
    for( i = 0; i < m_pEvidences.size(); i++ )
    {
        delete m_pEvidences[i];
    }
    m_pEvidences.clear();
}

void CSamplingInfEngine::FindNdsForSampling( intVector* nds, intVector* mixtureNds )
{
    int nnodes = GetModel()->GetNumberOfNodes();
    if( GetModel()->GetModelType() == mtBNet )
    {
        const CBNet *pBNet = static_cast<const CBNet *>( GetModel() );
        
        nds->reserve(nnodes);
        
        const CFactor* pCPD;
        int i;
        for( i = 0; i < nnodes; i++)
        { 
            pCPD = pBNet->GetFactor(i);
            if( pCPD->GetDistributionType() == dtMixGaussian )
            {
                int node = static_cast< const CMixtureGaussianCPD* >
                    (pCPD)->GetNumberOfMixtureNode();
		
		mixtureNds->push_back(node);
                
		std::remove( nds->begin(), nds->end(), node );
            }
            nds->push_back( i );
        }
    }
    else
    {
        nds->resize(nnodes);
        int i;
        for( i = 0; i < nnodes; i++)
        { 
            (*nds)[i] = i;
        }
    }
}


bool CSamplingInfEngine::CheckIsAllNdsTab() const
{
    const CStaticGraphicalModel *pGrModel = GetModel();
    int nnodes = pGrModel->GetNumberOfNodes();
    const CModelDomain *pMD = GetModel()->GetModelDomain();
    int i;
    for( i = 0; i < nnodes; i++ )
    {
	if(! pMD->GetVariableType(i)->IsDiscrete())
	{
	    return false;
	}
    }
    return true;
}



void CSamplingInfEngine::FindCurrentNdsForSampling(boolVector *flags) const
{
    intVector ndsForSampling;
    GetNdsForSampling(&ndsForSampling);
    
    int nnds = ndsForSampling.size();
    
    const CEvidence *pEv = m_pEvidence;
    flags->assign(nnds, true);
    int i;
    for( i = 0; i < nnds; i++ )
    {
	if( pEv->IsNodeObserved(ndsForSampling[i]) )
	{
	    (*flags)[i] = false;
	}
    }
}

void CSamplingInfEngine::Continue( int dt )
{
    PNL_CHECK_LEFT_BORDER( dt, 0);
    
    int startTime = GetMaxTime();
    PNL_CHECK_LEFT_BORDER( startTime, 0);
    
    int endTime = startTime + dt;
    SetMaxTime(endTime);
    Sampling( startTime, endTime );
}



#ifdef PNL_RTTI
const CPNLType CSamplingInfEngine::m_TypeInfo = CPNLType("CSamplingInfEngine", &(CInfEngine::m_TypeInfo));

#endif