\name{GetEvidBufLogLik}
\alias{GetEvidBufLogLik}
\title{Returns array of logarithms of likelihood for evidences from buffer }
\description{
The function returns array of logarithms of likelihood for evidences from buffer.
 }
\usage{
GetEvidBufLogLik(BayesNet)
}
\arguments{
\item{BayesNet}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
}
\examples{
##Let's generate evidences, put them to buffer and find  their logarifmic likelihood. 

GenerateEvidences(net, 1000) 
bufLogLiks <- GetEvidBufLogLik(net) 

\keyword{GetEvidBufLogLik}