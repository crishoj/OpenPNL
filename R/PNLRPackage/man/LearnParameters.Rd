\name{LearnParameters}
\alias{LearnParameters}
\title{Learns network parameters }
\description{
The function learns network parameters using evidence buffer. Learning algorithm may be set to property "Learning" with method SetProperty(). By default EM learning is used. Besides Bayes learning is available. 
}
\usage{
LearnParameters(Net) 
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
}
\examples{
AddEvidToBuf(net, "NodeA^true NodeB^false")
AddEvidToBuf(net, "NodeA^false NodeC^true")
LearnParameters(net) 
}
\keyword{LearnParameters}