/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  File:      parPNLTest.cpp                                              //
//                                                                         //
//  Purpose:   Test for parPNL                                             //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "pnl_dll.hpp"

#include "CreateBNets.h"
#include "LoadBNet.h"

#if defined(PAR_OMP)
    #include <omp.h>
#elif defined(PAR_MPI)
    #if defined(WIN32)
	#ifdef _DEBUG
	    #pragma comment(lib, "mpichd")
	#else
	    #pragma comment(lib, "mpich")
	#endif
	#include "mpi.h"
    #endif
    #define MPICH_AVAILABLE
#endif

// ============================================================================
// macros creation

#define INTEL_PNL // let to use intel classes
//#define PAR_PNL   // already defined in pnlParConfig.hpp (don't uncomment)

#if defined(INTEL_PNL) && defined(PAR_PNL) && defined(PAR_RESULTS_RELIABILITY)
#define COMPARE_RESULTS
    // let compare results of original and parallel versions
#endif

//#define PRINT_RESULT // print marginal distribution
#define CHECK_TIME // use time check functions on not

#if defined(MPICH_AVAILABLE)
    #define MPI_PRINT(rank) if (myrank == rank)
#else
    #define MPI_PRINT(rank) if (1)
#endif

#define D_PRINT(step) printf("step %.1f\n", step); \

#ifdef CHECK_TIME
    #ifdef MPICH_AVAILABLE
	#define TIME_S(s) s = MPI_Wtime();
	#define TIME_E(s,f,d)				\
	    f = MPI_Wtime();				\
	    d = f - s;
    #else
	#include <time.h>
	#define TIME_S(s) s = clock();
	#define TIME_E(s,f,d) 				\
	    f = clock();				\
	    d = (f - s) / (double)(CLOCKS_PER_SEC);
    #endif
#else
    #define TIME_S(s)
    #define TIME_E(s,f,d)
#endif

// end of macros creation
// ============================================================================

using namespace pnl;

//---------------------------------------------------------------------------
void GenerateEvidence(CStaticGraphicalModel *pBNet, int NumOfEvidence)
{
  int i;
  int numberOfNodes = pBNet->GetNumberOfNodes();
  const CNodeType **nodeTypes = new const CNodeType* [numberOfNodes];
  
  int *NumOfNodeVal = new int [numberOfNodes];
  for (i = 0; i < numberOfNodes; i++)
  {
    nodeTypes[i] = pBNet->GetModelDomain()->GetVariableType(i);
    NumOfNodeVal[i] = nodeTypes[i]->GetNodeSize();
  }
  
  FILE *stream;
  float *Evidence = new float [numberOfNodes];
  srand((unsigned)time(NULL));
  if ((stream = fopen("Evidence.txt", "w")) != NULL)
  {
    for (i = 0; i < NumOfEvidence; i++)
    {
      for (int j = 0; j < numberOfNodes; j++)
      {
        if (nodeTypes[j]->IsDiscrete())
          Evidence[j] = (float)(rand() % NumOfNodeVal[j]);
        else
          Evidence[j] = (rand() / (float)RAND_MAX) * 10.0f;
      }
      
      for (int k = 0; k < numberOfNodes; k++)
        fprintf(stream, "%.6f ", Evidence[k]);
      fprintf(stream, "\n");
    }
    fclose(stream);
  }
  else
  {
    printf("\nCan't open file");
  }
}
//---------------------------------------------------------------------------

