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

  for (int i = 0; i < NumVal; i++)
  {
    float val = pnlRand(StartVal, FinishVal);
    (vls)[i] = val; 
  }

  return vls;
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
}
// end of file ----------------------------------------------------------------
