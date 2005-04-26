/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnl_dll.hpp                                                 //
//                                                                         //
//  Purpose:   Header file, which includes all headers of the library      //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
#ifndef __PNL_DLL_HPP__
#define __PNL_DLL_HPP__

#include "pnlConfig.hpp"
#include "pnlCPD.hpp"
#include "pnlBNet.hpp"
#include "pnlException.hpp"
#include "pnlPotential.hpp"
#include "pnlIDPotential.hpp"
#include "pnlGraph.hpp"
#include "pnlStaticGraphicalModel.hpp"
#include "pnlInferenceEngine.hpp"
#include "pnlJtreeInferenceEngine.hpp"
#include "pnlJunctionTree.hpp"
#include "pnlMatrix.hpp"
#include "pnlMNet.hpp"
#include "pnlModelTypes.hpp"
#include "pnlMRF2.hpp"
#include "pnlNaiveInferenceEngine.hpp"
#include "pnlNodeType.hpp"
#include "pnlFactor.hpp"
#include "pnlFactors.hpp"
#include "pnlPearlInferenceEngine.hpp"
#include "pnlSpecPearlInferenceEngine.hpp"
#include "pnlLIMIDInfEngine.hpp"
#include "pnlReferenceCounter.hpp"
#include "pnlTabularCPD.hpp"
#include "pnlTabularPotential.hpp"
#include "pnlIDTabularPotential.hpp"
#include "pnlGaussianPotential.hpp"
#include "pnlGaussianCPD.hpp"
#include "pnlSoftMaxCPD.hpp"
#include "pnlTypeDefs.hpp"
#include "pnlEvidence.hpp"
#include "pnlLearningEngine.hpp"
#include "pnlMlLearningEngine.hpp"
#include "pnlStaticLearningEngine.hpp"
#include "pnlDynamicLearningEngine.hpp"
#include "pnlEmLearningEngine.hpp"
#include "pnlBicLearningEngine.hpp"
#include "pnlEmLearningEngineDBN.hpp"
#include "pnlDynamicGraphicalModel.hpp"
#include "pnlDBN.hpp"
#include "pnlIDNet.hpp"
#include "pnlDynamicInferenceEngine.hpp"
#include "pnl2TBNInferenceEngine.hpp"
#include "pnl1_5SliceJtreeInferenceEngine.hpp"
#include "pnlRing.hpp"
#include "pnlExampleModels.hpp"
#include "pnl2DNumericDenseMatrix.hpp"
#include "pnlNumericDenseMatrix.hpp"
#include "pnlDenseMatrix.hpp"
#include "pnlSparseMatrix.hpp"
#include "pnlNumericSparseMatrix.hpp"
#include "pnlNumericMatrix.hpp"
#include "pnlMixtureGaussianCPD.hpp"
#include "pnlGibbsSamplingInferenceEngine.hpp"
#include "pnlModelDomain.hpp"
#include "pnlFactorGraph.hpp"
#include "pnlFGSumMaxInferenceEngine.hpp"
#include "pnlBayesLearningEngine.hpp"
#include "pnlTimer.hpp"
#include "pnlBKInferenceEngine.hpp"
#include "pnlGibbsWithAnnealingInferenceEngine.hpp"
#include "pnlRng.hpp"
#include "pnlDistribFun.hpp"
#include "pnlTabularDistribFun.hpp"
#include "pnlGaussianDistribFun.hpp"
#include "pnlScalarDistribFun.hpp"
#include "pnlCondGaussianDistribFun.hpp"
#include "pnlSoftMaxDistribFun.hpp"
#include "pnlCondSoftMaxDistribFun.hpp"
#include "pnlParNewDelete.hpp"
#include "pnlParPearlInferenceEngine.hpp"
#include "pnlParJtreeInferenceEngine.hpp"
#include "pnlParGibbsSamplingInferenceEngine.hpp"
#include "pnlParEmLearningEngine.hpp"
#include "pnlSoftMaxDistribFun.hpp"

#include "pnl2TPFInferenceEngine.hpp"
#include "pnlLWSamplingInferenceEngine.hpp"
#include "pnlContext.hpp"
#include "pnlContextPersistence.hpp"
//#include "pnliNumericDenseMatrix.hpp"
//#include "pnliNumericSparseMatrix.hpp"
//#include "pnli2DNumericDenseMatrix.hpp"
//using namespace pnl;
//#include "pnlTabularData.hpp"
//#include "pnlData.hpp"

#endif // __PNL_DLL_HPP__

/* end of file */
