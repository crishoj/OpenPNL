/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ASaveLoadEvidences.cpp                                      //
//                                                                         //
//  Purpose:   Test saving and loading evidences                           //
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
#include <assert.h>
#include "tCreateKjaerulffDBN.hpp"
#include "tUtil.h"


static char func_name[] = "testSaveLoadEvidences";

static char* test_desc = "Save&LoadEvidences";

static char* test_class = "Algorithm";

bool IsEvidencesEqual(const CEvidence* pEv1, const CEvidence* pEv2, float eps);

int testSaveLoadEvidences()
{
    int ret = TRS_OK;
    
    const float eps = 0.001f;
    const int nsamples = 1000;
    
    const char* argv = "testEv.txt";
    /*
    this file will be deleted automatically
    */
    
    CBNet *pBNet = pnlExCreateWasteBNet();
    
    pEvidencesVector pEv;
    pBNet->GenerateSamples( &pEv, nsamples );
    
    int ev;
    for( ev = 0; ev < nsamples; ev++ )
    {
	int numElem = pnlRand( 0, pBNet->GetNumberOfNodes() );
	int *vec = new int[numElem];
	pnlRand( numElem, vec, 0, pBNet->GetNumberOfNodes() - 1 );
	pEv[ev]->ToggleNodeState( numElem, vec );

	delete []vec;
    }
    
    if( !CEvidence::Save( argv, *((pConstEvidenceVector*)&pEv)) )
    {
	ret = TRS_FAIL;
	return trsResult( ret, ret == TRS_OK ? "No errors" : 
	"Bad test on saving - loading: can not open file for saving ");
    }
    
    pEvidencesVector pEvFromFile;
    if( !CEvidence::Load( argv, &pEvFromFile, pBNet->GetModelDomain()) )
    {
	ret = TRS_FAIL;
	return trsResult( ret, ret == TRS_OK ? "No errors" : 
	"Bad test on saving - loading: can not open file for loading ");
    }
    
    
    
    if( pEvFromFile.size() == nsamples )
    {
	int i;
	for( i = 0 ; i< nsamples; i++ )
	{
	    	    
	    if( !IsEvidencesEqual( pEv[i], pEvFromFile[i], eps ) )
	    {
		ret = TRS_FAIL;
		break;
	    }
	}
    }
    else
    {
	ret = TRS_FAIL;
    }	
    
    
    
    int i;
    for( i = 0; i < pEv.size(); i++ )
    {
	delete pEv[i];
    }
    for( i = 0; i < pEvFromFile.size(); i++ )
    {
	delete pEvFromFile[i];
    }
    delete pBNet;
    
    
    // string cmd = "del ";
    // cmd += argv;
    // system(cmd.c_str());
    
    remove(argv);

    const int maxNSeries = 500;
    const int maxNslices = 50;
    pBNet = pnlExCreateBatNetwork();
   //   pBNet = pnlExCreateRndArHMM();
    CDBN *pDBN = CDBN::Create(pBNet);
    
   
    
    intVector nSlices;
    nSlices.resize(pnlRand(1, maxNSeries));
    
    pnlRand( nSlices.size(), &nSlices.front(), 1, maxNslices );
    
    pEvidencesVecVector evVec;
    pDBN->GenerateSamples( &evVec, nSlices );
    
    for( i = 0; i < nSlices.size(); i++ )
    {
	for( ev = 0; ev < nSlices[i]; ev++ )
	{
	    int numElem = pnlRand( 0, pDBN->GetNumberOfNodes() );
	    int *vec = new int[numElem];
	    
	    pnlRand( numElem, vec, 0, pDBN->GetNumberOfNodes() - 1 );
	    
	    (evVec[i])[ev]->ToggleNodeState( numElem, vec );
	    
	    delete []vec;
	}
    }
    
    CEvidence::Save(argv, evVec);
    pEvidencesVecVector evVecFromFile;
    CEvidence::Load(argv, &evVecFromFile, pDBN->GetModelDomain());

    if( evVec.size() != evVecFromFile.size())
    {
	ret = TRS_FAIL;
    }
    else
    {
	for( i = 0; i < nSlices.size(); i++ )
	{
	    if( (evVec[i]).size() != (evVecFromFile[i]).size() )
	    {
		ret = TRS_FAIL;
		break;
	    }
	    int j;
	    for( j = 0; j < nSlices[i]; j++ )
	    {
		ret = ret && IsEvidencesEqual((evVec[i])[j],(evVecFromFile[i])[j], eps );
	    }
	}
    }
    
    for( i = 0; i < evVec.size(); i++ )
    {
	int j;
	for( j = 0; j < evVec[i].size(); j++ )
	{
	    delete (evVec[i])[j];
	}
    }
    delete pDBN;
    remove(argv);

   
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
    "Bad test on saving - loading ");
    
}

bool IsEvidencesEqual(const CEvidence *pEv1, const CEvidence* pEv2, float eps)
{
    const CModelDomain *pMD = pEv1->GetModelDomain();
    if( pMD != pEv2->GetModelDomain() )
    {
	return false;
    }
    int nnodes = pMD->GetNumberVariables();
    
    int j;
    for( j = 0; j < nnodes; j++ )
    {
	int isObs1, isObs2;
	isObs1 = pEv1->IsNodeObserved(j);
	isObs2 = pEv2->IsNodeObserved(j);
	if( isObs1 == isObs2 )
	{
	    if( isObs1 )
	    {
		const Value * val1 = pEv1->GetValue(j);
		const Value * val2 = pEv2->GetValue(j);
		const CNodeType *nt = pMD->GetVariableType(j);
		if( nt->IsDiscrete() )
		{
		    if( val1->GetInt() != val2->GetInt() )
		    {
			return false;
		    }
		}
		else
		{
		    int k;
		    for( k = 0; k < nt->GetNodeSize(); k++ )
		    {
			float delta = (float)fabs( val1[k].GetFlt() - val2[k].GetFlt() );
			if(  delta > eps )
			{
			    return false;
			}
		    }
		}
	    }
	}	
	else
	{
	    return false;
	}
    }
    return true;
}

void initASaveLoadEvidences()
{
    trsReg(func_name, test_desc, test_class, testSaveLoadEvidences);
}
