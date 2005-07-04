#include "pnlHigh.hpp"
#include <conio.h>
#include "console.h"

PNLW_BEGIN

BayesNet *AsiaModel()
{ 
    BayesNet *net;
    net = new BayesNet();

    // nodes
    textcolor(WHITE);
    net->AddNode(discrete^"Smoking", "True False"); 
    printf("\n net->AddNode(discrete^\"Smoking\", \"True False\");");
    textcolor(LIGHTGREEN);

    printf("\t\t\t\tAdding of nodes is in process....");
    _sleep(2000);
    textcolor(WHITE);

    net->AddNode(discrete^"VisitToAsia", "True False");
    printf("\n net->AddNode(discrete^\"VisitToAsia\", \"True False\");");
    _sleep(1000);

    net->AddNode(discrete^"Bronchitis", "True False"); 
    printf("\n net->AddNode(discrete^\"Bronchitis\", \"True False\");");
    _sleep(1000);

    net->AddNode(discrete^"LungCancer", "True False"); 
    printf("\n net->AddNode(discrete^\"LungCancer\", \"True False\");");
    _sleep(1000);
    
    net->AddNode(discrete^"Tuberculosis", "True False"); 
    printf("\n net->AddNode(discrete^\"Tuberculosis\", \"True False\");");
    _sleep(1000);

    net->AddNode(discrete^"Dyspnoea", "True False"); 
    printf("\n net->AddNode(discrete^\"Dyspnoea\", \"True False\");");
    _sleep(1000);

    net->AddNode(discrete^"CancerOrTuberculosis", "True False");
    printf("\n net->AddNode(discrete^\"CancerOrTuberculosis\", \"True False\");");
    _sleep(1000);
    
    net->AddNode(discrete^"XRayPositive", "True False");
    printf("\n net->AddNode(discrete^\"XRayPositive\", \"True False\");");
    _sleep(1000);

    textcolor(LIGHTGREEN);
    printf("\n ......All nodes are added....\n");
    getch();

    // arcs
    textcolor(WHITE);
    net->AddArc("Smoking", "Bronchitis LungCancer");
    printf("\n net->AddArc(\"Smoking\", \"Bronchitis LungCancer\");");

    textcolor(LIGHTGREEN);
    printf("\t\t\t\tAdding of arcs is in process....");
    _sleep(2000);

    textcolor(WHITE);
    net->AddArc("VisitToAsia", "Tuberculosis");
    printf("\n net->AddArc(\"VisitToAsia\", \"Tuberculosis\");");
    _sleep(1000);

    net->AddArc("LungCancer Tuberculosis", "CancerOrTuberculosis");
    printf("\n net->AddArc(\"LungCancer Tuberculosis\", \"CancerOrTuberculosis\");");
    _sleep(1000);

    net->AddArc("Bronchitis", "Dyspnoea");
    printf("\n net->AddArc(\"Bronchitis\", \"Dyspnoea\");");
    _sleep(1000);

    net->AddArc("CancerOrTuberculosis", "Dyspnoea XRayPositive");
    printf("\n net->AddArc(\"CancerOrTuberculosis\", \"Dyspnoea XRayPositive\");");
    _sleep(1000);

    textcolor(LIGHTGREEN);
    printf("\n ......All arcs are added....\n");
    getch();
    // distributions
    textcolor(WHITE);
    net->SetPTabular("Smoking^True Smoking^False", "0.5 0.5");
    printf("\n net->SetPTabular(\"Smoking^True Smoking^False\", \"0.5 0.5\");");
    
    textcolor(LIGHTGREEN);
    printf("\t\t\tAdding of distributions is in process....");
    _sleep(2000);

    textcolor(WHITE);
    net->SetPTabular("VisitToAsia^True VisitToAsia^False", "0.01 0.99");
    printf("\n net->SetPTabular(\"VisitToAsia^True VisitToAsia^False\", \"0.01 0.99\");");
    _sleep(1000);

    net->SetPTabular("Bronchitis^True", "0.3", "Smoking^False");
    printf("\n net->SetPTabular(\"Bronchitis^True\", \"0.3\", \"Smoking^False\");");
    _sleep(1000);

    net->SetPTabular("Bronchitis^False", "0.7", "Smoking^False");
    printf("\n net->SetPTabular(\"Bronchitis^False\", \"0.7\", \"Smoking^False\");");
    _sleep(1000);

    net->SetPTabular("Bronchitis^True", "0.4", "Smoking^True");
    printf("\n net->SetPTabular(\"Bronchitis^True\", \"0.4\", \"Smoking^True\");");
    _sleep(1000);

    net->SetPTabular("Bronchitis^False", "0.6", "Smoking^True");
    printf("\n net->SetPTabular(\"Bronchitis^False\", \"0.6\", \"Smoking^True\");");
    _sleep(1000);

    net->SetPTabular("LungCancer^True", "0.01","Smoking^False");
    printf("\n net->SetPTabular(\"LungCancer^True\", \"0.01\",\"Smoking^False\");");
    _sleep(1000);

    net->SetPTabular("LungCancer^False", "0.99","Smoking^False");
    printf("\n net->SetPTabular(\"LungCancer^False\", \"0.99\",\"Smoking^False\");");
    _sleep(1000);

    net->SetPTabular("LungCancer^True", "0.1","Smoking^True");
    printf("\n net->SetPTabular(\"LungCancer^True\", \"0.1\",\"Smoking^True\");");
    _sleep(1000);

    net->SetPTabular("LungCancer^False", "0.9","Smoking^True");
    printf("\n net->SetPTabular(\"LungCancer^False\", \"0.9\",\"Smoking^True\");");
    _sleep(1000);

    net->SetPTabular("Tuberculosis^True", "0.01", "VisitToAsia^False");
    printf("\n net->SetPTabular(\"Tuberculosis^True\", \"0.01\", \"VisitToAsia^False\");");
    _sleep(1000);

    net->SetPTabular("Tuberculosis^False", "0.99", "VisitToAsia^False");
    printf("\n net->SetPTabular(\"Tuberculosis^False\", \"0.99\", \"VisitToAsia^False\");");
    _sleep(1000);

    net->SetPTabular("Tuberculosis^True", "0.05", "VisitToAsia^True" );
    printf("\n net->SetPTabular(\"Tuberculosis^True\", \"0.05\", \"VisitToAsia^True\" );");
    _sleep(1000);
  
    net->SetPTabular("Tuberculosis^False", "0.95", "VisitToAsia^True" );
    printf("\n net->SetPTabular(\"Tuberculosis^False\", \"0.95\", \"VisitToAsia^True\" );");
    _sleep(1000);

    net->SetPTabular("CancerOrTuberculosis^True", "0.0", "LungCancer^False Tuberculosis^False");
    printf("\n net->SetPTabular(\"CancerOrTuberculosis^True\", \"0.0\", \n \t \"LungCancer^False Tuberculosis^False\");");
    _sleep(1000);
    
    net->SetPTabular("CancerOrTuberculosis^False", "1.0", "LungCancer^False Tuberculosis^False");
    printf("\n net->SetPTabular(\"CancerOrTuberculosis^False\", \"1.0\", \n \t \"LungCancer^False Tuberculosis^False\");");
    _sleep(1000);

    net->SetPTabular("CancerOrTuberculosis^True CancerOrTuberculosis^False", "1.0 0.0", "LungCancer^False Tuberculosis^True");
    printf("\n net->SetPTabular(\"CancerOrTuberculosis^True CancerOrTuberculosis^False\",\n \t \"1.0 0.0\", \"LungCancer^False Tuberculosis^True\");");
    _sleep(1000);
    
    net->SetPTabular("CancerOrTuberculosis^True CancerOrTuberculosis^False", "1.0 0.0", "LungCancer^True Tuberculosis^False");
    printf("\n net->SetPTabular(\"CancerOrTuberculosis^True CancerOrTuberculosis^False\", \n \t\"1.0 0.0\", \"LungCancer^True Tuberculosis^False\");");
    _sleep(1000);
    
    net->SetPTabular("CancerOrTuberculosis^True CancerOrTuberculosis^False", "1.0 0.0", "LungCancer^True Tuberculosis^True");
    printf("\n net->SetPTabular(\"CancerOrTuberculosis^True CancerOrTuberculosis^False\", \n \t\"1.0 0.0\", \"LungCancer^True Tuberculosis^True\");");
    _sleep(1000);

    net->SetPTabular("Dyspnoea^True Dyspnoea^False", "0.1 0.9", "Bronchitis^False CancerOrTuberculosis^False");
    printf("\n net->SetPTabular(\"Dyspnoea^True Dyspnoea^False\", \"0.1 0.9\", \n \t\"Bronchitis^False CancerOrTuberculosis^False\");");
    _sleep(1000);
    
    net->SetPTabular("Dyspnoea^True Dyspnoea^False", "0.7 0.3", "Bronchitis^False CancerOrTuberculosis^True");
    printf("\n net->SetPTabular(\"Dyspnoea^True Dyspnoea^False\", \"0.7 0.3\", \n \t\"Bronchitis^False CancerOrTuberculosis^True\");");
    _sleep(1000);
    
    net->SetPTabular("Dyspnoea^True Dyspnoea^False", "0.8 0.2", "Bronchitis^True CancerOrTuberculosis^False");
    printf("\n net->SetPTabular(\"Dyspnoea^True Dyspnoea^False\", \"0.8 0.2\", \n \t\"Bronchitis^True CancerOrTuberculosis^False\");");
    _sleep(1000);
    
    net->SetPTabular("Dyspnoea^True Dyspnoea^False", "0.9 0.1", "Bronchitis^True CancerOrTuberculosis^True");
    printf("\n net->SetPTabular(\"Dyspnoea^True Dyspnoea^False\", \"0.9 0.1\", \n \t\"Bronchitis^True CancerOrTuberculosis^True\");");
    _sleep(1000);

    net->SetPTabular("XRayPositive^True XRayPositive^False", "0.05 0.95", "CancerOrTuberculosis^False");	
    printf("\n net->SetPTabular(\"XRayPositive^True XRayPositive^False\", \"0.05 0.95\", \n \t\"CancerOrTuberculosis^False\");");
    _sleep(1000);
    
    net->SetPTabular("XRayPositive^True XRayPositive^False", "0.98 0.02", "CancerOrTuberculosis^True");
    printf("\n net->SetPTabular(\"XRayPositive^True XRayPositive^False\", \"0.98 0.02\", \n \t\"CancerOrTuberculosis^True\");");
    _sleep(1000);

    textcolor(LIGHTGREEN);
    printf("\n ......All distributions are added....\n");
    textcolor(WHITE);
    getch();
    return net;
}

