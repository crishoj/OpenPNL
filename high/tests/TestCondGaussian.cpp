#include "test_conf.hpp"
#include "pnlException.hpp"
#include "pnl_dll.hpp"
#include "WDistribFun.hpp"
#include "pnlWProbabilisticNet.hpp"
#include "pnlWDistributions.hpp"
#include "pnlMatrix.hpp"

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
    }

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
}

void TestsPnlHigh::TestDefaultDistribution()
{
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
    }

    if (pCGDF->GetAValue(pnl::matCovariance, aIndex) != 1.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    }

    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    }

    aIndex[5] = 1;

    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    }

    aIndex[0] = 1;

    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    }

    aIndex[5] = 0;

    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Wrong default distribution");
    }

    //WGaussianDistribFun *pGDF = dynamic_cast<WGaussianDistribFun *>(net->m_pNet->m_paDistribution->Distribution(2));
    //pGDF->m_pDistrib->Dump();

    delete net;
}

void TestsPnlHigh::TestCondGaussianFillData()
{
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
    }

    aIndex[5] = 1;

    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 3.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    }

    aIndex[IndexTab0] = 1;
    aIndex[5] = 0;

    if ((pCGDF->GetAValue(pnl::matMean, aIndex) != -1.5f)||
	(pCGDF->GetAValue(pnl::matCovariance, aIndex) != 0.75f)||
	(pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.5f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    }

    aIndex[5] = 1;

    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 2.5f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    }

    delete net;
}

void TestsPnlHigh::TestSetPGaussian()
{
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
    }

    aIndex[5] = 1;

    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 3.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    }

    aIndex[IndexTab0] = 1;
    aIndex[5] = 0;

    if ((pCGDF->GetAValue(pnl::matMean, aIndex) != -1.5f)||
	(pCGDF->GetAValue(pnl::matCovariance, aIndex) != 0.75f)||
	(pCGDF->GetAValue(pnl::matWeights, aIndex) != 0.5f))
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    }

    aIndex[5] = 1;

    if (pCGDF->GetAValue(pnl::matWeights, aIndex) != 2.5f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "There is an error in FillData");
    }

    delete net;
}

void TestsPnlHigh::TestGetDiscreteParentValuesIndexes()
{
    BayesNet *net = SimpleCGM1();

    net->SetPGaussian("Cont1", "0.0", "2.5", "1.0 3.0", "Tab0^State0");
    net->SetPGaussian("Cont1", "-1.5", "0.75", "0.5 2.5", "Tab0^State1");

    WCondGaussianDistribFun *pCGDF = dynamic_cast<WCondGaussianDistribFun *>(net->m_pNet->m_paDistribution->Distribution(1));

    TokArr ta("Tab0^State0");
    Vector<int> dpInd = pCGDF->GetDiscreteParentValuesIndexes(ta);

    if (dpInd.size()!=1)
    {
	PNL_THROW(pnl::CAlgorithmicException, "Size of dpInd is wrong");
    }

    if (dpInd[0] != 0)
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetDiscreteParentValuesIndexes works incorrectly");
    }

    TokArr ta2("Tab0^State1");
    dpInd = pCGDF->GetDiscreteParentValuesIndexes(ta2);

    if (dpInd[0] != 1)
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetDiscreteParentValuesIndexes works incorrectly");
    }
}

void TestsPnlHigh::TestGetGaussianMean()
{
    BayesNet *net = SimpleCGM1();

    net->SetPGaussian("Cont1", "0.0", "2.5", "1.0 3.0", "Tab0^State0");
    net->SetPGaussian("Cont1", "-1.5", "0.75", "0.5 2.5", "Tab0^State1");

    if (net->GetGaussianMean("Cont1", "Tab0^State0")[0].FltValue() != 0.0f)
    {
	PNL_THROW(pnl::CAlgorithmicException, "GetGaussianMean works incorrectly");
    }
}