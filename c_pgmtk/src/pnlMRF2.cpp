/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMRF2.cpp                                                 //
//                                                                         //
//  Purpose:   CMRF2 class member functions implementation                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlMRF2.hpp"

PNL_USING


CMRF2* CMRF2::Create( int numberOfCliques, const int *cliqueSizes,
                     const int **cliques, CModelDomain* pMD)
{
    /* bad-args check */
    PNL_CHECK_LEFT_BORDER( numberOfCliques, 1 );
    PNL_CHECK_IS_NULL_POINTER(cliqueSizes);
    PNL_CHECK_IS_NULL_POINTER(cliques);
    PNL_CHECK_IS_NULL_POINTER( pMD )
        /* bad-args check end */
        
        /* creating the model */
        CMRF2 *pMRF2 = new CMRF2( numberOfCliques, cliqueSizes, cliques, pMD );
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pMRF2);
    
    return pMRF2;
}

CMRF2* CMRF2::Create( const intVecVector& clqs, CModelDomain* pMD )
{
    int i;
    
    int numOfClqs = clqs.size();
    
    intVector clqsSizes(numOfClqs);
    
    pConstIntVector pClqs(numOfClqs);
    
    for( i = 0; i < numOfClqs; ++i )
    {
        clqsSizes[i] = clqs[i].size();
        pClqs[i] = &clqs[i].front();
    }
    
    return Create( numOfClqs, &clqsSizes.front(), &pClqs.front(), pMD );
}

CMRF2* CMRF2::Create( int numberOfNodes, int numberOfNodeTypes,
                     const CNodeType *nodeTypes, 
                     const int *nodeAssociation, int numberOfCliques, 
                     const int *cliqueSizes, const int **cliques )
{
    /* bad-args check */
    PNL_CHECK_LEFT_BORDER( numberOfNodes, 1 );
    PNL_CHECK_RANGES( numberOfNodeTypes, 1, numberOfNodes );
    PNL_CHECK_IS_NULL_POINTER(nodeTypes);
    PNL_CHECK_IS_NULL_POINTER(nodeAssociation);
    PNL_CHECK_LEFT_BORDER( numberOfCliques, 1 );
    PNL_CHECK_IS_NULL_POINTER(cliqueSizes);
    PNL_CHECK_IS_NULL_POINTER(cliques);
    /* bad-args check end */
    
    /* creating the model */
    CMRF2 *pMRF2 = new CMRF2( numberOfNodes, numberOfNodeTypes,
        nodeTypes, nodeAssociation, numberOfCliques, cliqueSizes, cliques );
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pMRF2);
    
    return pMRF2;
}

CMRF2* CMRF2::Create( int numberOfNodes, const nodeTypeVector& nodeTypes, 
                     const intVector& nodeAssociation,
                     const intVecVector& clqs )
{
    int i;
    
    int numOfClqs = clqs.size();
    
    intVector clqsSizes(numOfClqs);
    
    pnlVector<const int *> pClqs(numOfClqs);
    
    
    for( i = 0; i < numOfClqs; ++i )
    {
        clqsSizes[i] = clqs[i].size();
        
        pClqs[i] = &clqs[i].front();
    }
    
    return Create( numberOfNodes, nodeTypes.size(), &nodeTypes.front(),
        &nodeAssociation.front(), numOfClqs, &clqsSizes.front(),
        &pClqs.front() );
}

