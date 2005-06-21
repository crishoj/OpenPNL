#include <conio.h>
#include "console.h"
#include "models.h"
PNLW_BEGIN

std::ostream &operator<<(std::ostream &str, const TokArr &ta)
{
    return str << String(ta);
}

void AsiaDemo()
{
    BayesNet *Asia;
    Asia = AsiaModel();

    TokArr jpdLungCancer1 = Asia->GetJPD("LungCancer");

    textcolor(WHITE);
    cout << "\n TokArr jpdLungCancer1 = Asia->GetJPD(\"LungCancer\");\t\t\t\t"; 

    textcolor(YELLOW);
    cout <<jpdLungCancer1<<"\n";
    _sleep(1000);

    Asia->SetProperty("Inference", "jtree");

    textcolor(WHITE);
    cout << "\n Asia->SetProperty(\"Inference\", \"jtree\");" <<"\n";
    _sleep(1000);

    Asia->EditEvidence("Smoking^True VisitToAsia^False");
    cout << "\n Asia->EditEvidence(\"Smoking^True VisitToAsia^False\");" <<"\n";
    _sleep(1000);
    
    TokArr jpdLungCancer2 = Asia->GetJPD("LungCancer");
    cout << "\n TokArr jpdLungCancer2 = Asia->GetJPD(\"LungCancer\");\t\t\t\t"; 

    textcolor(YELLOW);
    cout <<jpdLungCancer2<<"\n";
    _sleep(1000);

    textcolor(WHITE);

    Asia->ClearEvid();
    cout << "\n Asia->ClearEvid();\n";
    _sleep(1000);

    Asia->SetProperty("Learning", "em");
    cout << "\n Asia->SetProperty(\"Learning\", \"em\");";

    textcolor(LIGHTGREEN);
    cout << "\t\t\t\t\t\tLearning is in process....\n";
    textcolor(WHITE);
    _sleep(1000);

    Asia->LoadEvidBuf("observations.csv");
    cout << "\n Asia->LoadEvidBuf(\"observations.csv\");\n";
    _sleep(1000);

    Asia->LearnParameters();
    cout << "\n Asia->LearnParameters();\n";
    _sleep(1000);

    TokArr jpdLungCancer3 = Asia->GetJPD("LungCancer");
    cout << "\n TokArr jpdLungCancer3 = Asia->GetJPD(\"LungCancer\");\t\t\t\t";

    textcolor(YELLOW);
    cout << jpdLungCancer3 <<"\n";
    textcolor(WHITE);

    getch();

    delete Asia;
}

void PigsDemo()
{
    LIMID *Pigs;
    Pigs = PigsModel();
    
    textcolor(WHITE);
    TokArr exp = Pigs->GetExpectation();
    cout << "\nTokArr exp = Pigs->GetExpectation();" << "\t\t\t\t\t\t\t";
    _sleep(1000);
    textcolor(YELLOW);
    cout << exp <<"\n";
    textcolor(WHITE);
    _sleep(1000);

    TokArr politics = Pigs->GetPolitics();
    cout << "\nTokArr politics = Pigs->GetPolitics();" << "\t\t\t\t\t\t\t";
    _sleep(1000);
    Tok p1 = politics[0];
    Tok p2 = politics[1];
    Tok p3 = politics[2];
    
    textcolor(YELLOW);
    cout << p1 << "\n\t\t\t\t\t\t\t\t\t\t\t" << p2 << "\n\t\t\t\t\t\t\t\t\t\t\t" << p3 << "\n";
//    cout << politics <<"\n";
    textcolor(WHITE);

    getch();
    delete Pigs;
}

void ScalarGaussianBNetDemo()
{
    BayesNet *ScalarNet;
    ScalarNet = ScalarGaussianBNetModel();
    
    ScalarNet->GenerateEvidences(100);
    cout <<"\n\n net->GenerateEvidences(100);";
    
    textcolor(LIGHTGREEN);
    cout << "\t\t\t\t\t\t\tGenerating of evidences is in process....";
    textcolor(WHITE);
    _sleep(1000);

    ScalarNet->SetProperty("Inference", "gibbs");
    cout <<"\n\n net->SetProperty(\"Inference\", \"gibbs\");";
    _sleep(1000);

    ScalarNet->SetProperty("GibbsThresholdIteration", "50"); 
    cout <<"\n\n net->SetProperty(\"GibbsThresholdIteration\", \"50\"); ";
    _sleep(1000);

    ScalarNet->SetProperty("GibbsNumberOfIterations", "1000"); 
    cout <<"\n\n net->SetProperty(\"GibbsNumberOfIterations\", \"1000\"); ";
    _sleep(1000);

    cout <<"\n\n TokArr jpdNodeA = net->GetJPD(\"NodeA\");\t\t\t\t\t";
    TokArr jpdNodeA = ScalarNet->GetJPD("NodeA");
    _sleep(1000);

    textcolor(YELLOW);
    cout << jpdNodeA <<"\n";
    textcolor(WHITE);

    getch();
    delete ScalarNet;
}