LIMID *PigsModel()
{
    // Steffen L. Lauritzen, Dennis Nilsson 
    // Representing and Solving Decision Problems with Limited Information, 2001

    LIMID *net;
    net = new LIMID();

    textcolor(WHITE);
    net->AddNode(chance^"h1 t1", "False True");
    printf("\n net->AddNode(chance^\"h1 t1\", \"False True\");");
    
    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\t\tAdding of nodes is in process....");
    _sleep(2000);
    textcolor(WHITE);

    net->AddNode(decision^"d1", "False True");
    printf("\n net->AddNode(decision^\"d1\", \"False True\");");
    _sleep(1000);

    net->AddNode(value^"u1", "Cost");    
    printf("\n net->AddNode(value^\"u1\", \"Cost\");");
    _sleep(1000);

    net->AddNode(chance^"h2 t2", "False True");
    printf("\n net->AddNode(chance^\"h2 t2\", \"False True\");");
    _sleep(1000);

    net->AddNode(decision^"d2", "False True");
    printf("\n net->AddNode(decision^\"d2\", \"False True\");");
    _sleep(1000);

    net->AddNode(value^"u2", "Cost");
    printf("\n net->AddNode(value^\"u2\", \"Cost\");");
    _sleep(1000);

    net->AddNode(chance^"h3 t3", "False True");
    printf("\n net->AddNode(chance^\"h3 t3\", \"False True\");");
    _sleep(1000);

    net->AddNode(decision^"d3", "False True");
    printf("\n net->AddNode(decision^\"d3\", \"False True\");");
    _sleep(1000);

    net->AddNode(value^"u3", "Cost");
    printf("\n net->AddNode(value^\"u3\", \"Cost\");");
    _sleep(1000);

    net->AddNode(chance^"h4", "False True");
    printf("\n net->AddNode(chance^\"h4\", \"False True\");");
    _sleep(1000);

    net->AddNode(value^"u4", "Cost");
    printf("\n net->AddNode(value^\"u4\", \"Cost\");");

    textcolor(LIGHTGREEN);
    printf("\n ......All nodes are added....\n");
    getch();

    // arcs
    textcolor(WHITE);
    net->AddArc("h1", "h2 t1");
    printf("\n net->AddArc(\"h1\", \"h2 t1\");");

    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\t\t\t\tAdding of arcs is in process....");
    _sleep(2000);

    textcolor(WHITE);    
    net->AddArc("h2", "h3 t2");
    printf("\n net->AddArc(\"h2\", \"h3 t2\");");
    _sleep(1000);
    
    net->AddArc("t1", "d1");
    printf("\n net->AddArc(\"t1\", \"d1\");");
    _sleep(1000);
    
    net->AddArc("t2", "d2");
    printf("\n net->AddArc(\"t2\", \"d2\");");
    _sleep(1000);
    
    net->AddArc("d1", "h2 u1");
    printf("\n net->AddArc(\"d1\", \"h2 u1\");");
    _sleep(1000);
    
    net->AddArc("h3", "h4 t3");
    printf("\n net->AddArc(\"h3\", \"h4 t3\");");
    _sleep(1000);
    
    net->AddArc("t3", "d3");
    printf("\n net->AddArc(\"t3\", \"d3\");");
    _sleep(1000);
    
    net->AddArc("d2", "h3 u2");
    printf("\n net->AddArc(\"d2\", \"h3 u2\");");
    _sleep(1000);
    
    net->AddArc("d3", "h4 u3");
    printf("\n net->AddArc(\"d3\", \"h4 u3\");");
    _sleep(1000);
    
    net->AddArc("h4", "u4");
    printf("\n net->AddArc(\"h4\", \"u4\");");
    _sleep(1000);

    textcolor(LIGHTGREEN);
    printf("\n ......All arcs are added....\n");
    getch();

    // distributions
    textcolor(WHITE);

    net->SetPChance("h1^False h1^True", "0.9 0.1");
    printf("\n net->SetPChance(\"h1^False h1^True\", \"0.9 0.1\");");

    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\tAdding of distributions is in process....");
    _sleep(2000);
    textcolor(WHITE);

    net->SetPChance("t1^False t1^True", "0.1 0.9", "h1^False");
    printf("\n net->SetPChance(\"t1^False t1^True\", \"0.1 0.9\", \"h1^False\");");
    _sleep(1000);
    
    net->SetPChance("t1^False t1^True", "0.8 0.2", "h1^True");
    printf("\n net->SetPChance(\"t1^False t1^True\", \"0.8 0.2\", \"h1^True\");");
    _sleep(1000);
    
    net->SetPDecision("d1^False d1^True", "0.5 0.5", "t1^False");
    printf("\n net->SetPDecision(\"d1^False d1^True\", \"0.5 0.5\", \"t1^False\");");
    _sleep(1000);
    
    net->SetPDecision("d1^False d1^True", "0.5 0.5", "t1^True");
    printf("\n net->SetPDecision(\"d1^False d1^True\", \"0.5 0.5\", \"t1^True\");");
    _sleep(1000);

    net->SetValueCost("u1^Cost", "-100.0", "d1^False");
    printf("\n net->SetValueCost(\"u1^Cost\", \"-100.0\", \"d1^False\");");
    _sleep(1000);
    
    net->SetValueCost("u1^Cost", "0.0", "d1^True");
    printf("\n net->SetValueCost(\"u1^Cost\", \"0.0\", \"d1^True\");");
    _sleep(1000);
    
    net->SetPChance("h2^False h2^True", "0.9 0.1", "h1^False d1^False");
    printf("\n net->SetPChance(\"h2^False h2^True\", \"0.9 0.1\", \"h1^False d1^False\");");
    _sleep(1000);
    
    net->SetPChance("h2^False h2^True", "0.8 0.2", "h1^False d1^True");
    printf("\n net->SetPChance(\"h2^False h2^True\", \"0.8 0.2\", \"h1^False d1^True\");");
    _sleep(1000);
    
    net->SetPChance("h2^False h2^True", "0.5 0.5", "h1^True d1^False");
    printf("\n net->SetPChance(\"h2^False h2^True\", \"0.5 0.5\", \"h1^True d1^False\");");
    _sleep(1000);
    
    net->SetPChance("h2^False h2^True", "0.1 0.9", "h1^True d1^True");
    printf("\n net->SetPChance(\"h2^False h2^True\", \"0.1 0.9\", \"h1^True d1^True\");");
    _sleep(1000);
    
    net->SetPChance("t2^False t2^True", "0.1 0.9", "h2^False");
    printf("\n net->SetPChance(\"t2^False t2^True\", \"0.1 0.9\", \"h2^False\");");
    _sleep(1000);
    
    net->SetPChance("t2^False t2^True", "0.8 0.2", "h2^True");
    printf("\n net->SetPChance(\"t2^False t2^True\", \"0.8 0.2\", \"h2^True\");");
    _sleep(1000);
    
    net->SetPDecision("d2^False d2^True", "0.5 0.5", "t2^False");
    printf("\n net->SetPDecision(\"d2^False d2^True\", \"0.5 0.5\", \"t2^False\");");
    _sleep(1000);
    
    net->SetPDecision("d2^False d2^True", "0.5 0.5", "t2^True");
    printf("\n net->SetPDecision(\"d2^False d2^True\", \"0.5 0.5\", \"t2^True\");");
    _sleep(1000);
    
    net->SetValueCost("u2^Cost", "-100.0", "d2^False");
    printf("\n net->SetValueCost(\"u2^Cost\", \"-100.0\", \"d2^False\");");
    _sleep(1000);
    
    net->SetValueCost("u2^Cost", "0.0", "d2^True");
    printf("\n net->SetValueCost(\"u2^Cost\", \"0.0\", \"d2^True\");");
    _sleep(1000);
    
    net->SetPChance("h3^False h3^True", "0.9 0.1", "h2^False d2^False");
    printf("\n net->SetPChance(\"h3^False h3^True\", \"0.9 0.1\", \"h2^False d2^False\");");
    _sleep(1000);
    
    net->SetPChance("h3^False h3^True", "0.5 0.5", "h2^False d2^True");
    printf("\n net->SetPChance(\"h3^False h3^True\", \"0.5 0.5\", \"h2^False d2^True\");");
    _sleep(1000);
    
    net->SetPChance("h3^False h3^True", "0.8 0.2", "h2^True d2^False");
    printf("\n net->SetPChance(\"h3^False h3^True\", \"0.8 0.2\", \"h2^True d2^False\");");
    _sleep(1000);
    
    net->SetPChance("h3^False h3^True", "0.1 0.9", "h2^True d2^True");
    printf("\n net->SetPChance(\"h3^False h3^True\", \"0.1 0.9\", \"h2^True d2^True\");");
    _sleep(1000);
    
    net->SetPChance("t3^False t3^True", "0.1 0.9", "h3^False");
    printf("\n net->SetPChance(\"t3^False t3^True\", \"0.1 0.9\", \"h3^False\");");
    _sleep(1000);
    
    net->SetPChance("t3^False t3^True", "0.8 0.2", "h3^True");
    printf("\n net->SetPChance(\"t3^False t3^True\", \"0.8 0.2\", \"h3^True\");");
    _sleep(1000);
    
    net->SetPDecision("d3^False d3^True", "0.5 0.5", "t3^False");
    printf("\n net->SetPDecision(\"d3^False d3^True\", \"0.5 0.5\", \"t3^False\");");
    _sleep(1000);
    
    net->SetPDecision("d3^False d3^True", "0.5 0.5", "t3^True");
    printf("\n net->SetPDecision(\"d3^False d3^True\", \"0.5 0.5\", \"t3^True\");");
    _sleep(1000);
    
    net->SetValueCost("u3^Cost", "-100.0", "d3^False");
    printf("\n net->SetValueCost(\"u3^Cost\", \"-100.0\", \"d3^False\");");
    _sleep(1000);
    
    net->SetValueCost("u3^Cost", "0.0", "d3^True");
    printf("\n net->SetValueCost(\"u3^Cost\", \"0.0\", \"d3^True\");");
    _sleep(1000);
    
    net->SetPChance("h4^False h4^True", "0.9 0.1", "h3^False d3^False");
    printf("\n net->SetPChance(\"h4^False h4^True\", \"0.9 0.1\", \"h3^False d3^False\");");
    _sleep(1000);
    
    net->SetPChance("h4^False h4^True", "0.8 0.2", "h3^False d3^True");
    printf("\n net->SetPChance(\"h4^False h4^True\", \"0.8 0.2\", \"h3^False d3^True\");");
    _sleep(1000);
    
    net->SetPChance("h4^False h4^True", "0.5 0.5", "h3^True d3^False");
    printf("\n net->SetPChance(\"h4^False h4^True\", \"0.5 0.5\", \"h3^True d3^False\");");
    _sleep(1000);
    
    net->SetPChance("h4^False h4^True", "0.1 0.9", "h3^True d3^True");
    printf("\n net->SetPChance(\"h4^False h4^True\", \"0.1 0.9\", \"h3^True d3^True\");");
    _sleep(1000);
    
    net->SetValueCost("u4^Cost", "1000.0", "h4^False");
    printf("\n net->SetValueCost(\"u4^Cost\", \"1000.0\", \"h4^False\");");
    _sleep(1000);
    
    net->SetValueCost("u4^Cost", "300.0", "h4^True");
    printf("\n net->SetValueCost(\"u4^Cost\", \"300.0\", \"h4^True\");");

    textcolor(LIGHTGREEN);
    printf("\n ......All distributions are added....\n");
    textcolor(WHITE);
    getch();

    return net;
}


