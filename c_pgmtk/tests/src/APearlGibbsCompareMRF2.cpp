/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      APearlGibbsCompareMRF2.cpp                                      //
//                                                                         //
//  Purpose:   Test and compare Pearl inference and Gibbs sampling on      //
//             "big models", that are MRF2                                 //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"
#include "pnl_dll.hpp"
#include <sstream>
#include <fstream>
#include <time.h>
#include <math.h>

static char func_name[] = "testPearlGibbsCompareMRF2";

static char* test_desc = "Test Pearl and Gibbs on big models";

static char* test_class = "Algorithm";

using namespace std;
using namespace pnl;

class CLogger
{
    //To write the log in file
public:
    
    CLogger(ostream& log) : m_pLog(&log) {}
    
    void PrintToLog( const string& message = " time elapsed - ") const
    {
        (*m_pLog) << message.c_str()  << endl;
    }
    
    void ChangeLogStream(ostream &newLog)
    {
        m_pLog = &newLog;
    }

private:
    mutable ostream *m_pLog;
};

//////////////////////////////////////////////////////////////////////////
// Model and task parameters

    int m_Kolkand;          // tne number of categories
    int m_numOfRows;        //            of rows in the model
    int m_numOfCols;        //               columns in the model
    int MaxNumIter;         // max number of iteration
    int IterTheSameLL; // The max number of iteration with the same likelihood
    double PreviousLL; // For keeping of current LL value
    int theFirstIter;  // switch if the first iteration
    int NumIterPorog;  // the number of iteration with equal log(L) to stop the inference
    int AlgKoef;       // coefficient for Gibbs sampling
    int toContinue;    // Switch - if we are already happy with likelihood
    int StartInferenceAt;
    int NumInference;
    float sigmahor, sigmaver; // potential factors
    int printModel;    // 0 - no printing, 1 - print all, 2 - only the last state
    int printPot;      // if 1 - print potentials
    typedef enum       //Edges direction in the model
    {
        cdHorizontal, cdVertical
    } EClqDirs;
    pnl::intVecVector      m_clqs;         // the list of cliqes
    std::vector<EClqDirs>  m_clqDirection; // their directions
    pnl::CMRF2            *m_pGrModel;     // The model - a grid m_numOfRows X m_numOfCols


static CEvidence* CreateTestEvidence(CMRF2 *pMRF2T)
{
    //Initialization of empty Evidence
    const int numOfObsNds  = 0;
    const int *obsNds     = NULL;
    valueVector obsNdsVals(0);
   
    CEvidence *pEvidence = CEvidence::Create( pMRF2T, numOfObsNds, obsNds,
        obsNdsVals );

    return pEvidence;
}
static CEvidence* CreateObservEvidence(CMRF2 *pMRF2T)
{
    const int numOfObsNds  = 12;
    int obsNds[12] = {0,1,10,11,12,13,22,23,29,30,41,42};
    valueVector obsNdsVals(numOfObsNds);

    obsNdsVals[0].SetInt(m_Kolkand-1);
    obsNdsVals[1].SetInt(m_Kolkand-1);
    obsNdsVals[2].SetInt(m_Kolkand/4);
    obsNdsVals[3].SetInt(m_Kolkand/4);
    obsNdsVals[4].SetInt(m_Kolkand-1);
    obsNdsVals[5].SetInt(m_Kolkand-1);
    obsNdsVals[6].SetInt(m_Kolkand/4);
    obsNdsVals[7].SetInt(m_Kolkand/4);
    obsNdsVals[8].SetInt(m_Kolkand/2);
    obsNdsVals[9].SetInt(m_Kolkand/2);
    obsNdsVals[10].SetInt(m_Kolkand/2);
    obsNdsVals[11].SetInt(m_Kolkand/2);

    CEvidence *pEvidence = CEvidence::Create( pMRF2T, numOfObsNds, obsNds,
        obsNdsVals );
    return pEvidence;
}


// Functions

void FillInClqAndDirection( int clqNum, int firstNodeNum, int secondNodeNum,
                       EClqDirs clqDirection )
//the function for cliqes data creation
{

    m_clqs[clqNum][0]      = firstNodeNum;
    
    m_clqs[clqNum][1]      = secondNodeNum;
    
    m_clqDirection[clqNum] = clqDirection;
}

int NodeToRight(int currNodeNum)
// Function to get the neighbour node to the right
{
    return currNodeNum + 1;
}

int NodeUnder(int currNodeNum)
// Function to get the neighbour node to the down
{
    return currNodeNum + m_numOfCols;
}

