\name{EditEvidence}
\alias{EditEvidence}
\title{Edits current evidence }
\description{
The function sets or edits current evidence. New observed nodes are added to current evidence with EditEvidence method or old values of observed nodes are changed.
 }
\usage{
EditEvidence(Net, values)
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{values}{String. Array of observed nodes with values.
}
\examples{
EditEvidence(net, "NodeA^true NodeB^false")
EditEvidence(net, "NodeA^false NodeC^true") 

##In current evidence there are three observed tabular nodes: "NodeA", "NodeB" 
##(both are false) and "NodeC"(true). 

EditEvidence(net, "ContNodeA^dim1^-15.0 ContNodeA^dim2^6.4")
EditEvidence(net, "ContNodeB^dim1^5.0") 

##In current evidence there are two observed continuous nodes: 
##"NodeA" (with 2 dimensions), "NodeB" (with one dimension). 
}
\keyword{EditEvidence}