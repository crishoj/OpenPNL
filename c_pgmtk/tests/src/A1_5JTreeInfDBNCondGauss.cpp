/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      A1_5JTreeInfDBNCondGauss.cpp                                //
//                                                                         //
//  Purpose:   Test on 1.5 Slice Junction tree inference for DBN           //
//             with conditional gaussian distribution                      //
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
#include "tCreateArHMMwithGaussObs.hpp"

// this file must be last included file
#include "tUtil.h"
PNL_USING
static char func_name[] = "test1_5JTreeInfDBNCondGauss";

static char* test_desc = "Provide 1.5 Slice Inf";

static char* test_class = "Algorithm";


void CreateEvidencesArHMM(CDBN *pDBN, int nTimeSlices,
			  pEvidencesVector *EvidencesForDBN );
CEvidence * CreateEvidenceForUnrolledArHMM(CBNet *pUnrolledDBN, int nTimeSlices,
					   pEvidencesVector &evidencesForDBN );
void DefineQueryArHMM(CDBN *pDBN, int nTimeSlice,
		      intVector * queryForDBNPrior,
		      intVector * queryForDBN,
		      intVecVector *queryForUnrollBnet);
int CompareArHMM(const CPotential *pot1, const CPotential *pot2, float eps);

int CompareSmoothingArHMM(CDBN* pDBN, int nTimeSlice, float eps);
int CompareViterbyArHMM(CDBN* pDBN, int nTimeSlice, float eps);
int CompareFilteringArHMM(CDBN* pDBN, int nTimeSlice, float eps);
int CompareFixLagSmoothingArHMM(CDBN* pDBN, int nTimeSlice, float eps);

int test1_5JTreeInfDBNCondGauss()
{
    

    int ret = TRS_OK;
    int seed = pnlTestRandSeed();
    std::cout<<"seed"<<seed<<std::endl;

    srand( seed );
    CDBN *pDBN = tCreateArHMMwithGaussObs();


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
    result = CompareSmoothingArHMM( pDBN, nTimeSlice,  eps );
    if( !result )
    {
	ret = TRS_FAIL;
    }

    result = CompareFilteringArHMM( pDBN, nTimeSlice,  eps );

    if( !result )
    {
	ret = TRS_FAIL;
    }

    result = CompareFixLagSmoothingArHMM( pDBN, nTimeSlice,  eps );
    if( !result )
    {
	ret = TRS_FAIL;
    }
    result = CompareViterbyArHMM( pDBN, nTimeSlice,  eps );
    if( !result )
    {
	ret = TRS_FAIL;
    }

    //////////////////////////////////////////////////////////////////////////
    delete pDBN;
    return trsResult( ret, ret == TRS_OK ? "No errors"
	: "Bad test on 1_5 Slice JTree DBN");
}

void initAtest1_5JTreeInfDBNCondGauss()
{
    trsReg(func_name, test_desc, test_class, test1_5JTreeInfDBNCondGauss);
}

CEvidence* CreateEvidenceForUnrolledArHMM(CBNet *pUnrolledDBN, int nTimeSlices,
					  pEvidencesVector &evidencesForDBN )
{

    intVector obsNodesInUnrolledDBN;
    obsNodesInUnrolledDBN.resize( nTimeSlices );
    valueVector valuesForUnrolledDBN;
    valuesForUnrolledDBN.resize( nTimeSlices );

    int nnodesInDBN = 2;
    int slice;
    for( slice = 0; slice < nTimeSlices; slice++)
    {
	obsNodesInUnrolledDBN[slice] = 1 + nnodesInDBN*slice;
	valuesForUnrolledDBN[slice] = *(evidencesForDBN[slice]->GetValueBySerialNumber(0));
    }
    return CEvidence::Create( pUnrolledDBN->GetModelDomain(),
	obsNodesInUnrolledDBN, valuesForUnrolledDBN );
}

void CreateEvidencesArHMM( CDBN *pDBN, int nTimeSlices,
			  pEvidencesVector *myEvidencesForDBN )
{

    if( myEvidencesForDBN->size() )
    {
        assert( 0 );
    }
    /////////////////////////////////////////////////////////////////////////////
    // Create values for evidence in every slice from t=0 to t=nTimeSlice
    /////////////////////////////////////////////////////////////////////////////


    intVector obsNodes(1, 1);
    int slice;
    valueVecVector valuesPerSlice;

    for( slice = 0; slice < nTimeSlices; slice++ )
    {
	valueVector tmpVal(1);
	tmpVal[0].SetFlt((float )( rand()%10 ));
	valuesPerSlice.push_back( tmpVal );
    }


    for( slice = 0; slice < nTimeSlices; slice++ )
    {
	myEvidencesForDBN->push_back( CEvidence::Create( pDBN->GetModelDomain(),
	    obsNodes, valuesPerSlice[slice] ) );
    }
}

