#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"
#include "WDistribFun.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWDistributions.hpp"

PNLW_USING

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

void TestsPnlHigh::TestDesc()
{
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
}
