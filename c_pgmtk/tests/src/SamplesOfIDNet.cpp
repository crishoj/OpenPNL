/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      SamplesOfIDNet.cpp                                          //
//                                                                         //
//  Purpose:   Influence Diagram net examples                              //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "SamplesOfIDNet.h"
#include <time.h>

PNL_USING

CIDNet* CreatePigsLIMID()
{
  const int nnodes = 14;
  const int numberOfNodeTypes = 14;
  
  int i;
  
  CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
  pGraph->AddNodes(nnodes);
  pGraph->AddEdge(0,1,1);
  pGraph->AddEdge(0,3,1);
  pGraph->AddEdge(1,2,1);
  pGraph->AddEdge(2,3,1);
  pGraph->AddEdge(3,4,1);
  pGraph->AddEdge(3,6,1);
  pGraph->AddEdge(4,5,1);
  pGraph->AddEdge(5,6,1);
  pGraph->AddEdge(6,7,1);
  pGraph->AddEdge(6,9,1);
  pGraph->AddEdge(7,8,1);
  pGraph->AddEdge(8,9,1);
  pGraph->AddEdge(2,10,1);
  pGraph->AddEdge(5,11,1);
  pGraph->AddEdge(8,12,1);
  pGraph->AddEdge(9,13,1);
  
  CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];
  
  nodeTypes[0].SetType(1, 2, nsChance);
  nodeTypes[1].SetType(1, 2, nsChance);
  nodeTypes[2].SetType(1, 2, nsDecision);
  nodeTypes[3].SetType(1, 2, nsChance);
  nodeTypes[4].SetType(1, 2, nsChance);
  nodeTypes[5].SetType(1, 2, nsDecision);
  nodeTypes[6].SetType(1, 2, nsChance);
  nodeTypes[7].SetType(1, 2, nsChance);
  nodeTypes[8].SetType(1, 2, nsDecision);
  nodeTypes[9].SetType(1, 2, nsChance);
  nodeTypes[10].SetType(1, 1, nsValue);
  nodeTypes[11].SetType(1, 1, nsValue);
  nodeTypes[12].SetType(1, 1, nsValue);
  nodeTypes[13].SetType(1, 1, nsValue);
  
  int *nodeAssociation = new int[nnodes];
  for (i = 0; i < nnodes; i++)
  {
    nodeAssociation[i] = i;
  }
  
  CIDNet *pIDNet = CIDNet::Create(nnodes, numberOfNodeTypes, nodeTypes,
    nodeAssociation, pGraph);
  
  CModelDomain* pMD = pIDNet->GetModelDomain();
  
  CFactor **myParams = new CFactor*[nnodes];
  int *nodeNumbers = new int [nnodes];
  
  int domain0[] = { 0 };
  nodeNumbers[0] =  1;
  int domain1[] = { 0, 1 };
  nodeNumbers[1] =  2;
  int domain2[] = { 1, 2 };
  nodeNumbers[2] =  2;
  int domain3[] = { 2, 10 };
  nodeNumbers[3] =  2;
  int domain4[] = { 0, 2, 3 };
  nodeNumbers[4] =  3;
  int domain5[] = { 3, 4 };
  nodeNumbers[5] =  2;
  int domain6[] = { 4, 5 };
  nodeNumbers[6] =  2;
  int domain7[] = { 5, 11 };
  nodeNumbers[7] =  2;
  int domain8[] = { 3, 5, 6 };
  nodeNumbers[8] =  3;
  int domain9[] = { 6, 7 };
  nodeNumbers[9] =  2;
  int domain10[] = { 7, 8 };
  nodeNumbers[10] =  2;
  int domain11[] = { 8, 12 };
  nodeNumbers[11] =  2;
  int domain12[] = { 6, 8, 9 };
  nodeNumbers[12] =  3;
  int domain13[] = { 9, 13 };
  nodeNumbers[13] =  2;
  
  int *domains[] = { domain0, domain1, domain2, domain3, domain4,
    domain5, domain6, domain7, domain8, domain9, domain10, domain11, 
    domain12, domain13 };
  
  pIDNet->AllocFactors();
  
  for (i = 0; i < nnodes; i++)
  {
    myParams[i] = CTabularCPD::Create(domains[i], nodeNumbers[i], pMD);
  }
  
  float data0[] = {0.900000f, 0.100000f};
  float data1[] = {0.100000f, 0.900000f, 0.800000f, 0.200000f};
  float data2[] = {0.500000f, 0.500000f, 0.500000f, 0.500000f};
  float data3[] = {-100.000000f, 0.000000f};
  float data4[] = {0.900000f, 0.100000f, 0.800000f, 0.200000f, 0.500000f, 0.500000f, 0.100000f, 0.900000f};
  float data5[] = {0.100000f, 0.900000f, 0.800000f, 0.200000f};
  float data6[] = {0.500000f, 0.500000f, 0.500000f, 0.500000f};
  float data7[] = {-100.000000f, 0.000000f};
  float data8[] = {0.900000f, 0.100000f, 0.800000f, 0.200000f, 0.500000f, 0.500000f, 0.100000f, 0.900000f};
  float data9[] = {0.100000f, 0.900000f, 0.800000f, 0.200000f};
  float data10[] = {0.500000f, 0.500000f, 0.500000f, 0.500000f};
  float data11[] = {-100.000000f, 0.000000f};
  float data12[] = {0.900000f, 0.100000f, 0.800000f, 0.200000f, 0.500000f, 0.500000f, 0.100000f, 0.900000f};
  float data13[] = {1000.000000f, 300.000000f};
  
  float *data[] = { data0, data1, data2, data3, data4,
    data5, data6, data7, data8, data9,
    data10, data11, data12, data13 };
  
  for (i = 0; i < nnodes; i++)
  {
    myParams[i]->AllocMatrix(data[i], matTable);
    pIDNet->AttachFactor(myParams[i]);
  }
  
  delete [] nodeTypes;
  delete [] nodeAssociation;
  
  return pIDNet;
}
// ----------------------------------------------------------------------------

