\name{GetPChanceString}
\alias{GetPChanceString}
\title{Returns probability of chance node as a string}
\description{
The function returns string with probabilities of chance node. If values of parents are given then probabilites only for these parents configuration will be returned. If concrete values of node are given in array node then only probabilities of these values will be returned. 
}
\usage{
GetPChanceString(Limid, node)
GetPChanceString(Limid, node, parents)
}
\arguments{
\item{Limid}{LIMID, which has been already created in the workspace. The action is carried out on this network. }
\item{node}{String. Name of node or array of node values whose probabilities are needed.}
\item{parents}{String. Values of parents of the node.}
}
\examples{
##Let's consider getting of probabilities for node "NodeB" from example for SetPChance method. 
##Some variants to get "NodeB" probabilities are available: 

PNodeB <- GetPChanceString(id, "NodeB")
PNodeBTrue <- GetPChanceString(id, "NodeB^true")
PNodeBNodeATrue <- GetPChanceString(id, "NodeB", "NodeA^true")
PNodeBTrueNodeAFalse <- GetPChanceString(id, "NodeB^true", "NodeA^false") 

##Results of these calls: 

print(PNodeB) 
"NodeB^true^NodeA^true^0.99 NodeB^false^NodeA^true^0.01 NodeB^true^NodeA^false^0.4 NodeB^false^NodeA^false^0.6" 
print(PNodeBTrue)
"NodeB^true^NodeA^true^0.99 NodeB^true^NodeA^false^0.4" 
print(PNodeBNodeATrue)
"NodeB^true^NodeA^true^0.99 NodeB^false^NodeA^true^0.01" 
print(PNodeBTrueNodeAFalse)
"NodeB^true^NodeA^false^0.4" 

}
\keyword{GetPChanceString}