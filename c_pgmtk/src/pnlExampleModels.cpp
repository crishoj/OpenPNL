/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlExampleModels.cpp                                        //
//                                                                         //
//  Purpose:   Implementation of methods creating popular Graphical Models //                                                                     //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnlExampleModels.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlRng.hpp"
#include "pnlMNet.hpp"
#include "pnlMRF2.hpp"


PNL_BEGIN

PNL_API CBNet* pnlExCreateWaterSprinklerBNet()
{
    // Creation Water-Sprinkler Bayesian network
    
    const int numOfNds = 4;
    
    // 1 STEP:
    // need to specify the graph structure of the model;
    // there are two way to do it
    
    CGraph *pGraph;
    if(1)
    {
	// Graph creation using adjacency matrix
	
	int numAdjMatDims  = 2;
	
	int ranges[] = { numOfNds, numOfNds };
	
	intVector matrixData( numOfNds*numOfNds, 0 );
	
	CDenseMatrix<int>* adjMat = CDenseMatrix<int>::Create( numAdjMatDims,
	    ranges, &matrixData.front() );
	
	int indices[] = { 0, 1 };
	
	adjMat->SetElementByIndexes( 1, indices );
	
	indices[1] = 2;
	adjMat->SetElementByIndexes( 1, indices );
	
	indices[0] = 1;
	indices[1] = 3;
	adjMat->SetElementByIndexes( 1, indices );
	
	indices[0] = 2;
	adjMat->SetElementByIndexes( 1, indices );
	
	// this is a creation of directed graph for the BNet model based on adjacency matrix
	pGraph = CGraph::Create(adjMat);
	delete adjMat;
    }
    else
    {
	// Graph creation using neighbors list
	
	int numOfNbrs[numOfNds] = { 2, 2, 2, 2 };
	int nbrs0[] = { 1, 2 };
	int nbrs1[] = { 0, 3 };
	int nbrs2[] = { 0, 3 };
	int nbrs3[] = { 1, 2 };
	
	// number of neighbors for every node
	int *nbrs[] = { nbrs0, nbrs1, nbrs2, nbrs3 };
	
	// neighbors can be of either one of the three following types:
	// a parent, a child (for directed arcs) or just a neighbor (for undirected graphs).
	// Accordingly, the types are ntParent, ntChild or ntNeighbor.
	
	ENeighborType nbrsTypes0[] = { ntChild, ntChild };
	ENeighborType nbrsTypes1[] = { ntParent, ntChild };
	ENeighborType nbrsTypes2[] = { ntParent, ntChild };
	ENeighborType nbrsTypes3[] = { ntParent, ntParent };
		
	ENeighborType *nbrsTypes[] = { nbrsTypes0, nbrsTypes1,
	    nbrsTypes2, nbrsTypes3 };
	
	// this is creation of a directed graph for the BNet model using neighbors list
	pGraph = CGraph::Create( numOfNds, numOfNbrs, nbrs, nbrsTypes );
    }
    
    // 2 STEP:
    // Creation NodeType objects and specify node types for all nodes of the model.
   
    nodeTypeVector  nodeTypes;
    
    // number of node types is 1, because all nodes are of the same type
    // all four are discrete and binary
    CNodeType nt(1,2);
    nodeTypes.push_back(nt);

    intVector nodeAssociation;
    // reflects association between node numbers and node types
    // nodeAssociation[k] is a number of node type object in the
    // node types array for the k-th node
    nodeAssociation.assign(numOfNds, 0);
    
    // 2 STEP:
    // Creation base for BNet using Graph, types of nodes and nodes association

    CBNet* pBNet = CBNet::Create( numOfNds, nodeTypes, nodeAssociation, pGraph );
    
    // 3 STEP:
    // Allocation space for all factors of the model
    pBNet->AllocFactors();
    
    // 4 STEP:
    // Creation factors and attach their to model
    
    //create raw data tables for CPDs
    float table0[] = { 0.5f, 0.5f };
    float table1[] = { 0.5f, 0.5f, 0.9f, 0.1f };
    float table2[] = { 0.8f, 0.2f, 0.2f, 0.8f };
    float table3[] = { 1.0f, 0.0f, 0.1f, 0.9f, 0.1f, 0.9f, 0.01f, 0.99f };

    float* table[] = { table0, table1, table2, table3 };

    int i;
    for( i = 0; i < numOfNds; ++i )
    {
        pBNet->AllocFactor(i);

        CFactor* pFactor = pBNet->GetFactor(i);

        pFactor->AllocMatrix( table[i], matTable );
    }

    return pBNet;
}

typedef enum       //Edges direction in the model
{
    cdHorizontal, cdVertical
} EClqDirs;

PNL_API CMRF2* pnlExCreateBigMRF2(int variant, int m_numOfRows, int m_numOfCols, int m_Kolkand,
                                  float sigmahor, float sigmaver)
{
    // MRF2 model for different potentials creation
    // Factors:
    // 
    // variant             variant of potential function (see 'switch')       
    // m_Kolkand           tne number of categories
    // m_numOfRows;                   of rows in the model
    // m_numOfCols;                      columns in the model
    // sigmahor, sigmaver; // potential factors

    intVecVector      m_clqs;         // the list of cliqes
    pnlVector<EClqDirs> m_clqDirection; // their directions

    CMRF2            *m_pGrModel;     // The model - a grid m_numOfRows X m_numOfCols


    const int  numOfNds  = m_numOfRows*m_numOfCols;

    const int  numOfClqs = ( m_numOfRows - 1 )*m_numOfCols
        + ( m_numOfCols - 1 )*m_numOfRows ;

    const bool bNotOneColModel = m_numOfCols == 1 ? false : true,
               bNotOneRowModel = m_numOfRows == 1 ? false : true;
    
    
    m_clqs.resize(numOfClqs, intVector(2) );
    
    m_clqDirection.resize(numOfClqs);
    

    int nodeNum;
    int clqNum = 0;

    for( nodeNum=0; nodeNum < numOfNds ; ++nodeNum )
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
                m_clqs[clqNum][0] = nodeNum;
                m_clqs[clqNum][1] = nodeNum+1;
                m_clqDirection[clqNum] = cdHorizontal;
                clqNum++; 
            }
            
            if( bNotOneRowModel )
            {
                m_clqs[clqNum][0] = nodeNum;
                m_clqs[clqNum][1] = nodeNum+m_numOfCols;
                m_clqDirection[clqNum] = cdVertical;
                clqNum++; 
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
                m_clqs[clqNum][0] = nodeNum;
                m_clqs[clqNum][1] = nodeNum+m_numOfCols;
                m_clqDirection[clqNum] = cdVertical;
                clqNum++; 
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
                m_clqs[clqNum][0] = nodeNum;
                m_clqs[clqNum][1] = nodeNum+1;
                m_clqDirection[clqNum] = cdHorizontal;
                clqNum++; 
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
                m_clqs[clqNum][0] = nodeNum;
                m_clqs[clqNum][1] = nodeNum+1;
                m_clqDirection[clqNum] = cdHorizontal;
                clqNum++; 
            }
            
            m_clqs[clqNum][0] = nodeNum;
            m_clqs[clqNum][1] = nodeNum+m_numOfCols;
            m_clqDirection[clqNum] = cdVertical;
            clqNum++; 
            
            bClqsNotFilled = false;
        }
        
        //right side
        //            0
        //            |
        //            0
        //clique- edge to the left was already created
        if( bClqsNotFilled && ( currColNum == ( m_numOfCols - 1 ) ) )
        {
            m_clqs[clqNum][0] = nodeNum;
            m_clqs[clqNum][1] = nodeNum+m_numOfCols;
            m_clqDirection[clqNum] = cdVertical;
            clqNum++; 
            
            bClqsNotFilled = false;
        }
        
        //upper side
        //            0-0
        //            |
        //            0
        //
        if( bClqsNotFilled && ( currRowNum == 0 ) )
        {
            m_clqs[clqNum][0] = nodeNum;
            m_clqs[clqNum][1] = nodeNum+1;
            m_clqDirection[clqNum] = cdHorizontal;
            clqNum++; 
            
            if( bNotOneRowModel )
            {
                m_clqs[clqNum][0] = nodeNum;
                m_clqs[clqNum][1] = nodeNum+m_numOfCols;
                m_clqDirection[clqNum] = cdVertical;
                clqNum++; 
            }
            
            bClqsNotFilled = false;
        }
        
        //lower side
        //clique like  0-0
        if( bClqsNotFilled && ( currRowNum == ( m_numOfRows - 1 ) ) )
        {
            m_clqs[clqNum][0] = nodeNum;
            m_clqs[clqNum][1] = nodeNum+1;
            m_clqDirection[clqNum] = cdHorizontal;
            clqNum++; 
            
            bClqsNotFilled = false;
        }
        
        //inner part of the matrix
        //            0-0
        //            |
        //            0
        //Now cretion of cliques inside model
        if( bClqsNotFilled )
        {
            m_clqs[clqNum][0] = nodeNum;
            m_clqs[clqNum][1] = nodeNum+1;
            m_clqDirection[clqNum] = cdHorizontal;
            clqNum++; 
            
            m_clqs[clqNum][0] = nodeNum;
            m_clqs[clqNum][1] = nodeNum+m_numOfCols;
            m_clqDirection[clqNum] = cdVertical;
            clqNum++; 
        }
    }
    //Creation of the model-grid like
    //like    0-0-0-
    //        | | |
    //        0-0-0-
    //        | | |
    
    nodeTypeVector nodeTypes(1);         // type of nodes
    nodeTypes[0].SetType( 1, m_Kolkand );// can get values from 0 to (m_Kolkand-1)
    intVector nodeAssociation( numOfNds, 0 );//all nodes are of the type 0
    //Creation of the model
    m_pGrModel = pnl::CMRF2::Create( numOfNds, nodeTypes, nodeAssociation,
        m_clqs );
    
    //creation of the potentials
    
    m_pGrModel->AllocFactors();
    floatVector tableData;

    double value = 0.0;
    double dist, coef;
    double PI = 3.1415926;
    double x, y;
    int    i, j;

    for( clqNum = 0; clqNum < numOfClqs; ++clqNum )
    {
        m_pGrModel->AllocFactor(clqNum);
        
        tableData.resize( (m_Kolkand)*(m_Kolkand)   );
        //        ComputePotentialMatrixForClq( clqNum, &tableData, logfilename, variant, sigmahor, sigmaver);
        for ( i = 0 ; i < m_Kolkand; i++ )
        {
            for ( j = 0; j < m_Kolkand; j++ )
            {
                
                switch ( variant )
                {
                case 0: 
                    if ( m_clqDirection[clqNum] == cdHorizontal )
                        value = (float)exp(- (j - i)*(j - i ) / ( 2 * sigmahor ) );
                    else 
                        value = (float)exp( - (j - i - 1)*(j - i - 1) / ( 2 * sigmaver )) ;
                    break;
                case 1:
                    value = (float)exp(- (j - i)*(j - i ) / ( 2 * sigmahor*sigmahor ) );
                case 2: 
                    value = fabs(0.1+sin(PI*j/m_Kolkand));
                case 3: 
                    if ( m_clqDirection[clqNum] == cdHorizontal )
                        value = fabs(0.1+sin(PI*j/m_Kolkand))*(float)exp(- (j - i)*(j - i ) / ( 2 * sigmahor ) );
                    else if ( m_clqDirection[clqNum] == cdVertical )
                        value = fabs(0.1+sin(PI*j/m_Kolkand))*(float)exp( - (j - i - 1)*(j - i - 1) / ( 2 * sigmaver ) );
                    break;
                case 4: 
                    value = fabs(sin(PI*j/m_Kolkand));
                    break;
                case 5: 
                    if ( m_clqDirection[clqNum] == cdHorizontal )
                        value = fabs(sin(PI*j/m_Kolkand))*(float)exp(- (j - i)*(j - i ) / ( 2 * sigmahor ) );
                    else if ( m_clqDirection[clqNum] == cdVertical )
                        value = fabs(sin(PI*j/m_Kolkand))*(float)exp( - (j - i - 1)*(j - i - 1) / ( 2 * sigmaver ) );
                    break;
                case 6: 
                    value = 2*(i-j)*(i-j)*exp(-(i-j)*(i-j)/2.); // Hermit function for difference: "feedback"  
                    if (value<0.0001) value = 0.0001;
                    break;
                case 7: 
                    value = 2*(i-j)*(i-j)*exp(-(i-j)*(i-j)/2.); // Hermit function for difference: "feedback"
                    if (value<0.0001) value = 0.0001;
                    
                    if ( m_clqDirection[clqNum] == cdHorizontal )
                        value = value*(float)exp(- (j - i)*(j - i ) / ( 2 * sigmahor ) );
                    else if ( m_clqDirection[clqNum] == cdVertical )
                        value = value*(float)exp( - (j - i - 1)*(j - i - 1) / ( 2 * sigmaver ) );
                    break;
                case 8: 
                    if ( m_clqDirection[clqNum] == cdHorizontal )
                        value = 2*(i-j)*(i-j)*exp(-(i-j)*(i-j)/2.); // Hermit function with difference for different directions
                    else 
                        value = 2*(i-j-1)*(i-j-1)*exp(-(i-j-1)*(i-j-1)/2.); 
                    if (value<0.0001) value = 0.0001;
                    break;
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
                    break;
                case 10:
                    x = 4./m_Kolkand * (i - m_Kolkand/2);
                    y = 4./m_Kolkand * (j - m_Kolkand/2);
                    value = 2*1.526*4.2*x*y*exp (-1.5*x*x/2.-1.5*y*y/2.)/PI;
                    if (value<0.0001) value = 0.0001;
                    break;
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
                    break;
                case 12: 
                    if ( m_clqDirection[clqNum] == cdHorizontal ) // here Pearl converges to classic pic.
                        value = 0.98f*(float)exp(-(j - i)*(j - i )/ 2.f)+ //with lines: 0,1,2,...
                        0.99f*(float)exp(-(j-i-3)*(j-i-3 )/ 2.f); 
                    else 
                        value = (float)exp( - (j - i-1 )*(j - i-1 ) / 2.f ) ;
                    if (value>1.0f) value = 1.0;
                    break;
                case 13:
                    if ( m_clqDirection[clqNum] == cdHorizontal )   // for Pearl converges to the columns of 0,1,2
                        value = 0.5f*(float)exp(-(j - i)*(j - i )/ 2.f)+
                        0.55f*(float)exp(-(j-i-1)*(j-i-1 )/ 2.f);
                    else 
                        value = (float)exp( - (j - i )*(j - i ) / 2.f ) ;
                    if (value>1.0f) value = 1.0;
                    break;
                    
                case 14: 
                    if ( m_clqDirection[clqNum] == cdHorizontal )    // the same, so we have convergence to 
                        value = 0.5f*(float)exp(-(j - i)*(j - i )/ 2.f)+ // local maximum
                        0.55f*(float)exp(-(j-i-1)*(j-i-1 )/ 2.f);
                    else 
                        value = (float)exp( - (j - i-1 )*(j - i-1 ) / 2.f ) ;
                    if (value>1.0f) value = 1.0;
                    break;
                    
                case 15: 
                    if ( m_clqDirection[clqNum] == cdHorizontal )
                    {
                        if (i-j==0) value = 0.8f;
                        else
                        {
                            dist = double(((i - j) > 0) ? i - j:j - i);
                            coef = 0.2f*m_Kolkand+0.4f;
                            value = (float)exp(-fabs((coef+0.01)*sin(4.*PI/m_Kolkand*dist))/ 2.f*sigmahor);
                        }
                    }
                    else 
                        value = (float)exp( - (j - i -1 )*(j - i -1 ) / 2.f ) ;
                    break;
                case 16: 
                    //        if ( m_clqDirection[clqNum] == cdHorizontal )    
                    value = 0.991f*(float)exp(-(j - i)*(j - i )/ (2.f*sigmahor))+ 
                        0.992f*(float)exp(-(j-i-4)*(j-i-4 )/ (2.f*sigmahor));
                    //        else 
                    //            value = (float)exp( - (j - i-1)*(j - i-1 ) / 2.f ) ;
                    //        if (value>1.0f) value = 1.0;
                    break;
                case 17:
                    value = (i+1)*(float)exp(- (j - i)*(j - i ) / ( 2.f * sigmahor ) );
                default: value = 1.; break;
                }
                (tableData)[ i*(m_Kolkand)+ j ] = (float) value;
            }
        }
        m_pGrModel->GetFactor(clqNum)->AllocMatrix( &tableData.front(),
            matTable );
    }
    
    return m_pGrModel;    

}


