/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlIDNet.hpp                                                //
//                                                                         //
//  Purpose:   CIDNet class definition                                     //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLIDNET_HPP__
#define __PNLIDNET_HPP__

#include "pnlBNet.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

// a class to represent Influence Diagram Network structure and operations
class PNL_API CIDNet : public CBNet
{
public:
  
#ifdef PNL_OBSOLETE
  static CIDNet* Create(int numberOfNodes, int numberOfNodeTypes,
    const CNodeType *nodeTypesIn,  const int *nodeAssociationIn, 
    CGraph *pGraphIn);
#endif
  
  static CIDNet* Create(int numberOfNodes, const nodeTypeVector& nodeTypesIn,
    const intVector& nodeAssociationIn, CGraph *pGraphIn);
  
  static CIDNet* Create(CGraph *pGraphIn, CModelDomain* pMD);
  
  static CIDNet* Copy(const CIDNet* pIDNetIn);
  
  intVector* GetUnValueNodes() const;

  void GetChanceNodes(intVector& chanceNodesOut) const;

  void GetDecisionNodes(intVector& decisionNodesOut) const;

  void GetValueNodes(intVector& valueNodesOut) const;

  bool IsValid(std::string* descriptionOut = NULL) const;

  bool IsLIMID(void) const;
  
#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CIDNet::m_TypeInfo;
  }
#endif
protected:

  CIDNet(CGraph *pGraph, CModelDomain* pMD);
  
  CIDNet(int numberOfNodes, int numberOfNodeTypes, const CNodeType *nodeTypes,
    const int *nodeAssociation, CGraph *pGraph);
  
  CIDNet(const CIDNet& rIDNet);
  
#ifdef PNL_RTTI
  static const CPNLType m_TypeInfo;
#endif
private:
  
};

PNL_END

#endif // __PNLIDNET_HPP__