BayesNet *ScalarGaussianBNetModel()
{
    // NodeA  NodeB 
    //     \ /
    //    NodeC
    //     / \
    // NodeD  NodeE

    BayesNet *net;
    net = new BayesNet();

    textcolor(WHITE);
    net->AddNode(continuous^"NodeA NodeB NodeC", "dim1");
    printf("\n net->AddNode(continuous^\"NodeA NodeB NodeC\", \"dim1\");");
    textcolor(LIGHTGREEN);

    printf("\t\t\t\tAdding of nodes is in process....");
    _sleep(2000);
    textcolor(WHITE);

    net->AddNode(continuous^"NodeD NodeE", "dim1");
    printf("\n net->AddNode(continuous^\"NodeD NodeE\", \"dim1\");");
    _sleep(1000);

    textcolor(LIGHTGREEN);
    printf("\n ......All nodes are added....\n");
    getch();

    // arcs
    textcolor(WHITE);
    net->AddArc("NodeA NodeB", "NodeC");
    printf("\n net->AddArc(\"NodeA NodeB\", \"NodeC\");");
    
    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\t\tAdding of arcs is in process....");
    _sleep(2000);

    textcolor(WHITE);
    
    net->AddArc("NodeC", "NodeD NodeE");
    printf("\n net->AddArc(\"NodeC\", \"NodeD NodeE\");");
    _sleep(1000);

    textcolor(LIGHTGREEN);
    printf("\n ......All arcs are added....\n");
    getch();
    // distributions
    textcolor(WHITE);

    net->SetPGaussian("NodeA", "1.0", "4.0");
    printf("\n net->SetPGaussian(\"NodeA\", \"1.0\", \"4.0\");");
    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\tAdding of distributions is in process....");
    _sleep(2000);

    textcolor(WHITE);

    net->SetPGaussian("NodeB", "1.0", "1.0");
    printf("\n net->SetPGaussian(\"NodeB\", \"1.0\", \"1.0\");");
    _sleep(1000);

    net->SetPGaussian("NodeC", "0.0", "2.0", "1.0 2.0");
    printf("\n net->SetPGaussian(\"NodeC\", \"0.0\", \"2.0\", \"1.0 2.0\");");
    _sleep(1000);

    net->SetPGaussian("NodeD", "0.0", "4.0", "1.1");
    printf("\n net->SetPGaussian(\"NodeD\", \"0.0\", \"4.0\", \"1.1\");");
    _sleep(1000);

    net->SetPGaussian("NodeE", "-0.8", "1.2", "2.0");
    printf("\n net->SetPGaussian(\"NodeE\", \"-0.8\", \"1.2\", \"2.0\");");
    textcolor(LIGHTGREEN);
    printf("\n ......All distributions are added....\n");
    getch();
    textcolor(WHITE);

    return net;
}

