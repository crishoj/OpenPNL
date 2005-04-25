\name{DelArc}
\alias{DelArc}
\title{Deletes arcs}
\description{
The function deletes one or several arcs from the network. 
 }
\usage{
DelArc(Net, from, to)
}
\arguments{
\item{Net}{Bayesian network, LIMID or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{from}{String. Array of starting nodes of the arcs to delete.}
\item{to}{String. Array of ending nodes of the arcs to delete.}
}
\examples{
## Delete edge from node "NodeA" to node "NodeB"
DelArc(net, "NodeA", "NodeB") 
##Delete following two edges if they exist: from "NodeA" to "NodeB" and from "NodeA" to "NodeC"
DelArc(net, "NodeA", "NodeB NodeC") 
##Delete all edges with starting node "NodeA" or "NodeB" and ending node "NodeC" or "NodeD": 
DelArc(net, "NodeA NodeB", "NodeC NodeD") 

}
\keyword{DelArc}