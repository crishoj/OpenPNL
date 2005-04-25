\name{GetValueCostString}
\alias{GetValueCostString}
\title{Returns utility function of value node as a string}
\description{
The function returns utility function of value node. If values of parents are given then probabilites only for these parents configuration will be returned. If concrete values of node are given in array node then only probabilities of these values will be returned. 
}
\usage{
GetValueCostString(Limid, node)
GetValueCostString(Limid, node, parents)
}
\arguments{
\item{Limid}{LIMID, which has been already created in the workspace. The action is carried out on this network. }
\item{node}{String. Name of node or array of node values whose probabilities are needed.}
\item{parents}{String. Values of parents of the node.}
}
\examples{
##Let's consider getting of utility function for node "NodeB" from example for SetValueCost method. 
##Some variants to get "NodeB" utility function are available: 

PNodeB <- GetValueCostString(id, "NodeB")
PNodeBNodeATrue <- GetValueCostString(id, "NodeB", "NodeA^true")
PNodeBTrueNodeAFalse <- GetValueCost(id, "NodeB^cost", "NodeA^false")

Results of these calls: 

print(PNodeB)
"NodeB^cost^NodeA^true^-1000.0 NodeB^cost^NodeA^false^300.0" 
print(PNodeBNodeATrue)
"NodeB^cost^NodeA^true^-1000.0" 
print(PNodeBTrueNodeAFalse) 
"NodeB^cost^NodeA^false^300.0" 

}
\keyword{GetValueCostString}