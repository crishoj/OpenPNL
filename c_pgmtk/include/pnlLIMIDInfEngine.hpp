/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLIMIDInferenceEngine.hpp                                 //
//                                                                         //
//  Purpose:   CLIMIDInfEngine class definition                            //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLLIMIDINFERENCEENGINE_HPP__
#define __PNLLIMIDINFERENCEENGINE_HPP__

#include "pnlObject.hpp"
#include "pnlJunctionTree.hpp"
#include "pnlIDNet.hpp"
#include "pnlIDTabularPotential.hpp"
#include "pnlTypeDefs.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CLIMIDInfEngine : public CPNLBase
{
public:
  // create engine
  static CLIMIDInfEngine* Create(const CStaticGraphicalModel *pGM);

  // destroy engine
  static void Release(CLIMIDInfEngine** pInfEng);

  // Set max amount of iterations
  void SetIterMax(int IterMax);

  // main method of this engine
  void DoInference();

  // Get all politics
  pFactorVector* GetPolitics(void) const;

  // Get final expectation
  float GetExpectation();

  // Get number of iterations
  int GetIterNum();

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CLIMIDInfEngine::m_TypeInfo;
  }
#endif
protected:

  CLIMIDInfEngine(const CStaticGraphicalModel *pOldGM, 
    const CStaticGraphicalModel *pNewGM, intVector& vec_association);

  int GetClqNumContainingSubset(int numOfNdsInSubset, const int *subset)const;

  ~CLIMIDInfEngine();

  // initialzation
  void InitEngine();

  // create ID that contains utility nodes as last
  static CIDNet* CreateIDWithLastUtilityNodes(
    const CStaticGraphicalModel *pOldGM, intVector& vec_association);

  // Retract step of main algorithm
  void Retract();

  // Collect step of main algorithm
  void Collect();

  // Marginalazation step of main algorithm
  void Marginalization();

  // Contraction step of main algorithm
  void Contraction();

  // Optimization step of main algorithm
  void Optimization();

  // Calculate current expectation
  void CalculateExpectation();

  // Check if it's necessary to stop execution
  bool CheckStopCondition();

  // get clique's number, containing family of Node
  int GetCluqueContFamilyOfNode(int Node);

  // build all roots for JunctionTree
  void BuildAllRootsOfTree();

  // build path from Node1 to Node2
  void BuildPathBetweenTwoNodes(int Node1, int Node2, intVector &Path);

  // build paths from root to root
  void BuildPathsBetweenRoots();

  virtual void PropagateBetweenClqs( int source, int sink);

  inline int GetJTreeRootNode() const;

  inline void SetJTreeRootNode(int nodeNum);

  int GetMailBoxNumber(int clqFrom, int clqTo); // get mailBox number between cliques

  void GetCliquesNumbers(int mailBox, int &clqFrom, int &clqTo); // get cliques near mailBox

  void InitMailBoxes();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:
  int          m_JTreeRootNode;   // root of junction tree
  intVecVector m_collectSequence; // layers of junction tree

  pIDPotentialVector m_mailBoxes; // mail boxes vector
  intVector m_edgesDescription;   // mail boxes indexes: node i have neighbour 
                                  // m_edgesDescription[i], edge have mail box number i
  int m_IterNum;
  int m_IterMax;
  bool m_FirstIter;

  intVector m_Roots;
  intVecVector m_Paths;
  CJunctionTree *m_pJTree;

  const CIDNet *m_pIDNet;     // Reformed Influence Diagram net
  const CIDNet *m_pBaseIDNet; // Base Influence Diagram net

  intVector m_Association;

  pIDPotentialVector m_nodePots;
  pFactorVecVector m_DesNodePolitics;

  CIDPotential *m_RetractPotential;         // result of Retract step
  CIDPotential *m_CollectPotential;         // result of Collect step
  CIDPotential *m_pMargFromFamilyPotential; // result of Marginalization step
  CNumericDenseMatrix<float> *m_pMatContr;  // result of Contraction step

  intVecVector m_decisionNodesOfClqs;
  intVector m_decisionNodes;
  intVector m_cliquesContDecisionNodes;
  int m_currIndexOfDecisionNode;

  float m_exp; // current expectation
  float m_oldexp; // expectation from previous iteration

  void RebuildTreeFromRoot();

  void PrintCliques();
};
//----------------------------------------------------------------------

inline int CLIMIDInfEngine::GetJTreeRootNode() const
{
  return m_JTreeRootNode;
}
//----------------------------------------------------------------------

inline void CLIMIDInfEngine::SetJTreeRootNode(int nodeNum)
{
  PNL_CHECK_RANGES(nodeNum, 0, m_pJTree->GetNumberOfNodes() - 1);
  
  if (nodeNum != m_JTreeRootNode)
  {
    m_JTreeRootNode = nodeNum;
    RebuildTreeFromRoot();
  }
}
//----------------------------------------------------------------------

PNL_END

#endif // __PNLLIMIDINFERENCEENGINE_HPP__
