
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      ARockPaperScissors.cpp                                      //
//                                                                         //
//  Purpose:   Test on bnet factor learning during the game                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"

#include "pnl_dll.hpp"
#include "pnlLearningEngine.hpp"
#include <iostream>

static char func_name[] = "testFactorLeaningInGame";

static char* test_desc = "Test factor leaning on Bnet in game";

static char* test_class = "Application";


int bestPossibleAnswer( int humMes, char *outComMes)
// to define the right answer to the pointed humMessage
{
    switch (humMes)
    {
    case 0: sprintf(outComMes, "PAPER"); return 1;
    case 1: sprintf(outComMes, "SCISSORS"); return 2;
    case 2: sprintf(outComMes, "ROCK"); return 0;
    default:sprintf(outComMes, "ERROR!"); return -1;
    }
}

int isHumanWinner( int comMes, int humMes)
// to define, who wins?
{
    int i = (int) fabs(float(comMes-humMes));
    switch (i)
    {
    case 0: return -1;
    case 1:
        if (comMes<humMes) return 1;
        else return 0;
    default:
        if (comMes==0) comMes = 3;    else humMes = 3;
        if (comMes<humMes) return 1;
        else return 0;
    }
}

int testFactorLeaningInGame()
{
    PNL_USING
    // model for the game
    // 
    /*
    
     0 
      \
       2
      /
     1 

    */
    
    //create this model
    int numOfNdTypes = 1;
    
    const int numberOfNodes = 3;// number of nodes is 
    const int numberOfNodeTypes = 1;// number of distribution types
    CNodeType *nodeTypes = new CNodeType[numOfNdTypes];

    int i, j;
    
    int obsNodes[]={ 0, 1, 2};
    int numOfNeigh[] = { 1, 1, 2};
    
    int neigh0[] = { 2 };
    int neigh1[] = { 2 };
    int neigh2[] = { 0, 1 };
    
    ENeighborType orient0[] = { ntChild };
    ENeighborType orient1[] = { ntChild };
    ENeighborType orient2[] = { ntParent, ntParent };
    
    int *neigh[] = { neigh0, neigh1, neigh2};
    ENeighborType *orientation[] = { orient0, orient1, orient2};
    
    CGraph *graph;
  	CBNet *bnet;

    graph = CGraph::Create( numberOfNodes, numOfNeigh, neigh, orientation );    
    
    //Create graphical model 
    
    nodeTypes = new CNodeType [numberOfNodeTypes];
    int *nodeAss = new int [numberOfNodes];
    
    nodeTypes[0].SetType(1, 3);
    
//    all nodes are discret and may have 3 values

    nodeAss[0] = 0;
    nodeAss[1] = 0;
    nodeAss[2] = 0;
    
    // conditional probabilities before learning
    float OneThird = 1.f/3.f;
    float data0[3];
    float data1[3];
    float data2[27];

    for (i=0; i<3; i++) 
    {
        data0[i] = data1[i] = OneThird;
    }
    for (i=0; i<27; i++) data2[i] = OneThird;

    pnlVector <float*> prior;
        
    prior.push_back( data0 ); 
    prior.push_back( data1 ); 
    prior.push_back( data2 ); 
        
    bnet =  CBNet::Create(numberOfNodes, numberOfNodeTypes,
        nodeTypes, nodeAss,    graph );
    
    bnet -> AllocFactors();
        
    
    for ( int node = 0; node < numberOfNodes; node++ )
    {
        bnet->AllocFactor( node );
        (bnet->GetFactor( node )) ->AllocMatrix( prior[node], matTable );
        (bnet->GetFactor( node )) ->AllocMatrix( prior[node], matDirichlet );
    }
    
   	ofstream fout("CPDafterLearning");
   	CNumericDenseMatrix<float> *pMatrix;
	int length = 0;
	const float *output;
    
    ///////////Load Evidences From File ////////////
    char *argv = "../testdata/RockPaperScissors54.txt";
   	ifstream fin(argv);

    const int numberOfEvidences = 54;
	CEvidence **m_pEv;
    
    m_pEv = new CEvidence *[numberOfEvidences];
    pnlVector<valueVector> data(numberOfEvidences);
    for( i = 0; i < numberOfEvidences; i++)
    {
        data[i].resize(numberOfNodes);
    }
   
    if( !fin )
    {
        printf( "can't open file %s\n", argv );
        
        exit(1);
    }
        
    i = 0;
    j = 0;
    int num;
    for ( i =0; i< numberOfEvidences; i++)
    { 
        for (j = 0; j<numberOfNodes; j++)
        {
            fin >> num;
            if (num==EOF) break;
            (data[i])[j].SetInt(num);
           
        }
    }
    
    for (i=0;i<numberOfEvidences;i++)
    {
        m_pEv[i] = CEvidence::Create(bnet, numberOfNodes, obsNodes, data[i]);
    
    }
    
    // Creating learning process
    
    CBayesLearningEngine *pLearnTabular = CBayesLearningEngine::Create(bnet);
    pLearnTabular->SetData(54, m_pEv);
    pLearnTabular->Learn();

    printf("initial conditional probability for BNet \n");
    for (i = 0; i < numberOfNodes; i++)
    {
        pMatrix = static_cast<CNumericDenseMatrix<float>*>
            (bnet->GetFactor(i)->GetMatrix(matTable));
        pMatrix->GetRawData(&length, &output);
        for (int j = 0; j < length; j++)
        {
            printf("(%4.3f)\t",output[j]);
        }
        printf("\n");
    }
    
    int comMes, humMes, humMesInf;
    int numWin, numLost, numTie;
    int newWin;
    CEvidence *m_pEvNew;
    pnl::valueVector dataNew(1);
    dataNew.resize(numberOfNodes);
    char reply[80];
    char s='N'; // Let be: 'E' - the end of session, 
                // 'N' - to start from the  beginning 
                // 'R', 'P' and 'S' - the correspondent things
    char outComMes[80], hm[80];
    int query[] = { 2 };
    int querySize = 1;
    intVecVector queries(2);
    int NumIter;
    int numNodeVar = 2;
    dataNew[0].SetInt(2);
    dataNew[1].SetInt(1);
    dataNew[2].SetInt(1);
    m_pEvNew = CEvidence::Create(bnet, numberOfNodes, obsNodes, dataNew);
    int numGame = 0;

    NumIter = 3;//numberOfNodes;
    
to_start:   //  start new game 
    numWin = 0; numLost = 0; numTie = 0; 
    comMes = 2;
    sprintf(outComMes, "SCISSORS");
    humMes = -1;

    while(s!='E'&&s!='e')
    {
        if (humMes>-1)
        {
            CPearlInfEngine* pPearlEng = CPearlInfEngine::Create(bnet);
            pPearlEng->SetMaxNumberOfIterations(NumIter);
            m_pEvNew -> SetData( dataNew );
            int flag = m_pEvNew->IsNodeObserved(2);
            if (flag) m_pEvNew -> ToggleNodeState( 1, &numNodeVar );

            pPearlEng->EnterEvidence(m_pEvNew,1);       

            pPearlEng->MarginalNodes( query, querySize ); //to calculate CPD for the node 2
            humMesInf = (int)pPearlEng->GetMPE()->GetValue(2)->GetInt();//possible human choice
        
            comMes = bestPossibleAnswer(humMesInf, outComMes);//The best computer answer

            delete pPearlEng;
        }
        printf("Please enter R,P,S or N for new series, E to exit \n");
        fgets(reply, sizeof(reply), stdin);
        sscanf(reply,"%c",&s);
        humMes = -1;
        switch (s)
        {
        case 'n':
        case 'N': printf("WIN %d - LOSE %d. \nWe start the new game. ",numWin, numLost); numGame++; goto to_start;
        case 'r':
        case 'R': humMes = 0;  sprintf(hm,"ROCK"); break;
        case 'p':
        case 'P': humMes = 1;  sprintf(hm,"PAPER"); break;
        case 's':
        case 'S': humMes = 2;  sprintf(hm,"SCISSORS"); break;
        case 'e':
        case 'E': printf("You WIN %d - LOSE %d. \n Bye!",numWin, numLost);break;
        default: break;
        }

        dataNew[2].SetInt(humMes); //At last we have it!

        if (humMes>-1)
        {   
            //form and append new evidence

            if (numWin==0 && numLost==0 && numTie==0 && numGame==0) // For the first time
            {
                // just to have any values
                dataNew[0].SetInt(comMes);            
                dataNew[1].SetInt(humMes);
                m_pEvNew->SetData( dataNew );
//                m_pEvNew->MakeNodeObserved(2);
                pLearnTabular->SetData(1, &m_pEvNew);
                pLearnTabular->Learn();
            }


            if (numWin!=0 || numLost!=0 || numTie!=0) // if this is not first time
            {
                // append human choise
                m_pEvNew->SetData( dataNew );
                int flag = m_pEvNew->IsNodeObserved(2);
                if (!flag) m_pEvNew->MakeNodeObserved(2);
                
                pLearnTabular->AppendData(1, &m_pEvNew);
                pLearnTabular->Learn();
                
                for (i = 0; i < numberOfNodes; i++)
                {
                    pMatrix = static_cast<CNumericDenseMatrix<float>*>
                        (bnet->GetFactor(i)->GetMatrix(matTable));
                    pMatrix->GetRawData(&length, &output);
                    for (int j = 0; j < length; j++)
                    {
                        printf("(%4.3f)\t",output[j]);
                    }
                    printf("\n");
                }
                //change new evidence to human and computer choise
                dataNew[0].SetInt(comMes);
                dataNew[1].SetInt(humMes);
            }
            
            
            newWin = isHumanWinner( comMes, humMes);

            printf("Your choice is %s.  My choice is %s",hm,outComMes);
            switch (newWin)
            {
            case -1: printf("  TIE game! \n\n");numTie++; break;
            case  1: printf("  You WIN \n\n"); numWin++; break;
            case  0: printf("  You LOSE \n\n");numLost++; break;
            default: break;
            }
        }   
    }

    delete(m_pEvNew);
    delete(bnet);
    delete(pLearnTabular);
return TRS_OK;
}

void initARockPaperScissors()
{
    trsReg(func_name, test_desc, test_class, testFactorLeaningInGame);
}

