/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      APearlInfEngine.cpp                                         //
//                                                                         //
//  Purpose:   Test on Pearl inference algorithm on BNet model             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <math.h>
#include <float.h>
// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testPearlInfEngine";

static char* test_desc = "Provide Pearl inference";

static char* test_class = "Algorithm";

int testPearlInfEngine()
{
    int ret = TRS_OK;
    int i=0, j=0, k=0, t=0;
    int nnodes = -1;
    int numNt = -1;
    //read number of nodes in Graph
    while((nnodes<=1)||(nnodes>15))
    {
	trsiRead( &nnodes, "3", "Number of Nodes in Model");
    }

    float eps = -1.0f;
    while(eps<=0)
    {
	trssRead( &eps, "1e-4f", "accuracy in test");
    }
    //read number of node types in model
    while((numNt<=0)||(numNt>nnodes))
    {
	trsiRead( &numNt, "1", "Number of node types in Model");
    }

    int seed1 = pnlTestRandSeed();
    //create string to display the value
    char *value = new char[20];
#if 0
    _itoa(seed1, value, 10);
#else
    sprintf( value, "%d", seed1 );
#endif
    trsiRead(&seed1, value, "Seed for srand to define NodeTypes etc.");
    delete []value;
    trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);
    srand(seed1);

    CNodeType *nodeTypes = new CNodeType [numNt];

    for(i=0; i<numNt; i++)
    {
	nodeTypes[i] = CNodeType(1,i+2);
    }
    int *nodeAssociation = (int*)trsGuardcAlloc( nnodes, sizeof(int) );

    for(i = 0; i<nnodes; i++ )
    {
	nodeAssociation[i] = rand()%(numNt);
    }

    //to create graph we need to have an algorithm of creation DAG
    //to make dag we use arrays of ancestors for every node
    //and create arcs only to non-ancestors