// ============================================================================
// BNet Asia defenition
CBNet* CreateAsiaBNet(void)
{
  CBNet *pBNet;
  int i;
  const int nnodes = 8;
  const int numberOfNodeTypes = 1;

  int numOfNeigh[] = { 2, 1, 2, 2, 2, 4, 2, 1 };

  int neigh0[] = { 2, 3 };
  int neigh1[] = { 4 };
  int neigh2[] = { 0, 6 };
  int neigh3[] = { 0, 5 };
  int neigh4[] = { 1, 5 };
  int neigh5[] = { 3, 4, 6, 7 };
  int neigh6[] = { 2, 5 };
  int neigh7[] = { 5 };

  ENeighborType orient0[] = { ntChild, ntChild };
  ENeighborType orient1[] = { ntChild };
  ENeighborType orient2[] = { ntParent, ntChild };
  ENeighborType orient3[] = { ntParent, ntChild };
  ENeighborType orient4[] = { ntParent, ntChild };
  ENeighborType orient5[] = { ntParent, ntParent, ntChild, ntChild };
  ENeighborType orient6[] = { ntParent, ntParent };
  ENeighborType orient7[] = { ntParent };

  int *neigh[] = { neigh0,  neigh1, neigh2, neigh3, neigh4, neigh5, neigh6, neigh7 };
  
  ENeighborType *orient[] = { orient0, orient1, orient2, orient3, orient4, orient5, orient6, orient7 };

  CGraph *pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );

  CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];
  nodeTypes[0].SetType(1, 2);

  int *nodeAssociation = new int[nnodes];

  for (i = 0; i < nnodes; i++)
    nodeAssociation[i] = 0;
  
  pBNet = CBNet::Create(nnodes, numberOfNodeTypes, nodeTypes, nodeAssociation, pGraph);
  
  CModelDomain* pMD = pBNet->GetModelDomain();
  
//number of parameters is the same as number of nodes - one CPD per node
  CFactor **myParams = new CFactor*[nnodes];
  int *nodeNumbers = new int [nnodes];
  int domain0[] = { 0 };
  int domain1[] = { 1 };
  int domain2[] = { 0, 2 };
  int domain3[] = { 0, 3 };
  int domain4[] = { 1, 4 };
  int domain5[] = { 3, 4, 5 };
  int domain6[] = { 2, 5, 6 };
  int domain7[] = { 5, 7 };
  int *domains[] = { domain0, domain1, domain2, domain3, domain4, domain5, domain6, domain7 };
  nodeNumbers[0] = 1;
  nodeNumbers[1] = 1;
  nodeNumbers[2] = 2;
  nodeNumbers[3] = 2;
  nodeNumbers[4] = 2;
  nodeNumbers[5] = 3;
  nodeNumbers[6] = 3;
  nodeNumbers[7] = 2;
  
  pBNet->AllocParameters();
  
  for (i = 0; i < nnodes; i++)
    myParams[i] = CTabularCPD::Create( domains[i], nodeNumbers[i], pMD );
  
// data creation for all CPDs of the model
  float data0[] = { 0.5f, 0.5f };
  float data1[] = { 0.99f, 0.01f };
  float data2[] = { 0.7f, 0.3f, 0.4f, 0.6f };
  float data3[] = { 0.99f, 0.01f, 0.9f, 0.1f };
  float data4[] = { 0.99f, 0.01f, 0.95f, 0.05f };
  float data5[] = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f };
  float data6[] = { 0.9f, 0.1f, 0.3f, 0.7f, 0.2f, 0.8f, 0.1f, 0.9f };
  float data7[] = { 0.95f, 0.05f, 0.02f, 0.98f };
  float *data[] = { data0, data1, data2, data3, data4, data5, data6, data7 };
  
  for (i = 0; i < nnodes; i++)
  {
    myParams[i]->AllocMatrix(data[i], matTable);
    pBNet->AttachParameter(myParams[i]);
  }
  
  delete [] nodeTypes;
  
  return pBNet;
}
// end of BNet Asia defenition
// ============================================================================

// ============================================================================
int myrank, numproc;
int TestNum;
double IntelTime[10000], ParTime[10000];

