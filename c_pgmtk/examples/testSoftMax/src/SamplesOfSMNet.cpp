/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      SamplesOfSMNet.cpp                                          //
//                                                                         //
//  Purpose:   BNet examples with SoftMax distribution type nodes          //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Bader, Chernishova, Gergel, Senin, Sidorov,     //
//             Sysoyev, Vinogradov                                         //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "SamplesOfSMNet.h"
#include "pnlSoftMaxCPD.hpp"
// ----------------------------------------------------------------------------

#ifdef _DEBUG
  #define SM_TEST
#endif

PNL_USING

void SetTwoNodeExEvidences(CBNet* pBNet, CEMLearningEngine *pLearnEng)
{
  const CModelDomain *pMD = pBNet->GetModelDomain();
  valueVector vls;
  vls.resize(1);

  int NumOfNodes = pBNet->GetNumberOfNodes();
  CEvidence **m_pEv;
  m_pEv = new CEvidence *[NumOfNodes];
  
  int nObsNds = NumOfNodes;
  int *obsNds = new int [nObsNds];
  for (int i = 0; i < nObsNds; i++)
    obsNds[i] = i;
  
  vls.resize(NumOfNodes);
  (vls)[0].SetFlt(3);
  (vls)[1].SetFlt(1);
  m_pEv[0] = CEvidence::Create(pMD, nObsNds, obsNds, vls);
  (vls)[0].SetFlt(1);
  (vls)[1].SetFlt(0);
  m_pEv[1] = CEvidence::Create(pMD, nObsNds, obsNds, vls);
  (vls)[0].SetFlt(2);
  (vls)[1].SetFlt(1);
  m_pEv[2] = CEvidence::Create(pMD, nObsNds, obsNds, vls);
  (vls)[0].SetFlt(2);
  (vls)[1].SetFlt(0);
  m_pEv[3] = CEvidence::Create(pMD, nObsNds, obsNds, vls);
  (vls)[0].SetFlt(1);
  (vls)[1].SetFlt(1);
  m_pEv[4] = CEvidence::Create(pMD, nObsNds, obsNds, vls);
  
  pLearnEng->SetData(5, m_pEv);
}
// ----------------------------------------------------------------------------

CBNet* CreateTwoNodeEx(void)
{
  const int numOfNds = 2;
  int numOfNbrs[numOfNds] = { 1, 1 };

  int nbrs0[] = { 1 };
  int nbrs1[] = { 0 };
  
  ENeighborType nbrsTypes0[] = { ntChild };
  ENeighborType nbrsTypes1[] = { ntParent };

  int *nbrs[] = { nbrs0, nbrs1 };
  ENeighborType *nbrsTypes[] = { nbrsTypes0, nbrsTypes1 };
  
  CGraph* pGraph = CGraph::Create(numOfNds, numOfNbrs, nbrs, nbrsTypes);
  
  CModelDomain* pMD;
  
  nodeTypeVector variableTypes;
  
  int nVariableTypes = 2;
  variableTypes.resize(nVariableTypes);
  
  variableTypes[0].SetType(0, 1);
  variableTypes[1].SetType(1, 2);
  
  intVector variableAssociation;
  int nnodes = pGraph->GetNumberOfNodes();
  variableAssociation.assign(nnodes, 1);
  variableAssociation[0] = 0;
  variableAssociation[1] = 1;
  
  pMD = CModelDomain::Create(variableTypes, variableAssociation);
  
  CBNet *pBNet = CBNet::Create(pGraph, pMD);
  
  pBNet->AllocFactors();
  
  int nnodes0 = 1;
  int domain0[] = { 0 };
  float mean0 = 0.0f;
  float cov0 = 1.0f;
  CGaussianCPD *pCPD0 = CGaussianCPD::Create(domain0, nnodes0, pMD);
  pCPD0->AllocDistribution(&mean0, &cov0, 1.0f, NULL);
  pBNet->AttachFactor(pCPD0);
  
  int nnodes1 = 2;
  int domain1[] = { 0, 1 };
  
  CSoftMaxCPD *pCPD3 = CSoftMaxCPD::Create(domain1, nnodes1, pMD);
  
  int parInd0[] = { 0 };

//  float weight30[] = { -1.0f,-1.0f };
//  float offset30[] = { 1.0f, 1.0f };

  float weight30[] = { -0.3059f, -1.1777f };
  float offset30[] = { 0.0886f, 0.2034f };

  pCPD3->AllocDistribution(weight30, offset30, parInd0);

  pBNet->AttachFactor(pCPD3);
  
  return pBNet;
}
// ----------------------------------------------------------------------------

