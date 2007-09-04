#include <time.h>
#include "test_conf.hpp"

#include "pnl_dll.hpp"
//#include "pnlRng.hpp"
//#include "pnlGraphicalModel.hpp"
//#include "BNet.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWGraph.hpp"

using namespace std;

//Adding a strange node: discrete2^CurrentHumanTurn


static char func_name[] = "testTokens";

static char* test_desc = "Provide all tests for tokens";

static char* test_class = "Algorithm";

bool TestResolve1()
{
    bool bCurrentError = false, bError = false;
    //Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    //Try to add a strange node
    try
    {
	bCurrentError = true;
	net.AddNode("discrete2^CurrentHumanTurn", "Rock Paper Scissors");
    }
    catch(pnl::CException e)
    {
	bCurrentError = false;
    }
    catch(...)
    {
	;
    }
    if(bCurrentError)
    {
	cout << "Error during adding bad node\n";
	bError = true;
    }

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");

    try 
    {
	net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
	net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
	net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
	net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    }
    catch (...) //There is an exception 
    {
	bError = true;
	cout << "Error during SetPTabular\n";
    }

    Tok tok;
    tok.Resolve(TokIdNode::root);
    if (!tok.IsResolved()) 
    {
	std::cout << "Is unresolved" << "\n";
	bError = true;
    }

    cout << "TestResolve1\n";

    return bError == false;
}

//Localization of a single node in the net
bool TestResolve2()
{
    //Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    //	net.MakeUniformDistribution();

    net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");


    Tok tok("discrete^PreviousHumanTurn");
    tok.Resolve(TokIdNode::root);

    if (!((tok.node.size()==1) && tok.IsResolved())) return false;

    //try to reinit toket with the same value
    tok.Init("discrete^PreviousHumanTurn");
    tok.Resolve(TokIdNode::root);

    if (!((tok.node.size()==1)&& tok.IsResolved())) return false;

    //
    tok.Init("discrete^PreviousCompTurn");
    tok.Resolve(TokIdNode::root);
    if (!((tok.node.size()==1)&& tok.IsResolved())) return false;

    tok.Init("discrete^CurrentHumanTurn");
    tok.Resolve(TokIdNode::root);
    if (!((tok.node.size()==1)&& tok.IsResolved())) return false;

    tok.Init("discrete2^PreviousHumanTurn2");
    tok.Resolve(TokIdNode::root);
    if (!((tok.node.size()==0) && !tok.IsResolved())) return false;

    tok.Init("discrete^PreviousHumanTurn2");
    tok.Resolve(TokIdNode::root);
    if (!((tok.node.size()==1) && !tok.IsResolved())) return false;

    return true;
}

//Localization of many nodes in the net (one token -> many nodes)
bool TestResolve3()
{
    //Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    //	net.MakeUniformDistribution();

    net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");


    Tok tok("Paper");
    tok.Resolve(TokIdNode::root);
    if (!((tok.node.size() == 0) && !tok.IsResolved())) return false;

    return true;
}

//Localization of many nodes in the net (if they are connected with used "^")
bool TestResolve4()
{
    //Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    //	net.MakeUniformDistribution();

    net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");


    Tok tok("discrete^PreviousCompTurn^discrete^PreviousHumanTurn");
    tok.Resolve(TokIdNode::root);

    //std::cout << tok.unres[2].front().id << "\n";

    std::cout << tok.node.size() << "\n";
    //std::cout << tok.unres[2].front() << "\n";

    if (!((tok.node.size()==2)&& tok.IsResolved())) return false;

    return true;
}

//Localization of many nodes in the net (if they are connected with used "^")
bool TestResolve4_5()
{
    //Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    //	net.MakeUniformDistribution();

    net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");


    Tok tok("PreviousCompTurn^PreviousHumanTurn");
    tok.Resolve(TokIdNode::root);

    //std::cout << tok.unres[2].front().id << "\n";

    if (!((tok.node.size()==2)&& tok.IsResolved())) return false;

    return true;
}


//In this test we try to add 2 and more different aliases to one node
void TestAlias()
{
    //Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    //	net.MakeUniformDistribution();

    net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");

    Tok tok("CurrentHumanTurn^Paper");
    tok.Resolve(TokIdNode::root);
    TokIdNode *pTokIdNode = tok.Node();

    if (!pTokIdNode) 
    {
	PNL_THROW(pnl::CNULLPointer, "pTokIdNode == NULL");
    }

    tok.Init("CurrentHumanTurn^Paper");
    tok.Resolve(TokIdNode::root);

    if (!((tok.node.size()==1)&& tok.IsResolved()))
    {
	PNL_THROW(pnl::CAlgorithmicException, "Can't resolve CurrentHumanTurn^Paper node");
    }

    pTokIdNode->Alias(String("Paper2"));

    tok.Init("CurrentHumanTurn^Paper2");
    tok.Resolve(TokIdNode::root);

    if (!((tok.node.size()==1)&& tok.IsResolved()))
    {
	PNL_THROW(pnl::CAlgorithmicException, "Can't resolve CurrentHumanTurn^Paper2 node");
    }

    tok.Init("CurrentHumanTurn^Paper3");
    tok.Resolve(TokIdNode::root);

    if (tok.IsResolved())
    {
	PNL_THROW(pnl::CAlgorithmicException, "CurrentHumanTurn^Paper3 is resolved but must be not");
    }

    tok.Init("CurrentHumanTurn^Paper");
    tok.Resolve(TokIdNode::root);

    if (!((tok.node.size()==1)&& tok.IsResolved()))
    {
	PNL_THROW(pnl::CAlgorithmicException, "Can't resolve CurrentHumanTurn^Paper node");
    }
}

//In this test we try to add one alias to brothers 
void TestAlias2()
{
    //Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    //	net.MakeUniformDistribution();

    net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");

    Tok tok("CurrentHumanTurn^Paper");
    tok.Resolve(TokIdNode::root);
    TokIdNode *pTokIdNodePaper = tok.Node();
    tok.Init("CurrentHumanTurn^Rock");
    tok.Resolve(TokIdNode::root);
    TokIdNode *pTokIdNodeRook = tok.Node();

    bool WasExceptionGenerated = false;

    try 
    {
	pTokIdNodePaper->Alias(String("NewName"));
	pTokIdNodeRook->Alias(String("NewName"));
    }
    catch(pnl::CBadArg e)
    {
	WasExceptionGenerated = true;
    }

    if (!WasExceptionGenerated)
    {
	PNL_THROW(pnl::CAlgorithmicException, "System allows to add one alias to brothers");
    }

    WasExceptionGenerated = false;
}

