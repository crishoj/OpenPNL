/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlContext.cpp                                              //
//                                                                         //
//  Purpose:   Base class for traversal classes                            //
//             (for example saving/loading classes)                        //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include <sstream>
#include "pnlContext.hpp"
#include "pnlObject.hpp"
#include "pnlObjHandler.hpp"
#include "pnlPersistCover.hpp"
#include "pnlGraph.hpp"
#include "pnlDBN.hpp"
#include "pnlMRF2.hpp"
#include "pnlIDNet.hpp"
#include "pnlNodeValues.hpp"
#include "pnlEvidence.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlCondGaussianDistribFun.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlScalarDistribFun.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlCondSoftMaxDistribFun.hpp"

#ifdef DEBUG_PERSISTENCE
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_NEW new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
#define new DEBUG_NEW
#endif

PNL_USING

CContext::CContext(): m_pObjectHandler(0), m_bEnumeration(true), m_iAccelTree(-1)
{
    m_Indices.reserve(4);
    //m_Indices.push_back(TreeLeafIndex(&m_Tree, 0, 0));
}

void CContext::Put(CPNLBase *pObj, const char *name, bool bAutoDelete)
{
    int iTree;
    
    if(!m_Indices.size())
    {
        iTree = 0;
        if(!m_Tree.size())
        {
            m_Tree.reserve(8);
            m_Tree.resize(1);
            m_Tree[0].reserve(4);
        }
    }
    else if((iTree = Current().m_iSubTree) == -1)
    {
        iTree = Current().m_iSubTree = m_Tree.size();
        m_Tree.resize(iTree + 1);
        m_Tree[iTree].reserve(4);
    }

    m_iAccelTree = -1;
    m_Tree[iTree].push_back(TreeEntry(name, -1, pObj));
    if(bAutoDelete)
    {
        AutoDelete(pObj);
    }
}

CPNLBase *CContext::Get(const char *name)
{
    pnlString nameStr(name);
    int iTree = m_Indices.size() ? Current().m_iSubTree:0;
    int i;

    if(iTree < 0)
    {
        PNL_THROW(CInternalError, "Request of subobjects for object without child");
        return 0;
    }

    if(m_Tree[iTree].size() > 32)
    {
        if(m_iAccelTree != iTree)
        {
            // build accelerator
            m_Accelerator.clear();
            for(i = m_Tree[iTree].size(); --i >= 0; )
            {
                m_Accelerator[m_Tree[iTree][i].m_Name] = i;
            }
            m_iAccelTree = iTree;
        }

        std::map<pnlString, int>::iterator it = m_Accelerator.find(nameStr);
        if(it != m_Accelerator.end())
        {
            i = (*it).second;
            const TreeEntry &rEnt = m_Tree[iTree][i];
            
            return rEnt.m_pObject ? rEnt.m_pObject:rEnt.m_PathToReplacing.back()().m_pObject;
        }

    }
    else
    {
        for(i = 0; i < m_Tree[iTree].size(); ++i)
        {
            if(m_Tree[iTree][i].m_Name == name)
            {
                const TreeEntry &rEnt = m_Tree[iTree][i];
                
                return rEnt.m_pObject ? rEnt.m_pObject:rEnt.m_PathToReplacing.back()().m_pObject;
            }
        }
    }

    return 0;
}

