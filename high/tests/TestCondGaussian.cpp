#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"
#include "WDistribFun.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWDistributions.hpp"
#include "pnlMatrix.hpp"
#include "TokenCover.hpp"

PNLW_USING

static char func_name[] = "testCondGaussian";

static char* test_desc = "Provide all tests for networks with conditinal Gaussian nodes";

static char* test_class = "Algorithm";

BayesNet *VerySimpleGaussianModel()
{
    // NodeA -> NodeB -> NodeC
    BayesNet *net;
    net = new BayesNet();
    
    net->AddNode(continuous^"NodeB NodeA NodeC", "dim1");
    //    net->AddNode(continuous^"NodeA NodeB NodeC", "dim1");
    net->AddArc("NodeA", "NodeB");
    net->AddArc("NodeB", "NodeC");
    
    net->SetPGaussian("NodeA", "1.0", "10.0");
    net->SetPGaussian("NodeB", "0.5", "7.0", "0.1");
    net->SetPGaussian("NodeC", "0.8", "3.5", "0.4");
    
    return net;
}

BayesNet *VerySimpleMultivariateGaussianModel()
{
    // NodeA -> NodeB -> NodeC
    BayesNet *net;
    net = new BayesNet();
    
    net->AddNode(continuous^"NodeB NodeA NodeC", "dim1 dim2");
    //    net->AddNode(continuous^"NodeA NodeB NodeC", "dim1");
    net->AddArc("NodeA", "NodeB");
    net->AddArc("NodeB", "NodeC");
    
    net->SetPGaussian("NodeA", "1.0 2.0", "10.0 0.1 0.1 10.0");
    net->SetPGaussian("NodeB", "0.5 1.5", "7.0 0.0 0.0 7.0", "0.1 0.2 0.3 0.4");
    net->SetPGaussian("NodeC", "0.8 1.8", "3.5 0.5 0.5 3.5", "0.5 0.6 0.7 0.8");
    
    return net;
}


void TestsPnlHigh::TestDesc()
{
    printf("TestDesc\n");

    BayesNet *net;
    net = GetSimpleTabularModel();
    delete net;
    
    net = VerySimpleGaussianModel();
    delete net;
}

BayesNet *TestsPnlHigh::CreateCondGaussianModel1()
{
    // Cont1  Tab1
    //    |   |
    //    \/ \/
    //    Cont2
    BayesNet *net;
    net = new BayesNet();
    
    net->AddNode(continuous^"Cont1 Cont2", "dim1"); 
    net->AddNode(discrete^"Tab1", "dim1 dim2");
    
    net->AddArc("Cont1 Tab1", "Cont2");
    
    return net;
}

void TestsPnlHigh::TestNTabNCont()
{
    printf("TestNTabNCont\n");

    BayesNet *net = CreateCondGaussianModel1();
    DistribFunDesc *pDesc2 = net->m_pNet->Distributions().Distribution(2)->desc();
    DistribFunDesc *pDesc1 = net->m_pNet->Distributions().Distribution(1)->desc();
    DistribFunDesc *pDesc0 = net->m_pNet->Distributions().Distribution(0)->desc();
    
    int indexT;
    int indexC0;
    int indexC1;
    int value;
    
    Tok t("Tab1^dim1"), c1("Cont1^dim1"), c2("Cont2^dim1");
    t.Resolve();
    c1.Resolve();
    c2.Resolve();
    
    pDesc1->getIndexAndValue(&indexT, &value, t);
    pDesc1->getIndexAndValue(&indexC0, &value, c1);
    pDesc1->getIndexAndValue(&indexC1, &value, c2);
    
    if (pDesc1->isTabular(indexC1)||pDesc1->isTabular(indexC0)||!pDesc1->isTabular(indexT))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is error in the function TestNTabNCont");
    };
    
    delete net;
}

BayesNet *TestsPnlHigh::SimpleCGM1()
{
    //Cont0(3) Tab0(0)
    //  |      |
    //  \/    \/
    //   Cont1(1)
    //    |
    //    \/
    //   Cont2(2)
    BayesNet *net = new BayesNet();
    
    net->AddNode(discrete^"Tab0", "State0 State1");
    net->AddNode(continuous^"Cont1 Cont2", "Dim0");
    net->AddNode(continuous^"Cont0", "Dim0 Dim1");
    
    net->AddArc("Cont0 Tab0", "Cont1");
    net->AddArc("Cont1", "Cont2");
    
    return net;
};