void WasteDemo()
{
    BayesNet *WasteNet;
    WasteNet = WasteModel();
    
    WasteNet->EditEvidence("FilterState^intact TypeOfWaste^household BurningRegime^stable");
    printf("\nWasteNet->EditEvidence(\"FilterState^intact TypeOfWaste^household BurningRegime^stable\");");
    _sleep(1000);

    WasteNet->SetProperty("Inference", "jtree");
    printf("\n\nWasteNet->SetProperty(\"Inference\", \"jtree\");");
    _sleep(1000);

    TokArr jpdNode4 = WasteNet->GetJPD("MetalEmission");
    printf("\n\nTokArr jpdNode4 = WasteNet->GetJPD(\"MetalEmission\");\t\t\t\t\t");
    _sleep(1000);

    textcolor(YELLOW);
    cout << jpdNode4 <<"\n";
    textcolor(WHITE);

    getch();
    delete WasteNet;
}

void KjaerulfsBNetDemo()
{
    BayesNet *DBN;
    DBN = KjaerulfsBNetModel();
    
    getch();
    delete DBN;
}

void RPSDemo()
{
    BayesNet *RPS;
    RPS = RPSModel();

//    RPS->EditEvidence("PreviousCompTurn^Paper PreviousHumanTurn^Rock");
//    RPS->SetProperty("Inference", "gibbs");
    TokArr fPaper = RPS->GetJPD("CurrentHumanTurn");
    cout << fPaper ;
    getch();
    delete RPS;
}

void CropDemo()
{
    //  Subsidy(d) Crop(c)
    //          |   |
    //          V   V
    //         Price(c)
    //            |
    //            V
    //          Buy(d)

    BayesNet *net;
    net = CropModel();

    net->EditEvidence("Subsidy^No Buy^Yes");
    printf("\n\n net->EditEvidence(\"Subsidy^No Buy^Yes\");");
    _sleep(1000);
    net->SetProperty("Inference", "jtree");
    printf("\n\n net->SetProperty(\"Inference\", \"jtree\");");
    _sleep(1000);

    TokArr PriceJPD = net->GetJPD("Price");
    printf("\n\n TokArr PriceJPD = net->GetJPD(\"Price\");\t\t\t\t\t\t");
    _sleep(1000);

    textcolor(YELLOW);
    cout << PriceJPD << "\n";
    textcolor(WHITE);

/*    net->ClearEvidBuf();

    net->EditEvidence("Subsidy^Yes Buy^No");
 */
/*    net->SetProperty("Inference", "gibbs");

    TokArr PriceJPD = net->GetJPD("Price");
    cout << PriceJPD<< "\n" ;
*/
//    RPS->EditEvidence("PreviousCompTurn^Paper PreviousHumanTurn^Rock");
//    RPS->SetProperty("Inference", "gibbs");
//    
    getch();
    delete net;
}

void Structural()
{
    BayesNet *net;
    net = new BayesNet();
    
    net->AddNode("node0 node1 node2 node3 node4 node5 node6", "true false");
    net->LoadEvidBuf("seven.csv");
    net->LearnStructure();

    TokArr neib0 = net->GetNeighbors("node0");
    cout << neib0<< "\n" ;
    TokArr neib1 = net->GetNeighbors("node1");
    cout << neib1<< "\n" ;
    TokArr neib2 = net->GetNeighbors("node2");
    cout << neib2<< "\n" ;
    TokArr neib3 = net->GetNeighbors("node3");
    cout << neib3<< "\n" ;
    TokArr neib4 = net->GetNeighbors("node4");
    cout << neib4<< "\n" ;
    TokArr neib5 = net->GetNeighbors("node5");
    cout << neib5<< "\n" ;
    TokArr neib6 = net->GetNeighbors("node6");
    cout << neib6<< "\n" ;


}

void FraudDemo()
{
    BayesNet *net;
    net = FraudModel();
    
//    net->GenerateEvidences(100);
//    net->SaveEvidBuf("obs.csv");

    net->LoadEvidBuf("fraud.csv");
    net->LearnStructure();
    
    TokArr neib0 = net->GetNeighbors("Fraud");
    cout << neib0<< "\n" ;
    TokArr neib1 = net->GetNeighbors("Gas");
    cout << neib1<< "\n" ;
    TokArr neib2 = net->GetNeighbors("Jewelry");
    cout << neib2<< "\n" ;
    TokArr neib3 = net->GetNeighbors("Age");
    cout << neib3<< "\n" ;
    TokArr neib4 = net->GetNeighbors("Sex");
    cout << neib4<< "\n" ;
    
    getch();
    delete net;
}

PNLW_END