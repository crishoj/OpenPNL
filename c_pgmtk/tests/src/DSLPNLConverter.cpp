
#include "DSLPNLConverter.h"

PNL_USING

DSLPNLConverter::DSLPNLConverter()
{
}

DSLPNLConverter::~DSLPNLConverter()
{
}

CBNet* DSLPNLConverter::CreateBNet(DSL_network &dslNet)
{
    CBNet* pnlBNet = NULL;
    
    // Create mapping nodeId <-> number
    theIds.CleanUp();
    
    // Traverse through all the nodes and check if we have only CPTs
    // if happens noisy-MAX, convert it to CPT
    // Create a list of DSL_ids
    int handle = dslNet.GetFirstNode();
    while (handle>=0)
    {
        if (dslNet.GetNode(handle)->Definition()->GetType()!=DSL_CPT)
        {
            if (dslNet.GetNode(handle)->Definition()->GetType()==DSL_NOISY_MAX)
            {
                int res = dslNet.GetNode(handle)->ChangeType(DSL_CPT);
                if (res!=DSL_OKAY)
                    return NULL;
            }
            else
            {
                return NULL;
            }
        }
        theIds.Add(dslNet.GetNode(handle)->Info().Header().GetId());
        handle = dslNet.GetNextNode(handle);
    }
    
    // Read number of nodes in the net
    // Just for sake of safety
    int numberOfNodes = dslNet.GetNumberOfNodes();
    if (numberOfNodes!=theIds.NumItems())
    {
        std::cout << "something went wrong!" << std::endl;
        return NULL;
    }
    
    // some debug stuff
#ifdef DSLPNL_DEBUG
    int i;
    std::cerr << "Number of nodes  : " << numberOfNodes << std::endl;
    std::cerr << "DSL_ids: " << std::endl;
    for (i=0; i<numberOfNodes; i++)
        std::cerr << i << " : " << theIds[i] << std::endl;
#endif
    
    // Create CGraph
    CGraph* pnlGraph = CreateCGraph(dslNet);
    if (pnlGraph==NULL)
    {
        std::cout << "PNL graph not created!" << std::endl;
        return NULL;
    }
    
    // Create BNet 
    pnlBNet = CreateCBNet(dslNet,pnlGraph);
    if (pnlBNet==NULL)
    {
        std::cout << "PNL Bnet not created!" << std::endl;
        return NULL;
    }
    
    // Allcoate factors
    CreateFactors(dslNet,pnlBNet);
    
    return pnlBNet;
}

CGraph* DSLPNLConverter::CreateCGraph(DSL_network &dslNet)
{
    CGraph* pnlGraph = NULL; 
    int i,j;
    
    // Read number of nodes in the net
    int numberOfNodes = dslNet.GetNumberOfNodes();
    
    // Create and the structure for number of neighbors
    int *numberOfNeighbors = new int[numberOfNodes];
    
    // Create the structure for lists of neighbors
    int **theNeighbors = new int*[numberOfNodes];
    ENeighborType **neighborsTypes = new ENeighborType*[numberOfNodes];
    
    
    // Here is a big deal. We are trying to do several things at a time 
    for (i=0;i<numberOfNodes;i++)
    {
        int nodeHandle = dslNet.FindNode(theIds[i]);
        
        // below should never happen 
        if (nodeHandle<0)
        {
            std::cout << "Handle <0 !" << std::endl;
            return NULL;
        }
        
        int numberOfParents = dslNet.GetParents(nodeHandle).NumItems();
        int numberOfChildren = dslNet.GetChildren(nodeHandle).NumItems();
        
        // Assign number of neighbors
        numberOfNeighbors[i] = numberOfParents + numberOfChildren;
        
        // Create list of neighbors
        if (numberOfNeighbors[i]>0)
        {
            // Allocate memory (here we know how much for ith node
            theNeighbors[i] = new int[numberOfNeighbors[i]];
            neighborsTypes[i] = new ENeighborType[numberOfNeighbors[i]];
            
            // Since SMILE keeps parents and children separetly, we have to do this separetly
            
            // first always go parents 
            for (j=0;j<numberOfParents;j++)
            {
                int parentHandle = dslNet.GetParents(nodeHandle).operator[](j);
                theNeighbors[i][j] = theIds.FindPosition(dslNet.GetNode(parentHandle)->Info().Header().GetId());
                // Some sanity check
                if (theNeighbors[i][j]<0)
                {
                    std::cout << "Handle <0 !" << std::endl;
                    return NULL;
                }
                neighborsTypes[i][j] = ntParent;
            }
            
            // no we do children  
            for (j=numberOfParents;j<numberOfChildren+numberOfParents;j++)
            {
                int childHandle = dslNet.GetChildren(nodeHandle).operator[](j-numberOfParents);
                theNeighbors[i][j] = theIds.FindPosition(dslNet.GetNode(childHandle)->Info().Header().GetId());
                // Some sanity check
                if (theNeighbors[i][j]<0)
                {
                    std::cout << "Handle <0 !" << std::endl;
                    return NULL;
                }
                neighborsTypes[i][j] = ntChild;
            }
        }
        else
        {
            // this is when there are no neighbors (BTW is it ok for PNL)
            theNeighbors[i] = NULL;
            neighborsTypes[i] = NULL;
        }
    }
    
    
    // some debug stuff
#ifdef DSLPNL_DEBUG
    std::cerr << "Neighbors and their types: " << std::endl;
    for (i=0; i<numberOfNodes; i++)
    {
        std::cerr << i << " : ";
        for (j=0;j<numberOfNeighbors[i];j++)
        {
            std::cerr << "(" << theNeighbors[i][j] << ",";
            if (neighborsTypes[i][j]==ntParent)
                std::cerr <<  "p) ";
            else 
                std::cerr <<  "c) ";
        }
        std::cerr << std::endl;
    }
#endif
    
    // call PNL
    pnlGraph = CGraph::Create(numberOfNodes, numberOfNeighbors, theNeighbors, neighborsTypes);
    
    // Clean up the mess
    delete[] numberOfNeighbors;
    for (i=0;i<numberOfNodes;i++)
    {
        if (theNeighbors[i]!=NULL)
        {
            delete[] theNeighbors[i];
            delete[] neighborsTypes[i];
        }
    }
    delete[] theNeighbors;
    delete[] neighborsTypes;
    
    return pnlGraph;
}