double PotentialFun(int clqNum, int i, int j, int variant, float sigmahor, float sigmaver)
//
// Function for correcting of potentials depending on displacement in the model
//
{
    double value;
    double PI = 3.1415926;
    double x, y;
    double dist;
    double coef;

    switch ( variant )
    {
    case 0: 
        if ( m_clqDirection[clqNum] == cdHorizontal )
            value = (float)exp(- (j - i)*(j - i ) / ( 2 * sigmahor ) );
        else 
            value = (float)exp( - (j - i - 1)*(j - i - 1) / ( 2 * sigmaver )) ;
        return value;
        break;
    case 1:
        value = (float)exp(- (j - i)*(j - i ) / ( 2.f * sigmahor ) );
        return value;
    case 2: 
        return  fabs(0.1+sin(PI*j/m_Kolkand));
    case 3: 
        if ( m_clqDirection[clqNum] == cdHorizontal )
            return  fabs(0.1+sin(PI*j/m_Kolkand))*(float)exp(- (j - i)*(j - i ) / ( 2 * sigmahor ) );
        else if ( m_clqDirection[clqNum] == cdVertical )
            return   fabs(0.1+sin(PI*j/m_Kolkand))*(float)exp( - (j - i - 1)*(j - i - 1) / ( 2 * sigmaver ) );
        break;
    case 4: 
        return  fabs(sin(PI*j/m_Kolkand));
    case 5: 
        if ( m_clqDirection[clqNum] == cdHorizontal )
            return  fabs(sin(PI*j/m_Kolkand))*(float)exp(- (j - i)*(j - i ) / ( 2 * sigmahor ) );
        else if ( m_clqDirection[clqNum] == cdVertical )
            return   fabs(sin(PI*j/m_Kolkand))*(float)exp( - (j - i - 1)*(j - i - 1) / ( 2 * sigmaver ) );
    case 6: 
        value = 2*(i-j)*(i-j)*exp(-(i-j)*(i-j)/2.); // Hermit function for difference: "feedback"  
        if (value<0.0001) value = 0.0001;
        
        return  value;
    case 7: 
        value = 2*(i-j)*(i-j)*exp(-(i-j)*(i-j)/2.); // Hermit function for difference: "feedback"
        if (value<0.0001) value = 0.0001;
        
        if ( m_clqDirection[clqNum] == cdHorizontal )
            return  value*(float)exp(- (j - i)*(j - i ) / ( 2 * sigmahor ) );
        else if ( m_clqDirection[clqNum] == cdVertical )
            return   value*(float)exp( - (j - i - 1)*(j - i - 1) / ( 2 * sigmaver ) );
        return  value;
    case 8: 
        if ( m_clqDirection[clqNum] == cdHorizontal )
            value = 2*(i-j)*(i-j)*exp(-(i-j)*(i-j)/2.); // Hermit function with difference for different directions
        else 
            value = 2*(i-j-1)*(i-j-1)*exp(-(i-j-1)*(i-j-1)/2.); 
        if (value<0.0001) value = 0.0001;
        return  value;
    case 9:
        if ( m_clqDirection[clqNum] == cdHorizontal )
        {
            x = 4./m_Kolkand * (i - m_Kolkand/2);
            y = 4./m_Kolkand * (j - m_Kolkand/2);
        }
        else //if ( m_clqDirection[clqNum] == cdVertical )
        {
            x = 4./m_Kolkand * (i - 1 - m_Kolkand/2);
            y = 4./m_Kolkand * (j - 1 - m_Kolkand/2);
        }
        value = 2*1.526*4.2*x*y*exp (-1.5*x*x/2.-1.5*y*y/2.)/PI;
        if (value<0.0001) value = 0.0001;
        return  value;
    case 10:
        x = 4./m_Kolkand * (i - m_Kolkand/2);
        y = 4./m_Kolkand * (j - m_Kolkand/2);
        value = 2*1.526*4.2*x*y*exp (-1.5*x*x/2.-1.5*y*y/2.)/PI;
        if (value<0.0001) value = 0.0001;
        return  value;
    case 11:
        if ( m_clqDirection[clqNum] == cdHorizontal )
        {
            x = 4./m_Kolkand * (i - m_Kolkand/2);
            y = 4./m_Kolkand * (j - m_Kolkand/2);
        }
        else //if ( m_clqDirection[clqNum] == cdVertical )
        {
            x = 4./m_Kolkand * (i - 1 - m_Kolkand/2);
            y = 4./m_Kolkand * (j - 1 - m_Kolkand/2);
        }
        value = 2*12.3*x*y*exp (-2.76*x*x/2.-2.76*y*y/2.)/PI;
        if (value<0.0001) value = 0.0001;
        return  value;
    case 12: 
        if ( m_clqDirection[clqNum] == cdHorizontal ) // here Pearl converges to classic pic.
            value = 0.98f*(float)exp(-(j - i)*(j - i )/ 2.f)+ //with lines: 0,1,2,...
            0.99f*(float)exp(-(j-i-3)*(j-i-3 )/ 2.f); 
        else 
            value = (float)exp( - (j - i-1 )*(j - i-1 ) / 2.f ) ;
        if (value>1.0f) value = 1.0;
        return value;
        break;
    case 13:
        if ( m_clqDirection[clqNum] == cdHorizontal )   // for Pearl converges to the columns of 0,1,2
            value = 0.5f*(float)exp(-(j - i)*(j - i )/ 2.f)+
            0.55f*(float)exp(-(j-i-1)*(j-i-1 )/ 2.f);
        else 
            value = (float)exp( - (j - i )*(j - i ) / 2.f ) ;
        if (value>1.0f) value = 1.0;
        return value;
        break;
        
    case 14: 
        if ( m_clqDirection[clqNum] == cdHorizontal )    // the same, so we have convergence to 
            value = 0.5f*(float)exp(-(j - i)*(j - i )/ 2.f)+ // local maximum
            0.55f*(float)exp(-(j-i-1)*(j-i-1 )/ 2.f);
        else 
            value = (float)exp( - (j - i-1 )*(j - i-1 ) / 2.f ) ;
        if (value>1.0f) value = 1.0;
        return value;
        break;
        
    case 15: 
        if ( m_clqDirection[clqNum] == cdHorizontal )
        {
            if (i-j==0) value = 0.8f;
            else
            {
                dist = double((i - j > 0) ? i - j : j - i);
                coef = 0.2f*m_Kolkand+0.4f;
                value = (float)exp(-fabs((coef+0.01)*sin(4.*PI/m_Kolkand*dist))/ 2.f*sigmahor);
            }
        }
        else 
            value = (float)exp( - (j - i -1 )*(j - i -1 ) / 2.f ) ;
        return value;
        break;
    case 16: 
//        if ( m_clqDirection[clqNum] == cdHorizontal )    
            value = 0.991f*(float)exp(-(j - i)*(j - i )/ (2.f*sigmahor))+ 
            0.992f*(float)exp(-(j-i-4)*(j-i-4 )/ (2.f*sigmahor));
//        else 
//            value = (float)exp( - (j - i-1)*(j - i-1 ) / 2.f ) ;
//        if (value>1.0f) value = 1.0;
        return value;
        break;
    case 17:
        value = (i+1)*(float)exp(- (j - i)*(j - i ) / ( 2.f * sigmahor ) );
        return value;
   
        
    default: break;
        
        
    }
    return 1.f;

}

