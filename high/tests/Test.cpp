#include <iostream>
#include <time.h>
#include "pnlTok.hpp"

#include "pnlHigh.hpp"
#include "console.hpp"

#include "pnlException.hpp"

#include "pnlRng.hpp"

using namespace std;

static int sMode = 0;
const int cShowPrediction = 1;
const int cShowMatrix = 2;

// This function computes the result of round for computer if human make turn
// denoted by variable 'human' and computer make turn denoted by variable
// 'computer'. Returned value is symbolic result. If 'result' variable is not
// null, it is filled with integer result. If 'best' variable is not null,
// it is filled with best turn for given human turn.
static const char *round(int *best, int *result, int human, int computer)
{
    static const char *saResult[] = { "Draw", "Lose", "Win" };
    
    if(result)
    {
	*result = ((human + 3) - computer) % 3;
    }
    if(best)
    {
	*best = (human + 1) % 3;
    }
    
    return saResult[((human + 3) - computer) % 3];
}

std::ostream &operator<<(std::ostream &str, TokArr &ta)
{
    return str << String(ta);
}

// used by ReadHumanResponse
static void reportExpectedResponse()
{
    fprintf(stderr, "'Rock', 'Paper' or 'Scissors' is expected\n");
    fprintf(stderr, "Word can be replaced with abbreviation ('r','p','s') or index (0,1,2)\n");
    fprintf(stderr, "Current mode is: ");
    if(sMode & cShowPrediction)
    {
	fprintf(stderr, "show_prediction ");
    }
    if(sMode & cShowMatrix)
    {
	fprintf(stderr, "show_matrix ");
    }
    else if(sMode == 0)
    {
	fprintf(stderr, "hide_all");
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "Mode toggling:\n\tmm - toggle show_matrix\n"
	"\tmp - toggle show_prediction\n");
}

// read from stdin human response
int ReadHumanResponse()
{
    // function prints usage information once per 8 calls
    static int count = 0;

    if((count++ % 8) == 0)
    {
	reportExpectedResponse();
    }

    while(true)
    {
	char buf[40];
	
	if(fgets(buf, sizeof(buf), stdin) != buf)
	{
	    return -1;
	}
	
	char *ptr = buf;
	int len = strlen(buf);
	int i;
	
	// Skip trailing and leading spaces
	while(isspace(*ptr))
	{
	    len--;
	    ptr++;
	}
	
	while(isspace(ptr[len - 1]))
	{
	    len--;
	}
	
	ptr[len] = 0;
	for(i = 0; i < len; ++i)
	{
	    ptr[i] = char(tolower(ptr[i]));
	}	    
	
	if(len > 3 && isalpha(*ptr))
	{
	    // read 'Rock', 'Paper' or 'Scissors'
	    std::string str(ptr);
	    if(str == "rock")
	    {
		return 0;
	    }
	    if(str == "paper")
	    {
		return 1;
	    }
	    if(str == "scissor" || str == "scissors")
	    {
		return 2;
	    }
	    reportExpectedResponse();
	    continue;
	}
	
	if(isdigit(*ptr))
	{
	    return atoi(ptr);
	}
	
	switch(*ptr)
	{
	case 'r': return 0;
	case 'p': return 1;
	case 's': return 2;
	case 'e': return -1;
	case 'm': 
	    switch(ptr[1])
	    {
	    case 'm': sMode ^= cShowMatrix; break;
	    case 'p': sMode ^= cShowPrediction; break;
	    default:  break;
	    }
	    // FALLTHROUGH
	default:
	    reportExpectedResponse();
	    break;
	}
    }
}

