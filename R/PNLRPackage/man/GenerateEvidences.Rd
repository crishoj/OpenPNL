\name{GenerateEvidences}
\alias{GenerateEvidences}
\title{Generate samples for the network }
\description{
The function generates the specified number of samples from joint probability distribution of the network. Samples may be generated using the current evidence (set by EditEvidence function) or ignoring the current evidence. The function allows generating samples with specified portion of missing variables. If current evidence is taken into account (ignoreCurrEvid == true) then sampled values of nodes present in evidence will be equal to values of current evidence. The sampled data can be generated so that there will be some portion of missing values - for every node user may specify the portion of non-missing values in the generated array. The newly generated samples will be added to internal buffer (learning buffer). To clear learning buffer use ClearEvidBuf. 
}
\usage{
GenerateEvidences(BayesNet, nSamples, ignoreCurrEvid, whatNodes) 
}
\arguments{
\item{BayesNet}{Bayesian network, which has been already created in the workspace. The action is carried out on this network. }
\item{nSamples}{Integer. Number of samples to generate.}
\item{ignoreCurrEvid}{Logistic. Flag of current evidence ignorance. If true - the current evidence will be ignored. Default value is false. }
\item{whatNodes}{String. Array of names of variables to generate and portion of observed data. Default empty array means that all samples will be generated.}
}
\examples{
##Generate 100 samples for all network nodes ignoring current evidence: 

GenerateEvidences(net,  100, true) 

##Generate 100 samples only for nodes "NodeA","NodeB" and "NodeC" using current evidence: 

GenerateEvidences(net, 100, false, "NodeA NodeB NodeC") 

##Generate 100 samples only for nodes "NodeA","NodeB" and "NodeC" using current evidence. "NodeA" should contain 90% of observed values, "NodeB" and "NodeC" should contain 100% of observed data: 

GenerateEvidences(net, 100, false, "NodeA^0.9 NodeB^1.0 NodeC^1.0") 

##Generate 100 samples for nodes "NodeA","NodeB" and "NodeC" using current evidence. All nodes should contain 90% of observed values: 

GenerateEvidences(net, 100, false, "NodeA^0.9 NodeB^0.9 NodeC^0.9 ") 

##Generate 100 samples for all nodes using current evidence. All nodes should contain 90% of observed values: 

GenerateEvidences(net, 100, false, whatNodesToSample, "0.9") 
}
\keyword{Bayesian Network}