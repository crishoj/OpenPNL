/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistCPD.cpp                                           //
//                                                                         //
//  Purpose:   Saving/Loading of CPDs                                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include <ctype.h>
#include "pnlContext.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlPersistCPD.hpp"
#include "pnlPersistCover.hpp"
#include "pnlFactor.hpp"
#include "pnlDistribFun.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlScalarPotential.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlSoftMaxCPD.hpp"

PNL_USING

void
CPersistFactor::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CFactor *pFactor = dynamic_cast<CFactor*>(pObj);
    intVector *pVec = new intVector;
    
    pContext->Put(pFactor->GetModelDomain(), "ModelDomain");
    pFactor->GetDomain(pVec);
    pContext->Put(new CCoverDel<intVector>(pVec), "Domain", true);

    pFactor->GetObsPositions(pVec = new intVector);
    if(!dynamic_cast<CCPD*>(pFactor))
    {
	pContext->Put(new CCoverDel<intVector>(pVec), "ObservedPositions", true);
    }
    else if(pFactor->GetDistributionType() == dtMixGaussian)
    {
	CMixtureGaussianCPD *pM = static_cast<CMixtureGaussianCPD*>(pFactor);
	floatVector *pProbabilities = new floatVector;

	pM->GetProbabilities(pProbabilities);
	pContext->Put(new CCoverDel<floatVector>(pProbabilities), "Probabilities", true);
    }

    pContext->Put(pFactor->GetDistribFun(), "DistributionFunction");
}

void
CPersistFactor::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CFactor *pFactor = dynamic_cast<CFactor*>(pObj);

    pContext->AddAttribute("FactorType", pFactor->GetFactorType());
    pContext->AddAttribute("DistributionType", pFactor->GetDistributionType());
}

CPNLBase *
CPersistFactor::Load(CContextLoad *pContext)
{
    CFactor *pFactor = 0;
    intVector *pDomain = static_cast<CCover<intVector>*>(pContext->Get("Domain"))->GetPointer();
    CDistribFun *pDistribFun = static_cast<CDistribFun*>(pContext->Get("DistributionFunction"));

    CModelDomain *pMD = static_cast<CModelDomain*>(pContext->Get("ModelDomain"));
    EFactorType factorType;
    EDistributionType distrType;

    pContext->GetAttribute((int*)&factorType, "FactorType");
    pContext->GetAttribute((int*)&distrType, "DistributionType");

    switch(factorType)
    {
    case ftPotential:
	{
	    intVector *pObsPositions = static_cast<CCover<intVector>*>(
		pContext->Get("ObservedPositions"))->GetPointer();
	    
	    switch(distrType)
	    {
	    case dtTabular:
		pFactor = CTabularPotential::Create(*pDomain, pMD, 0, *pObsPositions);
		break;
	    case dtGaussian:
		pFactor = CGaussianPotential::Create(*pDomain, pMD, -1,
		    floatVector(), floatVector(), 0.0, *pObsPositions);
		break;
	    case dtScalar:
		pFactor = CScalarPotential::Create(*pDomain, pMD, *pObsPositions);
		break;
	    }
	}
	break;

    case ftCPD:
        switch(distrType)
        {
        case dtTabular:
	    pFactor = CTabularCPD::Create(*pDomain, pMD);
	    break;
        case dtGaussian:
	    pFactor = CGaussianCPD::Create(*pDomain, pMD);
	    break;
        case dtSoftMax:
            pFactor = CSoftMaxCPD::Create(*pDomain, pMD);
            break;
        case dtMixGaussian:
	    floatVector *pX = static_cast<CCover<floatVector>*>(
		pContext->Get("Probabilities"))->GetPointer();
	    pFactor = CMixtureGaussianCPD::Create(*pDomain, pMD, *pX);
	    break;
        }
    }

    if(!pFactor)
    {
	PNL_THROW(CInternalError, "Unknown Factor type");
    }

    pFactor->SetDistribFun(pDistribFun);

    return pFactor;
}

bool CPersistFactor::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CFactor*>(pObj) != 0;
}
