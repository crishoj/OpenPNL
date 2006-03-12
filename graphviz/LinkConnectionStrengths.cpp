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
#include "pnlWGraph.hpp"
#include "PNLtoGraphviz.hpp"
#include "TokenCover.hpp"
#include "LinkConnectionStrengths.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#define EPSILON 0.0000000001
using namespace std;

PNLW_USING


// --------------------------------------------------------------------------
   // --- Define auxiliary functions used below ---
   // --- These are implementated at the END of this file, but used before. ---

double log2( double x );
int max_node_states (BayesNet * BNet_p);
bool IsDiscreteNode( int index, ProbabilisticNet & PNet);
bool IsParentOf( int parent_index, int child_index, WGraph & myGraph );
bool Check_and_assign_parent_child_indices( int index1, int index2, 
			  int & child_index, int & parent_index,
					    WGraph & myGraph);
int Get_number_discrete_states( int node_index, BayesNet & BNet );
int Get_discrete_states( int node_index, BayesNet & BNet, 
			 int & n_states, Vector<String> & state_names );
string make_string_Nodename_State( int node_index, const String & state, 
				   WGraph & myGraph);
string make_string_Nodename( int node_index, WGraph & myGraph);
string make_string_Nodenames( const Vector<int> & node_indices, 
			      WGraph & myGraph);
string make_string_Nodenames_States( const Vector<int> & node_indices, 
			const Vector<String> & node_states, WGraph & myGraph);
int find_pos( Vector<int> indexVec, int target_index );
int JPD_RawData_index( const Vector<int> & queryNodes, 
 		       const Vector<int> & query_n_states, 
		       const Vector<int> & queryCurrentStates);
float JPD_value( const TokArr & JPD, 
	 const Vector<int> & queryNodes, const Vector<int> & query_n_states, 
	 const Vector<int> & queryCurrentStates, 
	 const Vector<String> & queryCurrentStatesStr, WGraph & myGraph);
float JPD_X_Y_value(const TokArr & JPD_X_Y, int X_index, int Y_index, 
		    int X_n_states, int Y_n_states, int X_state, int Y_state,
		    const String X_state_str, const String Y_state_str,
		    WGraph & myGraph);
float CPT_value( int child_index, String child_state_str, 
		 Vector<int> parent_indices, Vector<String> parent_states_str, 
		 WGraph & myGraph, BayesNet & BNet);

void TrueAverageCalculation( int Y_index, int X_index, int X_pos,
     const Vector<int> & X_Z_indices, const Vector<int> & X_Z_n_states, 
     Vector<int> & X_Z_states_num, 
     Vector<String> & X_Z_states_str, 
     BayesNet & BNet, double & U_Y_given_Z, double & U_Y_given_X_Z, 
     const TokArr & JPD_X_Z);

void BlindAverageCalculation( int Y_index, int X_index, int X_pos,
     const Vector<int> & X_Z_indices, const Vector<int> & X_Z_n_states, 
     Vector<int> & X_Z_states_num, 
     Vector<String> & X_Z_states_str, 
     BayesNet & BNet, double & U_Y_given_Z, double & U_Y_given_X_Z );

void LinkStrengthRecursion( int current_depth, 
			    int Y_index, int X_index, int X_pos,
			    const Vector<int> & X_Z_indices, 
			    const Vector<int> & X_Z_n_states,
			    Vector<int> & X_Z_states_num,
			    Vector<String> & X_Z_states_str, 
			    BayesNet & BNet, double & U_Y_given_Z, 
			    double & U_Y_given_X_Z, 
			    const string formula, 
			    const TokArr & JPD_X_Z=TokArr("") );

// --------------------------------------------------------------------------
   // This function calculates entropy of node X_index.
   //
   // Input variables: 
   //    X_index            = index of node
   //    BNet               = Discrete Bayesian Network to which X_index belongs.
   //
   // Returns: entropy value of X_index
   //
double Entropy( int X_index, BayesNet & BNet )
{
      ProbabilisticNet & PNet=BNet.Net();  // Get PNet
      WGraph & myGraph = PNet.Graph();  // get graph of PNet

      // get P(X) for all states of X:
      BNet.ClearEvid();  // Clear evidence before using inference
      BNet.SetProperty("Inference", "jtree");
      string string_X_node = make_string_Nodename(X_index,myGraph);
      TokArr JPD_X = BNet.GetJPD( string_X_node.c_str() );

      String result = String(JPD_X);      
      //     cout << "========= JPD_X Request ===========" << endl; 
      // cout << string_X_node.c_str() << endl;

      //      cout << "========= JPD_X Result ===========" << endl;
      //      cout << result.c_str() << endl << endl;

      double entropy = 0;
      int X_n_states = Get_number_discrete_states( X_index, BNet );
      for (int ix=0; ix< X_n_states; ix++)
      {
	    float P = JPD_X[ix].FltValue();
//	    cout << "P= " << P << endl;
	    if (P > EPSILON)
		    entropy += P * log2( 1 / P );
      }
//      cout << "Entropy of node " << X_index << " is " << entropy << endl;
      return(entropy); 
}
// --------------------------------------------------------------------------
   // This function calculates Mutual Information for nodes X_index, Y_index.
   // 
   // Two values are calculated:
   // MI = U(Y) - U(Y|X) 
   //    = mutual information 
   //    = absolute amount of uncertainty in Y removed by knowing X
   //
   // MI_perc = ( MI / U(Y) ) * 100 
   //         = PERCENTAGE of uncertainty in Y removed by knowing X
   //     
   // Note: MI is symmetric in X,Y, but MI_perc is not!  
   // Thus BE CAREFUL about using correct order of X_index, Y_index 
   // when calling this function!
   // 
   // Input variables:
   //   X_index    = node whose relevance on Y is tested
   //   Y_index    = node for which we want to know uncertainty reduction
   //      (See also definition of MI_perc above to distinguish between X & Y.)
   //
   //   BNet       = Discrete Bayesian Network to which X, Y belong
   //   MI         = see formula above (for more details see manual)
   //   MI_perc    = see formula above (for more details see manual)
   //
