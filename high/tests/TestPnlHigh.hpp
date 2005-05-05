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

    //Evidence
    void Test2EditEvidence();

private:
    BayesNet *CreateCondGaussianModel1();
    BayesNet *SimpleCGM1();
};

PNLW_END

#endif