#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <algorithm>
#include "tCreateKjaerulffDBN.hpp"
#include "tCreateIncineratorBNet.hpp"
#include "pnlTreeCPD.hpp"

// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testGibbsInference";

static char* test_desc = "Provide Gibbs Sampling Inference";

static char* test_class = "Algorithm";

CMNet* myCreateDiscreteMNet();

int GibbsMPEforScalarGaussianBNet( float eps);
int GibbsForInceneratorBNet( float eps);
int GibbsForMNet( float eps);
int GibbsForGaussianBNet( float eps);
int GibbsForAsiaBNet(float eps);
int GibbsForSimplestGaussianBNet(float eps);
int GibbsForScalarGaussianBNet(float eps);
int GibbsForMixtureBNet(float eps);
int GibbsMPEForMNet(float eps);
int GibbsForSingleGaussian(float eps);
int GibbsForSparseBNet(float eps);
int GibbsForTreeBNet();

int testGibbsInference()
{
    int ret = TRS_OK;
    int seed = time(0);
    std::cout<<"seed = "<< seed<<std::endl;
    pnlSeed(seed);
   
 /////////////////////////////////////////////////////////////////////////////

    float eps = -1.0f;
    while( eps <= 0 )
    {
	trssRead( &eps, "1.5e-1f", "accuracy in test" );
    }

	ret = ret && GibbsForTreeBNet();
    ret = ret && GibbsMPEforScalarGaussianBNet( eps );
    ret = ret && GibbsForSingleGaussian( eps );
    ret = ret && GibbsForMixtureBNet(eps);
    ret = ret && GibbsForScalarGaussianBNet(  eps );
    ret = ret && GibbsForSimplestGaussianBNet( eps );
    ret = ret && GibbsForGaussianBNet( eps );
    ret = ret && GibbsForAsiaBNet( eps );
    ret = ret && GibbsForMNet( eps );
    ret = ret && GibbsMPEForMNet( eps );
    ret = ret && GibbsForInceneratorBNet( eps );
    ret = ret && GibbsForSparseBNet( eps );

    return trsResult( ret, ret == TRS_OK ? "No errors" :
    "Bad test on Gibbs Sampling Inference" );

}

CMNet* myCreateDiscreteMNet()
{
    const int numOfNds = 4;
    const int numOfNodeTypes = 1;
    const int numOfClqs = 4;

    intVector clqSizes( numOfClqs, 2 );

    int clq0[] = { 0, 1 };
    int clq1[] = { 1, 2 };
    int clq2[] = { 2, 3 };
    int clq3[] = { 3, 0 };

    const int *clqs[] = { clq0, clq1, clq2, clq3 };

    CNodeType nodeType( 1, 2 );

    intVector nodeAssociation( numOfNds, 0 );

    CMNet *pMNet = CMNet::Create( numOfNds, numOfNodeTypes, &nodeType,
	&nodeAssociation.front(), numOfClqs, &clqSizes.front(), clqs );

    pMNet->AllocFactors();

    float data0[]  = { 0.79f, 0.21f, 0.65f, 0.35f };
    float data1[]  = { 0.91f, 0.09f, 0.22f, 0.78f };
    float data2[]  = { 0.45f, 0.55f, 0.24f, 0.76f };
    float data3[]  = { 0.51f, 0.49f, 0.29f, 0.71f };

    float *data[] = { data0, data1, data2, data3 };

    int i = 0;

    for( ; i < numOfClqs; ++i )
    {
	pMNet->AllocFactor(i);

	pMNet->GetFactor(i)->AllocMatrix( data[i], matTable );
    }

    return pMNet;
}

int GibbsForAsiaBNet( float eps )
{
    ///////////////////////////////////////////////////////////////////////////////

    std::cout<<std::endl<<" Asia BNet "<< std::endl;
    CBNet* pBnet = pnlExCreateAsiaBNet();
    int ret;
    pEvidencesVector evidences;

    pBnet->GenerateSamples( &evidences, 1 );

    const int ndsToToggle[] = { 1, 2, 5, 7 };

    evidences[0]->ToggleNodeState( 4, ndsToToggle );

    CGibbsSamplingInfEngine *pGibbsInf;

    pGibbsInf = CGibbsSamplingInfEngine::Create(pBnet);

    intVecVector queries(1);

    queries[0].push_back(0);
    queries[0].push_back(2);
    queries[0].push_back(7);

    pGibbsInf->SetQueries(queries);

    pGibbsInf->EnterEvidence( evidences[0] );

    CJtreeInfEngine *pJTreeInf = CJtreeInfEngine::Create(pBnet);

    pJTreeInf->EnterEvidence( evidences[0] );

    const int querySz = 2;
    const int query[] = {0, 2};

    pGibbsInf->MarginalNodes( query,querySz );

    pGibbsInf->MarginalNodes( query,querySz );

    pJTreeInf->MarginalNodes( query,querySz );

    const CPotential *pQueryPot1 = pGibbsInf->GetQueryJPD();
    const CPotential *pQueryPot2 = pJTreeInf->GetQueryJPD();

    ret = pQueryPot1-> IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    std::cout<<"Test on gibbs for asia bnet"<<std::endl;
    std::cout<<"result of gibbs"<<std::endl;
    pQueryPot1->Dump();
    std::cout<<std::endl<<"result of junction"<<std::endl;
    pQueryPot2->Dump();

    delete evidences[0];
    //CJtreeInfEngine::Release(&pJTreeInf);
    delete pJTreeInf;
    delete pGibbsInf;
    delete pBnet;

    return ret;
    ///////////////////////////////////////////////////////////////////////////////
}

