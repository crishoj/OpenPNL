/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlJunctionTree.cpp                                         //
//                                                                         //
//  Purpose:   CJunctionTree class member functions implementation         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlEvidence.hpp"
#include "pnlJunctionTree.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlScalarPotential.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlSoftMaxCPD.hpp"
#include "pnlCPD.hpp"
#include "pnlInferenceEngine.hpp"
#include "pnlIDNet.hpp"
#include "pnlDistribFun.hpp"
#include <sstream>

#ifdef PAR_OMP
#include <omp.h>
#endif

PNL_USING

static inline bool IsNodeObserved( int nodeToCheck,
                                   const intVector& obsNodesNumbers )
{
    return std::find( obsNodesNumbers.begin(), obsNodesNumbers.end(),
        nodeToCheck ) != obsNodesNumbers.end();
}
//////////////////////////////////////////////////////////////////////////

// this is an OBSOLETE Create function
CJunctionTree* CJunctionTree::Create( const CStaticGraphicalModel *pGrModel, 
                                      int numOfSubGrToConnect,
                                      const int *subGrToConnectSizes,
                                      const int **subGrToConnect )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    
    const EModelTypes modelType = pGrModel->GetModelType();
    
    if( ( modelType != mtBNet ) &&  ( modelType != mtMNet )
        && ( modelType != mtMRF2 ) && ( modelType != mtIDNet ))
    {
        PNL_THROW( CInconsistentType,
            " can construct JTree from either MNet, BNet , MRF2 or IDNet" );
    }
    
    if( numOfSubGrToConnect || subGrToConnectSizes || subGrToConnect )
    {
        PNL_CHECK_LEFT_BORDER( numOfSubGrToConnect, 1 );

        int i = 0;
        
        for( ; i < numOfSubGrToConnect; ++i )
        {
            PNL_CHECK_RANGES( subGrToConnectSizes[i], 2,
                pGrModel->GetNumberOfNodes() );
            
            PNL_CHECK_IS_NULL_POINTER(subGrToConnect);
        }
    }
    // bad-args check end
    
    CJunctionTree *pJTree = new CJunctionTree( pGrModel, numOfSubGrToConnect,
        subGrToConnectSizes, subGrToConnect );
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pJTree);
    
    return pJTree;
}
//////////////////////////////////////////////////////////////////////////

CJunctionTree* CJunctionTree::Create( const CStaticGraphicalModel* pGrModel,
                                      const intVecVector& subGrToConnect )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    
    const EModelTypes modelType = pGrModel->GetModelType();
    
    if( ( modelType != mtBNet ) &&  ( modelType != mtMNet )
        && ( modelType != mtMRF2 ) && ( modelType != mtIDNet ))
    {
        PNL_THROW( CInconsistentType,
            " can construct JTree from either MNet, BNet, MRF2 or IDNet"  );
    }
    // bad-args check end
    
    CJunctionTree *pJTree = new CJunctionTree( pGrModel, subGrToConnect );
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pJTree);
    
    return pJTree;
}
//////////////////////////////////////////////////////////////////////////

CJunctionTree* CJunctionTree::Copy(const CJunctionTree* pJTree)
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pJTree);
    // bad-args check end
    
    CJunctionTree* pJTreeCopy = new CJunctionTree(*pJTree);
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pJTreeCopy);
    
    return pJTreeCopy;
}
//////////////////////////////////////////////////////////////////////////

/*
void CJunctionTree::Release(CJunctionTree** pJTree)
{
    delete *pJTree;

    *pJTree = NULL;
}
*/

//////////////////////////////////////////////////////////////////////////

// this is an OBSOLETE constructor
CJunctionTree::CJunctionTree( const CStaticGraphicalModel *pGrModel,
                              int numOfSubGrToConnect,
                              const int *subGrToConnectSizes,
                              const int **subGrToConnect ) 
                              : m_factAssignToClq(
                              pGrModel->GetNumberOfFactors(), 0 ),
                              m_pMoralGraph(
                              ((pGrModel->GetModelType() == mtBNet)
                              ||(pGrModel->GetModelType() == mtIDNet)) ?
                              CGraph::MoralizeGraph(pGrModel->GetGraph())
                              : CGraph::Copy(pGrModel->GetGraph())),
                              m_bChargeInitialized(false),
                              CStaticGraphicalModel( pGrModel->GetModelDomain() )
{
    m_modelType = mtJTree;
    
    // pass Model to ConstructJTree
    ConstructJTree( pGrModel, numOfSubGrToConnect, subGrToConnectSizes,
        subGrToConnect );
}
//////////////////////////////////////////////////////////////////////////

CJunctionTree::CJunctionTree( const CStaticGraphicalModel* pGrModel,
                              const intVecVector& subGrToConnect )
                              : m_factAssignToClq(
                              pGrModel->GetNumberOfFactors(), 0 ),
                              m_pMoralGraph(
                              ((pGrModel->GetModelType() == mtBNet)
                              ||(pGrModel->GetModelType() == mtIDNet)) ?
                              CGraph::MoralizeGraph(pGrModel->GetGraph())
                              : CGraph::Copy(pGrModel->GetGraph())),
                              m_bChargeInitialized(false),
                              CStaticGraphicalModel( pGrModel->GetModelDomain() )
{
    m_modelType = mtJTree;
    
    // pass Model to ConstructJTree
    ConstructJTree( pGrModel, subGrToConnect );
}
//////////////////////////////////////////////////////////////////////////

CJunctionTree::CJunctionTree(const CJunctionTree& rJTree)
                             : m_clqsContSubset(0), m_sepDomain(0),
                             m_nodeContents( rJTree.m_nodeContents.begin(),
                             rJTree.m_nodeContents.end() ),
                             m_nodesToConnect( rJTree.m_nodesToConnect.
                             begin(), rJTree.m_nodesToConnect.end() ),
                             m_perfectNumbering( rJTree.m_perfectNumbering.
                             begin(), rJTree.m_perfectNumbering.end() ),
                             m_pMoralGraph(CGraph::Copy(rJTree.
                             m_pMoralGraph)),
                             CStaticGraphicalModel( rJTree.GetModelDomain() )
{
    m_modelType = mtJTree;
    
    m_numberOfNodes = rJTree.m_numberOfNodes;

    m_pGraph = CGraph::Copy(rJTree.m_pGraph);

    m_pGraph->ProhibitChange();

    m_pMoralGraph->ProhibitChange();

    
    if( rJTree.IsChargeInitialized() )
    {
        m_bChargeInitialized = true;
        
        m_nodePots.assign( m_numberOfNodes, (CPotential *)0 );
        m_separatorPots.assign( m_numberOfNodes, m_nodePots );
        
        m_nodeAssociation.assign( rJTree.m_nodeAssociation.begin(),
            rJTree.m_nodeAssociation.end() );
        
        m_nodeTypes.assign( rJTree.m_nodeTypes.begin(),
            rJTree.m_nodeTypes.end() );
        
        m_factAssignToClq.assign( rJTree.m_factAssignToClq.begin(),
            rJTree.m_factAssignToClq.end() );
        
        int i, j;
        
        for( i = 0; i < m_numberOfNodes; ++i )
        {
            // creating node potential according to the types of the
            //potentials of the original jtree
            switch(rJTree.m_nodePots[i]->GetDistributionType())
            {
                case dtTabular:
                {
                    m_nodePots[i] = CTabularPotential::Copy(
                        static_cast<CTabularPotential *>
                        (rJTree.m_nodePots[i]));
                    break;
                }
                case dtGaussian:
                {
                    m_nodePots[i] = CGaussianPotential::Copy(
                        static_cast<CGaussianPotential *>
                        (rJTree.m_nodePots[i]));
                    break;
                }
                case dtScalar:
                {
                    m_nodePots[i] = CScalarPotential::Copy(
                        static_cast<CScalarPotential *>
                        (rJTree.m_nodePots[i]));
                    break;
                }
                default:
                {
                    PNL_THROW( CInconsistentType,
                        " the pot should be talular, gaussian or mixt " );
                }
            }
            
            for( j = i + 1; j < m_numberOfNodes; j++ )
            {
                if( rJTree.m_separatorPots[i][j] )
                {
                    // creating separator potential according to the types
                    // of the potentials of the original jtree
                    switch( rJTree.m_separatorPots[i][j]
                        ->GetDistributionType() )
                    {
                        case dtTabular:
                        {
                            m_separatorPots[i][j] = CTabularPotential::Copy(
                                static_cast<CTabularPotential *>
                                (rJTree.m_separatorPots[i][j]) );
                            break;
                        }
                        case dtGaussian:
                        {
                            m_separatorPots[i][j] = CGaussianPotential::Copy(
                                static_cast<CGaussianPotential *>
                                (rJTree.m_separatorPots[i][j]) );
                            break;
                        }
                        case dtScalar:
                        {
                            m_separatorPots[i][j] = CScalarPotential::Copy(
                                static_cast<CScalarPotential *>
                                (rJTree.m_separatorPots[i][j]) );
                            break;
                        }
                        default:
                        {
                            PNL_THROW( CInconsistentType,
                                " the pot should be talular or gaussian " );
                        }
                    }
                    
                    m_separatorPots[j][i] = m_separatorPots[i][j];
                }
            }
        }
    }
    else
    {
        m_bChargeInitialized = false;
    }
}
//////////////////////////////////////////////////////////////////////////