void DefineQueryArHMM( CDBN *pDBN, int nTimeSlice,
		      intVector * queryForDBNPrior,
		      intVector * queryForDBN,
		      intVecVector *queryForUnrollBnet )
{
    ///////////////////////////////////////////////////////////////////////////
    //Define query for unrolled DBN and DBN (it is parents of some random node)
    /////////////////////////////////////////////////////////////////////////////
    int numOfNodesInDBN=pDBN->GetNumberOfNodes();
    int numOfInterfaceNodes;
    const int * interfaceNodes;
    pDBN->GetInterfaceNodes( &numOfInterfaceNodes, &interfaceNodes );
    /////////////////////////////////////////////////////////////////////////////
    int node = ( int )( rand()%( numOfNodesInDBN - 1 ) ) ;
    /////////////////////////////////////////////////////////////////////////////
    (*queryForUnrollBnet).resize( nTimeSlice );

    /////////////////////////////////////////////////////////////////////////////
    int numOfNeighbors;
    const int * neighbors;
    const ENeighborType * types;
    /////////////////////////////////////////////////////////////////////////////

    CGraph *pPriorGraph = pDBN->CreatePriorSliceGraph();
    pPriorGraph->GetNeighbors( node, &numOfNeighbors, &neighbors, &types );
    int i;
    for( i = 0; i < numOfNeighbors; i++ )
    {
	if( types[i] == ntParent )
	{
	    (*queryForDBNPrior).push_back( neighbors[i] );
	    (*queryForUnrollBnet)[0].push_back( neighbors[i] );
	}

    }
    (*queryForDBNPrior).push_back( node );
    (*queryForUnrollBnet)[0].push_back( node );
    delete pPriorGraph;

    //////////////////////////////////////////////////////////////////////////////
    node +=  numOfNodesInDBN;
    pDBN->GetGraph()->GetNeighbors( node, &numOfNeighbors, &neighbors, &types );


    int slice;

    for( i = 0; i < numOfNeighbors; i++ )
    {
	if( types[i] == ntParent )
	{
	    (*queryForDBN).push_back( neighbors[i] );
	    for( slice = 0; slice < nTimeSlice - 1; slice++ )
	    {
		(*queryForUnrollBnet)[slice+1].push_back( neighbors[i] +
		    slice*numOfNodesInDBN );
	    }
	}

    }
    (*queryForDBN).push_back( node );


    for( slice = 0; slice < nTimeSlice - 1; slice++ )
    {
	(*queryForUnrollBnet)[slice+1].push_back( node + slice*numOfNodesInDBN );
    }

}

int CompareArHMM( const CPotential *pot1, const CPotential *pot2, float eps )
{
    const float *data1;
    int data_length;
    const float *data2;
    int data_length1;
    int stop;
    CNumericDenseMatrix<float>* mat = static_cast<CNumericDenseMatrix<float>*>(
        pot1->GetMatrix(matTable));
    mat->GetRawData( &data_length, &data1 );
    mat = static_cast<CNumericDenseMatrix<float>*>(pot2->GetMatrix(matTable));
    mat->GetRawData( &data_length1, &data2 );
    int result = pot1-> IsFactorsDistribFunEqual( pot2, eps );
    if(  !result )
    {
	stop = 1;
    }
    return result;

}