void MutualInformation_with_perc ( int X_index, int Y_index, BayesNet & BNet,
      double & MI, double & MI_perc )
{
      ProbabilisticNet & PNet=BNet.Net();  // Get PNet

      // check whether both nodes are discrete 
      if ( !IsDiscreteNode( X_index, PNet) || 
	   !IsDiscreteNode( Y_index, PNet) ) 
      { 
         // return impossible values if one node is not discrete
         MI=-1; MI_perc=-1; return; 
      } 

      WGraph & myGraph = PNet.Graph();  // get graph of PNet
      TokenCover * myToken = &(PNet.Token());  // Get access to token cover

      BNet.ClearEvid();  // Clear evidence before using inference
      BNet.SetProperty("Inference", "jtree");
      // want JPD for all states of X and Y: assemble string for GetJPD request
      string string_X_Y_nodes = make_string_Nodename(X_index,myGraph) + " "
	                        + make_string_Nodename(Y_index,myGraph);
      TokArr JPD_X_Y = BNet.GetJPD( string_X_Y_nodes.c_str() );
#if 0
      String result = String(JPD_X_Y);      
      cout << "========= JPD_X_Y Request: ===========" << endl; 
      cout << string_X_Y_nodes.c_str() << endl;

      cout << "========= JPD_X_Y Result ===========" << endl;
      cout << result.c_str() << endl << endl;
#endif

      // now JPD_X_Y contains JPD of X,Y for all of their states.
      // Need to get values for individual state combinations from that. 

      int X_n_states, Y_n_states;
      Vector<String> X_state_names, Y_state_names;
      Get_discrete_states( X_index, BNet, X_n_states, X_state_names );
      Get_discrete_states( Y_index, BNet, Y_n_states, Y_state_names );

      // pre-calculate P(X=x) for all states of X:
      vector<float> P_X(X_n_states);
      // P(X=x) = sum_y P(X=x,Y=y)
      for (int ix=0; ix< X_n_states; ix++)
      {
	 P_X[ix]=0;
         for (int iy=0; iy< Y_n_states; iy++)  
         {
	    P_X[ix] += JPD_X_Y_value(JPD_X_Y, X_index, Y_index, 
				     X_n_states, Y_n_states, ix, iy,
			  X_state_names[ix], Y_state_names[iy], myGraph);
         }
      }

      // pre-calculate P(Y=y) for all states of Y:
      vector<float> P_Y(Y_n_states);
      // P(Y=y) = sum_x P(X=x,Y=y)
      for (int iy=0; iy< Y_n_states; iy++)
      {
	 P_Y[iy]=0;
         for (int ix=0; ix< X_n_states; ix++)
         {
	    P_Y[iy] += JPD_X_Y_value(JPD_X_Y, X_index, Y_index, 
				     X_n_states, Y_n_states, ix, iy,
			  X_state_names[ix], Y_state_names[iy], myGraph); 
         }
      }

      // finally calculate Mutual Information:
      // double MI = 0;
      double U_Y = 0;
      double U_Y_given_X = 0;

      for (int ix=0; ix< X_n_states; ix++)
      {
         for (int iy=0; iy< Y_n_states; iy++) 
         {
		 // P = P(x,y)
	    float P = JPD_X_Y_value( JPD_X_Y, X_index, Y_index, 
				     X_n_states, Y_n_states, ix, iy,
			  X_state_names[ix], Y_state_names[iy], myGraph);
//	    cout << "P= " << P << endl;
	    if (P > EPSILON)
            {
		    // MI += P * log2( P / (P_X[ix] * P_Y[iy]) );
		    U_Y += P * log2( 1 / P_Y[iy] );
		    U_Y_given_X += P * log2( P_X[ix] / P );
            }
         }
      }

      // Assign MI
      MI = U_Y - U_Y_given_X;
         // Theoretically, MI>=0 always, 
         // but we can get MI<0 due to numerical error.
         // That can cause problems for choosing gray scale, etc., later on.
         // Current Solution (this isn't perfect, but does the job):   
         //    Check whether MI is JUST below zero (error of 10^(-4) or less).
         //    If so: set MI to zero. 
         //     
      if ( MI < 0 && MI > -0.0001 )  
      {   
             // Optional feature: alert user that value is being rounded up.
             // Uncomment following 3 lines if desired.
          // cout << "    -- Note: MI(" << myGraph.NodeName(X_index) << ",";
          // cout << myGraph.NodeName(Y_index) << ") rounded from ";
          // cout << fixed << setprecision(10) << MI << " up to 0.0." << endl;
	  
          MI=0.0;  // round up
      }

      // Before assigning MI_perc, test whether U_Y=0.  
      // If so, then MI_perc is undefined ! 
      if (U_Y < EPSILON)
      {
	 MI_perc = -1; // assign impossible value
	 cout << " === WARNING:  MI% UNDEFINED FOR PAIR   "; 
         cout << myGraph.NodeName(X_index).c_str() << " , ";
         cout << myGraph.NodeName(Y_index).c_str() << " ===" << endl;
      }
      else MI_perc = 100 * MI / U_Y;

//      cout << " ========== MI RESULT ============ " << MI << endl;
//      cout << " ========== MI RESULT ============ " << endl;  
//      cout << " U(Y)=" << U_Y << "  U(Y|X)=" << U_Y_given_X << endl;
//      cout << " MI=U(Y)-U(Y|X)=" << (U_Y - U_Y_given_X) << endl;
//      cout << " Should match MI: " << MI << endl;
//      cout << " MI Percentage = " << MI/U_Y << endl << endl;     

}
// --------------------------------------------------------------------------
   // This function provides a short cut for calling MutualInformation_with_perc
   // (see above).
   //
   // It returns: Mutual Information  IF want_percentage=false,
   //             Mutual Information percentage  IF want_percentage = true.
   //
double MutualInformation( int X_index, int Y_index, BayesNet & BNet, 
		            bool want_percentage)
{
   double TotalValue;
   double PercentageValue;
   MutualInformation_with_perc( X_index, Y_index, BNet, TotalValue, 
				PercentageValue);
   if (want_percentage==true) return(PercentageValue);
   else return(TotalValue);
}
// --------------------------------------------------------------------------
   // This function calculates Link Strength for a PAIR OF ADJACENT NODES 
   // (index1,index2).

   // Order of nodes doesn't matter: 
   // The function determines automatically which node is the parent (X) and 
   // which one is the child (Y).
   //
   // Notation:
   //   Y = child                Use Y for variable, y for states
   //   X = designated parent    Use X for variable, x for states
   //   Thus: looking at arc X -> Y

   //   Z = set of all other parents
   //   X_Z = set of all parents (X AND Z combined).  
   //
   //
   // Two values are calculated:
   // LinkStrength = U(Y|Z) - U(Y|X,Z) 
   //              = reduction in uncertainty in Y by knowing X 
   //                averaged over all parent states Z.
   // LinkStrengthPercentage = ( [ U(Y|Z) - U(Y|X,Z) ] / U(Y|Z) ) * 100
   //    = percentage of uncertainty reduction in Y by knowing X.
   //
   // Different formulas are available (see manual for more details):
   //    formula = TrueAverage or BlindAverage.
   //
   // Input variables:
   //    index1, index2 = indices of nodes
   //    BNet           = discrete BN to which nodes belong
   //    formula        = "TrueAverage" or "BlindAverage" 
   //
   // Output variables:
   //    LinkStrength           =     U(Y|Z) - U(Y|X,Z)
   //    LinkStrengthPercentage = ( [ U(Y|Z) - U(Y|X,Z) ] / U(Y|Z) ) * 100
   //    (for more details see manual)
   //