CJunctionTree::~CJunctionTree()
{
    int i, j;

    delete m_pMoralGraph;

    if( m_bChargeInitialized )
    {
        for( i = 0; i < m_numberOfNodes; ++i )
        {
            delete m_nodePots[i];
            
            for( j = i + 1; j < m_numberOfNodes; ++j )
            {
                delete m_separatorPots[i][j];
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////

bool CJunctionTree::IsValid(std::string* description) const
{
    if( ! m_pMoralGraph )
    {
	if( description )
	{
	    std::stringstream st;
	    st<<"Junction Tree have invalid graph"<<std::endl;
            std::string s = st.str();
	    description->insert( description->begin(), s.begin(), s.end() );
	}
	
	return false;
    }
    int nPots = m_nodePots.size();
    
    int i; 
    for( i = 0; i < nPots; i++ )
    {
	if( !m_nodePots[i]->IsValid() )
	{
	    if( description )
	    {
		std::stringstream st;
		st<<"Junction Tree have invalid node potential"<<std::endl;
                std::string s = st.str();
		description->insert( description->begin(), s.begin(), s.end() );
	    }
	    
	    return false;
	}
    }
    for( i = 0; i < m_separatorPots.size(); i++ )
    {
	int j;
	for( j = 0; j < m_separatorPots[i].size(); j++ )
	{
	    if( !m_separatorPots[i][j]->IsValid() )
	    {
		if( description )
		{
		    std::stringstream st;
		    st<<"Junction Tree have invalid separator potential"<<std::endl;
                    std::string s = st.str();
		    description->insert( description->begin(), s.begin(), s.end() );
		}
	    }
	}
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

// this is an OBSOLETE version of Construction
void CJunctionTree::ConstructJTree( const CStaticGraphicalModel* pGrModel,
                                    int numOfSubGrToConnect,
                                    const int *subGrToConnectSizes, 
                                    const int **subGrToConnect )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    // bad-args check end
    
    // check model validity
    if( !pGrModel->IsValid() )
    {
        PNL_THROW( CInvalidOperation,
            " trying to construct a JTRee from invalid input model " );
    }
    
    // connect nodes, which user wants to appear in one clique
    AssureSubGraphsConnectedInCliques( numOfSubGrToConnect,
        subGrToConnectSizes, subGrToConnect );
    
    // do maximum cardinality search to find the perfect numbering
    // and extract cliques from the internally stored graph
    while( !MaximumCardinalitySearch() )
    {
        RunTriangulation();
    }
    
    // builds the tree structure itself (which clique is connected to which)
    // without initializing potentials for the cliques
    BuildTreeStructure();
}
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::ConstructJTree( const CStaticGraphicalModel* pGrModel,
                                    const intVecVector& subGrToConnect )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    // bad-args check end
    
    
    // check model validity
    if( !pGrModel->IsValid() )
    {
        PNL_THROW( CInvalidOperation,
            " trying to construct a JTRee from invalid input model " );
    }
    
    // connect nodes, which user wants to appear in one clique
    AssureSubGraphsConnectedInCliques(subGrToConnect);
    
    // do maximum cardinality search to find the perfect numbering
    // and extract cliques from the internally stored graph
    while( !MaximumCardinalitySearch() )
    {
        RunTriangulation();
    }
    
    // builds the tree structure itself (which clique is connected to which)
    // without initializing potentials for the cliques
    BuildTreeStructure();
}


int CJunctionTree::MaximumCardinalitySearch() const
{
    // Returns 1 if the m_pMoralGraph is triangulated and 0 otherwise. Also
    // fills the "order" vector with the node numbers in "perfect numbering"
    // (see Cowell, Dawid and Lauritzen)
    
    // NOTE: uses maximum cardinality search algorithm
    
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(m_pMoralGraph);
    // bad-args check end
    
    int numOfNds = m_pMoralGraph->GetNumberOfNodes();
    int i, j;
    
    int max;
    int node;
    int nodeIndex;
    
    int lastCard;
    
    intVector::iterator location;
    
    int                 numOfNbrs;
    const int           *nbrs;
    const ENeighborType *nbrsTypes;
    const int           *nbrsIt, *nbrs_end;
    
    intVector UnNodesed;
    intVector::const_iterator unIt, un_end;
    
    intVector c( numOfNds, 0 );
    intVector intersectWithNumbered;
    intVector intersectWithUnNodesed;
    
    m_perfectNumbering.assign(1, 0);
    
    lastCard = 0;
    intVector clique;
    
    // reserve storage space
    UnNodesed.reserve(numOfNds);
    intersectWithNumbered.reserve(numOfNds);
    intersectWithUnNodesed.reserve(numOfNds);
    
    // try to change the numbered and unnumbered to the flags
    for( i = 1; i < numOfNds; ++i )
    {
        UnNodesed.push_back(i);
    }
    
    m_pMoralGraph->GetNeighbors( 0, &numOfNbrs, &nbrs, &nbrsTypes );
    
    for( nbrsIt = nbrs, nbrs_end = nbrs + numOfNbrs; nbrs_end - nbrsIt;
    ++nbrsIt )
    {
        c[*nbrsIt]++;
    }
    
    for( i = 1; i < numOfNds; i++ )
    {
    /* find node, so that it maximises c(node), for all nodes
        from the unnumbered nodes vector */
        nodeIndex = 0;
        node      = UnNodesed.front();
        max       = c[node];
        
        for( unIt = UnNodesed.begin() + 1, un_end = UnNodesed.end();
        un_end - unIt; ++unIt )
        {
            if( c[*unIt] > max )
            {
                nodeIndex = unIt - UnNodesed.begin();
                node      = *unIt;
                max       = c[node];
            }
        }
        
        /* find intersections for neighbors of the node with both
        numbered and unnumbered nodes */
        m_pMoralGraph->GetNeighbors( node, &numOfNbrs, &nbrs,
            &nbrsTypes );
        
        for( j = 0; j < numOfNbrs; j++ )
        {
            location = std::find( m_perfectNumbering.begin(),
                m_perfectNumbering.end(), nbrs[j] );
            
            if( location != m_perfectNumbering.end() )
            {
                intersectWithNumbered.push_back(nbrs[j]);
            }
            else
            {
                intersectWithUnNodesed.push_back(nbrs[j]);
            }
        }
        
        /* look if intersection of neighbors of the node and nodes already
        numbered  is complete in terms of the graph we are working with */
        if( !m_pMoralGraph->IsCompleteSubgraph( intersectWithNumbered.size(),
            &intersectWithNumbered.front() ) )
        {
            /* clears the cliques vector */
            m_nodeContents.clear();
            
            /* clears the perfect numbering vector */
            m_perfectNumbering.clear();
            
            /* returns zero to show that the m_pMoralGraph is not triangulated */
            return 0;
        }
        
        /* next is forming cliques (algorithm carried out online) */
        if( intersectWithNumbered.size() < lastCard + 1 )
        {
            m_nodeContents.push_back(clique);
        }
        
        lastCard = intersectWithNumbered.size();
        
        clique.assign( intersectWithNumbered.begin(),
            intersectWithNumbered.end() );
        clique.push_back(node);
        
        if( i == numOfNds - 1 )
        {
            m_nodeContents.push_back(clique);
        }
        
        /* otherwise increase the value of c(v) for all v from the
        intersection of neighbors of node and UnNodesed */
        for( j = 0; j < intersectWithUnNodesed.size(); j++ )
        {
            c[intersectWithUnNodesed[j]]++;
        }
        
        /* clear intersections, move the node from UnNodesed to order */
        intersectWithNumbered.clear();
        intersectWithUnNodesed.clear();
        UnNodesed.erase( UnNodesed.begin() + nodeIndex );
        m_perfectNumbering.push_back(node);
    }
    
    return 1;
}
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::RunTriangulation()
{
    // performs triangulation procedure for m_pMoralGraph. Triangulation 
    // is done by adding extra fill-in edges to the graph, so it becomes
    // chordal (see Cowell and Lauritzen)
    
    int numberOfNodes = m_pMoralGraph->GetNumberOfNodes();
    
    // operation validity check
    if( int(m_perfectNumbering.size()) == numberOfNodes )
    {
        PNL_THROW( CInvalidOperation, " graph's triangulated already " );
    }
    // operation validity check end
    
    int node;
    const int *nbr;
    intVector::const_iterator iter1, iter2;
    
    int                  numOfNbrs;
    const int            *nbrs;
    const ENeighborType  *nbrsTypes;
    
    intVector  triangOrder;
    
    intVector  neighborsToConnect;
    intVector  connectThisTime;
    boolVector eliminated(numberOfNodes);
    
    bool bFirst;
    int  numberOfFillIns;
    int  minimNumberOfFillIns = 0;
    
    int i, j;
    
    for( i = 0; i < numberOfNodes; ++i )
    {
        /* find the node for which the number of fill-in edges is minimal */
        bFirst = true;
        
        for( j = 0; j < numberOfNodes; ++j )
        {
            if( !eliminated[j] )
            {
                numberOfFillIns = 0;
                
                m_pMoralGraph->GetNeighbors( j, &numOfNbrs, &nbrs,
                    &nbrsTypes );
                
                for( nbr = nbrs; ( nbrs + numOfNbrs ) - nbr; ++nbr )
                {
                    if( !eliminated[*nbr] )
                    {
                        for( iter1 = neighborsToConnect.begin();
                        neighborsToConnect.end() != iter1; ++iter1 )
                        {
                            if( !m_pMoralGraph->IsExistingEdge( *nbr,
                                *iter1 ) )
                            {
                                ++numberOfFillIns;
                            }
                        }
                        
                        neighborsToConnect.push_back(*nbr);
                    }
                }
                
                if( bFirst )
                {
                    bFirst = false;
                    
                    node = j;
                    minimNumberOfFillIns = numberOfFillIns;
                    connectThisTime.assign( neighborsToConnect.begin(),
                        neighborsToConnect.end() );
                }
                else if( numberOfFillIns < minimNumberOfFillIns )
                {
                    node = j;
                    minimNumberOfFillIns = numberOfFillIns;
                    connectThisTime.assign( neighborsToConnect.begin(),
                        neighborsToConnect.end() );
                }
            }
            
            neighborsToConnect.clear();
        }
        
        /* add the node to the perfectNumbering vector */
        m_perfectNumbering.push_back(node);
        
        /* connect uneliminated neighbors of the chosen node */
        for( iter1 = connectThisTime.begin(); connectThisTime.end() != iter1;
        ++iter1 )
        {
            for( iter2 = (iter1 + 1); connectThisTime.end() != iter2; ++iter2 )
            {
                m_pMoralGraph->AddEdge( (*iter1), (*iter2), 0);
            }
        }
        
        /* eliminate the node */
        eliminated[node] = true;
        
        connectThisTime.clear();
    }
}
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::BuildTreeStructure()
{
    // building tree structure means connecting cliques
    // (nodes of jtree), so that it has a tree structure
    
    m_numberOfNodes = m_nodeContents.size();
    m_pGraph = CGraph::Create( m_numberOfNodes, NULL );
    
    int numberOfFound;
    int maxNumberOfFound;
    int connectTo = 0;
    
    intVector::const_iterator    clqIt, clq_end;
    intVecVector::const_iterator clqListIt1 = m_nodeContents.begin() + 1,
        clqListIt2, clqList_end = m_nodeContents.end();
    
    for( ; clqListIt1 != clqList_end; ++clqListIt1 )
    {
        maxNumberOfFound = 0;
        
        for( clqListIt2 = m_nodeContents.begin(); clqListIt2 != clqListIt1;
        ++clqListIt2 )
        {
            numberOfFound = 0;
            
            for( clqIt = clqListIt2->begin(), clq_end = clqListIt2->end();
            clqIt != clq_end; ++clqIt )
            {
                if( std::find( clqListIt1->begin(), clqListIt1->end(),
                    *clqIt ) != clqListIt1->end() )
                {
                    ++numberOfFound;
                }
            }
            
            if( numberOfFound > maxNumberOfFound )
            {
                maxNumberOfFound = numberOfFound;
                connectTo = clqListIt2 - m_nodeContents.begin();
            }
        }
        
        m_pGraph->AddEdge( clqListIt1 - m_nodeContents.begin(),
            connectTo, 0 );
    }
    
    m_pGraph->ProhibitChange();
}
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::InitCharge( const CStaticGraphicalModel *pGrModel,
                                const CEvidence *pEvidence,
                                int sumOnMixtureNode )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    // bad-args check end
    
    AssignFactorsToClq(pGrModel);
    
    SetNodeTypes( pGrModel, pEvidence );
    
    if( pGrModel->GetModelType() == mtBNet )
    {
        InitNodePotsFromBNet( static_cast<const CBNet*>(pGrModel),
            pEvidence, sumOnMixtureNode );
    }
    else
    {
        InitNodePotsFromMNet( static_cast<const CMNet*>(pGrModel),
            pEvidence );
    }
    
    InitSeparatorsPots( pGrModel, pEvidence );
    
    m_bChargeInitialized = true;
}
//////////////////////////////////////////////////////////////////////////

#ifdef PAR_OMP
void CJunctionTree::InitChargeOMP( const CStaticGraphicalModel *pGrModel,
    const CEvidence *pEvidence, int sumOnMixtureNode )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pGrModel);
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    // bad-args check end

    AssignFactorsToClqOMP(pGrModel);

    SetNodeTypes( pGrModel, pEvidence );

    if( pGrModel->GetModelType() == mtBNet )
    {
        InitNodePotsFromBNetOMP( static_cast<const CBNet*>(pGrModel),
            pEvidence, sumOnMixtureNode );
    }
    else
    {
        InitNodePotsFromMNet( static_cast<const CMNet*>(pGrModel),
            pEvidence );
    }

    InitSeparatorsPots( pGrModel, pEvidence );

    m_bChargeInitialized = true;
}
#endif // PAR_OMP
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::ClearCharge()
{
    if( m_bChargeInitialized )
    {
        int i, j;
        
        for( i = 0; i < m_numberOfNodes; ++i )
        {
            delete m_nodePots[i];
            
            for( j = i + 1; j < m_numberOfNodes; ++j )
            {
                delete m_separatorPots[i][j];
            }
        }
        
        m_factAssignToClq.clear();
        
        m_nodeTypes.clear();
        
        m_nodeAssociation.clear();
        
        m_nodePots.clear();
        
        m_separatorPots.clear();
        
        m_bChargeInitialized = false;
    }
}
//////////////////////////////////////////////////////////////////////////

#ifdef PAR_OMP
void CJunctionTree::ClearChargeOMP()
{
    if( m_bChargeInitialized )
    {
        int i, j;

        omp_lock_t **sep_lock;
        sep_lock = new omp_lock_t* [m_numberOfNodes];
        for (i = 0; i < m_numberOfNodes; i++)
        {
            sep_lock[i] = new omp_lock_t [m_numberOfNodes];
            for (j = 0; j < m_numberOfNodes; j++)
            {
                omp_init_lock(&sep_lock[i][j]);
            }
        }

        #pragma omp parallel for private(i)
        for (i = 0; i < m_numberOfNodes; ++i)
        {
            delete m_nodePots[i];

            int                 numOfNbrs;
            const int           *nbrs;
            const ENeighborType *nbrsTypes;

            m_pGraph->GetNeighbors( i, &numOfNbrs, &nbrs, &nbrsTypes );

            const int *nbrsIt = nbrs;
            const int *nbrs_end = nbrs + numOfNbrs;

            for (; nbrsIt != nbrs_end; ++nbrsIt)
            {
                omp_set_lock(&sep_lock[*nbrsIt][i]);
                omp_set_lock(&sep_lock[i][*nbrsIt]);
                delete m_separatorPots[i][*nbrsIt];
                omp_unset_lock(&sep_lock[i][*nbrsIt]);
                omp_unset_lock(&sep_lock[*nbrsIt][i]);
            }
        }

        for (i = 0; i < m_numberOfNodes; i++)
        {
            for (j = 0; j < m_numberOfNodes; j++)
            {
                omp_destroy_lock(&sep_lock[i][j]);
            } 
            delete [] sep_lock[i];
        }
        delete [] sep_lock;

        m_factAssignToClq.clear();

        m_nodeTypes.clear();

        m_nodeAssociation.clear();

        m_nodePots.clear();

        m_separatorPots.clear();

        m_bChargeInitialized = false;
    }
}
#endif // PAR_OMP
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::AssignFactorsToClq(const CStaticGraphicalModel
                                       *pGrahicalModel) const
{
    // assigns a potential of original graphical model to a clique of jtree
    int i, j;
    
    int numOfFacs = pGrahicalModel->GetNumberOfFactors();
    int numOfClqs = m_numberOfNodes;
    
    int domSize;
    const int *domain;
    
    m_factAssignToClq.assign( numOfFacs, -1 );
    
    for( i = 0; i < numOfFacs; ++i )
    {
        pGrahicalModel->GetFactor(i)->GetDomain( &domSize, &domain );
        
        for( j = 0; j < numOfClqs; ++j )
        {
            if( IsASubClq( domSize, domain, j ) )
            {
                *( m_factAssignToClq.begin() + i ) = j;
            }
        }
        
        if( *( m_factAssignToClq.begin() + i ) == -1 )
        {
        /* means that there was no clique found which contains
            all nodes from the potential domain */
            PNL_THROW( CInconsistentState,
                " no clique found to match the domain " );
        }
    }
}
//////////////////////////////////////////////////////////////////////////

#ifdef PAR_OMP
void CJunctionTree::AssignFactorsToClqOMP(
    const CStaticGraphicalModel *pGrahicalModel) const
{
    // assigns a potential of original graphical model to a clique of jtree
    int i;

    int numOfFacs = pGrahicalModel->GetNumberOfFactors();
    int numOfClqs = m_numberOfNodes;

    m_factAssignToClq.assign(numOfFacs, -1);

    #pragma omp parallel for private (i)
    for (i = 0; i < numOfFacs; ++i)
    {
        int domSize;
        const int *domain;
        pGrahicalModel->GetFactor(i)->GetDomain(&domSize, &domain);

        int j;
        for(j = 0; j < numOfClqs; ++j)
        {
            if (IsASubClq(domSize, domain, j))
            {
                *(m_factAssignToClq.begin() + i) = j;
            }
        }

        //if( *( m_factAssignToClq.begin() + i ) == -1 )
        //{
        /* means that there was no clique found which contains
            all nodes from the potential domain */
            //PNL_THROW( CInconsistentState,
                //" no clique found to match the domain " );
        //}
    }
}
#endif // PAR_OMP
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::SetNodeTypes( const CStaticGraphicalModel
                                 *pGraphicalModel,
                                 const CEvidence *pEvidence )
{
    intVecVector::const_iterator   ndContIt   = m_nodeContents.begin(),
        ndCont_end = m_nodeContents.end();
    intVector::const_iterator      clqIt, clq_end;
    nodeTypeVector::const_iterator ndTypeIt, ndType_end;
    
    intVector obsNodesNums;
    pConstValueVector obsNodesVals;
    
    intVector obsNodesInClq;
    pConstNodeTypeVector ndTypesForClqNds;
    
    pEvidence->GetObsNodesWithValues( &obsNodesNums, &obsNodesVals );
    
    m_nodeAssociation.resize(m_numberOfNodes);
    
    for( ; ndContIt != ndCont_end; ++ndContIt )
    {
        for( clqIt = ndContIt->begin(), clq_end = ndContIt->end();
        clqIt != clq_end; ++clqIt )
        {
            if( IsNodeObserved( *clqIt, obsNodesNums ) )
            {
                obsNodesInClq.push_back( clqIt - ndContIt->begin() );
            }
            
            ndTypesForClqNds.push_back(pGraphicalModel->GetNodeType(*clqIt));
        }
        
        EDistributionType distrType = pnlDetermineDistributionType( 
            ndContIt->size(), obsNodesInClq.size(), &obsNodesInClq.front(),
            &ndTypesForClqNds.front() );
        
        ndTypesForClqNds.clear();
        obsNodesInClq.clear();
        
        bool bDiscrete;
        int  nodeSize;
        
        switch (distrType)
        {
        case dtTabular:
            {
                bDiscrete = true;
                nodeSize  = 1;
                break;
            }
        case dtGaussian:
            {
                bDiscrete = false;
                nodeSize  = 0;
                break;
            }
        case dtScalar:
            {
                bDiscrete = true;
                nodeSize = 0;
                break;
            }
        case dtSoftMax: case dtCondSoftMax:
            {
                bDiscrete = true;
                nodeSize = 1;
                break;
            }
        default:
            {
                PNL_THROW( CInvalidOperation,
                    " invalid distribution for JTree construction " );
            }
        }
        
        for( clqIt = ndContIt->begin(), clq_end = ndContIt->end();
        clqIt != clq_end; ++clqIt )
        {
            if( std::find( obsNodesNums.begin(), obsNodesNums.end(), *clqIt )
                == obsNodesNums.end() )
            {
                bDiscrete ?
                    nodeSize *= pGraphicalModel->GetNodeType(*clqIt)
                    ->GetNodeSize()
                    : nodeSize += pGraphicalModel->GetNodeType(*clqIt)
                    ->GetNodeSize();
            }
        }
        
        for( ndTypeIt = m_nodeTypes.begin(), ndType_end = m_nodeTypes.end();
        ndTypeIt != ndType_end; ++ndTypeIt )
        {
            if( ( ndTypeIt->GetNodeSize() == nodeSize )
                && ( ndTypeIt->IsDiscrete() == bDiscrete ) )
            {
                break;
            }
        }
        
        if( ndTypeIt == ndType_end )
        {
            /* means that was no type found which matches the new one */
            CNodeType nt( bDiscrete, nodeSize );
            
            m_nodeTypes.push_back(nt);
            m_nodeAssociation[ndContIt - m_nodeContents.begin()] =
                m_nodeTypes.size() - 1;
        }
        else
        {
            m_nodeAssociation[ndContIt - m_nodeContents.begin()] = 
                ndTypeIt - m_nodeTypes.begin();
        }
    }
}
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::InitNodePotsFromBNet( const CBNet* pBNet,
                                          const CEvidence* pEvidence,
                                          int sumOnMixtureNode )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pBNet);
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    // bad-args check end
    
    // cannot initialize, if the initialization have been carried out already
    if( m_bChargeInitialized )
    {
        PNL_THROW( CInvalidOperation,
            " cannot reinitialize pots, run Clear() first " );
    }


    intVector         obsNdsNums;
    pConstValueVector obsNdsVals;

    pEvidence->GetObsNodesWithValues( &obsNdsNums, &obsNdsVals );

    pConstNodeTypeVector nodeTypes;

    // number of potentials is equal to the number of nodes in jtree
    m_nodePots.resize(m_numberOfNodes);

    intVector::iterator factAssToClq_begin = m_factAssignToClq.begin(),
                        factAssToClq_end   = m_factAssignToClq.end();
    
    pConstCPDVector assignedFactors;
    
    intVecVector::const_iterator ndContIt = m_nodeContents.begin();

    int i = 0, k;
    
    bool allDiscrObs = pEvidence->IsAllDiscreteNodesObs(pBNet);
    bool allCountObs = pEvidence->IsAllCountinuesNodesObs(pBNet);

    if (allDiscrObs)
    {
        for (k = 0; k < pBNet->GetNumberOfNodes(); k++)
        {
            if (pBNet->GetFactor(k)->GetDistributionType() == dtSoftMax)
            {
                pBNet->GetModelDomain()->ChangeNodeType(k, 1);
            }
        }
    }

    
   for( ; i < m_numberOfNodes; ++i, ++ndContIt )
    {
        // finding out if the unit function is to be sparse or not
        bool bDenseUnitFunction = true;

        intVector::iterator potsAssIt = std::find( factAssToClq_begin, 
            factAssToClq_end, i );

        intVector summarizeNodes;

        while( potsAssIt != factAssToClq_end )
        {
            CCPD* pAssCPD = static_cast<CCPD*>(pBNet
                ->GetFactor( potsAssIt - factAssToClq_begin ));

            assignedFactors.push_back(pAssCPD);
            
            if( pAssCPD->IsSparse() )
            {
                bDenseUnitFunction = false;
            }
            if( sumOnMixtureNode )
            {
                if( pAssCPD->GetDistributionType() == dtMixGaussian )
                {
                    summarizeNodes.push_back(static_cast<CMixtureGaussianCPD*>
                        (pAssCPD)->GetNumberOfMixtureNode());            
                }
            }

            potsAssIt = std::find( ++potsAssIt,	factAssToClq_end, i );
        }


        intVector::const_iterator clqIt   = ndContIt->begin(),
                                  clq_end = ndContIt->end();
        intVector obsPositionsInDom;

        for( ; clqIt != clq_end; ++clqIt )
        {
            if( std::find( summarizeNodes.begin(), summarizeNodes.end(),
                *clqIt ) != summarizeNodes.end() )
            {
                nodeTypes.push_back(CInfEngine::GetObsTabNodeType());
                obsPositionsInDom.push_back( clqIt - ndContIt->begin() );
            }
            else if( IsNodeObserved( *clqIt, obsNdsNums ) )
            {
                pBNet->GetNodeType(*clqIt)->IsDiscrete()?
                    nodeTypes.push_back(CInfEngine::GetObsTabNodeType())
                    : nodeTypes.push_back(CInfEngine::GetObsGauNodeType());
                obsPositionsInDom.push_back( clqIt - ndContIt->begin() );
            }
            else
            {
                nodeTypes.push_back(pBNet->GetNodeType(*clqIt));
            }
        }
        //find observed posittions in this domain
        
        
        // creating the unit function itself
        CPotential *pPot = NULL;
        const CNodeType nT = m_nodeTypes[m_nodeAssociation[i]];
        if((nT.IsDiscrete()) && (nT.GetNodeSize() == 0))
        {
            pPot = CScalarPotential::Create( &ndContIt->front(),
                        ndContIt->size(), m_pMD, obsPositionsInDom );
        }
        else
        {
            if( nT.IsDiscrete() )
            {
                pPot = CTabularPotential::CreateUnitFunctionDistribution(
                    &ndContIt->front(), ndContIt->size(), m_pMD, bDenseUnitFunction, 
                    obsPositionsInDom);
            }
            else
            {
                pPot = CGaussianPotential::CreateUnitFunctionDistribution(
                    &ndContIt->front(), ndContIt->size(), m_pMD, 1,
                    obsPositionsInDom);
            }
        }
        //pPot->Dump();
        
        //CPotential* pShrPot = pPot->ShrinkObservedNodes( pEvidence );
        //delete pPot ;
        //pPot = pShrPot;

        // multiplying unit function by all the BNet's CPDs attached to it
        pConstCPDVector::const_iterator
                                       assFactsIt   = assignedFactors.begin(),
                                       assFacts_end = assignedFactors.end();

        bool AllNodesInClsObs = IsAllNodesInClsObs(i, pEvidence );

        for( ; assFactsIt != assFacts_end; ++assFactsIt )
        {
            const CPotential *pTmpPot = NULL;
            
            EDistributionType dt = (*assFactsIt)->GetDistribFun()->GetDistributionType();
                        
            intVector domain;
            (*assFactsIt)->GetDomain(&domain);
            int FactorNode = domain[domain.size()-1];

            if (dt == dtSoftMax)
            {
                if (allDiscrObs)
                {
                    if (AllNodesInClsObs)
                    {

                        for (k = 0; k < pBNet->GetNumberOfNodes(); k++)
                        {
                            if (pBNet->GetFactor(k)->GetDistributionType() == dtSoftMax)
                            {
                                pBNet->GetModelDomain()->ChangeNodeType(k, 0);
                            }
                        }

                        pBNet->GetModelDomain()->ChangeNodeType(FactorNode, 0);

                        pTmpPot = ((CSoftMaxCPD*)(*assFactsIt))->
                                ConvertWithEvidenceToTabularPotential( pEvidence);

                        for (k = 0; k < pBNet->GetNumberOfNodes(); k++)
                        {
                            if (pBNet->GetFactor(k)->GetDistributionType() == dtSoftMax)
                            {
                                pBNet->GetModelDomain()->ChangeNodeType(k, 1);
                            }
                        }
//                        pTmpPot->Dump();

                    }
                    else
                    {
                        floatVector MeanContParents;
                        C2DNumericDenseMatrix<float>* CovContParents;

                        for (k = 0; k < pBNet->GetNumberOfNodes(); k++)
                        {
                            if (pBNet->GetFactor(k)->GetDistributionType() == dtSoftMax)
                            {
                                pBNet->GetModelDomain()->ChangeNodeType(k, 0);
                            }
                        }

                        pBNet->GetModelDomain()->ChangeNodeType(FactorNode, 1);

                        static_cast<const CSoftMaxCPD*>(*assFactsIt)->
                            CreateMeanAndCovMatrixForNode(FactorNode, pEvidence, pBNet, 
                            MeanContParents, &CovContParents);
                    
                        pTmpPot = static_cast<const CSoftMaxCPD*>(*assFactsIt)->
                            ConvertWithEvidenceToGaussianPotential( pEvidence, 
                            MeanContParents, CovContParents);

                        for (k = 0; k < pBNet->GetNumberOfNodes(); k++)
                        {
                            if (pBNet->GetFactor(k)->GetDistributionType() == dtSoftMax)
                            {
                                pBNet->GetModelDomain()->ChangeNodeType(k, 1);
                            }
                        }

//                        pTmpPot->Dump();
                    }
                }
                else 
                    if (allCountObs)
                    {
                        pTmpPot = ((CSoftMaxCPD*)(*assFactsIt))->
                            ConvertWithEvidenceToTabularPotential( pEvidence);
//                        pTmpPot->Dump();
                    }
                
            } 
            else 
                if (dt == dtCondSoftMax)
                {
                    if (allDiscrObs)
                    {
                        if (AllNodesInClsObs)
                        {
                            pBNet->GetModelDomain()->ChangeNodeType(FactorNode, 0);

                            pTmpPot = ((CSoftMaxCPD*)(*assFactsIt))->
                                ConvertWithEvidenceToTabularPotential( pEvidence);

//                            pTmpPot->Dump();

                        }
                        else
                        {
                            floatVector MeanContParents;
                            C2DNumericDenseMatrix<float>* CovContParents;

                            for (k = 0; k < pBNet->GetNumberOfNodes(); k++)
                            {
                                if (pBNet->GetFactor(k)->GetDistributionType() == dtSoftMax)
                                {
                                    pBNet->GetModelDomain()->ChangeNodeType(k, 0);
                                }
                            }

                            pBNet->GetModelDomain()->ChangeNodeType(FactorNode, 1);
    
                            static_cast<const CSoftMaxCPD*>(*assFactsIt)->
                                CreateMeanAndCovMatrixForNode(FactorNode, pEvidence, 
                                pBNet, MeanContParents, &CovContParents);

                            intVector discParents;
                            pBNet->GetDiscreteParents(FactorNode, &discParents);
                        
                            int *parentComb = new int [discParents.size()];
                        
                            intVector pObsNodes;
                            pConstValueVector pObsValues;
                            pConstNodeTypeVector pNodeTypes;
                            pEvidence->GetObsNodesWithValues(&pObsNodes, &pObsValues,
                                &pNodeTypes);
                        
                            int location;
                            for ( k = 0; k < discParents.size(); k++)
                            {
                                location = 
                                    std::find(pObsNodes.begin(), pObsNodes.end(), 
                                    discParents[k]) - pObsNodes.begin();
                                parentComb[k] = pObsValues[location]->GetInt();
                            }
                        
                            pTmpPot = static_cast<const CSoftMaxCPD*>(*assFactsIt)->
                                ConvertWithEvidenceToGaussianPotential( pEvidence, 
                                MeanContParents, CovContParents, parentComb );

                            for (k = 0; k < pBNet->GetNumberOfNodes(); k++)
                            {
                                if (pBNet->GetFactor(k)->GetDistributionType() == dtSoftMax)
                                {
                                    pBNet->GetModelDomain()->ChangeNodeType(k, 1);
                                }
                            }

//                            pTmpPot->Dump();
                        
                            delete [] parentComb;
                        }
                    }
                    else 
                        if (allCountObs)
                        {
                            pTmpPot = ((CSoftMaxCPD*)(*assFactsIt))->
                                ConvertWithEvidenceToTabularPotential( pEvidence);
 //                           pTmpPot->Dump();
                        }
                }
                else
                {
                    intVector Parents;
                    pBNet->GetGraph()->GetParents(FactorNode, &Parents);

                    if (allDiscrObs)
                    {
                        for ( k = 0; k < Parents.size(); k++)
                        {
                            if (pBNet->GetFactor(Parents[k])->GetDistributionType() == dtSoftMax)
                            {
                                pBNet->GetModelDomain()->ChangeNodeType(Parents[k], 0);
                            }
                        }
                    }    

                    pTmpPot = (*assFactsIt)->ConvertWithEvidenceToPotential( pEvidence,
                            sumOnMixtureNode );

 //                   pTmpPot->Dump();
                        
                }  
                
                *pPot *= *pTmpPot;

//                pPot->Dump();              

                delete pTmpPot;
        }
        
        *( m_nodePots.begin() + i ) = pPot;

        nodeTypes.clear();
        
        assignedFactors.clear();
    }
}
//////////////////////////////////////////////////////////////////////////

