/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistDistribFun.cpp                                    //
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
#include "pnlPersistCover.hpp"
#include "pnlPersistDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlCondGaussianDistribFun.hpp"
#include "pnlScalarDistribFun.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlCondSoftMaxDistribFun.hpp"
PNL_USING

// Common methods

static void 
SaveForDistribFun(CDistribFun *pDF, CContextSave *pContext)
{
    pContext->AddAttribute("NumberOfNodes", pDF->GetNumberOfNodes());
    pContext->AddAttribute("IsUnitFunction",
	(pDF->IsDistributionSpecific() == 1) ? true:false);
}

static void 
LoadForDistribFun(bool *pbUnitFun, int *pnNode, const CNodeType * const ** pppNodeType, CContextLoad *pContext)
{
    pContext->GetAttribute(pbUnitFun, "IsUnitFunction");
    pContext->GetAttribute(pnNode, "NumberOfNodes");
    *pppNodeType = (const CNodeType *const*)&(
	static_cast<CCover<pNodeTypeVector> *>(pContext->Get("NodeTypes"))->GetPointer()
	)->front();
}

static void 
TraverseDistribFunSubobjects(CDistribFun *pDF, CContext *pContext)
{
    if(!pDF->IsValid())
    {
	PNL_THROW(CInvalidOperation, "DistribFun is invalid");
    }

    pContext->Put(new CCoverDel<pNodeTypeVector>(
	(pNodeTypeVector*)pDF->GetNodeTypesVector()), "NodeTypes");
}


// GaussianDistibFun

void
CPersistGaussianDistribFun::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CGaussianDistribFun *pDF = dynamic_cast<CGaussianDistribFun*>(pObj);
    bool isUnitFun = pDF->IsDistributionSpecific() == 1 ? true:false;
    bool isCanonical = pDF->GetCanonicalFormFlag() ? true:false;
    bool isFactor = pDF->GetFactorFlag() ? true:false;

    SaveForDistribFun(pDF, pContext);
    pContext->AddAttribute("IsCanonical", isCanonical);
    pContext->AddAttribute("IsFactor", isFactor);

    if(isUnitFun)
    {
	return;
    }

    bool isDelta = pDF->IsDistributionSpecific() == 2 ? true:false;
    bool isMoment = pDF->GetMomentFormFlag() ? true:false;

    pContext->AddAttribute("IsDelta", isDelta);
    pContext->AddAttribute("IsMoment", isMoment);
    if(isDelta)
    {
	return;
    }

    if(isCanonical)
    {
	pContext->AddAttribute("g", pDF->GetCoefficient(1));
    }

    if(isMoment)
    {
	pContext->AddAttribute("NormCoefficient", pDF->GetCoefficient(0));
    }
}

static CDenseMatrix<float> *
getMatrixByName(const char *name, CContextLoad *pContext)
{
    return static_cast<CDenseMatrix<float> *>(pContext->Get(name));
}

