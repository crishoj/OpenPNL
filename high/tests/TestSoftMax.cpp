#include "test_conf.hpp"

BayesNet *SimpleSoftMaxModel()
{ 
    BayesNet *net;
    net = new BayesNet();
    
    net->AddNode("continuous^node0");
    net->AddNode("continuous^node1");
    net->AddNode("continuous^node2");
    net->AddNode("discrete^node5", "True False");

    net->AddArc("node0", "node5");
    net->AddArc("node1", "node5");
    net->AddArc("node2", "node5");
  
    net->SetPGaussian("node0", "0.1", "0.9");
    net->SetPGaussian("node1", "0.2", "0.8");
    net->SetPGaussian("node2", "0.3", "0.7");

    net->SetPSoftMax("node5^True node5^False", "0.3 0.4 0.5 0.6 0.7 0.8", "0.1 0.1");
    return net;
}


BayesNet *SimpleCondSoftMaxModel()
{ 
    BayesNet *net;
    net = new BayesNet();
    
    net->AddNode("continuous^node0");
    net->AddNode("continuous^node1");
    net->AddNode("continuous^node2");
    net->AddNode("discrete^node3", "True False"); 

    net->AddNode("discrete^node5", "True False");// condsoftmax node
    net->AddNode("discrete^node6", "True False");

    net->AddArc("node0", "node5");
    net->AddArc("node1", "node5");
    net->AddArc("node2", "node5");
    net->AddArc("node3", "node5");
    net->AddArc("node5", "node6");

    net->SetPGaussian("node0", "0.1", "0.9");
    net->SetPGaussian("node1", "0.2", "0.8");
    net->SetPGaussian("node2", "0.3", "0.7");
  
    net->SetPTabular("node6^True node6^False", "0.3 0.7", "node5^True");
    net->SetPTabular("node6^True node6^False", "0.5 0.5", "node5^False");

    net->SetPSoftMax("node5^True node5^False", "0.3 0.4 0.5 0.6 0.7 0.8", "0.1 0.1", "node3^True");
    net->SetPSoftMax("node5^True node5^False", "0.23 0.24 0.25 0.26 0.27 0.28", "0.21 0.21", "node3^False");

    return net;
}

