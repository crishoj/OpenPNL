\name{GetPChanceFloat}
\alias{GetPChanceFloat}
\title{Returns probability of chance node as float vector}
\description{
The function returns float vector with probabilities of chance node. If values of parents are given then probabilites only for these parents configuration will be returned. If concrete values of node are given in array node then only probabilities of these values will be returned. 
}
\usage{
GetPChanceFloat(Limid, node)
GetPChanceFloat(Limid, node, parents)
}
\arguments{
\item{Limid}{LIMID, which has been already created in the workspace. The action is carried out on this network. }
\item{node}{String. Name of node or array of node values whose probabilities are needed.}
\item{parents}{String. Values of parents of the node.}
}
\examples{
##Let's consider getting of probabilities for node "NodeB" from example for SetPChance method. 
##Some variants to get "NodeB" probabilities are available: 

PNodeB <- GetPChanceFloat(id, "NodeB")
PNodeBTrue <- GetPChanceFloat(id, "NodeB^true")
PNodeBNodeATrue <- GetPChanceFloat(id, "NodeB", "NodeA^true")
PNodeBTrueNodeAFalse <- GetPChanceFloat(id, "NodeB^true", "NodeA^false") 

##Results of these calls: 

print(PNodeB) 
0.99 0.01 0.4 0.6 
print(PNodeBTrue)
0.99 0.4 
print(PNodeBNodeATrue)
0.99 0.01 
print(PNodeBTrueNodeAFalse)
0.4 

}
\keyword{GetPChanceString}