PNL_API CBNet* pnlExCreateAsiaBNet()
{
    // BNet Asia from Cowell book, p.20
    const int nnodes            = 8;
    const int numberOfNodeTypes = 1;

    int i;

    int numOfNeigh[] = { 2, 1, 2, 2, 2, 4, 2, 1 };

    int neigh0[] = { 2, 3 };
    int neigh1[] = { 4 };
    int neigh2[] = { 0, 6 };
    int neigh3[] = { 0, 5 };
    int neigh4[] = { 1, 5};
    int neigh5[] = { 3, 4, 6, 7};
    int neigh6[] = { 2, 5 };
    int neigh7[] = { 5 };

    ENeighborType orient0[] = { ntChild, ntChild };
    ENeighborType orient1[] = { ntChild };
    ENeighborType orient2[] = { ntParent, ntChild };
    ENeighborType orient3[] = { ntParent, ntChild };
    ENeighborType orient4[] = { ntParent, ntChild };
    ENeighborType orient5[] = { ntParent, ntParent, ntChild, ntChild };
    ENeighborType orient6[] = { ntParent, ntParent };
    ENeighborType orient7[] = { ntParent };

    int *neigh[] = { neigh0, neigh1, neigh2, neigh3, neigh4, neigh5,
	neigh6, neigh7 };

    ENeighborType *orient[] = { orient0, orient1, orient2, orient3, orient4,
	orient5, orient6, orient7 };

    CGraph* pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient);

    CNodeType* nodeTypes = new CNodeType [numberOfNodeTypes];

    nodeTypes[0].SetType(1, 2);

    int *nodeAssociation = new int[nnodes];

    for( i = 0; i < nnodes; ++i )
    {
	nodeAssociation[i] = 0;
    }

    CBNet* pBNet = CBNet::Create( nnodes, numberOfNodeTypes, nodeTypes,
	nodeAssociation, pGraph );

    CModelDomain* pMD = pBNet->GetModelDomain();

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

    pBNet->AllocFactors();

    for( i = 0; i < nnodes; ++i )
    {
	myParams[i] = CTabularCPD::Create( domains[i], nodeNumbers[i], pMD );
    }

    // data creation for all CPDs of the model
    float data0[] = {0.5f, 0.5f};
    float data1[] = {0.99f, 0.01f};
    float data2[] = {0.7f, 0.3f, 0.4f, 0.6f};
    float data3[] = {0.99f, 0.01f, 0.9f, 0.1f};
    float data4[] = {0.99f, 0.01f, 0.95f, 0.05f};
    float data5[] = {0.9f, 0.1f, 0.3f, 0.7f, 0.2f, 0.8f, 0.1f, 0.9f};
    float data6[] = {0.9f, 0.1f, 0.3f, 0.7f, 0.2f, 0.8f, 0.1f, 0.9f};
    float data7[] = {0.95f, 0.05f, 0.02f, 0.98f};
    float *data[] = { data0, data1, data2, data3, data4, data5, data6, data7 };

    for( i = 0; i < nnodes; ++i )
    {
	myParams[i]->AllocMatrix(data[i], matTable);

        pBNet->AttachFactor(myParams[i]);
    }

    delete [] nodeTypes;
    delete [] nodeAssociation;
    delete [] nodeNumbers;
    delete [] myParams;

    return pBNet;
}

