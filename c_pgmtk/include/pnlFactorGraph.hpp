/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlFactorGraph.hpp                                          //
//                                                                         //
//  Purpose:   CFactorGraph class definition                               //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLFACTORGRAPH_HPP__
#define __PNLFACTORGRAPH_HPP__

#include "pnlTypeDefs.hpp"
#include "pnlStaticGraphicalModel.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class CBNet;
class CMNet;

#ifdef SWIG
%rename(CreateByFactors) CFactorGraph::Create(CModelDomain*, const CFactors*);
#endif

class PNL_API CFactorGraph : public CStaticGraphicalModel
{
public:
    static CFactorGraph* Create( CModelDomain* pMD, const CFactors* pFactors );
    static CFactorGraph* Create( CModelDomain* pMD, int numFactors );
    static CFactorGraph* Copy( const CFactorGraph* pFG );

    static CFactorGraph* ConvertFromBNet(const CBNet* pBNet);
    static CFactorGraph* ConvertFromMNet(const CMNet* pMNet);
    
    //allocate the memory for factors (if it isn't get before)
    virtual void AllocFactors() ;
    
#ifdef PNL_OBSOLETE
    /* the function to set the factor on the domain */
    virtual void AllocFactor( int numberOfNodesInDomain, const int *domainIn );
#endif
    
    /* the function to set the factor on the domain, the number input 
       factor is the number of the child node in BNet model and the number of
       the clique in the MNet model */
    virtual void AllocFactor(int number);
    
    /* functions to work with each factor separately to attach the input
       factor to the model and store it in m_pParams*/
    virtual void AttachFactor(CFactor *paramIn);
    
#ifdef PNL_OBSOLETE
    /* function returns all the factors which are set on the domains
       which contain the nodes as a subdomain */
    virtual void GetFactors( int numberOfNodes, const int* nodesIn,
        int *numberOfFactorsOut, CFactor ***paramsOut ) const;
#endif
    
#ifdef PNL_OBSOLETE
    /* function returns all the factors which are set on the domains
       which contain the nodes as a subdomain */
    virtual int GetFactors( int numberOfNodes, const int* nodesIn,
        pFactorVector *paramsOut ) const;
#endif
    bool IsValid(std::string* descriptionOut = NULL) const;
    //we can shrink factor graph - all its potentials shrinks with evidence
    CFactorGraph* Shrink( const CEvidence* pEvidence )const;
    inline int GetNumFactorsAllocated() const;
    inline void GetNbrFactors( int node, intVector* nbrsFactorsOut) const;
    inline int GetNumNbrFactors(int node) const;
    virtual ~CFactorGraph(){}

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CFactorGraph::m_TypeInfo;
  }
#endif
protected:
    int IsDomainFromMD( int numNodesInDomain, const int* domain) const;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:
    CFactorGraph( CModelDomain* pMD, int numFactors );
    CFactorGraph( CModelDomain* pMD, CFactors* pFactors );
    CFactorGraph( const CFactorGraph& pFG );
    intVecVector m_nbrFactors;
    int m_numFactorsAllocated;
    
};

#ifndef SWIG

inline int CFactorGraph::GetNumFactorsAllocated() const
{
    return m_numFactorsAllocated;
}

inline void CFactorGraph::GetNbrFactors(int node, intVector* nbrsFactors)const
{
    PNL_CHECK_IS_NULL_POINTER( nbrsFactors );
    PNL_CHECK_RANGES( node, 0, m_nbrFactors.size() - 1);
    nbrsFactors->assign(m_nbrFactors[node].begin(), m_nbrFactors[node].end());
}

inline int CFactorGraph::GetNumNbrFactors(int node)const
{
    PNL_CHECK_RANGES( node, 0, m_nbrFactors.size() - 1 );
    return m_nbrFactors[node].size();
}

#endif

PNL_END

#endif // __PNLFACTORGRAPH_HPP__
