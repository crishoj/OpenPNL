\name{SetPTabular}
\alias{SetPTabular}
\title{Assigns probability to one or several values of one discrete node }
\description{
The function sets [conditional] probability for one or several values of the discrete node [for given values of parents]. Sizes of values array and probabilities array must be the same. While distribution of probabilities is not specified it is considered uniform.

For MRFs:
The function sets probability to one or several values combinations of the nodes in one discrete clique. If clique doesn't exist, then it will be created. In value array a node may be specified without value. It means that probabilities will be set for values combinations with all values of this node in the same order as values were specified with AddNode method for the node. If distribution of probabilities is not specified it is considered as uniform. 
 }
\usage{
SetPTabular(Net, value, prob, parentValue)
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{value}{String. Array of node values for wich probabilities are set.}
\item{prob}{Either String or Float Vector. Array of values of probabilities.}
\item{parentValue}{String. Array of values of discrete parents of the node.}
}
\examples{
##Let's assume that node "NodeA" has no parents and can take on two values: "true" and "false". 
##Its distribution of probabilities is unconditional. 

SetPTabular(net, "NodeA^true NodeA^false", "0.7 0.3") 

##Let's node "NodeB" can take on values "true" and "false" too and has only one parent "NodeA". 
##Distribution of probabilities of "NodeB" is conditional. It is necessary to call SetPTabular method 
##for each configuration of parents values

probB1 <- c(0.99, 0.01) 
probB2 <- c(0.4, 0.6) 
SetPTabular(net, "NodeB^true NodeB^false", probB1, "NodeA^true")
SetPTabular(net, "NodeB^true NodeB^false", probB2, "NodeA^false") 

##Now we'll see how to specify probabilities for MRF nodes

##Consider "NodeA" and "NodeB" nodes both can take on one of two values: "true" and "false". 
##Let's join them into clique and set probabilities for some of their values: 

SetPTabular(mrf, "NodeA^true NodeB^false", "0.3")

##Let's consider how to specify some values of probabilities for clique omitting values for nodes: 

AddNode(mrf, "NodeA NodeB", "true false")
SetPTabular(mrf, "NodeB NodeA", c(0.01, 0.46, 0.3, 0.22))

}
\keyword{SetPTabular}