//In this test we try to add one alias to brothers 
void TestAlias3()
{
    //Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    //	net.MakeUniformDistribution();

    net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");

    Tok tok("discrete^PreviousCompTurn");
    tok.Resolve(TokIdNode::root);
    TokIdNode *pTokIdNode1 = tok.Node();
    tok.Init("discrete^PreviousHumanTurn");
    tok.Resolve(TokIdNode::root);
    TokIdNode *pTokIdNode2 = tok.Node();

    bool WasExceptionGenerated = false;

    try 
    {
	pTokIdNode1->Alias(String("NewName"));
	pTokIdNode2->Alias(String("NewName"));
    }
    catch(pnl::CBadArg e)
    {
	WasExceptionGenerated = true;
    }

    if (!WasExceptionGenerated)
    {
	PNL_THROW(pnl::CAlgorithmicException, "System allows to add one alias to brothers");
    }

    WasExceptionGenerated = false;
}

//This function tests: is regiter important or not
void TestAlias4()
{
    //Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    //	net.MakeUniformDistribution();

    net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
    net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");

    Tok tok("discrete^PreviousCompTurn");
    tok.Resolve(TokIdNode::root);
    TokIdNode *pTokIdNode1 = tok.Node();
    tok.Init("discrete^PreviousHumanTurn");
    tok.Resolve(TokIdNode::root);
    TokIdNode *pTokIdNode2 = tok.Node();

    bool WasExceptionGenerated = false;

    try 
    {
	pTokIdNode1->Alias(String("NewName"));
	pTokIdNode2->Alias(String("newname"));
    }
    catch(pnl::CBadArg e)
    {
	WasExceptionGenerated = true;
    }

    if (!WasExceptionGenerated)
    {
	PNL_THROW(pnl::CAlgorithmicException, "System allows to add one alias to brothers");
    }

    WasExceptionGenerated = false;
}

//This function checks all basic operations with tokens
void SimpleTest()
{
    TokIdNode *p = TokIdNode::root;
    TokIdNode *a, *b, *a1, *a2, *b1, *b2, *a11, *a12;
    a = p->Add("a");
    fprintf(stderr, "a\n");
    b = p->Add("b");
    fprintf(stderr, "b\n");
    a1 = a->Add("1");
    fprintf(stderr, "...\n");
    a2 = a->Add("2");
    fprintf(stderr, "...\n");
    a11 = a1->Add("1");
    fprintf(stderr, "...\n");
    a12 = a1->Add("2");
    fprintf(stderr, "...\n");
    b1 = b->Add("1");
    fprintf(stderr, "...\n");
    b2 = b->Add("2");
    fprintf(stderr, "...\n");

    if((Tok("a^1").Node() != a1)||(Tok("a").Node() != a)||(Tok("b").Node() != b)||(Tok("a^2").Node() != a2)
	||(Tok("b^1").Node() != b1)||(Tok("b^2").Node() != b2)||(Tok("a^1^1").Node() != a11)||(Tok("a^1^2").Node() != a12))
    {
	PNL_THROW(pnl::CAlgorithmicException, "Can't find ndo");
    }
}

PNL_USING
using namespace std;


void TestMultivariateWrapperJPD()
{
    int i;
    vector<CEvidence*> evidences;
    CBNet *pBNet = NULL;
    pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML("./TestMultivariate/5Nodes.xml", &evidences));
    if (!pBNet)
    {
	pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML("../tests/TestMultivariate/5Nodes.xml", &evidences));
    }

    CEvidence *empty = CEvidence::Create(pBNet, 0, NULL, valueVector(0));

    const int nodes1[] = { 0 };
    const int nodes2[] = { 1 };
    const int nodes3[] = { 2 };
    const int nodes4[] = { 3 };
    const int nodes5[] = { 4 };
    const int nodes6[] = { 0, 1, 2 };
    const int nodes7[] = { 1, 3 };
    const int nodes8[] = { 3, 4 };

    const int *nodes[] = {nodes1, nodes2, nodes3, nodes4, nodes5, nodes6, nodes7, nodes8 };

    // Pearl Inference JPD
    printf("\n\n\nJPD Test (Pearl Inference, no evidences)\n");
    CPearlInfEngine *perlInfEng = CPearlInfEngine::Create(pBNet);
    perlInfEng->EnterEvidence(empty);

    /*  perlInfEng->MarginalNodes(nodes2, 1);
    printf("JPD y\n");
    perlInfEng->GetQueryJPD()->Dump();
    */
    for (i = 0; i <  8; i++) 
    {
	int NNumber = 1;

	if (i == 5) NNumber = 3;

	if (i > 5) NNumber = 2;

	perlInfEng->MarginalNodes(nodes[i], NNumber);
	printf("JPD %d\n", i);
	perlInfEng->GetQueryJPD()->Dump();
    }

    printf("\n\n\nJPD Test (Pearl Inference, Evidences: x^dim1 = -15  x^dim2 = 5 z = 15.67)\n");
    int ObsNodes[] = {1, 4};
    valueVector obsValues(3);
    obsValues[0].SetFlt(-15.0);
    obsValues[1].SetFlt(5.0);
    obsValues[2].SetFlt(15.67);
    CEvidence *pEvid = CEvidence::Create(pBNet, 2, ObsNodes, obsValues);
    perlInfEng = CPearlInfEngine::Create(pBNet);
    perlInfEng->EnterEvidence(pEvid);

    for (i = 0; i <  8; i++) 
    {
	int NNumber = 1;

	if (i == 5) NNumber = 3;

	if (i > 5) NNumber = 2;

	perlInfEng->MarginalNodes(nodes[i], NNumber);
	printf("JPD %d\n", i);
	perlInfEng->GetQueryJPD()->Dump();
    }

    delete pBNet;
    for (i = 0; i<evidences.size(); i++)
    {
	delete evidences[i];
    };

}


