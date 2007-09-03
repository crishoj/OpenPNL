/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistTypes.cpp                                         //
//                                                                         //
//  Purpose:   Saving/Loading                                              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlMatrix_impl.hpp" // since we'll use CMatrix<CDistribFun*>
#include <ctype.h>
#include <sstream>
#include "pnlContext.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlContextLoad.hpp"
#include "pnlContextSave.hpp"
#include "pnlGroup.hpp"
#include "pnlPersistCover.hpp"
#include "pnlPersistTypes.hpp"
#include "pnlNodeType.hpp"
#include "pnlNodeValues.hpp"
#include "pnlEvidence.hpp"
#include "pnlDistribFun.hpp"

#include "pnlXMLRead.hpp"

PNL_BEGIN

void
CPersistNodeType::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CNodeType *pNodeType = dynamic_cast<CNodeType*>(pObj);

    pContext->AddAttribute("NodeSize", pNodeType->GetNodeSize());
    pContext->AddAttribute("IsDiscrete", pNodeType->IsDiscrete());
    if(pNodeType->GetNodeState())
    {
        // if nodeState is absent, it loaded as 0
        pContext->AddAttribute("NodeState", (int)pNodeType->GetNodeState());
    }
}

CPNLBase *
CPersistNodeType::Load(CContextLoad *pContext)
{
    int nodeSize;
    bool bDiscrete;
    int nodeState = 0;
    
    pContext->GetAttribute(&nodeSize, "NodeSize");
    pContext->GetAttribute(&bDiscrete, "IsDiscrete");
    // if nodeState is absent, it will loaded as 0 - hint
    pContext->GetAttribute(&nodeState, "NodeState");

    return new CNodeType(bDiscrete, nodeSize, (EIDNodeState)nodeState);
}

bool CPersistNodeType::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CNodeType*>(pObj) != 0;
}

void
CPersistNodeValues::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CNodeValues *pValues = dynamic_cast<CNodeValues*>(pObj);
    valueVector aValue;

    pValues->GetRawData(&aValue);
    pContext->AddAttribute("NumberOfNodes", pValues->GetNumberObsNodes());
    pContext->AddAttribute("NumberOfValues", int(aValue.size()));
}

CPNLBase *
CPersistNodeValues::Load(CContextLoad *pContext)
{
    int nNode, nValue;

    pContext->GetAttribute(&nNode, "NumberOfNodes");
    pContext->GetAttribute(&nValue, "NumberOfValues");
        
    pNodeTypeVector *paNodeType = static_cast<CCoverDel<pNodeTypeVector>*>(
        pContext->Get("pNodeTypes"))->GetPointer();
    valueVector *paValue = static_cast<CCoverDel<valueVector>*>(
        pContext->Get("Values"))->GetPointer();

    return CNodeValues::Create(*(const pConstNodeTypeVector*)paNodeType, *paValue);
}

void
CPersistNodeValues::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CNodeValues *pValues = dynamic_cast<CNodeValues*>(pObj);
    valueVector *paValue = new valueVector;
    pNodeTypeVector *papNodeType = new pNodeTypeVector;
    
    {
        papNodeType->reserve(pValues->GetNumberObsNodes());
        CNodeType** aaNodeType = (CNodeType**)pValues->GetNodeTypes();
        papNodeType->assign(aaNodeType, aaNodeType + pValues->GetNumberObsNodes());
    }

    pValues->GetRawData(paValue);

    CCoverDel<valueVector> *pCovValues = new CCoverDel<valueVector>(paValue);
    CCoverDel<pNodeTypeVector> *pCov = new CCoverDel<pNodeTypeVector>(papNodeType);

    pContext->Put(pCovValues, "Values", true);
    pContext->Put(pCov, "pNodeTypes", true);
}

bool CPersistNodeValues::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CNodeValues*>(pObj) != 0;
}

// Evidence

