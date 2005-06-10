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

    textcolor(YELLOW);
    cout << exp <<"\n";
    textcolor(WHITE);

    TokArr politics = Pigs->GetPolitics();
    cout << "\nTokArr politics = Pigs->GetPolitics();" << "\t\t\t\t\t\t\t";

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

    ScalarNet->SetProperty("Inference", "gibbs");
    cout <<"\n\n net->SetProperty(\"Inference\", \"gibbs\");";
    
    ScalarNet->SetProperty("GibbsThresholdIteration", "50"); 
    cout <<"\n\n net->SetProperty(\"GibbsThresholdIteration\", \"50\"); ";
    
    ScalarNet->SetProperty("GibbsNumberOfIterations", "1000"); 
    cout <<"\n\n net->SetProperty(\"GibbsNumberOfIterations\", \"1000\"); ";
    
    cout <<"\n\n TokArr jpdNodeA = net->GetJPD(\"NodeA\");\t\t\t\t\t";
    TokArr jpdNodeA = ScalarNet->GetJPD("NodeA");

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
    
    WasteNet->EditEvidence("node0^True node1^False node3^False");
    WasteNet->SetProperty("Inference", "jtree");

    TokArr jpdNode4 = WasteNet->GetJPD("node4");
    cout << jpdNode4 <<"\n";

/*    WasteNet->GenerateEvidences(200);
    WasteNet->LearnParameters();
    
    TokArr jpdNode4afterLearn = WasteNet->GetJPD("node4");
    cout << jpdNode4afterLearn <<"\n";
*/    
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
PNLW_END