/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDistribFun.cpp                                           //
//                                                                         //
//  Purpose:   CDistribFun class member functions implementation           //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlDistribFun.hpp"

PNL_USING

CDistribFun::CDistribFun(EDistributionType dt, int numNodes,
					const CNodeType *const* nodeTypes,
                                        int isUnitFunction, bool allocTheMatrices):
					m_NumberOfNodes( numNodes ),
					m_NodeTypes( nodeTypes, nodeTypes + numNodes ),
					m_bUnitFunctionDistribution( isUnitFunction ),
                                        m_bAllMatricesAreValid(0)//,
                                        //m_bMatricesAreAllocated(allocTheMatrices)
{
    m_DistributionType = dt;
}
CDistribFun::CDistribFun( EDistributionType dt )
:m_NumberOfNodes(0), m_bUnitFunctionDistribution(0), m_bAllMatricesAreValid(0)//, m_bMatricesAreAllocated(0)
{
    m_DistributionType = dt;
}

void CDistribFun::ResetNodeTypes( pConstNodeTypeVector &nodeTypes )
{
    if( int(nodeTypes.size()) != m_NumberOfNodes )
    {
        PNL_THROW( COutOfRange, "number of node types" );
    }
    int i;
    for( i = 0; i < m_NumberOfNodes; i++)
    {
        PNL_CHECK_IS_NULL_POINTER( nodeTypes[i] );
        if( *m_NodeTypes[i] != *nodeTypes[i] )
        {
            PNL_THROW( CInconsistentType, "node types must be equal");
        }
    }
    m_NodeTypes.assign( nodeTypes.begin(), nodeTypes.end() );
}

#ifdef PNL_RTTI
const CPNLType CDistribFun::m_TypeInfo = CPNLType("CDistribFun", &(CPNLBase::m_TypeInfo));

#endif