CBNet* DSLPNLConverter::CreateCBNet(DSL_network& dslNet, CGraph* pnlGraph)
{
    CBNet* pnlBNet = NULL;
    int i;
    
    // Read number of nodes in the net
    int numberOfNodes = dslNet.GetNumberOfNodes();
    
    // For now, we assume only DSL_cpt (ensured earlier!)
    // Traverse through nodes and try to identify types (number of outcomes)
    
    DSL_intArray outcomeNumbers;
    int numberOfOutcomes;
    for (i=0;i<numberOfNodes;i++)
    {
        int handle = dslNet.FindNode(theIds[i]);
        numberOfOutcomes = dslNet.GetNode(handle)->Definition()->GetNumberOfOutcomes();
        
        // Check if we have already got this type, if not, add it
        if (outcomeNumbers.FindPosition(numberOfOutcomes)==DSL_OUT_OF_RANGE)
            outcomeNumbers.Add(numberOfOutcomes);
    }
    int numberOfNodeTypes = outcomeNumbers.NumItems();
    
    
    // Create and initialize an array of node types
    CNodeType *nodeTypes = new CNodeType[numberOfNodeTypes];
    for (i=0;i<numberOfNodeTypes;i++)
        nodeTypes[i].SetType(1,outcomeNumbers[i]);
    
#ifdef DSLPNL_DEBUG
    std::cerr << "Number of node types: " << numberOfNodeTypes << std::endl;
    std::cerr << "Number types: " << std::endl;
    for (i=0;i<numberOfNodeTypes;i++)
        std::cerr << nodeTypes[i].GetNodeSize() << " ";
    std::cerr << std::endl;
#endif
    
    // Create an array of node associations
    int* nodeTypeAssociation = new int[numberOfNodes];
    for (i=0;i<numberOfNodes;i++)
    {
        numberOfOutcomes = dslNet.GetNode(dslNet.FindNode(theIds[i]))->Definition()->GetNumberOfOutcomes();
        nodeTypeAssociation[i] = outcomeNumbers.FindPosition(numberOfOutcomes);
    }
    
#ifdef DSLPNL_DEBUG
    std::cerr << "Node type associations: " << std::endl;
    for (i=0;i<numberOfNodes;i++)
    {
        std::cerr << i << " :  " << nodeTypeAssociation[i] << " (" ;
        std::cerr << nodeTypes[nodeTypeAssociation[i]].GetNodeSize() << ")" << std::endl;
    }
    std::cerr << std::endl;
#endif
    
    // Create finally CBNet 
    pnlBNet = CBNet::Create(numberOfNodes, numberOfNodeTypes, nodeTypes, nodeTypeAssociation, pnlGraph);
    
    // Clean up the mess
    delete[] nodeTypeAssociation;
    delete[] nodeTypes;
    
    return pnlBNet;
}