mark:
    intVecVector ancestors(nnodes);
    intVecVector nonAncestors;
    intVector nonAnc;
    intVector anc;
    for ( i = 0; i < nnodes; i++ )
    {
	for( j = i + 1; j < nnodes; j++ )
	{
	    if(j!=i)
	    {
		nonAnc.push_back(j);
	    }
	}
	nonAncestors.push_back(nonAnc);
	nonAnc.clear();
    }
    //create random graph - number of nodes for every node is rand too
    intVecVector neighbors(nnodes);
    neighborTypeVecVector orientation(nnodes);

    int **neighb = (int **)trsGuardcAlloc(nnodes, sizeof(int *));
    ENeighborType **orient = (ENeighborType **)trsGuardcAlloc(nnodes,
	sizeof(ENeighborType *));
    int *numOfNeighb = (int *)trsGuardcAlloc(nnodes, sizeof(int));
    //to fill the same in Matlab
    double *dag = (double *)trsGuardcAlloc(nnodes*nnodes, sizeof(double));
    for(i=0; i<nnodes*nnodes; i++)
    {
	dag[i] = 0.0;
    }
    int lineNum = 0;//to fill dag - it will be fill columnwise
    int numOfChild = 0;
    int child = 0;
    int index = 0;
    for(i=0; i<nnodes; i++)
    {
	if(!nonAncestors[i].size()) break;//we can't divide by zero
	numOfChild = rand()%(2/*nonAncestors[i].size()*/);
	//add only numbers more than i can be children
	intVector CanBeChildren = intVector(nonAncestors[i].begin(),
	    nonAncestors[i].begin()+nonAncestors[i].size());
	for(j=0; j<numOfChild; j++)
	{
	    index = rand()%(CanBeChildren.size());
	    child = CanBeChildren[index];
	    CanBeChildren.erase(CanBeChildren.begin()+index);
	    //fill neighbors and orientation
	    neighbors[i].push_back(child);
	    orientation[i].push_back(ntChild);
	    neighbors[child].push_back(i);
	    orientation[child].push_back(ntParent);
	    //fill dag columnwise nums from 1 to nnodes
	    lineNum = (child)*nnodes+i;
	    dag[lineNum] = 1.0;
	    //remove values to support acyclic property
	    ancestors[child].push_back(i);
            intVector::iterator point = std::find(nonAncestors[child].begin(), nonAncestors[child].end(),i);
	    if( point!=nonAncestors[child].end())
	    {
		nonAncestors[child].erase(point);
	    }
	    for (k=0; k<ancestors[i].size(); k++)
	    {
		//we need to put all ancestors of node i to ancestors[child]
		//and remove them from nonAncestors of child
		t = (ancestors[i])[k];
		ancestors[child].push_back(t);
                intVector::iterator pointNew = std::find(nonAncestors[child].begin(), nonAncestors[child].end(),t);
		if(pointNew!=nonAncestors[child].end())
		{
		    nonAncestors[child].erase(pointNew);
		}
	    }
	}
	CanBeChildren.clear();
    }
    for(i=0; i<nnodes; i++)
    {
	numOfNeighb[i] = neighbors[i].size();
	if( numOfNeighb[i]==0 )
	{
	    neighbors.clear();
	    orientation.clear();
	    ancestors.clear();
	    nonAncestors.clear();
	    nonAnc.clear();
	    goto mark;
	}
	//neighb[i] = new int [numOfNeighb[i]];
	//orient[i] = new int [numOfNeighb[i]];
	neighb[i] = &neighbors[i].front();
	orient[i] = &orientation[i].front();
    }

    CGraph* theGraph = CGraph::Create( nnodes, numOfNeighb, neighb,
	orient);

    CBNet *myModel = CBNet::Create(	nnodes, numNt, nodeTypes,
	nodeAssociation, theGraph );

    CModelDomain* pMD = myModel->GetModelDomain();

    myModel->GetGraph()->Dump();
    //we creates every factor - it is CPD

    //number of factors is the same as number of nodes - one CPD per node
    CFactor **myParams = new CFactor*[nnodes];
    //to create factors we need to create their domains and CPD
    int **domains = (int **)trsGuardcAlloc(nnodes, sizeof(int *));
    int *domainsSizes = (int *)trsGuardcAlloc(nnodes, sizeof(int));
    int num = 0;
    int counter1 = 0;
    for(i=0; i<nnodes; i++)
    {
	num = 1;// to add the node - it is child in this domain
	for(j=0; j<numOfNeighb[i]; j++)
	{
	    if((orient[i])[j]==0)
	    {
		num++;
	    }
	}
	domainsSizes[i] = num;
	//alloc memory for every domain
	domains[i] = (int *)trsGuardcAlloc(num, sizeof(int));
	counter1 = 0;
	for(j=0; j<numOfNeighb[i]; j++)
	{
	    if((orient[i])[j]==0)
	    {
		(domains[i])[counter1] = neighb[i][j];
		counter1++;
	    }
	}
	domains[i][num-1] = i;
    }
    //compute Data sizes for every Factor
    int *dataSizes = (int *)trsGuardcAlloc(nnodes, sizeof(int));
    //create array of node sizes for Matlab
    double *NodeSizes = (double*)trsGuardcAlloc(nnodes, sizeof(double));
    for(i=0; i<nnodes; i++)
    {
	NodeSizes[i] = (double)(myModel->GetNodeType(i))->GetNodeSize();
	dataSizes[i] = 1;
	for(j=0; j<domainsSizes[i]; j++)
	{
	    dataSizes[i] = ((int)dataSizes[i])*(myModel->GetNodeType((domains[i])
		[j]))->GetNodeSize();
	}
    }
    //create container for Factors
    myModel->AllocFactors();
    //create CPD
    //create array of data for every parameter
    float **Datas = (float**)trsGuardcAlloc(nnodes, sizeof(float*));
    for(i=0; i<nnodes; i++)
    {
	Datas[i] = (float *)trsGuardcAlloc(dataSizes[i], sizeof(float));
    }
    int domSize=0;
    //create temporary storage for normalized CPD
    //CTabularCPD* tempCPD = NULL;
    for(i=0; i<nnodes; i++)
    {
	domSize = domainsSizes[i];

	myParams[i] = CTabularCPD::Create( domains[i], domSize, pMD);
	int childSize = pMD->GetVariableType((domains[i])[domSize-1])->GetNodeSize();
	for(j=0; j<dataSizes[i]; j++)
	{
	    if( !(j%childSize) )
	    {
		Datas[i][j] = 1.0f;
	    }
	    else
	    {
		Datas[i][j] = (float)(rand()%10);
	    }
	}
	myParams[i]->AllocMatrix(Datas[i], matTable);
	(static_cast<CTabularCPD*>(myParams[i]))->NormalizeCPD();
	//checking all CPD: are they finite?
	/*	CNumericDenseMatrix<float> *pTempMatr = tempCPD->GetMatrix(matTable);
	const pnl::floatVector*tempVect = pTempMatr->GetVector();
	for(pnl::floatVector::const_iterator it = tempVect->begin();
	it != tempVect->end(); it++)
	{
	assert(_finite(*it));
	}
	*/
	myModel->AttachFactor((CTabularCPD*)myParams[i]);
    }
    //now we can start inference
    //first we start inference without evidence
    //create evidence
    CEvidence *myEvid = CEvidence::Create(myModel, 0, NULL, valueVector());

    /////////////////////////////////////////////////////////////////
    //now we can compare Naive Inf results with Pearl Inf results
    //pearl algorithm
    CPearlInfEngine *myInfP =
	CPearlInfEngine::Create(myModel);
    myInfP->EnterEvidence(myEvid);
    //naive
    CNaiveInfEngine *myInfN =
	CNaiveInfEngine::Create(myModel);
    myInfN->EnterEvidence(myEvid);
    //int NumIters = myInfP->GetNumberOfProvideIterations();
    //ask about marginals for every single node and compare results;

    // ask about query - number of nodes in it
    int querySize = 1;
    int *query = (int *)trsGuardcAlloc(querySize, sizeof(int));
    const CPotential* myQueryJPDP;
    const CPotential* myQueryJPDN;
    CNumericDenseMatrix<float>* myMatrP = NULL;
    CNumericDenseMatrix<float>* myMatrN = NULL;
    int matPSize; int matNSize;
    for( i = 0; i < nnodes; i++ )
    {
	query[0] = i;
	myInfP->MarginalNodes( query, querySize );
	myInfN->MarginalNodes( query, querySize );
	myQueryJPDP = myInfP->GetQueryJPD();
	myQueryJPDN = myInfN->GetQueryJPD();
	myMatrP = static_cast<CNumericDenseMatrix<float>*>(myQueryJPDP->
	    GetMatrix( matTable ));
	myMatrN = static_cast<CNumericDenseMatrix<float>*>(myQueryJPDN->
	    GetMatrix( matTable ));
	const pnl::floatVector *margVectorP = myMatrP->GetVector();
	const pnl::floatVector *margVectorN = myMatrN->GetVector();
	matPSize = margVectorP->size();
	matNSize = margVectorN->size();
	if( matPSize != matNSize )
	{
	    ret = TRS_FAIL;
	    break;
	}
	for( j = 0; j < matPSize; j++ )
	{
	    if( fabs( (*margVectorP)[j] - (*margVectorN)[j] ) >= eps )
	    {
		ret = TRS_FAIL;
		break;
	    }
	}
    }

    //check-up for all possible queries (from the same parameter)
    int numParams = myModel->GetNumberOfFactors();
    CFactor *tempParam = NULL;
    const int *domPar; int domPSize;
    for ( i = 0; i < numParams; i++ )
    {
	tempParam = myModel->GetFactor(i);
	tempParam->GetDomain(& domPSize, &domPar);
	myInfP->MarginalNodes( domPar, domPSize );
	myQueryJPDP = myInfP->GetQueryJPD();
	myInfN->MarginalNodes( domPar, domPSize );
	myQueryJPDN = myInfN->GetQueryJPD();
	myMatrP = static_cast<CNumericDenseMatrix<float>*>(
	    myQueryJPDP->GetMatrix( matTable ));
	myMatrN = static_cast<CNumericDenseMatrix<float>*>(
	    myQueryJPDN->GetMatrix( matTable ));
	const pnl::floatVector *margVectorP = myMatrP->GetVector();
	const pnl::floatVector *margVectorN = myMatrN->GetVector();
	matPSize = margVectorP->size();
	matNSize = margVectorN->size();
	if( matPSize != matNSize )
	{
	    ret = TRS_FAIL;
	    break;
	}
	for( j = 0; j < matPSize; j++ )
	{
	    if( fabs( (*margVectorP)[j] - (*margVectorN)[j] ) >= eps )
	    {
		ret = TRS_FAIL;
		break;
	    }
	}
    }

    //second - start inference with evidence (number of observed nodes - from console)
    int numObsNodes = 0;
    /*while((numObsNodes<1)||(numObsNodes>nnodes))
    {
    trsiRead( &numObsNodes, "4", "number of observed nodes");
    }
    int *ObsNodes = (int *)trsGuardcAlloc(numObsNodes, sizeof(int));
    int *ObsValues = (int *)trsGuardcAlloc(numObsNodes, sizeof(int));
    intVector residuaryNodesFor ;
    for(i=0; i<nnodes; i++)
    {
    residuaryNodesFor.push_back(i);
    }
    for(i=0; i<numObsNodes; i++)
    {
    j = rand()%(nnodes-i);
    ObsNodes[i] = residuaryNodesFor[j];
    residuaryNodesFor.erase(residuaryNodesFor.begin()+j);
    ObsValues[i] = rand()%(myModel->GetNodeType(ObsNodes[i])->GetNodeSize());
    }
    residuaryNodesFor.clear();*/
    //temp!!!
    numObsNodes = 2;
    int ObsNodes[2] = {1, 2};
    valueVector ObsValues;
    ObsValues.assign( 2, (Value)0 );
    ObsValues[0].SetInt(0);
    ObsValues[1].SetInt(0);

    CEvidence *myEvidWithObs = CEvidence::Create(myModel, numObsNodes,
	ObsNodes, ObsValues);
    myInfP->EnterEvidence(myEvidWithObs);
    //	NumIters = myInfP->GetNumberOfProvideIterations();
    myInfN->EnterEvidence(myEvidWithObs);
    for( i = 0; i < nnodes; i++ )
    {
	query[0] = i;
	myInfP->MarginalNodes( query, querySize );
	myInfN->MarginalNodes( query, querySize );
	myQueryJPDP = myInfP->GetQueryJPD();
	myQueryJPDN = myInfN->GetQueryJPD();
	myMatrP = static_cast<CNumericDenseMatrix<float>*>(
	    myQueryJPDP->GetMatrix( matTable ));
	myMatrN = static_cast<CNumericDenseMatrix<float>*>(
	    myQueryJPDN->GetMatrix( matTable ));
	const pnl::floatVector *margVectorP = myMatrP->GetVector();
	const pnl::floatVector *margVectorN = myMatrN->GetVector();
	matPSize = margVectorP->size();
	matNSize = margVectorN->size();
	/*if( matPSize != matNSize )
	{
	ret = TRS_FAIL;
	break;
    }*/
	for( j = 0; j < matPSize; j++ )
	{
	    if( fabs( (*margVectorP)[j] - (*margVectorN)[j] ) >= eps )
	    {
		ret = TRS_FAIL;
		break;
	    }
	}
    }
    for ( i = 0; i < numParams; i++ )
    {
	tempParam = myModel->GetFactor(i);
	tempParam->GetDomain(& domPSize, &domPar);
	myInfP->MarginalNodes( domPar, domPSize );
	myQueryJPDP = myInfP->GetQueryJPD();
	myInfN->MarginalNodes( domPar, domPSize );
	myQueryJPDN = myInfN->GetQueryJPD();
	myMatrP = static_cast<CNumericDenseMatrix<float>*>(
	    myQueryJPDP->GetMatrix( matTable ));
	myMatrN = static_cast<CNumericDenseMatrix<float>*>(
	    myQueryJPDN->GetMatrix( matTable ));
	const pnl::floatVector *margVectorP = myMatrP->GetVector();
	const pnl::floatVector *margVectorN = myMatrN->GetVector();
	matPSize = margVectorP->size();
	matNSize = margVectorN->size();
	if( matPSize != matNSize )
	{
	    ret = TRS_FAIL;
	    break;
	}
	for( j = 0; j < matPSize; j++ )
	{
	    if( fabs( (*margVectorP)[j] - (*margVectorN)[j] ) >= eps )
	    {
		ret = TRS_FAIL;
		break;
	    }
	}
    }
    ////////////////////////////////////////////////////////////
    //check all and free memory

    int *flag = new int[15];
    flag[0] = 0;//trsGuardCheck(ObsNodes);
    flag[1] = 0;//trsGuardCheck(ObsValues);
    flag[2] = trsGuardCheck(query);
    flag[3] = 0;
    for(i=0; i<nnodes; i++)
    {
	if(trsGuardCheck(Datas[i]))
	{
	    flag[3] = trsGuardCheck(Datas[i]);
	    break;
	}
    }
    flag[4] = trsGuardCheck(Datas);
    flag[5] = trsGuardCheck(NodeSizes);
    flag[6] = trsGuardCheck(dataSizes);
    flag[7] = 0;
    for(i=0; i<nnodes; i++)
    {
	if(trsGuardCheck(domains[i]))
	{
	    flag[7] = trsGuardCheck(domains[i]);
	    break;
	}
    }
    flag[8] = trsGuardCheck(domains);
    flag[9] = trsGuardCheck(domainsSizes);
    flag[10] = trsGuardCheck(dag);
    flag[11] = trsGuardCheck(numOfNeighb);
    flag[12] = trsGuardCheck(neighb);
    flag[13] = trsGuardCheck(orient);
    flag[14] = trsGuardCheck(nodeAssociation);
    int fl=0;
    for (i=0; i<15; i++)
    {
	fl = fl+(int)fabs((float)flag[i]);
    }
    delete []flag;
    if(fl)
    {
	return trsResult( TRS_FAIL, "Dirty memory");
    }
    //destroy
    //	delete margJPDwithObserved;
    delete myEvidWithObs;
    //	delete marginalizedJPD;
    //	trsGuardFree(ObsNodes);
    //	trsGuardFree(ObsValues);

    trsGuardFree(query);
    //	delete (marginalAllJPD);
    //CPearlInfEngine::Release(&myInfP);
    delete (myInfP);
    delete (myInfN);
    delete (myEvid);
    for(i=0; i<nnodes; i++)
    {
	trsGuardFree(Datas[i]);
    }
    trsGuardFree(Datas);
    trsGuardFree(NodeSizes);
    trsGuardFree(dataSizes);
    for( i=0; i<nnodes; i++)
    {
	trsGuardFree(domains[i]);
    }
    trsGuardFree(domains);
    trsGuardFree(domainsSizes);
    //	for (i = 0; i < numParams; i++)
    //	{
    //		delete myParams[i];
    //	}
    delete []myParams;
    delete (myModel);
    trsGuardFree(dag);
    trsGuardFree(numOfNeighb);
    trsGuardFree(neighb);
    trsGuardFree(orient);
    neighbors.clear();
    orientation.clear();
    ancestors.clear();
    nonAncestors.clear();
    trsGuardFree(nodeAssociation);
    delete []nodeTypes;
    return trsResult( ret, ret == TRS_OK ? "No errors"
	: "Bad test on PearlInfEngine");
}

void initAPearlInfEngine()
{
    trsReg(func_name, test_desc, test_class, testPearlInfEngine);
}
