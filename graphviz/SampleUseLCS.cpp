/* Version 1.01.  Mar 11, 2006 */
/* For documentation and updates go to www.DataOnStage.com. */
/*
 * Copyright (c) 2006 Imme Ebert-Uphoff
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "pnlHigh.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "models.h"
#include "PNLtoGraphviz.hpp"
#include "LinkConnectionStrengths.hpp"
#include <string>
#include <map>
using namespace std;


PNLW_USING

int main(int argc, char* argv[])
{

    if (argc <= 1)
    {
	cout << endl << "Please call with a model number [1-8] as argument!";
	cout << endl << endl; 
        cout << "Example:       \"SampleUseLCS 2\"";
	cout << endl << endl;

        cout << "--- Model Numbers ---" << endl << endl;
        cout << "To Generate Output on Screen:" << endl;
        cout << "  (1) Strong + Weak Links example (Figure 1 of manual)" <<endl;
        cout << "  (2) Two Node example with parameters (Figure 7 of manual)";
        cout << endl;
        cout << "  (3) Deterministic OR model (Figure 8 of manual)" << endl;
        cout << "  (4) Corrected Asia Model (Figure 13 of manual)" << endl;
        cout << endl;
        cout << "To Generate Graph Files:" << endl;
        cout << "  (5) Strong + Weak Links example (Figure 1 of manual)" <<endl;
        cout << "  (6) Two Node example with parameters (Figure 7 of manual)";
        cout << endl;
        cout << "  (7) Deterministic OR model (Figure 8 of manual)" << endl;
        cout << "  (8) Corrected Asia Model (Figure 13 of manual)" << endl;
        cout << "  (9) Two-Node Model WITHOUT uncertainty" << endl;
        cout << endl;

	return 1;
    }

 

   // get model number from argument
   int n = (argv[1])[0] - int('0');  

   // Load pre-defined model.
   cout << "Loading Model." << endl << endl;
   BayesNet * BNet_p;

   switch (n) { 
      case 1: case 5:
         cout << "Strong + Weak Links example (from Figure 1)." << endl;
         BNet_p =  Strong_Weak_example(); 
         break;

      case 2: case 6:
         cout <<"Two Node example with a=0.5. b1=0.1, b2=0.9 (Figure 7)" <<endl;
         BNet_p = TwoNode_example(0.5, 0.1, 0.9);
         break;

      case 3: case 7:
         cout << "Deterministic OR model: D=A v B v C" << endl;
         BNet_p = Deterministic_OR_model();
         break;
    
      case 4: case 8: 
         cout << "Corrected Asia Model (Figure 13)" << endl;
         BNet_p = AsiaModelCorrected();  
         break;

      case 9: 
	 cout << "Two-Node Model WITHOUT Uncertainty" << endl;
	 BNet_p = TwoNode_example(1.0,1.0,1.0);
         break;

      default:
         cout << "Model number not recognized!" << endl << endl;
         break; 
   } // end of switch


   // Now generate the desired output
   switch (n) {
   case 1: case 2: case 3: case 4: 
         cout << endl;
         cout << "Print Results on Screen" << endl;
         Print_Graph_Structure( BNet_p->Net().Graph() );
         Print_Entropy( *BNet_p );
         Print_Link_Strengths( *BNet_p, "TrueAverage", false);
         Print_Link_Strengths( *BNet_p, "TrueAverage", true);
         Print_Link_Strengths( *BNet_p, "BlindAverage", false);       
         Print_Link_Strengths( *BNet_p, "BlindAverage", true);
         Print_Mutual_Information_For_All_Nodes( *BNet_p, false);
         Print_Mutual_Information_For_All_Nodes( *BNet_p, true);
	 break;
   
   case 5: case 6: case 7: case 8: case 9:
	 cout << endl;
         cout << "Generate Sample Graph Files:" << endl << endl;

         PNLtoGraphviz_with_Entropy ( BNet_p, "Entropy.dot" );
         PNLtoGraphviz_with_LS ( BNet_p, "LS_Blind.dot", "BlindAverage", false);
         PNLtoGraphviz_with_LS ( BNet_p, "LS_Blind_P.dot", "BlindAverage",true);
         PNLtoGraphviz_with_LS ( BNet_p, "LS_True.dot", "TrueAverage", false );
         PNLtoGraphviz_with_LS ( BNet_p, "LS_True_P.dot", "TrueAverage", true );
         cout <<"Generate connection strength graphs only for first 2 nodes";
         cout << " here:" << endl << endl;
         PNLtoGraphviz_with_MI ( BNet_p, "MI_0.dot", 0, false );
         PNLtoGraphviz_with_MI ( BNet_p, "MI_1.dot", 1, false );
         PNLtoGraphviz_with_MI ( BNet_p, "MI_P_0.dot", 0, true );
         PNLtoGraphviz_with_MI ( BNet_p, "MI_P_1.dot", 1, true );
         cout << endl;
         cout <<" To generate graph rendering, use dot-command of GraphViz.";
         cout << endl << endl;
	 break;

      default:
         cout << "Model number not recognized!" << endl << endl;
         break; 
   } // end of switch

} // end of main
