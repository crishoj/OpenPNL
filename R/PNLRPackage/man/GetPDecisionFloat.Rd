\name{GetPDecisionFloat}
\alias{GetPDecisionFloat}
\title{Returns probability of decision node as a float vector }
\description{
The function returns probability of decision node as a float vector. If values of parents are given then probabilities only for these parents configuration will be returned. If concrete values of node are given in array node then only probabilities of these values will be returned. 
}
\usage{
GetPDecisionFloat(Limid, node)
GetPDecisionFloat(Limid, node, parents)
}
\arguments{
\item{Limid}{LIMID, which has been already created in the workspace. The action is carried out on this network. }
\item{node}{String. Name of node or array of node values whose probabilities are needed.}
\item{parents}{String. Values of parents of the node.}
}
\examples{
##Let's consider getting of probabilities for node "NodeB" from example for SetPDecision method. 
##Some variants to get "NodeB" probabilities are available: 

PNodeB <- GetPDecisionFloat(id, "NodeB")
PNodeBTrue <- GetPDecisionFloat(id, "NodeB^true")
PNodeBNodeATrue <- GetPDecisionFloat(id, "NodeB", "NodeA^true")
PNodeBTrueNodeAFalse <- GetPDecisionFloat(id, "NodeB^true", "NodeA^false")

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
\keyword{GetPDecisionString}