PNL_API CBNet* pnlExCreateKjaerulfsBNet()
{
    // two slice Kjaerulf model for DBNs
    const int nnodes = 16;
    const int numNt  = 1;

    int i;

    nodeTypeVector nodeTypes( numNt, CNodeType( 1, 2 ) );
    intVector nodeAssociation( 16, 0 );

    int numOfNeigh[] = { 3, 2, 3, 4, 2, 2, 3, 2, 3, 2, 3, 4, 2, 2, 3, 2};

    int neigh0[] = { 1, 2, 8 };
    int neigh1[] = { 0, 2 };
    int neigh2[] = { 0, 1, 3 };
    int neigh3[] = { 2, 4, 5, 11 };
    int neigh4[] = { 3, 6 };
    int neigh5[] = { 3, 6 };
    int neigh6[] = { 4, 5, 7 };
    int neigh7[] = { 6, 15 };

    int neigh8[] =  { 0, 9, 10 };
    int neigh9[] =  { 8, 10};
    int neigh10[] = { 8, 9, 11 };
    int neigh11[] = { 3, 10, 12, 13 };
    int neigh12[] = { 11, 14 };
    int neigh13[] = { 11, 14 };
    int neigh14[] = { 12, 13, 15 };
    int neigh15[] = { 7, 14 };

    ENeighborType orient0[] = { ntChild, ntChild, ntChild };
    ENeighborType orient1[] = { ntParent, ntChild };
    ENeighborType orient2[] = { ntParent, ntParent, ntChild };
    ENeighborType orient3[] = { ntParent, ntChild, ntChild, ntChild };
    ENeighborType orient4[] = { ntParent, ntChild };
    ENeighborType orient5[] = { ntParent, ntChild };
    ENeighborType orient6[] = { ntParent, ntParent, ntChild };
    ENeighborType orient7[] = { ntParent, ntChild };

    ENeighborType orient8[] = {  ntParent, ntChild, ntChild };
    ENeighborType orient9[] = {  ntParent, ntChild };
    ENeighborType orient10[] = { ntParent, ntParent, ntChild };
    ENeighborType orient11[] = { ntParent, ntParent, ntChild, ntChild };
    ENeighborType orient12[] = { ntParent, ntChild };
    ENeighborType orient13[] = { ntParent, ntChild };
    ENeighborType orient14[] = { ntParent, ntParent, ntChild };
    ENeighborType orient15[] = { ntParent, ntParent };

    int *neigh[] = { neigh0, neigh1, neigh2, neigh3, neigh4, neigh5, neigh6,
	neigh7, neigh8, neigh9, neigh10, neigh11, neigh12, neigh13, neigh14,
	neigh15 };

    ENeighborType *orient[] = { orient0, orient1, orient2, orient3, orient4,
	orient5, orient6, orient7, orient8, orient9, orient10, orient11,
	orient12, orient13, orient14, orient15 };

    CGraph* pGraph = CGraph::Create( nnodes, numOfNeigh, neigh,
	orient );

    CBNet* pBNet = CBNet::Create( nnodes, numNt, &nodeTypes.front(),
	&nodeAssociation.front(), pGraph );

    // factor creation part
    CFactor **pParams = new CFactor*[nnodes];

    int domain0[] = { 0 };
    int domain1[] = { 0, 1 };
    int domain2[] = { 0, 1, 2 };
    int domain3[] = { 2, 3 };
    int domain4[] = { 3, 4 };
    int domain5[] = { 3, 5 };
    int domain6[] = { 4, 5, 6 };
    int domain7[] = { 6, 7 };
    int domain8[] = { 0, 8 };
    int domain9[] = { 8, 9 };
    int domain10[] = { 8, 9, 10 };
    int domain11[] = { 3, 10, 11 };
    int domain12[] = { 11, 12 };
    int domain13[] = { 11, 13 };
    int domain14[] = { 12, 13, 14 };
    int domain15[] = { 7, 14, 15 };

    int *domains[] = { domain0, domain1, domain2, domain3, domain4,
	domain5, domain6, domain7, domain8, domain9, domain10, domain11,
	domain12, domain13, domain14, domain15 };

    intVector nodeNumbers(nnodes);
    nodeNumbers[0] = 1;
    nodeNumbers[1] = 2;
    nodeNumbers[2] = 3;
    nodeNumbers[3] = 2;
    nodeNumbers[4] = 2;
    nodeNumbers[5] = 2;
    nodeNumbers[6] = 3;
    nodeNumbers[7] = 2;
    nodeNumbers[8] = 2;
    nodeNumbers[9] = 2;
    nodeNumbers[10] = 3;
    nodeNumbers[11] = 3;
    nodeNumbers[12] = 2;
    nodeNumbers[13] = 2;
    nodeNumbers[14] = 3;
    nodeNumbers[15] = 3;

    pBNet->AllocFactors();

    CModelDomain* pMD = pBNet->GetModelDomain();

    for( i = 0; i < nnodes; ++i )
    {
	pParams[i] = CTabularCPD::Create( domains[i], nodeNumbers[i], pMD );
    }

    // allocation matrices part
    float data0[]  = {0.5f, 0.5f};
    float data1[]  = {0.98f, 0.02f, 0.5f, 0.5f};
    float data2[]  = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.5f, 0.5f};
    float data3[]  = {0.99f, 0.01f, 0.8f, 0.2f};
    float data4[]  = {0.96f, 0.04f, 0.92f, 0.08f};
    float data5[]  = {0.79f, 0.21f, 0.65f, 0.35f};
    float data6[]  = {0.9f, 0.1f, 0.37f, 0.63f, 0.21f, 0.79f, 0.2f, 0.8f};
    float data7[]  = {0.91f, 0.09f, 0.22f, 0.78f};
    float data8[]  = {0.45f, 0.55f, 0.24f, 0.76f};
    float data9[]  = {0.51f, 0.49f, 0.29f, 0.71f};
    float data10[] = {0.98f, 0.02f, 0.4f, 0.6f, 0.2f, 0.8f, 0.5f, 0.5f};
    float data11[] = {0.36f, 0.64f, 0.23f, 0.77f, 0.78f, 0.22f, 0.11f, 0.89f};
    float data12[] = {0.955f, 0.045f, 0.42f, 0.58f};
    float data13[] = {0.57f, 0.43f, 0.09f, 0.91f};
    float data14[] = {0.91f, 0.09f, 0.13f, 0.87f, 0.12f, 0.88f, 0.01f, 0.99f};
    float data15[] = {0.39f, 0.61f, 0.37f, 0.63f, 0.255f, 0.745f, 0.1f, 0.9f};

    float *data[] = { data0, data1, data2, data3, data4, data5, data6,
	data7, data8, data9, data10, data11, data12, data13, data14, data15 };

    for( i = 0; i < nnodes; i++ )
    {
	pParams[i]->AllocMatrix( data[i], matTable );

        pBNet->AttachFactor(pParams[i]);
    }
    delete []pParams;

    return pBNet;
}

PNL_API CBNet* pnlExCreateWasteBNet()
{
    // for details go to Cowell p. 145. model name "waste"
    int i;

    const int nnodes = 9;

    int numOfNeigh[] = { 1, 3, 3, 2, 1, 5, 2, 2, 1 };

    int neigh0[] = { 2 };
    int neigh1[] = { 2, 5, 6 };
    int neigh2[] = { 0, 1, 5  };
    int neigh3[] = { 4, 5 };
    int neigh4[] = { 3 };
    int neigh5[] = { 1, 2, 3, 7, 8 };
    int neigh6[] = { 1, 7 };
    int neigh7[] = { 5, 6 };
    int neigh8[] = { 5 };

    ENeighborType orient0[] = { ntChild };
    ENeighborType orient1[] = { ntChild, ntChild, ntChild };
    ENeighborType orient2[] = { ntParent, ntParent, ntChild };
    ENeighborType orient3[] = { ntChild, ntChild };
    ENeighborType orient4[] = { ntParent };
    ENeighborType orient5[] = { ntParent, ntParent, ntParent, ntChild, ntChild };
    ENeighborType orient6[] = { ntParent, ntChild };
    ENeighborType orient7[] = { ntParent, ntParent };
    ENeighborType orient8[] = { ntParent };

    int *neigh[] = { neigh0, neigh1, neigh2, neigh3, neigh4, neigh5, neigh6,
	neigh7, neigh8 };
    ENeighborType *orient[] = { orient0, orient1, orient2, orient3, orient4,
        orient5, orient6, orient7, orient8 };

    CGraph* pWasteGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );

    int numNodeTypes = 2;
    CNodeType* ndTypes = new CNodeType[numNodeTypes];
    ndTypes[0].SetType( 1, 2 );
    ndTypes[1].SetType( 0, 1 );

    intVector nodeAssociation;
    nodeAssociation.assign( nnodes, 1 );
    nodeAssociation[0] = 0;
    nodeAssociation[1] = 0;
    nodeAssociation[3] = 0;

    float table0[] = { 0.95f, 0.05f };// intact - defective
    float table1[] = { 0.28571428571428571428571428571429f, 0.71428571428571428571428571428571f };
    // industrial - household
    float table3[] = { 0.85f, 0.15f };// stable - unstable

    float mean2w01 = -3.2f; float cov2w01 = 0.00002f;
    float mean2w11 = -0.5f; float cov2w11 = 0.0001f;
    float mean2w00 = -3.9f; float cov2w00 = 0.00002f;
    float mean2w10 = -0.4f; float cov2w10 = 0.0001f;

    float mean4w0 = -2.0f; float cov4w0 = 0.1f;
    float mean4w1 = -1.0f; float cov4w1 = 0.3f;

    float mean5w00 = 6.5f; float cov5w00 = 0.03f; float weight5 = 1.0f;
    float mean5w01 = 7.5f; float cov5w01 = 0.1f;
    float mean5w10 = 6.0f; float cov5w10 = 0.04f;
    float mean5w11 = 7.0f; float cov5w11 = 0.1f;

    float mean6w0 = 0.5f; float cov6w0 = 0.01f;
    float mean6w1 = -0.5f; float cov6w1 = 0.005f;

    float mean7 = 0.0f; float cov7 = 0.002f;
    float weight7p5 = 1.0f; float weight7p6 = 1.0f;

    float mean8 = 3.0f; float cov8 = 0.25f; float weight8 = -0.5f;

    CBNet* pWasteBNet = CBNet::Create( nnodes, numNodeTypes, ndTypes,
        &nodeAssociation.front(), pWasteGraph );

    CModelDomain* pMD = pWasteBNet->GetModelDomain();

    pWasteBNet->AllocFactors();

    CCPD** params = new CCPD *[nnodes];

    pConstNodeTypeVector ntDomain;
    ntDomain.reserve(nnodes);

    intVector domain;
    domain.reserve(nnodes);

    for( i = 0; i < nnodes; ++i )
    {
        pWasteGraph->GetParents( i, &domain );

        domain.push_back( i );

        if( i == 0 || i == 1 || i == 3 )
        {
            params[i] = CTabularCPD::Create( &domain.front(), domain.size(), pMD );
        }
        else
        {
            params[i] = CGaussianCPD::Create( &domain.front(), domain.size(), pMD );
        }

        domain.clear();
        ntDomain.clear();
    }

    //allocate tabular matrix for tabular nodes 0, 1
    params[0]->AllocMatrix( table0, matTable );
    params[1]->AllocMatrix( table1, matTable );
    //need to allocate conditional Gaussian distribution for node 2 & 5
    intVector discrParentVals;
    discrParentVals.assign(2, 0);
    params[2]->AllocMatrix( &mean2w00, matMean, -1, &discrParentVals.front() );
    params[2]->AllocMatrix( &cov2w00, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &mean5w00, matMean, -1, &discrParentVals.front() );
    params[5]->AllocMatrix( &cov5w00, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &weight5, matWeights, 0, &discrParentVals.front() );
    discrParentVals[1] = 1;
    params[2]->AllocMatrix( &mean2w01, matMean, -1, &discrParentVals.front() );
    params[2]->AllocMatrix( &cov2w01, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &mean5w01, matMean, -1, &discrParentVals.front() );
    params[5]->AllocMatrix( &cov5w01, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &weight5, matWeights, 0, &discrParentVals.front() );
    discrParentVals[0] = 1;
    params[2]->AllocMatrix( &mean2w11, matMean, -1, &discrParentVals.front() );
    params[2]->AllocMatrix( &cov2w11, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &mean5w11, matMean, -1, &discrParentVals.front() );
    params[5]->AllocMatrix( &cov5w11, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &weight5, matWeights, 0, &discrParentVals.front() );
    discrParentVals[1] = 0;
    params[2]->AllocMatrix( &mean2w10, matMean, -1, &discrParentVals.front() );
    params[2]->AllocMatrix( &cov2w10, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &mean5w10, matMean, -1, &discrParentVals.front() );
    params[5]->AllocMatrix( &cov5w10, matCovariance, -1,
        &discrParentVals.front() );
    params[5]->AllocMatrix( &weight5, matWeights, 0, &discrParentVals.front() );
    //tabular matrix for node 3
    params[3]->AllocMatrix( table3, matTable );
    //conditional Gaussian distribution for node 4 & 6
    int parentVal = 0;
    params[4]->AllocMatrix( &mean4w0, matMean, -1, &parentVal );
    params[4]->AllocMatrix( &cov4w0, matCovariance, -1, &parentVal );
    params[6]->AllocMatrix( &mean6w0, matMean, -1, &parentVal );
    params[6]->AllocMatrix( &cov6w0, matCovariance, -1, &parentVal );
    parentVal = 1;
    params[4]->AllocMatrix( &mean4w1, matMean, -1, &parentVal );
    params[4]->AllocMatrix( &cov4w1, matCovariance, -1, &parentVal );
    params[6]->AllocMatrix( &mean6w1, matMean, -1, &parentVal );
    params[6]->AllocMatrix( &cov6w1, matCovariance, -1, &parentVal );
    //Gaussian distribution for node 7
    params[7]->AllocMatrix( &mean7, matMean );
    params[7]->AllocMatrix( &cov7, matCovariance);
    params[7]->AllocMatrix( &weight7p5, matWeights, 0 );
    params[7]->AllocMatrix( &weight7p6, matWeights, 1 );
    //Gaussian distribution for node 8
    params[8]->AllocMatrix( &mean8, matMean );
    params[8]->AllocMatrix( &cov8, matCovariance );
    params[8]->AllocMatrix( &weight8, matWeights, 0 );

    for( i = 0; i < nnodes; ++i )
    {
        pWasteBNet->AttachFactor(params[i]);
    }

    delete [] ndTypes;
    delete [] params;

    return pWasteBNet;
}