CIDNet* CreateAppleJackLIMID()
{
  const int nnodes = 9;
  const int numberOfNodeTypes = 9;
  
  int i;
  
  CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
  pGraph->AddNodes(nnodes);
  pGraph->AddEdge(0,2,1);
  pGraph->AddEdge(1,2,1);
  pGraph->AddEdge(4,6,1);
  pGraph->AddEdge(5,6,1);
  pGraph->AddEdge(0,4,1);
  pGraph->AddEdge(1,5,1);
  pGraph->AddEdge(3,4,1);
  pGraph->AddEdge(3,7,1);
  pGraph->AddEdge(4,8,1);
  
  CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];
  
  nodeTypes[0].SetType(1, 2, nsChance);
  nodeTypes[1].SetType(1, 2, nsChance);
  nodeTypes[2].SetType(1, 2, nsChance);
  nodeTypes[3].SetType(1, 2, nsDecision);
  nodeTypes[4].SetType(1, 2, nsChance);
  nodeTypes[5].SetType(1, 2, nsChance);
  nodeTypes[6].SetType(1, 2, nsChance);
  nodeTypes[7].SetType(1, 1, nsValue);
  nodeTypes[8].SetType(1, 1, nsValue);
  
  int *nodeAssociation = new int[nnodes];
  for (i = 0; i < nnodes; i++)
  {
    nodeAssociation[i] = i;
  }
  
  CIDNet *pIDNet = CIDNet::Create(nnodes, numberOfNodeTypes, nodeTypes,
    nodeAssociation, pGraph);
  
  CModelDomain* pMD = pIDNet->GetModelDomain();
  
  CFactor **myParams = new CFactor*[nnodes];
  int *nodeNumbers = new int [nnodes];
  
  int domain0[] = { 0 };
  nodeNumbers[0] =  1;
  int domain1[] = { 1 };
  nodeNumbers[1] =  1;
  int domain2[] = { 0, 1, 2 };
  nodeNumbers[2] =  3;
  int domain3[] = { 0, 3, 4 };
  nodeNumbers[3] =  3;
  int domain4[] = { 1, 5 };
  nodeNumbers[4] =  2;
  int domain5[] = { 4, 5, 6 };
  nodeNumbers[5] =  3;
  int domain6[] = { 3 };
  nodeNumbers[6] =  1;
  int domain7[] = { 3, 7 };
  nodeNumbers[7] =  2;
  int domain8[] = { 4, 8 };
  nodeNumbers[8] =  2;
  
  int *domains[] = { domain0, domain1, domain2, domain3, domain4,
    domain5, domain6, domain7, domain8 };
  
  pIDNet->AllocFactors();
  
  for (i = 0; i < nnodes; i++)
  {
    myParams[i] = CTabularCPD::Create(domains[i], nodeNumbers[i], pMD);
  }
  
  float data0[] = {0.100000f, 0.900000f};
  float data1[] = {0.100000f, 0.900000f};
  float data2[] = {0.950000f, 0.050000f, 0.900000f, 0.100000f, 0.850000f, 0.150000f, 0.020000f, 0.980000f};
  float data3[] = {0.200000f, 0.800000f, 0.990000f, 0.010000f, 0.010000f, 0.990000f, 0.020000f, 0.980000f};
  float data4[] = {0.600000f, 0.400000f, 0.050000f, 0.950000f};
  float data5[] = {0.950000f, 0.050000f, 0.900000f, 0.100000f, 0.850000f, 0.150000f, 0.020000f, 0.980000f};
  float data6[] = {0.500000f, 0.500000f};
  float data7[] = {-8000.000000f, 0.000000f};
  float data8[] = {3000.000000f, 20000.000000f};
  
  float *data[] = { data0, data1, data2, data3, data4,
    data5, data6, data7, data8 };
  
  for (i = 0; i < nnodes; i++)
  {
    myParams[i]->AllocMatrix(data[i], matTable);
    pIDNet->AttachFactor(myParams[i]);
  }
  
  delete [] nodeTypes;
  delete [] nodeAssociation;
  
  return pIDNet;
}
// ----------------------------------------------------------------------------

