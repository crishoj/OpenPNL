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
#include "pnlWGraph.hpp"
#include "PNLtoGraphviz.hpp"
#include "LinkConnectionStrengths.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
using namespace std;

PNLW_USING


// --------------------------------------------------------------------------
// --- Define a few auxiliary functions ---
// --- These are implemented at the END of this file, but used before. ---

void write_single_node( const String & nodeName, ProbabilisticNet & PNet, 
			map<string,string> node_shape, 
		        ofstream & file);

void write_header( const String & TypeNet, const string & formula, 
		   bool want_percentage, int target_node_index, 
		   WGraph & myGraph, ofstream & file );

void write_all_nodes( int iNodeMax, WGraph & myGraph, 
		      ProbabilisticNet & PNet, const String & TypeNet, 
		      map<string,string> node_shape, ofstream & file);

void write_selected_nodes( int iNodeMax, WGraph & myGraph, 
			   ProbabilisticNet & PNet, const String & TypeNet, 
			   map<string,string> node_shape, 
			   ofstream & file, int cluster );

void write_all_nodes_with_MI( int iNodeMax, WGraph & myGraph, BayesNet & BNet, 
			   bool want_percentage,
			   int target_node_index, double scale_factor, 
			   ofstream & file);

void write_all_nodes_with_entropy(  int iNodeMax, WGraph & myGraph, 
				    BayesNet & BNet, ofstream & file);

void write_incoming_arcs_to_node(int index, WGraph & myGraph, ofstream & file);

void write_incoming_arcs_to_node_with_LS( int index, WGraph & myGraph, 
				 ofstream & file,
				 BayesNet * BNet, double scale_factor,
				 const string & formula, bool want_percentage);
