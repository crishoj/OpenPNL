\name{SetPSoftMax}
\alias{SetPSoftMax}
\title{Sets parameters for SoftMax variate}
\description{
The function sets parameters for SoftMax variate. SoftMax is a distribution on the discrete node, when it has continuous parents. 
 }
\usage{
SetPSoftMax(Net, node, weight, offset, DiscreteParentsValue)
}
\arguments{
\item{Net}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
\item{node}{String. Name of node which probability is setted. 
}
\item{weight}{Either String or Float Vector. Vector of weight matrix elements.}
\item{offset}{Either String or Float Vector. Vector of offsets. }
\item{DiscreteParentsValue}{Optional parameter. String. List of tabular parents values. This parameter is necessary in the case of mixture nets only, when current SoftMax node has both continuous and discrete parents. }
}
\examples{
##Consider the following network (nodes x0 and x1 are continuous, node x2 is discrete):

## x0  x1
##  \  /
##   x2


##Let's set SoftMax distribution for discrete node with two states and two continuous parents.

SetPSoftMax(net, "x2", "1 2 3 4", "0.1 0.3") 

##Consider the following network (nodes x0 and x1 are continuous, node x2 and x3 are discrete):

## x0 x1 x2
##  \  | /
##   \ |/
##    x3

##Let's set SoftMax distribution for discrete node with two states and two continuous parents.

SetPSoftMax(net, "x3", "1 2 3 4", "0.1 0.3", "x2^true") 
SetPSoftMax(net, "x3", "4 3 2 1", "0.5 0.7", "x2^false") 
}
\keyword{SetPGaussian}