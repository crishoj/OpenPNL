/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGraph.cpp                                                //
//                                                                         //
//  Purpose:   CGraph class member functions implementation                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlGraph.hpp"
#include "pnlMatrix.hpp"
#include "pnlImpDefs.hpp"
#include <assert.h>
#include "pnlLog.hpp"
#include "pnlLogDriver.hpp"

PNL_BEGIN

#define PNL_CHECK_IS_GRAPH_CHANGE_ALLOWED                                \
	if( m_bChangeAllowed == false ) {                                \
    PNL_THROW( CInvalidOperation, " graph change's not allowed " ); }

#define PNL_CHECK_IS_EXISTING_EDGE( __stNode, __enNode )                 \
    if( !IsExistingEdge( (__stNode), (__enNode) ) )	{                \
    PNL_THROW( CInvalidOperation,                                        \
    " trying to work with unexisting edge " ); }

#define PNL_CHECK_IS_GRAPH_NOT_EMPTY                                     \
	if( m_nbrsList.empty() ) {					 \
	PNL_THROW( CInconsistentState, " the graph is empty " );}

PNL_END

PNL_USING

#define PNL_GRAPH_EXPECTED_MAX_SIZE 40
#define PNL_GRAPH_EXPECTED_MAX_EDGES 512

CGraph* CGraph::Create( int numOfNds, const int *numOfNbrs, const int *const
		       *nbrsList, const ENeighborType *const *nbrsTypes )
{
    // bad-args checks
    if( numOfNbrs || nbrsList || nbrsTypes )
    {
	PNL_CHECK_LEFT_BORDER( numOfNds, 1 );
	PNL_CHECK_IS_NULL_POINTER(numOfNbrs);
	PNL_CHECK_IS_NULL_POINTER(nbrsList);
	PNL_CHECK_IS_NULL_POINTER(nbrsTypes);

	int i, j;

	for( i = 0; i < numOfNds; i++ )
	{
	    if( nbrsList[i] || nbrsTypes[i] )
	    {
		PNL_CHECK_RANGES( numOfNbrs[i], 0, numOfNds - 1 );
		PNL_CHECK_IS_NULL_POINTER(nbrsList[i]);
		PNL_CHECK_IS_NULL_POINTER(nbrsTypes[i]);

		for( j = 0; j < numOfNbrs[i]; j++ )
		{
		    PNL_CHECK_RANGES( nbrsList[i][j], 0, numOfNds - 1 );
		    PNL_CHECK_RANGES( nbrsTypes[i][j], 0, 2 );
		}
	    }
	    else
	    {
		PNL_CHECK_FOR_NON_ZERO(numOfNbrs[i]);
	    }
	}
    }
    else
    {
	PNL_CHECK_LEFT_BORDER( numOfNds, 0 );
    }
    // bad-args checks end

    CGraph *pGraph = new CGraph( numOfNds, numOfNbrs, nbrsList, nbrsTypes );

    PNL_CHECK_IF_MEMORY_ALLOCATED(pGraph);

    return pGraph;
}
//////////////////////////////////////////////////////////////////////////

CGraph* CGraph::Create( const intVecVector& nbrsList,
		       const neighborTypeVecVector& nbrsTypesList )
{
    if( nbrsList.size() != nbrsTypesList.size() )
    {
        PNL_THROW( CInvalidOperation,
            " number of neighbors and number of neighbors types dont match" );
    }

    int numOfNds = nbrsList.size();
    intVector numOfNbrs(numOfNds);
    pnlVector< const ENeighborType * > pNbrsTypesList(numOfNds);
    pnlVector< const int * >           pNbrsList(numOfNds);
    int i = 0;

    for( ; i < numOfNds; ++i )
    {
	numOfNbrs[i]      = nbrsList[i].size();
	pNbrsList[i]      = &nbrsList[i].front();
	pNbrsTypesList[i] = &nbrsTypesList[i].front();
    }

    return Create( numOfNds, &numOfNbrs.front(), &pNbrsList.front(),
	&pNbrsTypesList.front() );
}
//////////////////////////////////////////////////////////////////////////

CGraph* CGraph::Create( int numOfNds, const int *const *adjMat )
{
    int i, j;

    // bad-args check
    if( adjMat )
    {
	PNL_CHECK_LEFT_BORDER( numOfNds, 1 );

	for( i = 0; i < numOfNds; ++i )
	{
	    PNL_CHECK_IS_NULL_POINTER( adjMat[i] );

	    for( j = 0; j < numOfNds; ++j )
	    {
		PNL_CHECK_RANGES( adjMat[i][j], 0, 1 );
	    }
	}
    }
    else
    {
	PNL_CHECK_LEFT_BORDER( numOfNds, 0 );
    }
    // bad-args check end

    CGraph *pGraph = new CGraph( numOfNds, adjMat );

    PNL_CHECK_IF_MEMORY_ALLOCATED(pGraph);

    return pGraph;
}
//////////////////////////////////////////////////////////////////////////

CGraph* CGraph::Create(const CMatrix<int>* pAdjMat)
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pAdjMat);
    // bad-args check end

    CGraph *pGraph = new CGraph(pAdjMat);

    PNL_CHECK_IF_MEMORY_ALLOCATED(pGraph);

    return pGraph;
}
//////////////////////////////////////////////////////////////////////////

CGraph* CGraph::Copy(const CGraph *pGraph)
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pGraph);
    // bad-args check end

    CGraph *pGraphCopy = new CGraph(*pGraph);

    PNL_CHECK_IF_MEMORY_ALLOCATED(pGraphCopy);

    return pGraphCopy;
}
//////////////////////////////////////////////////////////////////////////

CGraph::CGraph( int numOfNds, const int *numOfNbrs,
	       const int *const *nbrsList,
	       const ENeighborType *const *nbrsTypes )
	       : m_nbrsList( numOfNds, (intVector *)0 ), m_nbrsTypes( numOfNds, (neighborTypeVector *)0 ),
	       m_bChangeAllowed(true)
{
    // no bad-args checks in constructor, all the checks done in Create

    if( nbrsList )
    {
	int i = 0;

	for( ; i < numOfNds; ++i )
	{
	    if( numOfNbrs[i] )
	    {
		m_nbrsList[i]  = new intVector( nbrsList[i], nbrsList[i]
		    + numOfNbrs[i] );

		m_nbrsTypes[i] = new neighborTypeVector( nbrsTypes[i],
		    nbrsTypes[i] + numOfNbrs[i] );

		PNL_CHECK_IF_MEMORY_ALLOCATED(m_nbrsList[i]);
		PNL_CHECK_IF_MEMORY_ALLOCATED(m_nbrsTypes[i]);

		SortNodeNeighbors(i);
	    }
	    else
	    {
		m_nbrsList[i]  = new intVector;
		m_nbrsTypes[i] = new neighborTypeVector;

		PNL_CHECK_IF_MEMORY_ALLOCATED(m_nbrsList[i]);
		PNL_CHECK_IF_MEMORY_ALLOCATED(m_nbrsTypes[i]);
	    }
	}
    }
    else
    {
	CreateEmptyNbrsListForAllNodes();
    }
}
//////////////////////////////////////////////////////////////////////////

CGraph::CGraph(const CMatrix<int>* pAdjMat) : m_bChangeAllowed(true)
{
    // no bad-args checks in constructor, all the checks done in Create
    assert( pAdjMat != NULL );

    int       numOfDims;
    const int *ranges;

    pAdjMat->GetRanges( &numOfDims, &ranges );

    // adjacency matrix is always 2D square
    if( ( numOfDims != 2 ) || ( ranges[0] != ranges[1] ) )
    {
        PNL_THROW( CInvalidOperation,
            " adj. mat. should be a square 2D matrix " );
    }

    m_nbrsList.assign( *ranges, (intVector *)0 );

    m_nbrsTypes.assign( *ranges, (neighborTypeVector *)0 );

    CreateEmptyNbrsListForAllNodes();


    intVector indices, rindices(2);

    CMatrixIterator<int> *iter = pAdjMat->InitIterator();

    for( ; pAdjMat->IsValueHere(iter); pAdjMat->Next(iter) )
    {
	pAdjMat->Index( iter, &indices );

        // assure adj. matrix is a square 2D matrix
	assert( indices.size() == 2 );

        if( pAdjMat->GetElementByIndexes( &indices.front() ) == 1 )
        {
	    rindices[0] = indices[1];
	    rindices[1] = indices[0];

            if( pAdjMat->GetElementByIndexes( &rindices.front() ) == 1 )
            {
                // assure all the diagonal elements are zeros
                assert( indices[0] != indices[1] );

                if( indices[1] > indices[0] )
                {
                    AddEdge( indices[0], indices[1], 0 );
                }
            }
            else if( pAdjMat->GetElementByIndexes( &rindices.front() ) != 0 )
            {
                PNL_THROW( CInvalidOperation,
                    " there should be only zeros and ones in adj. mat " );
            }
            else
            {
                AddEdge( indices[0], indices[1], 1 );
            }
        }
        else if( pAdjMat->GetElementByIndexes( &indices.front() ) != 0 )
        {
            PNL_THROW( CInvalidOperation,
                " there should be only zeros and ones in adj. mat " );
        }
    }

    delete iter;
}
//////////////////////////////////////////////////////////////////////////