// This net is designed to predict human turn on previous computer turn
// and previous human turn
int scenario()
{
    // create model
    
    // prepare types
    BayesNet net;
   // BayesNet net2;
    
    TokArr aChoice = "False True";// possible values for nodes
    
    // build Graph
    // add nodes to net
    net.AddNode(discrete ^ "Predictor", aChoice);
    net.AddNode(discrete ^ "Response", aChoice);
    net.AddNode(discrete ^ "Response2", aChoice);

    net.AddArc("Predictor", "Response");
    net.AddArc("Predictor","Response2");
    
    // initial with uniform distribution
//    net.MakeUniformDistribution();

//    net.SetPTabular("Predictor^True", Tok(0.3f) );
//    net.SetPTabular("Predictor^False", Tok(0.7f) );


    
    
    


  //  net2.AddNode(discrete ^ "Predictor", aChoice);
  //  net2.AddNode(discrete ^ "Response", aChoice);
  //  net2.AddArc("Predictor", "Response");

    TokArr ev = (Tok("Predictor")^"True") & (Tok("Response")^"True") & (Tok("Response2")^"True");
//    cout << ev;

    net.EditEvidence(ev);
   // net.EditEvidence(Tok("Response")^"False");
   // net.EditEvidence(Tok("Response")^"True");
   // net.EditEvidence(Tok("Response")^"True");

    pnl::pnlSeed(time(0));
    //Scripting scr;
    //scr.Execute(stdin, &net);

    //TokArr whatNodes = TokArr("Predictor")&"Response";
    //TokArr whatNodes = (Tok("Predictor")^0.9f) & (Tok("Response")^0.5f);
    //Tok aa0 = 0.1f;
    //Tok aa1 = "predictor";
    //Tok aa2 = aa1 ^ aa0;
    //Tok aa3 = aa1 ^ 0.1f;
    //cout << "hehe " << aa0 << " o " << aa1 << " o " << aa2 << " o " << aa3 << endl;
    //cout << aa0.fload[0].fl << " " << aa0.fload[0].IsUndef() << endl;
    //cout << whatNodes <<endl;

//    net.GenerateEvidences( 100, true);//, whatNodes ); 

   
//    net.SaveLearnBuf("classif.csv");
//    net.LoadLearnBuf("classif.csv");
    

    BayesNet net2;

    net2.AddNode(discrete ^ "Response", aChoice);
    net2.AddNode(discrete ^ "Response2", aChoice);
    net2.AddNode(discrete ^ "Predictor", aChoice);
   
//    net2.MakeUniformDistribution();

    net.EditEvidence(ev);
 
    
    //net2.LoadLearnBuf("classif.csv");
    //net2.LearnStructure(0,0);
    //net2.SaveLearnBuf("classif2.csv");
   
    //net2.SaveNet("network.xml");


    return 0;
}

int rpsMain2()
{
    // create model
    
    // prepare types
    BayesNet net;
    int outcomes[3] = {0,1,2};
    TokArr aChoice = TokArr(outcomes,3);// possible values for nodes
    int score[3];
    
    score[0] = score[1] = score[2] = 0;
    
    // build Graph
    // add nodes to net
//    net.AddNode(discrete ^ "PreviousCompTurn PreviousHumanTurn CurrentHumanTurn", aChoice);
    net.AddNode(discrete ^ "PreviousCompTurn", aChoice);
    net.AddNode(discrete ^ "PreviousHumanTurn", aChoice);
    net.AddNode(discrete ^ "CurrentHumanTurn", aChoice);
    
    // add arcs to create following Bayes net:
    //
    //  PreviousCompTurn    PreviousHumanTurn
    //               |         |
    //               V	       V
    //             CurrentHumanTurn
    net.AddArc("PreviousCompTurn", "CurrentHumanTurn");
    net.AddArc("PreviousHumanTurn", "CurrentHumanTurn");
    // next commented command makes the same
    // net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    
    // initial with uniform distribution
//    net.MakeUniformDistribution();
    
    bool bSkipInitialLearning = false;
    // initial learning from file
    try
    {
	net.LoadEvidBuf("rpsLearn123.csv");// CSV (as default)
    }

    catch(...)
    {// fail to load file
	bSkipInitialLearning = true;
	fprintf(stderr, "Fail to load file with learn data"
	    " - initial with uniform distribution\n");
    }

    if(bSkipInitialLearning == false)
    {
	net.SaveEvidBuf("rpsLearnCheck.csv");// for checking
	net.LearnParameters();
	net.ClearEvidBuf();
    }
    
    // Values for pseudo-values of previous game
    // I'll remove these pseudo-values a bit later
    // We drop learning buffer so create pseudo-values of previous game
    Tok humanResponse(1);
    Tok computerTurn(2);
    TokArr evidence;
    
    for(;;)
    {// every cycle iteration is a game round
	// fill evidence with data from previous game
	evidence = ("PreviousCompTurn" ^ computerTurn)
	    & ("PreviousHumanTurn" ^ humanResponse);

	int computer;

	// prediction of human turn for current round
	if(sMode & cShowPrediction)
	{
	    cout << "Data for prediction " << evidence << "\n";
	}
	
	net.EditEvidence(evidence);
	TokArr prediction = net.GetMPE("CurrentHumanTurn");	    
	
	if(sMode & cShowPrediction)
	{
	    cout << "prediction = " << prediction << "\n";
	}
	
	round(&computer, 0, prediction[0].IntValue(), 0);
	// variable 'computer' contains best answer for predicted human turn now
	
	// read real human turn
	int human = ReadHumanResponse();

	// is end of game?
	if(human == -1)
	{
	    break;
	}

        // Turn
        // make turn with 'computer' and 'human'
	int best, result;
	const char *answ = round(&best, &result, human, computer);
	cout << "Computer - " << aChoice[computer].Name() << "; Human - " << aChoice[human].Name() << "\n";
	cout << "Computer " << answ << " for this round\n";

	score[result] ++;

	best = score[2] - score[1];
	result = (best < 0) ? -best:best;
	computerTurn = aChoice[computer].Name();
	humanResponse = aChoice[human].Name();
	cout << "Computer " << ((best > 0) ? "win ":"lose ") << result << "\n";

	// No matter of round's result - learn!
	// Add real data to evidence - now all nodes are observed
	evidence &= (Tok("CurrentHumanTurn") ^ human);

        // Learn with data
        net.EditEvidence(evidence);
        net.LearnParameters();
    }
    
    cout << "Computer score: " << score[2] << " WINs, " << score[0] << " DRAWs and " << score[1] << " LOSEs\n";
    net.SaveEvidBuf("rps.csv");
    
    return 0;
}