void LinkStrength_with_perc( int index1, int index2, BayesNet & BNet, 
		     const string & formula, double & LinkStrength, 
		     double & LinkStrengthPercentage )
{
   ProbabilisticNet & PNet=BNet.Net();  // Get PNet
   WGraph & myGraph = PNet.Graph();  // get graph of PNet

   // for each node: check whether discrete type   
   if ( !IsDiscreteNode( index1, PNet) || 
	!IsDiscreteNode( index2, PNet) ) 
   { 
      // return impossible values if one node is not discrete
      LinkStrength=-1; LinkStrengthPercentage=-1; return; 
   } 

   // for all other cases:
   // check which one of index1, index2 is parent and which one is the child:
   int child_index, parent_index;
   Check_and_assign_parent_child_indices( index1, index2, child_index, 
					  parent_index, myGraph);  


#if 0 
   // This is a feature that would allow one to print mutual information
   // for any node pair connected by a link. 
   // While MutualInformation measures connection strength, not link strength,
   // it may sometimes be helpful to compare the link strength of an arc 
   // to the connection strength of its two variables. 
   //
   // However, I decided to disable this feature, since it appears to be
   // more confusing than helpful, leading easily to misunderstandings. 
   //
   // It's fully working though and could be added back in later. 

   if ( formula == "MutualInformation")  // simplest case
   {
      MutualInformation_with_perc( parent_index, child_index, BNet, 
				   LinkStrength, LinkStrengthPercentage );
      return;
   }   
#endif

   if ( formula == "MutualInformation")  // shouldn't get here !
   {
      cout << endl << "This feature has been disabled.  Error!!!" << endl;
      return;
   }

#if 0
   cout << "==============================================" << endl;
   cout << "   Calculating Link Strength for Arc" << endl;
   cout << "      " << myGraph.NodeName(parent_index).c_str() << " --> ";
   cout << "      " << myGraph.NodeName(child_index).c_str() << endl;
   cout << "==============================================" << endl << endl;
#endif

     // For better readability, use following notation from now on:
     // Considered arc goes from X -> Y
     //   Y = child                Use Y for variable, y for states
     //   X = designated parent    Use X for variable, x for states
     //   Z = set of all OTHER parents of Y
     //   X_Z = set of all parents (X AND Z combined).  
   
   // assign proper values to X,Y,Z, etc.
   int X_index = parent_index;
   int Y_index = child_index;
   int X_Z_n_nodes = myGraph.nParent(child_index);
   Vector<int> X_Z_indices; // vector for indices of all parents
   myGraph.GetParents( &X_Z_indices, Y_index ); // get all parents of Z
   // now find position of X in vector X_Z_indices:
   int X_pos = find_pos( X_Z_indices, X_index );
   // cout << "X_pos (int): " << X_pos << endl;
   if (X_pos == -1 ) { cout << "Something wrong here!" << endl;}

   Vector<int> X_Z_n_states(X_Z_n_nodes); // # of discrete states for each node
   for (int i=0; i < X_Z_n_nodes ; i++)  
      X_Z_n_states[i] = Get_number_discrete_states( X_Z_indices[i], BNet );

   // initialize U(Y|Z) and U(Y|X,Z)
   double U_Y_given_Z = 0;
   double U_Y_given_X_Z = 0;

   // create vector to hold one state combination for X and Z
   Vector<int> X_Z_states_num(X_Z_n_nodes);    // vector for states' numbers
   Vector<String> X_Z_states_str(X_Z_n_nodes); // vector for states' strings

   if ( formula == "BlindAverage" )
   {   
      // Start Recursion for LinkStrength Calculation
      LinkStrengthRecursion( 0, Y_index, X_index, X_pos,
			X_Z_indices, X_Z_n_states, 
		        X_Z_states_num, X_Z_states_str,
		        BNet, U_Y_given_Z, U_Y_given_X_Z, formula);
   }
   else if ( formula == "TrueAverage" )
   {
      // Get JPD for parents: P(X,Z) 
      BNet.ClearEvid();  // Clear evidence before using inference
      BNet.SetProperty("Inference", "jtree");
      string string_X_Z_nodes = make_string_Nodenames( X_Z_indices, myGraph);
      TokArr JPD_X_Z = BNet.GetJPD( string_X_Z_nodes.c_str() );

      // cout << "JPD request: " << string_Z_X_nodes.c_str() << endl;
//      String JPD_Z_X_String = String(JPD_Z_X);
//      cout << " Here comes the (whole) JPD ------- " << endl;
//      cout << JPD_Z_X_String.c_str() << endl << endl;

      LinkStrengthRecursion( 0, Y_index, X_index, X_pos,
		       X_Z_indices, X_Z_n_states, 
		       X_Z_states_num, X_Z_states_str,
		       BNet, U_Y_given_Z, U_Y_given_X_Z, formula, 
		       JPD_X_Z);
   }

   // Assign LS
   LinkStrength = U_Y_given_Z - U_Y_given_X_Z;
      // Theoretically, LinkStrength>=0 always, but we can get LinkStrength<0 
      // due to numerical error.
      // That can cause problems for choosing gray scale, etc., later on.
      // Current Solution (this isn't perfect, but does the job):   
      //    Check whether LinkStrength is JUST below zero 
      //    (Error of 10^(-4) or less.)
      //    If so: set LinkStrength to zero. 
      //     
   if ( LinkStrength < 0 && LinkStrength > -0.0001 )  
   {
          // Optional feature: alert user that value is being rounded up.
          // Uncomment following 3 lines if desired.
       // cout << "    -- Note: LS(" << myGraph.NodeName(parent_index) << "-->";
       // cout << myGraph.NodeName(child_index) << ") rounded from ";
       // cout << fixed << setprecision(10) << LinkStrength <<" up to 0.0."<<endl;
	     
       LinkStrength=0.0;  // round up
   }

   // Before assigning LS_perc, test whether U_Y_given_Z=0.
   // If so, LinkStrengthPercentage is undefined !
   if ( U_Y_given_Z < EPSILON )
   {
      LinkStrengthPercentage = -1;  // assign impossible value
      cout << " -- WARNING:  LS% UNDEFINED FOR EDGE "; 
      cout << myGraph.NodeName(parent_index).c_str() << "-->";
      cout << myGraph.NodeName(child_index).c_str() << " ===" << endl;
   }
   else LinkStrengthPercentage = 100*(LinkStrength / U_Y_given_Z);

#if 0
   cout << endl << "LinkStrength (" << formula << "): " << endl;
   cout << " U(Y|Z) =" << U_Y_given_Z << "  U(Y|X,Z) =" << U_Y_given_X_Z<<endl;
   cout << " LS: " << LinkStrength << "  LS% : " << LinkStrengthPercentage;
   cout << endl;
   cout << endl;
#endif 
}
// --------------------------------------------------------------------------
// This function returns only one value for Link Strength:
// either absolute or percentage, depending on value of want_percentage.
// 
double LinkStrength( int index1, int index2, BayesNet & BNet, 
		     const string & formula, bool want_percentage)
{
   double TotalValue =0;
   double PercentageValue =0;
   LinkStrength_with_perc( index1, index2, BNet, formula, TotalValue, 
			 PercentageValue);
   if (want_percentage==true) return(PercentageValue);
   else return(TotalValue);
}
// --------------------------------------------------------------------------
   // This recursion is used only by LinkStrength_with_perc and only 
   // for formulas TrueAverage and BlindAverage.
   //
   // This recursion generates ALL POSSIBLE STATE COMBINATIONS of node set Z.
   // At the end of the recursion the contribution 
   // of that Z-combination to LinkStrength is calculated (and added)
   // by calling TrueAverageCalculation or BlindAverageCalculation.
   //
   //
   // Notation:
   //   Y = child                Use Y for variable, y for states
   //   X = designated parent    Use X for variable, x for states
   //   Thus: looking at arc X -> Y

   //   Z = set of all other parents
   //   X_Z = set of all parents (X AND Z combined).  
   //
   // Input variables:
   //    current_depth = recursion depth (from 0 to # parents of Y)
   //                  = which parent from parent list is currently considered
   //    Y_index       = node index of child
   //    X_index       = node index of designated parent
   //    X_pos         = at which position of vector X_Z_indices is X_index?
   //    X_Z_indices   = lists indices of all parents of Y (including X)
   //    X_Z_n_states  = lists # of states for each parent
   //    X_Z_states_num= current state combination of parents (partially filled)
   //    X_Z_states_str= contains corresponding names to X_Z_states_num
   //                    (also partially filled)
   //    BNet          = discrete Bayesian Network to which nodes belong
   //    formula       = TrueAverage of BlindAverage (NOT MutualInformation)
   //    JPD_X_Z       = optional parameter, ONLY used for TrueAverage.
   //                  = Joint probability distribution for parents of Y
   //    U_Y_given_Z   = U(Y|Z)  (partial contribution, continuously updated)
   //    U_Y_given_X_Z = U(Y|X,Z) (partial contribution, continuously updated)
   //    
   // Output variables:
   //    X_Z_states_num = current state combination of parents (Z only) 
   //                is being filled one by one at increasing depth of recursion
   //                State of X is only filled at max. recursion depth.
   //    X_Z_states_str = NAMES of parent states are also updated 
   //    U_Y_given_Z   = U(Y|Z) = updated only at max depth of recursion
   //                             (adds contribution for each state combination)
   //    U_Y_given_X_Z = U(Y|X,Z) = updated only at max depth of recursion
   //                             (Adds contribution for each state combination)
   