int GibbsForGaussianBNet(float eps)
{
    std::cout<<std::endl<<"test on gibbs for gaussian BNet (5 nodes)"<< std::endl;
    CBNet *pBnet;
    pEvidencesVector evidences;

    CGibbsSamplingInfEngine *pGibbsInf;
    const CPotential *pQueryPot1, *pQueryPot2;
    int i, ret;

    ////////////////////////////////////////////////////////////////////////
    //Do the example from Satnam Alag's PhD thesis, UCB ME dept 1996 p46
    //Make the following polytree, where all arcs point down
    //
    // 0   1
    //  \ /
    //   2
    //  / \
    // 3   4
    //
    //////////////////////////////////////////////////////////////////////

    int nnodes = 5;
    int numnt = 2;
    CNodeType *nodeTypes = new CNodeType[numnt];
    nodeTypes[0] = CNodeType(0,2);
    nodeTypes[1] = CNodeType(0,1);
    intVector nodeAssociation = intVector(nnodes,0);
    nodeAssociation[1] = 1;
    nodeAssociation[3] = 1;
    int nbs0[] = { 2 };
    int nbs1[] = { 2 };
    int nbs2[] = { 0, 1, 3, 4 };
    int nbs3[] = { 2 };
    int nbs4[] = { 2 };
    ENeighborType ori0[] = { ntChild };
    ENeighborType ori1[] = { ntChild };
    ENeighborType ori2[] = { ntParent, ntParent, ntChild, ntChild };
    ENeighborType ori3[] = { ntParent };
    ENeighborType ori4[] = { ntParent };
    int *nbrs[] = { nbs0, nbs1, nbs2, nbs3, nbs4 };
    ENeighborType *orient[] = { ori0, ori1, ori2, ori3, ori4 };
    intVector numNeighb = intVector(5,1);
    numNeighb[2] = 4;
    CGraph *graph;
    graph = CGraph::Create( nnodes, &numNeighb.front(), nbrs, orient );

    pBnet = CBNet::Create( nnodes, numnt, nodeTypes, &nodeAssociation.front(),graph );
    pBnet->AllocFactors();

    for( i = 0; i < nnodes; i++ )
    {
	pBnet->AllocFactor(i);
    }
    //now we need to create data for factors - we'll create matrices
    floatVector smData = floatVector(2,0.0f);
    floatVector bigData = floatVector(4,1.0f);
    intVector ranges = intVector(2, 1);
    ranges[0] = 2;
    smData[0] = 1.0f;
    CNumericDenseMatrix<float> *mean0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &smData.front() );
    bigData[0] = 4.0f;
    bigData[3] = 4.0f;

    bigData[1] = 0.30f;
    bigData[2] = 0.3f;
    ranges[1] = 2;
    CNumericDenseMatrix<float> *cov0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &bigData.front() );
    pBnet->GetFactor(0)->AttachMatrix(mean0, matMean);
    pBnet->GetFactor(0)->AttachMatrix(cov0, matCovariance);
    ranges[0] = 1;
    ranges[1] = 1;
    float val = 1.0f;
    //float val = 2.0f;
    CNumericDenseMatrix<float> *mean1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    CNumericDenseMatrix<float> *cov1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    pBnet->GetFactor(1)->AttachMatrix(mean1, matMean);
    pBnet->GetFactor(1)->AttachMatrix(cov1, matCovariance);
    smData[0] = 0.0f;
    smData[1] = 0.0f;
    //smData[0] = 3.0f;
    //smData[1] = 4.0f;
    ranges[0] = 2;
    CNumericDenseMatrix<float> *mean2 =	CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front() );
    smData[0] = 2.0f;
    smData[1] = 1.0f;
    CNumericDenseMatrix<float> *w21 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    bigData[0] = 2.0f;
    bigData[1] = 0.0f;
    bigData[2] = 0.0f;
    bigData[3] = 1.0f;
    ranges[1] = 2;
    CNumericDenseMatrix<float> *cov2 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[0] = 2.0f;
    bigData[1] = 1.0f;
    bigData[2] = 1.0f;
    bigData[3] = 3.0f;
    CNumericDenseMatrix<float> *w20 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(2)->AttachMatrix( mean2, matMean );
    pBnet->GetFactor(2)->AttachMatrix( cov2, matCovariance );
    pBnet->GetFactor(2)->AttachMatrix( w20, matWeights,0 );
    pBnet->GetFactor(2)->AttachMatrix( w21, matWeights,1 );

    val = 0.0f;
    ranges[0] = 1;
    ranges[1] = 1;
    CNumericDenseMatrix<float> *mean3 =	CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);
    val = 1.0f;
    CNumericDenseMatrix<float> *cov3 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);
    ranges[1] = 2;
    smData[0] = 1.0f;
    smData[1] = 1.0f;
    CNumericDenseMatrix<float> *w30 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    pBnet->GetFactor(3)->AttachMatrix( mean3, matMean );
    pBnet->GetFactor(3)->AttachMatrix( cov3, matCovariance );
    pBnet->GetFactor(3)->AttachMatrix( w30, matWeights,0 );

    ranges[0] = 2;
    ranges[1] = 1;
    smData[0] = 0.0f;
    smData[1] = 0.0f;
    CNumericDenseMatrix<float> *mean4 =	CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    ranges[1] = 2;
    bigData[0] = 1.0f;
    bigData[1] = 0.5f;
    bigData[2] = 0.5f;
    bigData[3] = 1.0f;
    CNumericDenseMatrix<float> *cov4 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[2] = 1.0f;
    CNumericDenseMatrix<float> *w40 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(4)->AttachMatrix( mean4, matMean );
    pBnet->GetFactor(4)->AttachMatrix( cov4, matCovariance );
    pBnet->GetFactor(4)->AttachMatrix( w40, matWeights,0 );


    evidences.clear();
    pBnet->GenerateSamples( &evidences, 1 );

    const int ndsToToggle2[] = { 0, 1, 2 };
    evidences[0]->ToggleNodeState( 3, ndsToToggle2 );
    const int *flags1 = evidences[0]->GetObsNodesFlags();
    std::cout<<"observed nodes"<<std::endl;
    for( i = 0; i < pBnet->GetNumberOfNodes(); i++ )
    {
	if ( flags1[i] )
	{
	    std::cout<<"node "<<i<<"; ";
	}
    }
    std::cout<<std::endl<<std::endl;

    const int querySz2 = 1;
    const int query2[] = { 2, 0 };

    CNaiveInfEngine *pNaiveInf = CNaiveInfEngine::Create(pBnet);
    pNaiveInf->EnterEvidence( evidences[0] );
    pNaiveInf->MarginalNodes( query2,querySz2 );

    pGibbsInf = CGibbsSamplingInfEngine::Create( pBnet );
    pGibbsInf->SetMaxTime(2000);
    pGibbsInf->SetBurnIn(300);
    pGibbsInf->SetNumStreams(3);


    intVecVector queries(1);
    queries[0].clear();
    //queries[0].push_back( 0 );
    queries[0].push_back( 2 );
    //queries[0].push_back( 4 );
    pGibbsInf->SetQueries( queries );

    pGibbsInf->EnterEvidence( evidences[0] );
    pGibbsInf->MarginalNodes( query2, querySz2 );

    pQueryPot1 = pGibbsInf->GetQueryJPD();
    pQueryPot2 = pNaiveInf->GetQueryJPD();
    std::cout<<"result of gibbs"<<std::endl<<std::endl;
    pQueryPot1->Dump();
    std::cout<<"result of naive"<<std::endl;
    pQueryPot2->Dump();

    ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
    delete pNaiveInf;
    delete pGibbsInf;
    delete pBnet;

    return ret;

    ////////////////////////////////////////////////////////////////////////////////////////
}