void TestsPnlHigh::TestDefaultDistribution()
{
    printf("TestDefaultDistribution\n");

    BayesNet *net = SimpleCGM1();
    
    //1 is equal to "Cont1"
    WCondGaussianDistribFun *pCGDF = dynamic_cast<WCondGaussianDistribFun *>(net->m_pNet->m_paDistribution->Distribution(1));
    //pCGDF->m_pDistrib->Dump();
    
    Vector<int> aIndex(pCGDF->desc()->nNode()+3, 0);
    aIndex[0] = 0;
    aIndex[4] = 0;
    aIndex[5] = 0;
    
    if (pCGDF->GetAValue(pnl::matMean, aIndex) != 1.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    };
    
    if (pCGDF->GetAValue(pnl::matCovariance, aIndex) != 1.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    };
    
    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    };
    
    aIndex[5] = 1;
    
    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    };
    
    aIndex[0] = 1;
    
    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    };
    
    aIndex[5] = 0;
    
    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    };
    
    //WGaussianDistribFun *pGDF = dynamic_cast<WGaussianDistribFun *>(net->m_pNet->m_paDistribution->Distribution(2));
    //pGDF->m_pDistrib->Dump();
    
    delete net;
};

void TestsPnlHigh::TestCondGaussianFillData()
{
    printf("TestCondGaussianFillData\n");

    BayesNet *net = SimpleCGM1();
    
    WCondGaussianDistribFun *pCGDF = dynamic_cast<WCondGaussianDistribFun *>(net->m_pNet->m_paDistribution->Distribution(1));
    
    int IndexTab0;
    int ValueTab0;
    Tok t("Tab0^State0");
    pCGDF->desc()->getIndexAndValue(&IndexTab0, &ValueTab0, t);
    
    //void WDistribFun::FillData(int matrixId, TokArr value, TokArr probability, TokArr parentValue)
    pCGDF->FillData(pnl::matMean, "Cont1", "0.0", "Tab0^State0");
    pCGDF->FillData(pnl::matMean, "Cont1", "-1.5", "Tab0^State1");
    pCGDF->FillData(pnl::matCovariance, "Cont1", "2.5", "Tab0^State0");
    pCGDF->FillData(pnl::matCovariance, "Cont1", "0.75", "Tab0^State1");
    pCGDF->FillData(pnl::matWeights, "Cont1", "1.0 3.0", "Tab0^State0");
    pCGDF->FillData(pnl::matWeights, "Cont1", "0.5 2.5", "Tab0^State1");
    
    Vector<int> aIndex(pCGDF->desc()->nNode()+3, 0);
    aIndex[IndexTab0] = 0;
    aIndex[4] = 0;
    aIndex[5] = 0;
    
    if ((pCGDF->GetAValue(pnl::matMean, aIndex) != 0.0f)||
	(pCGDF->GetAValue(pnl::matCovariance, aIndex) != 2.5f)||
	(pCGDF->GetAValue(pnl::matWeights, aIndex) != 1.0f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    };
    
    aIndex[5] = 1;
    
    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 3.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    };
    
    aIndex[IndexTab0] = 1;
    aIndex[5] = 0;
    
    if ((pCGDF->GetAValue(pnl::matMean, aIndex) != -1.5f)||
	(pCGDF->GetAValue(pnl::matCovariance, aIndex) != 0.75f)||
	(pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.5f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    };
    
    aIndex[5] = 1;
    
    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 2.5f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    };
    
    delete net;
}

