/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      AMRF2PearlInf.cpp                                           //
//                                                                         //
//  Purpose:   Test on Pearl Inference Engine correctness for MRF2         //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"
#include "pnl_dll.hpp"
#include "AMRF2PearlInf.hpp"


static char func_name[]  = "testPearlInfEngForMRF2";

static char test_desc[]  = "test correctness of Pearl for MRF2";

static char test_class[] = "Algorithm";

PNL_USING;
void SetRndObsNdsAndVals( const CModelDomain* pModelDomain, intVector* obsNds,
                          valueVector* obsNdsVals );

int testPearlInfEngForMRF2()
{
    
    int ret = TRS_OK; // of course ;)


    int numOfRows = 0;

    while( ( numOfRows < 1 ) || ( numOfRows > MAX_NUM_OF_ROWS ) )
    {
        trsiRead( &numOfRows, "3", "Number of rows in a 2-layered MRF2" );
    }
    
	
	int numOfCols = 0;

    while( ( numOfCols < 1 ) || ( numOfCols > MAX_NUM_OF_COLS ) )
    {
        trsiRead( &numOfCols, "3", "Number of columns in a 2-layered MRF2" );
    }


    int numOfNodeVals = 0;

    while( ( numOfNodeVals < 1 ) || ( numOfNodeVals > MAX_NUM_OF_NODE_VALS ) )
    {
        trsiRead( &numOfNodeVals, "10",
            "Number of values each node in a 2-layered MRF2 can take" );
    }


    int numOfNodeValDims = 0;

    while( ( numOfNodeValDims < 1 )
        || ( numOfNodeValDims > MAX_NUM_OF_NODE_VAL_DIMS ) )
    {
        trsiRead( &numOfNodeValDims, "10",
            "Number of dimensions of each node value in a 2-layered MRF2" );
    }


    int equalValNum = -2;

    while( ( equalValNum < -1 ) || ( equalValNum >= numOfNodeVals  ) )
    {
        trsiRead( &equalValNum, "0",
            "values of this number will be equal for all nodes MRF2\n"
            "if you choose value -1 (negative one), there will be no\n"
            "equal values for all nodes");
    }

    int maxNumOfIters = 0;

    while( ( maxNumOfIters < 1 ) || ( maxNumOfIters > MAX_NUM_OF_ITERS ) )
    {
        trsiRead( &maxNumOfIters, "3",
            "Number of iterations Pearl Inference engine will run for" );
    }

	int chosenType = 1;
	char *strVal;
	strVal = trsInt(chosenType);
	int typeOfPotential = -1;
    
    while( ( typeOfPotential < 0 )  )
    {
        trsiRead( &typeOfPotential, strVal, "Type of potential in created model" );
    }


    const int numOfNds    = 2*numOfRows*numOfCols;

    int numOfObsNds = numOfNds/2;


    nodeTypeVector nodeTypes(2);

    nodeTypes[0].SetType( true, numOfNodeVals );

    nodeTypes[1].SetType( true, 1 );

    intVector nodeAssociation( numOfNds, 0 );

    std::fill( nodeAssociation.begin() + numOfNds/2,
        nodeAssociation.end(), 1 );


    CModelDomain* pModelDomain = CModelDomain::Create( nodeTypes,
        nodeAssociation );

    pnlVector< floatVecVector > nodeVals( numOfNds/2,
        floatVecVector( numOfNodeVals,
        floatVector(numOfNodeValDims) ) );

    int i;

    for( i = 0; i < numOfNds/2; ++i )
    {
        int j;

        for( j = 0; j < numOfNodeVals; ++j )
        {
            if( j != equalValNum )
            {
                super_helper::GetRandomNodeVal( &nodeVals[i][j] );
            }
            else
            {
                super_helper::GetEqualNodeVal( &nodeVals[i][j] );
            }
        }
    }

//    for( i = 0 ; i < numOfNds/2; ++i )
//    {
//        for_each( nodeVals[i].begin(), nodeVals[i].end(),
//            super_helper::PrintVector<float>() );
//    }

    CMRF2* p2LMRF2Model = SuperResolution2lMRF2( numOfRows, numOfCols,
        pModelDomain, nodeVals );


    intVector   obsNds(numOfObsNds);

    valueVector obsNdsVals(numOfObsNds);


    for( i = 0; i < numOfObsNds; ++i )
    {
        obsNds[i] = numOfObsNds + i;

        obsNdsVals[i].SetInt(0);
    }

    CEvidence* pEvidence = CEvidence::Create( p2LMRF2Model,
        obsNds, obsNdsVals );

    CPearlInfEngine* pPearlInfEngine = 
        CPearlInfEngine::Create(p2LMRF2Model);

    CSpecPearlInfEngine* pSpecPearlInfEngine = 
       CSpecPearlInfEngine::Create(p2LMRF2Model);


    pPearlInfEngine->SetMaxNumberOfIterations(maxNumOfIters);
    pSpecPearlInfEngine->SetMaxNumberOfIterations(maxNumOfIters);

    pPearlInfEngine->EnterEvidence( pEvidence, true );
    pSpecPearlInfEngine->EnterEvidence( pEvidence, true );

    intVector maxIndices(numOfNds/2);
    intVector specMaxIndices(numOfNds/2);


    for( i = 0; i < numOfNds/2; ++i )
    {
        pPearlInfEngine->MarginalNodes( &i, 1 );

        const CEvidence* pMPE = pPearlInfEngine->GetMPE();

        pSpecPearlInfEngine->MarginalNodes( &i, 1 );

        const CEvidence* pSpecMPE = pSpecPearlInfEngine->GetMPE();

        intVector MPEobsNdsNums;

        pConstValueVector MPEobsNdsVals;

        pConstNodeTypeVector MPENodeTypes;

        pMPE->GetObsNodesWithValues( &MPEobsNdsNums, &MPEobsNdsVals,
            &MPENodeTypes );

        maxIndices[i] = MPEobsNdsVals[0]->GetInt();

        pSpecMPE->GetObsNodesWithValues( &MPEobsNdsNums, &MPEobsNdsVals,
            &MPENodeTypes );

        specMaxIndices[i] = MPEobsNdsVals[0]->GetInt();
    }

    std::cout << "Here're the numbers of node values chosen"
        " by Pearl (max_indices)" << std::endl;

    std::for_each( maxIndices.begin(), maxIndices.end(),
        super_helper::Print<int>() );
    //compare results
    int numIndices = maxIndices.size();
    for( i = 0; i < numIndices; i++ )
    {
        if( specMaxIndices[i] != maxIndices[i] )
        {
            ret = TRS_FAIL;
        }
    }

    if( ( equalValNum != -1 )
        && ( std::count_if( maxIndices.begin(), maxIndices.end(),
        super_helper::NotEqual<int>(equalValNum) ) > 0 ) )
    {
        ret = TRS_FAIL;
    }

    //add the code for testing specPearl 
	CMRF2* pModelToWorkWith = pnlExCreateBigMRF2( typeOfPotential, 15, 15, 4, 1.0f, 1.0f );

	const int numNodes = pModelToWorkWith->GetNumberOfNodes();

	maxNumOfIters = numNodes/2;

	numOfObsNds = rand()%( numNodes - 2 );
	
	intVector   obsNdsSp(numOfObsNds);
  
    valueVector obsNdsValsSp(numOfObsNds);

    SetRndObsNdsAndVals( pModelToWorkWith->GetModelDomain(), &obsNdsSp,
        &obsNdsValsSp );
    
    CEvidence* pEvidenceSp = CEvidence::Create( pModelToWorkWith,
        obsNdsSp, obsNdsValsSp );


    CPearlInfEngine* pPearlEng = CPearlInfEngine::Create(pModelToWorkWith);
    
    pPearlEng->SetMaxNumberOfIterations(maxNumOfIters);
    
//	trsTimerStart(0);
    
    pPearlEng->EnterEvidence(pEvidenceSp);

//    trsTimerStop(0);
    
//     double timeOfEnterEvidenceForPearl = trsTimerSec(0);
    
    int numProvIters = pPearlEng->GetNumberOfProvideIterations();
    
    CSpecPearlInfEngine* pPearlEng1 = CSpecPearlInfEngine::Create(pModelToWorkWith);
    
    pPearlEng1->SetMaxNumberOfIterations(maxNumOfIters);
    
//     trsTimerStart(0);
    
    pPearlEng1->EnterEvidence(pEvidenceSp);
    
//     trsTimerStop(0);
    
//     double timeOfEnterEvidenceForPearl1 = trsTimerSec(0);
    
    int numProvIters1 = pPearlEng1->GetNumberOfProvideIterations();
    
    //check are the potentials the same

    int potsAreTheSame = 1;
    float eps = 1e-5f;
    float maxDiff = 0.0f;
    const CPotential* potPearl = NULL;
    const CPotential* pot1Pearl = NULL;
    for( i = 0; i < numNodes; i++ )
    {
        pPearlEng->MarginalNodes(&i, 1);
        potPearl = pPearlEng->GetQueryJPD();
        pPearlEng1->MarginalNodes(&i, 1);
        pot1Pearl = pPearlEng1->GetQueryJPD();
        if( !potPearl->IsFactorsDistribFunEqual(pot1Pearl, eps, 0, &maxDiff ) )
        {
            potsAreTheSame = 0;
        }
    }
	std::cout<<"num iterations pearl: "<<numProvIters<<std::endl;
	std::cout<<"num iterations spec pearl: "<<numProvIters1<<std::endl;
//	std::cout<<"time pearl: "<<timeOfEnterEvidenceForPearl<<std::endl;
//	std::cout<<"time spec pearl: "<<timeOfEnterEvidenceForPearl1<<std::endl;

    delete pPearlEng;

    delete pPearlEng1;
    delete pModelToWorkWith;

	delete pEvidenceSp;
	//create other model

	//add the code for testing specPearl 

    CMRF2* pOtherModel= pnlExCreateBigMRF2( 5, 5, 5, 6, 1.0f, 1.0f );

    const int numOtherNodes = pOtherModel->GetNumberOfNodes();

	numOfObsNds = rand()%( numOtherNodes - 2 );

	intVector   obsNdsOtherSp(numOfObsNds);
  
    valueVector obsNdsValsOtherSp(numOfObsNds);

    SetRndObsNdsAndVals( pOtherModel->GetModelDomain(), &obsNdsOtherSp,
        &obsNdsValsOtherSp );
    
    CEvidence* pEvidenceOtherSp = CEvidence::Create( pOtherModel,
        obsNdsOtherSp, obsNdsValsOtherSp );

    CPearlInfEngine* pOtherPearlEng = CPearlInfEngine::Create(pOtherModel);
    
    pOtherPearlEng->SetMaxNumberOfIterations(maxNumOfIters);
    
    pOtherPearlEng->EnterEvidence(pEvidenceOtherSp);
    
    CSpecPearlInfEngine* pOtherPearlEng1 = CSpecPearlInfEngine::Create(pOtherModel);
    
    pOtherPearlEng1->SetMaxNumberOfIterations(maxNumOfIters);
    
    pOtherPearlEng1->EnterEvidence(pEvidenceOtherSp);
    
    //check are the potentials the same
    potsAreTheSame = 1;
    maxDiff = 0.0f;
    for( i = 0; i < numOtherNodes; i++ )
    {
        pOtherPearlEng->MarginalNodes(&i, 1);
        potPearl = pOtherPearlEng->GetQueryJPD();
        pOtherPearlEng1->MarginalNodes(&i, 1);
        pot1Pearl = pOtherPearlEng1->GetQueryJPD();
        if( !potPearl->IsFactorsDistribFunEqual(pot1Pearl, eps, 0, &maxDiff ) )
        {
            potsAreTheSame = 0;
        }
    }

    delete pEvidenceOtherSp;
	delete pOtherPearlEng;
    delete pOtherPearlEng1;
    delete pOtherModel;

    delete pEvidence;
    //CPearlInfEngine::Release(&pPearlInfEngine);
    delete pPearlInfEngine;
    delete pSpecPearlInfEngine;
    delete p2LMRF2Model;
    delete pModelDomain;


    return ret;
}
//////////////////////////////////////////////////////////////////////////

