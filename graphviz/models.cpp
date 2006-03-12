/* Version 1.01.  Mar 11, 2006 */
/* For documentation and updates go to www.DataOnStage.com. */

/* This file is derived from the file models.cpp provided in 
   directory PNL/high/DEMO of PNL Release 1.0.

   The following changes were made:

   * A few commands in the existing models were changed to eliminate 
     Windows-specific commands in order for the file to compile easily 
     also on other platforms.

   * The Asia Model was corrected in two ways and is now called 
     AsiaModelCorrected:
     1) Two CPT values were corrected for P(Bronchitis|Smoking)
     2) Order of node definition was changed to make it topologically sorted.
     Details of these changes are given below (directly before model).

   * New models were added at the beginning of the file to demonstrate
     link and connection strengths.

*/

#include "pnlHigh.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#define textcolor(x)
#define _sleep(x)
#define getch() getchar()
PNLW_BEGIN




//--------------------------------------------------------------------
// New models to demonstrate link and connection strengths
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Two-node Network with parameters for probabilities
//    Two binary nodes: A,B.
//    One arc: A->B
//    Probabilities:  
//       P(A=True) = a
//       P(B=True| A=True)  = b1
//       P(B=True| A=False) = b2
//   
BayesNet * TwoNode_example(double a, double b1, double b2)
{
    BayesNet *net;
    net = new BayesNet();

    // nodes
    net->AddNode(discrete^"A", "True False"); 
    printf("\n net->AddNode(discrete^\"A\", \"True False\");");

    net->AddNode(discrete^"B", "True False"); 
    printf("\n net->AddNode(discrete^\"B\", \"True False\");");

    // arcs
    net->AddArc("A", "B");
    printf("\n net->AddArc(\"A\", \"B\");");

    printf("\t\t\tAdding of distributions is in process....\n\n");
    // A has no parents

    // create string of the form "a 1-a"
    char probability_str[80];
    sprintf(probability_str, "%f %f", a, 1-a);
    // printf(probability_str);  
    net->SetPTabular("A^True A^False", probability_str);

    // net->SetPTabular("A^True A^False", "0.5 0.5");
    // B has parent A
    sprintf(probability_str, "%f %f", b1, 1-b1);
    net->SetPTabular("B^True B^False", probability_str, "A^True");

    sprintf(probability_str, "%f %f", b2, 1-b2);
    net->SetPTabular("B^True B^False", probability_str, "A^False");

    return net;
}

//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Example of strong/weak link connection from Figure 1 of manual.
//    Three nodes: A,B,C. 
//    Strong connection from X to Z, and from Z to Y.
//    Weak connection from X to Y.
BayesNet * Strong_Weak_example()
{
    BayesNet *net;
    net = new BayesNet();

    // nodes
    net->AddNode(discrete^"X", "True False"); 
    printf("\n net->AddNode(discrete^\"X\", \"True False\");");

    net->AddNode(discrete^"Y", "True False"); 
    printf("\n net->AddNode(discrete^\"Y\", \"True False\");");

    net->AddNode(discrete^"Z", "True False"); 
    printf("\n net->AddNode(discrete^\"Z\", \"True False\");");
    printf("\n ......All nodes are added....\n");

    // arcs
    net->AddArc("X", "Y Z");
    printf("\n net->AddArc(\"X\", \"Y Z\");");

    net->AddArc("Z", "Y");
    printf("\n net->AddArc(\"Z\", \"Y\");");
    printf("\n ......All arcs are added....\n");

    printf("\t\t\tAdding of distributions is in process....\n\n");
    // X has no parents
    net->SetPTabular("X^True X^False", "0.5 0.5");
    // Z has parent X
    net->SetPTabular("Z^True Z^False", "0.9 0.1", "X^True");
    net->SetPTabular("Z^True Z^False", "0.1 0.9", "X^False");
    // Y has parents X and Z
    net->SetPTabular("Y^True Y^False", "0.9 0.1", "X^True Z^True");
    net->SetPTabular("Y^True Y^False", "0.89 0.11", "X^False Z^True");
    net->SetPTabular("Y^True Y^False", "0.1 0.9", "X^True Z^False");
    net->SetPTabular("Y^True Y^False", "0.11 0.89", "X^False Z^False");

    return net;
}

