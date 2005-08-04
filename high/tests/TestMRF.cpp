#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"

#if !defined(WIN32)
#define itoa(__val,__str,__rad) sprintf((__str), "%i", (__val))
#endif

PNLW_USING
using namespace std;

static char func_name[] = "testMRF";

static char* test_desc = "Provide all tests for MRF";

static char* test_class = "Algorithm";

MRF *VerySimpleMRFModel()
{
//    node0 -- node1
//      |        |    
//    node2 -- node3

    // MRF creation
    MRF *pMRF; 
    pMRF = new MRF();
    // nodes creation 
    pMRF->AddNode(discrete ^ "node0 node3 node1 node2", "value0 value1"); 

    // set cliques and distributions on them
    pMRF->SetPTabular("node0 node1", "0.18 0.23 0.35 0.24");
    pMRF->SetPTabular("node1 node3", "0.06 0.42 0.14 0.38");
    pMRF->SetPTabular("node3 node2", "0.15 0.33 0.41 0.11");
    pMRF->SetPTabular("node2 node0", "0.16 0.37 0.19 0.28");

    return pMRF;
}

MRF *SimpleMRFModel()
{
//    node0 -- node1
//      |   /    |    
//    node2 -- node3

    // MRF creation
    MRF *pMRF; 
    pMRF = new MRF();
    // nodes creation 
    pMRF->AddNode(discrete ^ "node0 node3", "Value0 Value1 Value2"); 
    pMRF->AddNode(discrete ^ "node2 node1", "value0 value1"); 

    // set cliques and distributions on them
    pMRF->SetPTabular("node0 node1 node2", "0.05 0.1 0.08 0.2 0.09 0.04 0.07 0.11 0.03 0.01 0.16 0.06");
    pMRF->SetPTabular("node1 node3 node2", "0.06 0.02 0.12 0.105 0.035 0.03 0.19 0.02 0.01 0.18 0.03 0.11");

    return pMRF;
}

MRF *GridMRFModel()
{
//    node0 -- node1 -- node2
//      |        |        |
//    node3 -- node4 -- node5
//      |        |        |
//    node6 -- node7 -- node8
    // node values
    TokArr aChoice = "value0 value1";// possible values for nodes
    // MRF creation
    MRF *pMRF; 
    pMRF = new MRF();
    // nodes creation 
    pMRF->AddNode(discrete ^ "node0 node1 node2 node3 node4 node5 node6 node7 node8", aChoice); 
   
    pMRF->SetClique("node0 node1"); 
    pMRF->SetPTabular("node0 node1", "0.3 0.39 0.1 0.21");
    pMRF->SetPTabular("node1 node2", "0.26 0.34 0.24 0.16");
    pMRF->SetPTabular("node0 node3", "0.29 0.26 0.14 0.31");
    pMRF->SetPTabular("node1 node4", "0.45 0.04 0.33 0.18");
    pMRF->SetPTabular("node2 node5", "0.17 0.3 0.3 0.23");
    pMRF->SetPTabular("node3 node4", "0.4 0.06 0.34 0.2");
    pMRF->SetPTabular("node4 node5", "0.23 0.4 0.15 0.22");
    pMRF->SetPTabular("node6 node3", "0.34 0.24 0.33 0.09");
    pMRF->SetPTabular("node4 node7", "0.34 0.29 0.17 0.2");
    pMRF->SetPTabular("node5 node8", "0.14 0.31 0.14 0.41");
    pMRF->SetPTabular("node6 node7", "0.15 0.34 0.01 0.5");
    pMRF->SetPTabular("node7 node8", "0.14 0.46 0.21 0.19");

    return pMRF;
}

MRF *MRFModel()
{
    // Node6--Node5--Node0--Node1 
    //     \   |  \ /  |   /  |
    //       \ |  / \  | /    |
    //        Node4--Node3--Node2

    MRF *net;
    net = new MRF();

    net->AddNode(discrete^"Node0 Node2 Node4 Node6", "value0 value2 value3");
    net->AddNode(discrete^"Node1 Node3 Node5", "value0 value2 value3 value 4");

    net->SetClique("Node1 Node2 Node3");
    net->SetClique("Node0 Node1 Node3");

    // ...
    return net;
}

