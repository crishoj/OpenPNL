/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlGraph.hpp                                                //
//                                                                         //
//  Purpose:   CGraph class definition                                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLGRAPH_HPP__
#define __PNLGRAPH_HPP__

#include <limits.h>
#include "pnlTypeDefs.hpp"
#include "pnlException.hpp"
#include "pnlMatrix.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

// to represent the relationships between neighboring nodes in the graph */

// there are three types of the neighbors for each node, a neighbors
// can either be: a parent, a child or just a neighbor, which means
// that the edge connecting those two nodes is undirected */
typedef PNL_API enum
{
    ntParent,
    ntChild,
    ntNeighbor
} ENeighborType;

typedef PNL_API enum
{
    PNL_GRAPH_NODE_FOLK = -1,
    PNL_GRAPH_NODE_SEPARATOR = -2,
    PNL_GRAPH_NODE_SWAMP = -3,
    PNL_GRAPH_NODE_FAKE = INT_MAX
} EGraphNodePattern;

typedef pnlVector<ENeighborType>        neighborTypeVector;
typedef pnlVector< neighborTypeVector > neighborTypeVecVector;
typedef pnlVector<neighborTypeVector *> neighborTypeVecPVector;

#ifdef SWIG
%rename(CreateFromAdjMat) CGraph::Create(const CMatrix<int>* pAdjMat);
%rename(GetAncestralClosureMask) CGraph::GetAncestralClosure(intVector const &, boolVector *) const;
%rename(GetDReachableSubgraphByNode) CGraph::GetDReachableSubgraph(int, EGraphNodePattern const[], intVector *) const;
%rename(GetReachableSubgraphByNode) CGraph::GetReachableSubgraph(int, bool *[], intVector *) const;
%rename(GetSubgrConnectComponents) CGraph::GetSubgraphConnectivityComponents( intVector const &subGraph, intVecVector *decompositionOut ) const;
#endif

#ifndef SWIG
template <typename T>
class CMatrix;
#endif

/* represents the graph structure and implements basic graph algorithms */
class PNL_API CGraph : public CPNLBase
{
public:

#ifdef PNL_OBSOLETE
    // creates a graph from the list of neighbors
    static CGraph* Create( int numOfNds, const int *numOfNbrsIn,
                           const int *const *nbrsListIn,
                           const ENeighborType *const *nbrsTypesIn );
#endif

    static CGraph* Create( const intVecVector& nbrsListIn,
                           const neighborTypeVecVector& nbrsTypesListIn );

#ifdef PNL_OBSOLETE
    // creates a graph using adjacency matrix
    static CGraph* Create( int numOfNds, const int *const *adjMatIn );
#endif

    static CGraph* Create(const CMatrix<int>* pAdjMat);

    // creates a copy of the input graph
    static CGraph* Copy(const CGraph *pGraphIn);

    // creates a graph by moralizing the input one
    static CGraph* MoralizeGraph(const CGraph *pGraphIn);

    void GetTopologicalOrder( intVector *OldToNewMapOut ) const;

    // returns 1 if the change of the graph is allowed and 0 otherwise
    inline bool IsChangeAllowed() const;

    // prohibits the change of the graph
    inline void ProhibitChange() const;

    // returns number of nodes in the graph
    inline int GetNumberOfNodes() const;

    int GetNumberOfEdges() const;

    /* sets number of nodes to be equal to the input newNumberOfNodes */
    void AddNodes(int newNumOfNds);

#ifdef PNL_OBSOLETE
    /* connects all the nodes from the subgraph between each other */
    void FormCliqueFromSubgraph( int numOfNds, const int *subGraphIn );
#endif

    void FormCliqueFromSubgraph(const intVector& subGraphIn);

    /* returns the number of neighbors */
    inline int GetNumberOfNeighbors(int nodeNum) const;

    /* returns the number of parents */
    inline int GetNumberOfParents(int nodeNum) const;

