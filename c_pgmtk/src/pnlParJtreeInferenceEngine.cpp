/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlParJtreeInferenceEngine.cpp                              //
//                                                                         //
//  Purpose:   CParJTreeInfEngine class member functions implementation    //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnlConfig.hpp"
#include "pnlParJtreeInferenceEngine.hpp"

#ifdef PAR_PNL

#include "pnlTabularDistribFun.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlScalarPotential.hpp"
#include "pnlGaussianPotential.hpp"

#ifdef PAR_OMP
#include <omp.h>
#endif

PNL_USING

// ----------------------------------------------------------------------------

CParJtreeInfEngine* CParJtreeInfEngine::Create(const CStaticGraphicalModel 
    *pGraphicalModel, const intVecVector& SubgrToConnect)
{
    if(SubgrToConnect.size() == 0)
    {
        return Create(pGraphicalModel, 0, NULL, NULL);
    }
    else
    {
        int i;
        int numOfSubGrToConnect = SubgrToConnect.size();

        intVector SubgrToConnectSizes(numOfSubGrToConnect);
        pnlVector< const int * > pSubgrToConnect(numOfSubGrToConnect);

        for(i = 0; i < numOfSubGrToConnect; i++)
        {
            SubgrToConnectSizes[i] = SubgrToConnect[i].size();
            pSubgrToConnect[i] = &SubgrToConnect[i].front();
        }

        return Create(pGraphicalModel, numOfSubGrToConnect,
            &SubgrToConnectSizes.front(), &pSubgrToConnect.front());
    }
}
// ----------------------------------------------------------------------------

CParJtreeInfEngine* CParJtreeInfEngine::Create(const CStaticGraphicalModel 
    *pGraphicalModel, int numOfSubGrToConnect, const int *SubGrToConnectSizes,
    const int **SubgrToConnect)
{
    PNL_CHECK_IS_NULL_POINTER(pGraphicalModel);

    CParJtreeInfEngine *pJTreeInfEngine = new CParJtreeInfEngine(pGraphicalModel,
        numOfSubGrToConnect, SubGrToConnectSizes, SubgrToConnect);

    PNL_CHECK_IF_MEMORY_ALLOCATED(pJTreeInfEngine);

    return pJTreeInfEngine;
}
// ----------------------------------------------------------------------------

CParJtreeInfEngine* CParJtreeInfEngine::Create(const CStaticGraphicalModel
    *pGraphicalModel, CJunctionTree *pJTree)
{
    PNL_CHECK_IS_NULL_POINTER(pGraphicalModel);
    PNL_CHECK_IS_NULL_POINTER(pJTree);

    CParJtreeInfEngine *pJTreeInfEngine =
        new CParJtreeInfEngine(pGraphicalModel, pJTree);

    PNL_CHECK_IF_MEMORY_ALLOCATED(pJTreeInfEngine);

    return pJTreeInfEngine;
}
// ----------------------------------------------------------------------------

CParJtreeInfEngine::~CParJtreeInfEngine()
{
#ifdef PAR_MPI
    if (m_pWeightesOfNodes != NULL)
        delete m_pWeightesOfNodes;
    
    if (m_pWeightesOfSubTrees != NULL)
        delete m_pWeightesOfSubTrees;
#endif // PAR_MPI
}
// ----------------------------------------------------------------------------

void CParJtreeInfEngine::Release(CParJtreeInfEngine **JtreeInfEngine)
{
    delete *JtreeInfEngine;
    *JtreeInfEngine = NULL;
}
// ----------------------------------------------------------------------------

CParJtreeInfEngine::CParJtreeInfEngine(const CStaticGraphicalModel
    *pGraphicalModel, int numOfSubGrToConnect, const int *SubGrToConnectSizes,
    const int **SubgrToConnect): CJtreeInfEngine(pGraphicalModel, 
    numOfSubGrToConnect, SubGrToConnectSizes, SubgrToConnect)
{
#ifdef PAR_MPI
    InitMPIConsts();
    InitWeightArrays();
    
    m_NumberOfUsedProcesses = -1;
    m_UsedNodes.clear();
#endif // PAR_MPI

    SetNbrsTypes();

#ifdef PAR_MPI
    m_NodeProcesses.clear();
    m_NodesOfProcess.clear();
    m_Roots.clear();
    m_CollectRanks.resize(1);
    m_CollectRanks[0] = 0;
#endif // PAR_MPI
}
// ----------------------------------------------------------------------------

CParJtreeInfEngine::CParJtreeInfEngine(const CStaticGraphicalModel
    *pGraphicalModel, CJunctionTree *pJTree): CJtreeInfEngine(pGraphicalModel,
    pJTree)
{
#ifdef PAR_MPI
    InitMPIConsts();
    InitWeightArrays();
    
    m_NumberOfUsedProcesses = -1;
    m_UsedNodes.clear();
#endif // PAR_MPI

    SetNbrsTypes();

#ifdef PAR_MPI
    m_NodeProcesses.clear();
    m_NodesOfProcess.clear();
    m_Roots.clear();
    m_CollectRanks.resize(1);
    m_CollectRanks[0] = 0;
#endif // PAR_MPI
}
// ----------------------------------------------------------------------------

CParJtreeInfEngine::CParJtreeInfEngine(const CJtreeInfEngine& rJTreeInfEngine):
    CJtreeInfEngine(rJTreeInfEngine)
{
#ifdef PAR_MPI
    InitMPIConsts();
    InitWeightArrays();
    
    m_NumberOfUsedProcesses = -1;
    m_UsedNodes.clear();
#endif // PAR_MPI

    SetNbrsTypes();

#ifdef PAR_MPI
    m_NodeProcesses.clear();
    m_NodesOfProcess.clear();
    m_Roots.clear();
    m_CollectRanks.resize(1);
    m_CollectRanks[0] = 0;
#endif // PAR_MPI
}
// ----------------------------------------------------------------------------

CParJtreeInfEngine::CParJtreeInfEngine(const CParJtreeInfEngine& rParJTreeInfEngine):CJtreeInfEngine((CJtreeInfEngine &)rParJTreeInfEngine)
{
    m_QueueNodes = rParJTreeInfEngine.m_QueueNodes;     
    m_NodeConditions = rParJTreeInfEngine.m_NodeConditions; 
    m_UpdateRatios = rParJTreeInfEngine.m_UpdateRatios;   
    m_NbrsTypes = rParJTreeInfEngine.m_NbrsTypes;     

    SetNbrsTypes();

#ifdef PAR_MPI
    m_NodeProcesses = rParJTreeInfEngine.m_NodeProcesses;         
    m_NodesOfProcess = rParJTreeInfEngine.m_NodesOfProcess;        
    m_NumberOfProcesses = rParJTreeInfEngine.m_NumberOfProcesses;     
    m_NumberOfUsedProcesses = rParJTreeInfEngine.m_NumberOfUsedProcesses; 
    m_MyRank = rParJTreeInfEngine.m_MyRank;                
    m_CollectRanks = rParJTreeInfEngine.m_CollectRanks;          
                             
    m_pWeightesOfNodes = rParJTreeInfEngine.m_pWeightesOfNodes;   
    m_pWeightesOfSubTrees = rParJTreeInfEngine.m_pWeightesOfSubTrees;
    m_UsedNodes = rParJTreeInfEngine.m_UsedNodes;          
                          
    m_Roots = rParJTreeInfEngine.m_Roots;              
                          
    m_Routes = rParJTreeInfEngine.m_Routes;             
    m_IsPropagated = rParJTreeInfEngine.m_IsPropagated;       

    World_group = rParJTreeInfEngine.World_group;
    CollectEv_group = rParJTreeInfEngine.CollectEv_group;
    CollectEv_comm = rParJTreeInfEngine.CollectEv_comm;

#endif

}
// ----------------------------------------------------------------------------

