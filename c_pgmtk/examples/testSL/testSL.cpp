/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AStructureLearning.cpp                                      //
//                                                                         //
//  Purpose:   Test on structure learning of BNet                          //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <math.h>
#include <float.h>
#include "pnlMlStaticStructLearn.hpp"
#include "pnlMlStaticStructLearnHC.hpp"

PNL_USING
CBNet* CreateAlarmBNet()
{
    const int nnodes = 37;
    const int numberOfNodeTypes = 37;
    
    int i;
    
    CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
    pGraph->AddNodes(37);
    pGraph->AddEdge(0,1,1);
    pGraph->AddEdge(0,3,1);
    pGraph->AddEdge(2,3,1);
    pGraph->AddEdge(3,4,1);
    pGraph->AddEdge(3,5,1);
    pGraph->AddEdge(0,6,1);
    pGraph->AddEdge(2,6,1);
    pGraph->AddEdge(11,12,1);
    pGraph->AddEdge(10,13,1);
    pGraph->AddEdge(12,13,1);
    pGraph->AddEdge(8,14,1);
    pGraph->AddEdge(9,14,1);
    pGraph->AddEdge(13,14,1);
    pGraph->AddEdge(8,15,1);
    pGraph->AddEdge(14,15,1);
    pGraph->AddEdge(15,16,1);
    pGraph->AddEdge(15,19,1);
    pGraph->AddEdge(18,19,1);
    pGraph->AddEdge(8,21,1);
    pGraph->AddEdge(20,21,1);
    pGraph->AddEdge(19,22,1);
    pGraph->AddEdge(21,22,1);
    pGraph->AddEdge(23,24,1);
    pGraph->AddEdge(16,25,1);
    pGraph->AddEdge(17,25,1);
    pGraph->AddEdge(22,25,1);
    pGraph->AddEdge(24,25,1);
    pGraph->AddEdge(25,26,1);
    pGraph->AddEdge(7,27,1);
    pGraph->AddEdge(26,27,1);
    pGraph->AddEdge(26,29,1);
    pGraph->AddEdge(28,29,1);
    pGraph->AddEdge(26,30,1);
    pGraph->AddEdge(28,30,1);
    pGraph->AddEdge(14,31,1);
    pGraph->AddEdge(16,31,1);
    pGraph->AddEdge(8,32,1);
    pGraph->AddEdge(14,32,1);
    pGraph->AddEdge(20,33,1);
    pGraph->AddEdge(8,34,1);
    pGraph->AddEdge(9,34,1);
    pGraph->AddEdge(13,34,1);
    pGraph->AddEdge(6,35,1);
    pGraph->AddEdge(26,35,1);
    pGraph->AddEdge(24,36,1);
    pGraph->AddEdge(35,36,1);
    
    CNodeType *nodeTypes = new CNodeType [37];
    
    nodeTypes[0].SetType(1, 2);
    nodeTypes[1].SetType(1, 2);
    nodeTypes[2].SetType(1, 2);
    nodeTypes[3].SetType(1, 3);
    nodeTypes[4].SetType(1, 3);
    nodeTypes[5].SetType(1, 3);
    nodeTypes[6].SetType(1, 3);
    nodeTypes[7].SetType(1, 2);
    nodeTypes[8].SetType(1, 3);
    nodeTypes[9].SetType(1, 2);
    nodeTypes[10].SetType(1, 2);
    nodeTypes[11].SetType(1, 3);
    nodeTypes[12].SetType(1, 4);
    nodeTypes[13].SetType(1, 4);
    nodeTypes[14].SetType(1, 4);
    nodeTypes[15].SetType(1, 4);
    nodeTypes[16].SetType(1, 3);
    nodeTypes[17].SetType(1, 2);
    nodeTypes[18].SetType(1, 2);
    nodeTypes[19].SetType(1, 3);
    nodeTypes[20].SetType(1, 2);
    nodeTypes[21].SetType(1, 2);
    nodeTypes[22].SetType(1, 3);
    nodeTypes[23].SetType(1, 2);
    nodeTypes[24].SetType(1, 3);
    nodeTypes[25].SetType(1, 2);
    nodeTypes[26].SetType(1, 3);
    nodeTypes[27].SetType(1, 3);
    nodeTypes[28].SetType(1, 2);
    nodeTypes[29].SetType(1, 3);
    nodeTypes[30].SetType(1, 3);
    nodeTypes[31].SetType(1, 4);
    nodeTypes[32].SetType(1, 4);
    nodeTypes[33].SetType(1, 3);
    nodeTypes[34].SetType(1, 4);
    nodeTypes[35].SetType(1, 3);
    nodeTypes[36].SetType(1, 3);
    
    int *nodeAssociation = new int[nnodes];
    
    for ( i = 0; i < nnodes; i++ )
    {
	nodeAssociation[i] = i;
    }
    CBNet *pBNet = CBNet::Create( nnodes, numberOfNodeTypes, nodeTypes,
	nodeAssociation, pGraph );
    
    CModelDomain* pMD = pBNet->GetModelDomain();
    
    //number of factors is the same as number of nodes - one CPD per node
    CFactor **myParams = new CFactor*[nnodes];
    int *nodeNumbers = new int [nnodes];
    
    int domain0[] = { 0 };
    nodeNumbers[0] =  1;
    int domain1[] = { 0, 1 };
    nodeNumbers[1] =  2;
    int domain2[] = { 2 };
    nodeNumbers[2] =  1;
    int domain3[] = { 0, 2, 3 };
    nodeNumbers[3] =  3;
    int domain4[] = { 3, 4 };
    nodeNumbers[4] =  2;
    int domain5[] = { 3, 5 };
    nodeNumbers[5] =  2;
    int domain6[] = { 0, 2, 6 };
    nodeNumbers[6] =  3;
    int domain7[] = { 7 };
    nodeNumbers[7] =  1;
    int domain8[] = { 8 };
    nodeNumbers[8] =  1;
    int domain9[] = { 9 };
    nodeNumbers[9] =  1;
    int domain10[] = { 10 };
    nodeNumbers[10] =  1;
    int domain11[] = { 11 };
    nodeNumbers[11] =  1;
    int domain12[] = { 11, 12 };
    nodeNumbers[12] =  2;
    int domain13[] = { 10, 12, 13 };
    nodeNumbers[13] =  3;
    int domain14[] = { 8, 9, 13, 14 };
    nodeNumbers[14] =  4;
    int domain15[] = { 8, 14, 15 };
    nodeNumbers[15] =  3;
    int domain16[] = { 15, 16 };
    nodeNumbers[16] =  2;
    int domain17[] = { 17 };
    nodeNumbers[17] =  1;
    int domain18[] = { 18 };
    nodeNumbers[18] =  1;
    int domain19[] = { 15, 18, 19 };
    nodeNumbers[19] =  3;
    int domain20[] = { 20 };
    nodeNumbers[20] =  1;
    int domain21[] = { 8, 20, 21 };
    nodeNumbers[21] =  3;
    int domain22[] = { 19, 21, 22 };
    nodeNumbers[22] =  3;
    int domain23[] = { 23 };
    nodeNumbers[23] =  1;
    int domain24[] = { 23, 24 };
    nodeNumbers[24] =  2;
    int domain25[] = { 16, 17, 22, 24, 25 };
    nodeNumbers[25] =  5;
    int domain26[] = { 25, 26 };
    nodeNumbers[26] =  2;
    int domain27[] = { 7, 26, 27 };
    nodeNumbers[27] =  3;
    int domain28[] = { 28 };
    nodeNumbers[28] =  1;
    int domain29[] = { 26, 28, 29 };
    nodeNumbers[29] =  3;
    int domain30[] = { 26, 28, 30 };
    nodeNumbers[30] =  3;
    int domain31[] = { 14, 16, 31 };
    nodeNumbers[31] =  3;
    int domain32[] = { 8, 14, 32 };
    nodeNumbers[32] =  3;
    int domain33[] = { 20, 33 };
    nodeNumbers[33] =  2;
    int domain34[] = { 8, 9, 13, 34 };
    nodeNumbers[34] =  4;
    int domain35[] = { 6, 26, 35 };
    nodeNumbers[35] =  3;
    int domain36[] = { 24, 35, 36 };
    nodeNumbers[36] =  3;
    
    int *domains[] = { domain0, domain1, domain2, domain3, domain4,
	domain5, domain6, domain7, domain8, domain9,
	domain10, domain11, domain12, domain13, domain14,
	domain15, domain16, domain17, domain18, domain19,
	domain20, domain21, domain22, domain23, domain24,
	domain25, domain26, domain27, domain28, domain29,
	domain30, domain31, domain32, domain33, domain34,
	domain35, domain36};
    
    pBNet->AllocFactors();
    
    for( i = 0; i < nnodes; i++ )
    {
	myParams[i] = CTabularCPD::Create( domains[i], nodeNumbers[i], pMD);
    }
    
    // data creation for all CPDs of the model
    float data0[] = {0.050000f, 0.950000f};
    float data1[] = {0.900000f, 0.100000f, 0.010000f, 0.990000f};
    float data2[] = {0.200000f, 0.800000f};
    float data3[] = {0.950000f, 0.040000f, 0.010000f, 0.980000f, 0.010000f, 0.010000f, 0.010000f, 0.090000f, 0.900000f, 0.050000f, 0.900000f, 0.050000f};
    float data4[] = {0.950000f, 0.040000f, 0.010000f, 0.040000f, 0.950000f, 0.010000f, 0.010000f, 0.290000f, 0.700000f};
    float data5[] = {0.950000f, 0.040000f, 0.010000f, 0.040000f, 0.950000f, 0.010000f, 0.010000f, 0.040000f, 0.950000f};
    float data6[] = {0.980000f, 0.010000f, 0.010000f, 0.950000f, 0.040000f, 0.010000f, 0.500000f, 0.490000f, 0.010000f, 0.050000f, 0.900000f, 0.050000f};
    float data7[] = {0.050000f, 0.950000f};
    float data8[] = {0.920000f, 0.030000f, 0.050000f};
    float data9[] = {0.040000f, 0.960000f};
    float data10[] = {0.100000f, 0.900000f};
    float data11[] = {0.050000f, 0.900000f, 0.050000f};
    float data12[] = {0.050000f, 0.930000f, 0.010000f, 0.010000f, 0.050000f, 0.010000f, 0.930000f, 0.010000f, 0.050000f, 0.010000f, 0.010000f, 0.930000f};
    float data13[] = {0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f};
    float data14[] = {0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.300000f, 0.680000f, 0.010000f, 0.010000f, 0.950000f, 0.030000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.950000f, 0.030000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.500000f, 0.480000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.400000f, 0.580000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.300000f, 0.680000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f};
    float data15[] = {0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.030000f, 0.950000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.940000f, 0.040000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.880000f, 0.100000f, 0.010000f};
    float data16[] = {0.010000f, 0.010000f, 0.980000f, 0.010000f, 0.010000f, 0.980000f, 0.040000f, 0.920000f, 0.040000f, 0.900000f, 0.090000f, 0.010000f};
    float data17[] = {0.100000f, 0.900000f};
    float data18[] = {0.050000f, 0.950000f};
    float data19[] = {1.000000f, 0.000000f, 0.000000f, 0.950000f, 0.040000f, 0.010000f, 1.000000f, 0.000000f, 0.000000f, 0.010000f, 0.950000f, 0.040000f, 0.990000f, 0.010000f, 0.000000f, 0.950000f, 0.040000f, 0.010000f, 0.950000f, 0.040000f, 0.010000f, 0.010000f, 0.010000f, 0.980000f};
    float data20[] = {0.010000f, 0.990000f};
    float data21[] = {0.100000f, 0.900000f, 0.950000f, 0.050000f, 0.100000f, 0.900000f, 0.950000f, 0.050000f, 0.010000f, 0.990000f, 0.050000f, 0.950000f};
    float data22[] = {0.980000f, 0.010000f, 0.010000f, 0.980000f, 0.010000f, 0.010000f, 0.010000f, 0.980000f, 0.010000f, 0.980000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.980000f, 0.690000f, 0.300000f, 0.010000f};
    float data23[] = {0.010000f, 0.990000f};
    float data24[] = {0.980000f, 0.010000f, 0.010000f, 0.300000f, 0.400000f, 0.300000f};
    float data25[] = {0.010000f, 0.990000f, 0.010000f, 0.990000f, 0.700000f, 0.300000f, 0.010000f, 0.990000f, 0.050000f, 0.950000f, 0.700000f, 0.300000f, 0.010000f, 0.990000f, 0.050000f, 0.950000f, 0.950000f, 0.050000f, 0.010000f, 0.990000f, 0.050000f, 0.950000f, 0.700000f, 0.300000f, 0.010000f, 0.990000f, 0.050000f, 0.950000f, 0.950000f, 0.050000f, 0.050000f, 0.950000f, 0.050000f, 0.950000f, 0.950000f, 0.050000f, 0.010000f, 0.990000f, 0.010000f, 0.990000f, 0.700000f, 0.300000f, 0.010000f, 0.990000f, 0.050000f, 0.950000f, 0.700000f, 0.300000f, 0.010000f, 0.990000f, 0.050000f, 0.950000f, 0.990000f, 0.010000f, 0.010000f, 0.990000f, 0.050000f, 0.950000f, 0.700000f, 0.300000f, 0.010000f, 0.990000f, 0.050000f, 0.950000f, 0.990000f, 0.010000f, 0.050000f, 0.950000f, 0.050000f, 0.950000f, 0.990000f, 0.010000f, 0.010000f, 0.990000f, 0.010000f, 0.990000f, 0.100000f, 0.900000f, 0.010000f, 0.990000f, 0.010000f, 0.990000f, 0.100000f, 0.900000f, 0.010000f, 0.990000f, 0.010000f, 0.990000f, 0.300000f, 0.700000f, 0.010000f, 0.990000f, 0.010000f, 0.990000f, 0.100000f, 0.900000f, 0.010000f, 0.990000f, 0.010000f, 0.990000f, 0.300000f, 0.700000f, 0.010000f, 0.990000f, 0.010000f, 0.990000f, 0.300000f, 0.700000f};
    float data26[] = {0.050000f, 0.900000f, 0.050000f, 0.010000f, 0.090000f, 0.900000f};
    float data27[] = {0.980000f, 0.010000f, 0.010000f, 0.300000f, 0.400000f, 0.300000f, 0.010000f, 0.980000f, 0.010000f, 0.400000f, 0.590000f, 0.010000f, 0.980000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.980000f};
    float data28[] = {0.100000f, 0.900000f};
    float data29[] = {0.333333f, 0.333333f, 0.333333f, 0.010000f, 0.980000f, 0.010000f, 0.980000f, 0.010000f, 0.010000f, 0.333333f, 0.333333f, 0.333333f, 0.333333f, 0.333333f, 0.333333f, 0.010000f, 0.010000f, 0.980000f};
    float data30[] = {0.333333f, 0.333333f, 0.333333f, 0.010000f, 0.980000f, 0.010000f, 0.980000f, 0.010000f, 0.010000f, 0.333333f, 0.333333f, 0.333333f, 0.333333f, 0.333333f, 0.333333f, 0.010000f, 0.010000f, 0.980000f};
    float data31[] = {0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f};
    float data32[] = {0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.500000f, 0.480000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.500000f, 0.480000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.600000f, 0.380000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f};
    float data33[] = {0.010000f, 0.190000f, 0.800000f, 0.050000f, 0.900000f, 0.050000f};
    float data34[] = {0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.050000f, 0.250000f, 0.250000f, 0.450000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.200000f, 0.750000f, 0.040000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.290000f, 0.300000f, 0.400000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.900000f, 0.080000f, 0.010000f, 0.010000f, 0.300000f, 0.490000f, 0.200000f, 0.010000f, 0.150000f, 0.250000f, 0.590000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.200000f, 0.700000f, 0.090000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.080000f, 0.900000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.380000f, 0.600000f, 0.010000f, 0.010000f, 0.080000f, 0.900000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.010000f, 0.970000f, 0.010000f, 0.010000f, 0.010000f, 0.100000f, 0.840000f, 0.050000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f, 0.400000f, 0.580000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.010000f, 0.970000f};
    float data35[] = {0.980000f, 0.010000f, 0.010000f, 0.950000f, 0.040000f, 0.010000f, 0.800000f, 0.190000f, 0.010000f, 0.950000f, 0.040000f, 0.010000f, 0.040000f, 0.950000f, 0.010000f, 0.010000f, 0.040000f, 0.950000f, 0.300000f, 0.690000f, 0.010000f, 0.010000f, 0.300000f, 0.690000f, 0.010000f, 0.010000f, 0.980000f};
    float data36[] = {0.980000f, 0.010000f, 0.010000f, 0.980000f, 0.010000f, 0.010000f, 0.900000f, 0.090000f, 0.010000f, 0.980000f, 0.010000f, 0.010000f, 0.100000f, 0.850000f, 0.050000f, 0.050000f, 0.200000f, 0.750000f, 0.300000f, 0.600000f, 0.100000f, 0.050000f, 0.400000f, 0.550000f, 0.010000f, 0.090000f, 0.900000f};
    float *data[] = { data0, data1, data2, data3, data4,
	data5, data6, data7, data8, data9,
	data10, data11, data12, data13, data14,
	data15, data16, data17, data18, data19,
	data20, data21, data22, data23, data24,
	data25, data26, data27, data28, data29,
	data30, data31, data32, data33, data34,
	data35, data36};
    
    for( i = 0; i < nnodes; i++ )
    {
	myParams[i]->AllocMatrix(data[i], matTable);
	pBNet->AttachFactor(myParams[i]);
    }
    
    delete [] nodeTypes;
    delete[] nodeAssociation;

    CContextPersistence xmlContext;
    xmlContext.Put(pBNet, "MyModel");
    if(!xmlContext.SaveAsXML("myFavoriteObjects.xml"))
    {
	fprintf(stderr, "Persistence: error!\n");
	// something goes wrong – can’t create
    }
    else
    {
	fprintf(stderr, "Persistence: good!\n");
    }
    
    return pBNet;
}