CGraph::CGraph( int numOfNds, const int *const *adjMat )
: m_bChangeAllowed(true), m_nbrsList( numOfNds, (intVector *)0 ),
m_nbrsTypes( numOfNds, (neighborTypeVector *)0 )
{
    // no bad-args checks in constructor, all the checks done in Create
    CreateEmptyNbrsListForAllNodes();

    int i, j;

    if( adjMat )
    {
	for( i = 0; i < numOfNds; ++i )
	{
	    for( j = 0; j < numOfNds; ++j )
	    {
		if( adjMat[i][j] == 1 )
		{
		    if( adjMat[j][i] == 1 )
		    {
			if( j > i )
			{
			    AddEdge( i, j, 0 );
			}
		    }
		    else
		    {
			AddEdge( i, j, 1 );
		    }
		}
	    }
	}
    }
}
//////////////////////////////////////////////////////////////////////////

CGraph::CGraph(const CGraph& rGraph)
: m_nbrsList( rGraph.m_nbrsList.size(), NULL ),
m_nbrsTypes( rGraph.m_nbrsList.size(), NULL ),
m_bChangeAllowed(true)
{
    // no bad-args checks in constructor, all the checks done in Create

    if( !rGraph.m_nbrsList.empty() )
    {
	intVecPVector::iterator          listIt  = m_nbrsList.begin();
	neighborTypeVecPVector::iterator typesIt = m_nbrsTypes.begin();

	intVecPVector::const_iterator    list_end = m_nbrsList.end();

	intVecPVector::const_iterator gr_listIt = rGraph.m_nbrsList.begin();
	neighborTypeVecPVector::const_iterator gr_typesIt = rGraph.
	    m_nbrsTypes.begin();

	for( ; listIt != list_end; ++listIt, ++typesIt, ++gr_listIt,
	    ++gr_typesIt )
	{
	    *listIt = new intVector( (*gr_listIt)->begin(),
		(*gr_listIt)->end() );

	    *typesIt = new neighborTypeVector( (*gr_typesIt)->begin(),
		(*gr_typesIt)->end() );

	    PNL_CHECK_IF_MEMORY_ALLOCATED(*listIt);
	    PNL_CHECK_IF_MEMORY_ALLOCATED(*typesIt);
	}
    }
}
//////////////////////////////////////////////////////////////////////////

CGraph::~CGraph()
{
    m_bChangeAllowed = true;

    ClearGraph();
}
//////////////////////////////////////////////////////////////////////////

CGraph* CGraph::MoralizeGraph(const CGraph *pGraph)
{
    // moralize means to add edges between the parents
    // of each node and change all the directed edges to undirected

    // note: this function creates a new pGraph, which is a "moral"
    // analogue of the input one, and then returns a pointer to it

    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(pGraph);
    /* bad-args check */

    int                                i;
    intVector::const_iterator          nbrsIt, nbrs_end;
    neighborTypeVector::const_iterator nbrsTypeIt;

    const int numOfNds = pGraph->m_nbrsList.size();

    intVector                 parents;
    intVector::const_iterator parentIt, parents_end;

    CGraph *pMoralGraph = new CGraph( numOfNds, NULL, NULL, NULL );

    PNL_CHECK_IF_MEMORY_ALLOCATED(pMoralGraph);

    for( i = 0; i < numOfNds; i++ )
    {
	parents.reserve(pGraph->GetNumberOfParents(i));

	for( nbrsIt = pGraph->m_nbrsList[i]->begin(),
	    nbrs_end = pGraph->m_nbrsList[i]->end(),
	    nbrsTypeIt = pGraph->m_nbrsTypes[i]->begin(); nbrs_end - nbrsIt;
	++nbrsIt, ++nbrsTypeIt )
	{
	    if( *nbrsTypeIt != ntChild )
	    {
		pMoralGraph->AddEdge( i, *nbrsIt, 0 );

		if( *nbrsTypeIt == ntParent )
		{
		    /* connecting all the parents between each other */
		    for( parentIt = parents.begin(),
			parents_end = parents.end(); parents_end - parentIt;
		    ++parentIt )
		    {
			pMoralGraph->AddEdge( *nbrsIt, *parentIt, 0 );
		    }

		    parents.push_back(*nbrsIt);
		}
	    }
	}

	parents.clear();
    }

    return pMoralGraph;
}
//////////////////////////////////////////////////////////////////////////

