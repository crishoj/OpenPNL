/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AJTreeInfDBN.cpp                                            //
//                                                                         //
//  Purpose:   Test on 1.5 Slice Junction tree inference for DBN           //
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

// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testJTreeInfDBN";

static char* test_desc = "Provide 1.5 Slice Inf";

static char* test_class = "Algorithm";


void CreateEvidences(CDBN *pDBN, int nTimeSlices, pEvidencesVector *myEvidencesForDBN);

CEvidence*
CreateEvidenceForUnrolledDBN(CBNet *pUnrolledDBN, int nTimeSlices,
			     pEvidencesVector &evidencesForDBN);
void DefineQuery(CDBN *pDBN, int nTimeSlice,
		 intVector * queryForDBNPrior,
		 intVector * queryForDBN,
		 intVecVector *queryForUnrollBnet);
int Compare(const CPotential *pot1, const CPotential *pot2, float eps);

int CompareSmoothing(CDBN* pDBN, CDBN* pOtherDBN, int nTimeSlice, float eps);
int CompareMPE(CDBN* pDBN, CDBN* pOtherDBN, int nTimeSlice, float eps);
int CompareFiltering(CDBN* pDBN, CDBN* pOtherDBN, int nTimeSlice, float eps);
int CompareFixLagSmoothing(CDBN* pDBN, CDBN* pOtherDBN, int nTimeSlice, float eps);

#define SPARSE 1
int testJTreeInfDBN()
{
    int ret = TRS_OK;
    int seed = pnlTestRandSeed();

    std::cout<<"seed"<<seed<<std::endl;

    srand( seed );

    CDBN *pDBN = tCreateKjaerulffDBN();
    //create sparse copy ofDBN
    const CBNet* pBnet = static_cast<const CBNet*>(pDBN->GetStaticModel());


    CBNet* pSpBnet =  NULL;
    CDBN *pSpDBN = NULL;
#if SPARSE
    pSpBnet =  pBnet->ConvertToSparse();
    pSpDBN = CDBN::Create(pSpBnet);
#endif

    /////////////////////////////////////////////////////////////////////////////
    int nTimeSlice = -1;
    while(nTimeSlice <= 0)
    {
	trsiRead (&nTimeSlice, "4", "Number of slices");
    }

    float eps = -1.0f;
    while( eps <= 0 )
    {
	trssRead( &eps, "1e-2f", "accuracy in test");
    }


    int result = 1;

    result = CompareSmoothing(pDBN, pSpDBN, nTimeSlice,  eps);
    if(!result)
    {
	ret = TRS_FAIL;
    }



    result = CompareFiltering(pDBN,pSpDBN, nTimeSlice,  eps);
    if(!result)
    {
	ret = TRS_FAIL;
    }

    result = CompareFixLagSmoothing(pDBN, pSpDBN, nTimeSlice, eps);
    if(!result)
    {
	ret = TRS_FAIL;
    }
    result = CompareMPE(pDBN, pSpDBN, nTimeSlice,  eps);
    if(!result)
    {
	ret = TRS_FAIL;
    }

    delete (pDBN);
    delete (pSpDBN);
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    return trsResult( ret, ret == TRS_OK ? "No errors" :
    "Bad test on Unroll DBN");

}

void initAJTreeInfDBN()
{
    trsReg(func_name, test_desc, test_class, testJTreeInfDBN);
}


CEvidence*
CreateEvidenceForUnrolledDBN(CBNet *pUnrolledDBN, int nTimeSlice, pEvidencesVector &evidencesForDBN)

{

    int numObsNodesInDBN = evidencesForDBN[0]->GetNumberObsNodes();
    int numOfNodesInDBN = pUnrolledDBN->GetNumberOfNodes()/nTimeSlice;
    const int* obsNodes = evidencesForDBN[0]->GetAllObsNodes();

    intVector obsNodesInUnrolledDBN;
    obsNodesInUnrolledDBN.resize(numObsNodesInDBN*nTimeSlice);
    valueVector valuesForUnrolledDBN;
    valuesForUnrolledDBN.resize(numObsNodesInDBN*nTimeSlice);
    int slice;
    int i=0;
    for(slice = 0; slice < nTimeSlice; slice++)
    {

	int obsNode;
	for(obsNode = 0; obsNode < numObsNodesInDBN; obsNode++)
	{
	    obsNodesInUnrolledDBN[i] = obsNodes[obsNode] +
		slice*numOfNodesInDBN;
	    valuesForUnrolledDBN[i] = *(evidencesForDBN[slice]->GetValueBySerialNumber(obsNode));
	    i++;
	}

    }
    return  CEvidence::Create(pUnrolledDBN->GetModelDomain(),
	obsNodesInUnrolledDBN, valuesForUnrolledDBN);
}