#ifdef PAR_OMP
void CJunctionTree::InitNodePotsFromBNetOMP( const CBNet* pBNet,
    const CEvidence* pEvidence, int sumOnMixtureNode )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pBNet);
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    // bad-args check end

    // cannot initialize, if the initialization have been carried out already
    if(m_bChargeInitialized)
    {
        PNL_THROW(CInvalidOperation,
            " cannot reinitialize pots, run Clear() first ");
    }

    intVector         obsNdsNums;
    pConstValueVector obsNdsVals;

    pEvidence->GetObsNodesWithValues(&obsNdsNums, &obsNdsVals);

    // number of potentials is equal to the number of nodes in jtree
    m_nodePots.resize(m_numberOfNodes);

    intVector::iterator factAssToClq_begin = m_factAssignToClq.begin(),
                        factAssToClq_end   = m_factAssignToClq.end();

#pragma omp parallel
{
    int i;
    int nProcs = omp_get_num_procs();
    int threadNum = omp_get_thread_num();
    int nNdsOfProc = m_numberOfNodes / nProcs;
    int leftBound = threadNum * nNdsOfProc;
    int rightBound = leftBound + nNdsOfProc;
    if (threadNum == nProcs - 1)
    {
        rightBound = m_numberOfNodes;
    }

    intVecVector::const_iterator ndContIt = m_nodeContents.begin() + leftBound;

    for(i = leftBound; i < rightBound; ++i, ++ndContIt)
    {
        //pConstNodeTypeVector nodeTypes;
        pnlVector<const CNodeType*> nodeTypes;
        //pConstCPDVector assignedFactors;
        pnlVector<const CCPD*> assignedFactors;


        // finding out if the unit function is to be sparse or not
        bool bDenseUnitFunction = true;

        intVector::iterator potsAssIt = std::find(factAssToClq_begin, 
            factAssToClq_end, i);

        //intVector summarizeNodes;
        pnlVector<int> summarizeNodes;

        while(potsAssIt != factAssToClq_end)
        {
            CCPD* pAssCPD = static_cast<CCPD*>(pBNet
                ->GetFactor(potsAssIt - factAssToClq_begin));

            assignedFactors.push_back(pAssCPD);

            if(pAssCPD->IsSparse())
            {
                bDenseUnitFunction = false;
            }
            if(sumOnMixtureNode)
            {
                if(pAssCPD->GetDistributionType() == dtMixGaussian)
                {
                    summarizeNodes.push_back(static_cast<CMixtureGaussianCPD*>
                        (pAssCPD)->GetNumberOfMixtureNode());            
                }
            }

            potsAssIt = std::find(++potsAssIt, factAssToClq_end, i);
        }

        intVector::const_iterator clqIt   = ndContIt->begin(),
            clq_end = ndContIt->end();
        //intVector obsPositionsInDom;
        pnlVector<int> obsPositionsInDom;

        for(; clqIt != clq_end; ++clqIt)
        {
            if( std::find( summarizeNodes.begin(), summarizeNodes.end(),
                *clqIt ) != summarizeNodes.end() )
            {
                nodeTypes.push_back(CInfEngine::GetObsTabNodeType());
                obsPositionsInDom.push_back( clqIt - ndContIt->begin() );
            }
            else if(IsNodeObserved( *clqIt, obsNdsNums ))
            {
                pBNet->GetNodeType(*clqIt)->IsDiscrete()?
                    nodeTypes.push_back(CInfEngine::GetObsTabNodeType())
                    : nodeTypes.push_back(CInfEngine::GetObsGauNodeType());
                obsPositionsInDom.push_back( clqIt - ndContIt->begin() );
            }
            else
            {
                nodeTypes.push_back(pBNet->GetNodeType(*clqIt));
            }
        }
        //find observed posittions in this domain
        
        
        // creating the unit function itself
        CPotential *pPot = NULL;
        const CNodeType* nT = &m_nodeTypes[m_nodeAssociation[i]];
        if((nT->IsDiscrete()) && (nT->GetNodeSize() == 0))
        {
            pPot = CScalarPotential::Create( &ndContIt->front(),
                ndContIt->size(), m_pMD, obsPositionsInDom );
        }
        else
        {
            if(nT->IsDiscrete())
            {
                pPot = CTabularPotential::CreateUnitFunctionDistribution(
                    &ndContIt->front(), ndContIt->size(), m_pMD, bDenseUnitFunction, 
                    obsPositionsInDom);
            }
            else
            {
                pPot = CGaussianPotential::CreateUnitFunctionDistribution(
                    &ndContIt->front(), ndContIt->size(), m_pMD, 1,
                    obsPositionsInDom);
            }
        }

        // multiplying unit function by all the BNet's CPDs attached to it
        pConstCPDVector::const_iterator
            assFactsIt   = assignedFactors.begin(),
            assFacts_end = assignedFactors.end();

        for (; assFactsIt != assFacts_end; ++assFactsIt)
        {
            const CPotential *pTmpPot;
            
            pTmpPot = (*assFactsIt)->ConvertWithEvidenceToPotential(pEvidence,
                sumOnMixtureNode);

            *pPot *= *pTmpPot;

            delete pTmpPot;
        }

        *(m_nodePots.begin() + i) = pPot;
    }
} // end of parallel section

}
#endif // PAR_OMP
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::InitNodePotsFromMNet( const CMNet *pMNet,
                                          const CEvidence *pEvidence )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pMNet);
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    // bad-args check end
    
    // cannot initialize, if the initialization have been carried out already
    if( m_bChargeInitialized )
    {
        PNL_THROW( CInvalidOperation,
            " cannot reinitialize pots, run Clear() first " );
    }


    intVector         obsNdsNums;
    pConstValueVector obsNdsVals;

    pEvidence->GetObsNodesWithValues( &obsNdsNums, &obsNdsVals );

    pConstNodeTypeVector nodeTypes;

    // number of potentials is equal to the number of nodes in jtree
    m_nodePots.resize(m_numberOfNodes);

    intVector::iterator factAssToClq_begin = m_factAssignToClq.begin(),
                        factAssToClq_end   = m_factAssignToClq.end();
    
    pConstPotVector assignedFactors;
    
    intVecVector::const_iterator ndContIt = m_nodeContents.begin();

    int i = 0;
    
    for( ; i < m_numberOfNodes; ++i, ++ndContIt )
    {
        // finding out if the unit function is to be sparse or not
        bool bDenseUnitFunction = true;

        intVector::iterator potsAssIt = std::find( factAssToClq_begin, 
            factAssToClq_end, i );

        while( potsAssIt != factAssToClq_end )
        {
            CPotential* pAssPot = static_cast<CPotential*>(pMNet
                ->GetFactor( potsAssIt - factAssToClq_begin ));

            assignedFactors.push_back(pAssPot);
            
            if( pAssPot->IsSparse() )
            {
                bDenseUnitFunction = false;
            }

            potsAssIt = std::find( ++potsAssIt,	factAssToClq_end, i );
        }

        intVector::const_iterator clqIt   = ndContIt->begin(),
                                  clq_end = ndContIt->end();
        intVector obsPositionsInDom;

        for( ; clqIt != clq_end; ++clqIt )
        {
            if( IsNodeObserved( *clqIt, obsNdsNums ) )
            {
                pMNet->GetNodeType(*clqIt)->IsDiscrete()?
                    nodeTypes.push_back(CInfEngine::GetObsTabNodeType())
                    : nodeTypes.push_back(CInfEngine::GetObsGauNodeType());
                obsPositionsInDom.push_back( clqIt - ndContIt->begin() );
            }
            else
            {
                nodeTypes.push_back(pMNet->GetNodeType(*clqIt));
            }
        }
        //find observed posittions in this domain
        
        // creating the unit function itself
        CPotential *pPot = NULL;

        const CNodeType nT = m_nodeTypes[m_nodeAssociation[i]];
        
        if( ( nT.IsDiscrete() ) && ( nT.GetNodeSize() == 0 ) )
        {
            pPot = CScalarPotential::Create( &ndContIt->front(),
                ndContIt->size(), m_pMD, obsPositionsInDom );
        }
        else
        {

            if( nT.IsDiscrete() )
            {
                pPot = CTabularPotential::CreateUnitFunctionDistribution(
                    &ndContIt->front(), ndContIt->size(), m_pMD,
                    bDenseUnitFunction, obsPositionsInDom);
            }
            else
            {
                pPot = CGaussianPotential::CreateUnitFunctionDistribution(
                    &ndContIt->front(), ndContIt->size(), m_pMD, 1,
                    obsPositionsInDom);
            }
        }

        // multiplying unit function by all the BNet's CPDs attached to it
        pConstPotVector::const_iterator
                                       assFactsIt   = assignedFactors.begin(),
                                       assFacts_end = assignedFactors.end();

        for( ; assFactsIt != assFacts_end; ++assFactsIt )
        {
            const CPotential *pTmpPot = (*assFactsIt)->ShrinkObservedNodes(
                pEvidence );

            *pPot *= *pTmpPot;

            delete pTmpPot;
        }
        
        *( m_nodePots.begin() + i ) = pPot;

        nodeTypes.clear();
        
        assignedFactors.clear();
    }