void
CPersistEvidence::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CPersistNodeValues::TraverseSubobject(pObj, pContext);
    CEvidence *pEvidence = static_cast<CEvidence*>(pObj);
    intVector *pV = new intVector;
    
    pV->resize(pEvidence->GetNumberObsNodes());
    memcpy((void*)&pV->front(), pEvidence->GetAllObsNodes(),
        pEvidence->GetNumberObsNodes()*sizeof(pV->front()));

    pContext->Put(const_cast<CModelDomain*>(pEvidence->GetModelDomain()), "ModelDomain");
    pContext->Put(new CCoverDel<intVector>(pV), "ObservedNodes", true);
}

CPNLBase *CPersistEvidence::Load(CContextLoad *pContext)
{
    valueVector *paValue;
    if(static_cast<CCoverDel<valueVector>*>(pContext->Get("Values")) == 0)
    {
        paValue = new valueVector;
        paValue->resize(static_cast<CModelDomain*>(pContext->Get("ModelDomain"))->GetNumberVariables()/2);
    }
    else
    {
        paValue = static_cast<CCoverDel<valueVector>*>(
            pContext->Get("Values"))->GetPointer();
    }
    CModelDomain *pMD = static_cast<CModelDomain*>(pContext->Get("ModelDomain"));
    intVector *pV = static_cast<CCover<intVector>*>(pContext->Get("ObservedNodes"))->GetPointer();

    return CEvidence::Create(pMD, *pV, *paValue);
}

bool CPersistEvidence::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CEvidence*>(pObj) != 0;
}

// Matrix

// Numeric matrices

// Common methods for matrices
static struct
{
    EMatrixClass m_Class;
    const char *m_Name;
} sMapMatrixType[] =
{   mcSparse,       "Sparse"
,   mcDense,        "Dense"
,   mcNumericDense, "NumericDense"
,   mcNumericSparse,"NumericSparse"
,   mc2DNumericDense,  "2DNumericDense"
,   mc2DNumericSparse, "2DNumericSparse"
};

template<typename Type>
void MatrixCommonSave(CMatrix<Type>* pMat, CContextSave *pContext)
{
    int i, nDim;
    const int *ranges;

    pMat->GetRanges(&nDim, &ranges);
    pContext->AddAttribute("NumberOfDimensions", nDim);
    {
        pnlString buf;

        SaveArray<int>(buf, ranges, nDim);
        pContext->AddAttribute("Ranges", buf.c_str());
    }
    for(i = 0; i < sizeof(sMapMatrixType)/sizeof(sMapMatrixType[0]); ++i)
    {
        if(sMapMatrixType[i].m_Class == pMat->GetMatrixClass())
        {
            pContext->AddAttribute("MatrixClass", sMapMatrixType[i].m_Name);
            break;
        }
    }
    pContext->AddAttribute("IsClamped", pMat->GetClampValue());
}

struct CommonMatrixAttrs
{
    EMatrixClass m_Class;
    int m_nDim;
    int m_bClamp;
    intVector m_Ranges;
};

void MatrixCommonLoad(CommonMatrixAttrs *attrs, CContextLoad *pContext)
{
    int i;
    pnlString attr;

    pContext->GetAttribute(&attrs->m_nDim, "NumberOfDimensions");
    pContext->GetAttribute(attr, "MatrixClass");
    for(i = 0; i < sizeof(sMapMatrixType)/sizeof(sMapMatrixType[0]); ++i)
    {
        if(attr == sMapMatrixType[i].m_Name)
        {
            attrs->m_Class = sMapMatrixType[i].m_Class;
            break;
        }
    }
    pContext->GetAttribute(attr, "Ranges");
    {
        std::istringstream buf(attr.c_str());
        LoadArray<int>(buf, attrs->m_Ranges);
    }
    pContext->GetAttribute(&attrs->m_bClamp, "IsClamped");
}

void CPersistMatrixFlt::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CMatrix<float> *pMat = static_cast<CMatrix<float>*>(pObj);

    MatrixCommonSave<float>(pMat, pContext);
    pnlString buf;
    if(dynamic_cast<CDenseMatrix<float>*>(pMat))
    {
        CDenseMatrix<float> *pDenseMat = static_cast<CDenseMatrix<float>*>(pMat);
        int len;
        const float *pData;

        pDenseMat->GetRawData(&len, &pData);
        SaveArray<float>(buf, pData, len);
    }
    else
    {// Sparse
        CSparseMatrix<float> *pSparseMat = static_cast<CSparseMatrix<float>*>(pMat);
        CMatrixIterator<float> *it = pSparseMat->InitIterator();
        intVector index;

        for(; pSparseMat->IsValueHere(it); pSparseMat->Next(it))
        {
            pSparseMat->Index(it, &index);
            SaveArray<int>(buf, &index.front(), index.size());
            buf << ":" << *pSparseMat->Value(it) << '\n';
        }

        delete it;
    }

    pContext->AddText(buf.c_str());
}