void LinkStrengthRecursion( int current_depth, 
			    int Y_index, int X_index, int X_pos,
			    const Vector<int> & X_Z_indices, 
			    const Vector<int> & X_Z_n_states,
			    Vector<int> & X_Z_states_num,
			    Vector<String> & X_Z_states_str, 
			    BayesNet & BNet, double & U_Y_given_Z, 
			    double & U_Y_given_X_Z, 
			    const string formula, 
			    const TokArr & JPD_X_Z )
{
#if 0
	cout << "Entering LinkStrengthRecursion" << endl;
	cout << " Depth= " << current_depth << endl;
#endif

   int X_Z_n_nodes = X_Z_indices.size();  // # of parents of Y
   // if last parent index not yet reached 
   if ( current_depth < X_Z_n_nodes )  
   {
      // Consider next node index in parent list
      int this_Z_index = X_Z_indices[current_depth];

      // Want to generate all combinations of Z, but not X here.
      // --> If current index corresponds to X: skip it and go to next level
      if (this_Z_index == X_index)
      {
	  // cout << "Skip this index" << endl;
 	  // dive into recursion for next Z_node
	  LinkStrengthRecursion( current_depth+1,  
	     Y_index, X_index, X_pos, X_Z_indices, 
	     X_Z_n_states, X_Z_states_num, X_Z_states_str, 
 	     BNet, U_Y_given_Z, U_Y_given_X_Z, formula, JPD_X_Z ); 
      }
      else
      {
         // The current node belongs to Z: 
         //   --> go through all states of current node

         // First get list of discrete states of current Z_node
         int n_states;  
         Vector<String> state_names;
         Get_discrete_states(this_Z_index, BNet, n_states, state_names);

         // Go through all discrete states of Z_node
         for ( int state = 0; state < n_states; state++)
         {
	     // set current state for Z_node
	     X_Z_states_num[current_depth] = state;
	     X_Z_states_str[current_depth] = state_names[state];

	     // dive into recursion for next Z_node
	     LinkStrengthRecursion( current_depth+1,
	        Y_index, X_index, X_pos, X_Z_indices, 
	        X_Z_n_states, X_Z_states_num, X_Z_states_str, 
 	        BNet, U_Y_given_Z, U_Y_given_X_Z, formula, JPD_X_Z );
         }
      } // end of else (this_Z_index == X_index)
   } 
   else
   {
       // Maximal recursion depth reached, i.e. X_Z_states now
       // contains one complete state combination for Z nodes,
       // but not yet for X.

       // Proceed to calculations:
       //    The two function calls below calculate contribution of Z-combination
       //    to U(Y|Z) and U(Y|X,Z) and update those variables accordingly. 
       //    Afterward U(Y|Z) and U(Y|X,Z) contain the ACCUMULATED contribution,
       //    including all state combinations of Z considered thus far.
       if (formula == "BlindAverage")
       {
	   BlindAverageCalculation( Y_index, X_index, X_pos, X_Z_indices, 
				    X_Z_n_states, X_Z_states_num,X_Z_states_str,
				    BNet, U_Y_given_Z, U_Y_given_X_Z);         
       }
       else if (formula == "TrueAverage")
       {
           TrueAverageCalculation( Y_index, X_index, X_pos, X_Z_indices, 
				   X_Z_n_states, X_Z_states_num, X_Z_states_str,
				   BNet, U_Y_given_Z, U_Y_given_X_Z, JPD_X_Z );
       }
   }
}
// --------------------------------------------------------------------------
   // Calculate LinkStrength contribution using TrueAverage formula.
   // (See manual for description of the formula.)
   //
   // This function SHOULD ONLY BE CALLED from function LinkStrengthRecursion.
   //
   // This function calculates contribution to link strength for ONE SPECIFIC 
   // state combination of all Z nodes, but for ALL states of X and Y.
   // The link strength is calculated in terms of U(Y|Z) and U(Y|X,Z), 
   // which allows calculation of both link_strength and 
   // link_strength_percentage later on.
   //
   // By calling this function for EACH state combination of the Z nodes and 
   // adding the results, we obtain the total link strength. 
   // Note that the ADDITION is already taking place inside this function. 
   // Namely, this function automatically ADDS the calculated contribution
   // for U(Y|Z) and U(Y|X,Z) to its previous value, thus returning 
   // the ACCUMULATED VALUES for U(Y|Z) and U(Y|X,Z), including all 
   // Z-combinations previously considered.
   //
   //
   // Notation:
   //   Y = child                Use Y for variable, y for states
   //   X = designated parent    Use X for variable, x for states
   //   Thus: looking at arc X -> Y

   //   Z = set of all other parents
   //   X_Z = set of all parents (X AND Z combined).  
   //
   // Input variables:
   //    Y_index       = node index of child
   //    X_index       = node index of designated parent
   //    X_pos         = at which position of vector X_Z_indices is X_index?
   //    X_Z_indices   = lists indices of all parents of Y (including X)
   //    X_Z_n_states  = lists # of states for each parent
   //    X_Z_states_num= current state combination of parents 
   //                    (filled except for state of X)
   //    X_Z_states_str= contains corresponding names to X_Z_states_num
   //                    (filled except for state of X)
   //    BNet          = discrete Bayesian Network to which nodes belong
   //    JPD_X_Z       = Joint probability distribution for parents of Y
   //    U_Y_given_Z   = U(Y|Z)  (value before current Z state combi is included)
   //    U_Y_given_X_Z = U(Y|X,Z) (value before current Z state combi is incl.)
   //    
   // Output variables:
   //    U_Y_given_Z   = U(Y|Z)  (updated value, including contribution from 
   //                             current state combination of Z)
   //    U_Y_given_X_Z = U(Y|X,Z) = (updated value, including contribution from 
   //                             current state combination of Z)
   