BayesNet *WasteModel()
{
    BayesNet *net;
    net = new BayesNet();

    textcolor(WHITE);

    net->AddNode(discrete^"TypeOfWaste", "industrial household");
    net->AddNode(discrete^"FilterState", "intact defective");
    net->AddNode(discrete^"BurningRegime", "stable unstable");

    printf("\n net->AddNode(discrete^\"TypeOfWaste\", \"industrial household\");");
    textcolor(LIGHTGREEN);

    printf("\t\t\t\tAdding of nodes is in process....");
    _sleep(2000);
    textcolor(WHITE);

    printf("\n net->AddNode(discrete^\"FilterState\", \"intact defective\");");
    _sleep(1000);
    printf("\n net->AddNode(discrete^\"BurningRegime\", \"stable unstable\");");
    _sleep(1000);
    net->AddNode(continuous^"FilterEfficiency CO2Emission DustEmission", "dim1");
    printf("\n net->AddNode(continuous^\"FilterEfficiency CO2Emission DustEmission\", \"dim1\");");
    _sleep(1000);

    net->AddNode(continuous^"MetalInWaste MetalEmission Light", "dim1");
    printf("\n net->AddNode(continuous^\"MetalInWaste MetalEmission Light\", \"dim1\");");
    
    textcolor(LIGHTGREEN);
    printf("\n ......All nodes are added....\n");
    getch();

    // arcs
    textcolor(WHITE);
    net->AddArc("TypeOfWaste FilterState", "FilterEfficiency");
    printf("\n net->AddArc(\"TypeOfWaste FilterState\", \"FilterEfficiency\");");

    textcolor(LIGHTGREEN);
    printf("\t\t\t\tAdding of arcs is in process....");
    _sleep(2000);

    textcolor(WHITE);
       
    net->AddArc("FilterState FilterEfficiency BurningRegime", "DustEmission") ;
    printf("\n net->AddArc(\"FilterState FilterEfficiency BurningRegime\", \"DustEmission\") ;");
    _sleep(1000);

    net->AddArc( "FilterState", "MetalInWaste");
    printf("\n net->AddArc(\"FilterState\", \"MetalInWaste\");");
    _sleep(1000);

    net->AddArc( "BurningRegime", "CO2Emission");
    printf("\n net->AddArc(\"BurningRegime\", \"CO2Emission\");");
    _sleep(1000);
    
    net->AddArc( "DustEmission MetalInWaste", "MetalEmission");
    printf("\n net->AddArc(\"DustEmission MetalInWaste\", \"MetalEmission\");");
    _sleep(1000);
    
    net->AddArc( "MetalInWaste", "Light");
    printf("\n net->AddArc(\"MetalInWaste\", \"Light\");");
    _sleep(1000);
    textcolor(LIGHTGREEN);
    printf("\n ......All arcs are added....\n");
    getch();
    // distributions
    textcolor(WHITE);
    
    net->SetPTabular( "TypeOfWaste^industrial TypeOfWaste^household", "0.95 0.05");
    printf("\n net->SetPTabular(\"TypeOfWaste^industrial TypeOfWaste^household\", \"0.95 0.05\");");
    textcolor(LIGHTGREEN);
    printf("\t\tAdding of distributions is in process....");
    _sleep(2000);

    textcolor(WHITE);

    net->SetPTabular( "FilterState^intact FilterState^defective", "0.285714 0.714286");
    printf("\n net->SetPTabular(\"FilterState^intact FilterState^defective\", \"0.285714 0.714286\");");
    _sleep(1000);

    net->SetPTabular( "BurningRegime^stable BurningRegime^unstable", "0.85 0.15");
    printf("\n net->SetPTabular(\"BurningRegime^stable BurningRegime^unstable\", \"0.85 0.15\");");
    _sleep(1000);

    net->SetPGaussian( "FilterEfficiency", "-3.2", "0.00002", "", "TypeOfWaste^industrial FilterState^defective");
    printf("\n net->SetPGaussian(\"FilterEfficiency\", \"-3.2\", \"0.00002\", \"\", \n\t\t\"TypeOfWaste^industrial FilterState^defective\");");
    _sleep(1000);

    net->SetPGaussian( "FilterEfficiency", "-0.5", "0.0001", "", "TypeOfWaste^industrial FilterState^intact");
    printf("\n net->SetPGaussian(\"FilterEfficiency\", \"-0.5\", \"0.0001\", \"\", \n\t\t\"TypeOfWaste^industrial FilterState^intact\");");
    _sleep(1000);

    net->SetPGaussian( "FilterEfficiency", "-3.9", "0.00002", "", "TypeOfWaste^household FilterState^defective");
    printf("\n net->SetPGaussian(\"FilterEfficiency\", \"-3.9\", \"0.00002\", \"\", \n\t\t\"TypeOfWaste^household FilterState^defective\");");
    _sleep(1000);

    net->SetPGaussian( "FilterEfficiency", "-0.4", "0.0001", "", "TypeOfWaste^household FilterState^intact");
    printf("\n net->SetPGaussian(\"FilterEfficiency\", \"-0.4\", \"0.0001\", \"\", \n\t\t\"TypeOfWaste^household FilterState^intact\");");
    _sleep(1000);


    net->SetPGaussian( "CO2Emission", "-2", "0.1", "", "BurningRegime^stable");
    printf("\n net->SetPGaussian(\"CO2Emission\", \"-2\", \"0.1\", \"\", \"BurningRegime^stable\");");
    _sleep(1000);

    net->SetPGaussian( "CO2Emission", "-1", "0.3", "", "BurningRegime^unstable");
    printf("\n net->SetPGaussian(\"CO2Emission\", \"-1\", \"0.3\", \"\", \"BurningRegime^unstable\");");
    _sleep(1000);


    net->SetPGaussian( "DustEmission", "6.5", "0.03", "1.0", "FilterState^intact BurningRegime^stable");
    printf("\n net->SetPGaussian(\"DustEmission\", \"6.5\", \"0.03\", \"1.0\", \"FilterState^intact \n\t\tBurningRegime^stable\");");
    _sleep(1000);

    net->SetPGaussian( "DustEmission", "7.5", "0.1", "1.0", "FilterState^intact BurningRegime^unstable");
    printf("\n net->SetPGaussian(\"DustEmission\", \"7.5\", \"0.1\", \"1.0\", \"FilterState^intact \n\t\tBurningRegime^unstable\");");
    _sleep(1000);

    net->SetPGaussian( "DustEmission", "6.0", "0.04", "1.0", "FilterState^defective BurningRegime^stable");
    printf("\n net->SetPGaussian(\"DustEmission\", \"6.0\", \"0.04\", \"1.0\", \"FilterState^defective \n\t\tBurningRegime^stable\");");
    _sleep(1000);

    net->SetPGaussian( "DustEmission", "7.0", "0.01", "1.0", "FilterState^defective BurningRegime^unstable");
    printf("\n net->SetPGaussian(\"DustEmission\", \"7.0\", \"0.01\", \"1.0\", \"FilterState^defective \n\t\tBurningRegime^unstable\");");
    _sleep(1000);


    net->SetPGaussian( "MetalInWaste", "0.5", "0.01", "", "FilterState^intact");
    printf("\n net->SetPGaussian(\"MetalInWaste\", \"0.5\", \"0.01\", \"\", \"FilterState^intact\");");
    _sleep(1000);

    net->SetPGaussian( "MetalInWaste", "-0.5", "0.005", "", "FilterState^defective");
    printf("\n net->SetPGaussian(\"MetalInWaste\", \"-0.5\", \"0.005\", \"\", \"FilterState^defective\");");
    _sleep(1000);


    net->SetPGaussian( "MetalEmission", "0.0", "0.02", "1.0 1.0");
    printf("\n net->SetPGaussian(\"MetalEmission\", \"0.0\", \"0.02\", \"1.0 1.0\");");
    _sleep(1000);

    net->SetPGaussian( "Light", "3.0", "0.25", "-0.5");
    printf("\n net->SetPGaussian(\"Light\", \"3.0\", \"0.25\", \"-0.5\");");
    _sleep(1000);
    textcolor(LIGHTGREEN);
    printf("\n ......All distributions are added....\n");
    getch();
    textcolor(WHITE);

    return net;
}


