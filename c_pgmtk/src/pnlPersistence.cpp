/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlPersistence.cpp                                          //
//                                                                         //
//  Purpose:   Base pure virtual class. Classes derived from CPersistence  //
//             designed to perform saving or loading                       //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlContext.hpp"
#include "pnlPersistence.hpp"
#include "pnlPersistGraph.hpp"
#include "pnlPersistTypes.hpp"
#include "pnlPersistCover.hpp"
#include "pnlPersistModelDomain.hpp"
#include "pnlPersistModel.hpp"
#include "pnlPersistDistribFun.hpp"
#include "pnlPersistCPD.hpp"

PNL_USING

CPersistenceZoo *GetZoo();

static CPersistence *spQueueHead = 0;
static int siQueueUpdate;

class PersistenceUsing
{
public:
    PersistenceUsing(): m_pZoo(0) { }

    ~PersistenceUsing()
    {
        if(m_pZoo)
        {
            delete m_pZoo;
        }
	m_pZoo = 0;
    }

    CPersistenceZoo *m_pZoo;
};

static PersistenceUsing persistenceZooHolder;

CPersistence::CPersistence()
{
    m_pNext = spQueueHead;
    m_pPrev = 0;
    if(m_pNext)
    {
	m_pNext->m_pPrev = this;
    }
    spQueueHead = this;
    ++siQueueUpdate;
}

CPersistence::~CPersistence()
{
    // remove from queue
    if(m_pPrev)
    {
	m_pPrev->m_pNext = m_pNext;
    }
    if(m_pNext)
    {
	m_pNext->m_pPrev = m_pPrev;
    }

    if(persistenceZooHolder.m_pZoo)
    {
	persistenceZooHolder.m_pZoo->Unregister(this, true/* from DTOR */);
    }
}

CPersistenceZoo *GetZoo()
{
    static CGraphPersistence graphSaveLoader;
    static CPersistNodeType nodeTypeSaveLoader;
    static CPersistNodeValues nodeValuesSaveLoader;
    static CPersistNumericVector<int> vectorIntSaveLoader("int");
    static CPersistNumericVector<float> vectorFloatSaveLoader("float");
    static CPersistNumericVecVector<int> vecVectorIntSaveLoader("int");
    static CPersistValueVector valueVectorSaveLoader;
    static CPersistGaussianDistribFun gaussianDistribFunSaveLoader;
    static CPersistTabularDistribFun tabularDistribFunSaveLoader;
    static CPersistSoftMaxDistribFun softMaxDistribFunSaveLoader;
    static CPersistCondGaussianDistribFun condGaussianDistribFunSaveLoader;
    static CPersistCondSoftMaxDistribFun condSoftMaxDistribFunSaveLoader;
    static CPersistScalarDistribFun scalarDistribFunSaveLoader;
    static CPersistModelDomain modelDomainSaveLoader;
    static CPersistDBN dbnSaveLoader;
    static CPersistBNet bnetSaveLoader;
    static CPersistIDNet idnetSaveLoader;
    static CPersistMNet mnetSaveLoader;
    static CPersistMRF2 mrf2SaveLoader;
    static CPersistFactor factorSaveLoader;
    static CPersistNodeTypeVector nodeTypeVectorSaveLoader;
    static CPersistPNodeTypeVector pNodeTypeVectorSaveLoader;
    static CPersistMatrixFlt matrixFltSaveLoader;
    static CPersistMatrixDistribFun matrixDistribFunSaveLoader;
    static CPersistEvidence evidenceSaveLoader;
    static CPersistGroup groupSaveLoader;

    if(!persistenceZooHolder.m_pZoo)
    {
        persistenceZooHolder.m_pZoo = new CPersistenceZoo;
    }

    return persistenceZooHolder.m_pZoo;
}

void CPersistenceZoo::Register(CPersistence *pPersist)
{
    m_aFuncMap[Map::key_type(pnlString(pPersist->Signature()))] = pPersist;
}

void CPersistenceZoo::Unregister(CPersistence *pPersist, bool bDTOR)
{
    Map::iterator it;

    if(bDTOR)
    {
	for(it = m_aFuncMap.begin(); it != m_aFuncMap.end(); it++)
	{
	    if(it->second == pPersist)
	    {
		m_aFuncMap.erase(it);
		break;
	    }
	}
    }
    else
    {
	it = m_aFuncMap.find(pPersist->Signature());
	
	if(it != m_aFuncMap.end())
	{
	    m_aFuncMap.erase(it);
	}
	// else { does not exists };
    }
}

bool CPersistenceZoo::GetClassName(pnlString *pName, CPNLBase *pObj)
{
    RescanIfNeed();
    Map::const_iterator it = m_aFuncMap.begin();

    for(; it != m_aFuncMap.end(); ++it)
    {
	if(it->second->IsHandledType(pObj))
	{
	    *pName = it->first;
	    return true;
	}
    }

    return false;
}

CPersistence *CPersistenceZoo::ObjectBySignature(pnlString &name)
{
    RescanIfNeed();
    Map::iterator it = m_aFuncMap.find(name);
    return (it == m_aFuncMap.end()) ? 0:(*it).second;
}

void CPersistenceZoo::RescanIfNeed()
{
    if(m_iUpdate != siQueueUpdate)
    {
	CPersistence *pPers = spQueueHead;
	
	// register without checking all unregistered CPersistence
	for(;pPers && m_aFuncMap.find(pPers->Signature()) == m_aFuncMap.end();
	    pPers = pPers->m_pNext)
	{
	    m_aFuncMap[pPers->Signature()] = pPers;
	}

	m_iUpdate = siQueueUpdate;
    }
}

CPersistenceZoo::~CPersistenceZoo() { }

CPersistenceZoo::CPersistenceZoo(): m_iUpdate(0) {}
