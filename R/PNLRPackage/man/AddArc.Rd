\name{AddArc}
\alias{AddArc}
\title{Adds arcs}
\description{
The function adds one or several arcs to the network. Each starting node is connected with each ending node.
 }
\usage{
AddArc(BayesNet, from, to) 
}
\arguments{
\item{net}{Bayesian network, LIMID or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{from}{String. Array of starting nodes of the arcs. Nodes are separated by the space.}
\item{to}{String. Array of ending nodes of the arcs. Nodes are separated by the space.}
}
\examples{
## Add edge from node "NodeA" to node "NodeB"
AddArc(net, "NodeA", "NodeB")  
##Add two edges: from "NodeA" to "NodeB" and from "NodeA" to "NodeC"
AddArc(net, "NodeA", "NodeB NodeC") 
##Add four edges: from "NodeA" to "NodeC", from "NodeA" to "NodeD", from "NodeB" to "NodeC" and from "NodeB" to "NodeD"
AddArc(net, "NodeA NodeB", "NodeC NodeD") 

}
\keyword{AddArc}