void RowColByClqNum(int clqNum, int *row1, int *col1, int *row2, int *col2 )
// For definition for cliqe it's start and end nodes row and column 
{
    int firstNum, secondNum;

    firstNum = m_clqs[clqNum][0];
    *row1 = firstNum / m_numOfCols;
    *col1 = firstNum % m_numOfCols;
    secondNum = m_clqs[clqNum][1];
    *row2 = secondNum / m_numOfCols;
    *col2 = secondNum % m_numOfCols;
    return;
}

void ComputePotentialMatrixForClq( int clqNum,
                                  pnl::floatVector *tableData, const string& logfilename,
                                  int variant, float sigmahor, float sigmaver)
// Function for potential creation in the model nodes
// Every node may have the value from 0 to (m_Kolkand-1)
{ 

int row1, row1ToPrint = 0; 
int col1, col1ToPrint = 1;
int row2, row2ToPrint = m_Kolkand-1; 
int col2, col2ToPrint = m_Kolkand-1;
int i,j;


    for ( i = 0 ; i < m_Kolkand; i++ )
    {
       for ( j = 0; j < m_Kolkand; j++ )
        {
            (*tableData)[ i*(m_Kolkand)+ j ] = (float) PotentialFun(clqNum,i,j, variant, sigmahor, sigmaver);
            
        }
    }

    ofstream logfile( logfilename.c_str(), ios::app );
    CLogger logger(logfile); 

    RowColByClqNum(clqNum, &row1, &col1, &row2, &col2);

    if ((printPot==1)&&
        (((row1==row1ToPrint) && (col1==col1ToPrint)) || ((row2==row2ToPrint) &&(col2==col2ToPrint))))
    {
        stringstream m1;
        
        m1 << "variant:" << variant << "====== sigma:" << sigmahor <<"   " << sigmaver <<endl;
        m1 << "clqNum= "<<clqNum<<"type "<< m_clqDirection[clqNum]<< "row1 = "<< row1 << "col1 = " << col1 << endl;
        logger.PrintToLog( m1.str() );
        
        for (i = 0; i</*m_Kolkand*/10; i++) //printing of potentials 
        {
            stringstream m2;
            
            for (j = 0; j</*m_Kolkand*/10; j++)
            {
                m2 << (*tableData)[ i*(m_Kolkand) + j]<< "  ";
                
            }
            logger.PrintToLog( m2.str() );
        }
        
    }
 
}