int GibbsForMNet(float eps)
{

    std::cout<<std::endl<<"Gibbs for discrete MNet"<< std::endl;

    pEvidencesVector evidences;

    CGibbsSamplingInfEngine *pGibbsInf;

    int ret;
    CMNet* pMNet = myCreateDiscreteMNet();
    pMNet->GenerateSamples( &evidences, 1 );

    const int ndsToToggleMNet[] = { 0, 3 };

    evidences[0]->ToggleNodeState( 2, ndsToToggleMNet );

    pGibbsInf = CGibbsSamplingInfEngine::Create(pMNet);

    intVecVector queries(1);
    //pGibbsInf->SetParemeter(pMaxTime, 500);
    queries[0].clear();

    queries[0].push_back(0);
    queries[0].push_back(3);


    pGibbsInf->SetQueries(queries);

    pGibbsInf->EnterEvidence( evidences[0] );

    CNaiveInfEngine* pInf = CNaiveInfEngine::Create(pMNet);

    pInf->EnterEvidence( evidences[0] );

    const int querySzMNet = 2;
    const int queryMNet[] = {0, 3};

    pGibbsInf->MarginalNodes( queryMNet,querySzMNet );
    pInf->MarginalNodes( queryMNet,querySzMNet );

    const CPotential *pQueryPot1MNet = pGibbsInf->GetQueryJPD();
    const CPotential *pQueryPot2MNet = pInf->GetQueryJPD();

    ret = pQueryPot1MNet-> IsFactorsDistribFunEqual( pQueryPot2MNet, eps, 0 );

    std::cout<<"result of gibbs"<<std::endl;
    pQueryPot1MNet->Dump();
    std::cout<<std::endl<<"result of naive"<<std::endl;
    pQueryPot2MNet->Dump();



    delete pInf;
    delete pGibbsInf;
    delete evidences[0];
    delete pMNet;
    ////////////////////////////////////////////////////////////////////////////////////////
    return ret;

}

int GibbsForInceneratorBNet(float eps)
{

    std::cout<<std::endl<<"Gibbs for Incenerator BNet"<< std::endl;

    CBNet *pBnet;
    pEvidencesVector evidences;
    CJtreeInfEngine *pJTreeInf;
    CGibbsSamplingInfEngine *pGibbsInf;
    const CPotential *pQueryPot1, *pQueryPot2;
    int i, ret;

    pBnet = tCreateIncineratorBNet();

    evidences.clear();
    pBnet->GenerateSamples( &evidences, 1 );


    const int ndsToToggle1[] = { 0, 1, 3 };
    evidences[0]->ToggleNodeState( 3, ndsToToggle1 );
    const int *flags = evidences[0]->GetObsNodesFlags();
    std::cout<<"observed nodes"<<std::endl;
    for( i = 0; i < pBnet->GetNumberOfNodes(); i++ )
    {
	if ( flags[i] )
	{
	    std::cout<<"node "<<i<<"; ";
	}
    }
    std::cout<<std::endl<<std::endl;

    const int querySz1 = 2;
    const int query1[] = { 0, 1 };

    pJTreeInf = CJtreeInfEngine::Create(pBnet);
    pJTreeInf->EnterEvidence( evidences[0] );
    pJTreeInf->MarginalNodes( query1,querySz1 );

    pGibbsInf = CGibbsSamplingInfEngine::Create( pBnet );

    intVecVector queries(1);
    queries[0].clear();
    queries[0].push_back( 0 );
    queries[0].push_back( 1 );
    pGibbsInf->SetQueries( queries );

    pGibbsInf->EnterEvidence( evidences[0] );
    pGibbsInf->MarginalNodes( query1, querySz1 );

    pQueryPot1 = pGibbsInf->GetQueryJPD();
    pQueryPot2 = pJTreeInf->GetQueryJPD();
    std::cout<<"result of gibbs"<<std::endl<<std::endl;
    pQueryPot1->Dump();
    std::cout<<"result of junction"<<std::endl;
    pQueryPot2->Dump();

    ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
    //CJtreeInfEngine::Release(&pJTreeInf);
    delete pJTreeInf;
    delete pGibbsInf;
    delete pBnet;

    return ret;

    ///////////////////////////////////////////////////////////////////////////////

}