/*
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pMNet);
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    // bad-args check end
    
    // cannot initialize, if the initialization have been carried out already
    if( m_bChargeInitialized )
    {
        PNL_THROW( CInvalidOperation, " cannot reinitialize pots " );
    }
  
    intVector                         obsNdsNums;
    pnlVector< const unsigned char* > obsNdsVals;
    
    pEvidence->GetObsNodesWithValues( &obsNdsNums, &obsNdsVals );

    pConstNodeTypeVector nodeTypes;
 
    // number of potentials is equal to the number of nodes in jtree
    m_nodePots.resize(m_numberOfNodes);

    intVector::iterator factAssToClq_begin = m_factAssignToClq.begin(),
                        factAssToClq_end   = m_factAssignToClq.end();
    
    pConstCPDVector assignedFactors;    
    
    intVecVector::const_iterator ndContIt = m_nodeContents.begin();
    
    int i = 0;

    for( ; i < m_numberOfNodes; ++i, ++ndContIt )
    {
        intVector::const_iterator clqIt   = ndContIt->begin(),
                                  clq_end = ndContIt->end();

        // collecting CNodeTypes objects for all nodes in domain
        for( ; clqIt != clq_end; ++clqIt )
        {
            if( IsNodeObserved( *clqIt, obsNdsNums ) )
            {
                pMNet->GetNodeType(*clqIt)->IsDiscrete() ?
                    nodeTypes.push_back(CInfEngine::GetObsTabNodeType())
                    : nodeTypes.push_back(CInfEngine::GetObsGauNodeType());
            }
            else
            {
                nodeTypes.push_back(pMNet->GetNodeType(*clqIt));
            }
        }
        
        // finding out if the unit function is to be sparse or not
        bool bDenseUnitFunction = true;

        intVector::iterator potsAssIt = std::find( factAssToClq_begin, 
            factAssToClq_end, i );

        while( potsAssIt != factAssToClq_end )
        {
            assignedFactors.push_back(static_cast<CPotential *>(pMNet
                ->GetFactor( potsAssIt - factAssToClq_begin )));

            if( assignedFactors.back()->IsSparse() )
            {
                bDenseUnitFunction = false;
            }
            
            potsAssIt = std::find( ++potsAssIt,	factAssToClq_end, i );
        }

        // creating the unit function itself
        CPotential *pPot;

        if( GetNodeType(i)->IsDiscrete() )
        {
            pPot = CTabularPotential::CreateUnitFunctionDistribution(
                &nodeTypes.front(), &ndContIt->front(), ndContIt->size(),
                bDenseUnitFunction );
        }
        else
        {
            pPot = CGaussianPotential::CreateUnitFunctionDistribution(
                &nodeTypes.front(), &ndContIt->front(), ndContIt->size(), 1 );
        }
        
        // multiplying unit function by all the BNet's CPDs attached to it
        pConstCPDVector::const_iterator
                                       assFactsIt   = assignedFactors.begin(),
                                       assFacts_end = assignedFactors.end();
        
        for( ; assFactsIt != assFacts_end; ++assFactsIt )
        {
            const CPotential *pTmpPot;

            intVector cpdDom;

            (*assFactsIt)->GetDomain(&cpdDom);

            intVector::const_iterator cpdDomIt   = cpdDom.begin(),
                                      cpdDom_end = cpdDom.end();

            for( ; cpdDomIt != cpdDom_end; ++cpdDomIt )
            {
                if( IsNodeObserved( *cpdDomIt, obsNdsNums ) )
                {

                }
            }

            pTmpPot
            
            

            *pPot *= *pTmpPot;

            delete pTmpPot;
        }
        
        *( m_nodePots.begin() + i ) = pPot;

        nodeTypes.clear();

        assignedFactors.clear();
    }*/
}
//////////////////////////////////////////////////////////////////////////

