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
//  Purpose:   CTreeCPD class member functions implementation          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//GaussianCPD.cpp
///////////////////////////////////////////////////////////////////////////////
#include <float.h>
#include <math.h>

#include "pnlConfig.hpp"
#include "pnlTreeCPD.hpp"
#include "pnlDistribFun.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlTreeDistribFun.hpp"


PNL_USING

CTreeCPD* CTreeCPD::Create( const intVector& domain, CModelDomain* pMD )
{
    return CTreeCPD::Create( &domain.front(), domain.size(), pMD );
}

CTreeCPD* CTreeCPD::Create( const int *domain, int nNodes,
                            CModelDomain* pMD )
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_LEFT_BORDER( nNodes, 1 );
	
	CTreeCPD *pNewParam = new CTreeCPD( domain, nNodes, pMD);
    PNL_CHECK_IF_MEMORY_ALLOCATED( pNewParam );
	return pNewParam;
}

CTreeCPD* CTreeCPD::Copy( const CTreeCPD* pTreeCPD )
{
	PNL_CHECK_IS_NULL_POINTER( pTreeCPD );

	CTreeCPD *retCPD = new CTreeCPD( *pTreeCPD );
	PNL_CHECK_IF_MEMORY_ALLOCATED( retCPD );
	return retCPD;
}


CFactor* CTreeCPD::CloneWithSharedMatrices()
{
    CTreeCPD* resCPD = new CTreeCPD(this);
    PNL_CHECK_IF_MEMORY_ALLOCATED(resCPD);
    return resCPD;
}

CFactor* CTreeCPD::Clone() const
{
    const CTreeCPD* self = this;
    CTreeCPD* res = CTreeCPD::Copy(self);
    return res;
}

CTreeCPD::CTreeCPD( const CTreeCPD& TreeCPD )
:CCPD( dtTree, ftCPD, TreeCPD.GetModelDomain() )
{
	//m_CorrespDistribFun = TreeCPD.m_CorrespDistribFun->CloneDistribFun();
	if( TreeCPD.m_CorrespDistribFun->GetDistributionType() == dtTree )
    {
        delete m_CorrespDistribFun;
        m_CorrespDistribFun = CTreeDistribFun::Copy( 
            static_cast<CTreeDistribFun*>(TreeCPD.m_CorrespDistribFun ));
    }
    else
    {
        PNL_THROW( CInconsistentType, "distribution must be tree" );
    }
	m_Domain = intVector( TreeCPD.m_Domain );
}

CTreeCPD::CTreeCPD( const int *domain, int nNodes, CModelDomain* pMD )
						:CCPD( dtTree, ftCPD, domain, nNodes, pMD )
{
}

CTreeCPD::CTreeCPD( const CTreeCPD* pTreeCPD ):CCPD(pTreeCPD)
{
	
}


CPotential *CTreeCPD::ConvertToPotential() const
{
	PNL_THROW( CNotImplemented, "this opeartion" );
	return NULL;
}

void CTreeCPD::UpdateStatisticsEM( const CPotential *pMargPot,
                                     const CEvidence *pEvidence )
{
	if( !pMargPot )
	{
		PNL_THROW( CNULLPointer, "evidences" )//no corresp evidences
	}

    intVector obsPos;
    pMargPot->GetObsPositions(&obsPos);
    
    m_CorrespDistribFun->UpdateStatisticsEM( pMargPot->GetDistribFun(), pEvidence, 1.0f,
            &m_Domain.front() );
}

float CTreeCPD::ProcessingStatisticalData( int numberOfEvidences)
{
    return m_CorrespDistribFun->ProcessingStatisticalData((float)numberOfEvidences);
}


void CTreeCPD::UpdateStatisticsML(const CEvidence* const* pEvidences,
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


void CTreeCPD::GenerateSample( CEvidence* evidence, int maximize ) const
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
	static_cast<CTreeDistribFun*>(m_CorrespDistribFun)->GenerateSample(evidence, maximize);
}


CPotential* CTreeCPD::ConvertStatisticToPot(int ) const
{
   	PNL_THROW( CNotImplemented, "this opeartion" );
    return NULL;
}

float CTreeCPD::GetLogLik( const CEvidence* pEv, const CPotential* ) const
{
	return static_cast<CTreeDistribFun*>(m_CorrespDistribFun)->GetLogLik(pEv);
}

void CTreeCPD :: NormalizeCPD()
{
}

#ifdef PAR_PNL
void CTreeCPD::UpdateStatisticsML(CFactor *pPot)
{
    PNL_THROW(CNotImplemented, 
        "UpdateStatisticsML for CTreeCPD not implemented yet");
};
#endif // PAR_OMP

void CTreeCPD::UpdateTree(const CGraph* pGraphTree, TreeNodeFields *fields )
{
    static_cast<CTreeDistribFun*>(m_CorrespDistribFun)->SetDomain(m_Domain);
    static_cast<CTreeDistribFun*>(m_CorrespDistribFun)->UpdateTree(pGraphTree,fields);
}

CPotential* CTreeCPD::ConvertWithEvidenceToPotential(const CEvidence* pEvidence, 
                    int )const
{
    int DomSize = m_CorrespDistribFun->GetNumberOfNodes();
    intVector obsVec;
    valueVecVector valVec;
    pConstNodeTypeVector NTVec;
    pEvidence->GetObsNodesWithValues( &obsVec, &valVec, &NTVec );
    const pConstNodeTypeVector *NodeTVector = m_CorrespDistribFun->GetNodeTypesVector();
    CPotential *resPot; 
    intVector obsIndInDomain;
    int i;
    for( i = 0; i < m_Domain.size(); ++i )
    {
        if (pEvidence->IsNodeObserved(m_Domain[i]))
            obsIndInDomain.push_back(i);
    }
    if ((*NodeTVector)[DomSize - 1]->IsDiscrete())
    {
        for( i = 0; i < m_Domain.size(); ++i)
        {
            if(!((*NodeTVector)[i]->IsDiscrete()) && !(pEvidence->IsNodeObserved(m_Domain[i])))
                obsIndInDomain.push_back(i);
        }
        floatVector prob = static_cast<CTreeDistribFun*>(m_CorrespDistribFun)->GetProbability(pEvidence);
        resPot = CTabularPotential::Create(	&m_Domain.front(), m_Domain.size(), GetModelDomain(), 
            &prob.front(), obsIndInDomain);
    }
    else
    {
        for( i = 0; i < m_Domain.size(); ++i)
        {
            if(((*NodeTVector)[i]->IsDiscrete()) && !(pEvidence->IsNodeObserved(m_Domain[i])))
                obsIndInDomain.push_back(i);
        }
        float exp, var;
        static_cast<CTreeDistribFun*>(m_CorrespDistribFun)->GetAdjectives( pEvidence, exp, var );
        resPot = CGaussianPotential::Create( &m_Domain.front(), m_Domain.size(), GetModelDomain(), 1, 
            &exp, &var, 0.0f, obsIndInDomain);
    }
    return resPot;
}

#ifdef PNL_RTTI
const CPNLType CTreeCPD::m_TypeInfo = CPNLType("CTreeCPD", &(CCPD::m_TypeInfo));

#endif