void CreateEvidences(CDBN *pDBN, int nTimeSlices, pEvidencesVector *myEvidencesForDBN )
{


    if(myEvidencesForDBN->size())
    {
	assert(0);
    }
    /////////////////////////////////////////////////////////////////////////////
    // Create values for evidence in every slice from t=0 to t=nTimeSlice
    /////////////////////////////////////////////////////////////////////////////

    int numObsNodes;
    intVector obsNodes;
#if 1
    numObsNodes = rand()%(pDBN->GetNumberOfNodes()-1);

    if( numObsNodes <= 0)
    {
	numObsNodes = 1;
    }

    int value;
    do
    {
	do
	{
	    value = rand()%(pDBN->GetNumberOfNodes()-1);

	}while(std::find(obsNodes.begin(), obsNodes.end(), value) != obsNodes.end());

	obsNodes.push_back(value);

    }while(obsNodes.size() != numObsNodes);
#else
    numObsNodes = 2;
    obsNodes.push_back(2);
    obsNodes.push_back(6);
#endif


    int slice;
    int obsNode;
    valueVecVector values;
    CBNet *myBNet = static_cast<CBNet *>(pDBN->GetStaticModel());

    for(slice = 0; slice < nTimeSlices; slice++)
    {

	valueVector obsInSlice(numObsNodes);
	for(obsNode = 0; obsNode < obsNodes.size(); obsNode++)
	{

	    const CNodeType *nt = myBNet->GetNodeType(obsNodes[obsNode]);
	    obsInSlice[obsNode].SetInt(rand()%(nt->GetNodeSize()));

	}

	values.push_back(obsInSlice);
    }

    ////////////////////////////////////////////////////////////////////////////

    for(slice = 0; slice < nTimeSlices; slice++)
    {
	myEvidencesForDBN->push_back(CEvidence::Create(pDBN->GetModelDomain(),
	    obsNodes, values[slice]));
    }


    ////////////////////////////////////////////////////////////////////////////

}

void DefineQuery(CDBN *pDBN, int nTimeSlice,
		 intVector * queryForDBNPrior,
		 intVector * queryForDBN,
		 intVecVector *queryForUnrollBnet)
{
    ///////////////////////////////////////////////////////////////////////////
    //Define query for unrolled DBN and DBN (it is parents of some random node)
    /////////////////////////////////////////////////////////////////////////////
    int numOfNodesInDBN=pDBN->GetNumberOfNodes();
    int numOfInterfaceNodes;
    const int * interfaceNodes;
    pDBN->GetInterfaceNodes(&numOfInterfaceNodes, &interfaceNodes);
    /////////////////////////////////////////////////////////////////////////////
    int node = (int)(rand()%(numOfNodesInDBN-1)) ;
    /////////////////////////////////////////////////////////////////////////////
    (*queryForUnrollBnet).resize(nTimeSlice);

    /////////////////////////////////////////////////////////////////////////////
    int numOfNeighbors;
    const int * neighbors;
    const ENeighborType * types;
    /////////////////////////////////////////////////////////////////////////////

    CGraph *pPriorGraph = pDBN->CreatePriorSliceGraph();
    pPriorGraph->GetNeighbors(node, &numOfNeighbors, &neighbors, &types);
    int i;
    for(i = 0; i < numOfNeighbors; i++)
    {
	if(types[i] == ntParent)
	{
	    (*queryForDBNPrior).push_back(neighbors[i]);
	    (*queryForUnrollBnet)[0].push_back(neighbors[i]);
	}

    }
    (*queryForDBNPrior).push_back(node);
    (*queryForUnrollBnet)[0].push_back(node);
    delete(pPriorGraph);

    //////////////////////////////////////////////////////////////////////////////
    node +=  numOfNodesInDBN;
    pDBN->GetGraph()->GetNeighbors(node, &numOfNeighbors, &neighbors, &types);


    int slice;

    for(i = 0; i < numOfNeighbors; i++)
    {
	if(types[i] == ntParent)
	{
	    (*queryForDBN).push_back(neighbors[i]);
	    for(slice = 0; slice < nTimeSlice-1; slice++)
	    {
		(*queryForUnrollBnet)[slice+1].push_back(neighbors[i] +
		    slice*numOfNodesInDBN);

	    }
	}

    }
    (*queryForDBN).push_back(node);


    for(slice = 0; slice < nTimeSlice - 1; slice++)
    {
	(*queryForUnrollBnet)[slice+1].push_back(node + slice*numOfNodesInDBN);

    }


}

