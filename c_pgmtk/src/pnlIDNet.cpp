/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlIDNet.cpp                                                //
//                                                                         //
//  Purpose:   CIDNet class definition                                     //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlIDNet.hpp"
#include <sstream>
#include <string>
// ----------------------------------------------------------------------------

PNL_USING

CIDNet* CIDNet::Create(int numberOfNodes, int numberOfNodeTypes,
  const CNodeType *nodeTypes, const int *nodeAssociation, CGraph *pGraph)
{
  PNL_CHECK_LEFT_BORDER(numberOfNodes, 1);
  PNL_CHECK_RANGES(numberOfNodeTypes, 1, numberOfNodes);
  PNL_CHECK_IS_NULL_POINTER(nodeTypes);

  PNL_CHECK_IS_NULL_POINTER(nodeAssociation);
  
  // creating the model
  CIDNet *pIDNet = new CIDNet(numberOfNodes, numberOfNodeTypes,
    nodeTypes, nodeAssociation, pGraph);
  
  PNL_CHECK_IF_MEMORY_ALLOCATED(pIDNet);
  
  // graph validity check
  if (!pIDNet->GetGraph()->IsDAG())
  {
    PNL_THROW( CInconsistentType, " the graph should be a DAG " );
  }
  
  if (!pIDNet->GetGraph()->IsTopologicallySorted())
  {
    PNL_THROW(CInconsistentType,
      " the graph should be sorted topologically ");
  }
  // graph validity check end
  
  return pIDNet;
}
// ----------------------------------------------------------------------------

CIDNet* CIDNet::Create(int numberOfNodes, const nodeTypeVector& nodeTypes,
  const intVector& nodeAssociation, CGraph *pGraph)
{
  return Create(numberOfNodes, nodeTypes.size(), &nodeTypes.front(),
    &nodeAssociation.front(), pGraph);
}
// ----------------------------------------------------------------------------

CIDNet* CIDNet::Create(CGraph *pGraph, CModelDomain* pMD)
{
  PNL_CHECK_IS_NULL_POINTER(pGraph);
  PNL_CHECK_IS_NULL_POINTER(pMD);
  CIDNet* pIDNet = new CIDNet(pGraph, pMD);
  
  PNL_CHECK_IF_MEMORY_ALLOCATED(pIDNet);
  
  // graph validity check
  if (!pIDNet->GetGraph()->IsDAG())
  {
    PNL_THROW(CInconsistentType, " the graph should be a DAG ");
  }
  
  if (!pIDNet->GetGraph()->IsTopologicallySorted())
  {
    PNL_THROW(CInconsistentType,
      " the graph should be sorted topologically ");
  }
  // graph validity check end
  
  return pIDNet;
}
// ----------------------------------------------------------------------------

CIDNet* CIDNet::Copy(const CIDNet* pIDNet)
{
  // bad-args check
  PNL_CHECK_IS_NULL_POINTER(pIDNet);
  // bad-args check end
  
  // creating the model
  CIDNet *pCopyIDNet = new CIDNet(*pIDNet);
  
  PNL_CHECK_IF_MEMORY_ALLOCATED(pCopyIDNet);
  
  return pCopyIDNet;
}
// ----------------------------------------------------------------------------

intVector* CIDNet::GetUnValueNodes() const
{
  int i;
  intVector* UnValueNodes = new intVector(0);
  UnValueNodes->resize(0);
  int NumOfNodes = GetGraph()->GetNumberOfNodes();
  for (i = 0; i < NumOfNodes; i++)
  {
    if (GetNodeType(i)->GetNodeState() != nsValue)
      UnValueNodes->push_back(i);
  }

  return UnValueNodes;
}
// ----------------------------------------------------------------------------

void CIDNet::GetChanceNodes(intVector& chanceNodesOut) const
{
  chanceNodesOut.resize(0);
  int nnodes = GetNumberOfNodes();
  int i;

  for (i = 0; i < nnodes; i++)
  {
    if (GetNodeType(i)->GetNodeState() == nsChance)
      chanceNodesOut.push_back(i);
  }
}
// ----------------------------------------------------------------------------

void CIDNet::GetDecisionNodes(intVector& decisionNodesOut) const
{
  decisionNodesOut.resize(0);
  int nnodes = GetNumberOfNodes();
  int i;

  for (i = 0; i < nnodes; i++)
  {
    if (GetNodeType(i)->GetNodeState() == nsDecision)
      decisionNodesOut.push_back(i);
  }
}
// ----------------------------------------------------------------------------