DBN* KjaerulfsBNetModel()
{
    DBN *net;
    net = new DBN();
    
    textcolor(WHITE);

    net->AddNode(discrete^"node0-0 node1-0 node2-0 node3-0 node4-0 node5-0 node6-0 node7-0", "true false");
    printf("\n net->AddNode(discrete^\"node0-0 node1-0 node2-0 \n\t node3-0 node4-0 node5-0 node6-0 node7-0\", \"true false\");");
    textcolor(LIGHTGREEN);
    
    printf("\t\t\tAdding of nodes is in process....");
    _sleep(2000);
    textcolor(WHITE);
    
    net->AddNode(discrete^"node0-1 node1-1 node2-1 node3-1 node4-1 node5-1 node6-1 node7-1", "true false");
    printf("\n net->AddNode(discrete^\"node0-1 node1-1 node2-1 \n\t node3-1 node4-1 node5-1 node6-1 node7-1\", \"true false\");");
    _sleep(1000);
    
    textcolor(LIGHTGREEN);
    printf("\n ......All nodes are added....\n");
    getch();
    
    // arcs
    textcolor(WHITE);

    net->AddArc("node0-0", "node1-0 node2-0");
    printf("\n net->AddArc(\"node0-0\", \"node1-0 node2-0\");");
    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\t\tAdding of arcs is in process....");
    _sleep(2000);

    textcolor(WHITE);
   
    net->AddArc("node1-0", "node2-0");
    printf("\n net->AddArc(\"node1-0\", \"node2-0\");");
    
    net->AddArc("node2-0", "node3-0");
    printf("\n net->AddArc(\"node2-0\", \"node3-0\");");
    
    net->AddArc("node3-0", "node4-0 node5-0");
    printf("\n net->AddArc(\"node3-0\", \"node4-0 node5-0\");");
    
    net->AddArc("node4-0 node5-0", "node6-0");
    printf("\n net->AddArc(\"node4-0 node5-0\", \"node6-0\");");
    
    net->AddArc("node6-0", "node7-0");
    printf("\n net->AddArc(\"node6-0\", \"node7-0\");");

    net->AddArc("node0-1", "node1-1 node2-1");
    printf("\n net->AddArc(\"node0-1\", \"node1-1 node2-1\");");
    
    net->AddArc("node1-1", "node2-1");
    printf("\n net->AddArc(\"node1-1\", \"node2-1\");");
    
    net->AddArc("node2-1", "node3-1");
    printf("\n net->AddArc(\"node2-1\", \"node3-1\");");
    
    net->AddArc("node3-1", "node4-1 node5-1");
    printf("\n net->AddArc(\"node3-1\", \"node4-1 node5-1\");");
    
    net->AddArc("node4-1 node5-1", "node6-1");
    printf("\n net->AddArc(\"node4-1 node5-1\", \"node6-1\");");
    
    net->AddArc("node6-1", "node7-1");
    printf("\n net->AddArc(\"node6-1\", \"node7-1\");");

    net->AddArc("node0-0", "node0-1");
    printf("\n net->AddArc(\"node0-0\", \"node0-1\");");
    
    net->AddArc("node3-0", "node3-1");
    printf("\n net->AddArc(\"node3-0\", \"node3-1\");");
    
    net->AddArc("node7-0", "node7-1");
    printf("\n net->AddArc(\"node7-0\", \"node7-1\");");
    textcolor(LIGHTGREEN);
    printf("\n ......All arcs are added....\n");
    getch();
    // distributions
    textcolor(WHITE);

    net->SetPTabular("node0-0^false node0-0^true", "0.5 0.5");
    printf("\n net->SetPTabular(\"node0-0^false node0-0^true\", \"0.5 0.5\");");
    textcolor(LIGHTGREEN);
    printf("\t\t\t\tAdding of distributions is in process....");
    _sleep(2000);

    textcolor(WHITE);
    net->SetPTabular("node1-0^false node1-0^true", "0.98 0.02", "node0-0^false");
    printf("\n net->SetPTabular(\"node1-0^false node1-0^true\", \"0.98 0.02\",  \n\t \"node0-0^false\");");
    
    net->SetPTabular("node1-0^false node1-0^true", "0.5 0.5", "node0-0^true");
    printf("\n net->SetPTabular(\"node1-0^false node1-0^true\", \"0.5 0.5\",  \n\t \"node0-0^true\");");

    net->SetPTabular("node2-0^false node2-0^true", "1.0 0.0", "node0-0^false node1-0^false");
    printf("\n net->SetPTabular(\"node2-0^false node2-0^true\", \"1.0 0.0\",  \n\t \"node0-0^false node1-0^false\");");
    
    net->SetPTabular("node2-0^false node2-0^true", "0.0 1.0", "node0-0^false node1-0^true");
    printf("\n net->SetPTabular(\"node2-0^false node2-0^true\", \"0.0 1.0\",  \n\t \"node0-0^false node1-0^true\");");
    
    net->SetPTabular("node2-0^false node2-0^true", "0.0 1.0", "node0-0^true node1-0^false");
    printf("\n net->SetPTabular(\"node2-0^false node2-0^true\", \"0.0 1.0\",  \n\t \"node0-0^true node1-0^false\");");
    
    net->SetPTabular("node2-0^false node2-0^true", "0.5 0.5", "node0-0^true node1-0^true");
    printf("\n net->SetPTabular(\"node2-0^false node2-0^true\", \"0.5 0.5\",  \n\t \"node0-0^true node1-0^true\");");


    net->SetPTabular("node3-0^false node3-0^true", "0.99 0.01", "node2-0^false");
    printf("\n net->SetPTabular(\"node3-0^false node3-0^true\", \"0.99 0.01\",  \n\t \"node2-0^false\");");
    
    net->SetPTabular("node3-0^false node3-0^true", "0.8 0.2", "node2-0^true");
    printf("\n net->SetPTabular(\"node3-0^false node3-0^true\", \"0.8 0.2\",  \n\t \"node2-0^true\");");

    net->SetPTabular("node4-0^false node4-0^true", "0.99 0.01", "node3-0^false");
    printf("\n net->SetPTabular(\"node4-0^false node4-0^true\", \"0.99 0.01\",  \n\t \"node3-0^false\");");
    
    net->SetPTabular("node4-0^false node4-0^true", "0.92 0.08", "node3-0^true");
    printf("\n net->SetPTabular(\"node4-0^false node4-0^true\", \"0.92 0.08\",  \n\t \"node3-0^true\");");

    net->SetPTabular("node5-0^false node5-0^true", "0.79 0.21", "node3-0^false");
    printf("\n net->SetPTabular(\"node5-0^false node5-0^true\", \"0.79 0.21\",  \n\t \"node3-0^false\");");
    
    net->SetPTabular("node5-0^false node5-0^true", "0.65 0.35", "node3-0^true");
    printf("\n net->SetPTabular(\"node5-0^false node5-0^true\", \"0.65 0.35\",  \n\t \"node3-0^true\");");


    net->SetPTabular("node6-0^false node6-0^true", "0.9 0.1", "node4-0^false node5-0^false");
    printf("\n net->SetPTabular(\"node6-0^false node6-0^true\", \"0.9 0.1\",  \n\t \"node4-0^false node5-0^false\");");
    
    net->SetPTabular("node6-0^false node6-0^true", "0.37 0.63", "node4-0^false node5-0^true");
    printf("\n net->SetPTabular(\"node6-0^false node6-0^true\", \"0.37 0.63\",  \n\t \"node4-0^false node5-0^true\");");
    
    net->SetPTabular("node6-0^false node6-0^true", "0.21 0.79", "node4-0^true node5-0^false");
    printf("\n net->SetPTabular(\"node6-0^false node6-0^true\", \"0.21 0.79\",  \n\t \"node4-0^true node5-0^false\");");
    
    net->SetPTabular("node6-0^false node6-0^true", "0.2 0.8", "node4-0^true node5-0^true");
    printf("\n net->SetPTabular(\"node6-0^false node6-0^true\", \"0.2 0.8\",  \n\t \"node4-0^true node5-0^true\");");


    net->SetPTabular("node7-0^false node7-0^true", "0.91 0.09", "node6-0^false");
    printf("\n net->SetPTabular(\"node7-0^false node7-0^true\", \"0.91 0.09\",  \n\t \"node6-0^false\");");
    
    net->SetPTabular("node7-0^false node7-0^true", "0.22 0.78", "node6-0^true");
    printf("\n net->SetPTabular(\"node7-0^false node7-0^true\", \"0.22 0.78\",  \n\t \"node6-0^true\");");

    net->SetPTabular("node0-1^false node0-1^true", "0.45 0.55", "node0-0^false");
    printf("\n net->SetPTabular(\"node0-1^false node0-1^true\", \"0.45 0.55\",  \n\t \"node0-0^false\");");

    net->SetPTabular("node0-1^false node0-1^true", "0.24 0.76", "node0-0^true");
    printf("\n net->SetPTabular(\"node0-1^false node0-1^true\", \"0.24 0.76\",  \n\t \"node0-0^true\");");

    net->SetPTabular("node1-1^false node1-1^true", "0.51 0.49", "node0-1^false");
    printf("\n net->SetPTabular(\"node1-1^false node1-1^true\", \"0.51 0.49\",  \n\t \"node0-1^false\");");
    
    net->SetPTabular("node1-1^false node1-1^true", "0.29 0.71", "node0-1^true");
    printf("\n net->SetPTabular(\"node1-1^false node1-1^true\", \"0.29 0.71\",  \n\t \"node0-1^true\");");

    net->SetPTabular("node2-1^false node2-1^true", "0.98 0.02", "node0-1^false node1-1^false");
    printf("\n net->SetPTabular(\"node2-1^false node2-1^true\", \"0.98 0.02\",  \n\t \"node0-1^false node1-1^false\");");
    
    net->SetPTabular("node2-1^false node2-1^true", "0.4 0.6", "node0-1^false node1-1^true");
    printf("\n net->SetPTabular(\"node2-1^false node2-1^true\", \"0.4 0.6\",  \n\t \"node0-1^false node1-1^true\");");
    
    net->SetPTabular("node2-1^false node2-1^true", "0.2 0.8", "node0-1^true node1-1^false");
    printf("\n net->SetPTabular(\"node2-1^false node2-1^true\", \"0.2 0.8\",  \n\t \"node0-1^true node1-1^false\");");
    
    net->SetPTabular("node2-1^false node2-1^true", "0.5 0.5", "node0-1^true node1-1^true");
    printf("\n net->SetPTabular(\"node2-1^false node2-1^true\", \"0.5 0.5\",  \n\t \"node0-1^true node1-1^true\");");

    net->SetPTabular("node3-1^false node3-1^true", "0.36 0.64", "node3-0^false node2-1^false");
    printf("\n net->SetPTabular(\"node3-1^false node3-1^true\", \"0.36 0.64\",  \n\t \"node3-0^false node2-1^false\");");
    
    net->SetPTabular("node3-1^false node3-1^true", "0.23 0.77", "node3-0^false node2-1^true");
    printf("\n net->SetPTabular(\"node3-1^false node3-1^true\", \"0.23 0.77\",  \n\t \"node3-0^false node2-1^true\");");
    
    net->SetPTabular("node3-1^false node3-1^true", "0.78 0.22", "node3-0^true node2-1^false");
    printf("\n net->SetPTabular(\"node3-1^false node3-1^true\", \"0.78 0.22\",  \n\t \"node3-0^true node2-1^false\");");
    
    net->SetPTabular("node3-1^false node3-1^true", "0.11 0.89", "node3-0^true node2-1^true");
    printf("\n net->SetPTabular(\"node3-1^false node3-1^true\", \"0.11 0.89\",  \n\t \"node3-0^true node2-1^true\");");

    net->SetPTabular("node4-1^false node4-1^true", "0.955 0.045", "node3-1^false");
    printf("\n net->SetPTabular(\"node4-1^false node4-1^true\", \"0.955 0.045\",  \n\t \"node3-1^false\");");
    
    net->SetPTabular("node4-1^false node4-1^true", "0.42 0.58", "node3-1^true");
    printf("\n net->SetPTabular(\"node4-1^false node4-1^true\", \"0.42 0.58\",  \n\t \"node3-1^true\");");

    net->SetPTabular("node5-1^false node5-1^true", "0.57 0.43", "node3-1^false");
    printf("\n net->SetPTabular(\"node5-1^false node5-1^true\", \"0.57 0.43\",  \n\t \"node3-1^false\");");
    
    net->SetPTabular("node5-1^false node5-1^true", "0.09 0.91", "node3-1^true");
    printf("\n net->SetPTabular(\"node5-1^false node5-1^true\", \"0.09 0.91\",  \n\t \"node3-1^true\");");

    net->SetPTabular("node6-1^false node6-1^true", "0.91 0.09", "node4-1^false node5-1^false");
    printf("\n net->SetPTabular(\"node6-1^false node6-1^true\", \"0.91 0.09\",  \n\t \"node4-1^false node5-1^false\");");
    
    net->SetPTabular("node6-1^false node6-1^true", "0.13 0.87", "node4-1^false node5-1^true");
    printf("\n net->SetPTabular(\"node6-1^false node6-1^true\", \"0.13 0.87\",  \n\t \"node4-1^false node5-1^true\");");
    
    net->SetPTabular("node6-1^false node6-1^true", "0.12 0.88", "node4-1^true node5-1^false");
    printf("\n net->SetPTabular(\"node6-1^false node6-1^true\", \"0.12 0.88\",  \n\t \"node4-1^true node5-1^false\");");
    
    net->SetPTabular("node6-1^false node6-1^true", "0.01 0.99", "node4-1^true node5-1^true");
    printf("\n net->SetPTabular(\"node6-1^false node6-1^true\", \"0.01 0.99\",  \n\t \"node4-1^true node5-1^true\");");

    net->SetPTabular("node7-1^false node7-1^true", "0.39 0.61", "node6-1^false node7-0^false");
    printf("\n net->SetPTabular(\"node7-1^false node7-1^true\", \"0.39 0.61\",  \n\t \"node6-1^false node7-0^false\");");
    
    net->SetPTabular("node7-1^false node7-1^true", "0.37 0.63", "node6-1^false node7-0^true");
    printf("\n net->SetPTabular(\"node7-1^false node7-1^true\", \"0.37 0.63\",  \n\t \"node6-1^false node7-0^true\");");
    
    net->SetPTabular("node7-1^false node7-1^true", "0.255 0.745", "node6-1^true node7-0^false");
    printf("\n net->SetPTabular(\"node7-1^false node7-1^true\", \"0.255 0.745\",  \n\t \"node6-1^true node7-0^false\");");
    
    net->SetPTabular("node7-1^false node7-1^true", "0.1 0.9", "node6-1^true node7-0^true");
    printf("\n net->SetPTabular(\"node7-1^false node7-1^true\", \"0.1 0.9\",  \n\t \"node6-1^true node7-0^true\");");

    _sleep(1000);
    textcolor(LIGHTGREEN);
    printf("\n ......All distributions are added....\n");
    textcolor(WHITE);
   
    return net;
    
}