CIDNet* CreateOilLIMID()
{
  const int nnodes = 6;
  const int numberOfNodeTypes = 6;
  
  int i;
  
  CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
  pGraph->AddNodes(nnodes);
  pGraph->AddEdge(0,2,1);
  pGraph->AddEdge(0,4,1);
  pGraph->AddEdge(1,2,1);
  pGraph->AddEdge(1,5,1);
  pGraph->AddEdge(2,3,1);
  pGraph->AddEdge(3,4,1);
  
  CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];
  
  nodeTypes[0].SetType(1, 3, nsChance);
  nodeTypes[1].SetType(1, 2, nsDecision);
  nodeTypes[2].SetType(1, 3, nsChance);
  nodeTypes[3].SetType(1, 2, nsDecision);
  nodeTypes[4].SetType(1, 1, nsValue);
  nodeTypes[5].SetType(1, 1, nsValue);
  
  int *nodeAssociation = new int[nnodes];
  for (i = 0; i < nnodes; i++)
  {
    nodeAssociation[i] = i;
  }
  
  CIDNet *pIDNet = CIDNet::Create(nnodes, numberOfNodeTypes, nodeTypes,
    nodeAssociation, pGraph);
  
  CModelDomain* pMD = pIDNet->GetModelDomain();
  
  CFactor **myParams = new CFactor*[nnodes];
  int *nodeNumbers = new int [nnodes];
  
  int domain0[] = { 0 };
  nodeNumbers[0] =  1;
  int domain1[] = { 1 };
  nodeNumbers[1] =  1;
  int domain2[] = { 0, 1, 2 };
  nodeNumbers[2] =  3;
  int domain3[] = { 2, 3 };
  nodeNumbers[3] =  2;
  int domain4[] = { 0, 3, 4 };
  nodeNumbers[4] =  3;
  int domain5[] = { 1, 5 };
  nodeNumbers[5] =  2;
  
  int *domains[] = { domain0, domain1, domain2, domain3, domain4, domain5 };
  
  pIDNet->AllocFactors();
  
  for (i = 0; i < nnodes; i++)
  {
    myParams[i] = CTabularCPD::Create(domains[i], nodeNumbers[i], pMD);
  }
  
  float data0[] = {0.500000f, 0.300000f, 0.200000f};
  float data1[] = {0.500000f, 0.500000f};
  float data2[] = {0.100000f, 0.300000f, 0.600000f, 0.333333f, 0.333333f, 0.333333f, 0.300000f, 0.400000f, 0.300000f, 0.333333f, 0.333333f, 0.333333f, 0.500000f, 0.400000f, 0.100000f, 0.333333f, 0.333333f, 0.333333f};
  float data3[] = {0.500000f, 0.500000f, 0.500000f, 0.500000f, 0.500000f, 0.500000f};
  float data4[] = {-70000.000000f, 0.000000f, 50000.000000f, 0.000000f, 200000.000000f, 0.000000f};
  float data5[] = {-10000.000000f, 0.000000f};
  
  float *data[] = { data0, data1, data2, data3, data4, data5 };
  
  for (i = 0; i < nnodes; i++)
  {
    myParams[i]->AllocMatrix(data[i], matTable);
    pIDNet->AttachFactor(myParams[i]);
  }
  
  delete [] nodeTypes;
  delete [] nodeAssociation;
  
  return pIDNet;
}
// ----------------------------------------------------------------------------