    /* returns the number of children */
    inline int GetNumberOfChildren(int nodeNum) const;

#ifdef PNL_OBSOLETE
    /* returns 1 if the input subset is complete and 0 otherwise */
    inline int IsCompleteSubgraph( int numOfNdsInSubgraph,
                                   const int *subgraphIn ) const;
#endif
    
    inline int IsCompleteSubgraph( const intVector& subGraphIn ) const;

    /* returns 1 if the graph is directed and 0 otherwise */
    inline int IsDirected() const;
    
    /* returns 1 if the graph is undirected and 0 otherwise */
    inline int IsUndirected() const;

    /* returns 1 if the edge startNode->endNode exists and 0 otherwise */
    inline int IsExistingEdge( int startNode, int endNode ) const;
    
#ifndef SWIG
    /* returns a pointer to the adjacency matrix for the graph */
    inline void GetAdjacencyMatrix(CMatrix<int>** pAdjMatOut) const;
#endif

    inline CMatrix<int>* CreateAdjacencyMatrix(bool bDense = true) const;

    
    /* cleares the graph by deleting the lists of neighbors and nbrs types */
    inline void ClearGraph();

    /* checks if the graph structures perfectly match */
    inline bool operator==(const CGraph &graphIn) const;

    /* checks if the graph structures dont match */
    inline bool operator!=(const CGraph &graphIn) const;

    /* returns a vector of node numbers of parents of the node "nodeNum" */
    inline void GetParents(int nodeNum, intVector *parentsOut) const;
    
    /* returns a vector of node numbers of children of the node "nodeNum" */
    inline void GetChildren( int nodeNum, intVector *childrenOut ) const;

#ifdef PNL_OBSOLETE
    /* returns neighbors of the node, num of which is equal to nodeNumber */
    void GetNeighbors( int nodeNum, int *numOfNbrsOut, const int **nbrsOut,
                       const ENeighborType **nbrsTypesOut ) const;
#endif

    void GetNeighbors( int nodeNum, intVector* nbrsOut,
                       neighborTypeVector *nbrsTypesOut ) const;

#ifdef PNL_OBSOLETE
    /* sets new neighbors to the node, num of which is equal to nodeNumber */
    void SetNeighbors( int nodeNum, int numOfNbrs, const int *nbrsIn,
                       const ENeighborType *nbrsTypesIn );
#endif

    void SetNeighbors( int nodeNum, const intVector& nbrsIn,
        const neighborTypeVector& nbrsTypesIn );
    
    /* adds edge to the graph */
    void AddEdge( int startNode, int endNode, int directed );
    
    /* removes edge from the graph */
    void RemoveEdge( int startNode, int endNode );
    
    /* changes direction of the edge startNode->endNode */
    void ChangeEdgeDirection( int startNode, int endNode );
    
    /* returns 1 if graph is a directed acyclic graph (DAG) and 0 otherwise */
    int IsDAG() const;
    
    /* returns 1 if the graph is topologically sorted */
    int IsTopologicallySorted() const;

	/* returns 1 if graph is a binary tree and 0 otherwise */
    int IsBinaryTree() const;
    
    /* returns the number of connectivity components of the graph */
    int NumberOfConnectivityComponents() const;
    
    /* fills in closureOut with ancestry of input subgraph */
    void GetAncestry( intVector const &subGraph,
        intVector *closureOut ) const;
    
    /* fills in closureOut with ancestral closure of input subgraph */
    void GetAncestralClosure( intVector const &subGraph,
        intVector *closureOut ) const;
    
        /* fills in closureMaskOut with boolean mask
    of the ancestral closure of input subgraph */
    void GetAncestralClosure( intVector const &subGraph,
        boolVector *closureMaskOut ) const;
    
    /* fills in decompositionOut with connectivity components of the induced subgraph */
    void GetSubgraphConnectivityComponents( intVector const &subGraph,
        intVecVector *decompositionOut ) const;
    
    /* fills in decompositionOut with connectivity components */
    void GetConnectivityComponents( intVecVector *decompositionOut ) const;

    /* returns new graph structure corresponding to subgrph */
    CGraph *ExtractSubgraph( intVector const &subGraph ) const;

