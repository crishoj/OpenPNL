\name{GetGaussianCovar}
\alias{GetGaussianCovar}
\title{Returns variance of gaussian distribution }
\description{
The function returns variance of gaussian distribution as float vector. 
}
\usage{
GetGaussianCovar(Net, nodes, DiscreteParentsValues) 
}
\arguments{
\item{Net}{Bayesian network or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Name of node whose variance are needed. }
\item{DiscreteParentsValues}{Optional Parameter. String. When you deal with mixture networks and current continuous node has discrete parents, it is necessary to define their values}
}
\examples{
CovarX2 <- GetGaussianCovar(net, "x2") 
}
\keyword{GetGaussianCovar}