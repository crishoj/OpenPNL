#include "trsapi.h"
#include "pnl_dll.hpp"
#include <float.h>
#include <assert.h>
#include "pnlLIMIDInfEngine.hpp"

using namespace pnl;
// this file must be last included file
#include "tUtil.h"
PNL_USING
static char func_name[] = "test1_LIMIDInfEngine";

static char* test_desc = "Provide First Test of LIMID";

static char* test_class = "Algorithm";
//=============================================================================
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
//-----------------------------------------------------------------------------
int test1_LIMIDInfEngine()
{
	int ret = TRS_OK;
	double time1;
	CIDNet* pPigs;
    trsTimerStart(0);
    pPigs = CreatePigsLIMID();
	time1 = trsTimerClock(0);
    std::cout<<"timing of CIDNet graph intialization "
		<<time1<<std::endl; 
	
	trsTimerStart(0);
	
	CLIMIDInfEngine *pInfEng = NULL;
	pInfEng = CLIMIDInfEngine::Create(pPigs);
	
	time1 = trsTimerClock(0);
    std::cout<<"timing of LIMID graph intialization "
		<<time1<<std::endl; 
	
	trsTimerStart(0);
	
	pInfEng->DoInference();
	
	time1 = trsTimerClock(0);
    std::cout<<"timing of DoInferense function "
		<<time1<<std::endl; 
    pFactorVector *Vec = pInfEng->GetPolitics();
    printf("\n=====================\nPolitics are:\n");
    for (int i = 0; i < Vec->size(); i++)
    {
      (*Vec)[i]->GetDistribFun()->Dump();
    }
    float res = pInfEng->GetExpectation();
    printf("\nNumber of iterations is %d", pInfEng->GetIterNum());
    printf("\nExpectation is %.3f", res);
	trsTimerStart(0);
	
	CLIMIDInfEngine::Release(&pInfEng);
	
	time1 = trsTimerClock(0);
    std::cout<<"\ntiming of graph destruction "
		<<time1<<std::endl; 
	
	delete pPigs;
	return trsResult( ret, ret == TRS_OK ? "No errors" : "Graph FAILED");
	
}
//===============================================================================
void initA_LIMIDinfEngine()
{
    trsReg(func_name, test_desc, test_class, test1_LIMIDInfEngine);
}