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

#ifndef GRAPHVIZINTERFACE_HPP
#define GRAPHVIZINTERFACE_HPP

#include "pnlHigh.hpp"
#include <string>
#include <map>



// --------------------------------------------------------------------------
// 1) Functions for vizualizing the graph structure of BNs, DBNs and LIMIDs:
// --------------------------------------------------------------------------

// For BN of any type (nodes can be discrete, continuous or mixed)
int PNLtoGraphviz ( pnlw::BayesNet * net, const std::string & filename, 
	       std::map<std::string,std::string> 
	       customized_node_shape=std::map<std::string, std::string>() );

// For DBNs of any type (nodes can be discrete, continuous or mixed)
int PNLtoGraphviz ( pnlw::DBN * net, const std::string & filename, 
	       std::map<std::string,std::string> 
	       customized_node_shape=std::map<std::string, std::string>() );

// For LIMID
int PNLtoGraphviz ( pnlw::LIMID * net, const std::string & filename, 
	       std::map<std::string,std::string> 
	       customized_node_shape=std::map<std::string, std::string>() );

// For MRF: this only writes an error message stating that
//          MRFs are not yet implemented!
int PNLtoGraphviz ( pnlw::MRF * net, const std::string & filename,
	       std::map<std::string,std::string> 
	       customized_node_shape=std::map<std::string, std::string>() );

// --------------------------------------------------------------------------
// 2) Functions only for discrete Bayesian Networks:
//    to vizualize also entropy, connection strengths or link strengths.
//    See manual for information on formulas used, etc.
// --------------------------------------------------------------------------

// Print graph with entropy for each node included
int PNLtoGraphviz_with_Entropy ( pnlw::BayesNet * net, 
				 const std::string & filename);

// Print graph with mutual information (relative to target_node) included
int PNLtoGraphviz_with_MI ( pnlw::BayesNet * net, const std::string & filename,
	       int target_node_index, bool want_percentage);

// Same as above, but using Node Name instead of Node Index for target_node
int PNLtoGraphviz_with_MI ( pnlw::BayesNet * net, const std::string & filename,
	       const String target_node_name, bool want_percentage);

// Print graph with link strengths (different formulas to choose from)
int PNLtoGraphviz_with_LS ( pnlw::BayesNet * net, const std::string & filename,
	       const std::string & formula, bool want_percentage,
               std::map<std::string,std::string>
               customized_node_shape=std::map<std::string, std::string>() );


// --------------------------------------------------------------------------
// 3) The following functions are NOT intended for external use. 
// --------------------------------------------------------------------------

// Dummy function that allows all the functions above to call one single 
// main routine. 
int PNLtoGraphviz ( pnlw::ProbabilisticNet & PNet, 
	       const std::string & filename, const String & TypeNet,
	       std::map<std::string,std::string> 
	       customized_node_shape=std::map<std::string, std::string>() );

// Main function called by all functions above. 
int PNLtoGraphviz ( pnlw::ProbabilisticNet & PNet, 
	       const std::string & filename, const String & TypeNet,
	       pnlw::BayesNet * net, const std::string & formula,
	       bool want_percentage, int target_node_index,
	       std::map<std::string,std::string> 
	       customized_node_shape=std::map<std::string, std::string>() );
#endif

