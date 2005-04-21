\name{GetParents}
\alias{GetParents}
\title{Returns list of parents for given node}
\description{
The function returns list of parents for given node or some nodes. If parents of several nodes are demanded then parents of every node are in the result.
 }
\usage{
GetParents(Net, nodes)
}
\arguments{
\item{Net}{Bayesian network or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. One or several nodes whose parents are needed.}
}
\examples{
##Get parents of node "NodeA"
NodeAParents <- GetParents(net, "NodeA") 
##Get parents of node "NodeA" and parents of node "NodeB". If node "NodeC" is parent of both "NodeA" and "NodeB", it will occure once in result list of parents
NodeAandBParents <- GetParents(net, "NodeA NodeB") 
}
\keyword{GetParents}