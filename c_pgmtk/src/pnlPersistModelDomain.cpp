/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistMD.cpp                                            //
//                                                                         //
//  Purpose:   Saving/Loading ModelDomain                                  //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include <ctype.h>
#include <sstream>
#include "pnlContext.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlPersistModelDomain.hpp"
#include "pnlPersistCover.hpp"
#include "pnlModelDomain.hpp"

PNL_USING

void
CPersistModelDomain::Save(CPNLBase *pObj, CContextSave *pContext)
{
}

CPNLBase *
CPersistModelDomain::Load(CContextLoad *pContext)
{
    nodeTypeVector *paNodeType = static_cast<CCover<nodeTypeVector> *>(
	pContext->Get("NodeTypes"))->GetPointer();
    intVector *paVarAssociation = static_cast<CCover<intVector> *>(
	pContext->Get("VarAssociations"))->GetPointer();

    CModelDomain *pMD = CModelDomain::Create(*paNodeType, *paVarAssociation);

    // factors will attached on Graphical Model creating

    return pMD;
}

void
CPersistModelDomain::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CModelDomain *pMD = dynamic_cast<CModelDomain*>(pObj);
    nodeTypeVector *paNodeType = new nodeTypeVector;
    intVector *paVarAssociation = new intVector;

    pMD->GetVariableAssociations(paVarAssociation);
    pMD->GetVariableTypes(paNodeType);

    pContext->Put(new CCoverDel<intVector>(paVarAssociation), "VarAssociations");
    pContext->Put(new CCoverDel<nodeTypeVector>(paNodeType), "NodeTypes");
}

bool CPersistModelDomain::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CModelDomain*>(pObj) != 0;
}
