\name{GetJPDFloat}
\alias{GetJPDFloat}
\title{Returns JPD as float vector }
\description{
The function returns joint probability distribution (JPD) for specified nodes using current evidence. The inference algorithm may be set to property "Inference" with method SetProperty(). By default Pearl inference is used. Besides Gibbs sampling, Junction Tree inference and Naive inference are available. Corresponding property "Inference" values are "pearl", "gibbs", "jtree", "naive". 
}
\usage{
GetJPDFloat(Net, nodes)
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Array of nodes from one family.}
}
\examples{
##Get JPD for node "NodeA" with Junction Tree inference: 

SetProperty(net, "Inference", "jtree")
jtreeJPDNodeAFlt = GetJPDFloat(net, "NodeA") 

##Get JPD for nodes "NodeA" and "NodeB" with Pearl inference 
##(nodes "NodeA" and "NodeB" must be from the same family): 

SetProperty(net, "Inference", "pearl")
pearlJPDNodeAFlt = GetJPDFloat(net, "NodeA NodeB") 
}
\keyword{GetJPDFloat}