CPNLBase *
CPersistGaussianDistribFun::Load(CContextLoad *pContext)
{
    int nNode;
    bool isUnitFun;
    bool isCanonical;
    bool isMoment;
    bool isFactor;
    CGaussianDistribFun *pDF = 0;
    const CNodeType *const* ppNodeType;

    LoadForDistribFun(&isUnitFun, &nNode, &ppNodeType, pContext);
    pContext->GetAttribute(&isCanonical, "IsCanonical");
    pContext->GetAttribute(&isFactor, "IsFactor");

    if(isUnitFun)
    {
	pDF = CGaussianDistribFun::CreateUnitFunctionDistribution(nNode,
	    ppNodeType, isFactor ? 1:0, isCanonical ? 1:0);
	return pDF;
    }

    bool isDelta;
    CDenseMatrix<float> *pMat;
    CDenseMatrix<float> *pMat2;
    const float *pData;
    int len;

    pContext->GetAttribute(&isDelta, "IsDelta");
    pContext->GetAttribute(&isMoment, "IsMoment");

    if(isDelta)
    {
	getMatrixByName("MatMean", pContext)->GetRawData(&len, &pData);
	pDF = CGaussianDistribFun::CreateDeltaDistribution(nNode,
            ppNodeType, pData, isMoment ? 1:0, isFactor ? 1:0);
	return pDF;
    }

    if(isCanonical)
    {
	float g;
	pContext->GetAttribute(&g, "g");
	pMat = getMatrixByName("MatH", pContext);
	pMat2 = getMatrixByName("MatK", pContext);
	pDF = CGaussianDistribFun::CreateInCanonicalForm(nNode, ppNodeType, NULL, NULL, g);
	pDF->AttachMatrix(pMat, matH);
	pDF->AttachMatrix(pMat2, matK);
    }

    if(isMoment)
    {
	float normCoefficient;
	if(!pDF)
	{
	    const float *pDataCov;
	    getMatrixByName("MatMean", pContext)->GetRawData(&len, &pData);
	    getMatrixByName("MatCovariance", pContext)->GetRawData(&len, &pDataCov);
	    pDF = CGaussianDistribFun::CreateInMomentForm(isFactor, nNode, ppNodeType, pData, pDataCov);
	}
	else
	{
	    pDF->AttachMatrix(getMatrixByName("MatCovariance", pContext), matCovariance);
	    pDF->AttachMatrix(getMatrixByName("MatMean", pContext), matMean);
	}
	pContext->GetAttribute(&normCoefficient, "NormCoefficient");
	pDF->SetCoefficient(normCoefficient, 0);
	if(!isFactor)
	{
	    for(int i = 0; i < nNode - 1; ++i)
	    {
		std::stringstream name;

		name << "MatWeight" << i;
		pDF->AttachMatrix(getMatrixByName(name.str().c_str(), pContext), matWeights, i);
	    }
	}
    }

    return pDF;
}

void
CPersistGaussianDistribFun::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CGaussianDistribFun *pDF = dynamic_cast<CGaussianDistribFun*>(pObj);

    if(pDF->IsDistributionSpecific() == 3)
    {
	PNL_THROW(CInvalidOperation, "DistribFun is Gaussian multiplied by Delta");
    }

    TraverseDistribFunSubobjects(pDF, pContext);

    if(pDF->IsDistributionSpecific() == 1)
    {
	return;
    }

    if(pDF->IsDistributionSpecific() == 2)
    {
	pContext->Put(pDF->GetMatrix(matMean), "MatMean");
	return;
    }

    if(pDF->GetCanonicalFormFlag())
    {
	pContext->Put(pDF->GetMatrix(matH), "MatH");
	pContext->Put(pDF->GetMatrix(matK), "MatK");
    }

    if(pDF->GetMomentFormFlag())
    {
	pContext->Put(pDF->GetMatrix(matMean), "MatMean");
	pContext->Put(pDF->GetMatrix(matCovariance), "MatCovariance");
	if(!pDF->GetFactorFlag())
	{
	    int nParent = pDF->GetNumberOfNodes() - 1;

	    for(int i = 0; i < nParent; ++i)
	    {
		std::stringstream name;

		name << "MatWeight" << i;
		pContext->Put(pDF->GetMatrix(matWeights, i), name.str().c_str());
	    }
	}
    }
}

bool CPersistGaussianDistribFun::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CGaussianDistribFun*>(pObj) != 0;
}


// SoftMaxDistribFun

void 
CPersistSoftMaxDistribFun::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CSoftMaxDistribFun *pDF = dynamic_cast<CSoftMaxDistribFun*>(pObj);
    bool isUnitFun = pDF->IsDistributionSpecific() == 1 ? true:false;

    SaveForDistribFun(pDF, pContext);

    if(isUnitFun)
    {
        return;
    }

    pContext->AddAttribute("MaximizingMethod", pDF->GetMaximizingMethod());
}

