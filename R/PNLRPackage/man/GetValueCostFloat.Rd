\name{GetValueCostFloat}
\alias{GetValueCostFloat}
\title{Returns utility function of value node as a float vector}
\description{
The function returns utility function of value node as a float vector. If values of parents are given then probabilites only for these parents configuration will be returned. If concrete values of node are given in array node then only probabilities of these values will be returned. 
}
\usage{
GetValueCostFloat(Limid, node)
GetValueCostFloat(Limid, node, parents)
}
\arguments{
\item{Limid}{LIMID, which has been already created in the workspace. The action is carried out on this network. }
\item{node}{String. Name of node or array of node values whose probabilities are needed.}
\item{parents}{String. Values of parents of the node.}
}
\examples{
##Let's consider getting of utility function for node "NodeB" from example for SetValueCost method. 
##Some variants to get "NodeB" utility function are available: 

PNodeB <- GetValueCostFloat(id, "NodeB")
PNodeBNodeATrue <- GetValueCostFloat(id, "NodeB", "NodeA^true")
PNodeBTrueNodeAFalse <- GetValueFloat(id, "NodeB^cost", "NodeA^false")

Results of these calls: 

print(PNodeB)
-1000.0 300.0 
print(PNodeBNodeATrue)
-1000.0 
print(PNodeBTrueNodeAFalse) 
300.0 

}
\keyword{GetValueCostString}