static const char *GetClassName(CPNLBase *pObj)
{
#ifdef PNL_RTTI
    if (pObj->GetTypeInfo() == CGraph::GetStaticTypeInfo())
    {
        return "Graph";
    }
    else if(pObj->GetTypeInfo() == CGraphicalModel::GetStaticTypeInfo())
    {
        if(pObj->GetTypeInfo() == CDBN::GetStaticTypeInfo())
        {
            return "DBN";
        }
        else if(pObj->GetTypeInfo() == CIDNet::GetStaticTypeInfo())
        {
            return "IDNet";
        }
        else if(pObj->GetTypeInfo() == CBNet::GetStaticTypeInfo())
        {
            return "BNet";
        }
        else if(pObj->GetTypeInfo() == CMRF2::GetStaticTypeInfo())
        {
            return "MRF2";
        }
        else if(pObj->GetTypeInfo() == CMNet::GetStaticTypeInfo())
        {
            return "MNet";
        }
    }
    else if(pObj->GetTypeInfo() == CFactor::GetStaticTypeInfo())
    {
        return "Factor";
    }
    else if(pObj->GetTypeInfo() == CNodeType::GetStaticTypeInfo())
    {
        return "NodeType";
    }
    else if(pObj->GetTypeInfo() == CNodeValues::GetStaticTypeInfo())
    {
        if(pObj->GetTypeInfo() == CEvidence::GetStaticTypeInfo())
        {
            return "Evidence";
        }
        else
        {
            return "NodeValues";
        }
    }
    
    
    else if(dynamic_cast<CCoverGen*>(pObj) != 0)
    {
        if(dynamic_cast<CCover<intVector>*>(pObj) != 0)
        {
            return "intVector";
        }
        else if(dynamic_cast<CCover<intVecVector>*>(pObj) != 0)
        {
            return "intVecVector";
        }
        else if(dynamic_cast<CCover<floatVector>*>(pObj) != 0)
        {
            return "floatVector";
        }
        else if(dynamic_cast<CCover<nodeTypeVector>*>(pObj) != 0)
        {
            return "NodeTypeVector";
        }
        else if(dynamic_cast<CCover<pNodeTypeVector>*>(pObj) != 0)
        {
            return "PNodeTypeVector";
        }
        else if(dynamic_cast<CCover<valueVector>*>(pObj) != 0)
        {
            return "ValueVector";
        }
        else
        {
            return "";
        }
    }
    else if(dynamic_cast<CDistribFun*>(pObj) != 0)
    {
        if(dynamic_cast<CGaussianDistribFun*>(pObj) != 0)
        {
            return "GaussianDistribFun";
        }
        else if(dynamic_cast<CTabularDistribFun*>(pObj) != 0)
        {
            return "TabularDistribFun";
        }
        else if(dynamic_cast<CSoftMaxDistribFun*>(pObj) != 0)
        {
            return "SoftMaxDistribFun";
        }
        else if(dynamic_cast<CCondGaussianDistribFun*>(pObj) != 0)
        {
            return "ConditionalGaussianDistribFun";
        }
        else if(dynamic_cast<CCondSoftMaxDistribFun*>(pObj) != 0)
        {
            return "ConditionalSoftMaxDistribFun";
        }
        else if(dynamic_cast<CScalarDistribFun*>(pObj) != 0)
        {
            return "ScalarDistribFun";
        }
    }
    else if(dynamic_cast<CMatrix<float>*>(pObj) != 0)
    {
        return "MatrixOfFloat";
    }
    else if(dynamic_cast<CMatrix<CDistribFun*>*>(pObj) != 0
        || dynamic_cast<CMatrix<CGaussianDistribFun*>*>(pObj) != 0
        || dynamic_cast<CMatrix<CSoftMaxDistribFun*>*>(pObj) != 0)
    {
        return "MatrixDistribFun";
    }
    else if(dynamic_cast<CModelDomain*>(pObj) != 0)
    {
        return "ModelDomain";
    }
    
    return "";
    // THROW(UNHANDLED TYPE);??
#else
    if(dynamic_cast<CGraph*>(pObj) != 0)
    {
        return "Graph";
    }
    else if(dynamic_cast<CGraphicalModel*>(pObj) != 0)
    {
        if(dynamic_cast<CDBN*>(pObj) != 0)
        {
            return "DBN";
        }
        else if(dynamic_cast<CIDNet*>(pObj) != 0)
        {
            return "IDNet";
        }
        else if(dynamic_cast<CBNet*>(pObj) != 0)
        {
            return "BNet";
        }
        else if(dynamic_cast<CMRF2*>(pObj) != 0)
        {
            return "MRF2";
        }
        else if(dynamic_cast<CMNet*>(pObj) != 0)
        {
            return "MNet";
        }
    }
    else if(dynamic_cast<CFactor*>(pObj) != 0)
    {
        return "Factor";
    }
    else if(dynamic_cast<CNodeType*>(pObj) != 0)
    {
        return "NodeType";
    }
    else if(dynamic_cast<CNodeValues*>(pObj) != 0)
    {
        if(dynamic_cast<CEvidence*>(pObj) != 0)
        {
            return "Evidence";
        }
        else
        {
            return "NodeValues";
        }
    }
    else if(dynamic_cast<CCoverGen*>(pObj) != 0)
    {
        if(dynamic_cast<CCover<intVector>*>(pObj) != 0)
        {
            return "intVector";
        }
        else if(dynamic_cast<CCover<intVecVector>*>(pObj) != 0)
        {
            return "intVecVector";
        }
        else if(dynamic_cast<CCover<floatVector>*>(pObj) != 0)
        {
            return "floatVector";
        }
        else if(dynamic_cast<CCover<nodeTypeVector>*>(pObj) != 0)
        {
            return "NodeTypeVector";
        }
        else if(dynamic_cast<CCover<pNodeTypeVector>*>(pObj) != 0)
        {
            return "PNodeTypeVector";
        }
        else if(dynamic_cast<CCover<valueVector>*>(pObj) != 0)
        {
            return "ValueVector";
        }
        else
        {
            return "";
        }
    }
    else if(dynamic_cast<CDistribFun*>(pObj) != 0)
    {
        if(dynamic_cast<CGaussianDistribFun*>(pObj) != 0)
        {
            return "GaussianDistribFun";
        }
        else if(dynamic_cast<CTabularDistribFun*>(pObj) != 0)
        {
            return "TabularDistribFun";
        }
        else if(dynamic_cast<CSoftMaxDistribFun*>(pObj) != 0)
        {
            return "SoftMaxDistribFun";
        }
        else if(dynamic_cast<CCondGaussianDistribFun*>(pObj) != 0)
        {
            return "ConditionalGaussianDistribFun";
        }
        else if(dynamic_cast<CCondSoftMaxDistribFun*>(pObj) != 0)
        {
            return "ConditionalSoftMaxDistribFun";
        }
        else if(dynamic_cast<CScalarDistribFun*>(pObj) != 0)
        {
            return "ScalarDistribFun";
        }
    }
    else if(dynamic_cast<CMatrix<float>*>(pObj) != 0)
    {
        return "MatrixOfFloat";
    }
    else if(dynamic_cast<CMatrix<CDistribFun*>*>(pObj) != 0
        || dynamic_cast<CMatrix<CGaussianDistribFun*>*>(pObj) != 0
        || dynamic_cast<CMatrix<CSoftMaxDistribFun*>*>(pObj) != 0)
    {
        return "MatrixDistribFun";
    }
    else if(dynamic_cast<CModelDomain*>(pObj) != 0)
    {
        return "ModelDomain";
    }
    
    return "";
    // THROW(UNHANDLED TYPE);??
#endif
}