/////////////////////////////////////////////////////////////////////////////
int CompareSmoothingArHMM( CDBN* pDBN, int nTimeSlice, float eps )
{
    CBNet * pUnrolledDBN;
    pUnrolledDBN = static_cast<CBNet *>( pDBN->UnrollDynamicModel( nTimeSlice ) );

    int result = 0;
    /////////////////////////////////////////////////////////////////////////////
    //Create inference for unrolled DBN
    ////////////////////////////////////////////////////////////////////////////
    CEvidence *myEvidenceForUnrolledDBN;
    pEvidencesVector myEvidencesForDBN;
    CreateEvidencesArHMM( pDBN, nTimeSlice, &myEvidencesForDBN);

    CJtreeInfEngine *pUnrolJTree = CJtreeInfEngine::Create( pUnrolledDBN );
    myEvidenceForUnrolledDBN =
        CreateEvidenceForUnrolledArHMM(pUnrolledDBN, nTimeSlice, myEvidencesForDBN);

    //////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////
    //Create inference (smoothing) for DBN

    C1_5SliceJtreeInfEngine *pDynamicJTree;
    pDynamicJTree = C1_5SliceJtreeInfEngine::Create( pDBN );
    pUnrolJTree->EnterEvidence( myEvidenceForUnrolledDBN );
    pDynamicJTree->DefineProcedure( ptSmoothing, nTimeSlice );
    pDynamicJTree->EnterEvidence( &myEvidencesForDBN.front(), nTimeSlice );
    pDynamicJTree->Smoothing();
    /////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////
    intVector queryForDBN, queryForDBNPrior;
    intVecVector queryForUnrollBnet;
    DefineQueryArHMM( pDBN, nTimeSlice,
	&queryForDBNPrior,	&queryForDBN, &queryForUnrollBnet );
    int itogResult = TRUE;
    int slice;
    for(slice = 0; slice < nTimeSlice; slice++)
    {

	pUnrolJTree->MarginalNodes( &(queryForUnrollBnet[slice]).front(),
	    (queryForUnrollBnet[slice]).size() );

	if( slice )
	{
	    pDynamicJTree->MarginalNodes( &queryForDBN.front(),
		queryForDBN.size(), slice );
	}
	else
	{
	    pDynamicJTree->MarginalNodes( &queryForDBNPrior.front(),
		queryForDBNPrior.size(), slice );
	}

	result =  CompareArHMM( pUnrolJTree->GetQueryJPD(),
	    pDynamicJTree->GetQueryJPD(), eps );
	if( !result )
	{
	    itogResult = FALSE;
	}
    }

    for( slice = 0; slice < myEvidencesForDBN.size(); slice++ )
    {
	delete myEvidencesForDBN[slice];
    }
    delete myEvidenceForUnrolledDBN;
    //CJtreeInfEngine::Release(&pUnrolJTree);
    delete pUnrolJTree;
    delete pUnrolledDBN;
    delete pDynamicJTree;

    return result;
}