int RunTest(int argc, char* argv[])
{
  CBNet *pBNet = NULL, *pBNet1 = NULL;
  CInfEngine *pInfEng = NULL, *pParInfEng = NULL;
  CEMLearningEngine *pLearnEng = NULL, *pParLearnEng = NULL;
  vector<CEvidence*> evidences;

#ifdef MPICH_AVAILABLE
  double start, finish, duration;
#else
  clock_t start, finish;
  double duration;
#endif

  int i;

  // Parametrs to create random model:
  int nnodes = 30;        
  // common number of nodes
  int num_indep_nodes = 3;
  // number of nodes that hanen't got any parents
  int max_family_size = 4;
  // maximum of family size
  int max_number_of_states = 4;
  // upper limit of number of states
  long num_edges;
  // number of edges
  
// - BNet creation ------------------------------------------------------------
  if (argc <= 1)
  {
    printf("\nUsage: .exe [random|Asia|fname] [JTree|Pearl|Gibbs|EMLearn] {num of iteration}\n");
    return 1;
  }
  if (strcmp(argv[1],"Asia") == 0)
    pBNet = CreateAsiaBNet();
  else
  if (strcmp(argv[1],"random") == 0)
    pBNet = Create_BNet_Default(nnodes, max_number_of_states, num_indep_nodes,
      max_family_size, num_edges);
  else
    pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML(argv[1], &evidences));
  if (pBNet == NULL)
  {
    printf("\n!!!Unexpected error!!!\n");
    return 1;
  }
  if (strcmp(argv[2],"EMLearn") == 0)
  {
    if (strcmp(argv[1],"Asia") == 0)
      pBNet1 = CreateAsiaBNet();
    else
    if (strcmp(argv[1],"random") == 0)
      pBNet1 = Create_BNet_Default(nnodes, max_number_of_states, num_indep_nodes,
        max_family_size, num_edges);
    else
      pBNet1 = dynamic_cast<CBNet*>(LoadGrModelFromXML(argv[1], &evidences));
    if (pBNet1 == NULL)
    {
      printf("\n!!!Unexpected error!!!\n");
      return 1;
    }
  }

// - Choosen inference engine creation ----------------------------------------
  if (strcmp(argv[2],"JTree") == 0)
  {
    #if defined(INTEL_PNL)
      pInfEng = CJtreeInfEngine::Create(pBNet);
    #endif
    #if defined(PAR_PNL)
      pParInfEng = CParJtreeInfEngine::Create(pBNet);
    #endif
  }
  else
  if (strcmp(argv[2],"Pearl") == 0)
  {
    #if defined(INTEL_PNL)
      pInfEng = CPearlInfEngine::Create(pBNet);
    #endif
    #if defined(PAR_PNL)
      pParInfEng = CParPearlInfEngine::Create(pBNet);
    #endif
  }
  else
  if (strcmp(argv[2],"Gibbs") == 0)
  {
    #if defined(INTEL_PNL)
      pInfEng = CGibbsSamplingInfEngine::Create(pBNet);
    #endif
    #if defined(PAR_PNL)
      pParInfEng = CParGibbsSamplingInfEngine::Create(pBNet);
    #endif
  }
  else
  if (strcmp(argv[2],"EMLearn") == 0)
  {
    #if defined(INTEL_PNL)
      pLearnEng = CEMLearningEngine::Create(pBNet);
    #endif
    #if defined(PAR_PNL)
      pParLearnEng = CParEMLearningEngine::Create(pBNet1);
    #endif
  }
  else
  {
    printf("\nUsage: .exe [random|Asia|fname] [JTree|Pearl|Gibbs|EMLearn] {num of iteration}\n");
    return 1;
  }
  MPI_PRINT(0) printf("\n=========================");
  MPI_PRINT(0) printf("\n  Test name:   %s", argv[1]);
  MPI_PRINT(0) printf("\n  Engine:      %s", argv[2]);
  MPI_PRINT(0) printf("\n=========================\n\n");

