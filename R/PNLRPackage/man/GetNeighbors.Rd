\name{GetNeighbors}
\alias{GetNeighbors}
\title{Returns list of neighbours for given node as one string.}
\description{
The function returns list of neighbours for given node or some nodes. If neighbours of several nodes are demanded then neighbors of every node are in the result. 
 }
\usage{
GetNeighbours(Net, nodes) 
}
\arguments{
\item{Net}{Bayesian network or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. One or several nodes whose neighbours are need. Nodes are separated with the space.}
}
\examples{
##Get neighbors of node "NodeA"
NodeANeighbors <- GetNeighbours(net, "NodeA") 
##Get neighbors of node "NodeA" and neighbors of node "NodeB". If node "NodeC" is neighbor of both "NodeA" and "NodeB" then it is once in result list of neighbors
NodeAandBNeighbors <- GetNeighbours(net, "NodeA NodeB") 
}
\keyword{GetNeighbors}