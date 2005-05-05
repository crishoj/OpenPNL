\name{SaveNet}
\alias{SaveNet}
\title{Saves network to file}
\description{
The function saves network to file of "xml" format with given name. Net structure and nodes distributions of probabilities are stored to file.
 }
\usage{
SaveNet(Net, filename)
}
\arguments{
\item{Net}{Bayesian network, LIMID, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{filename}{String. File name.}
}
\examples{
SaveNet(net, "net.xml") 
}
\keyword{SaveNet}