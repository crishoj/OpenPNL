/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlScalarPotential.cpp                                     //
//                                                                         //
//  Purpose:   CScalarPotential class member functions implementation     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlScalarPotential.hpp"
#include "pnlScalarDistribFun.hpp"
#include <math.h>

PNL_USING
CScalarPotential* CScalarPotential::Create( const intVector& domain,
                                CModelDomain* pMD, const intVector& obsIndices )
{
    return CScalarPotential::Create( &domain.front(), domain.size(), pMD, obsIndices );
}
CScalarPotential* CScalarPotential::Create( const int *domain, int nNodes,
                                  CModelDomain* pMD, const intVector& obsIndices )
{
    PNL_CHECK_IS_NULL_POINTER( domain );
    PNL_CHECK_IS_NULL_POINTER( pMD );
    PNL_CHECK_RANGES( nNodes, 1, pMD->GetNumberVariables() );

    CScalarPotential* resPot = new CScalarPotential( domain, nNodes, pMD, obsIndices );
    PNL_CHECK_IF_MEMORY_ALLOCATED(resPot);
    return resPot;
}
CScalarPotential* CScalarPotential::Copy( const CScalarPotential *pScalarPotential )
{
    PNL_CHECK_IS_NULL_POINTER( pScalarPotential );

    CScalarPotential* resPot = new CScalarPotential( *pScalarPotential );
    PNL_CHECK_IF_MEMORY_ALLOCATED(resPot);
    return resPot;
}
CFactor* CScalarPotential::CloneWithSharedMatrices()
{
    CScalarPotential* resPot = new CScalarPotential(this);
    PNL_CHECK_IF_MEMORY_ALLOCATED(resPot);

    return resPot;  
}

CFactor* CScalarPotential::Clone() const
{
    const CScalarPotential* self = this;
    CScalarPotential* res = CScalarPotential::Copy(self);
    return res;
}

float CScalarPotential::ProcessingStatisticalData(int numberOfEvidences)
{
    return 0.0f;
}
void CScalarPotential::UpdateStatisticsEM(const CPotential *pInfData, 
        const CEvidence *pEvidence )
{

}
void CScalarPotential::UpdateStatisticsML(const pConstEvidenceVector& evidencesIn)
{

}
void CScalarPotential::UpdateStatisticsML(const CEvidence* const* pEvidencesIn,
        int EvidenceNumber)
{

}
void CScalarPotential::Dump()const
{
    int i;
    dump()<<"I'm a factor of "<<m_Domain.size()
        <<" nodes. My Distribution type is Scalar.\n";
    dump()<<"My domain is\n";
    for( i = 0; i < m_Domain.size(); i++ )
    {
        dump()<<m_Domain[i]<<" ";
    }
    dump()<<"\n";
    m_CorrespDistribFun->Dump();
}
void CScalarPotential::GenerateSample( CEvidence* evidenceIO, int maximize ) const
{
    PNL_THROW(CAlgorithmicException, "undefined for scalar potential")
}
CPotential* CScalarPotential::ConvertStatisticToPot(int numOfSamples) const
{
    return NULL;
}

float CScalarPotential::GetLogLik( const CEvidence* pEv, const CPotential* pShrInfRes ) const
{
    PNL_THROW(CAlgorithmicException, "undefined for scalar potential")
}

CScalarPotential::CScalarPotential( const int *domain, int nNodes,
                                CModelDomain* pMD, const intVector& obsIndices ):
             CPotential( dtScalar, ftPotential, domain, nNodes, pMD, obsIndices )
{

}
CScalarPotential::CScalarPotential( const CScalarPotential &pScalarPotential ):
                CPotential( dtScalar, ftPotential,
                &pScalarPotential.m_Domain.front(),
                pScalarPotential.m_Domain.size(),
                pScalarPotential.GetModelDomain(),
                pScalarPotential.m_obsPositions )
{
    m_Domain = intVector( pScalarPotential.m_Domain );
    delete m_CorrespDistribFun;
    m_CorrespDistribFun = CScalarDistribFun::Copy( 
        static_cast<CScalarDistribFun*>(pScalarPotential.m_CorrespDistribFun ));
}

CScalarPotential::CScalarPotential( const CScalarPotential* pScalarPotential ):
CPotential(pScalarPotential)
{
    
}

#ifdef PAR_PNL
void CScalarPotential::UpdateStatisticsML(CFactor *pPot)
{
    PNL_THROW(CNotImplemented, 
        "UpdateStatisticsML for CScalarPotential not implemented yet");
};
#endif // PAR_OMP

#ifdef PNL_RTTI
const CPNLType CScalarPotential::m_TypeInfo = CPNLType("CScalarPotential", &(CPotential::m_TypeInfo));

#endif