void DSLPNLConverter::CreateFactors(DSL_network& dslNet, CBNet* pnlBNet)
{
    int i,j,k;
    
    // Read number of nodes in the net
    int numberOfNodes = dslNet.GetNumberOfNodes();
    
    // This is a way PNL likes it	
    pnlBNet->AllocFactors();
    
    DSL_Dmatrix* dslMatrix;
    CCPD* pnlCPD;
    
    for (i=0;i<numberOfNodes;i++)
    {
        
        // Get parents of the ith node 
        // IMPORTANT -- we should preserve order from DSL_network, since
        // probabilities will be according DSL ordering
        
        DSL_intArray dslParents;
        dslParents = dslNet.GetParents(dslNet.FindNode(theIds[i]));
        
        // establish sizes and allocate memory
        int numberOfNodesInDomain = dslParents.NumItems() + 1;
        int* domain = new int[numberOfNodesInDomain];
        CNodeType** nodeTypes = new CNodeType*[numberOfNodesInDomain];
        
        // establish members of the domain
        for (j=0;j<numberOfNodesInDomain-1;j++)
            domain[j] = dslParents[j];
        
        domain[numberOfNodesInDomain-1] = i;
        
        // Fill up node types
        for (j=0;j<numberOfNodesInDomain;j++)
            nodeTypes[j] = const_cast <CNodeType*> (pnlBNet->GetNodeType(domain[j]));
        
        // Read CPT from SMILE
        dslNet.GetNode(dslNet.FindNode(theIds[i]))->Definition()->GetDefinition(&dslMatrix);
        
        // Alloc space for CPT
        int sizeOfCPT = dslMatrix->GetSize();
        float* flatCPT = new float[sizeOfCPT];
        
        // Here we convert 'copy' numbers from SMILE to PNL
        // The painful part is convert double to float.
        // Additionally we check if after conversion they sum-up to 1
        int numberOfMyStates = nodeTypes[numberOfNodesInDomain-1]->GetNodeSize();
        
        int iterations  = sizeOfCPT/numberOfMyStates;
        for (j=0;j<iterations;j++)
        {
            float sum = 0.0f;
            for (k=0;k<numberOfMyStates;k++)
            {
                flatCPT[j*numberOfMyStates+k] = static_cast <float> (dslMatrix->Subscript(j*numberOfMyStates+k));
                sum += flatCPT[j*numberOfMyStates+k];
            }
            if (sum!=1.0f)
            {
                for (k=0;k<numberOfMyStates;k++)
                    flatCPT[j*numberOfMyStates+k] /= sum;      
            }
        }
        
#ifdef DSLPNL_DEBUG
        std::cerr << "Node "<< i << " domain : ";
        for (j=0;j<numberOfNodesInDomain;j++)
        {
            std::cerr << domain[j] << " ";
        }
        std::cerr <<  std::endl;
        for (j=0;j<sizeOfCPT;j++)
            std::cerr << flatCPT[j] << " ";
        std::cerr <<  std::endl;
#endif
        
        CModelDomain* pMD = pnlBNet->GetModelDomain();
        pnlCPD = CTabularCPD::Create(domain, numberOfNodesInDomain, pMD, flatCPT);
        if (pnlCPD==NULL)
        {
            std::cout << "We got a problem with creating CPD" << std::endl;
            return;
        }
        pnlBNet->AttachFactor(pnlCPD);
        
        delete[] nodeTypes;
        delete[] domain;
        delete[] flatCPT; 
    }
}


CEvidence* DSLPNLConverter::CreateEvidence(DSL_network &dslNet, CBNet *pnlNet)
{
    int i, handle;
    
    int numberOfNodes = dslNet.GetNumberOfNodes();
    intVector evidenceNode;
    
    for (i=0;i<numberOfNodes;i++)
    {
        handle = dslNet.FindNode(theIds[i]);
        int evidence = dslNet.GetNode(handle)->Value()->GetEvidence();
        if (evidence>=0)
        {
            evidenceNode.push_back(i);
        }
    }
    int numberOfEvidenceNodes = evidenceNode.size();
    
    valueVector evidenceValue(numberOfEvidenceNodes);
    
    for (i=0;i<numberOfEvidenceNodes;i++)
    {
        handle = dslNet.FindNode(theIds[evidenceNode[i]]);
        evidenceValue[i].SetInt(dslNet.GetNode(handle)->Value()->GetEvidence());
    }
    
#ifdef DSLPNL_DEBUG
    std::cerr << "Evidence: " << std::endl;
    for (i=0;i<numberOfEvidenceNodes;i++)
        std::cerr << evidenceNode[i] << " " << evidenceValue[i] << std::endl;
    std::cerr <<  std::endl;
#endif
    
    return CEvidence::Create(pnlNet->GetModelDomain(),evidenceNode,evidenceValue);
}
