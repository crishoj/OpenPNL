/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlJunctionTree.hpp                                         //
//                                                                         //
//  Purpose:   CJunctionTree class definition                              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLJUNCTIONTREE_HPP__
#define __PNLJUNCTIONTREE_HPP__

#include "pnlParConfig.hpp"
#include "pnlMNet.hpp"
#include "pnlPotential.hpp"
#include "pnlLog.hpp"
#include "pnlLogDriver.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN


class PNL_API CJunctionTree : public CStaticGraphicalModel
{
public:

#ifdef PNL_OBSOLETE    
    /* function of jtree model creation from either BNet, MNet or MRF2 */
    static CJunctionTree* Create( const CStaticGraphicalModel *pGrModel,
                                  int numOfSubGrToConnect = 0,
                                  const int *subGrToConnectSizes = NULL,
                                  const int **subGrToConnect = NULL );
#endif

    static CJunctionTree* Create( const CStaticGraphicalModel* pGrModel,
                                  const intVecVector& subGrToConnect
                                  = intVecVector() );

    /* creates a copy of the input JTree */
    static CJunctionTree* Copy(const CJunctionTree* pJTree);
    
    /* destroys the junction tree */
    virtual ~CJunctionTree();

    
    //static void Release(CJunctionTree** pJTree);
    
    /* checks if the input model is valid for the JTree construction */
    
    /* returns each node contents, which is a clq of the moralized graph */
#ifndef SWIG
    inline void GetNodeContent( int nodeNumber, int *nodeContentSz,
	const int **content ) const;
    
	/* returns a set of nodes, which were connected by the user 
    while constructing the junction tree */
    inline void GetNodesConnectedByUser( int nodeSetNum, int *numOfNds,
	const int **nds ) const;
    
    /* returns the original factors association to tree pots */
    inline void GetFactorAssignmentToClique( int *numberOfFactors, 
	const int **factorAssign ) const;
    
	/* returns a domain of a separator between two cliques,
    which numbers are equal to firstClqNum and secondClqNum */
    inline void GetSeparatorDomain( int firstClqNum, int secondClqNum,
	int *domSize, const int **domain ) const;
#endif
	/* returns a pointer to a potential attached to a node, number
    of which is equal to nodeNumber */
    inline CPotential* GetNodePotential(int nodeNum);
    
    /* returns a pointer to a potential attached to a separator between
    two cliques, which numbers are equal to firstClqNum and secondClqNum */
    inline CPotential* GetSeparatorPotential( int firstClqNum,
        int secondClqNum );
    
    /* returns clique numbers array, each containing the input node set */
#ifndef SWIG
    inline void GetClqNumsContainingSubset( int numOfNdsInSubset,
	const int *subset, int *numOfClqs,
	const int **clqsContSubset )
	const;
#endif
    
	/* sets charge for the jtree model (charge is a set of 
    potentials for nodes and separators of junction tree) */
    void InitCharge( const CStaticGraphicalModel *pGrModel,
                     const CEvidence *pEvidence, int sumOnMixtureNode = 1 );
#ifdef PAR_OMP
    void InitChargeOMP( const CStaticGraphicalModel *pGrModel,
                        const CEvidence *pEvidence, int sumOnMixtureNode = 1 );
#endif // PAR_OMP

    void ClearCharge();
#ifdef PAR_OMP
    void ClearChargeOMP();
#endif // PAR_OMP

    
    /* checks if the structure of the input JTree matches with the structure
    of the JTree for which the member function is called */
    inline int IsMatchingTreeStructure(const CJunctionTree *pJTree) const;
    
    /* copying initialization (works if the structure of the input JTree and 
    of the original one match perfectly). The copying then is just the data 
    replacement for all the potentials of the original JTree */
    CJunctionTree& operator=(const CJunctionTree &JTree);
    
    inline virtual int GetNumberOfNodes() const;
    
