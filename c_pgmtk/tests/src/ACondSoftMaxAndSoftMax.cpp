/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ACondSoftMaxAndSoftMax.cpp                                  //
//                                                                         //
//  Purpose:   Test on  Junction tree inference in the net contains        //
//             conditional softmax and softmax nodes.                      // 
//             Test on Learning in the net contains                        //
//             conditional softmax and softmax nodes                       //
//                                                                         //
//  Author(s):  (in alphabetical order)                                    //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Vinogradov                                         //
//             NNSU                                                        //             
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include "pnlSoftMaxCPD.hpp"
#include "pnlSoftMaxDistribFun.hpp"


PNL_USING

static char func_name[] = "testConditionalSoftMaxInfAndLearn";

static char* test_desc = "Provide inference and learning for net contains SoftMax nodes";

static char* test_class = "Algorithm";
//-----------------------------------------------------------------------------
CBNet* CreateSevenNodesNetForTests()
{

  // 0  1   2 
  // |\ \  / 
  // | \ \ /
  // |    3 
  // |   / \ 
  // |  4   5    
  // |/
  // 6
  // 0, 1, 5 - continuous nodes
  // 3, 6 - conditional Softmax nodes
  // 2, 4 - discrete nodes

  const int numOfNds = 7;
  int numOfNbrs[numOfNds] = { 2, 1, 1, 5, 2, 1, 2 };

  int nbrs0[] = { 3, 6 };
  int nbrs1[] = { 3 };
  int nbrs2[] = { 3 };
  int nbrs3[] = { 0, 1, 2, 4, 5 };
  int nbrs4[] = { 3, 6 };
  int nbrs5[] = { 3 };
  int nbrs6[] = { 0, 4 };

  ENeighborType nbrsTypes0[] = { ntChild, ntChild };
  ENeighborType nbrsTypes1[] = { ntChild };
  ENeighborType nbrsTypes2[] = { ntChild };
  ENeighborType nbrsTypes3[] = { ntParent, ntParent, ntParent, ntChild, ntChild };
  ENeighborType nbrsTypes4[] = { ntParent, ntChild };
  ENeighborType nbrsTypes5[] = { ntParent };
  ENeighborType nbrsTypes6[] = { ntParent, ntParent };

  int *nbrs[] = { nbrs0, nbrs1, nbrs2, nbrs3, nbrs4, nbrs5, nbrs6 };
  ENeighborType *nbrsTypes[] = { nbrsTypes0, nbrsTypes1, nbrsTypes2, nbrsTypes3, nbrsTypes4,
  nbrsTypes5, nbrsTypes6 };

  CGraph* pGraph = CGraph::Create( numOfNds, numOfNbrs, nbrs, nbrsTypes );
    
    // 2) Creation of the Model Domain.
  CModelDomain* pMD;

  nodeTypeVector variableTypes;
    
  int nVariableTypes = 4;
  variableTypes.resize( nVariableTypes );
   
  variableTypes[0].SetType( 0, 1 ); // continuous node
  variableTypes[1].SetType( 1, 2 ); // discrete node, node size is 2
  
  variableTypes[2].SetType( 1, 3 ); // discrete node, node size is 3
  variableTypes[3].SetType( 1, 4 ); // discrete node, node size is 4
  
  intVector variableAssociation;  
  int nnodes = pGraph->GetNumberOfNodes();
  variableAssociation.assign(nnodes, 1);
  variableAssociation[0] = 0;
  variableAssociation[1] = 0;
  variableAssociation[2] = 2;
  variableAssociation[3] = 1;
  variableAssociation[4] = 2;
  variableAssociation[5] = 0;
  variableAssociation[6] = 1;

  pMD = CModelDomain::Create( variableTypes, variableAssociation );
    
  // 2) Creation base for BNet using Graph, and Model Domain
    
  CBNet *pBNet = CBNet::Create(pGraph, pMD);
    
  // 3)Allocation space for all factors of the model
  pBNet->AllocFactors();

  int nnodes0 = 1;
  int domain0[] = { 0 };
  float mean0 = 0.0f;
  float cov0 = 1.0f;
  CGaussianCPD *pCPD0 = CGaussianCPD::Create( domain0, nnodes0, pMD );
  pCPD0->AllocDistribution( &mean0, &cov0, 1.0f, NULL );
  pBNet->AttachFactor( pCPD0 );
  
  int nnodes1 = 1;
  int domain1[] = { 1 };
  float mean1 = 0.0f;
  float cov1 = 1.0f;
  CGaussianCPD *pCPD1 = CGaussianCPD::Create( domain1, nnodes1, pMD );
  pCPD1->AllocDistribution( &mean1, &cov1, 1.0f, NULL );
  pBNet->AttachFactor( pCPD1 );

  int nnodes2 = 1;
  int domain2[] = { 2 };
  float table2[] = { 0.3f, 0.4f, 0.3f};
  CTabularCPD *pCPD2 = CTabularCPD::Create( domain2, nnodes2, pMD, table2 );
  pCPD2->AllocMatrix(table2, matTable);
  pBNet->AttachParameter(pCPD2);

  int nnodes3 = 4;
  int domain3[] = { 0, 1, 2, 3 };
  CSoftMaxCPD *pCPD3 = CSoftMaxCPD::Create( domain3, nnodes3, pMD );

  int parInd30[] = { 0 };
  float weight30[] = { 0.5f, 1.5f, 0.5f, 0.7f, 1.0f, -0.5f };
  float offset30[] = { 0.3f, 0.5f, 0.7f};

  pCPD3->AllocDistribution( weight30, offset30, parInd30 );
  
  int parInd31[] = { 1 };
  float weight31[] = { 0.5f, 1.5f, 0.5f, 0.7f, 0.5f, 0.7f};
  float offset31[] = { 0.3f, 0.5f, 1.0f };

  pCPD3->AllocDistribution( weight31, offset31, parInd31 );

  int parInd32[] = { 2 };
  float weight32[] = { 0.5f, 1.5f, 0.5f, 0.7f, 0.5f, 0.7f };
  float offset32[] = { 0.3f, 0.5f, 1.0f };

  pCPD3->AllocDistribution( weight32, offset32, parInd32 );

  pBNet->AttachFactor( pCPD3 );

  int nnodes4 = 2;
  int domain4[] = { 3, 4 };
  float table4[] = { 0.3f, 0.4f, 0.3f, 0.5f, 0.1f, 0.4f };

  CTabularCPD *pCPD4 = CTabularCPD::Create( domain4, nnodes4, pMD, table4 );
  pCPD4->AllocMatrix(table4, matTable);
  pBNet->AttachParameter(pCPD4);

  int nnodes5 = 2;
  int domain5[] = { 3, 5 };
  CGaussianCPD *pCPD5 = CGaussianCPD::Create( domain5, nnodes5, pMD );
 
  float mean50 = 0.0f;
  float cov50 = 1.0f;
  int parInd50[] = { 0 };
  pCPD5->AllocDistribution( &mean50, &cov50, 1.0f, NULL, parInd50 );

  float mean51 = 0.5f;
  float cov51 = 0.5f;
  int parInd51[] = { 1 };
  pCPD5->AllocDistribution( &mean51, &cov51, 1.0f, NULL, parInd51 );
/* 
  float mean52 = 0.0f;
  float cov52 = 1.f;
  int parInd52[] = { 2 };
  pCPD5->AllocDistribution( &mean52, &cov52, 1.0f, NULL, parInd52 );

  float mean53 = 0.3f;
  float cov53 = 1.f;
  int parInd53[] = { 3 };
  pCPD5->AllocDistribution( &mean53, &cov53, 1.0f, NULL, parInd53 );
*/
  pBNet->AttachFactor(pCPD5);

  int nnodes6 = 3;
  int domain6[] = { 0, 4, 6 };
  CSoftMaxCPD *pCPD6 = CSoftMaxCPD::Create( domain6, nnodes6, pMD );
  int parInd60[] = { 0 };
  
  float weight60[] = { 0.5f, 1.5f, 3.2f };
  float offset60[] = { 0.7f, 0.3f, 0.1f };
  
  pCPD6->AllocDistribution( weight60, offset60, parInd60 );
  
  int parInd61[] = { 1 };

  float weight61[] = { 0.8f, 0.2f, 0.7f };
  float offset61[] = { 0.1f, 0.9f, 1.7f };

  pCPD6->AllocDistribution( weight61, offset61, parInd61 );

  int parInd62[] = { 2 };

  float weight62[] = { 0.8f, 0.2f, 0.7f };
  float offset62[] = { 0.1f, 0.9f, 1.7f };

  pCPD6->AllocDistribution( weight62, offset62, parInd62 );

  pBNet->AttachFactor( pCPD6 );

  return pBNet;
}
//-------------------------------------------------------------------------------

