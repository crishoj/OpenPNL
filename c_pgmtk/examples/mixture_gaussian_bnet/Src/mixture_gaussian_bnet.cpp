/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      mixture_gaussian_bnet.cpp                                   //
//                                                                         //
//  Purpose:   Showing of creation and inference for simple                //
//             Mixture Gaussian BNet model                                 //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"
//#include <fstream>

PNL_USING

CCPD * CreateTabularCPDForNode0( CModelDomain *pMD );
CCPD * CreateTabularCPDForNode1( CModelDomain *pMD );
CCPD * CreateGaussianCPDForNode2( CModelDomain *pMD );
CCPD * CreateMixtureGaussianCPDForNode3( CModelDomain *pMD );
CCPD * CreateGaussianCPDForNode4( CModelDomain *pMD );

CBNet * CreateValidityMixtureGaussianBNet()
{
    /*
     Let we want to create bayesian network
     
	     A
	     |
             V	 
	 B-->C-->D
	 
    where A is the discrete node; B, D are gaussian; C is mixture gaussian node
    
    In PNL graph for this model seems as
    
        0  1
         \ |
           v   
       2-->3-->4
      
       0 - Tabular node corresponding to node A
       2 - Gaussian node corresponing to node B
       4 - Gaussian node corresponing to node D

       1 - Tabular nodes. It is a special node - mixture node. 
	   It is used for storage summing coefficients for gaussians
	   It must be a last dicrete node among all discrete parents 
	   for gausian mixture node.
       3 - Gaussain mixture node.   
       Nodes 1 and 3 together correspond to node C.
    
    */
    
    // 1) First need to specify the graph structure of the model;
    
    const int numOfNds = 5;
    int numOfNbrs[numOfNds] = { 1, 1, 1, 4, 1 };
    
    int nbrs0[] = { 3 };
    int nbrs1[] = { 3 };
    int nbrs2[] = { 3 };
    int nbrs3[] = { 0, 1, 2, 4 };
    int nbrs4[] = { 3 };
    
    // neighbors can be of either one of three following types:
    // a parent, a child or just a neighbor - for undirected graphs.
    // if a neighbor of a node is it's parent, then neighbor type is ntParent
    // if it's a child, then ntChild and if it's a neighbor, then ntNeighbor
    ENeighborType nbrsTypes0[] = { ntChild };
    ENeighborType nbrsTypes1[] = { ntChild };
    ENeighborType nbrsTypes2[] = { ntChild };
    ENeighborType nbrsTypes3[] = { ntParent, ntParent, ntParent, ntChild };
    ENeighborType nbrsTypes4[] = { ntParent };
    
    int *nbrs[] = { nbrs0, nbrs1, nbrs2, nbrs3, nbrs4 };
    
    ENeighborType *nbrsTypes[] = { nbrsTypes0, nbrsTypes1, nbrsTypes2,
	nbrsTypes3, nbrsTypes4 };
    
    CGraph* pGraph = CGraph::Create( numOfNds, numOfNbrs, nbrs, nbrsTypes );
    
    // 2) Creation of the Model Domain.
    CModelDomain* pMD;

    nodeTypeVector variableTypes;
    
    // There are 3 several types of nodes in the example: 
    // 1) discrete nodes 0, 1with 2 possible values 
    // 2) scalar continuous (gaussian) nodes 2, 3
    // 3) multivariate gaussian node 4 (consists of 2 values) 
    int nVariableTypes = 3;
    variableTypes.resize( nVariableTypes );
    
    variableTypes[0].SetType( 1, 2 );
    variableTypes[1].SetType( 0, 1 );
    variableTypes[2].SetType( 0, 2 );
    
    intVector variableAssociation;  
    int nnodes = pGraph->GetNumberOfNodes();
    variableAssociation.assign(nnodes, 1);
    variableAssociation[0] = 0;
    variableAssociation[1] = 0;
    variableAssociation[4] = 2;

    pMD = CModelDomain::Create( variableTypes, variableAssociation );
    
    // 2) Creation base for BNet using Graph, and Model Domain
    
    CBNet *pBNet = CBNet::Create(pGraph, pMD);
    
    // 3)Allocation space for all factors of the model
    pBNet->AllocFactors();
    
    
    // 4) Creation factors and attach their to model
    CCPD *pCPD;
    
    pCPD = CreateTabularCPDForNode0( pMD );
    pBNet->AttachFactor( pCPD );
    
    pCPD = CreateTabularCPDForNode1( pMD );
    pBNet->AttachFactor( pCPD );
    
    pCPD = CreateGaussianCPDForNode2( pMD );
    pBNet->AttachFactor( pCPD );
    
    pCPD = CreateGaussianCPDForNode4( pMD );
    pBNet->AttachFactor( pCPD );
    
    pCPD = CreateMixtureGaussianCPDForNode3( pMD );
    pBNet->AttachFactor( pCPD );

    return pBNet;
}