int Compare(const CPotential *pot1, const CPotential *pot2, float eps)
{


    const float *data1;
    int data_length;
    const float *data2;
    int data_length1;

    CMatrix<float>* mat = pot1->GetMatrix(matTable);
    CDenseMatrix<float>* matDense = mat->ConvertToDense();
    matDense->GetRawData(&data_length, &data1);
    CMatrix<float>* mat1 = pot2->GetMatrix(matTable);
    CDenseMatrix<float>* mat1Dense = mat1->ConvertToDense();
    mat1Dense->GetRawData(&data_length1, &data2);
    int result = pot1-> IsFactorsDistribFunEqual(pot2, eps);
    delete matDense;
    delete mat1Dense;
    return result;


}


/////////////////////////////////////////////////////////////////////////////
int CompareSmoothing(CDBN* pDBN, CDBN* pOtherDBN,int nTimeSlice, float eps)
{
    
    CBNet * pUnrolledDBN;
    pUnrolledDBN = static_cast<CBNet *>(pDBN->UnrollDynamicModel(nTimeSlice));
    
    CBNet* pUnrollOther = NULL;
    if( pOtherDBN )
    {
	pUnrollOther = static_cast<CBNet*>( pOtherDBN->UnrollDynamicModel(nTimeSlice) );
    }
    
    
    int result = 0;
    /////////////////////////////////////////////////////////////////////////////
    //Create inference for unrolled DBN
    ////////////////////////////////////////////////////////////////////////////

    pEvidencesVector myEvidencesForDBN;
    pEvidencesVector myEvidencesForDBNOther;
    CEvidence *myEvidenceForUnrolledDBN = NULL;
    CEvidence *myEvidenceForUnrolledDBNOther = NULL;

    CreateEvidences( pDBN, nTimeSlice, &myEvidencesForDBN );
    myEvidenceForUnrolledDBN = CreateEvidenceForUnrolledDBN(pUnrolledDBN,nTimeSlice, myEvidencesForDBN );

    CJtreeInfEngine *pUnrolJTree = CJtreeInfEngine::Create(pUnrolledDBN);
    CJtreeInfEngine* pUnrollOtherJTree = NULL;
    if( pOtherDBN )
    {
	CreateEvidences( pOtherDBN, nTimeSlice, &myEvidencesForDBNOther );

	myEvidenceForUnrolledDBNOther =  CreateEvidenceForUnrolledDBN(pUnrollOther, nTimeSlice, myEvidencesForDBNOther);
	pUnrollOtherJTree = CJtreeInfEngine::Create(pUnrollOther);
    }

    //////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////
    //Create inference (smoothing) for DBN

    C1_5SliceJtreeInfEngine *pDynamicJTree;
    pDynamicJTree = C1_5SliceJtreeInfEngine::Create(pDBN);
    trsTimerStart(0);
    pUnrolJTree->EnterEvidence(myEvidenceForUnrolledDBN);
    pDynamicJTree->DefineProcedure(ptSmoothing, nTimeSlice);
    pDynamicJTree->EnterEvidence( &myEvidencesForDBN.front(), nTimeSlice );



    pDynamicJTree->Smoothing();

    double time = trsTimerClock(0);
    printf( "timing of smoothing procedure %f\n", time);

    C1_5SliceJtreeInfEngine *pOtherDynamicJTree = NULL;

    if( pOtherDBN )
    {

	pOtherDynamicJTree = C1_5SliceJtreeInfEngine::Create(pOtherDBN);
	pUnrollOtherJTree->EnterEvidence(myEvidenceForUnrolledDBNOther);
	pOtherDynamicJTree->DefineProcedure(ptSmoothing, nTimeSlice);
	pOtherDynamicJTree->EnterEvidence( &myEvidencesForDBNOther.front(), nTimeSlice );
	pOtherDynamicJTree->Smoothing();
    }
    /////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////
    intVector queryForDBN, queryForDBNPrior;
    intVecVector queryForUnrollBnet;
    DefineQuery(pDBN, nTimeSlice,
	&queryForDBNPrior,	&queryForDBN, &queryForUnrollBnet);

    int itogResult = TRUE;
    int slice;
    for(slice = 0; slice < nTimeSlice; slice++)
    {

	pUnrolJTree->MarginalNodes(&(queryForUnrollBnet[slice]).front(),
	    (queryForUnrollBnet[slice]).size());

	if( pUnrollOtherJTree )
	{
	    pUnrollOtherJTree->MarginalNodes(&(queryForUnrollBnet[slice]).front(),
		(queryForUnrollBnet[slice]).size());
	}

	if(slice)
	{
	    pDynamicJTree->MarginalNodes(&queryForDBN.front(),
		queryForDBN.size(), slice);
	    if( pOtherDynamicJTree )
	    {
		pOtherDynamicJTree->MarginalNodes(&queryForDBN.front(),
		    queryForDBN.size(), slice);
	    }
	}
	else
	{
	    pDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
		queryForDBNPrior.size(), slice);
	    if( pOtherDynamicJTree )
	    {
		pOtherDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
		    queryForDBNPrior.size(), slice);
	    }
	}

	const CPotential* pJPD1 = pUnrolJTree->GetQueryJPD();
	const CPotential* pJPD2 = pDynamicJTree->GetQueryJPD();
	const CPotential* pOtherJPD1 = NULL;
	const CPotential* pOtherJPD2 = NULL;
	if( pUnrollOtherJTree )
	{
	    pOtherJPD1 = pUnrollOtherJTree->GetQueryJPD();
	    //pOtherJPD1->Dump();
	}
	if( pOtherDynamicJTree )
	{
	    pOtherJPD2 = pOtherDynamicJTree->GetQueryJPD();
	    //pOtherJPD2->Dump();
	}
	result =  Compare( pJPD1, pJPD2, eps);
	if(!result)
	{
	    pJPD1->Dump();
	    pJPD2->Dump();
	    itogResult = FALSE;
	}
