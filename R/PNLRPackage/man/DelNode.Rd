\name{DelNode}
\alias{DelNode}
\title{Deletes nodes}
\description{
The function deletes one or several nodes from the network together with all adjacent edges. 
 }
\usage{
DelNode(Net, nodes)
}
\arguments{
\item{Net}{Bayesian network, LIMID, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Array of names of nodes to delete.}
}
\examples{
## Delete nodes "NodeA" and "NodeB" from Bayessian net
DelNode(net, "NodeA NodeB")  
}
\keyword{DelNode}