void TrueAverageCalculation( int Y_index, int X_index, int X_pos,
     const Vector<int> & X_Z_indices, const Vector<int> & X_Z_n_states, 
     Vector<int> & X_Z_states_num, Vector<String> & X_Z_states_str, 
     BayesNet & BNet, double & U_Y_given_Z, double & U_Y_given_X_Z, 
     const TokArr & JPD_X_Z)
{          
     WGraph & myGraph = BNet.Net().Graph();  // get graph of BNet
     // Get states of X and Y
     int n_X_states, n_Y_states;  
     Vector<String> X_states, Y_states;
     Get_discrete_states( Y_index, BNet, n_Y_states, Y_states);
     Get_discrete_states( X_index, BNet, n_X_states, X_states);
 
     // -------------------------- STEP 1 ---------------------------------
     // For all states of X: 
     //       Get P(x,z) and store in vector P_x_z
     Vector<float> P_x_z(n_X_states); // store P(x,z) for CURRENT z and ALL x
     for (int ix=0; ix < n_X_states; ix++)
     {     
	X_Z_states_num[X_pos] = ix;
	X_Z_states_str[X_pos] = X_states[ix];
        // extract joint probability value for SPECIFIC state combi of X,Z
        // from JPD table for X,Z (table calculated previously by inference) 
        P_x_z[ix] = JPD_value( JPD_X_Z, X_Z_indices, X_Z_n_states, 
		   X_Z_states_num, X_Z_states_str, myGraph);
        // Now P_X_Z[ix] = P(x,z) = P(X=x, Z1=z1, Z2=z2, ...)
     }
     // -------------------------- STEP 2 --- ------------------------------
     // Calculate P(z) = sum_x P(x,z)
     float P_z=0;
     for (int ix=0; ix < n_X_states; ix++)
	     P_z += P_x_z[ix];

     // if P_z = 0, then this z-combination does not contribute anything
     // to LinkStrength --> skip this z-combination
     if (P_z <= EPSILON) return;  

     // -------------------------- STEP 3 ---------------------------------
     // For all states of Y:  pre-calculate P(y|z)
     //    P(y|z) = sum_x [ P(y|x,z) * P(x,z) / P(z) ]
     for (int iy=0; iy<n_Y_states; iy++)
     {
	float P_y_given_z = 0; 

        // Get P(y|x,z) for all x
        // AND calculate P(y|z) on the way
        Vector<float> P_y_given_x_z(n_X_states);

        for (int ix=0; ix < n_X_states; ix++)
        {
	   // set current state of X in X_Z_states_num & X_Z_states_str:
	   X_Z_states_num[X_pos] = ix;
	   X_Z_states_str[X_pos] = X_states[ix];
           // Get P(y|x,z) for current x,y,z
	   P_y_given_x_z[ix] = CPT_value( Y_index, Y_states[iy], 
			 X_Z_indices, X_Z_states_str, myGraph, BNet);

	   // Update P_y_given_z for current y,z
	   P_y_given_z += P_y_given_x_z[ix] * P_x_z[ix] / P_z;
        }   
        // Now P(y|x,z) is available for CURRENT z,y and ALL x
        // and P(y|  z) is available for CURRENT z,y. 

        // -------------------------- STEP 4 ---------------------------------
        // Finally calculate contribution to LinkStrength, i.e. to
        // U(Y|Z) and U(Y|X,Z).
        for (int ix=0; ix < n_X_states; ix++)
        {
		// if P(x,z)=0 or P(y|x,z)=0, skip this x-value.
	   if ( P_x_z[ix] > EPSILON && P_y_given_x_z[ix] > EPSILON )
           {
		   float P = P_y_given_x_z[ix];
		   U_Y_given_Z += P_x_z[ix] * P * log2( 1 / P_y_given_z );
		   U_Y_given_X_Z += P_x_z[ix] * P * log2( 1 / P );
	   }
        } // end of x-loop  
     } // end of y-loop
}
// --------------------------------------------------------------------------
   // Calculate LinkStrength contribution using BlindAverage formula.
   // (See manual for description of the formula.)
   //
   // This function SHOULD ONLY BE CALLED from function LinkStrengthRecursion.
   //
   // Description: same as for TrueAverageCalculation above, except that 
   // different formula is used.
   //
   //
   // Notation:
   //   Y = child                Use Y for variable, y for states
   //   X = designated parent    Use X for variable, x for states
   //   Thus: looking at arc X -> Y

   //   Z = set of all other parents
   //   X_Z = set of all parents (X AND Z combined).  
   //
   // Input variables:
   //    Y_index       = node index of child
   //    X_index       = node index of designated parent
   //    X_pos         = at which position of vector X_Z_indices is X_index?
   //    X_Z_indices   = lists indices of all parents of Y (including X)
   //    X_Z_n_states  = lists # of states for each parent
   //    X_Z_states_num= current state combination of parents 
   //                    (filled except for state of X)
   //    X_Z_states_str= contains corresponding names to X_Z_states_num
   //                    (filled except for state of X)
   //    BNet          = discrete Bayesian Network to which nodes belong
   //    U_Y_given_Z   = U(Y|Z)  (value before current Z state combi is included)
   //    U_Y_given_X_Z = U(Y|X,Z) (value before current Z state combi is incl.)
   //    
   // Output variables:
   //    U_Y_given_Z   = U(Y|Z)  (updated value, including contribution from 
   //                             current state combination of Z)
   //    U_Y_given_X_Z = U(Y|X,Z) = (updated value, including contribution from 
   //                             current state combination of Z)
   
void BlindAverageCalculation( int Y_index, int X_index, int X_pos,
     const Vector<int> & X_Z_indices, const Vector<int> & X_Z_n_states, 
     Vector<int> & X_Z_states_num, 
     Vector<String> & X_Z_states_str, 
     BayesNet & BNet, double & U_Y_given_Z, double & U_Y_given_X_Z )
{          
     WGraph & myGraph = BNet.Net().Graph();  // get graph of BNet

#if 0
     cout << "BlindAverageCalculation called with z-values:" << endl; 

     for (int i=0; i<X_Z_indices.size(); i++)
     {
	     int my_index = X_Z_indices[i];
             if (my_index != X_index) { 
		     cout << "  Node " << my_index;
	             cout << " state: " << X_Z_states_str[i].c_str();
			     cout << endl;
             }
     }
#endif

     // ---------- Start calculations -------------------
     // Get lists of discrete states for X and Y
     int n_X_states, n_Y_states;  
     Vector<String> X_states, Y_states;
     Get_discrete_states( Y_index, BNet, n_Y_states, Y_states);
     Get_discrete_states( X_index, BNet, n_X_states, X_states);

     // variable for LS contribution from one state combination of Z
     double U_Y_given_Z_contribution=0; 
     double U_Y_given_X_Z_contribution=0; 

     // For all states of Y:
     for (int iy=0; iy<n_Y_states; iy++)
     {	
        // STEP 1: -------------------------------
        //    For all states of X: 
        //       Get P(y|x,z) and store in vector P_y_given_x_z
        Vector<float> P_y_given_x_z(n_X_states);
        for (int ix=0; ix < n_X_states; ix++)
        {
	   // set current state of X in X_Z_states_num & X_Z_states_str:
	   X_Z_states_num[X_pos] = ix;  // X_Z_states_num not really needed
	   X_Z_states_str[X_pos] = X_states[ix];
           // Get P(y|x,z) for current x,y,z
	   P_y_given_x_z[ix] = 
	      CPT_value( Y_index, Y_states[iy], X_Z_indices, X_Z_states_str, 
			 myGraph, BNet);
        }
	     	     
        // STEP 2: -------------------------------
	//    Calculate P_y_given_z = 1/|X| * sum_x  P_y_given_x_z
	float P_y_given_z = 0;
	for (int ix=0; ix < n_X_states; ix++)
	{
	    P_y_given_z += P_y_given_x_z[ix];  // sum P_y_given_x_z over x
        }
	P_y_given_z = P_y_given_z / ( n_X_states );

        // STEP 3: -------------------------------
	//    For all states of X:
	//       If P(y|x,z) > epsilon:
	//          Increment LS by term P(y|x,z)*log2( P(y|x,z)/ P(y|z) )
	for (int ix=0; ix < n_X_states; ix++)
	{
	   if ( P_y_given_x_z[ix] > EPSILON )
           {
		   // LinkStrength_contribution += P_y_given_x_z[ix] * 
		   // log2( P_y_given_x_z[ix] / P_y_given_z );
	      U_Y_given_Z_contribution += P_y_given_x_z[ix] * 
		   log2( 1 / P_y_given_z );
	      U_Y_given_X_Z_contribution += P_y_given_x_z[ix] * 
		   log2( 1 / P_y_given_x_z[ix] );
           }
	} // end of ix loop
     } // end of iy loop

     // scale contributions & add them to output variables
     int n_X_Z_combined =1;  // calculate total # of parent state combinations 
     for (int i_xz=0; i_xz<X_Z_indices.size() ; i_xz++)  
        n_X_Z_combined *= X_Z_n_states[i_xz];
     U_Y_given_Z +=  U_Y_given_Z_contribution / n_X_Z_combined;
     U_Y_given_X_Z += U_Y_given_X_Z_contribution / n_X_Z_combined;
}


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//            ---  SIMPLE FUNCTIONS FOR OUTPUT ON SCREEN ---
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// (Functions for output to Graphviz file are included in PNLtoGraphviz.cpp.)

