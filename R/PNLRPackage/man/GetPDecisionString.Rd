\name{GetPDecisionString}
\alias{GetPDecisionString}
\title{Returns probability of decision node }
\description{
The function returns probability of decision node. If values of parents are given then probabilities only for these parents configuration will be returned. If concrete values of node are given in array node then only probabilities of these values will be returned. 
}
\usage{
GetPDecisionString(Limid, node)
GetPDecisionString(Limid, node, parents)
}
\arguments{
\item{Limid}{LIMID, which has been already created in the workspace. The action is carried out on this network. }
\item{node}{String. Name of node or array of node values whose probabilities are needed.}
\item{parents}{String. Values of parents of the node.}
}
\examples{
##Let's consider getting of probabilities for node "NodeB" from example for SetPDecision method. 
##Some variants to get "NodeB" probabilities are available: 

PNodeB <- GetPDecisionString(id, "NodeB")
PNodeBTrue <- GetPDecisionString(id, "NodeB^true")
PNodeBNodeATrue <- GetPDecisionString(id, "NodeB", "NodeA^true")
PNodeBTrueNodeAFalse <- GetPDecisionString(id, "NodeB^true", "NodeA^false")

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
\keyword{GetPDecisionString}