/////////////////////////////////////////////////////////////////////////////
int CompareViterbyArHMM( CDBN* pDBN, int nTimeSlice, float eps )
{
    CBNet * pUnrolledDBN;
    pUnrolledDBN = static_cast<CBNet *>( pDBN->UnrollDynamicModel( nTimeSlice ) );


    /////////////////////////////////////////////////////////////////////////////
    //Create inference for unrolled DBN
    ////////////////////////////////////////////////////////////////////////////
    CEvidence *myEvidenceForUnrolledDBN;
    pEvidencesVector myEvidencesForDBN;
    CreateEvidencesArHMM( pDBN, nTimeSlice, &myEvidencesForDBN );
    myEvidenceForUnrolledDBN =
        CreateEvidenceForUnrolledArHMM(pUnrolledDBN , nTimeSlice, myEvidencesForDBN);
    CJtreeInfEngine *pUnrolJTree = CJtreeInfEngine::Create( pUnrolledDBN );

    //////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////////
    //Create inference (smoothing) for DBN

    C1_5SliceJtreeInfEngine *pDynamicJTree;
    pDynamicJTree = C1_5SliceJtreeInfEngine::Create( pDBN );
    pUnrolJTree->EnterEvidence( myEvidenceForUnrolledDBN, 1 );
    pDynamicJTree->DefineProcedure( ptViterbi, nTimeSlice );
    pDynamicJTree->EnterEvidence( &myEvidencesForDBN.front(), nTimeSlice );
    pDynamicJTree->FindMPE();
    /////////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////
    intVector queryForDBN, queryForDBNPrior;
    intVecVector queryForUnrollBnet;
    DefineQueryArHMM( pDBN, nTimeSlice,
	&queryForDBNPrior,	&queryForDBN, &queryForUnrollBnet );
    int itogResult = TRUE;
    int slice;
    for(slice = 0; slice < nTimeSlice; slice++)
    {

	pUnrolJTree->MarginalNodes( &(queryForUnrollBnet[slice]).front(),
	    (queryForUnrollBnet[slice]).size() );

	if( slice )
	{
	    pDynamicJTree->MarginalNodes( &queryForDBN.front(),
		queryForDBN.size(), slice );
	}
	else
	{
	    pDynamicJTree->MarginalNodes( &queryForDBNPrior.front(),
		queryForDBNPrior.size(), slice );
	}



	intVector pObsNodesOut1;
	intVector pObsNodesOut2;
	pnlVector<const unsigned char*> vals1;
	pnlVector<const unsigned char*> vals2;
	const CEvidence* pEv1 = pUnrolJTree->GetMPE();
	const CEvidence* pEv2 = pDynamicJTree->GetMPE();



	int nObsNodes = pEv1->GetNumberObsNodes();
	const CNodeType *const* nt = pEv1->GetNodeTypes();
	for(int i = 0; i < nObsNodes; i++)
	{
	    if(nt[i]->IsDiscrete())
	    {
		const int v1 = (pEv1->GetValueBySerialNumber(i)->GetInt());
		const int v2 = (pEv2->GetValueBySerialNumber(i)->GetInt());
		if (v2 != v1)
		{
		    itogResult = 0;
		    break;
		}
		
	    }
	    else
	    {
		int nodeSz = nt[i]->GetNodeSize();
		for( int j = 0; j < nodeSz; j++)
		{
		    const float v1 = pEv1->GetValueBySerialNumber(i)[j].GetFlt();
		    const float v2 = pEv2->GetValueBySerialNumber(i)[j].GetFlt();
		    if (v2 != v1)
		    {
			itogResult = 0;
			break;
		    }
		}
	    }
	    
	}
	if( !itogResult )
	{
	    if( pUnrolJTree->GetQueryMPE()->
		IsFactorsDistribFunEqual(pDynamicJTree->GetQueryMPE(), eps ) )
	    {
		itogResult = 1;
	    }
	    else
	    {
		pEv1->Dump();
		pEv2->Dump();
	    }
	}
	

    }
    for( slice = 0; slice < myEvidencesForDBN.size(); slice++ )
    {
	    delete myEvidencesForDBN[slice];
    }
    
    delete myEvidenceForUnrolledDBN;
    
    delete pUnrolJTree;
    
    delete pUnrolledDBN;

    delete pDynamicJTree;

    return itogResult;
}

/////////////////////////////////////////////////////////////////////////////
int CompareFilteringArHMM(CDBN* pDBN, int nTimeSlice, float eps)
{
    CBNet * pUnrolledDBN;
    pUnrolledDBN = static_cast<CBNet *>(pDBN->UnrollDynamicModel(nTimeSlice));


    int result = 1;
    /////////////////////////////////////////////////////////////////////////////
    //Create inference for unrolled DBN
    ////////////////////////////////////////////////////////////////////////////
    C1_5SliceJtreeInfEngine *pDynamicJTree;
    pDynamicJTree = C1_5SliceJtreeInfEngine::Create(pDBN);
    pDynamicJTree->DefineProcedure(ptFiltering);



    CEvidence *myEvidenceForUnrolledDBN;
    pEvidencesVector myEvidencesForDBN;
    CreateEvidencesArHMM(pDBN, nTimeSlice, &myEvidencesForDBN);
    intVector queryForDBN, queryForDBNPrior;
    intVecVector queryForUnrollBnet;
    DefineQueryArHMM(pDBN, nTimeSlice,
	&queryForDBNPrior,	&queryForDBN, &queryForUnrollBnet);

    delete pUnrolledDBN;

    int slice;
    for(slice = 0; slice < nTimeSlice; slice++)
    {

	CBNet *	pUnrolledOnISlice =
	    static_cast<CBNet *>(pDBN->UnrollDynamicModel(slice+1));
	myEvidenceForUnrolledDBN =
	    CreateEvidenceForUnrolledArHMM(pUnrolledOnISlice , slice+1, myEvidencesForDBN);
	CJtreeInfEngine *pUnrolJTree =
	    CJtreeInfEngine::Create(pUnrolledOnISlice);


	//////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	//Create inference (smoothing) for DBN


	pDynamicJTree->EnterEvidence(&(myEvidencesForDBN[slice]), 1);
	pDynamicJTree->Filtering(slice);
	pUnrolJTree->EnterEvidence(myEvidenceForUnrolledDBN);


	/////////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////


	pUnrolJTree->MarginalNodes(&(queryForUnrollBnet[slice]).front(),
	    (queryForUnrollBnet[slice]).size());


	if(slice)
	{
	    pDynamicJTree->MarginalNodes(&queryForDBN.front(),
		queryForDBN.size());

	}
	else
	{
	    pDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
		queryForDBNPrior.size());

	}

	const CPotential* qJPD1 = pUnrolJTree->GetQueryJPD();
	const CPotential* qJPD2 = pDynamicJTree->GetQueryJPD();
	qJPD1->Dump();
	qJPD2->Dump();
	int res = CompareArHMM( qJPD1, qJPD2, eps);
	result = result&&res;
	delete myEvidenceForUnrolledDBN;
        delete pUnrolJTree;
	delete (pUnrolledOnISlice);
    }

    for( slice = 0; slice < (myEvidencesForDBN).size(); slice++ )
    {
	delete myEvidencesForDBN[slice];

    }
    delete pDynamicJTree;
    return result;
}