void TestsPnlHigh::TestSetPGaussian()
{
    printf("TestSetPGaussian\n");

    BayesNet *net = SimpleCGM1();
    
    net->SetPGaussian("Cont1", "0.0", "2.5", "1.0 3.0", "Tab0^State0");
    net->SetPGaussian("Cont1", "-1.5", "0.75", "0.5 2.5", "Tab0^State1");
    
    WCondGaussianDistribFun *pCGDF = dynamic_cast<WCondGaussianDistribFun *>(net->m_pNet->m_paDistribution->Distribution(1));
    
    int IndexTab0;
    int ValueTab0;
    Tok t("Tab0^State0");
    pCGDF->desc()->getIndexAndValue(&IndexTab0, &ValueTab0, t);
    
    Vector<int> aIndex(pCGDF->desc()->nNode()+3, 0);
    aIndex[IndexTab0] = 0;
    aIndex[4] = 0;
    aIndex[5] = 0;
    
    if ((pCGDF->GetAValue(pnl::matMean, aIndex) != 0.0f)||
	(pCGDF->GetAValue(pnl::matCovariance, aIndex) != 2.5f)||
	(pCGDF->GetAValue(pnl::matWeights, aIndex) != 1.0f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    };
    
    aIndex[5] = 1;
    
    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 3.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    };
    
    aIndex[IndexTab0] = 1;
    aIndex[5] = 0;
    
    if ((pCGDF->GetAValue(pnl::matMean, aIndex) != -1.5f)||
	(pCGDF->GetAValue(pnl::matCovariance, aIndex) != 0.75f)||
	(pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.5f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    };
    
    aIndex[5] = 1;
    
    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 2.5f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    };
    
    delete net;
};

void TestsPnlHigh::TestGetDiscreteParentValuesIndexes()
{
    printf("TestGetDiscreteParentValuesIndexes\n");

    BayesNet *net = SimpleCGM1();
    
    net->SetPGaussian("Cont1", "0.0", "2.5", "1.0 3.0", "Tab0^State0");
    net->SetPGaussian("Cont1", "-1.5", "0.75", "0.5 2.5", "Tab0^State1");
    
    WCondGaussianDistribFun *pCGDF = dynamic_cast<WCondGaussianDistribFun *>(net->m_pNet->m_paDistribution->Distribution(1));
    
    TokArr ta("Tab0^State0");
    Vector<int> dpInd = pCGDF->GetDiscreteParentValuesIndexes(ta);
    
    if (dpInd.size()!=1)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Size of dpInd is wrong");
    };
    
    if (dpInd[0] != 0)
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetDiscreteParentValuesIndexes works incorrectly");
    };
    
    TokArr ta2("Tab0^State1");
    dpInd = pCGDF->GetDiscreteParentValuesIndexes(ta2);
    
    if (dpInd[0] != 1)
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetDiscreteParentValuesIndexes works incorrectly");
    };
    
    delete net;
};

void TestsPnlHigh::TestGetGaussianMeanCovarWeights()
{
    printf("TestGetGaussianMeanCovarWeights\n");
    
    BayesNet *net = SimpleCGM1();
    
    net->SetPGaussian("Cont1", "0.0", "2.5", "1.0 3.0", "Tab0^State0");
    net->SetPGaussian("Cont1", "-1.5", "0.75", "0.5 2.5", "Tab0^State1");
    
    if (net->GetGaussianMean("Cont1", "Tab0^State0")[0].FltValue() != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetGaussianMean works incorrectly");
    };
    
    if (net->GetGaussianMean("Cont1", "Tab0^State1")[0].FltValue() != -1.5f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetGaussianMean works incorrectly");
    };
    
    if (net->GetGaussianCovar("Cont1", "Tab0^State0")[0].FltValue() != 2.5f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetGaussianCovar works incorrectly");
    };
    
    if (net->GetGaussianCovar("Cont1", "Tab0^State1")[0].FltValue() != 0.75f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetGaussianCovar works incorrectly");
    };  
    
    if (String(net->GetGaussianWeights("Cont1", "Cont0", "Tab0^State0")[0]) != "1.000000^3.000000")
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetGaussianCovar works incorrectly");
    }; 
    
    if (String(net->GetGaussianWeights("Cont1", "Cont0", "Tab0^State1")[0]) != "0.500000^2.500000")
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetGaussianCovar works incorrectly");
    }; 
    
    delete net;
};