// - Evidence creation --------------------------------------------------------
// const int numOfObsNds  = 2;
  CEvidence *pEvidence = NULL;
  intVecVector queries;

  if (strcmp(argv[2],"EMLearn") == 0)
  {
    const int numberOfEvidences = 1000;
    
    GenerateEvidence(pBNet, numberOfEvidences);

    int numberOfNodes = pBNet->GetNumberOfNodes();
    pnlVector<valueVector> EvidenceData(numberOfEvidences);
    for(i = 0; i < numberOfEvidences; i++)
    {
      EvidenceData[i].resize(numberOfNodes);
    }
    
    FILE *fp;
    
    char* fileWithData = "Evidence.txt";
    int j;
    if((fp = fopen(fileWithData, "r")) == NULL)
    {
      printf("can't open file with evidences\n");
      return 1;
    }
    else
    {
      float val;
      for (i = 0; i < numberOfEvidences; ++i)
      {
        for (j =0; j < numberOfNodes; ++j)
        {
          fscanf(fp, "%f ", &val);
          if (pBNet->GetNodeType(j)->IsDiscrete())
            (EvidenceData[i])[j].SetInt((int)val);
          else
            (EvidenceData[i])[j].SetFlt(val);
        }
        fscanf(fp, "\n");
      }
    }
    fclose(fp);
    
    int numOfObsNds = pBNet->GetNumberOfNodes();
    int *obsNodes = new int [numOfObsNds];
    for (i = 0; i < numOfObsNds; i++)
      obsNodes[i] = i;
    int offset = numOfObsNds - 2;
    int *unObsNds = new int [numOfObsNds - offset];

    intVector nodes(numOfObsNds, 0);
    for (i = 0; i < numOfObsNds; ++i)
      nodes[i] = i;

    for (i = 0; i < numOfObsNds - offset; ++i)
    {
      int num = rand() % nodes.size();
      unObsNds[i] = nodes[num];
      nodes.erase(nodes.begin() + num);
    }
/*
    for (i = 0;i < numOfObsNds - offset;i++)
      unObsNds[i] = i + offset;
*/    
    CEvidence **m_pEv, **m_pEv1;
    m_pEv = new CEvidence *[numberOfEvidences];
    m_pEv1 = new CEvidence *[numberOfEvidences];
    
    for (i = 0; i < numberOfEvidences; i++)
    {
      m_pEv[i] = CEvidence::Create(pBNet, numberOfNodes, obsNodes,
        EvidenceData[i]);
      m_pEv[i] ->ToggleNodeState(numOfObsNds - offset, unObsNds);
      m_pEv1[i] = CEvidence::Create(pBNet1, numberOfNodes, obsNodes, 
        EvidenceData[i]);
      m_pEv1[i] ->ToggleNodeState(numOfObsNds - offset, unObsNds);
    }
    #ifdef INTEL_PNL
      pLearnEng->SetData(numberOfEvidences, m_pEv);
      pLearnEng->SetMaxIterEM(numberOfEvidences);
    #endif
    #ifdef PAR_PNL
      pParLearnEng->SetData(numberOfEvidences, m_pEv1);
      pParLearnEng->SetMaxIterEM(numberOfEvidences);
    #endif
  }
  else
  if (strcmp(argv[2],"Gibbs") == 0)
  {
    if (evidences.empty())
    {
      pEvidencesVector Evidences;

      pBNet->GenerateSamples(&Evidences, 1);
      pEvidence = Evidences[0];
      const int ndsToToggle[] = { 1, 2, 5, 7 };
      pEvidence->ToggleNodeState(4, ndsToToggle);
    }
    else
    {
      pEvidence = evidences.front();
    }
    
    queries.resize(2);
    queries[0].resize(0);
    queries[0].push_back(2);
    //queries[0].push_back(3);
    //queries[0].push_back(4);
    //queries[0].push_back(5);
    //queries[0].push_back(7);
    
    queries[1].resize(0);
    queries[1].push_back(1);
    //queries[1].push_back(2);
  }
  else
  {
    if (evidences.empty())
    {
      const int numOfObsNds = 2;
      const int obsNds[] = { 0, 3 };
      valueVector obsNdsVals(2);
      obsNdsVals[0].SetInt(1);
      obsNdsVals[1].SetInt(0);
      pEvidence = CEvidence::Create(pBNet, numOfObsNds, obsNds, obsNdsVals);
    }
    else
    {
      pEvidence = evidences.front();
    }
  }

  int gibbs_max_time = 1000;
  int gibbs_burnin = 100;
  int gibbs_num_streams = 8; // more then number of MPI process
// Run choosen inference engine 
// - INTEL section ------------------------------------------------------------
  #ifdef INTEL_PNL
    MPI_PRINT(0) printf("Computation (INTEL) ... ");

    if (strcmp(argv[2],"Gibbs") == 0)
    {
      CGibbsSamplingInfEngine* pGibbsInfEng = 
        (CGibbsSamplingInfEngine*)pInfEng;
      pGibbsInfEng->SetQueries(queries);
      pGibbsInfEng->SetMaxTime(gibbs_max_time);
      pGibbsInfEng->SetBurnIn(gibbs_burnin);
      pGibbsInfEng->SetNumStreams(gibbs_num_streams);
    }

    TIME_S(start)
    
    if (strcmp(argv[2],"EMLearn") == 0)
    {
      pLearnEng->Learn();
    }
    else
    {
      pInfEng->EnterEvidence(pEvidence);
    }
    TIME_E(start, finish, duration)
    IntelTime[TestNum] = duration;
    MPI_PRINT(0) printf("done\n");
  #endif //INTEL_PNL

  #ifdef PAR_MPI
    gibbs_num_streams /= numproc;
  #endif