int rpsMain()
{
    // create model
    
    // prepare types
#if 0
    Scripting scr;
    FILE *fp = fopen("rpsCreate.pnl", "r");

    if(fp)
    {
	fprintf(stderr, "\nExecution of script from rpsCreate.pnl:\n\n");
	scr.Execute(fp);
	fclose(fp);
	fprintf(stderr, "\nEnd of script\n");
    }
#endif

    BayesNet net;
    TokArr aChoice = "Rock Paper Scissors";// possible values for nodes
    int score[3];
    
    score[0] = score[1] = score[2] = 0;
    
    // build Graph
    // add nodes to net
//    net.AddNode(discrete ^ "PreviousCompTurn PreviousHumanTurn CurrentHumanTurn", aChoice);
    net.AddNode(discrete ^ "PreviousCompTurn", aChoice);
    net.AddNode(discrete ^ "PreviousHumanTurn", aChoice);
    net.AddNode(discrete ^ "CurrentHumanTurn", aChoice);
    
    // add arcs to create following Bayes net:
    //
    //  PreviousCompTurn    PreviousHumanTurn
    //               |         |
    //               V	       V
    //             CurrentHumanTurn
    net.AddArc("PreviousCompTurn", "CurrentHumanTurn");
    net.AddArc("PreviousHumanTurn", "CurrentHumanTurn");
    // next commented command makes the same
    // net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
    
    // initial with uniform distribution
//    net.MakeUniformDistribution();
    
    bool bSkipInitialLearning = false;
    // initial learning from file
    try
    {
	net.LoadEvidBuf("rpsLearn.csv");// CSV (as default)
    }

    catch(...)
    {// fail to load file
	bSkipInitialLearning = true;
	fprintf(stderr, "Fail to load file with learn data"
	    " - initial with uniform distribution\n");
    }

    if(bSkipInitialLearning == false)
    {
	net.SaveEvidBuf("rpsLearnCheck.csv");// for checking
	net.LearnParameters();
	net.ClearEvidBuf();
    }

#if 0
    Scripting scr;
    FILE *fp = fopen("rpsCreate.pnl", "r");
    if(fp)
    {
	fprintf(stderr, "\nExecution of script from rpsCreate.pnl:\n\n");
	scr.Execute(fp, &net);
	fclose(fp);
	fprintf(stderr, "\nEnd of script\n");
    }
    else
    {
	scr.Execute(stdin, &net);
    }
#endif

    // Values for pseudo-values of previous game
    // I'll remove these pseudo-values a bit later
    // We drop learning buffer so create pseudo-values of previous game
    Tok humanResponse("Rock");
    Tok computerTurn("Scissors");
    TokArr evidence;
    
    for(;;)
    {// every cycle iteration is a game round
	// fill evidence with data from previous game
	evidence = ("PreviousCompTurn" ^ computerTurn)
	    & ("PreviousHumanTurn" ^ humanResponse);

	int computer;

	// prediction of human turn for current round
	if(sMode & cShowPrediction)
	{
	    cout << "Data for prediction " << evidence << "\n";
	}
	
	net.EditEvidence(evidence);
	TokArr prediction = net.GetMPE("CurrentHumanTurn");	    
	
	if(sMode & cShowPrediction)
	{
	    cout << "prediction = " << prediction << "\n";
	}
	
	round(&computer, 0, prediction[0].IntValue(), 0);
	// variable 'computer' contains best answer for predicted human turn now
	
	// read real human turn
	int human = ReadHumanResponse();

	// is end of game?
	if(human == -1)
	{
	    break;
	}

        // Turn
        // make turn with 'computer' and 'human'
	int best, result;
	const char *answ = round(&best, &result, human, computer);
	cout << "Computer - " << aChoice[computer].Name()
	    << "; Human - " << aChoice[human].Name() << "\n";
	cout << "Computer " << answ << " for this round\n";

	score[result] ++;

	best = score[2] - score[1];
	result = (best < 0) ? -best:best;
	computerTurn = aChoice[computer].Name();
	humanResponse = aChoice[human].Name();
	cout << "Computer " << ((best > 0) ? "win ":"lose ") << result << "\n";

	// No matter of round's result - learn!
	// Add real data to evidence - now all nodes are observed
	evidence &= (Tok("CurrentHumanTurn") ^ human);

        // Learn with data
        net.EditEvidence(evidence);
        net.LearnParameters();
    }
    
    cout << "Computer score: " << score[2] << " WINs, "
	<< score[0] << " DRAWs and " << score[1] << " LOSEs\n";
    net.SaveEvidBuf("rps.csv");
    
    return 0;
}

