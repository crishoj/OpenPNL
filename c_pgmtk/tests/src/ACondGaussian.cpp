/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ACondGaussian.cpp                                           //
//                                                                         //
//  Purpose:   Test on inference for Conditional Gaussian model            //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "tConfig.h"
#include <fstream>
#include "tCreateIncineratorBNet.hpp"
#include "pnl_dll.hpp"


static char func_name[] = "testConditionalGaussianInf";

static char* test_desc = "Provide inference for conditional Gaussian model with all continuous nodes observed";

static char* test_class = "Algorithm";


int testConditionalGaussianInf()
{
    PNL_USING
   
    int ret = TRS_OK;
    float eps = 1e-4f;
   
    int i;
 
    const int nSimnodes = 3;
	
    int numOfNeigh[] = { 1, 1, 2 };
	
    int neigh0[] = { 2 };
    int neigh1[] = { 2 };
    int neigh2[] = { 0, 1  };
	
    ENeighborType orient0[] = { ntChild };
    ENeighborType orient1[] = { ntChild };
    ENeighborType orient2[] = { ntParent, ntParent };
	
    int *neigh[] = { neigh0, neigh1, neigh2 };
    ENeighborType *orient[] = { orient0, orient1, orient2 };

    CGraph* pGraph1 = CGraph::Create( nSimnodes, numOfNeigh, neigh, orient );
    pGraph1->Dump();

    int numNodeTypes = 2;
    CNodeType* nTypes = new CNodeType[numNodeTypes];
    nTypes[0].SetType( 1, 2 );
    nTypes[1].SetType( 0, 1 );

    intVector nodeAssociation;
    nodeAssociation.assign( nSimnodes, 1 );
    nodeAssociation[0] = 0;
    
    CBNet* pBNet1 = CBNet::Create( nSimnodes, numNodeTypes, nTypes,
        &nodeAssociation.front(), pGraph1 );
    
    CModelDomain* pMD = pBNet1->GetModelDomain();

    pBNet1->AllocFactors();
    
    //create factors
    int node = 0;
    const CNodeType* nt = &nTypes[0];
    float data[] = { 0.95f, 0.05f  };
    CTabularCPD* tabCPD = CTabularCPD::Create( &node, 1, pMD, data );
    pBNet1->AttachFactor( tabCPD );
    node = 1;
    float dataM[] = { 0.0f };
    float dataC[] = { 1.0f };
    nt = &nTypes[1];
    
    CGaussianCPD* gauCPD = CGaussianCPD::Create( &node, 1, pMD );
    gauCPD->AllocDistribution( dataM, dataC, 1.0f, NULL );
    node = 2;
    pConstNodeTypeVector nTypesT;
    nTypesT.assign( 3, (CNodeType const *)0 );
    nTypesT[0] = &nTypes[0];
    nTypesT[1] = &nTypes[1];
    nTypesT[2] = &nTypes[1];
    pBNet1->AttachFactor( gauCPD );
    intVector dom;
    dom.assign( 3, 0 );
    for( i = 0; i < 3; i++ )
    {
        dom[i] = i;
    }

    CGaussianCPD* pGauCPD1 = CGaussianCPD::Create( &dom.front(), 3, pMD );
    float dataM1[] = { -1.0f };
    float dataC1[] = { 2.0f };
    float dataW1 = 0.5f;
    const float* W1 = &dataW1;
    int parentComb = 0;
    pGauCPD1->AllocDistribution(dataM1, dataC1, 0.0f, &W1, &parentComb);
    float dataM2[] = { 0.0f };
    float dataC2[] = { 1.0f };
    parentComb = 1;
    dataW1 = -0.5f;
    pGauCPD1->AllocDistribution( dataM2, dataC2, 0.0f, NULL, &parentComb );
    //get sizes for weight matrix
    const pConstNodeTypeVector* argTypes = pGauCPD1->GetArgType();
    int childSize = (*argTypes)[2]->GetNodeSize();
    int contParentSize = (*argTypes)[1]->GetNodeSize();
    intVector lineSizes;
    lineSizes.assign(2,childSize);
    lineSizes[1] = contParentSize;
    C2DNumericDenseMatrix<float>* weight = 
        C2DNumericDenseMatrix<float>::Create( &lineSizes.front(), &dataW1 );
    pGauCPD1->AttachMatrix( weight, matWeights, 0, &parentComb );
    float coeff = pGauCPD1->GetCoefficient( &parentComb );
    //compute coeff by hands and check
    float p1 = (float)pow((double)( 2 * PNL_PI ),( 0.5 ));
    float detCov = 1.0f;
    float p2 = (float)pow((double)detCov,0.5);
    float coeffComputed = 1/(p1*p2);
    if( fabs(coeff - coeffComputed) > eps )
    {
        ret = TRS_FAIL;
    }
    pBNet1->AttachFactor( pGauCPD1 );
    
    //check some methods of Conditional Gaussian
    CDistribFun* CondGauCPD = pGauCPD1->GetDistribFun();
    CDistribFun* copyCGCPD = CondGauCPD->Clone();
    C2DNumericDenseMatrix<float>* mean2 = static_cast<
        C2DNumericDenseMatrix<float>*>(copyCGCPD->GetMatrix( matMean, -1,
        &parentComb ));
    intVector indexMean = intVector(2, 0);
    mean2->SetElementByIndexes( 5.0f, &indexMean.front() );
    copyCGCPD->IsEqual( CondGauCPD, 1e-5f );
    (*copyCGCPD) = (*CondGauCPD);
    //check operator = for conditional gaussian
    (*copyCGCPD) = (*copyCGCPD);
    delete copyCGCPD;
    CCondGaussianDistribFun* CondGauClone = static_cast<CCondGaussianDistribFun*>(
        CondGauCPD->CloneWithSharedMatrices());
    delete CondGauClone;
    //check some more methods of conditional Gaussian
    //create default distribution
    //get node types
    intVector vars;
    vars.assign(2,1);
    vars[1] = 2;
    pConstNodeTypeVector nTypesGau;
    pGauCPD1->GetModelDomain()->GetVariableTypes( vars, &nTypesGau );
    CGaussianDistribFun* pDefault = 
        CGaussianDistribFun::CreateUnitFunctionDistribution( 2,
        &nTypesGau.front(), 0, 0 );
    const pConstNodeTypeVector* nodeTypes = CondGauCPD->GetNodeTypesVector();
    CCondGaussianDistribFun* CondGauSparse = CCondGaussianDistribFun::Create(  
        0, 3, &nodeTypes->front(), 0, pDefault  );
    delete pDefault;
    CGaussianDistribFun* pDef = 
        static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
        CondGauSparse->GetMatrixWithDistribution())->GetDefaultValue();
    pDef->Dump();
	trsWrite( TW_RUN|TW_DEBUG|TW_CON, "\n" );
    //CondGauSparse->AllocDistribFun( &parentComb, 1, 0, 0 );
    CondGauSparse->SetCoefficient( 0.0f, 0, &parentComb );
	CondGauSparse->AllocMatrix( dataM1, matMean, -1, &parentComb );
	CondGauSparse->AllocMatrix( dataC1, matCovariance,  -1, &parentComb );
    CondGauSparse->AllocMatrix( &dataW1, matWeights, 0, &parentComb );
    //create copy of conditional Gaussian Sparse distribution
    CCondGaussianDistribFun* condGauSpCopy = CCondGaussianDistribFun::Copy( 
       CondGauSparse );
    CSparseMatrix<CGaussianDistribFun*>* matDistr = 
        static_cast<CSparseMatrix<CGaussianDistribFun*>*>(
        condGauSpCopy->GetMatrixWithDistribution());
    //created copy of the matrix is absolutely the same,
    //it doesn't create the copies of distributions - copy pointers
    CMatrix<CGaussianDistribFun*>* matDistrCopy = 
        CSparseMatrix<CGaussianDistribFun*>::Copy(matDistr);
	//can convert to dense
	CMatrix<CGaussianDistribFun*>* matDistrDense = matDistrCopy->ConvertToDense();
	//and to sparse
	CMatrix<CGaussianDistribFun*>* matDistrSparse = matDistrCopy->ConvertToSparse();
	//can change the matrix and after that use operator = for checking the result
	CGaussianDistribFun* newDistr = CGaussianDistribFun::CreateInMomentForm( 0,
		2, &nTypesGau.front(), NULL, NULL, NULL );
	newDistr->CreateDefaultMatrices();
	parentComb = 0;
	matDistrSparse->SetElementByIndexes( newDistr, &parentComb );
	matDistrSparse->SetDataFromOtherMatrix( matDistr );

	delete newDistr;

	delete matDistrDense;
	delete matDistrSparse;
	delete matDistrCopy;
	delete condGauSpCopy;

	//wish to enter discrete evidence
	node = 0;
	int val = 0;
	CCondGaussianDistribFun* condGauSpWithEv = 
		CondGauSparse->EnterDiscreteEvidence( 1, &node, &val,
		pMD->GetObsTabVarType() );
	condGauSpWithEv->Dump();
	trsWrite( TW_RUN|TW_DEBUG|TW_CON, "\n" );
	delete condGauSpWithEv;
	delete CondGauSparse;
	
    //moment
    try
    {
        //create invalid gaussian distribution - it should be conditional Gaussian
        const pConstNodeTypeVector* ntCondGau  = pGauCPD1->GetArgType();
        CGaussianDistribFun* pGauDistr = CGaussianDistribFun::CreateInMomentForm(0, 3,
            &ntCondGau->front(), dataM1,  dataC1, &W1);
        delete pGauDistr;
        //if we catch the exception the code won't run 
        ret = TRS_FAIL;
    }
    catch (pnl::CException ex)
    {
        //the distribution function can't be created here
    }

    //canonical
    try
    {
        //create invalid gaussian distribution - it should be conditional Gaussian
        const pConstNodeTypeVector* ntCondGau  = pGauCPD1->GetArgType();
        CGaussianDistribFun* pGauDistr = CGaussianDistribFun::CreateInCanonicalForm(3,
            &ntCondGau->front(), dataM1,  dataC1, 0.0f);
        delete pGauDistr;
        //if we catch the exception the code won't run 
        ret = TRS_FAIL;
    }
    catch (pnl::CException ex)
    {
        //the distribution function can't be created here
    }

    //delta
    try
    {
        //create invalid gaussian distribution - it should be conditional Gaussian
        const pConstNodeTypeVector* ntCondGau  = pGauCPD1->GetArgType();
        CGaussianDistribFun* pGauDistr = CGaussianDistribFun::CreateDeltaDistribution(3,
            &ntCondGau->front(), dataM1);
        delete pGauDistr;
        //if we catch the exception the code won't run 
        ret = TRS_FAIL;
    }
    catch (pnl::CException ex)
    {
        //the distribution function can't be created here
    }

    //unit
    try
    {
        //create invalid gaussian distribution - it should be conditional Gaussian
        const pConstNodeTypeVector* ntCondGau  = pGauCPD1->GetArgType();
        CGaussianDistribFun* pGauDistr = CGaussianDistribFun::CreateUnitFunctionDistribution(3,
            &ntCondGau->front());
        delete pGauDistr;
        //if we catch the exception the code won't run 
        ret = TRS_FAIL;
    }
    catch (pnl::CException ex)
    {
        //the distribution function can't be created here
    }


    int nSimObsNodes = 2;
    int obsSimNodes[] = {  1, 2  };
    valueVector obsSimVals;
    obsSimVals.assign( 2, (Value)0 );
    obsSimVals[0].SetFlt(0.0f);
    obsSimVals[1].SetFlt(0.0f);
    CEvidence* evidSim = CEvidence::Create( pBNet1, nSimObsNodes, obsSimNodes,
        obsSimVals );

    CNaiveInfEngine* pSimNaiveInf = CNaiveInfEngine::Create( pBNet1 );
    pSimNaiveInf->EnterEvidence( evidSim );
    int discrNode = 0;
    pSimNaiveInf->MarginalNodes( &discrNode, 1 );

    CJtreeInfEngine* pSimJtreeInf = CJtreeInfEngine::Create( pBNet1 );
    pSimJtreeInf->EnterEvidence( evidSim );
    pSimJtreeInf->MarginalNodes( &discrNode, 1 );

    const CPotential* resSimPot = pSimNaiveInf->GetQueryJPD();
    resSimPot->Dump();
	trsWrite( TW_RUN|TW_DEBUG|TW_CON, "\n" );

    const CPotential* resSimJtPot = pSimJtreeInf->GetQueryJPD();
    resSimJtPot->Dump();
    trsWrite( TW_RUN|TW_DEBUG|TW_CON, "\n" );
  
    CBNet* pWasteBNet = tCreateIncineratorBNet();
    int nnodes = pWasteBNet->GetNumberOfNodes();    

    //start inference with all continuos nodes observed
    int nObsNodes = 7;
    int obsNodes[] = { 2, 3, 4, 5, 6, 7, 8 };
    valueVector obsVals;
    obsVals.assign( 7, (Value)0 );
    obsVals[0].SetFlt(-2.0f);
    obsVals[1].SetInt(1);
    obsVals[2].SetFlt(-1.5f);
    obsVals[3].SetFlt(5.0f);
    obsVals[4].SetFlt(-0.2f);
    obsVals[5].SetFlt(0.8f);
    obsVals[6].SetFlt(1.1f);
    CEvidence* fullGauEvid = CEvidence::Create( pWasteBNet, nObsNodes,
        obsNodes, obsVals );
    int obsTabNode = 3;
    fullGauEvid->ToggleNodeState(1, &obsTabNode);
    
    CJtreeInfEngine* pJtreeInf = CJtreeInfEngine::Create( pWasteBNet );
    pJtreeInf->EnterEvidence( fullGauEvid );
    CNaiveInfEngine* pNaiveInf = CNaiveInfEngine::Create( pWasteBNet );
    pNaiveInf->EnterEvidence( fullGauEvid );
    
    int numUnObsNodes = nnodes - nObsNodes + 1;
    int unObsNodes[] = { 0, 1, 3 };
    const CPotential* resPot;
    const CPotential* resPotOther;
    for( i = 0; i < numUnObsNodes; i++ )
    {
        pJtreeInf->MarginalNodes( &(unObsNodes[i]), 1 );
        resPot = pJtreeInf->GetQueryJPD();
        //resPot->Dump();
		//trsWrite( TW_RUN|TW_DEBUG|TW_CON, "\n" );

        pNaiveInf->MarginalNodes( &(unObsNodes[i]), 1 );
        resPotOther = pNaiveInf->GetQueryJPD();
        if( !resPot->IsFactorsDistribFunEqual( resPotOther, eps ) )
        {
            ret = TRS_FAIL;
        }
        //resPot->Dump();
		//trsWrite( TW_RUN|TW_DEBUG|TW_CON, "\n" );
        //resPotOther->Dump();
		//trsWrite( TW_RUN|TW_DEBUG|TW_CON, "\n" );
    }

    delete fullGauEvid;
    delete pJtreeInf;
    delete pNaiveInf;
    delete pWasteBNet;
    delete evidSim;
    delete pSimNaiveInf;
    delete pSimJtreeInf;
    delete pBNet1;
    delete []nTypes;
    return trsResult( ret, ret == TRS_OK ? "No errors" : 
		"Bad test on Conditional Gaussian inference");
}

void initAConditionalGaussianInf()
{
    trsReg(func_name, test_desc, test_class, testConditionalGaussianInf);
}
