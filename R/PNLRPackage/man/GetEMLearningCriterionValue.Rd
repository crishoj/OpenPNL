\name{GetEMLearningCriterionValue}
\alias{GetEMLearningCriterionValue}
\title{Returns criterion value for last EM learning execution }
\description{
The function returns criterion value that was got during last EM learning execution. 
 }
\usage{
GetEMLearningCriterionValue(Net)
}
\arguments{
\item{Net}{Bayesian network or DBN, which has been already created in the workspace. The action is carried out on this network. }
}
\examples{
SetProperty(net, "Learning", "em")
LearnParameters(net)
resLogLik <- GetEMLearningCriterionValue(net)

}
\keyword{GetEMLearningCriterionValue}