float * GenerateFloatArray(int NumVal, float StartVal, float FinishVal)
{
  float *vls = new float[NumVal];

  float val;
  for (int i = 0; i < NumVal; i++)
  {
    val = pnlRand(StartVal, FinishVal);
    (vls)[i] = val; 
  }

  return vls;
}
// ----------------------------------------------------------------------------
CBNet* CreateGaussianExample(void)
{
  CBNet *pBNet;
  int i;
  const int nnodes = 3;

  int numOfNeigh[] = { 1, 2, 1 };

  int neigh0[] = { 1 };
  int neigh1[] = { 0, 2 };
  int neigh2[] = { 1 };

  ENeighborType orient0[] = { ntChild };
  ENeighborType orient1[] = { ntParent, ntChild };
  ENeighborType orient2[] = { ntParent };

  int *neigh[] = { neigh0,  neigh1, neigh2 };
  
  ENeighborType *orient[] = { orient0, orient1, orient2 };

  CGraph *pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );

  const int numberOfNodeTypes = 1;

  CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];

  nodeTypes[0].SetType(0, 1);

  int *nodeAssociation = new int[nnodes];

  for (i = 0; i < nnodes; i++)
    nodeAssociation[i] = 0; 
  
  pBNet = CBNet::Create(nnodes, numberOfNodeTypes, nodeTypes, nodeAssociation, pGraph);
  
  CModelDomain* pMD = pBNet->GetModelDomain();

  int domain0[] = { 0 };
  int domain1[] = { 0, 1 };
  int domain2[] = { 1, 2 };

  int *domains[] = { domain0, domain1, domain2 };

  float mean0 = 0.0f;
  float cov0 = 1.0f;
  CGaussianCPD* pCPD = CGaussianCPD::Create( domain0, 1, pMD );
  pCPD->AllocDistribution( &mean0, &cov0, 1.0f, NULL );
  pBNet->AttachFactor(pCPD);

  float meanNode1 = 8.0f;
  float covNode1 = 1.0f;
  float weightNode1[] = { 0.01f };
    
  pCPD = CGaussianCPD::Create( domain1, 2, pMD );
  
  const float *pData = weightNode1;
  pCPD->AllocDistribution( &meanNode1, &covNode1, 0.5f, &pData );
  pBNet->AttachFactor(pCPD);

  float meanNode2 = 1.0f ;
  float covNode2 = 0.01f;
  float weightNode2[] = {0.01f};
    
  pCPD = CGaussianCPD::Create( domain2, 2, pMD );
	const float *pData2 = weightNode2;
	pCPD->AllocDistribution( &meanNode2, &covNode2, 0.5f, &pData2 );
  
  pBNet->AttachFactor(pCPD);

  delete [] nodeTypes;
  delete [] nodeAssociation;
  return pBNet;
}
// ----------------------------------------------------------------------------

