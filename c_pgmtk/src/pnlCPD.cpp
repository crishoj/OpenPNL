/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlCPD.cpp                                                  //
//                                                                         //
//  Purpose:   CCPD class member functions implementation                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// CPD.cpp: implementation of the CCPD class.
#include "pnlConfig.hpp"
#include "pnlCPD.hpp"
#include "pnlDistribFun.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlException.hpp"
#include "pnlInferenceEngine.hpp"

PNL_USING

CCPD::CCPD(EDistributionType dt,
				 EFactorType pt, const int *domain, int nNodes,
                 CModelDomain* pMD)  
				 : CFactor( dt, pt, domain, nNodes, pMD)
{
}

CCPD::CCPD( EDistributionType dt, EFactorType ft, CModelDomain* pMD )
					:CFactor( dt, ft, pMD )
{

}

CCPD::CCPD(const CCPD* cpd ):CFactor(cpd)
{
}

/*
float CCPD::ProcessingStatisticalData( int numberOfEvidences )
{
    return m_CorrespDistribFun->ProcessingStatisticalData( 
        static_cast<float>(numberOfEvidences) );
}*/


CPotential* CCPD::ConvertWithEvidenceToPotential(const CEvidence* pEvidence, 
                    int flagSumOnMixtureNode )const
{

    //need to convert to potential and after that add evidence
    CPotential* potWithoutEv = ConvertToPotential();
    CPotential* potWithEvid = potWithoutEv->ShrinkObservedNodes(pEvidence);
    delete potWithoutEv;
    return potWithEvid;
}

#ifdef PNL_RTTI
const CPNLType CCPD::m_TypeInfo = CPNLType("CCPD", &(CFactor::m_TypeInfo));

#endif
