\name{GetPTabularFloat}
\alias{GetPTabularFloat}
\title{Returns probability of discrete node as float vector  }
\description{
The function returns probability of discrete node as float vector. If values of parents are given then probabilities only for these parents configuration will be returned. If concrete values of node are given in array node then only probabilities of these values will be returned. 
 }
\usage{
GetPTabularFloat(Net, node, parents)
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{node}{String. Name of node or array of node values whose probabilities are needed. }
\item{parents}{String. Values of parents of the node.}
}
\examples{
##Let's consider getting of probabilities for node "NodeB" from example for SetPTabular method. 
##Some variants to get "NodeB" probabilites are available: 

PNodeB <- GetPTabularString(net, "NodeB")
PNodeBTrue <- GetPTabularString(net, "NodeB^true")
PNodeBNodeATrue <- GetPTabularString(net, "NodeB", "NodeA^true")
PNodeBTrueNodeAFalse <- GetPTabularString("NodeB^true", "NodeA^false") 

## Results of these calls: 

PNodeB
0.99  0.01  0.4  0.6

PNodeBTrue
0.99  0.4

PNodeBNodeATrue
0.99  0.01

PNodeBTrueNodeAFalse
0.4
}
\keyword{GetPTabularFloat}