BayesNet* CropModel()
{
    //  Subsidy(d) Crop(c)
    //          |   |
    //          V   V
    //         Price(c)
    //            |
    //            V
    //          Buy(d)

    BayesNet *net;
    net = new BayesNet();

    textcolor(WHITE);

    net->AddNode(discrete^"Subsidy", "Yes No");
    printf("\n net->AddNode(discrete^\"Subsidy\", \"Yes No\");");
    textcolor(LIGHTGREEN);
    
    printf("\t\t\t\t\t\tAdding of nodes is in process....");
    _sleep(2000);
    textcolor(WHITE);

    net->AddNode(continuous ^ "Crop");
    printf("\n net->AddNode(continuous ^ \"Crop\");");
    _sleep(1000);
    
    net->AddNode(continuous ^ "Price");
    printf("\n net->AddNode(continuous ^ \"Price\");");
    _sleep(1000);

    net->AddNode(discrete^"Buy", "Yes No");
    printf("\n net->AddNode(discrete^\"Buy\", \"Yes No\");");

    textcolor(LIGHTGREEN);
    printf("\n ......All nodes are added....\n");
    getch();
    
    // arcs
    textcolor(WHITE);

    net->AddArc("Subsidy Crop", "Price");
    printf("\n net->AddArc(\"Subsidy Crop\", \"Price\");");
    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\t\t\tAdding of arcs is in process....");
    _sleep(2000);
    
    textcolor(WHITE);
    net->AddArc("Price", "Buy");
    printf("\n net->AddArc(\"Price\", \"Buy\");");
    textcolor(LIGHTGREEN);
    printf("\n ......All arcs are added....\n");
    getch();
    
    // distributions
    textcolor(WHITE);

    net->SetPTabular("Subsidy^Yes Subsidy^No", "0.3 0.7");
    printf("\n net->SetPTabular(\"Subsidy^Yes Subsidy^No\", \"0.3 0.7\");");
    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\tAdding of distributions is in process....");
    _sleep(2000);

    textcolor(WHITE);

    net->SetPGaussian("Crop", "5.0", "1.0");
    printf("\n net->SetPGaussian(\"Crop\", \"5.0\", \"1.0\");");
    _sleep(1000);

    net->SetPGaussian("Price", "10.0", "1.0", "-1.0", "Subsidy^Yes");
    printf("\n net->SetPGaussian(\"Price\", \"10.0\", \"1.0\", \"-1.0\", \"Subsidy^Yes\");");
    _sleep(1000);
    
    net->SetPGaussian("Price", "20.0", "1.0", "-1.0", "Subsidy^No");
    printf("\n net->SetPGaussian(\"Price\", \"20.0\", \"1.0\", \"-1.0\", \"Subsidy^No\");");
    _sleep(1000);

    net->SetPSoftMax("Buy^Yes Buy^No", "-1.0 1.0", "5.0 -5.0");
    printf("\n net->SetPSoftMax(\"Buy^Yes Buy^No\", \"-1.0 1.0\", \"5.0 -5.0\");");

    textcolor(LIGHTGREEN);
    printf("\n ......All distributions are added....\n");
    getch();
    textcolor(WHITE);

    return net;
}

