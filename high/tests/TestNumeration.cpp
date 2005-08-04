#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"
PNLW_USING

static char func_name[] = "testNumeration";

static char* test_desc = "Provide several tests on network numeration";

static char* test_class = "Algorithm";

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

BayesNet *GetSimpleTabularModelWithDeletedElement()
{
    // One  Two (deleted) Three
    //   |         |     |
    //   |         \/    |
    //   o------>Zero<---o
    BayesNet *net;
    net = new BayesNet();

    net->AddNode(discrete^"One Two Three Zero", "State1 State2");
    net->DelNode("Two");
    net->AddArc("One Three", "Zero");
    
    net->SetPTabular("One^State1", "0.5");
    net->SetPTabular("One^State2", "0.5");
    net->SetPTabular("Three^State1", "0.4");
    net->SetPTabular("Three^State2", "0.6");
    net->SetPTabular("Zero^State1","0.1" ,"One^State1 Three^State1");
    net->SetPTabular("Zero^State2","0.9" ,"One^State1 Three^State1");
    net->SetPTabular("Zero^State1","0.2" ,"One^State1 Three^State2");
    net->SetPTabular("Zero^State2","0.8" ,"One^State1 Three^State2");
    net->SetPTabular("Zero^State1","0.3" ,"One^State2 Three^State1");
    net->SetPTabular("Zero^State2","0.7" ,"One^State2 Three^State1");
    net->SetPTabular("Zero^State1","0.8" ,"One^State2 Three^State2");
    net->SetPTabular("Zero^State2","0.2" ,"One^State2 Three^State2");

    return net;
}

BayesNet *GetSimpleTabularModel()
{
    // Zero       One
    //   |         |
    //   |         |
    //   o->Three<-o
    BayesNet *net;
    net = new BayesNet();

    net->AddNode(discrete^"Zero One Three", "State1 State2");
    //net->DelNode("Two");
    net->AddArc("Zero One", "Three");
    
/*    net->SetPTabular("Zero^State1", "0.4");
    net->SetPTabular("Zero^State2", "0.6");
    net->SetPTabular("One^State1", "0.4");
    net->SetPTabular("One^State2", "0.6");
    net->SetPTabular("Three^State1","0.1" ,"Zero^State1 One^State1");
    net->SetPTabular("Three^State2","0.9" ,"Zero^State1 One^State1");
    net->SetPTabular("Three^State1","0.2" ,"Zero^State1 One^State2");
    net->SetPTabular("Three^State2","0.8" ,"Zero^State1 One^State2");
    net->SetPTabular("Three^State1","0.3" ,"Zero^State2 One^State1");
    net->SetPTabular("Three^State2","0.7" ,"Zero^State2 One^State1");
    net->SetPTabular("Three^State1","0.8" ,"Zero^State2 One^State2");
    net->SetPTabular("Three^State2","0.2" ,"Zero^State2 One^State2");
*/
    net->SetPTabular("Zero^State1 Zero^State2", "0.4 0.6");
    net->SetPTabular("One^State1 One^State2", "0.4 0.6");
    net->SetPTabular("Three^State1 Three^State2","0.1 0.9" ,"Zero^State1 One^State1");
    net->SetPTabular("Three^State1 Three^State2","0.2 0.8" ,"Zero^State1 One^State2");
    net->SetPTabular("Three^State1 Three^State2","0.3 0.7" ,"Zero^State2 One^State1");
    net->SetPTabular("Three^State1 Three^State2","0.8 0.2" ,"Zero^State2 One^State2");

    return net;
}

void TestForGetPTabular()
{
    BayesNet *net = GetSimpleTabularModelWithEntanglementNumeration();

    //TokArr BayesNet::GetPTabular(TokArr child, TokArr parents)
    TokArr Zero = net->GetPTabular("Zero");
    TokArr One = net->GetPTabular("One");
    TokArr Two = net->GetPTabular("Two");

    std::cout << Zero << "\n";
    std::cout << One << "\n";
    std::cout << Two << "\n";

    if ((Zero[0].FltValue() != 0.1f)||(Zero[1].FltValue() != 0.9f)||
	(Zero[2].FltValue() != 0.2f)||(Zero[3].FltValue() != 0.8f)||
	(Zero[4].FltValue() != 0.3f)||(Zero[5].FltValue() != 0.7f)||
	(Zero[6].FltValue() != 0.8f)||(Zero[7].FltValue() != 0.2f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is error in the function TestForGetPTabular 1");
    };

    if ((One[0].FltValue() != 0.5f)||(One[1].FltValue() != 0.5f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is error in the function TestForGetPTabular 2");
    };

    if ((Two[0].FltValue() != 0.4f)||(Two[1].FltValue() != 0.6f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is error in the function TestForGetPTabular 3");
    };

	delete net;
}

void TestForSetInferenceProperties()
{
    BayesNet *net = GetSimpleTabularModelWithEntanglementNumeration();

    net->SetProperty("Inference","gibbs");
    //net->SetProperty("GibbsNumberOfIterations","1000");
    //net->SetProperty("GibbsNumberOfStreams","2");
    //net->SetProperty("GibbsThresholdIteration","100");

    net->GetJPD("One");

	delete net; 
}

void TestForGetMPE()
{
    BayesNet *net = GetSimpleTabularModel();

    net->SetProperty("Inference", "naive");

    std::cout << net->GetJPD("Zero") << "\n";
    std::cout << net->GetJPD("One") << "\n";
    std::cout << net->GetJPD("Three") << "\n";
    std::cout << net->GetJPD("Zero One Three") << "\n";

    std::cout << net->GetMPE("Zero") << "\n";
    std::cout << net->GetMPE("One") << "\n";
    std::cout << net->GetMPE("Three") << "\n";
    std::cout << net->GetMPE("Zero One Three") << "\n";

	delete net;
};


void Test2Bnets()
{
    BayesNet *net1 = GetSimpleTabularModel();
    BayesNet *net2 = GetSimpleTabularModel();
	std::cout << "2Bnets" << "\n";
    delete net1;
    delete net2;
};

int testNumeration()
{
	int ret = TRS_OK;
	try 
    {
       // Test2Bnets(); // does not work
        TestForGetPTabular();
        TestForSetInferenceProperties();
        TestForGetMPE();
    }
    catch(pnl::CException e)
    {
        std::cout << e.GetMessage();
        ret = false;
    }
	/*
	Test2Bnets();
	TestForGetPTabular();
	TestForSetInferenceProperties();
	TestForGetMPE();*/
	return ret;

}

void initTestsNumeration()
{
    trsReg(func_name, test_desc, test_class, testNumeration);
}
