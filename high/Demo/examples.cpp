//#include "Samples.h"
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

    getch();
    delete Pigs;
}

void ScalarGaussianBNetDemo()
{
    BayesNet *ScalarNet;
    ScalarNet = ScalarGaussianBNetModel();
    
    getch();
    delete ScalarNet;
}

void WasteDemo()
{
    BayesNet *WasteNet;
    WasteNet = WasteModel();
    
    getch();
    delete WasteNet;
}


PNLW_END