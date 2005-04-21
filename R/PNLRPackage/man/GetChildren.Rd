\name{GetChildren}
\alias{GetChildren}
\title{Returns list of children for given node}
\description{
The function returns list of children for given node or some nodes. 
 }
\usage{
GetChildren(Net, nodes)
}
\arguments{
\item{Net}{Bayesian network or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. One or several nodes whose children are needed.}
}
\examples{
##Get children of node "NodeA"
NodeAChildren <- GetChildren(net, "NodeA") 
##Get children of node "NodeA" and children of node "NodeB". If node "NodeC" is child both "NodeA" and "NodeB" then it is once in result list of children
NodeAandBChildren <- GetChildren(net, "NodeA NodeB") 
}
\keyword{GetChildren}