\name{LearnStructure}
\alias{LearnStructure}
\title{Learns structure of the network }
\description{
The function learns structure of the network using evidence buffer.
}
\usage{
LearnStructure(BayesNet) 
}
\arguments{
\item{BayesNet}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
}
\examples{
AddEvidToBuf(net, "NodeA^true NodeB^false")
AddEvidToBuf(net, "NodeA^false NodeC^true")
LearnStructure(net) 
}
\keyword{LearnStructure}