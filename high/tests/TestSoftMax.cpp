#include "test_conf.hpp"

int main(int argc, char* argv[])
{
    BayesNet net;
    net.AddNode("continuous^node0");
    net.AddNode("continuous^node1");
    net.AddNode("continuous^node2");
    net.AddNode("discrete^node5", "True False");

    net.AddArc("node0", "node5");
    net.AddArc("node1", "node5");
    net.AddArc("node2", "node5");
   
    net.SetPGaussian("node0", "0.1", "0.9");
    
    TokArr MN0 = net.GetGaussianMean("node0");
    TokArr CN0 = net.GetGaussianCovar("node0");
    cout<< "node0:\t"<<MN0 << "\t"<< CN0 << "\n";

    net.SetPGaussian("node1", "0.2", "0.8");
    net.SetPGaussian("node2", "0.3", "0.7");

    TokArr MN1 = net.GetGaussianMean("node1");
    TokArr CN1 = net.GetGaussianCovar("node1");
    cout<< "node1:\t"<<MN1 << "\t"<< CN1 << "\n";
    
    TokArr MN2 = net.GetGaussianMean("node2");
    TokArr CN2 = net.GetGaussianCovar("node2");
    cout<< "node2:\t"<<MN2 << "\t"<< CN2 << "\n";

    net.SetPSoftMax("node5", "0.3 0.4 0.5 0.6 0.7 0.8", "0.1 0.1");

    TokArr off = net.GetSoftMaxOffset("node5");
    TokArr w = net.GetSoftMaxWeights("node5");

    cout<< "SoftMax:\t"<<off << "\t"<<w << "\n";

    return 0;
}