CBNet* CreateSixNodeEx(void)
{
  int i;
  const int numOfNds = 6;
  int numOfNbrs[numOfNds] = { 1, 1, 1, 1, 1, 5 };

  int nbrs0[] = { 5 };
  int nbrs1[] = { 5 };
  int nbrs2[] = { 5 };
  int nbrs3[] = { 5 };
  int nbrs4[] = { 5 };
  int nbrs5[] = { 0, 1, 2, 3, 4 };
  
  ENeighborType nbrsTypes0[] = { ntChild };
  ENeighborType nbrsTypes1[] = { ntChild };
  ENeighborType nbrsTypes2[] = { ntChild };
  ENeighborType nbrsTypes3[] = { ntChild };
  ENeighborType nbrsTypes4[] = { ntChild };
  ENeighborType nbrsTypes5[] = { ntParent, ntParent, ntParent, ntParent,
                                 ntParent };

  int *nbrs[] = { nbrs0, nbrs1, nbrs2, nbrs3, nbrs4, nbrs5};
  ENeighborType *nbrsTypes[] = { nbrsTypes0, nbrsTypes1, nbrsTypes2, 
                                 nbrsTypes3, nbrsTypes4, nbrsTypes5 };
  
  CGraph* pGraph = CGraph::Create(numOfNds, numOfNbrs, nbrs, nbrsTypes);
  
  CModelDomain* pMD;
  
  nodeTypeVector variableTypes;
  
  int nVariableTypes = 2;
  variableTypes.resize(nVariableTypes);
  
  variableTypes[0].SetType(0, 1);
//  variableTypes[0].SetType(1, 4);
  variableTypes[1].SetType(1, 2);

  intVector variableAssociation;
  int nnodes = pGraph->GetNumberOfNodes();
  variableAssociation.assign(nnodes, 1);
  variableAssociation[0] = 0;
  variableAssociation[1] = 0;
  variableAssociation[2] = 0;
  variableAssociation[3] = 0;
  variableAssociation[4] = 0;
  variableAssociation[5] = 1;

  pMD = CModelDomain::Create(variableTypes, variableAssociation);
  
  CBNet *pBNet = CBNet::Create(pGraph, pMD);
  
  pBNet->AllocFactors();
  
  int nnodes0 = 1;
  int domain0[] = { 0 };
  float mean0 = 0.0f;
  float cov0 = 1.0f;

  CGaussianCPD *pCPD0 = CGaussianCPD::Create(domain0, nnodes0, pMD);
  pCPD0->AllocDistribution(&mean0, &cov0, 1.0f, NULL);
  pBNet->AttachFactor(pCPD0);
  
  int nnodes1 = 1;
  int domain1[] = { 1 };
  float mean1 = 2.0f;
  float cov1 = 1.0f;

  CGaussianCPD *pCPD1 = CGaussianCPD::Create(domain1, nnodes1, pMD);
  pCPD1->AllocDistribution(&mean1, &cov1, 1.0f, NULL);
  pBNet->AttachFactor(pCPD1);

  int nnodes2 = 1;
  int domain2[] = { 2 };
  float mean2 = 1.0f;
  float cov2 = 1.0f;

  CGaussianCPD *pCPD2 = CGaussianCPD::Create(domain2, nnodes2, pMD);
  pCPD2->AllocDistribution(&mean2, &cov2, 1.0f, NULL);
  pBNet->AttachFactor(pCPD2);

  int nnodes3 = 1;
  int domain3[] = { 3 };
  float mean3 = 5.0f;
  float cov3 = 1.0f;

  CGaussianCPD *pCPD3 = CGaussianCPD::Create(domain3, nnodes3, pMD);
  pCPD3->AllocDistribution(&mean3, &cov3, 1.0f, NULL);
  pBNet->AttachFactor(pCPD3);

  int nnodes4 = 1;
  int domain4[] = { 4 };
  float mean4 = 5.0f;
  float cov4 = 1.0f;

  CGaussianCPD *pCPD4 = CGaussianCPD::Create(domain4, nnodes4, pMD);
  pCPD4->AllocDistribution(&mean4, &cov4, 1.0f, NULL);
  pBNet->AttachFactor(pCPD4);

  int nnodes5 = 6;
  int domain5[] = { 0, 1, 2, 3, 4, 5 };

  CSoftMaxCPD *pCPD5 = CSoftMaxCPD::Create(domain5, nnodes5, pMD);

  int parInd0[] = { 0 };

  float *weight30;
  int SizeOfWeight = (variableTypes[1].GetNodeSize()) * (numOfNds - 1);
  weight30 = GenerateFloatArray((variableTypes[1].GetNodeSize()) * 
    (numOfNds - 1), 1.0f, 5.0f);

#ifdef SM_TEST
  printf("\nweight30\n");
  for (i = 0; i < SizeOfWeight; i++)
  {
    printf("%f\t", weight30[i]);
  }
#endif

  float *offset30;
  int SizeOfOffset = variableTypes[1].GetNodeSize();
  offset30 = GenerateFloatArray(variableTypes[1].GetNodeSize(), 1.0f, 5.0f);

#ifdef SM_TEST
  printf("\noffset30\n");
  for (i = 0; i < SizeOfOffset; i++)
  {
    printf("%f\t", offset30[i] );
  }
  printf("\n\n");
#endif

  pCPD5->AllocDistribution(weight30, offset30, parInd0);

  pBNet->AttachFactor(pCPD5);
  
  return pBNet;
}
// ----------------------------------------------------------------------------
CBNet* CreateSevenNodeEx(void)
{

  // 0  1   2 
  // |\ \  / 
  // | \ \ /
  // |    3 
  // |   / \ 
  // |  4   5    
  // |/
  // 6
  // 0, 1, 5 - continuous
  // 3, 6 - softmax 
  // 2, 4 - discrete

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
    
  int nVariableTypes = 2;
  variableTypes.resize( nVariableTypes );
   
  variableTypes[0].SetType( 0, 1 ); // continuous
  variableTypes[1].SetType( 1, 2 ); // discrete, 2 states
  
  intVector variableAssociation;  
  int nnodes = pGraph->GetNumberOfNodes();
  variableAssociation.assign(nnodes, 1);
  variableAssociation[0] = 0;
  variableAssociation[1] = 0;
  variableAssociation[2] = 1;
  variableAssociation[3] = 1;
  variableAssociation[4] = 1;
  variableAssociation[5] = 0;
  variableAssociation[6] = 1;

  pMD = CModelDomain::Create( variableTypes, variableAssociation );
    
  // 2) Creation base for BNet using Graph, and Model Domain
    
  CBNet *pBNet = CBNet::Create(pGraph, pMD);
    
  // 3)Allocation space for all factors of the model
  pBNet->AllocFactors();

  int nnodes0 = 1;
  int domain0[] = { 0 };
  float mean0 = 0.5f;
  float cov0 = 1.0f;
  CGaussianCPD *pCPD0 = CGaussianCPD::Create( domain0, nnodes0, pMD );
  pCPD0->AllocDistribution( &mean0, &cov0, 1.0f, NULL );
  pBNet->AttachFactor( pCPD0 );
  
  int nnodes1 = 1;
  int domain1[] = { 1 };
  float mean1 = 0.5f;
  float cov1 = 1.0f;
  CGaussianCPD *pCPD1 = CGaussianCPD::Create( domain1, nnodes1, pMD );
  pCPD1->AllocDistribution( &mean1, &cov1, 1.0f, NULL );
  pBNet->AttachFactor( pCPD1 );

  int nnodes2 = 1;
  int domain2[] = { 2 };
  float table2[] = { 0.3f, 0.7f};
  CTabularCPD *pCPD2 = CTabularCPD::Create( domain2, nnodes2, pMD, table2 );
  pCPD2->AllocMatrix(table2, matTable);
  pBNet->AttachParameter(pCPD2);

  int nnodes3 = 4;
  int domain3[] = { 0, 1, 2, 3 };
  CSoftMaxCPD *pCPD3 = CSoftMaxCPD::Create( domain3, nnodes3, pMD );

  int parInd30[] = { 0 };
//  float weight30[] = { 0.5f, 0.5f, 0.5f, 0.7f, 0.3f, 0.7f };
//  float offset30[] = { 0.3f, 0.5f, 1.2f };
  float weight30[] = { 0.5f, 0.4f, 0.5f, 0.7f };
  float offset30[] = { 0.3f, 0.5f };

  pCPD3->AllocDistribution( weight30, offset30, parInd30 );
  
  int parInd31[] = { 1 };
  float weight31[] = { 0.5f, 0.1f, 0.5f, 0.7f };
  float offset31[] = { 0.3f, 0.5f };
//  float weight31[] = { 0.5f, 0.5f, 0.5f, 0.7f, 0.3f, 0.7f };
//  float offset31[] = { 0.3f, 0.5f, 5.4f };

  pCPD3->AllocDistribution( weight31, offset31, parInd31 );
  pBNet->AttachFactor( pCPD3 );

  int nnodes4 = 2;
  int domain4[] = { 3, 4 };
  float table4[] = { 0.3f, 0.7f, 0.8f, 0.2f };
//  float table4[] = { 0.3f, 0.7f, 0.5, 0.5, 0.1, 0.9 };

  CTabularCPD *pCPD4 = CTabularCPD::Create( domain4, nnodes4, pMD, table4 );
  pCPD4->AllocMatrix(table4, matTable);
  pBNet->AttachParameter(pCPD4);

  int nnodes5 = 2;
  int domain5[] = { 3, 5 };
  CGaussianCPD *pCPD5 = CGaussianCPD::Create( domain5, nnodes5, pMD );
 
  float mean50 = 1.0f;
  float cov50 = 1.0f;
  int parInd50[] = { 0 };
  pCPD5->AllocDistribution( &mean50, &cov50, 1.0f, NULL, parInd50 );

  float mean51 = 0.5f;
  float cov51 = 0.5f;
  int parInd51[] = { 1 };
  pCPD5->AllocDistribution( &mean51, &cov51, 1.0f, NULL, parInd51 );
 
/*  float mean52 = 0.0f;
  float cov52 = 1.f;
  int parInd52[] = { 2 };
  pCPD5->AllocDistribution( &mean52, &cov52, 1.0f, NULL, parInd52 );
*/
  pBNet->AttachFactor(pCPD5);

  int nnodes6 = 3;
  int domain6[] = { 0, 4, 6 };
  CSoftMaxCPD *pCPD6 = CSoftMaxCPD::Create( domain6, nnodes6, pMD );
  int parInd60[] = { 0 };
  
  float weight60[] = { 0.5f, 0.9f, 3.2f };
  float offset60[] = { 0.7f, 0.3f, 0.1f };
  
  pCPD6->AllocDistribution( weight60, offset60, parInd60 );
  
  int parInd61[] = { 1 };

//  float weight61[] = { 0.8f, 0.2f, 0.5f };
//  float offset61[] = { 0.1f, 0.9f, 1.9f };
  float weight61[] = { 0.8f, 0.2f };
  float offset61[] = { 0.1f, 0.9f };

  pCPD6->AllocDistribution( weight61, offset61, parInd61 );

  pBNet->AttachFactor( pCPD6 );

  return pBNet;
}
// ----------------------------------------------------------------------------
CBNet* CreateSevenNodeExDiscrete(void)
{

  // 0  1   2 
  // |\ \  / 
  // | \ \ /
  // |    3 
  // |   / \ 
  // |  4   5    
  // |/
  // 6
  // 0, 1, 5 -  continuous 
  // 3, 6 - softmax
  // 2, 4 - discrete

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
    
  int nVariableTypes = 2;
  variableTypes.resize( nVariableTypes );
   
  variableTypes[0].SetType( 0, 1 ); // continuous
  variableTypes[1].SetType( 1, 2 ); // discrete, 2 states
  
  intVector variableAssociation;  
  int nnodes = pGraph->GetNumberOfNodes();
  variableAssociation.assign(nnodes, 1);
  variableAssociation[0] = 1;
  variableAssociation[1] = 1;
  variableAssociation[2] = 1;
  variableAssociation[3] = 1;
  variableAssociation[4] = 1;
  variableAssociation[5] = 1;
  variableAssociation[6] = 1;

  pMD = CModelDomain::Create( variableTypes, variableAssociation );
    
  // 2) Creation base for BNet using Graph, and Model Domain
    
  CBNet *pBNet = CBNet::Create(pGraph, pMD);
    
  // 3)Allocation space for all factors of the model
  pBNet->AllocFactors();

  int nnodes0 = 1;
  int domain0[] = { 0 };
  float table0[] = { 0.3f, 0.7f};
  CTabularCPD *pCPD0 = CTabularCPD::Create( domain0, nnodes0, pMD, table0 );
  pCPD0->AllocMatrix(table0, matTable);
  pBNet->AttachParameter(pCPD0);

  int nnodes1 = 1;
  int domain1[] = { 1 };
  float table1[] = { 0.3f, 0.7f};
  CTabularCPD *pCPD1 = CTabularCPD::Create( domain1, nnodes1, pMD, table1 );
  pCPD1->AllocMatrix(table1, matTable);
  pBNet->AttachParameter(pCPD1);

  int nnodes2 = 1;
  int domain2[] = { 2 };
  float table2[] = { 0.3f, 0.7f};
  CTabularCPD *pCPD2 = CTabularCPD::Create( domain2, nnodes2, pMD, table2 );
  pCPD2->AllocMatrix(table2, matTable);
  pBNet->AttachParameter(pCPD2);

  int nnodes3 = 4;
  int domain3[] = { 0, 1, 2, 3 };
  float table3[] = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f};
  CTabularCPD *pCPD3 = CTabularCPD::Create( domain3, nnodes3, pMD, table3 );
  pCPD3->AllocMatrix(table3, matTable);
  pBNet->AttachParameter(pCPD3);
  
  int nnodes4 = 2;
  int domain4[] = { 3, 4 };
  float table4[] = { 0.3f, 0.7f, 0.5, 0.5 };
  CTabularCPD *pCPD4 = CTabularCPD::Create( domain4, nnodes4, pMD, table4 );
  pCPD4->AllocMatrix(table4, matTable);
  pBNet->AttachParameter(pCPD4);

  int nnodes5 = 2;
  int domain5[] = { 3, 5 };
  float table5[] = { 0.3f, 0.7f, 0.5f, 0.5f };
  CTabularCPD *pCPD5 = CTabularCPD::Create( domain5, nnodes5, pMD, table5 );
  pCPD5->AllocMatrix(table5, matTable);
  pBNet->AttachParameter(pCPD5);

  int nnodes6 = 3;
  int domain6[] = { 0, 4, 6 };
  float table6[] = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f };
  CTabularCPD *pCPD6 = CTabularCPD::Create( domain6, nnodes6, pMD, table6 );
  pCPD6->AllocMatrix(table6, matTable);
  pBNet->AttachParameter(pCPD6);

  return pBNet;
}

