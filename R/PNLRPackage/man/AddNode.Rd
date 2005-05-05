\name{AddNode}
\alias{AddNode}
\title{Adds nodes}
\description{
The function adds one or several nodes with the same probable states into the network. If the node you want to add is discrete, it is not necessary to specify its type directly. I.e. if node type is leaved blank, we mean that node is discrete.
 }
\usage{
AddNode(net, nodes, values)
}
\arguments{
\item{net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. List of nodes with nodes type (discrete or continuous) specifying. For DBN it is necessary to specify the slice, where node will be added. For LIMIDs the possible node types are "chance", "decision" and "value"}
\item{values}{String. Array of values that nodes can take (or names of dimensions for continuous nodes). }
}
\examples{
## Add one discrete node "NodeA" with states "true" and "false" to bayes net
AddNode(net, "discrete^NodeA", "true false") 
##or 
AddNode(net, "NodeA", "true false") 
## Add one-dimensional continuous node "NodeA" to Bayessian net
AddNode(net, "continuous^NodeA", "dim1") 
## Add discrete nodes "NodeA" and "NodeB" with states "true" and "false" to bayes net
AddNode(net, "discrete^NodeA discrete^NodeB", "true false") 
## Add multivariate nodes "NodeA" with 2 dimensions "dim1" and "dim2" to bayes net
AddNode(net, "continuous^NodeA", "dim1 dim2") 

##To specify DBN is to define two time slices. Now we are going to create DBN, 
##each of which slices consists of 3 nodes
AddNode(dbn, "discrete^Street-0", "true false")
dbn.AddNode(dbn, "discrete^House-0", "true false")
dbn.AddNode(dbn, "discrete^Flat-0", "true false")

dbn.AddNode(dbn, "discrete^Street-1", "true false")
dbn.AddNode(dbn, "discrete^House-1", "true false")
dbn.AddNode(dbn, "discrete^Flat-1", "true false") 

## Now we are going to add nodes to LIMID
##Add one chance node "NodeA" with states "true" and "false" to LIMID: 

AddNode(id, "chance^NodeA", "true false") 

##Add one decision node "NodeB" with states "true" and "false" to LIMID: 

AddNode(id, "decision^NodeB", "true false")

##Add one value node "NodeC" with one state "cost" to LIMID: 

AddNode(id, "value^NodeC", "cost")

##Now we'll see, how to add nodes to Markov random fields
##Let's add one discrete node "NodeA" with states "true" and "false" to the net: 

AddNode(mrf, "discrete^NodeA", "true false") 

##Now we are going to add discrete nodes "NodeA" and "NodeB" with states "true" and "false" to the net: 

AddNode(mrf, "discrete^NodeA discrete^NodeB", "true false") 


}
\keyword{AddNode}