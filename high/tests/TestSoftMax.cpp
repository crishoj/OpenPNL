#include "test_conf.hpp"

static char func_name[] = "testSoftMax";

static char* test_desc = "Provide several tests on networks with SoftMax nodes" ;

static char* test_class = "Algorithm";

BayesNet *SimpleSoftMaxModel()
{ 
    BayesNet *net;
    net = new BayesNet();
    
    net->AddNode(continuous^"node0 node1 node2");
    net->AddNode(discrete^"node5", "True False");

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

BayesNet *SevenNodesModel()
{ 
    BayesNet *net;
    net = new BayesNet();
    
    net->AddNode(continuous^"node0 node1 node5");
    net->AddNode(discrete^"node2 node3 node4 node6", "False True");

    net->AddArc("node0", "node3");
    net->AddArc("node1", "node3");
    net->AddArc("node2", "node3");
    net->AddArc("node3", "node4");
    net->AddArc("node3", "node5");
    net->AddArc("node0", "node6");
    net->AddArc("node4", "node6");
  
    net->SetPGaussian("node0", "0.5", "1.0");
    
    net->SetPGaussian("node1", "0.5", "1.0");
    
    net->SetPTabular("node2^True node2^False", "0.3 0.7");
    
    net->SetPSoftMax("node3^True node3^False", "0.5 0.4 0.5 0.7", "0.3 0.5", "node2^False");
    net->SetPSoftMax("node3^True node3^False", "0.5 0.1 0.5 0.7", "0.3 0.5 ", "node2^True");

    net->SetPTabular("node4^True node4^False", "0.8 0.2", "node3^True");
    net->SetPTabular("node4^True node4^False", "0.3 0.7", "node3^False");

    net->SetPGaussian("node5", "0.5", "0.5", "1.0", "node3^True");
    net->SetPGaussian("node5", "1.0", "1.0", "1.0", "node3^False");

    net->SetPSoftMax("node6^True node6^False", "0.8 0.2", "0.1 0.9", "node4^True");
    net->SetPSoftMax("node6^True node6^False", "0.5 0.9", "0.7 0.3", "node4^False");

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
	std::cout << "TestSetDistributionSoftMax is completed successfully" << std::endl;

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
	std::cout << "TestSetDistributionCondSoftMax is completed successfully" << std::endl;
}

void CrashTestJtreeInferenceSoftMax()
{
    BayesNet *net = SimpleSoftMaxModel();
    
    net->SetProperty("Inference", "jtree");
    TokArr jpd5 = net->GetJPD("node5");
	std::cout<< "jpd node5:\t"<<jpd5 << "\n";

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
	std::cout<< "jpd node5:\t"<<jpd5 << "\n";

    delete net;
	std::cout << "TestJtreeInferenceSoftMax1 is completed successfully" << std::endl;

}

void TestJtreeInferenceSoftMax2()
{
    BayesNet *net = SimpleSoftMaxModel();
    
    // all discrete nodes are observed
    net->EditEvidence("node5^True");
    net->EditEvidence("node1^0.2");
    
    net->SetProperty("Inference", "jtree");

    TokArr jpd0 = net->GetJPD("node0");
	std::cout<< "jpd node0:\t"<<jpd0 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
	std::cout<< "jpd node2:\t"<<jpd2 << "\n";

    delete net;
	std::cout << "TestJtreeInferenceSoftMax2 is completed successfully" << std::endl;

}


void TestGibbsInferenceSoftMax()
{
    BayesNet *net = SimpleSoftMaxModel();
    
    // no observed nodes
    net->SetProperty("Inference", "gibbs");

    TokArr jpd0 = net->GetJPD("node0");
	std::cout<< "jpd node0:\t"<<jpd0 << "\n";

    TokArr jpd1 = net->GetJPD("node1");
	std::cout<< "jpd node1:\t"<<jpd1 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
	std::cout<< "jpd node2:\t"<<jpd2 << "\n";

    TokArr jpd5 = net->GetJPD("node5");
	std::cout<< "jpd node5:\t"<<jpd5 << "\n";

    delete net;
	std::cout << "TestGibbsInferenceSoftMax is completed successfully" << std::endl;

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
	std::cout<< "jpd node3:\t"<<jpd3 << "\n";

    TokArr jpd5 = net->GetJPD("node5");
	std::cout<< "jpd node5:\t"<<jpd5 << "\n";
    
    TokArr jpd6 = net->GetJPD("node6");
	std::cout<< "jpd node6:\t"<<jpd6 << "\n";

    delete net;
	std::cout << "TestJtreeInferenceCondSoftMax1 is completed successfully" << std::endl;

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
	std::cout<< "jpd node0:\t"<<jpd0 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
	std::cout<< "jpd node2:\t"<<jpd2 << "\n";

    delete net;
	std::cout << "TestJtreeInferenceCondSoftMax2 is completed successfully" << std::endl;

}


void TestGibbsInferenceCondSoftMax()
{
    BayesNet *net = SimpleCondSoftMaxModel();
    
    // no observed nodes
    net->SetProperty("Inference", "gibbs");

    TokArr jpd0 = net->GetJPD("node0");
	std::cout<< "jpd node0:\t"<<jpd0 << "\n";


    TokArr jpd1 = net->GetJPD("node1");
	std::cout<< "jpd node1:\t"<<jpd1 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
	std::cout<< "jpd node2:\t"<<jpd2 << "\n";

    TokArr jpd5 = net->GetJPD("node5");
	std::cout<< "jpd node5:\t"<<jpd5 << "\n";

    delete net;
	std::cout << "TestGibbsInferenceCondSoftMax is completed successfully" << std::endl;

}


void TestSoftMaxParamLearning(bool DeleteNet)
{
//    BayesNet *net = SimpleSoftMaxModel();
    BayesNet *netToLearn = SimpleSoftMaxModel();
    float eps = 1e-1f;

    int nEvid = 100;
    netToLearn->GenerateEvidences(nEvid);

    netToLearn->LearnParameters();

    String nodes[] = {"node0", "node1", "node2"};

/*    int i, j;
    TokArr LearnParam, Param;
    for(i = 0; i < 3; i++)
    {
        LearnParam = netToLearn->GetGaussianMean(nodes[i]);
        Param = net->GetGaussianMean(nodes[i]);
        if(LearnParam[0].fload.size() != Param[0].fload.size())
        {
            PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
        }
        for(j = 0; j < LearnParam[0].fload.size(); j++)
        {
            if( LearnParam[0].FltValue(j).fl - Param[0].FltValue(j).fl > eps)
            {
                PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
            }
        }

        LearnParam = netToLearn->GetGaussianCovar(nodes[i]);
        Param = net->GetGaussianCovar(nodes[i]);
        if(LearnParam[0].fload.size() != Param[0].fload.size())
        {
            PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
        }
        for(j = 0; j < LearnParam[0].fload.size(); j++)
        {
            if( LearnParam[0].FltValue(j).fl - Param[0].FltValue(j).fl > eps)
            {
                PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
            }
        }
    }
*/
    if (DeleteNet)
    {
	delete netToLearn;
    };

	std::cout << "TestSoftMaxParamLearning is completed successfully" << std::endl;
}

void TestCondSoftMaxParamLearning(bool DeleteNet)
{
//    BayesNet *net = SimpleCondSoftMaxModel();
    BayesNet *netToLearn = SimpleCondSoftMaxModel();
    float eps = 1e-1f;

    int nEvid = 100;
    netToLearn->GenerateEvidences(nEvid);

    netToLearn->LearnParameters();

    String nodes[] = {"node0", "node1", "node2"};

/*    int i, j;
    TokArr LearnParam, Param;
    for(i = 0; i < 3; i++)
    {
        LearnParam = netToLearn->GetGaussianMean(nodes[i]);
        Param = net->GetGaussianMean(nodes[i]);
        if(LearnParam[0].fload.size() != Param[0].fload.size())
        {
            PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
        }
        for(j = 0; j < LearnParam[0].fload.size(); j++)
        {
            if( LearnParam[0].FltValue(j).fl - Param[0].FltValue(j).fl > eps)
            {
                PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
            }
        }

        LearnParam = netToLearn->GetGaussianCovar(nodes[i]);
        Param = net->GetGaussianCovar(nodes[i]);
        if(LearnParam[0].fload.size() != Param[0].fload.size())
        {
            PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
        }
        for(j = 0; j < LearnParam[0].fload.size(); j++)
        {
            if( LearnParam[0].FltValue(j).fl - Param[0].FltValue(j).fl > eps)
            {
                PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
            }
        }
    }
*/
    if (DeleteNet)
    {
	delete netToLearn;
    };

	std::cout << "TestCondSoftMaxParamLearning is completed successfully" << std::endl;
}

void TestSetDistributionSevenNodesModel()
{
    BayesNet *net = SevenNodesModel();
    if (net->GetGaussianMean("node0")[0].FltValue() != 0.5f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node0 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianMean("node1")[0].FltValue() != 0.5f)
    {
        PNL_THROW(pnl::CAlgorithmicException, "node1 : Setting or getting gaussian parameters is wrong");
    }
    
    if (net->GetGaussianCovar("node0")[0].FltValue() != 1.0f)
    {
        PNL_THROW(pnl::CAlgorithmicException, "node0 : Setting or getting gaussian parameters is wrong");
    }
    if (net->GetGaussianCovar("node1")[0].FltValue() != 1.0f)
    {
         PNL_THROW(pnl::CAlgorithmicException, "node1 : Setting or getting gaussian parameters is wrong");
    }

    float val12 = net->GetPTabular("node2")[0].FltValue();
    float val22 = net->GetPTabular("node2")[1].FltValue();

    if ((net->GetPTabular("node2")[0].FltValue() != 0.7f)||
        (net->GetPTabular("node2")[1].FltValue() != 0.3f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node2 : Setting or getting tabular parameters is wrong");
    };

    TokArr off5True = net->GetSoftMaxOffset("node3", "node2^True");
    
    if ((off5True[0].FltValue(0).fl != 0.3f)||
        (off5True[0].FltValue(1).fl != 0.5f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node3 : Setting or getting softmax parameters is wrong");
    };

    TokArr off5False = net->GetSoftMaxOffset("node3", "node2^False");

    float val1off = off5False[0].FltValue(0).fl;
    float val2off = off5False[0].FltValue(1).fl;
    if ((off5False[0].FltValue(0).fl != 0.3f)||
        (off5False[0].FltValue(1).fl != 0.5f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node3 : Setting or getting softmax parameters is wrong");
    };

    TokArr node5True = net->GetSoftMaxWeights("node3", "node2^True");
    
    if ((node5True[0].FltValue(0).fl != 0.5f)||
        (node5True[0].FltValue(1).fl != 0.1f)||
	(node5True[0].FltValue(2).fl != 0.5f)||
        (node5True[0].FltValue(3).fl != 0.7f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node3 : Setting or getting softmax parameters is wrong");
    };

    TokArr node5False = net->GetSoftMaxWeights("node3", "node2^False");
    float val0 = node5False[0].FltValue(0).fl;
    float val1 = node5False[0].FltValue(1).fl;
    float val2 = node5False[0].FltValue(2).fl;
    float val3 = node5False[0].FltValue(3).fl;

    if ((node5False[0].FltValue(0).fl != 0.5f)||
        (node5False[0].FltValue(1).fl != 0.4f)||
	(node5False[0].FltValue(2).fl != 0.5f)||
        (node5False[0].FltValue(3).fl != 0.7f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node3 : Setting or getting softmax parameters is wrong");
    };

    float val40 = net->GetPTabular("node4", "node3^False")[0].FltValue();
    float val41 = net->GetPTabular("node4", "node3^False")[1].FltValue();
    float val42 = net->GetPTabular("node4", "node3^True")[0].FltValue() ;
    float val43 = net->GetPTabular("node4", "node3^True")[1].FltValue() ;

    if ((net->GetPTabular("node4", "node3^False")[0].FltValue() != 0.7f)||
        (net->GetPTabular("node4", "node3^False")[1].FltValue() != 0.3f)||
        (net->GetPTabular("node4", "node3^True")[0].FltValue() != 0.2f)||
        (net->GetPTabular("node4", "node3^True")[1].FltValue() != 0.8f))
    {
        PNL_THROW(pnl::CAlgorithmicException, "node4 : Setting or getting tabular parameters is wrong");
    };

    if ((net->GetGaussianMean("node5", "node3^True")[0].FltValue() != 0.5f)||
        (net->GetGaussianMean("node5", "node3^False")[0].FltValue() != 1.0f))
    {
        PNL_THROW(pnl::CAlgorithmicException, "node5 : Setting or getting gaussian parameters is wrong");
    }
    
    if ((net->GetGaussianCovar("node5", "node3^True")[0].FltValue() != 0.5f)||
        (net->GetGaussianCovar("node5", "node3^False")[0].FltValue() != 1.0f))
    {
        PNL_THROW(pnl::CAlgorithmicException, "node5 : Setting or getting gaussian parameters is wrong");
    }
    
    TokArr off6True = net->GetSoftMaxOffset("node6", "node4^True");
    
    if ((off6True[0].FltValue(0).fl != 0.1f)||
        (off6True[0].FltValue(1).fl != 0.9f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node6 : Setting or getting softmax parameters is wrong");
    };
    
    TokArr off6False = net->GetSoftMaxOffset("node6", "node4^False");
    
    if ((off6False[0].FltValue(0).fl != 0.7f)||
        (off6False[0].FltValue(1).fl != 0.3f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node6 : Setting or getting softmax parameters is wrong");
    };


    TokArr node6True = net->GetSoftMaxWeights("node6", "node4^True");
    
    if ((node6True[0].FltValue(0).fl != 0.8f)||
        (node6True[0].FltValue(1).fl != 0.2f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node6 : Setting or getting softmax parameters is wrong");
    };

    TokArr node6False = net->GetSoftMaxWeights("node6", "node4^False");
    float val06 = node5False[0].FltValue(0).fl;
    float val16 = node5False[0].FltValue(1).fl;

    if ((node6False[0].FltValue(0).fl != 0.5f)||
        (node6False[0].FltValue(1).fl != 0.9f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "node6 : Setting or getting softmax parameters is wrong");
    };

    delete net;
	std::cout << "TestSetDistributionSevenNodesModel is completed successfully" << std::endl;

}


void TestJtreeInference1SevenNodesModel()
{
    BayesNet *net = SevenNodesModel();
    
    // all continuous nodes are observed
    net->EditEvidence("node0^0.3");
    net->EditEvidence("node1^0.2");
    net->EditEvidence("node5^0.9");

    net->EditEvidence("node4^True");

    net->SetProperty("Inference", "jtree");
    TokArr jpd3 = net->GetJPD("node3");
	std::cout<< "jpd node3:\t"<<jpd3 << "\n";

    TokArr jpd6 = net->GetJPD("node6");
	std::cout<< "jpd node6:\t"<<jpd6 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
	std::cout<< "jpd node2:\t"<<jpd2 << "\n";
    
    delete net;
	std::cout << "TestJtreeInference1SevenNodesModel is completed successfully" << std::endl;

}

void TestJtreeInference2SevenNodesModel()
{
    BayesNet *net = SevenNodesModel();
    
    // all discrete nodes are observed
    net->EditEvidence("node2^True");
    net->EditEvidence("node3^False");
    net->EditEvidence("node4^False");
    net->EditEvidence("node6^True");
    
    net->EditEvidence("node1^0.55");

    net->SetProperty("Inference", "jtree");

    TokArr jpd0 = net->GetJPD("node0");
	std::cout<< "jpd node0:\t"<<jpd0 << "\n";

    TokArr jpd5 = net->GetJPD("node1");
	std::cout<< "jpd node5:\t"<<jpd5 << "\n";

    delete net;
	std::cout << "TestJtreeInference2SevenNodesModel is completed successfully" << std::endl;

}

void TestDelArc()
{
    BayesNet *net = SevenNodesModel();
    
    net->DelArc("node0", "node6");
  
    net->SetPTabular("node6^True node6^False", "0.2 0.8", "node4^True");
    net->SetPTabular("node6^True node6^False", "0.1 0.9", "node4^False");

    // all continuous nodes are observed
    net->EditEvidence("node0^0.3");
    net->EditEvidence("node1^0.2");
    net->EditEvidence("node5^0.9");

    net->EditEvidence("node4^True");

    net->SetProperty("Inference", "jtree");
    TokArr jpd3 = net->GetJPD("node3");
	std::cout<< "jpd node3:\t"<<jpd3 << "\n";

    TokArr jpd6 = net->GetJPD("node6");
	std::cout<< "jpd node6:\t"<<jpd6 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
	std::cout<< "jpd node2:\t"<<jpd2 << "\n";
    
    delete net;

	std::cout << "TestDelArc is completed successfully" << std::endl;

}

void TestAddArc()
{
    BayesNet *net = SevenNodesModel();
    
    net->AddArc("node2", "node5");
  
//    net->SetPTabular("node6^True node6^False", "0.2 0.8", "node4^True");
//    net->SetPTabular("node6^True node6^False", "0.1 0.9", "node4^False");

    // all continuous nodes are observed
    net->EditEvidence("node0^0.3");
    net->EditEvidence("node1^0.2");
    net->EditEvidence("node5^0.9");

    net->EditEvidence("node4^True");

    net->SetProperty("Inference", "jtree");
    TokArr jpd3 = net->GetJPD("node3");
	std::cout<< "jpd node3:\t"<<jpd3 << "\n";

    TokArr jpd6 = net->GetJPD("node6");
	std::cout<< "jpd node6:\t"<<jpd6 << "\n";

    TokArr jpd2 = net->GetJPD("node2");
	std::cout<< "jpd node2:\t"<<jpd2 << "\n";
    
    delete net;

	std::cout << "TestDelArc is completed successfully" << std::endl;

}

void TestDelNode()
{
    BayesNet *net = SevenNodesModel();
    
    net->DelNode("node2");
  
    // all continuous nodes are observed
    net->EditEvidence("node0^0.3");
    net->EditEvidence("node1^0.2");
    net->EditEvidence("node5^0.9");

    net->EditEvidence("node4^True");

    net->SetProperty("Inference", "jtree");
    TokArr jpd3 = net->GetJPD("node3");
	std::cout<< "jpd node3:\t"<<jpd3 << "\n";

    TokArr jpd6 = net->GetJPD("node6");
	std::cout<< "jpd node6:\t"<<jpd6 << "\n";

    delete net;

	std::cout << "TestDelArc is completed successfully" << std::endl;

}

void TestNodeTypes()
{
   BayesNet *net = SimpleCondSoftMaxModel();

   TokArr n0t = net->GetNodeType("node0");
   TokArr n1t = net->GetNodeType("node1");
   TokArr n2t = net->GetNodeType("node2");
   TokArr n3t = net->GetNodeType("node3");
   TokArr n5t = net->GetNodeType("node5");
   TokArr n6t = net->GetNodeType("node6");

   printf("\nNodes types\n");
   printf("Node 0 type: %s\n",  String(n0t).c_str());
   printf("Node 1 type: %s\n",String(n1t).c_str());
   printf("Node 2 type: %s\n",String(n2t).c_str());
   printf("Node 3 type: %s\n",String(n3t).c_str());
   printf("Node 5 type: %s\n",String(n5t).c_str());
   printf("Node 6 type: %s\n",String(n6t).c_str());

   delete net;
}

int testSoftMax()
{
	
	int res = TRS_OK;
	TestNodeTypes();
	TestSoftMaxParamLearning();
	TestSetDistributionSoftMax();
	TestJtreeInferenceSoftMax1();
	TestJtreeInferenceSoftMax2();
	TestGibbsInferenceSoftMax();
	return res;
}

void initTestsSoftMax()
{
    trsReg(func_name, test_desc, test_class, testSoftMax);
}