// - PAR PNL section ----------------------------------------------------------
  #ifdef PAR_PNL
    MPI_PRINT(0) printf("Computation (PAR PNL) ... ");

    if (strcmp(argv[2],"Gibbs") == 0)
    {
      CGibbsSamplingInfEngine* pGibbsInfEng = 
        (CGibbsSamplingInfEngine*)pParInfEng;
      pGibbsInfEng->SetQueries(queries);
      pGibbsInfEng->SetMaxTime(gibbs_max_time);
      pGibbsInfEng->SetBurnIn(gibbs_burnin);
      pGibbsInfEng->SetNumStreams(gibbs_num_streams);
    }

    if (strcmp(argv[2],"JTree") == 0)
    {
      TIME_S(start)
      #ifdef PAR_OMP
        static_cast<CParJtreeInfEngine*>(pParInfEng)->EnterEvidenceOMP(pEvidence);
      #else // we aren't need starting of both versions
        static_cast<CParJtreeInfEngine*>(pParInfEng)->EnterEvidence(pEvidence);
      #endif
      TIME_E(start, finish, duration)
    }
    else
    if (strcmp(argv[2],"Pearl") == 0)
    {
      TIME_S(start)
      #ifdef PAR_OMP
        static_cast<CParPearlInfEngine*>(pParInfEng)->EnterEvidenceOMP(pEvidence);
      #else // we aren't need starting of both versions
        static_cast<CParPearlInfEngine*>(pParInfEng)->EnterEvidence(pEvidence);
      #endif
      TIME_E(start, finish, duration)
    }
    else
    if (strcmp(argv[2],"Gibbs") == 0)
    {
      TIME_S(start)
        // ParGibbs has got one method for MPI and(or) OMP versions
        static_cast<CParGibbsSamplingInfEngine*>(pParInfEng)->EnterEvidence(pEvidence);
      TIME_E(start, finish, duration)
    }
    else
    if (strcmp(argv[2],"EMLearn") == 0)
    {
      TIME_S(start)
      #ifdef PAR_OMP
        static_cast<CParEMLearningEngine*>(pParLearnEng)->LearnOMP();
      #else // we aren't need starting of both versions
        static_cast<CParEMLearningEngine*>(pParLearnEng)->Learn();
        //static_cast<CParEMLearningEngine*>(pParLearnEng)->LearnContMPI();
      #endif
      TIME_E(start, finish, duration)
    }
    ParTime[TestNum] = duration;
    MPI_PRINT(0) printf("done\n");
  #endif //PAR_PNL

// - comparing result section -------------------------------------------------
  #ifdef COMPARE_RESULTS
    MPI_PRINT(0)
    {
      if (strcmp(argv[2],"Pearl") == 0)
      {
          int flag = pnl::EqualResults(
            dynamic_cast<CPearlInfEngine&>(*pInfEng), 
            dynamic_cast<CPearlInfEngine&>(*pParInfEng), 0.0001f);
        if (flag)
          printf("Beliefs comparing:  OK\n");
        else
          printf("Beliefs comparing:  not OK\n");
      }

      if (strcmp(argv[2],"JTree") == 0)
      {
        int flag = pnl::EqualResults(
            dynamic_cast<CJtreeInfEngine&>(*pInfEng),
            dynamic_cast<CJtreeInfEngine&>(*pParInfEng), 0.0001f);
        if (flag)
          printf("Factors comparing:  OK\n");
        else
          printf("Factors comparing:  not OK\n");
      }

      if (strcmp(argv[2],"EMLearn") == 0)
      {
          int flag = pnl::EqualResults(
              dynamic_cast<CEMLearningEngine&>(*pLearnEng),
              dynamic_cast<CEMLearningEngine&>(*pParLearnEng), 0.001f);
        if (flag)
          printf("Factors comparing:  OK\n");
        else
          printf("Factors comparing:  not OK\n");
      }

    }
  #endif // COMPARE_RESULTS