int GibbsForSimplestGaussianBNet( float eps)
{
    std::cout<<std::endl<<"Gibbs for simplest gaussian BNet (3 nodes) "<<std::endl;

    int nnodes = 3;
    int numnt = 2;
    CNodeType *nodeTypes = new CNodeType[numnt];
    nodeTypes[0] = CNodeType(0,1);
    nodeTypes[1] = CNodeType(0,2);
    intVector nodeAssociation = intVector(nnodes,1);
    nodeAssociation[0] = 0;
    int nbs0[] = { 1 };
    int nbs1[] = { 0, 2 };
    int nbs2[] = { 1 };
    ENeighborType ori0[] = { ntChild };
    ENeighborType ori1[] = { ntParent, ntChild  };
    ENeighborType ori2[] = { ntParent };
    int *nbrs[] = { nbs0, nbs1, nbs2 };
    ENeighborType *orient[] = { ori0, ori1, ori2 };

    intVector numNeighb = intVector(3);
    numNeighb[0] = 1;
    numNeighb[1] = 2;
    numNeighb[2] = 1;

    CGraph *graph;
    graph = CGraph::Create(nnodes, &numNeighb.front(), nbrs, orient);

    CBNet *pBnet = CBNet::Create( nnodes, numnt, nodeTypes,
	&nodeAssociation.front(),graph );
    pBnet->AllocFactors();

    for(int i = 0; i < nnodes; i++ )
    {
	pBnet->AllocFactor(i);

    }

    floatVector data(1,0.0f);
    intVector ranges(2,1);

    ///////////////////////////////////////////////////////////////////
    CNumericDenseMatrix<float> *mean0 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    data[0] = 0.3f;
    CNumericDenseMatrix<float> *cov0 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    pBnet->GetFactor(0)->AttachMatrix( mean0, matMean );
    pBnet->GetFactor(0)->AttachMatrix( cov0, matCovariance );
    /////////////////////////////////////////////////////////////////////

    ranges[0] = 2;
    data.resize(2);
    data[0] = -1.0f;
    data[1] = 0.0f;
    CNumericDenseMatrix<float> *mean1 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    ranges[1] = 2;
    data.resize(4);
    data[0] = 1.0f;
    data[1] = 0.1f;
    data[3] = 3.0f;
    data[2] = 0.1f;
    CNumericDenseMatrix<float> *cov1 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    ranges[1] =1;
    data.resize(2);
    data[0] = 1.0f;
    data[1] = 0.5f;
    CNumericDenseMatrix<float> *weight1 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());


    pBnet->GetFactor(1)->AttachMatrix( mean1, matMean );
    pBnet->GetFactor(1)->AttachMatrix( cov1, matCovariance );
    pBnet->GetFactor(1)->AttachMatrix( weight1, matWeights,0 );
    ///////////////////////////////////////////////////////////////////////////


    ranges[0] = 2;
    data.resize(2);
    data[0] = 1.0f;
    data[1] = 20.5f;
    CNumericDenseMatrix<float> *mean2 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());

    ranges[1] = 2;
    data.resize(4);
    data[0] = 1.0f;
    data[1] = 0.0f;
    data[3] = 9.0f;
    data[2] = 0.0f;
    CNumericDenseMatrix<float> *cov2 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());


    data.resize(2);
    data[0] = 1.0f;
    data[1] = 3.5f;
    data[2] = 1.0f;
    data[3] = 0.5f;
    CNumericDenseMatrix<float> *weight2 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &data.front());


    pBnet->GetFactor(2)->AttachMatrix( mean2, matMean );
    pBnet->GetFactor(2)->AttachMatrix( cov2, matCovariance );
    pBnet->GetFactor(2)->AttachMatrix( weight2, matWeights,0 );
    ///////////////////////////////////////////////////////////////////////////

    pEvidencesVector evidences;

    pBnet->GenerateSamples( &evidences, 1 );

    const int ndsToToggle[] = { 0, 1 };
    evidences[0]->ToggleNodeState( 2, ndsToToggle );

    intVector query(1,1);



    CNaiveInfEngine *pNaiveInf = CNaiveInfEngine::Create(pBnet);
    pNaiveInf->EnterEvidence( evidences[0] );
    pNaiveInf->MarginalNodes( &query.front(),query.size() );

    CGibbsSamplingInfEngine *pGibbsInf = CGibbsSamplingInfEngine::Create( pBnet );
    intVecVector queryes(1);
    queryes[0].push_back(1);
    pGibbsInf->SetQueries( queryes);
    pGibbsInf->EnterEvidence( evidences[0] );
    pGibbsInf->MarginalNodes( &query.front(),query.size() );

    const CPotential *pQueryPot1 = pGibbsInf->GetQueryJPD();
    const CPotential *pQueryPot2 = pNaiveInf->GetQueryJPD();
    std::cout<<"result of gibbs"<<std::endl<<std::endl;
    pQueryPot1->Dump();
    std::cout<<"result of naive"<<std::endl;
    pQueryPot2->Dump();

    int ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
    delete pNaiveInf;
    delete pGibbsInf;
    delete pBnet;

    return ret;

}

