#include "test_conf.hpp"

//#define SKIP_TOKEN
//#define SKIP_LIMID
//#define SKIP_DBN
//#define SKIP_MRF
//#define SKIP_NUMERATION

int main(char* argv[], int argc)
{
    bool bTestOK = true;

#ifndef SKIP_TOKEN
    bTestOK = TestResolve1() && bTestOK;
    bTestOK = TestResolve2() && bTestOK;
    bTestOK = TestResolve3() && bTestOK;
    bTestOK = TestResolve4() && bTestOK;
    bTestOK = TestResolve4_5() && bTestOK;
    bTestOK = testExtractTok() && bTestOK;
    try 
    {
        TestGaussianModelCreate();
        TestGaussianInference();
        TestGaussianParamLearning();
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
//        LimidTopology();
        DelNodes();
        TestPigs();
	OilTest();
	testRandom1();
        testRandom2();
	testRandom3();
    }
    catch(pnl::CException e)
    {
	std::cout << e.GetMessage();
	bTestOK = false;
    }

#endif
#ifndef SKIP_DBN
    int res = 0;
    
    res = testDBN();
    res = testDBNSmothing() & res;
    res = testDBNFixLagSmothing() & res;
    res = testDBNFiltering() & res;
    res = testDBNMPE() & res;
    res = testDBNLearning() & res; 
    res = testDBNSaveLoad() & res;

    bTestOK = bTestOK && (res != 0);
#endif

#ifndef SKIP_MRF
    res = 0;    
    res = testMRF();

    bTestOK = bTestOK && (res != 0);
#endif
#ifndef SKIP_NUMERATION
    
    try 
    {
        TestForGetPTabular();
	TestForSetInferenceProperties();
    }
    catch(pnl::CException e)
    {
	std::cout << e.GetMessage();
	bTestOK = false;
    }
#endif

    if(bTestOK)
    {
	std::cout << "All tests were completed successfully" << "\n";
    }

    return 0;
}
