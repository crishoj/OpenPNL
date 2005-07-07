#include <conio.h>
#include "console.h"
#include "models.h"
#include "examples.h"
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
    cout << "\n TokArr exp = Pigs->GetExpectation();" << "\t\t\t\t\t\t\t";
    _sleep(1000);
    textcolor(YELLOW);
    cout << exp <<"\n";
    textcolor(WHITE);
    _sleep(1000);

    TokArr politics = Pigs->GetPolitics();
    cout << "\n TokArr politics = Pigs->GetPolitics();" << "\t\t\t\t\t\t\t";
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
    DBN *pDBN;
    pDBN = KjaerulfsBNetModel();
    
    pDBN->SetNumSlices(4);
    printf("\n pDBN->SetNumSlices(4);");

    pDBN->AddEvidToBuf("node0-0^true");
    printf("\n\n pDBN->AddEvidToBuf(\"node0-0^true\");");
    
    pDBN->AddEvidToBuf("node0-1^true");
    printf("\n\n pDBN->AddEvidToBuf(\"node0-1^true\");");
    
    pDBN->AddEvidToBuf("node0-2^true");
    printf("\n\n pDBN->AddEvidToBuf(\"node0-2^true\");");
    
    pDBN->AddEvidToBuf("node0-3^true");
    printf("\n\n pDBN->AddEvidToBuf(\"node0-3^true\");");

    pDBN->SetProperty("Inference","smooth");
    printf("\n\n pDBN->SetProperty(\"Inference\",\"smooth\");");

    TokArr jpd1 = pDBN->GetJPD("node2-2");
    printf("\n\n TokArr jpd1 = pDBN->GetJPD(\"node2-2\");\t\t\t\t\t\t\t");
    
    cout << jpd1 <<"\n";
    
    pDBN->AddEvidToBuf("node1-0^false");
    printf("\n\n pDBN->AddEvidToBuf(\"node1-0^false\");");

    pDBN->AddEvidToBuf("node1-1^true");
    printf("\n\n pDBN->AddEvidToBuf(\"node1-1^true\");");

    pDBN->SetProperty("Inference","filt");
    printf("\n\n pDBN->SetProperty(\"Inference\",\"filt\");");

    TokArr jpd2 = pDBN->GetJPD("node2-1");
    printf("\n\n TokArr jpd2 = pDBN->GetJPD(\"node2-1\");\t\t\t\t\t\t\t");

    cout << jpd2 <<"\n";
    
    pDBN->GenerateEvidences("5 5 5 5");
    printf("\n\n pDBN->GenerateEvidences(\"5 5 5 5\");");

    pDBN->LearnParameters();
    printf("\n\n pDBN->LearnParameters();");

    float value = pDBN->GetEMLearningCriterionValue();
    printf("\n\n float value = pDBN->GetEMLearningCriterionValue();\t\t\t\t\t");
    cout << value <<"\n";

    delete pDBN;
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

