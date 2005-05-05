\name{GetGaussianWeights}
\alias{GetGaussianWeights}
\title{Returns weights of gaussian distribution.  }
\description{
The function returns weights of parent of specified node as float vector. 
}
\usage{
GetGaussianWeights(Net, nodes, ContinuousParent, DiscreteParentsValues) 
}
\arguments{
\item{Net}{Bayesian network or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Name of node whose weights are needed. }
\item{ContinuousParent}{String. A parent of node which weights we are interested in. }
\item{DiscreteParentsValues}{Optional Parameter. String. When you deal with mixture networks and current continuous node has discrete parents, it is necessary to define their values}
}
\examples{
WeightsYX2 <- GetGaussianWeights(net, "y", "x2") 
}
\keyword{GetGaussianWeights}