    /*/////////////////////////////////////////////////////////////////*/
    /*/////////////////////////////////////////////////////////////////*/
    /* these are pure virtual functions (members of StaticGraphicalModel)
    which have to be defined, so that JTree can be instanciated */
    void AllocFactors(){};
    void AllocFactor( int numberOfNodesInDomain, const int *domain ){}
    void AllocFactor(int number){}
    void AttachFactor(CFactor *param){}
    CFactor* GetFactor(int domainNodes) const {return NULL;}
#ifndef SWIG
    void GetFactors( int numberOfNodes, const int* nodes,
        int *numberOfFactors, CFactor ***params ) const {}
#endif
    int GetFactors( int numberOfNodes, const int* nodes,
                                    pFactorVector *params) const {return 0;}
    bool IsValid(std::string* descriptionOut = NULL) const;
    /*/////////////////////////////////////////////////////////////////*/
    /*/////////////////////////////////////////////////////////////////*/
    
    inline void DumpNodeContents() const;
    
    bool IsAllNodesInClsObs(int NumCls, const CEvidence *pEvidence );

	/* returns node type of the node with a number == nodeNumber */
	inline const CNodeType* GetNodeType(int nodeNumber) const;

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CJunctionTree::m_TypeInfo;
  }
#endif
protected:
    
#ifdef PNL_OBSOLETE
    CJunctionTree( const CStaticGraphicalModel *pGrModel,
                   int numOfSubGrToConnect, const int *subGrToConnectSizes,
                   const int **subGrToConnect );
#endif

    CJunctionTree( const CStaticGraphicalModel* pGrModel,
                   const intVecVector& subGrToConnect );
    
    CJunctionTree( const CJunctionTree& rJTree );

    
    
    // constructs JTree from the input model
#ifdef PNL_OBSOLETE
    void ConstructJTree( const CStaticGraphicalModel *pGrModel,
        int numOfSubGrToConnect,
        const int *subGrToConnectSizes, 
        const int **subGrToConnect );
#endif

    void ConstructJTree( const CStaticGraphicalModel* pGrModel,
                         const intVecVector& subGrToConnect );
    
    // initializes the internal graph of the JTree for which the
    // triangulation is carried out and cliques are extracted from
    void InitInternalGraph(const CStaticGraphicalModel* pGrModel);
    
    // checks if the internal graph is chordal or not:
    // if m_pMoralGraph is chordal then the function returns 1, and 
    // m_perfectNumbering vector shows the perfect numbering for the nodes
    // of m_pMoralGraph so that the cliques can be easily found, otherwise
    // returns zero and clears m_perfectNumbering
    int MaximumCardinalitySearch() const;
    
    // triangulates the input graph
    void RunTriangulation();
    
    // builds a tree structure of JTree out of using cliques
    void BuildTreeStructure();
    
    // determines node types for all nodes of jtree and initializes
    // nodeAssociation vector
    void SetNodeTypes( const CStaticGraphicalModel* pGraphicalModel,
                       const CEvidence* pEvidence );
    
    /* assigns nodes to cliques so that each node is assigned to a clique */
    void AssignFactorsToClq(const CStaticGraphicalModel* pGrahicalModel)
        const;
#ifdef PAR_OMP
    void AssignFactorsToClqOMP(const CStaticGraphicalModel* pGrahicalModel)
        const;
#endif // PAR_OMP

    /* initializes potentials for the nodes of the junction tree */
    /* two functions are necessary cause the initialization is carried out
    differently for the case of BNet and MNet models */
    void InitNodePotsFromMNet( const CMNet *pMNet,
                               const CEvidence *pEvidence );
    
    void InitNodePotsFromBNet( const CBNet *pBNet, const CEvidence *pEvidence,
                               int sumOnMixtureNode = 1);
#ifdef PAR_OMP
    void InitNodePotsFromBNetOMP( const CBNet *pBNet, const CEvidence *pEvidence,
                                  int sumOnMixtureNode = 1);