//--------------------------------------------------------------------
// Example to test code on nodes with more than 2 states.
//    Three nodes: A,B,C. 
//    A has 4 states.  B has 2 states.  C has 3 states. 
//    
BayesNet * States_4_2_3_example()
{
    BayesNet *net;
    net = new BayesNet();

    // nodes
    net->AddNode(discrete^"A", "a1 a2 a3 a4"); 
    printf("\n net->AddNode(discrete^\"A\", \"a1 a2 a3 a4\");");

    net->AddNode(discrete^"B", "b1 b2"); 
    printf("\n net->AddNode(discrete^\"B\", \"b1 b2\");");

    net->AddNode(discrete^"C", "c1 c2 c3"); 
    printf("\n net->AddNode(discrete^\"C\", \"c1 c2 c3\");");
    printf("\n ......All nodes are added....\n");

    // arcs
    net->AddArc("A", "B C");
    printf("\n net->AddArc(\"A\", \"B C\");");

    net->AddArc("B", "C");
    printf("\n net->AddArc(\"B\", \"C\");");
    printf("\n ......All arcs are added....\n");


    printf("\t\t\tAdding of distributions is in process....\n\n");
    // A has no parents
    net->SetPTabular("A^a1 A^a2 A^a3 A^a4", "0.2 0.1 0.55 0.15");


    // B has parent A
    net->SetPTabular("B^b1 B^b2", "0.1 0.9", "A^a1");
    net->SetPTabular("B^b1 B^b2", "0.2 0.8", "A^a2");
    net->SetPTabular("B^b1 B^b2", "0.8 0.2", "A^a3");
    net->SetPTabular("B^b1 B^b2", "0.7 0.3", "A^a4");


    // C has parents A and B
    net->SetPTabular("C^c1 C^c2 C^c3", "0.2 0.3 0.5", "A^a1 B^b1");
    net->SetPTabular("C^c1 C^c2 C^c3", "0.4 0.5 0.1", "A^a1 B^b2");

    net->SetPTabular("C^c1 C^c2 C^c3", "0.6 0.2 0.2", "A^a2 B^b1");
    net->SetPTabular("C^c1 C^c2 C^c3", "0.5 0.3 0.2", "A^a2 B^b2");

    net->SetPTabular("C^c1 C^c2 C^c3", "0.1 0.5 0.4", "A^a3 B^b1");
    net->SetPTabular("C^c1 C^c2 C^c3", "0.0 0.4 0.6", "A^a3 B^b2");

    net->SetPTabular("C^c1 C^c2 C^c3", "0.4 0.3 0.3", "A^a4 B^b1");
    net->SetPTabular("C^c1 C^c2 C^c3", "0.9 0.1 0.0", "A^a4 B^b2");
#if 0
#endif
 
    return net;
}

//--------------------------------------------------------------------
// OR function (deterministic function)
//    Four-node model: A,B,C,D.  Each with two states: True, False.
//    D = A or B or C.
//
BayesNet * Deterministic_OR_model()
{
    BayesNet *net;
    net = new BayesNet();

    // nodes
    net->AddNode(discrete^"A", "True False"); 
    printf("\n net->AddNode(discrete^\"A\", \"True False\");");

    net->AddNode(discrete^"B", "True False"); 
    printf("\n net->AddNode(discrete^\"B\", \"True False\");");

    net->AddNode(discrete^"C", "True False"); 
    printf("\n net->AddNode(discrete^\"C\", \"True False\");");

    net->AddNode(discrete^"D", "True False"); 
    printf("\n net->AddNode(discrete^\"D\", \"True False\");");
    printf("\n ......All nodes are added....\n");

    // arcs: from A,B,C to D
    net->AddArc("A B C", "D");
    printf("\n net->AddArc(\"A B C\", \"D\");");
    printf("\n ......All arcs are added....\n");

    printf("\t\t\tAdding of distributions is in process....\n\n");
    // A,B,C have no parents
    net->SetPTabular("A^True A^False", "0.5 0.5");
    net->SetPTabular("B^True B^False", "0.5 0.5");
    net->SetPTabular("C^True C^False", "0.5 0.5");
    // D has parents A,B,C:  D = A v B v C
    net->SetPTabular("D^True D^False", "1.0 0.0", "A^True B^True C^True");
    net->SetPTabular("D^True D^False", "1.0 0.0", "A^True B^True C^False");
    net->SetPTabular("D^True D^False", "1.0 0.0", "A^True B^False C^True");
    net->SetPTabular("D^True D^False", "1.0 0.0", "A^True B^False C^False");

    net->SetPTabular("D^True D^False", "1.0 0.0", "A^False B^True C^True");
    net->SetPTabular("D^True D^False", "1.0 0.0", "A^False B^True C^False");
    net->SetPTabular("D^True D^False", "1.0 0.0", "A^False B^False C^True");
    net->SetPTabular("D^True D^False", "0.0 1.0", "A^False B^False C^False");
    return net;
}

