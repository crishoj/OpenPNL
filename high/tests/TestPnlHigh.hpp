#ifndef __TEST_PNL_HIGH
#define __TEST_PNL_HIGH

PNLW_BEGIN
    class BayesNet;

class TestsPnlHigh
{
public:
    void TestDesc();	
    void TestNTabNCont();
    void Test2Bnets();
    void TestDefaultDistribution();
    void TestCondGaussianFillData();
    void TestSetPGaussian();
    void TestGetDiscreteParentValuesIndexes();
    void TestGetGaussianMean();

private:
    BayesNet *CreateCondGaussianModel1();
    BayesNet *SimpleCGM1();
};

PNLW_END

#endif