\name{DestroyClique}
\alias{DestroyClique}
\title{Destroys clique}
\description{
The function destroys clique. Nodes are not deleted from the network
}
\usage{
DestroyClique(MRF, nodes)
}
\arguments{
\item{MRF}{MRF, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Node array of clique, that is destroyed. }
}
\examples{
## Let's destroy clique of "NodeA" and "NodeB"nodes: 

DestroyClique(mrf, "NodeA NodeB")

}
\keyword{DestroyClique}