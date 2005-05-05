\name{GetPTabularString}
\alias{GetPTabularString}
\title{Returns probability of discrete node as one string }
\description{
The function returns probability of discrete node as string. If values of parents are given then probabilities only for these parents configuration will be returned. If concrete values of node are given in array node then only probabilities of these values will be returned. 
 }
\usage{
GetPTabularString(Net, node, parents)
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
NodeB^true^NodeA^true^0.99 NodeB^false^NodeA^true^0.01 NodeB^true^NodeA^false^0.4 NodeB^false^NodeA^false^0.6"

PNodeBTrue
"NodeB^true^NodeA^true^0.99 NodeB^true^NodeA^false^0.4"

PNodeBNodeATrue
"NodeB^true^NodeA^true^0.99 NodeB^false^NodeA^true^0.01"

PNodeBTrueNodeAFalse
"NodeB^true^NodeA^false^0.4"
}
\keyword{GetPTabularString}