\name{SetValueCost}
\alias{SetValueCost}
\title{Assigns utility function to one value node }
\description{
The function sets utility function for value node for given values of parents. While utility function is not specified each cost value equals zero.  
}
\usage{
SetValueCost(Limid, value, prob, parentValue) 
}
\arguments{
\item{Limid}{LIMID, which has been already created in the workspace. The action is carried out on this network. }
\item{value}{String. Array of node values for wich costs are set.}
\item{prob}{Either String or Float Vector. Array of cost values.}
\item{parentValue}{String. Array of values of discrete parents of the node.}
}
\examples{
##Let node "NodeB" can take on value "cost" and has one parent "NodeA". 
##Let node "NodeA" can take on two values: "true" and "false". 
##To set utility function of "NodeB" it is necessary to call SetValueCost method for each configuration of parent values: 

SetValueCost(id, "NodeB^cost", "-1000.0", "NodeA^true")
SetValueCost(id, "NodeB^cost", "300.0", "NodeA^false") 

}
\keyword{SetValueCost}