void GenerateEvidence(CStaticGraphicalModel *pBNet, float StartVal,
  float FinishVal, valueVector &vls, int nObsNds, int* obsNds)
{
  EDistributionType dt;
  int i;

  const CNodeType *nodeTypes;// = new CNodeType[numberOfNodes];

  int *NumOfNodeVal = new int[nObsNds];
  for (i = 0; i < nObsNds; i++)
  {
    nodeTypes = pBNet->GetNodeType(i);
    NumOfNodeVal[i] = nodeTypes->GetNodeSize();
  }

  vls.resize(nObsNds);
  for (i = 0; i < nObsNds; i++)
  {
    dt = pBNet->GetFactor(obsNds[i])->GetDistributionType();
    if ((dt == dtSoftMax) || (dt == dtTabular))
    {
      int valInt = rand() % NumOfNodeVal[i];
#ifdef SM_TEST
  printf("%3d", valInt);
#endif

      (vls)[i].SetInt(valInt);
    }
    if (dt == dtGaussian)
    {
      float valFl = pnlRand(StartVal, FinishVal);
#ifdef SM_TEST
  printf("%f\t", valFl);
#endif
      (vls)[i].SetFlt(valFl);
    }
  }
#ifdef SM_TEST
  printf("\n");
#endif
  delete [] NumOfNodeVal;

}
//------------------------------------------------------------------------------
void GenerateSoftMaxEvidence(CStaticGraphicalModel *pBNet, float StartVal,
  float FinishVal, valueVector &vls)
{
    int numberOfNodes = pBNet->GetNumberOfNodes();
    EDistributionType dt;
    int i;
    
    const CNodeType *nodeTypes;// = new CNodeType[numberOfNodes];
    
    int *NumOfNodeVal = new int[numberOfNodes];
    for (i = 0; i < numberOfNodes; i++)
    {
        nodeTypes = pBNet->GetNodeType(i);
        NumOfNodeVal[i] = nodeTypes->GetNodeSize();
    }
    
    vls.resize(numberOfNodes);
    for (i = 0; i < numberOfNodes; i++)
    {
        dt = pBNet->GetFactor(i)->GetDistributionType();
        if ((dt == dtSoftMax) || (dt == dtTabular))
        {
            int valInt = rand() % NumOfNodeVal[i];
            
            printf("%3d", valInt);
            
            
            (vls)[i].SetInt(valInt);
        }
        if (dt == dtGaussian)
        {
            float valFl = pnlRand(StartVal, FinishVal);
            
            
            printf("%f\t", valFl);
            
            
            (vls)[i].SetFlt(valFl);
        }
    }
    
    
    printf("\n");
    delete [] NumOfNodeVal;

}
//-------------------------------------------------------------------------------
void SetRandEvidences(CBNet* pBNet, CEMLearningEngine *pLearnEng, 
  float StartVal, float FinishVal, int NumOfEvidences, int nObsNds, int* obsNds)
{
  int i;

  const CModelDomain *pMD = pBNet->GetModelDomain();
  valueVector vls;
  vls.resize(1);

  int NumOfNodes = pBNet->GetNumberOfNodes();
  CEvidence **m_pEv;
  m_pEv = new CEvidence *[NumOfEvidences];
  
  for (i = 0; i < NumOfEvidences; i++)
  {
    GenerateEvidence(pBNet, StartVal, FinishVal, vls, nObsNds, obsNds);
    m_pEv[i] = CEvidence::Create(pMD, nObsNds, obsNds, vls);
  }

  pLearnEng->SetData(NumOfEvidences, m_pEv);
}

