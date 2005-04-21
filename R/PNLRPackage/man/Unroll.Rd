\name{Unroll}
\alias{Unroll}
\title{Unrolls current Dynamic Bayessian Network }
\description{
The function creates the object of pnlBNet class and put the result of unrolling the current DBN to this net.  
 }
\usage{
Unroll(dbn)
}
\arguments{
\item{dbn}{DBN, which has been already created in the workspace. The action is carried out on this network. }
}
\examples{
net <- Unroll(dbn)
}
\keyword{Unroll}