/*    net->EditEvidence("Subsidy^No Buy^Yes");
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
*/
    net->AddEvidToBuf("Subsidy^Yes Buy^No Crop^5.45 Price^5.0");
    printf("\n net->AddEvidToBuf(\"Subsidy^Yes Buy^No Crop^5.45 Price^5.0\");");
    _sleep(1000);

    net->AddEvidToBuf("Subsidy^No Buy^Yes Crop^8.6 Price^3.3");
    printf("\n net->AddEvidToBuf(\"Subsidy^No Buy^Yes Crop^8.6 Price^3.3\");");
    _sleep(1000);

    net->AddEvidToBuf("Subsidy^No Buy^No Crop^7.4 Price^2.4");
    printf("\n net->AddEvidToBuf(\"Subsidy^No Buy^No Crop^7.4 Price^2.4\");");
    _sleep(1000);

    net->AddEvidToBuf("Subsidy^Yes Buy^Yes Crop^3.2 Price^1.8");
    printf("\n net->AddEvidToBuf(\"Subsidy^Yes Buy^Yes Crop^3.2 Price^1.8\");");
    _sleep(1000);

    net->LearnParameters();
    printf("\n\n net->LearnParameters();");
    _sleep(1000);

    float val =  net->GetEMLearningCriterionValue();
    printf("\n\n net->GetEMLearningCriterionValue();");
    textcolor(YELLOW);
    printf("\t\t\t\t\t\t\t%f\n", val);
    textcolor(WHITE);
    _sleep(1000);

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
    
    net->AddEvidToBuf("Fraud^No Gas^No Jewelry^No Age^MiddleAged Sex^Female");
    printf("\n net->AddEvidToBuf(\"Fraud^No Gas^No Jewelry^No Age^MiddleAged Sex^Female\");");
    textcolor(LIGHTGREEN);
    printf("\t\tAdding of observations is in process....");
    _sleep(2000);
    textcolor(WHITE);

    net->AddEvidToBuf("Fraud^No Gas^No Jewelry^No Age^MiddleAged Sex^Male");
    printf("\n net->AddEvidToBuf(\"Fraud^No Gas^No Jewelry^No Age^MiddleAged Sex^Male\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Fraud^Yes Gas^Yes Jewelry^Yes Age^InYears Sex^Male");
    printf("\n net->AddEvidToBuf(\"Fraud^Yes Gas^Yes Jewelry^Yes Age^InYears Sex^Male\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Fraud^No Gas^No Jewelry^No Age^MiddleAged Sex^Male");
    printf("\n net->AddEvidToBuf(\"Fraud^No Gas^No Jewelry^No Age^MiddleAged Sex^Male\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Fraud^No Gas^Yes Jewelry^No Age^Young Sex^Female");
    printf("\n net->AddEvidToBuf(\"Fraud^No Gas^Yes Jewelry^No Age^Young Sex^Female\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Fraud^No Gas^No Jewelry^No Age^Young Sex^Female");
    printf("\n net->AddEvidToBuf(\"Fraud^No Gas^No Jewelry^No Age^Young Sex^Female\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Fraud^No Gas^No Jewelry^No Age^InYears Sex^Male");
    printf("\n net->AddEvidToBuf(\"Fraud^No Gas^No Jewelry^No Age^InYears Sex^Male\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Fraud^No Gas^No Jewelry^Yes Age^MiddleAged Sex^Female");
    printf("\n net->AddEvidToBuf(\"Fraud^No Gas^No Jewelry^Yes Age^MiddleAged Sex^Female\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Fraud^No Gas^Yes Jewelry^No Age^Young Sex^Male");
    printf("\n net->AddEvidToBuf(\"Fraud^No Gas^Yes Jewelry^No Age^Young Sex^Male\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Fraud^No Gas^No Jewelry^No Age^Young Sex^Female");
    printf("\n net->AddEvidToBuf(\"Fraud^No Gas^No Jewelry^No Age^Young Sex^Female\");");
    _sleep(1000);

    net->SetProperty("LearningStructureMethod", "PreAs");
    printf("\n\n net->SetProperty(\"LearningStructureMethod\", \"PreAs\");");
    _sleep(1000);
    
    net->SetProperty("LearningStructureScoreFun", "BIC");
    printf("\n net->SetProperty(\"LearningStructureScoreFun\", \"BIC\");");
    _sleep(1000);

    net->LearnStructure();
    printf("\n\n net->LearnStructure();");
    _sleep(1000);

    TokArr FraudChild = net->GetChildren("Fraud");
    printf("\n\n TokArr FraudChild = net->GetChildren(\"Fraud\");");

    textcolor(YELLOW);
    cout << "\t\t\t\t\t\tFraud: " << FraudChild<< "\n" ;
    _sleep(1000);
    
    TokArr AgeChild = net->GetChildren("Age");
    textcolor(WHITE);
    printf("\n TokArr AgeChild = net->GetChildren(\"Age\");");
    textcolor(YELLOW);
    cout << "\t\t\t\t\t\tAge: "<< AgeChild<< "\n" ;
    textcolor(WHITE);
    _sleep(1000);
    
    TokArr SexChild = net->GetChildren("Sex");
    printf("\n TokArr SexChild = net->GetChildren(\"Sex\");");
    textcolor(YELLOW);
    cout <<"\t\t\t\t\t\tSex: "<< SexChild<< "\n" ;
    textcolor(WHITE);
    _sleep(1000);
    
    TokArr GasChild = net->GetChildren("Gas");
    printf("\n TokArr GasChild = net->GetChildren(\"Gas\");");
    textcolor(YELLOW);
    cout << "\t\t\t\t\t\tGas: "<< GasChild<< "\n" ;
    textcolor(WHITE);
    _sleep(1000);
    
    TokArr JewelryChild = net->GetChildren("Jewelry");
    printf("\n TokArr JewelryChild = net->GetChildren(\"Jewelry\");");
    textcolor(YELLOW);
    cout <<"\t\t\t\t\tJewelry: "<< JewelryChild<< "\n" ;
    textcolor(WHITE);