CCPD* CreateTabularCPDForNode0( CModelDomain *pMD )
{
    //create tabular CPD for domain [0]
    int nnodes = 1;
    int domain[] = { 0 };
    float table[] = { 0.7f, 0.3f};
    
    return CTabularCPD::Create( domain, nnodes, pMD, table );
    
}

CCPD* CreateTabularCPDForNode1(CModelDomain *pMD)
{
    //create tabular CPD for domain [1]
    int nnodes = 1;
    int domain[] = { 1 };
    float table[] = { 0.4f, 0.6f};
    return CTabularCPD::Create( domain, nnodes, pMD, table );
}

CCPD* CreateGaussianCPDForNode2( CModelDomain *pMD )
{
    //create gaussian CPD for domain [2]
    int domain[] = { 2 };
    int nnodes = 1;
    float mean0 = 0.0f;
    float cov0 = 1.0f;

    CGaussianCPD* pCPD = CGaussianCPD::Create( domain, nnodes, pMD );
    pCPD->AllocDistribution( &mean0, &cov0, 1.0f, NULL );
    return pCPD;

    
}

CCPD *CreateGaussianCPDForNode4( CModelDomain *pMD )
{
    //create gaussian CPD for domain [3, 4]
    // node 4 hase the node 3 as parent
    int nnodes = 2;
    int domain[] = { 3, 4 };
    
    float mean[] = { 8.0f, 1.0f };
    float cov[] = { 0.01f, 0.0f, 0.0f, 0.02f };
    float weight[] = { 0.01f, 0.03f };
    
    CGaussianCPD* pCPD = CGaussianCPD::Create( domain, nnodes, pMD );
    
    if(1)
    {
	const float *pData = weight;
	pCPD->AllocDistribution( mean, cov, 0.5f, &pData );
    }
    else
    {
	//Create Factor using attach matrix
	
	int range[] = {2, 1};
	C2DNumericDenseMatrix<float> *pMatMean = C2DNumericDenseMatrix<float>::Create( range, mean );
	pCPD->AttachMatrix( pMatMean, matMean );
	
	C2DNumericDenseMatrix<float> *pMatWeight = C2DNumericDenseMatrix<float>::Create( range, weight );
	pCPD->AttachMatrix( pMatWeight, matWeights, 0 );
	
	range[1] = 2;
	C2DNumericDenseMatrix<float> *pMatCov = C2DNumericDenseMatrix<float>::Create( range, cov );
	pCPD->AttachMatrix( pMatCov, matCovariance );
    }
    return pCPD;
    
}

