
check_PROGRAMS = tests_pnl_c

include $(top_srcdir)/Makefile.incl

INCLUDES = $(PNL_INCL) -I$(srcdir)/../include -I$(top_srcdir)/trs/include

tests_pnl_c_SOURCES =     \
     A1_5JTreeInfDBNCondGauss.cpp     \
     A2TPFInfDBN.cpp     \
     AAnnealingGibbsInfEng.cpp     \
     ABayesParameterLearning.cpp     \
     ABKInfDBN.cpp     \
     ABKInfUsingClusters.cpp     \
     AComputeLikelihood.cpp     \
     ACondGaussian.cpp     \
     AConditionalGaussianLearning.cpp     \
     ACondSoftMaxAndSoftMax.cpp     \
     ADetermineDistrType.cpp     \
     ADSeparation.cpp     \
     AExInfEngine.cpp     \
     AExLearnEngine.cpp     \
     AFactorGraph.cpp     \
     AGaussianFactorOperations.cpp     \
     AGaussianInference.cpp     \
     AGaussianMRF2.cpp     \
     AGaussianParameterLearning.cpp     \
     AGeneralInference.cpp     \
     AGenerateEvidences.cpp     \
     AGetParametersTest.cpp     \
     AGibbsInfEngine.cpp     \
     AGraph.cpp     \
     AInfForRndBNet.cpp     \
     AJTreeInfDBN.cpp     \
     AJtreeInfMixtureDBN.cpp     \
     ALearningCondGaussDBN.cpp     \
     ALIMIDInfEngine.cpp     \
     ALog.cpp     \
     ALWInfEngine.cpp     \
     AMarginalize.cpp     \
     AMixtureGaussainLearning.cpp     \
     AMixtureGaussainModel.cpp     \
     AMRF2PearlInf.cpp     \
     AMultiDMatrix.cpp     \
     AMultiDMatrix_exp.cpp     \
     AMultiply.cpp     \
     AMultiplyVolume.cpp     \
     ANaiveInfEngine.cpp     \
     ANormalizeCPD.cpp     \
     ANormalizeFactor.cpp     \
     AParameterLearning.cpp     \
     AParameterLearningDBN.cpp     \
     AParameterTakeObsInto.cpp     \
     APearlGibbsCompareMRF2.cpp     \
     APearlInfEngine.cpp     \
     APearlInfEngineMRF2.cpp     \
     APerformanceJTreeInf.cpp     \
     APersistence.cpp     \
     ARandomFactors.cpp     \
     AReduceDims.cpp     \
     AReduceOp.cpp     \
     AReferenceCounter.cpp     \
     ARockPaperScissors.cpp     \
     ASaveLoadEvidences.cpp     \
     ASetStatistics.cpp     \
     ASparseMatrixInfernceBNet.cpp     \
     ASparseReduceOp.cpp     \
     AStackAllocator.cpp     \
     AStructuralLearningWithJTreeInference.cpp     \
     AStructureLearning.cpp     \
     ATreeCPDLearn.cpp     \
     AUnrollDBN.cpp     \
     SEEvidence.cpp     \
     SEGaussian.cpp     \
     SEParameter.cpp     \
     TBKInference.cpp     \
     tCreateArHMMwithGaussObs.cpp     \
     tCreateIncineratorBNet.cpp     \
     tCreateKjaerulffDBN.cpp     \
     tCreateRandStructs.cpp     \
     TJTreeInfEngTiming.cpp     \
     TPearlInfEngTiming.cpp     \
     tsys.cpp     \
     tUtil.cpp

tests_pnl_c_LDADD = $(PNL_LIBS) $(top_builddir)/trs/src/libtrs.a

TESTS = run_pnlcheck

run_pnlcheck: tests_pnl_c
	@echo "./tests_pnl_c -R FAIL -s $(top_builddir)/checkfails.sum -m -B" >$@; \
	chmod a+x $@