void Construct2Clqs()
// Function for construction of cliques and their directions
{
    const int  numOfNds  = m_numOfRows*m_numOfCols;

    const int  numOfClqs = ( m_numOfRows - 1 )*m_numOfCols
        + ( m_numOfCols - 1 )*m_numOfRows ;
    
    const bool bNotOneColModel = m_numOfCols == 1 ? false : true,
               bNotOneRowModel = m_numOfRows == 1 ? false : true;
    
    
    m_clqs.resize(numOfClqs, intVector(2) );
    
    m_clqDirection.resize(numOfClqs);
    

    int nodeNum = 0, clqNum = 0;
    
    for( ; nodeNum < numOfNds ; ++nodeNum )
    {
        bool bClqsNotFilled = true;
        
        int currRowNum = nodeNum / m_numOfCols,
            currColNum = nodeNum % m_numOfCols;
        // Traversal by nodes and construct cliques
        // First the bound cases are considered
        // then the edges like
        //  0-0
        //  |
        //  0
        if( ( currRowNum == 0 ) && ( currColNum == 0 ) )
        {
            if( bNotOneColModel )
            {
                FillInClqAndDirection( clqNum++, nodeNum,
                    NodeToRight(nodeNum), cdHorizontal );
            }
            
            if( bNotOneRowModel )
            {
                FillInClqAndDirection( clqNum++, nodeNum, NodeUnder(nodeNum),
                    cdVertical );
            }
            
            bClqsNotFilled = false;
        }
        
        //upper right corner
        //clique like 0
        //            |
        //            0
        //the clique - edge to the left is to be already created
        if( bClqsNotFilled && ( currRowNum == 0 )
            && ( currColNum == ( m_numOfCols - 1 ) ) )
        {
            if( bNotOneRowModel )
            {
                FillInClqAndDirection( clqNum++, nodeNum, NodeUnder(nodeNum),
                    cdVertical );
            }
            
            bClqsNotFilled = false;
        }
        
        //lower left corner
        //clique like  0-0
        //            
        // 
        //the clique - edge to the up is to be already created

        if( bClqsNotFilled && ( currRowNum == ( m_numOfRows - 1 ) )
            && ( currColNum == 0 ) )
        {
            if( bNotOneColModel )
            {
                FillInClqAndDirection( clqNum++, nodeNum,
                    NodeToRight(nodeNum), cdHorizontal );
            }
            
            bClqsNotFilled = false;
        }
        
        //lower right corner
        // here nothing to create: all cliques-edges are already created
        if( bClqsNotFilled && ( currRowNum == ( m_numOfRows - 1 ) )
            && ( currColNum == ( m_numOfCols - 1 ) ) )
        {
            bClqsNotFilled = false;
        }
        
        //left side
        //            0-0
        //            |
        //            0
        //
        if( bClqsNotFilled && ( currColNum == 0 ) )
        {
            if( bNotOneColModel )
            {
                FillInClqAndDirection( clqNum++, nodeNum,
                    NodeToRight(nodeNum), cdHorizontal );
            }
            
            FillInClqAndDirection( clqNum++, nodeNum, NodeUnder(nodeNum),
                cdVertical);
            
            bClqsNotFilled = false;
        }
        
        //right side
        //            0
        //            |
        //            0
        //clique- edge to the left was already created
        if( bClqsNotFilled && ( currColNum == ( m_numOfCols - 1 ) ) )
        {
            FillInClqAndDirection( clqNum++, nodeNum, NodeUnder(nodeNum),
                cdVertical);

            bClqsNotFilled = false;
        }
        
        //upper side
        //            0-0
        //            |
        //            0
        //
        if( bClqsNotFilled && ( currRowNum == 0 ) )
        {
            FillInClqAndDirection( clqNum++, nodeNum,
                NodeToRight(nodeNum), cdHorizontal );
            
            if( bNotOneRowModel )
            {
                FillInClqAndDirection( clqNum++, nodeNum, NodeUnder(nodeNum),
                    cdVertical);
            }
            
            bClqsNotFilled = false;
        }
        
        //lower side
        //clique like  0-0
        if( bClqsNotFilled && ( currRowNum == ( m_numOfRows - 1 ) ) )
        {
            FillInClqAndDirection( clqNum++, nodeNum, NodeToRight(nodeNum),
                cdHorizontal );
            
            bClqsNotFilled = false;
        }
        
        //inner part of the matrix
        //            0-0
        //            |
        //            0
        //Now cretion of cliques inside model
        if( bClqsNotFilled )
        {
            FillInClqAndDirection( clqNum++, nodeNum, NodeToRight(nodeNum),
                cdHorizontal );

            FillInClqAndDirection( clqNum++, nodeNum, NodeUnder(nodeNum),
                cdVertical);
        }
    }
}

