\name{GetCurEvidenceLogLik}
\alias{GetCurEvidenceLogLik}
\title{Returns logarithm of likelihood for current evidence}
\description{
The function returns logarithm of likelihood for current evidence.
 }
\usage{
GetCurEvidenceLogLik(BayesNet)
}
\arguments{
\item{BayesNet}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
}
\examples{
EditEvidence(net, "NodeA^true NodeB^false")
logLik <- GetCurEvidenceLogLik(net) 
}
\keyword{GetCurEvidenceLogLik}