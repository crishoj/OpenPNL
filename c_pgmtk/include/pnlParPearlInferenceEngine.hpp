/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlParPearlInferenceEngine.hpp                              //
//                                                                         //
//  Purpose:   CParPearlInfEngine class definition                         //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLPARPEARLINFERENCEENGINE_HPP__
#define __PNLPARPEARLINFERENCEENGINE_HPP__

#include "pnlParConfig.hpp"

#ifdef PAR_PNL

#include "pnlPearlInferenceEngine.hpp"
#include <set>

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CParPearlInfEngine : public CPearlInfEngine
{
public:

    // static function of parallel Pearl Inf creation
    static CParPearlInfEngine* Create(const CStaticGraphicalModel *pGrModel);

    static void Release(CParPearlInfEngine **PearlInfEngine);

#ifdef PAR_MPI
    virtual void EnterEvidence(const CEvidence *evidence, int maximize = 0,
        int sumOnMixtureNode = 1);
#endif
    
#ifdef PAR_OMP
    virtual void EnterEvidenceOMP(const CEvidence *evidence, int maximize = 0,
        int sumOnMixtureNode = 1);
#endif

#ifdef PAR_MPI
    void SetCollectRanks(int Count, int *pCollectRanks);
#endif

    void MarginalNodes(const int* query, int querySize, int notExpandJPD = 0);
    
    void SetIsMRF2Grille(bool Val);

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CParPearlInfEngine::m_TypeInfo;
  }
#endif
protected:

    CParPearlInfEngine(const CStaticGraphicalModel *pGraphicalModel);

    ~CParPearlInfEngine();

    void InitEngine(const CEvidence*evidence);

    void InitMessages(const CEvidence* evidence);

    // compute message and put it to corresponding element of 
    // m_msgsFromNeighbors
    void ComputeMessage(int destination, int source);

    // compute pi and put it to corresponding element of m_productPi
    void ComputeProductPi(int nodeNumber, int except = -1);

    // compute lambda and put it to corresponding element of m_productLambda
    void ComputeProductLambda(int nodeNumber, message lambda, CDistribFun * res, 
        int except = -1);

    // compute lambda and put it to corresponding element of m_productLambda
    void ProductLambdaMsgs( int nodeNumber, int except = -1);

    // compute belief and put it to corresponding element of m_bels
    void ComputeBelief(int nodeNumber);

    // return current VecVector of m_bels
    virtual messageVector& GetCurBeliefs();

#ifdef PAR_MPI
    void ParallelProtocol();
    void ParallelProtocolContMPI();

#endif

#ifdef PAR_OMP
    void ParallelProtocolOMP();
    void ParallelProtocolContOMP();
#endif

    void CPD_to_pi(int nodeNumber, CDistribFun *const* allPiMessages,
        int *multParentIndices, int numMultNodes, int posOfExceptParent = -1, 
        int maximizeFlag = 0);

    void CPD_to_lambda(int nodeNumber, CDistribFun *const* allPiMessages,
        int *multParentIndices, int numMultNodes, CDistribFun* res,
        int posOfExceptNode = -1, int maximizeFlag = 0);

    // marginalize matrix of message and put result to pUpdateData
    void MarginalizeDataInMessage(CDistribFun *pUpdateData,
        const CDistribFun *pOldData, const int *DimOfKeep, int NumDimsOfKeep,
        int maximize);

    // normalize matrix of message
    void NormalizeDataInMessage(CDistribFun* Data);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:

    messageVector m_productPi;           // facient pi of messages for each node
    messageVector m_productLambda;       // facient lambda of messages for each 
                                         // node
    messageVector m_factorDistrib;       // distributions of nodes (for bayessian
                                         // networks)
    messageVecVector m_factorCliqueDistrib; // distributions of cliques (for MRF2)

    messageVecVector m_msgsFromNeighbors[2]; // messages from node to node 
                                             // for each pair of neighbors 
    messageVector m_bels[2];             //beliefs for every node
    int m_index; // index 0 or 1 to show what of m_msgsFromNeighbors is current
                 // and what of m_bels is current
    // we use 2 VecVectores m_msgsFromNeighbors and 2 vectors m_bels. The first 
    // iteration fills 1st of that vectores, the second iteration fills - 2nds,
    // the next itaration - 1st, the next - 2nd etc. So we always have messages
    // and beliefs for 2 successive iterations and we don't create new objects
    // every itaration. The vector that is changed this iteration we call 
    // current, the other that stayed from previous iteration we call old.
    bool m_IsMRF2Grid;

#ifdef PAR_MPI
    set<int> m_UsedNodes;                // used in method that divide nodes
                                         // between processes 
    doubleVector *m_pWeightesOfNodes;    // array of weightes of cliques
    doubleVector *m_pWeightesOfSubTrees; // array of weightes of subtrees
    int m_MyRank;                        // process number
    int m_NumberOfProcesses;             // max amount of processes
    int m_NumberOfUsedProcesses;         // amount of really used processes
    intVector m_Roots;                   // for every process it's root of
                                         // subtree
    intVector m_CollectRanks;            // process on which beliefs must be
                                         // collected after EnterEvidence
    int m_RootNode;                      // root node
                                         // if -1, root node is undefine now
                                         // (need to run procedure GetRootNode)
    intVector m_NodeProcesses;           // number of process for each node
    intVector m_NodesOfProcess;          // numbers of nodes of this process
    const CEvidence *m_pMyEvidence;      // evidence (sets in DivideNodes)
    bool m_NeedToRebuildWeightesOfNodes; // need to rebuild weightes of nodes 
                          // (only GetWeightOfSubTreeWithReCalculating uses it)
    CGraph *m_pSkeleton;                 // skeleton of graph
    intVector *m_pTreeGraphAccord;       // array of accords between tree 
                                         // (m_pSkeleton) and graph