void TestMultivariateWrapperLearn()
{
    vector<CEvidence*> evidences;
    CBNet *pBNet = NULL;

    pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML("./TestMultivariate/5Nodes.xml", &evidences));
    if (!pBNet)
    {
	pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML("../tests/TestMultivariate/5Nodes.xml", &evidences));
    }

    int nSamples = 20;  
    pEvidencesVector evidBnet(nSamples);
    intVector obsNodes(5);
    obsNodes[0] = 0;
    obsNodes[1] = 1;
    obsNodes[2] = 2;
    obsNodes[3] = 3;
    obsNodes[4] = 4;

    valueVector obsValues(10);

    for (int ev = 0; ev < nSamples; ev++)
    {
	switch (ev)
	{
	case 0:
	    obsValues[0].SetFlt(-1.314024);
	    obsValues[1].SetFlt(2.792187);
	    obsValues[2].SetFlt(2.959786);
	    obsValues[3].SetFlt(0.586695);
	    obsValues[4].SetFlt(3.930957);
	    obsValues[5].SetFlt(3.473877);
	    obsValues[6].SetFlt(13.123615);
	    obsValues[7].SetFlt(18.016819);
	    obsValues[8].SetFlt(22.066160);
	    obsValues[9].SetFlt(142.535583);
	    break;
	case 1:
	    obsValues[0].SetFlt(1.516309);
	    obsValues[1].SetFlt(-2.071994);
	    obsValues[2].SetFlt(1.431869);
	    obsValues[3].SetFlt(0.513529);
	    obsValues[4].SetFlt(1.555178);
	    obsValues[5].SetFlt(2.488395);
	    obsValues[6].SetFlt(7.941903);
	    obsValues[7].SetFlt(7.503506);
	    obsValues[8].SetFlt(12.018921);
	    obsValues[9].SetFlt(74.253006);
	    break;
	case 2:
	    obsValues[0].SetFlt(0.772414);
	    obsValues[1].SetFlt(1.280176);
	    obsValues[2].SetFlt(2.784338);
	    obsValues[3].SetFlt(0.592082);
	    obsValues[4].SetFlt(0.847529);
	    obsValues[5].SetFlt(4.232452);
	    obsValues[6].SetFlt(8.659535);
	    obsValues[7].SetFlt(12.584901);
	    obsValues[8].SetFlt(17.584988);
	    obsValues[9].SetFlt(108.877640);
	    break;
	case 3:
	    obsValues[0].SetFlt(	2.969814	);
	    obsValues[1].SetFlt(	0.690449	);
	    obsValues[2].SetFlt(	-0.161992	);
	    obsValues[3].SetFlt(	2.977564	);
	    obsValues[4].SetFlt(	1.232187	);
	    obsValues[5].SetFlt(	3.291015	);
	    obsValues[6].SetFlt(	8.872681	);
	    obsValues[7].SetFlt(	15.076061	);
	    obsValues[8].SetFlt(	18.509748	);
	    obsValues[9].SetFlt(	115.510925	);
	    break;
	case 4:
	    obsValues[0].SetFlt(	1.951843	);
	    obsValues[1].SetFlt(	-0.308100	);
	    obsValues[2].SetFlt(	1.074400	);
	    obsValues[3].SetFlt(	0.251534	);
	    obsValues[4].SetFlt(	3.094074	);
	    obsValues[5].SetFlt(	1.402124	);
	    obsValues[6].SetFlt(	7.841857	);
	    obsValues[7].SetFlt(	11.138101	);
	    obsValues[8].SetFlt(	12.521315	);
	    obsValues[9].SetFlt(	84.961678	);
	    break;
	case 5:
	    obsValues[0].SetFlt(	0.747681	);
	    obsValues[1].SetFlt(	1.348657	);
	    obsValues[2].SetFlt(	1.818290	);
	    obsValues[3].SetFlt(	-0.420393	);
	    obsValues[4].SetFlt(	1.441154	);
	    obsValues[5].SetFlt(	1.523428	);
	    obsValues[6].SetFlt(	5.019363	);
	    obsValues[7].SetFlt(	7.473093	);
	    obsValues[8].SetFlt(	11.312609	);
	    obsValues[9].SetFlt(	67.377525	);

	    break;
	case 6:
	    obsValues[0].SetFlt(	0.312565	);
	    obsValues[1].SetFlt(	0.441745	);
	    obsValues[2].SetFlt(	1.035443	);
	    obsValues[3].SetFlt(	0.038601	);
	    obsValues[4].SetFlt(	1.209402	);
	    obsValues[5].SetFlt(	1.733555	);
	    obsValues[6].SetFlt(	5.187507	);
	    obsValues[7].SetFlt(	6.917145	);
	    obsValues[8].SetFlt(	9.996355	);
	    obsValues[9].SetFlt(	61.404808	);
	    break;
	case 7:
	    obsValues[0].SetFlt(	2.718882	);
	    obsValues[1].SetFlt(	-0.785555	);
	    obsValues[2].SetFlt(	3.049016	);
	    obsValues[3].SetFlt(	1.716233	);
	    obsValues[4].SetFlt(	-0.760518	);
	    obsValues[5].SetFlt(	2.427294	);
	    obsValues[6].SetFlt(	5.681553	);
	    obsValues[7].SetFlt(	8.862487	);
	    obsValues[8].SetFlt(	12.503481	);
	    obsValues[9].SetFlt(	74.564087	);
	    break;
	case 8:
	    obsValues[0].SetFlt(	-1.121321	);
	    obsValues[1].SetFlt(	1.733628	);
	    obsValues[2].SetFlt(	1.306383	);
	    obsValues[3].SetFlt(	0.293589	);
	    obsValues[4].SetFlt(	0.543559	);
	    obsValues[5].SetFlt(	3.344014	);
	    obsValues[6].SetFlt(	6.040576	);
	    obsValues[7].SetFlt(	9.374549	);
	    obsValues[8].SetFlt(	13.150803	);
	    obsValues[9].SetFlt(	80.041092	);
	    break;
	case 9:
	    obsValues[0].SetFlt(	-0.360260	);
	    obsValues[1].SetFlt(	1.217422	);
	    obsValues[2].SetFlt(	2.224474	);
	    obsValues[3].SetFlt(	0.466464	);
	    obsValues[4].SetFlt(	1.284630	);
	    obsValues[5].SetFlt(	1.706728	);
	    obsValues[6].SetFlt(	9.596039	);
	    obsValues[7].SetFlt(	10.672682	);
	    obsValues[8].SetFlt(	13.843320	);
	    obsValues[9].SetFlt(	90.556984	);
	    break;
	case 10:
	    obsValues[0].SetFlt(	1.109505	);
	    obsValues[1].SetFlt(	-0.030694	);
	    obsValues[2].SetFlt(	1.439583	);
	    obsValues[3].SetFlt(	1.614625	);
	    obsValues[4].SetFlt(	2.275853	);
	    obsValues[5].SetFlt(	2.585066	);
	    obsValues[6].SetFlt(	10.824589	);
	    obsValues[7].SetFlt(	12.524777	);
	    obsValues[8].SetFlt(	16.171900	);
	    obsValues[9].SetFlt(	101.475403	);
	    break;
	case 11:
	    obsValues[0].SetFlt(	-0.391896	);
	    obsValues[1].SetFlt(	1.590519	);
	    obsValues[2].SetFlt(	3.769660	);
	    obsValues[3].SetFlt(	1.700410	);
	    obsValues[4].SetFlt(	-1.098416	);
	    obsValues[5].SetFlt(	2.047136	);
	    obsValues[6].SetFlt(	9.699523	);
	    obsValues[7].SetFlt(	11.976645	);
	    obsValues[8].SetFlt(	12.292547	);
	    obsValues[9].SetFlt(	86.394669	);
	    break;
	case 12:
	    obsValues[0].SetFlt(	2.213909	);
	    obsValues[1].SetFlt(	-0.018399	);
	    obsValues[2].SetFlt(	1.968136	);
	    obsValues[3].SetFlt(	0.594479	);
	    obsValues[4].SetFlt(	0.980654	);
	    obsValues[5].SetFlt(	2.739914	);
	    obsValues[6].SetFlt(	7.519217	);
	    obsValues[7].SetFlt(	9.284959	);
	    obsValues[8].SetFlt(	12.982949	);
	    obsValues[9].SetFlt(	82.558670	);
	    break;
	case 13:
	    obsValues[0].SetFlt(	0.906269	);
	    obsValues[1].SetFlt(	-0.191971	);
	    obsValues[2].SetFlt(	1.248619	);
	    obsValues[3].SetFlt(	1.601107	);
	    obsValues[4].SetFlt(	1.852811	);
	    obsValues[5].SetFlt(	1.027700	);
	    obsValues[6].SetFlt(	5.874196	);
	    obsValues[7].SetFlt(	10.684824	);
	    obsValues[8].SetFlt(	12.331678	);
	    obsValues[9].SetFlt(	79.168434	);
	    break;
	case 14:
	    obsValues[0].SetFlt(	-0.363741	);
	    obsValues[1].SetFlt(	-0.154812	);
	    obsValues[2].SetFlt(	2.758295	);
	    obsValues[3].SetFlt(	0.883432	);
	    obsValues[4].SetFlt(	0.878577	);
	    obsValues[5].SetFlt(	1.137673	);
	    obsValues[6].SetFlt(	4.925128	);
	    obsValues[7].SetFlt(	7.360548	);
	    obsValues[8].SetFlt(	10.508783	);
	    obsValues[9].SetFlt(	64.697655	);
	    break;
	case 15:
	    obsValues[0].SetFlt(	-0.444381	);
	    obsValues[1].SetFlt(	-0.658948	);
	    obsValues[2].SetFlt(	2.038244	);
	    obsValues[3].SetFlt(	-0.678944	);
	    obsValues[4].SetFlt(	0.299459	);
	    obsValues[5].SetFlt(	0.842612	);
	    obsValues[6].SetFlt(	3.658324	);
	    obsValues[7].SetFlt(	3.702670	);
	    obsValues[8].SetFlt(	6.275494	);
	    obsValues[9].SetFlt(	39.931580	);
	    break;
	case 16:
	    obsValues[0].SetFlt(	0.380659	);
	    obsValues[1].SetFlt(	-0.114203	);
	    obsValues[2].SetFlt(	1.262064	);
	    obsValues[3].SetFlt(	0.992305	);
	    obsValues[4].SetFlt(	2.888480	);
	    obsValues[5].SetFlt(	3.225816	);
	    obsValues[6].SetFlt(	10.485537	);
	    obsValues[7].SetFlt(	16.212713	);
	    obsValues[8].SetFlt(	17.348351	);
	    obsValues[9].SetFlt(	116.609734	);
	    break;
	case 17:
	    obsValues[0].SetFlt(	1.728963	);
	    obsValues[1].SetFlt(	1.383902	);
	    obsValues[2].SetFlt(	1.616821	);
	    obsValues[3].SetFlt(	0.478158	);
	    obsValues[4].SetFlt(	-0.505311	);
	    obsValues[5].SetFlt(	2.282217	);
	    obsValues[6].SetFlt(	6.962461	);
	    obsValues[7].SetFlt(	10.439819	);
	    obsValues[8].SetFlt(	10.104074	);
	    obsValues[9].SetFlt(	75.615761	);
	    break;
	case 18:
	    obsValues[0].SetFlt(	0.776983	);
	    obsValues[1].SetFlt(	1.884337	);
	    obsValues[2].SetFlt(	1.044936	);
	    obsValues[3].SetFlt(	-1.312427	);
	    obsValues[4].SetFlt(	0.476133	);
	    obsValues[5].SetFlt(	3.073510	);
	    obsValues[6].SetFlt(	0.751903	);
	    obsValues[7].SetFlt(	6.565332	);
	    obsValues[8].SetFlt(	10.350166	);
	    obsValues[9].SetFlt(	56.209145	);
	    break;
	case 19:
	    obsValues[0].SetFlt(	-0.713351	);
	    obsValues[1].SetFlt(	0.111894	);
	    obsValues[2].SetFlt(	1.480657	);
	    obsValues[3].SetFlt(	0.156705	);
	    obsValues[4].SetFlt(	2.330819	);
	    obsValues[5].SetFlt(	3.500447	);
	    obsValues[6].SetFlt(	11.273463	);
	    obsValues[7].SetFlt(	10.621621	);
	    obsValues[8].SetFlt(	14.949851	);
	    obsValues[9].SetFlt(	96.590942	);
	    break;

	}

	evidBnet[ev] = pnl::CEvidence::Create(pBNet, obsNodes, obsValues);
    }


    pnl::CEMLearningEngine *m_Learning = pnl::CEMLearningEngine::Create(pBNet);

    m_Learning->SetData( nSamples, &evidBnet.front() );
    m_Learning->SetMaxIterEM(10);

    m_Learning->Learn();

    for( int i = 0; i < 5; i++ )
    {
	std::cout<<"\n ___ node "<<i<<"_________________________\n";
	std::cout<<"\n____ BNet_________________________________\n";
	pBNet->GetFactor(i)->GetDistribFun()->Dump();
    }
}