void TestMRFModelCreate()
{
    MRF *net = SimpleMRFModel();

    TokArr P = net->GetPTabular("node2 node0^Value1 node1");
    //cout << String(P) << endl;
    if( P[0].FltValue() != 0.09f ||
        P[1].FltValue() != 0.07f ||
        P[2].FltValue() != 0.04f ||
        P[3].FltValue() != 0.11f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting of tabular parameters for MRF is wrong");
    }

    P = net->GetPTabular("node1^value0 node0 node2");
    //cout << String(P) << endl;
    if( P[0].FltValue() != 0.05f ||
        P[1].FltValue() != 0.1f ||
        P[2].FltValue() != 0.09f ||
        P[3].FltValue() != 0.04f ||
        P[4].FltValue() != 0.03f ||
        P[5].FltValue() != 0.01f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting of tabular parameters for MRF is wrong");
    }

    P = net->GetPTabular("node2^value0 node1^value1 node3^Value2");
    //cout << String(P) << endl;
    if( P[0].FltValue() != 0.03f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting of tabular parameters for MRF is wrong");
    }

    net->DestroyClique("node3 node2 node1");
    net->SetPTabular("node3^Value2 node1", "0.08 0.13");
    net->SetPTabular("node3^Value0 node1", "0.09 0.23");
    net->SetPTabular("node3^Value1 node1", "0.14 0.33");

    P = net->GetPTabular("node1^value1 node3^Value0");
    if( P[0].FltValue() != 0.23f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting of tabular parameters for MRF is wrong");
    }

    net->DelNode("node2");
    net->SetClique("node0 node1");
    net->AddNode("node4", "value0 value1 value2 value3");
    net->SetClique("node1 node4");
    net->SetPTabular("node0 node1", "0.21 0.28 0.13 0.23 0.11 0.04");

    P = net->GetPTabular("node1 node0");
    if( P[0].FltValue() != 0.21f ||
        P[1].FltValue() != 0.13f ||
        P[2].FltValue() != 0.11f ||
        P[3].FltValue() != 0.28f ||
        P[4].FltValue() != 0.23f ||
        P[5].FltValue() != 0.04f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting of tabular parameters for MRF is wrong");
    }

    /*P = net->GetPTabular("node1 node3");
    cout << String(P) << endl;
    if( P[0].FltValue() != 0.09f ||
        P[1].FltValue() != 0.14f ||
        P[2].FltValue() != 0.08f ||
        P[3].FltValue() != 0.23f ||
        P[4].FltValue() != 0.33f ||
        P[5].FltValue() != 0.13f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting of tabular parameters for MRF is wrong");
    }*/

    float defaultProb = 1.0f / 8.0f;
    P = net->GetPTabular("node1 node4");
    for(int i = 0; i < 8; i++)
    {
        if( P[i].FltValue() != defaultProb )
        {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting of tabular parameters for MRF is wrong");
        }
    }

    MRF *netGrid = GridMRFModel();
    P = netGrid->GetPTabular("node7 node6");
    //cout << String(P) << endl;
    if( P[0].FltValue() != 0.15f ||
        P[1].FltValue() != 0.01f ||
        P[2].FltValue() != 0.34f ||
        P[3].FltValue() != 0.5f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting of tabular parameters for MRF is wrong");
    }

    cout << "TestMRFModelCreate is completed successfully" << endl;

	delete net;
	delete netGrid;
}

bool bTokArrEqual(TokArr first, TokArr second, float eps)
{
    if(first.size() != second.size())
    {
        return false;
    }
    int i;
    for(i = 0; i < first.size(); i++)
    {
	first[i].Resolve();
	second[i].Resolve();
        if(first[i].FltValue() - second[i].FltValue() > eps)
        {
            return false;
        }
    }
    return true;
}

void TestMRFGetJPD()
{
    MRF *net = GridMRFModel();

    float eps1 = 1e-6f;
    float eps2 = 1e-3f;
    float eps3 = 1e-1f;
    int nNode = net->GetNumberOfNodes();
    int i;
    TokArr jJPD, nJPD, pJPD, gJPD;

    char nodeName[6];
    strcpy(nodeName, "node");
    for(i = 0; i < nNode; i++)
    {
        itoa(i, nodeName + 4, 10);
        
        net->SetProperty("Inference", "jtree");
        jJPD = net->GetJPD(nodeName);
        net->SetProperty("Inference", "naive");
        nJPD = net->GetJPD(nodeName);
        
        if(!bTokArrEqual(jJPD, nJPD, eps1))
        {
            PNL_THROW(pnl::CAlgorithmicException, "Results of JTree and Naive inferences are diferent");
        }
        
        net->SetProperty("Inference", "pearl");
        pJPD = net->GetJPD(nodeName);
        
        if(!bTokArrEqual(pJPD, nJPD, eps2))
        {
            PNL_THROW(pnl::CAlgorithmicException, "Results of Pearl and Naive inferences are diferent");
        }
        
        net->SetProperty("Inference", "gibbs");
        gJPD = net->GetJPD(nodeName);
       
        if(!bTokArrEqual(gJPD, nJPD, eps3))
        {
            PNL_THROW(pnl::CAlgorithmicException, "Results of Gibbs and Naive inferences are diferent");
        }
    }

    net->EditEvidence("node0^value1 node2^value0 node8^value1 node5^value0");
    for(i = 0; i < nNode; i++)
    {
        itoa(i, nodeName + 4, 10);
        
        net->SetProperty("Inference", "jtree");
        jJPD = net->GetJPD(nodeName);
        net->SetProperty("Inference", "naive");
        nJPD = net->GetJPD(nodeName);
        
        if(!bTokArrEqual(jJPD, nJPD, eps1))
        {
            PNL_THROW(pnl::CAlgorithmicException, "Results of JTree and Naive inferences are diferent");
        }
        
        net->SetProperty("Inference", "pearl");
        pJPD = net->GetJPD(nodeName);
        
        if(!bTokArrEqual(pJPD, nJPD, eps2))
        {
            PNL_THROW(pnl::CAlgorithmicException, "Results of Pearl and Naive inferences are diferent");
        }
       
        net->SetProperty("Inference", "gibbs");
        gJPD = net->GetJPD(nodeName);
        
        if(!bTokArrEqual(gJPD, nJPD, eps3))
        {
            PNL_THROW(pnl::CAlgorithmicException, "Results of Gibbs and Naive inferences are diferent");
        }
    }
    net->SetProperty("Inference", "jtree");
    jJPD = net->GetJPD("node7 node6");
    net->SetProperty("Inference", "naive");
    nJPD = net->GetJPD("node7 node6");
    
    if(!bTokArrEqual(jJPD, nJPD, eps1))
    {
        PNL_THROW(pnl::CAlgorithmicException, "Results of JTree and Naive inferences are diferent");
    }
    
    /*
    net->SetProperty("Inference", "pearl");
    pJPD = net->GetJPD("node7 node6");
    
    //cout << String(nJPD) << endl;
    //cout << String(pJPD) << endl;
    if(!bTokArrEqual(pJPD, nJPD, eps3))
    {
        PNL_THROW(pnl::CAlgorithmicException, "Results of Pearl and Naive inferences are diferent");
    }
    */
    
    cout << "TestMRFModelCreate is completed successfully" << endl;

	delete net;
}