void SetRandomEvidences(CBNet* pBNet, CEMLearningEngine *pLearnEng, 
  float StartVal, float FinishVal, int NumOfEvidences)
{
  int i;

  const CModelDomain *pMD = pBNet->GetModelDomain();
  valueVector vls;
  vls.resize(1);

  int NumOfNodes = pBNet->GetNumberOfNodes();
  CEvidence **m_pEv;
  m_pEv = new CEvidence *[NumOfEvidences];
  
  int nObsNds = NumOfNodes;
  int *obsNds = new int [nObsNds];
  for (i = 0; i < nObsNds; i++)
  {
    obsNds[i] = i;
  }
  
  for (i = 0; i < NumOfEvidences; i++)
  {
    GenerateSoftMaxEvidence(pBNet, StartVal, FinishVal, vls);
    m_pEv[i] = CEvidence::Create(pMD, nObsNds, obsNds, vls);
  }

  pLearnEng->SetData(NumOfEvidences, m_pEv);
}
//---------------------------------------------------------------------------------
int testConditionalSoftMaxInfAndLearn()
{
    
    int ret = TRS_OK;
    std::cout<<"Test on inference on seven nodes net with all continuous nodes observed "<<std::endl;
    std::cout<<"0  1   2 "<<std::endl;
    std::cout<<"|\\ \\  / "<<std::endl;
    std::cout<<"| \\ \\ / "<<std::endl;
    std::cout<<"|    3 "<<std::endl;
    std::cout<<"|   / \\ "<<std::endl;
    std::cout<<"|  4   5  "<<std::endl;
    std::cout<<"|/"<<std::endl;
    std::cout<<" 6"<<std::endl;
    std::cout<<" 0, 1, 5 - continuous nodes "<<std::endl;
    std::cout<<" 3, 6 - conditional Softmax nodes"<<std::endl;
    std::cout<<" 2, 4 - discrete nodes "<<std::endl;     
    
    CBNet* pBNet = NULL;
    CEvidence *pEvidence = NULL;

    pBNet = CreateSevenNodesNetForTests();
    
    if (pBNet == NULL)
    {
        ret = TRS_FAIL;
        return trsResult( ret, ret == TRS_OK ? "No errors" : 
        "FAIL Net creation");
    }
    else
    {
        std::cout<<"Net creation OK"<<std::endl; 
    };
    
    CJtreeInfEngine *pInfEng = CJtreeInfEngine::Create(pBNet); 

    if (pInfEng == NULL)
    {
        ret = TRS_FAIL;
        return trsResult( ret, ret == TRS_OK ? "No errors" : 
        "FAIL Junction tree inference creation");
    }
    else
    {
        std::cout<<"Junction tree inference creation OK"<<std::endl; 
    }

    std::cout<<"Test on inference on seven nodes net with all discrete nodes observed "<<std::endl;  

    int numOfObsNds  = 5;
    const int obsNds[] = { 0, 2, 3, 4, 6 };
    
    valueVector obsNdsVals(numOfObsNds);
    obsNdsVals[0].SetFlt(0);
    obsNdsVals[1].SetInt(1);
    obsNdsVals[2].SetInt(0);
    obsNdsVals[3].SetInt(1);
    obsNdsVals[4].SetInt(1);
    std::cout<<"All discrete nodes are observed "<<std::endl;
    std::cout<<"Observed nodes are 0, 2, 3, 4, 6  "<<std::endl;
    
    pEvidence = CEvidence::Create(pBNet, numOfObsNds, obsNds, obsNdsVals);
    
    if (pEvidence == NULL)
    {
        ret = TRS_FAIL;
        return trsResult( ret, ret == TRS_OK ? "No errors" : 
        "FAIL in evidence creation");
    }
    else
    {
        std::cout<<"Evidence creation OK"<<std::endl; 
    };

    pInfEng->EnterEvidence(pEvidence);

    const int querySz = 1;
    int query[] = { 1 };
    pInfEng->MarginalNodes(query,querySz);
    printf("\nDump function results\n");
    pInfEng->GetQueryJPD()->Dump();
    delete pEvidence;

    //std::cout<<"Next test. Press any key "<<endl;
    //getch();
    std::cout<<"Test on inference on seven nodes net with all continuous nodes observed "<<std::endl;  

    numOfObsNds  = 4;
    const int obsNodes[] = { 0, 1, 2, 5 };
    
    obsNdsVals.resize(numOfObsNds);
    obsNdsVals[0].SetFlt(0);
    obsNdsVals[1].SetFlt(1);
    obsNdsVals[2].SetInt(0);
    obsNdsVals[2].SetFlt(0);
    std::cout<<"All continuous nodes are observed "<<std::endl;
    std::cout<<"Observed nodes are 0, 1, 2, 5 "<<std::endl;

    pEvidence = CEvidence::Create(pBNet, numOfObsNds, obsNodes, obsNdsVals);
    
    if (pEvidence == NULL)
    {
        ret = TRS_FAIL;
        return trsResult( ret, ret == TRS_OK ? "No errors" : 
        "FAIL in evidence creation");
    }
    else
    {
        std::cout<<"Evidence creation OK"<<std::endl; 
    };

    pInfEng->EnterEvidence(pEvidence);

    query[0] = 3;
    pInfEng->MarginalNodes(query, querySz);
    printf("\nDump function results\n");
    pInfEng->GetQueryJPD()->Dump();
    delete pEvidence;
    delete pInfEng;

    //std::cout<<"Next test. Press any key "<<endl;
    //getch();
    std::cout<<"Test on learning on seven nodes net with all nodes observed "<<std::endl;  
  
    CEMLearningEngine *pLearnEng = NULL;  
    pLearnEng = CEMLearningEngine::Create(pBNet);

    if (pLearnEng == NULL)
    {
        ret = TRS_FAIL;
        return trsResult( ret, ret == TRS_OK ? "No errors" : 
        "FAIL Learning creation");
    }
    else
    {
        std::cout<<"Learning creation OK"<<std::endl; 
    }
    
    int nObsNdsLearn = 7;
    int *obsNdsLearn = new int [nObsNdsLearn];
    int i;
    for(i = 0; i < nObsNdsLearn; i++)
        obsNdsLearn[i] = i; 

    SetRandEvidences(pBNet, pLearnEng, -1.0, 1.0, 20, nObsNdsLearn, obsNdsLearn);
    pLearnEng->SetMaximizingMethod(mmGradient);
    
    pLearnEng->Learn();
    
    printf("\nResults\n========\n");
    int numOfNdsTmp = pBNet->GetNumberOfNodes();
    for (i = 0; i < numOfNdsTmp; i++)
    {
        pBNet->GetFactor(i)->GetDistribFun()->Dump();
    }
    
    delete pLearnEng;

    //std::cout<<"Next test. Press any key "<<endl;
    //getch();
    std::cout<<"Test on learning on seven nodes net with all continuous nodes observed "<<std::endl;   

    pLearnEng = CEMLearningEngine::Create(pBNet);
    
    int nObsNdsLearnForConExp = 3;
    int *obsNdsLearnForConExp = new int [nObsNdsLearnForConExp];
    obsNdsLearnForConExp[0] = 0; 
    obsNdsLearnForConExp[1] = 1;
    obsNdsLearnForConExp[2] = 5;

    SetRandEvidences(pBNet, pLearnEng, -1.0, 1.0, 20, nObsNdsLearnForConExp, 
        obsNdsLearnForConExp);
    pLearnEng->SetMaximizingMethod(mmGradient);
    
    pLearnEng->Learn();
           
    printf("\nResults\n========\n");
    for (i = 0; i < pBNet->GetNumberOfNodes(); i++)
    {
        pBNet->GetFactor(i)->GetDistribFun()->Dump();
    }
    
    delete pLearnEng;
    delete pBNet;
    
    return trsResult( ret, ret == TRS_OK ? "No errors" : "Tests FAILED");
}

void initACondSoftMaxAndSoftMaxInfAndLearn()
{
    trsReg(func_name, test_desc, test_class, testConditionalSoftMaxInfAndLearn);
}