#endif // PAR_OMP

    inline bool IsChargeInitialized() const;
    
    /* initializes potentials for separators of the junction tree */
    void InitSeparatorsPots( const CStaticGraphicalModel *pGraphicalModel,
                             const CEvidence *pEvidence );

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif    
private:
    
    // the triangulated graph, which is necessary to find all cliques
    CGraph *m_pMoralGraph;
    
    // node perfect numbering received as a result of the  maximum 
    // cardinality search algorithm
    mutable intVector m_perfectNumbering;
    
    // each i-th element is a set of nodes of which each node of the
    // junction tree consists (a clique)
    mutable intVecVector m_nodeContents;
    
    // nodes, which are to be connected before the JTree constructed
    intVecVector m_nodesToConnect;
    
    // each node should be assigned to exactly one clique
    mutable intVector m_factAssignToClq;
    
    // an array of pointers to the factors, which are attached
    //to the nodes of the jtree
    potsPVector m_nodePots;
    
    // a 2D array of pointers to factors on the jtree separators
    potsPVecVector m_separatorPots;
    
    // separator domain (we only need one separator at a time for 
    // a tree protocol), if we decide to go serial, will have to think'a 
    // something better to store the separator domain
    mutable intVector m_sepDomain;
    
    // vector containing clq numbers, which all contain some common     node set.
    //Getting filled with the call of a function GetClqsContainingSubset(...)
    mutable intVector m_clqsContSubset;
    
    bool m_bChargeInitialized;
    
    //TEMPORARILY - will be removed after full inclusion of Model Domain
    intVector m_nodeAssociation;

    nodeTypeVector m_nodeTypes;
    
    // makes sure nodes are connected into cliques by adding extra edges
#ifdef PNL_OBSOLETE
    inline void AssureSubGraphsConnectedInCliques( int numOfSubGrToConnect,
                                                   const int *subGrToConnectSizes, 
                                                   const int **subGrToConnect );
#endif    

    inline void AssureSubGraphsConnectedInCliques(const intVecVector&
                                                  subGrToConnect);
    
    // checks if clique with a number equal to clqNum contains
    // all nodes from domain
    inline int IsASubClq( int domSize, const int *domain, int clqNum ) const;
    
    inline EDistributionType DetermineDistributionType(int nodeNumber);
};
//////////////////////////////////////////////////////////////////////////

inline void 
CJunctionTree::GetNodeContent( int nodeNumber, int *nodeContentSz,
                              const int **content ) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( nodeNumber, 0, m_nodeContents.size() - 1 );
    PNL_CHECK_IS_NULL_POINTER(nodeContentSz);
    PNL_CHECK_IS_NULL_POINTER(content);
    /* bad-args check end */
    
    *nodeContentSz = m_nodeContents[nodeNumber].size();
    *content = &m_nodeContents[nodeNumber].front();
}
//////////////////////////////////////////////////////////////////////////

inline void 
CJunctionTree::GetNodesConnectedByUser( int nodeSetNum, int *numOfNds,
                                        const int **nds ) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( nodeSetNum, 0, m_nodesToConnect.size() - 1 );
    PNL_CHECK_IS_NULL_POINTER(numOfNds);
    PNL_CHECK_IS_NULL_POINTER(nds);
    /* bad-args check end */
    
    *numOfNds = ( m_nodesToConnect.begin() + nodeSetNum )->size();
    *nds      = &( m_nodesToConnect.begin() + nodeSetNum)->front();
}
//////////////////////////////////////////////////////////////////////////

inline void 
CJunctionTree::GetFactorAssignmentToClique( int *numberOfFactors, 
                                            const int **factorAssign ) const
{
    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(numberOfFactors);
    PNL_CHECK_IS_NULL_POINTER(factorAssign);
    /* bad-args check end */
    
    *numberOfFactors = m_factAssignToClq.size();
    *factorAssign = &m_factAssignToClq.front();
}
//////////////////////////////////////////////////////////////////////////

inline void 
CJunctionTree::GetSeparatorDomain( int firstClqNum, int secondClqNum, 
                                   int *domSize, const int **domain ) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( firstClqNum, 0, m_numberOfNodes - 1 );
    PNL_CHECK_RANGES( secondClqNum, 0, m_numberOfNodes - 1 );
    PNL_CHECK_IS_NULL_POINTER(domSize);
    PNL_CHECK_IS_NULL_POINTER(domain);
    /* bad-args check */
    
    /* operation validity check */
    if( !m_pGraph->IsExistingEdge( firstClqNum, secondClqNum ) )
    {
        PNL_THROW( CInvalidOperation,
            " there is no edge between these nodes " )
    }
    /* operation validity check end */
    
    m_sepDomain.clear();
    
    intVecVector::const_iterator ncIter1 = ( m_nodeContents.begin()
        + firstClqNum ),
        ncIter2 = ( m_nodeContents.begin()
        + secondClqNum );
    
    intVector::const_iterator    clIter  = ncIter1->begin();
    
    for( ; ncIter1->end() - clIter; clIter++ )
    {
        if( std::find( ncIter2->begin(), ncIter2->end(), *clIter )
            != ncIter2->end() )
        {
            m_sepDomain.push_back(*clIter);
        }
    }
    
    *domSize = m_sepDomain.size();
    *domain = &m_sepDomain.front();
}
//////////////////////////////////////////////////////////////////////////

