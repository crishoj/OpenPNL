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
    }

    CPersistenceZoo *m_pZoo;
};

static PersistenceUsing persistenceZooHolder;

CPersistenceZoo *GetZoo()
{
    if(!persistenceZooHolder.m_pZoo)
    {
        CPersistenceZoo *pZoo = persistenceZooHolder.m_pZoo
          = new CPersistenceZoo;
        pZoo->Register(new CGraphPersistence());
        pZoo->Register(new CPersistNodeType());
        pZoo->Register(new CPersistNodeValues());
        pZoo->Register(new CPersistNumericVector<int>("int"));
        pZoo->Register(new CPersistNumericVector<float>("float"));
        pZoo->Register(new CPersistNumericVecVector<int>("int"));
        pZoo->Register(new CPersistGaussianDistribFun());
        pZoo->Register(new CPersistTabularDistribFun());
        pZoo->Register(new CPersistCondGaussianDistribFun());
        pZoo->Register(new CPersistScalarDistribFun());
        pZoo->Register(new CPersistModelDomain());
        pZoo->Register(new CPersistBNet());
        pZoo->Register(new CPersistIDNet());
        pZoo->Register(new CPersistMNet());
        pZoo->Register(new CPersistMRF2());
        pZoo->Register(new CPersistFactor());
        pZoo->Register(new CPersistNodeTypeVector());
        pZoo->Register(new CPersistPNodeTypeVector());
        pZoo->Register(new CPersistMatrixFlt());
        pZoo->Register(new CPersistMatrixDistribFun());
        pZoo->Register(new CPersistEvidence());
    }

    return persistenceZooHolder.m_pZoo;
}

void CPersistenceZoo::Register(CPersistence *pPersist)
{
    m_aFuncMap[Map::key_type(std::string(pPersist->Signature()))] = pPersist;
}

void CPersistenceZoo::Unregister(CPersistence *pPersist)
{
    Map::iterator it = m_aFuncMap.find(pPersist->Signature());
    
    if(it != m_aFuncMap.end())
    {
        m_aFuncMap.erase(it);
    }
    // else { does not exists };
}
