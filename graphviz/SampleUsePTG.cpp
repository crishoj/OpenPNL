/* Version 1.01.  Mar 11, 2006 */
/* For documentation and updates go to www.DataOnStage.com. */
/*
 * Copyright (c) 2005, 2006 Imme Ebert-Uphoff
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
#include <string>
#include <map>
using namespace std;

PNLW_USING

int main(int argc, char* argv[])
{

    if (argc <= 1)
    {
	cout << endl << "Please call with a model number [1-9] as argument!";
	cout << endl << endl; 
        cout << "Example:       \"SampleUsePTG 2\"";
	cout << endl << endl;
        cout << "Models are from models.cpp of PNL/high/Demo of PNL";
	cout << "Release 1.0." << endl << endl;

        cout << "--- Model Numbers ---" << endl;
        cout << "Discrete BNs:" << endl;
        cout << "  (1) Corrected AsiaModel" << endl;
        cout << "  (2) FraudModel" << endl << endl;
        cout << "Continuous BNs:" << endl;
        cout << "  (3) ScalarGaussianBNetModel" << endl <<endl;
        cout << "Mixed Discrete-Continuous BN:" << endl;
        cout << "  (4) WasteModel" << endl;
        cout << "  (5) CropModel" << endl << endl;
        cout << "Dynamic Bayesian Network (DBN):" << endl;
        cout << "  (6) KjaerulfsBNetModel" << endl;
        cout << "  (7) DBNModel" << endl << endl;
        cout << "Influence Diagram (LIMID):" << endl;
        cout << "  (8) PigsModel" << endl << endl;
        cout << "Example of Customizing Node Shape:" << endl;
        cout << "  (9) WasteModel" << endl;
        cout << endl << "Examples of graphs with link strengths and";
        cout << " connection strengths:" << endl;
        cout << "  Included in SampleUseLCS!" << endl << endl;

	return 1;
    }

    // get model number from argument
    int n = (argv[1])[0] - int('0');  

    cout << "---------------------------------- " << endl;
    cout << "Models are from models.cpp of PNL/high/Demo of PNL ";
    cout << "Release 1.0." << endl;
    cout << "---------------------------------- " << endl << endl;

    BayesNet * Bnet;
    DBN      * DBnet;
    LIMID    * LIMIDnet;

    // Load pre-defined model, then write Graphviz representation to file.

 switch (n) { 
    // ********** Discrete BNs **********
    case 1:
       cout << "Example of discrete BN: AsiaModel." << endl;
       Bnet =  AsiaModelCorrected(); 
       PNLtoGraphviz( Bnet, "AsiaModel.dot" );
       break;

    case 2:
       cout << "Example of discrete BN: FraudModel." << endl;
       Bnet = FraudModel();
       cout << "FraudModel has no arcs. ";
       cout << "Good for testing for disconnected graph." << endl;
       PNLtoGraphviz( Bnet, "FraudModel.dot" );
       break;

    // ********** Continuous BNs **********
    case 3:
       cout << "Example of continuous BN: ";
       cout << "ScalarGaussianBNetModel." << endl;
       Bnet = ScalarGaussianBNetModel();
       PNLtoGraphviz( Bnet, "ScalarGaussianBNetModel.dot" );
       break;
    
    // ********** Mixed discrete-contintuous BNs **********
    case 4:
       cout << "Example of mixed discrete-continuous BN: WasteModel." << endl;
       Bnet = WasteModel();
       PNLtoGraphviz( Bnet, "WasteModel.dot" );
       break;
   
    case 5:
       cout << "Example of mixed discrete-continuous BN: ";
       cout << "CropModel." << endl;
       Bnet = CropModel(); 
       PNLtoGraphviz( Bnet, "CropModel.dot" );
       break;
    
    // ********** Dynamic Bayesian Networks (all discrete) **********
    case 6:
       cout << "Example of large DBN: KjaerulfsBNetModel." << endl;
       DBnet = KjaerulfsBNetModel();  // large net
       cout << "Print DBN as DBN (standard)" << endl;
       PNLtoGraphviz( DBnet, "KjaerulfsBNetModel.dot" );

       // now misuse interface: writes DBN as BayesNet
       // This type of use is NOT recommended !!!
       // cout << "Print DBN as BayesNet !" << endl;
       // PNLtoGraphviz( DBnet->Net(), "KjaerulfsBNetModel2.dot", "BayesNet" );
       break;
    
    case 7:       
       cout << "Example of small DBN: DBNModel." << endl;
       DBnet = DBNModel();  // small net 
       PNLtoGraphviz( DBnet, "DBNModel.dot" );
       break;

    // ********** Influence Diagrams **********
    case 8:
       cout << "Example of Influence Diagram: PigsModel." << endl;
       LIMIDnet = PigsModel();
       PNLtoGraphviz( LIMIDnet, "PigsModel.dot" );
       break;

    // ********** Example of how to customize node shapes ********** 
    case 9:
       cout << "Example of how to customize node shapes.  Uses WasteModel.";
       cout << endl;
    
       // Node types are: discrete, continuous, chance, decision, value.
       // Only need to describe shapes for types to be CHANGED from default:
       //
       // Shape specification according to node attributes in dot-language.  
       // (See GraphViz manual "Drawing graphs with dot".)
       {
          map<string,string> my_node_shape;
          my_node_shape["discrete"]="[shape=doublecircle]";
          my_node_shape["continuous"]=
	     "[shape=doubleoctagon,style=filled,fillcolor=gray80]";

          // let's see what a mixed discrete-continuous graph looks like:
          Bnet = WasteModel();

          // graph with default settings
          cout << "Mixed discrete-continuous graph with default settings.";
	  cout << endl;
          PNLtoGraphviz( Bnet, "WasteModel.dot" );
       
          // graph with customized settings
          cout << "Mixed discrete-continuous graph with customized settings.";
          cout << endl;
          PNLtoGraphviz( Bnet, "WasteModel_silly.dot", my_node_shape );       
       }
       break;

   default:
       cout << "Model number not recognized!" << endl << endl;
       break;


 } // end of switch
} // end of main


