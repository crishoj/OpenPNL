\name{AddEvidToBuf}
\alias{AddEvidToBuf}
\title{Adds evidence to the evidence buffer }
\description{
The function adds evidence to the buffer. Current evidence is not changed.
 }
\usage{
AddEvidToBuf(Net, values)
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{values}{String. Array of observed nodes with values.}
}
\examples{
AddEvidToBuf(net, "NodeA^true NodeB^false")
AddEvidToBuf(net, "NodeA^false NodeC^true") 

##In evidence buffer there are two evidences now. In first evidence "NodeA" takes on 
##"true" value and "NodeB" takes on "false" value . In second one "NodeA" takes on "false" 
##value and "NodeC" takes on "true" value. 

AddEvidToBuf(net, "ContNodeA^dim1^-15.0 ContNodeA^dim2^6.4")
AddEvidToBuf(net, "ContNodeB^dim1^5.0") 

##There are two evidences for continuous nodes: "NodeA" (with 2 dimensions) and 
##"NodeB" (with one dimension) in the last example which we add to the buffer. 
}
\keyword{AddEvidToBuf}