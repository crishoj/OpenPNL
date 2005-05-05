\name{SaveEvidBuf}
\alias{SaveEvidBuf}
\title{Saves evidence buffer content to file }
\description{
The function saves evidence buffer content to file of "cvs" format. The separating character is ','. 
}
\usage{
SaveEvidBuf(Net, filename) 
}
\arguments{
\item{Net}{Bayesian network, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{filename}{String. File name.}
}
\examples{
##Save two evidences to file with ',' separator: 

ClearEvidBuf(net)
AddEvidToBuf(net, "NodeA^true NodeB^false")
AddEvidToBuf(net, "NodeA^false NodeC^true")
SaveEvidBuf(net, "evidence1.csv") 


}
\keyword{SaveEvidBuf}