CBNet* CreateTwoNodeExDiscrete(void)
{

  const int numOfNds = 2;
  int numOfNbrs[numOfNds] = { 1, 1 };

  int nbrs0[] = { 1 };
  int nbrs1[] = { 0 };

  ENeighborType nbrsTypes0[] = { ntChild };
  ENeighborType nbrsTypes1[] = { ntParent };

  int *nbrs[] = { nbrs0, nbrs1 };
  ENeighborType *nbrsTypes[] = { nbrsTypes0, nbrsTypes1 };

  CGraph* pGraph = CGraph::Create( numOfNds, numOfNbrs, nbrs, nbrsTypes );
    
    // 2) Creation of the Model Domain.
  CModelDomain* pMD;

  nodeTypeVector variableTypes;
    
  int nVariableTypes = 1;
  variableTypes.resize( nVariableTypes );
  variableTypes[0].SetType( 1, 2 ); // discrete, 2 states
  
  intVector variableAssociation;  
  int nnodes = pGraph->GetNumberOfNodes();
  variableAssociation.assign(nnodes, 1);
  variableAssociation[0] = 0;
  variableAssociation[1] = 0;
  
  pMD = CModelDomain::Create( variableTypes, variableAssociation );
    
  // 2) Creation base for BNet using Graph, and Model Domain
    
  CBNet *pBNet = CBNet::Create(pGraph, pMD);
    
  // 3)Allocation space for all factors of the model
  pBNet->AllocFactors();

  int nnodes0 = 1;
  int domain0[] = { 0 };
  float table0[] = { 0.3f, 0.7f};
  CTabularCPD *pCPD0 = CTabularCPD::Create( domain0, nnodes0, pMD, table0 );
  pCPD0->AllocMatrix(table0, matTable);
  pBNet->AttachParameter(pCPD0);

  int nnodes1 = 2;
  int domain1[] = { 0, 1 };
  float table1[] = { 0.3f, 0.7f, 0.3f, 0.7f};
  CTabularCPD *pCPD1 = CTabularCPD::Create( domain1, nnodes1, pMD, table1 );
  pCPD1->AllocMatrix(table1, matTable);
  pBNet->AttachParameter(pCPD1);

  return pBNet;
}

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