PNL_API CBNet* pnlExCreateWaterSprinklerSparseBNet()
{
    // first need to specify the graph structure of the model;
    const int numOfNds = 4;
    int numOfNbrs[numOfNds] = { 2, 2, 2, 2 };

    int nbrs0[] = { 1, 2 };
    int nbrs1[] = { 0, 3 };
    int nbrs2[] = { 0, 3 };
    int nbrs3[] = { 1, 2 };

    // neighbors can be of either one of three following types:
    // a parent, a child or just a neighbor - for undirected graphs.
    // if a neighbor of a node is it's parent, then neighbor type is ntParent
    // if it's a child, then ntChild and if it's a neighbor, then ntNeighbor
    ENeighborType nbrsTypes0[] = { ntChild,  ntChild };
    ENeighborType nbrsTypes1[] = { ntParent, ntChild };
    ENeighborType nbrsTypes2[] = { ntParent, ntChild };
    ENeighborType nbrsTypes3[] = { ntParent, ntParent };

    int *nbrs[] = { nbrs0, nbrs1, nbrs2, nbrs3 };
    ENeighborType *nbrsTypes[] = { nbrsTypes0, nbrsTypes1, nbrsTypes2,
	nbrsTypes3 };

    // next step - create NodeType objects and specify node types for
    // all nodes of the model;
    const int numOfNdTypes = 1;
    // number of node types is 1, because all nodes are of the same type
    // all four are discrete and binary

    CNodeType *nodeTypes = new CNodeType [numOfNdTypes];
    int *nodeAssociation = new int [numOfNds];
    // reflects association between node numbers and node types
    // nodeAssociation[k] is a number of node type object in the
    // node types array for the kth node

    nodeTypes[0].SetType(1, 2);// node type - discrete and binary

    int i;

    for( i = 0; i < numOfNds; ++i )
    {
	nodeAssociation[i] = 0;
	// means that all nodes are of the same node type,
	// which is 0th one in the array of node types for the model
    }

    CBNet *pBNet;
    CGraph *pDGraph;

    // this is a creation of directed graph for the BNet model
    pDGraph = CGraph::Create( numOfNds, numOfNbrs, nbrs, nbrsTypes );

    pBNet = CBNet::Create( numOfNds, numOfNdTypes, nodeTypes,
	nodeAssociation, pDGraph );

    CModelDomain* pMD = pBNet->GetModelDomain();
    //create raw data tables
    float table0[] = { 0.5f, 0.5f };
    float table1[] = { 0.5f, 0.5f, 0.9f, 0.1f };
    float table2[] = { 0.8f, 0.2f, 0.2f, 0.8f };
    float table3[] = { 1.0f, 0.0f, 0.1f, 0.9f, 0.1f, 0.9f, 0.01f, 0.99f };

    //allocate factors
    pBNet->AllocFactors();

    //create storage for CPD
    CCPD** cpds = new CCPD *[numOfNds];

    intVector domain;
    domain.reserve(numOfNds);

    pConstNodeTypeVector nt;
    nt.reserve(numOfNds);
    //create sparse matrices
    CNumericSparseMatrix<float>** matrices = new
        CNumericSparseMatrix<float>*[numOfNds];

    intVector ranges;
    ranges.assign( 3, 2 );

    intVector indices;
    indices.assign( 3, 0 );

    matrices[0] = CNumericSparseMatrix<float>::Create( 1, &ranges.front(), 0 );
    matrices[0]->SetElementByIndexes( table0[0], &indices.front() );
    indices[0] = 1;
    matrices[0]->SetElementByIndexes( table0[1], &indices.front() );
    indices[0] = 0;

    matrices[1] = CNumericSparseMatrix<float>::Create( 2, &ranges.front(), 0 );
    matrices[1]->SetElementByIndexes( table1[0], &indices.front() );
    indices[1] = 1;
    matrices[1]->SetElementByIndexes( table1[1], &indices.front() );
    indices[1] = 0;
    indices[0] = 1;
    matrices[1]->SetElementByIndexes( table1[2], &indices.front() );
    indices[1] = 1;
    matrices[1]->SetElementByIndexes( table1[3], &indices.front() );
    indices[0] = 0;
    indices[1] = 0;

    matrices[2] = CNumericSparseMatrix<float>::Create( 2, &ranges.front(), 0 );
    matrices[2]->SetElementByIndexes( table2[0], &indices.front() );
    indices[1] = 1;
    matrices[2]->SetElementByIndexes( table2[1], &indices.front() );
    indices[1] = 0;
    indices[0] = 1;
    matrices[2]->SetElementByIndexes( table2[2], &indices.front() );
    indices[1] = 1;
    matrices[2]->SetElementByIndexes( table2[3], &indices.front() );
    indices[0] = 0;
    indices[1] = 0;

    matrices[3] = CNumericSparseMatrix<float>::Create( 3, &ranges.front(), 0 );
    matrices[3]->SetElementByIndexes( table3[0], &indices.front() );
    indices[2] = 1;
    matrices[3]->SetElementByIndexes( table3[1], &indices.front() );
    indices[2] = 0;
    indices[1] = 1;
    matrices[3]->SetElementByIndexes( table3[2], &indices.front() );
    indices[2] = 1;
    matrices[3]->SetElementByIndexes( table3[3], &indices.front() );
    indices[2] = 0;
    indices[1] = 0;
    indices[0] = 1;
    matrices[3]->SetElementByIndexes( table3[4], &indices.front() );
    indices[2] = 1;
    matrices[3]->SetElementByIndexes( table3[5], &indices.front() );
    indices[2] = 0;
    indices[1] = 1;
    matrices[3]->SetElementByIndexes( table3[6], &indices.front() );
    indices[2] = 1;
    matrices[3]->SetElementByIndexes( table3[7], &indices.front() );

    //create, alloc and attach factors
    for( i = 0; i < numOfNds; i++ )
    {
        pDGraph->GetParents( i, &domain );
        domain.push_back( i );
	cpds[i] = CTabularCPD::Create( &domain.front(), domain.size(), pMD );
        cpds[i]->AttachMatrix(matrices[i], matTable);
	cpds[i]->NormalizeCPD();
        pBNet->AttachFactor( cpds[i] );
        domain.clear();
        nt.clear();
    }

    delete [] nodeAssociation;
    delete [] nodeTypes;
    delete [] cpds;

    return pBNet;
}

PNL_API CBNet* pnlExCreateSimpleGauMix()
{
    //the model is
    //  X(0)  V(1)
    //    \    |
    //     \   V
    //  Y(2)->Z(3)->W(4)
    //  X, V are Tabular nodes, Z - gaussain mixture node, Y,Z - gaussian
    // V is mixture node


    // first need to specify the graph structure of the model;
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

    const int numOfNdTypes = 2;
    // number of node types is 1, because all nodes are of the same type
    // all four are discrete and binary

    CNodeType *nodeTypes = new CNodeType [numOfNdTypes];
    int *nodeAssociation = new int [numOfNds];
    nodeTypes[0].SetType( 1, 2 );
    nodeTypes[1].SetType( 0, 1 );
    nodeAssociation[0] = 0;
    nodeAssociation[1] = 0;
    nodeAssociation[2] = 1;
    nodeAssociation[3] = 1;
    nodeAssociation[4] = 1;


    CGraph*	pDGraph = CGraph::Create( numOfNds, numOfNbrs, nbrs, nbrsTypes );

    CBNet* pBNet = CBNet::Create( numOfNds, numOfNdTypes, nodeTypes,
	nodeAssociation, pDGraph );

    CModelDomain* pMD = pBNet->GetModelDomain();

    //set factors
    pBNet->AllocFactors();
    //create arrays of data for factors
    float table0[] = { 0.7f, 0.3f };//node X
    float table1[] = { 0.4f, 0.6f };//node V
    float mean0 = 0.0f, cov0 = 1.0f;//node Y
    float mean100 =   1.0f, cov100 = 0.005f, weight100 = 0.02f;//node Z for X = 0, V = 0
    float mean110 =  -5.0f, cov110 = 0.01f, weight110 = 0.01f;//node Z for X = 1, V = 0
    float mean101 =  -3.0f, cov101 = 0.01f, weight101 = 0.01f;//node Z for X = 0, V = 1
    float mean111 =   2.0f, cov111 = 0.002f, weight111 = 0.05f;//node Z for X = 1, V = 1
    float mean2 = 8.0f, cov2 = 0.01f, weight2 = 0.01f;//for node W

    //create domains
    int *nodeNumbers = new int [numOfNds];
    int domain0[] = { 0 };
    int domain1[] = { 1 };
    int domain2[] = { 2 };
    int domain3[] = { 0, 1, 2, 3 };
    int domain4[] = { 3, 4 };
    int *domains[] = { domain0, domain1, domain2, domain3, domain4 };
    nodeNumbers[0] = 1;
    nodeNumbers[1] = 1;
    nodeNumbers[2] = 1;
    nodeNumbers[3] = 4;
    nodeNumbers[4] = 2;

    CTabularCPD* myCPD0 = CTabularCPD::Create( domains[0], nodeNumbers[0], pMD, table0 );
    pBNet->AttachFactor( myCPD0 );
    CTabularCPD* myCPD00 = CTabularCPD::Create( domains[1], nodeNumbers[1], pMD, table1 );
    pBNet->AttachFactor( myCPD00 );
    CGaussianCPD* myCPD1 = CGaussianCPD::Create( domains[2], nodeNumbers[2], pMD );
    myCPD1->AllocDistribution( &mean0, &cov0, 1.0f, NULL );
    pBNet->AttachFactor( myCPD1 );
    CMixtureGaussianCPD* myCPD2 = CMixtureGaussianCPD::Create( domains[3],
        nodeNumbers[3], pMD, table1 );
    intVector parVal;
    parVal.assign( 2, 0 );
    float* t = &weight100;
    myCPD2->AllocDistribution( &mean100, &cov100, 2.0f, &t, &parVal.front() );
    parVal[1] = 1;
    t = &weight101;
    myCPD2->AllocDistribution( &mean101, &cov101, 1.0f, &t, &parVal.front());
    parVal[0] = 1;
    t = &weight111;
    myCPD2->AllocDistribution( &mean111, &cov111, 1.0f, &t, &parVal.front() );
    parVal[1] = 0;
    t = &weight110;
    myCPD2->AllocDistribution( &mean110, &cov110, 1.0f, &t, &parVal.front() );
    pBNet->AttachFactor( myCPD2 );
    CGaussianCPD* myCPD3 = CGaussianCPD::Create( domains[4], nodeNumbers[4], pMD );
    t = &weight2;
    myCPD3->AllocDistribution( &mean2, &cov2, 0.5f, &t );
    pBNet->AttachFactor( myCPD3 );

    delete []nodeAssociation;
    delete []nodeTypes;
    delete []nodeNumbers;

    return pBNet;
}