void CIDNet::GetValueNodes(intVector& valueNodesOut) const
{
  valueNodesOut.resize(0);
  int nnodes = GetNumberOfNodes();
  int i;

  for (i = 0; i < nnodes; i++)
  {
    if (GetNodeType(i)->GetNodeState() == nsValue)
      valueNodesOut.push_back(i);
  }
}
// ----------------------------------------------------------------------------

CIDNet::CIDNet(int numberOfNodes, int numberOfNodeTypes,
  const CNodeType *nodeTypes, const int *nodeAssociation, CGraph *pGraph):
  CBNet(numberOfNodes, numberOfNodeTypes, nodeTypes, nodeAssociation, pGraph)
{
  m_modelType = mtIDNet;
}
// ----------------------------------------------------------------------------

CIDNet::CIDNet(CGraph *pGraph, CModelDomain* pMD):CBNet(pGraph, pMD)
{
  m_modelType = mtIDNet;
}
// ----------------------------------------------------------------------------

CIDNet::CIDNet(const CIDNet& rIDNet):CBNet(CGraph::Copy(rIDNet.m_pGraph),
  rIDNet.GetModelDomain())
{
  m_modelType = mtIDNet;
}
// ----------------------------------------------------------------------------

bool CIDNet::IsValid(std::string* descriptionOut) const
{
  bool ret = 1;
  ret = ret && CBNet::IsValid(descriptionOut);
  
  int i;
  CGraph *pGraph = GetGraph();
  int nnodes = pGraph->GetNumberOfNodes();

  for (i = 0; i < nnodes; i++)
  {
    if ((GetNodeType(i)->GetNodeState() == nsValue) &&
         (pGraph->GetNumberOfChildren(i) > 0))
    {
      if (descriptionOut)
      {
        std::stringstream st;
        st<<"The Influence Diagram can't have utility nodes with nonzero list of childs."<<std::endl;
        std::string s = st.str();
        descriptionOut->insert(descriptionOut->begin(), s.begin(), s.end());
      }
      ret = 0;
      break;
    }
  }

  intVector DesNodes;
  GetDecisionNodes(DesNodes);

  if (DesNodes.size() == 0)
  {
      if (descriptionOut)
      {
          std::stringstream st;
          st<<"The Influence Diagram hasn't decision nodes."<<std::endl;
          std::string s = st.str();
          descriptionOut->insert(descriptionOut->begin(), s.begin(), s.end());
      }
      ret = 0;
  }

  intVector ValNodes;
  GetValueNodes(ValNodes);
  if (ValNodes.size() == 0)
  {
      if (descriptionOut)
      {
          std::stringstream st;
          st<<"The Influence Diagram hasn't value nodes."<<std::endl;
          std::string s = st.str();
          descriptionOut->insert(descriptionOut->begin(), s.begin(), s.end());
      }
      ret = 0;
  }

  intVector chanceNodes;
  GetChanceNodes(chanceNodes);
  if (chanceNodes.size() == 0)
  {
      if (descriptionOut)
      {
          std::stringstream st;
          st<<"The Influence Diagram hasn't chance nodes."<<std::endl;
          std::string s = st.str();
          descriptionOut->insert(descriptionOut->begin(), s.begin(), s.end());
      }
      ret = 0;
  }

  return ret;
}
// ----------------------------------------------------------------------------

bool CIDNet::IsLIMID(void) const
{
  bool ret = 1;
  int i, j;
  CGraph* pGraph = GetGraph();
  int nnodes = pGraph->GetNumberOfNodes();
  intVector childs(0);

  for (i = 0; (i < nnodes) && (ret); i++)
  {
    if (GetNodeType(i)->GetNodeState() == nsDecision)
    {
      pGraph->GetChildren(i, &childs);
      for (j = 0; (j < childs.size()) && (ret); j++)
        if (GetNodeType(childs[j])->GetNodeState() == nsDecision) ret = 0;
    }
  }

  return ret;
}
// ----------------------------------------------------------------------------

#ifdef PNL_RTTI
const CPNLType CIDNet::m_TypeInfo = CPNLType("CIDNet", &(CBNet::m_TypeInfo));

#endif

// end of file ----------------------------------------------------------------