CPNLBase *
CPersistSoftMaxDistribFun::Load(CContextLoad *pContext)
{
    int nNode;
    bool isUnitFun;
    CSoftMaxDistribFun *pDF = 0;
    const CNodeType *const* ppNodeType;

    LoadForDistribFun(&isUnitFun, &nNode, &ppNodeType, pContext);

    if(isUnitFun)
    {
        pDF = CSoftMaxDistribFun::CreateUnitFunctionDistribution(nNode,
            ppNodeType);
        return pDF;
    }

    const float *pWeightsData;
    floatVector *pOffsets;
    int len;
    int MaxMethod;

    getMatrixByName("MatWeights", pContext)->GetRawData(&len, &pWeightsData);
    pOffsets = (floatVector*)(static_cast<CCover<floatVector>*>(
        pContext->Get("Offsets"))->GetPointer());

    pDF = CSoftMaxDistribFun::Create(nNode, ppNodeType, pWeightsData,
        &pOffsets->front());

    pContext->GetAttribute(&MaxMethod, "MaximizingMethod");
    pDF->SetMaximizingMethod((EMaximizingMethod)MaxMethod);
    
    return pDF;
}

void 
CPersistSoftMaxDistribFun::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CSoftMaxDistribFun *pDF = dynamic_cast<CSoftMaxDistribFun*>(pObj);

    TraverseDistribFunSubobjects(pDF, pContext);

    if(pDF->IsDistributionSpecific() == 1)
    {
        return;
    }

    pContext->Put(pDF->GetMatrix(matWeights), "MatWeights");
    pContext->Put(new CCoverDel<floatVector>(pDF->GetOffsetVector()), "Offsets");
}

bool CPersistSoftMaxDistribFun::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CSoftMaxDistribFun*>(pObj) != 0;
}


// TabularDistribFun

void
CPersistTabularDistribFun::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CTabularDistribFun *pDF = dynamic_cast<CTabularDistribFun*>(pObj);
    bool isUnitFun = pDF->IsDistributionSpecific() == 1 ? true:false;

    SaveForDistribFun(pDF, pContext);

    if(isUnitFun)
    {
        pContext->AddAttribute("IsDense", pDF->IsDense() ? true:false);
	return;
    }
}

CPNLBase *
CPersistTabularDistribFun::Load(CContextLoad *pContext)
{
    bool isUnitFun;
    CTabularDistribFun *pDF;
    const CNodeType *const* ppNodeType;
    int nNode;

    LoadForDistribFun(&isUnitFun, &nNode, &ppNodeType, pContext);

    if(isUnitFun)
    {
	bool bDense;
	pContext->GetAttribute(&bDense, "IsDense");
	pDF = CTabularDistribFun::CreateUnitFunctionDistribution(nNode,
	    ppNodeType, bDense);
	return pDF;
    }

    CMatrix<float> *pMat = static_cast<CMatrix<float>*>(pContext->Get("MatTable"));

    pDF = CTabularDistribFun::Create(nNode, ppNodeType, NULL);
    pDF->AttachMatrix(pMat, matTable);

    return pDF;
}

void
CPersistTabularDistribFun::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CTabularDistribFun *pDF = dynamic_cast<CTabularDistribFun*>(pObj);

    TraverseDistribFunSubobjects(pDF, pContext);

    if(pDF->IsDistributionSpecific() == 1)
    {
	return;
    }

    pContext->Put(pDF->GetMatrix(matTable), "MatTable");
}

bool CPersistTabularDistribFun::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CTabularDistribFun*>(pObj) != 0;
}


// ConditionalGaussianDistribFun

void
CPersistCondGaussianDistribFun::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CCondGaussianDistribFun *pDF = dynamic_cast<CCondGaussianDistribFun*>(pObj);

    TraverseDistribFunSubobjects(pDF, pContext);

    pContext->Put(pDF->GetMatrixWithDistribution(), "DistributionMatrix");
}

void
CPersistCondGaussianDistribFun::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CCondGaussianDistribFun *pDF = dynamic_cast<CCondGaussianDistribFun*>(pObj);

    pContext->AddAttribute("NumberOfNodes", pDF->GetNumberOfNodes());
    pContext->AddAttribute("IsFactor", false);
}