CMRF2* CMRF2::CreateWithRandomMatrices( const intVecVector& clqs,
                                       CModelDomain* pMD)
{
    CMRF2* pMRF2 = CMRF2::Create( clqs, pMD );
    
    pMRF2->AllocFactors();
    int numFactors = pMRF2->GetNumberOfFactors();
    int i;
    for( i = 0; i < numFactors; i++ )
    {
        pMRF2->AllocFactor( i );
        CFactor* ft = pMRF2->GetFactor(i);
        ft->CreateAllNecessaryMatrices();
    }
    
    return pMRF2;
}
//create mnet with random matrices
CMRF2* CMRF2::CreateWithRandomMatrices( int numberOfCliques,
                                       const int *cliqueSizes,
                                       const int **cliques,
                                       CModelDomain* pMD)
{
    CMRF2* pMRF2 = CMRF2::Create( numberOfCliques, cliqueSizes, cliques, pMD );
    
    pMRF2->AllocFactors();
    int numFactors = pMRF2->GetNumberOfFactors();
    int i;
    for( i = 0; i < numFactors; i++ )
    {
        pMRF2->AllocFactor( i );
        CFactor* ft = pMRF2->GetFactor(i);
        ft->CreateAllNecessaryMatrices();
    }
    
    return pMRF2;
}


CMRF2::CMRF2( int numberOfCliques, const int *cliqueSizes,
             const int **cliques, CModelDomain* pMD )
             :CMNet( numberOfCliques, cliqueSizes, cliques, pMD )
{
    int i;
    
    /* clique validity check */
    for( i = 0; i < numberOfCliques; i++ )
    {
        if( cliqueSizes[i] != 2 )
        {
            PNL_THROW( CInconsistentType,
                " not all the cliques are of two nodes " );
        }
    }
    /* clique validity check */
    
    m_modelType = mtMRF2;
}

CMRF2::CMRF2( int numberOfNodes, int numberOfNodeTypes,
             const CNodeType *nodeTypes, const int *nodeAssociation,
             int numberOfCliques, const int *cliqueSizes,
             const int **cliques )
             :CMNet( numberOfNodes, numberOfNodeTypes, nodeTypes,
             nodeAssociation, numberOfCliques, cliqueSizes, cliques )
{
    int i;
    
    /* clique validity check */
    for( i = 0; i < numberOfCliques; i++ )
    {
        if( cliqueSizes[i] != 2 )
        {
            PNL_THROW( CInconsistentType,
                " not all the cliques are of two nodes " );
        }
    }
    /* clique validity check */
    
    m_modelType = mtMRF2;
}

CMRF2::CMRF2(const CMRF2& rMRF2):CMNet(rMRF2)
{
    m_modelType = mtMRF2;
}

int CMRF2::GetFactors( int numberOfNodes, const int *nodes,
                      pFactorVector *params ) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( numberOfNodes, 1, 2 );
    PNL_CHECK_IS_NULL_POINTER(nodes);
    PNL_CHECK_IS_NULL_POINTER(params);
    /* bad-args check end */
    
    params->clear();
    
    int       numOfClqsFrstNode;
    const int *clqsFrstNode;
    
    GetClqsNumsForNode( *nodes, &numOfClqsFrstNode, &clqsFrstNode );
    
    if( numberOfNodes == 1 )
    {
        const int *clqsIt = clqsFrstNode,
            *clqs_end = clqsFrstNode + numOfClqsFrstNode;
        
        for( ; clqs_end - clqsIt; ++clqsIt )
        {
            params->push_back(GetFactor(*clqsIt));
        }
    }
    else
    {
        int       numOfClqsScndNode;
        const int *clqsScndNode;
        
        GetClqsNumsForNode( *(nodes + 1), &numOfClqsScndNode, &clqsScndNode );
        
        const int *pClqNum = std::find_first_of( clqsFrstNode,
            clqsFrstNode + numOfClqsFrstNode, clqsScndNode,
            clqsScndNode + numOfClqsScndNode );
        
        if( pClqNum == clqsFrstNode + numOfClqsFrstNode )
        {
            return 0;
        }
        
        params->push_back(GetFactor(*pClqNum));
    }
    
    assert( params->size() != 0 );
    
    return 1;
}

#ifdef PNL_RTTI
const CPNLType CMRF2::m_TypeInfo = CPNLType("CMRF2", &(CMNet::m_TypeInfo));

#endif