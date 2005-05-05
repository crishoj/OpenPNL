\name{LoadEvidBuf}
\alias{LoadEvidBuf}
\title{Loads evidence buffer content from file }
\description{
The function loads evidence buffer content from file. Buffer is cleared before loading. We can load evidences created in another net with another node names but with the same values of nodes. For this we must set new node names in evidences using array "columns". The function returns number of evidences loaded from file. 
}
\usage{
LoadEvidBuf(Net, filename, columns)
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{filename}{String. File name.}
\item{columns}{String. Names of nodes.}
}
\examples{
##Load evidences from file "evidence1.csv" with ',' separators and perform learning with them: 

LoadEvidBuf(net, "evidence1.csv")
LearnParameters(net) 

}
\keyword{LoadEvidBuf}