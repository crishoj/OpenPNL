/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlModelDomain.cpp                                          //
//                                                                         //
//  Purpose:   CModelDomain class member functions implementation          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlModelDomain.hpp"
#include "pnlFactor.hpp"
#include "pnlGraphicalModel.hpp"

PNL_USING

CModelDomain*
CModelDomain::Create(int numVariables, const CNodeType& commonVariableType,
		     CGraphicalModel* pCreaterOfMD )
{
    PNL_CHECK_LEFT_BORDER( numVariables, 1 );

    CModelDomain* resDomain = new CModelDomain( numVariables, commonVariableType );
    PNL_CHECK_IF_MEMORY_ALLOCATED(resDomain);
    return resDomain;
}

CModelDomain*
CModelDomain::Create(const nodeTypeVector& variableTypes,
		     const intVector& variableAssociation,
		     CGraphicalModel* pCreaterOfMD )
{
    int numVarTypes = variableTypes.size();
    int numVars = variableAssociation.size();
#if 1
    if( numVars < numVarTypes )
    {
	PNL_THROW( CInconsistentSize,
	    "number of all variable types must be not more than number of variables" )
    }
#endif
    CModelDomain* resDomain = new CModelDomain( variableTypes,
	variableAssociation, pCreaterOfMD );
    PNL_CHECK_IF_MEMORY_ALLOCATED( resDomain );
    return resDomain;
}


int CModelDomain::AttachFactor(const CFactor *pFactor)
{
    int retNum;

    if( m_freeNumsInHeap.size() > 0 )
    {
	retNum = m_freeNumsInHeap.front();
	m_freeNumsInHeap.pop();

	m_factorsHeap[retNum] = pFactor;
    }
    else
    {
	retNum = m_factorsHeap.size();

	m_factorsHeap.push_back(pFactor);
    }

    return retNum;
}

void CModelDomain::ReleaseFactor(const CFactor *pFactor)
{
    int factorNum = pFactor->GetNumInHeap();

    m_factorsHeap[factorNum] = NULL;
    m_freeNumsInHeap.push(factorNum);
}

bool CModelDomain::IsAFactorOwner(const CFactor *pFactor)
{
    int numInHeap = pFactor->GetNumInHeap();
    if( numInHeap >= m_factorsHeap.size())
    {
        return 0;
    }
    PNL_CHECK_LEFT_BORDER(numInHeap, 0);
    return m_factorsHeap[numInHeap] == pFactor;
}

const CNodeType* CModelDomain::GetVariableType(int nodeNumber) const
{
    int numNodesInModelDom = m_variableAssociation.size();
    PNL_CHECK_RANGES( nodeNumber, 0, numNodesInModelDom - 1 );

    return &(m_variableTypes[m_variableAssociation[nodeNumber]]);
}

void CModelDomain::GetVariableTypes(intVector& vars,
	                            pConstNodeTypeVector* varTypes ) const
{
    int numNodesInModelDom = m_variableAssociation.size();
    int numVarTypes = vars.size();
    varTypes->resize( numVarTypes );
    for( int i = 0; i < numVarTypes; i++ )
    {
	int nodeNumber = vars[i];
	PNL_CHECK_RANGES( nodeNumber, 0, numNodesInModelDom - 1 );
	(*varTypes)[i] = &m_variableTypes[m_variableAssociation[nodeNumber]];
    }
}

void CModelDomain::GetVariableTypes( nodeTypeVector* varTypes )const
{
    PNL_CHECK_IS_NULL_POINTER( varTypes );

    varTypes->assign( m_variableTypes.begin(), m_variableTypes.end() );
}

void CModelDomain::GetVariableTypes( pConstNodeTypeVector* varTypes ) const
{
    PNL_CHECK_IS_NULL_POINTER( varTypes );

    int numVars = m_variableTypes.size();
    varTypes->resize( numVars );
    for( int i = 0; i < numVars; i++ )
    {
	(*varTypes)[i] = &m_variableTypes[m_variableAssociation[i]];
    }
}

