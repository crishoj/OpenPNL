/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDynamicGraphicalModel.cpp                                //
//                                                                         //
//  Purpose:   Implementation of the base class for all Dynamic graphical  //
//             models                                                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include <math.h>
#include "pnlDynamicGraphicalModel.hpp"
#include "pnlBNet.hpp"
#include <sstream>

PNL_USING

CDynamicGraphicalModel::
CDynamicGraphicalModel( EModelTypes modelType, CStaticGraphicalModel *pGrModel):
CGraphicalModel(pGrModel->GetModelDomain())
{
    // Check the validity of the model: 
    if( !pGrModel->IsValidAsBaseForDynamicModel() )
    {
        PNL_THROW( CInconsistentType,
            "static model doesn't valid for creation dynamic model" )
    }
   
    m_pGrModel = pGrModel;
    m_modelType = modelType;
    m_nnodesPerSlice = pGrModel->GetNumberOfNodes() / 2;

    FindInterfaceNodes();
}

CDynamicGraphicalModel::~CDynamicGraphicalModel()
{
    if(m_pGrModel)
    {
	delete (m_pGrModel);
    }
    m_InterfaceNodes.clear();
}


void CDynamicGraphicalModel::FindInterfaceNodes()
{
    int i, j;

    CGraph *graph = m_pGrModel->GetGraph();

    int nnodes = graph->GetNumberOfNodes();

    int                    numberOfNeighbors;
    const int              *neighbors;
    const ENeighborType *orientation;

    for( i = 0; i < nnodes/2; i++ )
    {
	graph->GetNeighbors(i, &numberOfNeighbors, &neighbors, &orientation);

	for( j = 0; j < numberOfNeighbors; j++ )
	{
	    if( neighbors[j] >= nnodes/2 )
	    {
		m_InterfaceNodes.push_back(i);
                break;
	    }
	}
    }
}


CFactors* CDynamicGraphicalModel::AttachFactors( CFactors *params )
{
    return (m_pGrModel->AttachFactors(params));
}


CFactor* CDynamicGraphicalModel::GetFactor(int domainNodes) const
{
    return (m_pGrModel->GetFactor(domainNodes));
}


void CDynamicGraphicalModel::AllocFactors()
{
    m_pGrModel->AllocFactors();
}

void CDynamicGraphicalModel::AllocFactor( int numberOfNodesInDomain, const int *domain )
{
    m_pGrModel->AllocFactor(numberOfNodesInDomain, domain);
}


void CDynamicGraphicalModel::AllocFactor( int number )
{
    m_pGrModel->AllocFactor( number );
}

void CDynamicGraphicalModel::AttachFactor(CFactor *param)
{
    m_pGrModel->AttachFactor( param );
}


void CDynamicGraphicalModel::GetFactors( int numberOfNodes, const int* nodes,
					int *numberOfFactors,
					CFactor ***params ) const
{
    m_pGrModel->GetFactors( numberOfNodes, nodes, numberOfFactors, params );
}

int CDynamicGraphicalModel::GetFactors( int numberOfNodes, const int* nodes,
				       pFactorVector *params) const
{
    return (m_pGrModel->GetFactors( numberOfNodes, nodes, params ));
}

bool CDynamicGraphicalModel::IsValid( std::string* description ) const
{
    if( !m_pGrModel )
    {
        std::stringstream st;
        st<<"Static Graphical Model (base for DBN) hase null pointer"<<std::endl;
        std::string s = st.str();
	description->insert( description->begin(), s.begin(), s.end() );
	return false;
    }
    else
    {
        
	if( !m_pGrModel->IsValidAsBaseForDynamicModel(description) )
	{
	    return false;
	}
	
    }
    return true;
}

#ifdef PNL_RTTI
const CPNLType CDynamicGraphicalModel::m_TypeInfo = CPNLType("CDynamicGraphicalModel", &(CGraphicalModel::m_TypeInfo));

#endif