void Construct2MRF2(const string& logfilename, int variant, float sigmahor, float sigmaver)
//Creation of the model-grid like
//like    0-0-0-
//        | | |
//        0-0-0-
//        | | |
{

    const int numOfNds      = m_numOfRows*m_numOfCols;  //the number of nodes
    const int numOfClqs     = m_clqs.size();            //the number of cliques
    

    nodeTypeVector nodeTypes(1);         // type of nodes
    nodeTypes[0].SetType( 1, m_Kolkand );// can get values from 0 to (m_Kolkand-1)
    intVector nodeAssociation( numOfNds, 0 );//all nodes are of the type 0
    //Creation of the model
    m_pGrModel = pnl::CMRF2::Create( numOfNds, nodeTypes, nodeAssociation,
        m_clqs );
    
    //creation of the potentials

    m_pGrModel->AllocFactors();
    floatVector tableData;
    int clqNum = 0;

    for( ; clqNum < numOfClqs; ++clqNum )
    {
            m_pGrModel->AllocFactor(clqNum);

            tableData.resize( (m_Kolkand)*(m_Kolkand)   );
            ComputePotentialMatrixForClq( clqNum, &tableData, logfilename, variant, sigmahor, sigmaver);

            //insert data..
            m_pGrModel->GetFactor(clqNum)->AllocMatrix( &tableData.front(),
            matTable );
    }
}