void CModelDomain::GetVariableAssociations(intVector* nodeAssociation) const
{
    PNL_CHECK_IS_NULL_POINTER( nodeAssociation );

    nodeAssociation->assign(m_variableAssociation.begin(),
	m_variableAssociation.end());
}


CModelDomain::~CModelDomain()
{
    // destroy all the data
    int numOfRefModels = GetNumOfReferences();

    if( numOfRefModels > 1 )
    {
	PNL_THROW( CInvalidOperation, " can't release model domain,"
	    " there are models exist attached to it " );
    }
    else
    {
	if( ( ( numOfRefModels == 1 ) && m_bSelfCreated )
	    || ( numOfRefModels == 0 ) )
	{
	    int heapSize = m_factorsHeap.size();

	    int i = 0;

	    for( ; i < heapSize; ++i )
	    {
	        delete m_factorsHeap[i];
	    }

	    m_factorsHeap.clear();
	}
	else
	{
	    PNL_THROW( CInvalidOperation,
	        " can't release model domain,"
	        " there are models exist attached to it " );
	}
    }
}

static int IsNodeTypeNotInitialized(CNodeType nt)
{
    return nt.GetNodeSize() == -1;
}

CModelDomain::CModelDomain( int numVariables,
			   const CNodeType& commonVariableType,
			   CGraphicalModel* pCreaterOfMD )
			   : m_variableTypes( 1, commonVariableType ),
			   m_variableAssociation( numVariables, 0 ),
			   m_obsGauVarType( 0, 0 ),
			   m_obsTabVarType( 1, 1 ),
			   m_bSelfCreated(false)
{
    if( pCreaterOfMD == NULL )
    {
	AddRef(this);

	m_bSelfCreated = true;
    }
    else
    {
	AddRef(pCreaterOfMD);
    }
}

CModelDomain::CModelDomain( const nodeTypeVector& variableTypes,
			   const intVector& variableAssociation,
			   CGraphicalModel* pCreaterOfMD)
			   : m_variableTypes( variableTypes.begin(),
			   variableTypes.end() ),
			   m_variableAssociation( variableAssociation.begin(),
			   variableAssociation.end()),
			   m_obsGauVarType( 0, 0 ),
			   m_obsTabVarType( 1, 1 ),
			   m_bSelfCreated(false)
{
    m_factorsHeap.clear();
    nodeTypeVector::const_iterator ntIter;

    ntIter = std::find_if( m_variableTypes.begin(), m_variableTypes.end(),
	IsNodeTypeNotInitialized );

    if( ntIter != m_variableTypes.end() )
    {
	PNL_THROW( CInconsistentType,
	    " variable types have not been initialized " );
    }
    /* variable types validity check end */

    /* variable association validity check */
    intVector::const_iterator naIter = m_variableAssociation.begin();

    int numVarTypes = m_variableAssociation.size();

#if 1
    for( ; naIter != m_variableAssociation.end(); ++naIter )
    {
	PNL_CHECK_RANGES( *naIter, 0, numVarTypes - 1 );
    }
#endif

    if( pCreaterOfMD == NULL )
    {
	AddRef(this);

	m_bSelfCreated = true;
    }
    else
    {
	AddRef(pCreaterOfMD);
    }
}

void CModelDomain::ChangeNodeType(int NodeNumber, bool ToCont)
{
    int i;
    for (i = 0; i < m_variableTypes.size(); i++)
    {
        if (ToCont)
        {
            if ((!m_variableTypes[i].IsDiscrete())&&(m_variableTypes[i].GetNodeSize() == 1))
            {
                m_variableAssociation[NodeNumber] = i;
                break;
            }
        }
        else
        {
            if ((m_variableTypes[i].IsDiscrete())&&(m_variableTypes[i].GetNodeSize() == 2))
            {
                m_variableAssociation[NodeNumber] = i;
                break;
            }
        }
    }
}