/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ANaiveInfEngine.cpp                                         //
//                                                                         //
//  Purpose:   Test on naive inference for BNet model                      //
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

static char func_name[] = "testNaiveInfEngine";

static char* test_desc = "Provide inference, compare with results from Matlab";

static char* test_class = "Algorithm";

int testNaiveInfEngine()
{
    int ret = TRS_OK;
    int i=0, j=0, k=0, t=0;
    int nnodes = -1;
    int numNt = -1;
    //read number of nodes in Graph
    while((nnodes<=1)||(nnodes>15))
    {
	trsiRead( &nnodes, "7", "Number of Nodes in Model");
    }

    float eps = -1.0f;
    while(eps<=0)
    {
	trssRead( &eps, "1e-3f", "accuracy in test");
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
mark:intVecVector ancestors(nnodes);
     intVecVector nonAncestors;
     intVector nonAnc;
     intVector anc;
     for (i=0; i<nnodes; i++)
     {
	 for(j=i+1; j<nnodes; j++)
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
	 numOfChild = rand()%(nonAncestors[i].size());
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
             intVector::iterator point = std::find( nonAncestors[child].begin(), nonAncestors[child].end(),i );
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
                 intVector::iterator pointNew = std::find( nonAncestors[child].begin(), nonAncestors[child].end(),t );
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

#if 0
     theGraph->SortTopologically();

     double *dag = (double *)trsGuardcAlloc(nnodes*nnodes, sizeof(double));
     for(i=0; i<nnodes*nnodes; i++)
     {
	 dag[i] = 0.0;
     }
     int lineNum = 0;//to fill dag - it will be fill columnwise
     const int *nei = NULL;
     const int *ori = NULL;
     int numNeis = 0;
     //we create dag columnwise - like in Matlab
     for(i=0; i<nnodes; i++)
     {
	 theGraph->GetNeighbors(i, &numNeis, &nei, &ori );
	 for(j=0; j<numNeis; j++)
	 {
	     if(ori[j])
	     {
		 lineNum = (nei[j])*nnodes+i;
		 dag[lineNum] = 1.0;
	     }
	 }
     }
#endif

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
	     if((orient[i])[j] == ntParent )
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
	 ((CTabularCPD*)myParams[i])->NormalizeCPD();
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
     CNaiveInfEngine *myInf = CNaiveInfEngine::Create( myModel);
     myInf->EnterEvidence(myEvid);

     //first we marginalize to all nodes:
     int *queryAll = (int *)trsGuardcAlloc(nnodes, sizeof(int));
     for(i=0; i<nnodes; i++)
     {
	 queryAll[i] = i;
     }
     myInf->MarginalNodes(queryAll, nnodes);
     const CPotential *marginalAllJPD = myInf->GetQueryJPD();
     CNumericDenseMatrix<float> *matrAll = static_cast<CNumericDenseMatrix<float>
	 *>(marginalAllJPD->GetMatrix(matTable));
     //	const pnl::floatVector *margAllVector = matrAll->GetVector();
     //	int margAllVectorSize = margAllVector->size();
     //now we can check - how it is working? sum of all data must be 1
     float sum = matrAll->SumAll();
     if( fabs( sum - 1.0 )>eps)
     {
	 ret = TRS_FAIL;
     }

     // ask about query - number of nodes in it
     int querySize = 1;
     //while((querySize<1)||(querySize>nnodes))
     //{
     //	trsiRead( &querySize, "1", "query size for marginalize");
     //}
     int *query = (int *)trsGuardcAlloc(querySize, sizeof(int));
     //intVector residuaryNodes ;
     //for(i=0; i<nnodes; i++)
     //{
     //	residuaryNodes.push_back(i);
     //}
     //for(i=0; i<querySize; i++)
     //{
     //	j = rand()%(nnodes-i);
     //	query[i] = residuaryNodes[j];
     //	residuaryNodes.erase(residuaryNodes.begin()+j);
     //}
     //residuaryNodes.clear();


     //we want to find factor whichhave only self in his domain and put him in query
     for( i = 0; i < nnodes; i++ )
     {
	 if(domainsSizes[i]==1)
	 {
	     query[0] = i;
	     break;
	 }
     }
     if( i==7 )
     {
	 //we have cyclic graph!
	 trsGuardFree(query);
	 delete (marginalAllJPD);
	 trsGuardFree(queryAll);
	 delete (myInf);
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
	 delete (myModel);
	 delete (myParams);
	 trsGuardFree(dag);
	 trsGuardFree(numOfNeighb);
	 trsGuardFree(neighb);
	 trsGuardFree(orient);
	 neighbors.clear();
	 orientation.clear();
	 ancestors.clear();
	 nonAncestors.clear();
	 goto mark;
     }

     myInf->MarginalNodes(query, querySize);
     const CPotential *marginalizedJPD = myInf->GetQueryJPD();
     CNumericDenseMatrix<float> *matr = static_cast<CNumericDenseMatrix<float>*
	 >(marginalizedJPD->GetMatrix(matTable));
     const pnl::floatVector *margVector = matr->GetVector();
     int margVecSize = margVector->size();

     CTabularCPD* tCPD = static_cast<CTabularCPD*>(myModel->GetFactor(query[0]));
     CNumericDenseMatrix<float> *tMatr = static_cast<CNumericDenseMatrix<float>*
	 >(tCPD->GetMatrix(matTable));
     const pnl::floatVector *myVec = tMatr->GetVector();
     //compare these results - with exact result
     for( i=0; i<margVecSize; i++ )
     {
	 if(fabs((*margVector)[i]-(*myVec)[i]) > eps)
	 {
	     ret = TRS_FAIL;
	     break;
	 }
     }
     //delete tCPD;
     //second - start inference with evidence (number of observed nodes - from console)
     int numObsNodes = 0;
     while((numObsNodes<1)||(numObsNodes>nnodes))
     {
	 trsiRead( &numObsNodes, "2", "number of observed nodes");
     }
     int *ObsNodes = (int *)trsGuardcAlloc(numObsNodes, sizeof(int));
     valueVector ObsValues;
     ObsValues.assign( numObsNodes, (Value)0 );
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
	 ObsValues[i].SetInt(rand()%(myModel->GetNodeType(ObsNodes[i])->GetNodeSize()));
     }
     residuaryNodesFor.clear();
     CEvidence *myEvidWithObs = CEvidence::Create(myModel, numObsNodes,
	 ObsNodes, ObsValues);
     myInf->EnterEvidence(myEvidWithObs);
     myInf->MarginalNodes(query, querySize);
#if 0
     const CPotential *margJPDwithObserved = myInf->GetQueryJPD();
     CNumericDenseMatrix<float> *matrWithObs = margJPDwithObserved->GetMatrix(matTable);
     const pnl::floatVector *margVectorWithObs = matrWithObs->GetVector();
     int margWithObsVecSize = margVectorWithObs->size();


     //now we can start MATLab and do the same here

     Engine *ep;
     ep = engOpen(NULL);
     if(!(ep))
     {
	 ret = TRS_FAIL;
	 return trsResult( TRS_FAIL, "Can't start Matlab");
     }
     else
     {
	 //for domain & node sizes
	 mxArray *mxDag = NULL;
	 mxArray *mxNodeSizes = NULL;
	 //create double version of domain and nodeSizes(MATLAB works with double!)
	 mxDag = mxCreateDoubleMatrix(nnodes, nnodes, mxREAL);
	 mxNodeSizes = mxCreateDoubleMatrix(1, nnodes, mxREAL);
	 memcpy( (char*)mxGetPr(mxDag), (char*)dag, nnodes*nnodes*sizeof(double));
	 memcpy( (char*)mxGetPr(mxNodeSizes), (char*)NodeSizes,
	     nnodes*sizeof(double));
	 mxSetName(mxDag, "mxDag");
	 mxSetName(mxNodeSizes, "mxNodeSizes");
	 engPutArray( ep, mxDag );
	 engPutArray( ep, mxNodeSizes );
	 //	engEvalString( ep, "discrete_nodes = 1:N;");
	 //	engEvalString( ep, "bnet = mk_bnet(mxDag, mxNodeSizes, discrete_nodes);" );
	 engEvalString( ep, "bnet = mk_bnet(mxDag, mxNodeSizes);" );
	 //set CPD to all nodes
	 mxArray *mxCPDTable = NULL;
	 for(i=0; i<nnodes; i++)
	 {
	     int dataSize = dataSizes[i];
	     double *doubleTable = new double[dataSize];
	     //we use noramalized data - from CPD
	     CTabularCPD* ttCPD = static_cast<CTabularCPD*>(myModel->GetFactor(i));
	     CNumericDenseMatrix<float> *ttMatr = ttCPD->GetMatrix(matTable);
	     const pnl::floatVector *myVec = ttMatr->GetVector();
	     for ( j=0; j<dataSize; j++)
	     {
		 doubleTable[j] = (double)(*myVec)[j];
	     }
	     mxCPDTable = mxCreateDoubleMatrix(1, dataSize, mxREAL);
	     memcpy( (char*)mxGetPr(mxCPDTable), (char*)doubleTable,
		 dataSize*sizeof(double));
	     delete []doubleTable;
	     mxSetName(mxCPDTable, "mxCPDTable");
	     engPutArray( ep, mxCPDTable);
	     mxArray *mxCounter = NULL;
	     mxCounter = mxCreateDoubleMatrix(1,1,mxREAL);
	     double ii = (double)(i+1);
	     memcpy((char*)mxGetPr(mxCounter), (char*)&ii, 1*sizeof(double));
	     mxSetName(mxCounter, "mxCounter");
	     engPutArray( ep, mxCounter );
	     engEvalString( ep, "fam = family(mxDag, mxCounter);" );
	     engEvalString( ep, "ns = mxNodeSizes(fam);" );
	     engEvalString( ep, "mxCPDTable = reshape(mxCPDTable, ns);" );
	     engEvalString( ep, "num = size(fam,2); order = zeros(1,num);" );
	     engEvalString( ep, "for i=1:num order(i) = num-i+1; end");
	     engEvalString( ep, "mxCPDTable = permute(mxCPDTable, order);");
	     engEvalString( ep, "mxCPDTable = mxCPDTable(:)';");
	     engEvalString( ep, "bnet.CPD{mxCounter}=tabularCPD(bnet,mxCounter,mxCPDTable);");
	     if(i==nnodes-1)
	     {
		 engEvalString( ep, "N = mxCounter;");
	     }
	     mxDestroyArray(mxCounter);
	 }
	 mxDestroyArray(mxCPDTable);
	 //inference without evidence
	 engEvalString( ep, "evidence = cell(1, N);");
	 engEvalString( ep, "engine = global_joint_inf_engine(bnet);");
	 engEvalString( ep, "[engine, loglik] = enter_evidence(engine, evidence);");
	 //fill query here
	 // marginalize to all nodes
	 engEvalString( ep, "queryAll = [1:N];");
	 engEvalString( ep, "mAll = marginal_nodes(engine, queryAll);" );
	 engEvalString( ep, "TAll = mAll.T;");
	 engEvalString( ep, "invAllOrder=zeros(1, N);");
	 engEvalString( ep, "for i=1:N  invAllOreder(i)=N-i+1; end");
	 engEvalString( ep, "TAll = permute(TAll, invAllOrder);");
	 engEvalString( ep, "margAllTable = TAll(:);");
	 double *doubleMargAllTable = new double[margAllVectorSize];
	 mxArray *margAllTable = NULL;
	 margAllTable = mxCreateDoubleMatrix(1, margAllVectorSize, mxREAL);
	 margAllTable = engGetArray( ep, "margAllTable");
	 //to get variable - put it to mxArray
	 memcpy( (char*)doubleMargAllTable, (char*)mxGetPr(margAllTable),
	     margAllVectorSize*sizeof(double));
	 //now we can marginalize to query

	 mxArray *mxQuery = NULL;
	 mxQuery = mxCreateDoubleMatrix(1, querySize, mxREAL);
	 double *doubleQuery = new double[querySize];
	 for(i=0; i<querySize; i++)
	 {
	     doubleQuery[i] = (double)(query[i]+1);
	 }
	 memcpy((char*)mxGetPr(mxQuery), (char*)doubleQuery,
	     querySize*sizeof(double));
	 delete []doubleQuery;
	 mxSetName( mxQuery, "mxQuery");
	 engPutArray( ep, mxQuery);
	 mxArray *margTable = NULL;
	 margTable = mxCreateDoubleMatrix(1,margVecSize ,mxREAL);
	 mxSetName( margTable, "margTable");
	 engPutArray( ep, margTable );
	 engEvalString( ep, "m = marginal_nodes(engine, mxQuery);" );
	 engEvalString( ep, "T = m.T;");
	 engEvalString( ep, "querySize = size(mxQuery, 2); invOrder=zeros(1, querySize);");
	 engEvalString( ep, "for i=1:querySize  invOreder(i)=querySize-i+1; end");
	 engEvalString( ep, "T = permute(T, invOrder);");
	 engEvalString( ep, "margTable = T(:);");
	 double *doubleMargTable = new double[margVecSize];
	 margTable = engGetArray( ep, "margTable");//to get variable - put it to mxArray
	 memcpy( (char*)doubleMargTable, (char*)mxGetPr(margTable),
	     margVecSize*sizeof(double));

	 //add evidence to the BNet
	 engEvalString( ep, "evid = cell(1, N);");
	 //create array of Observed Nodes and Values
	 mxArray *mxObsNodes = NULL;
	 mxArray *mxObsValues = NULL;
	 mxObsNodes = mxCreateDoubleMatrix(1, numObsNodes, mxREAL);
	 mxObsValues = mxCreateDoubleMatrix(1, numObsNodes, mxREAL);
	 double *doubleObsNodes = new double[numObsNodes];
	 double *doubleObsValues = new double[numObsNodes];
	 for(i=0; i<numObsNodes; i++)
	 {
	     doubleObsNodes[i] = (double)ObsNodes[i];
	     doubleObsValues[i] = (double)ObsValues[i];
	 }
	 memcpy((char*)mxGetPr(mxObsNodes), (char*)doubleObsNodes,
	     numObsNodes*sizeof(double));
	 memcpy((char*)mxGetPr(mxObsValues), (char*)doubleObsValues,
	     numObsNodes*sizeof(double));
	 delete []doubleObsNodes;
	 delete []doubleObsValues;
	 mxSetName( mxObsNodes, "mxObsNodes");
	 mxSetName( mxObsValues, "mxObsValues");
	 engPutArray( ep, mxObsNodes );
	 engPutArray( ep, mxObsValues );
	 engEvalString( ep, "numObsNodes = size(mxObsNodes,2);");
	 //create evidence
	 engEvalString( ep, "for i=1:numObsNodes evid{mxObsNodes(i)}=mxObsValues(i); end");
	 engEvalString( ep, "eng = global_joint_inf_engine(bnet);");
	 engEvalString( ep, "eng = enter_evidence(eng, evid)");
	 //marginalize to query
	 mxArray *margTableWithObs;
	 margTableWithObs = mxCreateDoubleMatrix(1, margWithObsVecSize ,mxREAL);
	 mxSetName( margTableWithObs, "margTableWithObs");
	 engPutArray( ep, margTableWithObs );
	 engEvalString( ep, "ma = marginal_nodes(eng, mxQuery);" );
	 engEvalString( ep, "Ta = ma.T;");
	 engEvalString( ep, "querySize = size(mxQuery, 2); invOrder=zeros(1, querySize);");
	 engEvalString( ep, "for i=1:querySize  invOreder(i)=querySize-i+1; end");
	 engEvalString( ep, "Ta = permute(Ta, invOrder);");
	 engEvalString( ep, "margTableWithObs = Ta(:);");
	 double *doubleMargTableWithObs = new double[margWithObsVecSize];
	 margTableWithObs = engGetArray( ep, "margTableWithObs");//to get variable - put it to mxArray
	 memcpy( (char*)doubleMargTableWithObs, (char*)mxGetPr(margTableWithObs),
	     margWithObsVecSize*sizeof(double));
	 mxDestroyArray(mxDag);
	 mxDestroyArray(margTableWithObs);
	 mxDestroyArray(margTable);
	 mxDestroyArray(mxNodeSizes);
	 mxDestroyArray(mxObsNodes);
	 mxDestroyArray(mxObsValues);
	 mxDestroyArray(mxQuery);
	 engEvalString( ep, "clear");
	 int matlab_close_flag = engClose(ep);
	 if(matlab_close_flag)
	 {
	     ret = TRS_FAIL;
	 }
	 //now we can compare results
	 //first - for marginalize without evidence
	 for(i=0; i<margAllVectorSize; i++)
	 {
	     if(fabs((*margAllVector)[i]-(float)(doubleMargAllTable[i])) > eps)
	     {
		 ret = TRS_FAIL;
		 break;
	     }
	 }

	 for(i=0; i<margVecSize; i++)
	 {

	     if(fabs((*margVector)[i]-(float)(doubleMargTable[i])) > eps)
	     {
		 ret = TRS_FAIL;
		 break;
	     }
	 }
	 //second  - for marginalize with evidence
	 for(i=0; i<margWithObsVecSize; i++)
	 {
	     if(fabs((*margVectorWithObs)[i]-(float)(doubleMargTableWithObs[i])) > eps)
	     {
		 ret = TRS_FAIL;
		 break;
	     }
	 }
	 //we create arrays here - need to destroy
	 delete[]doubleMargTableWithObs;
	 delete[]doubleMargTable;
	 delete []doubleMargAllTable;
	 if(matlab_close_flag)
	 {
	     return trsResult( ret, ret == TRS_OK ? "No errors" : "Can't close MATLAB");
	 }

	}//else (if !ep)
#endif

	//check all memory and destroy all arrays
	//check memory
	int *flag = new int[16];
	flag[0] = trsGuardCheck(ObsNodes);
	flag[1] = 0;
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
	flag[15] = trsGuardCheck(queryAll);
	int fl=0;
	for (i=0; i<16; i++)
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
	trsGuardFree(ObsNodes);
	//	trsGuardFree(ObsValues);

	trsGuardFree(query);
	//	delete (marginalAllJPD);
	trsGuardFree(queryAll);
	delete (myInf);
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
	delete (myModel);
	delete (myParams);
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
	return trsResult( ret, ret == TRS_OK ? "No errors" :
	"Bad test on NaiveInfEngine");
}


#if 0
int testNaiveInfEngine()
{
    //BNet Asia from Cowell book, p.20 compare inference results with
    //matlab results and results from Cowell book
    int i=0, j=0;
    float eps = 0.0001f;
    int ret = TRS_OK;
    const int nnodes = 2;

    int numOfNeigh[] = { 1, 1};

    int neigh0[] = { 1 };
    int neigh1[] = { 0 };

    int orient0[] = { 1 };
    int orient1[] = { 0 };

    int *neigh[] = {neigh0,  neigh1 };
    int *orient[] = { orient0, orient1};

    CGraph *theGraph =  pnlCreateGraph( nnodes, numOfNeigh, neigh, orient);

    int numberOfNodeTypes = 1;
    CNodeType *nodeTypes = new CNodeType [1];
    nodeTypes[0] = CNodeType();
    int *nodeAssociation = new int[nnodes];
    for( i=0; i<nnodes; i++ )
    {
	nodeAssociation[i] = 0;
    }
    CBNet *myModel = static_cast<CBNet*>(pnlCreateStaticModel( mtBNet,
	nnodes, numberOfNodeTypes,
	nodeTypes, nodeAssociation,
	theGraph ));

    //fixme - here we should create factors through the model
    //but now we do it without using Factors class

    //we creates every factor - it is CPD
    EFactorType pt = ftCPD;
    EDistributionType dt = dtTabular;
    //number of factors is the same as number of nodes - one CPD per node
    CFactor **myParams = new CFactor*[nnodes];
    int *nodeNumbers = new int [nnodes];
    int domain0[] = { 0 };
    int domain1[] = { 0, 1 };

    int *domains[] = { domain0, domain1 };
    nodeNumbers[0] = 1;
    nodeNumbers[1] = 2;

    myModel->AllocFactors();
    for( i=0; i<nnodes; i++ )
    {
	const CNodeType **nt = new const CNodeType*[nodeNumbers[i]];
	for( j=0; j<nodeNumbers[i]; j++)
	{
	    nt[j] = myModel->GetNodeType((domains[i])[j]);
	}
	myParams[i] = CTabularCPD::Create( nt, domains[i],
	    nodeNumbers[i]);
	delete []nt;
    }

    //now we can alloc matrix to every parameter
    //creates data
    float data0[] = {0.99f, 0.01f};
    float data1[] = {0.5f, 0.5f, 0.5f, 0.5f};

    float *data[] = {data0, data1};
    //alloc matrices
    for (i=0; i<nnodes; i++)
    {
	myParams[i]->AllocMatrix(data[i], matTable);
	myModel->AttachFactor(myParams[i]);
    }

    //now we provide inference without evidence  - to get common JPD
    CEvidence *myEvid = CEvidence::Create(myModel, 0, NULL, NULL);
    //we can create inference with evidence - and do the same at Matlab
    //int NObsNodes = 2;
    //int *obsNodes = new int [2];
    //obsNodes[0] = 0;
    //obsNodes[1] = 7;
    //int values [2] = {0,1};
    //CEvidence *myEv = CEvidence::Create(myModel, NObsNodes,obsNodes,values);
    CNaiveInfEngine *myInf =
	pnlNaiveInfEngine::Create(myModel);
    myInf->EnterEvidence(myEvid);
    int querySize = 1;
    int *query  = new int[querySize];
    query[0] = 0;
    CPotential *myJPD= myInf->MarginalNodes(query, querySize);
    CNumericDenseMatrix<float> *myMatJPD = myJPD->GetMatrix(matTable);
    const pnl::floatVector *myMatrixVector = myMatJPD->GetVector();
    for(i=0; i<2; i++)
    {
	if(fabs(data0[i]-(*myMatrixVector)[i]) > eps)
	{
	    ret = TRS_FAIL;
	    break;
	}
    }
    //delete all
    delete []nodeTypes;
    delete []nodeAssociation;
    delete []myParams;
    delete []nodeNumbers;
    delete myModel;
    delete myEvid;
    delete myInf;
    delete myJPD;

    return trsResult( ret, ret == TRS_OK ? "No errors" :
    "Bad test on NaiveInfEngine");
}


//the previous version of the test
int testNaiveInfEngine()
{
    //BNet Asia from Cowell book, p.20 compare inference results with
    //matlab results and results from Cowell book
    int i=0, j=0;
    float eps = 0.0001f;
    int ret = TRS_OK;
    const int nnodes = 8;

    int numOfNeigh[] = { 2, 1, 2, 2, 2, 4, 2, 1};

    int neigh0[] = { 2, 3 };
    int neigh1[] = { 4 };
    int neigh2[] = { 0, 6 };
    int neigh3[] = { 0, 5 };
    int neigh4[] = { 1, 5};
    int neigh5[] = { 3, 4, 6, 7};
    int neigh6[] = { 2, 5 };
    int neigh7[] = { 5 };

    int orient0[] = { 1, 1 };
    int orient1[] = { 1 };
    int orient2[] = { 0, 1 };
    int orient3[] = { 0, 1 };
    int orient4[] = { 0, 1 };
    int orient5[] = { 0, 0, 1, 1 };
    int orient6[] = { 0, 0 };
    int orient7[] = { 0 };

    int *neigh[] = {neigh0,  neigh1, neigh2, neigh3, neigh4, neigh5,
	neigh6, neigh7 };
    int *orient[] = { orient0, orient1, orient2, orient3, orient4,
	orient5, orient6, orient7 };

    CGraph *theGraph =  pnlCreateGraph( nnodes, numOfNeigh, neigh, orient);

    int numberOfNodeTypes = 1;
    CNodeType *nodeTypes = new CNodeType [1];
    nodeTypes[0] = CNodeType();
    int *nodeAssociation = new int[nnodes];
    for( i=0; i<nnodes; i++ )
    {
	nodeAssociation[i] = 0;
    }
    CBNet *myModel = static_cast<CBNet*>(pnlCreateStaticModel( mtBNet,
	nnodes, numberOfNodeTypes,
	nodeTypes, nodeAssociation,
	theGraph ));

    //fixme - here we should create factors through the model
    //but now we do it without using Factors class

    //we creates every factor - it is CPD
    EFactorType pt = ftCPD;
    EDistributionType dt = dtTabular;
    //number of factors is the same as number of nodes - one CPD per node
    CFactor **myParams = new CFactor*[nnodes];
    int *nodeNumbers = new int [nnodes];
    int domain0[] = { 0 };
    int domain1[] = { 1 };
    int domain2[] = { 0, 2 };
    int domain3[] = { 0, 3 };
    int domain4[] = { 1, 4 };
    int domain5[] = { 3, 4, 5 };
    int domain6[] = { 2, 5, 6 };
    int domain7[] = { 5, 7 };
    int *domains[] = { domain0, domain1, domain2, domain3, domain4,
	domain5, domain6, domain7 };
    nodeNumbers[0] = 1;
    nodeNumbers[1] = 1;
    nodeNumbers[2] = 2;
    nodeNumbers[3] = 2;
    nodeNumbers[4] = 2;
    nodeNumbers[5] = 3;
    nodeNumbers[6] = 3;
    nodeNumbers[7] = 2;

    myModel->AllocFactors();
    for( i=0; i<nnodes; i++ )
    {
	const CNodeType **nt = new const CNodeType*[nodeNumbers[i]];
	for( j=0; j<nodeNumbers[i]; j++)
	{
	    nt[j] = myModel->GetNodeType((domains[i])[j]);
	}
	myParams[i] = CTabularCPD::Create( nt, domains[i],
	    nodeNumbers[i]);
	delete []nt;
    }

    //now we can alloc matrix to every parameter
    //creates data
    float data0[] = {0.5f, 0.5f};
    float data1[] = {0.99f, 0.01f};
    float data2[] = {0.7f, 0.3f, 0.4f, 0.6f};
    float data3[] = {0.99f, 0.01f, 0.9f, 0.1f};
    float data4[] = {0.99f, 0.01f, 0.95f, 0.05f};
    float data5[] = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f};
    float data6[] = {0.9f, 0.1f, 0.3f, 0.7f, 0.2f, 0.8f, 0.1f, 0.9f};
    float data7[] = {0.95f, 0.05f, 0.02f, 0.98f};
    float *data[] = {data0, data1, data2, data3, data4, data5, data6, data7};
    //alloc matrices
    for (i=0; i<nnodes; i++)
    {
	myParams[i]->AllocMatrix(data[i], matTable);
	myModel->AttachFactor(myParams[i]);
    }

    //now we provide inference without evidence  - to get common JPD
    CEvidence *myEvid = CEvidence::Create(myModel, 0, NULL, NULL);
    //we can create inference with evidence - and do the same at Matlab
    //int NObsNodes = 2;
    //int *obsNodes = new int [2];
    //obsNodes[0] = 0;
    //obsNodes[1] = 7;
    //int values [2] = {0,1};
    //CEvidence *myEv = CEvidence::Create(myModel, NObsNodes,obsNodes,values);
    CNaiveInfEngine *myInf =
	pnlNaiveInfEngine::Create( myModel);
    myInf->EnterEvidence(myEvid);
    int *query = new int[8];
    for(i=0; i<nnodes; i++)
    {
	query[i] = i;
    }
    CPotential *myJPD = myInf->MarginalNodes(query, 8);
    delete [] query;

    //compare results with MATLAB version:
    //we know all results and can compare. Here we have all results in C form
    CNumericDenseMatrix<float> *myMatJPD = myJPD->GetMatrix(matTable);
    const pnl::floatVector *myMatrixVector = myMatJPD->GetVector();
    //all JPD must have matrix of one element 1.0 with probable error eps
    if((myMatrixVector->size())!=256)//all node sizes of 8 nodes are 2
    {
	ret = TRS_FAIL;
	return trsResult( ret, ret == TRS_OK ? "No errors" :
	"Bad test on all JPD at NaiveInfEbngine");
    }
    else
    {
	//if we want to compare with MATLAb, we can call:
	//	N = 8;
	//	dag = zeros(N,N);
	//	dag(1,3)=1; dag(1,4)=1; dag(2,5)=1;
	//	dag(3,7)=1; dag(4,6)=1; dag(5,6)=1; dag(6,7)=1; dag(6,8)=1
	//	discrete_nodes = 1:N;
	//	node_sizes = 2*ones(1,N);
	//	bnet = mk_bnet(dag, node_sizes, discrete_nodes);
	//	bnet.CPD{1} = tabularCPD(bnet, 1, [0.5 0.5]);
	//	bnet.CPD{2} = tabularCPD(bnet, 2, [0.99 0.01]);
	//	bnet.CPD{3} = tabularCPD(bnet, 3, [0.7 0.4 0.3 0.6]);
	//	bnet.CPD{4} = tabularCPD(bnet, 4, [0.99 0.9 0.01 0.1]);
	//	bnet.CPD{5} = tabularCPD(bnet, 5, [0.99 0.95 0.01 0.05]);
	//	bnet.CPD{6} = tabularCPD(bnet, 6, [1 0 0 0 0 1 1 1]);
	//	bnet.CPD{7} = tabularCPD(bnet, 7, [0.9 0.2 0.3 0.1 0.1 0.8 0.7 0.9]);
	//	bnet.CPD{8} = tabularCPD(bnet, 8, [0.95 0.02 0.05 0.98]);
	//	evidence = cell(1,8);
	//	engine = global_joint_inf_engine(bnet);
	//	[engine, loglik] = enter_evidence(engine, evidence)
	//	[m, pot] = marginal_nodes(engine, [1 2 3 4 5 6 7 8])
	//	and compare this results with ours.
	//	So we can call:
	Engine *ep;
	mxArray *N = NULL, *Table = NULL;
	ep = engOpen(NULL);
	if(!(ep))
	{
	    assert(0);//can't start MATLAB engine
	}
	else
	{
	    double Nn[1] = {8};//sourse - from it we copy
	    N = mxCreateDoubleMatrix( 1, 1, mxREAL );
	    //dag = mxCreateDoubleMatrix(8,8, mxREAL);
	    mxSetName(N, "N");
	    Table = mxCreateDoubleMatrix(1,256, mxREAL);
	    mxSetName(Table, "Table");
	    double tTable[] = {0, 0.1, 0.2} ;
	    memcpy( (char*)mxGetPr(Table), (char*)tTable, 3*sizeof(double) );
	    memcpy( (char*)mxGetPr(N), (char*)Nn, 1*sizeof(double) );
	    engPutArray( ep, N );
	    engPutArray( ep, Table);
	    engEvalString( ep, "dag = zeros(N,N);dag(1,3)=1;dag(1,4)=1;dag(2,5)=1;");
	    engEvalString( ep, "dag(3,7)=1;dag(4,6)=1;dag(5,6)=1;dag(6,7)=1;dag(6,8)=1");
	    engEvalString( ep, "discrete_nodes = 1:N; node_sizes = 2*ones(1,N);");
	    //this version is used in BNT
	    engEvalString( ep, "bnet = mk_bnet(dag, node_sizes, discrete_nodes);");
	    engEvalString( ep, "bnet.CPD{1} = tabularCPD(bnet, 1, [0.5 0.5]);");
	    engEvalString( ep, "bnet.CPD{2} = tabularCPD(bnet, 2, [0.99 0.01]);");
	    engEvalString( ep, "bnet.CPD{3} = tabularCPD(bnet, 3, [0.7 0.4 0.3 0.6]);");
	    engEvalString( ep, "bnet.CPD{4} = tabularCPD(bnet, 4, [0.99 0.9 0.01 0.1]);");
	    engEvalString( ep, "bnet.CPD{5} = tabularCPD(bnet, 5, [0.99 0.95 0.01 0.05]);");
	    engEvalString( ep, "bnet.CPD{6} = tabularCPD(bnet, 6, [1 0 0 0 0 1 1 1]);");
	    engEvalString( ep, "bnet.CPD{7} = tabularCPD(bnet, 7, [0.9 0.2 0.3 0.1 0.1 0.8 0.7 0.9]);");
	    engEvalString( ep, "bnet.CPD{8} = tabularCPD(bnet, 8, [0.95 0.02 0.05 0.98]);");
	    //this version is used in TK
	    //engEvalString( ep, "CPDs = cell(1,N);")
	    //...
	    engEvalString( ep, "evidence = cell(1,8);");
	    //this version is used in BNT
	    engEvalString( ep, "engine = global_joint_inf_engine(bnet);");
	    engEvalString( ep, "[engine, loglik] = enter_evidence(engine, evidence)");
	    engEvalString( ep, "[m, pot] = marginal_nodes(engine, [1 2 3 4 5 6 7 8])");
	    engEvalString( ep, "Table = m.T;");
	    engEvalString( ep, "Table = permute(Table, [8 7 6 5 4 3 2 1]);");
	    engEvalString( ep, "Table = Table(:);");
	    double *doubleDataFromMatlab = new double[256];
	    Table = engGetArray( ep, "Table");//to get variable - put it to mxArray
	    memcpy( (char*)doubleDataFromMatlab, (char*)mxGetPr(Table), 256*sizeof(double));
	    float *dataFromMatlab = (float*)trsGuardcAlloc( 256, sizeof(float) );
	    for(i=0; i<256; i++)
	    {
		dataFromMatlab[i] = (float)doubleDataFromMatlab[i];
	    }
	    delete []doubleDataFromMatlab;
	    mxDestroyArray(N);
	    mxDestroyArray(Table);
	    engEvalString( ep, "clear");
	    engClose(ep);
	    for(i=0; i<256; i++)
	    {
		if(fabs(dataFromMatlab[i]-(*myMatrixVector)[i]) > eps)
		{
		    ret = TRS_FAIL;
		    break;
		}
	    }
	    //to compare Matlab resuts with our results we need to convert Matlab Matrix to our Matrix
	    //put results from MATLab and compare in cycle
	    float res7[] = { 0.8897f, 0.1103f };
	    float res6[] = { 0.5640f, 0.4360f };
	    float res5[] = { 0.9352f, 0.0648f };
	    float res4[] = { 0.9896f, 0.0104f };
	    float res3[] = { 0.9450f, 0.0550f };
	    float res2[] = { 0.55f, 0.45f };
	    float res1[] = { 0.99f, 0.01f };
	    float res0[] = { 0.5f, 0.5f };
	    float *MATLabRes[] = { res0, res1, res2, res3, res4, res5, res6,
		res7 };
	    for( i=0; i<nnodes; i++ )
	    {
		//all JPD have size 2 here
		int query[] = { i };
		int querySize = 1;
		myJPD = myInf->MarginalNodes(query, querySize);
		myMatJPD = myJPD->GetMatrix(matTable);
		const pnl::floatVector *myQueryMatrixVector =
		    myMatJPD->GetVector();
		if(myQueryMatrixVector->size()!=2)
		{
		    ret = TRS_FAIL;
		    break;
		}
		else
		{
		    if( fabs
			(((*myQueryMatrixVector)[0])-(MATLabRes[i])[0])>eps )
		    {
			ret = TRS_FAIL;
			break;
		    }
		    else
		    {
			if( fabs(
			    ((*myQueryMatrixVector)[1])-(MATLabRes[i])[1])>eps )
			{
			    ret = TRS_FAIL;
			    break;
			}
		    }
		}
	    }
	}
    }

    //delete all
    delete []nodeTypes;
    delete []nodeAssociation;
    delete []myParams;
    delete []nodeNumbers;
    delete myModel;
    delete myEvid;
    delete myInf;
    delete myJPD;

    return trsResult( ret, ret == TRS_OK ? "No errors" :
    "Bad test on NaiveInfEngine");
}

#endif

void initANaiveInfEngine()
{
    trsReg(func_name, test_desc, test_class, testNaiveInfEngine);
}