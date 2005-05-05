\name{GetSoftMaxWeights}
\alias{GetSoftMaxWeights}
\title{Returns weights matrix of SoftMax distribution}
\description{
The function returns weights matrix of SoftMax distribution as float vector.
}
\usage{
GetSoftMaxOffset(Net, node, DiscreteParentsValues)
}
\arguments{
\item{Net}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
\item{node}{String. Name of node whose weight matrix is needed}
\item{DiscreteParentsValues}{Optional Parameter. String. When you deal with mixture networks and current discrete node has both continuous and discrete parents, it is necessary to define discrete parents values}
}
\examples{
MeanX2 <- GetSoftMaxWeights(net, "x2") 
}
\keyword{GetSoftMaxWeights}