#endif // PAR_MPI

    // copy matrix of message sourceMessage to matrix of sinkMessage
    void CopyMatrixOfMessage(message sinkMessage, message sourceMessage);

    // multiply matrix of message thisData into matrix of otherData
    void MultiplyOnMessage(message thisData, const int *pBigDomain,
        const int *pSmallDomain, const message otherData);

    // return current VecVector of m_msgsFromNeighbors
    messageVecVector* GetCurMsgsFromNeighbors();

    // return new VecVector of m_msgsFromNeighbors
    messageVecVector*GetNewMsgsFromNeighbors();

    // return old VecVector of m_bels
    messageVector& GetOldBeliefs();

    // change current of vectors m_msgsFromNeighbors, m_bels current
    void GoNextIteration();

    // function gets number of nodes 
    int GetNumberOfNodes();

    // get Parents of node NumOfNode
    void GetParents(int NumOfNode, intVector *pParents);

#ifdef PAR_MPI
    // initialization of consts from mpi framework
    void InitMPIConsts();

    // init of arrays m_pWeightesOfNodes m_pWeightesOfSubTrees (set NULL)
    void InitWeightArrays();

    // divide nodes between processes
    void DivideNodes(const CEvidence *pEvidence, int AlgType = 2);

    // build skeleton of the graph
    void PickOutSkeleton();

    // set list of nodes (sequence of including in procedure of deviding tree)
    void SetNodesList(list<int> *pNodesList);

    // return to CurrentRoot next free node (using m_UsedNodes set) and change
    // NodesListIterator, CurrentRoot, PrevCurrentRoot (PrevCurrentNode sets -1)
    // if there are no free nodes, function sets -1 to *CurrentRoot
    void GetNextNodeInList(const list <int> *pNodesList, 
        list <int>::const_iterator *pNodesListIterator, int *pCurrentRoot,
        int *pPrevCurrentRoot);

    // get weight of subtree 
    double GetWeightOfSubTree(int NumOfNode);

    // function gets root node of the tree
    int GetRootNode();

    // get parent in skeleton 
    void GetParentsInSkeleton(int NumOfNode, intVector *pParents);

    // return node with min deviation from W*
    int GetNodeWithOptimalSubTreeWeight(int CurrentRoot, int PrevCurrentRoot,
        double WThreshold);

    // return node with min deviation from W* 
    // (select from CurentRoot and it's children)
    // there are no rebuild of weightes
    int GetNodeWithOptimalSubTreeWeight(int CurrentRoot, double WThreshold);

    // add info to arrays: m_NodeProcesses, m_NodesOfProcess 
    // (set info to the process LastUsedProcess + 1)
    void SendASubTreeToNextProcess(int A, int LastUsedProcess);

    // add to the m_UsedNodes nodes of A - subtree 
    void AddNodesNumbersOfSubTreeToUsedNodes(int A);

    // build m_pWeightesOfNodes array (create and fill values)
    void BuildWeightesOfNodes();

    // build m_pWeightesOfSubTrees array (create and fill values)
    void BuildWeightesOfSubTrees();

    // return number of unused nodes in list
    int GetNumberOfFreeNodesInList(const list<int> *pNodesList, 
        list<int>::const_iterator *pNodesListIterator);

    // get weight of edge. If there are no edges between nodes returns -1.
    double GetWeightOfEdge(int RootFrom, int RootTo);

    // get Children in skeleton
    void GetChildrenInSkeleton(int NumOfNode, intVector *pChildren);

    // get weight of subtree with recalculating
    double GetWeightOfSubTreeWithReCalculating(int NumOfNode, 
        doubleVector *pWeightArray = NULL, bool NeedToCountUpUsedNodes = false);

    // get weight of node with recalculating
    double GetWeightOfNodeWithReCalculating(int NumOfNode, 
        bool NeedToCountUpUsedNodes = false);

    // convert number of figure from skeleton to graph (using m_pTreeGraphAccord)
    int NumberFromGraphToSkeleton(int NumberInGraph);

    // convert number of figure from graph to skeleton (using m_pTreeGraphAccord)
    int NumberFromSkeletonToGraph(int NumberInSkeleton);

    // function gets unused nodes in subtree
    void GetUnusedNodesInSubTree(int SubTreeRoot, 
        intVector *pUnusedNodesInSubTree);

    // divide nodes between processes
    void DivideNodesWithMinimumDeviationSearch(const CEvidence *pEvidence);

    // return node with min deviation from W* (select from all nodes)
    int GetNodeWithMinimumDeviation(double WThreshold);

    // collect parallel EnterEvidence results on choosen (m_CollectRanks)
    // processes
    void CollectBeliefsOnProcess(int MainProcNum);
    void CollectBeliefsOnProcessContMPI(int MainProcNum);

    // set necessary parameters for message sending via MPI
    void SetFarNeighbors(intVector *sinks, intVector *sources, int &size);

    // set necessary parameters for message sending via MPI
    void SetFarNeighbors(intVector* sinks, intVector* sources, 
        intVector* messLengthSend, intVector* messLengthRecv, int &size, int Discrete_Continuous,intVecVector *neighbNumbers);

    // divide nodes between processes if model is grille
    void DivideGrille();

    //Return an answer "Is model grille?"
    bool IsModelGrille();

    //Get neighbours
    void GetNbrsInGrilleModel(int Node, intVector *pnbr);
#endif // PAR_MPI

};

PNL_END

#endif // PAR_PNL

#endif // __PNLPARPEARLINFERENCEENGINE_HPP__
