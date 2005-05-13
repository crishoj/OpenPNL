/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlLIMIDInferenceEngine.cpp                                 //
//                                                                         //
//  Purpose:   CLIMIDInfEngine class member functions implementation       //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlLIMIDInfEngine.hpp"
#include "pnlJunctionTree.hpp"
#include "pnlContextPersistence.hpp"
#include "pnlIDNet.hpp"
#include "pnlIDTabularPotential.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlException.hpp"
//-----------------------------------------------------------------------------

#ifdef _DEBUG
//  #define LIMID_TEST
#endif

PNL_USING

CLIMIDInfEngine* CLIMIDInfEngine::Create(
  const CStaticGraphicalModel *pGraphicalModel)
{
  PNL_CHECK_IS_NULL_POINTER(pGraphicalModel);

  std::string description;
  if(!pGraphicalModel->IsValid( &description) )
  {
    PNL_THROW(CAlgorithmicException, description )
  }

  intVector vec_association(0);
  CIDNet* pConvertedIDNet = CreateIDWithLastUtilityNodes(pGraphicalModel, 
    vec_association);

  CLIMIDInfEngine *pInfEngine = new CLIMIDInfEngine(pGraphicalModel, 
    pConvertedIDNet, vec_association);

  PNL_CHECK_IF_MEMORY_ALLOCATED(pInfEngine);

  return pInfEngine;
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::Release(CLIMIDInfEngine** pInfEng)
{
  delete *pInfEng;

  *pInfEng = NULL;
}
//-----------------------------------------------------------------------------

CLIMIDInfEngine::CLIMIDInfEngine(const CStaticGraphicalModel *pOldGM, 
  const CStaticGraphicalModel *pNewGM, intVector& vec_association):
  m_Paths(0), m_Association(vec_association), 
  m_pJTree(CJunctionTree::Create(pNewGM, 0, NULL, NULL)),
  m_decisionNodesOfClqs(0), m_cliquesContDecisionNodes(0), m_IterMax(10)
{
  m_pIDNet = static_cast <const CIDNet*> (pNewGM);
  m_pBaseIDNet = static_cast <const CIDNet*> (pOldGM);
  m_JTreeRootNode = -1;
  SetJTreeRootNode(0);
  m_pIDNet->GetDecisionNodes(m_decisionNodes);

  if (m_decisionNodes.size() == 0)
    PNL_THROW(CInternalError, "CLIMIDInfEngine:: there are no desision nodes in net");

  m_RetractPotential = NULL;
  m_CollectPotential = NULL;
  m_pMargFromFamilyPotential = NULL;
  m_pMatContr = NULL;
}
//-----------------------------------------------------------------------------

CLIMIDInfEngine::~CLIMIDInfEngine()
{
  delete m_pJTree;

  delete m_RetractPotential;
  delete m_CollectPotential;
  delete m_pMargFromFamilyPotential;
  delete m_pMatContr;

  if (m_pBaseIDNet != m_pIDNet) 
    delete m_pIDNet;
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::SetIterMax(int IterMax)
{
  m_IterMax = IterMax;
}
//-----------------------------------------------------------------------------

bool CLIMIDInfEngine::CheckStopCondition()
{
  if (m_exp == m_oldexp)
    return true;
  else
    return false;
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::PrintCliques()
{
  const CGraph *pGraph = m_pJTree->GetGraph();
  int NumClick = pGraph->GetNumberOfNodes();
  printf("\nClique's potentials:\n========================\n", m_IterNum);
  for (int i = 0; i < NumClick; i++)
    m_nodePots[i]->Dump();
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::DoInference()
{
  m_FirstIter = true;
  m_IterNum = 0;

  InitEngine();
  InitMailBoxes();
  BuildAllRootsOfTree();
  BuildPathsBetweenRoots();
  SetJTreeRootNode(m_Roots[0]);

  // main cycle
  bool IsStop = false;
  while (!IsStop)
  {
    // we need to update each of decision factor (policy)
    m_oldexp = m_exp;
    for (int i = 0; i < m_decisionNodes.size(); i++)
    {
      m_currIndexOfDecisionNode = m_IterNum % m_decisionNodes.size();
      Retract();
      Collect();
      Marginalization();
      Contraction();
      Optimization();
      CalculateExpectation();
      m_IterNum++;

#ifdef LIMID_TEST
  printf("\n\nClique's potentials after %d iterations:\n", m_IterNum);
  PrintCliques();
#endif
    }
    // check if it's time to stop
    IsStop = (CheckStopCondition() || 
      ((m_IterNum / m_decisionNodes.size()) >= m_IterMax));
  }
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::InitEngine()
{
#ifdef LIMID_TEST
  printf("\n***** InitEngine Start *****\n\n");
#endif

  m_exp = 0.0;

  int i, j,k;
  int NodeContentSize;  // clique's size
  const int *Content;   // pointer to the fist node in clique
  int Count = 1;        // clique's size * nodeAssociationOut
  int numoftype;        // state node
//  CIDTabularPotential *Potential;

  int NumberOfNodes = m_pIDNet->GetNumberOfNodes();
  if (NumberOfNodes <= 0)
    PNL_THROW(CInternalError, "CLIMIDInfEngine::Graph is empty");

  const CGraph *pGraph = m_pJTree->GetGraph();
  int NumClick = pGraph->GetNumberOfNodes();
  m_nodePots.resize(NumClick);
  m_DesNodePolitics.resize(NumClick);
  m_decisionNodesOfClqs.resize(NumClick);

//  int Size = m_decisionNodes.size();
  m_cliquesContDecisionNodes.resize(m_decisionNodes.size());

  // Put empty potential to each clique
  for (i = 0; i < NumClick; i++)
  {
    Count = 1;
    m_pJTree->GetNodeContent(i, &NodeContentSize, &Content);
    for (j = 0; j < NodeContentSize; j++)
    {
      int nextnode = Content[j];
      numoftype = 
        m_pJTree->GetModelDomain()->GetVariableType(nextnode)->GetNodeSize();
      Count *= numoftype;
    }
    floatVector probData(Count, 1);
    floatVector UtilityData(Count, 0);

#ifdef LIMID_TEST
  printf("\ncontent[%d]    :", i);
  for (j = 0; j < NodeContentSize; j++)
    printf("\t%d", Content[j]);
#endif

  int *OldContent;
  OldContent = new int [NodeContentSize];
  for (int k=0; k< NodeContentSize; k++)
    OldContent[k] = Content[k];

#ifdef LIMID_TEST
  printf("\nold_content[%d]:", i);
  for (j = 0; j < NodeContentSize; j++)
    printf("\t%d", OldContent[j]);
#endif

    m_nodePots[i] = CIDTabularPotential::Create(OldContent, NodeContentSize,
      m_pJTree->GetModelDomain(), &probData.front(), &UtilityData.front());
  }

  EIDNodeState NodeState;
  CFactor *pNodePot = NULL;
  const int *nodeDomain;
  int *newnodeDomain;
  int nodeDomainSize;
  const int *clqDomain;
  int clqDomainSize;
  int ClqWithFam;

  for (i = 0; i < NumberOfNodes; i++)
  {
    NodeState = m_pIDNet->GetNodeType(i)->GetNodeState();
    m_pIDNet->GetFactor(i)->GetDomain(&nodeDomainSize, &nodeDomain);
    newnodeDomain = new int[nodeDomainSize];
    for (k = 0; k < nodeDomainSize; k++)
    {
      newnodeDomain[k] = nodeDomain[k];
    }
    switch(NodeState)
    {
      case nsChance:
      {
        ClqWithFam = GetClqNumContainingSubset(nodeDomainSize, newnodeDomain);
        pNodePot = m_pIDNet->GetFactor(i);

#ifdef LIMID_TEST
  cout<<endl<<"input to "<<ClqWithFam<<endl;
  m_nodePots[ClqWithFam]->Dump();
  cout<<endl<<"mult to "<<i<<endl;
  pNodePot->GetDistribFun()->Dump();
#endif

        m_nodePots[ClqWithFam]->GetDomain(&clqDomainSize, &clqDomain);
        CDistribFun *nodeDistrib = m_nodePots[ClqWithFam]->GetProbDistribFun();
        nodeDistrib->MultiplyInSelfData(clqDomain, newnodeDomain, 
          pNodePot->GetDistribFun());

#ifdef LIMID_TEST
  cout<<endl<<"Result: "<<endl;
  m_nodePots[ClqWithFam]->Dump();
#endif

        break;
      }
      case nsDecision:
      {
        ClqWithFam = GetClqNumContainingSubset(nodeDomainSize, newnodeDomain);

#ifdef LIMID_TEST
  cout<<endl<<"input to "<<ClqWithFam<<endl;
  m_nodePots[ClqWithFam]->Dump();
  cout<<endl<<"insert "<<i<<endl;
#endif

        m_DesNodePolitics[ClqWithFam].push_back(
          m_pIDNet->GetFactor(i)->Clone());
        m_decisionNodesOfClqs[ClqWithFam].push_back(i);
      
        for (int k = 0; k < m_decisionNodes.size(); k++)
        {
          if (m_decisionNodes[k] == i)
          {
            m_cliquesContDecisionNodes[k] = ClqWithFam;
            break;
          }
        }

#ifdef LIMID_TEST
  cout<<endl<<"Result: "<<endl;
  m_nodePots[ClqWithFam]->Dump();
#endif

        break;
      }
      case nsValue:
      {
//        int *fam = new int [nodeDomainSize - 1];
        int *fam = new int [nodeDomainSize];
//        memcpy(fam, newnodeDomain, (nodeDomainSize - 1) * sizeof(int));
        memcpy(fam, newnodeDomain, nodeDomainSize * sizeof(int));
      
//        ClqWithFam = GetClqNumContainingSubset(nodeDomainSize - 1, fam);
        ClqWithFam = GetClqNumContainingSubset(nodeDomainSize, fam);
        pNodePot = m_pIDNet->GetFactor(i);
      
        float *data;
        int dataSize;
        (static_cast<CDenseMatrix<float>*>(pNodePot->GetDistribFun()->
          GetMatrix(matTable)))->GetRawData(&dataSize, (const float**)&data);
        const pConstNodeTypeVector *nodeTypes = 
          pNodePot->GetDistribFun()->GetNodeTypesVector();

        CDistribFun* smallDistrib = CTabularDistribFun::Create(
//          nodeDomainSize - 1, nodeTypes->begin(), data);
          nodeDomainSize, &nodeTypes->front(), data);

#ifdef LIMID_TEST
  cout<<endl<<"input to "<<ClqWithFam<<endl;
  m_nodePots[ClqWithFam]->Dump();
  cout<<endl<<"add to "<<i<<endl;
  smallDistrib->Dump();
#endif

        m_nodePots[ClqWithFam]->GetDomain(&clqDomainSize, &clqDomain);
        CDistribFun *nodeDistrib = 
          m_nodePots[ClqWithFam]->GetUtilityDistribFun();
        nodeDistrib->SumInSelfData(clqDomain, fam, smallDistrib);

#ifdef LIMID_TEST
  cout<<endl<<"Result: "<<endl;
  m_nodePots[ClqWithFam]->Dump();
#endif

        delete [] newnodeDomain;
        delete smallDistrib;
        delete [] fam;
        break;
      }
    }
  }

#ifdef LIMID_TEST
  printf("\n\nClique's potentials:\n");
  for (i = 0; i < NumClick; i++)
    m_nodePots[i]->Dump();
  printf("\n\n***** InitEngine End *****\n");
#endif
}
//-----------------------------------------------------------------------------

CIDNet* CLIMIDInfEngine::CreateIDWithLastUtilityNodes(
  const CStaticGraphicalModel *pOldGM, intVector& vec_association)
{
  const CIDNet* pIDNet = static_cast<const CIDNet*>(pOldGM);
  
  PNL_CHECK_IS_NULL_POINTER(pIDNet);
  
  CGraph* pGraph = pIDNet->GetGraph();
  int nnodes = pGraph->GetNumberOfNodes();
  int i, j;

  bool IsLastValueNodes = true;
  bool IsExistsValueNode = false;
  for (i = 0; (i < nnodes) && IsLastValueNodes; i++)
  {
    if (pIDNet->GetNodeType(i)->GetNodeState() == nsValue)
      IsExistsValueNode = true;
    else
      if (IsExistsValueNode && (pIDNet->GetNodeType(i)->GetNodeState() != 
        nsValue))
        IsLastValueNodes = false;
  }

  CIDNet* pNewIDNet;
  CGraph* pNewGraph;

  vec_association.resize(nnodes, -1);
  
  int offset = 0;
  
  for (i = 0; i < nnodes; i++)
  {
    if (pIDNet->GetNodeType(i)->GetNodeState() == nsValue)
    {
      offset++;
      vec_association[i] = nnodes - offset;
    }
    else
      vec_association[i] = i - offset;
  }

#ifdef LIMID_TEST
  printf("\nAssociation:\t");
  for (i = 0; i < nnodes; i++) 
    printf("\t%d", vec_association[i]);
#endif

  if (IsLastValueNodes)
  {
    pNewIDNet = const_cast <CIDNet*> (pIDNet);
    return pNewIDNet;
  }

  pNewGraph = CGraph::Create(0, NULL, NULL, NULL);
  
  PNL_CHECK_IF_MEMORY_ALLOCATED( pNewGraph );
    
  pNewGraph->AddNodes(nnodes);

  intVector parents(0);
  for (i = 0; i < nnodes; i++)
  {
    pGraph->GetParents(i, &parents);
    for (j = 0; j < parents.size(); j++)
      pNewGraph->AddEdge(vec_association[parents[j]], vec_association[i], 1);
  }

#ifdef LIMID_TEST
  pGraph->Dump();
  pNewGraph->Dump();
#endif

  CNodeType *nodeTypes = new CNodeType [nnodes];
  CNodeType nt;

  for (i = 0; i < nnodes; i++)
  {
    nt = *pIDNet->GetNodeType(i);
    nodeTypes[vec_association[i]].SetType(nt.IsDiscrete(), 
      nt.GetNodeSize(), nt.GetNodeState());
  }

#ifdef LIMID_TEST
  printf("\ntypes:\t");
  for (i = 0; i < nnodes; i++) 
    printf("\t%d:%d", nodeTypes[i].GetNodeSize(), nodeTypes[i].GetNodeState());
#endif

  int *nodeAssociation = new int[nnodes];
  for ( i = 0; i < nnodes; i++)
  {
    nodeAssociation[i] = i;
  }

  pNewIDNet = CIDNet::Create( nnodes, nnodes, nodeTypes,
                              nodeAssociation, pNewGraph ); 

  PNL_CHECK_IF_MEMORY_ALLOCATED( pNewIDNet );

  CModelDomain* pMD = pNewIDNet->GetModelDomain();
    
  int domainSize;
  const int* domain;

  CFactor **myParams = new CFactor*[nnodes];
  int *nodeNumbers = new int[nnodes];
  int **new_domains = new int*[nnodes];

  for (i = 0; i < nnodes; i++)
  {
    pIDNet->GetFactor(i)->GetDomain(&domainSize, &domain);

#ifdef LIMID_TEST
    printf("\ndomain_old[%d]:\t", i);
    for (j = 0; j < domainSize; j++) 
      printf("\t%d", domain[j]);
#endif

    nodeNumbers[vec_association[i]] = domainSize;

    new_domains[vec_association[i]] = new int[domainSize];
    for (j = 0; j < domainSize; j++)
      new_domains[vec_association[i]][j] = vec_association[domain[j]];

#ifdef LIMID_TEST
    printf("\ndomain_new[%d]:\t", vec_association[i]);
    for (j = 0; j < domainSize; j++) 
      printf("\t%d", new_domains[vec_association[i]][j]);
#endif
  }

  pNewIDNet->AllocFactors();

  for(i = 0; i < nnodes; i++)
  {
    myParams[i] = CTabularCPD::Create(new_domains[i], nodeNumbers[i], pMD);
  }

  CNumericDenseMatrix<float>* matrix;
  float **data_copy = new float*[nnodes];
  int data_length;
  const float *data;

  for (i = 0; i < nnodes; i++)
  {
    matrix = static_cast <CNumericDenseMatrix<float>*>
      (pIDNet->GetFactor(i)->GetMatrix(matTable));
    matrix->GetRawData(&data_length, &data);

    data_copy[vec_association[i]] = new float[data_length];
    for (j =0; j < data_length; j++)
      data_copy[vec_association[i]][j] = data[j];
  }

  for(i = 0; i < nnodes; i++ )
  {
    myParams[i]->AllocMatrix(data_copy[i], matTable);
    pNewIDNet->AttachFactor(myParams[i]);
  }    

  delete [] nodeTypes;
  delete [] nodeAssociation;

  return pNewIDNet;
}
//-----------------------------------------------------------------------------

int CLIMIDInfEngine::GetClqNumContainingSubset(int numOfNdsInSubset,
  const int *subset) const
{
  const CGraph *pGraph = m_pJTree->GetGraph();
  int NumClick = pGraph->GetNumberOfNodes();
  int CurNumClique = 0;
  int res = -1;
  int i;
//  bool flag = false;
  int NodeContentSize; // clique's size
  const int *Content;  // pointer to the fist node in clique

  while ((res == -1) && (CurNumClique < NumClick))
  {
    m_pJTree->GetNodeContent(CurNumClique, &NodeContentSize, &Content);
//    bool flag = false;
    for (i = 0; i < numOfNdsInSubset; i++)
    {
      if (std::find(Content, Content + NodeContentSize, subset[i]) == 
        Content + NodeContentSize)
      {
        CurNumClique++;
        break;
      }
    }
    if (i == numOfNdsInSubset)
      res = CurNumClique;
  }

  return res;
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::Retract()
{
  int DesPos = m_IterNum % m_decisionNodes.size();
  int NumClq = m_cliquesContDecisionNodes[DesPos];
  int NDes = m_decisionNodesOfClqs[NumClq].size();
  int i;

#ifdef LIMID_TEST
  printf("\nm_decisionNodesOfClqs[%d]:", NumClq);
  for (i = 0; i < m_decisionNodesOfClqs[NumClq].size(); i++)
    printf ("\t%d", m_decisionNodesOfClqs[NumClq][i]);
  printf("\nm_cliquesContDecisionNodes:");
  for (i = 0; i < m_decisionNodes.size(); i++)
    printf("\t%d", m_cliquesContDecisionNodes[i]);
#endif  
  
  if (m_RetractPotential)
  {
    delete m_RetractPotential;
  }

  if (NDes == 1) 
    m_RetractPotential = dynamic_cast <CIDPotential*>
      (m_nodePots[m_cliquesContDecisionNodes[m_currIndexOfDecisionNode]]->
      Clone());
  else
  {
    m_RetractPotential = dynamic_cast <CIDPotential*>
      (m_nodePots[m_cliquesContDecisionNodes[m_currIndexOfDecisionNode]]->
      Clone());

    for (i = 0; i < NDes; i++)
    {
      if (m_decisionNodesOfClqs[NumClq][i] != 
        m_decisionNodes[m_currIndexOfDecisionNode])
      {
        m_RetractPotential->InsertPoliticsInSelf(
          static_cast <CCPD*> (m_DesNodePolitics[NumClq][i]));
      }
    }
  }
#ifdef LIMID_TEST
  printf("\n***** Retract Start *****\n\n");
  m_RetractPotential->Dump();
  printf("\n\n***** Retract End *****\n");
#endif
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::Collect()
{
  int numOfNbrs;
  const int *nbrs;
  const ENeighborType *nbrsTypes;
  int i;
  CIDPotential *tempCollectPotential;
//  CIDPotential *tempPotential;
  const CGraph *pGraph = m_pJTree->GetGraph();

  if (m_CollectPotential)
  {
    delete m_CollectPotential;
  }

#ifdef LIMID_TEST
  printf("\n***** Collect Start *****\n\n");
#endif

  if (m_FirstIter)  // first iteration
  {
    const int *nbr, *nbrs_end;
    intVector::const_iterator sourceIt, source_end;
    intVecVector::const_iterator layerIt = m_collectSequence.begin(),
      collSeq_end = m_collectSequence.end();
    
    boolVector nodesSentMessages(m_pJTree->GetNumberOfNodes(), false);
    
    // at each step the propagation is from m_collectSequence i-th layer's
    // node to it's neighbors which has not sent the message yet
    for (; layerIt != collSeq_end; ++layerIt)
    {
      for (sourceIt = layerIt->begin(), source_end = layerIt->end();
        sourceIt != source_end; ++sourceIt)
      {
        pGraph->GetNeighbors(*sourceIt, &numOfNbrs, &nbrs, &nbrsTypes);
        
        for (nbr = nbrs, nbrs_end = nbrs + numOfNbrs; nbr != nbrs_end; ++nbr)
        {
          if (!nodesSentMessages[*nbr])
          {

#ifdef LIMID_TEST
  printf("\nPropagateBetweenClqs %d and %d \n", *sourceIt, *nbr);
#endif

            PropagateBetweenClqs(*sourceIt, *nbr);

#ifdef LIMID_TEST
  m_mailBoxes[GetMailBoxNumber(*sourceIt, *nbr)]->Dump();
#endif

            break;
          }
        }
        nodesSentMessages[*sourceIt] = true;
      }
    }

#ifdef LIMID_TEST
    printf("\nm_decisionNodes\t");
    for (int i = 0; i < m_decisionNodes.size(); i++)
      printf("%d\t", m_decisionNodes[i]);

    printf("\nm_cliquesContDecisionNodes\t");
    for (i = 0; i < m_cliquesContDecisionNodes.size(); i++)
      printf("%d\t", m_cliquesContDecisionNodes[i]);
#endif

    m_CollectPotential = 
      dynamic_cast <CIDPotential*> (m_RetractPotential->Clone());
    pGraph->GetNeighbors(m_Roots[0], &numOfNbrs, &nbrs, &nbrsTypes);
    for (i = 0; i < numOfNbrs; i++)
    {
      int Num;
      Num = GetMailBoxNumber(m_Roots[0], nbrs[i]);

#ifdef LIMID_TEST
  printf("\nmailbox:\n");
  m_mailBoxes[Num]->Dump();
  printf("m_CollectPotential before Combine:\n");
  m_CollectPotential->Dump();
#endif

      tempCollectPotential = m_CollectPotential->Combine(m_mailBoxes[Num]);
      delete m_CollectPotential;
      m_CollectPotential = tempCollectPotential;

#ifdef LIMID_TEST
  printf("m_CollectPotential after Combine:\n");
  m_CollectPotential->Dump();
#endif
    }
    m_FirstIter = false;
  }
  else // other iterations
  {
//    int Size = (m_Roots).size();
    int NumOfPath = m_IterNum % ((m_Roots).size());
    // path number from root to root
    if (NumOfPath == 0)
      NumOfPath = (m_Roots).size() - 1;
    else
      NumOfPath--;

#ifdef LIMID_TEST
  printf("\nNumOfPath = %d", NumOfPath);
#endif

    if ((m_Roots).size() > 1)
    {
      
      for (int j = (m_Paths[NumOfPath].size()) - 1; j > 0; j--)
      {

#ifdef LIMID_TEST
  printf("\nPropagateBetweenClqs %d and %d ", m_Paths[NumOfPath][j],
    m_Paths[NumOfPath][j - 1]);
#endif

        PropagateBetweenClqs(m_Paths[NumOfPath][j], m_Paths[NumOfPath][j - 1]);

#ifdef LIMID_TEST
  m_mailBoxes[GetMailBoxNumber(m_Paths[NumOfPath][j], m_Paths[NumOfPath][j - 1])]->Dump();
#endif
      }
    }
    int NumOfRoot;
    NumOfRoot = NumOfPath + 1;
    if (NumOfRoot == m_Roots.size())
      NumOfRoot = 0;

    m_CollectPotential = 
      dynamic_cast <CIDPotential*> (m_RetractPotential->Clone());

// !!!
//    if (m_Paths[NumOfPath].size() == 0)
//      return;
// !!!

    pGraph->GetNeighbors((m_Roots)[NumOfRoot], &numOfNbrs, &nbrs, &nbrsTypes);
    for (i = 0; i <numOfNbrs; i++)
    {
      int Num = GetMailBoxNumber(m_Roots[NumOfRoot], nbrs[i]);

#ifdef LIMID_TEST
  printf("\nmailbox:\n");
  m_mailBoxes[Num]->Dump();
  printf("m_CollectPotential before Combine:\n");
  m_CollectPotential->Dump();
#endif

      tempCollectPotential = m_CollectPotential->Combine(m_mailBoxes[Num]);
      delete m_CollectPotential;
      m_CollectPotential = tempCollectPotential;

#ifdef LIMID_TEST
  printf("m_CollectPotential after Combine:\n");
  m_CollectPotential->Dump();
#endif
    }
  }

#ifdef LIMID_TEST
  printf("\n\n***** Collect End *****\n");
#endif
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::Marginalization()
{
#ifdef LIMID_TEST
  printf("\n***** Marginalization Start *****\n\n");
  m_CollectPotential->Dump();
#endif

  if (m_pMargFromFamilyPotential)
  {
    delete m_pMargFromFamilyPotential;
  }

  const int *domain;
  int domainSize;

  int curDesNode = m_decisionNodes[m_currIndexOfDecisionNode];
  m_pIDNet->GetFactor(curDesNode)->GetDomain(&domainSize, &domain);

  m_pMargFromFamilyPotential = 
    m_CollectPotential->Marginalize(domain, domainSize);

#ifdef LIMID_TEST
  m_pMargFromFamilyPotential->Dump();
  printf("\n***** Marginalization End *****\n\n");
#endif
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::Contraction()
{
  PNL_CHECK_IS_NULL_POINTER(m_pMargFromFamilyPotential);

#ifdef LIMID_TEST
  printf("\n***** Contraction Start *****\n\n");
  m_pMargFromFamilyPotential->Dump();
#endif

  if (m_pMatContr)
  {
    delete m_pMatContr;
  }
  m_pMatContr = static_cast <CNumericDenseMatrix<float>*>
    (m_pMargFromFamilyPotential->Contraction());

#ifdef LIMID_TEST
  int NumOfDims; 
  const int *pRanges;
  int data_length; 
  const float *data;

  m_pMatContr->GetRanges(&NumOfDims, &pRanges);
  m_pMatContr->GetRawData(&data_length, &data);
  
  printf("\ncontents of matrix after Contraction():\n");
  for (int i = 0; i < data_length; i++ )
    printf("   %7.3f", data[i]);
  printf("\n");
  printf("\n\n***** Contraction End *****\n");
#endif
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::Optimization()
{
  PNL_CHECK_IS_NULL_POINTER(m_pMatContr);
  
  int numClique;
  int currDecisionNode;
  int i, j;

  currDecisionNode = m_decisionNodes[m_currIndexOfDecisionNode];
  numClique = m_cliquesContDecisionNodes[m_currIndexOfDecisionNode];

  CFactor* policy = NULL;
  for (i = 0; i < m_decisionNodesOfClqs[numClique].size(); i++)
  {
    if (m_decisionNodesOfClqs[numClique][i] == currDecisionNode)
    {
      policy = m_DesNodePolitics[numClique][i];
      break;
    }
  }

  PNL_CHECK_IS_NULL_POINTER(policy);

#ifdef LIMID_TEST
  printf("\n***** Optimization Start *****\n\n");
  policy->GetDistribFun()->Dump();
  std::cout.flush();
#endif
  
  int NumStatesOfDecisionNode = 
    m_pIDNet->GetNodeType(currDecisionNode)->GetNodeSize();
  int NumFamConfigs = 1;
  intVector parents(0);

  m_pIDNet->GetGraph()->GetParents(currDecisionNode, &parents);
  for (i = 0; i < parents.size(); i++)
  {
    NumFamConfigs *= m_pIDNet->GetNodeType(parents[i])->GetNodeSize();
  }
  
  int indexOfMaxValue;

  int NumOfDims; 
  const int *pRanges;
  int data_length; 
  const float *data;

  m_pMatContr->GetRanges(&NumOfDims, &pRanges);
  m_pMatContr->GetRawData(&data_length, &data);
  float *NewPolicyData = new float[data_length];

  for (i = 0; i < data_length; i++)
    NewPolicyData[i] = 0.0;

//  m_exp = 0.0;
  for (i = 0; i < NumFamConfigs; i++)
  {
    indexOfMaxValue = 0;
    for (j = 1; j < NumStatesOfDecisionNode; j++)
    {
      if (data[i * NumStatesOfDecisionNode + j] >= 
          data[i * NumStatesOfDecisionNode + indexOfMaxValue])
        indexOfMaxValue = j;
    }
    NewPolicyData[i * NumStatesOfDecisionNode + indexOfMaxValue] = 1.0;
    // !!! fix me: remove next calculation
//    m_exp += data[i * NumStatesOfDecisionNode + indexOfMaxValue];
  }

#ifdef LIMID_TEST
  printf("\ncurrent expectation: %f\n", m_exp);
  std::cout.flush();
#endif

  CNumericDenseMatrix<float>* NewPolicyMatrix = 
    CNumericDenseMatrix<float>::Create(NumOfDims, pRanges, NewPolicyData);
  policy->AttachMatrix(NewPolicyMatrix, matTable);

#ifdef LIMID_TEST
  policy->GetDistribFun()->Dump();
  printf("\n\n***** Optimization End *****\n");
  std::cout.flush();
#endif

  delete [] NewPolicyData;
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::CalculateExpectation()
{
  int currDecisionNode = m_decisionNodes[m_currIndexOfDecisionNode];
  int numClique = m_cliquesContDecisionNodes[m_currIndexOfDecisionNode];
  int i;

  CFactor* policy = NULL;
  for (i = 0; i < m_decisionNodesOfClqs[numClique].size(); i++)
  {
    if (m_decisionNodesOfClqs[numClique][i] == currDecisionNode)
    {
      policy = m_DesNodePolitics[numClique][i];
      break;
    }
  }

  m_CollectPotential->InsertPoliticsInSelf(
    static_cast <const CCPD*> (policy));

  m_CollectPotential->GetProbDistribFun()->Normalize();

  CNumericDenseMatrix<float>* temp_matr = 
    static_cast <CNumericDenseMatrix<float>*>
    (m_CollectPotential->Contraction());

  int data_length;
  const float* data;
  temp_matr->GetRawData(&data_length, &data);

  m_exp = 0.0;
  for (i = 0; i < data_length; i++)
  {
    m_exp += data[i];
  }
}
//-----------------------------------------------------------------------------

int CLIMIDInfEngine::GetCluqueContFamilyOfNode(int Node)
{
  int i;
  intVector Temp;
  m_pIDNet->GetGraph()->GetParents(Node, &Temp);

#ifdef LIMID_TEST
  printf("\nNode %d \t", Node);
  for (i = 0; i < Temp.size(); i++)
    printf("%d\t", (Temp)[i]);
#endif

  intVector Family(0);
  Family.clear();
  for (i = 0; i < Temp.size(); i++)
    Family.push_back(Temp[i]);
  Family.push_back(Node);

#ifdef LIMID_TEST
  printf("\nNode %d \t", Node);
  for (i = 0; i < Family.size(); i++)
    printf("%d\t", (Family)[i]);
#endif

  int numOfNdsInSubset = Family.size();
  int numOfClqs;
  const int *clqsContSubset;
  m_pJTree->GetClqNumsContainingSubset(numOfNdsInSubset, &Family.front(), 
    &numOfClqs, &clqsContSubset);

#ifdef LIMID_TEST
  printf("\nfor Node %d \t", Node);
  for (i = 0; i < numOfClqs; i++)
    printf("%d\t", clqsContSubset[i]);
#endif

  if (numOfClqs == 0)
    return -1;

  return clqsContSubset[0];
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::BuildAllRootsOfTree()
{
#ifdef LIMID_TEST
  printf("\n***** BuildAllRootsOfTree Start *****\n\n");
#endif

  int i;
  m_Roots.resize(0);
  int NumOfNodes = m_pIDNet->GetNumberOfNodes();
  for (i = 0; i < NumOfNodes; i++)
  {
    if (m_pIDNet->GetNodeType(i)->GetNodeState() == 
      nsDecision)
    {
      int Num = GetCluqueContFamilyOfNode(i);
      if (Num == -1)
        PNL_THROW(CBadConst, " there is no clique, containing family ");
      m_Roots.push_back(Num);
    }
  }

#ifdef LIMID_TEST
  printf("\nRoots\t");
  for (i = 0; i < m_Roots.size(); i++)
    printf("%d\t", m_Roots[i]);
  printf("\n\n***** BuildAllRootsOfTree End *****\n");
#endif
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::BuildPathBetweenTwoNodes(int Node1, int Node2,
  intVector &Path)
{
  if (Node1 == Node2)
    PNL_THROW(CBadConst, " it is impossible to build path between two equal nodes");
 
  int j;
  int numOfNbrs;
  const int *nbrs;
  const ENeighborType *nbrsTypes;
  int NumberOfNodes = m_pJTree->GetNumberOfNodes();

  boolVector eliminated(NumberOfNodes);
  int *flags = new int [NumberOfNodes];
  memset(flags, -1, sizeof(int)*NumberOfNodes);
  
  intQueue m_QueueNodes;
  m_QueueNodes.push(Node1);
  flags[Node1] = Node1;
  eliminated[Node1] = true;

  int currNode;
  bool EndSearch = 0;

  while(!EndSearch)
  {
    currNode = m_QueueNodes.front();
    m_QueueNodes.pop();
    if (flags[currNode] != -1)
    {
      m_pJTree->GetGraph()->GetNeighbors(currNode, &numOfNbrs, &nbrs, 
        &nbrsTypes );
      for (j = 0; j < numOfNbrs; j++)
      {
        if (!eliminated[nbrs[j]])
        {
          m_QueueNodes.push(nbrs[j]);
          if (nbrs[j] == Node2)
            EndSearch = true;
          flags[nbrs[j]] = currNode;
          eliminated[nbrs[j]] = true;
        }
      }
    }
  }
  
  int Node = Node2;
  Path.push_back(Node);
  int i = 1;
  while (Node != Node1)
  {
    Path.push_back(flags[Node]);
    i++;
    Node = flags[Node];
  }

#ifdef LIMID_TEST
  printf("\nPath from %d to %d:\t", Node2, Node1);
  for (j = 0; j < Path.size(); j++)
    printf("%d\t", Path[j]);
#endif
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::BuildPathsBetweenRoots()
{
  if (m_Roots.size() == 1)
    return;

#ifdef LIMID_TEST
  printf("\n***** BuildPathsBetweenRoots Start *****\n\n");
#endif

  int i;
  int NumOfRoots = m_Roots.size();
  m_Paths.resize(NumOfRoots);
  for (i = 0; i < NumOfRoots - 1; i++)
  {
    if (m_Roots[i] != m_Roots[i + 1])
	  BuildPathBetweenTwoNodes(m_Roots[i], m_Roots[i + 1], m_Paths[i]);

#ifdef LIMID_TEST
  printf("\nPath from %d to %d:\t", m_Roots[i], m_Roots[i + 1]);
//  int Size = m_Paths[i].size();
  for (int j = 0; j < m_Paths[i].size(); j++)
    printf("%d\t", m_Paths[i][j]);
#endif

  }

  if (m_Roots[NumOfRoots - 1] != m_Roots[0])
    BuildPathBetweenTwoNodes(m_Roots[NumOfRoots - 1], m_Roots[0],
    m_Paths[NumOfRoots - 1]);

#ifdef LIMID_TEST
  printf("\nPath from %d to %d:\t", m_Roots[NumOfRoots - 1], m_Roots[0]);
//  int Size = m_Paths[NumOfRoots - 1].size();
  for (int j = 0; j < m_Paths[NumOfRoots - 1].size(); j++)
    printf("%d\t", m_Paths[NumOfRoots - 1][j]);
  printf("\n\n***** BuildPathsBetweenRoots End *****\n");
#endif
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::RebuildTreeFromRoot()
{
// find all the leaves of the JTree and push them to the first layer
// of the collect evidence sequence
  const int numOfNds = m_pJTree->GetNumberOfNodes();
  const CGraph* pGraph = m_pJTree->GetGraph();
  
  int numOfNbrs1, numOfNbrs2;
  const int *nbrs1, *nbrs2;
  const ENeighborType *nbrsTypes1, *nbrsTypes2;
  
  intVector collectSeqCurrLayer;
  boolVector checkedNodes(numOfNds, false);
  
  m_collectSequence.clear();
  m_collectSequence.reserve(numOfNds);
  collectSeqCurrLayer.reserve(numOfNds);
  
  int i;
  
  for (i = 0; i < numOfNds; i++)
  {
    pGraph->GetNeighbors(i, &numOfNbrs1, &nbrs1, &nbrsTypes1);
    
    // i-th node is a leaf if has one neighbor and not a root node
    if ((numOfNbrs1 == 1) && (i != m_JTreeRootNode))
    {
      collectSeqCurrLayer.push_back(i);
      checkedNodes[i] = true;
    }
  }
  
  m_collectSequence.push_back(collectSeqCurrLayer);
  collectSeqCurrLayer.clear();
  
  const int *nbr1, *nbr2, *nbrs_end1, *nbrs_end2;
  intVector::const_iterator nodeIt, layer_end;
  
  int numOfCheckedNbrs;
  
  for (intVecVector::const_iterator layerIt = m_collectSequence.begin();;)
  {
    for (nodeIt = layerIt->begin(), layer_end = layerIt->end();
      layer_end - nodeIt; ++nodeIt)
    {
      pGraph->GetNeighbors(*nodeIt, &numOfNbrs1, &nbrs1, &nbrsTypes1);
      
      for (nbr1 = nbrs1, nbrs_end1 = nbrs1 + numOfNbrs1; nbrs_end1 - nbr1;
        ++nbr1)
      {
        if (!checkedNodes[*nbr1])
        {
          pGraph->GetNeighbors(*nbr1, &numOfNbrs2, &nbrs2, &nbrsTypes2);
          
          numOfCheckedNbrs = 0;
          
          for (nbr2 = nbrs2, nbrs_end2 = nbrs2 + numOfNbrs2; nbrs_end2 - nbr2;
            ++nbr2)
          {
            if (checkedNodes[*nbr2])
            {
              ++numOfCheckedNbrs;
            }
          }
          
          if ((numOfCheckedNbrs == numOfNbrs2 - 1) &&
            (*nbr1 != m_JTreeRootNode))
          {
            collectSeqCurrLayer.push_back(*nbr1);
            checkedNodes[*nbr1] = true;
          }
        }
      }
    }
    
    // if the only one left is the root node, then the search is over
    if (collectSeqCurrLayer.empty())
    {
      collectSeqCurrLayer.push_back(m_JTreeRootNode);
      m_collectSequence.push_back(collectSeqCurrLayer);
      break;
    }
    
    // check all the nodes from the current layer
    m_collectSequence.push_back(collectSeqCurrLayer);
    layerIt = m_collectSequence.end() - 1;
    collectSeqCurrLayer.clear();
  }
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::PropagateBetweenClqs(int source, int sink)
{
  //Number of cliques in tree
  const int numOfNds = m_pJTree->GetNumberOfNodes();

  // bad-args check
  PNL_CHECK_RANGES(source, 0, numOfNds - 1);
  PNL_CHECK_RANGES(sink, 0, numOfNds - 1);
    
  // operation validity check
  if (source == sink)
  {
    PNL_THROW(CInvalidOperation, " source and sink should differ ");
  }
    
  if (!m_pJTree->GetGraph()->IsExistingEdge(source, sink))
  {
    PNL_THROW(CInvalidOperation,
      " there is no edge between source and sink ");
  }
  // operation validity check end

  // Combine clique number source potential with all it's neighbours but sink
  CIDPotential *combinedPotential = 
    dynamic_cast <CIDPotential*> (m_nodePots[source]->Clone());

  // Set in all politics
  int polSize = m_DesNodePolitics[source].size();
  for (int politic = 0; politic < polSize; politic++)
  {

#ifdef LIMID_TEST
  printf("\nSet in policy:\n");
  m_DesNodePolitics[source][politic]->GetDistribFun()->Dump();
#endif

    combinedPotential->InsertPoliticsInSelf(
      static_cast <const CCPD*> (m_DesNodePolitics[source][politic]));
  }
  
  intVector nbrsOut;
  neighborTypeVector nbrsTypesOut;
//  const ENeighborType *nbrsTypes;
  const CGraph *pGraph = m_pJTree->GetGraph();
  
  pGraph->GetNeighbors(source, &nbrsOut, &nbrsTypesOut);
  
  // for all source's neighbours ... 
  for (intVector::iterator it = nbrsOut.begin(); it != nbrsOut.end(); it++)
  {
    //... but sink 
    if (*it != sink)
    {
      //combine potential with neighbour *it
        CIDPotential *tempcombinedPotential;

        PNL_CHECK_IS_NULL_POINTER(m_mailBoxes[GetMailBoxNumber(*it, source)]);
        tempcombinedPotential = combinedPotential->Combine(
          m_mailBoxes[GetMailBoxNumber(*it,source)]);

        delete combinedPotential;
        combinedPotential = tempcombinedPotential;
    }
  }

  // Marginalize pot. combinedPotential 
  CIDPotential *marginalizedPotential = NULL;
  int domSize;
  const int *domain;

  m_pJTree->GetSeparatorDomain(source, sink, &domSize, &domain);

#ifdef LIMID_TEST
  printf("\nClique potential before Marg:\n");
  combinedPotential->Dump();
#endif

  marginalizedPotential = combinedPotential->Marginalize(domain, domSize);
  delete combinedPotential;

  // Send potential to the mailbox between source and sink cliques
  if (m_mailBoxes[GetMailBoxNumber(source, sink)] != NULL)
    delete m_mailBoxes[GetMailBoxNumber(source, sink)];
  m_mailBoxes[GetMailBoxNumber(source, sink)] = marginalizedPotential;
}
//-----------------------------------------------------------------------------

int CLIMIDInfEngine::GetMailBoxNumber(int clqFrom, int clqTo)
{
  const int numOfNds = m_pJTree->GetNumberOfNodes();

  //chack ranges
  PNL_CHECK_RANGES( clqFrom, 0, numOfNds-1 );
  PNL_CHECK_RANGES( clqTo, 0, numOfNds-1 );

  // operation validity check
  if (clqFrom == clqTo)
  {
    PNL_THROW(CInvalidOperation, " clqFrom and clqTo should differ ");
  }

  if (!m_pJTree->GetGraph()->IsExistingEdge(clqFrom, clqTo))
  {
    PNL_THROW(CInvalidOperation,
      " there is no edge between clqFrom and clqTo ");
  }

  //mailBox has number clqFrom or clqTo
  if (clqFrom < numOfNds-1)
  if (m_edgesDescription[clqFrom] == clqTo)
    return clqFrom;

  if (clqTo < numOfNds - 1)
  if (m_edgesDescription[clqTo] == clqFrom)
    return clqTo;

  PNL_THROW(CInternalError, 
    "Error in m_edgesDescription vector: there are no necessary mailBox");
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::GetCliquesNumbers(int mailBox, int &clqFrom, int &clqTo)
{
  const int numOfNds = m_pJTree->GetNumberOfNodes();

  //chack ranges
  PNL_CHECK_RANGES(mailBox, 0, numOfNds - 2);

  clqFrom = mailBox;
  clqTo = m_edgesDescription[mailBox];
}
//-----------------------------------------------------------------------------

void CLIMIDInfEngine::InitMailBoxes()
{
  int numOfNds = m_pJTree->GetNumberOfNodes();
  int minusOnePosition = -1;
  m_edgesDescription.resize(numOfNds, -1);

//  int source;
  for (int nds = 0; nds < numOfNds; nds++)
  {
    intVector nbrsOut;
    neighborTypeVector nbrsTypesOut;
//    const ENeighborType *nbrsTypes;
    const CGraph *pGraph = m_pJTree->GetGraph();
    pGraph->GetNeighbors(nds, &nbrsOut, &nbrsTypesOut);

    int nbr_size = nbrsOut.size();
    for (int nbr = 0; nbr < nbr_size; nbr++)
    {
      if (m_edgesDescription[nbrsOut[nbr]] != nds)
      {
        m_edgesDescription[nds] = nbrsOut[nbr];
        break;
      }
    } // for nbr

    if (m_edgesDescription[nds] == -1)
      minusOnePosition = nds;
  } // for nds

  if (minusOnePosition != numOfNds - 1)
  {
    intVector Path;
    BuildPathBetweenTwoNodes(numOfNds- 1 , minusOnePosition, Path);

    int pathLength = Path.size();

    for (int i = 0; i < pathLength-1; i++)
    {
      m_edgesDescription[Path[i]] = Path[i + 1];
    }
  }

  m_edgesDescription.resize(numOfNds - 1);

  m_mailBoxes.resize(numOfNds - 1);
  for (int mb = 0; mb < numOfNds - 1; mb++)
    m_mailBoxes[mb] = NULL;
}
//-----------------------------------------------------------------------------

pFactorVector* CLIMIDInfEngine::GetPolitics(void) const
{
  pFactorVector *Vec = new pFactorVector;

  Vec->resize(0);

  int numClique;
  int currDecisionNode;
  int i, j;

  for (i = 0; i < m_decisionNodes.size(); i++)
  {
    currDecisionNode = m_decisionNodes[i];
    numClique = m_cliquesContDecisionNodes[i];
    CFactor* policy;
    for (j = 0; j < m_decisionNodesOfClqs[numClique].size(); j++)
    {
      if (m_decisionNodesOfClqs[numClique][j] == currDecisionNode)
      {
        policy = m_DesNodePolitics[numClique][j];
        break;
      }
    }
    Vec->push_back(policy);
  }

  return Vec;
}
//-----------------------------------------------------------------------------

float CLIMIDInfEngine::GetExpectation()
{
  return m_exp;
}
//-----------------------------------------------------------------------------

int CLIMIDInfEngine::GetIterNum()
{
  return m_IterNum;
}
//-----------------------------------------------------------------------------

#ifdef PNL_RTTI
const CPNLType CLIMIDInfEngine::m_TypeInfo = CPNLType("CLIMIDInfEngine", &(CPNLBase::m_TypeInfo));

#endif
// end of file ----------------------------------------------------------------
