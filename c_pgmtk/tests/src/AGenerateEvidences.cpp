/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AGenerateEvidences.cpp                                      //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <algorithm>
#include "tCreateKjaerulffDBN.hpp"
#include "tCreateArHMMwithGaussObs.hpp"
#include "tCreateIncineratorBNet.hpp"

// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testGenerateEvidences";

static char* test_desc = "Provide learning using created evidences";

static char* test_class = "Algorithm";
CEvidence* delete_ptr(CEvidence *pEv)
{
    PNL_USING;
    delete pEv;
    return NULL;
}

int testGenerateEvidences()
{
    PNL_USING;
    int ret = TRS_OK;
    int seed = pnlTestRandSeed();

    std::cout<<"seed = "<<seed<<std::endl;
    int nEvidencesForMixBNet = -1;
    while( nEvidencesForMixBNet <= 99 )
    {
	trsiRead (&nEvidencesForMixBNet, "1000", "Number of evidences  for Mixture BNEt(not less then 1000)");
    }
    int nEvidencesForBNet = -1;
    while( nEvidencesForBNet <= 14999 )
    {
	trsiRead (&nEvidencesForBNet, "15000", "Number of evidences for Gauss BNet (not less then 10000)");
    }
    int nEvidencesForDBN = -1;
    while( nEvidencesForDBN <= 99 )
    {
	trsiRead (&nEvidencesForDBN, "100", "Number of evidences fro DBN (not less then 100)");
    }


    float eps = -1.0f;

    while( eps <= 0 )
    {
	trssRead( &eps, "1.5e-1f", "accuracy in test");
    }

    int i;
   
    const CBNet* pBNet = tCreateIncineratorBNet();
    std::cout<<"BNet creation done \n";

    pEvidencesVector evidences;


    pBNet->GenerateSamples( &evidences, nEvidencesForBNet );
    std::cout<<"evidences for BNet created \n";

    CBNet *pBNetToLearn = CBNet::Copy(pBNet);

    CEMLearningEngine *pLearn;
    pLearn = CEMLearningEngine::Create(pBNetToLearn);
    std::cout<<"Learning creation done \n";

    pLearn->SetData( nEvidencesForBNet, &evidences.front() );
    std::cout<<"Set data done \n";
    pLearn->Learn();
    std::cout<<"Learning BNet creation done \n";

    int nnodes = pBNet->GetNumberOfNodes();

    for( i = 0; i < nnodes; i++ )
    {

	if( !pBNet->GetFactor( i )->
	    IsFactorsDistribFunEqual( pBNetToLearn->GetFactor( i ), eps ) )
	{
	    ret = TRS_FAIL;
	    return trsResult( ret, ret == TRS_OK ? "No errors" :
	    "Bad test on CreateEvidences for BNet");
	}
    }

    for( i = 0; i < nEvidencesForBNet; i++ )
    {
	delete evidences[i];
    }
    std::cout<<"deletint evidences done \n";
    delete pBNet;
    delete pBNetToLearn;
    delete pLearn;

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    CDBN *pDBN = tCreateArHMMwithGaussObs();
    std::cout<<"DBN creation done \n";

    CDBN* pDBNtoLearn =
	CDBN::Create(static_cast<CBNet*> (pDBN->UnrollDynamicModel(2)));
    std::cout<<"DBN to learn creation done \n";
    intVector nslices(nEvidencesForDBN);
    for( i = 0; i < nslices.size(); i++)
    {
	nslices[i] = pnlRand(1, 40);
    }
    pEvidencesVecVector evidencesForDBN;
    pDBN->GenerateSamples( &evidencesForDBN, nslices );
    std::cout<<"Evidences creation done \n";


    CEMLearningEngineDBN *pLearnDBN = CEMLearningEngineDBN::Create(pDBNtoLearn);
    std::cout<<"Learning engine creation done \n";
    pLearnDBN->SetData(evidencesForDBN);
    std::cout<<"Data setting done \n";
    pLearnDBN->Learn();
    std::cout<<"Learning DBN done \n";

    int nnodesInDBN = pDBN->GetNumberOfNodes();
    for( i = 0; i < 2*nnodesInDBN; i++ )
    {

	if( !pDBN->GetFactor( i )->
	    IsFactorsDistribFunEqual( pDBNtoLearn->GetFactor( i ), eps ) )
	{
	    ret = TRS_FAIL;
	    return trsResult( ret, ret == TRS_OK ? "No errors" :
	    "Bad test on CreateEvidences for DBN");
	}
    }

    for( i = 0; i < nEvidencesForDBN; i++ )
    {
	int j;
	for( j = 0; j < nslices[i]; j++ )
	{
		delete (evidencesForDBN[i])[j];
	}
    }
    std::cout<<"Evidences deletion done \n";
    delete pDBN;
    delete pDBNtoLearn;
    delete pLearnDBN;
   /////////////////////////////////////////////////////////////////////////

    return trsResult( ret, ret == TRS_OK ? "No errors"
	: "Bad test on CreateEvidences");
}

void initAGenerateEvidences()
{
    trsReg(func_name, test_desc, test_class, testGenerateEvidences);
}

