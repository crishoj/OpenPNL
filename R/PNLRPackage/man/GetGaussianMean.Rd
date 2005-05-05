\name{GetGaussianMean}
\alias{GetGaussianMean}
\title{Returns mean of gaussian distribution  }
\description{
The function returns mean of gaussian distribution as float vector.
}
\usage{
GetGaussianMean(Net, nodes, DiscreteParentsValues)
}
\arguments{
\item{Net}{Bayesian network or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Name of node whose mean is needed}
\item{DiscreteParentsValues}{Optional Parameter. String. When you deal with mixture networks and current continuous node has discrete parents, it is necessary to define their values}
}
\examples{
MeanX2 <- GetGaussianMean(net, "x2") 
}
\keyword{GetGaussianMean}