void TestMultivariateWrapperLearnPartOfObservation()
{
    vector<CEvidence*> evidences;
    CBNet *pBNet = NULL;

    pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML("./TestMultivariate/5Nodes.xml", &evidences));
    if (!pBNet)
    {
	pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML("../tests/TestMultivariate/5Nodes.xml", &evidences));
    }

    int nSamples = 20;  
    pEvidencesVector evidBnet(nSamples);
    intVector obsNodes(3);
    obsNodes[0] = 0;
    obsNodes[1] = 2;
    obsNodes[2] = 3;


    valueVector obsValues(7);

    for (int ev = 0; ev < nSamples; ev++)
    {
	switch (ev)
	{
	case 0:
	    obsValues[0].SetFlt(-1.314024);
	    obsValues[1].SetFlt(0.586695);
	    obsValues[2].SetFlt(3.930957);
	    obsValues[3].SetFlt(3.473877);
	    obsValues[4].SetFlt(13.123615);
	    obsValues[5].SetFlt(18.016819);
	    obsValues[6].SetFlt(22.066160);
	    break;
	case 1:
	    obsValues[0].SetFlt(1.516309);
	    obsValues[1].SetFlt(0.513529);
	    obsValues[2].SetFlt(1.555178);
	    obsValues[3].SetFlt(2.488395);
	    obsValues[4].SetFlt(7.941903);
	    obsValues[5].SetFlt(7.503506);
	    obsValues[6].SetFlt(12.018921);
	    break;
	case 2:
	    obsValues[0].SetFlt(0.772414);
	    obsValues[1].SetFlt(0.592082);
	    obsValues[2].SetFlt(0.847529);
	    obsValues[3].SetFlt(4.232452);
	    obsValues[4].SetFlt(8.659535);
	    obsValues[5].SetFlt(12.584901);
	    obsValues[6].SetFlt(17.584988);
	    break;
	case 3:
	    obsValues[0].SetFlt(	2.969814	);
	    obsValues[1].SetFlt(	2.977564	);
	    obsValues[2].SetFlt(	1.232187	);
	    obsValues[3].SetFlt(	3.291015	);
	    obsValues[4].SetFlt(	8.872681	);
	    obsValues[5].SetFlt(	15.076061	);
	    obsValues[6].SetFlt(	18.509748	);
	    break;
	case 4:
	    obsValues[0].SetFlt(	1.951843	);
	    obsValues[1].SetFlt(	0.251534	);
	    obsValues[2].SetFlt(	3.094074	);
	    obsValues[3].SetFlt(	1.402124	);
	    obsValues[4].SetFlt(	7.841857	);
	    obsValues[5].SetFlt(	11.138101	);
	    obsValues[6].SetFlt(	12.521315	);
	    break;
	case 5:
	    obsValues[0].SetFlt(	0.747681	);
	    obsValues[1].SetFlt(	-0.420393	);
	    obsValues[2].SetFlt(	1.441154	);
	    obsValues[3].SetFlt(	1.523428	);
	    obsValues[4].SetFlt(	5.019363	);
	    obsValues[5].SetFlt(	7.473093	);
	    obsValues[6].SetFlt(	11.312609	);
	    break;
	case 6:
	    obsValues[0].SetFlt(	0.312565	);
	    obsValues[1].SetFlt(	0.038601	);
	    obsValues[2].SetFlt(	1.209402	);
	    obsValues[3].SetFlt(	1.733555	);
	    obsValues[4].SetFlt(	5.187507	);
	    obsValues[5].SetFlt(	6.917145	);
	    obsValues[6].SetFlt(	9.996355	);
	    break;
	case 7:
	    obsValues[0].SetFlt(	2.718882	);
	    obsValues[1].SetFlt(	1.716233	);
	    obsValues[2].SetFlt(	-0.760518	);
	    obsValues[3].SetFlt(	2.427294	);
	    obsValues[4].SetFlt(	5.681553	);
	    obsValues[5].SetFlt(	8.862487	);
	    obsValues[6].SetFlt(	12.503481	);
	    break;
	case 8:
	    obsValues[0].SetFlt(	-1.121321	);
	    obsValues[1].SetFlt(	0.293589	);
	    obsValues[2].SetFlt(	0.543559	);
	    obsValues[3].SetFlt(	3.344014	);
	    obsValues[4].SetFlt(	6.040576	);
	    obsValues[5].SetFlt(	9.374549	);
	    obsValues[6].SetFlt(	13.150803	);
	    break;
	case 9:
	    obsValues[0].SetFlt(	-0.360260	);
	    obsValues[1].SetFlt(	0.466464	);
	    obsValues[2].SetFlt(	1.284630	);
	    obsValues[3].SetFlt(	1.706728	);
	    obsValues[4].SetFlt(	9.596039	);
	    obsValues[5].SetFlt(	10.672682	);
	    obsValues[6].SetFlt(	13.843320	);
	    break;
	case 10:
	    obsValues[0].SetFlt(	1.109505	);
	    obsValues[1].SetFlt(	1.614625	);
	    obsValues[2].SetFlt(	2.275853	);
	    obsValues[3].SetFlt(	2.585066	);
	    obsValues[4].SetFlt(	10.824589	);
	    obsValues[5].SetFlt(	12.524777	);
	    obsValues[6].SetFlt(	16.171900	);
	    break;
	case 11:
	    obsValues[0].SetFlt(	-0.391896	);
	    obsValues[1].SetFlt(	1.700410	);
	    obsValues[2].SetFlt(	-1.098416	);
	    obsValues[3].SetFlt(	2.047136	);
	    obsValues[4].SetFlt(	9.699523	);
	    obsValues[5].SetFlt(	11.976645	);
	    obsValues[6].SetFlt(	12.292547	);
	    break;
	case 12:
	    obsValues[0].SetFlt(	2.213909	);
	    obsValues[1].SetFlt(	0.594479	);
	    obsValues[2].SetFlt(	0.980654	);
	    obsValues[3].SetFlt(	2.739914	);
	    obsValues[4].SetFlt(	7.519217	);
	    obsValues[5].SetFlt(	9.284959	);
	    obsValues[6].SetFlt(	12.982949	);
	    break;
	case 13:
	    obsValues[0].SetFlt(	0.906269	);
	    obsValues[1].SetFlt(	1.601107	);
	    obsValues[2].SetFlt(	1.852811	);
	    obsValues[3].SetFlt(	1.027700	);
	    obsValues[4].SetFlt(	5.874196	);
	    obsValues[5].SetFlt(	10.684824	);
	    obsValues[6].SetFlt(	12.331678	);
	    break;
	case 14:
	    obsValues[0].SetFlt(	-0.363741	);
	    obsValues[1].SetFlt(	0.883432	);
	    obsValues[2].SetFlt(	0.878577	);
	    obsValues[3].SetFlt(	1.137673	);
	    obsValues[4].SetFlt(	4.925128	);
	    obsValues[5].SetFlt(	7.360548	);
	    obsValues[6].SetFlt(	10.508783	);
	    break;
	case 15:
	    obsValues[0].SetFlt(	-0.444381	);
	    obsValues[1].SetFlt(	-0.678944	);
	    obsValues[2].SetFlt(	0.299459	);
	    obsValues[3].SetFlt(	0.842612	);
	    obsValues[4].SetFlt(	3.658324	);
	    obsValues[5].SetFlt(	3.702670	);
	    obsValues[6].SetFlt(	6.275494	);
	    break;
	case 16:
	    obsValues[0].SetFlt(	0.380659	);
	    obsValues[1].SetFlt(	0.992305	);
	    obsValues[2].SetFlt(	2.888480	);
	    obsValues[3].SetFlt(	3.225816	);
	    obsValues[4].SetFlt(	10.485537	);
	    obsValues[5].SetFlt(	16.212713	);
	    obsValues[6].SetFlt(	17.348351	);
	    break;
	case 17:
	    obsValues[0].SetFlt(	1.728963	);
	    obsValues[1].SetFlt(	0.478158	);
	    obsValues[2].SetFlt(	-0.505311	);
	    obsValues[3].SetFlt(	2.282217	);
	    obsValues[4].SetFlt(	6.962461	);
	    obsValues[5].SetFlt(	10.439819	);
	    obsValues[6].SetFlt(	10.104074	);
	    break;
	case 18:
	    obsValues[0].SetFlt(	0.776983	);
	    obsValues[1].SetFlt(	-1.312427	);
	    obsValues[2].SetFlt(	0.476133	);
	    obsValues[3].SetFlt(	3.073510	);
	    obsValues[4].SetFlt(	0.751903	);
	    obsValues[5].SetFlt(	6.565332	);
	    obsValues[6].SetFlt(	10.350166	);
	    break;
	case 19:
	    obsValues[0].SetFlt(	-0.713351	);
	    obsValues[1].SetFlt(	0.156705	);
	    obsValues[2].SetFlt(	2.330819	);
	    obsValues[3].SetFlt(	3.500447	);
	    obsValues[4].SetFlt(	11.273463	);
	    obsValues[5].SetFlt(	10.621621	);
	    obsValues[6].SetFlt(	14.949851	);
	    break;
	}

	evidBnet[ev] = pnl::CEvidence::Create(pBNet, obsNodes, obsValues);
    }


    pnl::CEMLearningEngine *m_Learning = pnl::CEMLearningEngine::Create(pBNet);

    m_Learning->SetData( nSamples, &evidBnet.front() );
    m_Learning->SetMaxIterEM(10);

    m_Learning->Learn();

    for( int i = 0; i < 5; i++ )
    {
	std::cout<<"\n ___ node "<<i<<"_________________________\n";
	std::cout<<"\n____ BNet_________________________________\n";
	pBNet->GetFactor(i)->GetDistribFun()->Dump();
    }
}

