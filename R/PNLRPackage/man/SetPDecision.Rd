\name{SetPDecision}
\alias{SetPDecision}
\title{Assigns probability to one or several values of one decision node }
\description{
The function sets [conditional] probability for one or several values of the decision node [for given values of parents]. Sizes of values array and probabilities array must be the same. While distribution of probabilities is not specified it is considered uniform. 
 }
\usage{
SetPDecision(Limid, value, prob, parentValue) 
}
\arguments{
\item{Limid}{LIMID, which has been already created in the workspace. The action is carried out on this network. }
\item{value}{String. Array of node values for wich probabilities are set.}
\item{prob}{Either String or Float Vector. Array of values of probabilities.}
\item{parentValue}{String. Array of values of discrete parents of the node.}
}
\examples{
##Let's node "NodeA" has no parents and can take on two values: "true" and "false". 
##Its probability distribution is unconditional. 

SetPDecision(id, "NodeA^true NodeA^false", "0.7 0.3") 

##Let's node "NodeB" can take on values "true" and "false" too and has only one parent "NodeA". 
##Distribution of probabilities of "NodeB" is conditional. It is necessary to call SetPDecision 
##method for each configuration of parents values: 

Prob1 <- c(0.99, 0.01)
Prob2 <- c(0.4, 0.6)
SetPDecision(id, "NodeB^true NodeB^false", Prob1, "NodeA^true");
SetPDecision(id, "NodeB^true NodeB^false", Prob2, "NodeA^false"); 
}
\keyword{SetPDecision}