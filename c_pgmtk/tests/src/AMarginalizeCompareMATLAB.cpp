/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMarginalizeCompareMATLAB.cpp                               //
//                                                                         //
//  Purpose:   Test on marginalization of potentails                       //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdio.h>
#include "trsapi.h"
#include "pnl_dll.hpp"
#include <string.h>

// this file must be last included file
#include "tUtil.h"

PNL_USING

static char func_name[] = "testMarginalizeCompareMATLAB";

static char* test_desc = "Marginalize table in C and MatLab, compare results";

static char* test_class = "Algorithm";


int testMarginalizeCompareMATLAB()
{
    int ret = TRS_OK;
	
	float eps = -1.0f;
	while(eps<=0)
	{
		trssRead( &eps, "1e-3f", "accuracy in test");
	}
	
	int i=0, j=0, k=0, t=0;
	int nnodes = -1;
	int numnt = -1;
	/*read number of nodes in Factor domain*/
	while((nnodes<=1)||(nnodes>30))//only if nnodes<=30
	{
		trsiRead( &nnodes, "8", "Number of Nodes in domain");
	}
	/*read number of node types in model*/
	while((numnt<=0)||(numnt>nnodes))
	{
		trsiRead( &numnt, "1", "Number of node types in Domain");
	}
	
	int seed1 = pnlTestRandSeed();
	/*create string to display the value*/
	char *value = new char[20];
#if 0
        value = _itoa( seed1, value, 10 );
#else
        sprintf( value, "%d", seed1 );
#endif
	trsiRead(&seed1, value, "Seed for srand to define NodeTypes etc.");
	delete []value;
	trsWrite(TW_CON|TW_RUN|TW_DEBUG|TW_LST, "seed for rand = %d\n", seed1);
	
	nodeTypeVector nodeTypes;
    nodeTypes.assign( numnt, CNodeType() );
	
	for(i=0; i<numnt; i++)
	{
		nodeTypes[i] = CNodeType(1,i+2);
	}
	intVector nodeAssociation ;
    nodeAssociation.assign(nnodes);
	int *domain = (int*)trsGuardcAlloc( nnodes, sizeof(int) );
	
	for(i = 0; i<nnodes; i++ ) 
	{
		nodeAssociation[i] = rand()%(numnt);
		domain[i] = i;
	}
	
	int dataSize = 1;
	int nodeSize = 0;

    CModelDomain* pMD = CModelDomain::Create( nodeTypes, nodeAssociation );
	
	const CNodeType** ntForParam = (const CNodeType**)
		trsGuardcAlloc(nnodes, sizeof(CNodeType*));
	for(i=0; i<nnodes; i++)
	{
		nodeSize = nodeTypes[nodeAssociation[i]].GetNodeSize();
		dataSize = dataSize*nodeSize;
		ntForParam[i] = &nodeTypes[nodeAssociation[i]];
	}
	float *data = (float *)trsGuardcAlloc(dataSize, sizeof(float)) ;
	for( i=0; i<dataSize; i++)
	{
		data[i] = (float)rand()/10000;
	}
	
	CFactor *pParam = CTabularPotential::Create(domain, nnodes, pMD);
	//fixme - we need to normalize our matrix for CPD
	
	pParam->AllocMatrix(data, matTable);
	//first we marginalize all in C
	
	//marginalize to one node
	//we need to crate array of marginalized factors and destroy it afterall
	CFactor **paramMargToNode = (CFactor**)trsGuardcAlloc(nnodes,
											sizeof(CFactor*));
	const CNumericDenseMatrix<float> **margToNode = (const CNumericDenseMatrix<float>**)
		trsGuardcAlloc(nnodes, sizeof(const CNumericDenseMatrix<float>*));
	int domSize=1;
	int *pSmallDom = (int*)trsGuardcAlloc( domSize, sizeof(int) );
	for(i=0; i<nnodes; i++)
	{
		pSmallDom[0] = i;
		paramMargToNode[i]=static_cast<CPotential*>(pParam)->Marginalize(
                    pSmallDom, domSize, 0);
		margToNode[i] = static_cast<CNumericDenseMatrix<float>*>(
            paramMargToNode[i]->GetMatrix(matTable));
	}
	int pSmallDom_b = trsGuardCheck(pSmallDom);
	if(pSmallDom_b)
	{
		ret = TRS_FAIL;
		return trsResult( TRS_FAIL, "Dirty memory");
	}
	else
	{
		trsGuardFree(pSmallDom);
	}
	
	//marginalize to pair of nodes - only if nnodes<=12 (can't compute nPairs )
	//alloc memory
	int nPairs = nnodes*(nnodes-1)/2;
	
	CFactor** paramMargToPair = (CFactor**)trsGuardcAlloc(nPairs,
										sizeof(CFactor*));
	const CNumericDenseMatrix<float> **margToPair = (const CNumericDenseMatrix<float>**)
		trsGuardcAlloc(nPairs, sizeof(const CNumericDenseMatrix<float>*));
	int nThree = nnodes*(nnodes-1)*(nnodes-2)/6;
	CFactor** paramMargToThree = (CFactor**)trsGuardcAlloc(nThree,
										sizeof(CFactor*));
	const CNumericDenseMatrix<float> **margToThree = (const CNumericDenseMatrix<float>**)
		trsGuardcAlloc(nThree, sizeof(const CNumericDenseMatrix<float>*));
	if(nnodes>2)
	{
		domSize = 2;
		t=0;
		int *pSmallDom = (int*)trsGuardcAlloc( domSize, 2*sizeof(int) );
		int pSmallDom_b = trsGuardCheck(pSmallDom);
		for( i=0; i<nnodes; i++)
			for( j=i+1; j<nnodes; j++)
			{
				pSmallDom[0] = i;
				pSmallDom[1] = j;
				pSmallDom_b = trsGuardCheck(pSmallDom);
				paramMargToPair[t]=static_cast<CPotential*>(
                            pParam)->Marginalize(pSmallDom, domSize, 0);
				margToPair[t] = static_cast<CNumericDenseMatrix<float>*>(
                    paramMargToPair[t]->GetMatrix(matTable));
				t++;
			}
			/*int*/ pSmallDom_b = trsGuardCheck(pSmallDom);
			if(pSmallDom_b)
			{
				ret = TRS_FAIL;
				return trsResult( TRS_FAIL, "Dirty memory");
			}
			else
			{
				trsGuardFree(pSmallDom);
			}
			//marginalize to three nodes
			if(nnodes>3)
			{
				domSize = 3;
				t=0;
				int *pSmallDom = (int*)trsGuardcAlloc( domSize, 3*sizeof(int) );
				for( i=0; i<nnodes; i++)
					{
					for( j=i+1; j<nnodes; j++)
						{
						for(k=j+1; k<nnodes; k++)
							{
								pSmallDom[0] = i;
								pSmallDom[1] = j;
								pSmallDom[2] = k;
								paramMargToThree[t]=
                                    static_cast<CPotential*>(
                                    pParam)->Marginalize(pSmallDom,
									domSize, 0);
								margToThree[t] = static_cast<
                                    CNumericDenseMatrix<float>*>(
                                    paramMargToThree[t]->GetMatrix(matTable));
								t++;
							}
						}
					}
					int pSmallDom_b = trsGuardCheck(pSmallDom);
					if(pSmallDom_b)
					{
						ret = TRS_FAIL;
						return trsResult( TRS_FAIL, "Dirty memory");
					}
					else
					{
						trsGuardFree(pSmallDom);
					}
			}//nnodes>3
	}//nnodes>2
	//we marginalize only to one, two and three nodes
	
	//second  - we marginalize all in Matlab
	
	//first we need to create strings (or put data straight to it)
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
		mxArray *mxDomain = NULL;
		mxArray *mxNodeSizes = NULL;
		//create double version of domain and nodeSizes(MATLAB works with double!)
		double *dom = new double [nnodes];
		double *nodeSizes = new double[nnodes];
		for (i=0; i<nnodes; i++)
		{
			dom[i] = (double)(domain[i]+1);
			nodeSizes[i] = (double)nodeTypes[nodeAssociation[i]].GetNodeSize();
		}
		mxDomain = mxCreateDoubleMatrix(1, nnodes, mxREAL);
		mxNodeSizes = mxCreateDoubleMatrix(1, nnodes, mxREAL);
		memcpy( (char*)mxGetPr(mxDomain), (char*)dom, nnodes*sizeof(double));
		memcpy( (char*)mxGetPr(mxNodeSizes), (char*)nodeSizes,
			nnodes*sizeof(double));
		delete []dom;
		mxSetName(mxDomain, "mxDomain");
		mxSetName(mxNodeSizes, "mxNodeSizes");
		engPutArray( ep, mxDomain );
		engPutArray( ep, mxNodeSizes );
		//create data 
		mxArray *mxData = NULL;
		double *doubleData = new double[dataSize];
		for ( i=0; i<dataSize; i++)
		{
			doubleData[i] = (double)data[i];
		}
		mxData = mxCreateDoubleMatrix(1, dataSize, mxREAL);
		memcpy( (char*)mxGetPr(mxData), (char*)doubleData,
			dataSize*sizeof(double));
		delete []doubleData;
		mxSetName(mxData, "mxData");
		//Matlab has columnwise matrix - we need to reshape and permute it
		//we can do it by reverse order in permute
		engPutArray( ep, mxData );
		mxArray *mxPermute = NULL;
		mxPermute = mxCreateDoubleMatrix(1,nnodes, mxREAL);
		double *permuteOrder = new double [nnodes];
		for(i=0; i<nnodes; i++)
		{
			permuteOrder[i] = (double)(nnodes-domain[i]);
		}
		memcpy((char*)mxGetPr(mxPermute), (char*)permuteOrder,
			nnodes*sizeof(double));
		delete []permuteOrder;
		mxSetName(mxPermute, "mxPermute");
		engPutArray( ep, mxPermute);
        engEvalString( ep, "format long" );
		engEvalString( ep, "data = reshape(mxData, mxNodeSizes);" );
		engEvalString( ep, "data = permute(data,mxPermute )");
		engEvalString( ep, "bigpot = dpot(mxDomain, mxNodeSizes, data);");
		//marginalize to one node - for every of them
		//create array to carry data from matlab
		double ** margToOneFrom = new double*[nnodes];
		for( i=0; i<nnodes; i++)
		{
			margToOneFrom[i] = new double[(int)nodeSizes[i]];
		}
		for(i=0; i<nnodes; i++)
		{
			mxArray *Table = NULL;
			double *pSmallDom = new double[1];
			pSmallDom[0] = (double)(i+1);
			mxArray *dom = NULL;
			dom = mxCreateDoubleMatrix( 1,1,mxREAL );
			Table = mxCreateDoubleMatrix(2,1,mxREAL);
			memcpy((char*)mxGetPr(dom),(char*)pSmallDom, sizeof(double) );
			delete []pSmallDom;
			mxSetName(dom, "dom");
			engPutArray( ep, dom );
			engEvalString( ep, "smallpot = marginalize_pot(bigpot, dom);" );
			engEvalString( ep, "marg = pot_to_marginal(smallpot);" );
			engEvalString( ep, "Tab = marg.T;" );
			engEvalString (ep, "Tab = Tab(:);");//we don't need to permute only for one node
			Table = engGetArray( ep, "Tab" );//to get variable - put it to mxArray
			/*I need to get array size - it can be done
			by mxGetN(returns number of columns), mxGetM(number of rows)*/
			int nRows = mxGetM(Table);
			if(nRows!=(int)nodeSizes[i])
			{
				ret = TRS_FAIL;	
				//we marginalize to one node - node sizes should be the same
			}
			else
			{
				memcpy((char*)margToOneFrom[i], (char*)mxGetPr(Table),
					nRows*sizeof(double) );
			}
			engEvalString(ep, "Table = 0;");
			mxDestroyArray(dom);
			mxDestroyArray(Table);
		}
		//to compare three nodes
		double **margToPairFrom = new double*[nPairs];
		int *pairDataSizes = new int[nPairs];
		int pairDataSize = 1;
		t=0;
		for(i=0; i<nnodes; i++)
		{
			for(j=i+1; j<nnodes; j++)
			{
				pairDataSize = pairDataSize*(int)nodeSizes[i];
				pairDataSize = pairDataSize*(int)nodeSizes[j];
				pairDataSizes[t] = pairDataSize;
				margToPairFrom[t] = new double[pairDataSize];
				pairDataSize=1;
				t++;
			}
		}
		//to compare for three
		double **margToThreeFrom = new double*[nThree];
		int *threeDataSizes = new int[nThree];
		int threeDataSize = 1;
		t=0;
		for(i=0; i<nnodes; i++)
			for(j=i+1; j<nnodes; j++)
				for(k=j+1;k<nnodes; k++ )
				{
					threeDataSize = threeDataSize*(int)nodeSizes[i];
					threeDataSize = threeDataSize*(int)nodeSizes[j];
					threeDataSize = threeDataSize*(int)nodeSizes[k];
					threeDataSizes[t] = threeDataSize;
					margToThreeFrom[t] = new double[threeDataSize];
					threeDataSize=1;
					t++;
				}
				if(nnodes>2)
				{
					t=0;
					for(i=0; i<nnodes; i++)
					{
						for(j=i+1; j<nnodes; j++)
						{
							mxArray *Table = NULL;
							double *pSmallDom = new double[2];
							pSmallDom[0] = (double)(i+1);
							pSmallDom[1] = (double)(j+1);
							mxArray *dom = NULL;
							dom = mxCreateDoubleMatrix( 2,1,mxREAL );
							Table = mxCreateDoubleMatrix(pairDataSizes[t],1,mxREAL);
							memcpy((char*)mxGetPr(dom),(char*)pSmallDom, 2*sizeof(double) );
							delete []pSmallDom;
							mxSetName(dom, "dom");
							engPutArray( ep, dom );
							engEvalString( ep, "smallpot = marginalize_pot(bigpot, dom);" );
							engEvalString( ep, "marg = pot_to_marginal(smallpot);" );
							engEvalString( ep, "Tab = marg.T;" );
							//we need to permute matrix rowwise - as in C
							engEvalString( ep, "Tab = permute(Tab, [2 1]);");
							engEvalString (ep, "Tab = Tab(:);");
							Table = engGetArray( ep, "Tab" );//to get variable - put it to mxArray
															 /*I need to get array size - it can be done
							by mxGetN(returns number of columns), mxGetM(number of rows)*/
							int nRows = mxGetM(Table);
							if(nRows!=pairDataSizes[t])
							{
								ret = TRS_FAIL;//we marginalize to pair of nodes
							}
							else
							{
								memcpy((char*)margToPairFrom[t], (char*)mxGetPr(Table),
									nRows*sizeof(double) );
							}
							engEvalString(ep, "Table = 0;");
							mxDestroyArray(dom);
							mxDestroyArray(Table);
							t++;
						}
					}
					if(nnodes>3)
					{
						t=0;
						for(i=0; i<nnodes; i++)
						{
							for(j=i+1; j<nnodes; j++)
							{
								for(k=j+1; k<nnodes; k++)
									{
										mxArray *Table = NULL;
										double *pSmallDom = new double[3];
										pSmallDom[0] = (double)(i+1);
										pSmallDom[1] = (double)(j+1);
										pSmallDom[2] = (double)(k+1);
										mxArray *dom = NULL;
										dom = mxCreateDoubleMatrix( 3,1,mxREAL );
										Table = mxCreateDoubleMatrix(threeDataSizes[t],1,mxREAL);
										memcpy((char*)mxGetPr(dom),(char*)pSmallDom, 3*sizeof(double) );
										delete []pSmallDom;
										mxSetName(dom, "dom");
										engPutArray( ep, dom );
										engEvalString( ep, "smallpot = marginalize_pot(bigpot, dom);" );
										engEvalString( ep, "marg = pot_to_marginal(smallpot);" );
										engEvalString( ep, "Tab = marg.T;" );
										//we need to permute matrix rowwise - as in C
										engEvalString( ep, "Tab = permute(Tab, [3 2 1]);");
										engEvalString (ep, "Tab = Tab(:);");
										Table = engGetArray( ep, "Tab" );
										//to get variable - put it to mxArray
										/*I need to get array size - it can be done
										by mxGetN(returns number of columns), mxGetM(number of rows)*/
										int nRows = mxGetM(Table);
										if(nRows!=threeDataSizes[t])
										{
											ret = TRS_FAIL;//we marginalize to pair of nodes
										}
										else
										{
											memcpy((char*)margToThreeFrom[t],
												(char*)mxGetPr(Table),
												nRows*sizeof(double) );
										}
										engEvalString(ep, "Table = 0;");
										mxDestroyArray(dom);
										mxDestroyArray(Table);
										t++;
									}
								}
							}
						}//if(nnodes>3)
				}//if (nnodes>2)
				mxDestroyArray(mxDomain);
				mxDestroyArray(mxData);
				mxDestroyArray(mxNodeSizes);
				mxDestroyArray(mxPermute);
				engEvalString( ep, "clear");
				int matlab_close_flag = engClose(ep);
				if(matlab_close_flag)
				{
					ret = TRS_FAIL;
				}
				//third - we compare C and Matlab results
				//we will compare margToOneFrom&MargToNode
				int dataSize = 0;
				for( i=0; i<nnodes; i++)
				{
					const floatVector *myQueryMatrixVector = 
						(margToNode[i])->GetVector();
					dataSize = myQueryMatrixVector->size();
					if(dataSize!=(int)nodeSizes[i])
					{
						ret = TRS_FAIL;
						break;
					}
					else
					{
						for(j=0; j<dataSize; j++)
						{
							if( fabs(((*myQueryMatrixVector)[j])-
								(float)((margToOneFrom[i])[j]))>eps )
							{
								ret = TRS_FAIL;
								break;
							}
						}
					}
				}
				if(nnodes>2)
				{
					//we compare MargToPairFrom & margToPair
					for( i=0; i<nPairs; i++)
					{
						const floatVector *myQueryMatrixVector = 
							(margToPair[i])->GetVector();
						int dataSize = myQueryMatrixVector->size();
						if(dataSize!=pairDataSizes[i])
						{
							ret = TRS_FAIL;
							break;
						}
						else
						{
							for(j=0; j<dataSize; j++)
							{
								if( fabs(((*myQueryMatrixVector)[j])-
									(float)(margToPairFrom[i])[j])>eps )
								{
									ret = TRS_FAIL;
									break;
								}
							}
						}
					}
					if(nnodes>3)
					{
						//we compare MargToPairFrom & margToPair
						for( i=0; i<nThree; i++)
						{
							const floatVector *myQueryMatrixVector = 
								(margToThree[i])->GetVector();
							int dataSize = myQueryMatrixVector->size();
							if(dataSize!=threeDataSizes[i])
							{
								ret = TRS_FAIL;
								break;
							}
							else
							{
								for(j=0; j<dataSize; j++)
								{
									if( fabs(((*myQueryMatrixVector)[j])-
										(float)(margToThreeFrom[i])[j])>eps )
									{
										ret = TRS_FAIL;
										break;
									}
								}
							}
						}
					}// if (nnodes>3)
					
				}//if (nnodes>2)
				for(i=0; i<nnodes; i++)
				{
					delete []margToOneFrom[i];
				}
				delete []margToOneFrom;
				for(i=0; i<nPairs; i++)
				{
					delete []margToPairFrom[i];
				}
				delete []margToPairFrom;
				for(i=0; i<nThree; i++)
				{
					delete []margToThreeFrom[i];
				}
				delete []margToThreeFrom;

				if(matlab_close_flag)
				{
				return trsResult( ret, ret == TRS_OK ? "No errors" : "Can't close MATLAB");
				}
				delete[]nodeSizes;
				delete[]pairDataSizes;
				delete []threeDataSizes;
		}//if (ep) = else (if(!ep))		
		delete pParam; 
		
		int paramMargToNode_mem_b = trsGuardCheck(paramMargToNode);
		int paramMargToPair_mem_b = trsGuardCheck(paramMargToPair);
		int paramMargToThree_mem_b = trsGuardCheck(paramMargToThree);
		int margToPair_mem_b = trsGuardCheck(margToPair);
		int margToThree_mem_b = trsGuardCheck(margToThree);
		int ntForParam_mem_b = trsGuardCheck(ntForParam);
		int margToNode_mem_b = trsGuardCheck(margToNode);
		int domain_memory_flag = trsGuardCheck( domain );
		
		if( domain_memory_flag||ntForParam_mem_b
			||margToNode_mem_b||margToPair_mem_b||paramMargToPair_mem_b
			||paramMargToNode_mem_b||paramMargToThree_mem_b
			||margToThree_mem_b)
		{
			return trsResult( TRS_FAIL, "Dirty memory");
		}
		for( i = 0; i <nnodes; i++ )
		{
			delete paramMargToNode[i];
		}
		trsGuardFree( paramMargToNode );
		for( i = 0; i < nPairs; i++ )
		{
			delete paramMargToPair[i];
		}
		trsGuardFree( paramMargToPair );
		for( i = 0; i < nThree; i++ )
		{
			delete paramMargToThree[i];
		}
		trsGuardFree( paramMargToThree );
		trsGuardFree( ntForParam );
		trsGuardFree( domain );
		trsGuardFree( margToNode );
		trsGuardFree( margToPair );
		trsGuardFree( margToThree );
		
		return trsResult( ret, ret == TRS_OK ? "No errors" : "Marginalize FAILED");
}

void initAMarginalizeCompareMATLAB()
{
    trsReg(func_name, test_desc, test_class, testMarginalizeCompareMATLAB);
}