CIDNet* CreateLIMIDWith2DecInClick()
{
  const int nnodes = 7;
  const int numberOfNodeTypes = 7;
  
  int i;
  
  CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
  pGraph->AddNodes(nnodes);
  pGraph->AddEdge(0,1,1);
  pGraph->AddEdge(2,3,1);
  pGraph->AddEdge(1,4,1);
  pGraph->AddEdge(3,4,1);
  pGraph->AddEdge(1,5,1);
  pGraph->AddEdge(4,6,1);
  
  CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];
  
  nodeTypes[0].SetType(1, 2, nsChance);
  nodeTypes[1].SetType(1, 2, nsDecision);
  nodeTypes[2].SetType(1, 2, nsChance);
  nodeTypes[3].SetType(1, 2, nsDecision);
  nodeTypes[4].SetType(1, 2, nsChance);
  nodeTypes[5].SetType(1, 1, nsValue);
  nodeTypes[6].SetType(1, 1, nsValue);
  
  int *nodeAssociation = new int[nnodes];
  for (i = 0; i < nnodes; i++)
  {
    nodeAssociation[i] = i;
  }
  
  CIDNet *pIDNet = CIDNet::Create(nnodes, numberOfNodeTypes, nodeTypes,
    nodeAssociation, pGraph);
  
  CModelDomain* pMD = pIDNet->GetModelDomain();
  
  CFactor **myParams = new CFactor*[nnodes];
  int *nodeNumbers = new int [nnodes];
  
  int domain0[] = { 0 };
  nodeNumbers[0] =  1;
  int domain1[] = { 0, 1 };
  nodeNumbers[1] =  2;
  int domain2[] = { 2 };
  nodeNumbers[2] =  1;
  int domain3[] = { 2, 3 };
  nodeNumbers[3] =  2;
  int domain4[] = { 1, 3, 4 };
  nodeNumbers[4] =  3;
  int domain5[] = { 1, 5 };
  nodeNumbers[5] =  2;
  int domain6[] = { 4, 6 };
  nodeNumbers[6] =  2;
  
  int *domains[] = { domain0, domain1, domain2, domain3, domain4,
    domain5, domain6 };
  
  pIDNet->AllocFactors();
  
  for (i = 0; i < nnodes; i++)
  {
    myParams[i] = CTabularCPD::Create(domains[i], nodeNumbers[i], pMD);
  }
  
  float data0[] = {0.200000f, 0.800000f};
  float data1[] = {0.500000f, 0.500000f, 0.500000f, 0.500000f};
  float data2[] = {0.600000f, 0.400000f};
  float data3[] = {0.500000f, 0.500000f, 0.500000f, 0.500000f};
  float data4[] = {0.500000f, 0.500000f, 0.100000f, 0.900000f, 0.800000f, 0.200000f, 0.020000f, 0.980000f};
  float data5[] = {10000.000000f, -2000.000000f};
  float data6[] = {-5000.000000f, 10000.000000f};
  
  float *data[] = { data0, data1, data2, data3, data4,
    data5, data6 };
  
  for (i = 0; i < nnodes; i++)
  {
    myParams[i]->AllocMatrix(data[i], matTable);
    pIDNet->AttachFactor(myParams[i]);
  }
  
  delete [] nodeTypes;
  delete [] nodeAssociation;
  
  return pIDNet;	
}
// ----------------------------------------------------------------------------