//   net->GenerateEvidences(500);
//    net->SaveEvidBuf("obs.csv");

/*    net->SetProperty("LearningStructureMethod", "MarLh");
    net->SetProperty("LearningStructureScoreFun", "WithoutPenalty");    
    net->LoadEvidBuf("obs.csv");
    net->LearnStructure();
    
    TokArr neib0 = net->GetChildren("Fraud");
    cout << "Fraud" << neib0<< "\n" ;
    TokArr neib1 = net->GetChildren("Gas");
    cout << "Gas" << neib1<< "\n" ;
    TokArr neib2 = net->GetChildren("Jewelry");
    cout << "Jewelry" <<neib2<< "\n" ;
    TokArr neib3 = net->GetChildren("Age");
    cout << "Age" <<neib3<< "\n" ;
    TokArr neib4 = net->GetChildren("Sex");
    cout << "Sex" <<neib4<< "\n" ;
*/    
    getch();
    delete net;
}

void DBNDemo()
{
    DBN *net;
    net = DBNModel();

    net->SetProperty("InferenceAlgorythm", "Boyen-Koller");
    printf("\n\n net->SetProperty(\"InferenceAlgorythm\", \"Boyen-Koller\");");
    _sleep(1000);

    net->SetNumSlices(4);
    printf("\n\n net->SetNumSlices(4);");
    _sleep(1000);

    net->AddEvidToBuf("Rain-0^True Umbrella-0^False");
    printf("\n\n net->AddEvidToBuf(\"Rain-0^True Umbrella-0^False\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Rain-1^True");
    printf("\n net->AddEvidToBuf(\"Rain-1^True\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Rain-2^False");
    printf("\n net->AddEvidToBuf(\"Rain-2^False\");");
    _sleep(1000);
    
    net->AddEvidToBuf("Umbrella-3^False");
    printf("\n net->AddEvidToBuf(\"Umbrella-3^False\");");
    _sleep(1000);

    TokArr JPD = net->GetJPD("Rain-0 Umbrella-0");
    printf("\n\n TokArr JPD = net->GetJPD(\"Rain-0 Umbrella-0\");\t\t\t\t\t\t\t");
//    cout << JPD << "\n" ;

    Tok j1 = JPD[0];
    Tok j2 = JPD[1];
    Tok j3 = JPD[2];
    Tok j4 = JPD[3];
    textcolor(YELLOW);
    cout << j1 << "\n\t\t\t\t\t\t\t\t\t\t\t\t" << j2 << "\n\t\t\t\t\t\t\t\t\t\t\t\t";
    cout << j3 << "\n\t\t\t\t\t\t\t\t\t\t\t\t"<< j4 <<"\n";
    textcolor(WHITE);
    getch();
}
PNLW_END