#if SPARSE
	result = Compare( pJPD1, pJPD2, eps );
	if(!result)
	{
	    itogResult = FALSE;
	}
	result = Compare( pOtherJPD1, pOtherJPD2, eps );
	if(!result)
	{
	    itogResult = FALSE;
	}
#endif

    }
    //CJtreeInfEngine::Release(&pUnrolJTree);
    delete pUnrolJTree;
    delete (pUnrolledDBN);
    delete (pDynamicJTree);
    //CJtreeInfEngine::Release(&pUnrollOtherJTree);
    delete pUnrollOtherJTree;
    delete pUnrollOther;
    delete pOtherDynamicJTree;

    for( slice = 0; slice < (myEvidencesForDBN).size(); slice++ )
    {
	delete myEvidencesForDBN[slice];

	if( pOtherDBN )
	{
	    delete myEvidencesForDBNOther[slice];
	}
    }

    delete myEvidenceForUnrolledDBN;
    delete myEvidenceForUnrolledDBNOther;
    return result;

}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
int CompareMPE(CDBN* pDBN, CDBN* pOtherDBN,int nTimeSlice, float eps)
{
    CBNet * pUnrolledDBN;
    pUnrolledDBN = static_cast<CBNet *>(pDBN->UnrollDynamicModel(nTimeSlice));

    CBNet* pUnrollOther = NULL;
    if( pOtherDBN )
    {
	pUnrollOther = static_cast<CBNet*>( pOtherDBN->UnrollDynamicModel(nTimeSlice) );
    }

    /////////////////////////////////////////////////////////////////////////////
    //Create inference for unrolled DBN
    ////////////////////////////////////////////////////////////////////////////

    pEvidencesVector myEvidencesForDBN;
    pEvidencesVector myEvidencesForDBNOther;
    CEvidence *myEvidenceForUnrolledDBN = NULL;
    CEvidence *myEvidenceForUnrolledDBNOther = NULL;

    CreateEvidences( pDBN, nTimeSlice, &myEvidencesForDBN );
    myEvidenceForUnrolledDBN = CreateEvidenceForUnrolledDBN(pUnrolledDBN,nTimeSlice, myEvidencesForDBN);

    CJtreeInfEngine *pUnrolJTree = CJtreeInfEngine::Create(pUnrolledDBN);
    CJtreeInfEngine* pUnrollOtherJTree = NULL;
    if( pOtherDBN )
    {
	CreateEvidences( pOtherDBN, nTimeSlice, &myEvidencesForDBNOther );
	myEvidenceForUnrolledDBNOther =
	    CreateEvidenceForUnrolledDBN(pUnrollOther,nTimeSlice, myEvidencesForDBNOther);
	pUnrollOtherJTree = CJtreeInfEngine::Create(pUnrollOther);
    }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////

    /////////////////////////////////////////////////////////////////////////////
    //Create inference (smoothing) for DBN

    C1_5SliceJtreeInfEngine *pDynamicJTree;
    pDynamicJTree = C1_5SliceJtreeInfEngine::Create(pDBN);
    pUnrolJTree->EnterEvidence(myEvidenceForUnrolledDBN, 1);
    pDynamicJTree->DefineProcedure(ptViterbi, nTimeSlice);
    pDynamicJTree->EnterEvidence( &myEvidencesForDBN.front(), nTimeSlice );
    pDynamicJTree->FindMPE();

    C1_5SliceJtreeInfEngine *pOtherDynamicJTree = NULL;

    if( pOtherDBN )
    {

	pOtherDynamicJTree = C1_5SliceJtreeInfEngine::Create(pOtherDBN);
	pUnrollOtherJTree->EnterEvidence(myEvidenceForUnrolledDBNOther, 1);
	pOtherDynamicJTree->DefineProcedure(ptViterbi, nTimeSlice);
	pOtherDynamicJTree->EnterEvidence( &myEvidencesForDBNOther.front(), nTimeSlice );
	pOtherDynamicJTree->FindMPE();
    }
    /////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////
    intVector queryForDBN, queryForDBNPrior;
    intVecVector queryForUnrollBnet;
    DefineQuery(pDBN, nTimeSlice,
	&queryForDBNPrior,	&queryForDBN, &queryForUnrollBnet);

    int itogResult = 1;
    int slice;
    for(slice = 0; slice < nTimeSlice; slice++)
    {

	pUnrolJTree->MarginalNodes(&(queryForUnrollBnet[slice]).front(),
	    (queryForUnrollBnet[slice]).size());

	if( pUnrollOtherJTree )
	{
	    pUnrollOtherJTree->MarginalNodes(&(queryForUnrollBnet[slice]).front(),
		(queryForUnrollBnet[slice]).size());
	}

	if(slice)
	{
	    pDynamicJTree->MarginalNodes(&queryForDBN.front(),
		queryForDBN.size(), slice);
	    if( pOtherDynamicJTree )
	    {
		pOtherDynamicJTree->MarginalNodes(&queryForDBN.front(),
		    queryForDBN.size(), slice);
	    }
	}
	else
	{
	    pDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
		queryForDBNPrior.size(), slice);
	    if( pOtherDynamicJTree )
	    {
		pOtherDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
		    queryForDBNPrior.size(), slice);
	    }
	}

	const CEvidence* pEv1 = pUnrolJTree->GetMPE();
	const CEvidence* pEv2 = pDynamicJTree->GetMPE();


	int nObsNodes = pEv1->GetNumberObsNodes();
	int i;
	for( i = 0; i < nObsNodes; i++)
	{
	    const int v1 = pEv1->GetValueBySerialNumber(i)->GetInt();
	    const int v2 = pEv2->GetValueBySerialNumber(i)->GetInt();
	    if (v2 != v1)
	    {
		itogResult = 0;
		break;
	    }

	}


	const CEvidence* pOtherEv1 = NULL;
	const CEvidence* pOtherEv2 = NULL;
	if( pUnrollOtherJTree )
	{
	    pOtherEv1 = pUnrollOtherJTree->GetMPE();
	}
	if( pOtherDynamicJTree )
	{
	    pOtherEv2 = pOtherDynamicJTree->GetMPE();
	}

	nObsNodes = pOtherEv1->GetNumberObsNodes();

	for( i = 0; i < nObsNodes; i++)
	{
	    const int v1 = pEv1->GetValueBySerialNumber(i)->GetInt();
	    const int v2 = pEv2->GetValueBySerialNumber(i)->GetInt();
	    if (v2 != v1)
	    {
		itogResult = 0;
		break;
	    }

	}

    }
    //CJtreeInfEngine::Release(&pUnrolJTree);
    delete pUnrolJTree;
    delete (pUnrolledDBN);
    delete (pDynamicJTree);
    //CJtreeInfEngine::Release(&pUnrollOtherJTree);
    delete pUnrollOtherJTree;
    delete pUnrollOther;
    delete pOtherDynamicJTree;

    for( slice = 0; slice < (myEvidencesForDBN).size(); slice++ )
    {
	delete myEvidencesForDBN[slice];

	if( pOtherDBN )
	{
	    delete myEvidencesForDBNOther[slice];
	}
    }

    delete myEvidenceForUnrolledDBN;
    delete myEvidenceForUnrolledDBNOther;
    return itogResult;

}
/////////////////////////////////////////////////////////////////////////////
int CompareFiltering(CDBN* pDBN,  CDBN* pOtherDBN, int nTimeSlice, float eps)
{
    
   int result = 1;


    /////////////////////////////////////////////////////////////////////////////
    //Create inference for unrolled DBN
    ////////////////////////////////////////////////////////////////////////////
    C1_5SliceJtreeInfEngine *pDynamicJTree;
    pDynamicJTree = C1_5SliceJtreeInfEngine::Create(pDBN);
    pDynamicJTree->DefineProcedure(ptFiltering);

    C1_5SliceJtreeInfEngine *pOtherDynamicJTree = NULL;

    if( pOtherDBN )
    {
	pOtherDynamicJTree = C1_5SliceJtreeInfEngine::Create(pOtherDBN);
	pOtherDynamicJTree->DefineProcedure(ptFiltering);
    }

    pEvidencesVector myEvidencesForDBN;
    pEvidencesVector myEvidencesForDBNOther;
    CEvidence *myEvidenceForUnrolledDBN = NULL;
    CEvidence *myEvidenceForUnrolledDBNOther = NULL;

    CreateEvidences( pDBN, nTimeSlice, &myEvidencesForDBN );
    if( pOtherDBN)
    {
	CreateEvidences( pOtherDBN, nTimeSlice, &myEvidencesForDBNOther );
    }
    intVector queryForDBN, queryForDBNPrior;
    intVecVector queryForUnrollBnet;
    DefineQuery(pDBN, nTimeSlice,
	&queryForDBNPrior,	&queryForDBN, &queryForUnrollBnet);


    double procedureTime = 0;
    int slice;
    for(slice = 0; slice < nTimeSlice; slice++)
    {

	CBNet *	pUnrolledOnISlice =
	    static_cast<CBNet *>(pDBN->UnrollDynamicModel(slice+1));
	CJtreeInfEngine *pUnrolJTree =
	    CJtreeInfEngine::Create(pUnrolledOnISlice);
	myEvidenceForUnrolledDBN = CreateEvidenceForUnrolledDBN(pUnrolledOnISlice, slice+1,
	    myEvidencesForDBN);

	CBNet* pOtherUnrolledOnISlice = NULL;
	CJtreeInfEngine *pOtherUnrollJTree = NULL;
	if( pOtherDBN )
	{
	    pOtherUnrolledOnISlice = static_cast<CBNet *>(
		pDBN->UnrollDynamicModel(slice+1));
	    myEvidenceForUnrolledDBNOther = CreateEvidenceForUnrolledDBN(pOtherUnrolledOnISlice, slice+1,
		myEvidencesForDBNOther);
	    pOtherUnrollJTree = CJtreeInfEngine::Create(pOtherUnrolledOnISlice);
	}


	//////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	//Create inference (smoothing) for DBN

	trsTimerStart(0);
	pDynamicJTree->EnterEvidence(&(myEvidencesForDBN[slice]), 1);
	pDynamicJTree->Filtering(slice);
	trsTimerStop(0);
	procedureTime += trsTimerSec(0);


	pUnrolJTree->EnterEvidence(myEvidenceForUnrolledDBN);

	if( pOtherDynamicJTree )
	{
	    pOtherDynamicJTree->EnterEvidence(&(myEvidencesForDBNOther[slice]), 1);
	    pOtherDynamicJTree->Filtering(slice);
	    pOtherUnrollJTree->EnterEvidence(myEvidenceForUnrolledDBNOther);
	}


	/////////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////


	pUnrolJTree->MarginalNodes(&(queryForUnrollBnet[slice]).front(),
	    (queryForUnrollBnet[slice]).size());

	if( pOtherUnrollJTree )
	{
	    pOtherUnrollJTree->MarginalNodes(&(queryForUnrollBnet[slice]).front(),
		(queryForUnrollBnet[slice]).size());
	}

	if(slice)
	{
	    pDynamicJTree->MarginalNodes(&queryForDBN.front(),
		queryForDBN.size());
	    if( pOtherDynamicJTree )
	    {
		pOtherDynamicJTree->MarginalNodes(&queryForDBN.front(),
		    queryForDBN.size());
	    }
	}
	else
	{
	    pDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
		queryForDBNPrior.size());
	    if( pOtherDynamicJTree )
	    {
		pOtherDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
		    queryForDBNPrior.size());
	    }
	}

	const CPotential* qJPD1 = pUnrolJTree->GetQueryJPD();

	const CPotential* qJPD2 = pDynamicJTree->GetQueryJPD();
	//qJPD1->Dump();
	//qJPD2->Dump();

	const CPotential* pOtherJPD1 = NULL;
	const CPotential* pOtherJPD2 = NULL;
	if( pOtherUnrollJTree )
	{
	    pOtherJPD1 = pOtherUnrollJTree->GetQueryJPD();
	}
	if( pOtherDynamicJTree )
	{
	    pOtherJPD2 = pOtherDynamicJTree->GetQueryJPD();
	}
	int res = Compare( qJPD1, qJPD2, eps);
	result = result&&res;
