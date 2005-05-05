\name{ClearEvid}
\alias{ClearEvid}
\title{Clears current evidence  }
\description{
The function clears current evidence. 
 }
\usage{
ClearEvid(Net)
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
}
\examples{
EditEvidence(net, "NodeA^true NodeB^false")
ClearEvid(net) 

##Current evidence is empty now.
}
\keyword{ClearEvid}