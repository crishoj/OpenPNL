\name{GetGaussianCovar}
\alias{GetGaussianCovar}
\title{Returns variance of gaussian distribution }
\description{
The function returns variance of gaussian distribution as float vector. 
}
\usage{
GetGaussianCovar(BayesNet, nodes) 
}
\arguments{
\item{BayesNet}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Name of node whose variance are needed. }
}
\examples{
CovarX2 <- GetGaussianCovar(net, "x2") 
}
\keyword{GetGaussianCovar}