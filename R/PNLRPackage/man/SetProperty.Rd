\name{SetProperty}
\alias{SetProperty}
\title{Returns value of the property with given name }
\description{
The function returns value of the property with given name. If property was not set with SetProperty method then empty string is returned. 
}
\usage{
GetProperty(Net, name)
}
\arguments{
\item{Net}{Bayesian network, LIMID, MRF or DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{name}{String. Name of property.}
}
\examples{
SetProperty(net, "Inference", "pearl")
inferenceType <- GetProperty(net, "Inference")
learningType <- GetProperty(net, "Learning") 

##As result string inferenceType is "pearl" and string learningType is empty
}
\keyword{SetProperty}