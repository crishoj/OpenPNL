/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistModels.cpp                                        //
//                                                                         //
//  Purpose:   Saving/Loading of Graphical Models                          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlContext.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlPersistModel.hpp"
#include "pnlPersistCover.hpp"
#include "pnlBNet.hpp"
#include "pnlMRF2.hpp"
#include "pnlDBN.hpp"
#include "pnlIDNet.hpp"

PNL_USING

static void
TraverseSubobjectOfGrModel(CGraphicalModel *pObj, CContext *pContext)
{
    if(!pObj->IsValid())
    {
	PNL_THROW(CInvalidOperation, "Model is invalid")
    }

    pContext->Put(pObj->GetModelDomain(), "ModelDomain");

    int nFactors = pObj->GetNumberOfFactors();
    for(int i = 0; i < nFactors; ++i)
    {
	std::stringstream name;
	CFactor *pFactor = pObj->GetFactor(i);

	name << "Factor" << i;
	pContext->Put(pFactor, name.str().c_str());
    }
}

static void
SaveGrModel(CGraphicalModel *pModel, CContextSave *pContext)
{
    PNL_CHECK_IS_NULL_POINTER(pModel);
    pContext->AddAttribute("NumberOfFactors", pModel->GetNumberOfFactors());
}

static void
LoadForGrModel(CGraphicalModel *pModel, CContextLoad *pContext)
{
    int nFactor;
    
    pContext->GetAttribute(&nFactor, "NumberOfFactors");
    for(int i = 0; i < nFactor; ++i)
    {
	std::stringstream name;

	name << "Factor" << i;

	CFactor *pFactor = static_cast<CFactor *>(pContext->Get(name.str().c_str()));

	pModel->AttachFactor(pFactor);
    }
}


// BNet

void
CPersistBNet::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CBNet *pModel = dynamic_cast<CBNet*>(pObj);

    pContext->Put(pModel->GetGraph(), "Graph");
    TraverseSubobjectOfGrModel(pModel, pContext);
}

void
CPersistBNet::Save(CPNLBase *pObj, CContextSave *pContext)
{
    SaveGrModel(dynamic_cast<CBNet*>(pObj), pContext);
}

CPNLBase *
CPersistBNet::Load(CContextLoad *pContext)
{
    CGraph *pGraph = static_cast<CGraph *>(pContext->Get("Graph"));
    CModelDomain *pMD = static_cast<CModelDomain *>(pContext->Get("ModelDomain"));

    CGraphicalModel *pModel = CBNet::Create(pGraph, pMD);
    LoadForGrModel(pModel, pContext);

    return pModel;
}

bool CPersistBNet::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CBNet*>(pObj) != 0;
}


// IDNet

void
CPersistIDNet::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CIDNet *pModel = dynamic_cast<CIDNet*>(pObj);

    pContext->Put(pModel->GetGraph(), "Graph");
    TraverseSubobjectOfGrModel(pModel, pContext);
}

void
CPersistIDNet::Save(CPNLBase *pObj, CContextSave *pContext)
{
    SaveGrModel(dynamic_cast<CIDNet*>(pObj), pContext);
}

CPNLBase *
CPersistIDNet::Load(CContextLoad *pContext)
{
    CGraph *pGraph = static_cast<CGraph *>(pContext->Get("Graph"));
    CModelDomain *pMD = static_cast<CModelDomain *>(pContext->Get("ModelDomain"));

    CGraphicalModel *pModel = CIDNet::Create(pGraph, pMD);
    LoadForGrModel(pModel, pContext);

    return pModel;
}

bool CPersistIDNet::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CIDNet*>(pObj) != 0;
}


// DBN

void
CPersistDBN::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    pContext->Put(dynamic_cast<CDBN*>(pObj)->GetStaticModel(), "StaticModel");
}

void
CPersistDBN::Save(CPNLBase *pObj, CContextSave *pContext)
{
}

CPNLBase *
CPersistDBN::Load(CContextLoad *pContext)
{
    return CDBN::Create(static_cast<CStaticGraphicalModel*>(pContext->Get("StaticModel")));
}

bool CPersistDBN::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CDBN*>(pObj) != 0;
}


// MNet

static void
TraverseSubobjectsForMNet(CMNet *pModel, CContext *pContext)
{
    TraverseSubobjectOfGrModel(pModel, pContext);
    intVecVector *paClique = new intVecVector;
    intVector clique;

    for(int i = 0; i < pModel->GetNumberOfCliques(); ++i)
    {
	pModel->GetClique(i, &clique);
	paClique->push_back(clique);
    }
    
    pContext->Put(new CCoverDel<intVecVector>(paClique), "Cliques", true);
}

void
CPersistMNet::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    TraverseSubobjectsForMNet(dynamic_cast<CMNet*>(pObj), pContext);
}

void
CPersistMNet::Save(CPNLBase *pObj, CContextSave *pContext)
{
    SaveGrModel(dynamic_cast<CMNet*>(pObj), pContext);
}

CPNLBase *
CPersistMNet::Load(CContextLoad *pContext)
{
    CModelDomain *pMD = static_cast<CModelDomain *>(pContext->Get("ModelDomain"));
    intVecVector *paClique = static_cast<CCover<intVecVector> *>(
	pContext->Get("Cliques"))->GetPointer();

    CMNet *pModel = CMNet::Create(*paClique, pMD);
    LoadForGrModel(pModel, pContext);

    return pModel;
}

bool CPersistMNet::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CMNet*>(pObj) != 0;
}


// MRF2

void
CPersistMRF2::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    TraverseSubobjectsForMNet(dynamic_cast<CMRF2*>(pObj), pContext);
}

void
CPersistMRF2::Save(CPNLBase *pObj, CContextSave *pContext)
{
    SaveGrModel(dynamic_cast<CMRF2*>(pObj), pContext);
}

CPNLBase *
CPersistMRF2::Load(CContextLoad *pContext)
{
    CModelDomain *pMD = static_cast<CModelDomain *>(pContext->Get("ModelDomain"));
    intVecVector *paClique = static_cast<CCover<intVecVector> *>(
	pContext->Get("Cliques"))->GetPointer();
    CMRF2 *pModel = CMRF2::Create(*paClique, pMD);

    LoadForGrModel(pModel, pContext);

    return pModel;
}

bool CPersistMRF2::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CMRF2*>(pObj) != 0;
}
