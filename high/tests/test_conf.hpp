#include <string>
#include <iostream>
#include "pnlHigh.hpp"
#include "Tokens.hpp"
#include "TestPnlHigh.hpp"
#include "trsapi.h"

// FORWARDS
namespace pnl
{
    class CGraphicalModel;
};

PNLW_USING

std::ostream &operator<<(std::ostream &str, const TokArr &ta);
void SimpleModel();

int testDBN();
int testDBNTopologicalSort1();
int testDBNTopologicalSort2();
int testDBNSmothing();
int testDBNFixLagSmothing();
int testDBNFiltering();
int testDBNMPE();
int testDBNLearning();
int testDBNSaveLoad();
int testDBNEvidences();

pnl::CGraphicalModel* LoadGrModelFromXML(const std::string& xmlname, std::vector<pnl::CEvidence*>* pEv);
void PureLimidModel1();
void PureLimidModel2();
void PureLimidModel3();
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
void TestGaussianModelCreate(bool DeleteNet = true);
void TestGaussianInference(bool DeleteNet = true);
void TestGaussianParamLearning(bool DeleteNet = true);
void TestGetLogLik(bool DeleteNet = true);
void TestForGetPTabular();
void TestForSetInferenceProperties();
void TestMRFGetJPD();
void TestMRFModelCreate();
void TestForGetMPE();
BayesNet *GetSimpleTabularModel();

void TestSetDistributionSoftMax();
void TestSetDistributionCondSoftMax();
void TestJtreeInferenceSoftMax1();
void TestJtreeInferenceSoftMax2();
void TestGibbsInferenceSoftMax();
void TestJtreeInferenceCondSoftMax1();
void TestJtreeInferenceCondSoftMax2();
void TestGibbsInferenceCondSoftMax();
void CrashTestJtreeInferenceSoftMax();
void TestSoftMaxParamLearning(bool DeleteNet = true);
void TestCondSoftMaxParamLearning(bool DeleteNet = true);
void TestSetDistributionSevenNodesModel();
void TestJtreeInference1SevenNodesModel();
void TestJtreeInference2SevenNodesModel();
void TestDelArc();
void TestAddArc();
void TestDelNode();
void Test2Bnets();
  