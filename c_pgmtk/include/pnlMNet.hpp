/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMNet.hpp                                                 //
//                                                                         //
//  Purpose:   CMNet class definition                                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLMNET_HPP__
#define __PNLMNET_HPP__

#include "pnlBNet.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

#ifdef SWIG
%rename(CreateByCliques)            CMNet::Create(int,const nodeTypeVector&,const intVector&,const intVecVector);
%rename(CreateByModelDomain)          CMNet::Create(const intVecVector&, CModelDomain*);
%rename(ConvertFromBNetUsingEv) CMNet::ConvertFromBNetUsingEvidence( const CBNet *, const CEvidence * );
%rename(AllocFactorByDomainNumber) CMNet::AllocFactor(int);
#endif

/* a class to represent Markov Network structure and operations */
class PNL_API CMNet : public CStaticGraphicalModel
{
public:

    /* static member-functions of MNet creation */

#ifdef PNL_OBSOLETE
    /* creates using the user-specified maximal cliques */  
    static CMNet* Create( int numberOfCliques, const int *cliqueSizesIn,
                          const int **cliquesIn, CModelDomain* pMD );
#endif

    static CMNet* Create( const intVecVector& clqsIn, CModelDomain* pMD );

#ifdef PNL_OBSOLETE
    /* creates using the user-specified maximal cliques */  
    static CMNet* Create( int numberOfNodes, int numberOfNodeTypes,
                          const CNodeType *nodeTypesIn,
                          const int *nodeAssociationIn, int numberOfCliques,
                          const int *cliqueSizesIn, const int **cliquesIn );
#endif

    static CMNet* Create( int numberOfNodes,
                          const nodeTypeVector& nodeTypesIn,
                          const intVector& nodeAssociationIn,
                          const intVecVector& clqsIn );
#ifdef PNL_OBSOLETE
    static CMNet* CreateWithRandomMatrices( int numberOfCliques,
                                            const int *cliqueSizesIn,
                                            const int **cliquesIn,
                                            CModelDomain* pMD);
#endif

    static CMNet* CreateWithRandomMatrices( const intVecVector& clqsIn,
                                            CModelDomain* pMD);
    //create mnet with random matrices
    //every gaussian distribution have matrix unit as covariance

    /* creates converting the input BNet */
    static CMNet* ConvertFromBNet(const CBNet *pBNetIn);

    /* creates converting the input BNet */
    static CMNet* ConvertFromBNetUsingEvidence( const CBNet *pBNetIn,
                                                const CEvidence *pEvidenceIn );

    /* creates a copy of the input MNet */
    static CMNet* Copy(const CMNet *pMNetIn);

    /* returns the number of cliques */
    inline int GetNumberOfCliques() const;

#ifdef PNL_OBSOLETE 
    /* returns a pointer to a clique in the set of cliques */
    inline void GetClique( int clqNum, int *clqSizeOut,
                           const int **clqOut ) const;
#endif

    inline void GetClique( int clqNum, intVector *clqOut ) const;

    /* allocates memory for the factors storage in the model */
    inline void AllocFactors();

#ifdef PNL_OBSOLETE
    /* allocates one specific factor in the set of factors */
    void AllocFactor( int numberOfNodesInDomain, const int *domainIn );
#endif  
    /* allocates one specific factor in the set of factors */
    void AllocFactor(int cliqueNumber);
    
    /* attaches one specific factor to the model */
    void AttachFactor(CFactor *paramIn);

    /* allocate factor and create matrix */
    void CreateTabularPotential( const intVector& domainIn,
        const floatVector& dataIn );

#ifdef PNL_OBSOLETE
    /* returns all the factors, which have "nodes" as a subdomain */
    virtual void GetFactors( int numberOfNodes, const int *nodesIn,
                             int *numberOfFactorsOut,
                             CFactor ***paramsOut ) const;
#endif

#ifdef PNL_OBSOLETE
    /* returns all the factors, which have "nodes" as a subdomain */
    virtual int GetFactors( int numberOfNodes, const int *nodesIn,
                            pFactorVector *paramsOut ) const;
#endif
    
   // void GenerateSamples( pEvidencesVector* evidencesOut, int nSamples) const;
    void GenerateSamples( pEvidencesVector* evidencesOut, int nSamples, const CEvidence *pEvIn = NULL ) const;

    virtual float ComputeLogLik( const CEvidence *pEv ) const;
    
    inline void GetClqsNumsForNode( int nodeNum, intVector *clqsNumsOut ) const;

    bool IsValid(std::string* descriptionOut = NULL) const; 

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CMNet::m_TypeInfo;
  }