void GibbsWithAnnealingMRF2( const string& logfilename, int var)
//Function to test Gibbs sampling with annealing simulation 
{   
    ofstream logfile( logfilename.c_str(), ios::app );
    
    CLogger logger(logfile); 
    
    float    GLik=0;    //Likelihood
    float    curTemp;   //current temperature
    int i;
    const int numOfObsNds  = m_pGrModel->GetNumberOfNodes();
    int NumOfNodes;
    
    NumOfNodes = m_pGrModel->GetNumberOfNodes();
    
    int *obsNdsG = NULL;
    float *m_logLik = NULL;
    float logLik;
    
    intVecVector queries(2);
    
    
    valueVector obsNdsValsG(numOfObsNds);   //the list of values for nodes observed
    
    CEvidence *pInitEvidenceG = CreateObservEvidence(m_pGrModel); //To use observation
//    CEvidence *pInitEvidenceG = CreateTestEvidence(m_pGrModel); //To use empty evidence

    stringstream m;
    
    m << endl<<"For GibbsWithAnnealing:=============== NUMR="<< m_numOfRows <<
        " NUMC=" << m_numOfCols << " Kolkand="<< m_Kolkand << " NIter= "<<MaxNumIter << endl;
    logger.PrintToLog( m.str() );
    trsWrite(TW_CON|TW_RUN|TW_DEBUG, "Kolkand=%d\n",m_Kolkand);
    
    CGibbsWithAnnealingInfEngine* pGibbsEng = CGibbsWithAnnealingInfEngine::Create(m_pGrModel);
    MaxNumIter = 20;//m_numOfCols;//10;//200;
    pGibbsEng->SetMaxTime( MaxNumIter);
    pGibbsEng->SetBurnIn( 0 );
    pGibbsEng->SetNumStreams( 1 );
    pGibbsEng->UseAdaptation( true );
    float s = 1.0f;
    float c = 1.f;
    pGibbsEng->SetAnnealingCoefficientS(s);
    pGibbsEng->SetAnnealingCoefficientC(c);
    pGibbsEng->EnterEvidence(pInitEvidenceG, 1);
    obsNdsG = (int *) malloc(numOfObsNds*sizeof(int));
    m_logLik = (float *) malloc(numOfObsNds*sizeof(float));

    stringstream m1;
    m1 << "++++++++++AnnealingCoefficientS: " << s << endl;
    m1 << "++++++++++AnnealingCoefficientC: " << c << endl;
    logger.PrintToLog( m1.str() );
    
    for (int NumIter = 0; NumIter <= 1500; NumIter ++ )
    {
        
        if( NumIter)
        {
            pGibbsEng->Continue( 1 );//pGibbsEng->Continue( 50 );
            curTemp = pGibbsEng->GetCurrentTemp();
            MaxNumIter += 1;
        }
        
        StartInferenceAt = m_numOfRows*10;
        NumInference = NumIter;
        
        //Calculating of most probable evidence by Gibbs
        
        GLik = 0.0f;
        intVector query;
        query.resize(NumOfNodes);
        for( i = 0; i < NumOfNodes; i++ )
        {
            query[i] = i;
        }   
        pGibbsEng->MarginalNodes( &query.front(), query.size() );
        const CEvidence *pEvMPE = pGibbsEng->GetMPE();
        logLik = m_pGrModel->ComputeLogLik(pEvMPE);
        
        for( i = 0; i < NumOfNodes; i++ )
        {
            obsNdsG[i] = pGibbsEng->GetMPE()->GetValue(i)->GetInt();
            
        }
        
        stringstream m2;
        
        m2 << "NumIter "<< MaxNumIter<< " curTemp = " << curTemp<<" LogLik = " << logLik  ;

        if (fabs(logLik-0.0f)<0.01) break;
        if (printModel==1)
        {
            m2 << "vertices: "<< endl;
            int iv, j;
            for (iv = 0; iv<m_numOfRows; iv++) //printing of model MPE values 
            {
                for (j = 0; j<m_numOfCols; j++)
                {
                    m2 << obsNdsG[iv*m_numOfCols+j] << "  ";
                }
                m2<<endl;
            }
        }      
        logger.PrintToLog( m2.str() );
       
        fprintf(stdout, "NumIter=%d LikGibbs =  %f\n", MaxNumIter, logLik);
        //        trsWrite(TW_CON|TW_RUN|TW_DEBUG, "NumIter=%d LikGibbs =  %f\n", MaxNumIter, logLik);        
        
    }
    
    stringstream m2;
    if (printModel==2)
    {
        m2 << "vertices: "<< endl;
        int iv, j;
        for (iv = 0; iv<m_numOfRows; iv++) //printing of model MPE values 
        {
            for (j = 0; j<m_numOfCols; j++)
            {
                m2 << obsNdsG[iv*m_numOfCols+j] << "  ";
            }
            m2<<endl;
        }
    }
    logger.PrintToLog( m2.str() );
    
    delete pGibbsEng;
    free(obsNdsG);
    free(m_logLik);
    delete pInitEvidenceG;
    
}
void GibbsMRF2( const string& logfilename, int var)
//Function to test Gibbs sampling
{   
    ofstream logfile( logfilename.c_str(), ios::app );
    
    CLogger logger(logfile); 
    
    float    GLik=0; //Likelihood
    int i;
    const int numOfObsNds  = m_pGrModel->GetNumberOfNodes();
    int NumOfNodes;
    int j;
    
    intVecVector queries(1);
    int *obsNdsG = NULL;
    float *m_logLik = NULL;
    float logLik;
    
    valueVector obsNdsValsG(numOfObsNds);   //the list of values for nodes observed
    
    CEvidence *pInitEvidenceG = CreateTestEvidence(m_pGrModel); 
    stringstream m;
    
    m << endl<<"For Gibbs:=============== NUMR="<< m_numOfRows <<
        " NUMC=" << m_numOfCols << " Kolkand="<< m_Kolkand << " NIter= "<<MaxNumIter << endl;
    logger.PrintToLog( m.str() );
    //    printf("Kolkand=%d\n",m_Kolkand);
    trsWrite(TW_CON|TW_RUN|TW_DEBUG, "Kolkand=%d\n",m_Kolkand);
    
    
    
    NumOfNodes = m_pGrModel->GetNumberOfNodes();
    
    // Attention! There are different ways to work with Gibbs sampling
    // Try to modify factors StartInferenceAt, NumInference
    //
    MaxNumIter = 1000;
    for (int NumIter = 1; NumIter <= MaxNumIter ; NumIter++ )
    {
        
        int ncliques = m_pGrModel->GetNumberOfCliques();
        intVecVector queryes(ncliques);
        stringstream m1;
        
        //NumInference = StartInferenceAt + NumIter;
        StartInferenceAt = 100;
        NumInference = NumIter;
        
        m1 << "____var= " << var << "__pBarnIn=100 We are search dependence from pMaxTime" << endl;
        m1 << "___StartInferenceAt = " << StartInferenceAt << "  NumInference = "<<NumInference<<"___m_numOfRows"<<m_numOfRows<<"__m_numOfCols" << m_numOfCols << endl;
        logger.PrintToLog( m1.str() );
        
        for( i = 0; i < ncliques; i++ )
        {
            m_pGrModel->GetClique( i, &queryes[i]);
        }
        
        CGibbsSamplingInfEngine* pGibbsEng = CGibbsSamplingInfEngine::Create(m_pGrModel);
        pGibbsEng->SetMaxTime(NumInference);
        pGibbsEng->SetBurnIn(StartInferenceAt);
        
        pGibbsEng->SetQueries(queryes);
        pGibbsEng->EnterEvidence(pInitEvidenceG, 1);
        
        //Calculating of most probable evidence by Gibbs
        
        GLik = 0.0f;
        obsNdsG = (int *) malloc(ncliques*sizeof(int));
        m_logLik = (float *) malloc(ncliques*sizeof(float));
        int num;
        for( i = 0; i < ncliques; i++ )
        {
            pGibbsEng->MarginalNodes( &queryes[i].front(), queryes[i].size() );
            num = queryes[i].front();
            const CEvidence *pEvMPE = pGibbsEng->GetMPE();
            logLik = m_pGrModel->GetFactor(i)->GetLogLik(pEvMPE);
            obsNdsG[num] = pGibbsEng->GetMPE()->GetValue(num)->GetInt();
            m_logLik[i] = logLik;
            GLik +=logLik; 
        }
        stringstream m2;
        m2 << "NUmIter "<< NumIter <<" LogLik = " << GLik << endl;
        if (printModel==1)
        {
	    int iv;
            m2 << "vertices: "<< endl;
            
            for (iv = 0; iv<m_numOfRows; iv++) //printing of model MPE values 
            {
                for (j = 0; j<m_numOfCols; j++)
                {
                    m2 << obsNdsG[iv*m_numOfCols+j] << "  ";
                }
                m2<<endl;
            }
            m2 << "LogLik for clicks "<< endl;
            
            for (iv = 0; iv<m_numOfRows; iv++) //printing of model MPE values 
            {
                for (j = 0; j<m_numOfCols; j++)
                {
                    m2 << m_logLik[iv*m_numOfCols+j] << "  ";
                }
                m2<<endl;
            }
            
            
            logger.PrintToLog( m2.str() );
        }
        //        fprintf(stdout, "NumIter=%d LikGibbs =  %f\n", NumIter, GLik);
        trsWrite(TW_CON|TW_RUN|TW_DEBUG, "NumIter=%d LikGibbs =  %f\n", NumIter, GLik);        
        delete pGibbsEng;
    }
    
    free(obsNdsG);
    free(m_logLik);
    delete pInitEvidenceG;
}

