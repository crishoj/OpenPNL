/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlParJtreeInferenceEngine.hpp                              //
//                                                                         //
//  Purpose:   CParJTreeInfEngine class definition                         //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLPARJTREEINFERENCEENGINE_HPP__
#define __PNLPARJTREEINFERENCEENGINE_HPP__

#include "pnlParConfig.hpp"

#ifdef PAR_PNL

#include "pnlJtreeInferenceEngine.hpp"
#include <set>

#ifdef PAR_MPI
#include <mpi.h>
#endif

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 
PNL_BEGIN

class PNL_API CParJtreeInfEngine : public CJtreeInfEngine
{
public:

    // static functions of parallel Junction Tree Inf creation
    static CParJtreeInfEngine* Create(const CStaticGraphicalModel
        *pGraphicalModelIn, const intVecVector& 
        SubgrToConnectIn = intVecVector());

    static CParJtreeInfEngine* Copy(const CParJtreeInfEngine *pJTreeInfEngIn);

#ifdef PNL_OBSOLETE
    static CParJtreeInfEngine* Create(const CStaticGraphicalModel
        *pGraphicalModelIn, int numOfSubGrToConnect,
        const int *SubGrToConnectSizesIn, const int **SubgrToConnectIn);
#endif

#ifndef SWIG
    static CParJtreeInfEngine* Create(const CStaticGraphicalModel
        *pGraphicalModelIn, CJunctionTree *pJTreeIn);
#endif

    static void Release(CParJtreeInfEngine **JtreeInfEngine);
    
    virtual ~CParJtreeInfEngine();

#ifdef PAR_OMP
    void EnterEvidenceOMP(const CEvidence *pEvidenceIn,
        int maximize = 0, int sumOnMixtureNode = 1);

    void CollectEvidenceOMP();
    void CollectEvidenceOMP_gau();

    void DistributeEvidenceOMP();

    void ShrinkObservedOMP( const CEvidence *pEvidenceIn,
        int maximize = 0, int sumOnMixtureNode = 1,
        bool bRebuildJTree = true );
#endif // PAR_OMP

#ifdef PAR_MPI
    virtual void EnterEvidence(const CEvidence *pEvidenceIn, 
        int maximize = 0, int sumOnMixtureNode = 1);

    virtual void CollectEvidence();
    
    virtual void DistributeEvidence();

    // method sets ranks of processes that will be collect results in 
    // MPI mode
    void SetCollectRanks(int Count, int *pCollectRanks);
#endif // PAR_MPI

#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CParJtreeInfEngine::m_TypeInfo;
  }
#endif
protected:

    // ctors
    CParJtreeInfEngine(const CStaticGraphicalModel *pGraphicalModel,
        int numOfSubGrToConnect, const int *SubGrToConnectSizes,
        const int **SubgrToConnect);

    CParJtreeInfEngine(const CStaticGraphicalModel *pGraphicalModel,
        CJunctionTree *pJTree);

    CParJtreeInfEngine(const CJtreeInfEngine& rJTreeInfEngine);

    CParJtreeInfEngine(const CParJtreeInfEngine& rParJTreeInfEngine);

#ifdef PAR_OMP
    virtual void PropagateBetweenClqsOMP(int source, int sink,
        bool isCollect);
#endif // PAR_OMP

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 
private:

    intQueue              m_QueueNodes;     // queue for cliques
    intVecVector          m_NodeConditions; // conditions of cliques
    potsPVecVector        m_UpdateRatios;   // update ratio for each clique
    neighborTypeVecVector m_NbrsTypes;      // parent - child

    // fill vector m_NbrsTypes - types of neighbours: parent or child
    void SetNbrsTypes();

    // return parents of choosen node
    void GetParents(int NumOfNode, intVector *Parents);

    // init some data structures for CollectEvidence
    void InitNodeConditionsAndUpdateRatios();

    // get type of jtree (all cliques are tabular, gaussian or scalar)
    EDistributionType GetJTreeType();

