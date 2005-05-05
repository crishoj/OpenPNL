\name{GetMPE}
\alias{GetMPE}
\title{Returns MPE  }
\description{
The function returns most probable explanation (MPE) for specified nodes using current evidence. The inference algorithm may be set to property "Inference" with method SetProperty(). By default Pearl inference is used. Besides Gibbs sampling, Junction Tree inference and Naive inference are available. Corresponding property "Inference" values are "pearl", "gibbs", "jtree", "naive".
 }
\usage{
GetMPE(Net, nodes)
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Array of nodes from one family.}
}
\examples{
##Get MPE for node "NodeA" with Junction Tree inference: 

SetProperty(net, "Inference", "jtree")
jtreeMPENodeA <- GetMPE(net, "NodeA") 

##Get MPE for nodes "NodeA" and "NodeB" with Pearl inference 
##(nodes "NodeA" and "NodeB" must be from the same family): 

SetProperty(net, "Inference", "pearl")
pearlMPENodeA <- GetMPE(net, "NodeA NodeB") 
}
\keyword{GetMPE}