#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"
PNLW_USING

BayesNet *GetSimpleTabularModelWithEntanglementNumeration()
{
    // One  Two
    //   |  |
    //   \/\/
    //   Zero
    BayesNet *net;
    net = new BayesNet();

    net->AddNode(discrete^"Zero One Two", "State1 State2");
    net->AddArc("One Two", "Zero");
    
    net->SetPTabular("One^State1", "0.5");
    net->SetPTabular("One^State2", "0.5");
    net->SetPTabular("Two^State1", "0.4");
    net->SetPTabular("Two^State2", "0.6");
    net->SetPTabular("Zero^State1","0.1" ,"One^State1 Two^State1");
    net->SetPTabular("Zero^State2","0.9" ,"One^State1 Two^State1");
    net->SetPTabular("Zero^State1","0.2" ,"One^State1 Two^State2");
    net->SetPTabular("Zero^State2","0.8" ,"One^State1 Two^State2");
    net->SetPTabular("Zero^State1","0.3" ,"One^State2 Two^State1");
    net->SetPTabular("Zero^State2","0.7" ,"One^State2 Two^State1");
    net->SetPTabular("Zero^State1","0.8" ,"One^State2 Two^State2");
    net->SetPTabular("Zero^State2","0.2" ,"One^State2 Two^State2");

    return net;
}

void TestForGetPTabular()
{
    BayesNet *net = GetSimpleTabularModelWithEntanglementNumeration();

    //TokArr BayesNet::GetPTabular(TokArr child, TokArr parents)
    TokArr Zero = net->GetPTabular("Zero");
    TokArr One = net->GetPTabular("One");
    TokArr Two = net->GetPTabular("Two");

    std::cout << Zero;

/*
    if( net->GetGaussianMean("NodeA")[0].FltValue() != 1.0f ||
        net->GetGaussianCovar("NodeA")[0].FltValue() != 10.0f ||
        net->GetGaussianMean("NodeB")[0].FltValue() != 0.5f ||
        net->GetGaussianCovar("NodeB")[0].FltValue() != 7.0f ||
        net->GetGaussianWeights("NodeB", "NodeA")[0].FltValue() != 0.1f ||
        net->GetGaussianMean("NodeC")[0].FltValue() != 0.8f ||
        net->GetGaussianCovar("NodeC")[0].FltValue() != 3.5f ||
        net->GetGaussianWeights("NodeC", "NodeB")[0].FltValue() != 0.4f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting gaussian parameters is wrong");
    }

    net->DelArc("NodeB", "NodeC");
    net->AddArc("NodeC", "NodeA");

    net->SetPGaussian("NodeA", "0.6", "12.0", "0.35");

    cout << String(net->GetGaussianWeights("NodeA", "NodeC")) << endl;
    if( net->GetGaussianMean("NodeA")[0].FltValue() != 0.6f ||
        net->GetGaussianCovar("NodeA")[0].FltValue() != 12.0f ||
        net->GetGaussianWeights("NodeA", "NodeC")[0].FltValue() != 0.35f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting gaussian parameters is wrong");
    }

    net->AddNode("NodeD", "dim1");
    net->AddArc("NodeC", "NodeD");
    net->AddArc("NodeD", "NodeA");

    net->SetPGaussian("NodeB", "0.12", "3.0", "0.21 0.9");

    if( net->GetGaussianMean("NodeB")[0].FltValue() != 0.12f ||
        net->GetGaussianCovar("NodeB")[0].FltValue() != 3.0f ||
        net->GetGaussianWeights("NodeB", "NodeA")[0].FltValue() != 0.21f ||
        net->GetGaussianWeights("NodeB", "NodeD")[0].FltValue() != 0.9f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting gaussian parameters is wrong");
    }

    net->DelNode("NodeA");

    if( net->GetGaussianMean("NodeB")[0].FltValue() != 1.0f ||
        net->GetGaussianCovar("NodeB")[0].FltValue() != 1.0f ||
        net->GetGaussianWeights("NodeB", "NodeD")[0].FltValue() != 0.0f )
    {
        PNL_THROW(pnl::CAlgorithmicException, "Setting or getting gaussian parameters is wrong");
    }

    cout << "TestGaussianModelCreate is completed successfully" << endl;
*/
}