#include "test_conf.hpp"

//#define SKIP_TOKEN
//#define SKIP_LIMID
#define SKIP_DBN
//#define SKIP_MRF
//#define SKIP_NUMERATION

int main(int argc, char* argv[])
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
        //TestGaussianModelCreate();
        //TestGaussianInference();
        //TestGaussianParamLearning();
	//TestMultivariateWrapperJPD();
	//TestMultivariateWrapperLearn();
	//TestMultivariateWrapperLearnPartOfObservation();
	//TestMultivariateWrapperMPE();
        SimpleModel();
//        TestGaussianModelCreate();// del arc
        TestGaussianInference();
        TestGaussianParamLearning();// too long
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
        LimidTopology();
#if 0
//        PureLimidModel();
        DelNodes();
        TestPigs();
	OilTest();
	testRandom1();
        testRandom2();
	testRandom3();
#endif
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
//	res = testDBNTopologicalSort1();
    res = testDBNTopologicalSort2();
    res = testDBNSmothing() & res;
    res = testDBNFixLagSmothing() & res;
    res = testDBNFiltering() & res;
    res = testDBNMPE() & res;
    res = testDBNLearning() & res; 
    res = testDBNSaveLoad() & res;

    bTestOK = bTestOK && (res != 0);
#endif
#ifndef SKIP_MRF
    try 
    {
        TestMRFModelCreate();
        TestMRFGetJPD();
    }
    catch(pnl::CException e)
    {
	std::cout << e.GetMessage();
	bTestOK = false;
    }
#endif

#ifndef SKIP_NUMERATION
    
    try 
    {
        TestForGetPTabular();
	TestForSetInferenceProperties();
	TestForGetMPE();
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