PNL_API CBNet* pnlExCreateVerySimpleGauMix()
{
    //the model is
    //         X(0)
    //         |
    //         V
    //  Y(1)->Z(2)->W(3)
    //  X is Tabular nodes, Z - gaussain mixture node, Y,Z - gaussian
    //  V is mixture node


    // first need to specify the graph structure of the model;
    const int numOfNds = 4;
    int numOfNbrs[numOfNds] = { 1, 1, 3, 1 };

    int nbrs0[] = { 2 };
    int nbrs1[] = { 2 };
    int nbrs2[] = { 0, 1, 3 };
    int nbrs3[] = { 2 };

    // neighbors can be of either one of three following types:
    // a parent, a child or just a neighbor - for undirected graphs.
    // if a neighbor of a node is it's parent, then neighbor type is ntParent
    // if it's a child, then ntChild and if it's a neighbor, then ntNeighbor
    ENeighborType nbrsTypes0[] = { ntChild };
    ENeighborType nbrsTypes1[] = { ntChild };
    ENeighborType nbrsTypes2[] = { ntParent, ntParent, ntChild };
    ENeighborType nbrsTypes3[] = { ntParent };
    int *nbrs[] = { nbrs0, nbrs1, nbrs2, nbrs3 };
    ENeighborType *nbrsTypes[] = { nbrsTypes0, nbrsTypes1, nbrsTypes2,
	nbrsTypes3 };

    const int numOfNdTypes = 2;
    // number of node types is 1, because all nodes are of the same type
    // all four are discrete and binary

    CNodeType *nodeTypes = new CNodeType [numOfNdTypes];
    int *nodeAssociation = new int [numOfNds];
    nodeTypes[0].SetType( 1, 2 );
    nodeTypes[1].SetType( 0, 1 );
    nodeAssociation[0] = 0;
    nodeAssociation[1] = 1;
    nodeAssociation[2] = 1;
    nodeAssociation[3] = 1;


    CGraph*	pDGraph = CGraph::Create( numOfNds, numOfNbrs, nbrs, nbrsTypes );

    CBNet* pBNet = CBNet::Create( numOfNds, numOfNdTypes, nodeTypes,
	nodeAssociation, pDGraph );
    CModelDomain* pMD = pBNet->GetModelDomain();

    //set factors
    pBNet->AllocFactors();
    //create arrays of data for factors
    float table0[] = { 0.7f, 0.3f };//node X
    float mean0 = 0.0f, cov0 = 1.0f;//node Y
    float mean00 = 1.0f, cov00 = 0.5f, weight00 = 2.0f;//node Z for X = 0
    float mean10 = -5.0f, cov10 = 4.0f, weight10 = 0.5f;//node Z for X = 1
    float mean2 = 8.0f, cov2 = 0.01f, weight2 = 3.0f;//for node W

    //create domains
    int *nodeNumbers = new int [numOfNds];
    int domain0[] = { 0 };
    int domain1[] = { 1 };
    int domain2[] = { 0, 1, 2 };
    int domain3[] = { 2, 3 };
    int *domains[] = { domain0, domain1, domain2, domain3 };
    nodeNumbers[0] = 1;
    nodeNumbers[1] = 1;
    nodeNumbers[2] = 3;
    nodeNumbers[3] = 2;

    CTabularCPD* myCPD0 = CTabularCPD::Create( domains[0], nodeNumbers[0], pMD, table0 );
    pBNet->AttachFactor( myCPD0 );
    CGaussianCPD* myCPD1 = CGaussianCPD::Create( domains[1], nodeNumbers[1], pMD );
    myCPD1->AllocDistribution( &mean0, &cov0, 1.0f, NULL );
    pBNet->AttachFactor( myCPD1 );
    CMixtureGaussianCPD* myCPD2 = CMixtureGaussianCPD::Create( domains[2],
        nodeNumbers[2], pMD, table0 );
    int parVal = 0;
    float* t = &weight00;
    myCPD2->AllocDistribution( &mean00, &cov00, 2.0f, &t, &parVal );
    parVal = 1;
    t = &weight10;
    myCPD2->AllocDistribution( &mean10, &cov10, 1.0f, &t, &parVal);
    pBNet->AttachFactor( myCPD2 );
    CGaussianCPD* myCPD3 = CGaussianCPD::Create( domains[3], nodeNumbers[3], pMD );
    t = &weight2;
    myCPD3->AllocDistribution( &mean2, &cov2, 0.5f, &t );
    pBNet->AttachFactor( myCPD3 );

    delete [] nodeAssociation;
    delete [] nodeTypes;

    return pBNet;
}

PNL_API CBNet* pnlExCreateCondGaussArBNet()
{
/*
Make an HMM with autoregressive gaussian observations
X0 ->X1
|     |
v     v
Y0 ->Y1
    */

    //Create static model
    const int nnodes = 4;//Number of nodes
    int numNt = 2;//number of Node types (all nodes are discrete)
    CNodeType *nodeTypes = new CNodeType [numNt];
    nodeTypes[0] = CNodeType( 1,2 );
    nodeTypes[1] = CNodeType( 0,1 );
    int nodeAssociation[] = { 0, 1, 0, 1 };

    //Create a dag
    int numOfNeigh[] = { 2, 2, 2, 2 };
    int neigh0[] = { 1, 2 };
    int neigh1[] = { 0, 3 };
    int neigh2[] = { 0, 3 };
    int neigh3[] = { 1, 2 };

    ENeighborType orient0[] = { ntChild, ntChild };
    ENeighborType orient1[] = { ntParent, ntChild };
    ENeighborType orient2[] = { ntParent, ntChild };
    ENeighborType orient3[] = { ntParent, ntParent };

    int *neigh[] = { neigh0, neigh1, neigh2, neigh3 };
    ENeighborType *orient[] = { orient0, orient1, orient2, orient3 };

    CGraph* pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient );

    //Create static BNet
    CBNet *pBNet =
        CBNet::Create( nnodes, numNt, nodeTypes, nodeAssociation, pGraph );
    pBNet->AllocFactors();

    //Let arbitrary distribution is
    float tableNode0[] = { 0.7f, 0.3f };
    float tableNode2[] = { 0.2f, 0.8f, 0.3f, 0.7f };

    float mean1w0 = -3.2f; float cov1w0 = 0.2f;
    float mean1w1 = -0.5f; float cov1w1 = 0.1f;

    float mean3w0 = 6.5f; float cov3w0 = 0.3f; float weight3w0 = 1.0f;
    float mean3w1 = 7.5f; float cov3w1 = 0.4f; float weight3w1 = 0.5f;


    pBNet->AllocFactor( 0 );
    pBNet->GetFactor(0)->AllocMatrix( tableNode0, matTable );

    pBNet->AllocFactor( 2 );
    pBNet->GetFactor(2)->AllocMatrix( tableNode2, matTable );

    int parent[] = { 0 };

    pBNet->AllocFactor( 1 );
    pBNet->GetFactor(1)->AllocMatrix( &mean1w0, matMean, -1, parent );
    pBNet->GetFactor(1)->AllocMatrix( &cov1w0, matCovariance, -1, parent );

    parent[0] = 1;

    pBNet->GetFactor(1)->AllocMatrix( &mean1w1, matMean, -1, parent );
    pBNet->GetFactor(1)->AllocMatrix( &cov1w1, matCovariance, -1, parent );

    parent[0] = 0;
    pBNet->AllocFactor(3);
    pBNet->GetFactor(3)->AllocMatrix( &mean3w0, matMean, -1, parent );
    pBNet->GetFactor(3)->AllocMatrix( &cov3w0, matCovariance, -1, parent );
    pBNet->GetFactor(3)->AllocMatrix( &weight3w0, matWeights, 0, parent );

    parent[0] = 1;

    pBNet->GetFactor(3)->AllocMatrix( &mean3w1, matMean,-1, parent );
    pBNet->GetFactor(3)->AllocMatrix( &cov3w1, matCovariance,-1, parent );
    pBNet->GetFactor(3)->AllocMatrix( &weight3w1, matWeights, 0, parent );

    delete []nodeTypes;

   return pBNet;
}

