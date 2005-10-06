/*
 * Copyright (c) 2005 Imme Ebert-Uphoff
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
#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;

PNLW_USING


// --------------------------------------------------------------------------
// --- Define a few auxiliary functions ---
void write_single_node( String nodeName, ProbabilisticNet & PNet, 
			map<string,string> node_shape, ofstream & file);

void write_selected_nodes( int iNodeMax, WGraph & myGraph, 
			   ProbabilisticNet & PNet, String TypeNet, 
			   map<string,string> node_shape, 
			   ofstream & file, int cluster=0 );

void write_arcs_to_node( int index, WGraph & myGraph, ofstream & file);

// --------------------------------------------------------------------------
// Call PNLtoGraphviz for BayesNet
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
//  This is the MAIN ROUTINE called for ALL types of nets.
//
//  This function is NOT intended for external use: only to be called by 
//  the PNLtoGraphviz functions for specific network types above.
//
int PNLtoGraphviz ( ProbabilisticNet & PNet, const string& filename, 
    String TypeNet, map<string,string> customized_node_shape )
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
    //   TypeNet         = BayesNet, DBN or LIMID  (MRF not yet implemented)
    //   customized_node_shape = map describing node_shape to be used for 
    //                           each nodeType 
    //   (default value is empty map --> then default shapes below are used)

    // Define default node shapes for all 5 node types:
    //
    // Node types are: discrete, continuous, chance, decision, value.
    // Shape specification according to node attributes in dot-language 
    // (See GraphViz manual "Drawing graphs with dot".) 
    map<string,string> node_shape;
    node_shape["discrete"]   = "[shape=ellipse]";
    node_shape["continuous"] = "[shape=ellipse,style=filled,fillcolor=gray90]";
    node_shape["chance"]     = "[shape=ellipse]";
    node_shape["decision"]   = "[shape=box]";
    node_shape["value"]      = "[shape=diamond]";

    // Now overwrite defaults by user supplied shapes:
    // go through all pairs of customized_node_shape (if any) and copy 
    // its records to node_shape
    // --> User only needs to describe those types to be changed from default.
    for ( map<string,string>::iterator pos = customized_node_shape.begin(); 
	  pos != customized_node_shape.end(); pos++)
	      node_shape[pos->first] = pos->second;

    WGraph & myGraph = PNet.Graph();  // get graph of PNet

    ofstream file(filename.c_str());  // open output file
    if ( !file)  
    {   
        cerr << "Can't open output file " << filename << endl;
        return(0);
    }    

    // tell user what's going on and where to find file:
    cout << "Writing graph of type  \"" << TypeNet << "\"  to file  \"";
    cout << filename << "\" ." << endl << endl;

    // write header for directed graph
    file << "digraph G {" << endl << endl;  

    // Get maximal node index used in graph.  
    // Note that there may be unused (invalid) indices!
    int iNodeMax = myGraph.iNodeMax();

    // Write all nodes with node shapes

    if ( TypeNet != "DBN" )  // for BayesNet or LIMID: write ALL nodes 
    {
           write_selected_nodes( iNodeMax, myGraph, PNet, TypeNet, node_shape,
               file );
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

    // Writing arcs:
    // Go through all nodes again and write for each node arcs 
    // from all its PARENTS to ITSELF
    for (int index=0; index<= iNodeMax; index++)
    {
        if (myGraph.IsValidINode(index) )  // if index is valid 
	    write_arcs_to_node( index, myGraph, file);  
            // writes arcs from its parents TO current node
    }

    file << endl << "} " << endl;  // last line of file: needs closing bracket
    return(1);     // file handle is closed automatically
}

// -------------------------------------------------------------------------
void write_single_node( String nodeName, ProbabilisticNet & PNet, 
   map<string,string> node_shape, ofstream & file)
{
   // determine nodeType -- can be: discrete, continuous, chance, decision or value
   TokArr nodeType = PNet.GetNodeType((TokArr)nodeName);  
   String nodeTypeString = nodeType[0].Name();

   // write quotes around name (otherwise dot has trouble with special characters in names)
   file << " \"" << nodeName << "\" ";
   // write shape properties depending on nodeType
   file << node_shape[nodeTypeString.c_str()] << ";" << endl;
}

// -------------------------------------------------------------------------
void write_selected_nodes( int iNodeMax, WGraph & myGraph, 
   ProbabilisticNet & PNet, String TypeNet, map<string,string> node_shape, 
   ofstream & file, int cluster )
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

// ---------------------------------------------------------------------------
void write_arcs_to_node( int index, WGraph & myGraph, ofstream & file)
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





