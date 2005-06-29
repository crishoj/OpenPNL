/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlDiagnostics.hpp                                          //
//                                                                         //
//  Purpose:   CDiagnostics class definition                               //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Tarasov, Vinogradov                                //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLDIAGNOSTICS_HPP__
#define __PNLDIAGNOSTICS_HPP__

#include "pnlBNet.hpp"
#include "pnlEvidence.hpp"
#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

class PNL_API CDiagnostics : public CPNLBase
{
public:
    static CDiagnostics *Create(const CBNet *pBNet);
    void SetTargetNodes(intVector &targetNodes);
    void SetObservationNodes(intVector &observationNodes);
    void SetTargetState(int node, int state);
    void SetTargetStates(int node, intVector &listStates);
    void GetTestsList(const intVector &pursuedHypNodes,
        const intVector &pursuedHypNodesState, intVector &listTestNodes,
		doubleVector &listVOI);
    void SetCost(int node, float nodeCost);
    void SetCostRatio(float costR);
    void SetAlgorithm(int algNumber);
    virtual ~CDiagnostics();
    double GetEntropyCostRatio();
    
/*-------- for GeNIe support -------*/

//    void EditEvidence(int node, int Value);
//    void ClearEvidence(int node);
//    void SetEvidence(CEvidence *pEvid);
//    float GetCost(int);
//    int IsTarget(int);
//    CNodeType GetDiagType(int);

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CDiagnostics::m_TypeInfo;
  }
#endif

protected:
#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 

private:

    CDiagnostics(const CBNet *pBNet);

    pnlVector<char> aFlag; // 1-target state; 2-observation node
    floatVector m_observCost;
    const CBNet* m_BNet;
    CEvidence *m_evid;
    int algorithmNumber;// 0 - Marginal Strength 1; 1 - Marginal Strength 2;
    float costRatio;
    pnlVector<pair<int,int> > targetStates;

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
};

PNL_END

#endif //__PNLDIAGNOSTICS_HPP__
