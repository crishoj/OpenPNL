/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlJtreeInferenceEngine.hpp                                 //
//                                                                         //
//  Purpose:   CJTreeInfEngine class definition                            //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLJTREEINFERENCEENGINE_HPP__
#define __PNLJTREEINFERENCEENGINE_HPP__

#include "pnlInferenceEngine.hpp"
#include "pnlJunctionTree.hpp"
#include "pnlEvidence.hpp"
#include "pnlDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"

PNL_BEGIN

#ifdef SWIG
%rename(CreateFromJTree) CJtreeInfEngine::Create( const CStaticGraphicalModel *, CJunctionTree *);
#endif

class PNL_API CJtreeInfEngine : public CInfEngine
{
public:

#ifdef PNL_OBSOLETE
    // static functions of Junction Tree Inf creation
    static CJtreeInfEngine* Create( const CStaticGraphicalModel 
                                    *pGraphicalModelIn,
                                    int numOfSubGrToConnect,
                                    const int *SubGrToConnectSizesIn,
                                    const int **SubgrToConnectIn );
#endif
    
    static CJtreeInfEngine* Create( const CStaticGraphicalModel 
                                    *pGraphicalModelIn,
                                    const intVecVector&
                                    SubgrToConnectIn = intVecVector() );
    

    static CJtreeInfEngine* Create( const CStaticGraphicalModel 
                                    *pGraphicalModelIn,
                                    CJunctionTree *pJTreeIn );


    //static void Release(CJtreeInfEngine** pJTreeInfEng);
    
    static CJtreeInfEngine* Copy(const CJtreeInfEngine *pJTreeInfEngIn);
    
    virtual ~CJtreeInfEngine();
    
    // inline functions, which allow read-only access to members and data
    // from internal Junction Tree
    inline const CEvidence* GetEvidence() const;
    
    inline void GetJTreeNodeContent(int nodeNum, intVector* ndsOut) const;
    inline int GetJTreeRootNode() const;
    
    
#ifdef PNL_OBSOLETE
    inline void GetClqNumsContainingSubset( int numOfNdsInSubset,
                                            const int *subsetIn,
                                            int *numOfClqsOut,
                                            const int **clqsContSubsetOut )
                                            const;
#endif
    
    inline void GetClqNumsContainingSubset( const intVector& subsetIn,
                                            intVector* clqsContSubsetOut )
                                            const;
    
#ifdef PNL_OBSOLETE
    inline void GetNodesConnectedByUser( int nodeSetNum, int *numOfNdsOut,
                                         const int **ndsOut ) const;
#endif
    
    inline void GetNodesConnectedByUser( int nodeSetNum,
                                         intVector* ndsOut ) const;
    
    // inline functions, which allow members changing
    inline virtual void SetJTreeRootNode(int nodeNum) const;
    virtual float GetLogLik() const;

#ifdef PNL_OBSOLETE    
    virtual void MultJTreeNodePotByDistribFun( int clqPotNum,
                                               const int *domainIn,
                                               const CDistribFun
                                               *pDistrFunIn );
    
    virtual void DivideJTreeNodePotByDistribFun( int clqPotNum,
                                                 const int *domainIn,
                                                 const CDistribFun 
                                                 *pDistrFunIn );
#endif

    // main public member-functions, which carry out the engine work
    virtual void EnterEvidence( const CEvidence *pEvidenceIn,
                                int maximize = 0, int sumOnMixtureNode = 1 );
    
    virtual void CollectEvidence();
    
    virtual void DistributeEvidence();
    
    virtual void ShrinkObserved( const CEvidence *pEvidenceIn,
                                 int maximize = 0, int sumOnMixtureNode = 1,
                                 bool bRebuildJTree = true );
    

#ifdef PNL_OBSOLETE    
    virtual void MarginalNodes( const int *queryIn, int querySz, int notExpandJPD = 0 );
#endif
    
    const CPotential* GetQueryJPD() const;
    
    const CPotential* GetQueryMPE() const;
        
    const CEvidence* GetMPE() const;
    
