/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  File:      CreateBNets.cpp                                             //
//                                                                         //
//  Purpose:   implementation of function to create random CBNet object    //
//                                                                         //
//  Author(s): (in alphabetical order)                                     //
//             Abrosimova, Chernishova, Gergel, Senin, Sysoyev, Vinogradov //
//             NNSU                                                        //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "CreateBNets.h"

PNL_USING

float GetBelief(void)
{
    float belief;
    int randInt = rand();
    belief = randInt / float(RAND_MAX);

    return belief;
}

float GetBelief(double porog)
{
    float belief = GetBelief();
    while (belief >= porog) 
        belief -= porog;
    
    return belief;
}

int GetUtility(int min, int max)
{
    int randInt = rand() % (max - min + 1);
    
    return (randInt + min);
}

int GetRandomNumberOfStates(int max_num_states, int min_num_states = 2)
{
    int num_states;
    num_states = rand() % (max_num_states - min_num_states + 1);
    return (num_states + min_num_states);
}

CGraph* CreateRandomGraph(int num_nodes, int num_indep_nodes, 
                          int max_size_family)
{
    PNL_CHECK_LEFT_BORDER( num_nodes, 10 );
    PNL_CHECK_RANGES( num_indep_nodes, 1, num_nodes-1 );
    PNL_CHECK_RANGES( max_size_family, 2, num_nodes );
    
    int i, j, k;
    
    CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
    PNL_CHECK_IF_MEMORY_ALLOCATED( pGraph );
    
    srand((unsigned int)time(NULL));

    pGraph->AddNodes(num_nodes);
    
    int num_parents;
    int ind_parent;
    intVector prev_nodes(0);
    for ( i = num_indep_nodes; i < num_nodes; i++)
    {
        prev_nodes.resize(0);
        for ( j = 0; j < i; j++) 
            prev_nodes.push_back(j);

        num_parents = rand() % (max_size_family - 1);
        num_parents += 1;
        num_parents = (num_parents > i) ? i : num_parents;
    
        for ( j = 0; j < num_parents; j++)
        {
            ind_parent = rand() % prev_nodes.size();
            pGraph->AddEdge(prev_nodes[ind_parent], i, 1);
            prev_nodes.erase(prev_nodes.begin() + ind_parent);
        }
    }

    return pGraph;
}

CGraph* CreateRandomGraphWithToyQMRSpecific(int num_nodes, 
    int num_indep_nodes, int max_size_family)
{
    PNL_CHECK_LEFT_BORDER( num_nodes, 10 );
    PNL_CHECK_RANGES( num_indep_nodes, 1, num_nodes-1 );
    PNL_CHECK_RANGES( max_size_family, 2, num_nodes );
    
    int i, j, k;
    
    CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
    PNL_CHECK_IF_MEMORY_ALLOCATED( pGraph );
    
    srand((unsigned int)time(NULL));

    pGraph->AddNodes(num_nodes);
    
    int num_parents;
    int ind_parent;
    intVector prev_nodes(0);
    for ( i = num_indep_nodes; i < num_nodes; i++)
    {
        prev_nodes.resize(0);
        for ( j = 0; j < num_indep_nodes; j++) 
            prev_nodes.push_back(j);

        num_parents = rand() % (max_size_family - 1);
        num_parents += 1;
        num_parents = (num_parents > i) ? i : num_parents;
    
        for ( j = 0; j < num_parents; j++)
        {
            ind_parent = rand() % prev_nodes.size();
            pGraph->AddEdge(prev_nodes[ind_parent], i, 1);
            prev_nodes.erase(prev_nodes.begin() + ind_parent);
        }
    }

    return pGraph;
}

CGraph* CreateGraphWithPyramidSpecific(int& num_nodes, int num_indep_nodes, 
                                       int num_layers)
{
    PNL_CHECK_LEFT_BORDER( num_indep_nodes, 1 );
    PNL_CHECK_LEFT_BORDER( num_layers, 1 );

    int i, j, k;
    
    CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
    PNL_CHECK_IF_MEMORY_ALLOCATED( pGraph );
    
    srand((unsigned int)time(NULL));

    num_nodes = num_indep_nodes;
	int num_nodes_into_curr_layer = num_indep_nodes;
	for (i = 0; i < num_layers - 1; i++)
	{
		num_nodes += num_nodes_into_curr_layer * 2 + 1;
		num_nodes_into_curr_layer = num_nodes_into_curr_layer * 2 + 1;
	}

	pGraph->AddNodes(num_nodes);
    
    int StartParent = 0,
        EndParent = num_indep_nodes - 1,
        StartCurrLayer,
        EndCurrLayer;
    int Child1,
        Child2,
        Child3;
    int NumParents;

    for (int layer = 0; layer < num_layers - 1; layer++ )
    {
        StartCurrLayer = EndParent + 1;
        EndCurrLayer = StartCurrLayer + 2 * (EndParent - StartParent + 1);
        NumParents = 0;

        for (j = StartParent; j <= EndParent; j++ )
        {
            Child1 = EndParent + NumParents * 2 + 1;
            Child2 = EndParent + NumParents * 2 + 2;
            Child3 = EndParent + NumParents * 2 + 3;

            pGraph->AddEdge(j, Child1, 1);
            pGraph->AddEdge(j, Child2, 1);
            pGraph->AddEdge(j, Child3, 1);

            NumParents++;
        }
        StartParent = StartCurrLayer;
        EndParent = EndCurrLayer;
    }    

    return pGraph;
}