BayesNet* FraudModel()
{
//    David Heckerman
//    A Tutorial on Learning With Bayesian Networks, March 1995
    BayesNet *net;
    net = new BayesNet();

    textcolor(WHITE);
    net->AddNode(discrete^"Fraud", "Yes No");
    printf("\n net->AddNode(discrete^\"Fraud\", \"Yes No\");");
    textcolor(LIGHTGREEN);
    
    printf("\t\t\t\t\t\tAdding of nodes is in process....");
    _sleep(2000);
    textcolor(WHITE);
    net->AddNode(discrete^"Age", "Young MiddleAged InYears");
    printf("\n net->AddNode(discrete^\"Age\", \"Young MiddleAged InYears\");");
    _sleep(1000);
    
    net->AddNode(discrete^"Sex", "Male Female");
    printf("\n net->AddNode(discrete^\"Sex\", \"Male Female\");");
    _sleep(1000);
    
    net->AddNode(discrete^"Gas", "Yes No");
    printf("\n net->AddNode(discrete^\"Gas\", \"Yes No\");");
    _sleep(1000);
    
    net->AddNode(discrete^"Jewelry", "Yes No");
    printf("\n net->AddNode(discrete^\"Jewelry\", \"Yes No\");");
    textcolor(LIGHTGREEN);
    printf("\n ......All nodes are added....\n");
    textcolor(WHITE);

    getch();
    
/*    net->AddArc("Fraud Age Sex", "Jewelry");
    net->AddArc("Fraud", "Gas");

    net->SetPTabular("Fraud^Yes Fraud^No", "0.00001 0.99999");
    net->SetPTabular("Age^Young Age^Middle-Aged Age^Elderly", "0.25 0.40 0.35");
    net->SetPTabular("Sex^Male Sex^Female", "0.5 0.5");

    net->SetPTabular("Gas^Yes Gas^No", "0.2 0.8", "Fraud^Yes");
    net->SetPTabular("Gas^Yes Gas^No", "0.01 0.99", "Fraud^No");
 
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.05 0.95", "Fraud^Yes Age^Young Sex^Male");
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.05 0.95", "Fraud^Yes Age^Middle-Aged Sex^Male");
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.05 0.95", "Fraud^Yes Age^Elderly Sex^Male");

    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.05 0.95", "Fraud^Yes Age^Young Sex^Female");
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.05 0.95", "Fraud^Yes Age^Middle-Aged Sex^Female");
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.05 0.95", "Fraud^Yes Age^Elderly Sex^Female");
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.00001 0.99999", "Fraud^No Age^Young Sex^Male");
  
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.0004 0.9996", "Fraud^No Age^Middle-Aged Sex^Male");
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.0002 0.9998", "Fraud^No Age^Elderly Sex^Male");
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.00005 0.99995", "Fraud^No Age^Young Sex^Female");
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.002 0.998", "Fraud^No Age^Middle-Aged Sex^Female");
    net->SetPTabular("Jewelry^Yes Jewelry^No", "0.001 0.999", "Fraud^No Age^Elderly Sex^Female");
*/
    return net;
}

