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

#ifndef LINK_CONNECTION_STRENGTH_HPP
#define LINK_CONNECTION_STRENGTH_HPP

#include "pnlHigh.hpp"
#include <string>
#include <map>

#define MY_DEBUG 


// CAUTION: All functions in this file APPLY ONLY FOR discrete Bayesian Networks!

double Entropy( int X_index, pnlw::BayesNet & BNet );
   // This function returns the entropy of node X_index.


void MutualInformation_with_perc( int X_index, int Y_index, 
		     pnlw::BayesNet & BNet, double & MI, double & MI_perc );
   // This function calculates the mutual information between X_index and Y_index
   // Two values are calculated:
   // MI = U(Y) - U(Y|X) = mutual information (absolute value) 
   // MI_perc = MI/U(Y) = percentage decrease of uncertainty in Y by knowing X

double MutualInformation( int index1, int index2, pnlw::BayesNet & BNet, 
			  bool want_percentage);
   // This function provides a short cut for calling MutualInformation_with_perc
   // It returns: Mutual Information  IF want_percentage=false,
   //             Mutual Information percentage  IF want_percentage = true.



void LinkStrength_with_perc( int index1, int index2, pnlw::BayesNet & BNet, 
		     const std::string & formula, 
	             double & TotalValue,
		     double & PercentageValue);
   // This function calculates Link Strength of two adjacent discrete nodes 
   // in a BN.  Two values are calculated:
   // TotalValue      = LinkStrength           =  U(Y|Z) - U(Y|X,Z)
   // PercentageValue = LinkStrengthPercentage = ([U(Y|Z) - U(Y|X,Z)] / U(Y|Z) ) * 100

double LinkStrength( int index1, int index2, pnlw::BayesNet & BNet,
                     const std::string & formula, bool want_percentage);
   // This function provides a short cut for calling LinkStrength_with_perc.
   //   Returns Link Strength (absolute value) if want_percentage=false
   //   Returns "Percentage" of Link Strength  if want_percentage=true.


double Entropy_bound( pnlw::BayesNet * BNet_p );
   // This function returns log2(max # states per node), where maximum is 
   // taken over all nodes of the network.  
   // Dividing link strengths or connection strength by this value always yields
   // a value between 0 and 1. (Used for determining gray scale when printing.) 


// ---------------- AUXILIARY PRINT ROUTINES ------------------------
// Print routines that calculate various quantities and print results 
// on the screen.  Fairly self-explanatory.  
// (Further documentation also available in manual and source file.)
//
void Print_Entropy( pnlw::BayesNet & BNet);
void Print_Link_Strengths( pnlw::BayesNet & BNet, 
	const std::string & formula, bool want_percentage);
void Print_Mutual_Information_For_Single_Node( pnlw::BayesNet & BNet, 
        String & target_node_name, bool want_percentage);
void Print_Mutual_Information_For_Single_Node( pnlw::BayesNet & BNet, 
        int target_node, bool want_percentage);
void Print_Mutual_Information_For_All_Nodes( pnlw::BayesNet & BNet, 
        bool want_percentage);
void Print_Graph_Structure( pnlw::WGraph & myGraph );
void Print_Summary_Report( pnlw::BayesNet & BNet );


#endif