// --------------------------------------------------------------------------
// Call PNLtoGraphviz for any BayesNet 
//    Nodes can be discrete, continuous or mixed.
int PNLtoGraphviz ( BayesNet * net, const string & filename, 
    map<string,string> customized_node_shape )
{
    ProbabilisticNet & PNet=net->Net();  // Get PNet
    PNLtoGraphviz( PNet, filename, "BayesNet", customized_node_shape );
}
// -------------------------------------------------------------------------
// Call PNLtoGraphviz for DBN
int PNLtoGraphviz ( DBN * net, const string & filename, 
    map<string,string> customized_node_shape )
{
    ProbabilisticNet & PNet=net->Net();  // Get PNet
    PNLtoGraphviz( PNet, filename, "DBN", customized_node_shape );
}
// -------------------------------------------------------------------------
// Call PNLtoGraphviz for LIMID
int PNLtoGraphviz ( LIMID * net, const string & filename, 
    map<string,string> customized_node_shape )
{
    ProbabilisticNet & PNet=net->Net();  // Get PNet
    PNLtoGraphviz( PNet, filename, "LIMID", customized_node_shape );
}
// -------------------------------------------------------------------------
// Call PNLtoGraphviz for MRF - only results in error message now
int PNLtoGraphviz ( MRF * net, const string & filename, 
    map<string,string> customized_node_shape )
{
    cerr << endl << "PNLtoGraphviz not yet implemented for MRF!" << endl << endl;  
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
//            Four Functions for discrete BNs only:
//  To include entropy, Mutual Information (=connection strengths)
//  or link strengths in the graph.
// -------------------------------------------------------------------------
// Call PNLtoGraphviz for discrete BN + add entropy for each node
int PNLtoGraphviz_with_Entropy ( BayesNet * net, const string & filename)
{
    ProbabilisticNet & PNet=net->Net();  // Get PNet
    PNLtoGraphviz( PNet, filename, "BN_WITH_ENTROPY", net, "", false, -1);
}
// -------------------------------------------------------------------------
// Call PNLtoGraphviz for discrete BN + add Mutual Information (= connection
// strength) between target node and all other nodes 
// (See manual for details)
int PNLtoGraphviz_with_MI ( BayesNet * net, const string & filename,
	       int target_node_index, bool want_percentage )
{
    ProbabilisticNet & PNet=net->Net();  // Get PNet
    WGraph & myGraph = PNet.Graph();

	// Check whether target_node_index is a valid index.
        // If not: print error message and return -1.
    if (!myGraph.IsValidINode(target_node_index))  
    {
       cout << "Problem in PNLtoGraphviz_with_MI: Node ";
       cout << target_node_index << " is not a valid node index!!!" << endl;
       return(-1);
    }

    PNLtoGraphviz( PNet, filename, "BN_WITH_MI", net, "MutualInformation", 
       want_percentage, target_node_index, map<string, string>() );
}
// -------------------------------------------------------------------------
// This function allows to call PNLtoGraphviz_with_MI using a node NAME
// rather than a node INDEX to denote the target_node. 
//
int PNLtoGraphviz_with_MI ( BayesNet * net, const string & filename,
	       const String & target_node_name, bool want_percentage)
{
	// Get index for target_node_name.
    ProbabilisticNet & PNet=net->Net();  // Get PNet
    int target_node_index = PNet.Graph().INode( target_node_name );
    PNLtoGraphviz_with_MI(net, filename, target_node_index, want_percentage);
}
// -------------------------------------------------------------------------
// Call PNLtoGraphviz for discrete BN + add Link Strengths of all arcs
// (See manual for details)
int PNLtoGraphviz_with_LS ( BayesNet * net, const string & filename,
	       const string & formula, bool want_percentage,
               map<string,string> customized_node_shape )
{
    ProbabilisticNet & PNet=net->Net();  // Get PNet
    PNLtoGraphviz(PNet, filename, "BN_WITH_LS", net,formula, want_percentage,
       -1, customized_node_shape );
}
// -------------------------------------------------------------------------







// ------------------ HERE ARE THE MAIN FUNCTIONS ----------------------- //
// -------------------------------------------------------------------------
// This is a dummy function that allows to call the main PNLtoGraphviz
// function without the following arguments: 
//     BayesNet * net, const string & formula, bool want_percentage, 
//     int target_node_index.
// (Those four variables are only required when seeking Link Strengths or 
// Connection Strengths.)  
//
// This function creates dummy values for these missing arguments. 
//
int PNLtoGraphviz ( ProbabilisticNet & PNet, const string & filename, 
    const String & TypeNet, map<string,string> customized_node_shape )
{
    PNLtoGraphviz ( PNet, filename, TypeNet, (BayesNet *) NULL, "", true, -1,
		    customized_node_shape );
}

// -------------------------------------------------------------------------
//  This is the MAIN ROUTINE called for ALL types of nets.
//
//  This function is NOT intended for external use: only to be called by 
//  the PNLtoGraphviz functions for specific network types above.
//
int PNLtoGraphviz ( ProbabilisticNet & PNet, const string & filename, 
    const String & TypeNet, BayesNet * net, const string & formula,
    bool want_percentage, int target_node_index,
    map<string,string> customized_node_shape )
{
    // Routine for visualization of graph structure.
    // Saves graph to file in format used by GraphViz package (dot language).

    // Resulting file can be used by GraphViz package for example as follows:
    //    dot -Tps file.dot > test.ps   (creates ps-file of graph)
    //    dot -Tpng file.dot > test.png (creates png-file of graph)
    // See GraphViz package for many other output options + graph editor.

    // Input variables: 
    //   PNet            = network to be considered
    //   filename        = output filename (recommend to use "file.dot")
    //   TypeNet         = BayesNet, DBN, LIMID,  (MRF not yet implemented)
    //                     BN_WITH_LS BN_WITH_MI or BN_WITH_ENTROPY
    //   formula         = which formula should be used for Link Strengths
    //                     (used only for BN_WITH_LS)
    //   want_percentage = should ouput for link strengths and connection 
    //                     strengths be in absolute values or percentage?
    //                     (used only for BN_WITH_LS and BN_WITH_MI)
    //   target_node_index = relative to which node should MI be calculated?
    //                     (used only for BN_WITH_MI)
    //   customized_node_shape = map describing node_shape to be used for 
    //                           each nodeType 
    //                     (used for all, except BN_WITH_MI)
    //   (default value is empty map --> then default shapes below are used)

    WGraph & myGraph = PNet.Graph();  // get graph of PNet

    // STEP 1: Define default node shapes for all 5 node types:
    // Node types are: discrete, continuous, chance, decision, value.
    // Shape specification according to node attributes in dot-language 
    // (See GraphViz manual "Drawing graphs with dot".) 
    map<string,string> node_shape;
    node_shape["discrete"]   = "[shape=ellipse]";
    node_shape["continuous"] = "[shape=ellipse,style=filled,fillcolor=gray90]";
    node_shape["chance"]     = "[shape=ellipse]";
    node_shape["decision"]   = "[shape=box]";
    node_shape["value"]      = "[shape=diamond]";
    //
    // Now overwrite defaults by user supplied shapes:
    // go through all pairs of customized_node_shape (if any) and copy 
    // its records to node_shape
    // --> User only needs to describe those types to be changed from default.
    for ( map<string,string>::iterator pos = customized_node_shape.begin(); 
	  pos != customized_node_shape.end(); pos++)
	      node_shape[pos->first] = pos->second;

    // STEP 2: Calculate global scale factor in case we want to 
    //          draw Link Strengths or Connection Strengths.

    double scale_factor = 1.0;  // initialize with default value
    if ( TypeNet == "BN_WITH_LS" || TypeNet == "BN_WITH_MI" ) 
       if (want_percentage==false)  
       {
	   // If percentage is used for LS or MI, then we don't need scaling.
           // Otherwise, scale LS by log2(max # of states per node), where max 
           // is taken over ALL nodes of network.
	   scale_factor = Entropy_bound(net);
	   // cout << "Scale_factor=" << scale_factor << endl;
       }

    // STEP 3: Open output file
    ofstream file(filename.c_str());  // open output file
    if ( !file)  
    {   
        cerr << "Can't open output file " << filename << endl;
        return(0);
    }    
    //
    // tell user what's going on and where to find file:
    cout << "Writing graph of type  \"" << TypeNet << "\"  to file  \"";
    cout << filename << "\" ." << endl << endl;

    // STEP 4: Write header to file
    // write header for directed graph with descriptive label
    write_header( TypeNet, formula, want_percentage, target_node_index, 
		  myGraph, file );

    // STEP 5: Write all nodes with node shapes

    // Get maximal node index used in graph.  
    // Note that there may be unused (invalid) indices!
    int iNodeMax = myGraph.iNodeMax();

    if ( TypeNet != "DBN" )  // for all but DBN: write ALL nodes 
    {
	   if ( TypeNet == "BN_WITH_MI" )
              write_all_nodes_with_MI( iNodeMax, myGraph, *net, 
		 want_percentage, target_node_index, scale_factor, file);
	   else if ( TypeNet == "BN_WITH_ENTROPY" )
              write_all_nodes_with_entropy( iNodeMax, myGraph, *net, file);
	   else 
              write_all_nodes( iNodeMax, myGraph, PNet, TypeNet, 
			       node_shape, file);
           file << endl;  // done writing the nodes
    }
    else   // for DBN:  group nodes of slices 0 & 1 into separate clusters
    {
	for (int cluster=0; cluster<=1; cluster++)
        {
	    file << "subgraph cluster_" << cluster << " {" << endl;
	    // for cluster #0: write all nodes ending with '0'
            // for cluster #1: write all nodes ending with '1'
            write_selected_nodes( iNodeMax, myGraph, PNet, TypeNet, node_shape,
               file, cluster );
	    file << " label = \"Slice " << cluster << "\";" << endl;
	    file << "}" << endl << endl;  // done writing all nodes of slice i
        }
    }

    // STEP 6: Write arcs (and calculate LS on the way if desired)
    // Go through all nodes again and write for each node all incoming arcs 
    for (int index=0; index<= iNodeMax; index++)
    {            
        // write arcs FROM its parents TO current node (incoming arcs to node)
        if (myGraph.IsValidINode(index) )  // if index is valid 
		if ( TypeNet != "BN_WITH_LS" )
		   write_incoming_arcs_to_node( index, myGraph, file);  
	        else 
		    write_incoming_arcs_to_node_with_LS( index, myGraph, file,
			 net, scale_factor, formula, want_percentage );
    }

    // Step 7:  Finish up file.
    file << endl << "} " << endl;  // last line of file: needs closing bracket
    return(1);     // file handle is closed automatically
}


// ------------------------------------------------------------------------
// --------------- DEFINITIONS OF AUXILIARY FUNCTIONS ---------------------
// ------------------------------------------------------------------------
void write_header( const String & TypeNet, const string & formula, 
		   bool want_percentage, int target_node_index, 
		   WGraph & myGraph, ofstream & file )
{
    // write header for directed graph
    file << "digraph G {" << endl;  

    // If printing with link strengths or connection strengths, 
    // add label for graph indicating which formula is used, etc.
    if ( TypeNet == "BN_WITH_LS" )  
    {
	    file << " label = \"Link Strengths using ";
            if (formula=="MutualInformation") file << "Mutual Information";
	    else if (formula=="TrueAverage")  file << "True Average";
	    else if (formula=="BlindAverage") file << "Blind Average";
            if (want_percentage==true) file << " Percentage";
	    file << "\";" << endl;
    }
    else if ( TypeNet == "BN_WITH_MI" )
    {
	    file << " label = \"Mutual Information";
            if (want_percentage==true) file << " Percentage";
	    file << " for all nodes relative to node ";
	    file << myGraph.NodeName(target_node_index).c_str();
	    file << "\";" << endl;
    }
//    else if (TypeNet == "BN_WITH_ENTROPY")
//	    file << " label = \"Network with entropy for all nodes\";";
       
    file << endl;
}
// -------------------------------------------------------------------------
void write_single_node( const String & nodeName, ProbabilisticNet & PNet, 
   map<string,string> node_shape, ofstream & file)
{
   // determine nodeType: discrete, continuous, chance, decision or value
   TokArr nodeType = PNet.GetNodeType((TokArr)nodeName);  
   String nodeTypeString = nodeType[0].Name();

   // write in the format:  "nodeName" [shape_specs]
   file << " \"" << nodeName << "\" ";
   // write shape properties depending on nodeType
   file << node_shape[nodeTypeString.c_str()] << ";" << endl;
}

// -------------------------------------------------------------------------
void write_selected_nodes( int iNodeMax, WGraph & myGraph, 
   ProbabilisticNet & PNet, const String & TypeNet, 
   map<string,string> node_shape, ofstream & file, int cluster )
{
    // If TypeNet = DBN: write all nodes whose names end with cluster # 
    //                   i.e. ('0' or '1')
    // Otherwise: write ALL nodes
 
    for (int index=0; index<= iNodeMax; index++)
    {
	if (myGraph.IsValidINode(index) )  // if index is valid
	{
	   String name = myGraph.NodeName(index);  // get node name
	   // if not DBN: always write node
           // if DBN: write node only if last character of name 
           //         matches cluster # ('0' or '1')
	   if ( (TypeNet != "DBN") || (name[name.size()-1] == ('0'+cluster)) )
	       write_single_node(name, PNet, node_shape, file);
	}
    }
}
// -------------------------------------------------------------------------
void write_all_nodes( int iNodeMax, WGraph & myGraph, ProbabilisticNet & PNet, 
   const String & TypeNet, map<string,string> node_shape, ofstream & file)
{
   write_selected_nodes( iNodeMax, myGraph, 
			 PNet, TypeNet, node_shape, file, 0 );
}
// -------------------------------------------------------------------------
void write_all_nodes_with_entropy(  int iNodeMax, WGraph & myGraph, 
			   BayesNet & BNet, ofstream & file)
{
    for (int index=0; index<= iNodeMax; index++)
	if (myGraph.IsValidINode(index) )  // if index is valid
	{
  	      String nodeName = myGraph.NodeName(index);  // get node name
	      double entropy = Entropy(index,BNet);
	      file << " \"" << nodeName << "\" ";
	      file << "[shape=ellipse,label=\"" << nodeName;
 	      file << "\\n(Entropy=" << fixed << setprecision(3) << entropy;
	      file << ")\"];" << endl;
	} // end of if
}
// -------------------------------------------------------------------------
void write_all_nodes_with_MI( int iNodeMax, WGraph & myGraph, 
			   BayesNet & BNet, 
			   bool want_percentage,
			   int target_node_index, double scale_factor, 
			   ofstream & file)
{
    for (int index=0; index<= iNodeMax; index++)
    {
	if (myGraph.IsValidINode(index) )  // if index is valid
	{
	   String nodeName = myGraph.NodeName(index);  // get node name
	   if (index == target_node_index)
           {
		   // For target_node: provide entropy.
                   // For output want something like this:
		   //    "X" [shape=doubleoctagon,label="X\n(Entropy=0.45)"];
	      double entropy = Entropy(index,BNet);
	      file << " \"" << nodeName << "\" ";
	      file << "[shape=doubleoctagon,label=\"" << nodeName;
 	      file << "\\n(Entropy=" << fixed << setprecision(3) << entropy;
	      file << ")\"];" << endl;
          }
	   else
	   {
		   // For all other nodes provide connection strength
	      // Step 1:calculate connection strength (target_node_index,index)
              double MI = MutualInformation( index, target_node_index, BNet,
					     want_percentage);

              // Step 2: scale result + calculate gray value between 0 and 100.
              int gray_value; // need value between 0 (black) and 100 (white)
	      if (want_percentage) // Percentage is between 0 and 100.  
                      if (MI<0)   gray_value = 100; // If MI% is undefined
	              else        gray_value = int(100.0-MI);
              else                 // MI/scale_factor is between 0 and 1
		      gray_value = int( 100*(1.0-MI/scale_factor) );

              // to make sure you can still read node label: 
	      if (gray_value < 30)  gray_value = 30; // set gray to at least 30

	      // Write to file something like this:
	      // "Y" [shape=ellipse,style=filled,fillcolor=gray50,
	      //      label="Y\n(0.543)"];
	      // or:  "Y" [shape=ellipse,style=filled,fillcolor=gray50,
	      //      label="Y\n(5.4%)"];

	      file << " \"" << nodeName << "\" ";
	      file << "[shape=ellipse,style=filled,fillcolor=gray";
	      file << gray_value << ",label=\"" << nodeName << "\\n(";
              if (want_percentage)
                      if (MI<0) file << "Undefined \%"; // if MI% undefined
	              else file << fixed << setprecision(1) << MI << "\%";
	      else    file << fixed << setprecision(3) << MI;
	      file << ")\"];" << endl;
           }  // end of else
	} // end of if
    } // end of for
}
// --------------------------------------------------------------------------
void write_incoming_arcs_to_node( int index, WGraph & myGraph, ofstream & file)
{
   String name = myGraph.NodeName(index);  // get node name
   int n_parents = myGraph.nParent(index);  // get # of parents
 
   // get array of parents (as vector of indices)
   Vector<int> parents;  
   myGraph.GetParents(&parents,index);

   for (int i=0; i<n_parents; i++)   // for all parents
   {
      // print line    "parentName" -> "nodeName";        to file
      file << " \"" << myGraph.NodeName(parents[i]) << "\" -> \"" << name;
      file << "\";" << endl;
   } 
}

// --------------------------------------------------------------------------
void write_incoming_arcs_to_node_with_LS( int index, WGraph & myGraph, 
       ofstream & file, BayesNet * BNet, double scale_factor,
       const string & formula, bool want_percentage)
{
   String name = myGraph.NodeName(index);  // get node name
   int n_parents = myGraph.nParent(index);  // get # of parents
 
   // get array of parents (as vector of indices)
   Vector<int> parents;  
   myGraph.GetParents(&parents,index);

   for (int i=0; i<n_parents; i++)   // for all parents
   {
      // calculate link strength for arc
      double LS = LinkStrength( index, parents[i], *BNet, formula, 
				want_percentage );

      int gray_value; // calculate value between 0 (black) and 100 (white)
      if (want_percentage)  // Percentage is between 0 and 100.
              if (LS < 0)   gray_value = 0;  // if LS% is undefined  
              else          gray_value = int(100.0-LS);
      else                  // LS/scale_factor is between 0 and 1.
	      gray_value = int( 100*(1.0-LS/scale_factor) );
      // if gray_scale too light to see, make darker (i.e. cap at 90), and 
      // use dashed line instead to indicate the change
      bool Weak_Link = false; // indicates whether dashed or solid line
      if (gray_value > 90) { gray_value = 90; Weak_Link = true; }
 
      // print expression    "parentName" -> "nodeName"        to file
      file << " \"" << myGraph.NodeName(parents[i]) << "\" -> \"" << name;
      file << "\"";
      // write label and gray_scale for arc. 
      // Example: ``[label="0.4",color=gray60];'' or ``[label="60.1%, ...]''
      file << " [label=\"";
      if (want_percentage) 
	      if (LS<0)  file << "Undefined \%";  // if LS% undefined
              else       file << fixed << setprecision(1) << LS << "\%";
      else    file << fixed << setprecision(3) << LS; 
      file << "\",color=gray" << gray_value;
      if (Weak_Link)   file << ",style=dashed";
      file << "];" << endl;
   } 
}
// --------------------------------------------------------------------------