void TestNodeType()
{
    MRF *net = SimpleMRFModel();
	TokArr type = net->GetNodeType("node0");
	printf("\n%s\n",String(type).c_str());
	delete net;
}

void TestSaveLoadMRF()
{
    MRF *net = GridMRFModel();
    
	int numCliques = net->GetNumberOfCliques();
    net->SaveNet("savedMRF.xml");
    delete net;

	MRF *newNet;
	newNet = new MRF;
	newNet->LoadNet("savedMRF.xml");
	
	newNet->SetProperty("Inference", "jtree");
	newNet->AddEvidToBuf("node0^value0");
    TokArr jJPD = newNet->GetJPD("node1");

	printf("\n%s\n",String(jJPD).c_str());

	delete newNet;	
}

void testEvidencesManipulationMRF()
{
    MRF *net = SimpleMRFModel();
	
	net->EditEvidence("node0^Value0");
	net->ClearEvid();
	net->GenerateEvidences(100);
	net->ClearEvidBuf();
	net->AddEvidToBuf("node0^Value0");
	TokArr jJPD = net->GetJPD("node1");

	printf("\n%s\n",String(jJPD).c_str());
    
	delete net;

}

void testPropertiesMRF()
{
    MRF net;
	//Adding new network property
	net.SetProperty("date","20july2005");
	//Network proprty value request
	String value = net.GetProperty("date");
	printf("\n%s\n",value.c_str());
}

void testLearningMRF()
{
	MRF *net;

    net = GridMRFModel(); 
	net->GenerateEvidences(100);
	//default learning
	net->LearnParameters();
	//1. EM learning algorithm
	net->SetProperty("Learning","em");
	net->SetProperty("EMMaxNumberOfIterations", "10");
    net->SetProperty("EMTolerance", "1e-4");
	net->LearnParameters();
	//2. Bayes learning algorithm
	net->SetProperty("Learning","bayes");
	net->LearnParameters();

	delete net;

	net = SimpleMRFModel(); 
	net->GenerateEvidences(100);
	//default learning
	net->LearnParameters();
	//1. EM learning algorithm
	net->SetProperty("Learning","em");
	net->SetProperty("EMMaxNumberOfIterations", "10");
    net->SetProperty("EMTolerance", "1e-4");
	net->LearnParameters();
	//2. Bayes learning algorithm
	net->SetProperty("Learning","bayes");
	net->LearnParameters();

	//memory free
	delete net;

}
void testPNLObjectsRequestsMRF()
{ 
	MRF *net = SimpleMRFModel();
	net->AddEvidToBuf("node0^Value0");

	//requests
	pnl::CMNet& pnlNet = net->Model();
	pnl::CInfEngine& pnlInfEngine = net->Inference();
	pnl::CStaticLearningEngine& pnlLearning = net->Learning();
	pnl::CEvidence *pnlEvid = net->GetPNLEvidence(); 

	//memory free 
	delete net;
}

int testMRF()
{
	int ret = TRS_OK;
	try 
    { 
		TestMRFGetJPD();
	    testLearningMRF();
	    TestSaveLoadMRF();
		testPNLObjectsRequestsMRF();
		testPropertiesMRF();
		testEvidencesManipulationMRF();
		TestNodeType();
        TestMRFModelCreate();
    }
    catch(pnl::CException e)
    {
        std::cout << e.GetMessage();
        ret = false;
    }
	return ret;

}

void initTestsMRF()
{
    trsReg(func_name, test_desc, test_class, testMRF);
}