float GetBelief(void)
{
  float belief;
  int randInt = rand();
  belief = randInt / float(RAND_MAX);
  
  return belief;
}
// ----------------------------------------------------------------------------

float GetBelief(float porog)
{
  float belief = GetBelief();
  while (belief >= porog) 
    belief -= porog;
  
  return belief;
}
// ----------------------------------------------------------------------------

int GetUtility(int min, int max)
{
  int randInt = rand() % (max - min + 1);

  return (randInt + min);
}
// ----------------------------------------------------------------------------

int GetRandomNumberOfStates(int max_num_states, int min_num_states = 2)
{
  int num_states;
  num_states = rand() % (max_num_states - min_num_states + 1);
  return (num_states + min_num_states);
}
// ----------------------------------------------------------------------------

CGraph* CreateRandomAndSpecificForIDNetGraph(int num_nodes,
  int num_indep_nodes, int max_size_family)
{
  PNL_CHECK_LEFT_BORDER(num_nodes, 10);
  PNL_CHECK_RANGES(num_indep_nodes, 1, num_nodes-1);
  PNL_CHECK_RANGES(max_size_family, 2, num_nodes);
  
  int i, j, k;
  
  CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
  PNL_CHECK_IF_MEMORY_ALLOCATED(pGraph);
  
  srand((unsigned int)time(NULL));
  
  pGraph->AddNodes(num_nodes);
  
  int num_parents;
  int ind_parent;
  intVector prev_nodes(0);
  for (i = num_indep_nodes; i < num_nodes; i++)
  {
    prev_nodes.resize(0);
    for (j = 0; j < i; j++)
      prev_nodes.push_back(j);
    
    num_parents = rand() % (max_size_family - 1);
    num_parents += 1;
    num_parents = (num_parents > i) ? i : num_parents;
    
    for (j = 0; j < num_parents; j++)
    {
      ind_parent = rand() % prev_nodes.size();
      pGraph->AddEdge(prev_nodes[ind_parent], i, 1);
      prev_nodes.erase(prev_nodes.begin() + ind_parent);
    }
  }
  
  intVector parents(0);
  intVector childs(0);
  for (i = 0; i < num_nodes; i++)
  {
    if (pGraph->GetNumberOfChildren(i) == 0)
    {
      pGraph->GetParents(i, &parents);
      for (j = 0; j < parents.size(); j++)
      {
        pGraph->GetChildren(parents[j], &childs);
        for (k = 0; k < childs.size(); k++)
          if ((childs[k] != i) && 
            (pGraph->GetNumberOfChildren(childs[k]) == 0) &&
            (pGraph->GetNumberOfParents(childs[k]) == 1))
          {
            if (i < childs[k])
            {
              pGraph->RemoveEdge(parents[j], childs[k]);
              pGraph->AddEdge(i, childs[k], 1);
            }
            else
            {
              pGraph->AddEdge(childs[k], i, 1);
            }
          }
      }
    }
  }
  
  return pGraph;
}
// ----------------------------------------------------------------------------