#ifdef SM_TEST
  printf("%3d\t", valInt);
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
// ----------------------------------------------------------------------------

void SetRandomEvidences(CBNet* pBNet, CEMLearningEngine *pLearnEng, 
  float StartVal, float FinishVal, int NumOfEvidences)
{
  int i;

  const CModelDomain *pMD = pBNet->GetModelDomain();
  valueVector vls;
  vls.resize(1);

  int NumOfNodes = pBNet->GetNumberOfNodes();
  CEvidence **m_pEv;
//  m_pEv = new CEvidence *[NumOfNodes];
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
  delete [] m_pEv;
}
// ----------------------------------------------------------------------------

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
// ----------------------------------------------------------------------------

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
  delete [] m_pEv;
}

CBNet* CreateFourNodeExample(void)
{

  CBNet *pBNet;
  const int nnodes = 4;
  const int numberOfNodeTypes = 2;

  int numOfNeigh[] = { 1, 1, 3, 1 };

  int neigh0[] = { 2 };
  int neigh1[] = { 2 };
  int neigh2[] = { 0, 1, 3 };
  int neigh3[] = { 2 };

  ENeighborType orient0[] = { ntChild };
  ENeighborType orient1[] = { ntChild };
  ENeighborType orient2[] = { ntParent, ntParent, ntChild };
  ENeighborType orient3[] = { ntParent };

  int *neigh[] = { neigh0,  neigh1, neigh2, neigh3 };
  
  ENeighborType *orient[] = { orient0, orient1, orient2, orient3 };

  CGraph *pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );

  CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];
  nodeTypes[0].SetType(1, 2);
  nodeTypes[1].SetType(0, 1);

  int *nodeAssociation = new int[nnodes];

  nodeAssociation[0]=0;
  nodeAssociation[1]=1;
  nodeAssociation[2]=1;
  nodeAssociation[3]=0;
  
  pBNet = CBNet::Create(nnodes, numberOfNodeTypes, nodeTypes, nodeAssociation, pGraph);
  
  CModelDomain* pMD = pBNet->GetModelDomain();
  