CPNLBase *
CPersistCondGaussianDistribFun::Load(CContextLoad *pContext)
{
    int nNode;
    bool bFactor;
    bool isUnitFun;
    CCondGaussianDistribFun *pDF;
    const CNodeType *const* ppNodeType;

    LoadForDistribFun(&isUnitFun, &nNode, &ppNodeType, pContext);

    pContext->GetAttribute(&nNode, "NumberOfNodes");
    pContext->GetAttribute(&bFactor, "IsFactor");
    
    CMatrix<CGaussianDistribFun*> *mat = static_cast<CMatrix<CGaussianDistribFun*>*>(
	pContext->Get("DistributionMatrix"));
    bool isDense(mat->GetMatrixClass() == mcDense);
	//dynamic_cast<CSparseMatrix<CGaussianDistribFun*>*>(mat) == 0;

    pDF = CCondGaussianDistribFun::Create(bFactor ? 1:0, nNode, ppNodeType, isDense);

    CMatrixIterator<CGaussianDistribFun*> *it = mat->InitIterator();
    intVector index;

    for(; mat->IsValueHere(it); mat->Next(it))
    {
	mat->Index(it, &index);
	pDF->SetDistribFun(*mat->Value(it), &index.front());
    }

    delete it;

    return pDF;
}

bool CPersistCondGaussianDistribFun::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CCondGaussianDistribFun*>(pObj) != 0;
}


// ConditionalSoftMaxDistribFun

void 
CPersistCondSoftMaxDistribFun::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CCondSoftMaxDistribFun *pDF = dynamic_cast<CCondSoftMaxDistribFun*>(pObj);

    pContext->AddAttribute("NumberOfNodes", pDF->GetNumberOfNodes());
    pContext->AddAttribute("IsFactor", false);
}

CPNLBase *
CPersistCondSoftMaxDistribFun::Load(CContextLoad *pContext)
{
    int nNode;
    bool bFactor;
    bool isUnitFun;
    CCondSoftMaxDistribFun *pDF;
    const CNodeType *const* ppNodeType;

    LoadForDistribFun(&isUnitFun, &nNode, &ppNodeType, pContext);

    pContext->GetAttribute(&nNode, "NumberOfNodes");
    pContext->GetAttribute(&bFactor, "IsFactor");
    
    CMatrix<CSoftMaxDistribFun*> *mat = static_cast<CMatrix<CSoftMaxDistribFun*>*>(
        pContext->Get("DistributionMatrix"));

    pDF = CCondSoftMaxDistribFun::Create(nNode, ppNodeType);

    CMatrixIterator<CSoftMaxDistribFun*> *it = mat->InitIterator();
    intVector index;

    for(; mat->IsValueHere(it); mat->Next(it))
    {
        mat->Index(it, &index);
        pDF->SetDistribFun(*mat->Value(it), &index.front());
    }

    delete it;

    return pDF;
}

void 
CPersistCondSoftMaxDistribFun::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CCondSoftMaxDistribFun *pDF = dynamic_cast<CCondSoftMaxDistribFun*>(pObj);

    TraverseDistribFunSubobjects(pDF, pContext);

    pContext->Put(pDF->GetMatrixWithDistribution(), "DistributionMatrix");
}

bool CPersistCondSoftMaxDistribFun::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CCondSoftMaxDistribFun*>(pObj) != 0;
}


// ScalarDistribFun

void
CPersistScalarDistribFun::TraverseSubobject(CPNLBase *pObj, CContext *pContext)
{
    CScalarDistribFun *pDF = dynamic_cast<CScalarDistribFun*>(pObj);

    TraverseDistribFunSubobjects(pDF, pContext);
}

void
CPersistScalarDistribFun::Save(CPNLBase *pObj, CContextSave *pContext)
{
    CScalarDistribFun *pDF = dynamic_cast<CScalarDistribFun*>(pObj);
    
    SaveForDistribFun(pDF, pContext);
    pContext->AddAttribute("IsDense", pDF->IsDense() ? true:false);
}

CPNLBase *
CPersistScalarDistribFun::Load(CContextLoad *pContext)
{
    bool isUnitFun;
    int nNode;
    const CNodeType *const* ppNodeType;

    LoadForDistribFun(&isUnitFun, &nNode, &ppNodeType, pContext);

    return CScalarDistribFun::Create(nNode, ppNodeType);
}

bool CPersistScalarDistribFun::IsHandledType(CPNLBase *pObj) const
{
    return dynamic_cast<CScalarDistribFun*>(pObj) != 0;
}
