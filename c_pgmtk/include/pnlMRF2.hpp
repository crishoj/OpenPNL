/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlMRF2.hpp                                                 //
//                                                                         //
//  Purpose:   CMRF2 class definition                                      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNLMRF2_HPP__
#define __PNLMRF2_HPP__

#include "pnlMNet.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

#ifdef SWIG
%rename(CreateByCliques) CMRF2::Create(int,const nodeTypeVector&,const intVector&,const intVecVector);
%rename(CreateByModelDomain) CMRF2::Create(const intVecVector&, CModelDomain*);
#endif

/* a class to represent pairwise Markov Random Field structure and operations */
class PNL_API CMRF2: public CMNet
{
public:

#ifdef PNL_OBSOLETE
    static CMRF2* Create( int numberOfCliques,  const int *cliqueSizesIn,
            const int **cliquesIn, CModelDomain* pMD );
#endif

    static CMRF2* Create( const intVecVector& clqsIn, CModelDomain* pMD );

#ifdef PNL_OBSOLETE
    static CMRF2* Create( int numberOfNodes, int numberOfNodeTypes,
                          const CNodeType *nodeTypesIn, 
                          const int *nodeAssociationIn, int numberOfCliques,
                          const int *cliqueSizesIn, const int **cliquesIn );
#endif

    static CMRF2* Create( int numberOfNodes,
                          const nodeTypeVector& nodeTypesIn,
                          const intVector& nodeAssociationIn,
                          const intVecVector& clqsIn );
#ifdef PNL_OBSOLETE
    static CMRF2* CreateWithRandomMatrices( int numberOfCliques,
                                            const int *cliqueSizesIn,
                                            const int **cliquesIn,
                                            CModelDomain* pMD);
#endif
    
    static CMRF2* CreateWithRandomMatrices( const intVecVector& clqsIn,
                                            CModelDomain* pMD);


#ifdef PNL_OBSOLETE
    virtual int GetFactors( int numberOfNodes, const int *nodesIn,
                            pFactorVector *paramsOut ) const;
#endif

    virtual ~CMRF2() {};

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CMRF2::m_TypeInfo;
  }
#endif
protected:

    CMRF2( int numberOfCliques, const int *cliqueSizes, const int **cliques,
                CModelDomain* pMD);

    CMRF2( int numberOfNodes, int numberOfNodeTypes,
           const CNodeType *nodeTypes, const int *nodeAssociation,
           int numberOfCliques, const int *cliqueSizes, const int **cliques );

    CMRF2(const CMRF2& rMRF2);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif   
private:

};

PNL_END

#endif// __PNLMRF2_HPP__