#endif
protected:

    /* constructor, which uses set of cliques for the model construction */
    CMNet( int numberOfCliques, const int *cliqueSizes, const int **cliques,
                CModelDomain* pMD);

    /* constructor, which uses set of cliques and node types */
    CMNet( int numberOfNodes, int numberOfNodeTypes,
           const CNodeType *nodeTypes, const int *nodeAssociation,
           int numberOfCliques, const int *cliqueSizes, const int **cliques );

    /* converts BNet to MNet by moralization */
    CMNet(const CBNet *pBNet);

    /* converts BNet to MNet by moralization using Evidence */
    CMNet( const CBNet *pBNet, const CEvidence *pEvidence );

    /* constructs mnet as a copy of the input MNet  */
    CMNet(const CMNet& rMNet);

    /* structure of the cliques storage */
    intVecVector m_cliques;

    /* returns a pointer to an array of clqs numbers for each node */
    inline void GetClqsNumsForNode( int nodeNum, int *numOfClqs,
                                    const int **clqsNums ) const;

    /* returns number of the clique, which has all the nodes from "domain" */
    int FindCliqueNumberByDomain( int numberOfNodesInDomain,
                                  const int *domain ) const;

    /* constructs graph from m_cliques and stores in m_pGraph */
    void ConstructGraphFromInternalCliques();

    /* constructs cliques from m_pGraph and stores in m_cliques */
    void ConstructCliquesFromInternalGraph();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif   
private:

    intVecVector m_clqsNumsForNds;

    inline void FillInClqsNumsForNdsFromInternalCliques();

    void ConstructClqsFromBNetsFamiliesAndInternalGraph(const CBNet *pBNet);

};

#ifndef SWIG

inline int CMNet::GetNumberOfCliques() const
{
    return m_cliques.size();
}

inline void CMNet::GetClique( int cliqueNumber, int *cliqueSize,
                              const int **clique ) const
{
    /* bad-args check */
    if( ( cliqueNumber < 0 ) || ( cliqueNumber >= m_cliques.size() ) )
    {
        PNL_THROW( COutOfRange,
            " cliqueNumber should be between 0 and (numberOfCliques - 1) " );
    }

    if( !cliqueSize )
    {
        PNL_THROW( CNULLPointer, " cliqueSize cant be a NULL pointer " );
    }

    if( !clique )
    {
        PNL_THROW( CNULLPointer, " clique cant be a NULL pointer " );
    }
    /* bad-args check end */

    /* returns the pointer to the begining of the clique array */
    *cliqueSize = m_cliques[cliqueNumber].size();
    *clique = &m_cliques[cliqueNumber].front();
}

inline void CMNet::GetClique( int clqNum, intVector *clq ) const
{
    PNL_CHECK_IS_NULL_POINTER(clq);

    clq->assign( m_cliques[clqNum].begin(), m_cliques[clqNum].end() );
}

inline void CMNet::AllocFactors()
{
    if( m_pParams )
    {
        /* cannot allocate factors, if there is one set already attached */
        PNL_THROW( CInconsistentState, 
            " there are factors already attached " );
    }

    /* need to allocate enough memory for the factors storing
    and for storing the factor indexing vector */
    m_pParams = CFactors::Create(m_cliques.size());
    m_paramInds = intVector(m_cliques.size());
}

inline void CMNet::GetClqsNumsForNode( int nodeNum, int *numOfClqs,
                                       const int **clqsNums ) const
{
    // bad-args check
    int numVars = m_pMD->GetNumberVariables();
    PNL_CHECK_RANGES( nodeNum, 0, numVars - 1 );
    PNL_CHECK_IS_NULL_POINTER(numOfClqs);
    PNL_CHECK_IS_NULL_POINTER(clqsNums);
    // bad-args check end

    *numOfClqs = m_clqsNumsForNds[nodeNum].size();
    *clqsNums  = &m_clqsNumsForNds[nodeNum].front();
}

inline void CMNet::GetClqsNumsForNode( int nodeNum, 
                                      intVector *clqsNums ) const
{
    PNL_CHECK_RANGES( nodeNum, 0, m_numberOfNodes - 1 );
    PNL_CHECK_IS_NULL_POINTER(clqsNums);
    clqsNums->assign( m_clqsNumsForNds[nodeNum].begin(), 
        m_clqsNumsForNds[nodeNum].end() );
    
}

inline void CMNet::FillInClqsNumsForNdsFromInternalCliques()
{
    assert( m_cliques.size() != 0 );
    assert( m_clqsNumsForNds.size() == m_pMD->GetNumberVariables() );

    intVecVector::const_iterator clqsIt   = m_cliques.begin(),
                                 clqs_end = m_cliques.end();

    intVector::const_iterator    clqIt, clq_end;

    int clqNum = 0;

    for( ; clqs_end - clqsIt; ++clqsIt, ++clqNum )
    {
        for( clqIt = clqsIt->begin(), clq_end = clqsIt->end();
            clq_end - clqIt; ++clqIt )
        {
            m_clqsNumsForNds[*clqIt].push_back(clqNum);
        }
    }
}

#endif

PNL_END

#endif //__PNLMNET_HPP__