BayesNet* RPSModel()
{

    BayesNet *net;
    net = new BayesNet();

    TokArr aChoice = "Rock Paper Scissors";// possible values for nodes

    // build Graph
    // add nodes to net
    net->AddNode(discrete ^ "PreviousCompTurn PreviousHumanTurn CurrentHumanTurn", aChoice);

    // add arcs to create following Bayes net:
    //
    //  PreviousCompTurn    PreviousHumanTurn
    //               |         |
    //               V	       V
    //             CurrentHumanTurn
    net->AddArc("PreviousCompTurn", "CurrentHumanTurn");
    net->AddArc("PreviousHumanTurn", "CurrentHumanTurn");

    return net;
}

DBN* DBNModel()
{
    
    DBN *net;
    net = new DBN();

    textcolor(WHITE);
    
    net->AddNode(discrete^"Rain-0 Umbrella-0", "True False");
    printf("\n net->AddNode(discrete^\"Rain-0 Umbrella-0\", \"True False\");");
    textcolor(LIGHTGREEN);
    
    printf("\t\t\t\t\tAdding of nodes is in process....");
    _sleep(2000);

    textcolor(WHITE);
    net->AddNode(discrete^"Rain-1 Umbrella-1", "True False");
    printf("\n net->AddNode(discrete^\"Rain-1 Umbrella-1\", \"True False\");");
    textcolor(LIGHTGREEN);
    printf("\n ......All nodes are added....\n");
    getch();
    
    // arcs
    textcolor(WHITE);
    net->AddArc("Rain-0", "Umbrella-0");
    printf("\n net->AddArc(\"Rain-0\", \"Umbrella-0\");");
    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\t\t\t\tAdding of arcs is in process....");
    _sleep(2000);
    
    textcolor(WHITE);    
    net->AddArc("Rain-0", "Rain-1");
    printf("\n net->AddArc(\"Rain-0\", \"Rain-1\");");
    _sleep(1000);
    
    net->AddArc("Rain-1", "Umbrella-1");
    printf("\n net->AddArc(\"Rain-1\", \"Umbrella-1\");");
    textcolor(LIGHTGREEN);
    printf("\n ......All arcs are added....\n");
    getch();
    
    // distributions
    textcolor(WHITE);
    net->SetPTabular("Rain-0^True Rain-0^False", "0.5 0.5");
    printf("\n net->SetPTabular(\"Rain-0^True Rain-0^False\", \"0.5 0.5\");");
    textcolor(LIGHTGREEN);
    printf("\t\t\t\t\tAdding of distributions is in process....");
    _sleep(2000);

    textcolor(WHITE);
    net->SetPTabular("Umbrella-0^True Umbrella-0^False", "0.9 0.1", "Rain-0^True");
    printf("\n net->SetPTabular(\"Umbrella-0^True Umbrella-0^False\", \"0.9 0.1\", \"Rain-0^True\");");
    _sleep(1000);

    net->SetPTabular("Umbrella-0^True Umbrella-0^False", "0.2 0.8", "Rain-0^False");
    printf("\n net->SetPTabular(\"Umbrella-0^True Umbrella-0^False\", \"0.2 0.8\", \"Rain-0^False\");");
    _sleep(1000);

    net->SetPTabular("Rain-1^True Rain-1^False", "0.7 0.3", "Rain-0^True");
    printf("\n net->SetPTabular(\"Rain-1^True Rain-1^False\", \"0.7 0.3\", \"Rain-0^True\");");
    _sleep(1000);

    net->SetPTabular("Rain-1^True Rain-1^False", "0.3 0.7", "Rain-0^False");
    printf("\n net->SetPTabular(\"Rain-1^True Rain-1^False\", \"0.3 0.7\", \"Rain-0^False\");");
    _sleep(1000);

    net->SetPTabular("Umbrella-1^True Umbrella-1^False", "0.9 0.1", "Rain-1^True");
    printf("\n net->SetPTabular(\"Umbrella-1^True Umbrella-1^False\", \"0.9 0.1\", \"Rain-1^True\");");
    _sleep(1000);

    net->SetPTabular("Umbrella-1^True Umbrella-1^False", "0.2 0.8", "Rain-1^False");
    printf("\n net->SetPTabular(\"Umbrella-1^True Umbrella-1^False\", \"0.2 0.8\", \"Rain-1^False\");");
    textcolor(LIGHTGREEN);
    printf("\n ......All distributions are added....\n");
    getch();
    textcolor(WHITE);

    return net;
}


PNLW_END