CGraph* CreateCompleteGraph(int num_nodes)
{
    PNL_CHECK_LEFT_BORDER( num_nodes, 1 );

    int i, j, k;
    
    CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
    PNL_CHECK_IF_MEMORY_ALLOCATED( pGraph );
    
    srand((unsigned int)time(NULL));

    pGraph->AddNodes(num_nodes);
    
    for (j = 1; j < num_nodes; j++ )
        for (i = 0; i < j; i++ )
            pGraph->AddEdge(i, j, 1);

    return pGraph;
}

CGraph* CreateGraphWithRegularGridSpecific(int& num_nodes, int width, 
    int height, int num_layers)
{
    PNL_CHECK_LEFT_BORDER( width, 2 );
    PNL_CHECK_LEFT_BORDER( height, 2 );
    PNL_CHECK_LEFT_BORDER( num_layers, 1 );

    int i, j, k;

    CGraph *pGraph = CGraph::Create(0, NULL, NULL, NULL);
    PNL_CHECK_IF_MEMORY_ALLOCATED( pGraph );

    srand((unsigned int)time(NULL));

    int num_nodes_one_layer = width * height;
    num_nodes = num_nodes_one_layer * num_layers;
    pGraph->AddNodes(num_nodes);
    
    for (i = 0; i < num_layers; i++)
    {
        for (j = 1; j < width; j++)
            pGraph->AddEdge(
                i * num_nodes_one_layer + j - 1,
                i * num_nodes_one_layer + j, 1);
        for (k = 1; k < height; k++)
            pGraph->AddEdge(
                i * num_nodes_one_layer + (k - 1) * width,
                i * num_nodes_one_layer + k * width, 1);

        for (j = 1; j < width; j++)
            for (k = 1; k < height; k++)
            {
                pGraph->AddEdge(
                    i * num_nodes_one_layer + (k - 1) * width + j, 
                    i * num_nodes_one_layer + k * width + j, 1);
                pGraph->AddEdge(
                    i * num_nodes_one_layer + k * width + j - 1,
                    i * num_nodes_one_layer + k * width + j, 1);
            }

        if (i)
        {
            for (j = 0; j < width; j++)
                for (k = 0; k < height; k++)
                    pGraph->AddEdge(
                        (i - 1) * num_nodes_one_layer + k * width + j,
                        i * num_nodes_one_layer + k * width + j, 1);
        }
    }
    
    return pGraph;
}