void TestMultivariateWrapperMPE()
{
    vector<CEvidence*> evidences;
    CBNet *pBNet = NULL;

    pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML("./TestMultivariate/5Nodes.xml", &evidences));
    if (!pBNet)
    {
	pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML("../tests/TestMultivariate/5Nodes.xml", &evidences));
    }

    const int nodes1[] = { 0 };
    const int nodes2[] = { 1 };
    const int nodes3[] = { 2 };
    const int nodes4[] = { 3 };
    const int nodes5[] = { 4 };
    const int nodes6[] = { 0, 1, 2 };
    const int nodes7[] = { 1, 3 };
    const int nodes8[] = { 3, 4 };

    const int *nodes[] = {nodes1, nodes2, nodes3, nodes4, nodes5, nodes6, nodes7, nodes8 };

    printf("\n\n\nJPD Test (Pearl Inference, Evidences: x^dim1 = -15  x^dim2 = 5 z = 15.67)\n");
    int ObsNodes[] = {1, 4};
    valueVector obsValues(3);
    obsValues[0].SetFlt(-15.0);
    obsValues[1].SetFlt(5.0);
    obsValues[2].SetFlt(15.67);
    CEvidence *pEvid = CEvidence::Create(pBNet, 2, ObsNodes, obsValues);
    CPearlInfEngine *perlInfEng = CPearlInfEngine::Create(pBNet);
    perlInfEng = CPearlInfEngine::Create(pBNet);
    perlInfEng->EnterEvidence(pEvid, true);

    for (int i = 0; i <  8; i++) 
    {
	int NNumber = 1;

	if (i == 5) NNumber = 3;

	if (i > 5) NNumber = 2;

	perlInfEng->MarginalNodes(nodes[i], NNumber);
	printf("JPD %d\n", i);
	perlInfEng->GetMPE()->Dump();
    }
}

