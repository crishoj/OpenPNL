/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlParPearlInferenceEngine.cpp                              //
//                                                                         //
//  Purpose:   CParPearlInfEngine class member functions implementation    //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlParPearlInferenceEngine.hpp"

#ifdef PAR_PNL

#include "pnlTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlMNet.hpp"
//#include "pnlAllocator.hpp"

#ifdef PAR_OMP
#include <omp.h>
#endif

#ifdef PAR_MPI
#include <mpi.h>
#endif

PNL_USING

#ifdef PAR_MPI
struct CEdgeValue
{
    int RootToInGraphSystem;       //root to in graph numeric system 
    int RootFromInSkeletonSystem;  //root from in skeleton numeric system
    double EdgeWeight;
    int Step;
/*
    Remark:
    In graph and skeleton systems nodes have different numbers. 
    According between systems sets by array m_pTreeGraphAccord
*/
    CEdgeValue()
    {
        RootFromInSkeletonSystem = RootToInGraphSystem = -1;
        EdgeWeight = -1;
        Step = -1;
    };
};
bool operator < (const CEdgeValue& lhs, const CEdgeValue& rhs)
{
    if (lhs.EdgeWeight < rhs.EdgeWeight)
	return true;
    else return false;
}
bool operator > (const CEdgeValue& lhs, const CEdgeValue& rhs)
{
    if (lhs.EdgeWeight > rhs.EdgeWeight)
        return true;
    else return false;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

CParPearlInfEngine* CParPearlInfEngine::Create(const CStaticGraphicalModel
  *pGraphicalModel)
{
  if(!pGraphicalModel)
  {
    PNL_THROW(CNULLPointer, "graphical model");
  }
  
  if((pGraphicalModel->GetModelType() != mtBNet) &&
    (pGraphicalModel->GetModelType() != mtMRF2))
  {
    PNL_THROW(CInconsistentType, "only for MRF2 & BNet")
  }
  
  CParPearlInfEngine* resInf = new CParPearlInfEngine(pGraphicalModel);
  PNL_CHECK_IF_MEMORY_ALLOCATED(resInf);

  return resInf;
}
// ----------------------------------------------------------------------------

CParPearlInfEngine::CParPearlInfEngine(const CStaticGraphicalModel 
  *pGraphicalModel):CPearlInfEngine(pGraphicalModel),
  m_productPi(GetNumberOfNodesInModel()), 
  m_productLambda(GetNumberOfNodesInModel()),
  m_factorDistrib(GetNumberOfNodesInModel()), m_index(0)
{
  m_IsMRF2Grid = false;
  const int numOfNdsInModel = GetNumberOfNodesInModel();
  m_bels[0].resize(numOfNdsInModel);
  m_bels[1].resize(numOfNdsInModel);
  m_msgsFromNeighbors[0].resize(numOfNdsInModel);
  m_msgsFromNeighbors[1].resize(numOfNdsInModel);
  messageVecVector::iterator
    mfn_begin = m_msgsFromNeighbors[0].begin(),
    mfn_end = m_msgsFromNeighbors[0].end(),
    mfnIt = mfn_begin,
    nmIt = m_msgsFromNeighbors[1].begin();
  
  const CGraph* pModelGraph = GetModelGraph();

  for(; mfnIt != mfn_end; ++mfnIt, ++nmIt)
  {
    int numOfNbrs = pModelGraph->GetNumberOfNeighbors(mfnIt - mfn_begin);
    
    mfnIt->resize( numOfNbrs, NULL );
    
    nmIt->resize( numOfNbrs, NULL );
  }

#ifdef PAR_MPI
  m_UsedNodes.clear();
  m_RootNode = -1;
  InitMPIConsts();
  m_NumberOfUsedProcesses = -1;
  m_pMyEvidence = NULL;
  InitWeightArrays();
  m_pSkeleton = NULL;
  m_pTreeGraphAccord = NULL;
  
  m_NeedToRebuildWeightesOfNodes = true;
  
  m_NodeProcesses.clear();
  m_NodesOfProcess.clear();
  m_Roots.clear();

  m_CollectRanks.resize(1);
  m_CollectRanks[0] = 0;
#endif // PAR_MPI

  if (m_pGraphicalModel->GetModelType() == mtMRF2)
  {
    int numOfNeighb;
    const int* neighbors;
    const ENeighborType* orientation;

    m_factorCliqueDistrib.resize(numOfNdsInModel);
    for (int i = 0; i < numOfNdsInModel; i++)
    {
      pModelGraph->GetNeighbors(i,  &numOfNeighb, &neighbors, &orientation);
      m_factorCliqueDistrib[i].resize(numOfNeighb + 1);
    }
  }
}
// ----------------------------------------------------------------------------

CParPearlInfEngine::~CParPearlInfEngine()
{
#ifdef PAR_MPI
  if (m_pWeightesOfNodes != NULL)
    delete m_pWeightesOfNodes;

  if (m_pWeightesOfSubTrees != NULL)
    delete m_pWeightesOfSubTrees;
  
  if (m_pSkeleton != NULL)
    delete m_pSkeleton;
  
  if (m_pTreeGraphAccord != NULL)
    delete m_pTreeGraphAccord;
#endif // PAR_MPI

  messageVector::iterator belIt = m_bels[0].begin(),
                          belIt1 = m_bels[1].begin(),
                          prodPiIt = m_productPi.begin(),
                          prodLamIt = m_productLambda.begin(),
                          bel_end = m_bels[0].end();
    
  messageVecVector::iterator  mfnIt = m_msgsFromNeighbors[0].begin(),
                              mfnIt1 = m_msgsFromNeighbors[1].begin();
  
  for(; belIt != bel_end; ++belIt, ++belIt1, ++prodPiIt, ++prodLamIt, 
    ++mfnIt, ++mfnIt1)
  {
    delete *belIt;
    *belIt = NULL;
    
    delete *belIt1;
    *belIt1 = NULL;

    delete *prodPiIt;
    *prodPiIt = NULL;

    delete *prodLamIt;
    *prodLamIt = NULL;

    messageVector::iterator messIt = mfnIt->begin(),
                            mess_end = mfnIt->end(),
                            messIt1 = mfnIt1->begin(); 

    for( ; messIt != mess_end; ++messIt, ++messIt1)
    {
      delete *messIt;
      *messIt = NULL;
      
      delete *messIt1;
      *messIt1 = NULL;
    }
  }
  switch (m_pGraphicalModel->GetModelType())
  {
    case mtBNet:
    {
      messageVector::iterator factDistIt = m_factorDistrib.begin(),
        factDistIt_end = m_factorDistrib.end();
      for( ; factDistIt != factDistIt_end; ++factDistIt)
      {
        delete *factDistIt;
        *factDistIt = NULL;
      }
      break;
    }
    case mtMRF2:
    {
      messageVecVector::iterator factClDistIt = m_factorCliqueDistrib.begin(),
        factClDistIt_end = m_factorCliqueDistrib.end();
      for( ; factClDistIt != factClDistIt_end; ++factClDistIt)
      {
        messageVector::iterator factIt = factClDistIt->begin(),
          fact_end = factClDistIt->end();
        for( ; factIt != fact_end; ++factIt)
        {
          delete *factIt;
          *factIt = NULL;
        }
      }
      break;
    }
  }
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::MarginalNodes(const int* query, int querySize,
  int notExpandJPD)
{
  if (notExpandJPD == 1)
  {
    PNL_THROW( CInconsistentType, 
      "pearl inference work with expanded distributions only" );
  }
  
  PNL_CHECK_LEFT_BORDER(querySize, 1);
  
  if (m_pQueryJPD)
  {
    delete m_pQueryJPD;
  }
  
  if (m_pEvidenceMPE)
  {
    delete m_pEvidenceMPE;
  }
  
  messageVector* tmpVec;
  if (!AllContinuousNodes(GetModel()))
    tmpVec = &GetCurBeliefs();
  else
    tmpVec = &CPearlInfEngine::GetCurBeliefs();
  messageVector& rfCurBeliefs = *tmpVec;

  CModelDomain* pMD = m_pGraphicalModel->GetModelDomain();
  if (querySize == 1)
  {
    if (m_bMaximize)
    {
      //compute MPE
      CNodeValues * values = rfCurBeliefs[query[0]]->GetMPE();
      CEvidence *evid = CEvidence::Create(values, querySize, query,
        m_pGraphicalModel->GetModelDomain());
      m_pEvidenceMPE = evid;
      delete values;
    }
    else
    {
      // get marginal for one node - cretae parameter on existing data - m_beliefs[query[0]];
      switch (m_modelDt)
      {
        case dtTabular:
        {
          m_pQueryJPD = CTabularPotential::Create(query, querySize, pMD);
          break;
        }
        case dtGaussian:
        {
          m_pQueryJPD = CGaussianPotential::Create( query, querySize,
            pMD);
          break;
        }
        default:
        {
          PNL_THROW( CNotImplemented, "only tabular & Gaussian now");
        }
      }
      //we need to add our beliefs here
      if(rfCurBeliefs[query[0]]->GetDistributionType() == dtGaussian)
      {
        static_cast<CGaussianDistribFun*>(rfCurBeliefs[query[0]])->
          UpdateMomentForm();
      }
      m_pQueryJPD->SetDistribFun( rfCurBeliefs[query[0]]->
        GetNormalized());
    }
  }
  else
  {
    int numParams;
    CFactor ** params;
    m_pGraphicalModel->GetFactors(querySize, query,
      &numParams ,&params);
    if (!numParams)
    {
      PNL_THROW(CBadArg, 
        "only members of one family can be in query instead of one node")
    }
    int i;
    // get informatiom from parameter on these nodes to create new parameter
    // with updated Data
    EDistributionType dt = params[0]->GetDistributionType();
    message allProduct = 
      params[0]->GetDistribFun()->ConvertCPDDistribFunToPot();
    int domLength;
    const int *domain;
    params[0]->GetDomain(&domLength, &domain);
    CGraph *graph = m_pGraphicalModel->GetGraph();
    int lastNodeInDomain = domain[domLength - 1];
    // get neighbors of last node in domain (child for CPD) 
    // to compute JPD for his family
    const int *nbrs;
    const ENeighborType *orient;
    int numNbrs;
    graph->GetNeighbors(lastNodeInDomain, &numNbrs, &nbrs, &orient);
    int smallDom;
    int *domPos = new int [domLength];
    PNL_CHECK_IF_MEMORY_ALLOCATED(domPos);
    for(i = 0; i < domLength; i++)
    {
      domPos[i] = i;
    }
    int location;
    //start multiply to add information after inference
    for(i = 0; i < numNbrs; i++)
    {
      location = std::find(domain, domain + domLength, nbrs[i]) - domain;
      if(location >= domLength)
      {
        break;
      }
      smallDom = location;
      allProduct->MultiplyInSelfData(domPos, &smallDom, 
        (*GetCurMsgsFromNeighbors())[domain[domLength - 1]][i]);
    }
    //add self-information
    messageVector& selfMessages = GetSelfMessages();
    smallDom = domLength - 1;
    allProduct->MultiplyInSelfData(domPos, &smallDom,
      selfMessages[lastNodeInDomain]);
    CDistribFun* normalizedData = allProduct->GetNormalized();
    if (normalizedData->GetDistributionType() == dtGaussian)
    {
      static_cast<CGaussianDistribFun*>(normalizedData)->UpdateMomentForm();
    }
    //create parameter on this nodes
    CPotential* pQueryJPD = NULL;
    switch (dt)
    {
      case dtTabular:
      {
        pQueryJPD = CTabularPotential::Create(domain, domLength, pMD);
        break;
      }
      case dtGaussian:
      {
        pQueryJPD = CGaussianPotential::Create(domain, domLength, pMD);
        break;
      }
      default:
      {
        PNL_THROW(CNotImplemented, "only tabular & Gaussian now");
      }
    }
    // we need to add data to this Parameter
    pQueryJPD->SetDistribFun(normalizedData);
    m_pQueryJPD = pQueryJPD->Marginalize(query, querySize, m_bMaximize);
    delete pQueryJPD;
    if(m_bMaximize)
    {
      //compute MPE
      CNodeValues * values = m_pQueryJPD->GetDistribFun()->GetMPE();
      CEvidence *evid = CEvidence::Create(values, domLength, domain,
        m_pGraphicalModel->GetModelDomain());
      m_pEvidenceMPE = evid;
      delete values;
      delete m_pQueryJPD;
      m_pQueryJPD = NULL;
    }
    delete normalizedData;
    delete []domPos;
  }
}
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::InitMPIConsts()
{
  MPI_Comm_size(MPI_COMM_WORLD, &m_NumberOfProcesses);
  MPI_Comm_rank(MPI_COMM_WORLD, &m_MyRank);
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::InitWeightArrays()
{
  m_pWeightesOfNodes = NULL;
  m_pWeightesOfSubTrees = NULL;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

void CParPearlInfEngine::Release(CParPearlInfEngine **PearlInfEngine)
{
  delete *PearlInfEngine;
  *PearlInfEngine = NULL;
}
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::EnterEvidence(const CEvidence *evidence, int maximize,
  int sumOnMixtureNode)
{
  PNL_CHECK_IS_NULL_POINTER(evidence);
  // divide nodes between processes
  if (!IsModelGrille())
    DivideNodes(evidence, 2);
  else 
    DivideGrille();
  m_bMaximize = maximize ? 1 : 0;
  // clear all the previous data
  ClearMessagesStorage();
  if (!AllContinuousNodes(GetModel()))
  {
    // init new data
    InitEngine(evidence);
    // initialize messages
    InitMessages(evidence);
    // start inference

    ParallelProtocol();

    for(int i = 0; i < m_CollectRanks.size(); i++)
        CollectBeliefsOnProcess(m_CollectRanks[i]);
  }
  else
  {
    // init new data
    CPearlInfEngine::InitEngine(evidence);
    // initialize messages
    CPearlInfEngine::InitMessages(evidence);
    // start inference
    ParallelProtocolContMPI();

    for(int i = 0; i < m_CollectRanks.size(); i++)
        CollectBeliefsOnProcessContMPI(m_CollectRanks[i]);
  }

}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParPearlInfEngine::EnterEvidenceOMP(const CEvidence *evidence,
  int maximize, int sumOnMixtureNode)
{
  PNL_CHECK_IS_NULL_POINTER(evidence);
  m_bMaximize = maximize ? 1 : 0;
  if (!AllContinuousNodes(GetModel()))
  {
    // clear all the previous data
    ClearMessagesStorage();
    // init new data
    InitEngine(evidence);
    // initialize messages
    InitMessages(evidence);
    // start inference
    ParallelProtocolOMP();
  }
  else
  {
    // clear all the previous data
    CPearlInfEngine::ClearMessagesStorage();
    // init new data
    CPearlInfEngine::InitEngine(evidence);
    // initialize messages
    CPearlInfEngine::InitMessages(evidence);
    // start inference
    ParallelProtocolContOMP();
  }
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::SetCollectRanks(int Count, int *pCollectRanks)
{
  m_CollectRanks.resize(Count);
  m_CollectRanks.assign(pCollectRanks, pCollectRanks + Count);
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::DivideNodes(const CEvidence *pEvidence, int AlgType)
{
  m_pMyEvidence = pEvidence;
  
  //Pick out skeleton from original graph model	
  PickOutSkeleton();
  
  if (AlgType == 2)
  {
    DivideNodesWithMinimumDeviationSearch(pEvidence);
    return;
  }
  
  int NumberOfNodes;
  int p = m_NumberOfProcesses; //number of processes
  int CurrentRoot;
  int PrevCurrentRoot;  //if undefined, -1
  double W; //Weight of tree
  double WThreshold; //threshold
  int A; //Root of subtree, which we will send to the next processor
  intVector Parents;
  int LastUsedProcess = -1;
  bool FunctionStillRunning = true;
  bool NeedToSendASubTreeToProcess = false;
  
  list<int> NodesList;
  list<int>::const_iterator NodesListIterator;
  
  NumberOfNodes = GetNumberOfNodes();
  
  m_UsedNodes.clear();
  
  //set some params 
  m_NodeProcesses.resize(NumberOfNodes, -1);
  m_NodesOfProcess.clear();
  
  SetNodesList(&NodesList);
  NodesListIterator = NodesList.begin();
  
  CurrentRoot = PrevCurrentRoot = -1;
  
  m_Roots.clear();
  if (m_NumberOfProcesses > 0) 
    m_Roots.resize(m_NumberOfProcesses, -1);
  else
  {
    PNL_CHECK_LEFT_BORDER(m_NumberOfProcesses, 1);
  }
  
  if (p > 0)
  {
    // check if tree is empty
    if (NumberOfNodes!=0)
    {
      GetNextNodeInList(&NodesList, &NodesListIterator, &CurrentRoot, 
        &PrevCurrentRoot);
      
      W = GetWeightOfSubTree(GetRootNode());
      WThreshold = W / p;
      
      //label 1 
      while (FunctionStillRunning)
      {
        if (GetWeightOfSubTree(CurrentRoot) >= WThreshold)
        {
          NeedToSendASubTreeToProcess = true;
        }
        else
        {
          Parents.clear();
          GetParentsInSkeleton(CurrentRoot, &Parents);
          
          if (Parents.size() != 0)
          {
            PrevCurrentRoot = CurrentRoot;
            CurrentRoot = Parents[0];
            NeedToSendASubTreeToProcess = false;
          }
          else //if (Parents.size()!=0...
          {
            A = CurrentRoot;
            NeedToSendASubTreeToProcess = true;
          }
        }
        
        if (NeedToSendASubTreeToProcess)
        {
          switch (AlgType)
          {
          case 0: //Search OptimalSubtree in CurrentRoot and PrevCurrentRoot variants
            A = GetNodeWithOptimalSubTreeWeight(CurrentRoot, PrevCurrentRoot, 
              WThreshold);
            break;
          default:
            A = GetNodeWithOptimalSubTreeWeight(CurrentRoot, WThreshold);
            break;
          };
          
          SendASubTreeToNextProcess(A, LastUsedProcess);
          LastUsedProcess++;
          
          W -= GetWeightOfSubTree(A);
          p--;
          
          AddNodesNumbersOfSubTreeToUsedNodes(A);
          
          BuildWeightesOfNodes();
          BuildWeightesOfSubTrees();
          
          if (GetNumberOfFreeNodesInList(&NodesList, &NodesListIterator) >0 )
          {
            if (p != 0)
            {
              WThreshold = W / p;
              GetNextNodeInList(&NodesList, &NodesListIterator, &CurrentRoot,
                &PrevCurrentRoot);
            }
            else
              PNL_THROW(CInternalError, 
                "Number of processes in CParPearlInfEngine::DivideNodes <=0 when node list is not empty")
          }
          else 
          {
            FunctionStillRunning = false;
          }
        } // if (Need...)
      } // while 
    } // if (NumberOfNodes!=...)
    else
    {
      PNL_THROW(COutOfRange, "CParPearlInfEngine::DivideNodes. Tree is empty.")
    }
  }
  else //if (p!=0...
  {
    PNL_THROW(COutOfRange, 
      "CParPearlInfEngine::DivideNodes. Number of processes is 0.");
  }
  m_NumberOfUsedProcesses = LastUsedProcess + 1;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::PickOutSkeleton()
{
  //Model and it's graph
  const CStaticGraphicalModel* pMyModel;
  CGraph *pMyGraph;
  //NumberOfNodes in graph
  int NumberOfNodes; 
  //Root to add in pTreeGraph
  int CurrentRoot;
  //Bool vector: is node in graph inserted in skeleton
  boolVector InsertedNodes;
  int i;
  
  if (m_pSkeleton == NULL)
    m_pSkeleton = CGraph::Create(0, NULL);
  else
  {
    delete m_pSkeleton;
    m_pSkeleton = CGraph::Create(0, NULL);
  }

  pMyModel = GetModel();
  PNL_CHECK_IS_NULL_POINTER(pMyModel);
  
  pMyGraph = pMyModel->GetGraph();
  PNL_CHECK_IS_NULL_POINTER(pMyGraph);
  
  NumberOfNodes = pMyGraph->GetNumberOfNodes();
  
  if (NumberOfNodes <= 0)
    PNL_THROW(CInternalError, 
      "CParPearlInfEngine::PickOutSkeleton. Graph is empty");
  
  CurrentRoot = 0;
  
  InsertedNodes.resize(NumberOfNodes, false);
  
  m_pSkeleton->ClearGraph();

  // list of structes EdgeValue, keeps all neighbours of all nodes in graph
  // that not in tree
  std::list <CEdgeValue> ValuesList;
  ValuesList.clear();
  
  // numbers in graph system array is needed to understand is some node
  // a neighbour of CurrentRoot - 1 node (last)
  //(it is true if element of array with index "number of neighbour" has
  // Step == CurrentRoot)
  std::vector <CEdgeValue> IsValueCurrentNeighbour;
  IsValueCurrentNeighbour.resize(NumberOfNodes, CEdgeValue());
  
  //numbers in graph system
  //neighbours of CurrentRoot - 1 (last) node, that not in tree
  std::vector <CEdgeValue> CurrentNeighbour;
  CurrentNeighbour.clear();

  //add 1 node
  m_pSkeleton->AddNodes(1);
  
  if (m_pTreeGraphAccord == NULL)
    m_pTreeGraphAccord = new intVector;

  m_pTreeGraphAccord->clear();
  m_pTreeGraphAccord->resize(NumberOfNodes, -1);
  
  (*m_pTreeGraphAccord)[CurrentRoot] = CurrentRoot;
  InsertedNodes[CurrentRoot] = true;
  CurrentRoot++;
  
  intVector NeighborsOut;
  neighborTypeVector NeighborsTypeVector;

  NeighborsOut.clear();
  pMyGraph->GetNeighbors((*m_pTreeGraphAccord)[CurrentRoot - 1], &NeighborsOut,
    &NeighborsTypeVector);

  int NumberOfNeighbors = NeighborsOut.size();

  CurrentNeighbour.clear();

  //we set arrays CurrentNeighbour and IsValueCurrentNeighbour with
  //using NeighbourOut and InsertedNodes
  CEdgeValue EdgeValue;
  for (i = 0; i<NumberOfNeighbors; i++)
    if (!InsertedNodes[NeighborsOut[i]])
    {
      EdgeValue.RootFromInSkeletonSystem = CurrentRoot - 1;
      EdgeValue.RootToInGraphSystem = NeighborsOut[i];
      EdgeValue.Step = CurrentRoot;
      EdgeValue.EdgeWeight = 
        GetWeightOfEdge((*m_pTreeGraphAccord)[CurrentRoot-1], NeighborsOut[i]);
      
      CurrentNeighbour.push_back(EdgeValue);
      IsValueCurrentNeighbour[EdgeValue.RootToInGraphSystem] = EdgeValue;
    }

  sort(CurrentNeighbour.begin(), CurrentNeighbour.end());

  int NumberOfCurrentNeighbors = CurrentNeighbour.size();
  for (i = 0; i < NumberOfCurrentNeighbors; i++)
    ValuesList.push_back(CurrentNeighbour[i]);
  
  //Is there any nodes not included in m_pSkeleton in graph
  while (NumberOfNodes != CurrentRoot)
  {
    m_pSkeleton->AddNodes(++CurrentRoot);
    m_pSkeleton->AddEdge(ValuesList.front().RootFromInSkeletonSystem,
      CurrentRoot - 1, 1);
    InsertedNodes[ValuesList.front().RootToInGraphSystem] = true;
    (*m_pTreeGraphAccord)[CurrentRoot - 1] = 
      ValuesList.front().RootToInGraphSystem;
    ValuesList.pop_front();
    
    NeighborsOut.clear();
    pMyGraph->GetNeighbors((*m_pTreeGraphAccord)[CurrentRoot - 1],
      &NeighborsOut, &NeighborsTypeVector);
    
    int NumberOfNeighbors = NeighborsOut.size();
    
    CurrentNeighbour.clear();
    
    CEdgeValue EdgeValue;
    for (i = 0; i<NumberOfNeighbors; i++) 
      if (!InsertedNodes[NeighborsOut[i]])
      {
        EdgeValue.RootFromInSkeletonSystem = CurrentRoot - 1;
        EdgeValue.RootToInGraphSystem = NeighborsOut[i];
        EdgeValue.Step = CurrentRoot;
        EdgeValue.EdgeWeight = GetWeightOfEdge((*m_pTreeGraphAccord)[
          CurrentRoot-1], NeighborsOut[i]);
        
        CurrentNeighbour.push_back(EdgeValue);
        IsValueCurrentNeighbour[EdgeValue.RootToInGraphSystem] = EdgeValue;
      }
      sort(CurrentNeighbour.begin(), CurrentNeighbour.end());

      std::list<CEdgeValue>::iterator ListIt;
      //iterator in ValuesList
      std::vector<CEdgeValue>::iterator ArrayIt;
      //iterator in CurrentNeighbour
      
      ListIt = ValuesList.begin();
      ArrayIt = CurrentNeighbour.begin();
      
      if (ArrayIt != CurrentNeighbour.end())
      {
        while (ListIt != ValuesList.end())
        {
          while ((ArrayIt != CurrentNeighbour.end()) && 
            (ListIt->EdgeWeight > ArrayIt->EdgeWeight))
          {
            if (IsValueCurrentNeighbour[ArrayIt->RootToInGraphSystem].Step ==
              CurrentRoot)
            {
              ListIt = ValuesList.insert(ListIt, *ArrayIt);
              ListIt++;
            };
            ArrayIt++;
          } //while ((ArrayIt != CurrentNeighbour.end
          
          if (IsValueCurrentNeighbour[ListIt->RootToInGraphSystem].Step == 
            CurrentRoot)
          {
            if (ListIt->EdgeWeight <=
              IsValueCurrentNeighbour[ListIt->RootToInGraphSystem].EdgeWeight)
            {
              IsValueCurrentNeighbour[ListIt->RootToInGraphSystem].Step = -1;
            }
            else
            {
              ListIt = ValuesList.erase(ListIt);
            }
          } //if (IsValueCurrentNeighbour[ListIt->RootToInG
          
          if (ListIt != ValuesList.end())
            ListIt++;
        } //while (ListIt != ValuesList.
        
        while (ArrayIt!=CurrentNeighbour.end())
        {
          if (IsValueCurrentNeighbour[ArrayIt->RootToInGraphSystem].Step == 
            CurrentRoot)
          {
            ValuesList.insert(ListIt, *ArrayIt);
            ListIt = ValuesList.end();
          }
          ArrayIt++;
        }
      } //if (ArrayIt != CurrentNeig
  } //While (NumberOfNodes !=...  
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

int CParPearlInfEngine::GetNumberOfNodes()
{
  const CStaticGraphicalModel *pMyModel;
  CGraph *pMyGraph;
  
  pMyModel = GetModel();
  pMyGraph = pMyModel->GetGraph();
  
  return pMyGraph->GetNumberOfNodes();
}
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::SetNodesList(list<int> *pNodesList)
{
  bool FunctionStillRunning = true;
  
  if (pNodesList==NULL)
  {
    PNL_CHECK_IS_NULL_POINTER(pNodesList)
  }
  
  set<int> CurrentLevelNodes;
  set<int>::iterator CurrentLevelNodesIterator;
  set<int> NextLevelNodes;
  
  intVector Children;
  Children.clear();
  
  CurrentLevelNodes.clear();
  CurrentLevelNodes.insert(GetRootNode());
  
  NextLevelNodes.clear();
  
  pNodesList->clear();
  
  while (FunctionStillRunning)
  {
    //set NextLevelNodes
    CurrentLevelNodesIterator = CurrentLevelNodes.begin();
    
    NextLevelNodes.clear();
    
    while (CurrentLevelNodesIterator!=CurrentLevelNodes.end())
    {
      GetChildrenInSkeleton(*CurrentLevelNodesIterator, &Children);
      
      NextLevelNodes.insert(Children.begin(), Children.end());
      
      pNodesList->push_front(*CurrentLevelNodesIterator);
      
      CurrentLevelNodesIterator++;
    }
    
    if (NextLevelNodes.size() != 0)
    {
      FunctionStillRunning = true;
      CurrentLevelNodes = NextLevelNodes;
    }
    else 
      FunctionStillRunning = false;
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::GetNextNodeInList(const list <int> *pNodesList,
  list <int>::const_iterator *pNodesListIterator, int *pCurrentRoot,
  int *pPrevCurrentRoot)
{
  *pPrevCurrentRoot = -1;
  
  while ((*pNodesListIterator != pNodesList->end()) && 
    (m_UsedNodes.count(**pNodesListIterator)))
  {
    (*pNodesListIterator)++;
  }
  
  if (*pNodesListIterator != pNodesList->end())
  {
    *pCurrentRoot = **pNodesListIterator;
    (*pNodesListIterator)++;
  }
  else
  {
    *pCurrentRoot = -1;
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
double CParPearlInfEngine::GetWeightOfSubTree(int NumOfNode) 
{
  if (m_pWeightesOfSubTrees!=NULL)
    return (*m_pWeightesOfSubTrees)[NumOfNode];
  else
  {
    BuildWeightesOfSubTrees();
    return (*m_pWeightesOfSubTrees)[NumOfNode];
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
double CParPearlInfEngine::GetWeightOfSubTreeWithReCalculating(
  int NumOfNodeInGraph, doubleVector *pWeightArray, 
  bool NeedToCountUpUsedNodes)
{
  bool NeedToDeleteArray = false;
  
  if (pWeightArray == NULL)
  {
    pWeightArray = new doubleVector;
    pWeightArray->clear();
    pWeightArray->resize(GetNumberOfNodes(), -1);
    NeedToDeleteArray = true;
  }
  
  double Result;
  Result = 0;
  intVector Children;
  intVector::iterator ChildrenIterator;
  
  bool IsFunctionFirst = false;
  if (m_NeedToRebuildWeightesOfNodes)
  {
    BuildWeightesOfNodes();
    IsFunctionFirst = true;
    m_NeedToRebuildWeightesOfNodes = false;
  }
  
  if ((*pWeightArray)[NumOfNodeInGraph] != -1)
  {
    if (NeedToDeleteArray) 
      delete pWeightArray;
    
    if (IsFunctionFirst) 
      m_NeedToRebuildWeightesOfNodes = true;
    
    return (*pWeightArray)[NumOfNodeInGraph];
  }
  else
  {
    if (NeedToCountUpUsedNodes)
      Result += (*m_pWeightesOfNodes)[NumOfNodeInGraph];
    else 
      Result += GetWeightOfNodeWithReCalculating(NumOfNodeInGraph, 
        NeedToCountUpUsedNodes);
    
    GetChildrenInSkeleton(NumOfNodeInGraph, &Children);
    
    if (Children.size()!= 0)
    {
      ChildrenIterator = Children.begin();
      
      while (ChildrenIterator!=Children.end())
      {
        Result += GetWeightOfSubTreeWithReCalculating(*ChildrenIterator,
          pWeightArray, NeedToCountUpUsedNodes);
        
        ChildrenIterator++;
      }
    }
    
    (*pWeightArray)[NumOfNodeInGraph] = Result;
    
    if (NeedToDeleteArray) 
      delete pWeightArray;
    
    if (IsFunctionFirst) 
      m_NeedToRebuildWeightesOfNodes = true;
    
    return Result;
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParPearlInfEngine::GetRootNode()
{
  if (m_RootNode != -1)
    return m_RootNode;
  
  m_RootNode = 0;
  intVector Parents;
  Parents.clear();
  bool FunctionStillRunning = true;
  
  while (FunctionStillRunning)
  {
    GetParentsInSkeleton(m_RootNode, &Parents);
    
    if (Parents.size() == 0) 
      FunctionStillRunning = false;
    else
    {
      m_RootNode = Parents[0];
      Parents.clear();
    }
  }
  
  return m_RootNode;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::GetParentsInSkeleton(int NumOfNodeInGraph,
  intVector *pParents)
{
  int NumberOfNodes = GetNumberOfNodes();
  
  PNL_CHECK_IS_NULL_POINTER(pParents);
  PNL_CHECK_RANGES(NumOfNodeInGraph, 0, NumberOfNodes - 1);
  
  int NumOfNodeInSkeleton = NumberFromGraphToSkeleton(NumOfNodeInGraph);
  int NumberOfParents = -1;
  
  m_pSkeleton->GetParents(NumOfNodeInSkeleton, pParents);
  
  NumberOfParents = pParents->size();
  
  for (int i = 0; i<NumberOfParents; i++)
  {
    pParents->operator[](i) = 
      NumberFromSkeletonToGraph(pParents->operator[](i));
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParPearlInfEngine::GetNodeWithOptimalSubTreeWeight(int CurrentRoot,
  int PrevCurrentRoot, double WThreshold)
{
  if (PrevCurrentRoot == -1)
    if (CurrentRoot != -1)
      return CurrentRoot;
    else 
      PNL_THROW(COutOfRange, 
        "CParPearlInfEngine::DivideNodes. CurrentRoot must be != -1.");
    
  double CurrentDivergence = 
    fabs(GetWeightOfSubTree(CurrentRoot) - WThreshold);
  double PrevDivergence = 
    fabs(GetWeightOfSubTree(PrevCurrentRoot) - WThreshold);
  
  if (CurrentDivergence <= PrevDivergence)
    return CurrentRoot;
  else 
    return PrevCurrentRoot;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParPearlInfEngine::GetNodeWithOptimalSubTreeWeight(int CurrentRoot,
  double WThreshold)
{
  PNL_CHECK_RANGES(CurrentRoot, 0, GetNumberOfNodes() - 1);
  if (WThreshold <= 0)
  {
    PNL_THROW(COutOfRange, 
      "CParPearlInfEngine::GetNodeWithOptimalSubTreeWeight. WThreshold must be > 0.");
  }
  
  intVector Children;
  intVector::iterator ChildrenIterator;
  GetChildrenInSkeleton(CurrentRoot, &Children);
  ChildrenIterator = Children.begin();
  
  int NodeWithMinDeviation = CurrentRoot;
  double MinDeviation = fabs(GetWeightOfSubTree(CurrentRoot) - WThreshold);
  double Deviation;
  
  while (ChildrenIterator != Children.end())
  {
    Deviation = fabs(GetWeightOfSubTree(*ChildrenIterator)- WThreshold);
    
    if (Deviation < MinDeviation)
    {
      NodeWithMinDeviation = *ChildrenIterator;
      MinDeviation = Deviation;
    }
    
    ChildrenIterator++;
  }
  
  return NodeWithMinDeviation;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::SendASubTreeToNextProcess(int A, int LastUsedProcess)
{
  intVector UnusedNodesInSubTree;
  intVector::iterator UnusedNodesInSubTreeIterator;
  UnusedNodesInSubTree.clear();
  
  int UsingProcess = LastUsedProcess + 1;
  
  GetUnusedNodesInSubTree(A, &UnusedNodesInSubTree);
  
  m_Roots[UsingProcess] = A;
  
  if (UsingProcess == m_MyRank) 
    m_NodesOfProcess = UnusedNodesInSubTree;
  
  UnusedNodesInSubTreeIterator = UnusedNodesInSubTree.begin();
  
  while (UnusedNodesInSubTreeIterator!= UnusedNodesInSubTree.end())
  {
    m_NodeProcesses[*UnusedNodesInSubTreeIterator] = UsingProcess;
    UnusedNodesInSubTreeIterator++;
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::AddNodesNumbersOfSubTreeToUsedNodes(int A)
{
  intVector UnusedNodesInSubTree; 
  UnusedNodesInSubTree.clear();
  
  GetUnusedNodesInSubTree(A, &UnusedNodesInSubTree);
  
  m_UsedNodes.insert(UnusedNodesInSubTree.begin(), UnusedNodesInSubTree.end());
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::BuildWeightesOfNodes()
{
  // number of nodes in graph
  int NumberOfNodes = GetNumberOfNodes();
  
  if (m_pWeightesOfNodes == NULL)
    m_pWeightesOfNodes = new doubleVector;
  
  m_pWeightesOfNodes->clear();
  m_pWeightesOfNodes->resize(NumberOfNodes, 0);
  
  for (int i = 0; i<NumberOfNodes; i++)
  {
    if (!m_UsedNodes.count(i))
      (*m_pWeightesOfNodes)[i] = GetWeightOfNodeWithReCalculating(i, false);
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::BuildWeightesOfSubTrees()
{
  // number of nodes in graph
  int NumberOfNodes = GetNumberOfNodes();
  
  if (m_pWeightesOfSubTrees == NULL)
    m_pWeightesOfSubTrees = new doubleVector;
  
  m_pWeightesOfSubTrees->clear();
  m_pWeightesOfSubTrees->resize(NumberOfNodes,0);
  
  //if (m_pWeightesOfNodes == NULL)
  BuildWeightesOfNodes();
  
  doubleVector WeightArray;
  WeightArray.clear();
  WeightArray.resize(NumberOfNodes, -1);
  
  if (NumberOfNodes>0)
  {
    (*m_pWeightesOfSubTrees)[0] = GetWeightOfSubTreeWithReCalculating(0,
      &WeightArray, true);
    
    for(int i = 1; i < NumberOfNodes; i++) 
      (*m_pWeightesOfSubTrees)[i] = WeightArray[i];
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParPearlInfEngine::GetNumberOfFreeNodesInList(const list<int> *pNodesList,
  list<int>::const_iterator *pNodesListIterator)
{
  int NumberOfUnfreeNodesInList = 0;
  
  list<int>::const_iterator TempNodesListIterator = *pNodesListIterator;
  
  int NodesListSizeSinceIterator = 0;
  
  while (TempNodesListIterator!=pNodesList->end())
  {
    if (m_UsedNodes.count(*TempNodesListIterator) > 0) 
      NumberOfUnfreeNodesInList++;
    
    NodesListSizeSinceIterator++;
    TempNodesListIterator++;
  }
  
  return NodesListSizeSinceIterator - NumberOfUnfreeNodesInList;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
double CParPearlInfEngine::GetWeightOfEdge(int RootFrom, int RootTo)
{
  const CStaticGraphicalModel* pMyModel;
  CGraph *pMyGraph;
  
  int NumberOfNodes;
  
  pMyModel = GetModel();
  
  PNL_CHECK_IS_NULL_POINTER(pMyModel);
  
  pMyGraph = pMyModel->GetGraph();
  
  PNL_CHECK_IS_NULL_POINTER(pMyGraph);
  
  NumberOfNodes = GetNumberOfNodes();
  
  if (!((RootFrom >= 0) && (RootTo >= 0) && (RootFrom < NumberOfNodes) && 
    (RootTo < NumberOfNodes)))
  {
    PNL_THROW(COutOfRange, 
      "CParPearlInfEngine::GetWeightOfEdge. Bad args: RootTo, RootFrom.");
  }
  
  if (!((pMyGraph->IsExistingEdge(RootFrom, RootTo)) || 
    (pMyGraph->IsExistingEdge(RootTo, RootFrom))))
    return -1;
  
  return GetWeightOfNodeWithReCalculating(RootTo, false);
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::GetChildrenInSkeleton(int NumOfNodeInGraph, 
  intVector* pChildren)
{
  int NumberOfNodes = GetNumberOfNodes();
  
  PNL_CHECK_IS_NULL_POINTER(pChildren);
  PNL_CHECK_RANGES(NumOfNodeInGraph, 0, NumberOfNodes - 1);
  
  int NumOfNodeInSkeleton = NumberFromGraphToSkeleton(NumOfNodeInGraph);
  int NumberOfChildren = -1;
  
  m_pSkeleton->GetChildren(NumOfNodeInSkeleton, pChildren);
  
  NumberOfChildren = pChildren->size();
  
  for (int i = 0; i<NumberOfChildren; i++)
  {
    pChildren->operator[](i) = 
      NumberFromSkeletonToGraph(pChildren->operator[](i));
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
double CParPearlInfEngine::GetWeightOfNodeWithReCalculating(int NumOfNode,
  bool NeedToCountUpUsedNodes) 
{
  if (NeedToCountUpUsedNodes&&m_UsedNodes.count(NumOfNode))
    return 0;
  
  int NumOfMsgsFromParents;
  int NumOfMsgsFromChildren;
  int NumberOfChildren;
  int NumberOfParents;
  int NumOfMsgsToParents;
  int NumOfMsgsToChildren;
  int PowerOfParent;     // Mp(x)
  int PowerOfNode;       // M(x)
  int NormalizationTime; // Number of cycles for normalization
  
  const CEvidence *pMyEvidence;
  int NumOfObserveNodes;
  
  intVector NodesInSubTree;
  
  NodesInSubTree.clear();
  
  pMyEvidence = m_pMyEvidence;
  NumOfObserveNodes = pMyEvidence->GetNumberObsNodes();

  int KPlus = 5;
  int KMult = 7;
  int KDiv = 22;
  
  const CStaticGraphicalModel* pMyModel;
  CGraph *pMyGraph;;
  const CNodeType *pNodeType;
  
  intVector Parents;
  
  double Weight = 0;
  
  pMyModel = GetModel();
  pMyGraph = pMyModel->GetGraph();
  
  //Set params
  //3. Set NumberOfChildren
  NumberOfChildren = pMyGraph->GetNumberOfChildren(NumOfNode);
  
  //4. Set NumberOfParents
  NumberOfParents = pMyGraph->GetNumberOfParents(NumOfNode);
  
  //1. Set NumOfMsgsFromChildren
  NumOfMsgsFromChildren = NumberOfChildren;
  
  //2. Set NumOfMsgsFromParents
  NumOfMsgsFromParents = NumberOfParents;
  
  //5. Set NumOfMsgsToParents
  NumOfMsgsToParents = NumberOfParents;
  
  //6. Set NumOfMsgsToChildren
  NumOfMsgsToChildren = NumberOfChildren;
  
  //7. Set PowerParents
  GetParents(NumOfNode, &Parents);
  if (Parents.size()!=0)
  {
    pNodeType = pMyModel->GetNodeType(Parents[0]);
    PowerOfParent = pNodeType->GetNodeSize();
  }
  else
  {
    PowerOfParent = 0;
  }
  
  //8. Set PowerNode
  pNodeType = pMyModel->GetNodeType(NumOfNode);
  PowerOfNode = pNodeType->GetNodeSize();
  
  //9. Set NormalizationTime
  NormalizationTime = PowerOfNode * KDiv + KPlus*(PowerOfNode - 1);
  
  Weight = NumOfMsgsFromParents * (((PowerOfParent - 1) * KPlus +
    PowerOfParent * KMult) * PowerOfNode + PowerOfNode * KMult) +
    NumOfMsgsFromChildren * (KMult*PowerOfNode * (NumberOfChildren - 1) +
    PowerOfNode * KMult) + NumOfMsgsToParents * (KPlus * (PowerOfNode - 1 ) + 
    KMult * PowerOfNode) * PowerOfParent + NumOfMsgsToChildren * 
    (NormalizationTime + KMult * PowerOfNode + (NumberOfChildren - 1) * 
    PowerOfNode * KMult);
  
  return Weight;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParPearlInfEngine::NumberFromGraphToSkeleton(int NumberInGraph)
{
  PNL_CHECK_RANGES(NumberInGraph, 0, GetNumberOfNodes() - 1);
  PNL_CHECK_IS_NULL_POINTER(m_pTreeGraphAccord);
  
  int NumberOfNodes = GetNumberOfNodes();
  int i;
  
  for (i = 0; i < NumberOfNodes; i++)
    if ((*m_pTreeGraphAccord)[i] == NumberInGraph)
      break;
    
  if (i != NumberOfNodes)
    return i;
  else 
    return -1;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParPearlInfEngine::NumberFromSkeletonToGraph(int NumberInSkeleton)
{
  PNL_CHECK_RANGES(NumberInSkeleton, 0, GetNumberOfNodes()-1);
  PNL_CHECK_IS_NULL_POINTER(m_pTreeGraphAccord);
  
  return (*m_pTreeGraphAccord)[NumberInSkeleton];
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::GetUnusedNodesInSubTree(int SubTreeRootInGraph,
  intVector *pUnusedNodesInSubTree)
{
  PNL_CHECK_RANGES( SubTreeRootInGraph, 0, GetNumberOfNodes() - 1);
  PNL_CHECK_IS_NULL_POINTER(pUnusedNodesInSubTree);
  
  bool FunctionStillRunning = true;
  
  set<int> CurrentLevelNodes;
  set<int>::iterator CurrentLevelNodesIterator;
  set<int> NextLevelNodes;
  
  intVector Children;
  Children.clear();
  
  CurrentLevelNodes.clear();
  CurrentLevelNodes.insert(SubTreeRootInGraph);
  
  NextLevelNodes.clear();
  
  pUnusedNodesInSubTree->clear();
  
  if (!m_UsedNodes.count(SubTreeRootInGraph))
    while (FunctionStillRunning)
    {
      //set NextLevelNodes
      CurrentLevelNodesIterator = CurrentLevelNodes.begin();
      
      NextLevelNodes.clear();
      
      while (CurrentLevelNodesIterator!=CurrentLevelNodes.end())
      {
        if (!m_UsedNodes.count(*CurrentLevelNodesIterator))
        {
          GetChildrenInSkeleton(*CurrentLevelNodesIterator, &Children);
          
          NextLevelNodes.insert(Children.begin(), Children.end());
          
          pUnusedNodesInSubTree->push_back(*CurrentLevelNodesIterator);
        }
        CurrentLevelNodesIterator++;
      }
      
      if (NextLevelNodes.size() != 0)
      {
        FunctionStillRunning = true;
        CurrentLevelNodes = NextLevelNodes;
      }
      else 
        FunctionStillRunning = false;
    } //while (FunctionStillRunning...
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

void CParPearlInfEngine::GetParents(int NumOfNode, intVector *pParents) 
{
  const CStaticGraphicalModel *pMyModel;
  CGraph *pMyGraph;;
  
  pMyModel = GetModel();
  pMyGraph = pMyModel->GetGraph();
  
  pMyGraph->GetParents(NumOfNode, pParents);
}
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::DivideNodesWithMinimumDeviationSearch(
  const CEvidence *pEvidence)
{
  int NumberOfNodes;
  int p = m_NumberOfProcesses;  // number of processes
  double W;                     // Weight of tree
  double WThreshold;            // threshold
  int A;                        // Root of subtree, which we will send to
                                // the next processor
  int LastUsedProcess = -1;
  bool FunctionStillRunning = true;
  
  NumberOfNodes = GetNumberOfNodes();
  
  m_UsedNodes.clear();
  
  //set some params 
  m_NodeProcesses.resize(NumberOfNodes, -1);
  m_NodesOfProcess.clear();
  
  m_Roots.clear();
  
  PNL_CHECK_LEFT_BORDER(m_NumberOfProcesses, 1);
  
  m_Roots.resize(m_NumberOfProcesses, -1);
  
  m_pMyEvidence = pEvidence;
  
  BuildWeightesOfNodes();
  BuildWeightesOfSubTrees();
  
  if (p > 0)
  {
    // check if tree is empty
    if (NumberOfNodes!=0)
    {
      W = (*m_pWeightesOfSubTrees)[GetRootNode()];
      WThreshold = W / p;
      
      //label 1 
      while (FunctionStillRunning)
      {
        A = GetNodeWithMinimumDeviation(WThreshold);
        
        SendASubTreeToNextProcess(A, LastUsedProcess);
        LastUsedProcess++;
        
        W -= (*m_pWeightesOfSubTrees)[A];
        p--;
        
        AddNodesNumbersOfSubTreeToUsedNodes(A);
        
        BuildWeightesOfNodes();
        BuildWeightesOfSubTrees();
        
        if (m_UsedNodes.size()!= NumberOfNodes) 
          if (p != 0)
          {
            WThreshold = W / p;
            FunctionStillRunning = true;
          }
          else
          {
            PNL_THROW(CInternalError, "Number of processes in CParPearlInfEngine::DivideNodes <=0 but the node list is not empty.");
          }
          else 
            FunctionStillRunning = false;
      } //while 
    } //if (NumberOfNodes!=...
    else 
      PNL_THROW(COutOfRange, "CParPearlInfEngine::DivideNodes. Tree is empty.");
  }
  else //if (p!=0...
    PNL_THROW(COutOfRange, 
    "CParPearlInfEngine::DivideNodes. Number of processes is 0.");
  
  m_NumberOfUsedProcesses = LastUsedProcess + 1;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParPearlInfEngine::GetNodeWithMinimumDeviation(double WThreshold)
{
  if (WThreshold <= 0)
    PNL_THROW(COutOfRange, 
      "CParPearlInfEngine::GetNodeWithMinimumDeviation. WThreshold must be > 0.");
  
  if (m_UsedNodes.size() >= GetNumberOfNodes())
    PNL_THROW(CInconsistentState, 
      "CParPearlInfEngine::GetNodeWithMinimumDeviation. There are no free nodes.");
  
  doubleVector::iterator WeightOfSubTreeIterator;
  
  int Counter = 0;
  
  WeightOfSubTreeIterator = m_pWeightesOfSubTrees->begin();
  
  int ResultNode = GetRootNode();
  double ResultNodeDeviation = 
    fabs(WThreshold - (*m_pWeightesOfSubTrees)[ResultNode]);
  double CurrentNodeDeviation = -1; //undefined now
  
  while (WeightOfSubTreeIterator != m_pWeightesOfSubTrees->end())
  {
    if ((*WeightOfSubTreeIterator)!=0.0)
    {
      CurrentNodeDeviation = fabs(WThreshold - (*WeightOfSubTreeIterator));
      
      if (CurrentNodeDeviation < ResultNodeDeviation)
      { 
        ResultNode = Counter;
        ResultNodeDeviation = CurrentNodeDeviation;
      } // if (CurrentNodeDeviation < ResultNodeDeviation
    } // if ((*m_pWeightesOfSubTrees)[*WeightOfSubTreeIterator]!=0)
    Counter ++;
    WeightOfSubTreeIterator++;
  }
  
  return ResultNode;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::CollectBeliefsOnProcess(int MainProcNum)
{
  if (m_NumberOfProcesses == 1)
    return;
  if (m_NumberOfUsedProcesses == 1)
    return;
  
  messageVector& rfCurBeliefs = GetCurBeliefs();
  int node, i;

  if (m_MyRank != MainProcNum)
  {
    // send belieifs on main process

    int dataLength = 0;
    float *pDataForSending;
    
    int NumberOfNodes = GetNumberOfNodes();
    for (node=0; node<NumberOfNodes; node++)
    {
      if (m_NodeProcesses[node] == m_MyRank)
      {
        dataLength += static_cast<CNumericDenseMatrix<float>*>(
          rfCurBeliefs[node]->GetMatrix(matTable))->GetRawDataLength();
      }
    }

    pDataForSending = new float[dataLength];

    int lastIndex = -1;
    for (node = 0; node < NumberOfNodes; node++)
    {
      if (m_NodeProcesses[node] == m_MyRank)
      {
        int smallDataLength = 0;
        const float *pSmallDataForSending;
        
        static_cast<CNumericDenseMatrix<float>*>(rfCurBeliefs[node]->
          GetMatrix(matTable))->GetRawData(&smallDataLength, 
          &pSmallDataForSending);
        
        for (i =0; i < smallDataLength; i++)
        {
          pDataForSending[lastIndex + 1 + i] = pSmallDataForSending[i];
        }
        lastIndex += smallDataLength;
      }
    }
    
    MPI_Send(pDataForSending, dataLength, MPI_FLOAT, MainProcNum, 
      m_MyRank, MPI_COMM_WORLD);
    
    delete pDataForSending;
  }
  else
  {
    int *dataLength;
    float *pDataForReceiving;
    dataLength = new int[m_NumberOfUsedProcesses];
    
    for (i = 0; i < m_NumberOfUsedProcesses; i++)
    {
      dataLength[i] = 0;
    }
    int NumberOfNodes = GetNumberOfNodes();
    for (node = 0; node < NumberOfNodes; node++)
    {
      dataLength[m_NodeProcesses[node]] += 
        static_cast<CNumericDenseMatrix<float>*>(rfCurBeliefs[node]->
        GetMatrix(matTable))->GetRawDataLength();
    }
    
    for (i = 0; i < m_NumberOfUsedProcesses - 1; i++)
    {
      MPI_Status status;
      MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
      int NumOfProc = status.MPI_SOURCE;
      
      pDataForReceiving = new float[dataLength[NumOfProc]];
      
      MPI_Recv(pDataForReceiving, dataLength[NumOfProc],MPI_FLOAT,NumOfProc,
        MPI_ANY_TAG,MPI_COMM_WORLD,&status);
      
      int lastUsedIndex = -1;
      
      for (node =0; node<NumberOfNodes; node++)
      {
        int matDim;
        const int *pMatRanges;
        
        if (m_NodeProcesses[node] == NumOfProc)
        {
          static_cast<CNumericDenseMatrix<float>*>(rfCurBeliefs[node]->
            GetMatrix(matTable))->GetRanges(&matDim, &pMatRanges);
          
          CNumericDenseMatrix<float> *RecvMatrix = 
            CNumericDenseMatrix<float>::Create(matDim, pMatRanges, 
            pDataForReceiving + 1 + lastUsedIndex);
          
          rfCurBeliefs[node] -> AttachMatrix(RecvMatrix, matTable);
          
          lastUsedIndex+=RecvMatrix->GetRawDataLength();
        }
      } //for (int node =0; node<NumberOfN...
      //���� ��� pDataForReceiving
      delete pDataForReceiving;
    } //for (i=0; i<m_NumberOfUsedProcesses-1...
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::CollectBeliefsOnProcessContMPI(int MainProcNum)
{
  if (m_NumberOfProcesses == 1)
    return;
  if (m_NumberOfUsedProcesses == 1)
    return;
  
  int node;
  messageVector& rfCurBeliefs = CPearlInfEngine::GetCurBeliefs();
  if (m_MyRank != MainProcNum)
  {
    // send belieifs on main process
    int dataLength = 0;
    float *pDataForSending;
  
    int NumberOfNodes = GetNumberOfNodes();
    int *flags=new int[NumberOfNodes];
    for(node = 0; node < NumberOfNodes; node++)
      flags[node]=0;
    int count = 0;
    
    for (node=0; node<NumberOfNodes; node++)
    {
      if ((m_NodeProcesses[node] == m_MyRank) && 
          (rfCurBeliefs[node]->IsDistributionSpecific() != 1) &&
          (rfCurBeliefs[node]->IsDistributionSpecific() != 2))
      {
        dataLength += static_cast<C2DNumericDenseMatrix<float>*>(
          rfCurBeliefs[node]->GetMatrix(matMean))->GetRawDataLength();     
        dataLength += static_cast<C2DNumericDenseMatrix<float>*>(
          rfCurBeliefs[node]->GetMatrix(matCovariance))->GetRawDataLength();     
        flags[node] = 1;
        count++;
      }
    }
    int FMessage = 0;
    if (count == 0) 
    {
      MPI_Send(&FMessage, 1, MPI_INT, MainProcNum,m_MyRank, MPI_COMM_WORLD);
    }
    else 
    {
      int newdataLength = count * 3 + dataLength + 1;
      FMessage = newdataLength;
      
      pDataForSending = new float[newdataLength];
      int lastIndex = 1;
      int i = 0;
      pDataForSending[0] = 0;
      
      for (node = 0; node < NumberOfNodes; node++)
      {
        if ( flags[node] == 1 )  
        {
          int smallDataLength = 0;
          const float *pSmallDataForSending;
          pDataForSending[0]++;

          pDataForSending[lastIndex] = (float) node;
          lastIndex++;
      
          static_cast<C2DNumericDenseMatrix<float>*>(rfCurBeliefs[node]->
            GetMatrix(matMean))->GetRawData(&smallDataLength, 
            &pSmallDataForSending);
          
          pDataForSending[lastIndex] = (float)smallDataLength;
          lastIndex++;
          memcpy(pDataForSending+lastIndex,pSmallDataForSending,smallDataLength*sizeof(float));
          lastIndex += smallDataLength;
          
          static_cast<C2DNumericDenseMatrix<float>*>(rfCurBeliefs[node]->
            GetMatrix(matCovariance))->GetRawData(&smallDataLength, 
            &pSmallDataForSending);
          memcpy(pDataForSending+lastIndex,pSmallDataForSending,smallDataLength*sizeof(float));
          lastIndex += smallDataLength;
          pDataForSending[lastIndex] = (static_cast<CGaussianDistribFun*>
             (rfCurBeliefs[node])->GetCoefficient(0));
          lastIndex++;
        }
       }
      
      MPI_Send(&FMessage, 1, MPI_INT, MainProcNum,m_MyRank, MPI_COMM_WORLD);
      MPI_Send(pDataForSending, newdataLength, MPI_FLOAT, MainProcNum, 
      m_MyRank, MPI_COMM_WORLD);
      delete [] pDataForSending;
      delete [] flags;
    }
  }
  else
  {
    int *dataLength;
    float *pDataForReceiving;
    int NumberOfNodes = GetNumberOfNodes();
    int Count = 0;
    int node;
    for (int i = 0; i < m_NumberOfUsedProcesses - 1; i++)
    { 
      MPI_Status status;
      MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
      int NumOfProc = status.MPI_SOURCE;
      int FMessageR = 0;
      MPI_Recv(&FMessageR,1,MPI_INT,NumOfProc,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
      
      if (FMessageR!=0) 
      {
        pDataForReceiving = new float[FMessageR];
        MPI_Recv(pDataForReceiving,FMessageR,MPI_FLOAT,NumOfProc,
        MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        int lastUsedIndex = 0;
        int dim;
        int CountMess = (int) pDataForReceiving[lastUsedIndex];
        lastUsedIndex++;
        for (int i = 0; i < CountMess ; i++)
        {
          int matDim;
          const int *pMatRanges;
        
          int node = (int) pDataForReceiving[lastUsedIndex];
          lastUsedIndex++;
          if (rfCurBeliefs[node]->IsDistributionSpecific()!=1)
          {
            lastUsedIndex++;
            static_cast<C2DNumericDenseMatrix<float>*>(rfCurBeliefs[node]->
            GetMatrix(matMean))->GetRanges(&matDim, &pMatRanges);
          
            C2DNumericDenseMatrix<float> *RecvMatrix = 
            C2DNumericDenseMatrix<float>::Create(pMatRanges, 
            pDataForReceiving + lastUsedIndex);
          
            rfCurBeliefs[node] -> AttachMatrix(RecvMatrix, matMean);
          
            lastUsedIndex += RecvMatrix->GetRawDataLength();

            static_cast<C2DNumericDenseMatrix<float>*>(rfCurBeliefs[node]->
              GetMatrix(matCovariance))->GetRanges(&matDim, &pMatRanges);
          
            C2DNumericDenseMatrix<float> *RecvMatrix1 = 
                C2DNumericDenseMatrix<float>::Create(pMatRanges, 
            pDataForReceiving + lastUsedIndex);
          
            rfCurBeliefs[node] -> AttachMatrix(RecvMatrix1, matCovariance);
        
            lastUsedIndex += RecvMatrix->GetRawDataLength();
            static_cast<CGaussianDistribFun*>(rfCurBeliefs[node])->SetCoefficient(pDataForReceiving[lastUsedIndex],0);
            lastUsedIndex++;
          } 
          else if(rfCurBeliefs[node]->IsDistributionSpecific() ==1)
          {
            dim = pDataForReceiving[lastUsedIndex];
            lastUsedIndex++;
            float *pDataForRecvingM;
            float *pDataForRecvingC;
            pDataForRecvingM = new float[dim];
            memcpy(pDataForRecvingM,pDataForReceiving+lastUsedIndex,dim*sizeof(float));
            lastUsedIndex += dim;
            pDataForRecvingC = new float[dim*dim];
            memcpy(pDataForRecvingC,pDataForReceiving+lastUsedIndex,dim*dim*sizeof(float));
            lastUsedIndex += dim*dim;
            float Coeff =  pDataForReceiving[lastUsedIndex];
            lastUsedIndex++;
            (rfCurBeliefs[node]->SetUnitValue(0));
            (rfCurBeliefs[node]->AllocMatrix(pDataForRecvingM,
              matMean));
            (rfCurBeliefs[node]->AllocMatrix(pDataForRecvingC,
              matCovariance));
            static_cast<CGaussianDistribFun*>(rfCurBeliefs[node])->SetCoefficient(Coeff,0);
            delete [] pDataForRecvingC;
            delete [] pDataForRecvingM;
          }
        } 
        delete pDataForReceiving;
      }
    } 
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

void CParPearlInfEngine::InitEngine(const CEvidence* pEvidence)
{
  const int nObsNodes = pEvidence->GetNumberObsNodes();
  const int *ObsNodes = pEvidence->GetAllObsNodes();
  const int *ReallyObs = pEvidence->GetObsNodesFlags();
  intVector ReallyObsNodes;
  int i = 0;
  
  for(; i < nObsNodes; i++)
  {
    if (ReallyObs[i])
    {
      ReallyObsNodes.push_back(ObsNodes[i]);
    }
  }
  
  int NumReallyObs = ReallyObsNodes.size();
  const int numOfNdsInModel = GetNumberOfNodesInModel();
  
  pConstNodeTypeVector nodeTypes(numOfNdsInModel);
  
  for (i = 0; i < numOfNdsInModel; i++)
  {
    nodeTypes[i] = m_pGraphicalModel->GetNodeType(i);
  }
  
  m_modelDt = pnlDetermineDistributionType(numOfNdsInModel,
    NumReallyObs, &ReallyObsNodes.front(), &nodeTypes.front());
  
  int numOfParams;
  CFactor ** param;
  const CFactor *parameter;
  // to check last node in domain
  int bigDomSize;
  const int *BigDomain; 
  // initialize beliefs
  for (i=0; i < numOfNdsInModel; i++)
  {
    message temp = InitMessage(i, nodeTypes[i]);
    (GetCurBeliefs())[i] = temp->GetNormalized();
    (GetOldBeliefs())[i] = temp->GetNormalized();
    m_productPi[i] = temp->GetNormalized();
    m_productLambda[i] = temp->GetNormalized();
    delete temp;
    
    switch (m_pGraphicalModel->GetModelType())
    {
      case mtBNet:
      {
        m_pGraphicalModel->GetFactors(1, &i, &numOfParams, &param);
        parameter = param[0];
        
        for (int j = 0; j < numOfParams; j++)
        {
          param[j]->GetDomain(&bigDomSize, &BigDomain);
          if (BigDomain[bigDomSize-1] == i)
          {
            parameter = param[j];
            break;
          }
        }
        message paramData = parameter->GetDistribFun();
        if (paramData->IsDistributionSpecific())
        {
          m_factorDistrib[i] = paramData->Clone();
        }
        else
        {
          m_factorDistrib[i] = CTabularDistribFun::Create(
            paramData->GetNumberOfNodes(), 
            &paramData->GetNodeTypesVector()->front(),NULL);
          m_factorDistrib[i]->AttachMatrix(paramData->GetMatrix(matTable)->
            Clone(), matTable);
        }
        break;
      }
      case mtMRF2:
      {
        int numOfNeighb;
        const int* neighbors;
        const ENeighborType* orientation;
        
        GetModelGraph()->GetNeighbors(i, &numOfNeighb, &neighbors, 
          &orientation);
        
        int numOfNeighbOfNieghb;
        const int *neighborsOfneighbors;
        const ENeighborType *orNeighbOfneighb;
        
        // initialize messages from neighbors
        for (int j = 0; j < numOfNeighb; ++j)
        {
          int nodes[] = { i, neighbors[j] };
          CFactor**params;
          
          pFactorVector paramVec(0);
          m_pGraphicalModel->GetFactors(2, nodes, &paramVec);
          params = &paramVec.front();
          
          message paramData = params[0]->GetDistribFun();
          
          m_pGraphicalModel->GetGraph()->GetNeighbors(neighbors[j],
            &numOfNeighbOfNieghb, &neighborsOfneighbors, &orNeighbOfneighb);
          int k = 0;
          while (neighborsOfneighbors[k] != i)
            k++;
          
          if (paramData->IsDistributionSpecific())
          {
            m_factorCliqueDistrib[neighbors[j]][k] = paramData->Clone();
          }
          else
          {
            m_factorCliqueDistrib[neighbors[j]][k] = 
              CTabularDistribFun::Create(paramData->GetNumberOfNodes(),
              &paramData->GetNodeTypesVector()->front(),NULL);
            m_factorCliqueDistrib[neighbors[j]][k]->AttachMatrix(paramData->
              GetMatrix(matTable)->Clone(), matTable);
          }
        }
        break;
      }
    }
  }
  
  EDistributionType dtWithoutEv = pnlDetermineDistributionType(
    numOfNdsInModel, 0, &ReallyObsNodes.front(), &nodeTypes.front());
  
  intVector& CONNNODES = GetConnectedNodes();

  switch (dtWithoutEv)
  {
    //create vector of connected nodes
    case dtTabular: case dtGaussian:
    {
      CONNNODES = intVector(numOfNdsInModel);

      for (i = 0; i < numOfNdsInModel; ++i)
      {
        CONNNODES[i] = i;
      }
      break;
    }
    case dtCondGaussian:
    {
      int loc;
      
      for (i = 0; i < numOfNdsInModel; i++)
      {
        loc = std::find(ReallyObsNodes.begin(), ReallyObsNodes.end(), i) -
          ReallyObsNodes.begin();
        
        if ((loc < ReallyObsNodes.size())&&
          (nodeTypes[ReallyObsNodes[loc]]->IsDiscrete()))
        {
          CONNNODES.push_back(i);
        }
      }
      break;
    }
    default:
    {
      PNL_THROW( CInconsistentType, 
        "only for fully tabular or fully gaussian models" )
        break;
    }
  }
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::InitMessages(const CEvidence* evidence)
{
  int i,j;
  const int numOfNdsInModel = GetNumberOfNodesInModel();
  const int  nObsNodes = evidence->GetNumberObsNodes();
  const int* ObsNodes = evidence->GetAllObsNodes();
  const int* ReallyObs = evidence->GetObsNodesFlags();
  intVector& areReallyObserved = GetSignsOfReallyObserved();
  messageVector& selfMessages = GetSelfMessages();

  areReallyObserved = intVector( numOfNdsInModel, 0) ;
  
  for (i = 0; i < nObsNodes; i++)
  {
    if(ReallyObs[i])
    {
      areReallyObserved[ObsNodes[i]] = 1;
    }
  }
  
  pConstNodeTypeVector nodeTypes(numOfNdsInModel);
  
  for (i = 0; i < numOfNdsInModel; i++)
  {
    nodeTypes[i] = m_pGraphicalModel->GetNodeType(i);
  }
  
  int numOfNeighb;
  const int* neighbors;
  const ENeighborType* orientation;
  int mesSize = GetMessagesFromNeighbors().size();
  
  for (i = 0; i < mesSize; i++)
  {
    GetModelGraph()->GetNeighbors(i,  &numOfNeighb, &neighbors, &orientation);
    
    // initialize messages from neighbors
    for (j = 0; j < numOfNeighb; ++j)
    {
      if (orientation[j] == ntParent)
      //message from parent (pi) - it has another type
      {
        m_msgsFromNeighbors[0][i][j] = 
          InitMessage(i, nodeTypes[neighbors[j]], 1);
        m_msgsFromNeighbors[1][i][j] = 
          InitMessage(i, nodeTypes[neighbors[j]], 1);
      }
      else
      {
        m_msgsFromNeighbors[0][i][j] = 
          InitMessage(i, nodeTypes[i], 0);
        m_msgsFromNeighbors[1][i][j] = 
          InitMessage(i, nodeTypes[i], 0 );
      }
    }
    // initialize messages from self
    if (!areReallyObserved[i])
    {
      selfMessages[i] = InitMessage(i, nodeTypes[i]);
    }
    else
    {
      const Value *vall = evidence->GetValue(i);
      if (nodeTypes[i]->IsDiscrete())
      {
        int val = vall->GetInt();
        selfMessages[i] = InitMessageWithEvidence(i, nodeTypes[i], val);
        if(m_bels[0][i])
        {
          delete m_bels[0][i];
          delete m_bels[1][i];
        }
        m_bels[0][i] = InitMessageWithEvidence(i, nodeTypes[i], val);
        m_bels[1][i] = InitMessageWithEvidence(i, nodeTypes[i], val);
      }
      else
      {
        float* val = ((float*)vall);
        if (m_bels[0][i])
        {
          delete m_bels[0][i];
          delete m_bels[1][i];
        }
        m_bels[0][i] = InitMessageWithEvidence(i, nodeTypes[i], val);
        m_bels[1][i] = InitMessageWithEvidence(i, nodeTypes[i], val);
        selfMessages[i] = InitMessageWithEvidence(i, nodeTypes[i], val);
      }
    }
  }
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::ComputeMessage(int destination, int source)
{
  int numOfNeighbOfNieghb;
  const int *neighborsOfneighbors;
  const ENeighborType *orNeighbOfneighb;

  messageVector& selfMessages = GetSelfMessages();

  m_pGraphicalModel->GetGraph()->GetNeighbors(destination,
    &numOfNeighbOfNieghb, &neighborsOfneighbors, &orNeighbOfneighb);
  int k = 0;
  while (neighborsOfneighbors[k] != source)
    k++;

  switch (orNeighbOfneighb[k])
  {
    case 0:
    {
      ComputeProductPi(source);
      ProductLambdaMsgs(source, destination);
      int dom = 0;
      MultiplyOnMessage(m_productPi[source], &dom, &dom,
        m_productLambda[source]);
      NormalizeDataInMessage(m_productPi[source]);
      CopyMatrixOfMessage(
        (*GetNewMsgsFromNeighbors())[destination][k],m_productPi[source]);
      break;
    }
    case 1:
    {
      ProductLambdaMsgs(source);
      ComputeProductLambda(source, m_productLambda[source], 
        (*GetNewMsgsFromNeighbors())[destination][k], destination);
      NormalizeDataInMessage((*GetNewMsgsFromNeighbors())[destination][k]);
      break;
    }
    case 2:
    {
      int i;
      int numNeighb;
      const int *nbrs;
      const ENeighborType *orient;
      m_pGraphicalModel->GetGraph()->GetNeighbors(source, &numNeighb, &nbrs,
        &orient);
      int domain = 0;

      CopyMatrixOfMessage(m_productLambda[source],selfMessages[source]);
      for (i = 0; i < numNeighb; i++)
      {
        if (nbrs[i] != destination)
        {
          MultiplyOnMessage(m_productLambda[source], &domain, &domain,
            (*GetCurMsgsFromNeighbors())[source][i]);
        }
      }
      int nodes[] = { source, destination };
      int numOfParams;
      CFactor**params;

      pFactorVector paramVec(0);
      m_pGraphicalModel->GetFactors(2, nodes, &paramVec);
      params = &paramVec.front();
      numOfParams = paramVec.size();

      message paramMes = params[0]->GetDistribFun();
      const int *ParamDomain; int domSize;
      params[0]->GetDomain( &domSize, &ParamDomain );
      int MarginalizeIndex, MultiplyIndex ; 

      int *posDom = new int [domSize];
      PNL_CHECK_IF_MEMORY_ALLOCATED(posDom);
      for (i = 0; i <domSize; i++)
      {
        posDom[i] = i;
        if (ParamDomain[i] == destination)
        {
          MarginalizeIndex = posDom[i];
        }
        else
        {
          MultiplyIndex = posDom[i];
        }
      }
      if (!paramMes->IsDistributionSpecific())
      {
        CopyMatrixOfMessage(m_factorCliqueDistrib[destination][k],paramMes);
      }
      else
      {
        if (!m_factorCliqueDistrib[destination][k]->IsDistributionSpecific())
        {
           m_factorCliqueDistrib[destination][k]->SetUnitValue(1);
        }
      }
      MultiplyOnMessage(m_factorCliqueDistrib[destination][k], posDom, 
        &MultiplyIndex, m_productLambda[source]);
            
      MarginalizeDataInMessage((*GetNewMsgsFromNeighbors())[destination][k],
        m_factorCliqueDistrib[destination][k], &MarginalizeIndex, 1, 
        m_bMaximize);
      
      NormalizeDataInMessage((*GetNewMsgsFromNeighbors())[destination][k]);
      delete []posDom;
      break;
    }
    default:
    {
      PNL_THROW(CInvalidOperation, "no such type of orientation")
    }
  }
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::CPD_to_pi(int nodeNumber,
  CDistribFun *const*allPiMessages, int *multParentIndices, int numMultNodes,
  int posOfExceptNode, int maximizeFlag)
{
  PNL_CHECK_LEFT_BORDER(numMultNodes, 0);
  if (numMultNodes > 0)
  {
    PNL_CHECK_IS_NULL_POINTER(allPiMessages);
    PNL_CHECK_IS_NULL_POINTER(multParentIndices);
  }
  int i;
  for (i = 0; i < numMultNodes; i++)
  {
    if (allPiMessages[i]->GetDistributionType() != dtTabular)
    {
      PNL_THROW( CInvalidOperation, 
        "Gaussian pi messages can't produce tabular pi" )
    }
  }
  int NumberOfNodes = m_factorDistrib[nodeNumber]->GetNumberOfNodes();
  pnlVector<int> domNumbers(NumberOfNodes);
  for (i = 0; i < NumberOfNodes; i++)
  {
    domNumbers[i] = i;
  }
  for (i = 0; i < numMultNodes; i++)
  {
    MultiplyOnMessage(m_factorDistrib[nodeNumber], &domNumbers.front(), 
      multParentIndices+i, allPiMessages[multParentIndices[i]]);
  }
  intVector keepedNodes(1, domNumbers[NumberOfNodes-1]);
    
  if (posOfExceptNode >= 0)
  {
    keepedNodes.push_back(posOfExceptNode);
  }
  MarginalizeDataInMessage(m_productPi[nodeNumber],
    m_factorDistrib[nodeNumber], &keepedNodes.front(), keepedNodes.size(),
    maximizeFlag);
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::ComputeProductPi(int nodeNumber, int except)
{
  int i;
  CGraph *theGraph = m_pGraphicalModel->GetGraph();
  
  int numNeighb;
  const int *neighbors;
  const ENeighborType *orientations;
  
  theGraph->GetNeighbors(nodeNumber, &numNeighb, &neighbors,
    &orientations);
  
  if (orientations[0] == ntNeighbor)
  {
    delete m_productPi[nodeNumber];
    m_productPi[nodeNumber] = InitMessage(nodeNumber,
      (m_pGraphicalModel->GetNodeType(nodeNumber)));
    return;
  }
  
  int nodes = nodeNumber;
  int numOfParams;
  CFactor ** param;
  
  pFactorVector paramVec(0);
  m_pGraphicalModel->GetFactors(1, &nodes, &paramVec);
  param = &paramVec.front();
  numOfParams = paramVec.size();
  
  if (!numOfParams)
  {
    PNL_THROW(CInvalidOperation, "no parameter")
  }
  
  const CFactor *parameter = param[0];
  
  int bigDomSize;
  const int *BigDomain; 
  
  int isSuchDom = 0;
  
  for (i = 0; i < numOfParams; i++)
  {
    param[i]->GetDomain(&bigDomSize, &BigDomain);
    if (BigDomain[bigDomSize-1] == nodeNumber)
    {
      parameter = param[i];
      isSuchDom = 1;
      break;
    }
  }
    
  if (!isSuchDom)
  {
    delete m_productPi[nodeNumber];
    m_productPi[nodeNumber] = 
      InitMessage(nodeNumber, m_pGraphicalModel->GetNodeType(nodeNumber));
    return;
  }
  CopyMatrixOfMessage(m_factorDistrib[nodeNumber],parameter->GetDistribFun());
  
  parameter->GetDomain(&bigDomSize, &BigDomain);
  intVector domNumbers(bigDomSize);
  for (i = 0; i < bigDomSize; i++)
  {
    domNumbers[i] = i;
  }

  int location;
  intVector parentIndices;
  messageVector parentMessages(bigDomSize - 1);
    
  int keepNode = -1;

  for (i = 0; i < numNeighb; i++)
  {
    location = std::find(BigDomain, BigDomain + bigDomSize,
      neighbors[i]) - BigDomain;
    if ((orientations[i] == ntParent) && (location < bigDomSize))
    {
      parentMessages[location] = (*GetCurMsgsFromNeighbors())[nodeNumber][i];
      if (neighbors[i] != except)
      {
        parentIndices.push_back(location);
      }
      if (neighbors[i] == except)
      {
        keepNode =  location;
      }
    }
  }
  CPD_to_pi(nodeNumber, &parentMessages.front(), &parentIndices.front(), 
    parentIndices.size(), keepNode, m_bMaximize);
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::CPD_to_lambda(int nodeNumber,
  CDistribFun *const* allPiMessages, int *multParentIndices, int numMultNodes,
  CDistribFun* res, int posOfExceptNode, int maximizeFlag)
{
  int i;
  int NumberOfNodes = m_factorDistrib[nodeNumber]->GetNumberOfNodes();
  if (numMultNodes > NumberOfNodes - 2)
  {
    PNL_THROW(CInconsistentSize, "numNodes is number of parents except one")
  }
  if ((posOfExceptNode >=0) && (numMultNodes + 2 != NumberOfNodes))
  {
    PNL_THROW(CInconsistentSize, "numNodes should connect")
  }
  if (m_productLambda[nodeNumber]->GetDistributionType() != dtTabular)
  {
    PNL_THROW(CInvalidOperation,
      "Gaussian lambda can't produce tabular message to parent")
  }
  for (i = 0; i < numMultNodes; i++)
  {
    if (allPiMessages[multParentIndices[i]]->GetDistributionType() != 
      dtTabular)
    {
      PNL_THROW(CInvalidOperation, 
        "Gaussian pi messages can't produce Tabular message to parent")
    }
  }
  intVector allDomain(NumberOfNodes);
  for (i= 0; i < NumberOfNodes; i++)
  {
    allDomain[i] = i;
  }
  CTabularDistribFun *resData = 
    static_cast<CTabularDistribFun*>(m_factorDistrib[nodeNumber]->Clone());
  for (i = 0; i < numMultNodes; i++)
  {
    resData->MultiplyInSelfData( &allDomain.front(), &multParentIndices[i],
      allPiMessages[multParentIndices[i]] );
  }
  int numThis = NumberOfNodes - 1;
  resData->MultiplyInSelfData( &allDomain.front(), &numThis, 
    m_productLambda[nodeNumber] );

  MarginalizeDataInMessage(res, resData, &posOfExceptNode, 1, maximizeFlag);
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::ComputeProductLambda(int nodeNumber, message lambda,
  CDistribFun* res, int except)
{
  int i;
  CGraph *theGraph = m_pGraphicalModel->GetGraph();
  
  int numNeighb;
  const int *neighbors;
  const ENeighborType *orientations;
  
  theGraph->GetNeighbors(nodeNumber, &numNeighb, &neighbors, &orientations);
  
  if (orientations[0] == ntNeighbor)
  {
    delete m_productLambda[nodeNumber];
    m_productLambda[nodeNumber] = InitMessage(nodeNumber, 
      m_pGraphicalModel->GetNodeType(nodeNumber));
    return;
  }
  
  int nodes = nodeNumber;
  int numOfParams;
  CFactor ** param;
  pFactorVector paramVec(0);

  m_pGraphicalModel->GetFactors(1, &nodes, &paramVec);
  param = &paramVec.front();
  numOfParams = paramVec.size();
  
  if (!numOfParams)
  {
    PNL_THROW(CInvalidOperation, "no parameter")
  }
  
  const CFactor *parameter = param[0];
  const int *BigDomain; int bigDomSize;
  int bSuchDom = 0;
  
  for (i = 0; i < numOfParams; i++)
  {
    param[i]->GetDomain(&bigDomSize, &BigDomain);
    if (BigDomain[bigDomSize-1] == nodeNumber)
    {
      parameter = param[i];
      bSuchDom = 1;
      break;
    }
  }
  
  if (!bSuchDom)
  {
    delete m_productLambda[nodeNumber];
    m_productLambda[nodeNumber] = 
      InitMessage(nodeNumber, m_pGraphicalModel->GetNodeType(nodeNumber));

    return;
  }
  message paramData = parameter->GetDistribFun();
  CopyMatrixOfMessage(m_factorDistrib[nodeNumber],paramData);
  
  parameter->GetDomain(&bigDomSize, &BigDomain);
  
  intVector domNumbers(bigDomSize);
  for (i = 0; i < bigDomSize; i++)
  {
    domNumbers[i] = i;
  }

  int location;
  intVector parentIndices;
  messageVector parentMessages(bigDomSize - 1);
  int keepNode = -1;

  for (i = 0; i < numNeighb; i++)
  {
    location = std::find(BigDomain, BigDomain + bigDomSize,
      neighbors[i]) - BigDomain;
    if ((orientations[i] == ntParent) && (location < bigDomSize))
    {
      parentMessages[location] = (*GetCurMsgsFromNeighbors())[nodeNumber][i];
      if (neighbors[i] != except)
      {
        parentIndices.push_back(location);
      }
      if (neighbors[i] == except)
      {
        keepNode =  location;
      }
    }
  }
  if (keepNode == -1)
  {
    keepNode = -1;
  }
  CPD_to_lambda(nodeNumber, &parentMessages.front(), &parentIndices.front(), 
    parentIndices.size(), res, keepNode, m_bMaximize);
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::ProductLambdaMsgs(int nodeNumber, int except)
{
  int i;
  
  int numNeighb;
  const int* nbrs;
  const ENeighborType* orient;
  
  m_pGraphicalModel->GetGraph()->GetNeighbors(nodeNumber, &numNeighb,
    &nbrs, &orient);
  
  int smallDom = 0;
  messageVector& selfMessages = GetSelfMessages();
  
  CopyMatrixOfMessage(m_productLambda[nodeNumber], selfMessages[nodeNumber]);
  
  for (i = 0 ; i <numNeighb; i++)
  {
    if ((orient[i] != ntParent) && (nbrs[i] != except))
    {
      MultiplyOnMessage(m_productLambda[nodeNumber], &smallDom, 
        &smallDom, (*GetCurMsgsFromNeighbors())[nodeNumber][i]);
    }
  }
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::ComputeBelief(int nodeNumber)
{
  int dom = 0;
  
  ComputeProductPi(nodeNumber);
  ProductLambdaMsgs(nodeNumber);
  
  MultiplyOnMessage(m_productPi[nodeNumber], &dom, &dom,
    m_productLambda[nodeNumber]);
  
  NormalizeDataInMessage(m_productPi[nodeNumber]);
  CopyMatrixOfMessage((GetCurBeliefs())[nodeNumber],m_productPi[nodeNumber]);
}
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::ParallelProtocol()
{
  if (GetMaxNumberOfIterations() == 0)
  {
    SetMaxNumberOfIterations(GetNumberOfNodesInModel());
  }
  
  int i, j;
  int i1;
  int converged = 0;
  int changed = 0;
  int iter = 0;

  const CGraph *pGraph = m_pGraphicalModel->GetGraph();
  intVector& CONNNODES = GetConnectedNodes();
  intVector& areReallyObserved = GetSignsOfReallyObserved();
  
  int nAllMes = GetCurMsgsFromNeighbors()->size();

  int numOfNeighb;
  const int *neighbors;
  const ENeighborType *orientation;
  intVector sources, sinks;
  int NumOfMsgs;
  intVector messLengthSend(m_NumberOfProcesses, 0);
  intVector messLengthRecv(m_NumberOfProcesses, 0);

  SetFarNeighbors(&sinks, &sources,&messLengthSend,&messLengthRecv, NumOfMsgs,0,NULL);

  float **messData=new float*[m_NumberOfProcesses];

  for (i1 = 0; i1 < m_NumberOfProcesses; i1++)
  {
    messData[i1] = new float[messLengthSend[i1]];
  }

  float **messDataRecv=new float*[m_NumberOfProcesses];

  for (i1 = 0; i1 < m_NumberOfProcesses; i1++)
  {
    messDataRecv[i1] = new float[messLengthSend[i1]];
  }

  int pos = 0;
  const int AllNumberNodes=m_NodesOfProcess.size();
  while ((!converged) && (iter < GetMaxNumberOfIterations()))
  {
    int Count = m_NodesOfProcess.size();
    for (i = 0; i < Count; i++)
    {
      pGraph->GetNeighbors(CONNNODES[m_NodesOfProcess[i]], 
        &numOfNeighb, &neighbors, &orientation);
      for (j = 0; j < numOfNeighb; j++)
      {
        ComputeMessage(neighbors[j], m_NodesOfProcess[i]);
      }
    }
    GoNextIteration();
    intVector indexmess(m_NumberOfProcesses,0);
    intVector indexmess1(m_NumberOfProcesses,0);
    for (i = 0; i < NumOfMsgs; i++)
    {
      // find the number of process to which we must send
      int NumOfProc = m_NodeProcesses[sinks[i]];
      CDenseMatrix<float> *matForSending;
      int dataLength;
      const float *pDataForSending;

      matForSending = static_cast<CDenseMatrix<float>*>
        ((*GetCurMsgsFromNeighbors())[sinks[i]][sources[i]]->GetMatrix(
          matTable));
    
      matForSending->GetRawData(&dataLength, &pDataForSending);
      messData[NumOfProc][indexmess[NumOfProc]]=(float)sinks[i]+0.3;
      indexmess[NumOfProc]++;
      messData[NumOfProc][indexmess[NumOfProc]]=(float)sources[i]+0.3;
      indexmess[NumOfProc]++;
      for(i1 = 0; i1 < dataLength; i1++)
      {
        messData[NumOfProc][indexmess[NumOfProc]] = pDataForSending[i1];
        indexmess[NumOfProc]++;
      }
    }

    for (i = 0; i <m_NumberOfProcesses; i++)
    {
      if (messLengthSend[i]>0)
      {
        if (m_MyRank != i)
          MPI_Send((float*)messData[i], messLengthSend[i], MPI_FLOAT, i,
            m_MyRank, MPI_COMM_WORLD);
      }
    }
    int NumOfProc_t = 0;
    for (i = 0; i < m_NumberOfProcesses; i++)
    {
      MPI_Status status;
      if (messLengthRecv[i] > 0)
      {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        NumOfProc_t = status.MPI_SOURCE;
        MPI_Recv((float*)messDataRecv[NumOfProc_t], 
          messLengthRecv[NumOfProc_t], MPI_FLOAT, NumOfProc_t, MPI_ANY_TAG,
          MPI_COMM_WORLD, &status);
      }
    }
    int flag1 = 0;
    for (int NumOfProc = 0; NumOfProc < m_NumberOfProcesses; NumOfProc++)
    {
      int indexmess_new = 0;
      while (indexmess_new < messLengthRecv[NumOfProc])
      {
        CDenseMatrix<float> *matFor;
        int dataLength;
        float *pDataForRecving;
        int sourceNode, sinkNode;
        sinkNode = (int)messDataRecv[NumOfProc][indexmess_new];
        
        indexmess_new++;
        sourceNode = (int)messDataRecv[NumOfProc][indexmess_new];
        indexmess_new++;
        matFor = static_cast<CDenseMatrix<float>*>
          ((*GetCurMsgsFromNeighbors())[sinkNode][sourceNode]->GetMatrix(
          matTable));
        matFor->GetRawData(&dataLength, (const float**)(&pDataForRecving));
        for (i1 = 0; i1 < dataLength; i1++) 
        {
          pDataForRecving[i1] = messDataRecv[NumOfProc][indexmess_new];
          indexmess_new++;
        }
      } //for
    } //for
    changed = 0;

    for (i = 0; i < m_NodesOfProcess.size(); i++)
    {
      if (!areReallyObserved[CONNNODES[m_NodesOfProcess[i]]])
      {
        ComputeBelief(CONNNODES[m_NodesOfProcess[i]]);
        changed += 
          !(GetOldBeliefs())[CONNNODES[m_NodesOfProcess[i]]]->IsEqual(
          (GetCurBeliefs())[CONNNODES[m_NodesOfProcess[i]]], GetTolerance());
      }
    }

    MPI_Allreduce(&changed, &converged, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    converged = !(converged);

    iter++;
  }

  for (i1 = 0; i1 < m_NumberOfProcesses; i1++)
  {
    delete [] messData[i1];
  }
  delete [] messData;
  for (i1 = 0; i1 < m_NumberOfProcesses; i1++) 
  {
    delete [] messDataRecv[i1];
  }
  
  delete [] messDataRecv;

  m_IterationCounter = iter;
}
#endif // PAR_MPI

// ----------------------------------------------------------------------------

#ifdef PAR_MPI

#define D_UNIFORM   1
#define D_DELTA     2
#define D_IN_MOMENT 8
#define D_UNI_DELTA (D_UNIFORM | D_DELTA)
#define D_CODE(__spec, __mom)           \
    ( __spec | ((__mom) ? D_IN_MOMENT : 0) )

void CParPearlInfEngine::ParallelProtocolContMPI()
{
  if (GetMaxNumberOfIterations() == 0)
  {
    SetMaxNumberOfIterations(GetNumberOfNodesInModel());
  }
  
  int i, j;
  int converged = 0;
  int changed = 0;
  int iter = 0;
  const CGraph *pGraph = m_pGraphicalModel->GetGraph();
  int nAllMes = GetMessagesFromNeighbors().size();
  messageVecVector newMessages(GetMessagesFromNeighbors());
  int numOfNeighb;
  const int *neighbors;
  const ENeighborType *orientation;

  intVector& CONNNODES = GetConnectedNodes();
  intVector& areReallyObserved = GetSignsOfReallyObserved();

  CGaussianDistribFun* msg;
  int msg_spec;
  bool msg_is_moment;
  bool delta_in_canon;
  int sendMsgLength;

  int NumSelfNodes = m_NodesOfProcess.size();
  intVector sources, sinks;
  int NumOfMsgs;
  intVector indexmess;
  intVector sendMaxLength(m_NumberOfProcesses, 0);
  intVector recvMaxLength(m_NumberOfProcesses, 0);
  intVecVector neighbNumbers;
  SetFarNeighbors(&sinks, &sources, &sendMaxLength, &recvMaxLength, NumOfMsgs, 1,
    &neighbNumbers);
  float **messData = new float*[m_NumberOfProcesses];
  float **messDataRecv = new float*[m_NumberOfProcesses];
  for (i = 0; i < m_NumberOfProcesses; i++)
  {
    messData[i] = new float[sendMaxLength[i]];
    messDataRecv[i] = new float[sendMaxLength[i]];
  }

  while ((!converged)&&(iter<GetMaxNumberOfIterations()))
  {
    for (i = 0; i < m_NumberOfProcesses; i++)
    {
      messData[i][0]=0;
    }
    for(i = 0; i < NumSelfNodes; i++)
    {
      pGraph->GetNeighbors(m_NodesOfProcess[i], &numOfNeighb, &neighbors, 
        &orientation);
      for(j = 0; j < numOfNeighb; j++)
      {
        newMessages[neighbors[j]][neighbNumbers[i][j]] = 
          CPearlInfEngine::ComputeMessage(neighbors[j], m_NodesOfProcess[i], 
          1 - orientation[j]);
      }
    }
    for(i = 0; i < NumSelfNodes; i++)
    {
      pGraph->GetNeighbors(m_NodesOfProcess[i],&numOfNeighb, &neighbors, 
        &orientation);
      for(j = 0; j < numOfNeighb; j++)
      {
        delete GetMessagesFromNeighbors()[neighbors[j]][neighbNumbers[i][j]];
        GetMessagesFromNeighbors()[neighbors[j]][neighbNumbers[i][j]] = 
          newMessages[neighbors[j]][neighbNumbers[i][j]];
      }
    }

    indexmess = intVector(m_NumberOfProcesses, 1);
    msg = NULL;
    for (i = 0; i < NumOfMsgs; i++)
    {
      int dataLength;
      const float *pDataForSending;
      C2DNumericDenseMatrix<float> *matForSending;
      
      int NumOfProc = m_NodeProcesses[sinks[i]];
      msg = static_cast<CGaussianDistribFun*>(GetMessagesFromNeighbors()[sinks[i]][sources[i]]);
      msg_spec = msg->IsDistributionSpecific();
      msg_is_moment = msg->GetMomentFormFlag();
      delta_in_canon = (msg_spec == 2) && !msg_is_moment;

      if (delta_in_canon)
      {
        msg->UpdateMomentForm();
        msg_is_moment = !msg_is_moment;
      }

      if (msg_spec == 1 || (msg_spec == 2 && msg_is_moment) || (msg_spec != 1 && msg_spec != 2))
      {
        messData[NumOfProc][0]++;
       
        messData[NumOfProc][indexmess[NumOfProc]] = (float)sinks[i] + 0.3f;
        indexmess[NumOfProc]++;
        messData[NumOfProc][indexmess[NumOfProc]] = (float)sources[i] + 0.3f;
        indexmess[NumOfProc]++;

        messData[NumOfProc][indexmess[NumOfProc]] = (float)(D_CODE(msg_spec, msg_is_moment)) + 0.3f;
        indexmess[NumOfProc]++;

        if (msg_spec == 1) // uniform
        {
          // that's all, we need to send only flag
          continue;
        }
        if (msg_is_moment) // moment form
        {
          matForSending = static_cast<C2DNumericDenseMatrix<float>*>(msg->GetMatrix(matMean));
          matForSending->GetRawData(&dataLength, &pDataForSending);
       
          messData[NumOfProc][indexmess[NumOfProc]] = (float)dataLength + 0.3f;
          indexmess[NumOfProc]++;
          memcpy(messData[NumOfProc] + indexmess[NumOfProc], pDataForSending,
            dataLength * sizeof(float));
          indexmess[NumOfProc] += dataLength;

          if (msg_spec != 2) // non delta 
          {
            matForSending = static_cast<C2DNumericDenseMatrix<float>*>(msg->GetMatrix(matCovariance));
            matForSending->GetRawData(&dataLength, &pDataForSending);
   
            memcpy(messData[NumOfProc] + indexmess[NumOfProc], pDataForSending,
              dataLength * sizeof(float));
            indexmess[NumOfProc] += dataLength;
       
            messData[NumOfProc][indexmess[NumOfProc]] = msg->GetCoefficient(0);
            indexmess[NumOfProc] += 1;
          }
        }
        else // canonical form
        {
          matForSending = static_cast<C2DNumericDenseMatrix<float>*>(msg->GetMatrix(matH));            
          matForSending->GetRawData(&dataLength, &pDataForSending);
            
          messData[NumOfProc][indexmess[NumOfProc]]=(float)dataLength + 0.3f;
          indexmess[NumOfProc]++;    
          memcpy(messData[NumOfProc] + indexmess[NumOfProc], pDataForSending, 
            dataLength * sizeof(float));
          indexmess[NumOfProc] += dataLength;
            
          matForSending = static_cast<C2DNumericDenseMatrix<float>*>(msg->GetMatrix(matK));  
          matForSending->GetRawData(&dataLength, &pDataForSending);
            
          memcpy(messData[NumOfProc] + indexmess[NumOfProc], pDataForSending,
            dataLength * sizeof(float));
          indexmess[NumOfProc] += dataLength;
            
          messData[NumOfProc][indexmess[NumOfProc]] = msg->GetCoefficient(1);
          indexmess[NumOfProc] += 1;        
        }
      }

      if (delta_in_canon)
        msg->UpdateCanonicalForm();
    }

    for (i = 0; i < m_NumberOfProcesses; i++)
    {
      sendMsgLength = ((int)(messData[i][0]) == 0) ? 0 : indexmess[i];

      if (m_MyRank != i)
      {
        MPI_Send(&sendMsgLength, 1, MPI_INT, i, m_MyRank, MPI_COMM_WORLD);
        if (sendMsgLength)
        {
          MPI_Send((float*)messData[i], sendMsgLength, MPI_FLOAT, i,
            m_MyRank, MPI_COMM_WORLD);        
        }
      }
    }

    int NumOfProc_t = 0;
    intVector recvMsgLength(m_NumberOfProcesses, 0);
    for (i = 0; i < m_NumberOfProcesses; i++)
    {
      if (m_MyRank != i) 
      {
        MPI_Status status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        NumOfProc_t = status.MPI_SOURCE;
        MPI_Recv(&recvMsgLength[NumOfProc_t], 1, MPI_INT, NumOfProc_t, MPI_ANY_TAG, 
          MPI_COMM_WORLD, &status);
        if (recvMsgLength[NumOfProc_t])  
        {          
          MPI_Recv((float*)messDataRecv[NumOfProc_t], recvMsgLength[NumOfProc_t], 
            MPI_FLOAT, NumOfProc_t, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
      }
    }
 
    msg = NULL;
    for (int NumOfProc = 0; NumOfProc < m_NumberOfProcesses; NumOfProc++)
    {
      if ((m_MyRank != NumOfProc) && (recvMsgLength[NumOfProc] > 0))
      {
        int indexmess_new = 0;
        int CountMess = 0;  

        CountMess = (int)messDataRecv[NumOfProc][indexmess_new];       
        indexmess_new++;     
        
        for(i = 0; i < CountMess; i++) 
        {
          C2DNumericDenseMatrix<float> *matFor;
          int dataLength;
          float *pDataForRecvingM;
          float *pDataForRecvingC;
          float Coeff;
          int sourceNode, sinkNode;
          int type;
          bool isDelta, isUniform;
            
          sinkNode = (int)messDataRecv[NumOfProc][indexmess_new];
          indexmess_new++;
          sourceNode = (int)messDataRecv[NumOfProc][indexmess_new];
          indexmess_new++;
          type = (int)messDataRecv[NumOfProc][indexmess_new];
          indexmess_new++;

          msg = static_cast<CGaussianDistribFun*>(GetMessagesFromNeighbors()[sinkNode][sourceNode]);
          msg_is_moment = (type & D_IN_MOMENT) == D_IN_MOMENT;
          isUniform = (type & D_UNI_DELTA) == D_UNIFORM;
          isDelta = (type & D_UNI_DELTA) == D_DELTA;

          if (isUniform) // uniform
          {
            msg->SetUnitValue(1);
            continue;
          }

          if (msg_is_moment) // moment form
          {
            dataLength = (int)messDataRecv[NumOfProc][indexmess_new];
            indexmess_new++;            

            pDataForRecvingM = new float[dataLength];
            memcpy(pDataForRecvingM, messDataRecv[NumOfProc] + indexmess_new, 
              dataLength * sizeof(float));
            indexmess_new+=dataLength;

            if (!isDelta) // not delta
            {
              pDataForRecvingC = new float[dataLength * dataLength];
              memcpy(pDataForRecvingC, messDataRecv[NumOfProc] + indexmess_new,
                dataLength * dataLength * sizeof(float));          
              indexmess_new+=dataLength * dataLength;
                        
              Coeff = messDataRecv[NumOfProc][indexmess_new];            
              indexmess_new++;
            }

            if (isDelta) //delta
            {
              if (msg->IsDistributionSpecific() != 2)
              {
                int nnodes = msg->GetNumberOfNodes();
                const pConstNodeTypeVector types(*msg->GetNodeTypesVector());
                
                delete GetMessagesFromNeighbors()[sinkNode][sourceNode];
                GetMessagesFromNeighbors()[sinkNode][sourceNode] = 
                  CGaussianDistribFun::CreateDeltaDistribution(nnodes, &types.front(), 
                  pDataForRecvingM);
              }
              else
              {
                msg->AllocMatrix(pDataForRecvingM, matMean);
              }
            }
            else
            {
              if (msg->IsDistributionSpecific() == 1)
              {
                msg->SetUnitValue(0);
                msg->AllocMatrix(pDataForRecvingM, matMean);
                msg->AllocMatrix(pDataForRecvingC, matCovariance);
                msg->SetCoefficient(Coeff,0);
              }
              else if (msg->IsDistributionSpecific() == 2)
              {
              }
              else
              {
                floatVector *vector;
                
                vector = (floatVector *)static_cast<C2DNumericDenseMatrix<float>*>
                  (msg->GetMatrix(matMean))->GetVector();
                memcpy(&vector->front(), pDataForRecvingM, dataLength * sizeof(float));
             
                vector = (floatVector *)static_cast<C2DNumericDenseMatrix<float>*>
                  (msg->GetMatrix(matCovariance))->GetVector();
                memcpy(&vector->front(), pDataForRecvingC, dataLength * dataLength * sizeof(float));
                
                msg->SetCoefficient(Coeff,0);
              }
            }

            delete [] pDataForRecvingM;
            if (!isDelta) // not delta
              delete [] pDataForRecvingC;
          }
          else // canonical form
          {
            dataLength = (int)messDataRecv[NumOfProc][indexmess_new];
            indexmess_new++;            
            
            pDataForRecvingM = new float[dataLength];
            memcpy(pDataForRecvingM, messDataRecv[NumOfProc] + indexmess_new,
              dataLength * sizeof(float));
            indexmess_new+=dataLength;

            pDataForRecvingC = new float[dataLength * dataLength];
            memcpy(pDataForRecvingC, messDataRecv[NumOfProc] + indexmess_new, 
              dataLength * dataLength * sizeof(float));          
            indexmess_new+=dataLength * dataLength;
              
            Coeff = messDataRecv[NumOfProc][indexmess_new];            
            indexmess_new++;

            if (msg->IsDistributionSpecific() == 1)
            {
              msg->SetUnitValue(0);
              msg->AllocMatrix(pDataForRecvingM, matH);
              msg->AllocMatrix(pDataForRecvingC, matK);
              msg->SetCoefficient(Coeff,1);
            }
            else
            {
              floatVector *vector;
              
              vector = (floatVector *)static_cast<C2DNumericDenseMatrix<float>*>
                (msg->GetMatrix(matH))->GetVector();
              memcpy(&vector->front(), pDataForRecvingM, dataLength * sizeof(float));
              
              vector = (floatVector *)static_cast<C2DNumericDenseMatrix<float>*>
                (msg->GetMatrix(matK))->GetVector();
              memcpy(&vector->front(), pDataForRecvingC, dataLength * dataLength * sizeof(float));
              
              msg->SetCoefficient(Coeff,1);              
            }
 
            delete [] pDataForRecvingM;
            delete [] pDataForRecvingC;
          }
        } //for
      }//if
    } //for          
    changed = 0;
   
    for(i = 0; i < m_NodesOfProcess.size(); i++)
    {
      if(!areReallyObserved[CONNNODES[m_NodesOfProcess[i]]])
      {
        message tempBel = CPearlInfEngine::GetCurBeliefs()[m_NodesOfProcess[i]];  
        CPearlInfEngine::ComputeBelief(CONNNODES[m_NodesOfProcess[i]]);
        changed += !tempBel->IsEqual(CPearlInfEngine::
          GetCurBeliefs()[CONNNODES[m_NodesOfProcess[i]]], CPearlInfEngine::GetTolerance());
        delete tempBel;         
      }
    }
    
    MPI_Allreduce(&changed, &converged, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    converged = !(converged);
    iter++;
  }

  if (m_NumberOfProcesses!=1)
  { 
    for (i = 0; i < m_NumberOfProcesses; i++)
    {
      if (m_MyRank != i)
      {
        delete [] messData[i];
        delete [] messDataRecv[i];
      }
    }
  }
  delete [] messData;
  delete [] messDataRecv;

  m_IterationCounter = iter;
}
#endif // PAR_MPI

// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParPearlInfEngine::ParallelProtocolOMP()
{
  if (GetMaxNumberOfIterations() == 0)
  {
    SetMaxNumberOfIterations(GetNumberOfNodesInModel());
  }
  
  int i, j;
  int converged = 0;
  int changed = 0;
  int iter = 0;

  omp_lock_t change_lock;
  omp_init_lock(&change_lock);

  const CGraph *pGraph = m_pGraphicalModel->GetGraph();
  
  int nAllMes = GetCurMsgsFromNeighbors()->size();
  int Count = GetNumberOfNodesInModel();

  int numOfNeighb;
  const int *neighbors;
  const ENeighborType *orientation;

#ifdef _CLUSTER_OPENMP
  intVector *connNodes = &(GetConnectedNodes());
  intVector *areReallyObserved = &(GetSignsOfReallyObserved());
#define CONNNODES (*connNodes)
#define AREREALLYOBSERVED (*areReallyObserved)
#else
  intVector& connNodes = GetConnectedNodes();
  intVector& areReallyObserved = GetSignsOfReallyObserved();
#define CONNNODES connNodes
#define AREREALLYOBSERVED areReallyObserved
#endif

  while ((!converged) && (iter < GetMaxNumberOfIterations()))
  {
    #pragma omp parallel for schedule(dynamic) private(i, j, numOfNeighb, neighbors, orientation)
    for (i = 0; i < Count; i++)
    {
      pGraph->GetNeighbors(CONNNODES[i], 
        &numOfNeighb, &neighbors, &orientation);
      for (j = 0; j < numOfNeighb; j++)
      {
        ComputeMessage(neighbors[j], i);
      }
    }
    GoNextIteration();

    changed = 0;

    #pragma omp parallel for schedule(dynamic) private(i)// reduction(+:changed)
    for (i = 0; i < Count; i++)
    {
      if (!AREREALLYOBSERVED[CONNNODES[i]])
      {
        ComputeBelief(CONNNODES[i]);
        if(!changed)
        {
            omp_set_lock(&change_lock);
            changed += 
                !(GetOldBeliefs())[CONNNODES[i]]->IsEqual(
                (GetCurBeliefs())[CONNNODES[i]], GetTolerance());
            omp_unset_lock(&change_lock);
        }
      }
    }
    converged = !changed;

    iter++;
  } //while ((!converged)&&(iter<m_numberOfIterations))

  omp_destroy_lock(&change_lock);

  m_IterationCounter = iter;
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParPearlInfEngine::ParallelProtocolContOMP()
{
  if (GetMaxNumberOfIterations() == 0)
  {
      SetMaxNumberOfIterations(GetNumberOfNodesInModel());        
  }

  int i, j;
  int converged = 0;
  int changed = 0;
  int iter = 0;
  const CGraph *pGraph = m_pGraphicalModel->GetGraph();
  intVector& connNodes = GetConnectedNodes();
  intVector& areReallyObserved = GetSignsOfReallyObserved();
  messageVecVector &messages = GetMessagesFromNeighbors();
  int nAllMes = messages.size();
  omp_lock_t change_lock;
  omp_init_lock(&change_lock);
  int Count = GetNumberOfNodesInModel();

  messageVecVector newMessages(GetMessagesFromNeighbors());

  while ((!converged) && (iter<GetMaxNumberOfIterations()))
  {   
    int                    numOfNeighb;
    const int              *neighbors;
    const ENeighborType *orientation;
    #pragma omp parallel for schedule(dynamic) private(i, j, numOfNeighb, neighbors, orientation)
    for( i = 0; i < Count; i++ )
    {
      pGraph->GetNeighbors( connNodes[i], &numOfNeighb, 
        &neighbors, &orientation );
      for( j = 0; j < numOfNeighb; j++)
      {                
        newMessages[connNodes[i]][j] = CPearlInfEngine::ComputeMessage( connNodes[i],
          neighbors[j], orientation[j] );                
      }
    }
     
    #pragma omp parallel for schedule(dynamic) private(i, j)
    for( i = 0; i < nAllMes	; i++)
    {
      for( j = 0; j <GetMessagesFromNeighbors()[i].size(); j++)
      {                
        delete GetMessagesFromNeighbors()[i][j];
        GetMessagesFromNeighbors()[i][j] =  newMessages[i][j];
      }
    }        

    changed = 0;
    #pragma omp parallel for schedule(dynamic) private(i)// reduction(+:changed)
    for (i = 0; i < Count; i++)
    {  
      if (!areReallyObserved[connNodes[i]])
      {
        message tempBel = CPearlInfEngine::GetCurBeliefs()[i];
        CPearlInfEngine::ComputeBelief(connNodes[i]);
            
        if(!changed)
        {
          omp_set_lock(&change_lock);
          changed += !tempBel->IsEqual(CPearlInfEngine::
          GetCurBeliefs()[connNodes[i]], CPearlInfEngine::GetTolerance());
          delete tempBel;
          omp_unset_lock(&change_lock);
        }
      }
    }
    converged = !changed;
    iter++;
    }//while ((!converged)&&(iter<m_numberOfIterations))
    //compute beliefs after last passing messages
    
    omp_destroy_lock(&change_lock);
    m_IterationCounter = iter;
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::SetFarNeighbors(intVector* sinks, intVector* sources,
  int &size)
{
  size = 0;
  sources->resize(0);
  sinks->resize(0);
  
  const CGraph *pGraph = m_pGraphicalModel->GetGraph();
  
  int numOfNeighb, numOfNeighbOfNieghb;
  const int *neighbors,*neighborsOfneighbors;
  const ENeighborType *orientation,*orNeighbOfneighb;
  
  for (int i = 0; i < m_NodesOfProcess.size(); i++)
  {
    pGraph->GetNeighbors(m_NodesOfProcess[i], &numOfNeighb, &neighbors, 
      &orientation);
    for (int j = 0; j < numOfNeighb; j++)
      if (m_NodeProcesses[m_NodesOfProcess[i]] != 
        m_NodeProcesses[neighbors[j]])
      {
        pGraph->GetNeighbors(neighbors[j], &numOfNeighbOfNieghb,
          &neighborsOfneighbors, &orNeighbOfneighb);
        int k = 0;
        while (neighborsOfneighbors[k] != m_NodesOfProcess[i])
          k++;
        sources->push_back(k);
        sinks->push_back(neighbors[j]);
        size++;
      }
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::SetFarNeighbors(intVector* sinks, intVector* sources,
  intVector* messLengthSend, intVector* messLengthRecv, int &size, int Discrete_Continuous,intVecVector *neighbNumbers)
{
  size = 0;
  sources->resize(0);
  sinks->resize(0);
  
  const CGraph *pGraph = m_pGraphicalModel->GetGraph();
  
  int numOfNeighb, numOfNeighbOfNieghb;
  const int *neighbors,*neighborsOfneighbors;
  const ENeighborType *orientation,*orNeighbOfneighb;

  if (Discrete_Continuous == 0)
  {
    for (int i = 0; i < m_NodesOfProcess.size(); i++)
    {
      pGraph->GetNeighbors(m_NodesOfProcess[i], &numOfNeighb, &neighbors, 
        &orientation);
      for (int j = 0; j < numOfNeighb; j++)
        if (m_NodeProcesses[m_NodesOfProcess[i]] != 
          m_NodeProcesses[neighbors[j]])
        {
          pGraph->GetNeighbors(neighbors[j], &numOfNeighbOfNieghb,
            &neighborsOfneighbors, &orNeighbOfneighb);
          int k = 0;
          while(neighborsOfneighbors[k] != m_NodesOfProcess[i])
            k++;
          sources->push_back(k);
          sinks->push_back(neighbors[j]);
          size++;

          CDenseMatrix<float> *matForSending;
          int dataLength;
          const float *pDataForSending;
        
          matForSending = static_cast<CDenseMatrix<float>*>
            ((*GetNewMsgsFromNeighbors())[neighbors[j]][k]->GetMatrix(
            matTable));
          matForSending->GetRawData(&dataLength, &pDataForSending);
          dataLength += 2;
          (*messLengthSend)[m_NodeProcesses[neighbors[j]]] += dataLength;
        
          matForSending = static_cast<CDenseMatrix<float>*>
            ((*GetNewMsgsFromNeighbors())[m_NodesOfProcess[i]][j]->GetMatrix(
            matTable));
          matForSending->GetRawData(&dataLength, &pDataForSending);
          dataLength += 2;
          (*messLengthRecv)[ m_NodeProcesses[neighbors[j]]]+=dataLength;
        }
    }
  }
  else
  {
    neighbNumbers->resize(m_NodesOfProcess.size());
    for (int i = 0; i < m_NodesOfProcess.size(); i++)
    {
      pGraph->GetNeighbors(m_NodesOfProcess[i], &numOfNeighb, &neighbors, 
        &orientation);
      for (int j = 0; j < numOfNeighb; j++)
      {
        
          pGraph->GetNeighbors(neighbors[j], &numOfNeighbOfNieghb,
            &neighborsOfneighbors, &orNeighbOfneighb);
          int k = 0;
          while(neighborsOfneighbors[k] != m_NodesOfProcess[i])
            k++;
          (*neighbNumbers)[i].push_back(k);
          if (m_NodeProcesses[m_NodesOfProcess[i]] != 
             m_NodeProcesses[neighbors[j]])
          {
            sources->push_back(k);
            sinks->push_back(neighbors[j]);
            size++;
            int dataLength=0;
            int NumberOfDimensions = ((GetMessagesFromNeighbors()[neighbors[j]][k]->GetNodeTypesVector())->front())->GetNodeSize();
            dataLength = NumberOfDimensions + NumberOfDimensions * NumberOfDimensions;
            dataLength += 5; //sinks,cource,coeff,dimensions 
            (*messLengthSend)[m_NodeProcesses[neighbors[j]]] += dataLength;
            (*messLengthRecv)[ m_NodeProcesses[neighbors[j]]] += dataLength;
          }
      }
    }
    for (int i1 = 0; i1 < m_NumberOfProcesses; i1++)
    {
       if ((*messLengthSend)[i1]!=0) (*messLengthSend)[i1]+=1;
       if ((*messLengthRecv)[i1]!=0) (*messLengthRecv)[i1]+=1;
    }
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

void CParPearlInfEngine::CopyMatrixOfMessage(message sinkMessage,
  message sourceMessage)
{
  CNumericDenseMatrix<float>* sinkMat =
    static_cast<CNumericDenseMatrix<float>*>(sinkMessage->GetMatrix(matTable));
  CNumericDenseMatrix<float>* sourceMat =
    static_cast<CNumericDenseMatrix<float>*>(sourceMessage->GetMatrix(matTable));
  floatVector *sinkVector = (floatVector*)(sinkMat->GetVector());
  const floatVector *sourceVector = sourceMat->GetVector();
  for (int k = 0; k < sinkVector->size(); k++)
  {
    (*sinkVector)[k] = (*sourceVector)[k];
  }
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::MultiplyOnMessage(message thisData,
  const int *pBigDomain, const int *pSmallDomain, const message otherData)
{
  PNL_CHECK_IS_NULL_POINTER(pBigDomain);
  PNL_CHECK_IS_NULL_POINTER(pSmallDomain);
  PNL_CHECK_IS_NULL_POINTER(thisData);
  PNL_CHECK_IS_NULL_POINTER(otherData);
  
  if (!thisData->IsValid() || !otherData->IsValid())
  {
    PNL_THROW(CInvalidOperation, "multiply invalid data");
  }
  EDistributionType dtOther = otherData->GetDistributionType();
  if ((dtOther != dtTabular) && (dtOther != dtScalar))
  {
    PNL_THROW(CInvalidOperation, "we can multiply only tabulars")
  }

  int smIsUnit = otherData->IsDistributionSpecific();
  // check if small distribution is Unit function or scalar - do nothing
  if ((dtOther == dtScalar) || (smIsUnit == 1))
  {
    return;
  }
  
  int smallNumNodes = otherData->GetNumberOfNodes();
  int bigNumNodes = thisData->GetNumberOfNodes();

  CMatrix<float> *pSmallMatrix = otherData->GetMatrix(matTable);
  CMatrix<float>* pNewMatrix; 
  
  int location;
  int uniFlag = 0;
  float uniVal = 0.0f;
  if (thisData->IsDistributionSpecific())
  {
    if( bigNumNodes == smallNumNodes )
    {
      // we need to copy small distribFun in self
      // check the order of nodes in domain before and reduce if its need
      // find order of small in big and rearrange dims
      intVector orderOfSmallInBig;
      orderOfSmallInBig.assign(bigNumNodes, 0);
      for (int i = 0; i < smallNumNodes; i++)
      {
        location = std::find( pSmallDomain,
          pSmallDomain + bigNumNodes, pBigDomain[i] ) - pSmallDomain;
        if (location < bigNumNodes)
        {
          orderOfSmallInBig[i] = location;
        }
      }
      thisData->SetUnitValue(0);
      pNewMatrix = thisData->GetMatrix(matTable);
      pNewMatrix = otherData->GetMatrix(matTable)->ReduceOp(
        &orderOfSmallInBig.front(), bigNumNodes, 0, NULL, 
        thisData->GetMatrix(matTable));
      return;
    }
    else
    {
      thisData->SetUnitValue(0);
      pNewMatrix = thisData->GetMatrix(matTable);
      pNewMatrix->SetUnitData();
      uniFlag = 1;
      uniVal = 1.0f;
    }
  }
  else
  {
    pNewMatrix = thisData->GetMatrix(matTable);
  }
  intVector orderOfBigInSmall;
  
  orderOfBigInSmall.assign(smallNumNodes, 0);
  for(int i = 0; i < smallNumNodes; i++)
  {
    location = std::find(pBigDomain,
      pBigDomain + bigNumNodes, pSmallDomain[i]) - pBigDomain;
    if(location < bigNumNodes)
    {
      orderOfBigInSmall[i] = location;
    }
  }
  
  ((iCNumericDenseMatrix< float > *)pNewMatrix)->MultiplyInSelf_plain(
    pSmallMatrix, orderOfBigInSmall.size(), &orderOfBigInSmall.front(),
    uniFlag, uniVal);
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::MarginalizeDataInMessage(CDistribFun *pUpdateData,
  const CDistribFun *pOldData, const int *DimOfKeep, int NumDimsOfKeep,
  int maximize)
{
  CDenseMatrix<float>* pOldMatrix = 
    static_cast<CDenseMatrix<float>*>(pOldData->GetMatrix(matTable));

  CDenseMatrix<float>* pNewMatrix = 
    static_cast<CDenseMatrix<float>*>(pUpdateData->GetMatrix(matTable));
  int i, j, k;
  int new_dims, old_dims;
  const int* new_ranges;
  const int* old_ranges;

  pOldMatrix->GetRanges(&old_dims, &old_ranges);
  pNewMatrix->GetRanges(&new_dims, &new_ranges);
  int new_bulk_size, old_bulk_size;
  float* old_bulk;
  float* new_bulk;
  pOldMatrix->GetRawData(&old_bulk_size, (const float**)(&old_bulk));
  pNewMatrix->GetRawData(&new_bulk_size, (const float**)(&new_bulk));
  intVector steps(old_dims);
  intVector backsteps(old_dims);
  intVector stats(old_dims);
  
  j = old_dims;
  while (j--)
  {
    stats[j] = 0;
  }
  for (i = old_dims; i--;)
  {
    steps[i] = 0;
  }
  for (i = NumDimsOfKeep, j = 1; i--;)
  {
    steps[DimOfKeep[i]] = j;
    j *= old_ranges[DimOfKeep[i]];
  }
  for (i = old_dims; i--;)
  {
    backsteps[i] = steps[i] * old_ranges[i];
  }
  
  if (maximize == 0)
  {
    for (i = new_bulk_size; i--;)
    {
      new_bulk[i] = 0.f;
    }
    for (i = 0, j = 0; i < old_bulk_size; ++i)
    {
      for (k = old_dims-1; stats[k] == old_ranges[k];)
      {
        stats[k] = 0;
        j -= backsteps[k--];
        ++stats[k];
        j += steps[k];
      }
      new_bulk[j] += old_bulk[i];
      ++stats[old_dims - 1];
      j += steps[old_dims - 1];
    }
  }
  else
  {
    for (i = new_bulk_size; i--;)
    {
      new_bulk[i] = -FLT_MAX;
    }
    for (i = 0, j = 0; i < old_bulk_size; ++i)
    {
      for (k = old_dims-1; stats[k] == old_ranges[k];)
      {
        stats[k] = 0;
        j -= backsteps[k--];
        ++stats[k];
        j += steps[k];
      }
      if (old_bulk[i] > new_bulk[j])
        new_bulk[j] = old_bulk[i];
      ++stats[old_dims - 1]; 
      j += steps[old_dims - 1];
    }
  }
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::NormalizeDataInMessage(CDistribFun* Data)
{
    PNL_CHECK_IS_NULL_POINTER(Data);
    
    CNumericDenseMatrix<float>* pDataMatrix;
    pDataMatrix = static_cast<CNumericDenseMatrix<float>*>(
        Data->GetMatrix(matTable));
    floatVector* RawData = (floatVector*)pDataMatrix->GetVector();
    int dataLength = RawData->size();

    float sum = pDataMatrix->SumAll();
    if( sum > FLT_MIN*10.f )
    {
        if( fabs(sum - 1) > 0.00001f )
        {
            float reciprocalSum = 1 / sum;
            floatVector::iterator it = RawData->begin();
            floatVector::iterator itE = RawData->end();
            for ( ; it != itE; it++ )
            {
                (*it) *= reciprocalSum;
            }
        }
    }
    else
    {
        sum = dataLength;
        float reciprocalSum = 1 / sum;
        floatVector::iterator it = RawData->begin();
        floatVector::iterator itE = RawData->end();
        for ( ; it != itE; it++ )
        {
            (*it) *= reciprocalSum;
        }
    }
}
// ----------------------------------------------------------------------------

messageVecVector* CParPearlInfEngine::GetCurMsgsFromNeighbors()
{
  return &m_msgsFromNeighbors[m_index];
}
// ----------------------------------------------------------------------------

messageVecVector* CParPearlInfEngine::GetNewMsgsFromNeighbors()
{
  int index = m_index == 0 ? 1 : 0;
  return &m_msgsFromNeighbors[index];
}
// ----------------------------------------------------------------------------

messageVector& CParPearlInfEngine::GetCurBeliefs()
{
  return m_bels[m_index];
}
// ----------------------------------------------------------------------------

messageVector& CParPearlInfEngine::GetOldBeliefs()
{
  int index = m_index == 0 ? 1 : 0;
  return m_bels[index];
}
// ----------------------------------------------------------------------------

void CParPearlInfEngine::GoNextIteration()
{
  m_index = m_index == 0 ? 1 : 0;
}
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::DivideGrille()
{
  if (!IsModelGrille())
  PNL_THROW(CInconsistentType, "Must be grille!!!")
  
  // Graphical model
  const CMNet *pMNet = static_cast<const CMNet *>( m_pGraphicalModel );
  // Number of nodes in model
  const int NNodes = pMNet->GetNumberOfNodes();

  int IndexInCorner4Nodes = 0;
  intVector Corner4Nodes;
  intVector AllNodesRank;
  intVector clqOut;
  int CornerNodeForChain = -1;

  boolVector RenumberNodes(NNodes, false);
  intVector PrevLayer;
  intVector NextLayer;
  
  //New number -> number in model
  intVector NewNumbers(NNodes, -1);

  Corner4Nodes.resize(4);
  AllNodesRank.resize(NNodes);

  //Last used number (element in array NewNumbers)
  int LastUsedNumber = -1;
  int Node, i, j;
  
  for (Node = 0; (Node < NNodes) || (CornerNodeForChain!=-1); Node++)
  {
    clqOut.clear();
    pMNet->GetClqsNumsForNode( Node, &clqOut);
    
    AllNodesRank[Node] = clqOut.size();
    
    if (AllNodesRank[Node] == 2)
      Corner4Nodes[IndexInCorner4Nodes++] = Node;
    else
      if (AllNodesRank[Node] == 1)
        CornerNodeForChain = Node;
  }
  
  if (CornerNodeForChain != -1)
  {
    RenumberNodes[CornerNodeForChain] = true;
    NewNumbers[0] = CornerNodeForChain;
    PrevLayer.resize(1);
    PrevLayer[0] = CornerNodeForChain;
  }
  else
  {
    bool Stop = false;
    
    intVector Road1(0), Road2(0);
    int FirstNode = Corner4Nodes[0];
    Road1.push_back(FirstNode);
    Road2.push_back(FirstNode);
    intVector nbr;
    nbr.clear();
    GetNbrsInGrilleModel(FirstNode, &nbr);
    Road1.push_back(nbr[0]);
    Road2.push_back(nbr[1]);
    if (AllNodesRank[nbr[0]] == 2)
    {
      PrevLayer = Road1;
      Stop = true;
    }
    
    if (AllNodesRank[nbr[1]] == 2)
    {
      PrevLayer = Road2;
      Stop = true;
    }
    
    while (!Stop)
    {
      intVector nbrs1, nbrs2;
      nbrs1.clear(); nbrs2.clear();
      
      GetNbrsInGrilleModel(Road1.back(), &nbrs1);
      GetNbrsInGrilleModel(Road2.back(), &nbrs2);
      
      int nbrsSize1 = nbrs1.size();
      int nbrsSize2 = nbrs2.size();
      
      for (j = 0; j< nbrsSize1; j++)
      {
        if ((AllNodesRank[nbrs1[j]] == 2)||(AllNodesRank[nbrs1[j]] == 3))
        {
          if (nbrs1[j]!=Road1[Road1.size()-2])
          {
            Road1.push_back(nbrs1[j]);
            break;
          }
        }
      }
          
      for (j = 0; j< nbrsSize2; j++)
      {
        if ((AllNodesRank[nbrs2[j]] == 2) || (AllNodesRank[nbrs2[j]] == 3))
        {
          if (nbrs2[j]!=Road2[Road2.size() - 2])
          {
            Road2.push_back(nbrs2[j]);
            break;
          }
        }
      }

      if (AllNodesRank[Road1.back()] == 2)
      {
        PrevLayer = Road1;
        Stop = true;
      }
      
      if (AllNodesRank[Road2.back()] == 2)
      {
        PrevLayer = Road2;
        Stop = true;
      }
    }
  }
  
  int SizePL = PrevLayer.size();
  for (i =0; i<SizePL; i++)
  {
    RenumberNodes[PrevLayer[i]] = true;
    NewNumbers[++LastUsedNumber] = PrevLayer[i];
  }
  
  while (LastUsedNumber < (NNodes - 1))
  {
    SizePL = PrevLayer.size();
    NextLayer.resize(SizePL);
    for (i = 0; i < SizePL; i++)
    {
      intVector nbrs(0);
      GetNbrsInGrilleModel(PrevLayer[i], &nbrs);
      
      int SizeNbrs = nbrs.size();
      for (j = 0; j < SizeNbrs; j++)
        if (!RenumberNodes[nbrs[j]])
        {
          NextLayer[i] = nbrs[j];
          break;
        }
    }
    
    PrevLayer = NextLayer;
    
    for (i = 0; i<SizePL; i++)
    {
      RenumberNodes[PrevLayer[i]] = true;
      NewNumbers[++LastUsedNumber] = PrevLayer[i];
    }
  }
  
  int Norm = NNodes / m_NumberOfProcesses;
  m_NodesOfProcess.clear();
  m_NodeProcesses.resize(NNodes, -1);
  
  for (Node = 0; Node < NNodes; Node++)
  {
    int ProcessNumber;
    
    if (Norm == 0)
      ProcessNumber = m_NumberOfProcesses - 1;
    else 
      ProcessNumber = Node / Norm;
    
    ProcessNumber = ProcessNumber % m_NumberOfProcesses;
    
    m_NodeProcesses[Node] = ProcessNumber;
    
    if (m_MyRank == ProcessNumber)
    {
      m_NodesOfProcess.push_back(Node);
    }
  }
  
  m_NumberOfUsedProcesses = m_NumberOfProcesses;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::GetNbrsInGrilleModel(int Node, intVector *pnbr)
{
  if (!IsModelGrille())
    return;

  pnbr->clear();
  //Graph model
  const CMNet *pMNet = static_cast<const CMNet *>(m_pGraphicalModel);
  //Numbers of clqs
  intVector clqOut;
  pMNet->GetClqsNumsForNode(Node, &clqOut);
  //Number of cliques
  int size = clqOut.size();

  for (int i = 0; i<size; i++)
  {
    //Numbers in clique i
    intVector clique;
    pMNet->GetClique(clqOut[i], &clique);
    if (clique[0] != Node)
      pnbr->push_back(clique[0]);
    else
      pnbr->push_back(clique[1]);
  }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
bool CParPearlInfEngine::IsModelGrille()
{
  /*Will be done later*/
  return m_IsMRF2Grid;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParPearlInfEngine::SetIsMRF2Grille(bool Val)
{
  m_IsMRF2Grid = Val;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------
#ifdef PNL_RTTI
const CPNLType CParPearlInfEngine::m_TypeInfo = CPNLType("CParPearlInfEngine", &(CPearlInfEngine::m_TypeInfo));

#endif

#endif // PAR_PNL

// end of file ----------------------------------------------------------------