inline CPotential* CJunctionTree::GetNodePotential(int nodeNum)
{
    /* bad-args check */
    PNL_CHECK_RANGES( nodeNum, 0, m_numberOfNodes - 1 );
    /* bad-args check end */
    
    return *( m_nodePots.begin() + nodeNum );
}
//////////////////////////////////////////////////////////////////////////

inline CPotential* CJunctionTree::GetSeparatorPotential( int firstClqNum,
                                                         int secondClqNum )
{
    /* bad-args check */
    PNL_CHECK_RANGES( firstClqNum, 0, m_numberOfNodes - 1 );
    PNL_CHECK_RANGES( secondClqNum, 0, m_numberOfNodes - 1 );
    /* bad-args check end */
    
    /* operation validity check */
    if( firstClqNum == secondClqNum )
    {
        PNL_THROW( CInvalidOperation,
            " clique numbers should not be equal to each other " );
    }
    
    if( !m_pGraph->IsExistingEdge( firstClqNum, secondClqNum ) )
    {
        PNL_THROW( CInvalidOperation,
            " there is no edge between these two cliques " );
    }
    /* operation validity check end */
    
    return *( ( m_separatorPots.begin() + firstClqNum )->begin()
        + secondClqNum );
}
//////////////////////////////////////////////////////////////////////////

inline void CJunctionTree::GetClqNumsContainingSubset( int numOfNdsInSubset,
                                                       const int *subset,
                                                       int *numOfClqs,
                                                       const int 
                                                       **clqsContSubset )
                                                       const
{
    intVecVector::const_iterator ncIter = m_nodeContents.begin();
    const int *subIter;
    
    const int *subset_end = subset + numOfNdsInSubset;
    
    m_clqsContSubset.clear();
    
    for( ; m_nodeContents.end() != ncIter; ncIter++ )
    {
        for( subIter = subset; subset_end - subIter; subIter++ )
        {
            if( std::find( ncIter->begin(), ncIter->end(), *subIter )
                == ncIter->end() )
            {
                break;
            }
        }
        
        if( subIter == subset_end )
        {
            m_clqsContSubset.push_back( ncIter - m_nodeContents.begin() );
        }
    }
    
    *numOfClqs = m_clqsContSubset.size();
    *clqsContSubset = &m_clqsContSubset.front();
}
//////////////////////////////////////////////////////////////////////////

inline int 
CJunctionTree::IsMatchingTreeStructure(const CJunctionTree *pJTree) const
{
    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(pJTree)
        /* bad-args check end */
        
        if( *m_pGraph != *pJTree->m_pGraph )
        {
            return 0;
        }
        
        int i = 0;
        
        for( ; i < m_numberOfNodes; i++ )
        {
            /* checking that all cliques are of the same content */
            if( !std::equal( m_nodeContents[i].begin(), m_nodeContents[i].end(),
                pJTree->m_nodeContents[i].begin() ) )
            {
                return 0;
            }
            
            /* checking all nodes are of the same type */
            if( *GetNodeType(i) != *pJTree->GetNodeType(i) )
            {
                return 0;
            }
        }
        
        return 1;
}
//////////////////////////////////////////////////////////////////////////

inline bool CJunctionTree::IsChargeInitialized() const
{
    return m_bChargeInitialized;
}
//////////////////////////////////////////////////////////////////////////