void CGraph::GetTopologicalOrder( intVector *NewToOldMap ) const
{
    int i, j;
    int cog, cur;
    intVector cogs;

    PNL_DEFINE_AUTOBUF( bool, mask, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_MAKE_LOCAL( int, sz, this, GetNumberOfNodes() );

    PNL_INIT_AUTOBUF( mask, sz, false );

    NewToOldMap->clear();
    NewToOldMap->resize( sz );

    cog = 0;
    cogs.assign( sz, 0 );

    for ( cur = 0; cur < sz; ++cur )
    {
        i = cog;
loop:
        for ( j = cogs[i]; j < m_nbrsList[i]->size(); ++j )
        {
            if ( !mask[(*m_nbrsList[i])[j]] )
            {
                if ( (*m_nbrsTypes[i])[j] == ntParent )
                {
                    cogs[i] = j;
                    i = (*m_nbrsList[i])[j];
                    goto loop;
                }
            }
        }
        mask[(*NewToOldMap)[cur] = i] = true;
        while ( cog < sz && mask[cog] )
        {
            ++cog;
        }
    }

    PNL_RELEASE_AUTOBUF( mask );
}

//////////////////////////////////////////////////////////////////////////

void CGraph::AddNodes(int newNumOfNds)
{
    int oldNumOfNds = m_nbrsList.size();

    /* bad-args check */
    PNL_CHECK_LEFT_BORDER( newNumOfNds, oldNumOfNds );
    /* bad-args check end */

    /* operation validity check */
    PNL_CHECK_IS_GRAPH_CHANGE_ALLOWED;
    /* operation validity check end */

    m_nbrsList.resize( newNumOfNds, NULL );
    m_nbrsTypes.resize( newNumOfNds, NULL );

    CreateEmptyNbrsListForAllNodes();
}
//////////////////////////////////////////////////////////////////////////

// an OBSOLETE version of this function
void CGraph::FormCliqueFromSubgraph( int numOfNds, const int *subGraph )
{
    /* bad-args check */
    PNL_CHECK_RANGES( numOfNds, 2, m_nbrsList.size() );
    PNL_CHECK_IS_NULL_POINTER(subGraph);
    /* bad-args check end */

    const int *sgIt1 = subGraph, *sgIt2, *subGr_end = subGraph + numOfNds;

    for( ; subGr_end - sgIt1 - 1; ++sgIt1 )
    {
	for( sgIt2 = sgIt1 + 1; subGr_end - sgIt2; ++sgIt2 )
	{
	    AddEdge( *sgIt1, *sgIt2, 0 );
	}
    }
}
//////////////////////////////////////////////////////////////////////////

void CGraph::FormCliqueFromSubgraph(const intVector& subGraph)
{
    // bad-args check
    PNL_CHECK_RANGES( subGraph.size(), 2, m_nbrsList.size() );
    // bad-args check end

    intVector::const_iterator sgIt1 = subGraph.begin(),
	sgIt2,
	sg_end = subGraph.end();

    for( ; sgIt1 + 1 != sg_end; ++sgIt1 )
    {
        for( sgIt2 = sgIt1 + 1; sgIt2 != sg_end; ++sgIt2 )
        {
            AddEdge( *sgIt1, *sgIt2, 0 );
        }
    }
}
//////////////////////////////////////////////////////////////////////////

void CGraph::GetNeighbors( int nodeNum, int *numOfNbrs, const int **nbrs,
			  const ENeighborType **nbrsTypes ) const
{
    /* operation validity check */
    PNL_CHECK_IS_GRAPH_NOT_EMPTY;
    /* operation validity check end */

    /* bad-args check */
    PNL_CHECK_RANGES( nodeNum, 0, m_nbrsList.size() - 1 );
    PNL_CHECK_IS_NULL_POINTER(numOfNbrs);
    PNL_CHECK_IS_NULL_POINTER(nbrs);
    PNL_CHECK_IS_NULL_POINTER(nbrsTypes);
    /* bad-args check end */

    *numOfNbrs = (*( m_nbrsList.begin()  + nodeNum ))->size();
    *nbrs      = &(*( m_nbrsList.begin()  + nodeNum ))->front();
    *nbrsTypes = &(*( m_nbrsTypes.begin() + nodeNum ))->front();
}
//////////////////////////////////////////////////////////////////////////

void CGraph::GetNeighbors( int nodeNum, intVector* nbrs,
			  neighborTypeVector *nbrsTypes ) const
{
    /* operation validity check */
    PNL_CHECK_IS_GRAPH_NOT_EMPTY;
    /* operation validity check end */

    /* bad-args check */
    PNL_CHECK_RANGES( nodeNum, 0, m_nbrsList.size() - 1 );
    PNL_CHECK_IS_NULL_POINTER(nbrs);
    PNL_CHECK_IS_NULL_POINTER(nbrsTypes);
    /* bad-args check end */

    nbrs->assign( m_nbrsList[nodeNum]->begin(), m_nbrsList[nodeNum]->end() );

    nbrsTypes->assign( m_nbrsTypes[nodeNum]->begin(),
	m_nbrsTypes[nodeNum]->end() );
}
//////////////////////////////////////////////////////////////////////////

void CGraph::SetNeighbors( int nodeNum, int numOfNbrs, const int *nbrs,
			  const ENeighborType *nbrsTypes )
{
    /* operation validity check */
    PNL_CHECK_IS_GRAPH_NOT_EMPTY;
    PNL_CHECK_IS_GRAPH_CHANGE_ALLOWED;
    /* operation validity check end */

    int numOfNds = m_nbrsList.size();

    /* bad-args check */
    PNL_CHECK_RANGES( nodeNum,   0, numOfNds - 1 );
    PNL_CHECK_RANGES( numOfNbrs, 1, numOfNds - 1 );
    PNL_CHECK_IS_NULL_POINTER(nbrs);
    PNL_CHECK_IS_NULL_POINTER(nbrsTypes);
    /* bad-args check end */

    int i = 0;

    for( ; i < numOfNbrs; i++ )
    {
	AddToNodeNeighbors( nodeNum, nbrs[i], nbrsTypes[i] );

	if( nbrsTypes[i] != ntNeighbor )
	{
	    AddToNodeNeighbors( nbrs[i], nodeNum, nbrsTypes[i] == ntChild ?
ntParent : ntChild );
	}
	else
	{
	    AddToNodeNeighbors( nbrs[i], nodeNum, ntNeighbor );
	}
    }
}
//////////////////////////////////////////////////////////////////////////

void CGraph::SetNeighbors( int nodeNum, const intVector& nbrs,
			  const neighborTypeVector& nbrsTypes )
{
    SetNeighbors( nodeNum, nbrs.size(), &nbrs.front(), &nbrsTypes.front() );
}
//////////////////////////////////////////////////////////////////////////

void CGraph::AddEdge( int startNode, int endNode, int directed )
{
    /* set directed != 0 if the edge to add is directed and == 0 otherwise */

    int numOfNds = m_nbrsList.size();

    /* bad-args check */
    PNL_CHECK_RANGES( startNode, 0, numOfNds - 1 );
    PNL_CHECK_RANGES( endNode,   0, numOfNds - 1 );
    /* bad-args check end */

    /* operation validity check */
    PNL_CHECK_IS_GRAPH_CHANGE_ALLOWED;
    /* operation validity check end */

    AddToNodeNeighbors( startNode, endNode  ,
	directed ? ntChild  : ntNeighbor );

    AddToNodeNeighbors( endNode  , startNode,
	directed ? ntParent : ntNeighbor );
}
//////////////////////////////////////////////////////////////////////////

void CGraph::RemoveEdge( int startNode, int endNode )
{
    /* operation validity check */
    PNL_CHECK_IS_GRAPH_NOT_EMPTY;
    PNL_CHECK_IS_GRAPH_CHANGE_ALLOWED;
    PNL_CHECK_IS_EXISTING_EDGE( startNode, endNode );
    /* operation validity check end */

    int numOfNds = m_nbrsList.size();

    /* bad-args check */
    PNL_CHECK_RANGES( startNode, 0, numOfNds - 1 );
    PNL_CHECK_RANGES( endNode,   0, numOfNds - 1 );
    /* bad-args check end */

    RemoveFromNodeNeighbors( startNode, endNode   );
    RemoveFromNodeNeighbors( endNode,   startNode );
}
//////////////////////////////////////////////////////////////////////////

void CGraph::ChangeEdgeDirection( int startNode, int endNode )
{
    /* operation validity check */
    PNL_CHECK_IS_GRAPH_NOT_EMPTY;
    PNL_CHECK_IS_GRAPH_CHANGE_ALLOWED;
    PNL_CHECK_IS_EXISTING_EDGE( startNode, endNode );
    /* operation validity check end */

    int numOfNds = m_nbrsList.size();

    /* bad-args check */
    PNL_CHECK_RANGES( startNode, 0, numOfNds - 1 );
    PNL_CHECK_RANGES( endNode,   0, numOfNds - 1 );
    /* bad-args check end */

    int offset;
    int nbrType;

    intVector          *nbrs;
    neighborTypeVector *nbrsType;

    nbrs     = *( m_nbrsList.begin()  + startNode );
    nbrsType = *( m_nbrsTypes.begin() + startNode );

    offset = std::find( nbrs->begin(), nbrs->end(), endNode ) - nbrs->begin();

    nbrType = *( nbrsType->begin() + offset );

    if( nbrType == ntNeighbor )
    {
	PNL_THROW( CInvalidOperation,
	    " changing direction of an undirected edge " );
    }

    *( nbrsType->begin() + offset ) = nbrType == ntParent
	? ntChild : ntParent;

    nbrs     = *( m_nbrsList.begin()  + endNode );
    nbrsType = *( m_nbrsTypes.begin() + endNode );

    offset = std::find( nbrs->begin(), nbrs->end(), startNode )
	- nbrs->begin();

    nbrType = *( nbrsType->begin() + offset );

    if( nbrType == ntNeighbor )
    {
	PNL_THROW( CInvalidOperation,
	    " changing direction of an undirected edge " );
    }

    *( nbrsType->begin() + offset ) = nbrType == ntParent
	? ntChild : ntParent;
}
//////////////////////////////////////////////////////////////////////////

int CGraph::IsDAG() const
{
    /* DAG == directed acyclic graph (without directed cycles only) */
    /* implemented with the BFS, cause the cycle-closing node can be
    found on any search layer */

    /* operation validity check */
    PNL_CHECK_IS_GRAPH_NOT_EMPTY;
    /* operation validity check end */

    PNL_DEFINE_AUTOBUF( bool, ban, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEMAND_AUTOBUF( ban, GetNumberOfNodes() );

    intVecPVector::const_iterator          nlIter = m_nbrsList.begin();
    neighborTypeVecPVector::const_iterator ntIter = m_nbrsTypes.begin();

    intVector::const_iterator          nbIter;
    neighborTypeVector::const_iterator tyIter;

    intQueue nodeQueue;

    int node;
    int neighbor;
    int i;

    for( node = 0; node < m_nbrsList.size(); node++ )
    {
	nodeQueue.push(node);

        for ( i = GetNumberOfNodes(); i--; )
        {
            ban[i] = false;
        }

	while( !nodeQueue.empty() )
	{
	    neighbor = nodeQueue.front();

	    nbIter = (*( nlIter + neighbor ))->begin();
	    tyIter = (*( ntIter + neighbor ))->begin();

	    for( ; (*( nlIter + neighbor ))->end() != nbIter; nbIter++,
		tyIter++ )
	    {
		if( (*tyIter) == ntNeighbor )
		{
		    /* DAG is a DIRECTED acyclic graph! Got the idea? */
		    PNL_THROW( CInconsistentType,
			" the graph has to be directed " );
		}

		if( (*tyIter) == ntChild )
		{
		    if( (*nbIter) == node )
		    {
			/* a cycle found in the graph */
			return 0;
		    }

                    if ( !ban[*nbIter] )
                    {
                        nodeQueue.push( *nbIter );
                        ban[*nbIter] = true;
                    }
		}
	    }

	    nodeQueue.pop();
	}
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////////

int CGraph::IsBinaryTree() const
{
    int res = 0;
    int NNodes = GetNumberOfNodes();
    int i;
	
    if (NNodes == 1) return 1; // tree with the only root node
    
    if ((IsDAG() == 1) && (NumberOfConnectivityComponents() == 1) && (IsTopologicallySorted() == 1) )
    {
        //finding the root
        int root = 0;
        //investigating the graph
        res = 1;
        for (i = 0; i < NNodes; i++)
        {
            if ( ! (((i == root) && (GetNumberOfChildren(i) == 2)) ||
                ((i != root) && (GetNumberOfParents(i) == 1) 
                && ((GetNumberOfChildren(i) == 2) || (GetNumberOfChildren(i) == 0)))))
            {
                res = 0;
                break;
            }
        }
    }
    return res;
}
//////////////////////////////////////////////////////////////////////////

int CGraph::IsTopologicallySorted() const
{
/* the graph is said to be topologically sorted if numbers
    of children are greater than numbers of parents */

    /* operation validity check */
    PNL_CHECK_IS_GRAPH_NOT_EMPTY;
    /* operation validity check end */

    intVecPVector::const_iterator          nlIter = m_nbrsList.begin();
    neighborTypeVecPVector::const_iterator ntIter = m_nbrsTypes.begin();

    intVector::const_iterator          nbIter;
    neighborTypeVector::const_iterator tyIter;

    for( ; m_nbrsList.end()	- nlIter; nlIter++, ntIter++ )
    {
	nbIter = (*nlIter)->begin();
	tyIter = (*ntIter)->begin();

	for( ;(*nlIter)->end() != nbIter; nbIter++, tyIter++ )
	{

	    if( *tyIter == ntNeighbor )
	    {
		/* topological sort is not defined for undirected graphs */
		PNL_THROW( CInconsistentType,
		    " graph has to be directed " );
	    }

	    if( ( *tyIter == ntParent )
		&&  ( *nbIter > ( nlIter - m_nbrsList.begin() ) ) )
	    {
	    /* there is a node in the graph, which has a parent	with
		a number, greater than the number of the node itself */
		return 0;
	    }

	    if( ( *tyIter == ntChild)
		&& ( *nbIter < ( nlIter - m_nbrsList.begin() ) ) )
	    {
	    /* there is a node in the graph, which has a child
		with a number, smaller than the number of that node */
		return 0;
	    }
	}
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////

int CGraph::NumberOfConnectivityComponents() const
{
    /* operation validity check */
    PNL_CHECK_IS_GRAPH_NOT_EMPTY;
    /* operation validity check end */

    /* if the graph is not empty there is at least one component */
    int numOfComponents = 1;

    intVecPVector::const_iterator nlIter = m_nbrsList.begin();
    intVector::const_iterator     nbIter;

    boolVector                 nodeMarks;
    boolVector::const_iterator location;

    int      startNode = 0;
    int      currentNode;
    intQueue nodeQueue;

    nodeMarks.assign( m_nbrsList.size(), false );

    for(;;)
    {
	/* add a start node to the list of nodes to be marked and mark it */
	nodeQueue.push(startNode);
	nodeMarks[startNode] = true;

	while( !nodeQueue.empty() )
	{
	    /* mark all the unmarked neighbors of all the nodes in the list */
	    currentNode = nodeQueue.front();

	    nbIter = (*( nlIter + currentNode ))->begin();

	    for( ; (*( nlIter + currentNode ))->end() != nbIter; nbIter++ )
	    {
		if( !nodeMarks[*nbIter] )
		{
		    nodeQueue.push(*nbIter);
		    nodeMarks[*nbIter] = true;
		}
	    }

	    nodeQueue.pop();
	}

	/* look if there are any unmarked nodes remain */
	location = std::find( nodeMarks.begin(), nodeMarks.end(), false );

	if( location == nodeMarks.end() )
	{
	    break;
	}
	else
	{
	    startNode = location - nodeMarks.begin();
	    numOfComponents++;
	}
    }

    return numOfComponents;
}
//////////////////////////////////////////////////////////////////////////

CGraph& CGraph::operator=(const CGraph& graph)
{
    // the copy of the graph is allowed for change
    m_bChangeAllowed = true;

    ClearGraph();

    if( graph.m_nbrsList.empty() )
    {
	return *this;
    }

    int numOfNds = graph.m_nbrsList.size();

    m_nbrsList.resize(numOfNds);
    m_nbrsTypes.resize(numOfNds);

    intVecPVector::iterator          nlIter = m_nbrsList.begin();
    neighborTypeVecPVector::iterator ntIter = m_nbrsTypes.begin();

    intVecPVector::const_iterator gr_nlIter = graph.m_nbrsList.begin();
    neighborTypeVecPVector::const_iterator gr_ntIter
	= graph.m_nbrsTypes.begin();

    for(; m_nbrsList.end() - nlIter; nlIter++, ntIter++,
	gr_nlIter++, gr_ntIter++ )
    {
	*nlIter = new intVector( (*gr_nlIter)->begin(), (*gr_nlIter)->end() );
	*ntIter = new neighborTypeVector( (*gr_ntIter)->begin(),
	    (*gr_ntIter)->end() );

	PNL_CHECK_IF_MEMORY_ALLOCATED(*nlIter);
	PNL_CHECK_IF_MEMORY_ALLOCATED(*ntIter);
    }

    return *this;
}
//////////////////////////////////////////////////////////////////////////

void CGraph::SortNodeNeighbors(int nodeNum)
{
    /* operation validity check */
    PNL_CHECK_IS_GRAPH_NOT_EMPTY;
    PNL_CHECK_IS_GRAPH_CHANGE_ALLOWED;
    /* operation validity check end */

    int numOfNds = m_nbrsList.size();

    /* bad-args check */
    PNL_CHECK_RANGES( nodeNum, 0, numOfNds - 1 )
	/* bad-args check end */

	int i, j;

    int              tmpNbr;
    ENeighborType tmpNbrType;

    intVector               *nbrs = *( m_nbrsList.begin()  + nodeNum );
    neighborTypeVector *nbrsTypes = *( m_nbrsTypes.begin() + nodeNum );

    for( i = nbrs->size() - 1; i > 0; i-- )
    {
	for( j = 0; j < i; j++ )
	{
	    if( (*nbrs)[j] > (*nbrs)[j + 1])
	    {
		tmpNbr = (*nbrs)[j + 1];
		(*nbrs)[j + 1] = (*nbrs)[j];
		(*nbrs)[j] = tmpNbr;

		tmpNbrType = (*nbrsTypes)[j + 1];
		(*nbrsTypes)[j + 1] = (*nbrsTypes)[j];
		(*nbrsTypes)[j] = tmpNbrType;
	    }
	}
    }
}
//////////////////////////////////////////////////////////////////////////

void CGraph::Dump() const
{
    Log dump("", eLOG_RESULT, eLOGSRV_PNL);

    if( m_nbrsList.empty() )
    {
	dump<<" the graph is empty\n";
	dump.flush();
	return;
    }

    intVecPVector::const_iterator          nlIter = m_nbrsList.begin();
    neighborTypeVecPVector::const_iterator ntIter = m_nbrsTypes.begin();

    intVector::const_iterator          nbIter;
    neighborTypeVector::const_iterator tyIter;

    const char *strTypeId;

    dump << '\n';
    for( ; m_nbrsList.end() - nlIter; nlIter++, ntIter++ )
    {
	nbIter = (*nlIter)->begin();
	tyIter = (*ntIter)->begin();

	dump<< ' ' << nlIter - m_nbrsList.begin()<<" : ";

	for( ; (*nlIter)->end() != nbIter; nbIter++, tyIter++ )
	{
	    if( *tyIter == ntNeighbor )
	    {
		strTypeId = "(n)";
	    }
	    else
	    {
		strTypeId = *tyIter == ntParent ? "(p)" : "(c)";
	    }

	    dump<<*nbIter<<"_"<<strTypeId << ' ';
	}

	dump << '\n';
    }

    dump << '\n';
    dump.flush();
}
//////////////////////////////////////////////////////////////////////////

int CGraph::GetNumberOfEdges() const
{
    int i;
    int coun;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );

    for ( i = graph_size, coun = 0; i--; )
    {
        coun += m_nbrsList[i]->size();
    }

    return coun / 2;
}
//////////////////////////////////////////////////////////////////////////

void CGraph::GetAncestry( intVector const &subgraph,
			 intVector *output ) const
{
    int i, j;
    int a, b;
    intVector::iterator it;
    neighborTypeVector::iterator nt;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sub_size, &subgraph, size() );

    PNL_DEFINE_AUTOBUF( bool, mask, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOMULSTACK( int, bin, 2, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_INIT_AUTOBUF( mask, graph_size, false );
    PNL_DEMAND_AUTOMULSTACK( bin, graph_size );

    for ( i = sub_size ; i--; )
    {
        PNL_AUTOMULSTACK_PUSH( bin, 0, subgraph[i] );
        mask[subgraph[i]] = true;
    }
    for ( a = 0, b = 1; PNL_AUTOMULSTACK_SIZE( bin, a ); a = b, b = 1 - a )
    {
        PNL_AUTOMULSTACK_CLEAR( bin, b );
        while ( PNL_AUTOMULSTACK_SIZE( bin, a ) )
        {
            j = PNL_AUTOMULSTACK_POP( bin, a );
            for ( it = m_nbrsList[j]->begin(), nt = m_nbrsTypes[j]->begin();
                  it != m_nbrsList[j]->end(); ++it, ++nt )
            {
                if ( *nt == ntParent && !mask[*it] )
                {
                    mask[*it] = true;
                    PNL_AUTOMULSTACK_PUSH( bin, b, *it );
                    output->push_back( *it );
                }
            }
        }
    }
    PNL_RELEASE_AUTOMULSTACK( bin );
    PNL_RELEASE_AUTOBUF( mask );
}
//////////////////////////////////////////////////////////////////////////

void CGraph::GetAncestralClosure( intVector const &subgraph,
                                  intVector *output ) const
{
    int i, j;
    int a, b;
    intVector::iterator it;
    neighborTypeVector::iterator nt;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sub_size, &subgraph, size() );

    PNL_DEFINE_AUTOBUF( bool, mask, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOMULSTACK( int, bin, 2, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_INIT_AUTOBUF( mask, graph_size, false );
    PNL_DEMAND_AUTOMULSTACK( bin, graph_size );

    for ( i = sub_size; i--; )
    {
        PNL_AUTOMULSTACK_PUSH( bin, 0, subgraph[i] );
        mask[subgraph[i]] = true;
        output->push_back( subgraph[i] );
    }
    for ( a = 0, b = 1; PNL_AUTOMULSTACK_SIZE( bin, a ); a = b, b = 1 - a )
    {
        PNL_AUTOMULSTACK_CLEAR( bin, b );
        while ( PNL_AUTOMULSTACK_SIZE( bin, a ) )
        {
            j = PNL_AUTOMULSTACK_POP( bin, a );
            for ( it = m_nbrsList[j]->begin(), nt = m_nbrsTypes[j]->begin();
                  it != m_nbrsList[j]->end(); ++it, ++nt )
            {
                if ( *nt == ntParent && !mask[*it] )
                {
                    mask[*it] = true;
                    PNL_AUTOMULSTACK_PUSH( bin, b, *it );
                    output->push_back( *it );
                }
            }
        }
    }
    PNL_RELEASE_AUTOMULSTACK( bin );
    PNL_RELEASE_AUTOBUF( mask );
}
//////////////////////////////////////////////////////////////////////////

void CGraph::GetAncestralClosure( intVector const &subgraph,
                                  boolVector *output ) const
{
    int i, j;
    int a, b;
    intVector::iterator it;
    neighborTypeVector::iterator nt;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sub_size, &subgraph, size() );

    PNL_DEFINE_AUTOMULSTACK( int, bin, 2, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_DEMAND_AUTOMULSTACK( bin, graph_size );

    output->resize( graph_size );
    for ( i = graph_size; i--; )
    {
        (*output)[i] = false;
    }
    for ( i = sub_size; i--; )
    {
        PNL_AUTOMULSTACK_PUSH( bin, 0, subgraph[i] );
        (*output)[subgraph[i]] = true;
    }
    for ( a = 0, b = 1; PNL_AUTOMULSTACK_SIZE( bin, a ); a = b, b = 1 - a )
    {
        PNL_AUTOMULSTACK_CLEAR( bin, b );
        while ( PNL_AUTOMULSTACK_SIZE( bin, a ) )
        {
            j = PNL_AUTOMULSTACK_POP( bin, a );
            for ( it = m_nbrsList[j]->begin(), nt = m_nbrsTypes[j]->begin();
                  it != m_nbrsList[j]->end(); ++it, ++nt )
            {
                if ( *nt == ntParent && !(*output)[*it] )
                {
                    PNL_AUTOMULSTACK_PUSH( bin, b, *it );
                    (*output)[*it] = true;
                }
            }
        }
    }
    PNL_RELEASE_AUTOMULSTACK( bin );
}
//////////////////////////////////////////////////////////////////////////

void CGraph::GetSubgraphConnectivityComponents( intVector const &subgraph,
                                                intVecVector *output ) const
{
    int i, j;
    int cog;
    int a, b;
    intVector::iterator it;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sub_size, &subgraph, size() );

    PNL_DEFINE_AUTOBUF( bool, mask, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOMULSTACK( int, bin, 2, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_INIT_AUTOBUF( mask, graph_size, false );
    PNL_DEMAND_AUTOMULSTACK( bin, sub_size );

    for ( i = sub_size; i--; )
    {
        mask[subgraph[i]] = true;
    }
    output->clear();
    for ( cog = 0; ; )
    {
        for ( ;; ++cog )
        {
            if ( cog == sub_size )
            {
                PNL_RELEASE_AUTOMULSTACK( bin );
                PNL_RELEASE_AUTOBUF( mask );

                return;
            }
            if ( mask[subgraph[cog]] )
            {
                break;
            }
        }
        output->push_back( intVector() );
        PNL_AUTOMULSTACK_PUSH( bin, 0, subgraph[cog] );
        mask[subgraph[cog]] = false;
        output->back().push_back( subgraph[cog++] );
        for ( a = 0, b = 1; PNL_AUTOMULSTACK_SIZE( bin, a ); a = b, b = 1 - a )
        {
            PNL_AUTOMULSTACK_CLEAR( bin, b );
            while ( PNL_AUTOMULSTACK_SIZE( bin, a ) )
            {
                j = PNL_AUTOMULSTACK_POP( bin, a );
                for ( it = m_nbrsList[j]->begin(); it != m_nbrsList[j]->end(); ++it )
                {
                    if ( mask[*it] )
                    {
                        mask[*it] = false;
                        PNL_AUTOMULSTACK_PUSH( bin, b, *it );
                        output->back().push_back( *it );
                    }
                }
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////

void CGraph::GetConnectivityComponents( intVecVector *output ) const
{
    int j;
    int cog;
    int a, b;
    intVector::iterator it;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );

    PNL_DEFINE_AUTOBUF( bool, mask, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOMULSTACK( int, bin, 2, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_INIT_AUTOBUF( mask, graph_size, false );
    PNL_DEMAND_AUTOMULSTACK( bin, graph_size );

    output->clear();
    for ( cog = 0; ; )
    {
        for ( ;; ++cog )
        {
            if ( cog == graph_size )
            {
                PNL_RELEASE_AUTOMULSTACK( bin );
                PNL_RELEASE_AUTOBUF( mask );

                return;
            }
            if ( !mask[cog] )
            {
                break;
            }
        }
        output->push_back( intVector() );
        PNL_AUTOMULSTACK_PUSH( bin, 0, cog );
        mask[cog] = true;
        output->back().push_back( cog++ );
        for ( a = 0, b = 1; PNL_AUTOMULSTACK_SIZE( bin, a ); a = b, b = 1 - a )
        {
            PNL_AUTOMULSTACK_CLEAR( bin, b );
            while ( PNL_AUTOMULSTACK_SIZE( bin, a ) )
            {
                j = PNL_AUTOMULSTACK_POP( bin, a );
                for ( it = m_nbrsList[j]->begin(); it != m_nbrsList[j]->end(); ++it )
                {
                    if ( !mask[*it] )
                    {
                        mask[*it] = true;
                        PNL_AUTOMULSTACK_PUSH( bin, b, *it );
                        output->back().push_back( *it );
                    }
                }
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////

struct Arrow { int node; int from; };

/* preliminary approach, now obsolete */
void CGraph::GetReachableSubgraph( intVector const &subgraph, bool *ban[],
                                   intVector *output ) const
{
    int i, j;
    int a, b, c;
    int node, fr;
    int top[3];
    intVector::iterator it;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sub_size, &subgraph, size() );

    PNL_DEFINE_AUTOBUF( int, mark, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOMULBUF( Arrow, bin, 3, PNL_GRAPH_EXPECTED_MAX_EDGES );

    PNL_INIT_AUTOBUF( mark, graph_size, 0 );
    PNL_DEMAND_AUTOMULBUF( bin, GetNumberOfEdges() );

    for ( i = sub_size, top[0] = 0; i--; )
    {
        mark[subgraph[i]] = -1;
        bin[0][top[0]++].node = bin[0][top[0]].from = subgraph[i];
        output->push_back( subgraph[i] );
    }
    for ( i = sub_size; i--; )
    {
        node = subgraph[i];
        for ( it = m_nbrsList[node]->begin(), top[1] = 0; it != m_nbrsList[node]->end(); ++it )
        {
            if ( mark[*it] == 0 )
            {
                mark[*it] = -1;
                bin[1][top[1]].node = *it;
                bin[1][top[1]++].from = node;
                output->push_back( *it );
            }
        }
    }
    for ( a = 1, b = 2, c = 0; top[a]; a = b, b = c, c = (b + 1) % 3 )
    {
        for ( i = top[a], top[b] = 0; i--; )
        {
            j = bin[a][i].node;
            fr = bin[a][i].from;
            for ( it = m_nbrsList[j]->begin(); it != m_nbrsList[j]->end(); ++it )
            {
                if ( mark[*it] >= 0 )
                {
                    if ( mark[*it] == 0 )
                    {
                        mark[*it] = 1;
                        bin[b][top[b]].node = *it;
                        bin[b][top[b]++].from = j;
                    }
                    if ( !ban[j][PNL_2DIDX( fr, *it, graph_size )] )
                    {
                        mark[*it] = -1;
                        output->push_back( *it );
                    }
                }
            }
        }
    }
    PNL_RELEASE_AUTOMULBUF( bin );
    PNL_RELEASE_AUTOBUF( mark );
}
//////////////////////////////////////////////////////////////////////////

typedef std::pair< int, int > intPair;

void CGraph::GetDReachableSubgraph( intVector const &subgraph,
                                    EGraphNodePattern const pattern[],
                                    intVector *output ) const
{
    int i, j;
    int a;
    int node;
    int prev;
    intVector::iterator it[2];
    neighborTypeVector::iterator nt[2];

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sub_size, &subgraph, size() );

    PNL_DEFINE_AUTOBUF( bool, mark, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOSTACK( intPair, bin, PNL_GRAPH_EXPECTED_MAX_EDGES );

    CGraph *tgr[3];
    
    tgr[0] = CGraph::Create( graph_size, 0 );
    tgr[1] = CGraph::Create( graph_size, 0 );
    tgr[2] = CGraph::Copy( this );

    PNL_INIT_AUTOBUF( mark, graph_size, false );
    PNL_DEMAND_AUTOSTACK( bin, GetNumberOfEdges() );

    for ( i = sub_size; i--; )
    {
        mark[node = subgraph[i]] = true;
        for ( it[0] = tgr[2]->m_nbrsList[node]->begin(), nt[0] = tgr[2]->m_nbrsTypes[node]->begin();
              it[0] != tgr[2]->m_nbrsList[node]->end(); ++it[0], ++nt[0] )
        {
            if ( *nt[0] == ntChild )
            {
                PNL_AUTOSTACK_PUSH( bin, std::make_pair( node, *it[0] ) );
                tgr[0]->AddEdge( node, *it[0], 1 );
            }
            else
            {
                PNL_AUTOSTACK_PUSH( bin, std::make_pair( *it[0], node ) );
                tgr[0]->AddEdge( *it[0], node, 1 );
            }
            mark[*it[0]] = true;
        }
        for ( j = PNL_AUTOSTACK_SIZE( bin ); j--; )
        {
            tgr[2]->RemoveEdge( bin[j].first, bin[j].second );
        }
        PNL_CLEAR_AUTOSTACK( bin );
    }

    for ( a = 0; ; a = 1 - a )
    {
        prev = tgr[2]->GetNumberOfEdges();
        for ( i = graph_size; i--; )
        {
            if ( pattern[i] == PNL_GRAPH_NODE_FOLK )
            {
                if ( tgr[a]->m_nbrsList[i]->size() )
                {
                    for ( it[1] = tgr[2]->m_nbrsList[i]->begin(), nt[1] = tgr[2]->m_nbrsTypes[i]->begin();
                          it[1] != tgr[2]->m_nbrsList[i]->end(); ++it[1], ++nt[1] )
                    {
                        if ( *nt[1] == ntChild )
                        {
                            PNL_AUTOSTACK_PUSH( bin, std::make_pair( i, *it[1] ) );
                            tgr[1-a]->AddEdge( i, *it[1], 1 );
                        }
                        else
                        {
                            PNL_AUTOSTACK_PUSH( bin, std::make_pair( *it[1], i ) );
                            tgr[1-a]->AddEdge( *it[1], i, 1 );
                        }
                        mark[*it[1]] = true;
                    }
                    for ( j = PNL_AUTOSTACK_SIZE( bin ); j--; )
                    {
                        tgr[2]->RemoveEdge( bin[j].first, bin[j].second );
                    }
                    PNL_CLEAR_AUTOSTACK( bin );
                }
            }
            else if ( pattern[i] == PNL_GRAPH_NODE_SEPARATOR )
            {
                if ( tgr[a]->GetNumberOfParents( i ) )
                {
                    for ( it[1] = tgr[2]->m_nbrsList[i]->begin(), nt[1] = tgr[2]->m_nbrsTypes[i]->begin();
                          it[1] != tgr[2]->m_nbrsList[i]->end(); ++it[1], ++nt[1] )
                    {
                        if ( *nt[1] == ntParent )
                        {
                            PNL_AUTOSTACK_PUSH( bin, std::make_pair( *it[1], i ) );
                            tgr[1-a]->AddEdge( *it[1], i, 1 );
                            mark[*it[1]] = true;
                        }
                    }
                    for ( j = PNL_AUTOSTACK_SIZE( bin ); j--; )
                    {
                        tgr[2]->RemoveEdge( bin[j].first, bin[j].second );
                    }
                    PNL_CLEAR_AUTOSTACK( bin );
                }
            }
            else if ( pattern[i] == PNL_GRAPH_NODE_SWAMP )
            {
                if ( tgr[a]->GetNumberOfChildren( i ) )
                {
                    for ( it[1] = tgr[2]->m_nbrsList[i]->begin(), nt[1] = tgr[2]->m_nbrsTypes[i]->begin();
                          it[1] != tgr[2]->m_nbrsList[i]->end(); ++it[1], ++nt[1] )
                    {
                        if ( *nt[1] == ntChild )
                        {
                            PNL_AUTOSTACK_PUSH( bin, std::make_pair( i, *it[1] ) );
                            tgr[1-a]->AddEdge( i, *it[1], 1 );
                        }
                        else
                        {
                            PNL_AUTOSTACK_PUSH( bin, std::make_pair( *it[1], i ) );
                            tgr[1-a]->AddEdge( *it[1], i, 1 );
                        }
                        mark[*it[1]] = true;
                    }
                    for ( j = PNL_AUTOSTACK_SIZE( bin ); j--; )
                    {
                        tgr[2]->RemoveEdge( bin[j].first, bin[j].second );
                    }
                    PNL_CLEAR_AUTOSTACK( bin );
                }
                if ( tgr[a]->GetNumberOfParents( i ) )
                {
                    for ( it[1] = tgr[2]->m_nbrsList[i]->begin(), nt[1] = tgr[2]->m_nbrsTypes[i]->begin();
                          it[1] != tgr[2]->m_nbrsList[i]->end(); ++it[1], ++nt[1] )
                    {
                        if ( *nt[1] == ntChild )
                        {
                            PNL_AUTOSTACK_PUSH( bin, std::make_pair( i, *it[1] ) );
                            tgr[1-a]->AddEdge( i, *it[1], 1 );
                            mark[*it[1]] = true;
                        }
                    }
                    for ( j = PNL_AUTOSTACK_SIZE( bin ); j--; )
                    {
                        tgr[2]->RemoveEdge( bin[j].first, bin[j].second );
                    }
                    PNL_CLEAR_AUTOSTACK( bin );
                }
            }
        }
        delete( tgr[a] );
        tgr[a] = CGraph::Create( graph_size, 0 );
        if ( prev == tgr[2]->GetNumberOfEdges() )
        {
            break;
        }
    }

    for ( i = graph_size; i--; )
    {
        if ( mark[i] )
        {
            output->push_back( i );
        }
    }

    delete( tgr[2] );
    delete( tgr[1] );
    delete( tgr[0] );

    PNL_RELEASE_AUTOBUF( bin );
    PNL_RELEASE_AUTOBUF( mark );
}

//////////////////////////////////////////////////////////////////////////

/* obsolete stuff */
void CGraph::GetReachableSubgraph( int node, bool *ban[],
                                   intVector *output ) const
{
    intVector dummy;
    dummy.push_back( node );
    GetReachableSubgraph( dummy, ban, output );
}
//////////////////////////////////////////////////////////////////////////

void CGraph::GetDReachableSubgraph( int node, EGraphNodePattern const pattern[],
                                    intVector *output ) const
{
    intVector dummy;
    dummy.push_back( node );
    GetDReachableSubgraph( dummy, pattern, output );
}
//////////////////////////////////////////////////////////////////////////

#if 0

#define PNL_INIT_BAN( i, j, k, ban )                \
    for ( i = sep_size; i--; )                      \
    {                                               \
        a = separator[i];                           \
        pars.clear();                               \
        chils.clear();                              \
        GetParents( a, &pars );                     \
        GetChildren( a, &chils );                   \
        for ( k = pars.size(); k--; )               \
        {                                           \
            for ( j = chils.size(); j--; )          \
            {                                       \
                ban[a][PNL_2DIDX( pars[k], chils[j], graph_size )] = true;\
                ban[a][PNL_2DIDX( chils[j], pars[k], graph_size )] = true;\
            }                                       \
        }                                           \
        for ( k = chils.size(); k--; )              \
        {                                           \
            for ( j = chils.size(); j--; )          \
            {                                       \
                ban[a][PNL_2DIDX( chils[k], chils[j], graph_size )] = true;\
            }                                       \
        }                                           \
    }                                               \
    for ( i = swamp.size(); i--; )                  \
    {                                               \
        a = swamp[i];                               \
        pars.clear();                               \
        GetParents( a, &pars );                     \
        for ( k = pars.size(); k--; )               \
        {                                           \
            for ( j = pars.size(); j--; )           \
            {                                       \
                ban[a][PNL_2DIDX( pars[k], pars[j], graph_size )] = true;\
            }                                       \
        }                                           \
    }

void CGraph::GetDConnectionList( int node, intVector const &separator,
                                 intVector *output ) const
{
    int i, j, k, a;
    intVector ac;
    intVector swamp;
    intVector pars, chils;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sep_size, &separator, size() );

    PNL_DEFINE_AUTOBUF( bool, mask, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOBUFBUF( bool, ban, PNL_GRAPH_EXPECTED_MAX_SIZE, 0 );

    PNL_INIT_AUTOBUF( mask, graph_size, false );
    PNL_INIT_AUTOBUFBUF( ban, graph_size, graph_size * graph_size, false );

    GetAncestralClosure( separator, &ac );
    for ( i = ac.size(); i--; )
    {
        mask[ac[i]] = true;
    }
    for ( i = 0; i < graph_size; ++i )
    {
        if ( !mask[i] )
        {
            swamp.push_back( i );
        }
    }
    PNL_INIT_BAN( i, j, k, ban );
    GetReachableSubgraph( node, ban, output );

    PNL_RELEASE_AUTOBUFBUF( ban );
    PNL_RELEASE_AUTOBUF( mask );
}

void CGraph::GetDConnectionTable( intVector const &separator,
                                  intVecVector *output ) const
{
    int i, j, k, a;
    intVector ac;
    intVector folk;
    intVecVector folk_prelim_separation;
    intVector swamp;
    intVector zone;
    intVector pars, chils;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sep_size, &separator, size() );

    PNL_DEFINE_AUTOBUF( int, mark, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOBUFBUF( bool, ban, PNL_GRAPH_EXPECTED_MAX_SIZE, 0 );
    PNL_DEFINE_AUTOBUF( bool, folk_mark, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_INIT_AUTOBUF( mark, graph_size, -2 );
    PNL_INIT_AUTOBUFBUF( ban, graph_size, graph_size * graph_size, false );

    GetAncestry( separator, &folk );
    for ( i = folk.size(); i--; )
    {
        mark[folk[i]] = 0;
    }
    for ( i = sep_size; i--; )
    {
        mark[separator[i]] = -1;
    }
    for ( i = 0; i < graph_size; ++i )
    {
        if ( mark[i] == -2 )
        {
            swamp.push_back( i );
        }
    }
    GetSubgraphConnectivityComponents( folk, &folk_prelim_separation );
    PNL_INIT_AUTOBUF( folk_mark, folk_prelim_separation.size(), false );
    for ( i = folk_prelim_separation.size(); i--; )
    {
        for ( j = folk_prelim_separation[i].size(); j--; )
        {
            mark[folk_prelim_separation[i][j]] = i;
        }
    }
    output->clear();
    output->resize( graph_size );
    PNL_INIT_BAN( i, j, k, ban );
    for ( i = folk_prelim_separation.size(); i--; )
    {
        if ( folk_mark[i] )
        {
            continue;
        }
        zone.clear();
        GetReachableSubgraph( folk_prelim_separation[i], ban, &zone );
        for ( j = zone.size(); j--; )
        {
            if ( mark[zone[j]] >= 0 )
            {
                if ( !(*output)[zone[j]].empty() )
                {
                    break;
                }
                (*output)[zone[j]] = zone;
                folk_mark[mark[zone[j]]] = true;
            }
        }
    }
    for ( i = swamp.size(); i--; )
    {
        GetReachableSubgraph( swamp[i], ban, &(*output)[swamp[i]] );
    }
    for ( i = separator.size(); i--; )
    {
        GetReachableSubgraph( separator[i], ban, &(*output)[separator[i]] );
    }
    PNL_RELEASE_AUTOBUF( folk_mark );
    PNL_RELEASE_AUTOBUFBUF( ban );
    PNL_RELEASE_AUTOBUF( mark );
}

#else

void CGraph::GetDConnectionList( int node, intVector const &separator,
                                 intVector *output ) const
{
    int i;
    intVector folk;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sep_size, &separator, size() );

    PNL_DEFINE_AUTOBUF( EGraphNodePattern, pattern, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_INIT_AUTOBUF( pattern, graph_size, PNL_GRAPH_NODE_SWAMP );

    for ( i = sep_size; i--; )
    {
        pattern[separator[i]] = PNL_GRAPH_NODE_SEPARATOR;
    }
    GetAncestry( separator, &folk );
    for ( i = folk.size(); i--; )
    {
        pattern[folk[i]] = PNL_GRAPH_NODE_FOLK;
    }

    GetDReachableSubgraph( node, pattern, output );

    PNL_RELEASE_AUTOBUF( pattern );
}
//////////////////////////////////////////////////////////////////////////

#endif

#if 1
void CGraph::GetDConnectionTable( intVector const &separator,
                                  intVecVector *output ) const
{
    int i, j;
    intVector ac;
    intVector folk;
    intVecVector folk_prelim_separation;
    intVector swamp;
    intVector zone;
    intVector pars, chils;

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sep_size, &separator, size() );

    PNL_DEFINE_AUTOBUF( EGraphNodePattern, pattern, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOBUF( bool, folk_mark, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_INIT_AUTOBUF( pattern, graph_size, PNL_GRAPH_NODE_SWAMP );

    for ( i = sep_size; i--; )
    {
        pattern[separator[i]] = PNL_GRAPH_NODE_SEPARATOR;
    }
    GetAncestry( separator, &folk );
    for ( i = folk.size(); i--; )
    {
        pattern[folk[i]] = PNL_GRAPH_NODE_FOLK;
    }
    for ( i = 0; i < graph_size; ++i )
    {
        if ( pattern[i] == PNL_GRAPH_NODE_SWAMP )
        {
            swamp.push_back( i );
        }
    }

    GetSubgraphConnectivityComponents( folk, &folk_prelim_separation );
    PNL_INIT_AUTOBUF( folk_mark, folk_prelim_separation.size(), false );
    for ( i = folk_prelim_separation.size(); i--; )
    {
        for ( j = folk_prelim_separation[i].size(); j--; )
        {
            pattern[folk_prelim_separation[i][j]] = (EGraphNodePattern)i;
        }
    }
    output->clear();
    output->resize( graph_size );
    for ( i = folk_prelim_separation.size(); i--; )
    {
        if ( folk_mark[i] )
        {
            continue;
        }
        zone.clear();
        GetDReachableSubgraph( folk_prelim_separation[i], pattern, &zone );
        for ( j = zone.size(); j--; )
        {
            if ( pattern[zone[j]] >= 0 )
            {
                if ( !(*output)[zone[j]].empty() )
                {
                    break;
                }
                (*output)[zone[j]] = zone;
                folk_mark[pattern[zone[j]]] = true;
            }
        }
    }
    for ( i = swamp.size(); i--; )
    {
        GetDReachableSubgraph( swamp[i], pattern, &(*output)[swamp[i]] );
    }
    for ( i = separator.size(); i--; )
    {
        GetDReachableSubgraph( separator[i], pattern, &(*output)[separator[i]] );
    }

    PNL_RELEASE_AUTOBUF( folk_mark );
    PNL_RELEASE_AUTOBUF( pattern );
}
//////////////////////////////////////////////////////////////////////////

#else

void CGraph::GetDConnectionTable( intVector const &separator,
                                  intVecVector *output ) const
{
    int i, j;
    intVector ac;
    intVector folk;
    intVecVector folk_prelim_separation;
    intVector swamp;
    intVector zone;
    intVector pars, chils;
    PNL_AUTOBUFLIST_ELTYPE *node;

    struct C

    PNL_MAKE_LOCAL( int, graph_size, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sep_size, &separator, size() );

    PNL_DEFINE_AUTOBUF( EGraphNodePattern, pattern, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOBUF( int, mark, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOBUF( bool, folk_mark, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_DEFINE_AUTOBUFLIST( int, lst, PNL_GRAPH_EXPECTED_MAX_SIZE, PNL_GRAPH_EXPECTED_MAX_SIZE );

    PNL_INIT_AUTOBUF( pattern, graph_size, PNL_GRAPH_NODE_SWAMP );
    PNL_INIT_AUTOBUF( mark, graph_size, -1 );

    PNL_PASS_ARR( pattern, sep_size, separator[i], /**/, /**/, = PNL_GRAPH_NODE_SEPARATOR );

    GetAncestry( separator, &folk );

    PNL_PASS_ARR( pattern, folk.size(), folk[i], /**/, /**/, = PNL_GRAPH_NODE_FOLK );

    for ( i = 0; i < graph_size; ++i )
    {
        if ( pattern[i] == PNL_GRAPH_NODE_SWAMP )
        {
            swamp.push_back( i );
        }
    }

    GetSubgraphConnectivityComponents( folk, &folk_prelim_separation );

    PNL_INIT_AUTOBUF( folk_mark, folk_prelim_separation.size(), false );

    PNL_DEMAND_AUTOBUFLIST( lst, folk_prelim_separation.size(), graph_size );

    for ( j = folk_prelim_separation.size(); j--; )
    {
        PNL_PASS_ARR( pattern, folk_prelim_separation[j].size(), folk_prelim_separation[j][i],
                      /**/, /**/, = (EGraphNodePattern)PNL_ID );
    }

    output->clear();
    output->resize( graph_size );

    for ( i = sep_size; i--; )
    {
        for ( it = m_nbrsList[separator[i]]->begin(), nt = m_nbrsTypes[separator[i]]->begin();
              it != m_nbrsList[separator[i]]->end(); ++it, ++nt )
        {
            if ( *nt != ntParent || pattern[*it] < 0 )
            {
                continue;
            }
            major = pattern[*it];
            for ( ; it != m_nbrsList[separator[i]]->end(); ++it, ++nt )
            {
                if ( *nt != ntParent || pattern[*it] != major )
                {
                    continue;
                }

                node = PNL_AUTOBUFLIST_HEAD( lst, pattern[*it] );
                while ( node )
                {
                    pattern[PNL_AUTOBUFLIST_VAL( node )] = major;
                    node = PNL_AUTOBUFLIST_NEXT( node );
                }
                PNL_AUTOBUFLIST_MERGE( lst, major, pattern[*it] );
            }

    for ( i = folk_prelim_separation.size(); i--; )
    {
        if ( folk_mark[i] )
        {
            continue;
        }
        zone.clear();
        GetDReachableSubgraph( folk_prelim_separation[i], pattern, &zone );
        for ( j = zone.size(); j--; )
        {
            if ( pattern[zone[j]] >= 0 )
            {
                if ( !(*output)[zone[j]].empty() )
                {
                    break;
                }
                (*output)[zone[j]] = zone;
                folk_mark[pattern[zone[j]]] = true;
            }
        }
    }
    for ( i = swamp.size(); i--; )
    {
        GetDReachableSubgraph( swamp[i], pattern, &(*output)[swamp[i]] );
    }
    for ( i = separator.size(); i--; )

    {
        GetDReachableSubgraph( separator[i], pattern, &(*output)[separator[i]] );
    }

    PNL_RELEASE_AUTOBUF( folk_mark );
    PNL_RELEASE_AUTOBUF( pattern );
}

#endif

CGraph *CGraph::ExtractSubgraph( intVector const &subGraph ) const
{
    int i, j;
    intVecVector nbrs;
    neighborTypeVecVector nbrs_types;

    PNL_MAKE_LOCAL( int, bsz, this, GetNumberOfNodes() );
    PNL_MAKE_LOCAL( int, sz, &subGraph, size() );

    PNL_DEFINE_AUTOBUF( int, corr, PNL_GRAPH_EXPECTED_MAX_SIZE );
    PNL_INIT_AUTOBUF( corr, bsz, -1 );

    for ( i = sz; i--; )
    {
        corr[subGraph[i]] = i;
    }

    nbrs.resize( sz );
    nbrs_types.resize( sz );
    
    for ( i = 0; i < sz; ++i )
    {
        for ( j = 0; j < m_nbrsList[subGraph[i]]->size(); ++j )
        {
            if ( corr[(*m_nbrsList[subGraph[i]])[j]] >= 0 )
            {
                nbrs[i].push_back( corr[(*m_nbrsList[subGraph[i]])[j]] );
                nbrs_types[i].push_back( (*m_nbrsTypes[subGraph[i]])[j] );
            }
        }
    }

    PNL_RELEASE_AUTOBUF( corr );

    return CGraph::Create( nbrs, nbrs_types );
}


#ifdef PNL_RTTI
const CPNLType CGraph::m_TypeInfo = CPNLType("CGraph", &(CPNLBase::m_TypeInfo));

#endif
