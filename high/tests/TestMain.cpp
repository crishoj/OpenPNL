#include "test_conf.hpp"

int main(char* argv[], int argc)
{
    bool bTestOK = true;

#ifndef SKIP_TOKEN
    bTestOK = TestResolve1() && bTestOK;
    bTestOK = TestResolve2() && bTestOK;
    bTestOK = TestResolve3() && bTestOK;
    bTestOK = TestResolve4() && bTestOK;
    bTestOK = TestResolve4_5() && bTestOK;
    bTestOK = testExtractTokArr() && bTestOK;
    try 
    {
	TestMultivariateWrapperJPD();
	TestMultivariateWrapperLearn();
	TestMultivariateWrapperLearnPartOfObservation();
	TestMultivariateWrapperMPE();
    }
    catch(pnl::CException e)
    {
	std::cout << e.GetMessage();
	bTestOK = false;
    }
#endif
#ifndef SKIP_LIMID
    try 
    {
	//            TestPigs();
	//            OilTest();
	testRandom();
    }
    catch(pnl::CException e)
    {
	std::cout << e.GetMessage();
	bTestOK = false;
    }

#endif
#ifndef SKIP_DBN
    int res = 0;
    res = testDBNSmothing() & res;
    res = testDBNFixLagSmothing() & res;
    res = testDBNFiltering() & res;
    res = testDBNMPE() & res;
    res = testDBNLearning() & res;
    bTestOK = bTestOK && (res != 0);
#endif

    if(bTestOK)
    {
	std::cout << "All tests completed successfully" << "\n";
    }

    return 0;
}
