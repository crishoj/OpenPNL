\name{CurEvidToBuf}
\alias{CurEvidToBuf}
\title{Stores current evidence to the evidence buffer}
\description{
The function stores current evidence to the buffer of evedences. Current evidence is not cleared. 
 }
\usage{
CurEvidToBuf(Net) 
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
}
\examples{
EditEvidence(net, "NodeA^true NodeB^false")
EditEvidence(net, "NodeA^false NodeC^true")
CurEvidToBuf(net)
EditEvidence(net, "NodeB^true")
CurEvidToBuf(net) 

##In current evidence there are three observed nodes now: "NodeA" takes on "false" value, 
##"NodeB" and "NodeC" take on "true" values. 

##In evidence buffer there are two evidences now. In first evidence "NodeA" and "NodeB" 
##take on "false" values, and "NodeC" takes on "true" value. In second one "NodeA" takes 
##on "false" value, "NodeB" and "NodeC" take on "true" values. 
}
\keyword{CurrEvidToBuf}