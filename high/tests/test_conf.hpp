#include <string>
#include <iostream>
#include "pnlHigh.hpp"
#include "pnlTok.hpp"

#define SKIP_DBN
#define SKIP_LIMID
//#define SKIP_TOKEN

// FORWARDS
namespace pnl
{
    class CGraphicalModel;
};

PNLW_USING

std::ostream &operator<<(std::ostream &str, TokArr &ta);
int testDBNSmothing();
int testDBNFixLagSmothing();
int testDBNFiltering();
int testDBNMPE();
int testDBNLearning();
pnl::CGraphicalModel* LoadGrModelFromXML(const string& xmlname, vector<pnl::CEvidence*>* pEv);
void OilTest();
void TestPigs();
void testRandom();
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
bool testExtractTokArr();
