\name{SetPGaussian}
\alias{SetPGaussian}
\title{Sets parameters for gaussian variate}
\description{
The function sets parameters for gaussian variate. 
 }
\usage{
SetPGaussian(BayesNet, var, mean, variance, weight)
}
\arguments{
\item{BayesNet}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
\item{var}{String. Name of node which probability is setted. 
}
\item{mean}{Either String or Float Vector. Vector of means.}
\item{variance}{Either String or Float Vector. Matrix of variance (for one-dimensional variate it is only one number). }
\item{weight}{Either String or Float Vector. Matrixes of weights. This parameter is needed only when the node has parents. For more details about matrixes of weights see User Guide, chapter "Multivariate Gaussian case"}
}
\examples{
##Let's set gaussian distribution for one - dimentional node without parents. 
SetPGaussian(net, "x0", "1.0", "4.0") 

##Let's set gaussian distribution for one - dimentional node with two parents. 
weightsX2 <- c(1.0, 2.0) 
SetPGaussian(net, "x2", "0.0", "2.0", weightsX2)

##Let's set gaussian distribution for two - dimensional node without parents. 
X0mean <- c(1.0, 2.5) 
X0variance <- c(4.0, 0.1, 0.1, 4.0) 
SetPGaussian(net, "x0", X0mean, X0variance) 

##Let's set gaussian distribution for two - dimentional node with three two - dimensional parents
SetPGaussian(net, "x2", "1.0 2.5", "4.0 0.1 0.1 4.0", "1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0") 

}
\keyword{SetPGaussian}