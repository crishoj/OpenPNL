/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlInferenceEngine.hpp                                      //
//                                                                         //
//  Purpose:   CInferenceEngine class definition                           //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// InfEngine.hpp: interface for the CInfEngine class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __PNLINFERENCEENGINE_HPP__
#define __PNLINFERENCEENGINE_HPP__

#include "pnlObject.hpp"
#include "pnlPotential.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class CStaticGraphicalModel;

class CEvidence;

class CPotential;

typedef enum
{
    // itEx is not really single inference type but flag-qualifier that can be OR-ed with real inference type
    // this qualifier designates CExInfEngine layer atop real inference engine
    itEx = 1,

    itJtree = itEx << 1,
    itDummy,
    itPearl,
    itNaive,
    itGibbsSampling,
    itFGSumMaxProduct,
    itLWSampling,
} EInfTypes;

#ifdef SWIG
%rename(pnlDetermineDistributionTypeByMD) pnlDetermineDistributionType( const CModelDomain*,int, const int*, const CEvidence*);// bad name. rename!
#endif

PNL_API EDistributionType 
pnlDetermineDistributionType( int nAllNodes, int numberObsNodes,
                              int* pObsNodesIndices, 
                              const CNodeType* const* pAllNodesTypes );

PNL_API EDistributionType
pnlDetermineDistributionType( const CModelDomain* pMD, int nQueryNodes,
                              const int* query, const CEvidence* pEvidence);

class PNL_API CInfEngine : public CPNLBase
{
public:

    inline const CStaticGraphicalModel* GetModel() const;
    
    static const CNodeType*      GetObsGauNodeType();
    
    static const CNodeType*      GetObsTabNodeType();

    // We add evidence to the model - main calculations are here
    virtual void              EnterEvidence( const CEvidence *evidence,
                                             int maximize = 0, 
                                             int sumOnMixtureNode = 1 ) = 0;

#ifdef PNL_OBSOLETE
    virtual void              MarginalNodes( const int *query, int querySize,
                                             int notExpandJPD = 0 ) = 0;
#endif

    virtual void              MarginalNodes( const intVector& queryNdsIn,
                                             int notExpandJPD = 0 );

    
    virtual const CPotential* GetQueryJPD() const;

    virtual const CEvidence*  GetMPE() const;

    virtual                   ~CInfEngine();

    EInfTypes                    m_InfType;

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CInfEngine::m_TypeInfo;
  }
#endif

protected:

    CInfEngine( EInfTypes infType, const CStaticGraphicalModel *pGM );

    
    int                          m_bMaximize;

    const CStaticGraphicalModel* m_pGraphicalModel;
    
    const CEvidence*             m_pEvidence;

    CPotential*                  m_pQueryJPD;
    
    CEvidence*                   m_pEvidenceMPE;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif   
private:

    static CNodeType             m_ObsTabNodeType;
    
    static CNodeType             m_ObsGaussNodeType;
    
    /*as we keep only pointers to NodeTypes anythere 
    instead of GraphicalModel,we need to have NodeType(1,1) - 
    observed node type. We keep it here because it is nesessary to have
    it only for inference. We get the pointer to this node type in methods
    which use Evidence (ShrinkObservedNodes, for instance)*/
};

#ifndef SWIG

inline const CStaticGraphicalModel* CInfEngine::GetModel() const
{
    return m_pGraphicalModel;
}

#endif

PNL_END

#endif // __PNLINFERENCEENGINE_HPP__