CIDNet* CreateRandomIDNet(int num_nodes, int num_indep_nodes,
  int max_size_family, int num_decision_nodes, int max_num_states_chance_nodes,
  int max_num_states_decision_nodes, int min_utility, int max_utility,
  bool is_uniform_start_policy)
{
  PNL_CHECK_RANGES(num_decision_nodes, 1, num_nodes-1);
  PNL_CHECK_LEFT_BORDER(max_num_states_chance_nodes, 1);
  PNL_CHECK_LEFT_BORDER(max_num_states_decision_nodes, 1);
  PNL_CHECK_LEFT_BORDER(max_utility, min_utility);
  
  CGraph* pGraph = 
    CreateRandomAndSpecificForIDNetGraph(num_nodes, num_indep_nodes,
    max_size_family);
  
  if (!pGraph->IsDAG())
  {
    PNL_THROW(CInconsistentType, " the graph should be a DAG ");
  }
  
  if (!pGraph->IsTopologicallySorted())
  {
    PNL_THROW(CInconsistentType, 
      " the graph should be sorted topologically ");
  }
  if (pGraph->NumberOfConnectivityComponents() > 1)
  {
    PNL_THROW(CInconsistentType, " the graph should be linked ");
  }
  
  int i, j, k;
  
  CNodeType *nodeTypes = new CNodeType [num_nodes];
  
  intVector nonValueNodes(0);
  intVector posibleDecisionNodes(0);
  nonValueNodes.resize(0);
  posibleDecisionNodes.resize(0);
  for (i = 0; i < num_nodes; i++)
  {
    if (pGraph->GetNumberOfChildren(i) == 0)
    {
      nodeTypes[i].SetType(1, 1, nsValue);
    }
    else
    {
      nonValueNodes.push_back(i);
      posibleDecisionNodes.push_back(i);
    }
  }
  int ind_decision_node;
  int num_states;
  int index;
  int node;
  intVector neighbors(0);
  neighborTypeVector neigh_types(0);

  num_decision_nodes = (num_decision_nodes > posibleDecisionNodes.size()) ? 
    posibleDecisionNodes.size() : num_decision_nodes;
  for (i = 0; (i < num_decision_nodes) && (posibleDecisionNodes.size()>0); i++)
  {
    ind_decision_node = rand() % posibleDecisionNodes.size();
    node = posibleDecisionNodes[ind_decision_node];
    num_states = GetRandomNumberOfStates(max_num_states_decision_nodes);
    nodeTypes[node].SetType(1, num_states, nsDecision);
    
    index = -1;
    for (j = 0; j < nonValueNodes.size(); j++)
    {
      if (nonValueNodes[j] == node)
      {
        index = j;
        break;
      }
    }
    if (index != -1)
      nonValueNodes.erase(nonValueNodes.begin() + index);
      
    posibleDecisionNodes.erase(posibleDecisionNodes.begin() + 
      ind_decision_node);
    pGraph->GetNeighbors(node, &neighbors, &neigh_types);
    for (j = 0; j < neighbors.size(); j++)
    {
      index = -1;
      for (k = 0; k < posibleDecisionNodes.size(); k++)
      {
        if (neighbors[j] == posibleDecisionNodes[k])
        {
          index = k;
          break;
        }
      }
      if (index != -1)
        posibleDecisionNodes.erase(posibleDecisionNodes.begin() + index);
    }
  }
  for (i = 0; i < nonValueNodes.size(); i++)
  {
    num_states = GetRandomNumberOfStates(max_num_states_chance_nodes);
    nodeTypes[nonValueNodes[i]].SetType(1, num_states, nsChance);
  }
  
  int *nodeAssociation = new int[num_nodes];
  for (i = 0; i < num_nodes; i++)
  {
    nodeAssociation[i] = i;
  }
  
  CIDNet *pIDNet = CIDNet::Create(num_nodes, num_nodes, nodeTypes,
    nodeAssociation, pGraph);
  pGraph = pIDNet->GetGraph();
  CModelDomain* pMD = pIDNet->GetModelDomain();
  
  CFactor **myParams = new CFactor*[num_nodes];
  int *nodeNumbers = new int[num_nodes];
  int **domains = new int*[num_nodes];
  
  intVector parents(0);
  for (i = 0; i < num_nodes; i++)
  {
    nodeNumbers[i] = pGraph->GetNumberOfParents(i) + 1;
    domains[i] = new int[nodeNumbers[i]];
    pGraph->GetParents(i, &parents);
    
    for (j = 0; j < parents.size(); j++)
    {
      domains[i][j] = parents[j];
    }
    domains[i][nodeNumbers[i]-1] = i;
  }
  
  pIDNet->AllocFactors();
  
  for (i = 0; i < num_nodes; i++)
  {
    myParams[i] = CTabularCPD::Create(domains[i], nodeNumbers[i], pMD);
  }
  
  float **data = new float*[num_nodes];
  int size_data;
  int num_states_node;
  int num_blocks;
  intVector size_nodes(0);
  float belief, sum_beliefs;
  
  for (i = 0; i < num_nodes; i++)
  {
    size_data = 1;
    size_nodes.resize(0);
    for (j = 0; j < nodeNumbers[i]; j++)
    {
      size_nodes.push_back(pIDNet->GetNodeType(domains[i][j])->GetNodeSize());
      size_data *= size_nodes[j];
    }
    num_states_node = size_nodes[size_nodes.size() - 1];
    num_blocks = size_data / num_states_node;
    
    data[i] = new float[size_data];
    switch (pIDNet->GetNodeType(i)->GetNodeState())
    {
      case nsChance:
      {
        for (j = 0; j < num_blocks; j++)
        {
          sum_beliefs = 0.0;
          for (k = 0; k < num_states_node - 1; k++)
          {
            belief = GetBelief(1.0f - sum_beliefs);
            data[i][j * num_states_node + k] = belief;
            sum_beliefs += belief;
          }
          belief = 1.0f - sum_beliefs;
          data[i][j * num_states_node + num_states_node - 1] = belief;
        }
        break;
      }
      case nsDecision:
      {
        if (is_uniform_start_policy)
        {
          belief = 1.0f / float(num_states_node);
          for (j = 0; j < num_blocks; j++)
          {
            sum_beliefs = 0.0;
            for (k = 0; k < num_states_node - 1; k++)
            {
              data[i][j * num_states_node + k] = belief;
              sum_beliefs += belief;
            }
            data[i][j * num_states_node + num_states_node - 1] = 
              1.0f - sum_beliefs;
          }
        }
        else
        {
          for (j = 0; j < num_blocks; j++)
          {
            sum_beliefs = 0.0;
            for (k = 0; k < num_states_node - 1; k++)
            {
              belief = GetBelief(1.0f - sum_beliefs);
              data[i][j * num_states_node + k] = belief;
              sum_beliefs += belief;
            }
            belief = 1.0f - sum_beliefs;
            data[i][j * num_states_node + num_states_node - 1] = belief;
          }
        }
        break;
      }
      case nsValue:
      {
        for (j = 0; j < num_blocks; j++)
        {
          data[i][j] = float(GetUtility(min_utility, max_utility));
        }
        break;
      }
    }
  }

  for (i = 0; i < num_nodes; i++)
  {
    myParams[i]->AllocMatrix(data[i], matTable);
    pIDNet->AttachFactor(myParams[i]);
  }

  delete [] nodeTypes;
  delete [] nodeAssociation;

  return pIDNet;
}
// end of file ----------------------------------------------------------------