#if 0
int main()
{
    int result = -1;

    try
    {
//    result = rpsMain();
    result = scenario();
    }

    catch(pnl::CException &ex)
    {
	cout << "\n\nException breaks normal program execution: " << ex.GetMessage() << "\n";
    }

    catch(...)
    {
	cout << "\n\nUnrecognized exception breaks normal program execution\n";
    }

    return result;
}

#else 

//Adding a strange node: discrete2^CurrentHumanTurn
bool TestResolve1()
{
	//Create a simple model
	BayesNet net;
	net.AddNode("discrete^PreviousCompTurn", "Rock Paper Scissors");
	net.AddNode("discrete^PreviousHumanTurn", "Rock Paper Scissors");
	net.AddNode("discrete^CurrentHumanTurn", "Rock Paper Scissors");

	//Try to add a strange node
	net.AddNode("discrete2^CurrentHumanTurn", "Rock Paper Scissors");
	net.AddArc("PreviousHumanTurn PreviousCompTurn", "CurrentHumanTurn");
//	net.MakeUniformDistribution();


	try 
	{
		net.SetPTabular("CurrentHumanTurn^Rock", "0.4", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
		net.SetPTabular("CurrentHumanTurn^Paper", "0.2666666", "PreviousHumanTurn^Rock PreviousCompTurn^Rock");
		net.SetPTabular("CurrentHumanTurn^Scissors", "0.5", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
		net.SetPTabular("CurrentHumanTurn^Paper", "0.1666666", "PreviousHumanTurn^Rock PreviousCompTurn^Paper");
	}
	catch (pnl::CBadArg e) //There is an exception 
	{
		return false;
	}

	Tok tok;
	tok.Resolve(TokIdNode::root);
	if (tok.IsNil()) 
	{
		std::cout << "Is nil" << "\n";
	};
	if (tok.IsUnres()) 
	{
		std::cout << "Is unres" << "\n";
	};
	return true;
};

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

	if (!((tok.node.size()==1)&&(!tok.IsUnres()))) return false;

	//try to reinit toket with the same value
	tok.Init("discrete^PreviousHumanTurn");
	tok.Resolve(TokIdNode::root);

	if (!((tok.node.size()==1)&&(!tok.IsUnres()))) return false;

	//
	tok.Init("discrete^PreviousCompTurn");
	tok.Resolve(TokIdNode::root);
	if (!((tok.node.size()==1)&&(!tok.IsUnres()))) return false;

	tok.Init("discrete^CurrentHumanTurn");
	tok.Resolve(TokIdNode::root);
	if (!((tok.node.size()==1)&&(!tok.IsUnres()))) return false;

	tok.Init("discrete2^PreviousHumanTurn2");
	tok.Resolve(TokIdNode::root);
	if (!((tok.node.size()==0)&&(tok.IsUnres()))) return false;

	tok.Init("discrete^PreviousHumanTurn2");
	tok.Resolve(TokIdNode::root);
	if (!((tok.node.size()==1)&&(tok.IsUnres()))) return false;

	return true;
};

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
	if (!((tok.node.size()==3)&&(!tok.IsUnres()))) return false;

	return true;
};

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

	if (!((tok.node.size()==2)&&(!tok.IsUnres()))) return false;

	return true;
};

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

	if (!((tok.node.size()==2)&&(!tok.IsUnres()))) return false;

	return true;
};


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
	TokIdNode *pTokIdNode = tok.Node(tok);

	if (!pTokIdNode) 
	{
	    PNL_THROW(pnl::CNULLPointer, "pTokIdNode == NULL");
	};

	tok.Init("CurrentHumanTurn^Paper");
	tok.Resolve(TokIdNode::root);

	if (!((tok.node.size()==1)&&(!tok.IsUnres())))
	{
	    PNL_THROW(pnl::CAlgorithmicException, "Can't resolve CurrentHumanTurn^Paper node");
	};

	pTokIdNode->Alias(String("Paper2"));

	tok.Init("CurrentHumanTurn^Paper2");
	tok.Resolve(TokIdNode::root);

	if (!((tok.node.size()==1)&&(!tok.IsUnres())))
	{
	    PNL_THROW(pnl::CAlgorithmicException, "Can't resolve CurrentHumanTurn^Paper2 node");
	};

	tok.Init("CurrentHumanTurn^Paper3");
	tok.Resolve(TokIdNode::root);

	if (!tok.IsUnres())
	{
	    PNL_THROW(pnl::CAlgorithmicException, "CurrentHumanTurn^Paper3 is resolved but must be not");
	};

	tok.Init("CurrentHumanTurn^Paper");
	tok.Resolve(TokIdNode::root);

	if (!((tok.node.size()==1)&&(!tok.IsUnres())))
	{
	    PNL_THROW(pnl::CAlgorithmicException, "Can't resolve CurrentHumanTurn^Paper node");
	};
};

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
	TokIdNode *pTokIdNodePaper = tok.Node(tok);
	tok.Init("CurrentHumanTurn^Rock");
	tok.Resolve(TokIdNode::root);
	TokIdNode *pTokIdNodeRook = tok.Node(tok);

	bool WasExceptionGenerated = false;

	try 
	{
		pTokIdNodePaper->Alias(String("NewName"));
		pTokIdNodeRook->Alias(String("NewName"));
	}
	catch(pnl::CBadArg e)
	{
		WasExceptionGenerated = true;
	};

	if (!WasExceptionGenerated)
	{
	    PNL_THROW(pnl::CAlgorithmicException, "System allows to add one alias to brothers");
	};

	WasExceptionGenerated = false;
};

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
	TokIdNode *pTokIdNode1 = tok.Node(tok);
	tok.Init("discrete^PreviousHumanTurn");
	tok.Resolve(TokIdNode::root);
	TokIdNode *pTokIdNode2 = tok.Node(tok);

	bool WasExceptionGenerated = false;

	try 
	{
		pTokIdNode1->Alias(String("NewName"));
		pTokIdNode2->Alias(String("NewName"));
	}
	catch(pnl::CBadArg e)
	{
		WasExceptionGenerated = true;
	};

	if (!WasExceptionGenerated)
	{
	    PNL_THROW(pnl::CAlgorithmicException, "System allows to add one alias to brothers");
	};

	WasExceptionGenerated = false;
};

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
	TokIdNode *pTokIdNode1 = tok.Node(tok);
	tok.Init("discrete^PreviousHumanTurn");
	tok.Resolve(TokIdNode::root);
	TokIdNode *pTokIdNode2 = tok.Node(tok);

	bool WasExceptionGenerated = false;

	try 
	{
		pTokIdNode1->Alias(String("NewName"));
		pTokIdNode2->Alias(String("newname"));
	}
	catch(pnl::CBadArg e)
	{
		WasExceptionGenerated = true;
	};

	if (!WasExceptionGenerated)
	{
	    PNL_THROW(pnl::CAlgorithmicException, "System allows to add one alias to brothers");
	};

	WasExceptionGenerated = false;
};

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
	};
};