int GibbsForScalarGaussianBNet( float eps)
{
    std::cout<<std::endl<<" Scalar gaussian BNet (5 nodes)"<< std::endl;
    CBNet *pBnet;
    pEvidencesVector evidences;

    CGibbsSamplingInfEngine *pGibbsInf;
    const CPotential *pQueryPot1, *pQueryPot2;
    int i, ret;

    ////////////////////////////////////////////////////////////////////////
    //Do the example from Satnam Alag's PhD thesis, UCB ME dept 1996 p46
    //Make the following polytree, where all arcs point down
    //
    // 0   1
    //  \ /
    //   2
    //  / \
    // 3   4
    //
    //////////////////////////////////////////////////////////////////////

    int nnodes = 5;
    int numnt = 1;
    CNodeType *nodeTypes = new CNodeType[numnt];
    nodeTypes[0] = CNodeType(0,1);

    intVector nodeAssociation = intVector(nnodes,0);

    int nbs0[] = { 2 };
    int nbs1[] = { 2 };
    int nbs2[] = { 0, 1, 3, 4 };
    int nbs3[] = { 2 };
    int nbs4[] = { 2 };
    ENeighborType ori0[] = { ntChild };
    ENeighborType ori1[] = { ntChild };
    ENeighborType ori2[] = { ntParent, ntParent, ntChild, ntChild };
    ENeighborType ori3[] = { ntParent };
    ENeighborType ori4[] = { ntParent };
    int *nbrs[] = { nbs0, nbs1, nbs2, nbs3, nbs4 };
    ENeighborType *orient[] = { ori0, ori1, ori2, ori3, ori4 };
    intVector numNeighb = intVector(5,1);
    numNeighb[2] = 4;
    CGraph *graph;
    graph = CGraph::Create(nnodes, &numNeighb.front(), nbrs, orient);

    pBnet = CBNet::Create( nnodes, numnt, nodeTypes, &nodeAssociation.front(),graph );
    pBnet->AllocFactors();

    for( i = 0; i < nnodes; i++ )
    {
	pBnet->AllocFactor(i);
    }
    //now we need to create data for factors - we'll create matrices
    floatVector smData = floatVector(1,0.0f);
    floatVector bigData = floatVector(1,1.0f);
    intVector ranges = intVector(2, 1);
    ranges[0] = 1;
    smData[0] = 1.0f;
    CNumericDenseMatrix<float> *mean0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &smData.front());
    bigData[0] = 4.0f;

    CNumericDenseMatrix<float> *cov0 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(0)->AttachMatrix(mean0, matMean);
    pBnet->GetFactor(0)->AttachMatrix(cov0, matCovariance);

    float val = 1.0f;

    CNumericDenseMatrix<float> *mean1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    CNumericDenseMatrix<float> *cov1 = CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    pBnet->GetFactor(1)->AttachMatrix(mean1, matMean);
    pBnet->GetFactor(1)->AttachMatrix(cov1, matCovariance);
    smData[0] = 0.0f;

    CNumericDenseMatrix<float> *mean2 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    smData[0] = 2.0f;

    CNumericDenseMatrix<float> *w21 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    bigData[0] = 2.0f;

    CNumericDenseMatrix<float> *cov2 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[0] = 1.0f;

    CNumericDenseMatrix<float> *w20 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(2)->AttachMatrix( mean2, matMean );
    pBnet->GetFactor(2)->AttachMatrix( cov2, matCovariance );
    pBnet->GetFactor(2)->AttachMatrix( w20, matWeights,0 );
    pBnet->GetFactor(2)->AttachMatrix( w21, matWeights,1 );

    val = 0.0f;

    CNumericDenseMatrix<float> *mean3 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);
    val = 4.0f;
    CNumericDenseMatrix<float> *cov3 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);

    smData[0] = 1.1f;

    CNumericDenseMatrix<float> *w30 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    pBnet->GetFactor(3)->AttachMatrix( mean3, matMean );
    pBnet->GetFactor(3)->AttachMatrix( cov3, matCovariance );
    pBnet->GetFactor(3)->AttachMatrix( w30, matWeights,0 );


    smData[0] = -0.8f;

    CNumericDenseMatrix<float> *mean4 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());

    bigData[0] = 1.2f;

    CNumericDenseMatrix<float> *cov4 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    bigData[0] = 2.0f;

    CNumericDenseMatrix<float> *w40 = CNumericDenseMatrix<float>::Create(2, &ranges.front(), &bigData.front());
    pBnet->GetFactor(4)->AttachMatrix( mean4, matMean );
    pBnet->GetFactor(4)->AttachMatrix( cov4, matCovariance );
    pBnet->GetFactor(4)->AttachMatrix( w40, matWeights,0 );


    evidences.clear();
    pBnet->GenerateSamples( &evidences, 1 );

    const int ndsToToggle2[] = { 0, 1, 2 };
    evidences[0]->ToggleNodeState( 3, ndsToToggle2 );
    const int *flags1 = evidences[0]->GetObsNodesFlags();
    std::cout<<"observed nodes"<<std::endl;
    for( i = 0; i < pBnet->GetNumberOfNodes(); i++ )
    {
	if ( flags1[i] )
	{
	    std::cout<<"node "<<i<<"; ";
	}
    }
    std::cout<<std::endl<<std::endl;

    const int querySz2 = 1;
    const int query2[] = { 0 };

    CNaiveInfEngine *pNaiveInf = CNaiveInfEngine::Create(pBnet);
    pNaiveInf->EnterEvidence( evidences[0] );
    pNaiveInf->MarginalNodes( query2,querySz2 );

    pGibbsInf = CGibbsSamplingInfEngine::Create( pBnet );
    pGibbsInf->SetNumStreams( 1 );
    pGibbsInf->SetMaxTime( 10000 );
    pGibbsInf->SetBurnIn( 1000 );
    


    intVecVector queries(1);
    queries[0].clear();
    queries[0].push_back( 0 );
    //queries[0].push_back( 2 );
    pGibbsInf->SetQueries( queries );

    pGibbsInf->EnterEvidence( evidences[0] );
    pGibbsInf->MarginalNodes( query2, querySz2 );

    pQueryPot1 = pGibbsInf->GetQueryJPD();
    pQueryPot2 = pNaiveInf->GetQueryJPD();
    std::cout<<"result of gibbs"<<std::endl<<std::endl;
    pQueryPot1->Dump();
    std::cout<<"result of naive"<<std::endl;
    pQueryPot2->Dump();

    ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
    delete pNaiveInf;
    delete pGibbsInf;
    delete pBnet;

    return ret;

    ////////////////////////////////////////////////////////////////////////////////////////
}


int GibbsForMixtureBNet( float eps)
{
    
   std::cout<<std::endl<<"Gibbs for mixture gaussian BNet"<<std::endl;

    CBNet* pMixBNet = pnlExCreateSimpleGauMix();

    //create evidence for inference
    int nObsNds = 3;
    int obsNds[] = { 2, 3, 4 };
    valueVector obsVals;

    obsVals.assign( 3, (Value)0 );
    obsVals[0].SetFlt(-.5f);
    obsVals[1].SetFlt(2.0f);
    obsVals[2].SetFlt(1.0f);

    CEvidence* evid = CEvidence::Create( pMixBNet, nObsNds, obsNds, obsVals );

    //create inference
    CNaiveInfEngine* infNaive = CNaiveInfEngine::Create( pMixBNet );
    CGibbsSamplingInfEngine* infGibbs = CGibbsSamplingInfEngine::Create( pMixBNet );

    //start inference with maximization
    intVector queryNode(1, 0);
    intVecVector queryes;
    queryes.push_back(queryNode);
    //naive inference
    infNaive->EnterEvidence( evid);
    infNaive->MarginalNodes( &queryNode.front(), 1 );
    const CPotential* pQueryPot1 = infNaive->GetQueryJPD();
    std::cout<<"Results of Naive"<<std::endl;
    pQueryPot1->Dump();

    infGibbs->SetQueries(queryes);
    infGibbs->EnterEvidence( evid);
    infGibbs->MarginalNodes( &queryNode.front(), 1 );
    const CPotential* pQueryPot2 = infGibbs->GetQueryJPD();
    std::cout<<"Results of Gibbs"<<std::endl;
    pQueryPot2->Dump();
    int ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evid;
    delete infNaive;
    delete infGibbs;
    delete pMixBNet;
    return ret;

}



