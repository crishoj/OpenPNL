\name{GetGaussianWeights}
\alias{GetGaussianWeights}
\title{Returns weights of gaussian distribution.  }
\description{
The function returns weights of parent of specified node as float vector. 
}
\usage{
GetGaussianWeights(BayesNet, nodes, parent) 
}
\arguments{
\item{BayesNet}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Name of node whose weights are needed. }
\item{parent}{String. A parent of node which weights we are interested in. }
}
\examples{
WeightsYX2 <- GetGaussianWeights(net, "y", "x2") 
}
\keyword{Bayesian Network}