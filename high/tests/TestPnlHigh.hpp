#ifndef __TEST_PNL_HIGH
#define __TEST_PNL_HIGH

PNLW_BEGIN
    class BayesNet;

class TestsPnlHigh
{
public:
    //Conditional Gaussian tests
    void TestDesc();	
    void TestNTabNCont();
    void Test2Bnets();
    void TestDefaultDistribution();
    void TestCondGaussianFillData();
    void TestSetPGaussian();
    void TestGetDiscreteParentValuesIndexes();
    void TestGetGaussianMeanCovarWeights();
    void TestConditionalGaussianGetJPD();
    void TestCondGaussianGetMPE();
    void TestCondGaussianParamLearning();

    //Evidence
    void Test2EditEvidence();

    //Gibbs BNet, MRF
    void TestGibbsRecreate();

    //Networks for testing purpose
    // Cont1  Tab1
    //    |   |
    //    \/ \/
    //    Cont2 (without parameters)
    BayesNet *CreateCondGaussianModel1();
   
    //Cont0(3) Tab0(0)
    //  |      |
    //  \/    \/
    //   Cont1(1)
    //    |
    //    \/
    //   Cont2(2) (without parameters)
    BayesNet *SimpleCGM1();

    //Cont0(3) Tab0(0)
    //  |      |
    //  \/    \/
    //   Cont1(1)
    //    |
    //    \/
    //   Cont2(2) (with parameters)
    BayesNet *SimpleCGM2();

};

PNLW_END

#endif