void TestsPnlHigh::TestConditionalGaussianGetJPD()
{
    printf("TestConditionalGaussianGetJPD\n");

    BayesNet *net = SimpleCGM1();
    
    net->SetPGaussian("Cont0", "1.5 -0.5", "1.0 0.3 0.3 2.0", TokArr(), TokArr());
    net->SetPGaussian("Cont1", "0.0", "2.5", "1.0 3.0", "Tab0^State0");
    net->SetPGaussian("Cont1", "-1.5", "0.75", "0.5 2.5", "Tab0^State1");

    net->SetProperty("Inference", "naive");

    net->EditEvidence("Tab0^State0");
    net->GetJPD("Cont0");
    net->GetJPD("Cont1");
    net->GetJPD("Cont2");

    net->ClearEvid();

    Tok tok0("Cont0^Dim0^0.0");
    Tok tok1("Cont0^Dim1^0.0");

    TokIdNode *id0 = tok0.Node();
    TokIdNode *id1 = tok1.Node();

    bool is_int0 = id0->id[id0->id.size()-1].is_int; 
    bool is_int1 = id1->id[id1->id.size()-1].is_int;

    int int_id0 = id0->id[id0->id.size()-1].int_id;
    int int_id1 = id1->id[id1->id.size()-1].int_id;

    TokIdNode *id = net->Net().Token().Node(Tok("Cont0"))->v_next;
    TokIdNode *id2 = net->Net().Token().Node(Tok("Cont1"))->v_next;
    TokIdNode *id3 = net->Net().Token().Node(Tok("Cont2"))->v_next;
    TokIdNode *id4 = net->Net().Token().Node(Tok("Tab0"))->v_next;

    //TokIdNode *a1 = id->v_next;
    //TokIdNode *a2 = a1->h_next;

    net->EditEvidence("Cont0^Dim0^0.0 Cont0^Dim1^1.0");
    net->EditEvidence("Cont1^Dim0^0.0");
    net->EditEvidence("Cont2^Dim0^0.0");

    net->GetJPD("Tab0");

    delete net;
};  

void TestsPnlHigh::Test2EditEvidence()
{
    BayesNet *net = VerySimpleMultivariateGaussianModel();

    net->EditEvidence("NodeB^dim1^0.0 NodeB^dim2^1.0");
    net->EditEvidence("NodeA^dim1^0.7 NodeA^dim2^1.7");
    net->EditEvidence("NodeC^dim1^0.7 NodeC^dim2^1.7");

    net->ClearEvid();

    net->EditEvidence("NodeB^dim1^0.0 NodeB^dim2^1.0");
    net->EditEvidence("NodeA^dim1^0.7 NodeA^dim2^1.7");
    net->EditEvidence("NodeC^dim1^0.7 NodeC^dim2^1.7");

    delete net;

    net = VerySimpleGaussianModel();

    net->EditEvidence("NodeB^dim1^0.0");
    net->EditEvidence("NodeA^dim1^0.7");
    net->EditEvidence("NodeC^dim1^1.4");

    delete net;
};

#define PRINT_TestCondGaussianGetMPE

void TestsPnlHigh::TestCondGaussianGetMPE()
{
    printf("TestCondGaussianGetMPE\n");

    BayesNet *net = SimpleCGM1();
    //Cont0(3) Tab0(0)
    //  |      |
    //  \/    \/
    //   Cont1(1)
    //    |
    //    \/
    //   Cont2(2)

    net->SetPGaussian("Cont0", "1.5 -0.5", "1.0 0.3 0.3 2.0", TokArr(), TokArr());
    net->SetPGaussian("Cont1", "0.0", "2.5", "1.0 3.0", "Tab0^State0");
    net->SetPGaussian("Cont1", "-1.5", "0.75", "0.5 2.5", "Tab0^State1");

    net->SetProperty("Inference", "naive");

    net->EditEvidence("Tab0^State1");
    #ifndef PRINT_TestCondGaussianGetMPE
	net->GetMPE("Cont0");
	net->GetMPE("Cont1");
	net->GetMPE("Cont2");
    #else
	std::cout << String(net->GetMPE("Cont0")) << "\n";
	std::cout << String(net->GetMPE("Cont1")) << "\n";
	std::cout << String(net->GetMPE("Cont2")) << "\n";
    #endif

    net->ClearEvid();

    net->EditEvidence("Cont0^Dim0^0.0 Cont0^Dim1^1.0");
    net->EditEvidence("Cont1^Dim0^0.0");
    net->EditEvidence("Cont2^Dim0^0.0");

    #ifndef PRINT_TestCondGaussianGetMPE
	net->GetMPE("Tab0");
    #else
	std::cout << String(net->GetMPE("Tab0")) << "\n";
    #endif

    delete net;
	std::cout << "TestCondGaussianGetMPE is completed successfully" << std::endl;

};