void CJunctionTree::InitSeparatorsPots( const CStaticGraphicalModel
                                       *pGraphicalModel, const CEvidence
                                       *pEvidence )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pGraphicalModel);
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    // bad-args check end
    
    // cannot initialize, if the initialization have been carried out already
    if( m_bChargeInitialized )
    {
        PNL_THROW(CInvalidOperation,
            " cannot reinitialize pots, which've been initialized already ");
    }
    
    pConstNodeTypeVector nodeTypes;
    

    intVector         obsNodesNums;
    pConstValueVector obsNodesVals;
    
    pEvidence->GetObsNodesWithValues( &obsNodesNums, &obsNodesVals );
    
    m_separatorPots.resize(m_numberOfNodes);
    
    int i = 0;
    
    for( ; i < m_numberOfNodes; ++i )
    {
        bool bSparseUnitFunction = false;
        
        if( GetNodePotential(i)->IsSparse() )
        {
            bSparseUnitFunction = true;
        }
        
        m_separatorPots[i].assign( m_numberOfNodes, (CPotential *)0 );
        
        int                 numOfNbrs;
        const int           *nbrs;
        const ENeighborType *nbrsTypes;
        
        m_pGraph->GetNeighbors( i, &numOfNbrs, &nbrs, &nbrsTypes );
        
        const int *nbrsIt = nbrs;
        const int *nbrs_end = nbrs + numOfNbrs;
        
        for( ; nbrsIt != nbrs_end; ++nbrsIt )
        {
            if( !m_separatorPots[*nbrsIt].empty() )
            {
                if( m_separatorPots[*nbrsIt][i] )
                {
                    m_separatorPots[i][*nbrsIt] = m_separatorPots[*nbrsIt][i];
                }
            }
            else
            {
                int numNdsInSepDom;
                const int *sepDomain;
                
                GetSeparatorDomain( i, *nbrsIt, &numNdsInSepDom, &sepDomain );
                
                const int *sepDomIt = sepDomain;
                const int *sepDom_end = sepDomain + numNdsInSepDom;

                intVector obsPosInDom;
                
                for( ; sepDomIt != sepDom_end; ++sepDomIt )
                {
                    if( IsNodeObserved( *sepDomIt, obsNodesNums ) )
                    {
                        pGraphicalModel->GetNodeType(*sepDomIt)->IsDiscrete()?
                            nodeTypes.push_back( 
                            CInfEngine::GetObsTabNodeType() )
                            : nodeTypes.push_back(
                            CInfEngine::GetObsGauNodeType() );
                        obsPosInDom.push_back( sepDomIt - sepDomain );
                    }
                    else
                    {
                        nodeTypes.push_back(pGraphicalModel
                            ->GetNodeType(*sepDomIt));
                    }
                }
                
                if( GetNodePotential(*nbrsIt)->IsSparse() )
                {
                    bSparseUnitFunction = true;
                }

                //need to determine distribution type here
                EDistributionType dt = pnlDetermineDistributionType( m_pMD,
                    numNdsInSepDom, sepDomain, pEvidence );
                
                if( dt == dtScalar )
                {
                    m_separatorPots[i][*nbrsIt] = 
                        CScalarPotential::Create( sepDomain, numNdsInSepDom,
                        m_pMD,obsPosInDom);
                }
                else
                {
                    if( dt == dtTabular )
                    {
                        m_separatorPots[i][*nbrsIt] = 
                        CTabularPotential::CreateUnitFunctionDistribution(
                        sepDomain, numNdsInSepDom, m_pMD,!bSparseUnitFunction,
                        obsPosInDom);
                    }
                    else
                    {
                        if( dt == dtGaussian )
                        {
                            m_separatorPots[i][*nbrsIt] = 
                        CGaussianPotential::CreateUnitFunctionDistribution(
                        sepDomain, numNdsInSepDom, m_pMD, 1, obsPosInDom);
                        }
                        else
                        {
                            PNL_THROW( CInconsistentType,
                                "have only Tabular, Gaussian or Scalar potentials " );
                        }
                    }
                }                
                nodeTypes.clear();
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////

CJunctionTree& CJunctionTree::operator=(const CJunctionTree &pJTree)
{
    if( this != &pJTree )
    {
        if( IsMatchingTreeStructure(&pJTree) )
        {
            // if structures of these Junction trees perfectly match, then
            // there should be no memory allocation done, just copying each
            // potential's matrices elements from source to sink
            
            int i, j;
            
            for( i = 0; i < m_numberOfNodes; i++ )
            {
                *(m_nodePots[i]) = *(pJTree.m_nodePots[i]);
                
                for( j = i + 1; j < m_numberOfNodes; j++ )
                {
                    if( m_separatorPots[i][j] )
                    {
                        *(m_separatorPots[i][j]) = *(pJTree.
                            m_separatorPots[i][j]);
                    }
                }
            }
        }
        else
        {
            // the structures of the trees dont match, so will have to clear
            // the sink and make a copy of the source there */
            
            PNL_THROW( CNotImplemented, " will do soon " );
        }
    }

    return *this;
}

bool CJunctionTree::IsAllNodesInClsObs(int NumCls, const CEvidence *pEvidence )
{
    intVector         obsNdsNums;
    pConstValueVector obsNdsVals;

    pEvidence->GetObsNodesWithValues( &obsNdsNums, &obsNdsVals );

    int i;
    intVector::iterator location;
    for (i = 0; i < m_nodeContents[NumCls].size(); i++)
    {
        location = std::find(obsNdsNums.begin(), obsNdsNums.end(), m_nodeContents[NumCls][i]);
        if (location == obsNdsNums.end())
        {
            return false;
        }
    }

    return true;
}

#ifdef PNL_RTTI
const CPNLType CJunctionTree::m_TypeInfo = CPNLType("CJunctionTree", &(CStaticGraphicalModel::m_TypeInfo));

#endif

//////////////////////////////////////////////////////////////////////////