//number of parameters is the same as number of nodes - one CPD per node
//  CFactor *myParams = new CFactor[1];
  int *nodeNumbers = new int [nnodes];

  int domain0[] = { 0 };
  int domain1[] = { 1 };
  int domain2[] = { 0, 1, 2 };
  int domain3[] = { 2, 3 };
  int *domains[] = { domain0, domain1, domain2, domain3 };
  nodeNumbers[0] = 1;
  nodeNumbers[1] = 1;
  nodeNumbers[2] = 3;
  nodeNumbers[3] = 2;
  
  pBNet->AllocParameters();
  
  CFactor *myParams = CTabularCPD::Create( domain0, nodeNumbers[0], pMD );
  
// data creation for all CPDs of the model
  float data0[] = { 0.5f, 0.5f };
  
  myParams->AllocMatrix(data0, matTable);
  pBNet->AttachParameter(myParams);

  float mean0 = 0.0f;
  float cov0 = 1.0f;
  CGaussianCPD* pCPD1 = CGaussianCPD::Create( domain1, 1, pMD );
  pCPD1->AllocDistribution( &mean0, &cov0, 1.0f, NULL);
  pBNet->AttachFactor(pCPD1);

/*  float mean1[] = { 8.0f };
  float mean2[] = { 2.0f };
  float cov1[] = { 1.0f };
  float cov2[] = { 1.0f };
*/
  float weight[] = { 0.01f, 0.03f };
	const float *pData = weight;

  CGaussianCPD* pCPD2 = CGaussianCPD::Create( domain2, 3, pMD );

  float mean20 = 1.0f;
  float cov20 = 1.0f;
  int parInd20[] = { 0 };
  pCPD2->AllocDistribution( &mean20, &cov20, 1.0f, &pData, parInd20 );

  float mean21 = 0.5f;
  float cov21 = 0.5f;
  int parInd21[] = { 1 };
  pCPD2->AllocDistribution( &mean21, &cov21, 1.0f, &pData, parInd21 );