//Cont0(3) Tab0(0)
//  |      |
//  \/    \/
//   Cont1(1)
//    |
//    \/
//   Cont2(2) (with parameters)
BayesNet *TestsPnlHigh::SimpleCGM2()
{
    BayesNet *net = SimpleCGM1();

    net->SetPGaussian("Cont0", "1.5 -0.5", "1.0 0.3 0.3 2.0", TokArr(), TokArr());
    net->SetPGaussian("Cont1", "0.0", "2.5", "1.0 3.0", "Tab0^State0");
    net->SetPGaussian("Cont1", "-1.5", "0.75", "0.5 2.5", "Tab0^State1");
    net->SetPGaussian("Cont2", "0.1", "1.1", "0.0");

    return net;

}

/*
void TestsPnlHigh::TestCondGaussianParamLearning()
{
    BayesNet *net = PolytreeModel();
    BayesNet *netToLearn = PolytreeModel();
    float eps = 1e-1f;

    int nEvid = 5000;
    netToLearn->GenerateEvidences(nEvid);

    netToLearn->LearnParameters();

    String nodes[] = {"NodeA", "NodeB", "NodeC", "NodeD", "NodeE"};

    int nNodes = 5;
    int i, j;
    TokArr LearnParam, Param;
    for(i = 0; i < nNodes; i++)
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

    delete net;
    delete netToLearn;
    
    cout << "TestGaussianParamLearning is completed successfully" << endl;
}
*/

void TestsPnlHigh::TestCondGaussianParamLearning()
{
    BayesNet *net = SimpleCGM2();
    BayesNet *netToLearn = SimpleCGM2();

    float eps = 1e-2f;

    int nEvidences = 5000;
    netToLearn->GenerateEvidences(nEvidences);

    netToLearn->LearnParameters();
/*
    int nNodes = netToLearn->Net().Graph().nNode();

    //Checking step
    int i; 
    int j;
    TokArr LearnParam, Param;
    for (i = 0; i < nNodes; i++)
    {
	//if it is gaussian without tabular parents
	if (true)
	{
	    LearnParam = netToLearn->GetGaussianMean(nodes[i]);
	    Param = net->GetGaussianMean(nodes[i]);
	    if(LearnParam[0].fload.size() != Param[0].fload.size())
	    {
		PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
	    };
	    for(j = 0; j < LearnParam[0].fload.size(); j++)
	    {
		if( LearnParam[0].FltValue(j).fl - Param[0].FltValue(j).fl > eps)
		{
		    PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
		}
	    };

	    LearnParam = netToLearn->GetGaussianCovar(nodes[i]);
	    Param = net->GetGaussianCovar(nodes[i]);
	    if(LearnParam[0].fload.size() != Param[0].fload.size())
	    {
		PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
	    };
	    for(j = 0; j < LearnParam[0].fload.size(); j++)
	    {
		if( LearnParam[0].FltValue(j).fl - Param[0].FltValue(j).fl > eps)
		{
		    PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
		}
	    };

	    TokArr parents = netToLearn->GetParents(nodes[i]);
	    LearnParam = netToLearn->GetGaussianWeights(nodes[i], parents);
	    Param = net->GetGaussianWeights(nodes[i], parents);
	    if(LearnParam[0].fload.size() != Param[0].fload.size())
	    {
		PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
	    };
	    for(j = 0; j < LearnParam[0].fload.size(); j++)
	    {
		if( LearnParam[0].FltValue(j).fl - Param[0].FltValue(j).fl > eps)
		{
		    PNL_THROW(pnl::CAlgorithmicException, "Parameters learning is wrong");
		}
	    };	
	};
    };
*/
    delete net;
    delete netToLearn;
}

int testCondGaussian()
{
	int res = TRS_OK;
	TestsPnlHigh tests;
	try 
    {
        tests.TestDesc();
        tests.TestNTabNCont();
        tests.TestDefaultDistribution();
        tests.TestCondGaussianFillData();
        tests.TestSetPGaussian();
        tests.TestGetDiscreteParentValuesIndexes();
        tests.TestGetGaussianMeanCovarWeights(); 
        tests.TestConditionalGaussianGetJPD();
        tests.Test2EditEvidence();
		tests.TestCondGaussianGetMPE();
		tests.TestCondGaussianParamLearning();
    }
    catch(pnl::CException e)
    {
        std::cout << e.GetMessage();
        res = TRS_FAIL;
    }
	return trsResult( res, res == TRS_OK ? "No errors"
		: "Bad test on DBN wrappers");
}

void initTestsCondGaussian()
{
    trsReg(func_name, test_desc, test_class, testCondGaussian);
}