    inline void DumpClqs() const
#ifndef SWIG
    {
        m_pOriginalJTree->GetGraph()->Dump();
        m_pOriginalJTree->DumpNodeContents();
    }
#else
    ;
#endif
    

protected:
    
    CJtreeInfEngine( const CStaticGraphicalModel *pGraphicalModel, 
                     int numOfSubGrToConnect, const int *SubGrToConnectSizes, 
                     const int **SubgrToConnect );
    
    CJtreeInfEngine( const CStaticGraphicalModel *pGraphicalModel,
                     CJunctionTree *pJTree );
    
    CJtreeInfEngine(const CJtreeInfEngine& rJTreeInfEngine);

    
    virtual void PropagateBetweenClqs( int source, int sink, bool isCollect );

    inline void GetObservedDomains(const CEvidence *pEv, intVector* domains ) const;

    
    
private:
    
    typedef enum
    {
        opsNotStarted, opsShrinkEv, opsCollect, opsDistribute, opsMargNodes
    } EOperations;
    
    CPotential *m_pPotMPE;
    
    mutable int	         m_JTreeRootNode;
    mutable intVecVector m_collectSequence;
    
    intVector	         m_actuallyObsNodes;
    //const CEvidence      *m_pEvidence;
    
    CJunctionTree *m_pOriginalJTree;
    CJunctionTree *m_pJTree;
    
    EOperations          m_lastOpDone;
    
    void RebuildTreeFromRoot() const;
    
    void MarginalizeCliqueToQuery( int clqNum, int querySz,
                                   const int *query, int notExpandJPD = 0 );
    void MyNormalize(CPotential *pot, bool isCollect);

    float m_norm;
};

#ifndef SWIG

inline const CEvidence* CJtreeInfEngine::GetEvidence() const
{
    return m_pEvidence;
}

inline void CJtreeInfEngine::GetJTreeNodeContent(int nodeNum, intVector* ndsOut) const
{
    int numOfNds;
    const int* nodes;
    m_pOriginalJTree->GetNodeContent( nodeNum, &numOfNds, &nodes );
    ndsOut->assign( nodes, nodes + numOfNds );
}

inline int CJtreeInfEngine::GetJTreeRootNode() const
{
    return m_JTreeRootNode;
}

inline void 
CJtreeInfEngine::GetClqNumsContainingSubset( int numOfNdsInSubset, 
                                             const int *subset,
                                             int *numOfClqs, 
                                             const int **clqsContSubset )
                                             const
{
    m_pOriginalJTree->GetClqNumsContainingSubset( numOfNdsInSubset,
        subset, numOfClqs, clqsContSubset );
}

inline void
CJtreeInfEngine::GetClqNumsContainingSubset( const intVector& subset,
                                             intVector* clqsContSubset )
                                             const
{
    int numOfClqs;
    
    const int *clqs;
    
    GetClqNumsContainingSubset( subset.size(), &subset.front(),
        &numOfClqs, &clqs );
    
    clqsContSubset->assign( clqs, clqs + numOfClqs );
}

inline void 
CJtreeInfEngine::GetNodesConnectedByUser( int nodeSetNum, int *numOfNds,
                                          const int **nds ) const
{
    m_pOriginalJTree->GetNodesConnectedByUser( nodeSetNum, numOfNds, nds );
}

inline void CJtreeInfEngine::SetJTreeRootNode(int nodeNum) const
{
    /* bad-args check */
    PNL_CHECK_RANGES( nodeNum, 0, m_pOriginalJTree->GetNumberOfNodes() - 1 );
    /* bad-args check end */
    
    if( nodeNum != m_JTreeRootNode )
    {
        m_JTreeRootNode = nodeNum;
        RebuildTreeFromRoot();
    }
}


inline void 
CJtreeInfEngine::GetNodesConnectedByUser( int nodeSetNum,
                                          intVector* nds ) const
{
    int numOfNds;
    
    const int *pNds;
    
    GetNodesConnectedByUser( nodeSetNum, &numOfNds, &pNds );
    
    nds->assign( pNds, pNds + numOfNds );
}

#endif // SWIG

PNL_END

#endif //__PNLJTREEINFERENCEENGINE_HPP__