CPNLBase *CPersistMatrixFlt::Load(CContextLoad *pContext)
{
    CommonMatrixAttrs attrs;
    pnlString text;

    pContext->GetText(text);
    MatrixCommonLoad(&attrs, pContext);
    if(attrs.m_Class == mcDense || attrs.m_Class == mcNumericDense
        || attrs.m_Class == mc2DNumericDense)
    {
        floatVector data;
        std::istringstream buf(text.c_str());

        LoadArray<float>(buf, data);
        switch(attrs.m_Class)
        {
        case mcDense:
            return CDenseMatrix<float>::Create(attrs.m_nDim, &attrs.m_Ranges.front(),
                &data.front(), attrs.m_bClamp);
        case mcNumericDense:
            return CNumericDenseMatrix<float>::Create(attrs.m_nDim,
                &attrs.m_Ranges.front(), &data.front(), attrs.m_bClamp);
        case mc2DNumericDense:
            return C2DNumericDenseMatrix<float>::Create(&attrs.m_Ranges.front(),
                &data.front(), attrs.m_bClamp);
        }
        PNL_THROW(CInvalidOperation, "Unknown matrix type");
    }

    CSparseMatrix<float> *pSparse;
    switch(attrs.m_Class)
    {
    case mcSparse:
        pSparse = CSparseMatrix<float>::Create(attrs.m_nDim, &attrs.m_Ranges.front(), 0);
        break;
    case mcNumericSparse:
        pSparse = CNumericSparseMatrix<float>::Create(attrs.m_nDim,
            &attrs.m_Ranges.front(), attrs.m_bClamp);
        break;
    case mc2DNumericSparse:
    default:
        PNL_THROW(CInvalidOperation, "Unknown matrix type");
        break;
    }

    std::istringstream bufSparse(text.c_str());
    intVector index;
    char ch;
    float val;
    for(;bufSparse.good();)
    {
        LoadArray(bufSparse, index);
        if(!bufSparse.good())
        {
            break;
        }
        if(bufSparse.peek() == ']')
        {
            bufSparse >> ch;
        }
        bufSparse >> ch;
        ASSERT(ch == ':');
        bufSparse >> val;
        pSparse->SetElementByIndexes(val, &index.front());
    }

    return pSparse;
}


// CMatrix<CDistribFun*>

void CPersistMatrixDistribFun::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CMatrix<CDistribFun*> *pMat = static_cast<CMatrix<CDistribFun*>*>(pObj);
    CMatrixIterator<CDistribFun*> *it = pMat->InitIterator();
    intVector index;
    
    for(; pMat->IsValueHere(it); pMat->Next(it))
    {
        std::stringstream buf;

        pMat->Index(it, &index);
        buf << "DistributionFunction" << index[0];
        for(int i = 1; i < index.size(); ++i)
        {
            buf << "_" << index[i];
        }
        pContext->Put(const_cast<CDistribFun*>(*pMat->Value(it)), buf.str().c_str());
    }
    
    delete it;
}

void CPersistMatrixDistribFun::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CMatrix<CDistribFun*> *pMat = static_cast<CMatrix<CDistribFun*>*>(pObj);

    MatrixCommonSave<CDistribFun*>(pMat, pContext);

    CMatrixIterator<CDistribFun*> *it = pMat->InitIterator();
    intVector index;
    pnlString buf;
    
    for(; pMat->IsValueHere(it); pMat->Next(it))
    {
        pMat->Index(it, &index);
        SaveArray<int>(buf, &index.front(), index.size());
        buf << '\n';
    }
    
    delete it;

    pContext->AddText(buf.c_str());
}

