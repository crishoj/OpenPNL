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


PNL_USING

void
CObjHandlerForPersistence::CallByName(std::string &name, CPNLBase *pObj, CContext *pContext)
{
    CPersistence *pPersist = m_pZoo->Function(name);
    if(pPersist)
    {
        Run(pPersist, pObj, pContext);
    }
    else
    {
	int i;
	char t[10];

	for(i = 0; i < 10; ++i)
	{
	    t[i] = name[i];
	}
    }
    // else throw?
}


void CObjSaver::Run(CPersistence *pPersist, CPNLBase *pObj, CContext *pContext)
{
    CContextSave *pContextSave = static_cast<CContextSave*>(pContext);

    pPersist->Save(pObj, pContextSave);
    pContextSave->PlanForWriting();
}

void CObjSaver::CallByNameForDup(std::string &name, CPNLBase *pObj, CContext *pContext)
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
    // ASSERT(pObj == NULL);

    m_pObject = pPersist->Load(pContextLoad);
}
