\name{SetNumSlices}
\alias{SetNumSlices}
\title{Sets number of slices in the network}
\description{
The function sets number of network slices.
 }
\usage{
SetNumSlices(DBN, nSlices)
}
\arguments{
\item{DBN}{DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nSlices}{Integer. Number of network slices.}
}
\examples{
##Setting 5 slices : 
SetNumSlices(dbn, 5) 
}
\keyword{SetNumSlices}