#include "RPSGame.h"
#include "pnlHigh.hpp"
#include "pnl_dll.hpp"
#include <time.h>

PNLW_USING

TokArr aChoice = "Rock Paper Scissors";

Vector<TokArr> Evidences;
Tok humanResponse("Rock");
Tok computerTurn("Scissors");

BayesNet *GetRPS()
{
    // possible values for nodes
    BayesNet *net = new BayesNet();

    // build Graph
    // add nodes to net
    net->AddNode(discrete ^ "PreviousCompTurn", aChoice);
    net->AddNode(discrete ^ "PreviousHumanTurn", aChoice);
    net->AddNode(discrete ^ "CurrentHumanTurn", aChoice);

    // add arcs to create following Bayes net:
    //
    //  PreviousCompTurn    PreviousHumanTurn
    //               |         |
    //               V	       V
    //             CurrentHumanTurn
    net->AddArc("PreviousCompTurn", "CurrentHumanTurn");
    net->AddArc("PreviousHumanTurn", "CurrentHumanTurn");


    return net;
};

RPSMove GenerateMove(float fRock, float fPaper, float fScissors)
{
    int value = rand();

    if (static_cast<float>(rand())/RAND_MAX < fRock)
    {
	return Rock;
    };

    if (static_cast<float>(rand())/RAND_MAX < fRock + fPaper)
    {
	return Paper;
    };


    return Scissors;
};

void SetRandomDistribution()
{
    Evidences.clear();
    Evidences.resize(0);

/*
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Scissors CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Scissors CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Scissors CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Paper CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Rock CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Paper CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Rock CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Scissors CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Scissors CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Scissors CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Paper CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Paper CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Rock CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Rock CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Scissors CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Paper CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Rock CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Rock CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Scissors CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Paper CurrentHumanTurn^Rock");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Paper");
*/

    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Paper CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Paper CurrentHumanTurn^Rock");

    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Rock CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Rock CurrentHumanTurn^Rock");

    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Scissors CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Scissors CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Paper PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");

    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Paper CurrentHumanTurn^Rock");

    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Rock CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Rock CurrentHumanTurn^Rock");

    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Scissors CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Scissors CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Rock PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");

    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Paper CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Paper CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Paper CurrentHumanTurn^Rock");

    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Rock CurrentHumanTurn^Rock");

    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Paper");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Scissors");
    Evidences.push_back("PreviousCompTurn^Scissors PreviousHumanTurn^Scissors CurrentHumanTurn^Rock");

    humanResponse = "Paper";
    computerTurn = "Scissors";

    srand( (unsigned)time( NULL ) );
};

RPSMove GetNextMove(RPSMove HumansMove)
{
    RPSMove Result = Paper;

    BayesNet *net = GetRPS();

    int size = Evidences.size();
    for (int i = 0; i < size; i++)
    {
	net->AddEvidToBuf(Evidences[i]);
    };

    net->LearnParameters();

    TokArr evidence;
    // fill evidence with data from previous game
    evidence = ("PreviousCompTurn" ^ computerTurn)
	& ("PreviousHumanTurn" ^ humanResponse);
    String SForTok;
    SForTok = "PreviousCompTurn^";
    SForTok += String(computerTurn);
    SForTok += " PreviousHumanTurn^";
    SForTok += String(humanResponse);
    SForTok += " CurrentHumanTurn^";
    char CharBuffer[1000];
    SForTok += String(_itoa(static_cast<int>(HumansMove),CharBuffer, 10));

    net->EditEvidence(evidence);
    TokArr prediction = net->GetMPE("CurrentHumanTurn");

    prediction[0].Resolve();
    
#ifdef OLD_VARIANT
    Result = static_cast<RPSMove>((prediction[0].IntValue() + 1)%3);
#else
    float fRock, fPaper, fScissors;
    GetCurrentDistribution(fRock, fPaper, fScissors);
    Result = GenerateMove(fRock, fPaper, fScissors);
#endif

    Evidences.push_back(TokArr(SForTok));    

    delete net;

    computerTurn = aChoice[static_cast<int>(Result)];
    humanResponse = aChoice[static_cast<int>(HumansMove)];

    return Result;
};

void GetCurrentDistribution(float &fRock, float &fPaper, float &fScissors)
{
    BayesNet *net = GetRPS();

    int size = Evidences.size();
    for (int i = 0; i < size; i++)
    {
	net->AddEvidToBuf(Evidences[i]);
    };

    net->LearnParameters();

    TokArr evidence;
    // fill evidence with data from previous game
    evidence = ("PreviousCompTurn" ^ computerTurn)
	& ("PreviousHumanTurn" ^ humanResponse);

    net->EditEvidence(evidence);
    fPaper = net->GetJPD("CurrentHumanTurn^Rock")[0].FltValue();
    fScissors = net->GetJPD("CurrentHumanTurn^Paper")[0].FltValue();
    fRock = net->GetJPD("CurrentHumanTurn^Scissors")[0].FltValue();

    delete net;

};