void TestSetDistributionSoftMax()
{
    BayesNet *net = SimpleSoftMaxModel();

    if (net->GetGaussianMean("node0")[0].FltValue() != 0.1f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node0 : Setting or getting gaussian parameters is wrong");
    }
        if (net->GetGaussianMean("node1")[0].FltValue() != 0.2f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node1 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianMean("node2")[0].FltValue() != 0.3f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node2 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianCovar("node0")[0].FltValue() != 0.9f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node0 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianCovar("node1")[0].FltValue() != 0.8f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node1 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianCovar("node2")[0].FltValue() != 0.7f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node2 : Setting or getting gaussian parameters is wrong");
    }

    if ((net->GetSoftMaxOffset("node5")[0].FltValue(0).fl != 0.1f)||
        (net->GetSoftMaxOffset("node5")[0].FltValue(1).fl != 0.1f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node5 : Setting or getting gaussian parameters is wrong");
    };

    TokArr node5= net->GetSoftMaxWeights("node5");
    float val0 = node5[0].FltValue(0).fl;
    float val1 = node5[0].FltValue(1).fl;
    float val2 = node5[0].FltValue(2).fl;
    float val3 = node5[0].FltValue(3).fl;
    float val4 = node5[0].FltValue(4).fl;
    float val5 = node5[0].FltValue(5).fl;

    if ((node5[0].FltValue(0).fl != 0.3f)||
        (node5[0].FltValue(1).fl != 0.4f)||
	(node5[0].FltValue(2).fl != 0.5f)||
        (node5[0].FltValue(3).fl != 0.6f)||
        (node5[0].FltValue(4).fl != 0.7f)||
        (node5[0].FltValue(5).fl != 0.8f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node5 : Setting or getting gaussian parameters is wrong");
    };

    delete net;
}

void TestSetDistributionCondSoftMax()
{
    BayesNet *net = SimpleCondSoftMaxModel();
    if (net->GetGaussianMean("node0")[0].FltValue() != 0.1f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node0 : Setting or getting gaussian parameters is wrong");
    }
        if (net->GetGaussianMean("node1")[0].FltValue() != 0.2f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node1 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianMean("node2")[0].FltValue() != 0.3f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node2 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianCovar("node0")[0].FltValue() != 0.9f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node0 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianCovar("node1")[0].FltValue() != 0.8f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node1 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianCovar("node2")[0].FltValue() != 0.7f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node2 : Setting or getting gaussian parameters is wrong");
    }

    if ((net->GetPTabular("node6")[0].FltValue() != 0.3f)||
        (net->GetPTabular("node6")[1].FltValue() != 0.7f)||
	(net->GetPTabular("node6")[2].FltValue() != 0.5f)||
        (net->GetPTabular("node6")[3].FltValue() != 0.5f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node6 : Setting or getting gaussian parameters is wrong");
    };

    TokArr off5True = net->GetSoftMaxOffset("node5", "node3^True");
    
    if ((off5True[0].FltValue(0).fl != 0.1f)||
        (off5True[0].FltValue(1).fl != 0.1f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node5 : Setting or getting gaussian parameters is wrong");
    };

    TokArr off5False = net->GetSoftMaxOffset("node5", "node3^False");

    float val1off = off5False[0].FltValue(0).fl;
    float val2off = off5False[0].FltValue(1).fl;
    if ((off5False[0].FltValue(0).fl != 0.21f)||
        (off5False[0].FltValue(1).fl != 0.21f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node5 : Setting or getting gaussian parameters is wrong");
    };

    TokArr node5True = net->GetSoftMaxWeights("node5", "node3^True");
    
    if ((node5True[0].FltValue(0).fl != 0.3f)||
        (node5True[0].FltValue(1).fl != 0.4f)||
	(node5True[0].FltValue(2).fl != 0.5f)||
        (node5True[0].FltValue(3).fl != 0.6f)||
        (node5True[0].FltValue(4).fl != 0.7f)||
        (node5True[0].FltValue(5).fl != 0.8f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node5 : Setting or getting gaussian parameters is wrong");
    };

    TokArr node5False = net->GetSoftMaxWeights("node5", "node3^False");
    float val0 = node5False[0].FltValue(0).fl;
    float val1 = node5False[0].FltValue(1).fl;
    float val2 = node5False[0].FltValue(2).fl;
    float val3 = node5False[0].FltValue(3).fl;
    float val4 = node5False[0].FltValue(4).fl;
    float val5 = node5False[0].FltValue(5).fl;

    if ((node5False[0].FltValue(0).fl != 0.23f)||
        (node5False[0].FltValue(1).fl != 0.24f)||
	(node5False[0].FltValue(2).fl != 0.25f)||
        (node5False[0].FltValue(3).fl != 0.26f)||
        (node5False[0].FltValue(4).fl != 0.27f)||
        (node5False[0].FltValue(5).fl != 0.28f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node5 : Setting or getting gaussian parameters is wrong");
    };

    delete net;
}

void CrashTestJtreeInferenceSoftMax()
{
    BayesNet *net = SimpleSoftMaxModel();
    
    net->SetProperty("Inference", "jtree");
    TokArr jpd5 = net->GetJPD("node5");
    cout<< "jpd node5:\t"<<jpd5 << "\n";

    delete net;
}

void TestJtreeInferenceSoftMax1()
{
    BayesNet *net = SimpleSoftMaxModel();
    
    // all continuous nodes are observed
    net->EditEvidence("node0^0.3");
    net->EditEvidence("node1^0.2");
    net->EditEvidence("node2^0.9");
    
    net->SetProperty("Inference", "jtree");
    TokArr jpd5 = net->GetJPD("node5");
    cout<< "jpd node5:\t"<<jpd5 << "\n";

    delete net;
}

void TestJtreeInferenceSoftMax2()
{
    BayesNet *net = SimpleSoftMaxModel();
    
    // all discrete nodes are observed
    net->EditEvidence("node5^True");
    net->EditEvidence("node1^0.2");
    
    net->SetProperty("Inference", "jtree");

    TokArr jpd0 = net->GetJPD("node0");
    cout<< "jpd node0:\t"<<jpd0 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
    cout<< "jpd node2:\t"<<jpd2 << "\n";

    delete net;
}


void TestGibbsInferenceSoftMax()
{
    BayesNet *net = SimpleSoftMaxModel();
    
    // no observed nodes
    net->SetProperty("Inference", "gibbs");

    TokArr jpd0 = net->GetJPD("node0");
    cout<< "jpd node0:\t"<<jpd0 << "\n";

    // gibbs inference crash
/*    TokArr jpd1 = net->GetJPD("node1");
    cout<< "jpd node1:\t"<<jpd1 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
    cout<< "jpd node2:\t"<<jpd2 << "\n";

    TokArr jpd5 = net->GetJPD("node5");
    cout<< "jpd node5:\t"<<jpd5 << "\n";
*/
    delete net;
}


void TestJtreeInferenceCondSoftMax1()
{
    BayesNet *net = SimpleCondSoftMaxModel();
    
    // all continuous nodes are observed
    net->EditEvidence("node0^0.3");
    net->EditEvidence("node1^0.2");
    net->EditEvidence("node2^0.9");
    
    net->SetProperty("Inference", "jtree");

    TokArr jpd3 = net->GetJPD("node3");
    cout<< "jpd node3:\t"<<jpd3 << "\n";

    TokArr jpd5 = net->GetJPD("node5");
    cout<< "jpd node5:\t"<<jpd5 << "\n";
    
    TokArr jpd6 = net->GetJPD("node6");
    cout<< "jpd node6:\t"<<jpd6 << "\n";

    delete net;
}

void TestJtreeInferenceCondSoftMax2()
{
    BayesNet *net = SimpleCondSoftMaxModel();
    
    // all discrete nodes are observed
    net->EditEvidence("node3^False");
    net->EditEvidence("node6^True");
    net->EditEvidence("node5^True");
    net->EditEvidence("node1^0.2");
    
    net->SetProperty("Inference", "jtree");

    TokArr jpd0 = net->GetJPD("node0");
    cout<< "jpd node0:\t"<<jpd0 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
    cout<< "jpd node2:\t"<<jpd2 << "\n";

    delete net;
}


void TestGibbsInferenceCondSoftMax()
{
    BayesNet *net = SimpleCondSoftMaxModel();
    
    // no observed nodes
    net->SetProperty("Inference", "gibbs");

    TokArr jpd0 = net->GetJPD("node0");
    cout<< "jpd node0:\t"<<jpd0 << "\n";

    // gibbs inference crash
/*    TokArr jpd1 = net->GetJPD("node1");
    cout<< "jpd node1:\t"<<jpd1 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
    cout<< "jpd node2:\t"<<jpd2 << "\n";

    TokArr jpd5 = net->GetJPD("node5");
    cout<< "jpd node5:\t"<<jpd5 << "\n";
*/
    delete net;
}


void TestSoftMaxParamLearning(bool DeleteNet)
{
    BayesNet *net = SimpleSoftMaxModel();
    float eps = 1e-1f;

    int nEvid = 50;
    net->GenerateEvidences(nEvid);

    net->LearnParameters();

    if (DeleteNet)
    {
	delete net;
    };
    
    cout << "TestGaussianParamLearning is completed successfully" << endl;
}