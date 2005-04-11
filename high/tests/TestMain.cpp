#include "test_conf.hpp"

//#define SKIP_TOKEN
//#define SKIP_LIMID
#define SKIP_DBN
//#define SKIP_MRF
//#define SKIP_NUMERATION

int main(int argc, char* argv[])
{
    bool bTestOK = true, lastTestRes;

#ifndef SKIP_TOKEN
    bTestOK = TestResolve1() && bTestOK;
    bTestOK = TestResolve2() && bTestOK;
    bTestOK = TestResolve3() && bTestOK;
    bTestOK = TestResolve4() && bTestOK;
    bTestOK = TestResolve4_5() && bTestOK;
    bTestOK = testExtractTok() && bTestOK;
    try 
    {
        SimpleModel();
	TestGaussianModelCreate();// del arc
	TestGaussianInference();
	TestGaussianParamLearning();// too long
	TestGetLogLik();
	TestMultivariateWrapperJPD();
	TestMultivariateWrapperLearn();
	TestMultivariateWrapperLearnPartOfObservation();
//	TestMultivariateWrapperMPE();// wild test. It doesn't test wrapper!
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
	lastTestRes = false;
        PureLimidModel1();
    }
    catch(pnl::CException e)
    {
	static const char messageOk[] = "CLIMIDInfEngine:: there are no desision nodes in net";

	if((e.GetCode() == pgmInternalError)
	    && strstr(e.GetMessage(), messageOk))
	{
	    lastTestRes = true;// this exception must be produced
	}
	else
	{
	    std::cout << e.GetMessage()<< "\n";
	}
    }
    bTestOK = bTestOK && lastTestRes;

    try 
    {
	PureLimidModel2();
    }
    catch(pnl::CException e)
    {
        std::cout << e.GetMessage()<< "\n";
	lastTestRes = false;
    }
    bTestOK = bTestOK && lastTestRes;
    
    try 
    {
	PureLimidModel3();
    }
    catch(pnl::CException e)
    {
        std::cout << e.GetMessage()<<"\n";
	lastTestRes = false;
    }
    bTestOK = bTestOK && lastTestRes;
    
    try 
    {
        LimidTopology();
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
    int res = 1;
    
    res = testDBN() & res;
    res = testDBNTopologicalSort1() & res;
    res = testDBNTopologicalSort2() & res;
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