int GibbsMPEForMNet( float eps)
{

    std::cout<<std::endl<<"Gibbs MPE for MNet"<< std::endl;

    pEvidencesVector evidences;

    CGibbsSamplingInfEngine *pGibbsInf;

    int ret = 1;
    CMNet* pMNet = myCreateDiscreteMNet();
    pMNet->GenerateSamples( &evidences, 1 );

    const int ndsToToggleMNet[] = { 0, 3 };

    evidences[0]->ToggleNodeState( 2, ndsToToggleMNet );

    pGibbsInf = CGibbsSamplingInfEngine::Create(pMNet);

    intVecVector queries(1);
    //pGibbsInf->SetParemeter(pMaxTime, 500);
    queries[0].clear();

    queries[0].push_back(0);
    queries[0].push_back(3);


    pGibbsInf->SetQueries(queries);

    pGibbsInf->EnterEvidence( evidences[0], 1 );

    CNaiveInfEngine* pInf = CNaiveInfEngine::Create(pMNet);

    pInf->EnterEvidence( evidences[0], 1 );

    const int querySzMNet = 2;
    const int queryMNet[] = {0, 3};

    pGibbsInf->MarginalNodes( queryMNet,querySzMNet );
    pInf->MarginalNodes( queryMNet,querySzMNet );

    const CEvidence *pEvGibbs = pGibbsInf->GetMPE();
    const CEvidence *pEvInf = pInf->GetMPE();
    int i;
    for( i = 0; i < querySzMNet; i++ )
    {
        if( pEvGibbs->GetValueBySerialNumber(i)->GetInt() != 
            pEvInf->GetValueBySerialNumber(i)->GetInt() )
        {
            ret = 0;
            break;
        }
    }

   
    std::cout<<"result of gibbs"<<std::endl;
    pEvGibbs->Dump();
    std::cout<<std::endl<<"result of naive"<<std::endl;
    pEvInf->Dump();



    delete pInf;
    delete pGibbsInf;
    delete evidences[0];
    delete pMNet;
    ////////////////////////////////////////////////////////////////////////////////////////
    return ret;

}


int GibbsForSingleGaussian(float eps)
{
    std::cout<<std::endl<<"Using Gibbs for testing samples from gaussian"<<std::endl;

    int nnodes = 1;
    int numnt = 1;
    CNodeType *nodeTypes = new CNodeType[numnt];
    nodeTypes[0] = CNodeType(0,2);
   
    intVector nodeAssociation = intVector(nnodes,0);
   
   
    CGraph *graph;
    graph = CGraph::Create(nnodes, 0, NULL, NULL);

    CBNet *pBnet = CBNet::Create( nnodes, numnt, nodeTypes,
	&nodeAssociation.front(),graph );
    pBnet->AllocFactors();
	pBnet->AllocFactor(0);


    float mean[2] = {0.0f, 0.0f};
    intVector ranges(2,1);
    ranges[0] = 2;

    ///////////////////////////////////////////////////////////////////
    CNumericDenseMatrix<float> *mean0 =	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), mean);

    ranges[1] = 2;
    float cov[4] = {1.0f, 0.3f, 0.3f, 1.0f};
    CNumericDenseMatrix<float> *cov0 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), cov);

    pBnet->GetFactor(0)->AttachMatrix( mean0, matMean );
    pBnet->GetFactor(0)->AttachMatrix( cov0, matCovariance );
    /////////////////////////////////////////////////////////////////////
    CGibbsSamplingInfEngine *pGibbsInf = CGibbsSamplingInfEngine::Create( pBnet );
    pGibbsInf->SetBurnIn( 100 );
    pGibbsInf->SetMaxTime( 5000 );

    pEvidencesVector evidences;
    pBnet->GenerateSamples(&evidences, 1 );
    
    const int ndsToToggle[] = { 0 };
    evidences[0]->ToggleNodeState( 1, ndsToToggle );
    
    intVector query(1,0);
    
    
    intVecVector queryes(1);
    queryes[0].push_back(0);
    pGibbsInf->SetQueries( queryes);
    pGibbsInf->EnterEvidence( evidences[0] );
    pGibbsInf->MarginalNodes( &query.front(),query.size() );

    const CPotential *pQueryPot1 = pGibbsInf->GetQueryJPD();
  
    std::cout<<"result of gibbs"<<std::endl<<std::endl;
    pQueryPot1->Dump();
    
    delete evidences[0];
   
    delete pGibbsInf;
    delete pBnet;
    delete []nodeTypes;

    return 1;

}


int GibbsMPEforScalarGaussianBNet( float eps)
{
    std::cout<<std::endl<<"Gibbs MPE for scalar gaussian BNet"<<std::endl;

    int ret =1;
    CBNet *pBnet = pnlExCreateScalarGaussianBNet();
    std::cout<<"BNet has been created \n";
    
    CGibbsSamplingInfEngine *pGibbsInf = CGibbsSamplingInfEngine::Create( pBnet );
    pGibbsInf->SetBurnIn( 100);
    pGibbsInf->SetMaxTime( 10000 );
    std::cout<<"burnIN and MaxTime have been defined \n";
    
    pEvidencesVector evidences;
    pBnet->GenerateSamples(&evidences, 1 );
    std::cout<<"evidence has been generated \n";
    
    const int ndsToToggle[] = { 0, 3 };
    evidences[0]->ToggleNodeState( 2, ndsToToggle );
    
    
    intVecVector queryes(1);
    queryes[0].push_back(0);
    pGibbsInf->SetQueries( queryes);
    std::cout<<"set queries"<<std::endl;
    
    pGibbsInf->EnterEvidence( evidences[0], 1 );
    std::cout<<"enter evidence"<<std::endl;
    
    
    intVector query(1,0);
    pGibbsInf->MarginalNodes( &query.front(),query.size() );
    std::cout<<"marginal nodes"<<std::endl;
    
    const CEvidence *pEvGibbs = pGibbsInf->GetMPE();

    CJtreeInfEngine *pJTreeInf = CJtreeInfEngine::Create(pBnet);
    pJTreeInf->EnterEvidence(evidences[0], 1);
    pJTreeInf->MarginalNodes(&query.front(), query.size());
    const CEvidence* pEvJTree = pJTreeInf->GetMPE();

    
   
    
    std::cout<<"result of gibbs"<<std::endl<<std::endl;
    pEvGibbs->Dump();
    pEvJTree->Dump();
    
    delete evidences[0];
   
    delete pGibbsInf;
    delete pJTreeInf;
    delete pBnet;
    return ret;
}