PNL_API CBNet* pnlExCreateBNetFormSMILE()
{
    const int nnodes = 37;
    const int numberOfNodeTypes = 37;

    int i;

    CGraph* pGraph = CGraph::Create( nnodes, NULL );

    pGraph->AddEdge(0,1,1);
    pGraph->AddEdge(0,3,1);
    pGraph->AddEdge(2,3,1);
    pGraph->AddEdge(3,4,1);
    pGraph->AddEdge(3,5,1);
    pGraph->AddEdge(0,6,1);
    pGraph->AddEdge(2,6,1);
    pGraph->AddEdge(11,12,1);
    pGraph->AddEdge(10,13,1);
    pGraph->AddEdge(12,13,1);
    pGraph->AddEdge(8,14,1);
    pGraph->AddEdge(9,14,1);
    pGraph->AddEdge(13,14,1);
    pGraph->AddEdge(8,15,1);
    pGraph->AddEdge(14,15,1);
    pGraph->AddEdge(15,16,1);
    pGraph->AddEdge(15,19,1);
    pGraph->AddEdge(18,19,1);
    pGraph->AddEdge(8,21,1);
    pGraph->AddEdge(20,21,1);
    pGraph->AddEdge(19,22,1);
    pGraph->AddEdge(21,22,1);
    pGraph->AddEdge(23,24,1);
    pGraph->AddEdge(16,25,1);
    pGraph->AddEdge(17,25,1);
    pGraph->AddEdge(22,25,1);
    pGraph->AddEdge(24,25,1);
    pGraph->AddEdge(25,26,1);
    pGraph->AddEdge(7,27,1);
    pGraph->AddEdge(26,27,1);
    pGraph->AddEdge(26,29,1);
    pGraph->AddEdge(28,29,1);
    pGraph->AddEdge(26,30,1);
    pGraph->AddEdge(28,30,1);
    pGraph->AddEdge(14,31,1);
    pGraph->AddEdge(16,31,1);
    pGraph->AddEdge(8,32,1);
    pGraph->AddEdge(14,32,1);
    pGraph->AddEdge(20,33,1);
    pGraph->AddEdge(8,34,1);
    pGraph->AddEdge(9,34,1);
    pGraph->AddEdge(13,34,1);
    pGraph->AddEdge(6,35,1);
    pGraph->AddEdge(26,35,1);
    pGraph->AddEdge(24,36,1);
    pGraph->AddEdge(35,36,1);

    CNodeType *nodeTypes = new CNodeType [37];

    nodeTypes[0].SetType(1, 2);
    nodeTypes[1].SetType(1, 2);
    nodeTypes[2].SetType(1, 2);
    nodeTypes[3].SetType(1, 3);
    nodeTypes[4].SetType(1, 3);
    nodeTypes[5].SetType(1, 3);
    nodeTypes[6].SetType(1, 3);
    nodeTypes[7].SetType(1, 2);
    nodeTypes[8].SetType(1, 3);
    nodeTypes[9].SetType(1, 2);
    nodeTypes[10].SetType(1, 2);
    nodeTypes[11].SetType(1, 3);
    nodeTypes[12].SetType(1, 4);
    nodeTypes[13].SetType(1, 4);
    nodeTypes[14].SetType(1, 4);
    nodeTypes[15].SetType(1, 4);
    nodeTypes[16].SetType(1, 3);
    nodeTypes[17].SetType(1, 2);
    nodeTypes[18].SetType(1, 2);
    nodeTypes[19].SetType(1, 3);
    nodeTypes[20].SetType(1, 2);
    nodeTypes[21].SetType(1, 2);
    nodeTypes[22].SetType(1, 3);
    nodeTypes[23].SetType(1, 2);
    nodeTypes[24].SetType(1, 3);
    nodeTypes[25].SetType(1, 2);
    nodeTypes[26].SetType(1, 3);
    nodeTypes[27].SetType(1, 3);
    nodeTypes[28].SetType(1, 2);
    nodeTypes[29].SetType(1, 3);
    nodeTypes[30].SetType(1, 3);
    nodeTypes[31].SetType(1, 4);
    nodeTypes[32].SetType(1, 4);
    nodeTypes[33].SetType(1, 3);
    nodeTypes[34].SetType(1, 4);
    nodeTypes[35].SetType(1, 3);
    nodeTypes[36].SetType(1, 3);

    int *nodeAssociation = new int[nnodes];

    for ( i = 0; i < nnodes; i++ )
    {
        nodeAssociation[i] = i;
    }
    CBNet *pBNet = CBNet::Create( nnodes, numberOfNodeTypes, nodeTypes,
        nodeAssociation, pGraph );

    CModelDomain* pMD = pBNet->GetModelDomain();

    //number of factors is the same as number of nodes - one CPD per node
    CFactor **myParams = new CFactor*[nnodes];
    int *nodeNumbers = new int [nnodes];

    int domain0[] = { 0 };
    nodeNumbers[0] =  1;
    int domain1[] = { 0, 1 };
    nodeNumbers[1] =  2;
    int domain2[] = { 2 };
    nodeNumbers[2] =  1;
    int domain3[] = { 0, 2, 3 };
    nodeNumbers[3] =  3;
    int domain4[] = { 3, 4 };
    nodeNumbers[4] =  2;
    int domain5[] = { 3, 5 };
    nodeNumbers[5] =  2;
    int domain6[] = { 0, 2, 6 };
    nodeNumbers[6] =  3;
    int domain7[] = { 7 };
    nodeNumbers[7] =  1;
    int domain8[] = { 8 };
    nodeNumbers[8] =  1;
    int domain9[] = { 9 };
    nodeNumbers[9] =  1;
    int domain10[] = { 10 };
    nodeNumbers[10] =  1;
    int domain11[] = { 11 };
    nodeNumbers[11] =  1;
    int domain12[] = { 11, 12 };
    nodeNumbers[12] =  2;
    int domain13[] = { 10, 12, 13 };
    nodeNumbers[13] =  3;
    int domain14[] = { 8, 9, 13, 14 };
    nodeNumbers[14] =  4;
    int domain15[] = { 8, 14, 15 };
    nodeNumbers[15] =  3;
    int domain16[] = { 15, 16 };
    nodeNumbers[16] =  2;
    int domain17[] = { 17 };
    nodeNumbers[17] =  1;
    int domain18[] = { 18 };
    nodeNumbers[18] =  1;
    int domain19[] = { 15, 18, 19 };
    nodeNumbers[19] =  3;
    int domain20[] = { 20 };
    nodeNumbers[20] =  1;
    int domain21[] = { 8, 20, 21 };
    nodeNumbers[21] =  3;
    int domain22[] = { 19, 21, 22 };
    nodeNumbers[22] =  3;
    int domain23[] = { 23 };
    nodeNumbers[23] =  1;
    int domain24[] = { 23, 24 };
    nodeNumbers[24] =  2;
    int domain25[] = { 16, 17, 22, 24, 25 };
    nodeNumbers[25] =  5;
    int domain26[] = { 25, 26 };
    nodeNumbers[26] =  2;
    int domain27[] = { 7, 26, 27 };
    nodeNumbers[27] =  3;
    int domain28[] = { 28 };
    nodeNumbers[28] =  1;
    int domain29[] = { 26, 28, 29 };
    nodeNumbers[29] =  3;
    int domain30[] = { 26, 28, 30 };
    nodeNumbers[30] =  3;
    int domain31[] = { 14, 16, 31 };
    nodeNumbers[31] =  3;
    int domain32[] = { 8, 14, 32 };
    nodeNumbers[32] =  3;
    int domain33[] = { 20, 33 };
    nodeNumbers[33] =  2;
    int domain34[] = { 8, 9, 13, 34 };
    nodeNumbers[34] =  4;
    int domain35[] = { 6, 26, 35 };
    nodeNumbers[35] =  3;
    int domain36[] = { 24, 35, 36 };
    nodeNumbers[36] =  3;

    int *domains[] = { domain0, domain1, domain2, domain3, domain4,
        domain5, domain6, domain7, domain8, domain9,
        domain10, domain11, domain12, domain13, domain14,
        domain15, domain16, domain17, domain18, domain19,
        domain20, domain21, domain22, domain23, domain24,
        domain25, domain26, domain27, domain28, domain29,
        domain30, domain31, domain32, domain33, domain34,
        domain35, domain36};

    pBNet->AllocFactors();

    for( i = 0; i < nnodes; i++ )
    {
        myParams[i] = CTabularCPD::Create( domains[i], nodeNumbers[i], pMD);
    }

    // data creation for all CPDs of the model
    float data0[] = {0.05f, 0.95f};
    float data1[] = {0.9f, 0.01f, 0.1f, 0.99f};
    float data2[] = {0.2f, 0.8f};
    float data3[] = {0.95f, 0.01f, 0.98f, 0.05f, 0.04f, 0.09f, 0.01f, 0.9f, 0.01f, 0.9f, 0.01f, 0.05f};
    float data4[] = {0.95f, 0.04f, 0.01f, 0.04f, 0.95f, 0.29f, 0.01f, 0.01f, 0.7f};
    float data5[] = {0.95f, 0.04f, 0.01f, 0.04f, 0.95f, 0.04f, 0.01f, 0.01f, 0.95f};
    float data6[] = {0.98f, 0.5f, 0.95f, 0.05f, 0.01f, 0.49f, 0.04f, 0.9f, 0.01f, 0.01f, 0.01f, 0.05f};
    float data7[] = {0.05f, 0.95f};
    float data8[] = {0.92f, 0.03f, 0.05f};
    float data9[] = {0.04f, 0.96f};
    float data10[] = {0.1f, 0.9f};
    float data11[] = {0.05f, 0.9f, 0.05f};
    float data12[] = {0.05f, 0.05f, 0.05f, 0.93f, 0.01f, 0.01f, 0.01f, 0.93f, 0.01f, 0.01f, 0.01f, 0.93f};
    float data13[] = {0.97f, 0.97f, 0.97f, 0.97f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f};
    float data14[] = {0.97f, 0.95f, 0.4f, 0.3f, 0.97f, 0.97f, 0.97f, 0.97f, 0.97f, 0.95f, 0.5f, 0.3f, 0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.97f, 0.97f, 0.97f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.03f, 0.58f, 0.68f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.03f, 0.48f, 0.68f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f};
    float data15[] = {0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.03f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.95f, 0.94f, 0.88f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.04f, 0.1f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f};
    float data16[] = {0.01f, 0.01f, 0.04f, 0.9f, 0.01f, 0.01f, 0.92f, 0.09f, 0.98f, 0.98f, 0.04f, 0.01f};
    float data17[] = {0.1f, 0.9f};
    float data18[] = {0.05f, 0.95f};
    float data19[] = {1.0f, 1.0f, 0.99f, 0.95f, 0.95f, 0.01f, 0.95f, 0.01f, 0.0f, 0.0f, 0.01f, 0.04f, 0.04f, 0.95f, 0.04f, 0.01f, 0.0f, 0.0f, 0.0f, 0.01f, 0.01f, 0.04f, 0.01f, 0.98f};
    float data20[] = {0.01f, 0.99f};
    float data21[] = {0.1f, 0.1f, 0.01f, 0.95f, 0.95f, 0.05f, 0.9f, 0.9f, 0.99f, 0.05f, 0.05f, 0.95f};
    float data22[] = {0.98f, 0.01f, 0.01f, 0.98f, 0.98f, 0.69f, 0.01f, 0.98f, 0.01f, 0.01f, 0.01f, 0.3f, 0.01f, 0.01f, 0.98f, 0.01f, 0.01f, 0.01f};
    float data23[] = {0.01f, 0.99f};
    float data24[] = {0.98f, 0.3f, 0.01f, 0.4f, 0.01f, 0.3f};
    float data25[] = {0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.05f, 0.05f, 0.01f, 0.01f, 0.01f, 0.01f, 0.05f, 0.05f, 0.01f, 0.05f, 0.05f, 0.01f, 0.05f, 0.05f, 0.01f, 0.05f, 0.05f, 0.01f, 0.05f, 0.05f, 0.01f, 0.7f, 0.7f, 0.1f, 0.7f, 0.7f, 0.1f, 0.7f, 0.7f, 0.1f, 0.95f, 0.99f, 0.3f, 0.95f, 0.99f, 0.3f, 0.95f, 0.99f, 0.3f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.99f, 0.95f, 0.95f, 0.99f, 0.99f, 0.99f, 0.99f, 0.95f, 0.95f, 0.99f, 0.95f, 0.95f, 0.99f, 0.95f, 0.95f, 0.99f, 0.95f, 0.95f, 0.99f, 0.95f, 0.95f, 0.99f, 0.3f, 0.3f, 0.9f, 0.3f, 0.3f, 0.9f, 0.3f, 0.3f, 0.9f, 0.05f, 0.01f, 0.7f, 0.05f, 0.01f, 0.7f, 0.05f, 0.01f, 0.7f};
    float data26[] = {0.05f, 0.01f, 0.9f, 0.09f, 0.05f, 0.9f};
    float data27[] = {0.98f, 0.4f, 0.3f, 0.98f, 0.01f, 0.01f, 0.01f, 0.59f, 0.4f, 0.01f, 0.98f, 0.01f, 0.01f, 0.01f, 0.3f, 0.01f, 0.01f, 0.98f};
    float data28[] = {0.1f, 0.9f};
    float data29[] = {0.333333f, 0.98f, 0.333333f, 0.01f, 0.333333f, 0.01f, 0.333333f, 0.01f, 0.333333f, 0.98f, 0.333333f, 0.01f, 0.333333f, 0.01f, 0.333333f, 0.01f, 0.333333f, 0.98f};
    float data30[] = {0.333333f, 0.98f, 0.333333f, 0.01f, 0.333333f, 0.01f, 0.333333f, 0.01f, 0.333333f, 0.98f, 0.333333f, 0.01f, 0.333333f, 0.01f, 0.333333f, 0.01f, 0.333333f, 0.98f};
    float data31[] = {0.97f, 0.97f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.97f};
    float data32[] = {0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.6f, 0.5f, 0.5f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.38f, 0.48f, 0.48f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.97f};
    float data33[] = {0.01f, 0.05f, 0.19f, 0.9f, 0.8f, 0.05f};
    float data34[] = {0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.1f, 0.05f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.4f, 0.2f, 0.2f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.3f, 0.01f, 0.01f, 0.01f, 0.84f, 0.25f, 0.15f, 0.01f, 0.29f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.58f, 0.75f, 0.7f, 0.01f, 0.9f, 0.01f, 0.01f, 0.01f, 0.49f, 0.08f, 0.01f, 0.01f, 0.05f, 0.25f, 0.25f, 0.01f, 0.3f, 0.08f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.04f, 0.09f, 0.01f, 0.08f, 0.38f, 0.01f, 0.01f, 0.2f, 0.9f, 0.97f, 0.01f, 0.01f, 0.45f, 0.59f, 0.01f, 0.4f, 0.9f, 0.97f, 0.01f, 0.01f, 0.01f, 0.97f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.6f, 0.97f};
    float data35[] = {0.98f, 0.95f, 0.3f, 0.95f, 0.04f, 0.01f, 0.8f, 0.01f, 0.01f, 0.01f, 0.04f, 0.69f, 0.04f, 0.95f, 0.3f, 0.19f, 0.04f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.69f, 0.01f, 0.95f, 0.98f};
    float data36[] = {0.98f, 0.98f, 0.3f, 0.98f, 0.1f, 0.05f, 0.9f, 0.05f, 0.01f, 0.01f, 0.01f, 0.6f, 0.01f, 0.85f, 0.4f, 0.09f, 0.2f, 0.09f, 0.01f, 0.01f, 0.1f, 0.01f, 0.05f, 0.55f, 0.01f, 0.75f, 0.9f};

    float *datas[] = { data0, data1, data2, data3, data4, data5, data6, data7,
        data8, data9, data10, data11, data12, data13, data14, data15, data16,
        data17, data18, data19, data20, data21, data22, data23, data24,
        data25, data26, data27, data28, data29, data30, data31, data32,
        data33, data34, data35, data36 };

    for( i = 0; i < nnodes; i++ )
    {
        myParams[i]->AllocMatrix(datas[i], matTable);
	static_cast<CCPD*>(myParams[i])->NormalizeCPD();
        pBNet->AttachFactor(myParams[i]);
    }

    delete [] nodeTypes;
    delete [] nodeAssociation;
    delete [] myParams;
    delete [] nodeNumbers;

    return pBNet;
}

