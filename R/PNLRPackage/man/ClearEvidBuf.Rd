\name{ClearEvidBuf}
\alias{ClearEvidBuf}
\title{Clears evidence buffer }
\description{
The function clears evidence buffer. 
 }
\usage{
ClearEvidBuf(BayesNet) 
}
\arguments{
\item{BayesNet}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
}
\examples{
AddEvidToBuf(net, "NodeA^true NodeB^false")
AddEvidToBuf(net, "NodeA^false NodeC^true")
ClearEvidBuf(net) 

##Evidence buffer is empty now. 

}
\keyword{ClearEvidBuf}