void PearlBigMRF2( const string& logfilename, float sigmahor)
// Function to work with Pearl Inference
{   
    ofstream logfile( logfilename.c_str(), ios::app );
    
    CLogger logger(logfile); 
    
    float  PLik=0; //likelihood
    int    i, j;
    const int numOfObsNds  = m_pGrModel->GetNumberOfNodes();
    int NumOfNodes, n;
    
    int query[] = { 1 };
    int querySize = 1;
    intVecVector queries(2);

    NumOfNodes = m_pGrModel->GetNumberOfNodes();
//    CEvidence *pInitEvidence = CreateObservEvidence(m_pGrModel);//Creation of Evidence with observation
    CEvidence *pInitEvidence = CreateTestEvidence(m_pGrModel);//Creation of empty Evidence
   
    CEvidence *pCalcPEvidence;//the distribution, calculated by Pearl
    stringstream m;
    
    m << endl<<"NUMR="<< m_numOfRows <<
        " NUMC=" << m_numOfCols << " Kolkand="<< m_Kolkand << " NIter= "<<MaxNumIter <<
        "  sigmahor=" << sigmahor << endl;
    logger.PrintToLog( m.str() );
    trsWrite(TW_CON|TW_RUN|TW_DEBUG, "Kolkand=%d\n",m_Kolkand);        

//    MaxNumIter = m_numOfRows*3;

    for(int NumIter = 1; NumIter <= MaxNumIter ; NumIter++ )
    {
        // if PLik is already 0 - stop the calculations
        if (toContinue==0) break;
        fprintf(stdout, "Iteration number %i\n", NumIter);
//        trsWrite(TW_CON|TW_RUN|TW_DEBUG, "Iteration number %i\n", NumIter);        

        CPearlInfEngine* pPearlEng    = CPearlInfEngine::Create(m_pGrModel);
        pPearlEng->SetMaxNumberOfIterations(NumIter);
        pPearlEng->EnterEvidence(pInitEvidence,1,1);
        
        //Calculating of most probability evidence by Pearl
        
        int *obsNds = (int *)malloc(numOfObsNds*sizeof(int));
        valueVector obsNdsVals(numOfObsNds);   //The list of observed nodes

        for( n = 0; n < NumOfNodes; n++ )
        {
            query[0] = n;
            
            pPearlEng->MarginalNodes( query, querySize );//calculate distribution for node number n
            int temp = (int)pPearlEng->GetMPE()->GetValue(n)->GetInt();//get the most probable value
            obsNdsVals[n].SetInt(temp); 
            obsNds[n] = n; 
            
//            trsWrite(TW_CON|TW_RUN|TW_DEBUG, "Node - %i\tValue - %i\n", n, temp);        
            
        }

        // what is the model state with the MP? - printing

        stringstream m1;
        if (printModel==1 || (printModel==2&&NumIter==MaxNumIter-1))
        {
            m1 << "___NumIter = " << NumIter << "___m_numOfRows"<<m_numOfRows<<"__m_numOfCols" << m_numOfCols<< endl;
            logger.PrintToLog( m1.str() );
            
            for (i = 0; i<m_numOfRows; i++) //printing of current MPE values 
            {
                stringstream m2;
                
                for (j = 0; j<m_numOfCols; j++)
                {
                    m2 << obsNdsVals[i*m_numOfRows+j].GetInt()<< "  ";
                }
                logger.PrintToLog( m2.str() );
            }
        }

        delete pPearlEng;
        
        
        //Create the distribution from the most probable values
        //
        pCalcPEvidence = CEvidence::Create( m_pGrModel, numOfObsNds , obsNds, obsNdsVals );
        
        free(obsNds);
        
        try
        {
            // To process exeption which may occur in the moment of ComputeLogLik work
            PLik=m_pGrModel->ComputeLogLik(const_cast<CEvidence *>(pCalcPEvidence));
            fprintf(stdout, "Iteration %d LikPerl =  %f\n", NumIter,PLik/**sigmahor*/);
//            trsWrite(TW_CON|TW_RUN|TW_DEBUG, "Iteration %d LikPerl =  %f\n", NumIter, PLik/**sigmahor*/);        

            stringstream mess3;
            
            mess3  <<NumIter << "  " << PLik*sigmahor << endl;
            logger.PrintToLog( mess3.str() );
            delete pCalcPEvidence;
            
            // To catch the moment when we have converged tj some meaning differs from 0
            if (theFirstIter==1)  
            {
                theFirstIter = 0;
                PreviousLL = PLik;
                IterTheSameLL = 0;
            }
            else
            {
                if (fabs(PLik-PreviousLL)<FLT_EPSILON*2.) IterTheSameLL++;
                else { PreviousLL = PLik; IterTheSameLL = 0;}
            }
            
            if ((fabs(PLik)<0.0001)||(IterTheSameLL==NumIterPorog)) 
            {
                toContinue=0;
//                logger.PrintToLog( mess3.str() );
            }
            else toContinue=1;
        }
        catch(CException& expres)
        {
            const char * cppr = expres.GetMessage();
            stringstream mess3;
            mess3 <<  "00: NumIter "<<NumIter<<endl;
            logger.PrintToLog( mess3.str() );
            mess3<< "   " << cppr <<endl;
            logger.PrintToLog( mess3.str() );
//            fprintf(stderr, "%s\n", cppr);
            trsWrite(TW_CON|TW_RUN|TW_DEBUG, "%s\n", cppr);        
            delete pCalcPEvidence;
        }
    }
            stringstream m1;

    delete pInitEvidence;
}