// - print result section -----------------------------------------------------
  #ifdef PRINT_RESULT
    int numOfNdsTmp;
  
    #ifdef INTEL_PNL
      MPI_PRINT(0)
      {
        if (strcmp(argv[2],"EMLearn") == 0)
        {
          printf("\nINTEL %s learn engine results\n====================================\n\n", argv[2]);
          numOfNdsTmp = pBNet->GetNumberOfNodes();
          int length = 0; 
          const float *output;
          CNumericDenseMatrix<float> *pMatrix;
          for (i = 0; i < numOfNdsTmp; i++)
          {
            pMatrix = static_cast<CNumericDenseMatrix<float>*>(
              pBNet->GetFactor(i)->GetMatrix(matTable));
            pMatrix->GetRawData(&length, &output);
            for (int j = 0; j < length; j++)
            {
              printf("(%.5f) ",output[j]);
            }
            printf("\n");
          }
        }
        else
        {
          printf("\nINTEL %s engine results\n====================================\n\n", argv[2]);
          if (strcmp(argv[2],"Gibbs") == 0)
          {
            const int querySz = 1;
            const int query[] = {2};
            pInfEng->MarginalNodes(query, querySz);
            pInfEng->GetQueryJPD()->Dump();
          }
          else
          {
            for (i = 0, numOfNdsTmp = pBNet->GetNumberOfNodes(); 
                 i < numOfNdsTmp; ++i)
            {
              pInfEng->MarginalNodes(&i, 1);
              pInfEng->GetQueryJPD()->Dump();
            }
          }
        }
      }
    #endif
  
    #ifdef PAR_PNL
      if (strcmp(argv[2],"EMLearn") == 0)
      {
        MPI_PRINT(0)
        {
          printf("\nPAR_PNL %s learn engine results\n====================================\n\n", argv[2]);
          numOfNdsTmp = pBNet1->GetNumberOfNodes();
          int length = 0; 
          const float *output;
          CNumericDenseMatrix<float> *pMatrix;
          for (i = 0; i < numOfNdsTmp; i++)
          {
            pMatrix = static_cast<CNumericDenseMatrix<float>*>(
              pBNet1->GetFactor(i)->GetMatrix(matTable));
            pMatrix->GetRawData(&length, &output);
            for (int j = 0; j < length; j++)
            {
              printf("(%.5f) ", output[j]);
            }
            printf("\n");
          }
        }
      }
      else
      {
        MPI_PRINT(0) 
          printf("\nPAR_PNL %s engine results\n====================================\n\n", argv[2]);
        if (strcmp(argv[2],"Gibbs") == 0)
        {
          const int querySz = 1;
          const int query[] = {2};
          pParInfEng->MarginalNodes(query, querySz);
          MPI_PRINT(0) pParInfEng->GetQueryJPD()->Dump();
        }
        else
        {
          MPI_PRINT(0)
          for (i = 0, numOfNdsTmp = pBNet->GetNumberOfNodes();
               i < numOfNdsTmp; ++i)
          {
            pParInfEng->MarginalNodes(&i, 1);
            pParInfEng->GetQueryJPD()->Dump();
          }
        }
      }
    #endif
  #endif //PRINT_RESULT
      
  delete pEvidence;
  delete pInfEng;
  delete pParInfEng;
  delete pLearnEng;
  delete pParLearnEng;
  delete pBNet;
  delete pBNet1;
  
  return 0;
}
// ============================================================================

// ============================================================================
int main(int argc, char* argv[])
{
  int Count = 1;
  int res;
  
  double AvgIntelTime, AvgParTime;
  
  if (argc == 4)
    Count = atoi(argv[3]);
  
#if defined(MPICH_AVAILABLE)
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &numproc);
#endif
  
  for (TestNum = 0; TestNum < Count; TestNum++)
  {
    res = RunTest(argc, argv);
    if (res) return 1;
    MPI_PRINT(0) printf("\nTest number %d has been completed correctly\n******************************************\n", TestNum);
  }
  
  AvgIntelTime = 0;
  AvgParTime = 0;
  
  for (TestNum = 0; TestNum < Count; TestNum++)
  {
    AvgIntelTime += IntelTime[TestNum];
    AvgParTime += ParTime[TestNum];
  }
  
  AvgIntelTime /= Count;
  AvgParTime /= Count;
  MPI_PRINT(0) printf("\nAverage time of execution\n=========================\nIntel:  %.6f   --   ParPNL:  %.6f\n", AvgIntelTime, AvgParTime);

#if defined(MPICH_AVAILABLE)  
  MPI_Finalize();
#endif

  return 0;
}
// end of file ----------------------------------------------------------------
