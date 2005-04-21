\name{GetGaussianMean}
\alias{GetGaussianMean}
\title{Returns mean of gaussian distribution  }
\description{
The function returns mean of gaussian distribution as float vector.
}
\usage{
GetGaussianMean(Net, nodes)
}
\arguments{
\item{Net}{Bayesian network or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Name of node whose mean is needed}
}
\examples{
MeanX2 <- GetGaussianMean(net, "x2") 
}
\keyword{GetGaussianMean}