CCPD *CreateMixtureGaussianCPDForNode3( CModelDomain *pMD )
{
    // create mixture gaussian CPD for domain [0, 1, 2, 3]
    // node 3 hase the nodes 0, 1, 2 as parents
    // last discrete node among all dicrete nodes in domain 
    // is the special node - mixture node
    // in this case node 1 is the mixture node
    int nnodes = 4;
    int domain[] = { 0, 1, 2, 3 };
    
    //data for probability distribution 
    
    //if node 0 = 0, node 1 = 0
    float mean00 =   1.0f;
    float cov00 = 0.005f; 
    float weight00 = 0.02f;
    
    //if node 0 = 1, node 1 = 0
    float mean10 =  -5.0f;
    float cov10 = 0.01f;
    float weight10 = 0.01f;
    
    //if node 0 = 0, node 1 = 1
    float mean01 =  -3.0f;
    float cov01 = 0.01f;
    float weight01 = 0.01f;
    
    //if node 0 = 1, node 1 = 1
    float mean11 =   2.0f;
    float cov11 = 0.002f;
    float weight11 = 0.05f;
    
        
    float table[] = { 0.4f, 0.6f };
    // this table must be equal to table for node 1,
    // because node 1 (mixture node) is used for storage 
    // summing coefficients for gaussians
    CMixtureGaussianCPD* pCPD = CMixtureGaussianCPD::Create( domain,
        nnodes, pMD, table );
    
    int parVal[] = {0, 0};
    float *pDataWeight;
    
    pDataWeight = &weight00;
    pCPD->AllocDistribution( &mean00, &cov00, 2.0f, &pDataWeight, parVal );
    
    parVal[1] = 1;
    pDataWeight = &weight01;
    pCPD->AllocDistribution( &mean01, &cov01, 1.0f, &pDataWeight, parVal );
    
    parVal[0] = 1;
    pDataWeight = &weight11;
    pCPD->AllocDistribution( &mean11, &cov11, 1.0f, &pDataWeight, parVal );
    
    parVal[1] = 0;
    pDataWeight = &weight10;
    pCPD->AllocDistribution( &mean10, &cov10, 1.0f, &pDataWeight, parVal );
    
    return pCPD;
}




int main()
{
    // create mixture gaussian BNet
    CBNet *pBNet = CreateValidityMixtureGaussianBNet();
    const CGraph *pGraph = pBNet->GetGraph();
    std::cout<<" Graph";
    pGraph->Dump();
    

    // create simple evidence for nodes 2, 3, 4 from BNet
    CEvidence *pEv;
    const CModelDomain *pMD = pBNet->GetModelDomain();
    
    // Let nodes 2, 3, 4 are observed
    int nObsNds = 3;
    int obsNds[] = { 2, 3, 4 };
    

    // 
    valueVector vls;
    
    int nVls = 0; 
    int i;
    for( i = 0; i < nObsNds; i++ )
    {
	nVls += pMD->GetNumVlsForNode( obsNds[i] );
    }
    // evidence for node 2 consists of 1 value
    // evidence for node 3 consists of 1 value
    // evidence for node 4 consists of 2 values
    // so, nVls = 4
    
    vls.resize(nVls);
    
    for( i = 0; i < nVls; i++ )
    {
	float val = pnlRand( -1.0f, 1.0f );
	vls[i].SetFlt( val );
    }
    
    pEv = CEvidence::Create( pMD, nObsNds, obsNds, vls );

    // create Junction tree inference engine
    CJtreeInfEngine *pJTreeInf = CJtreeInfEngine::Create( pBNet );
    
    // enter evidence created before and started inference procedure
    pJTreeInf->EnterEvidence(pEv);

    //get a marginal for query
    int numQueryNds = 1;
    int queryNds[] = { 0 };
    pJTreeInf->MarginalNodes( queryNds, numQueryNds );
    const CPotential *pQueryPot = pJTreeInf->GetQueryJPD();
    // node 0 is discrete, then query potential is tabular

    const CDistribFun *pDistribFun = pQueryPot->GetDistribFun();
    const CMatrix<float> *pMat = pDistribFun->GetMatrix( matTable );
    
    int node0Sz = pMD->GetVariableType(0)->GetNodeSize();
       
    int index;
    for( index = 0; index< node0Sz; index++ )
    {
	float val = pMat->GetElementByIndexes( &index );
	std::cout<<" Probability of event node 0 take on a value ";
	std::cout<<index<<" is ";
	std::cout<<val<<std::endl;
    }
    // distribution of the query hase dense matrrix
    // the row data is
    int nEl;
    const float *data;
    static_cast<const CNumericDenseMatrix<float> *>( pMat )->GetRawData( &nEl, &data );
    std::cout<<" The raw data of the query distribution is: ";
   
    for( i = 0; i < nEl; i++ )
    {
	std::cout<<data[i]<<" ";
    }
    std::cout<<std::endl;

    float ll = pJTreeInf->GetLogLik();
    std::cout<<" Log Likelihood of the evidence is "<<ll<<std::endl;
    
    delete pEv;
    delete pBNet;
    delete pJTreeInf;
    delete pMD;
    
    return 0;
}