    /* fills in output vector with DConnection list of given node */
    void GetDConnectionList( int node, intVector const &separator,
        intVector *dseparationListOut ) const;
    
    /* fills in output vectors with DConnection table */
    void GetDConnectionTable( intVector const &separator,
        intVecVector *dseparationTableOut ) const;
    
        /* fills in output vector with nodes reachable from subgraph subject to ban;
    for every node I, ban[I] is a 2D boolean array of bans */
    void GetReachableSubgraph( intVector const &subgraph, bool *ban[],
        intVector *closureOut ) const;
    
    /* fills in output vector with nodes reachable from node subject to ban */
    void GetReachableSubgraph( int node, bool *ban[],
        intVector *closureOut ) const;
    
    /* fills in output vector with nodes d-reachable from subgraph */
#ifdef PNL_OBSOLETE
    void GetDReachableSubgraph( intVector const &subgraph,
        EGraphNodePattern const pattern[],
        intVector *output ) const;
#endif
    /* fills in output vector with nodes d-reachable from given node */
    
#ifdef PNL_OBSOLETE
    void GetDReachableSubgraph( int node,
        EGraphNodePattern const pattern[],
        intVector *output ) const;
#endif
    /* initializing assignment */
    CGraph& operator=(const CGraph& graphIn);

    /* dumps neighbors and node relations to the standard output */
    void Dump() const;

    virtual ~CGraph();

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CGraph::m_TypeInfo;
    }
#endif
protected:

    CGraph( int numOfNds, const int* numOfNbrs, const int *const *nbrsList,
            const ENeighborType *const *nbrsTypes );

    CGraph( int numOfNds, const int *const *adjMat );

    CGraph(const CMatrix<int>* pAdjMat);

    CGraph(const CGraph& rGraph);

    void SortNodeNeighbors(int nodeNum);
    
    inline void CreateEmptyNbrsListForAllNodes();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:

    mutable bool m_bChangeAllowed;

    intVecPVector m_nbrsList;

    neighborTypeVecPVector m_nbrsTypes;

    inline void AddToNodeNeighbors( int nodeNum, int nbrToAdd,
                                    ENeighborType nbrToAddType );

    inline void RemoveFromNodeNeighbors( int nodeNum, int nbrToRemove );

};
//////////////////////////////////////////////////////////////////////////

#ifndef SWIG

inline bool CGraph::IsChangeAllowed() const
{
    return m_bChangeAllowed;
}
//////////////////////////////////////////////////////////////////////////

inline void CGraph::ProhibitChange() const
{
    m_bChangeAllowed = false;
}
//////////////////////////////////////////////////////////////////////////

inline int CGraph::GetNumberOfNodes() const
{
    return m_nbrsList.size();
}
//////////////////////////////////////////////////////////////////////////

inline int CGraph::GetNumberOfNeighbors(int nodeNum) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( nodeNum, 0, m_nbrsList.size() - 1 );
    /* bad-args check end */

    return (*(m_nbrsList.begin() + nodeNum))->size();
}
//////////////////////////////////////////////////////////////////////////

inline int CGraph::GetNumberOfParents(int nodeNum) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( nodeNum, 0, m_nbrsList.size() - 1 );
    /* bad-args check end */

    return std::count( m_nbrsTypes[nodeNum]->begin(),
        m_nbrsTypes[nodeNum]->end(), ntParent );
}
//////////////////////////////////////////////////////////////////////////

inline int CGraph::GetNumberOfChildren(int nodeNum) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( nodeNum, 0, m_nbrsList.size() - 1 );
    /* bad-args check end */

    return std::count( m_nbrsTypes[nodeNum]->begin(),
        m_nbrsTypes[nodeNum]->end(), ntChild );
}
//////////////////////////////////////////////////////////////////////////

