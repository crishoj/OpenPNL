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
#include <ctype.h>
#include <sstream>
#include "pnlContext.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlPersistCover.hpp"
#include "pnlPersistTypes.hpp"
#include "pnlNodeType.hpp"
#include "pnlNodeValues.hpp"
#include "pnlEvidence.hpp"
#include "pnlDistribFun.hpp"

PNL_USING

void
CPersistNodeType::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CNodeType *pNodeType = dynamic_cast<CNodeType*>(pObj);

    pContext->AddAttribute("NodeSize", pNodeType->GetNodeSize());
    pContext->AddAttribute("IsDiscrete", pNodeType->IsDiscrete());
}

CPNLBase *
CPersistNodeType::Load(CContextLoad *pContext)
{
    int nodeSize;
    bool bDiscrete;
    
    pContext->GetAttribute(&nodeSize, "NodeSize");
    pContext->GetAttribute(&bDiscrete, "IsDiscrete");

    return new CNodeType(bDiscrete, nodeSize);
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
    pContext->AutoDelete(pCovValues);

    CCoverDel<pNodeTypeVector> *pCov = new CCoverDel<pNodeTypeVector>(papNodeType);
    pContext->AutoDelete(pCov);

    pContext->Put(pCovValues, "Values");
    pContext->Put(pCov, "pNodeTypes");
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

void CPersistEvidence::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CPersistNodeValues::Save(pObj, pContext);
}

CPNLBase *CPersistEvidence::Load(CContextLoad *pContext)
{
    valueVector *paValue = static_cast<CCoverDel<valueVector>*>(
	pContext->Get("Values"))->GetPointer();
    CModelDomain *pMD = static_cast<CModelDomain*>(pContext->Get("ModelDomain"));
    intVector *pV = static_cast<CCover<intVector>*>(pContext->Get("ObservedNodes"))->GetPointer();

    return CEvidence::Create(pMD, *pV, *paValue);
}

// Matrix

// Numeric matrices

// Common methods for matrices
static struct
{
    EMatrixClass m_Class;
    const char *m_Name;
} sMapMatrixType[] =
{   mcSparse,	    "Sparse"
,   mcDense,	    "Dense"
,   mcNumericDense, "NumericDense"
,   mcNumericSparse,"NumericSparse"
,   mc2DNumericDense,  "2DNumericDense"
,   mc2DNumericSparse, "2DNumericSparse"
};

template<typename Type>
void SaveArray(std::stringstream &buf, const Type *pArray, int nElement, char delim = ' ')
{
	buf << '[';
	for(int i = 0; i < nElement; ++i)
	{
	    buf << pArray[i] << ((i == nElement - 1) ? ']':delim);
	}
}

template<typename Type>
void MatrixCommonSave(CMatrix<Type>* pMat, CContextSave *pContext)
{
    int i, nDim;
    const int *ranges;

    pMat->GetRanges(&nDim, &ranges);
    pContext->AddAttribute("NumberOfDimensions", nDim);
    {
	std::stringstream buf;

        SaveArray<int>(buf, ranges, nDim);
	pContext->AddAttribute("Ranges", buf.str().c_str());
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

template<typename Type>
void LoadArray(std::istringstream &buf, pnlVector<Type> &array, char delim = ' ')
{
    char ch;
    int ich;
    Type val;

    buf >> ch;
    ASSERT(ch == '[');
    array.reserve(16);
    array.resize(0);
    for(;buf.good();)
    {
	ich = buf.peek();
	if(ich == ']')
	{
	    buf >> ch;
	    break;
	}
	if(ich == delim)
	{
	    buf.get(ch);
	}

	buf >> val;
	array.push_back(val);
    }
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
    std::string attr;

    pContext->GetAttribute(&attrs->m_nDim, "NumberOfDimensions");
    pContext->GetAttribute(attr, "MatrixClass");
    for(i = 0; i < sizeof(sMapMatrixType)/sizeof(sMapMatrixType[0]); ++i)
    {
	if(sMapMatrixType[i].m_Name == attr)
	{
	    attrs->m_Class = sMapMatrixType[i].m_Class;
	    break;
	}
    }
    pContext->GetAttribute(attr, "Ranges");
    {
	std::istringstream buf(attr);
	LoadArray<int>(buf, attrs->m_Ranges);
    }
    pContext->GetAttribute(&attrs->m_bClamp, "IsClamped");
}

void CPersistMatrixFlt::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CMatrix<float> *pMat = static_cast<CMatrix<float>*>(pObj);

    MatrixCommonSave<float>(pMat, pContext);
    std::stringstream buf;
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

    pContext->AddText(buf.str().c_str());
}

CPNLBase *CPersistMatrixFlt::Load(CContextLoad *pContext)
{
    CommonMatrixAttrs attrs;
    std::string text;

    pContext->GetText(text);
    MatrixCommonLoad(&attrs, pContext);
    if(attrs.m_Class == mcDense || attrs.m_Class == mcNumericDense
	|| attrs.m_Class == mc2DNumericDense)
    {
        floatVector data;
	std::istringstream buf(text);

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

    std::istringstream bufSparse(text);
    intVector index;
    char ch;
    float val;
    for(;bufSparse.good();)
    {
	LoadArray(bufSparse, index);
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
    std::stringstream buf;
    
    for(; pMat->IsValueHere(it); pMat->Next(it))
    {
	pMat->Index(it, &index);
	SaveArray<int>(buf, &index.front(), index.size());
	buf << '\n';
    }
    
    delete it;

    pContext->AddText(buf.str().c_str());
}

CPNLBase *CPersistMatrixDistribFun::Load(CContextLoad *pContext)
{
    CommonMatrixAttrs attrs;
    std::string text;

    pContext->GetText(text);
    MatrixCommonLoad(&attrs, pContext);

    std::istringstream buf(text);
    intVecVector aIndex;
    pnlVector<CDistribFun*> aFun;
    char ch;
    int i;
    intVector index;

    index.resize(attrs.m_nDim);
    for(;buf.good();)
    {
	std::stringstream name;

	buf >> ch;
	if(ch != '[')
	{
	    break;
	}
	name << "DistributionFunction";
	for(i = 0; i < attrs.m_nDim && buf.good() && !buf.eof(); ++i)
	{
	    buf >> ch;
	    if(!isdigit(ch))
	    {
		break;
	    }
	    buf.unget();
	    buf >> index[i];
	    if(i)
	    {
		name << '_';
	    }
	    name << index[i];
	}
	if(i != attrs.m_nDim)
	{
	    break;
	}
	aIndex.push_back(index);
	aFun.push_back(static_cast<CDistribFun*>(pContext->Get(name.str().c_str())));
	if(buf.peek() == ']')
	{
	    buf >> ch;
	}
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

#if 0

void CPersistMatrixXXX::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
}

void CPersistMatrixXXX::Save(CPNLBase *pObj, CContextSave *pContext)
{
}

CPNLBase *CPersistMatrixXXX::Load(CContextLoad *pContext)
{
}

#endif
