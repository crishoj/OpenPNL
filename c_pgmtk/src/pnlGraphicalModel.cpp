/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGraphicalModel.cpp                                       //
//                                                                         //
//  Purpose:   CGraphicalModel class member functions implementation       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlGraphicalModel.hpp"

PNL_USING

void CGraphicalModel::GetNodeTypes(nodeTypeVector* nodeTypesOut)
{
    PNL_CHECK_IS_NULL_POINTER(nodeTypesOut);

    m_pMD->GetVariableTypes(nodeTypesOut);
};

void CGraphicalModel::GetNodeTypes(pConstNodeTypeVector *nodeTypesOut)
{
	PNL_CHECK_IS_NULL_POINTER(nodeTypesOut);

	m_pMD->GetVariableTypes( nodeTypesOut );
};

void CGraphicalModel::AllocFactor( const intVector& domainIn)
{
	AllocFactor( domainIn.size(), &domainIn.front() );
};

int CGraphicalModel::GetFactors( const intVector& subdomainIn,
		                    pFactorVector *paramsOut ) const
{
	return GetFactors( subdomainIn.size(), &subdomainIn.front(),
		paramsOut );
};

CGraphicalModel::CGraphicalModel(CModelDomain* pMD)
{
    m_pMD = pMD;
    void* pObj = this;
    m_pMD->AddRef(pObj);
}

CGraphicalModel::CGraphicalModel(int numberOfNodes, 
                                 int numberOfNodeTypes,
                                 const CNodeType *nodeTypes,
                                 const int *nodeAssociation )
{
    CGraphicalModel* pObj = this;
    nodeTypeVector nt = nodeTypeVector( nodeTypes, nodeTypes + numberOfNodeTypes );
    intVector nAssociation = intVector( nodeAssociation, 
        nodeAssociation + numberOfNodes );
    m_pMD = CModelDomain::Create( nt, nAssociation, pObj );
}


CGraphicalModel::~CGraphicalModel()
{
    void *pObj = this;
    m_pMD->Release(pObj);
}

#ifdef PNL_RTTI
const CPNLType CGraphicalModel::m_TypeInfo = CPNLType("CGraphicalModel", &(CPNLBase::m_TypeInfo));

#endif