CBNet* CreateRandomBayessian(CGraph* pGraph, int max_num_states)
{
    PNL_CHECK_LEFT_BORDER( max_num_states, 1 );
    PNL_CHECK_IF_MEMORY_ALLOCATED( pGraph );

    if( !pGraph->IsDAG() )
    {
        PNL_THROW( CInconsistentType, " the graph should be a DAG " );
    }
    if( !pGraph->IsTopologicallySorted() )
    {
        PNL_THROW( CInconsistentType, 
            " the graph should be sorted topologically " );
    }
    if (pGraph->NumberOfConnectivityComponents() > 1)
    {
        PNL_THROW( CInconsistentType, " the graph should be linked " );
    }

    int i, j, k;

    int num_nodes = pGraph->GetNumberOfNodes();
    CNodeType *nodeTypes = new CNodeType [num_nodes];
    int num_states;
    
    for ( i = 0; i < num_nodes; i++ )
    {
        num_states = GetRandomNumberOfStates(max_num_states);
        nodeTypes[i].SetType(1, num_states, nsChance);
    }

    int *nodeAssociation = new int[num_nodes];
    for ( i = 0; i < num_nodes; i++ )
    {
        nodeAssociation[i] = i;
    }

    CBNet *pBNet = CBNet::Create( num_nodes, num_nodes, nodeTypes,
                                     nodeAssociation, pGraph );
    
    CModelDomain* pMD = pBNet->GetModelDomain();
    
    CFactor **myParams = new CFactor*[num_nodes];
    int *nodeNumbers = new int[num_nodes];
    int **domains = new int*[num_nodes];

    intVector parents(0);
    for ( i = 0; i < num_nodes; i++)
    {
        nodeNumbers[i] = pGraph->GetNumberOfParents(i) + 1;
        domains[i] = new int[nodeNumbers[i]];
        pGraph->GetParents(i, &parents);
        
        for ( j = 0; j < parents.size(); j++ )
            domains[i][j] = parents[j];
        domains[i][nodeNumbers[i]-1] = i;
    }

    pBNet->AllocFactors();

    for( i = 0; i < num_nodes; i++ )
    {
        myParams[i] = CTabularCPD::Create( domains[i], 
            nodeNumbers[i], pMD);
    }

    float **data = new float*[num_nodes];
    int size_data;
    int num_states_node;
    int num_blocks;
    intVector size_nodes(0);
    float belief, sum_beliefs;

    for ( i = 0; i < num_nodes; i++ )
    {
        size_data = 1;
        size_nodes.resize(0);
        for ( j = 0; j < nodeNumbers[i]; j++ )
        {
            size_nodes.push_back(pBNet->GetNodeType(
                domains[i][j])->GetNodeSize());
            size_data *= size_nodes[j];
        }
        num_states_node = size_nodes[size_nodes.size() - 1];
        num_blocks = size_data / num_states_node;
        
        data[i] = new float[size_data];

        for ( j = 0; j < num_blocks; j++ )
        {
            sum_beliefs = 0.0;
            for ( k = 0; k < num_states_node - 1; k++ )
            {
                belief = GetBelief(1.0 - sum_beliefs);
                data[i][j * num_states_node + k] = belief;
                sum_beliefs += belief;
            }
            belief = 1.0 - sum_beliefs;
            data[i][j * num_states_node + num_states_node - 1] = belief;
        }
    }

    for( i = 0; i < num_nodes; i++ )
    {
        myParams[i]->AllocMatrix(data[i], matTable);
        pBNet->AttachFactor(myParams[i]);
    }    

    delete [] nodeTypes;
    delete [] nodeAssociation;

    return pBNet;
}

CBNet* Create_BNet_Default(int num_nodes, int max_num_states, 
    int num_indep_nodes, int max_size_family, long& num_edges)
{
    CGraph* pGraph = CreateRandomGraph(num_nodes, num_indep_nodes,
        max_size_family);
    CBNet* pBNet = CreateRandomBayessian(pGraph, max_num_states);
    num_edges = pBNet->GetGraph()->GetNumberOfEdges();

    return pBNet;
}

CBNet* Create_BNet_toyQMR(int num_nodes, int max_num_states, 
    int num_indep_nodes, int max_size_family, long& num_edges)
{
    CGraph* pGraph = CreateRandomGraphWithToyQMRSpecific(
        num_nodes, num_indep_nodes, max_size_family);
    CBNet* pBNet = CreateRandomBayessian(pGraph, max_num_states);
    num_edges = pBNet->GetGraph()->GetNumberOfEdges();

    return pBNet;
}

CBNet* Create_BNet_Pyramid(int &num_nodes, int max_num_states, 
    int num_indep_nodes, int num_layers, long& num_edges)
{
    CGraph* pGraph = CreateGraphWithPyramidSpecific(
        num_nodes, num_indep_nodes, num_layers);
    CBNet* pBNet = CreateRandomBayessian(pGraph, max_num_states);
    num_edges = pBNet->GetGraph()->GetNumberOfEdges();

    return pBNet;
}

CBNet* Create_BNet_CompleteGraph(int num_nodes, int max_num_states, 
    long &num_edges)
{
    CGraph* pGraph = CreateCompleteGraph(num_nodes);
    CBNet* pBNet = CreateRandomBayessian(pGraph, max_num_states);
    num_edges = pBNet->GetGraph()->GetNumberOfEdges();

    return pBNet;
}

CBNet* Create_BNet_RegularGrid(int& num_nodes, int width, int height, 
    int max_num_states, long& num_edges, int num_layers)
{
    CGraph* pGraph = CreateGraphWithRegularGridSpecific(num_nodes, 
        width, height, num_layers);
    CBNet* pBNet = CreateRandomBayessian(pGraph, max_num_states);
    num_edges = pBNet->GetGraph()->GetNumberOfEdges();

    return pBNet;
}