inline int CGraph::IsCompleteSubgraph( int numberOfNodesInSubgraph,
                                          const int *subgraph ) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( numberOfNodesInSubgraph, 0, m_nbrsList.size() );
    PNL_CHECK_IS_NULL_POINTER(subgraph);
    /* bad-args check end */

    const int *sgIter1 = subgraph, *sgIter2;
    const int *subgraph_end = subgraph + numberOfNodesInSubgraph;

    for( ; subgraph_end - sgIter1; sgIter1++ )
    {
        for( sgIter2 = sgIter1 + 1; subgraph_end - sgIter2; sgIter2++ )
        {
            if( std::find( m_nbrsList[*sgIter1]->begin(),
                m_nbrsList[*sgIter1]->end(), *sgIter2 )
                == m_nbrsList[*sgIter1]->end() )
            {
                return 0;
            }
        }
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////

inline int CGraph::IsCompleteSubgraph( const intVector& subGraph ) const
{
    return IsCompleteSubgraph( subGraph.size(), &subGraph.front() );
}
//////////////////////////////////////////////////////////////////////////

inline int CGraph::IsDirected() const
{
    neighborTypeVecPVector::const_iterator ntIter = m_nbrsTypes.begin();

    for( ; m_nbrsTypes.end() - ntIter; ntIter++ )
    {
        if( std::find( (*ntIter)->begin(), (*ntIter)->end(), ntNeighbor )
            != (*ntIter)->end() )
        {
            return 0;
        }
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////

inline int CGraph::IsUndirected() const
{
    neighborTypeVecPVector::const_iterator ntIter = m_nbrsTypes.begin();

    for( ; m_nbrsTypes.end() - ntIter; ntIter++ )
    {
        if( std::find( (*ntIter)->begin(), (*ntIter)->end(), ntParent )
            != (*ntIter)->end() )
        {
            return 0;
        }

        if( std::find( (*ntIter)->begin(), (*ntIter)->end(), ntChild )
            != (*ntIter)->end() )
        {
            return 0;
        }
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////

inline int CGraph::IsExistingEdge( int startNode, int endNode ) const
{
    int numberOfNodes = m_nbrsList.size();

    /* bad-args check */
    PNL_CHECK_RANGES( startNode, 0, numberOfNodes - 1 );
    PNL_CHECK_RANGES( endNode,   0, numberOfNodes - 1 );
    /* bad-args check end */

    if( std::find( m_nbrsList[startNode]->begin(),
        m_nbrsList[startNode]->end(), endNode )
        == m_nbrsList[startNode]->end() )
    {
        return 0;
    }

    return 1;
}
//////////////////////////////////////////////////////////////////////////

inline void CGraph::GetAdjacencyMatrix(CMatrix<int>** pAdjMat) const
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pAdjMat);
    // bad-args check end

    int       numOfDims;
    const int *ranges;

    (*pAdjMat)->GetRanges( &numOfDims, &ranges );

    const int numOfNds = GetNumberOfNodes();
    
    if( ( numOfDims != 2 ) || ( ranges [0] != ranges[1] )
        || ( ranges[0] != numOfNds ) )
    {
        PNL_THROW( CInvalidOperation,
            " adj. mat. should be a 2D matrix numOfNds x numOfNds " );
    }

    (*pAdjMat)->ClearData();

    intVector indices(2);
   
    int nodeNum = 0;
    
    for( ; nodeNum < numOfNds; ++nodeNum )
    {
        intVector::const_iterator neighIt = m_nbrsList[nodeNum]->begin(),
                                  neigh_end = m_nbrsList[nodeNum]->end();
 
        neighborTypeVector::const_iterator neighTypeIt
            = m_nbrsTypes[nodeNum]->begin();

        for( ; neighIt != neigh_end; ++neighIt, ++neighTypeIt )
        {
            if( *neighTypeIt == ntNeighbor )
            {
                if( nodeNum < *neighIt )
                {
                    indices[0] = nodeNum;
                    indices[1] = *neighIt;

                    (*pAdjMat)->SetElementByIndexes( 1, &indices.front() );
                    
                    indices[0] = *neighIt;
                    indices[1] = nodeNum;

                    (*pAdjMat)->SetElementByIndexes( 1, &indices.front() );
                }
            }
            else if( *neighTypeIt == ntChild )
            {
                indices[0] = nodeNum;
                indices[1] = *neighIt;
                
                (*pAdjMat)->SetElementByIndexes( 1, &indices.front() );
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////

inline void CGraph::AddToNodeNeighbors( int nodeNum, int nbrToAdd,
                                        ENeighborType nbrToAddType )
{
    // operation validity check
    if( m_nbrsList.empty() )
    {
        PNL_THROW( CInconsistentState, " the graph is empty " );
    }

    if( !IsChangeAllowed() )
    {
        PNL_THROW( CInvalidOperation,
            " trying to work with unexisting edge " );
    }
    // operation validity check end
    
    int numOfNds = m_nbrsList.size();
    
    // bad-args check
    PNL_CHECK_RANGES( nodeNum,  0, numOfNds - 1 );
    PNL_CHECK_RANGES( nbrToAdd, 0, numOfNds - 1 );
    // bad-args check end

    intVector          *nbrs      = *( m_nbrsList.begin()  + nodeNum );
    neighborTypeVector *nbrsTypes = *( m_nbrsTypes.begin() + nodeNum );

    if( std::find( nbrs->begin(),nbrs->end(), nbrToAdd ) == nbrs->end() )
    {
        intVector::iterator location = std::lower_bound( nbrs->begin(),
            nbrs->end(), nbrToAdd );
        
        if( location == nbrs->end() )
        {
            nbrs->push_back(nbrToAdd);
            nbrsTypes->push_back(nbrToAddType);
        }
        else
        {
            int offset = location - nbrs->begin();

            nbrs->insert( location, nbrToAdd );
            nbrsTypes->insert( nbrsTypes->begin() + offset, nbrToAddType );
        }
    }   
}
//////////////////////////////////////////////////////////////////////////

inline void CGraph::RemoveFromNodeNeighbors( int nodeNum, int nbrToRemove )
{
    // operation validity check
    if( m_nbrsList.empty() )
    {
        PNL_THROW( CInconsistentState, " the graph is empty " );
    }
    
    if( !IsChangeAllowed() )
    {
        PNL_THROW( CInvalidOperation,
            " trying to work with unexisting edge " );
    }
    // operation validity check end
    
    const int numOfNds = m_nbrsList.size();

    // bad-args check
    PNL_CHECK_RANGES( nodeNum    , 0, numOfNds - 1 );
    PNL_CHECK_RANGES( nbrToRemove, 0, numOfNds - 1 );
    // bad-args check end
    
    
    intVector          *nbrs      = *( m_nbrsList.begin()  + nodeNum );
    neighborTypeVector *nbrsTypes = *( m_nbrsTypes.begin() + nodeNum );

    intVector::iterator location = std::find( nbrs->begin(), nbrs->end(),
        nbrToRemove );

    if( location != nbrs->end() )
    {
        int offset = location - nbrs->begin();
        
        nbrs->erase(location);
        nbrsTypes->erase( nbrsTypes->begin() + offset );
    }
}
//////////////////////////////////////////////////////////////////////////

inline void CGraph::ClearGraph()
{
    // operation validity check
    if( !IsChangeAllowed() )
    {
        PNL_THROW( CInvalidOperation,
            " Can't clear graph - change's prohibited " );
    }
    // operation validity check end

    intVecPVector::const_iterator          nlIt = m_nbrsList.begin(),
                                         nl_end = m_nbrsList.end();
    neighborTypeVecPVector::const_iterator ntIt = m_nbrsTypes.begin();

    for( ; nlIt != nl_end; ++nlIt, ++ntIt )
    {
        delete *nlIt;

        delete *ntIt;
    }

    m_nbrsList.resize(0);

    m_nbrsTypes.resize(0);
}
//////////////////////////////////////////////////////////////////////////

inline bool CGraph::operator==(const CGraph &graph) const
{
    if( m_nbrsList.size() == graph.m_nbrsList.size() )
    {
        intVecPVector::const_iterator nbrsIt   = m_nbrsList.begin(),
                                      nbrs_end = m_nbrsList.end(),
                                      grNbrsIt = graph.m_nbrsList.begin();
        
        neighborTypeVecPVector::const_iterator 
                                      nbrsTypeIt   = m_nbrsTypes.begin(),
                                      grNbrsTypeIt = graph.m_nbrsTypes.begin();

        for( ; nbrsIt != nbrs_end; ++nbrsIt, ++grNbrsIt, ++nbrsTypeIt,
            ++grNbrsTypeIt )
        {
            if( (*nbrsIt)->size() == (*grNbrsIt)->size() )
            {
                if( ! ( std::equal( (*nbrsIt)->begin(), (*nbrsIt)->end(),
                    (*grNbrsIt)->begin() ) && std::equal(
                    (*nbrsTypeIt)->begin(), (*nbrsTypeIt)->end(),
                    (*grNbrsTypeIt)->begin() ) ) )
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////

inline bool CGraph::operator!=(const CGraph &graph) const
{
    return !(operator==(graph));
}
//////////////////////////////////////////////////////////////////////////

inline void CGraph::GetParents(int nodeNum, intVector *parents) const
{
    // bad-args check
    PNL_CHECK_RANGES( nodeNum, 0, m_nbrsList.size() - 1 );
    PNL_CHECK_IS_NULL_POINTER(parents);
    // bad-args check end

    parents->clear();
    
    intVector::const_iterator 
        neighIt   = (*( m_nbrsList.begin() + nodeNum ))->begin(),
        neigh_end = (*( m_nbrsList.begin() + nodeNum ))->end();

    neighborTypeVector::const_iterator
        neighTypeIt = (*( m_nbrsTypes.begin() + nodeNum ))->begin();

    for( ; neighIt != neigh_end; ++neighIt, ++neighTypeIt )
    {
        if( *neighTypeIt == ntParent )
        {
            parents->push_back(*neighIt);
        }
    }
}
//////////////////////////////////////////////////////////////////////////

inline void CGraph::GetChildren( int nodeNum, intVector *children ) const
{
    // bad-args check
    PNL_CHECK_RANGES( nodeNum, 0, m_nbrsList.size() - 1 );
    PNL_CHECK_IS_NULL_POINTER(children);
    // bad-args check end

    children->clear();

    intVector::const_iterator 
        neighIt   = (*( m_nbrsList.begin() + nodeNum ))->begin(),
        neigh_end = (*( m_nbrsList.begin() + nodeNum ))->end();

    neighborTypeVector::const_iterator
        neighTypeIt = (*( m_nbrsTypes.begin() + nodeNum ))->begin();

    for( ; neighIt != neigh_end; ++neighIt, ++neighTypeIt )
    {
        if( *neighTypeIt == ntChild )
        {
            children->push_back(*neighIt);
        }
    }
}
//////////////////////////////////////////////////////////////////////////

inline void CGraph::CreateEmptyNbrsListForAllNodes()
{
    intVecPVector::iterator listIt   = m_nbrsList.begin(),
                            list_end = m_nbrsList.end();

    neighborTypeVecPVector::iterator typesIt = m_nbrsTypes.begin();

    for(; listIt != list_end; ++listIt, ++typesIt )
    {
        if( !*listIt )
        {
            *listIt  = new intVector;
            *typesIt = new neighborTypeVector;
            
            PNL_CHECK_IF_MEMORY_ALLOCATED(*listIt);
            PNL_CHECK_IF_MEMORY_ALLOCATED(*typesIt);
        }
    }
}
//////////////////////////////////////////////////////////////////////////

inline CMatrix<int>* CGraph::CreateAdjacencyMatrix(bool bDense ) const
{
    CMatrix<int> *pMat;
    int nnodes = GetNumberOfNodes();
    int rng[] = {nnodes, nnodes};
    intVector tmp( nnodes*nnodes, 0);
    if( bDense )
    {
	pMat = CDenseMatrix<int>::Create( 2, rng, &tmp.front() );
    }
    else
    {
	pMat = CSparseMatrix<int>::Create( 2, rng, 0 );
    }
    GetAdjacencyMatrix( &pMat);
    return pMat;
}

#endif

PNL_END

#endif //__PNLGRAPH_HPP__