bool testExtractTok()
{
    BayesNet net;
    TokArr aChoice = "value0 value1 value2";// possible values for nodes

    net.AddNode(discrete ^ "node0", aChoice);
    net.AddNode(discrete ^ "node1", aChoice);
    net.AddNode(discrete ^ "node2", aChoice);
    net.AddNode(discrete ^ "node3", aChoice);
    net.AddNode(discrete ^ "node4", aChoice);

    net.AddArc("node0", "node2");
    net.AddArc("node1", "node2");
    net.AddArc("node2", "node3");
    net.AddArc("node3", "node4");

    Tok values = "node0^value1^node2^value0^node3^value1";
    //    net.Net().Token()->Resolve(values);
    Vector<TokIdNode *> nodes = values.Nodes();
    for(int i = 0; i < nodes.size(); i++)
    {
	if(nodes[i]->tag == eTagValue)
	{
	    nodes[i] = nodes[i]->v_prev;
	}
	if(nodes[i]->tag != eTagNetNode)
	{
	    ThrowUsingError("There is must be node", "testExtractTok");
	}
    }
    if(net.Net().Graph().INode(nodes[0]->Name()) != 0 ||
	net.Net().Graph().INode(nodes[1]->Name()) != 2 || 
	net.Net().Graph().INode(nodes[2]->Name()) != 3)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Extracted node isn't right");
    }
    cout << "testExtractTok is completed successfully" << endl;

    return true;
}

//#define PRINT_TestGibbsRecreate

void TestsPnlHigh::TestGibbsRecreate()
{
    BayesNet *net = GetSimpleTabularModel();
    // Zero       One
    //   |         |
    //   |         |
    //   o->Three<-o

    net->SetProperty("Inference", "gibbs");
    net->SetProperty("GibbsNumberOfIterations", "5000");
    net->SetProperty("GibbsNumberOfStreams", "2");
    net->SetProperty("GibbsThresholdIteration", "100");

    //1. Test many requests 
    #ifndef PRINT_TestGibbsRecreate
	net->GetJPD("Zero");
	net->GetJPD("One");
	net->GetJPD("Three");
    #else
	std::cout << String(net->GetJPD("Zero")) << "\n";
	std::cout << String(net->GetJPD("One")) << "\n";
	std::cout << String(net->GetJPD("Three")) << "\n";	
    #endif

    //2. Test set values
    if (dynamic_cast<CGibbsSamplingInfEngine *>(net->m_Inference)->GetMaxTime() != 5000)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in Inference() for gibbs case");
    };
    if (dynamic_cast<CGibbsSamplingInfEngine *>(net->m_Inference)->GetBurnIn() != 100)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in Inference() for gibbs case");
    };
    if (const_cast<const CSamplingInfEngine *>(dynamic_cast<CSamplingInfEngine *>(net->m_Inference) )->GetNumStreams() != 2)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in Inference() for gibbs case");
    };

    net->SetProperty("Inference", "jtree");

    #ifndef PRINT_TestGibbsRecreate
	net->GetJPD("Zero");
	net->GetJPD("One");
	net->GetJPD("Three");
    #else
	std::cout << String(net->GetJPD("Zero")) << "\n";
	std::cout << String(net->GetJPD("One")) << "\n";
	std::cout << String(net->GetJPD("Three")) << "\n";	
    #endif

    net->SetProperty("Inference", "gibbs");

    #ifndef PRINT_TestGibbsRecreate
	net->GetJPD("Zero");
	net->GetJPD("One");
	net->GetJPD("Three");
    #else
	std::cout << String(net->GetJPD("Zero")) << "\n";
	std::cout << String(net->GetJPD("One")) << "\n";
	std::cout << String(net->GetJPD("Three")) << "\n";	
    #endif

    if (dynamic_cast<CGibbsSamplingInfEngine *>(net->m_Inference)->GetMaxTime() != 5000)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in Inference() for gibbs case");
    };
    if (dynamic_cast<CGibbsSamplingInfEngine *>(net->m_Inference)->GetBurnIn() != 100)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in Inference() for gibbs case");
    };
    if (const_cast<const CSamplingInfEngine *>(dynamic_cast<CSamplingInfEngine *>(net->m_Inference) )->GetNumStreams() != 2)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in Inference() for gibbs case");
    };

    cout << "TestGibbsRecreate is completed successfully" << endl;

    delete net;
}