CTabularPotential* 
SuperResolutionTabPot( const intVector& clq, int clqInd,
                       const pnlVector < floatVecVector >& nodeVals,
                       CModelDomain* pModelDomain )
{
    const int numOfNodeVals = nodeVals[0].size();

    const int numOfValDims  = nodeVals[0][0].size();

    const int sampleSize    = (int)sqrt(float(numOfValDims));


    switch ( clqInd )
    {
        case 0:
        {
            floatVector tableData(numOfNodeVals*numOfNodeVals);

            int i = 0;

            for( ; i < numOfNodeVals; ++i )
            {
                const floatVector& nodeOneVal = nodeVals[clq[0]][i];

                int j = 0;

                for( ; j < numOfNodeVals; ++j )
                {
                    const floatVector& nodeTwoVal = nodeVals[clq[1]][j];
                    
                    float pen = 0;

                    int   k   = 0;

                    for( ; k < numOfValDims; ++k )
                    {
                        if( k % sampleSize )
                        {
                            pen += (float)(( nodeOneVal[k]
                                - nodeTwoVal[k - 1] )*( nodeOneVal[k]
                                - nodeTwoVal[k - 1]));
                        }
                    }

                    tableData[i*numOfNodeVals + j] =
                        (float)exp( -pen/( 2*SIGMA_SCENE*SIGMA_SCENE ) );
                }
            }

            CTabularPotential* pTPot = CTabularPotential::Create(
                clq, pModelDomain, &tableData.front() );

            pTPot->Normalize();
            
            return pTPot;
        }
        case 1:
        {
            floatVector tableData(numOfNodeVals*numOfNodeVals);

            int i = 0;

            for( ; i < numOfNodeVals; ++i )
            {
                const floatVector& nodeOneVal = nodeVals[clq[0]][i];

                int j = 0;
                
                for( ; j < numOfNodeVals; ++j )
                {
                    const floatVector& nodeTwoVal = nodeVals[clq[1]][j];
                    
                    float pen = 0;

                    int   k   = sampleSize;

                    for( ; k < numOfValDims; ++k )
                    {
                        pen += (float)(( nodeOneVal[k]
                            - nodeTwoVal[k - sampleSize] )
                            *( nodeOneVal[k] - nodeTwoVal[k - sampleSize] ));
                    }

                    tableData[i*numOfNodeVals + j] =
                        (float)exp( -pen/( 2*SIGMA_SCENE*SIGMA_SCENE ) );
                }
            }

            CTabularPotential* pTPot = CTabularPotential::Create(
                clq, pModelDomain, &tableData.front() );

            pTPot->Normalize();

            return pTPot;
        }
        case 2:
        {
            return CTabularPotential::CreateUnitFunctionDistribution(
                clq, pModelDomain );
        }
        default:
        {
            return NULL;
        }
    }
}
//////////////////////////////////////////////////////////////////////////