#if SPARSE
	res = Compare( pOtherJPD2, pOtherJPD1, eps );
	result = result&&res;

#endif
        delete pUnrolJTree;
	delete (pUnrolledOnISlice);
        delete pOtherUnrollJTree;
	delete pOtherUnrolledOnISlice;


	delete myEvidenceForUnrolledDBN;
	delete myEvidenceForUnrolledDBNOther;
	
	}


	for( slice = 0; slice < (myEvidencesForDBN).size(); slice++ )
	{
	    delete myEvidencesForDBN[slice];
	    if(pOtherDBN)
	    {
		delete myEvidencesForDBNOther[slice];
	    }
	}
	delete pDynamicJTree;
	delete pOtherDynamicJTree;
	printf( "timing of filtering procedure %fn", procedureTime);

	return result;

}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int CompareFixLagSmoothing(CDBN* pDBN, CDBN* pOtherDBN, int nTimeSlice, float eps)
{

    pEvidencesVector myEvidencesForDBN;
    pEvidencesVector myEvidencesForDBNOther;

    CEvidence *myEvidenceForUnrolledDBN = NULL;
    CEvidence *myEvidenceForUnrolledDBNOther = NULL;


    int itogResult = 1;
    int result = 0;

    int lag;
    if( nTimeSlice -1)
    {
	lag  = rand()%(nTimeSlice-1);
    }
    else
    {
	lag = 0;
    }
    //lag = 2;

    /////////////////////////////////////////////////////////////////////////////
    //Create inference for unrolled DBN
    ////////////////////////////////////////////////////////////////////////////
    C1_5SliceJtreeInfEngine *pDynamicJTree = NULL;
    pDynamicJTree = C1_5SliceJtreeInfEngine::Create(pDBN);
    pDynamicJTree->DefineProcedure(ptFixLagSmoothing, lag);
    CreateEvidences(pDBN, nTimeSlice, &myEvidencesForDBN );

    C1_5SliceJtreeInfEngine *pOtherDynamicJTree = NULL;

    if( pOtherDBN )
    {
	pOtherDynamicJTree = C1_5SliceJtreeInfEngine::Create(pOtherDBN);
	pOtherDynamicJTree->DefineProcedure(ptFixLagSmoothing, lag);
	CreateEvidences(pOtherDBN, nTimeSlice, &myEvidencesForDBNOther );

    }


    intVector queryForDBN, queryForDBNPrior;
    intVecVector queryForUnrollBnet;

    DefineQuery(pDBN, nTimeSlice,
	&queryForDBNPrior,	&queryForDBN, &queryForUnrollBnet);


    int slice;
    for(slice = 0; slice < nTimeSlice; slice++)
    {

	//////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	//Create inference (smoothing) for DBN
	pDynamicJTree->EnterEvidence(&(myEvidencesForDBN[slice]), 1);
	if( pOtherDynamicJTree )
	{
	    pOtherDynamicJTree->EnterEvidence(&(myEvidencesForDBNOther[slice]), 1);
	}

	if(slice >= lag)
	{
	    pDynamicJTree->FixLagSmoothing(slice);
	    CBNet *	pUnrolledOnISlice =
		static_cast<CBNet *>(pDBN->UnrollDynamicModel(slice+1));

	    CJtreeInfEngine *pUnrolJTree =
		CJtreeInfEngine::Create(pUnrolledOnISlice);
	    myEvidenceForUnrolledDBN = CreateEvidenceForUnrolledDBN(pUnrolledOnISlice, slice+1, myEvidencesForDBN);

	    pUnrolJTree->EnterEvidence(myEvidenceForUnrolledDBN);

	    pUnrolJTree->MarginalNodes(&(queryForUnrollBnet[slice-lag]).front(),
		(queryForUnrollBnet[slice-lag]).size());

	    CBNet *pOtherUnrolledOnISlice = NULL;
	    CJtreeInfEngine* pOtherUnrollJTree = NULL;
	    if( pOtherDBN )
	    {
		pOtherDynamicJTree->FixLagSmoothing(slice);
		pOtherUnrolledOnISlice = static_cast<CBNet*>(
		    pOtherDBN->UnrollDynamicModel(slice+1) );
		pOtherUnrollJTree = CJtreeInfEngine::Create( pOtherUnrolledOnISlice );
		myEvidenceForUnrolledDBNOther = CreateEvidenceForUnrolledDBN(pOtherUnrolledOnISlice, slice+1, myEvidencesForDBNOther);
		pOtherUnrollJTree->EnterEvidence(myEvidenceForUnrolledDBNOther);

		pOtherUnrollJTree->MarginalNodes(&(queryForUnrollBnet[slice-lag]).front(),
		    (queryForUnrollBnet[slice-lag]).size());
	    }


	    if(slice-lag)
	    {
		pDynamicJTree->MarginalNodes(&queryForDBN.front(),
		    queryForDBN.size());
		if( pOtherDBN )
		{
		    pOtherDynamicJTree->MarginalNodes(&queryForDBN.front(),
			queryForDBN.size());
		}

	    }
	    else
	    {
		pDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
		    queryForDBNPrior.size());
		if( pOtherDBN )
		{
		    pOtherDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
			queryForDBNPrior.size());
		}
	    }
	    const CPotential* qJPD1 = pUnrolJTree->GetQueryJPD();
	    const CPotential* qJPD2 = pDynamicJTree->GetQueryJPD();
	    const CPotential* pOtherJPD1 = NULL;
	    const CPotential* pOtherJPD2 = NULL;
	    if( pOtherUnrollJTree )
	    {
		pOtherJPD1 = pOtherUnrollJTree->GetQueryJPD();
	    }
	    if( pOtherDynamicJTree )
	    {
		pOtherJPD2 = pOtherDynamicJTree->GetQueryJPD();
	    }
	    result =  Compare( qJPD1, qJPD2, eps);
	    if(!result)
	    {
		itogResult = FALSE;
	    }
#if SPARSE
	    result = Compare( pOtherJPD2, pOtherJPD1, eps );
	    if(!result)
	    {
		pOtherJPD2->Dump();
		pOtherJPD1->Dump();
		itogResult = FALSE;
	    }

#endif

	    delete pOtherUnrollJTree;
            delete pUnrolJTree;
	    delete pOtherUnrolledOnISlice;
	    delete pUnrolledOnISlice;
	    delete myEvidenceForUnrolledDBN;
	    delete myEvidenceForUnrolledDBNOther;
	}
    }

    delete pOtherDynamicJTree;
    delete pDynamicJTree;
    for( slice = 0; slice < (myEvidencesForDBN).size(); slice++ )
    {
	delete myEvidencesForDBN[slice];
	if(pOtherDBN)
	{
	    delete myEvidencesForDBNOther[slice];
	}
    }
    

    return itogResult;

}
/////////////////////////////////////////////////////////////////////////////