int testDelArc()
{
	int res = TRS_OK;
	//Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
	net.AddArc("PreviousHumanTurn","PreviousCompTurn");
	//Deleting arc
	net.DelArc("PreviousHumanTurn","PreviousCompTurn");
	//Adding evidences
	net.AddEvidToBuf("PreviousHumanTurn^Rock");
	//JPD Request
	net.GetJPD("CurrentHumanTurn");
	return res;
}

int testNodeNeigborsRequests()
{
    int res = TRS_OK;
	//Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");

    TokArr n0c = net.GetChildren("PreviousCompTurn");
    TokArr n1c = net.GetChildren("PreviousHumanTurn");
    TokArr n2c = net.GetChildren("CurrentHumanTurn");  

//#ifdef PRINT_RESULT
    printf("\nChildren\n");
    printf("%s\n",String(n0c).c_str());
    printf("%s\n",String(n1c).c_str());
    printf("%s\n",String(n2c).c_str());
//#endif    
    
    TokArr n0p = net.GetParents("PreviousCompTurn");
    TokArr n1p = net.GetParents("PreviousHumanTurn");
    TokArr n2p = net.GetParents("CurrentHumanTurn");  

    printf("\nParents\n");
    printf("%s\n",String(n0p).c_str());
    printf("%s\n",String(n1p).c_str());
    printf("%s\n",String(n2p).c_str());   

    TokArr n0n = net.GetNeighbors("PreviousCompTurn");
    TokArr n1n = net.GetNeighbors("PreviousHumanTurn");
    TokArr n2n = net.GetNeighbors("CurrentHumanTurn");  

    printf("\nNeighbors\n");
    printf("%s\n",String(n0n).c_str());
    printf("%s\n",String(n1n).c_str());
    printf("%s\n",String(n2n).c_str());
   
    return res;
}

int testSaveLoad()
{
    int res = TRS_OK;
	//Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
	net.SaveNet("TempNet.csv");
	net.LoadNet("TempNet.csv");
    //Adding evidences
	net.AddEvidToBuf("PreviousHumanTurn^Rock");
	//JPD Request
	net.GetJPD("CurrentHumanTurn");
	return res;
}