// --------------------------------------------------------------------------
   // Prints entropy for all nodes of the network on the screen.
   //
void Print_Entropy( BayesNet & BNet)
{
    ProbabilisticNet & PNet=BNet.Net();  // Get PNet
    WGraph & myGraph = PNet.Graph();  // get graph of PNet

    cout << endl << "Entropy for all nodes:" << endl;

    // Get maximal node index used in graph.  
    // Note that there may be unused (invalid) indices!
    int iNodeMax = myGraph.iNodeMax();
    for (int index=0; index<= iNodeMax; index++)
	if (myGraph.IsValidINode(index) )  // if index is valid
	{
  	      String nodeName = myGraph.NodeName(index);  // get node name
	      double entropy = Entropy(index,BNet);
              // Print: node# nodeName  Entropy=XXX
	      cout << " Node " << index << ": " << nodeName << "    "; 
 	      cout << fixed << setprecision(3) << entropy << endl;
	} // end of if
    cout << endl;
}
// --------------------------------------------------------------------------
   // Prints link strength for ALL arcs in the network.
   // Formula used depends on values of "formula" and "want_percentage".
   // (see function LinkStrength for their possible values)
   //
void Print_Link_Strengths( BayesNet & BNet,
        const string & formula, bool want_percentage)
{
    ProbabilisticNet & PNet=BNet.Net();  // Get PNet
    WGraph & myGraph = PNet.Graph();  // get graph of PNet

    // Write a header of what is being printed
    cout << endl << "Link Strengths for all arcs using  ";
    // if (formula=="MutualInformation") cout << "Mutual Information";
    if (formula=="MutualInformation") cout << "Error!";
    else if (formula=="TrueAverage")  cout << "True Average";
    else if (formula=="BlindAverage") cout << "Blind Average";
    if (want_percentage==true) cout << " Percentage";
    cout << endl;

    // Get maximal node index used in graph.  
    // Note that there may be unused (invalid) indices!
    int iNodeMax = myGraph.iNodeMax();

    for (int index=0; index<= iNodeMax; index++)
    {            
        // write arcs FROM its parents TO current node (incoming arcs to node)
        if (myGraph.IsValidINode(index) )  // if index is valid 
        {
           String name = myGraph.NodeName(index);  // get node name
           int n_parents = myGraph.nParent(index);  // get # of parents
           Vector<int> parents;  // get array of parents (as vector of indices)
           myGraph.GetParents(&parents,index);

           for (int i=0; i<n_parents; i++)   // for all parents
           {
               // calculate link strength for arc
               double LS = LinkStrength( index, parents[i], BNet, formula, 
					 want_percentage );
              // print expression    parentName -> nodeName 
               cout << " " << myGraph.NodeName(parents[i]) << " -> " << name;
	       cout << "      LS = ";
	      if (want_percentage) 
	         cout << fixed << setprecision(1) << LS << "\%";
	      else                 
		 cout << fixed << setprecision(3) << LS; 
	      cout << endl;
           } // end of for
        } // end of if 
    } // end of for 
    cout << endl;
}
// --------------------------------------------------------------------------
   // Prints mutual information of all nodes relative to target_node.
   // Prints absolute value or percentage (depending on want_percentage).
   //
void Print_Mutual_Information_For_Single_Node( BayesNet & BNet, 
	int target_node_index, bool want_percentage)
{
   ProbabilisticNet & PNet=BNet.Net();  // Get PNet
   WGraph & myGraph = PNet.Graph();  // get graph of PNet
   String target_node_name = myGraph.NodeName(target_node_index);

   cout << endl << "Mutual Information ";
   if (want_percentage==true) cout << "Percentage ";
   cout << "for Target Node: " << target_node_name << endl;

   // Get maximal node index used in graph.  
   // Note that there may be unused (invalid) indices!
   int iNodeMax = myGraph.iNodeMax();

   for (int index=0; index<= iNodeMax; index++)
    {
	if (myGraph.IsValidINode(index) )  // if index is valid
	{
	   String node_name = myGraph.NodeName(index);  // get node name
	   if (index != target_node_index)
           {
	      // Step 1: Calculate Mutual Information (index,target_node_index)
              double MI = MutualInformation( index, target_node_index, BNet,
					     want_percentage);
	      // Print  MI(node_name,target_node_name) = ...   or
              // Print MI%(node_name,target_node_name) = ...
	      cout << " MI";
	      if (want_percentage) cout <<"%";
	      cout << "(" << node_name << "," << target_node_name;
	      cout << ") =   ";
              if (want_percentage)
		      cout << fixed << setprecision(1) << MI << "\%";
	      else    cout << fixed << setprecision(3) << MI;
	      cout << endl;
           }  // end of else
	} // end of if
    } // end of for
   cout << endl;
}
// --------------------------------------------------------------------------
   // Same as function above, but allows to specify target_node by name 
   // (rather than index).
   //
void Print_Mutual_Information_For_Single_Node( BayesNet & BNet, 
        String & target_node_name, bool want_percentage)
{
   	// Get index for target_node_name.
    ProbabilisticNet & PNet=BNet.Net();  // Get PNet
    int target_node_index = PNet.Graph().INode( target_node_name );
    Print_Mutual_Information_For_Single_Node( BNet, target_node_index, 
					      want_percentage);
}
// --------------------------------------------------------------------------
   // Calls Print_Mutual_Information_For_Single_Node in turn for 
   // each node of the network, thus using each one as the target_node.
   //
void Print_Mutual_Information_For_All_Nodes( BayesNet & BNet, 
					     bool want_percentage)
{
   ProbabilisticNet & PNet=BNet.Net();  // Get PNet
   WGraph & myGraph = PNet.Graph();  // get graph of PNet
   int iNodeMax = myGraph.iNodeMax();
   for (int index=0; index<= iNodeMax; index++)
   {
      if (myGraph.IsValidINode(index) )  // if index is valid
         Print_Mutual_Information_For_Single_Node( BNet,index,want_percentage);
   } 
}
// --------------------------------------------------------------------------
   // Simple way to print graph structure:
   // For each node of graph: print node index, node name + list of parents
   // 
void Print_Graph_Structure( WGraph & myGraph )
{
   cout << endl;
   cout << "Printing Graph Structure" << endl;

   int iNodeMax = myGraph.iNodeMax();

   for (int index=0; index <= iNodeMax; index++)
   {
      	if (myGraph.IsValidINode(index) )  // if index is valid
	{
	   String name = myGraph.NodeName(index);  // get node name
           int n_parents = myGraph.nParent(index);  // get # of parents
           Vector<int> parents;  // get array of parents (as vector of indices)
           myGraph.GetParents(&parents,index);

           cout << " Node " << index << ": " << name << endl;
           cout << "   Parents:";
	   for (int i=0; i<n_parents; i++)   // for all parents
		   cout << "   " << myGraph.NodeName(parents[i]);
	   cout << endl;
	}
        cout << endl;
   }
}
// --------------------------------------------------------------------------
   // This function provides an easy way to get all the main information 
   // of a network by a simple function call.  
   // Prints everything on the screen, so GraphViz is not needed.
   //
   // Prints the following on the screen:
   // * graph structure, 
   // * entropy for all nodes, 
   // * BlindAverage and TrueAverage link strengths for all arcs 
   //   (both absolute values and percentages),
   // * mutual information of all nodes relative to target nodes - where each 
   //   node serves in turn as target node (abs. value and percentage).
   //