PNL_API CBNet* pnlExCreateSingleGauMix()
{
    //the model is

    //  X(0)->Y(1)
    //  Y - gaussain mixture node

    // first need to specify the graph structure of the model;
    const int numOfNds = 2;
    int numOfNbrs[numOfNds] = { 1, 1 };

    int nbrs0[] = { 1 };
    int nbrs1[] = { 0 };

    // neighbors can be of either one of three following types:
    // a parent, a child or just a neighbor - for undirected graphs.
    // if a neighbor of a node is it's parent, then neighbor type is ntParent
    // if it's a child, then ntChild and if it's a neighbor, then ntNeighbor
    ENeighborType nbrsTypes0[] = { ntChild };

    ENeighborType nbrsTypes1[] = { ntParent };
    int *nbrs[] = { nbrs0, nbrs1 };
    ENeighborType *nbrsTypes[] = { nbrsTypes0, nbrsTypes1 };

    const int numOfNdTypes = 2;
    // number of node types is 1, because all nodes are of the same type
    // all four are discrete and binary

    CNodeType *nodeTypes = new CNodeType [numOfNdTypes];
    int *nodeAssociation = new int [numOfNds];
    nodeTypes[0].SetType( 1, 2 );
    nodeTypes[1].SetType( 0, 1 );
    nodeAssociation[0] = 0;
    nodeAssociation[1] = 1;

    CGraph*	pGraph = CGraph::Create( numOfNds, numOfNbrs, nbrs, nbrsTypes );

    CBNet* pBNet = CBNet::Create( numOfNds, numOfNdTypes, nodeTypes,
	nodeAssociation, pGraph );
    CModelDomain* pMD = pBNet->GetModelDomain();

    //set factors
    pBNet->AllocFactors();
    //create arrays of data for factors
    float table0[] = { 0.7f, 0.3f };//node X

    float mean00 = -1.0f, cov00 = 0.02f;//node Y for X = 0
    float mean10 =  5.0f, cov10 = 0.01f;//node Y for X = 1

    //create domains
    int *nodeNumbers = new int [numOfNds];
    int domain0[] = { 0 };
    int domain1[] = { 0, 1 };

    int *domains[] = { domain0, domain1 };
    nodeNumbers[0] = 1;
    nodeNumbers[1] = 2;


    CTabularCPD* myCPD0 = CTabularCPD::Create( domains[0], nodeNumbers[0], pMD, table0 );
    pBNet->AttachFactor( myCPD0 );


    CMixtureGaussianCPD* myCPD2 = CMixtureGaussianCPD::Create( domains[1],
        nodeNumbers[1], pMD, table0 );

    int parVal = 0;

    myCPD2->AllocDistribution( &mean00, &cov00, 2.0f, NULL, &parVal );

    parVal = 1;

    myCPD2->AllocDistribution( &mean10, &cov10, 1.0f, NULL, &parVal);
    pBNet->AttachFactor( myCPD2 );

    delete []nodeAssociation;
    delete []nodeTypes;
    delete []nodeNumbers;

    return pBNet;
}

PNL_API CBNet* pnlExCreateBatNetwork( int seed )
{

    int dim = 2;
    int nnodes = 56;
    int ranges[] = { nnodes, nnodes };

    /*
    int line[] =
        {

         0,     0,     2,     2,     2,     6,     7,     6,     9,     6,     9,    11,     3,
        12,    11,     6,    15,     5,    16,    16,    20,    20,     5,    22,    24,    26,
        28,    28,    30,    30,     6,    30,    34,    35,    34,    37,    34,    37,    39,
        31,    40,    39,    13,    15,    20,    16,    34,    43,    33,    44,    18,    44,
        19,    22,    13,    19,    20,    24,    26,    48,    22,    48,    33,    50,    24,
        52,    26,    54
        };


        int column[] =
        {
         1,     3,     3,     4,     7,     7,     8,     9,    10,    11,    12,    12,    13,
        13,    14,    16,    16,    17,    17,    18,    21,    22,    23,    23,    25,    27,
        29,    31,    31,    32,    34,    35,    35,    36,    37,    38,    39,    40,    40,
        41,    41,    42,    43,    43,    43,    44,    44,    44,    45,    45,    46,    46,
        47,    47,    48,    48,    48,    48,    48,    49,    50,    50,    51,    51,    52,
        53,    54,    55
        };
        */
    int line[] =
    {

     0, 0, 0, 0,     0,     2,     2,     2,     6,     7,     6,     9,     6,     9,    11,     3,
    12,    11,     6,    15,     5,    16,    16,    20,    20,     5,    22,    24,    26,
    28,    28,    30,    30,     6,    30,    34,    35,    34,    37,    34,    37,    39,
    31,    40,    39,    13,    15,    20,    16,    34,    43,    33,    44,    18,    44,
    19,    22,    13,    19,    20,    24,    26,    48,    22,    48,    33,    50,    24,
    52,    26,    54
    };


    int column[] =
    {
     24, 26, 19, 1,     3,     3,     4,     7,     7,     8,     9,    10,    11,    12,    12,    13,
    13,    14,    16,    16,    17,    17,    18,    21,    22,    23,    23,    25,    27,
    29,    31,    31,    32,    34,    35,    35,    36,    37,    38,    39,    40,    40,
    41,    41,    42,    43,    43,    43,    44,    44,    44,    45,    45,    46,    46,
    47,    47,    48,    48,    48,    48,    48,    49,    50,    50,    51,    51,    52,
    53,    54,    55
    };

/*
    int nNonZero = 68;
*/
    int nNonZero = 71;

    CSparseMatrix<int> *pAdjMat;
    pAdjMat = CSparseMatrix<int>:: Create( dim, ranges, 0 );
    int ind[2];
    int i;
    for( i = 0; i < nNonZero; i++ )
    {
        ind[0] = line[i];
        ind[1] = column[i];
        pAdjMat->SetElementByIndexes(1, ind);
    }

    /*
    CDenseMatrix<int> *pTmp = pAdjMat->ConvertToDense();
        CGraph *pGraph = CGraph::Create(pTmp);*/
    CGraph *pGraph = CGraph::Create(pAdjMat);

    intVecVector decomp;
    pGraph->GetConnectivityComponents(&decomp);

    delete pAdjMat;

    int ndSz = 2;
    CNodeType nt(1, ndSz);
    nodeTypeVector nodeTypes( 1, nt);
    intVector nodeAssociation( nnodes, 0 );
    CBNet *pBNet = CBNet::Create( nnodes, nodeTypes,nodeAssociation,pGraph );

    pBNet->AllocFactors();

    intVector domain;
    floatVector data;
    CTabularCPD *pCPD;
    srand(seed);

    for( i = 0; i < nnodes; i++ )
    {
        domain.clear();
        pGraph->GetParents(i, &domain);
        domain.push_back(i);
        pCPD = CTabularCPD::Create( &domain.front(), domain.size(), pBNet->GetModelDomain() );

        int dataSz =  ndSz;

	for(int j = pGraph->GetNumberOfParents( i ); --j >= 0; dataSz *= ndSz);

	// here dataSz = ndSz**(pGraph->GetNumberOfParents( i ) + 1)
        data.resize( dataSz );
        generate(data.begin(), data.end(), rand);

        pCPD->AllocMatrix( &data.front(), matTable);
        pCPD->NormalizeCPD();

        pBNet->AttachFactor( pCPD );
    }

    return pBNet;
}

PNL_API CBNet* pnlExCreateScalarGaussianBNet()
{
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
    ////////////////////////////////////////////////////////////////////////

    CBNet *pBnet;
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

    pBnet = CBNet::Create( nnodes, numnt, nodeTypes,
	&nodeAssociation.front(),graph );
    pBnet->AllocFactors();

    int i;
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
    CNumericDenseMatrix<float> *mean0 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    bigData[0] = 4.0f;

    CNumericDenseMatrix<float> *cov0 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(),&bigData.front());
    pBnet->GetFactor(0)->AttachMatrix(mean0, matMean);
    pBnet->GetFactor(0)->AttachMatrix(cov0, matCovariance);

    float val = 1.0f;

    CNumericDenseMatrix<float> *mean1 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    CNumericDenseMatrix<float> *cov1 =
	CNumericDenseMatrix<float>::Create( 2, &ranges.front(), &val );
    pBnet->GetFactor(1)->AttachMatrix(mean1, matMean);
    pBnet->GetFactor(1)->AttachMatrix(cov1, matCovariance);
    smData[0] = 0.0f;

    CNumericDenseMatrix<float> *mean2 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    smData[0] = 2.0f;

    CNumericDenseMatrix<float> *w21 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    bigData[0] = 2.0f;

    CNumericDenseMatrix<float> *cov2 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(),&bigData.front());
    bigData[0] = 1.0f;

    CNumericDenseMatrix<float> *w20 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(),&bigData.front());
    pBnet->GetFactor(2)->AttachMatrix( mean2, matMean );
    pBnet->GetFactor(2)->AttachMatrix( cov2, matCovariance );
    pBnet->GetFactor(2)->AttachMatrix( w20, matWeights,0 );
    pBnet->GetFactor(2)->AttachMatrix( w21, matWeights,1 );

    val = 0.0f;

    CNumericDenseMatrix<float> *mean3 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);
    val = 4.0f;
    CNumericDenseMatrix<float> *cov3 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(), &val);

    smData[0] = 1.1f;

    CNumericDenseMatrix<float> *w30 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(), &smData.front());
    pBnet->GetFactor(3)->AttachMatrix( mean3, matMean );
    pBnet->GetFactor(3)->AttachMatrix( cov3, matCovariance );
    pBnet->GetFactor(3)->AttachMatrix( w30, matWeights,0 );

    smData[0] = -0.8f;

    CNumericDenseMatrix<float> *mean4 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(),&smData.front());

    bigData[0] = 1.2f;

    CNumericDenseMatrix<float> *cov4 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(),&bigData.front());
    bigData[0] = 2.0f;

    CNumericDenseMatrix<float> *w40 =
	CNumericDenseMatrix<float>::Create(2, &ranges.front(),&bigData.front());
    pBnet->GetFactor(4)->AttachMatrix( mean4, matMean );
    pBnet->GetFactor(4)->AttachMatrix( cov4, matCovariance );
    pBnet->GetFactor(4)->AttachMatrix( w40, matWeights,0 );
    delete []nodeTypes;

    return pBnet;
}

PNL_API CBNet* pnlExCreateRndArHMM()
{
/*
Make Bayesian Network which defines a HMM with autoregressive discrete observations
X0 ->X1
|     |
v     v
Y0 ->Y1

all nodes are discrete and binary    
*/

    //Create static model
    const int nnodes = 4;//Number of nodes
    
    // 1) First need to specify the graph structure of the model;
    int numOfNeigh[] = {2, 2, 2, 2};
    int neigh0[] = {1, 2};
    int neigh1[] = {0, 3};
    int neigh2[] = {0, 3};
    int neigh3[] = {1, 2};

    ENeighborType orient0[] = { ntChild, ntChild };
    ENeighborType orient1[] = { ntParent, ntChild };
    ENeighborType orient2[] = { ntParent, ntChild };
    ENeighborType orient3[] = { ntParent, ntParent };

    int *neigh[] = { neigh0, neigh1, neigh2, neigh3 };
    ENeighborType *orient[] = { orient0, orient1, orient2, orient3 };

    
    CGraph* pGraph = CGraph::Create( nnodes, numOfNeigh, neigh, orient);
    
    // 2) Creation of the Model Domain.

    nodeTypeVector variableTypes;
    const int numNt = 1;//number of Node types (all nodes are discrete)
    variableTypes.resize(numNt);
    variableTypes[0].SetType(1, 2);

    intVector variableAssociation;
    variableAssociation.assign(nnodes, 0);
   
    CModelDomain *pMD;
    pMD = CModelDomain::Create( variableTypes, variableAssociation );


    // 3) Creation static BNet with random matrices
    CBNet *pBNet = CBNet::CreateWithRandomMatrices( pGraph, pMD );
  
    return pBNet;
}