int CompareFixLagSmoothingArHMM(CDBN* pDBN, int nTimeSlice, float eps)
{
    CBNet * pUnrolledDBN;
    pUnrolledDBN = static_cast<CBNet *>(pDBN->UnrollDynamicModel(nTimeSlice));

    int itogResult = 1;
    int result = 0;

    int lag = rand()%(nTimeSlice-1);
    lag = 2;
    /////////////////////////////////////////////////////////////////////////////
    //Create inference for unrolled DBN
    ////////////////////////////////////////////////////////////////////////////
    C1_5SliceJtreeInfEngine *pDynamicJTree;
    pDynamicJTree = C1_5SliceJtreeInfEngine::Create(pDBN);
    pDynamicJTree->DefineProcedure(ptFixLagSmoothing, lag);

    CEvidence *myEvidenceForUnrolledDBN;
    pEvidencesVector myEvidencesForDBN;
    CreateEvidencesArHMM(pDBN, nTimeSlice, &myEvidencesForDBN);
    intVector queryForDBN, queryForDBNPrior;
    intVecVector queryForUnrollBnet;
    DefineQueryArHMM(pDBN, nTimeSlice,
	&queryForDBNPrior,	&queryForDBN, &queryForUnrollBnet);

    int slice;
    for(slice = 0; slice < nTimeSlice; slice++)
    {

	//////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	//Create inference (smoothing) for DBN
	pDynamicJTree->EnterEvidence(&(myEvidencesForDBN[slice]), 1);


	if(slice >= lag)
	{
	    pDynamicJTree->FixLagSmoothing(slice);
	    CBNet *	pUnrolledOnISlice =
		static_cast<CBNet *>(pDBN->UnrollDynamicModel(slice+1));
	    myEvidenceForUnrolledDBN =
		CreateEvidenceForUnrolledArHMM(pUnrolledOnISlice , slice+1, myEvidencesForDBN);
	    CJtreeInfEngine *pUnrolJTree =
		CJtreeInfEngine::Create(pUnrolledOnISlice);


	    pUnrolJTree->EnterEvidence(myEvidenceForUnrolledDBN);

	    pUnrolJTree->MarginalNodes(&(queryForUnrollBnet[slice-lag]).front(),
		(queryForUnrollBnet[slice-lag]).size());


	    if(slice-lag)
	    {
		pDynamicJTree->MarginalNodes(&queryForDBN.front(),
		    queryForDBN.size());
	    }
	    else
	    {
		pDynamicJTree->MarginalNodes(&queryForDBNPrior.front(),
		    queryForDBNPrior.size());
	    }
	    const CPotential* qJPD1 = pUnrolJTree->GetQueryJPD();
	    const CPotential* qJPD2 = pDynamicJTree->GetQueryJPD();

	    result =  CompareArHMM( qJPD1, qJPD2, eps);
	    if(!result)
	    {
		itogResult = FALSE;
	    }

	    delete myEvidenceForUnrolledDBN;
	    //CJtreeInfEngine::Release(&pUnrolJTree);
            delete pUnrolJTree;
	    delete (pUnrolledOnISlice);
	   

	}
    }

    for( slice = 0; slice < (myEvidencesForDBN).size(); slice++ )
    {
	delete myEvidencesForDBN[slice];
    }
    delete pDynamicJTree;
    delete pUnrolledDBN;

    return itogResult;
}