#ifdef PAR_MPI
    intVector m_NodeProcesses;          // number of process for each clique
    intVector m_NodesOfProcess;         // numbers of cliques of this process
    int m_NumberOfProcesses;            // max amount of processes
    int m_NumberOfUsedProcesses;        // amount of really used processes
    int m_MyRank;                       // process number
    intVector m_CollectRanks;           // process on which must collect factors
                                        // after EnterEvidence
    doubleVector *m_pWeightesOfNodes;   // array of weightes of cliques
    doubleVector *m_pWeightesOfSubTrees;// array of weightes of subtrees
    set<int> m_UsedNodes;               // used in divide nodes between
                                        // processes method
    intVector m_Roots;                  // for every process it's root of
                                        // subtree
    intVecVector m_Routes;              // routes of nodes 
    boolVector m_IsPropagated;          // flag for each node of this process:
                                        // is it multiplied on update ratio
                                        // recieved from its parent. Used only
                                        // in DistributeEvidence

    MPI_Group World_group, CollectEv_group;
    MPI_Comm CollectEv_comm;

    // init consts from mpi framework
    void InitMPIConsts();

    // init arrays m_pWeightesOfNodes, m_pWeightesOfSubTrees (set NULL)
    void InitWeightArrays();

    // init queue for CollectEvidence
    void InitQueueNodes();

    // collect parallel EnterEvidence results on choosen (m_CollectRanks)
    // processes
    void CollectFactorsOnProcess(int MainProcNum);

    // calculate tag range for MPI send-receive operations
    int GetMaxTagForCollectEvidence();

    // divide cliques of the tree between processes
    void DivideNodes(int AglType = 2);

    // set list of nodes (sequence of including in procedure of deviding tree)
    void SetNodesList(list<int> *pNodesList);

    // return to CurrentRoot next free node (using m_UsedNodes set) and change
    // NodesListIterator, CurrentRoot, PrevCurrentRoot (PrevCurrentNode sets -1)
    // if there are no free nodes, function sets -1 to *CurrentRoot
    void GetNextNodeInList(const list <int> *pNodesList,
        list <int>::const_iterator *pNodesListIterator, int *pCurrentRoot,
        int *pPrevCurrentRoot);

    // get weight of a subtree 
    double GetSubTreeWeight(int nodenum);

    // return node with min deviation from W*
    // (from CurrentRoot and PrevCurrentRoot)
    int GetNodeWithOptimalSubTreeWeight(int CurrentRoot,
        int PrevCurrentRoot, double WThreshold);

    // return node with min deviation from W*
    // (from CurrentRoot and all it's children)
    int GetNodeWithOptimalSubTreeWeight(int CurrentRoot,
        double WThreshold);

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

    // return children of choosen node
    void GetChildren(int NumOfNode, intVector *Children);

    // gets unused nodes in subtree
    void GetUnusedNodesInSubTree(int SubTreeRoot,
        intVector *pUnusedNodesInSubTree);

    // return weight of choosen clique
    double GetWeightOfTabularClq(int NumOfClq);
    double GetWeightOfContinuousClq(int NumOfClq);

    // return "base" weight of choosen clique (used in BalanceTree)
    double GetBaseWeightOfClq(int NumOfClq);

    // is choosen node observed
    bool IsInActuallyObsNodes(int NumOfNode);

    // divide nodes with minimum deviation search from all nodes
    void DivideNodesWithMinimumDeviationSearch();

    // return node with minimum deviation from W* (select from all nodes)
    int GetNodeWithMinimumDeviation(double WThreshold);

    // calculate num of node from Tag
    int NumOfSendedNode(int Tag);
    int NumOfRecvedNode(int Tag);

    // calculate Tag for MPI send-receive operations
    int GetTagForSending(int sourceNode, int sinkNode);

    // create MPI communicator for CollectEvidence
    void CollectCommCreate();

    // build routes from root clique of the tree to root cliques of all subtrees
    void BuildRoutes();

    // process routes from root clique of the tree to root cliques of all subtrees
    void ProcessRoutes();

    // move the main root in the tree to balance weights of subtrees 
    // (used in case of two subtrees only)
    void BalanceTree(); 
#endif // PAR_MPI

#ifdef PAR_OMP
    // initialize some data structures for DistributeEvidenceOMP
    void InitNodeConditions();

    int GetDataForMargAndMultOMP(const int source, const int sink, 
        CNumericDenseMatrix<float> **margMatrix, int **dims_to_keep,
        int &num_dims_to_keep, CNumericDenseMatrix<float> **outputMatrix,
        CNumericDenseMatrix<float> **sinkMatrix, int **dims_to_mul, 
        int &num_dims_to_mul);

    void DoPropagateOMP(CNumericDenseMatrix<float> *sourceMatrix, 
        int *dims_to_keep, int num_dims_to_keep, 
        CNumericDenseMatrix<float> *sepMatrix, 
        CNumericDenseMatrix<float> *sinkMatrix, int *dims_to_mul, 
        int num_dims_to_mul, bool isCollect);

    // init queue for CollectEvidence
    void InitQueueNodesOMP();
    // ____________________________________________________________
#endif // PAR_OMP

};

PNL_END

#endif // PAR_PNL

#endif // __PNLPARJTREEINFERENCEENGINE_HPP__