int testGetProb()
{
    int res = TRS_OK;
	//Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
	
	//Specify discributions;
	net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
	net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
	net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
  	net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    
	//Distributions requests
    TokArr pctr = net.GetPTabular("PreviousCompTurn^Rock");
	TokArr pctp = net.GetPTabular("PreviousCompTurn^Paper");
	TokArr pcts = net.GetPTabular("PreviousCompTurn^Scissors");
    
	printf("\nPreviousCompTurn node distributions\n");
    printf("Rock: %s\n",String(pctr).c_str());
    printf("Paper: %s\n",String(pctp).c_str());
    printf("Scissors: %s\n",String(pcts).c_str());

	TokArr phtr = net.GetPTabular("PreviousHumanTurn^Rock");
	TokArr phtp = net.GetPTabular("PreviousHumanTurn^Paper");
	TokArr phts = net.GetPTabular("PreviousHumanTurn^Scissors");
    
	printf("\nPreviousHumanTurn node distributions\n");
    printf("Rock: %s\n",String(phtr).c_str());
    printf("Paper: %s\n",String(phtp).c_str());
    printf("Scissors: %s\n",String(phts).c_str());

    TokArr chtr = net.GetPTabular("CurrentHumanTurn^Rock","PreviousHumanTurn^Rock PreviousCompTurn^Rock");
	TokArr chtp = net.GetPTabular("CurrentHumanTurn^Paper","PreviousHumanTurn^Rock PreviousCompTurn^Rock");
	TokArr chts = net.GetPTabular("CurrentHumanTurn^Scissors","PreviousHumanTurn^Rock PreviousCompTurn^Paper");
	TokArr chtp1 = net.GetPTabular("CurrentHumanTurn^Paper","PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    
	printf("\nCurrentHumanTurn node distributions\n");
    printf("%s\n",String(phtr).c_str());
    printf("%s\n",String(phtp).c_str());
    printf("%s\n",String(phts).c_str());
    
	return res;
}

int testEvidencesManipulation()
{
    int res = TRS_OK;
	//Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
	//Evidences manipulation
	net.EditEvidence("PreviousHumanTurn^Rock");
	net.EditEvidence("CurrentHumanTurn^Paper PreviousHumanTurn^Scissors");
	net.ClearEvid();

	net.EditEvidence("PreviousHumanTurn^Rock");
	net.EditEvidence("CurrentHumanTurn^Paper PreviousHumanTurn^Scissors");
	net.CurEvidToBuf();
	net.ClearEvidBuf();

	net.EditEvidence("PreviousHumanTurn^Rock");
	net.EditEvidence("CurrentHumanTurn^Paper PreviousHumanTurn^Scissors");
	net.CurEvidToBuf();
	TokArr res1 = net.GetJPD("PreviousHumanTurn");
	net.ClearEvidBuf();
    
	net.AddEvidToBuf("PreviousHumanTurn^Rock");
	net.AddEvidToBuf("CurrentHumanTurn^Paper PreviousHumanTurn^Scissors");
	net.SaveEvidBuf("evbuff.csv");
	net.ClearEvidBuf();
	net.LoadEvidBuf("evbuff.csv");
	TokArr res2 = net.GetJPD("PreviousHumanTurn");
    
	printf("%s\n",String(res1).c_str());
    printf("%s\n",String(res2).c_str());
	
    return res;

}

int testNetProperties()
{
	int res = TRS_OK;
    BayesNet net;
	//Adding new network property
	net.SetProperty("date","19july2005");
	//Network proprty value request
	String value = net.GetProperty("date");
	printf("%s\n",value.c_str());
	return res;
}

int testInference()
{
    int res = TRS_OK;
	//Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

	net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");

	//Adding evidence
	net.AddEvidToBuf("CurrentHumanTurn^Paper PreviousHumanTurn^Scissors");
	//Inference requests
	//1. default algorithm
    String value1 = net.GetJPD("PreviousCompTurn");	
	printf("%s\n",value1.c_str());
    //2. Pearl 
	net.SetProperty("Inference","pearl");
    String value2 = net.GetJPD("PreviousCompTurn");
	printf("%s\n",value2.c_str());
	//3. Junction Tree 
	net.SetProperty("Inference","jtree");
    String value3 = net.GetJPD("PreviousCompTurn");
	printf("%s\n",value3.c_str());
	//4.Gibbs Sampling
	net.SetProperty("Inference","gibbs");
    String value4 = net.GetJPD("PreviousCompTurn");
	printf("%s\n",value4.c_str());
	//5. Naive
	net.SetProperty("Inference","naive");
    String value5 = net.GetJPD("PreviousCompTurn");
	printf("%s\n",value5.c_str());

	return res;

}

int testLearning()
{
    int res = TRS_OK;
	//Create a simple model
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

	net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");

	net.GenerateEvidences(100);
    // default algorithm
	net.LearnParameters();
	// EM algorithm
    net.SetProperty("Learning","em");
	net.LearnParameters();
    float emval = net.GetEMLearningCriterionValue();
    printf("%f\n",emval);
	// bayes algorithm
    net.SetProperty("Learning","bayes");
	net.LearnParameters();
	return res;
}

int testPNLObjectsRequests()
{
	int res = TRS_OK;
	
    BayesNet net;
    net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

	net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");

	net.AddEvidToBuf("CurrentHumanTurn^Paper PreviousHumanTurn^Scissors");

	pnl::CBNet& pnlNetwork = net.Model();
	pnl::CInfEngine& pnlInference = net.Inference();
	pnl::CLearningEngine& pnlLearning = net.Learning(); 
	pnl::CEvidence* pnlEvidence = net.GetPNLEvidence();

	return res;
}

int testStructuralLearning()
{
   	int res = TRS_OK;
	//Create a simple model
    BayesNet *net;
	net = new BayesNet();
    net->AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net->AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net->AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

	net->AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    
	//Specify discributions;
	net->SetPTabular("PreviousCompTurn^Rock","0.7");
	net->SetPTabular("PreviousCompTurn^Paper","0.2");

	net->SetPTabular("PreviousHumanTurn^Scissors","0.8");
	net->SetPTabular("PreviousHumanTurn^Paper","0.1");

	net->SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
	net->SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
	net->SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
  	net->SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
    
	//Generation observation
    net->GenerateEvidences(100);
	net->SaveEvidBuf("evsr.csv");
	delete net;

	//Creation new net
	BayesNet clearNet;
	clearNet.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    clearNet.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    clearNet.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    clearNet.LoadEvidBuf("evsr.csv");
    // 1. default learning
	clearNet.LearnStructure();
	// 2. using Maximized Likelyhood score method with BIC score function 
    clearNet.SetProperty("LearningStructureMethod","MaxLh");
    clearNet.SetProperty("LearningStructureScoreFun","BIC");
	clearNet.LearnStructure();
	// 3. using Maximized Likelyhood score method with AIC score function 
    clearNet.SetProperty("LearningStructureMethod","MaxLh");
    clearNet.SetProperty("LearningStructureScoreFun","AIC");
	clearNet.LearnStructure();
	// 4. using Predicting assesment score method with BIC score function 
    clearNet.SetProperty("LearningStructureMethod","PreAs");
    clearNet.SetProperty("LearningStructureScoreFun","BIC");
	clearNet.LearnStructure();
	// 5. using Predictive assesment score method with AIC score function 
    clearNet.SetProperty("LearningStructureMethod","PreAs");
    clearNet.SetProperty("LearningStructureScoreFun","AIC");
	clearNet.LearnStructure();
    // 6. using Marginal likelyhood score(BDe metric) method with Dirichlet priors
    clearNet.SetProperty("LearningStructureMethod","PreAs");
    clearNet.SetProperty("LearningStructureScoreFun","WithoutPenalty");
    clearNet.SetProperty("LearningStructurePrior","Dirichlet");
	clearNet.LearnStructure();
	// 7. using Marginal likelyhood score(BDe metric) method with K2 priors
    clearNet.SetProperty("LearningStructureMethod","PreAs");
    clearNet.SetProperty("LearningStructureScoreFun","WithoutPenalty");
    clearNet.SetProperty("LearningStructurePrior","K2");
	clearNet.LearnStructure();
	// 8. using Marginal likelyhood score(BDe metric) method with BDeu priors
    clearNet.SetProperty("LearningStructureMethod","PreAs");
    clearNet.SetProperty("LearningStructureScoreFun","WithoutPenalty");
    clearNet.SetProperty("LearningStructurePrior","BDeu");
	clearNet.LearnStructure();

    return res;
}

int testTwoNetsLoadEvidBuf()
{
   int res = TRS_OK;
	//Create a simple model
    BayesNet *net;
	net = new BayesNet();
    net->AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    net->AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    net->AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

	net->AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    
	//Generation observation
    net->GenerateEvidences(100);
	net->SaveEvidBuf("evsr1.csv");
	
	//Creation new net
	BayesNet clearNet;
	clearNet.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
    clearNet.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
    clearNet.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

    clearNet.LoadEvidBuf("evsr1.csv");// fall here

	delete net; 
	
	return res;
}

int testTokens()
{
	int res = TRS_OK;
	res = testExtractTok() & res; 
	res = testDelArc() & res; 
	res = testNodeNeigborsRequests() & res;
	res = testSaveLoad() & res;
	res = testGetProb() & res;
	res = testEvidencesManipulation() & res;
	res = testNetProperties() & res;
	res = testPNLObjectsRequests() & res;
	res = testInference() & res;
	res = testLearning() & res; 
	res = testStructuralLearning() & res;
	res = TestResolve1() & res;
	res = TestResolve2() & res;
	res = TestResolve3() & res;
	res = TestResolve4() & res;
	res = TestResolve4_5() & res;
//	res = testTwoNetsLoadEvidBuf() & res;// it is not work yet
//	
	try 
    {
        SimpleModel();
        TestMultivariateWrapperJPD();
        TestMultivariateWrapperLearn();
        TestMultivariateWrapperLearnPartOfObservation();
        //	TestMultivariateWrapperMPE();// wild test. It doesn't test wrapper!
    }
    catch(pnl::CException e)
    {
        std::cout << e.GetMessage();
        res = TRS_FAIL;
    }


	return trsResult( res, res == TRS_OK ? "No errors"
	: "Bad test on Tokens");
}

void initTestsTokens()
{
    trsReg(func_name, test_desc, test_class, testTokens);
}
