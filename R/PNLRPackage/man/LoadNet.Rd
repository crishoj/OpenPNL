\name{LoadNet}
\alias{LoadNet}
\title{Loads network from file  }
\description{
The function loads network from file with given name.
}
\usage{
LoadNet(BayesNet, filename) 
}
\arguments{
\item{BayesNet}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
\item{filename}{String. File name.}
}
\examples{
##Load net structure and distributions from file "net.xml": 

LoadNet(net, "net.xml") 
}
\keyword{LoadNet}