PNL_API CMNet* pnlCreateSmallDiscreteMNet()
{
    const int numOfNds       = 4;
    const int numOfNodeTypes = 1;
    const int numOfClqs      = 4;

    intVector clqSizes( numOfClqs, 2 );

    int clq0[] = { 0, 1 };
    int clq1[] = { 1, 2 };
    int clq2[] = { 2, 3 };
    int clq3[] = { 3, 0 };

    const int *clqs[] = { clq0, clq1, clq2, clq3 };

    CNodeType nodeType( 1, 2 );

    intVector nodeAssociation( numOfNds, 0 );

    CMNet* pMNet = CMNet::Create( numOfNds, numOfNodeTypes,
        &nodeType, &nodeAssociation.front(), numOfClqs, &clqSizes.front(),
        clqs );

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

        pMNet->GetFactor(i)->AllocMatrix( data[i], pnl::matTable );
    }

    return pMNet;
}

PNL_API CBNet* pnlExCreateRandomBNet( int nnodes, int numEdges, int nContNds, 
				      int maxSzDiscrNds,
				      int maxSzContNds,
                                      float edge_probability,
                                      bool disconnected_ok )
{
    PNL_CHECK_LEFT_BORDER(nnodes, 1);
    PNL_CHECK_LEFT_BORDER(numEdges, 1);
    
    PNL_CHECK_RANGES( nContNds, 0, nnodes );
    PNL_CHECK_LEFT_BORDER(maxSzDiscrNds, 2);
    PNL_CHECK_LEFT_BORDER(maxSzContNds, 1);

    int dim = 2;
    int ranges[] = {nnodes, nnodes};
    CSparseMatrix<int> *pAdjMat;
    intVector nodesToConnect;
    
    int ind[2];
    int i;
        
    intVector contNds;
    contNds.resize(nContNds);
    
    intVector discrNds;
    discrNds.resize(nnodes-nContNds);
    
    
    nodeTypeVector nodeTypes;
    CNodeType nt(true, 1);
    nodeTypes.assign(nnodes, nt);
    int nDiscrNds = nnodes - nContNds;
    nodeTypes.assign(nnodes, nt);
    contNds.resize(nContNds);
    discrNds.resize(nDiscrNds);
	   
    intVector allNds(nnodes, 0);
    for( i = 0; i < nnodes; i++ )
    {
	allNds[i] = i;
    }

newSample:
    pAdjMat = CSparseMatrix<int>:: Create( dim, ranges, 0 );
    std::random_shuffle( allNds.begin(), allNds.end() );
    for( i = 0; i < nContNds; i++ )
    {
	contNds[i] = allNds[i];
	nodeTypes[contNds[i]].SetType(false, pnlRand( 1, maxSzContNds ));
    }
    for(; i < nnodes; i++ )
    {
	discrNds[i-nContNds] = allNds[i];
	nodeTypes[discrNds[i-nContNds]].SetType(true, pnlRand( 2, maxSzDiscrNds ));
    }
	   
    std::sort( discrNds.begin(), discrNds.end() );
    
    if( nContNds )
    {
	int x = pnlRand(1, 10 );
	int maxnGr = x > nContNds ? nContNds : x;
	int nGroups = pnlRand(1, maxnGr);
	intVecVector groups;
	groups.resize(nGroups);
	
	for( i = 0; i < nContNds; i++ )
	{
	    (groups[pnlRand(0, nGroups - 1)]).push_back(contNds[i]);
	    
	}
	for( i = 0; i < nGroups; i++ )
	{
	    int grSz = groups[i].size();
	    if( grSz > 1 )
	    {
		
		std::sort( groups[i].begin(), groups[i].end() );
		intVector toConnect;
		int j;
		for( j = 0; j < grSz - 1; j++ )
		{
		    toConnect.assign(groups[i].begin()+j+1, groups[i].end()); 
		    
		    std::random_shuffle(toConnect.begin(), toConnect.end());
		    int x = pnlRand( 1, numEdges );
		    
		    int nEdges = x < toConnect.size() ? x : toConnect.size();
		   
		    ind[0] = (groups[i])[j];
		    int k;
		    for( k = 0; k < nEdges; k++ )
		    {
                        if ( pnlRand( 0.f, 1.f ) <= edge_probability )
                        {
			    ind[1] = toConnect[k];
                            pAdjMat->SetElementByIndexes(1, ind);
                        }
		    }
		}
		
	    }
	}
    }
    if( nDiscrNds)
    {
	intVector ndsToConnect;
	for( i = 0; i < discrNds.size() - 1; i++ )
	{
	    ind[0] = discrNds[i];
	    ndsToConnect.assign( discrNds.begin()+i+1, discrNds.end() );
	    std::random_shuffle( ndsToConnect.begin(), ndsToConnect.end() );
	    int x = pnlRand( 1, numEdges );
	    
	    int nEdges = x < ndsToConnect.size() ? x : ndsToConnect.size();
	    int j;
	    for( j = 0; j < nEdges; j++ )
	    {
                if ( pnlRand( 0.f, 1.f ) <= edge_probability )
                {
                    ind[1] = ndsToConnect[j];
                    pAdjMat->SetElementByIndexes(1, ind);
                }
	    }
	    
	}
    }
    
    CGraph *pGraph = CGraph::Create(pAdjMat);
    delete pAdjMat;
    
    intVecVector components;
    pGraph->GetConnectivityComponents(&components);
    
    int numConComp = components.size();
    int c1, c2;
    
repeat:
    c1 = 0;
    c2 = c1 + 1;
    while( !disconnected_ok && numConComp > 1 )
    {
	bool repeate = true;
	int i1 = 0;
	int i2 = 0;
	while (repeate) 
	{
	    int node1, node2;
	    
	    node1 =(components[c1])[i1];
	    node2 =(components[c2])[i2];
	    
	    
	    int statNode, endNode;
	    if( node1 < node2)
	    {
		statNode = node1; endNode = node2;
	    }
	    else
	    {
		statNode = node2; endNode = node1;
	    }
	    bool stType = nodeTypes[statNode].IsDiscrete();
	    bool endType = nodeTypes[endNode].IsDiscrete();
	    
	    if( stType || !endType )
	    {
	
		pGraph->AddEdge(statNode, endNode, 1 );
		
		--numConComp;
		repeate = false;
	    }
	    else
	    {
		if( i2 == int(components[c2].size()) - 1)
		{
		    i2 = 0;
		    if( i1 == int(components[c1].size()) -1 )
		    {
			repeate = false;
			if( components.size() == 2)
			{
			    delete pGraph;
			    goto newSample;
			}

		    }
		    else
		    {
			++i1;
		    }
		}
		else
		{
		    ++i2;
		}
	
		
	    }
	    
	}
	
	if( c2 == int(components.size()) - 1)
	{
	    if( c1 == int(components.size()) - 1)
	    {
		components.clear(); 
		pGraph->GetConnectivityComponents(&components);
		numConComp = components.size();
		c1 = 0;
		c2 = c1 + 1;
	    }
	    else
	    {
		++c1;
		if( c1 == c2 )
		{
		    components.clear(); 
		    pGraph->GetConnectivityComponents(&components);
		    numConComp = components.size();
		    c1 = 0;
		    c2 = c1 + 1;
		}
	    }
	}
	else
	{
	    ++c2;
	}
	
    }
    
    if( !disconnected_ok )
    {
	components.clear(); 
	pGraph->GetConnectivityComponents(&components);
	numConComp = components.size();
	if( numConComp > 1)
	{
	    goto repeat;
	}
    }
    intVector nodeAssociation( nnodes, 0 );
    for( i = 0; i < nnodes; i++ )
    {
	nodeAssociation[i] = i;
    }
    CBNet *pBNet = CBNet::Create( nnodes, nodeTypes,nodeAssociation,pGraph );
    pBNet->AllocFactors();
    for( i = 0; i < nnodes; i++ )
    {
	pBNet->AllocFactor(i);
	pBNet->GetFactor(i)->CreateAllNecessaryMatrices();
    }
    return pBNet;

}

PNL_API CDBN* pnlExCreateRandomDBN(int nnodesPerSlice, int numEdges, int nInterfNds, 
				   int nContNdsPerSlice, 
				   int maxSzDiscrNds,
				   int maxSzContNds)
{
    
    PNL_CHECK_LEFT_BORDER(nnodesPerSlice, 1);
    PNL_CHECK_LEFT_BORDER(numEdges, 1);
    PNL_CHECK_RANGES( nContNdsPerSlice, 0, nnodesPerSlice );
    PNL_CHECK_RANGES( nInterfNds, 1, nnodesPerSlice );
    PNL_CHECK_LEFT_BORDER(maxSzDiscrNds, 2);
    PNL_CHECK_LEFT_BORDER(maxSzContNds, 1);
    
    
    CGraph *pGraph = NULL;
    CBNet *pBNet = NULL;
    
    
    CBNet *pBNet0 = pnlExCreateRandomBNet( nnodesPerSlice, numEdges, nContNdsPerSlice, 
	maxSzDiscrNds, maxSzDiscrNds );
    if( ! pBNet0 )
    {
	return NULL;
    }
    const CModelDomain *pMD0 = pBNet0->GetModelDomain();
    const CGraph *pGraph0 = pBNet0->GetGraph();
    
    int nnodes0 = pGraph0->GetNumberOfNodes();
    
    intVecVector nbrs;
    neighborTypeVecVector nbrsTypes;
    nbrs.resize(2*nnodes0);
    nbrsTypes.resize(2*nnodes0);
    
    int i; 
    for( i = 0; i < nnodes0; i++ )
    {
    	pGraph0->GetNeighbors( i, &(nbrs[i]), &(nbrsTypes[i]));
	pGraph0->GetNeighbors( i, &(nbrs[i + nnodes0]), &(nbrsTypes[i +nnodes0]));
	int j;
	for( j = 0; j < nbrs[i + nnodes0].size(); j++ )
	{
	    (nbrs[i+nnodes0])[j] += nnodes0;
	}
    }

    nodeTypeVector nts;
    pMD0->GetVariableTypes(&nts);
    intVector nodeAss;
    nodeAss.resize(2*nnodes0);

    const int * nodeAss0 = pMD0->GetVariableAssociations();
  
   
    memcpy( &nodeAss.front(), nodeAss0, nnodes0*sizeof(int));
    memcpy( &nodeAss[nnodes0], nodeAss0, nnodes0*sizeof(int));
    delete pBNet0;
    pGraph = CGraph::Create( nbrs, nbrsTypes ); 
    
    intVector nds(nnodes0, 0);
    for( i = 0; i < nnodes0; i++ )
    {
	nds[i] = i;
    }
    std::random_shuffle( nds.begin(), nds.end() );

    for( i = 0; i < nInterfNds; i++ )
    {
	pGraph->AddEdge(nds[i], nds[i] + nnodes0 , 1 );
    }
    pBNet = CBNet::Create( 2*nnodes0, nts, nodeAss, pGraph );
    
    pBNet->AllocFactors();
    for( i = 0; i < 2*nnodes0; i++ )
    {
	pBNet->AllocFactor(i);
	pBNet->GetFactor(i)->CreateAllNecessaryMatrices();
    }

    return CDBN::Create(pBNet);

}
PNL_END
