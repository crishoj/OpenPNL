SUBDIRS = include

LIBS = -ldl -lm

include $(top_srcdir)/Makefile.incl

SOMAJOR=@PNLMAJOR@
SOMINOR=@PNLMINOR@
SOSUBMINOR=@PNLSUBMINOR@
INCLUDES=$(PNL_INCL)


lib_LTLIBRARIES=libpnl.la

libpnl_la_SOURCES=      \
     cvcart.cpp     \
     cvsvd.c     \
     dbginfo.cpp     \
     memtrack.cpp     \
     pnl1_5SliceInferenceEngine.cpp     \
     pnl1_5SliceJtreeInferenceEngine.cpp     \
     pnl2DBitwiseMatrix.cpp     \
     pnl2DNumericDenseMatrix.cpp     \
     pnl2TBNInferenceEngine.cpp     \
     pnl2TPFInferenceEngine.cpp     \
     pnlAllocator.cpp     \
     pnlBayesLearningEngine.cpp     \
     pnlBicLearningEngine.cpp     \
     pnlBKInferenceEngine.cpp     \
     pnlBNet.cpp     \
     pnlCondGaussianDistribFun.cpp     \
     pnlCondSoftMaxDistribFun.cpp     \
     pnlConfig.cpp     \
     pnlContext.cpp     \
     pnlContextLoad.cpp     \
     pnlContextPersistence.cpp     \
     pnlContextSave.cpp     \
     pnlCoverage.cpp     \
     pnlCPD.cpp     \
     pnlDAG.cpp     \
     pnlDBN.cpp     \
     pnlDenseMatrix.cpp     \
     pnlDiagnostics.cpp     \
     pnlDistribFun.cpp     \
     pnlDynamicGraphicalModel.cpp     \
     pnlDynamicInferenceEngine.cpp     \
     pnlDynamicLearningEngine.cpp     \
     pnlEmLearningEngine.cpp     \
     pnlEmLearningEngineDBN.cpp     \
     pnlEvidence.cpp     \
     pnlExampleModels.cpp     \
     pnlException.cpp     \
     pnlExInferenceEngine.cpp     \
     pnlFactor.cpp     \
     pnlFactorGraph.cpp     \
     pnlFactors.cpp     \
     pnlFakePtr.cpp     \
     pnlFGSumMaxInferenceEngine.cpp     \
     pnlGaussianCPD.cpp     \
     pnlGaussianDistribFun.cpp     \
     pnlGaussianPotential.cpp     \
     pnlGibbsSamplingInferenceEngine.cpp     \
     pnlGibbsWithAnnealingInferenceEngine.cpp     \
     pnlGraph.cpp     \
     pnlGraphicalModel.cpp     \
     pnlGroup.cpp     \
     pnli2DNumericDenseMatrix.cpp     \
     pnlIDNet.cpp     \
     pnlIDPotential.cpp     \
     pnlIDTabularPotential.cpp     \
     pnlInferenceEngine.cpp     \
     pnliNumericDenseMatrix.cpp     \
     pnliNumericSparseMatrix.cpp     \
     pnlJtreeInferenceEngine.cpp     \
     pnlJunctionTree.cpp     \
     pnlLearningEngine.cpp     \
     pnlLIMIDInfEngine.cpp     \
     pnlLog.cpp     \
     pnlLogDriver.cpp     \
     pnlLogMultiplexor.cpp     \
     pnlLogUsing.cpp     \
     pnlLWSamplingInferenceEngine.cpp     \
     pnlMatrix.cpp     \
     pnlMatrixIterator.cpp     \
     pnlMixtureGaussianCPD.cpp     \
     pnlMlDynamicStructLearn.cpp     \
     pnlMlStaticStructLearn.cpp     \
     pnlMlStaticStructLearnHC.cpp     \
     pnlMNet.cpp     \
     pnlModelDomain.cpp     \
     pnlMRF2.cpp     \
     pnlNaiveInferenceEngine.cpp     \
     pnlNodeType.cpp     \
     pnlNodeValues.cpp     \
     pnlNumericDenseMatrix.cpp     \
     pnlObject.cpp     \
     pnlObjHandler.cpp     \
     pnlParConfig.cpp     \
     pnlParEmLearningEngine.cpp     \
     pnlParGibbsSamplingInferenceEngine.cpp     \
     pnlParJtreeInferenceEngine.cpp     \
     pnlParNewDelete.cpp     \
     pnlParPearlInferenceEngine.cpp     \
     pnlPearlInferenceEngine.cpp     \
     pnlPersistCover.cpp     \
     pnlPersistCPD.cpp     \
     pnlPersistDistribFun.cpp     \
     pnlPersistence.cpp     \
     pnlPersistGraph.cpp     \
     pnlPersistModel.cpp     \
     pnlPersistModelDomain.cpp     \
     pnlPersistTypes.cpp     \
     pnlpnlType.cpp     \
     pnlPotential.cpp     \
     pnlReferenceCounter.cpp     \
     pnlRng.cpp     \
     pnlSamplingInferenceEngine.cpp     \
     pnlScalarDistribFun.cpp     \
     pnlScalarPotential.cpp     \
     pnlSoftMaxCPD.cpp     \
     pnlSoftMaxDistribFun.cpp     \
     pnlSparseMatrix.cpp     \
     pnlSpecPearlInferenceEngine.cpp     \
     pnlStaticGraphicalModel.cpp     \
     pnlStaticLearningEngine.cpp     \
     pnlStaticStructLearnSEM.cpp     \
     pnlString.cpp     \
     pnlTabularCPD.cpp     \
     pnlTabularDistribFun.cpp     \
     pnlTabularPotential.cpp     \
     pnlTimer.cpp     \
     pnlTreeCPD.cpp     \
     pnlTreeDistribFun.cpp     \
     pnlXMLRead.cpp     \
     pnlXMLWrite.cpp

libpnl_la_LDFLAGS = -no-undefined -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE) @LD_FLAGS@

