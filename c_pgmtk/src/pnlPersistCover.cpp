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

void
CPersistNodeTypeVector::Save(CPNLBase *pObj, CContextSave *pContext)
{
    nodeTypeVector *paNodeType = static_cast<CCover<nodeTypeVector>*>(pObj)->GetPointer();
    std::stringstream buf;

    for(int i = 0; i < paNodeType->size(); ++i)
    {
      buf << paNodeType[0][i].IsDiscrete() << ":"
        << paNodeType[0][i].GetNodeSize() << ":"
        << paNodeType[0][i].GetNodeState();
      if(i < paNodeType->size() - 1)
      {
        buf << ',';
      }
    }

    pContext->AddAttribute("NumberOfNodes", int(paNodeType->size()));
    pContext->AddText(buf.str().c_str());
}

CPNLBase *
CPersistNodeTypeVector::Load(CContextLoad *pContext)
{
    nodeTypeVector *paNodeType = new nodeTypeVector;
    std::string text;
    int nNode;
    int nodeSize;
    bool bDiscrete;
    int nodeState;
    char ch;
    
    pContext->GetText(text);

    std::istringstream buf(text);

    pContext->GetAttribute(&nNode, "NumberOfNodes");
    paNodeType->reserve(nNode);
    for(int i = 0; i < nNode; ++i)
    {
      buf >> bDiscrete;
      buf >> ch;
      ASSERT(ch == ':');
      buf >> nodeSize;
      buf >> ch;
      ASSERT(ch == ':');
      buf >> nodeState;
      paNodeType->push_back(CNodeType(bDiscrete, nodeSize, (EIDNodeState)nodeState));
      buf >> ch;
      ASSERT(ch == ',');
    }

    return new CCover<nodeTypeVector>(paNodeType);
}

void
CPersistPNodeTypeVector::Save(CPNLBase *pObj, CContextSave *pContext)
{
    pNodeTypeVector *paPNodeType = static_cast<CCover<pNodeTypeVector>*>(pObj)->GetPointer();
    std::stringstream buf;

    for(int i = 0; i < paPNodeType->size(); ++i)
    {
      buf << paPNodeType[0][i]->IsDiscrete() << ":"
        << paPNodeType[0][i]->GetNodeSize() << ":"
        << paPNodeType[0][i]->GetNodeState();
      if(i < paPNodeType->size() - 1)
      {
        buf << ',';
      }
    }

    pContext->AddAttribute("NumberOfNodes", int(paPNodeType->size()));
    pContext->AddText(buf.str().c_str());
}

CPNLBase *
CPersistPNodeTypeVector::Load(CContextLoad *pContext)
{
    pNodeTypeVector *paPNodeType = new pNodeTypeVector;
    std::string text;
    int nNode;
    int nodeSize;
    bool bDiscrete;
    int nodeState;
    char ch;
    
    pContext->GetText(text);

    std::istringstream buf(text);

    pContext->GetAttribute(&nNode, "NumberOfNodes");
    paPNodeType->reserve(nNode);
    for(int i = 0; i < nNode; ++i)
    {
      buf >> bDiscrete;
      buf >> ch;
      ASSERT(ch == ':');
      buf >> nodeSize;
      buf >> ch;
      ASSERT(ch == ':');
      buf >> nodeState;
      paPNodeType->push_back(new CNodeType(bDiscrete, nodeSize, (EIDNodeState)nodeState));
      buf >> ch;
      ASSERT(ch == ',');
    }

    return new CCover<pNodeTypeVector>(paPNodeType);
}

