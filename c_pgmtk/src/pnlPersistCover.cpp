/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistCovers.cpp                                        //
//                                                                         //
//  Purpose:   Covers (wrappers) for different types                       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlPersistCover.hpp"
#include "pnlNodeType.hpp"

PNL_USING

static void SaveNodeType(pnlString *pStr, const CNodeType &ntype, bool bLast)
{
    *pStr << ntype.IsDiscrete() << ":" << ntype.GetNodeSize();
    if(ntype.GetNodeState())
    {
	*pStr << ":" << ntype.GetNodeState();
    }
    if(!bLast)
    {
	*pStr << ',';
    }
}

static void LoadNodeType(CNodeType *pNodeType, std::istringstream *pStr, bool bLast)
{
    int nodeSize;
    bool bDiscrete;
    int nodeState = 0;
    char ch;

    *pStr >> bDiscrete;
    *pStr >> ch;
    ASSERT(ch == ':');
    *pStr >> nodeSize;
    *pStr >> ch;
    if(ch == ':')
    {
	*pStr >> nodeState >> ch;
    }
    *pNodeType = CNodeType(bDiscrete, nodeSize, (EIDNodeState)nodeState);
    ASSERT(bLast || ch == ',');
}

void
CPersistNodeTypeVector::Save(CPNLBase *pObj, CContextSave *pContext)
{
    nodeTypeVector *paNodeType = static_cast<CCover<nodeTypeVector>*>(pObj)->GetPointer();
    pnlString buf;

    for(int i = 0; i < paNodeType->size(); ++i)
    {
	SaveNodeType(&buf, paNodeType[0][i], i >= paNodeType->size() - 1);
    }

    pContext->AddAttribute("NumberOfNodes", int(paNodeType->size()));
    pContext->AddText(buf.c_str());
}

CPNLBase *
CPersistNodeTypeVector::Load(CContextLoad *pContext)
{
    nodeTypeVector *paNodeType = new nodeTypeVector();
    pnlString text;
    int nNode;
    CNodeType ntype;
    
    pContext->GetText(text);

    std::istringstream buf(text.c_str());

    pContext->GetAttribute(&nNode, "NumberOfNodes");
    paNodeType->reserve(nNode);
    for(int i = 0; i < nNode; ++i)
    {
	LoadNodeType(&ntype, &buf, i == (nNode - 1));
        paNodeType->push_back(ntype);
    }

    CCover<nodeTypeVector> *result = new CCover<nodeTypeVector>(paNodeType);
    pContext->AutoDelete(result);

    return result;
}

bool CPersistNodeTypeVector::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CCover<nodeTypeVector>*>(pObj) != 0;
}


void
CPersistPNodeTypeVector::Save(CPNLBase *pObj, CContextSave *pContext)
{
    pNodeTypeVector *paPNodeType = static_cast<CCover<pNodeTypeVector>*>(pObj)->GetPointer();
    pnlString buf;

    for(int i = 0; i < paPNodeType->size(); ++i)
    {
	SaveNodeType(&buf, paPNodeType[0][i][0], i >= paPNodeType->size() - 1);
    }

    pContext->AddAttribute("NumberOfNodes", int(paPNodeType->size()));
    pContext->AddText(buf.c_str());
}

CPNLBase *
CPersistPNodeTypeVector::Load(CContextLoad *pContext)
{
    pNodeTypeVector *paPNodeType = new pNodeTypeVector;
    pnlString text;
    int nNode;
    CNodeType ntype;
    
    pContext->GetText(text);

    std::istringstream buf(text.c_str());

    pContext->GetAttribute(&nNode, "NumberOfNodes");
    paPNodeType->reserve(nNode);
    for(int i = 0; i < nNode; ++i)
    {
	LoadNodeType(&ntype, &buf, i == (nNode - 1));
        paPNodeType->push_back(new CNodeType(ntype));
    }

    CCover<pNodeTypeVector> *result = new CCover<pNodeTypeVector>(paPNodeType);
    pContext->AutoDelete(result);

    return result;
}

bool CPersistPNodeTypeVector::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CCover<pNodeTypeVector>*>(pObj) != 0;
}

#ifdef PNL_RTTI
const CPNLType CCoverGen::m_TypeInfo = CPNLType("CCoverGen", &(CPNLBase::m_TypeInfo));

template<> 
const CPNLType & CCover<int>::GetStaticTypeInfo()
{
  return CCover<int>::m_TypeInfo;
}

template<> 
const CPNLType & CCoverDel<int>::GetStaticTypeInfo()
{
  return CCoverDel<int>::m_TypeInfo;
}
#endif