inline int 
CJunctionTree::IsASubClq( int domSize, const int *domain, int clqNum ) const
{
    /* bad-args check */
    PNL_CHECK_LEFT_BORDER( domSize, 1 );
    PNL_CHECK_IS_NULL_POINTER(domain);
    PNL_CHECK_RANGES( clqNum, 0, m_nodeContents.size() - 1 );
    /* bad-args check end */
    
    const int *dnIter = domain;
    intVecVector::const_iterator ncIter = m_nodeContents.begin() + clqNum;
    
    for( ; ( domain + domSize ) - dnIter; dnIter++ )
    {
        if( std::find( ncIter->begin(), ncIter->end(), *dnIter )
            == ncIter->end() )
        {
            return 0;
        }
    }
    
    return 1;
}
//////////////////////////////////////////////////////////////////////////

inline EDistributionType 
CJunctionTree::DetermineDistributionType(int nodeNumber)
{
    /* bad-args check */
    PNL_CHECK_RANGES( nodeNumber, 0, m_numberOfNodes - 1 );
    /* bad-args check end */
    
    return m_pMD->GetVariableType(nodeNumber)->IsDiscrete()
        ? dtTabular : dtGaussian;
}
//////////////////////////////////////////////////////////////////////////

// this is an OBSOLETE version of this function
inline void 
CJunctionTree::AssureSubGraphsConnectedInCliques( int numOfSubGrToConnect,
                                                  const int *subGrToConnectSizes, 
                                                  const int **subGrToConnect )
{
    int i;
    
    /* bad-args check */
    if( numOfSubGrToConnect || subGrToConnectSizes || subGrToConnect )
    {
        PNL_CHECK_LEFT_BORDER( numOfSubGrToConnect, 1 );
        
        for( i = 0; i < numOfSubGrToConnect; ++i )
        {
            PNL_CHECK_RANGES( subGrToConnectSizes[i], 2,
                m_pMoralGraph->GetNumberOfNodes() );
            PNL_CHECK_IS_NULL_POINTER(subGrToConnect);
        }
    }
    /* bad-args check end */
    
    m_nodesToConnect.resize(numOfSubGrToConnect);
    
    for( i = 0; i < numOfSubGrToConnect; ++i )
    {
        m_pMoralGraph->FormCliqueFromSubgraph( subGrToConnectSizes[i],
            subGrToConnect[i] );
        
        intVector tmp( subGrToConnect[i], subGrToConnect[i] +
            subGrToConnectSizes[i] );
        
        m_nodesToConnect[i] = tmp;
    }
}
//////////////////////////////////////////////////////////////////////////

inline void CJunctionTree::
AssureSubGraphsConnectedInCliques(const intVecVector& subGrToConnect)
{
    intVecVector::const_iterator sgsIt   = subGrToConnect.begin(),
                                 sgs_end = subGrToConnect.end();

    for( ; sgsIt != sgs_end; ++sgsIt )
    {
        m_pMoralGraph->FormCliqueFromSubgraph(*sgsIt);
    }
    
    m_nodesToConnect.assign( subGrToConnect.begin(), subGrToConnect.end() );
}
//////////////////////////////////////////////////////////////////////////

inline void CJunctionTree::DumpNodeContents() const
{
    intVecVector::const_iterator ndContIt = m_nodeContents.begin(),
        ndCont_end = m_nodeContents.end();
    
    intVector::const_iterator clqIt, clq_end;
    Log dump("JunctionTree: ", eLOG_RESULT, eLOGSRV_PNL);
    
    dump<<"\n";
    
    for( ; ndContIt != ndCont_end; ++ndContIt )
    {
        dump<<"clq # "<<ndContIt - m_nodeContents.begin()<<": ";
        
        for( clqIt = ndContIt->begin(), clq_end = ndContIt->end();
        clqIt != clq_end; ++clqIt )
        {
            dump<<*clqIt<<" ";
        }
        
        dump<<"\n";
    }
    
    dump<<"\n";
    dump.flush();
}
//////////////////////////////////////////////////////////////////////////

inline int CJunctionTree::GetNumberOfNodes() const
{
    return m_nodeContents.size();
}
//////////////////////////////////////////////////////////////////////////

inline const CNodeType* CJunctionTree::GetNodeType(int nodeNumber) const
{
    PNL_CHECK_RANGES( nodeNumber, 0, m_numberOfNodes - 1 );

    return &(m_nodeTypes[m_nodeAssociation[nodeNumber]]);
}
//////////////////////////////////////////////////////////////////////////

PNL_END

#endif // __PNLJUNCTIONTREE_HPP__
