\name{GetJPDString}
\alias{GetJPDString}
\title{Returns JPD as string}
\description{
The function returns joint probability distribution (JPD) for specified nodes using current evidence. The inference algorithm may be set to property "Inference" with method SetProperty(). By default Pearl inference is used. Besides Gibbs sampling, Junction Tree inference and Naive inference are available. Corresponding property "Inference" values are "pearl", "gibbs", "jtree", "naive". 
 }
\usage{
GetJPDString(Net, nodes)
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nodes}{String. Array of nodes from one family.}
}
\examples{
##Get JPD for node "NodeA" with Junction Tree inference: 

SetProperty(net, "Inference", "jtree")
jtreeJPDNodeAStr <- GetJPDString(net, "NodeA") 

##Get JPD for nodes "NodeA" and "NodeB" with Pearl inference 
##(nodes "NodeA" and "NodeB" must be from the same family): 

SetProperty(net, "Inference", "pearl")
pearlJPDNodeAStr <- GetJPDString(net, "NodeA NodeB") 
}
\keyword{GetJPDString}