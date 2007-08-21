/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlBKInferenceEngine.hpp                                    //
//                                                                         //
//  Purpose:   Class definition for the BK inference lgorithm              //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLBKINFERENCEENGINE_HPP__
#define __PNLBKINFERENCEENGINE_HPP__


#include "pnl1_5SliceInferenceEngine.hpp"
#include "pnlJunctionTree.hpp"
#include "pnlJtreeInferenceEngine.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

#ifdef SWIG
%rename(CreateUsingClusters) CBKInfEngine::Create( const CDynamicGraphicalModel *, intVecVector&);
%rename(CheckClusters) CBKInfEngine::CheckClustersValidity( intVecVector&, intVector&  );
#endif

class PNL_API CBKInfEngine : public C1_5SliceInfEngine
{
public:
    virtual ~CBKInfEngine();
    
    static CBKInfEngine* 
        Create( const CDynamicGraphicalModel *pGrModelIn, bool isFF = true );
    
    static CBKInfEngine* 
        Create( const CDynamicGraphicalModel *pGrModelIn, intVecVector& clusters );
    
    static bool CheckClustersValidity( intVecVector& clusters, intVector& interfNds );
    
    
    //forward step at time = 0
    void ForwardFirst(const CEvidence *pEvidenceIn, int maximize = 0);
    //forward step fot time > 0
    void Forward(const CEvidence *pEvidenceIn, int maximize = 0);
    //first backward step after last forward step
    void BackwardT();
    void Backward( int maximize = 0 );
    //Backward step for fixed-lag smoothing procedure
    void BackwardFixLag();
    
    virtual void DefineProcedure(EProcedureTypes procedureType, 
        int lag = 0);

#ifndef SWIG
        virtual void MarginalNodes( const int *queryIn, int querySize, 
        int time = 0,  int notExpandJPD = 0 );

        
    virtual void EnterEvidence(const CEvidence* const* evidenceIn, 
        int nSlices);
#endif        
    virtual const CPotential* GetQueryJPD();
    virtual const CEvidence* GetMPE();

#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
      return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
      return CBKInfEngine::m_TypeInfo;
    }
#endif
protected:
    
    CBKInfEngine(const CDynamicGraphicalModel *pGraphicalModel, intVecVector clusters );
  
    void Get1_5Clusters(int nnodesPerSlice, intVector& interfNds, intVecVector& clusters, 
        intVecVector* clusters1_5Sl) const;
    void Initialization();
    void FindingMinimalClqs(CJtreeInfEngine *pInf, intVecVector &clusters, intVector *cliques);
    inline int GetNumVlsForIntNode(int node) const;

    
   
    inline const int* GetOutgoingNds( bool is1_5Slice) const;
    inline const int* GetIngoingNds() const;
    inline int GetNumOfClusters() const;
    void RemoveUnitClusters(intVecVector &clustersIn, intVecVector *clustersOut);
    void DestroyData();

    
    CEvidence* CreateCurrentEvidence(const CEvidence* pEvIn, 
        const CEvidence* pEvPast = NULL);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif     
   
private:
    
    intVecVector m_clustersPrSl;
    intVecVector m_clusters1_5Sl;
    intVector m_clqsPrSl;
    intVector m_clqs1_5Sl;
    
    CJtreeInfEngine *m_pPriorSliceJtreeInf;
    CJtreeInfEngine *m_p1_5SliceJtreeInf;
    
    
    CRing<CJtreeInfEngine *> m_CRingJtreeInf;
    CRing<CJtreeInfEngine *>::iterator m_JTreeInfIter;
    
    CRing< distrPVector > m_CRingDistrOnSep;
    CRing< distrPVector >::iterator m_CDistrOnSepIter;
    
    
    intVector m_nVlsForIntNds;
    
    CJunctionTree *m_pJTreeTmp;
    
    CJtreeInfEngine *m_QuerryJTree;
    int m_interfaceClique;
    
  
    bool m_isExact;
};


inline int CBKInfEngine::GetNumVlsForIntNode(int node) const
{
    PNL_CHECK_RANGES( node, 0, GetNumIntNds() );
    return m_nVlsForIntNds[node];
}



inline int CBKInfEngine::GetNumOfClusters() const
{
    return m_clustersPrSl.size();
}

inline const int* CBKInfEngine::GetOutgoingNds(bool is1_5Slice) const
{
    return is1_5Slice ?  &GetRootNdsISlice()->front() : 
    &GetIntNdsPriorSlice()->front();
}

inline const int* CBKInfEngine::GetIngoingNds() const
{
    return &GetIntNdsISlice()->front();
}



PNL_END

#endif //__PNLBKINFERENCEENGINE_HPP__
