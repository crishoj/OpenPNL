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



