\name{MaskEvidBuf}
\alias{MaskEvidBuf}
\title{Hides nodes of current learning buffer with given probability }
\description{
The function hides nodes of current learning buffer with given probability. The function applies for all existing values (hidden and observed), i.e. observed may become hidden and vice versa. By default this function unhides all hidden values. If a node did not have sample, it can not be unhidden. 
}
\usage{
MaskEvidBuf(BayesNet, whatNodes) 
}
\arguments{
\item{BayesNet}{Bayesian network or MRF, which has been already created in the workspace. The action is carried out on this network. }
\item{whatNodes}{String. Array of nodes.}
}
\examples{
MaskEvidToBuf(net, "NodeA^true NodeB^false") 
}
\keyword{MaskEvidBuf}