#if 0
int main(char* argv[], int argc)
{
	bool IsTestOK = true;
	
	//IsTestOK = TestResolve1() && IsTestOK;
	//IsTestOK = TestResolve2() && IsTestOK;
	//IsTestOK = TestResolve3() && IsTestOK;
	//IsTestOK = TestResolve4() && IsTestOK;
	//IsTestOK = TestResolve4_5() && IsTestOK;
	try 
	{
		TestAlias4();
	}
	catch(pnl::CException e)
	{
		std::cout << e.GetMessage();
	}
	
	if (IsTestOK)
	{
		std::cout << "All tests completed successfully" << "\n";
	};
	
	return 0;
}
#endif

#include "pnl_dll.hpp"
#include "pnlGraphicalModel.hpp"
#include <string>

PNL_USING
using namespace std;

const string nameOfModel = "Model";
const string nameOfEvidence = "my_ev";


pnl::CGraphicalModel* LoadGrModelFromXML(const string& xmlname, vector<pnl::CEvidence*>* pEv)
{
    pnl::CGraphicalModel* pGM = NULL;
    
    pnl::CContextPersistence ContextLoad;
    if ( !ContextLoad.LoadXML(xmlname) )
    {
        cout << "\nfile " << xmlname.c_str() << "isn't correct as XML";
        return NULL;
    }
    pGM = static_cast<pnl::CGraphicalModel*>(ContextLoad.Get(nameOfModel.c_str()));    
    if (NULL == pGM)
    {
        cout << "\nfile " << xmlname.c_str() << 
            "isn't containing an object with name: " << nameOfModel.c_str();
        return NULL;
    }

    if (pEv)
    {
        CEvidence* ev = NULL;
        
        pEv->clear();
        ev = static_cast<CEvidence*>(
            ContextLoad.Get(string(nameOfEvidence + "0").c_str()));
        if (ev) 
            pEv->push_back(ev);
    }
    
    return pGM;
}