/*  int ParentCom[] = { 0, 1 };
  pCPD1->AllocDistribution( mean1, cov1, 0.5f, &pData, &ParentCom[0] );
  pCPD1->AllocDistribution( mean2, cov2, 0.5f, &pData, &ParentCom[1] );
*/
  pBNet->AttachFactor(pCPD2);
  
  CSoftMaxCPD *pCPD3 = CSoftMaxCPD::Create(domain3, nodeNumbers[3], pMD);
  
  int parInd0[] = { 0 };
  float weight30[] = { -1.32f,  0.0f };
  float offset30[] = { -0.111f,  0.0f };
  pCPD3->AllocDistribution(weight30, offset30, parInd0);

  pBNet->AttachFactor(pCPD3);
  delete [] nodeTypes;
  return pBNet;
}

CBNet* CreateFourNodeExampleNew(void)
{

  CBNet *pBNet;
  const int nnodes = 4;
  const int numberOfNodeTypes = 2;

  int numOfNeigh[] = { 1, 1, 3, 1 };

  int neigh0[] = { 2 };
  int neigh1[] = { 2 };
  int neigh2[] = { 0, 1, 3 };
  int neigh3[] = { 2 };

  ENeighborType orient0[] = { ntChild };
  ENeighborType orient1[] = { ntChild };
  ENeighborType orient2[] = { ntParent, ntParent, ntChild };
  ENeighborType orient3[] = { ntParent };

  int *neigh[] = { neigh0,  neigh1, neigh2, neigh3 };
  
  ENeighborType *orient[] = { orient0, orient1, orient2, orient3 };

  CGraph *pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );

  CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];
  nodeTypes[0].SetType(1, 2);
  nodeTypes[1].SetType(0, 1);

  int *nodeAssociation = new int[nnodes];

  nodeAssociation[0]=0;
  nodeAssociation[1]=1;
  nodeAssociation[2]=1;
  nodeAssociation[3]=1;
  
  pBNet = CBNet::Create(nnodes, numberOfNodeTypes, nodeTypes, nodeAssociation, pGraph);
  
  CModelDomain* pMD = pBNet->GetModelDomain();
  
//number of parameters is the same as number of nodes - one CPD per node
//  CFactor *myParams = new CFactor[1];
  int *nodeNumbers = new int [nnodes];

  int domain0[] = { 0 };
  int domain1[] = { 1 };
  int domain2[] = { 0, 1, 2 };
  int domain3[] = { 2, 3 };
  int *domains[] = { domain0, domain1, domain2, domain3 };
  nodeNumbers[0] = 1;
  nodeNumbers[1] = 1;
  nodeNumbers[2] = 3;
  nodeNumbers[3] = 2;
  
  pBNet->AllocParameters();
  
  CFactor *myParams = CTabularCPD::Create( domains[0], nodeNumbers[0], pMD );
  
