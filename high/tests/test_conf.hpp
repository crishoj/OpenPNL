#include <string>
#include <iostream>
#include "pnlHigh.hpp"
#include "pnlTok.hpp"

//#define SKIP_DBN
//#define SKIP_LIMID
#define SKIP_TOKEN

// FORWARDS
namespace pnl
{
    class CGraphicalModel;
};

PNLW_USING

std::ostream &operator<<(std::ostream &str, TokArr &ta);
int testMRF();

int testDBN();
int testDBNSmothing();
int testDBNFixLagSmothing();
int testDBNFiltering();
int testDBNMPE();
int testDBNLearning();
int testDBNSaveLoad();

pnl::CGraphicalModel* LoadGrModelFromXML(const string& xmlname, vector<pnl::CEvidence*>* pEv);
void LimidTopology();
void DelNodes();
void OilTest();
void TestPigs();
void testRandom1();
void testRandom2();
void testRandom3();
bool TestResolve1();
bool TestResolve2();
bool TestResolve3();
bool TestResolve4();
bool TestResolve4_5();
void TestAlias();
void TestAlias2();
void TestAlias3();
void TestAlias4();
void SimpleTest();
void TestMultivariateWrapperJPD();
void TestMultivariateWrapperLearn();
void TestMultivariateWrapperLearnPartOfObservation();
void TestMultivariateWrapperMPE();
bool testExtractTok();
void TestGaussianModelCreate();
void TestGaussianInference();
void TestGaussianParamLearning();
void TestForGetPTabular();