int main()
{
    int i, j;
    CBNet* pAlarm = CreateAlarmBNet();
    const int nnodes = pAlarm->GetNumberOfNodes();
    pAlarm->GetGraph()->Dump();
    
    CGraph *graph = CGraph::Create( nnodes, NULL, NULL, NULL );	
    for(i=0; i<nnodes-1; i++)
    {
	graph->AddEdge(i, i+1, 1);
    }
    
    
    CBNet *bnet = CBNet::CreateWithRandomMatrices( graph, pAlarm->GetModelDomain() );
    
      
    int nEv = 50;
  
    
    CEvidence **pEvidences = new CEvidence *[nEv];
    int dataSize = nnodes;//summ all sizes
    
    int* obs_nodes = new int[nnodes];
    for(i=0; i<nnodes; i++)obs_nodes[i] = i;
    
    //read data from file alarm.dat to evidences
    FILE * fp = fopen("../../examples/Data/alarm.dat", "r");
    
    if( !fp && !(fp = fopen("../../c_pgmtk/examples/Data/alarm.dat", "r")) )
    {
        std::cout<<"can't open cases file"<<std::endl;
        exit(1);
    }
    
    valueVector input_data;
    input_data.resize(dataSize);
    
    for(i = 0 ; i < nEv; i++)
    {
        for (j = 0; j < dataSize ; j++)
        {
            int val;
            fscanf(fp, "%d,", &val);
            input_data[j].SetInt(val);
        }
        pEvidences[i] = CEvidence::Create(pAlarm, nnodes, obs_nodes, input_data);
    }
    
    fclose(fp);
    intVector vA;
    intVector vD;
    CMlStaticStructLearn *pLearn = CMlStaticStructLearnHC::Create(bnet, itStructLearnML, 
	StructLearnHC, BIC, 5, vA, vD, 1);
    
    pLearn->SetData(nEv, pEvidences);
    pLearn->Learn();
    //	pLearn->CreateResultBNet(const_cast<CDAG*>(pLearn->GetResultDAG()));
    //	const CBNet* pBNet = pLearn->GetResultBNet();
    ///////////////////////////////////////////////////////////////////////////////
    const CDAG* pDAG = pLearn->GetResultDAG();
    CDAG* iDAG = CDAG::Create(*(pAlarm->GetGraph()));
    int diff = iDAG->SymmetricDifference(pDAG);
    pDAG->Dump();
    delete pLearn;
    delete iDAG;
    delete[] obs_nodes;
    delete pAlarm;
    delete bnet;
   
    for( i = 0; i < nEv; i++)
    {
	delete pEvidences[i];
    }
    return 1;
}