// data creation for all CPDs of the model
  float data0[] = { 0.5f, 0.5f };
  
  myParams->AllocMatrix(data0, matTable);
  pBNet->AttachParameter(myParams);

  float mean0 = 0.0f;
  float cov0 = 1.0f;
  CGaussianCPD* pCPD = CGaussianCPD::Create( domain1, 1, pMD );
  pCPD->AllocDistribution( &mean0, &cov0, 1.0f, NULL);
  pBNet->AttachFactor(pCPD);

  float mean1[] = { 8.0f };
  float mean2[] = { 2.0f };
  float cov1[] = { 1.0f };
  float cov2[] = { 1.0f };

  float weight[] = { 0.01f, 0.03f };
	float weight1[] = { 0.01f };
	const float *pData = weight;
	const float *pData1 = weight1;

  CGaussianCPD* pCPD1 = CGaussianCPD::Create( domain2, 3, pMD );
  int ParentCom[] = { 0, 1 };
  pCPD1->AllocDistribution( mean1, cov1, 0.5f, &pData, &ParentCom[0] );
  pCPD1->AllocDistribution( mean2, cov2, 0.5f, &pData, &ParentCom[1] );
  pBNet->AttachFactor(pCPD1);
  
  CGaussianCPD* pCPD2 = CGaussianCPD::Create( domain3, 2, pMD );
  pCPD2->AllocDistribution( mean1, cov1, 0.5f, &pData1 );
  pBNet->AttachFactor(pCPD2);


  delete [] nodeTypes;
  return pBNet;
}

CBNet* CreateThreeNodeExample(void)
{

  CBNet *pBNet;
  const int nnodes = 3;
  const int numberOfNodeTypes = 2;

  int numOfNeigh[] = { 1, 1, 2 };

  int neigh0[] = { 2 };
  int neigh1[] = { 2 };
  int neigh2[] = { 0, 1 };
  
  ENeighborType orient0[] = { ntChild };
  ENeighborType orient1[] = { ntChild };
  ENeighborType orient2[] = { ntParent, ntParent };
  
  int *neigh[] = { neigh0,  neigh1, neigh2 };
  
  ENeighborType *orient[] = { orient0, orient1, orient2};

  CGraph *pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );

  CNodeType *nodeTypes = new CNodeType [numberOfNodeTypes];
  nodeTypes[0].SetType(1, 2);
  nodeTypes[1].SetType(0, 1);

  int *nodeAssociation = new int[nnodes];

  nodeAssociation[0]=0;
  nodeAssociation[1]=1;
  nodeAssociation[2]=1;
  
  pBNet = CBNet::Create(nnodes, numberOfNodeTypes, nodeTypes, nodeAssociation, pGraph);
  
  CModelDomain* pMD = pBNet->GetModelDomain();
  
//number of parameters is the same as number of nodes - one CPD per node
//  CFactor *myParams = new CFactor[1];
  int *nodeNumbers = new int [nnodes];

  int domain0[] = { 0 };
  int domain1[] = { 1 };
  int domain2[] = { 0, 1, 2 };
  
  int *domains[] = { domain0, domain1, domain2};
  nodeNumbers[0] = 1;
  nodeNumbers[1] = 1;
  nodeNumbers[2] = 3;
  
  pBNet->AllocParameters();
  
  CFactor *myParams = CTabularCPD::Create( domains[0], nodeNumbers[0], pMD );
  
// data creation for all CPDs of the model
  float data0[] = { 0.5f, 0.5f };
  
  myParams->AllocMatrix(data0, matTable);
  pBNet->AttachParameter(myParams);

  float mean0 = 0.0f;
  float cov0 = 1.0f;
  CGaussianCPD* pCPD = CGaussianCPD::Create( domain1, 1, pMD );
  pCPD->AllocDistribution( &mean0, &cov0, 1.0f, NULL);
  pBNet->AttachFactor(pCPD);

  float mean1[] = { 8.0f };
  float mean2[] = { 2.0f };
  float cov1[] = { 1.0f };
  float cov2[] = { 1.0f };

  float weight[] = { 0.01f, 0.03f };
	const float *pData = weight;
  CGaussianCPD* pCPD1 = CGaussianCPD::Create( domain2, 3, pMD );
  int ParentCom[] = { 0, 1 };
  pCPD1->AllocDistribution( mean1, cov1, 0.5f, &pData, &ParentCom[0] );
  pCPD1->AllocDistribution( mean2, cov2, 0.5f, &pData, &ParentCom[1] );
  pBNet->AttachFactor(pCPD1);
  
  delete [] nodeTypes;
  return pBNet;
}
// end of file ----------------------------------------------------------------