int GibbsForSparseBNet( float eps)
{

    CBNet *pDenseBnet;
    CBNet *pSparseBnet;
    
    CGibbsSamplingInfEngine *pGibbsInfDense;
    CGibbsSamplingInfEngine *pGibbsInfSparse;
    
    pEvidencesVector evidences;
    const CPotential *pQueryPot1, *pQueryPot2;
    int ret;

    pDenseBnet = tCreateIncineratorBNet();
    pSparseBnet = pDenseBnet->ConvertToSparse();

    evidences.clear();
    pDenseBnet->GenerateSamples( &evidences, 1 );


    const int ndsToToggle1[] = { 0, 1, 3 };
    evidences[0]->ToggleNodeState( 3, ndsToToggle1 );
   
    const int querySz1 = 2;
    const int query1[] = { 0, 1 };
    
    pGibbsInfDense = CGibbsSamplingInfEngine::Create( pDenseBnet );
    pGibbsInfSparse = CGibbsSamplingInfEngine::Create( pSparseBnet );
    
    intVecVector queries(1);
    queries[0].clear();
    queries[0].push_back( 0 );
    queries[0].push_back( 1 );
    
    pGibbsInfSparse->SetQueries( queries );
    pGibbsInfSparse->EnterEvidence( evidences[0] );
    pGibbsInfSparse->MarginalNodes( query1, querySz1 );

    pGibbsInfDense->SetQueries( queries );
    pGibbsInfDense->EnterEvidence( evidences[0] );
    pGibbsInfDense->MarginalNodes( query1, querySz1 );


    pQueryPot1 = pGibbsInfDense->GetQueryJPD();
    pQueryPot2 = pGibbsInfSparse->GetQueryJPD();
    

    ret = pQueryPot1->IsFactorsDistribFunEqual( pQueryPot2, eps, 0 );

    delete evidences[0];
    delete pGibbsInfSparse;
    delete pGibbsInfDense;
    delete pDenseBnet;
    delete pSparseBnet;

    return ret;
}