void TestMultivariateWrapperJPD()
{
    int i;
    vector<CEvidence*> evidences;
    CBNet *pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML(".\\TestMultivariate\\5Nodes.xml", &evidences));

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
    };

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
    };
}


void TestMultivariateWrapperLearn()
{
    vector<CEvidence*> evidences;
    CBNet *pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML(".\\TestMultivariate\\5Nodes.xml", &evidences));

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

	};

	evidBnet[ev] = pnl::CEvidence::Create(pBNet, obsNodes, obsValues);
    };

   
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
    CBNet *pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML(".\\TestMultivariate\\5Nodes.xml", &evidences));

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
	};

	evidBnet[ev] = pnl::CEvidence::Create(pBNet, obsNodes, obsValues);
    };

   
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
    CBNet *pBNet = dynamic_cast<CBNet*>(LoadGrModelFromXML(".\\TestMultivariate\\5Nodes.xml", &evidences));

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
    };
}

int main(char* argv[], int argc)
{
	bool IsTestOK = true;
	
	//IsTestOK = TestResolve1() && IsTestOK;
	//IsTestOK = TestResolve2() && IsTestOK;
	//IsTestOK = TestResolve3() && IsTestOK;
	//IsTestOK = TestResolve4() && IsTestOK;
	//IsTestOK = TestResolve4_5() && IsTestOK;
	try 
	{
		TestMultivariateWrapperJPD();
		TestMultivariateWrapperLearn();
		TestMultivariateWrapperLearnPartOfObservation();
		TestMultivariateWrapperMPE();
	}
	catch(pnl::CException e)
	{
		std::cout << e.GetMessage();
	}
	
	if (IsTestOK)
	{
		std::cout << "All tests completed successfully" << "\n";
	};
	
	return 0;
}

#endif
