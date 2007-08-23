/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlObjHandler.cpp                                           //
//                                                                         //
//  Purpose:   Object handlers for context                                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlObjHandler.hpp"
#include "pnlPersistence.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlContextLoad.hpp"
#include "pnlContextSave.hpp"
#include "pnlString.hpp"

PNL_USING

void
CObjHandlerForPersistence::CallByName(pnlString &name, CPNLBase *pObj, CContext *pContext)
{
    PNL_CHECK_FOR_ZERO(name.size());
    CPersistence *pPersist = m_pZoo->ObjectBySignature(name);
    if(pPersist)
    {
        Run(pPersist, pObj, pContext);
    } else 
    {
	pnlString msg;
	msg << "No saver/loader has been created with signature: " << name;
	PNL_THROW(CBadArg, msg);
    }
}

bool CObjHandlerForPersistence::GetClassName(pnlString *pName, CPNLBase *pObj)
{
    return m_pZoo->GetClassName(pName, pObj);
}

void CObjSaver::Run(CPersistence *pPersist, CPNLBase *pObj, CContext *pContext)
{
    CContextSave *pContextSave = static_cast<CContextSave*>(pContext);

    pPersist->Save(pObj, pContextSave);
    pContextSave->PlanForWriting();
}

void CObjSaver::CallByNameForDup(pnlString &name, CPNLBase *pObj, CContext *pContext)
{
    static_cast<CContextSave*>(pContext)->PlanForWriting();
}

void CObjInclusionEnumerator::Run(CPersistence *pPersist, CPNLBase *pObj, CContext *pContext)
{
    pPersist->TraverseSubobject(pObj, pContext);
}

void CObjLoader::Run(CPersistence *pPersist, CPNLBase *pObj, CContext *pContext)
{
    CContextLoad *pContextLoad = static_cast<CContextLoad*>(pContext);

    m_pObject = pPersist->Load(pContextLoad);
}
