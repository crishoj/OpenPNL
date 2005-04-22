#ifndef __TEST_PNL_HIGH
#define __TEST_PNL_HIGH

PNLW_BEGIN
    class BayesNet;

class TestsPnlHigh
{
public:
    void TestDesc();	
    void TestNTabNCont();

private:
    BayesNet *CreateCondGaussianModel1();
};

PNLW_END

#endif