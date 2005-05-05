\name{GenerateEvidences}
\alias{GenerateEvidences}
\title{Generate samples for the network }
\description{
The function generates the specified number of samples from joint probability distribution of the network. Samples may be generated using the current evidence (set by EditEvidence function) or ignoring the current evidence. The function allows generating samples with specified portion of missing variables. If current evidence is taken into account (ignoreCurrEvid == true) then sampled values of nodes present in evidence will be equal to values of current evidence. The sampled data can be generated so that there will be some portion of missing values - for every node user may specify the portion of non-missing values in the generated array. The newly generated samples will be added to internal buffer (learning buffer). To clear learning buffer use ClearEvidBuf. There is the difference between usage of this functions for Bayesian Networks and DBNs. 
}
\usage{
GenerateEvidences(BayesNet, nSamples, ignoreCurrEvid, whatNodes) 
GenerateEvidences(dbn, numSlices) 
}
\arguments{
\item{BayesNet}{Bayesian network or MRF, which has been already created in the workspace. The action is carried out on this network. }
\item{dbn}{DBN, which has been already created in the workspace. The action is carried out on this network. }
\item{nSamples}{Integer. Number of samples to generate.}
\item{ignoreCurrEvid}{Logistic. Flag of current evidence ignorance. If true - the current evidence will be ignored. Default value is false. }
\item{whatNodes}{String. Array of names of variables to generate and portion of observed data. Default empty array means that all samples will be generated.}
\item{numSlices}{String. Number of samples on every slice to generate.}
}
\examples{
##Generate 100 samples for all bayesian network nodes ignoring current evidence: 

GenerateEvidences(net,  100, true) 

##Generate 100 samples only for nodes "NodeA","NodeB" and "NodeC" using current evidence: 

GenerateEvidences(net, 100, false, "NodeA NodeB NodeC") 

##Generate 100 samples only for nodes "NodeA","NodeB" and "NodeC" using current evidence. "NodeA" should contain 90% of observed values, "NodeB" and "NodeC" should contain 100% of observed data: 

GenerateEvidences(net, 100, false, "NodeA^0.9 NodeB^1.0 NodeC^1.0") 

##Generate 100 samples for nodes "NodeA","NodeB" and "NodeC" using current evidence. All nodes should contain 90% of observed values: 

GenerateEvidences(net, 100, false, "NodeA^0.9 NodeB^0.9 NodeC^0.9 ") 

##Generate 100 samples for all nodes using current evidence. All nodes should contain 90% of observed values: 

GenerateEvidences(net, 100, false, whatNodesToSample, "0.9") 

##Generate 100 samples on 0 slice and 55 on 1 slice (for DBN): 

GenerateEvidences(dbn, "100 50") 

##Generate 10 samples on 0 slice, 11 on 1 slice and 15 on 2 slice (for DBN): 

GenerateEvidences(dbn, "10 11 15")

}
\keyword{GenerateEvidences}