int testPearlGibbsCompareMRF2()
{
    char *fileOut = "GibbsA17_ProbaPot.txt";
    //Parameters definition
    printPot = 0;
    printModel = 1;
    NumIterPorog = 3;  // the max number of iteration with the same likelihood meaning
    MaxNumIter = 100;

    for (int var = 17; var < 18; var ++) // cycle for different potential functions. See numbers in PotentialFun
    {
        for (int i = 12 ; i < 13; i+=2 ) // the range for model dimensions
        {
            for(m_Kolkand = i; m_Kolkand < i+1;  m_Kolkand += 5/*(i-1)*/)// the range for categories changing
            {
                // Other parameters
                m_numOfRows = i;
                m_numOfCols = i;
                for (int ks = 1; ks < 2/*17*/; ks*=4)
                {
                    toContinue = 1;
                    theFirstIter = 1;
                    m_pGrModel=NULL;
                    //sigmahor = ks/4.f;
                    sigmahor = ks;
                    //sigmahor = 0.5f;
                    sigmaver = ks;
                    
                    // The work itself
                    Construct2Clqs(); // creation of cliques
                    Construct2MRF2(fileOut,var, sigmahor, sigmaver); // Model creation
                    
                    PearlBigMRF2(fileOut, sigmahor); // If you want to use Pearl
                    
                    
                    // GibbsMRF2(fileOut, var); // If you want to use Gibbs 
                    //GibbsWithAnnealingMRF2(fileOut, var); // If you want to use Gibbs 
                    
                    // And now you can compare the results by hands and eyes
                    delete m_pGrModel;
                }
            }
        }
    }

    return 	TRS_OK;

}
void initAPearlGibbsCompareMRF2()
{
    trsReg(func_name, test_desc, test_class, testPearlGibbsCompareMRF2);
}