CPNLBase *CPersistMatrixDistribFun::Load(CContextLoad *pContext)
{
    CommonMatrixAttrs attrs;
    pnlString text;

    pContext->GetText(text);
    MatrixCommonLoad(&attrs, pContext);

    std::istringstream buf(text.c_str());
    intVecVector aIndex;
    pnlVector<CDistribFun*> aFun;
    int i;
    intVector index;

    index.resize(attrs.m_nDim);
    for(;buf.good();)
    {
        pnlString name;

        LoadArray(buf, index);
        if(!index.size())
        {
            break;
        }
        PNL_CHECK_FOR_NON_ZERO(index.size() - attrs.m_nDim);
        name << "DistributionFunction" << index[0];
        for(i = 1; i < index.size(); ++i)
        {
            name << '_';
            name << index[i];
        }
        aIndex.push_back(index);
        aFun.push_back(static_cast<CDistribFun*>(pContext->Get(name.c_str())));
    }

    if(attrs.m_Class == mcSparse)
    {
        CSparseMatrix<CDistribFun*> *pSparseMat = CSparseMatrix<CDistribFun*>::Create(
            attrs.m_nDim, &attrs.m_Ranges.front(), 0, attrs.m_bClamp);

        for(i = 0; i < aIndex.size(); ++i)
        {
            pSparseMat->SetElementByIndexes(aFun[i], &aIndex[i].front());
        }

        pContext->AutoDelete(pSparseMat);

        return pSparseMat;
    }
    else if(attrs.m_Class == mcDense)
    {
        CDenseMatrix<CDistribFun*> *pDenseMat = CDenseMatrix<CDistribFun*>::Create(
            attrs.m_nDim, &attrs.m_Ranges.front(), &aFun.front(), attrs.m_bClamp);

        pContext->AutoDelete(pDenseMat);

        return pDenseMat;
    }
    else
    {
        PNL_THROW(CInvalidOperation, "Unknown matrix type: sparse or dense is expected");
    }

    return 0;
}

static
pnlString &operator<<(pnlString &str, const Value v)
{
    (v.IsDiscrete()) ? (str << v.GetInt()) : (str << v.GetFlt() << 'f');

    return str;
}

static
std::istream &operator>>(std::istream &str, Value &v)
{
    double val;

    str >> val;
    if(str.peek() == 'f')
    {
        char ch;

        str >> ch;
        v.SetFlt(float(val));
    }
    else
    {
        v.SetInt(int(val));
    }

    return str;
}

void CPersistValueVector::Save(CPNLBase *pObj, CContextSave *pContext)
{
    valueVector *pV = dynamic_cast<CCover<valueVector>*>(pObj)->GetPointer();

    if(!pV)
    {
        PNL_THROW(CInconsistentType, "valueVector must be covered");
    }

    SaveArray<Value>(*pContext, &pV->front(), pV->size());
}

CPNLBase *CPersistValueVector::Load(CContextLoad *pContext)
{
    CCover<valueVector> *pCov =
        new CCoverDel<valueVector >(LoadArray<Value>(*pContext));
    pContext->AutoDelete(pCov);

    return pCov;
}

bool CPersistValueVector::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CCover<valueVector>*>(pObj) != 0;
}

// saving/loading for grouping object
void CPersistGroup::Save(CPNLBase *pObj, CContextSave *pContext)
{
}

CPNLBase *CPersistGroup::Load(CContextLoad *pContext)
{
    pnlVector<pnlString> aChild;
    int i;
    CGroupObj *group = new CGroupObj;

    pContext->GetChildrenNames(&aChild);
    for(i = 0; i < aChild.size(); ++i)
    {
	group->Put(pContext->Get(aChild[i].c_str()), aChild[i].c_str(), false);
    }

    return group;
}

void CPersistGroup::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CGroupObj *group = dynamic_cast<CGroupObj*>(pObj);
    int i;
    pnlVector<pnlString> aChild;

    group->GetChildrenNames(&aChild);
    for(i = 0; i < aChild.size(); ++i)
    {
	pContext->Put(group->Get(aChild[i].c_str(), false), aChild[i].c_str(), false);
    }
}

bool CPersistGroup::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CGroupObj*>(pObj) != 0;
}

PNL_END
