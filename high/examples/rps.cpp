#include <time.h>
#include "pnlHigh.hpp"
#include "console.hpp"
#include "pnl_dll.hpp"

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
    String ta_str(ta);
    return str << ta_str;
//    return str << String(ta);
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
    net.AddNode(discrete ^ "Predictor Response Response2", aChoice);

    net.AddArc("Predictor", "Response Response2");

    TokArr ev = (Tok("Predictor")^"True") & (Tok("Response")^"True") & (Tok("Response2")^"True");
//    cout << ev;

    net.AddEvidToBuf(ev);
   // net.Evid(Tok("Response")^"False", true);
   // net.Evid(Tok("Response")^"True", true);
   // net.Evid(Tok("Response")^"True", true);

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

    net.AddEvidToBuf(ev);

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
        net.CurEvidToBuf();
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
	net.SetProperty("Inference","naive");
	TokArr prediction = net.GetMPE("CurrentHumanTurn");
	    cout << "prediction = " << prediction << "\n";

    printf("\n%s\n",String(prediction).c_str());
	if(sMode & cShowPrediction)
	{
	    cout << "prediction = " << prediction << "\n";
	}

	prediction[0].Resolve();
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
    net.SetProperty("Learning","bayes");
    net.EditEvidence(evidence);
    net.CurEvidToBuf();
    net.LearnParameters();
    }

    cout << "Computer score: " << score[2] << " WINs, "
	<< score[0] << " DRAWs and " << score[1] << " LOSEs\n";
    net.SaveEvidBuf("rps.csv");

    return 0;
}

MRF *CreateMRFModel()
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

int main(int ac, char **av)
{
    int result = -1;
    

    if(ac > 1 && av[1][0] == '-' && tolower(av[1][1]) == 's')
    {
	Scripting scr;
	const char *fname = (av[1][2] || ac < 3) ? av[1] + 2:av[2];
	FILE *fp = fopen(fname, "r");
	
	if(fp)
	{
	    fprintf(stderr, "\nExecution of script from '%s':\n\n", fname);
	    result = scr.Execute(fp);
	    fclose(fp);
	    fprintf(stderr, "\nEnd of script\n");
	}
	else
	{
	    fprintf(stderr, "\nCan't open file '%s'\n\n", fname);
	}
	return result;
    }
    
    try
    {
	result = rpsMain();
//    result = scenario();
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
