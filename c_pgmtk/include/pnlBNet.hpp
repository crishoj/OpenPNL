/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlBNet.hpp                                                 //
//                                                                         //
//  Purpose:   CBNet class definition                                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLBNET_HPP__       
#define __PNLBNET_HPP__

#include "pnlStaticGraphicalModel.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

#ifdef SWIG
%rename(AllocFactorByDomainNumber) CBNet::AllocFactor(int);
%rename(CreateByModelDomain)          CBNet::Create(CGraph *, CModelDomain*);
#endif

/* a class to represent Bayes Network structure and operations */
class PNL_API CBNet : public CStaticGraphicalModel
{
public:

#ifdef PNL_OBSOLETE
    static CBNet* Create( int numberOfNodes, int numberOfNodeTypes, 
                          const CNodeType *nodeTypesIn, 
                          const int *nodeAssociationIn, CGraph *pGraphIn );
#endif

    static CBNet* Create( int numberOfNodes,
                          const nodeTypeVector& nodeTypesIn,
                          const intVector& nodeAssociationIn,
                          CGraph *pGraphIn );

    static CBNet* Create( CGraph *pGraphIn, CModelDomain* pMD );
    
    static CBNet* Copy(const CBNet* pBNetIn);

    static CBNet* CreateWithRandomMatrices(CGraph* pGraph, CModelDomain* pMD);
    //create bnet with random matrices
    //for conditional gaussian CPD the matrix of distributions is dense,
    //every gaussian distribution have matrix unit as covariance

    //convert BNet with dense matrices to BNet with sparse matrices
    CBNet* ConvertToSparse() const;

    //convert BNet with sparse matrices to BNet with dense matrices
    CBNet* ConvertToDense() const;

    // allocates enough memory for factors storage
    inline void AllocFactors();

    //create tabular CPD with matrix
    void CreateTabularCPD( int childNodeNumber, const floatVector& matrixDataIn );

    void FindMixtureNodes(intVector* mixureNds) const;

    void FindSummarizeNodes( intVector* sumNodes )const;

#ifdef PNL_OBSOLETE
    // allocates one factor in the set of factors
    void AllocFactor( int numberOfNodesInDomain, const int *domainIn );
#endif

    // allocates one factor in the set of factors
    void AllocFactor(int childNodeNumber);

    // attaches an input factor to the model
    void AttachFactor(CFactor *paramIn);

#ifdef PNL_OBSOLETE
    // returns the factors, which have "nodes" as a subdomain
    void GetFactors( int numberOfNodes, const int *nodesIn,
                     int *numberOfFactorsOut, CFactor ***paramsOut ) const;
#endif

#ifdef PNL_OBSOLETE
    // returns the factors, which have "nodes" as a subdomain
    int GetFactors( int numberOfNodes, const int *nodesIn,
                    pFactorVector *paramsOut ) const;
#endif
    void GenerateSamples( pEvidencesVector* evidencesOut, int nSamples,
        const CEvidence *evidIn = NULL) const;
    virtual float ComputeLogLik( const CEvidence *pEv ) const;

    bool IsValid(std::string* descriptionOut = NULL) const; 

    void GetContinuousParents(int nodeNum, intVector *contParents) const;
    void GetDiscreteParents(int nodeNum, intVector *discParents) const;

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CBNet::m_TypeInfo;
  }
#endif
protected:
    
    CBNet( CGraph *pGraph, CModelDomain* pMD );

    CBNet( int numberOfNodes, int numberOfNodeTypes,
           const CNodeType *nodeTypes, const int *nodeAssociation,
           CGraph *pGraph );

    CBNet( const CBNet& rBNet );

    int IfDomainExists( int numberOfNodesInDomain, const int *domain) const;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    
};

#ifndef SWIG

inline void CBNet::AllocFactors()
{
    if( m_pParams )
    {
        /* cannot allocate factors, if there is one set already attached */
        PNL_THROW( CInconsistentState,
            " there are params already attached " );
    }

    /* any change of the graph is prohibited after the factors are set */
    m_pGraph->ProhibitChange();

    /* need to allocate enough memory for the factors storing
    and for storing the factor indexing vector */
    int numNodes = m_pMD->GetNumberVariables();
    m_pParams = CFactors::Create(numNodes);
    m_paramInds = intVector(numNodes);
}

#endif

PNL_END

#endif // __PNLBNET_HPP__