//--------------------------------------------------------------------
// Max function (deterministic function)
//    Three nodes: A,B,D.  Each with 3 ordered states: S0 < S1 < S2.
//    D = max(A,B).
//
BayesNet * Deterministic_MAX_model()
{
    BayesNet *net;
    net = new BayesNet();

    // nodes
    net->AddNode(discrete^"A", "S0 S1 S2"); 
    printf("\n net->AddNode(discrete^\"A\", \"S0 S1 S2\");");

    net->AddNode(discrete^"B", "S0 S1 S2"); 
    printf("\n net->AddNode(discrete^\"B\", \"S0 S1 S2\");");

    net->AddNode(discrete^"D", "S0 S1 S2"); 
    printf("\n net->AddNode(discrete^\"D\", \"S0 S1 S2\");");
    printf("\n ......All nodes are added....\n");

    // arcs: from A,B to D
    net->AddArc("A B", "D");
    printf("\n net->AddArc(\"A B\", \"D\");");
    printf("\n ......All arcs are added....\n");

    printf("\t\t\tAdding of distributions is in process....\n\n");
    // A,B have no parents

    net->SetPTabular("A^S0 A^S1 A^S2", "0.333 0.333 0.334");
    net->SetPTabular("B^S0 B^S1 B^S2", "0.333 0.333 0.334");

    // D has parents A,B:  D = max(A,B)
    net->SetPTabular("D^S0 D^S1 D^S2", "1.0 0.0 0.0", "A^S0 B^S0");
    net->SetPTabular("D^S0 D^S1 D^S2", "0.0 1.0 0.0", "A^S0 B^S1");
    net->SetPTabular("D^S0 D^S1 D^S2", "0.0 0.0 1.0", "A^S0 B^S2");

    net->SetPTabular("D^S0 D^S1 D^S2", "0.0 1.0 0.0", "A^S1 B^S0");
    net->SetPTabular("D^S0 D^S1 D^S2", "0.0 1.0 0.0", "A^S1 B^S1");
    net->SetPTabular("D^S0 D^S1 D^S2", "0.0 0.0 1.0", "A^S1 B^S2");

    net->SetPTabular("D^S0 D^S1 D^S2", "0.0 0.0 1.0", "A^S2 B^S0");
    net->SetPTabular("D^S0 D^S1 D^S2", "0.0 0.0 1.0", "A^S2 B^S1");
    net->SetPTabular("D^S0 D^S1 D^S2", "0.0 0.0 1.0", "A^S2 B^S2");

    return net;
}

//--------------------------------------------------------------------
//  Models from PNLHigh distribution (from PNL/high/DEMO/models.h):
//
//   Only changes made here were to eliminate Windows-specific commands in 
//   order for the file to compile easily also on other platforms
//   and the correction of node order and the CPT for Bronchitis in AsiaModel,
//   which is now called AsiaModelCorrected.
//--------------------------------------------------------------------


/*
   The Asia Model was corrected in two ways (to correct errors in existing model):
   1) Two CPT Values corrected for P(Bronchitis|Smoking)
      P(Bronchitis=True |Smoking=True)=0.6
      P(Bronchitis=False|Smoking=True)=0.4

   2) Correction of node order:
      The order of definition of node Dyspnoea and CancerOrTuberculosis was 
      switched, because otherwise the model is not topologically sorted, which 
      can cause problems in particular when using he SaveNet function:
      Now CancerOrTuberculosis comes BEFORE Dyspnoea, since it's 
      a parent of that node.
*/

BayesNet *AsiaModelCorrected()
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

    // Move definition of CancerOrTuberculosis BEFORE Dyspnoea (Mar 10, 06)
    net->AddNode(discrete^"CancerOrTuberculosis", "True False");
    printf("\n net->AddNode(discrete^\"CancerOrTuberculosis\", \"True False\");");
    _sleep(1000);
     
    net->AddNode(discrete^"Dyspnoea", "True False"); 
    printf("\n net->AddNode(discrete^\"Dyspnoea\", \"True False\");");
    _sleep(1000);

    net->AddNode(discrete^"XRayPositive", "True False");
    printf("\n net->AddNode(discrete^\"XRayPositive\", \"True False\");");
    _sleep(1000);

    textcolor(LIGHTGREEN);
    printf("\n ......All nodes are added....\n");
    //getchar();

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
    //getchar();
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

    // This value was 0.4.  Corrected to 0.6. (I. Ebert-Uphoff, Jan 14, 2006)
    net->SetPTabular("Bronchitis^True", "0.6", "Smoking^True");
    printf("\n net->SetPTabular(\"Bronchitis^True\", \"0.6\", \"Smoking^True\");");
    _sleep(1000);

    // This value was 0.6.  Corrected to 04.  (I. Ebert-Uphoff, Jan 14, 2006)
    net->SetPTabular("Bronchitis^False", "0.4", "Smoking^True");
    printf("\n net->SetPTabular(\"Bronchitis^False\", \"0.4\", \"Smoking^True\");");
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
    //getchar();
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
    //getch();

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
    //getch();

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
    //getch();

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
    //getch();

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
    //getch();
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
    //getch();
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
    //getch();

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
    //getch();
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
    //getch();
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
    //getch();
    
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
    //getch();
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
    //getch();
    
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
    //getch();
    
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
    //getch();
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

    //getch();
    
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
    //getch();
    
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
    //getch();
    
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
    //getch();
    textcolor(WHITE);

    return net;
}


PNLW_END
