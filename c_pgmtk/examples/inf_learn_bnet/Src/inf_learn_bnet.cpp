/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      inf_learn_bnet.cpp                                          //
//                                                                         //
//  Purpose:   Showing of inference&learning for simple BNet model         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnl_dll.hpp"
#include <fstream>

PNL_USING

CEvidence* CreateEvidenceForWSBNet( const CBNet* pWSBnet )
{
    //make one node observed
    int nObsNds = 1;
    //the observed node is 0
    int obsNds[] = { 0 };
    //node 0 takes its second value (from two possible values {0, 1})
    valueVector obsVals;
    obsVals.resize(1);
    obsVals[0].SetInt(1);
    CEvidence* pEvid = CEvidence::Create( pWSBnet, nObsNds, obsNds, obsVals );
    return pEvid;
}


int IsTheModelEqual( const CBNet* bnet1, const CBNet * bnet2, float epsilon )
{
    //compare every potential
    int numberOfNodes = bnet1->GetNumberOfNodes();
    
    int ret = 1;
    
    for( int i = 0; i < numberOfNodes; i++ )
    {
        if( !(bnet1->GetFactor(i)->IsFactorsDistribFunEqual( 
            bnet2->GetFactor(i), epsilon )))
        {
            ret = 0;
        }
    }
    
    return ret;
}

int main()
{
    //create Water - Sprinkler BNet
    CBNet* pWSBnet = pnlExCreateWaterSprinklerBNet();
    
    //display content of Graph
    pWSBnet->GetGraph()->Dump();
    
    //create simple evidence for node 0 from BNet
    CEvidence* pEvidForWS = CreateEvidenceForWSBNet(pWSBnet);
    
    //create Naive inference for BNet
    CNaiveInfEngine* pNaiveInf = CNaiveInfEngine::Create( pWSBnet );
    
    //enter evidence created before
    pNaiveInf->EnterEvidence( pEvidForWS );
    
    //get a marginal for query set of nodes
    int numQueryNds = 2;
    int queryNds[] = { 1, 3 };
    
    pNaiveInf->MarginalNodes( queryNds, numQueryNds );
    const CPotential* pMarg = pNaiveInf->GetQueryJPD();
    
    intVector obsNds;
    pConstValueVector obsVls;
    pEvidForWS->GetObsNodesWithValues(&obsNds, &obsVls);
    
    int i;
    for( i = 0; i < obsNds.size(); i++ )
    {
	std::cout<<" observed value for node "<<obsNds[i];
	std::cout<<" is "<<obsVls[i]->GetInt()<<std::endl;
    }
    
    int nnodes;
    const int* domain;
    pMarg->GetDomain( &nnodes, &domain );
    std::cout<<" inference results: \n";
    
    std::cout<<" probability distribution for nodes [ ";
    
    for( i = 0; i < nnodes; i++ )
    {
	std::cout<<domain[i]<<" ";
    }
    
    std::cout<<"]"<<std::endl;
    
    CMatrix<float>* pMat = pMarg->GetMatrix(matTable);
    
    // graphical model hase been created using dense matrix
    // so, the marginal is also dense
    EMatrixClass type = pMat->GetMatrixClass();
    if( ! ( type == mcDense || type == mcNumericDense || type == mc2DNumericDense ) )
    {
	assert(0);
    }
    
    int nEl;
    const float* data;
    static_cast<CNumericDenseMatrix<float>*>(pMat)->GetRawData(&nEl, &data);
    for( i = 0; i < nEl; i++ )
    {
	std::cout<<" "<<data[i];
    }
    std::cout<<std::endl;
    
    
    delete pEvidForWS;
    delete pNaiveInf;
    
    //start learning for this model
    //create WS BNet with different matrices
    
    std::cout<<"Learning procedure \n ";
    CGraph *pGraph = CGraph::Copy( pWSBnet->GetGraph() ); 
    CModelDomain *pMD = pWSBnet->GetModelDomain();
    
    CBNet* pWSLearnBNet = CBNet::CreateWithRandomMatrices( pGraph, pMD );
    
    //loading data from file 
    const char * fname = "../c_pgmtk/examples/Data/casesForWS";
    
    pEvidencesVector evVec;
    
    if( ! CEvidence::Load(fname,  &evVec, pMD) )
    {
	printf("can't open file with cases");
	exit(1);
    }
    int numOfSamples = evVec.size();
    std::cout<<"Number of cases for learning = "<<numOfSamples<<std::endl;
    
    //create learning engine
    CEMLearningEngine *pLearn = CEMLearningEngine::Create( pWSLearnBNet );
    
    //set data for learning
    
    pLearn->SetData( numOfSamples, &evVec.front() );
    pLearn->Learn();
    
    //get information from learned model
    int nFactors = pWSLearnBNet->GetNumberOfFactors();
    const CFactor *pCPD;
    const CNumericDenseMatrix<float> *pMatForCPD;
    int numOfEl;
    const float *dataCPD;
    int f;
    for( f = 0; f < nFactors; f++ )
    {
	
	std::cout<<std::endl<<" probability distribution for node "<<f<<std::endl; 
	pCPD = pWSLearnBNet->GetFactor(f);
	//all matrices are dense 
	pMatForCPD = static_cast<CNumericDenseMatrix<float> *>
	    (pCPD->GetMatrix(matTable));
	pMatForCPD->GetRawData( &numOfEl, &dataCPD );
	int j;
	for( j = 0; j < numOfEl; j++ )
	{
	    std::cout<<" "<<dataCPD[j];
	}
    }
    std::cout<<std::endl;
    //compare information from learned model with initial model
    //both BNet have the same topology and node types 
    //- we need only to compare CPDs
    //need to set tolerance
    float epsilon = 1e-1f;
    int isEqual = IsTheModelEqual( pWSBnet, pWSLearnBNet, epsilon );
    
    std::cout << " The model was learned. The learning was " << std::endl;
    
    if( isEqual )
    {
        std::cout << " successful " << std::endl;
    }
    else
    {
        std::cout << " unsuccessful " << std::endl;
    }
    
    int ev;
    for( ev = 0; ev < evVec.size(); ev++ )
    {
	delete evVec[ev];
    }
    delete pLearn;
    delete pWSBnet;
    delete pWSLearnBNet;
    
    
    getchar();
    return 0;
}