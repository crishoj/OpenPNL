/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGaussianFactorOperations.cpp                               //
//                                                                         //
//  Purpose:   Test on implementation of Gaussian Potential&CPD algorithms //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <float.h>
PNL_USING

static char func_name[] = "testGaussianFactorOperations";

static char* test_desc = "Create Gaussian factor(s) and provide operations";

static char* test_class = "Algorithm";

int testGaussianFactorOperations()
{
    int ret = TRS_OK;
    int i=0;

    nodeTypeVector nodeTypes;
    nodeTypes.assign( 3, CNodeType() );
    intVector NodeAssociate = intVector( 3 );

    for( i = 0; i < 3; i++ )
    {
	nodeTypes[i].SetType( 0, i + 1 );
        NodeAssociate[i] = i;
    }

    CModelDomain* pMD = CModelDomain::Create( nodeTypes, NodeAssociate );

    float smDataM0[] = { 0.1f, 0.2f, 0.3f };
    float smDataM1[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    float *smDataM[] = { smDataM0, smDataM1 };

    float smDataCov0[] = {2.0f, 3.0f, 4.0f, 3.0f, 6.0f, 7.0f, 4.0f, 7.0f, 1.0f	};
    float smDataCov1[] = { 7.4f, 7.5f, 7.6f, 7.4f, 7.3f, 7.5f, 7.2f, 7.3f, 7.3f, 7.5f, 7.6f, 7.3f, 7.8f, 7.1f, 7.1f, 7.4f, 7.3f, 7.1f, 7.1f, 7.6f, 7.3f, 7.5f, 7.1f, 7.6f, 7.3f  };
    float *smDataCov[] = { smDataCov0, smDataCov1};

    float smNorms[] = { 1.0f, 1.0f };

    float otherDataM0[] = { 1.1f, 1.2f, 1.3f };
    float otherDataM1[] = { 6.0f, 7.0f, 8.0f, 9.0f, 10.0f };
    float *otherDataM[] = { otherDataM0, otherDataM1 };

    float otherDataCov0[] = { 3.0f, 4.0f, 4.0f, 4.0f, 5.0f, 6.0f, 4.0f, 6.0f, 7.0f	};
    float otherDataCov1[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 2.0f, 6.0f, 7.0f, 8.0f, 9.0f, 3.0f, 7.0f, 10.0f, 11.0f, 12.0f, 4.0f, 8.0f, 11.0f, 13.0f, 14.0f, 5.0f, 9.0f, 12.0f, 14.0f, 15.0f  };
    float *otherDataCov[] = { otherDataCov0, otherDataCov1};

    float otherNorms[] = { 2.0f, 2.0f };


    intVector domain( 2,0 );

    CPotential **mySmallParams = new CPotential*[2];
    CPotential **myOtherParams = new CPotential*[2];

    for(i=0; i<2; i++)
    {
	    domain[0] = i;
	    domain[1] = i+1;
	    mySmallParams[i] = CGaussianPotential::Create( &domain.front(), 2, pMD );
        static_cast<CGaussianPotential*>(mySmallParams[i])->SetCoefficient(
                smNorms[i], 1 );
	    mySmallParams[i]->AllocMatrix(smDataM[i], matH);
	    mySmallParams[i]->AllocMatrix(smDataCov[i], matK);
	    
	    //creat other model
	    myOtherParams[i] = CGaussianPotential::Create( &domain.front(), 2, pMD );
        static_cast<CGaussianPotential*>(myOtherParams[i])->SetCoefficient(
                otherNorms[i], 1 );
	    myOtherParams[i]->AllocMatrix(otherDataM[i], matH);
	    myOtherParams[i]->AllocMatrix(otherDataCov[i], matK);
	    
    }
    for( i = 0; i < 2; i++ )
    {
	    (*myOtherParams[i]) = (*mySmallParams[i]);
    }
    float dataMean1[] = { 2.0f, 6.0f };
    float dataCov1[] = {  1.0f, 1.0f, 1.0f, 3.0f };
    float norm = 2.0f;

    int numNode = 1;
    CPotential* oneNodeFactor = CGaussianPotential::Create( &numNode, 1, pMD);
    static_cast<CGaussianPotential*>(oneNodeFactor)->SetCoefficient( norm, 1 );
    oneNodeFactor->AllocMatrix( dataMean1, matH );
    oneNodeFactor->AllocMatrix( dataCov1, matK );

    int size;
    const int *bigDom;
    mySmallParams[0]->GetDomain( &size, &bigDom );
    //create an object to multiply
    CPotential * multResFact = CGaussianPotential::Create( bigDom, size, pMD );
    multResFact->TieDistribFun(mySmallParams[0]);
    (*multResFact) *= (*oneNodeFactor);
    //create an object to divide
    CPotential * divResFact = CGaussianPotential::Create( bigDom, size, pMD );
    divResFact->TieDistribFun(mySmallParams[0]);
    (*divResFact)/=(*oneNodeFactor);

    delete []myOtherParams;
    delete []mySmallParams;
    delete oneNodeFactor;
    delete multResFact;
    delete divResFact;
    delete pMD;
    return trsResult( ret, ret == TRS_OK ? "No errors"
	: "Bad test on GaussianInf");
}

void initAGaussianFactorOperations()
{
    trsReg(func_name, test_desc, test_class, testGaussianFactorOperations);
}