void CParJtreeInfEngine::SetNbrsTypes()
{
    const int *nbr, *nbrs_end;
    intVector::const_iterator sourceIt, source_end;
    intVecVector::const_iterator layerIt = GetCollectSequence().begin(),
        collSeq_end = GetCollectSequence().end();

    const CGraph *pGraph = GetOriginalJTree()->GetGraph();
    const int numOfNds = GetOriginalJTree()->GetNumberOfNodes();

    m_NbrsTypes.resize(numOfNds);

    boolVector checkedNodes(numOfNds, false);
    int j;

    for (; layerIt != collSeq_end; ++layerIt)
    {
        for (sourceIt = layerIt->begin(), source_end = layerIt->end();
        sourceIt != source_end; ++sourceIt)
        {
            int numOfNbrs;
            const int *nbrs;
            const ENeighborType *nbrsTypes;

            pGraph->GetNeighbors(*sourceIt, &numOfNbrs, &nbrs, &nbrsTypes);

            m_NbrsTypes[*sourceIt].resize(numOfNbrs);
            if ((numOfNbrs == 1) && (*sourceIt) != GetJTreeRootNode()) //leaf
            {
                m_NbrsTypes[*sourceIt][0] = ntParent;
                checkedNodes[*sourceIt]=true;
            }
            else // not leaf
            {
                for (nbr = nbrs, nbrs_end = nbrs + numOfNbrs; nbr != nbrs_end;
                    ++nbr)
                {
                    if(checkedNodes[*nbr] == true) // children
                    {
                        for (j = 0; j < numOfNbrs; j++)
                            if (nbrs[j] == *nbr)
                            {
                                m_NbrsTypes[*sourceIt][j] = ntChild;
                            }
                    }
                    else // parent
                        for (j = 0; j < numOfNbrs; j++)
                            if (nbrs[j] == *nbr)
                            {
                                m_NbrsTypes[*sourceIt][j] = ntParent;
                            }
                }
                checkedNodes[*sourceIt] = true;
            }
        }
    }
}
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::InitQueueNodes()
{
    intVector::const_iterator sourceIt, source_end;
    intVecVector::const_iterator layerIt = GetCollectSequence().begin(),
        collSeq_end = GetCollectSequence().end();

    int i;
    const int numSelfNodes = m_NodesOfProcess.size();
    int CurNumSelfNodes = 0;

    for(; layerIt != collSeq_end && CurNumSelfNodes < numSelfNodes; ++layerIt)
    {
        for(sourceIt = layerIt->begin(), source_end = layerIt->end();
        sourceIt != source_end && CurNumSelfNodes < numSelfNodes; ++sourceIt)
        {
            for(i = 0; i < numSelfNodes; i++)
                if (*sourceIt == m_NodesOfProcess[i]) 
                {
                    m_QueueNodes.push(*sourceIt);
                    CurNumSelfNodes++;
                    break;
                }
        }
    }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

void CParJtreeInfEngine::InitNodeConditionsAndUpdateRatios()
{
    const int numOfNds = GetOriginalJTree()->GetNumberOfNodes();
    const CGraph* pGraph = GetOriginalJTree()->GetGraph();

    int numOfNbrs;
    const int *nbrs;
    const ENeighborType *nbrsTypes;
    int currNode;
    intVector::iterator ItNode, end_Node;

    m_NodeConditions.resize(numOfNds);
    m_UpdateRatios.resize(numOfNds);

    for(currNode = 0; currNode < numOfNds; currNode++)
    {
        pGraph->GetNeighbors(currNode, &numOfNbrs, &nbrs, &nbrsTypes);
        m_NodeConditions[currNode].resize(numOfNbrs + 1);
        m_UpdateRatios[currNode].resize(1);
        m_UpdateRatios[currNode][0] = NULL;

        for(ItNode = m_NodeConditions[currNode].begin(),
        end_Node = m_NodeConditions[currNode].end(); ItNode != end_Node;
        ++ItNode)
        {
            *ItNode=0; 
        }
        --ItNode;
        if(currNode != GetJTreeRootNode()) 
            *ItNode = numOfNbrs - 1;
        else
            *ItNode = numOfNbrs;
    }
}
// ----------------------------------------------------------------------------

void CParJtreeInfEngine::GetParents(int NumOfNode, intVector* Parents)
{
    const CGraph *pGraph = GetOriginalJTree()->GetGraph();
    int numOfNbrs;
    const int *nbrs;
    const ENeighborType *nbrsTypes;

    pGraph->GetNeighbors(NumOfNode, &numOfNbrs, &nbrs, &nbrsTypes);

    Parents->resize(1);
    (*Parents)[0] = -1;

    for (int i = 0; i < numOfNbrs; i++)
    {
        if (m_NbrsTypes[NumOfNode][i] == ntParent)
        {
            (*Parents)[0] = nbrs[i];
            break;
        }
    }
}
// ----------------------------------------------------------------------------
CParJtreeInfEngine* CParJtreeInfEngine::Copy(const CParJtreeInfEngine *pJTreeInfEng)
{
    /* bad-args check */
    PNL_CHECK_IS_NULL_POINTER(pJTreeInfEng);
    /* bad-args check end */
    
    CParJtreeInfEngine *pJTreeInfEngineCopy = new CParJtreeInfEngine(*pJTreeInfEng);
    
    PNL_CHECK_IF_MEMORY_ALLOCATED(pJTreeInfEngineCopy);
    
    return pJTreeInfEngineCopy;
}

// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParJtreeInfEngine::EnterEvidenceOMP(const CEvidence *pEvidence,
    int maximize, int sumOnMixtureNode)
{


    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    PNL_CHECK_RANGES( maximize, 0, 2 );

    if( pEvidence->GetModelDomain() != m_pGraphicalModel->GetModelDomain() )
    {
        PNL_THROW( CInvalidOperation, 
            "evidence and the Graphical Model must be on one Model Domain" );
    }
    // bad-args check end

    int i;

    if (GetModel()->GetModelType() == mtBNet)
    {

        ShrinkObservedOMP( pEvidence, maximize, sumOnMixtureNode );

        CJunctionTree *jtree = GetJTree();
        int nNodes = jtree->GetNumberOfNodes();
        EDistributionType isJtDiscr = GetJTreeType();


        
        if (isJtDiscr == dtTabular)
        {
            CollectEvidenceOMP();
        }
        else 
            if (isJtDiscr == dtGaussian)
            {
                CollectEvidenceOMP_gau();
            }
            else // dtScalar
                return;

            DistributeEvidenceOMP();
        #pragma omp parallel for schedule(dynamic) private(i)
        for (i = 0; i < nNodes; i++)
        {
            CDistribFun *df = jtree->GetNodePotential(i)->GetDistribFun();
            EDistributionType dt = df->GetDistributionType();
            if(dt == dtGaussian)
            {
                static_cast<CGaussianDistribFun*>(df)->UpdateCanonicalCoefficient();
            }
        }
        
        bool allDiscrObs = pEvidence->IsAllDiscreteNodesObs(GetModel());
        if (allDiscrObs)
        {
            for (i = 0; i < GetModel()->GetNumberOfNodes(); i++)
            {
                if (GetModel()->GetFactor(i)->GetDistributionType() == dtSoftMax)
                {
                    GetModel()->GetModelDomain()->ChangeNodeType(i, 0);
                }
            }
        }
    }
    else
    {
        PNL_THROW(CNotImplemented, "for BNets only");
    }

}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParJtreeInfEngine::ShrinkObservedOMP( const CEvidence *pEvidence, 
    int maximize, int sumOnMixtureNode,
    bool bRebuildJTree )
{
    // bad-args check
    PNL_CHECK_IS_NULL_POINTER(pEvidence);
    PNL_CHECK_RANGES( maximize, 0, 2 );
    // bad-args check end

    SetNormalizeCoefficient(-1);
    m_pEvidence = pEvidence;
    m_bMaximize = maximize;

    // deletes an old internal junction tree to start computations anew
    GetOriginalJTree()->ClearCharge();
    GetOriginalJTree()->InitChargeOMP( m_pGraphicalModel, pEvidence,
        sumOnMixtureNode );

    SetJTree(GetOriginalJTree());

    pConstValueVector allOffsets;

    m_pEvidence->GetObsNodesWithValues( &GetActuallyObservedNodes(),
        &allOffsets );

    m_lastOpDone = opsShrinkEv;
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParJtreeInfEngine::DistributeEvidenceOMP()
{
    intQueue source;
    omp_lock_t queue_lock;
    omp_lock_t count_lock;
    omp_lock_t find_zero_lock;
    
    const CGraph *pGraph = GetJTree()->GetGraph();
    const int numOfClqs = GetJTree()->GetNumberOfNodes();
    
    //int NumThreads = omp_get_num_procs();
    //omp_set_num_threads(NumThreads);
    int IsEndPar = 0;
    int Count = numOfClqs - 1;
    
    omp_init_lock(&find_zero_lock);
    omp_init_lock(&queue_lock);
    omp_init_lock(&count_lock);
    source.push(GetJTreeRootNode());
    
    InitNodeConditions();

#pragma omp parallel
{
    const int *nbr, *nbrs_end;
    //intVector Parents(0);
    pnlVector<int>* Parents = new pnlVector<int>;

    while (/*(!source.empty()) || */(IsEndPar == 0))
    {
        int sender;

        omp_set_lock(&queue_lock);
        if (source.empty())
        {
            omp_unset_lock(&queue_lock);
            continue;
        }
        sender = source.front();
        omp_unset_lock(&queue_lock);

        int numOfNbrs;
        const int *nbrs;
        const ENeighborType *nbrsTypes;

        pGraph->GetNeighbors(sender, &numOfNbrs, &nbrs, &nbrsTypes);
        GetParents(sender,Parents);

        for (nbr = nbrs, nbrs_end = nbrs + numOfNbrs; nbr != nbrs_end; ++nbr)
        {
            if (*nbr != (*Parents)[0])
            {
                int posNode = 0;
                omp_set_lock(&find_zero_lock);
                while ((posNode < numOfNbrs) && (nbrs[posNode] != *nbr))
                    posNode++;
                if (m_NodeConditions[sender][posNode] == 0)
                {
                    m_NodeConditions[sender][posNode] = 1;
                    --m_NodeConditions[sender][m_NodeConditions[sender].size() - 1];
                    omp_unset_lock(&find_zero_lock);
                }
                else
                {
                    omp_unset_lock(&find_zero_lock);
                    continue;
                }

                PropagateBetweenClqs(sender, *nbr, false);

                omp_set_lock(&count_lock);
                Count--;
                if (Count == 0)
                    IsEndPar = 1;
                omp_unset_lock(&count_lock);

                if (pGraph->GetNumberOfNeighbors(*nbr) > 1)
                {
                    omp_set_lock(&queue_lock);
                    source.push(*nbr);
                    omp_unset_lock(&queue_lock);
                }
            }
        } // for nbr
        
        if (numOfNbrs == 0)
        {   // it means that junction tree consist of one root node
            omp_set_lock(&count_lock);
            IsEndPar = 1;
            omp_unset_lock(&count_lock);            
        }

        if (m_NodeConditions[sender][m_NodeConditions[sender].size() - 1] == 0)
        {
            omp_set_lock(&queue_lock);
            if (!source.empty())
            {
                if (source.front() == sender)
                {
                    //omp_set_lock(&queue_lock);
                    source.pop();
                    //omp_unset_lock(&queue_lock);
                }
            }
            omp_unset_lock(&queue_lock);
        }
    } // while

    delete Parents;
} // end of parallel section

    omp_destroy_lock(&queue_lock);
    omp_destroy_lock(&count_lock);
    omp_destroy_lock(&find_zero_lock);

    m_lastOpDone = opsDistribute;
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParJtreeInfEngine::InitNodeConditions()
{
    const int numOfNds = GetOriginalJTree()->GetNumberOfNodes();
    const CGraph* pGraph = GetOriginalJTree()->GetGraph();

    int numOfNbrs;
    const int *nbrs;
    const ENeighborType *nbrsTypes;
    int currNode;
    intVector::iterator ItNode, end_Node;

    m_NodeConditions.resize(numOfNds);

    for (currNode = 0; currNode < numOfNds; currNode++)
    {
        pGraph->GetNeighbors(currNode, &numOfNbrs, &nbrs, &nbrsTypes);
        m_NodeConditions[currNode].resize(numOfNbrs + 1);
        
        for (ItNode = m_NodeConditions[currNode].begin(),
            end_Node = m_NodeConditions[currNode].end();
        ItNode != end_Node; ++ItNode)
        {
            *ItNode = 0; // there are no data now
        }
        --ItNode;
        if (currNode != GetJTreeRootNode())
            *ItNode = numOfNbrs - 1; // this is not a root so
                                     // number of children = numOfNbrs - 1
        else
            *ItNode = numOfNbrs; // this is not a root so
                                 // number of children = numOfNbrs
    }
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParJtreeInfEngine::CollectEvidenceOMP()
{
    // this is a phase of evidence propagation, when leaves send 
    // messages flow up to the root of the junction tree level by level

    // operation validity check
/*    if( m_lastOpDone != opsShrinkEv )
    {
        PNL_THROW( CInvalidOperation,
            " cannot perform collect, CParJtreeInfEngine is inconsistent " );
    }
*/    // operation validity check end

    SetNormalizeCoefficient(1.0f);

    const int *nbr, *nbrs_end;
    intVector::const_iterator sourceIt, source_end;
    intVecVector::const_iterator layerIt = GetCollectSequence().begin(),
        collSeq_end = GetCollectSequence().end();

    const CGraph *pGraph = GetJTree()->GetGraph();

    boolVector nodesSentMessages(GetJTree()->GetNumberOfNodes(), false);

    // at each step the propagation is from m_collectSequence i-th layer's
    // node to it's neighbors which has not sent the message yet
    for ( ; layerIt != collSeq_end; ++layerIt)
    {
        for (sourceIt = layerIt->begin(), source_end = layerIt->end();
        sourceIt != source_end; ++sourceIt)
        {
            int                 numOfNbrs;
            const int           *nbrs;
            const ENeighborType *nbrsTypes;

            pGraph->GetNeighbors(*sourceIt, &numOfNbrs, &nbrs, &nbrsTypes);

            for(nbr = nbrs, nbrs_end = nbrs + numOfNbrs; nbr != nbrs_end;
            ++nbr)
            {
                if(!nodesSentMessages[*nbr])
                {
                    PropagateBetweenClqsOMP(*sourceIt, *nbr, true);
                    break;
                }
            }

            nodesSentMessages[*sourceIt] = true;
        }
    }

    m_lastOpDone = opsCollect;
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParJtreeInfEngine::CollectEvidenceOMP_gau()
{
    int j;
    
    SetNormalizeCoefficient(1.0f);
    RebuildTreeFromRoot();

    const int NumVertex = GetOriginalJTree()->GetNumberOfNodes();
    omp_lock_t queue_lock, *parents_lock;
    parents_lock = new omp_lock_t[NumVertex];

    omp_init_lock(&queue_lock);
    for (j = 0; j < NumVertex; j++)
        omp_init_lock(&parents_lock[j]);
    
    InitQueueNodesOMP();
    InitNodeConditionsAndUpdateRatios();
    SetNbrsTypes();
    
    const CGraph *pGraph = GetOriginalJTree()->GetGraph();
#pragma omp parallel
    {
        int i;
        int currNode;
        int ValueOfSummator;
        intVector Parents(0);
        int                 numOfNbrs1;
        const int           *nbrs1;
        const ENeighborType *nbrsTypes1;
        int poschild;
        int                 numOfNbrs;
        const int           *nbrs;
        const ENeighborType *nbrsTypes;
        int       numNdsInSepDom;
        while (!m_QueueNodes.empty())
        {
            omp_set_lock(&queue_lock);
            if (m_QueueNodes.empty()) 
            { 
                omp_unset_lock(&queue_lock);
                continue; 
            }
            currNode = m_QueueNodes.front();
            m_QueueNodes.pop();
            omp_unset_lock(&queue_lock);
            
            ValueOfSummator=m_NodeConditions[currNode].begin()[m_NodeConditions[currNode].size()-1]; 
            if (ValueOfSummator==0)
            {
                if (currNode != GetJTreeRootNode())
                {
                    GetParents(currNode,&Parents);
                    
                    omp_set_lock(&parents_lock[Parents.begin()[0]]);
                    
                    PropagateBetweenClqs(currNode, Parents.begin()[0], 1);
                    
                    omp_unset_lock(&parents_lock[Parents.begin()[0]]);
                    
                    pGraph->GetNeighbors( Parents.begin()[0], &numOfNbrs1, &nbrs1, &nbrsTypes1 );
                    poschild=0;
                    while (poschild < numOfNbrs1 && nbrs1[poschild] != currNode)
                        poschild++;//?????? ????? currNode
                    m_NodeConditions[Parents.begin()[0]].begin()[poschild] = 1;
                }
            }
            else
            {
                
                for (i = 0; i < m_NodeConditions[currNode].size() - 1; i++)
                {
                    if (m_NodeConditions[currNode].begin()[i] == 1)
                    {
                        m_NodeConditions[currNode].begin()[i] = 2;
                        --m_NodeConditions[currNode].begin()[m_NodeConditions[currNode].size() - 1];
                    }
                }// ??? 1 ??????????
                
                omp_set_lock(&queue_lock);
                m_QueueNodes.push(currNode);
                omp_unset_lock(&queue_lock);
                
            }
        } //while
    } //parallel section

    omp_destroy_lock(&queue_lock);
    for (j = 0; j < NumVertex; j++)
    {
        omp_destroy_lock(&parents_lock[j]);
    }
    delete [] parents_lock;

    m_lastOpDone = opsCollect;
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParJtreeInfEngine::PropagateBetweenClqsOMP(int source, int sink,
    bool isCollect)
{
    CJunctionTree* pJTree = GetJTree();

    // bad-args check
    PNL_CHECK_RANGES( source, 0, pJTree->GetNumberOfNodes() - 1 );
    PNL_CHECK_RANGES( sink,   0, pJTree->GetNumberOfNodes() - 1 );
    // bad-args check end

    // operation validity check
    if( source == sink )
    {
        PNL_THROW( CInvalidOperation, " source and sink should differ " );
    }

    if( !pJTree->GetGraph()->IsExistingEdge( source, sink ) )
    {
        PNL_THROW( CInvalidOperation,
            " there is no edge between source and sink " );
    }
    /* operation validity check end */

    bool isDense = true;
    if( !pJTree->GetNodeType(source)->IsDiscrete() ||
        !pJTree->GetNodeType(sink)->IsDiscrete())
    {
        isDense = false;
    }
    CPotential *potSource = pJTree->GetNodePotential(source),
        *potSink   = pJTree->GetNodePotential(sink);

    if(potSource->IsSparse() || potSink->IsSparse())
    {
        isDense = false;
    }
    // check that nodes source and sink are discrete and their matrices
    // are dense

    if(isDense)
    {
        CNumericDenseMatrix< float > *sorceMatrix, *sepMatrix, *sinkMatrix;
        int *dims_to_keep, *dims_to_mul;
        int num_dims_to_keep, num_dims_to_mul;

        if (GetDataForMargAndMultOMP(source, sink, &sorceMatrix,
            &dims_to_keep, num_dims_to_keep, &sepMatrix, &sinkMatrix,
            &dims_to_mul, num_dims_to_mul))
        {
            DoPropagateOMP(sorceMatrix, dims_to_keep, num_dims_to_keep,
                sepMatrix, sinkMatrix, dims_to_mul, num_dims_to_mul,
                isCollect);
            delete [] dims_to_keep;
            delete [] dims_to_mul;
        }
        else
        {
            CPotential *potSink = pJTree->GetNodePotential(sink);
            potSink->Normalize();
        }
    }
    else
    {
        PNL_THROW(CNotImplemented, "only for numeric dense chance now ");
    }
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
int CParJtreeInfEngine::GetDataForMargAndMultOMP(const int source,
    const int sink, CNumericDenseMatrix< float > **sorceMatrix,
    int **dims_to_keep, int &num_dims_to_keep,
    pnl::CNumericDenseMatrix< float > **sepMatrix, 
    CNumericDenseMatrix< float > **sinkMatrix, int **dims_to_mul,
    int &num_dims_to_mul)
{
    CJunctionTree* pJTree = GetJTree();

    // bad-args check
    PNL_CHECK_RANGES(source, 0, pJTree->GetNumberOfNodes() - 1);
    PNL_CHECK_RANGES(sink, 0, pJTree->GetNumberOfNodes() - 1);
    // bad-args check end

    if (source == sink)
    {
        PNL_THROW(CInvalidOperation, " source and sink should differ ");
    }
    if (!pJTree->GetGraph()->IsExistingEdge(source, sink))
    {
        PNL_THROW(CInvalidOperation,
            " there is no edge between source and sink");
    }

    CPotential *potSource = pJTree->GetNodePotential(source),
        *potSink   = pJTree->GetNodePotential(sink);

    int numNdsInSourceDom, numNdsInSinkDom;
    const int *sourceDom, *sinkDom;
    potSource->GetDomain(&numNdsInSourceDom, &sourceDom);
    potSink->GetDomain(&numNdsInSinkDom, &sinkDom);

    CPotential *potSep = pJTree->GetSeparatorPotential(source, sink);
    int numNdsInSepDom;
    const int *sepDom;
    potSep->GetDomain(&numNdsInSepDom, &sepDom);

    EDistributionType sepDistType = potSep->GetDistributionType();

    num_dims_to_keep = numNdsInSepDom;
    *dims_to_keep = new int [num_dims_to_keep];

    int i;
    int* pEquivPos;
    for (i = 0; i < numNdsInSepDom; i++)
    {
        pEquivPos = (int*)std::find(sourceDom, sourceDom +
            numNdsInSourceDom, sepDom[i]);
        if (pEquivPos != sourceDom + numNdsInSourceDom)
        {
            (*dims_to_keep)[i] = (pEquivPos - sourceDom);
        }
        else 
        {
            PNL_THROW( CInconsistentSize,
                "small domain isn't subset of domain")
            return NULL;
        }
        //check that pSmallDom is m_Domain's subset
    }
    switch (sepDistType)
    {
    case dtTabular:
        {
            CDistribFun *sepDistrFun = potSep -> GetDistribFun();
            CDistribFun *sourceDistrFun = potSource -> GetDistribFun();
            CDistribFun *sinkDistrFun = potSink -> GetDistribFun();
            if (!sourceDistrFun->IsValid())
            {
                PNL_THROW( CInconsistentType,
                    "MarginalizeData is invalid" )
            }

            // check if distribution of potSource is
            // Unit Function - do nothing
            if(sourceDistrFun->IsDistributionSpecific())
            {
                return 0;
            }

            if ( sepDistrFun->IsDistributionSpecific() )
            {
                sepDistrFun->SetUnitValue(0);
            }

            *sorceMatrix =
                static_cast<CNumericDenseMatrix<float> *>(sourceDistrFun->
                GetMatrix(matTable));
            *sepMatrix = 
                static_cast<CNumericDenseMatrix<float> *>(sepDistrFun->
                GetMatrix(matTable));

            EDistributionType dtsink = sinkDistrFun->GetDistributionType();
            if ((dtsink != dtTabular) && (dtsink != dtScalar))
            {
                PNL_THROW(CInvalidOperation,
                    "we can multiply only tabulars")
            }

            int location;
            num_dims_to_mul = numNdsInSepDom;
            *dims_to_mul = new int [num_dims_to_mul];

            for (i = 0; i < numNdsInSepDom; i++)
            {
                location = 
                    std::find(sinkDom, sinkDom + numNdsInSinkDom,
                    sepDom[i]) - sinkDom;
                if (location < numNdsInSinkDom)
                {
                    (*dims_to_mul)[i] = location;
                }
            }

            if(sinkDistrFun->IsDistributionSpecific())
            {
                sinkDistrFun->SetUnitValue(0);
                floatVector *Vector = (floatVector *)((CDenseMatrix<float>*)sinkDistrFun->
                    GetMatrix(matTable))->GetVector();
                float *V = &Vector->front();
                int vectorSize = Vector->size();

#pragma omp parallel for private(i)
                for (i = 0; i < vectorSize; i++)
                    *(V + i) = 1;
            }

            *sinkMatrix = static_cast<CNumericDenseMatrix<float>*>(
                sinkDistrFun->GetMatrix(matTable));

            break;
        }
    case dtScalar:
        {
            // propagate isn't need
            return 0;
        }
    default:
        {
            PNL_THROW(CNotImplemented, "we have only Tabular now");
            return 0;
        }
    }

    if (numNdsInSepDom == 0)
    {
        PNL_THROW(COutOfRange, "domain size should be positive");
    }
    return 1;
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParJtreeInfEngine::DoPropagateOMP(
    CNumericDenseMatrix<float> *sourceMatrix, int *dims_to_keep, 
    int num_dims_to_keep, CNumericDenseMatrix<float> *sepMatrix,
    CNumericDenseMatrix<float> *sinkMatrix, int *dims_to_mul, 
    int num_dims_to_mul, bool isCollect)
{
    const int MAX_SIZE = 100;
    int i, j;
    int src_num_dims;
    int const *src_ranges;
    float const *src_bulk;
    int src_bulk_size, safe_bulk_size;
    float *dst_bulk;
    int dst_bulk_size, safe_dst_bulk_size;
    int safe_dst_num_dims;
    int const *safe_dst_ranges;

    int big_num_dims;
    int const *big_ranges;
    float *big_bulk;
    int big_bulk_size;
    sinkMatrix->GetRanges(&big_num_dims, &big_ranges);
    sinkMatrix->GetRawData(&big_bulk_size, (float const **)&big_bulk);

    sourceMatrix->GetRanges(&src_num_dims, &src_ranges);
    sourceMatrix->GetRawData(&safe_bulk_size, &src_bulk);

    int dst_ranges[MAX_SIZE];

    for (i = num_dims_to_keep, dst_bulk_size = 1; i--;)
    {
        dst_bulk_size *= dst_ranges[i] = src_ranges[dims_to_keep[i]];
    }

    if (sepMatrix)
    {
        sepMatrix->GetRanges(&safe_dst_num_dims, &safe_dst_ranges);
        sepMatrix->GetRawData(&safe_dst_bulk_size, (float const **)&dst_bulk);
        if (safe_dst_bulk_size < dst_bulk_size)
        {
            PNL_THROW(CInconsistentSize, "Marg: output matrix does not fit");
        }
    }
    else
    {
        PNL_THROW(CInconsistentSize, "Marg: output matrix does not fit");
        // matrix must exist
    }

// --- marg section -----------------------------------------------------------
    int dst_sum_count = safe_bulk_size / dst_bulk_size;
    int num_dims_unkeep = src_num_dims - num_dims_to_keep;

    int dims_unkeep[MAX_SIZE];
    int loc, pos = 0;
    for (i = 0; i < src_num_dims; i++)
    {
        loc = 0;
        for (j = 0; j < num_dims_to_keep; j++)
            if (i == dims_to_keep[j])
                loc = 1;
            if (!loc)
                dims_unkeep[pos++] = i;
    }

    int steps1[MAX_SIZE];
    int step = 1;
    for (i = src_num_dims - 1; i >=0; i--)
    {
        steps1[i] = step;
        step *= src_ranges[i];
    }

    int steps[MAX_SIZE];
    int srcranges[MAX_SIZE];

    for (i = 0; i < num_dims_to_keep; i++)
    {
        srcranges[i] = src_ranges[dims_to_keep[num_dims_to_keep - i - 1]];
        steps[i] = steps1[dims_to_keep[num_dims_to_keep - i - 1]];
    }

    for (i = num_dims_to_keep; i < src_num_dims; i++)
    {
        srcranges[i] = src_ranges[dims_unkeep[src_num_dims - i - 1]];
        steps[i] = steps1[dims_unkeep[src_num_dims - i - 1]];
    }

    int bsteps[MAX_SIZE];
    for (i = src_num_dims - 1; i >=0; i--)
    {
        bsteps[i] = steps[i] * srcranges[i];
    }

    int *offsets = new int[dst_sum_count];

    int counts[MAX_SIZE];
    memset(counts, 0, src_num_dims * sizeof(int));

    pos = 0;
    int k = num_dims_to_keep;
    for (j = 0; j < dst_sum_count; j++)
    {
        while (counts[k] == srcranges[k])
        {
            counts[k] = 0;
            pos -= bsteps[k];
            k++;
            counts[k]++;
            pos += steps[k];
        }
        k = num_dims_to_keep;
        offsets[j] = pos;
        counts[k]++;
        pos += steps[k];
    }

    for (i = 0; i < src_num_dims - 1; i++)
    {
        bsteps[i] = steps[i + 1] - bsteps[i];
    }

    int fsteps[MAX_SIZE];
    fsteps[0] = steps[0];
    for (i = 1; i < num_dims_to_keep; i++)
    {
        fsteps[i] = fsteps[i - 1] + bsteps[i - 1];
    }
    fsteps[num_dims_to_keep] = steps[num_dims_to_keep];
    for (i = num_dims_to_keep + 1; i < src_num_dims; i++)
    {
        fsteps[i] = fsteps[i - 1] + bsteps[i - 1];
    }

    int keep_steps[MAX_SIZE];
    step = 1;
    for (i = num_dims_to_keep - 1; i >=0; i--)
    {
        keep_steps[i] = step;
        step *= dst_ranges[i];
    }

    int dsteps1[MAX_SIZE];
    memset(dsteps1, 0, src_num_dims * sizeof(int));
    for (i = num_dims_to_keep - 1; i >=0; i--)
    {
        dsteps1[dims_to_keep[i]] = keep_steps[i];
    }
    int dsteps[MAX_SIZE];
    pos = num_dims_to_keep - 1;
    for (i = 0; i < src_num_dims; i++)
    {
        if (dsteps1[i] != 0)
            dsteps[pos--] = dsteps1[i];
    }

    int bdsteps[MAX_SIZE];
    for (i = 0; i < num_dims_to_keep; i++)
    {
        bdsteps[i] = dsteps[i] * srcranges[i];
    }
    for (i = 0; i < num_dims_to_keep - 1; i++)
    {
        bdsteps[i] = dsteps[i + 1] - bdsteps[i];
    }

    int fdsteps[MAX_SIZE];
    fdsteps[0] = dsteps[0];
    for (i = 1; i < num_dims_to_keep; i++)
    {
        fdsteps[i] = fdsteps[i - 1] + bdsteps[i - 1];
    }

// --- mult section -----------------------------------------------------------
    int dst_mult_count = big_bulk_size / dst_bulk_size;
    int num_dims_unkeep_mult = big_num_dims - safe_dst_num_dims;

    int keep_steps_mult[MAX_SIZE];
    step = 1;
    for (i = safe_dst_num_dims - 1; i >=0; i--)
    {
        keep_steps_mult[i] = step;
        step *= big_ranges[dims_to_mul[i]];
    }

    int dims_unkeep_mult[MAX_SIZE];
    pos = 0;
    for (i = 0; i < big_num_dims; i++)
    {
        loc = 0;
        for (j = 0; j < safe_dst_num_dims; j++)
            if (i == dims_to_mul[j])
                loc = 1;
            if (!loc)
                dims_unkeep_mult[pos++] = i;
    }

    int smal_steps_mult[MAX_SIZE];
    for (i = safe_dst_num_dims - 1; i >=0; i--)
    {
        smal_steps_mult[dims_to_mul[i]] = keep_steps_mult[i];
    }
    for (i = num_dims_unkeep_mult - 1; i >=0; i--)
    {
        smal_steps_mult[dims_unkeep_mult[i]] = 0;
    }
    int dbsteps_mult[MAX_SIZE];
    for (i = big_num_dims - 1; i >=0; i--)
    {
        dbsteps_mult[i] = smal_steps_mult[i] * big_ranges[i];
    }
    for (i = 0; i < big_num_dims - 1; i++)
    {
        dbsteps_mult[i] = smal_steps_mult[i] - dbsteps_mult[i + 1];
    }

    int big_steps[MAX_SIZE];
    step = 1;
    for (i = big_num_dims - 1; i >= 0; i--)
    {
        big_steps[i] = step;
        step *= big_ranges[i];
    }

// --- Main Loop (marg) -------------------------------------------------------
#ifdef _CLUSTER_OPENMP
    int NumOfProcs = omp_get_max_threads();
    float *sum_par = new float [NumOfProcs];
#else
    float *sum_par = new float [omp_get_max_threads()];
#endif
    int src_steps[MAX_SIZE];
    src_steps[0] = 1;
    for (j = 1; j < num_dims_to_keep; j++)
    {
        *(src_steps + j) = *(src_steps + j - 1) * (*(srcranges + j - 1));
    }
#pragma omp parallel
    {
#ifndef _CLUSTER_OPENMP
	int NumOfProcs = omp_get_num_threads();
#endif

        int threadNum = omp_get_thread_num();
        
        int numDstElemOfProc = dst_bulk_size / NumOfProcs;
        int leftDstBound = threadNum * numDstElemOfProc;
        int rightDstBound = (threadNum+1) * numDstElemOfProc;
        if (threadNum == NumOfProcs - 1)
        {
            rightDstBound = dst_bulk_size;
        }
        int counts_par[MAX_SIZE];

        int i_ind, j_ind;
        memset(counts_par, 0, src_num_dims * sizeof(int));
        int kc = leftDstBound;
        for (j_ind = num_dims_to_keep - 1; j_ind >= 0; j_ind--)
        {
            while (kc >= src_steps[j_ind])
            {
                counts_par[j_ind]++;
                kc -= src_steps[j_ind];
            }
        }

        int src_start = 0 - steps[0];
        for (j_ind = num_dims_to_keep - 1; j_ind >= 0; j_ind--)
        {
            src_start += counts_par[j_ind] * steps[j_ind];
        }
        int p = 0;

        float dst_el_old;
        if (!isCollect)
        {
            for (i_ind = leftDstBound; i_ind < rightDstBound; i_ind++)
            {
                while (counts_par[p] == srcranges[p])
                {
                    counts_par[p] = 0;
                    p++;
                    counts_par[p]++;
                }
                src_start += fsteps[p];
                p = 0;
                counts_par[p]++;

                dst_el_old = dst_bulk[i_ind];
                dst_bulk[i_ind] = 0;
                for (j_ind = 0; j_ind < dst_sum_count; j_ind++)
                {
                    dst_bulk[i_ind] += src_bulk[src_start + offsets[j_ind]];
                }
                if (dst_el_old)
                {
                    dst_bulk[i_ind] /= dst_el_old;
                }
            }
        }
        else
        {
            for (i_ind = leftDstBound; i_ind < rightDstBound; i_ind++)
            {
                while (counts_par[p] == srcranges[p])
                {
                    counts_par[p] = 0;
                    p++;
                    counts_par[p]++;
                }
                src_start += fsteps[p];
                p = 0;
                counts_par[p]++;

                dst_bulk[i_ind] = 0;
                for (j_ind = 0; j_ind < dst_sum_count; j_ind++)
                {
                    dst_bulk[i_ind] += src_bulk[src_start + offsets[j_ind]];
                }
            }
        }

        {
#pragma omp barrier
        }

        float sumOfProc = 0;
// --- Main Loop (mult) -------------------------------------------------------
        int numBigElemOfProc = big_bulk_size / NumOfProcs;
        int leftBigBound = threadNum * numBigElemOfProc;
        int rightBigBound = leftBigBound + numBigElemOfProc;
        if (threadNum == NumOfProcs - 1)
        {
            rightBigBound = big_bulk_size;
        }
        int counts_mult[MAX_SIZE];
        memset(counts_mult, 0, big_num_dims * sizeof(int));

        kc = leftBigBound;
        for (j_ind = 0; j_ind < big_num_dims; j_ind++)
        {
            while (kc >= big_steps[j_ind])
            {
                counts_mult[j_ind]++;
                kc -= big_steps[j_ind];
            }
        }
        int dst_pos = 0;
        for (j_ind = big_num_dims - 1; j_ind >= 0; j_ind--)
        {
            dst_pos += counts_mult[j_ind] * smal_steps_mult[j_ind];
        }

        p = big_num_dims - 1;
        for (i_ind = leftBigBound; i_ind < rightBigBound; i_ind++)
        {
            while (counts_mult[p] == big_ranges[p])
            {
                counts_mult[p] = 0;
                p--;
                counts_mult[p]++;
                dst_pos += dbsteps_mult[p];
            }

            big_bulk[i_ind] *= dst_bulk[dst_pos];
            sumOfProc += big_bulk[i_ind];

            p = big_num_dims - 1;
            counts_mult[p]++;
            dst_pos += smal_steps_mult[p];
        }
        sum_par[threadNum] = sumOfProc;

        {
#pragma omp barrier
        }

        float sum = 0;
        for (i_ind=0; i_ind < NumOfProcs; i_ind++)
        {
            sum += sum_par[i_ind];
        }

        float reciprocalSum = 1 / sum;

        for (i_ind = leftBigBound; i_ind < rightBigBound; i_ind++)
        {
            *(big_bulk + i_ind) *= reciprocalSum;
        }

        if(!threadNum)
        {   
            if(isCollect)
            {
                SetNormalizeCoefficient(GetNormalizeCoefficient() * sum);
            }
        }
    }// end of parallel section

    delete [] offsets;
    delete [] sum_par;
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_OMP
void CParJtreeInfEngine::InitQueueNodesOMP()
{
    intVector::const_iterator sourceIt, source_end;
    intVecVector::const_iterator layerIt = GetCollectSequence().begin(),
        collSeq_end = GetCollectSequence().end();

    for(; layerIt != collSeq_end; ++layerIt)
    {
        for(sourceIt = layerIt->begin(), source_end = layerIt->end();
        sourceIt != source_end; ++sourceIt)
        {
            m_QueueNodes.push(*sourceIt);
        }
    }
}
#endif // PAR_OMP
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::EnterEvidence(const CEvidence *pEvidence,
    int maximize, int sumOnMixtureNode)
{
    int i, j;
  
    ShrinkObserved(pEvidence, maximize, sumOnMixtureNode);
    
    if (GetJTreeType() == dtScalar) // all nodes are observed
        return;

    DivideNodes();
 
    if (m_NumberOfUsedProcesses == 2)
        BalanceTree();
    
    CollectCommCreate();

    CollectEvidence();
    DistributeEvidence();
    
    for(i = 0; i < m_CollectRanks.size(); i++)
        CollectFactorsOnProcess(m_CollectRanks[i]);

    for( i = 0; i < m_CollectRanks.size(); i++)
    {
	if (m_MyRank == m_CollectRanks[i])
	{
    	    for (j = 0; j < m_NodesOfProcess.size(); j++)
	    {
	        CPotential *Pot = GetJTree()->GetNodePotential(m_NodesOfProcess[j]);
	        EDistributionType dt = Pot->GetDistribFun()->GetDistributionType();
	        if(dt == dtGaussian)
    		{
		    static_cast<CGaussianDistribFun*>(Pot->GetDistribFun())->
		        UpdateCanonicalCoefficient();
		}
            }
        }
    }
	
    for( i = 0; i < m_CollectRanks.size(); i++)
    {
        if (m_MyRank == m_CollectRanks[i])
        {
            if (GetModel()->GetModelType() == mtBNet)
            {
                for (j = 0; j < GetModel()->GetNumberOfNodes(); j++)
                {
                    if (GetModel()->GetFactor(j)->GetDistributionType() == dtSoftMax)
                    {
                        GetModel()->GetModelDomain()->ChangeNodeType(j, 0);
                    }
                }
            }
        }
    }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::CollectEvidence()
{
    int i;
    int currNode;
    int ValueOfSummator;
    CPotential *potClq, *potSep;
    CPotential *newPotSep, *updateRatio;
    int numNdsInSepDom;
    const int *sepDom;
    intVector Parents(0);
    int numOfNbrs;
    const int *nbrs;
    const ENeighborType *nbrsTypes;
    int RankProcWithParentNode;
    int Tag, MaxTag;
    int flagRecvedMsg;
    MPI_Status status;
    int CurSendedNode;
    int CurRecvingNode;
    int myrank = m_MyRank;
    
    SetNormalizeCoefficient(1.0f);

    MaxTag = GetMaxTagForCollectEvidence();
    
    InitQueueNodes();
    InitNodeConditionsAndUpdateRatios();
    SetNbrsTypes();
    
    const CGraph *pGraph = GetJTree()->GetGraph();
    
    while (!m_QueueNodes.empty())
    {
        // Receive all data that are arrived on the process
        do
        {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, CollectEv_comm, 
                &flagRecvedMsg, &status);
            Tag = status.MPI_TAG;
            
            if ((flagRecvedMsg) && (Tag < MaxTag))
            {
                int dataLength;
                const float *pDataForRecving;

                const float *dataH, *dataK;
                int sizeH, sizeK;
                int flag;
                const float *dataMean, *dataCov;
                int sizeMean, sizeCov;

                CurSendedNode = NumOfSendedNode(Tag);
                CurRecvingNode = NumOfRecvedNode(Tag);
                pGraph->GetNeighbors(CurRecvingNode, &numOfNbrs, &nbrs,
                    &nbrsTypes);

                CPotential *Pot = GetJTree()->GetNodePotential(CurSendedNode);
               
                if (Pot->GetDistributionType() == dtTabular)
                {
                    MPI_Recv(&dataLength, 1, MPI_INT,
                        m_NodeProcesses[CurSendedNode], Tag, CollectEv_comm, &status);
                    pDataForRecving = new float [dataLength];
                    MPI_Recv((float*)pDataForRecving, dataLength, MPI_FLOAT,
                        m_NodeProcesses[CurSendedNode], Tag, CollectEv_comm, &status);
                } // if dtTabular

                if (Pot->GetDistributionType() == dtGaussian)
                {
                    int *Sizes = new int[3];
                    float *Data;
                    // gaussian distribution in canonical form
                    if (static_cast<CGaussianDistribFun *>(Pot->GetDistribFun())->
                        GetCanonicalFormFlag())
                    {
                        MPI_Recv((int*)Sizes, 3, MPI_INT,
                            m_NodeProcesses[CurSendedNode], Tag, CollectEv_comm, &status);

                        sizeH = Sizes[0];
                        sizeK = Sizes[1];
                        flag = Sizes[2];

                        if (flag == 1) // delta-function
                        {
                            float *dataMean = new float[sizeH];
                           
                            MPI_Recv((float*)dataMean, sizeH, MPI_FLOAT,
                                m_NodeProcesses[CurSendedNode], Tag, CollectEv_comm, &status);
                            
                        }
                        else
                        {
                            dataH = new float[sizeH];
                            dataK = new float[sizeK];
                            
                            Data = new float[sizeH + sizeK];
                            
                            MPI_Recv((float*)Data, sizeH + sizeK, MPI_FLOAT,
                                m_NodeProcesses[CurSendedNode], Tag, CollectEv_comm, &status);
                            
                            memcpy((void*)dataH, Data, sizeH*sizeof(float));
                            memcpy((void*)dataK, Data + sizeH, sizeK*sizeof(float));
                            
                            delete [] Data;
                        }
                        
                    }// if canonical form
                    else
                        // gaussian distribution in moment form
                        // branch is not tested
                    if (static_cast<CGaussianDistribFun *>(Pot->GetDistribFun())
                        ->GetMomentFormFlag())
                    {
                        MPI_Recv(&sizeMean, 1, MPI_INT, m_NodeProcesses[CurSendedNode],
                            Tag, CollectEv_comm, &status);
                        dataMean = new float[sizeMean];
                        MPI_Recv((float*)dataMean, sizeMean, MPI_FLOAT,
                            m_NodeProcesses[CurSendedNode], Tag, CollectEv_comm, &status);
                        
                        MPI_Recv(&sizeCov, 1, MPI_INT,
                            m_NodeProcesses[CurSendedNode], Tag, CollectEv_comm, &status);
                        dataCov = new float[sizeCov];
                        MPI_Recv((float*)dataCov, sizeCov, MPI_FLOAT,
                            m_NodeProcesses[CurSendedNode], Tag, CollectEv_comm, &status);

                    } // if moment form
                    delete [] Sizes;
                } // if dtGaussian

                int posNodeForCurr = 0;
                while ((posNodeForCurr < numOfNbrs) &&
                    (nbrs[posNodeForCurr] != CurSendedNode))
                    posNodeForCurr++;
                
                m_NodeConditions[CurRecvingNode][posNodeForCurr] = 1;
                
                CPotential *potSep = 
                    GetJTree()->GetSeparatorPotential(CurSendedNode, CurRecvingNode);
                CModelDomain* pMD=potSep->GetModelDomain();
                potSep->GetDomain(&numNdsInSepDom, &sepDom);

                intVector sepDomain(numNdsInSepDom);
                sepDomain.assign(sepDom, sepDom + numNdsInSepDom);
                
                intVector obsIndicesIn;
                int count = sepDomain.size();
                
                obsIndicesIn.clear();
                for (i = 0; i < count; i++)
                    if (m_pEvidence->IsNodeObserved(sepDomain[i]))
                        obsIndicesIn.push_back(i);

                EDistributionType  dt = potSep->GetDistributionType();

               if (potSep->GetDistributionType() == dtScalar)
               {
                   updateRatio = CScalarPotential::Create(sepDomain, pMD, obsIndicesIn);
               }
               if (potSep->GetDistributionType() == dtTabular)
               {
                   updateRatio = CTabularPotential::Create(sepDomain, pMD, 
                       pDataForRecving, obsIndicesIn);
                   delete [] (float*) pDataForRecving;
               }
               if (potSep->GetDistributionType() == dtGaussian)
               {
                   if (static_cast<CGaussianDistribFun *>(potSep->GetDistribFun())
                       ->GetCanonicalFormFlag())
                   {
                       if (flag == 1) // delta-function
                       {
                           updateRatio = CGaussianPotential::CreateDeltaFunction(&sepDomain.front(),
                               sepDomain.size(), pMD, dataMean, 1, obsIndicesIn);
                           static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())->
                               UpdateCanonicalForm();
                           delete [] (float*)dataMean;
                       }
                       else
                       {
                           updateRatio = CGaussianPotential::Create(&sepDomain.front(), 
                               sepDomain.size(), pMD, 0, dataH, dataK, 0.0f, obsIndicesIn);
                           
                           if (dataH != NULL)
                           {
                               delete [] (float*)dataH;
                           }
                           if (dataK != NULL)
                           {
                               delete [] (float*)dataK;
                           }
                       }
                   }
                   else
                       // branch is not tested
                       if (static_cast<CGaussianDistribFun *>(potSep->GetDistribFun())
                           ->GetMomentFormFlag())
                       {
                           updateRatio = CGaussianPotential::Create(&sepDomain.front(), 
                               sepDomain.size(), pMD, 1, dataMean, dataCov, 0.0f, obsIndicesIn);
                           if (dataMean != NULL)
                               delete [] (float*)dataMean;
                           if (dataCov != NULL)
                               delete [] (float*)dataCov;
                           
                       }
               } // if dtGaussian
               
                potSep->SetDistribFun(updateRatio->GetDistribFun());
                
                m_UpdateRatios[CurSendedNode][0] = updateRatio;

            } // if ((flagRecvedMsg) && (Tag < MaxTag))
        } // do
        while (flagRecvedMsg); // end of receiving
        
        currNode = m_QueueNodes.front();
        pGraph->GetNeighbors( currNode, &numOfNbrs, &nbrs, &nbrsTypes );
        
        ValueOfSummator = 
            m_NodeConditions[currNode][m_NodeConditions[currNode].size()-1];
        if ((ValueOfSummator == 0) && (currNode != GetJTreeRootNode()))
        {
            // leafe or node which collect data from all children
            GetParents(currNode,&Parents);
            potClq = GetJTree()->GetNodePotential(currNode);

            int sizeK;
            const float *dataK;
            int sizeH;
            const float *dataH;
            int flag;

            potSep = GetJTree()->GetSeparatorPotential(currNode, Parents[0]);

            potSep->GetDomain(&numNdsInSepDom, &sepDom);

            newPotSep = potClq->Marginalize(sepDom, numNdsInSepDom, m_bMaximize);
    
            updateRatio = newPotSep->Divide(potSep);

            potSep->SetDistribFun(newPotSep->GetDistribFun());
            
            delete newPotSep;
            
            RankProcWithParentNode = m_NodeProcesses[Parents[0]];
            if (RankProcWithParentNode == m_MyRank)
            {
                m_UpdateRatios[currNode][0] = updateRatio;
                
                int numOfNbrs1;
                const int *nbrs1;
                const ENeighborType *nbrsTypes1;
                int poschild = 0;
                
                pGraph->GetNeighbors( Parents[0], &numOfNbrs1, &nbrs1,
                    &nbrsTypes1 );
                while ((poschild < numOfNbrs1) && (nbrs1[poschild] != currNode))
                    poschild++;
                m_NodeConditions[Parents[0]][poschild] = 1;

            }
            else
            {
                // send an UpdateRatio to another process
                
                Tag = GetTagForSending(currNode, Parents[0]);
                
                EDistributionType dt = potClq->GetDistributionType();

                if (dt == dtTabular)
                {
                    if (updateRatio->GetDistributionType() == dtScalar)
                    {
                        MPI_Request *requests = new MPI_Request[3];

                        int dataLength = 0;
                        const float *pDataForSending = NULL;

                        MPI_Isend(&dataLength, 1, MPI_INT, RankProcWithParentNode, Tag,
                            CollectEv_comm, &(requests[1]));
                        MPI_Isend((float*)pDataForSending, dataLength, MPI_FLOAT,
                            RankProcWithParentNode, Tag, CollectEv_comm, &(requests[2]));
                        delete [] requests;

                    }
                    
                    if (updateRatio->GetDistributionType() == dtTabular)
                    {
                        MPI_Request *requests = new MPI_Request[3];

                        CNumericDenseMatrix<float> *matForSending;
                        int dataLength;
                        const float *pDataForSending;
                        
                        matForSending = static_cast<CNumericDenseMatrix<float>*>
                            ((updateRatio->GetDistribFun())->GetMatrix(matTable));
                        matForSending->GetRawData(&dataLength, &pDataForSending);
                        
                        MPI_Isend(&dataLength, 1, MPI_INT, RankProcWithParentNode, Tag,
                            CollectEv_comm, &(requests[1]));
                        MPI_Isend((float*)pDataForSending, dataLength, MPI_FLOAT,
                            RankProcWithParentNode, Tag, CollectEv_comm, &(requests[2]));
                        delete [] requests;
                    }
                } //if dtTabular
                if (dt == dtGaussian)
                {
                    const float *dataH, *dataK;
                    int sizeH, sizeK;
                    
                    const float *dataMean, *dataCov;
                    int sizeMean, sizeCov;
                    int *Sizes = new int[3];
                    float *Data;

                    if (updateRatio->GetDistributionType() == dtScalar)
                    {
                        dataH = NULL;
                        dataK = NULL;
                        
                        Sizes[0] = Sizes[1] = Sizes[2] = 0;

                        MPI_Request *requests = new MPI_Request[3];

                        MPI_Isend((int*)Sizes, 3, MPI_INT, RankProcWithParentNode,
                            Tag, CollectEv_comm, &(requests[1]));
                        MPI_Isend((float*)dataH, sizeH, MPI_FLOAT, RankProcWithParentNode,
                            Tag, CollectEv_comm, &(requests[2]));
                        delete [] requests;
                    } // if dtScalar
                    else
                    if (updateRatio->GetDistribFun()->IsDistributionSpecific() == 1)
                    {
                        dataH = NULL;
                        dataK = NULL;

                        Sizes[0] = Sizes[1] = Sizes[2] = 0;

                        MPI_Request *requests = new MPI_Request[3];

                        MPI_Isend((int*)Sizes, 3, MPI_INT, RankProcWithParentNode,
                            Tag, CollectEv_comm, &(requests[1]));
                        MPI_Isend((float*)dataH, sizeH, MPI_FLOAT, RankProcWithParentNode,
                            Tag, CollectEv_comm, &(requests[2]));
                        delete [] requests;

                    }
                    else
                    if (updateRatio->GetDistribFun()->IsDistributionSpecific() == 2) // delta
                    {
                        static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())
                            ->UpdateMomentForm();
                        int sizeMean;
                        const float *dataMean;

                        static_cast<CDenseMatrix<float>*>(updateRatio->GetDistribFun()
                            ->GetMatrix(matMean))->GetRawData(&sizeMean, &dataMean);

                        Sizes[0] = sizeMean;
                        Sizes[1] = 0;
                        Sizes[2] = 1;

                        MPI_Request *requests = new MPI_Request[3];

                        MPI_Isend((int*)Sizes, 3, MPI_INT, RankProcWithParentNode,
                            Tag, CollectEv_comm, &(requests[1]));
                        MPI_Isend((float*)dataMean, sizeMean, MPI_FLOAT, RankProcWithParentNode,
                            Tag, CollectEv_comm, &(requests[2]));
                        delete [] requests;
                        static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())->
                            UpdateCanonicalForm();

                    }
                    else
                    {
                        // gaussian distribution in canonical form
                        if (static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())
                            ->GetCanonicalFormFlag())
                        {
                            static_cast<CDenseMatrix<float>*>(updateRatio->GetDistribFun()
                                ->GetMatrix(matH))->GetRawData(&sizeH, &dataH);
                            
                            static_cast<CDenseMatrix<float>*>(updateRatio->GetDistribFun()
                                ->GetMatrix(matK))->GetRawData(&sizeK, &dataK);

                            Sizes[0] = sizeH;
                            Sizes[1] = sizeK;
//                            Sizes[2] = g;
                            Sizes[2] = 0;                            
                            Data = new float [sizeH + sizeK];
                            
                            memcpy(&Data[0], dataH, sizeH*sizeof(float));
                            memcpy(&Data[sizeH], dataK, sizeK*sizeof(float));

                            MPI_Request *requests = new MPI_Request[3];
                            
                            MPI_Isend((int*)Sizes, 3, MPI_INT, RankProcWithParentNode,
                                Tag, CollectEv_comm, &(requests[1]));
                            MPI_Isend((float*)Data, sizeH + sizeK, MPI_FLOAT, RankProcWithParentNode,
                                Tag, CollectEv_comm, &(requests[2]));
                            delete [] requests;
                            delete [] Data;

                        } // if canonical form
                        else
                            // gaussian distribution in moment form
                            // branch is not tested
                            if (static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())
                                ->GetMomentFormFlag())
                            {
                                MPI_Request *requests = new MPI_Request[6];

                                static_cast<CDenseMatrix<float>*>(updateRatio->GetMatrix(matMean))->
                                    GetRawData(&sizeMean, &dataMean);
                                
                                static_cast<CDenseMatrix<float>*>(updateRatio->
                                    GetMatrix(matCovariance))->GetRawData(&sizeCov, &dataCov);
/*                                
                                normCoeff = 
                                    static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())
                                    ->GetCoefficient(0);
*/                                
                                MPI_Isend(&sizeMean, 1, MPI_INT, RankProcWithParentNode, Tag,
                                    CollectEv_comm, &(requests[1]));
                                
                                MPI_Isend((float*)dataMean, sizeMean, MPI_FLOAT, RankProcWithParentNode,
                                    Tag, CollectEv_comm, &(requests[2]));
                                
                                MPI_Isend(&sizeCov, 1, MPI_INT, RankProcWithParentNode, Tag,
                                    CollectEv_comm, &(requests[3]));
                                
                                MPI_Isend((float*)dataCov, sizeCov, MPI_FLOAT,
                                    RankProcWithParentNode, Tag, CollectEv_comm, &(requests[4]));
                                delete [] requests;
                            } // if moment form
                    }
                } // if dtGaussian
            }// else

            m_QueueNodes.pop();

        } // if ((ValueOfSummator == 0) && (currNode != GetJTreeRootNode()))
        else
        {
            for(i = 0; i < m_NodeConditions[currNode].size() - 1; i++)
            {
                if (m_NodeConditions[currNode][i] == 1)
                {
                    int sizeK;
                    const float *dataK;
                    int sizeH;
                    const float *dataH;
                    float g;

                    potClq = GetJTree()->GetNodePotential(currNode);
                    
                    *potClq *= *m_UpdateRatios[nbrs[i]].front();
                    
                    m_UpdateRatios[nbrs[i]].front()->GetDomain(&numNdsInSepDom, &sepDom);

                    potClq->Normalize();

                    delete m_UpdateRatios[nbrs[i]].front();
                    m_NodeConditions[currNode][i] = 2;
                    --m_NodeConditions[currNode][m_NodeConditions[currNode].size() - 1];
                }
            } // for 

            if (m_NodeConditions[currNode][m_NodeConditions[currNode].size() - 1] > 0)
            {
                m_QueueNodes.pop();
                m_QueueNodes.push(currNode);
            }
            else if (currNode == GetJTreeRootNode())
                m_QueueNodes.pop();
        } // else
    } //while
    m_lastOpDone = opsCollect;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI

void CParJtreeInfEngine::DistributeEvidence()
{
    if (m_NodesOfProcess.empty())
    {
        m_lastOpDone = opsDistribute;
        return;
    }
    
    BuildRoutes();
    m_IsPropagated.resize(m_NodesOfProcess.size());
    for(int i = 0; i < m_NodesOfProcess.size(); i++)
        if (m_NodesOfProcess[i] == m_Roots[m_MyRank])
            m_IsPropagated[i] = true;
        else
            m_IsPropagated[i] = false;
        
    if (m_NumberOfUsedProcesses > 1)
        ProcessRoutes();
    
    const CGraph *pGraph = GetJTree()->GetGraph();
    const int numOfClqs = GetJTree()->GetNumberOfNodes();
    const int *nbr, *nbrs_end;
    boolVector nodesSentMessages(numOfClqs, false);
    intQueue source;

    source.push(m_Roots[m_MyRank]);
    while(!source.empty())
    {
        int sender = source.front();
        int numOfNbrs;
        const int *nbrs;
        const ENeighborType *nbrsTypes;
        
        pGraph->GetNeighbors(sender, &numOfNbrs, &nbrs, &nbrsTypes);
        
        for(nbr = nbrs, nbrs_end = nbrs + numOfNbrs; nbr != nbrs_end; ++nbr)
        {
            if(m_NodeProcesses[*nbr] == m_MyRank)
            {
                // find number of neighbours in m_NodesOfProcess
                int NumNbr = 0;
                while(m_NodesOfProcess[NumNbr] != (*nbr))
                    NumNbr++;
                
                if(!nodesSentMessages[*nbr])
                {
                    if(!m_IsPropagated[NumNbr])
                        PropagateBetweenClqs(sender, *nbr, false);
                    // need to save to the source queue, if its not a leaf node
                    if(pGraph->GetNumberOfNeighbors(*nbr) > 1)
                        source.push(*nbr);
                }
            }
        }
        nodesSentMessages[sender] = true;
        source.pop();
    }
    m_lastOpDone = opsDistribute;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::SetCollectRanks(int Count, int *pCollectRanks)
{
    m_CollectRanks.resize(Count);
    m_CollectRanks.assign(pCollectRanks, pCollectRanks + Count);
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::InitMPIConsts()
{
    MPI_Comm_size(MPI_COMM_WORLD, &m_NumberOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &m_MyRank);
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::InitWeightArrays()
{
    m_pWeightesOfNodes = NULL;
    m_pWeightesOfSubTrees = NULL;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::CollectFactorsOnProcess(int MainProcNum)
{
    int i, j;

    MPI_Group World_group, CollectF_group;
    MPI_Comm CollectF_comm;
    int *Ranks, m_size;
    
    MPI_Comm_size(MPI_COMM_WORLD, &m_size);
    Ranks = new int[m_size];
    for (i = 0; i < m_size; i++)
        Ranks[i] = i;
    MPI_Comm_group (MPI_COMM_WORLD, &World_group);
    MPI_Group_incl(World_group, m_size, Ranks, &CollectF_group);
    MPI_Comm_create(MPI_COMM_WORLD, CollectF_group, &CollectF_comm);
    
    int flag;
    if (m_NumberOfProcesses == 1)
        return;
    if (m_NumberOfUsedProcesses == 1)
        return;
    
    if (m_MyRank != MainProcNum)
    {
        int MsgsNum = m_NodesOfProcess.size();
        CPotential *potForSending; 
        CNumericDenseMatrix<float>* matForSending;
        int matDim;
        const float *pDataForSending;
        int dataLength;
        int maxTagInCollectEvidence = GetMaxTagForCollectEvidence();
        int Tag = maxTagInCollectEvidence + 1;
        
        for(int node = 0; node < MsgsNum; node++)
        {
            potForSending = GetJTree()->GetNodePotential(m_NodesOfProcess[node]);
            
            // send number of node

            MPI_Send(&m_NodesOfProcess[node], 1, MPI_INT, MainProcNum, Tag,
                CollectF_comm);

            CDistribFun *df = potForSending->GetDistribFun();

            if (df->GetDistributionType() == dtTabular)
            {
                static_cast<CNumericDenseMatrix<float>*>(df->GetMatrix(matTable))
                    ->GetRawData(&dataLength, &pDataForSending);

                MPI_Send(&dataLength, 1, MPI_INT, MainProcNum, Tag, CollectF_comm);
                MPI_Send((float*)pDataForSending, dataLength, MPI_FLOAT, MainProcNum,
                    Tag, CollectF_comm);

            }
            if (df->GetDistributionType() == dtGaussian)
            {
                const float *dataH, *dataK;
                int sizeH, sizeK;
                int flag;
                
                const float *dataMean, *dataCov;
                int sizeMean, sizeCov;
//                float normCoeff;
                
                int *sizes;
                
                if (df->IsDistributionSpecific() == 1)
                {
                    dataH = dataK = NULL;
                    sizeH = sizeK = 0;
                    flag = 0;

                    sizes = new int[3];
                    sizes[0] = sizes[1] = sizes[2] = 0;

                    MPI_Send(sizes, 3, MPI_INT, MainProcNum, Tag, CollectF_comm);
                    
                    MPI_Send((float*)dataK, sizeK, MPI_FLOAT, MainProcNum,
                        Tag, CollectF_comm);

                     delete [] sizes;
                }
                else
                    if (df->IsDistributionSpecific() == 2)
                    {
                        static_cast<CGaussianDistribFun *>(df)->UpdateMomentForm();
                        int sizeMean;
                        const float *dataMean;
                        
                        static_cast<CNumericDenseMatrix<float>*>(df
                            ->GetMatrix(matMean))->GetRawData(&sizeMean, &dataMean);
                        
                        sizes[0] = sizeMean;
                        sizes[1] = 0;
                        sizes[2] = 1;
                        
                        MPI_Send((int*)sizes, 3, MPI_INT, MainProcNum,
                            Tag, CollectF_comm);
                        
                        MPI_Send((float*)dataMean, sizeMean, MPI_FLOAT, MainProcNum, 
                            Tag, CollectF_comm);

                        static_cast<CGaussianDistribFun *>(df)->UpdateCanonicalForm();
                    }
                    else

                    if (static_cast<CGaussianDistribFun *>(df)->
                        GetCanonicalFormFlag())
                    {
                        static_cast<CNumericDenseMatrix<float>*>(df->
                            GetMatrix(matH))->GetRawData(&sizeH, &dataH);
                        
                        static_cast<CNumericDenseMatrix<float>*>(df->
                            GetMatrix(matK))->GetRawData(&sizeK, &dataK);
                        
//                        g = static_cast<CGaussianDistribFun *>(df)->GetCoefficient(1);
                        
                        sizes = new int[3];
                        sizes[0] = sizeH;
                        sizes[1] = sizeK;
                        sizes[2] = 0;

                        int DataSize = sizeH + sizeK;                        
                        float *Data = new float [DataSize];
                        
                        memcpy(&Data[0], dataH, sizeH*sizeof(float));
                        memcpy(&Data[sizeH], dataK, sizeK*sizeof(float));
                        
                        MPI_Send((int*)sizes, 3, MPI_INT, MainProcNum, Tag, CollectF_comm);
                        
                        MPI_Send((float*)Data, DataSize, MPI_FLOAT, MainProcNum, Tag, 
                            CollectF_comm);
                        
                        delete [] Data;
                        delete [] sizes;

                    }
                    else
                        // gaussian distribution in moment form
                        // branch is not tested
                        if (static_cast<CGaussianDistribFun *>(df)->
                            GetMomentFormFlag())
                        {
                            static_cast<CNumericDenseMatrix<float>*>(potForSending->GetMatrix(matMean))->
                                GetRawData(&sizeMean, &dataMean);
                            
                            static_cast<CNumericDenseMatrix<float>*>(potForSending->
                                GetMatrix(matCovariance))->GetRawData(&sizeCov, &dataCov);
                            
/*                            normCoeff = 
                                static_cast<CGaussianDistribFun *>(df)
                                ->GetCoefficient(0);
*/                           
                            MPI_Send(&sizeMean, 1, MPI_INT, MainProcNum, Tag, CollectF_comm);
                            
                            MPI_Send((float*)dataMean, sizeMean, MPI_FLOAT, MainProcNum,
                                Tag, CollectF_comm);
                            
                            MPI_Send(&sizeCov, 1, MPI_INT, MainProcNum, Tag, CollectF_comm);
                            
                            MPI_Send((float*)dataCov, sizeCov, MPI_FLOAT, MainProcNum, 
                                Tag, CollectF_comm);

//                            MPI_Send(&normCoeff, 1, MPI_FLOAT, MainProcNum, Tag, CollectF_comm);

                        }// if moment form
            }// if dtGaussuan
        }// for (node=0....
    }// if
    
    else
    {
        intVector NumOfNodesInProcess(m_NumberOfUsedProcesses, 0);
        int NumOfNds = m_NodeProcesses.size();
        for(i = 0; i < NumOfNds; i++)
            NumOfNodesInProcess[m_NodeProcesses[i]]++;
        
        MPI_Status status;
        CPotential *potNode;
        CPotential *TmpPot;
        int node;
        for(i = 0; i < m_NumberOfUsedProcesses; i++)
        {
            if (i != m_MyRank)

                for (j = 0; j < NumOfNodesInProcess[i]; j++)
                {
                    MPI_Recv(&node, 1, MPI_INT, i, MPI_ANY_TAG, CollectF_comm, &status);

                    potNode = GetJTree()->GetNodePotential(node);
                    
                    int numNdsInSepDom;
                    const int *sepDom;
                    
                    potNode->GetDomain(&numNdsInSepDom, &sepDom);
                    const pConstNodeTypeVector *nt = potNode->GetDistribFun()->
                        GetNodeTypesVector();

                    if (potNode->GetDistributionType() == dtTabular)
                    {
                        int dataLength;
                        const float *data;

                        MPI_Recv(&dataLength, 1, MPI_INT, i, MPI_ANY_TAG, CollectF_comm,
                            &status);
                        data = new float [dataLength];
                        MPI_Recv((float*)data, dataLength, MPI_FLOAT, i, MPI_ANY_TAG, CollectF_comm,
                            &status);
                        CTabularDistribFun *df = CTabularDistribFun::Create(numNdsInSepDom,
                            &nt->front(), data );
                        potNode->SetDistribFun(df);

                        delete [] (float*)data;
                        delete df;
                    }

                    if (potNode->GetDistributionType() == dtGaussian)
                    {
                        const float *dataH, *dataK;
                        int sizeH, sizeK;
                        int flag;

                        const float *dataMean, *dataCov;
                        int sizeMean, sizeCov;
//                        float normCoeff;
                        
                        int *sizes;
                        float *data;
                        // gaussian distribution in canonical form
                        if (static_cast<CGaussianDistribFun *>(potNode->GetDistribFun())
                            ->GetCanonicalFormFlag())
                        {
                            sizes = new int[3];
                            MPI_Recv(sizes, 3, MPI_INT, i, MPI_ANY_TAG, CollectF_comm, 
                                &status);
                            
                            sizeH = sizes[0];
                            sizeK = sizes[1];
                            flag = sizes[2];
                            
                            if (flag == 1) // delta
                            {
                                float *dataMean = new float[sizeH];
                                
                                MPI_Recv((float*)dataMean, sizeH, MPI_FLOAT, i, MPI_ANY_TAG, 
                                    CollectF_comm, &status);

                                CGaussianDistribFun *df = 
                                    CGaussianDistribFun::CreateDeltaDistribution(numNdsInSepDom, 
                                    &nt->front(), dataMean);
                                
                                df->UpdateCanonicalForm();
                                
                                potNode->SetDistribFun(df);
                            }
                            else
                            {
                                dataH = new float[sizeH];
                                dataK = new float[sizeK];
                                
                                data = new float [sizeH+sizeK];
                                
                                MPI_Recv((int*)data, sizeH+sizeK, MPI_INT, i, MPI_ANY_TAG, 
                                    CollectF_comm, &status);
                                
                                memcpy((void*)dataH, data, sizeH*sizeof(float));
                                memcpy((void*)dataK, data + sizeH, sizeK*sizeof(float));
                                
                                delete [] data;
                                
                                CGaussianDistribFun *df = 
                                    CGaussianDistribFun::CreateInCanonicalForm(numNdsInSepDom,
                                    &nt->front(), dataH, dataK, 0.0f );
                                
                                potNode->SetDistribFun(df);
                                
                                if (dataH != NULL)
                                    delete [] (float*)dataH;
                                if (dataK != NULL)
                                    delete [] (float*)dataK;
                                delete df;
                            }
                            delete [] sizes;
                        }
                        else
                            // gaussian distribution in moment form
                            // bransh is not tested
                            if (static_cast<CGaussianDistribFun *>(potNode->GetDistribFun())
                                ->GetMomentFormFlag())
                            {
                               
                                MPI_Recv(&sizeMean, 1, MPI_INT, i, MPI_ANY_TAG, CollectF_comm,
                                    &status);
                                
                                dataMean = new float[sizeMean];
                                
                                MPI_Recv((float*)dataMean, sizeMean, MPI_FLOAT, i, MPI_ANY_TAG, 
                                    CollectF_comm, &status);
                                
                                MPI_Recv(&sizeCov, 1, MPI_INT, i, MPI_ANY_TAG, CollectF_comm, 
                                    &status);
                                
                                dataCov = new float[sizeCov];
                                
                                MPI_Recv((float*)dataCov, sizeCov, MPI_FLOAT, i, MPI_ANY_TAG, 
                                    CollectF_comm, &status);

/*                                MPI_Recv(&normCoeff, 1, MPI_FLOAT, i, MPI_ANY_TAG, 
                                    CollectF_comm, &status);
*/
                                CGaussianDistribFun *df = 
                                    CGaussianDistribFun::CreateInMomentForm(1, numNdsInSepDom,
                                    &nt->front(), dataMean, dataCov, NULL);
                                df->SetCoefficient(0.0f);
                                potNode->SetDistribFun(df);

                                delete [] (float*)dataMean, (float*)dataCov;
                                delete df;
                            } // if moment form
                    }// if dtGaussian
                    
                }// for (j = 0...
        }// for (i = 0;...
    }// else
    delete [] Ranks;
}



#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParJtreeInfEngine::GetMaxTagForCollectEvidence()
{
    int tag = GetJTree()->GetNumberOfNodes() + 
        GetJTree()->GetNumberOfNodes() * GetJTree()->GetNumberOfNodes();
    return tag;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::DivideNodes(int AlgType)
{
    if (AlgType == 2)
    {
        DivideNodesWithMinimumDeviationSearch();
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
    
    const CJunctionTree* pJTree = GetJTree();

    PNL_CHECK_IS_NULL_POINTER(pJTree);
    
    NumberOfNodes = pJTree->GetNumberOfNodes();
    
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
        if (NumberOfNodes != 0)
        {
            GetNextNodeInList(&NodesList, &NodesListIterator, &CurrentRoot,
                &PrevCurrentRoot);
            W = GetSubTreeWeight(GetJTreeRootNode());
            WThreshold = W / p;
            
            //label 1 
            while (FunctionStillRunning)
            {
                if (GetSubTreeWeight(CurrentRoot) >= WThreshold)
                {
                    NeedToSendASubTreeToProcess = true;
                }
                else
                {
                    Parents.clear();
                    GetParents(CurrentRoot, &Parents);
                    
                    if (Parents[0] != -1)
                    {
                        PrevCurrentRoot = CurrentRoot;
                        CurrentRoot = Parents[0];
                        NeedToSendASubTreeToProcess = false;
                    }
                    else //if (Parents[0]!=-1...
                    {
                        A = CurrentRoot;
                        NeedToSendASubTreeToProcess = true;
                    }
                }
                
                if (NeedToSendASubTreeToProcess)
                {
                    switch(AlgType)
                    {
                    case 0:
                        A = GetNodeWithOptimalSubTreeWeight(CurrentRoot, PrevCurrentRoot,
                            WThreshold);
                        break;
                    default:
                        A = GetNodeWithOptimalSubTreeWeight(CurrentRoot, WThreshold);
                        break;
                    };
                    
                    SendASubTreeToNextProcess(A, LastUsedProcess);
                    LastUsedProcess++;
                    
                    W -= GetSubTreeWeight(A);
                    p--;
                    
                    AddNodesNumbersOfSubTreeToUsedNodes(A);
                    
                    BuildWeightesOfNodes();
                    BuildWeightesOfSubTrees();
                    
                    if (GetNumberOfFreeNodesInList(&NodesList, &NodesListIterator) > 0)
                    {
                        if (p != 0)
                        {
                            WThreshold = W / p;
                            GetNextNodeInList(&NodesList, &NodesListIterator, &CurrentRoot,
                                &PrevCurrentRoot);
                        }
                        else
                        {
                            PNL_THROW(CInternalError, "Number of processes in CParJtreeInfEngine::DivideNodes <=0 when node list is not empty")
                        }
                    }
                    else 
                    {
                        FunctionStillRunning = false;
                    }
                    
                } //if (Need...
            } //while 
        } //if (NumberOfNodes!=...
        else
        {
            PNL_THROW(COutOfRange, "CParJtreeInfEngine::DivideNodes. Tree is empty.");
        }
    }
    else //if (p!=0...
    {
        PNL_THROW(COutOfRange, "CParJtreeInfEngine::DivideNodes. Number of processes is 0.");
    }
    
    m_NumberOfUsedProcesses = LastUsedProcess + 1;
    m_UsedNodes.clear();
    
    BuildWeightesOfNodes();
    BuildWeightesOfSubTrees();
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::SetNodesList(list<int> *pNodesList)
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
    CurrentLevelNodes.insert(GetJTreeRootNode());
    
    NextLevelNodes.clear();
    
    pNodesList->clear();
    
    while (FunctionStillRunning)
    {
        //set NextLevelNodes
        CurrentLevelNodesIterator = CurrentLevelNodes.begin();
        
        NextLevelNodes.clear();
        
        while (CurrentLevelNodesIterator!=CurrentLevelNodes.end())
        {
            GetChildren(*CurrentLevelNodesIterator, &Children);
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
void CParJtreeInfEngine::GetNextNodeInList(const list <int> *pNodesList, 
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
double CParJtreeInfEngine::GetSubTreeWeight(int nodenum)
{
    if (m_pWeightesOfSubTrees == NULL)
        BuildWeightesOfSubTrees();
    
    if ((nodenum >= 0) && (nodenum < m_pWeightesOfSubTrees->size()))
    {
        return (*m_pWeightesOfSubTrees)[nodenum];
    }
    else
        return -1;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParJtreeInfEngine::GetNodeWithOptimalSubTreeWeight(int CurrentRoot,
    int PrevCurrentRoot, double WThreshold)
{
    if (PrevCurrentRoot == -1)
    {
        if (CurrentRoot != -1)
        {
            return CurrentRoot;
        }
        else 
        {
            PNL_THROW(COutOfRange, "CParJtreeInfEngine::DivideNodes. CurrentRoot must be != -1.");
        }
    }
    
    double CurrentDivergence = fabs(GetSubTreeWeight(CurrentRoot) - WThreshold);
    double PrevDivergence = fabs(GetSubTreeWeight(PrevCurrentRoot) - WThreshold);
    
    if (CurrentDivergence <= PrevDivergence)
        return CurrentRoot;
    else 
        return PrevCurrentRoot;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParJtreeInfEngine::GetNodeWithOptimalSubTreeWeight(int CurrentRoot,
    double WThreshold) 
{
    PNL_CHECK_RANGES(CurrentRoot, 0, GetJTree()->GetNumberOfNodes() - 1);
    if (WThreshold<=0)
    {
        PNL_THROW(COutOfRange, "CParJtreeInfEngine::GetNodeWithOptimalSubTreeWeight. WThreshold must be > 0.");
    }
    
    intVector Children;
    intVector::iterator ChildrenIterator;
    GetChildren(CurrentRoot, &Children);
    ChildrenIterator = Children.begin();
    
    int NodeWithMinDeviation = CurrentRoot;
    double MinDeviation = fabs(GetSubTreeWeight(CurrentRoot) - WThreshold);
    double Deviation;
    
    while (ChildrenIterator != Children.end())
    {
        Deviation = fabs(GetSubTreeWeight(*ChildrenIterator) - WThreshold);
        
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
void CParJtreeInfEngine::SendASubTreeToNextProcess(int A, int LastUsedProcess)
{
    intVector UnusedNodesInSubTree;
    intVector::iterator UnusedNodesInSubTreeIterator;
    UnusedNodesInSubTree.clear();
    
    int UsingProcess = LastUsedProcess + 1;
    
    GetUnusedNodesInSubTree(A, &UnusedNodesInSubTree);
    
    m_Roots[UsingProcess] = A;
    
    if (UsingProcess == m_MyRank)
    {
        m_NodesOfProcess = UnusedNodesInSubTree;
    }
    
    UnusedNodesInSubTreeIterator = UnusedNodesInSubTree.begin();
    
    while (UnusedNodesInSubTreeIterator != UnusedNodesInSubTree.end())
    {
        m_NodeProcesses[*UnusedNodesInSubTreeIterator] = UsingProcess;
        UnusedNodesInSubTreeIterator++;
    }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::AddNodesNumbersOfSubTreeToUsedNodes(int A)
{
    intVector UnusedNodesInSubTree; 
    UnusedNodesInSubTree.clear();
    
    GetUnusedNodesInSubTree(A, &UnusedNodesInSubTree);
    
    m_UsedNodes.insert(UnusedNodesInSubTree.begin(), UnusedNodesInSubTree.end());
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::BuildWeightesOfNodes()
{
    int numberofcliques = GetOriginalJTree()->GetNumberOfNodes();
    int i;
    
    if (m_pWeightesOfNodes == NULL)
        m_pWeightesOfNodes = new doubleVector;
    
    m_pWeightesOfNodes->clear();
    m_pWeightesOfNodes->resize(numberofcliques, 0);
    
    for (i = 0; i < numberofcliques; i++)
    {
        if (!m_UsedNodes.count(i))
        {
            EDistributionType dt = GetJTree()->GetNodePotential(i)->GetDistributionType();

            if (dt == dtTabular)
                (*m_pWeightesOfNodes)[i] = GetWeightOfTabularClq(i);
            else
                if (dt == dtGaussian)
                    (*m_pWeightesOfNodes)[i] = GetWeightOfContinuousClq(i);
                else
                    if (dt == dtScalar)
                        (*m_pWeightesOfNodes)[i] = 0;
                    else
                        PNL_THROW(CNotImplemented,
                        "Clique must be tabular, gaussian or scalar only");
        }
    }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::BuildWeightesOfSubTrees()
{
    int numberofcliques = GetOriginalJTree()->GetNumberOfNodes();
    
    if (m_pWeightesOfSubTrees == NULL)
        m_pWeightesOfSubTrees = new doubleVector;
    
    m_pWeightesOfSubTrees->clear();
    m_pWeightesOfSubTrees->resize(numberofcliques, 0);
    
    BuildWeightesOfNodes();
    
    boolVector nodesCollectedWeight;
    nodesCollectedWeight.resize(numberofcliques, false);
    
    const int *nbr, *nbrs_end;
    intVector::const_iterator sourceIt, source_end;
    intVecVector::const_iterator layerIt = GetCollectSequence().begin(),
        collSeq_end = GetCollectSequence().end();
    
    const CGraph *pGraph = GetOriginalJTree()->GetGraph();

    for(; layerIt != collSeq_end; ++layerIt)
    {
        for(sourceIt = layerIt->begin(), source_end = layerIt->end();
        sourceIt != source_end; ++sourceIt)
        {
            int numOfNbrs;
            const int *nbrs;
            const ENeighborType *nbrsTypes;
            
            nodesCollectedWeight[*sourceIt] = true;
            
            if (!m_UsedNodes.count(*sourceIt))
                (*m_pWeightesOfSubTrees)[*sourceIt] += 
                (*m_pWeightesOfNodes)[*sourceIt];
            
            pGraph->GetNeighbors(*sourceIt, &numOfNbrs, &nbrs, &nbrsTypes);
            
            for(nbr = nbrs, nbrs_end = nbrs + numOfNbrs; nbr != nbrs_end; ++nbr)
            {
                if(nodesCollectedWeight[*nbr])
                {
                    (*m_pWeightesOfSubTrees)[*sourceIt] += 
                        (*m_pWeightesOfSubTrees)[*nbr];
                }
            }
            
        }
    }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParJtreeInfEngine::GetNumberOfFreeNodesInList(const list<int> *pNodesList,
    list<int>::const_iterator *pNodesListIterator)
{
    int NumberOfUnfreeNodesInList = 0;
    
    list<int>::const_iterator TempNodesListIterator = *pNodesListIterator;
    
    int NodesListSizeSinceIterator = 0;
    
    while (TempNodesListIterator != pNodesList->end())
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
void CParJtreeInfEngine::GetChildren(int NumOfNode, intVector *Children)
{
    const CGraph *pGraph = GetOriginalJTree()->GetGraph();
    int numOfNbrs;
    const int *nbrs;
    const ENeighborType *nbrsTypes;
    
    pGraph->GetNeighbors(NumOfNode, &numOfNbrs, &nbrs, &nbrsTypes);
    if ((numOfNbrs == 1) && (NumOfNode != GetJTreeRootNode()))
        Children->resize(0);
    else 
        if ((NumOfNode == GetJTreeRootNode()))
            Children->resize(numOfNbrs);
        else
            Children->resize(numOfNbrs - 1);
        
    int j = 0;
    for (int i = 0;i < numOfNbrs; i++)
    {
        if (m_NbrsTypes[NumOfNode][i] == ntChild)
        {
            (*Children)[j] = nbrs[i];
            j++;
        }
    }
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::GetUnusedNodesInSubTree(int SubTreeRoot,
    intVector *pUnusedNodesInSubTree)
{
    bool FunctionStillRunning = true;
    
    set<int> CurrentLevelNodes;
    set<int>::iterator CurrentLevelNodesIterator;
    set<int> NextLevelNodes;
    
    intVector Children;
    Children.clear();
    
    CurrentLevelNodes.clear();
    CurrentLevelNodes.insert(SubTreeRoot);
    
    NextLevelNodes.clear();
    
    pUnusedNodesInSubTree->clear();
    
    if (!m_UsedNodes.count(SubTreeRoot))
        while (FunctionStillRunning)
        {
            //set NextLevelNodes
            CurrentLevelNodesIterator = CurrentLevelNodes.begin();
            
            NextLevelNodes.clear();
            
            while (CurrentLevelNodesIterator != CurrentLevelNodes.end())
            {
                if (!m_UsedNodes.count(*CurrentLevelNodesIterator))
                {
                    GetChildren(*CurrentLevelNodesIterator, &Children);
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
        } //while (FunctionStillRunning)...
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
double CParJtreeInfEngine::GetWeightOfTabularClq(int NumOfClq)
{
    double K = 1; // parameter
    int NodeContentSize; // clique's size
    const int *Content; // pointer to the fist node in clique
    
    GetOriginalJTree()->GetNodeContent(NumOfClq, &NodeContentSize, &Content);
    
    nodeTypeVector *vector = new nodeTypeVector();
    GetOriginalJTree()->GetNodeTypes(vector);
    
    // node weight
    double Weight = 1;
    
    int numOfNbrs;
    const int *nbrs;
    const ENeighborType *nbrsTypes;
    const CGraph *pGraph = GetOriginalJTree()->GetGraph();
    
    pGraph->GetNeighbors(NumOfClq, &numOfNbrs, &nbrs, &nbrsTypes);
    
    intVector  *nodeAssociationOut = new intVector();
    GetOriginalJTree()->GetModelDomain()->GetVariableAssociations(
        nodeAssociationOut);
    
    Weight = 1;
    for (int j = 0; j < NodeContentSize; j++)
    {
        int nextnode = Content[j];
        if (!IsInActuallyObsNodes(nextnode))
        {
            int numoftype = (*nodeAssociationOut)[nextnode];
            Weight *= (*vector)[numoftype].GetNodeSize();
        }
    }
    if (NumOfClq == GetJTreeRootNode()) 
        if (numOfNbrs != 0)
            Weight *= (2 * K + 1) * numOfNbrs;
        else
            if (numOfNbrs > 1)
                Weight *= (2 * K + 1) * (numOfNbrs - 1) + 1;
            
    return Weight;
}

double CParJtreeInfEngine::GetWeightOfContinuousClq(int NumOfClq)
{
    int NodeContentSize; // clique's size
    const int *Content; // pointer to the fist node in clique
    
    GetOriginalJTree()->GetNodeContent(NumOfClq, &NodeContentSize, &Content);

    nodeTypeVector *vector = new nodeTypeVector();
    GetOriginalJTree()->GetNodeTypes(vector);
    
    double AllSizes = 0;
    
    intVector  *nodeAssociationOut = new intVector();
    GetOriginalJTree()->GetModelDomain()->GetVariableAssociations(
        nodeAssociationOut);
    
    for (int j = 0; j < NodeContentSize; j++)
    {
        int numoftype = (*nodeAssociationOut)[Content[j]];
        AllSizes += (*vector)[numoftype].GetNodeSize();
    }

    double Weight = 1;
    Weight = (AllSizes + 1)*AllSizes;
  
    delete nodeAssociationOut;
    return Weight;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
double CParJtreeInfEngine::GetBaseWeightOfClq(int NumOfClq)
{
    double K = 1; // parameter
    int NodeContentSize; // clique's size
    const int *Content; // pointer to the fist node in clique
    
    // get size and pointer to the first node of clique
    GetOriginalJTree()->GetNodeContent(NumOfClq, &NodeContentSize, &Content);
    
    nodeTypeVector *vector = new nodeTypeVector();
    GetOriginalJTree()->GetNodeTypes(vector);
    
    // node weight
    double Weight=1;
    
    int numOfNbrs;
    const int *nbrs;
    const ENeighborType *nbrsTypes;
    const CGraph *pGraph = GetOriginalJTree()->GetGraph();
    
    pGraph->GetNeighbors(NumOfClq, &numOfNbrs, &nbrs, &nbrsTypes);
    
    intVector  *nodeAssociationOut = new intVector();
    GetOriginalJTree()->GetModelDomain()->GetVariableAssociations(
        nodeAssociationOut);
    
    Weight = 1;
    for (int j = 0; j < NodeContentSize; j++)
    {
        int nextnode = Content[j];
        if (!IsInActuallyObsNodes(nextnode))
        {
            int numoftype = (*nodeAssociationOut)[nextnode];
            Weight *= (*vector)[numoftype].GetNodeSize();
        }
    }
    return Weight;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
bool CParJtreeInfEngine::IsInActuallyObsNodes(int NumOfNode)
{
    const intVector& actuallyObsNodes = GetActuallyObservedNodes();
    bool Flag = 0;
    for(int i = 0; i < actuallyObsNodes.size(); i++)
        if (NumOfNode == actuallyObsNodes[i])
            Flag = 1;

    return Flag;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::DivideNodesWithMinimumDeviationSearch()
{
    int NumberOfNodes;
    int p = m_NumberOfProcesses; //number of processes
    double W; //Weight of tree
    double WThreshold; //threshold
    int A; //Root of subtree, which we will send to the next processor
    int LastUsedProcess = -1;
    bool FunctionStillRunning = true;
    
    NumberOfNodes = GetJTree()->GetNumberOfNodes();

    m_UsedNodes.clear();
    
    //set some params 
    m_NodeProcesses.resize(NumberOfNodes, -1);
    m_NodesOfProcess.clear();
    
    m_Roots.clear();
    
    PNL_CHECK_LEFT_BORDER(m_NumberOfProcesses, 1);
    
    m_Roots.resize(m_NumberOfProcesses, -1);
    
    BuildWeightesOfNodes();
    BuildWeightesOfSubTrees();
    
    
    if (p > 0)
    {
        if (NumberOfNodes != 0)
        {
            W = (*m_pWeightesOfSubTrees)[GetJTreeRootNode()];
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
                
                if (m_UsedNodes.size() !=  NumberOfNodes)
                {
                    if (p != 0)
                    {
                        WThreshold = W / p;
                        FunctionStillRunning = true;
                    }
                    else
                    {
                        PNL_THROW(CInternalError, "Number of processes in CParJtreeInfEngine::DivideNodes <=0 when node list is not empty")
                    }
                }
                else 
                    FunctionStillRunning = false;
            } //while
        } //if (NumberOfNodes != 0)...
        else 
            PNL_THROW(COutOfRange, "CParJtreeInfEngine::DivideNodes. Tree is empty.");
    }
    else //if (p! =0)...
        PNL_THROW(COutOfRange, "CParJtreeInfEngine::DivideNodes. Number of processes is 0.");
    
    m_NumberOfUsedProcesses = LastUsedProcess + 1;
    m_UsedNodes.clear();
    
    BuildWeightesOfNodes();
    BuildWeightesOfSubTrees();
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParJtreeInfEngine::GetNodeWithMinimumDeviation(double WThreshold) 
{
    if (WThreshold <= 0)
        PNL_THROW(COutOfRange, "CParJtreeInfEngine::GetNodeWithMinimumDeviation. WThreshold must be > 0.");
    
    if (m_UsedNodes.size() >= GetJTree()->GetNumberOfNodes())
        PNL_THROW(CInconsistentState, "CParJtreeInfEngine::GetNodeWithMinimumDeviation. There are no free nodes.");
    
    doubleVector::iterator WeightOfSubTreeIterator;
    
    int Counter = 0;
    
    WeightOfSubTreeIterator = m_pWeightesOfSubTrees->begin();
    
    int ResultNode = GetJTreeRootNode();
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
            }
        }
        Counter ++;
        WeightOfSubTreeIterator++;
    }
    
    return ResultNode;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParJtreeInfEngine::NumOfSendedNode(int Tag)
{
    //Tag = num(to) + num(from) * numOfNds
    int num = Tag / GetJTree()->GetNumberOfNodes();
    return num;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParJtreeInfEngine::NumOfRecvedNode(int Tag)
{
    //Tag = num(to) + num(from) * numOfNds
    int num = Tag % GetJTree()->GetNumberOfNodes();
    return num;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
int CParJtreeInfEngine::GetTagForSending(int sourceNode, int sinkNode)
{
    int tag = sinkNode + sourceNode * GetJTree()->GetNumberOfNodes();
    return tag;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::CollectCommCreate()
{
    int np;
    int *Ranks;
    int i;
    
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    Ranks = new int[np];
    for (i = 0; i < np; i++)
        Ranks[i] = i;
    MPI_Comm_group (MPI_COMM_WORLD, &World_group);
    MPI_Group_incl(World_group, np, Ranks, &CollectEv_group);
    MPI_Comm_create(MPI_COMM_WORLD, CollectEv_group, &CollectEv_comm);
    
    delete [] Ranks;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::BuildRoutes()
{
    m_Routes.resize(m_NumberOfUsedProcesses - 1);
    int* tempRoutes = new int [GetJTree()->GetNumberOfNodes()];
    int routeSize = 0;
    int rootNum, index; 
    intVector *parent = new intVector(1);
    for(rootNum = 0, index = 0; rootNum < m_NumberOfUsedProcesses;
    rootNum++,index++)
    {
        tempRoutes[0] = m_Roots[rootNum];
        routeSize = 1;
        bool IsRoot = 0;
        do
        {
            GetParents(tempRoutes[routeSize - 1], parent);
            if((*parent)[0] != -1)
            {
                tempRoutes[routeSize] = (*parent)[0];
                routeSize++;
                IsRoot = 0;
                for(int i = 0; i < m_NumberOfUsedProcesses; i++)
                    if((*parent)[0] == m_Roots[i])
                        IsRoot = 1;
            }
        }
        while(((*parent)[0]>-1) && !IsRoot);
        
        if(routeSize > 1)
        {
            m_Routes[index].resize(routeSize);
            m_Routes[index].assign(tempRoutes,tempRoutes+routeSize);
        }
        else
            index--;
    }
    delete [] tempRoutes;
}
#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI

void CParJtreeInfEngine::ProcessRoutes()
{
    CPotential *potClq,*potSep;
    CPotential *newPotSep, *updateRatio;
    int numNdsInSepDom;
    const int *sepDom;
    int i, j;
    MPI_Status status;

    // find on which process main root is
    int NumProcOfMainRoot = 0;
    while (m_Roots[NumProcOfMainRoot] != GetJTreeRootNode())
        NumProcOfMainRoot++;

    if (m_MyRank >= m_NumberOfUsedProcesses)
    {
        return;
    }

    if (m_MyRank != NumProcOfMainRoot) // process is waiting
    {
        int NumOfRoute = 0;
        
        while (m_Roots[m_MyRank] != m_Routes[NumOfRoute][0])
        { 
            NumOfRoute++;
        }
        
        // find process from which we are received
        int NumOfSendProc = 0;
        while (m_Roots[NumOfSendProc] !=
            m_Routes[NumOfRoute][m_Routes[NumOfRoute].size() - 1])
        {
            NumOfSendProc++;
        }
        // receiving of update ratio
        CPotential *UpdateRatio;
        CPotential *Pot = 
            GetJTree()->GetSeparatorPotential(m_Routes[NumOfRoute][1], m_Routes[NumOfRoute][0]);
        
        CPotential *PotCls = GetJTree()->GetNodePotential(m_Routes[NumOfRoute][1]);
        EDistributionType dtCls = PotCls->GetDistributionType();

        CModelDomain* pMD = Pot->GetModelDomain();
        Pot->GetDomain(&numNdsInSepDom, &sepDom);
        
        intVector sepDomain(numNdsInSepDom);
        sepDomain.assign(sepDom, sepDom + numNdsInSepDom);
        
        intVector obsIndicesIn;
        int count = sepDomain.size();
        
        obsIndicesIn.clear();
        for (i = 0; i < count; i++)
            if (m_pEvidence->IsNodeObserved(sepDomain[i]))
                obsIndicesIn.push_back(i);

        EDistributionType dt = Pot->GetDistribFun()->GetDistributionType();
       
        int dataLength;
        const float *pDataForRecving;
        
        const float *dataH, *dataK;
        int sizeH, sizeK;
        int flag;

        const float *dataMean, *dataCov;
        int sizeMean, sizeCov;
//        float normCoeff;

        if ( dtCls == dtTabular)
        {
            MPI_Recv(&dataLength, 1, MPI_INT, NumOfSendProc, MPI_ANY_TAG, 
                MPI_COMM_WORLD, &status);
            pDataForRecving = new float [dataLength];
            MPI_Recv((float*)pDataForRecving, dataLength, MPI_FLOAT, NumOfSendProc,
                MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
        else
        {
            if (dtCls == dtGaussian)
            {
                int *Sizes = new int [3];
                float *data;

                // gaussian distribution in canonical form
                if (static_cast<CGaussianDistribFun *>(Pot->GetDistribFun())
                    ->GetCanonicalFormFlag())
                {

                    MPI_Recv((int*)Sizes, 3, MPI_INT, NumOfSendProc, MPI_ANY_TAG, 
                        MPI_COMM_WORLD, &status);
                    sizeH = Sizes[0];
                    sizeK = Sizes[1];
                    flag = Sizes[2];

                    if (flag == 1) // delta
                    {
                        float *dataMean = new float[sizeH];
                        
                        MPI_Recv((float*)dataMean, sizeH, MPI_FLOAT,
                            NumOfSendProc, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    }
                    else
                    {
                        dataH = new float[sizeH];
                        dataK = new float[sizeK];
                        
                        int DataSize =  sizeH + sizeK;
                        data = new float [DataSize];
                        
                        MPI_Recv((float*)data, DataSize, MPI_FLOAT, NumOfSendProc, 
                            MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                        
                        memcpy((void*)dataH, data, sizeH*sizeof(float));
                        memcpy((void*)dataK, data + sizeH, sizeK*sizeof(float));
                        delete [] data;
                    }
                }// if canonical
                else                

                // gaussian distribution in moment form
                // branch is not tested
                if (static_cast<CGaussianDistribFun *>(Pot->GetDistribFun())
                    ->GetMomentFormFlag())
                {

                    MPI_Recv(&sizeMean, 1, MPI_INT, NumOfSendProc, MPI_ANY_TAG, 
                        MPI_COMM_WORLD, &status);
                    
                    dataMean = new float[sizeMean];
                    
                    MPI_Recv((float*)dataMean, sizeMean, MPI_FLOAT, NumOfSendProc, 
                        MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    
                    MPI_Recv(&sizeCov, 1, MPI_INT,
                        NumOfSendProc, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    
                    dataCov = new float[sizeCov];
                    
                    MPI_Recv((float*)dataCov, sizeCov, MPI_FLOAT, NumOfSendProc, 
                        MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    
/*                    MPI_Recv(&normCoeff, 1, MPI_FLOAT, NumOfSendProc, MPI_ANY_TAG, 
                        MPI_COMM_WORLD, &status);
*/
                }// if moment
                delete [] Sizes;
            }// dtGaussian
        } // else
        if (dt == dtScalar)
        {
            UpdateRatio = CScalarPotential::Create(sepDomain, pMD, obsIndicesIn);
        }
        else
            if(dt == dtTabular)
            {
                UpdateRatio = CTabularPotential::Create(sepDomain, pMD, 
                    pDataForRecving, obsIndicesIn);
                delete [] (float*)pDataForRecving; 
            }
            else
                if( dt == dtGaussian)
                {
                    if (static_cast<CGaussianDistribFun *>(Pot->GetDistribFun())
                        ->GetCanonicalFormFlag())
                    {
                        if (flag == 1) // delta
                        {
                            UpdateRatio = CGaussianPotential::CreateDeltaFunction(&sepDomain.front(),
                                sepDomain.size(), pMD, dataMean, 1, obsIndicesIn);
                            static_cast<CGaussianDistribFun *>(UpdateRatio->GetDistribFun())->
                                UpdateCanonicalForm();
                            delete [] (float*)dataMean;
                        }
                        else
                        {
                            UpdateRatio = CGaussianPotential::Create(&sepDomain.front(), 
                                sepDomain.size(), pMD, 0, dataH, dataK, 0.0f, obsIndicesIn);
                            if (dataH != NULL)
                                delete [] (float*)dataH;
                            if (dataK != NULL)
                                delete [] (float*)dataK;
                        }
                    }
                    if (static_cast<CGaussianDistribFun *>(Pot->GetDistribFun())
                        ->GetMomentFormFlag())
                    {
                        UpdateRatio = 
                            CGaussianPotential::Create(&sepDomain.front(), sepDomain.size(), 
                            pMD, 1, dataMean, dataCov, 0.0f, obsIndicesIn);
                        
                        delete [] (float*)dataMean, (float*)dataCov;
                    }
                }
        CPotential *potSink = GetJTree()->GetNodePotential(m_Routes[NumOfRoute][0]);
        *potSink *= *UpdateRatio;
        potSink->Normalize();
        delete UpdateRatio;
    }
    else
    {
        // nothing
    }
    // we are found the route

    for ( i=0; i < m_NumberOfUsedProcesses - 1; i++)
    {
        if (m_Routes[i][(m_Routes[i].size()) - 1] == m_Roots[m_MyRank])
        {
            for ( j = (m_Routes[i].size()) - 1; j > 1; j-- )
            {
                PropagateBetweenClqs(m_Routes[i][j], m_Routes[i][j-1], false);
                int index = 0;
                while (m_NodesOfProcess[index] != m_Routes[i][j - 1])
                    index++;
                m_IsPropagated[index] = true;
            }
            
            potSep = GetJTree()->GetSeparatorPotential(m_Routes[i][1], m_Routes[i][0]);
            potClq = GetJTree()->GetNodePotential(m_Routes[i][1]);
            potSep->GetDomain(&numNdsInSepDom, &sepDom);
            newPotSep = potClq->Marginalize(sepDom, numNdsInSepDom, m_bMaximize);
            updateRatio = newPotSep->Divide(potSep);
            potSep->SetDistribFun(newPotSep->GetDistribFun());
            
            int size = updateRatio->GetDomainSize();
            int NumOfRecvProc=0;
            while (m_Roots[NumOfRecvProc] != m_Routes[i][0])
                NumOfRecvProc++;

            if (potClq->GetDistributionType() == dtTabular)
            {
                int dataLength;
                const float *pDataForSending;

                if (updateRatio->GetDistributionType() == dtScalar)
                {
                    dataLength = 0;
                    pDataForSending = NULL;
                    MPI_Send(&dataLength, 1, MPI_INT, NumOfRecvProc, m_MyRank,
                        MPI_COMM_WORLD);
                    
                    MPI_Send((float*)pDataForSending, dataLength, MPI_FLOAT, 
                        NumOfRecvProc, m_MyRank, MPI_COMM_WORLD);
                }
                
                if (updateRatio->GetDistributionType() == dtTabular)
                {
                    static_cast<CNumericDenseMatrix<float>*>((updateRatio->GetDistribFun())
                        ->GetMatrix(matTable))->GetRawData(&dataLength, &pDataForSending);
                   
                    MPI_Send(&dataLength, 1, MPI_INT, NumOfRecvProc, m_MyRank,
                        MPI_COMM_WORLD);
                    
                    MPI_Send((float*)pDataForSending, dataLength, MPI_FLOAT, 
                        NumOfRecvProc, m_MyRank, MPI_COMM_WORLD);
                }
            }
            else
            {
                if (potClq->GetDistributionType() == dtGaussian)
                {
                    const float *dataH, *dataK;
                    int sizeH, sizeK;
                    int flag;
                    
                    const float *dataMean, *dataCov;
                    int sizeMean, sizeCov;
//                    float normCoeff;
                    int *Sizes = new int [3];  
                   
                    if (updateRatio->GetDistributionType() == dtScalar)
                    {
                        dataH = dataK = NULL;

                        Sizes[0] = Sizes[1] = Sizes[2] = 0;

                        MPI_Send((int*)Sizes, 3, MPI_INT, NumOfRecvProc,
                            m_MyRank, MPI_COMM_WORLD);
                        
                        MPI_Send((float*)dataK, sizeK, MPI_FLOAT, NumOfRecvProc, 
                            m_MyRank, MPI_COMM_WORLD);
                    } // if dtScalar
                    else    // gaussian distribution in canonical form
                    if (updateRatio->GetDistribFun()->IsDistributionSpecific() == 1)
                    {
                        dataH = NULL;
                        dataK = NULL;

                        Sizes[0] = Sizes[1] = Sizes[2] = 0;

                        MPI_Send((int*)Sizes, 3, MPI_INT, NumOfRecvProc,
                            m_MyRank, MPI_COMM_WORLD);
                        
                        MPI_Send((float*)dataK, sizeK, MPI_FLOAT, NumOfRecvProc, 
                            m_MyRank, MPI_COMM_WORLD);
                    }
                    else
                        if (updateRatio->GetDistribFun()->IsDistributionSpecific() == 2 ) // delta
                        {
                            static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())
                                ->UpdateMomentForm();
                            int sizeMean;
                            const float *dataMean;
                            
                            static_cast<CDenseMatrix<float>*>(updateRatio->GetDistribFun()
                                ->GetMatrix(matMean))->GetRawData(&sizeMean, &dataMean);
                            
                            Sizes[0] = sizeMean;
                            Sizes[1] = 0;
                            Sizes[2] = 1;
                            
                            MPI_Send((int*)Sizes, 3, MPI_INT, NumOfRecvProc,
                                m_MyRank, MPI_COMM_WORLD);
                            
                            MPI_Send((float*)dataMean, sizeMean, MPI_FLOAT, NumOfRecvProc, 
                                m_MyRank, MPI_COMM_WORLD);
                            static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())->
                                UpdateCanonicalForm();

                        }
                        else
                        {

                        if (static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())->
                            GetCanonicalFormFlag())
                        {
                            static_cast<CDenseMatrix<float>*>(updateRatio->GetDistribFun()->
                                GetMatrix(matH))->GetRawData(&sizeH, &dataH);
                            
                            static_cast<CDenseMatrix<float>*>(updateRatio->GetDistribFun()->
                                GetMatrix(matK))->GetRawData(&sizeK, &dataK);
                            
/*                            g = static_cast<CGaussianDistribFun *>(updateRatio->
                                GetDistribFun())->GetCoefficient(1);
*/
                            Sizes[0] = sizeH;
                            Sizes[1] = sizeK;
                            Sizes[2] = 0;
                            int DataSize = sizeH + sizeK;                        
                            float *Data = new float [DataSize];
                                
                            memcpy(&Data[0], dataH, sizeH*sizeof(float));
                            memcpy(&Data[sizeH], dataK, sizeK*sizeof(float));

                            MPI_Send((int*)Sizes, 3, MPI_INT, NumOfRecvProc,
                                m_MyRank, MPI_COMM_WORLD);
                            
                            MPI_Send((float*)Data, DataSize, MPI_FLOAT, NumOfRecvProc, 
                                m_MyRank, MPI_COMM_WORLD);
                            delete [] Data;
                        }
                        else  // gaussian distribution in moment form
                            // branch is not tested
                        if (static_cast<CGaussianDistribFun *>(updateRatio->GetDistribFun())->
                            GetMomentFormFlag())
                        {
                            const float *dataMean, *dataCov;
                            int sizeMean, sizeCov;
//                            float normCoeff;

                            static_cast<CDenseMatrix<float>*>(updateRatio->GetMatrix(matMean))->
                                GetRawData(&sizeMean, &dataMean);

                            static_cast<CDenseMatrix<float>*>(updateRatio->GetMatrix(matCovariance))->
                                GetRawData(&sizeCov, &dataCov);

/*                            normCoeff = static_cast<CGaussianDistribFun *>(updateRatio->
                                GetDistribFun())->GetCoefficient(0);
*/
                            MPI_Send(&sizeMean, 1, MPI_INT, NumOfRecvProc, m_MyRank,
                                MPI_COMM_WORLD);

                            MPI_Send((float*)dataMean, sizeMean, MPI_FLOAT, NumOfRecvProc,
                                m_MyRank, MPI_COMM_WORLD);

                            MPI_Send(&sizeCov, 1, MPI_INT, NumOfRecvProc, m_MyRank,
                                MPI_COMM_WORLD);

                            MPI_Send((float*)dataCov, sizeCov, MPI_FLOAT, NumOfRecvProc, 
                                m_MyRank, MPI_COMM_WORLD);
                        }
                    }
                delete [] Sizes;
               }
            }
            delete newPotSep, updateRatio;
        } // end of if
    }// end of for
}

#endif // PAR_MPI
// ----------------------------------------------------------------------------

#ifdef PAR_MPI
void CParJtreeInfEngine::BalanceTree()
{
    if (m_NumberOfUsedProcesses == 1)
        return;
    
    int numOfNbrs;
    const int *nbrs;
    const ENeighborType *nbrsTypes;
    const CGraph *pGraph = GetOriginalJTree()->GetGraph();
    
    double WR, WR1, WR2; // weights of roots
    double PR, PR1, PR2; // base weights of roots
    double T1, T2; // complexity of computation in dependence of balance way
    double W1, W2; // weights of subtrees
    
    intVector Parents;
    GetParents(m_Roots[0], &Parents);
    
    PR = GetBaseWeightOfClq(GetJTreeRootNode());
    PR1 = GetBaseWeightOfClq(Parents[0]);
    PR2 = GetBaseWeightOfClq(m_Roots[0]);

    EDistributionType dt = GetJTreeType();

    if (dt == dtTabular)
    {
        WR =  GetWeightOfTabularClq(GetJTreeRootNode()) - 2*PR;
        WR1 = GetWeightOfTabularClq(Parents[0]) + 2 * PR1;
        WR2 = GetWeightOfTabularClq(m_Roots[0]) + 2 * PR2;
    }
    else
        if (dt == dtGaussian)
        {
            WR =  GetWeightOfContinuousClq(GetJTreeRootNode()) - 2*PR;
            WR1 = GetWeightOfContinuousClq(Parents[0]) + 2 * PR1;
            WR2 = GetWeightOfContinuousClq(m_Roots[0]) + 2 * PR2;
        }
        else // dtScalar
        {
            WR =  0;
            WR1 = 0;
            WR2 = 0;
        }

    pGraph->GetNeighbors(Parents[0], &numOfNbrs, &nbrs, &nbrsTypes);
    double Tau1 = WR1 / numOfNbrs; 
    
    pGraph->GetNeighbors(m_Roots[0], &numOfNbrs, &nbrs, &nbrsTypes);
    
    double Tau2 = WR2/numOfNbrs;
    
    double WeightOfHalfTree = (*m_pWeightesOfSubTrees)[m_Roots[0]];
    double WeightOfWholeTree = (*m_pWeightesOfSubTrees)[GetJTreeRootNode()];
    
    W1 = WeightOfWholeTree - WeightOfHalfTree - 2 * PR + 2 * PR1;
    W2 = WeightOfHalfTree + 2 * PR2;
    
    if (WeightOfHalfTree > W1 - Tau1)
        T1 = WeightOfHalfTree + Tau1;
    else
        T1 = W1;
    
    if (WeightOfWholeTree - WeightOfHalfTree - 2 * PR > W2 - Tau2)
        T2 = WeightOfWholeTree - WeightOfHalfTree - 2 * PR + Tau2;
    else
        T2 = W2;
    
    // move main root
    if (T2 < T1) 
        SetJTreeRootNode(m_Roots[0]);
    else 
        SetJTreeRootNode(Parents[0]);
    
    // reset neighbours types
    SetNbrsTypes();
    
    // reset array of roots of subtrees
    m_Roots[1] = Parents[0];
    BuildWeightesOfSubTrees();
}
#endif // PAR_MPI


EDistributionType CParJtreeInfEngine::GetJTreeType()
{
    EDistributionType dt = dtScalar;
    CJunctionTree *jtree = GetJTree();
    
    int i = 0; 
    int nNodes = jtree->GetNumberOfNodes();
    do
    {
        EDistributionType potDt = jtree->GetNodePotential(i)->GetDistributionType();
        switch (potDt)
        {
        case dtTabular:
            return dtTabular;
        case dtGaussian:
            return dtGaussian;
        case dtScalar:
            i++;
            break;
        default:
            {
                PNL_THROW( CInvalidOperation, "invalid potential" );
            }
        }
    } while (i < nNodes);
    return dt;
}
// ----------------------------------------------------------------------------
#ifdef PNL_RTTI
const CPNLType CParJtreeInfEngine::m_TypeInfo = CPNLType("CParJtreeInfEngine", &(CJtreeInfEngine::m_TypeInfo));

#endif

#endif // PAR_PNL

// end of file ----------------------------------------------------------------