CMRF2* 
SuperResolution2lMRF2( int numOfRows, int numOfCols, 
                       CModelDomain* pModelDomain,
                       const pnlVector< floatVecVector >& nodeVals )
{
    const int numOfNds  = 2*numOfRows*numOfCols;
    const int numOfClqs = ( numOfRows - 1 )*numOfCols
        + ( numOfCols - 1 )*numOfRows + numOfRows*numOfCols;
    
    intVecVector clqs( numOfClqs, intVector(2) );
    intVector    clqDirs(numOfClqs);
    
    bool right = numOfCols == 1 ? false : true;
    bool down  = numOfRows == 1 ? false : true;
    
    int i = 0, j = 0;
    
    for( ; i < numOfNds/2; ++i )
    {
        bool set = false;
        
        int check1 = i/numOfCols;
        int check2 = i%numOfCols;
        
        clqs[j][0] = i;
        clqs[j][1] = i + numOfCols*numOfRows;
        clqDirs[j] = 2;
        
        j++;
        
        //upper left corner
        if( (check1 == 0) && (check2 == 0) )
        {
            if(right)
            {
                clqs[j][0] = i;
                clqs[j][1] = i + 1;
                clqDirs[j] = 0;
                
                j++;
            }
            
            if(down)
            {
                clqs[j][0] = i;
                clqs[j][1] = i + numOfCols;
                clqDirs[j] = 1;
                
                j++;
            }
            
            set = true;
        }
        
        //upper right corner
        if( ( check1 == 0 ) && ( check2 == ( numOfCols - 1 ) ) && ( !set ) )
        {
            if(down)
            {
                clqs[j][0] = i;
                clqs[j][1] = i + numOfCols;
                clqDirs[j] = 1;
                
                j++;
            }
            
            set = true;
        }
        
        //lower left corner
        if( ( check1 == (numOfRows - 1) ) && ( check2 == 0 ) && ( !set ) )
        {
            if(right)
            {
                clqs[j][0] = i;
                clqs[j][1] = i + 1;
                clqDirs[j] = 0;
                
                j++;
            }
            
            set = true;
        }
        
        //lower right corner
        if( ( check1 == ( numOfRows - 1 ) ) && ( check2 == ( numOfCols - 1 ) )
            && ( !set ) )
        {
            set = true;
        }
        
        //left side
        if( ( check2 == 0 ) && ( !set ) )
        {
            if( right )
            {
                clqs[j][0] = i;
                clqs[j][1] = i + 1;
                clqDirs[j] = 0;
                
                j++;
            }
            
            clqs[j][0] = i;
            clqs[j][1] = i + numOfCols;
            clqDirs[j] = 1;
            
            j++;
            
            set = true;
        }
        
        //right side
        if( ( check2 == ( numOfCols - 1 ) ) && ( !set ) )
        {
            clqs[j][0] = i;
            clqs[j][1] = i + numOfCols;
            clqDirs[j] = 1;
            
            j++;
            
            set = true;
        }
        
        //upper side
        if( ( check1 == 0 ) && ( !set ) )
        {
            clqs[j][0] = i;
            clqs[j][1] = i + 1;
            clqDirs[j] = 0;
            
            j++;
            
            if(down)
            {
                clqs[j][0] = i;
                clqs[j][1] = i + numOfCols;
                clqDirs[j] = 1;
                
                j++;
            }
            
            set = true;
        }
        
        //lower side
        if( ( check1 == ( numOfRows - 1 ) ) && ( !set ) )
        {
            clqs[j][0] = i;
            clqs[j][1] = i + 1;
            clqDirs[j] = 0;
            
            j++;
            
            set = true;
        }
        
        //inner part of the matrix
        if(!set)
        {
            clqs[j][0] = i;
            clqs[j][1] = i + 1;
            clqDirs[j] = 0;
            
            j++;
            
            clqs[j][0] = i;
            clqs[j][1] = i + numOfCols;
            clqDirs[j] = 1;
            
            j++;
        }
    }

    CMRF2* pModel = CMRF2::Create( clqs, pModelDomain );

    // allocate potentials and generate table for them

    pModel->AllocFactors();

    for( i = 0; i < numOfClqs; ++i )
    {
        CTabularPotential* pTPot = SuperResolutionTabPot( clqs[i],
            clqDirs[i], nodeVals, pModelDomain );

        pModel->AttachFactor(pTPot);
    }

    return pModel;
}
//////////////////////////////////////////////////////////////////////////

void initAMRF2PearlInf()
{
    trsReg( func_name, test_desc, test_class, testPearlInfEngForMRF2 );
}
