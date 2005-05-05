\name{SetClique}
\alias{SetClique}
\title{Joins nodes into a clique}
\description{
This function joins nodes into a clique. Default uniform distribution is set on the clique. Clique may also be formed with SetPTabular method. 
 }
\usage{
SetClique(MRF, nodes)
}
\arguments{
\item{MRF}{MRF, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Array of nodes for new clique. }
}
\examples{
##The following instruction joins "NodeA" and "NodeB" nodes to a clique: 

SetClique(mrf, "NodeA NodeB") 

}
\keyword{SetClique}