void CContext::Traverse(int iTree)
{
    ASSERT(m_pObjectHandler != 0);

    for(int i = 0; i < m_Tree[iTree].size(); ++i)
    {
        CPNLBase *pObj;
        pnlString objTypeName;

        m_Indices.push_back(TreeLeafIndex<TreeEntry>(m_Tree, iTree, i));
        GetObjWithObjTypeName(&pObj, &objTypeName, m_Tree[iTree][i]);

        if(pObj)
        {
	    objTypeName.assign(GetClassName(pObj));
	    if(!objTypeName.length())
	    {
		m_pObjectHandler->GetClassName(&objTypeName, pObj);
	    }
            if(!objTypeName.length())
            {
                continue;
            }
        }
	else if(!objTypeName.length()) // unknown object during loading
	{
	    continue;
	}

        if(IsEnumeration())
        {
            CPNLBase *pReal = pObj;
            CCoverGen *pCover = dynamic_cast<CCoverGen*>(pObj);
            if(pCover)
            {
                if(pCover->nObject() == 1)
                {
                    pReal = pCover->GetBaseObj();
                }
                else
                {
                    PNL_THROW(CNotImplemented, "Improve CCover for several objects");
                }
            }
            if(m_MapUniqueObj.find(pReal) == m_MapUniqueObj.end())
            {
                m_MapUniqueObj[pReal] = m_HeapUniqueObj.size();
                m_HeapUniqueObj.push_back(m_Indices);
            }
            else
            {
                m_Tree[iTree][i].m_pObject = 0;
                m_Tree[iTree][i].m_PathToReplacing = m_HeapUniqueObj[m_MapUniqueObj[pReal]];
                m_Tree[iTree][i].m_bDup = true;
            }
        }

        BeginTraverseObject(objTypeName, m_Tree[iTree][i]);
        if(!m_Tree[iTree][i].m_bDup)
        {
            if(HandleObjectBeforeInterior())
            {
                m_pObjectHandler->CallByName(objTypeName, pObj, this);
            }

            if(m_Tree[iTree][i].m_iSubTree > 0)
            {
                Traverse(m_Tree[iTree][i].m_iSubTree);
            }

            if(!HandleObjectBeforeInterior())
            {
                m_pObjectHandler->CallByName(objTypeName, pObj, this);
            }
        }
        else
        {
            m_pObjectHandler->CallByNameForDup(objTypeName, pObj, this);
        }
        EndTraverseObject(objTypeName, m_Tree[iTree][i]);
        m_Indices.pop_back();
    }
}

void CContext::BeginTraverse(bool bEnumeration)
{
    m_bEnumeration = bEnumeration;
    m_Indices.resize(0);
    Traverse(0);
    m_bEnumeration = true;
}