void Print_Summary_Report( BayesNet & BNet)
{
       Print_Graph_Structure( BNet.Net().Graph() );

       Print_Entropy( BNet );

       Print_Link_Strengths( BNet, "BlindAverage", false);       
       Print_Link_Strengths( BNet, "BlindAverage", true);
       Print_Link_Strengths( BNet, "TrueAverage", false);
       Print_Link_Strengths( BNet, "TrueAverage", true);

       Print_Mutual_Information_For_All_Nodes( BNet, false);
       Print_Mutual_Information_For_All_Nodes( BNet, true);
}
// --------------------------------------------------------------------------


// --- Here are the auxiliary functions ---

// --------------------------------------------------------------------------
   // Returns logarithm of x to base 2
   //
double log2( double x )
{
	return( log(x) /  log(2.0) );
}
// --------------------------------------------------------------------------
   // Determine upper bound on entropy value in network:
   // Find maximal number of nodes states throughout the network and 
   // return log2 of it. 
   //
double Entropy_bound( BayesNet * BNet_p )
{
	return( log2(max_node_states(BNet_p)) );
}

// --------------------------------------------------------------------------
   // For a discrete BN, determine the maximal number of node states 
   // throughout the network.
   //
   // Return result.
   //
int max_node_states (BayesNet * BNet_p)
{
    ProbabilisticNet & PNet=BNet_p->Net();  // Get PNet
    WGraph & myGraph = PNet.Graph();  // get graph of PNet
    int result = 1;
    TokenCover * myToken = &(PNet.Token());  // Get access to token cover

    // myGraph.iNodeMax() = maximal node number in graph
    // Note that there may be unused (invalid) indices!
    for (int index=0; index<= myGraph.iNodeMax() ; index++)  // for all nodes
    {
	if (myGraph.IsValidINode(index) )  // if index is valid
	   if ( IsDiscreteNode( index, PNet ) )  // if discrete node
	   {
              int n_states = myToken->nValue(index);  // get # states
              result = max( result, n_states );
           }
    }
    return(result);
}
// --------------------------------------------------------------------------
   // Get nodeType for considered node and check whether result is discrete.
   //
   // Return "true" if discrete, otherwise print error message and return "false"
   //
bool IsDiscreteNode( int index, ProbabilisticNet & PNet)
{
   WGraph & myGraph = PNet.Graph();
   TokArr nodeType = PNet.GetNodeType((TokArr)myGraph.NodeName(index) );
 
   String nodeTypeString = nodeType[0].Name();
   // cout << "Node type is " << nodeTypeString << "   ";
   if ( nodeTypeString == "discrete" )
        { return(true); }
   else
        {  cout << "Node is NOT DISCRETE!" << endl;  return(false); }
}
// --------------------------------------------------------------------------
   // Check whether parent_index denotes parent of node child_index
   // Return true if that's the case, false otherwise.
   //
bool IsParentOf( int parent_index, int child_index, WGraph & myGraph )
{
  Vector<int> parents; 
  myGraph.GetParents( &parents, child_index);  // get vector of ALL parents

  // search for parent_index in list of ALL parents of child
  Vector<int>::iterator pos;
  pos = find( parents.begin(), parents.end(), parent_index );

  if ( pos == parents.end() )  // if parent_index NOT found in list of parents
	  return(false);  
  else  
	  return(true);  
}
// --------------------------------------------------------------------------
   // Checks whether index1 and index2 are connected by an arc. 
   // If they are, then assign child_index and parent_index accordingly.
   // If neither one is parent of the other: print error message.
   // 
   // Input variables:
   //    index1, index2 = two node indices in arbitrary order
   //    myGraph        = graph of network to which nodes belong
   //
   // Output variables:
   //    child_index    = index of child
   //    parent_index   = index of parent
   //
   // Return value:  true if nodes are adjacent, false if they are not.
   //
bool Check_and_assign_parent_child_indices( int index1, int index2, 
			  int & child_index, int & parent_index,
			  WGraph & myGraph) 
{
   if ( IsParentOf( index2, index1, myGraph ) )
      { parent_index=index2; child_index = index1; return(true); }
   else 
      if ( IsParentOf( index1, index2, myGraph ) )
         { parent_index=index1; child_index = index2; return(true); }
      else 
         { 
	    cout << "Error in arguments of LinkStrength: " << endl;
	    cout << "Nodes " <<index1 << " and " << index2 <<" are not adjacent!";
	    cout << endl; 
	    cout << "LinkStrength is only defined for adjacent nodes!" << endl;
            return(false);
         }
}
// --------------------------------------------------------------------------
   // Returns number of discrete values of a discrete BN node.
   //
int Get_number_discrete_states( int node_index, BayesNet & BNet )
{
   ProbabilisticNet & PNet=BNet.Net();  // Get PNet
   TokenCover * myToken = &(PNet.Token());  // Get access to token cover
   return( myToken->nValue(node_index) );  // return # states
}
// --------------------------------------------------------------------------
   // Get all discrete values of a discrete BN node. 
   // Input variables:
   //    node_index = node of interest
   //    BNet       = discrete Bayesian Network to which node belongs
   //
   // Output variables:
   //    n_states = # of states of node
   //    state_names = vector of Strings denoting states of node
   // Returned value:  n_states
   //
int Get_discrete_states( int node_index, BayesNet & BNet, 
   int & n_states, Vector<String> & state_names )
{
   ProbabilisticNet & PNet=BNet.Net();  // Get PNet
   TokenCover * myToken = &(PNet.Token());  // Get access to token cover
   n_states = myToken->nValue(node_index);  // get # states
   // cout << "Child has " << n_child_states << " states: " << endl;
   myToken->GetValues( node_index, state_names ); // get names of all states
   return(n_states);
}
// --------------------------------------------------------------------------
   // Get nodename for node_index and make a string of the form: 
   //    "Nodename"
   //
string make_string_Nodename( int node_index, WGraph & myGraph)
{
	string result(myGraph.NodeName(node_index).c_str());
	return(result);
}
// --------------------------------------------------------------------------
   // Get nodeName for each element of vector node_indices.
   // Then make a string of all nodenames, separated by space: 
   //    "Nodename1 Nodename2 ..."
   //
string make_string_Nodenames( const Vector<int> & node_indices, 
      WGraph & myGraph)
{	
   string result(""); 
   for ( int i = 0 ; i < node_indices.size() ; i++ )
   {
      // unless beginning of string, add space first:
      if ( i > 0 )  result += " ";
      // add string for node name and its state 
      result += myGraph.NodeName(node_indices[i]).c_str();
   }
   return(result);
}
// --------------------------------------------------------------------------
   // Get nodeName for node_index and make a string of the form: 
   //    "NodeName^NodeState"  
   //
   // Input variables:
   //    node_index = node of interest
   //    state      = String denoting state of node
   // 
string make_string_Nodename_State( int node_index, const String & state, 
				   WGraph & myGraph)
{
	string result(myGraph.NodeName(node_index).c_str());
	result += "^";
	result +=  state.c_str();
	return(result);
}
// --------------------------------------------------------------------------
   // Get nodeName for each element of vector node_indices.
   // Then make a string of the form:
   //   "NodeName1^NodeState1 NodeName2^NodeState2 NodeName3^NodeState3 ..." 
   //
   // Input variables:
   //    node_indices = vector of node indices
   //    node_states  = vector of Strings denoting the current state for each of
   //               the nodes (order of node_indices and node_states must match)
   // 
