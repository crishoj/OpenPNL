/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlNodeValues.cpp                                           //
//                                                                         //
//  Purpose:   CNodeValues class member functions implementation           //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// pnlNodeValues.cpp - implementation of methods of CNodeValues class

#include "pnlConfig.hpp"
#include "pnlNodeValues.hpp"
#include "pnlException.hpp"

PNL_USING

CNodeValues* CNodeValues::Create( const pConstNodeTypeVector& obsNodesTypes,
                                 const valueVector& obsValues )
{
    int numObsNds = obsNodesTypes.size();
    const CNodeType* const* pObsNdsTypes = &obsNodesTypes.front();
    return CNodeValues::Create( numObsNds, pObsNdsTypes, obsValues );
}

void CNodeValues::ToggleNodeStateBySerialNumber( const intVector& numsOfNds )
{
    int numNds = numsOfNds.size();
    const int* pNumsOfNds = &numsOfNds.front();
    ToggleNodeStateBySerialNumber( numNds, pNumsOfNds );
}

CNodeValues* CNodeValues::Create( int NObsNodes,
				 const CNodeType* const* obsNodesTypes,
				 const valueVector& obs_values)
{
    if(( !obsNodesTypes ))
    {
	PNL_THROW( CNULLPointer, "obsNodesTypes must be not NULL " )
    }
    if( NObsNodes < 0 )
    {
	PNL_THROW( COutOfRange, "number of nodes in Values must be positive number" )
    }
    CNodeValues *pNewValues = new CNodeValues( NObsNodes, obsNodesTypes,
	obs_values);
    return pNewValues;
}

CNodeValues::CNodeValues( int nNodes, const CNodeType * const*ObsNodeTypes,
			 const valueVector& pValues )
			 :m_numberObsNodes(nNodes),
			 m_NodeTypes( ObsNodeTypes, ObsNodeTypes+nNodes ),
			 m_rawValues( pValues.begin(), pValues.end() )
{
    int i;
    int maxValue;
    int numValuesForNodes = 0;
    /*Create vector for NodeTypes of observed nodes, numbers of really
    observed nodes and offsets*/
    m_isObsNow = intVector( m_numberObsNodes, 1 );
    m_offset.assign( nNodes + 1, 0 );//last is for determinig the size of last node
    for( i = 0; i < m_numberObsNodes; i++ )
    {

	if ( ( m_NodeTypes[i] )->IsDiscrete() )
	{
	    /*checking up the values from pValues*/
	    maxValue = m_NodeTypes[i]->GetNodeSize();
	    if( pValues[m_offset[i]].GetInt() >= maxValue )
	    {
		PNL_THROW( COutOfRange, "value of node is more than range" );
		/*every observed value should be less maxValue*/
	    }
	    else
	    {
		numValuesForNodes++;
	    }
	}
	else
	{
	    numValuesForNodes += (m_NodeTypes[i]->GetNodeSize());
	}
        m_offset[i + 1] = numValuesForNodes;
    }
    //check validity of input vector
    if( numValuesForNodes != int(m_rawValues.size()) )
    {
        PNL_THROW( CInconsistentSize, "size of vector with values"
	    " should corresponds node sizes of observed values" );
    }
}

CNodeValues::~CNodeValues()
{
}

int CNodeValues::GetNumberObsNodes()const
{
    return m_numberObsNodes;
}

const int *CNodeValues::GetObsNodesFlags()const
{
    return &m_isObsNow.front();
    /*here is only 1-s and 0-s in this array - look throw it to find really
    observed*/
}

void CNodeValues::SetData(const valueVector& data)
{
/*we set all data - if all nodes are really observed and we need to know
    all values*/
    if ( m_numberObsNodes )
    {
	/*put new data (values of noes as char array) instead of old data*/
        if( m_rawValues.size() != data.size() )
        {
            PNL_THROW( CInconsistentSize,
                "input data size must corresponds node types of observed nodes" );
        }
	m_rawValues.assign( data.begin(), data.end() );
    }
}
void CNodeValues::ToggleNodeStateBySerialNumber(int nNodes, const int *nodeNumbers)
{
/*to change the status of node from really observed
    to potentially observed and backwards*/
    int i,j;
    if( ( nNodes<0 ) || ( nNodes>m_numberObsNodes ) )
    {
	PNL_THROW(COutOfRange,
	    "number of nodes to toggle must be less than m_numberObsNodes")/**/
    }
    else
    {
    int flagAllRight = 1;/*all nodes from nodeIndices are potentially
			 observed (if it is so - all numbers of this nodes
    are in m_ObsNodes*/
    int thereIsSuchNode = 0;/*check-up flag for node - is it
    from m_ObsNodes?*/
    int *nodeIndices = new int[nNodes];
    PNL_CHECK_IF_MEMORY_ALLOCATED( nodeIndices );
    /*checking up all input data*/
    for ( i=0; i < nNodes; i++ )
    {
	for ( j=0; j < m_numberObsNodes; j++ )
	{
	    if( nodeNumbers[i] == j )
	    {
		thereIsSuchNode = 1;
		nodeIndices[i] = j;
		break;
	    }
	}
	if( !thereIsSuchNode )
	{
	    flagAllRight = 0;
	    break;
	}
    }
    if ( flagAllRight )
    {
	for( i=0; i < nNodes; i++ )
	{
	    m_isObsNow[nodeIndices[i]] = 1-m_isObsNow[nodeIndices[i]];
	    //fixme !!! is it enougth to use offset? - I think yes
	}
    }
    else
    {
	PNL_THROW( COutOfRange,
	    "some node has number which is not in m_obsNodes" ) /**/
    }
    delete []nodeIndices;
    }
}

const int *CNodeValues::GetOffset()const
{
    return &m_offset.front();
}

void CNodeValues::GetRawData(valueVector* values) const
{
    PNL_CHECK_IS_NULL_POINTER(values);
    values->assign( m_rawValues.begin(), m_rawValues.end() );
}
const CNodeType *const* CNodeValues::GetNodeTypes()const
{
    return &m_NodeTypes.front();
}

void CNodeValues::GetObsNodesFlags( intVector* obsNodesFlagsOut ) const
{
    
    obsNodesFlagsOut->assign( m_isObsNow.begin(), m_isObsNow.end() );
}


#ifdef PNL_RTTI
const CPNLType CNodeValues::m_TypeInfo = CPNLType("CNodeValues", &(CPNLBase::m_TypeInfo));

#endif

