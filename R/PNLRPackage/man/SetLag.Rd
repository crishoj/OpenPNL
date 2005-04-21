\name{SetLag}
\alias{SetLag}
\title{Sets the number of lag for FixLagSmoothing prosedure.}
\description{
The function sets lag for FixLagSmoothing prosedure.
 }
\usage{
SetLag(DBN, Lag)
}
\arguments{
\item{DBN}{DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{Lag}{The number of lag}
}
\examples{
##Setting the number of lag : 
SetLag(dbn, 3) 
}
\keyword{SetLag}