string make_string_Nodenames_States( const Vector<int> & node_indices, 
       const Vector<String> & node_states, WGraph & myGraph)
{	
   string result(""); 
   for ( int i = 0 ; i < node_indices.size() ; i++ )
   {
      // unless beginning of string, add space first:
      if ( i > 0 )  result += " ";
      // add string for node name and its state 
      result += make_string_Nodename_State(node_indices[i], 
		 node_states[i], myGraph);
   }
   return(result);
}
// --------------------------------------------------------------------------
   // Determine position of value "target_index" in vector indexVec.
   //
   // Advantage of this function versus built-in member function for Vector
   // is that this function returns the position as an integer.
   //
int find_pos( Vector<int> indexVec, int target_index )
{
   int pos = -1;
   for (int i=0; i<indexVec.size(); i++)
      if (indexVec[i] == target_index) pos=i;
   return(pos);     
}
// --------------------------------------------------------------------------
   // When calling GetJPD, the output is an array that contains 
   // joint probability for ALL possible states of ALL request nodes.
   //
   // This routine determines the index for a specific state combination of
   // the request nodes 
   // -> used to find the JPD value for a specific state combination.
   //
   // Input variables:
   //    queryNodes         = Vector of query nodes in JPD request
   //    query_n_states     = Vector with # of states for each query node
   //    queryCurrentStates = specific state combination for query nodes 
   //                       = Vector with discrete state (as integer)
   //                         for each query node 
   //
   // Return value: index to be used with output of GetJPD to get JPD of 
   //               specific state combination (given by queryCurrentStates)
   // (See also JPD_value below.)
   //
int JPD_RawData_index( const Vector<int> & queryNodes, 
		   const Vector<int> & query_n_states, 
		   const Vector<int> & queryCurrentStates)
{
   double result;
   int my_index= queryCurrentStates[0]; // initialize with value of node 0
   // cout << "Step 0: " << my_index << endl;
   for (int node=1; node < queryNodes.size(); node++) //for all query nodes > 0
   {
       // cout << "Step " << node << ":   " << my_index << " * ";
       // cout << query_n_states[node] << " + " << queryCurrentStates[node];
       // cout << endl;
      my_index = my_index * query_n_states[node] + queryCurrentStates[node];
   }
   return my_index;
} 
// --------------------------------------------------------------------------
   // Given output from GetJPD (TokArr JPD), this function extracts and returns 
   // the joint probability for a SPECIFIC state combination of the request nodes
   // (See also JPD_RawData_index above.)
   //
   // Input variables:
   //    JPD                = TokArr returned by GetJPD request for queryNodes
   //    queryNodes         = Vector of query nodes used in GetJPD request
   //    query_n_states     = Vector with # of states for each query node
   //    queryCurrentStates = specific state combination for query nodes 
   //                       = Vector with discrete state (as integer)
   //                         for each query node 
   //    queryCurrentStatesStr = Strings corresponding to queryCurrentStates
   //                            (used only for debugging)
   //    myGraph            = graph of considered network
   //
   // Return value: JPD value of queryNodes for state combi "queryCurrentStates".
   // 
float JPD_value( const TokArr & JPD, 
	 const Vector<int> & queryNodes, const Vector<int> & query_n_states, 
	 const Vector<int> & queryCurrentStates, 
	 const Vector<String> & queryCurrentStatesStr, WGraph & myGraph)
{ 
#if 0  
   cout << "--- Testing INDEX for JPD: " << endl;
   for (int node=0; node < queryNodes.size(); node++) // for all query nodes
   {
	   cout << "  " << myGraph.NodeName(queryNodes[node]) << " " ;
	   cout << queryCurrentStatesStr[node] << "  ";
   }
   cout << endl;
#endif

   int my_index = JPD_RawData_index( queryNodes, query_n_states, 
				 queryCurrentStates);   
#if 0 
  // cout << "Index: " << my_index << endl;
   String result = String(JPD[my_index]);
   cout << " Actual string for index  ------- " << endl;
   cout << result.c_str() << endl;
   cout << "Final value: " << JPD[my_index].FltValue() << endl;
#endif
   return( JPD[my_index].FltValue() );
}
// --------------------------------------------------------------------------
   // This function does the same as JPD_value above for the special case 
   // when there are only two request nodes (X,Y), and where the nodes, etc.,
   // still need to be converted to vectors. 
   //
   // Input variables:
   //    JPD_X_Y                = TokArr returned by GetJPD request for X,Y
   //    X_index, Y_index       = indices for nodes X,Y
   //    X_n_states, Y_n_states = # of states of X and Y
   //    X_state, Y_state       = index of current state of X and Y
   //    X_state_str, Y_state_str = Strings corresponding to X_state, Y_state
   //                               (only used for debugging)
   //    myGraph                = graph of network to which X & Y belong
   //
   // Return value:  JPD value for X=X_state, Y=Y_state.
   //
float JPD_X_Y_value(const TokArr & JPD_X_Y, int X_index, int Y_index, 
		    int X_n_states, int Y_n_states, int X_state, int Y_state,
		    const String X_state_str, const String Y_state_str,
		    WGraph & myGraph)
{
      Vector <int> queryNodes;  
      queryNodes.push_back(X_index);      
      queryNodes.push_back(Y_index);

      Vector<int> query_n_states; 
      query_n_states.push_back( X_n_states );
      query_n_states.push_back( Y_n_states );

      Vector<int> queryCurrentStates;
      queryCurrentStates.push_back(X_state);
      queryCurrentStates.push_back(Y_state);

      Vector<String> queryCurrentStatesStr;
      queryCurrentStatesStr.push_back(X_state_str);
      queryCurrentStatesStr.push_back(Y_state_str);

      return( JPD_value( JPD_X_Y, queryNodes, query_n_states, 
		 queryCurrentStates, queryCurrentStatesStr, myGraph) );
}
// --------------------------------------------------------------------------
   // This function reads a specific Conditional Probability Value from the 
   // conditional probability table (CPT)
   // for specified child state and specified parent state combination.
   //
   // Input variables:
   //    child_index     = index of node of interest
   //    child_state_str = String version of child's current state
   //    parent_indices  = vector of parent nodes
   //    parent_states_str = vector of current parent states (String version)
   //    myGraph         = graph of network
   //    BNet            = discrete Bayesian network to which node belongs
   //
   // Return value: CPT value of child for given child_state & parent_states.
   //
float CPT_value( int child_index, String child_state_str, 
		 Vector<int> parent_indices, Vector<String> parent_states_str, 
		 WGraph & myGraph, BayesNet & BNet)
{
     // create string describing child state:
     string child_state_CPT_str = make_string_Nodename_State(child_index,
					child_state_str, myGraph);
     // create string describing parent states
     string parents_states_CPT_str = make_string_Nodenames_States( 
	     parent_indices, parent_states_str, myGraph );

     // Get the CPT value
     TokArr CPT_out = BNet.GetPTabular( child_state_CPT_str.c_str(), 
					parents_states_CPT_str.c_str() );

     float result = CPT_out[0].FltValue();
#if 0
     cout << " CPT Request Child:   " << child_state_CPT_str << endl;
     cout << " CPT Request Parents: " << parents_states_CPT_str << endl;
     cout << " CPT result: " << String(CPT_out) << endl;
     cout << " Value: " << result << endl << endl;
#endif

     return(result);
}
// --------------------------------------------------------------------------
