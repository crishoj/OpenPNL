\name{GetNodeType}
\alias{GetNodeType}
\title{Returns types of the given nodes as strings}
\description{
The function returns array of types (discrete or continuous) of given nodes.
 }
\usage{
GetNodeType(Net, nodes) 
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Array of nodes,  whose type we are interesting in.}
}
\examples{
##Get array of types of nodes "NodeA" and "NodeB": 
GetNodeType(net, "NodeA NodeB") 
##The output of this function call will be the following: "discrete" "discrete"
}
\keyword{GetNodeType}