CBNet *CreateTestTabularNetWithDecTreeNodeBNet()
{
	//   Baysam network     
	//        0 1...8 9       0,1, 9- discrete nodes distribution type is tabular, size is 2
	//        \ \  / /      10 - is discrete desigion tree node
	//            10        
	//          
	//      
	//
	// Desigion tree on the node 10 is
	//
	//     0
	//    / \	
	//   1	 2
	//  / \ / \
	// 3  4 5  6
	//  	   
    const int nnodes = 11; //Number of nodes
    const int numNt =  1; //number of Node types (all nodes are discrete)
    CNodeType* nodeTypes = new CNodeType [numNt];
	int size = 2;
    int i;
	nodeTypes[0] = CNodeType( 1,size );
    int *nodeAssociation = new int[nnodes];
    for( i = 0; i < nnodes; i++ )
	{
		nodeAssociation[i] = 0;
	};
    int *numOfNeigh;
    numOfNeigh = new int[nnodes];

	for( i = 0; i < nnodes - 1; i++ )
	{
		numOfNeigh[i] = 1;
	};
	numOfNeigh[nnodes - 1] = nnodes - 1;

    int **neigh;
	neigh = new int*[nnodes];
    for( i = 0; i < nnodes - 1; i++ )
	{
		neigh[i] = new int;
		neigh[i][0] = nnodes - 1;

	};
	neigh[nnodes - 1] = new int[nnodes - 1];

    for( i = 0; i < nnodes - 1; i++ )
	{
		neigh[nnodes - 1][i] = i;
	};

	ENeighborType **orient;
    orient = new ENeighborType*[nnodes];

	for( i = 0; i < nnodes - 1; i++ )
	{
		orient[i] = new ENeighborType;
		orient[i][0] = ntChild;

	};
    
	orient[nnodes - 1] = new ENeighborType[nnodes - 1];

	for( i = 0; i < nnodes - 1; i++ )
	{
		orient[nnodes - 1][i] = ntParent;
	};

    CGraph* pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient);

    //Create static BNet
    CBNet* pBNet = CBNet::Create( nnodes, numNt, nodeTypes, nodeAssociation, pGraph );
    pBNet->AllocFactors();


    int nnodesInDom = 1;
    int *domains;
	domains = new int[nnodes -1];
	for( i = 0; i < nnodes - 1; i++ )
	{
		domains[i] = i;
	};
    float table[] = { 0.3f, 0.7f};

    CTabularCPD **pCPDPar;
	pCPDPar = new CTabularCPD*[nnodes - 1];
	for( i = 0; i < nnodes - 1; i++ )
	{
		pCPDPar[i] = CTabularCPD::Create( &domains[i], nnodesInDom, pBNet->GetModelDomain(), table );
		pBNet->AttachFactor(pCPDPar[i]);
	};	
		

    int nnodesInChilddom = nnodes;
	int *domainChild; 
	domainChild = new int[ nnodes - 1];

		for( i = 0; i < nnodes; i++ )
		{
			domainChild[i] = i;
		};

    CTreeCPD *pCPDChild = CTreeCPD::Create( domainChild, nnodesInChilddom, pBNet->GetModelDomain());
    // creating tree on node 11
	// 1) start of graph creation
     const int nnodesT = 7; 
    int numOfNeighT[] = {2, 3, 3, 1, 1, 1, 1 };
    int neigh0T[] = {1, 2};
    int neigh1T[] = {0, 3, 4};
    int neigh2T[] = {0, 5, 6};
    int neigh3T[] = {1};
    int neigh4T[] = {1};
    int neigh5T[] = {2};
    int neigh6T[] = {2};
    

    ENeighborType orient0T[] = { ntChild, ntChild };
    ENeighborType orient1T[] = { ntParent, ntChild, ntChild };
    ENeighborType orient2T[] = { ntParent, ntChild, ntChild  };
    ENeighborType orient3T[] = { ntParent };
    ENeighborType orient4T[] = { ntParent };
    ENeighborType orient5T[] = { ntParent };
    ENeighborType orient6T[] = { ntParent };
   

    int *neighT[] = { neigh0T, neigh1T, neigh2T, neigh3T, neigh4T, neigh5T, neigh6T };
    ENeighborType *orientT[] = { orient0T, orient1T, orient2T, orient3T, orient4T, orient5T, orient6T };

    CGraph* pGraphT = CGraph::Create( nnodesT, numOfNeighT, neighT, orientT);
    // end of graph creation
	// 2) start of filling tree nodes
    TreeNodeFields fnode0; //This structures will contain properties of all     
    TreeNodeFields fnode1; //tree nodes 
    TreeNodeFields fnode2; //
    TreeNodeFields fnode3; //
    TreeNodeFields fnode4; //
    TreeNodeFields fnode5; //
    TreeNodeFields fnode6; //

	// start of filling information of node 0  
    fnode0.isTerminal = false; // means that this node is split
    fnode0.Question = 0; // question type on node 0. 
	                     // Value 0 means that that question type is "="
						 //	Value 1 means that that question type is ">"
    fnode0.questionValue = 0; //Asking value
    fnode0.node_index = 0;    // Index of asked desigion tree parent
	// end of filling information of node 0  

    // start of filling information of node 1  
    fnode1.isTerminal = false;
    fnode1.Question = 0;
    fnode1.questionValue = 0;
    fnode1.node_index = 1;
    // end of filling information of node 1  
    
    // start of filling information of node 2 
    fnode2.isTerminal = false;
    fnode2.Question = 0;
    fnode2.questionValue = 0;
    fnode2.node_index = 1;
	// end of filling information of node 2  

	// start of filling information of node 3	
    fnode3.isTerminal = true; //means that this node is terminal
    fnode3.probVect = new float[2];
    fnode3.probVect[0] = 0.3f;  //Terminal nodes probabilities.
    fnode3.probVect[1] = 0.7f;  //This properties doesn`t fill when desigion tree node
								// is continuous node.
	// end of filling information of node 3
    
	// start of filling information of node 4
    fnode4.isTerminal = true;
    fnode4.probVect = new float[2];
    fnode4.probVect[0] = 0.6f;
    fnode4.probVect[1] = 0.4f;
	// end of filling information of node 4

    // start of filling information of node 5
    fnode5.isTerminal = true;
    fnode5.probVect = new float[2];
    fnode5.probVect[0] = 0.9f;
    fnode5.probVect[1] = 0.1f;
	// end of filling information of node 5

	// start of filling information of node 6
    fnode6.isTerminal = true;
    fnode6.probVect = new float[2];
    fnode6.probVect[0] = 0.2f;
    fnode6.probVect[1] = 0.8f;
	// end of filling information of node 6
   
    TreeNodeFields fields[7];
    fields[0] = fnode0;
    fields[1] = fnode1;
    fields[2] = fnode2;
    fields[3] = fnode3;
    fields[4] = fnode4;
    fields[5] = fnode5;
    fields[6] = fnode6;
   
   
    pCPDChild->UpdateTree(pGraphT,fields);
    pBNet->AttachFactor(pCPDChild);
    return pBNet;
}

int GibbsForTreeBNet()
{
    std::cout<<std::endl<<"Gibbs for net that contains tree node"<<std::endl;
	int ret = 1; 
    CBNet *pBNet =	CreateTestTabularNetWithDecTreeNodeBNet();
    std::cout<<"BNet has been created \n";
    const int numOfObsNds  = 2;
    const int obsNds[]     = { 0, 1 };
    int i;
    
    printf("number of observed nodes: %d\n", numOfObsNds);
    printf("observed nodes:  ");

    for( i = 0; i < numOfObsNds; ++i)
        printf("%d  ", obsNds[i]);
    printf("\n");

    valueVector obsNdsVals(numOfObsNds);
    obsNdsVals[0].SetInt(0);
    obsNdsVals[1].SetInt(1);

    printf("observed values: ");
    for( i = 0; i < numOfObsNds; ++i)
        printf("%d  ", obsNdsVals[i].GetInt());
    printf("\n");
	
    CEvidence *pEvidence = CEvidence::Create( pBNet, numOfObsNds, obsNds,
        obsNdsVals );
    std::cout<<"evidence has been generated \n";
	
    CGibbsSamplingInfEngine *pGibbsInf;
	
    pGibbsInf = CGibbsSamplingInfEngine::Create( pBNet );
    
    pGibbsInf->SetMaxTime( 10000);
    pGibbsInf->SetBurnIn( 1000 );


    intVecVector queries(1);
    queries[0].clear();
    queries[0].push_back( 10 );
 
    pGibbsInf->SetQueries( queries );    
    pGibbsInf->EnterEvidence( pEvidence );

    const int querySz1 = 1;
    const int query1[] = { 10 };
    printf("number of Marginal nodes: %d\n", querySz1);
    printf("Marginal nodes: ");
    for( i = 0; i < querySz1; ++i)
        printf("%d  ", query1[i]);
    printf("\n");

    
    pGibbsInf->MarginalNodes( query1, querySz1 );
    
    const CPotential *pQueryPot;
    pQueryPot = pGibbsInf->GetQueryJPD();
    
    std::cout<<"result of gibbs"<<std::endl<<std::endl;
    pQueryPot->Dump();
   
	delete pEvidence;
    delete pGibbsInf;
	delete pBNet;
	

    return ret;
}

void initAGibbsInference()
{
    trsReg(func_name, test_desc, test_class, testGibbsInference);
}
