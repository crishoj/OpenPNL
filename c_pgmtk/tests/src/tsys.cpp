/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      tsys.cpp                                                    //
//                                                                         //
//  Purpose:   Starts tests                                                //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "trsapi.h"
#include <stdlib.h>
#include <string.h>
#include "pnl_dll.hpp"
#include "pnlException.hpp"
#include "pnlFakePtr.hpp"

#if defined(WIN32) && defined(_DEBUG)
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#define FUNC_ENTITY(A) A, #A

void initAGraph();
void initATestGetFactorsBNet();
void initATestGetFactorsMNet();
void initATestGetFactorsMRF2();
void initSEEvidence();
void initSEFactor();
void initAShrinkObservedNodes();
void initADetermineDistrType();
void initANaiveInfEngine();
void initAMultiDMatrix();
void initAMarginalizeCompareMATLAB();
void initAMultiDMatrix_expanded();
void initARefCounter();
void initAMarginalize();
void initAMultiply();
void initANormalizePotential();
void initANormalizeCPD();
void initAReduceDims();
void initAMultiplyMatrix();
void initAPearlInfEngine();
void initAPearlInfEngineMRF2();
void initAFactorLearningEngine();
void initAGaussianInf();
void initAGaussianMRF2();
void initAStructureLearningEngine();
void initAGaussianFactorLearningEngine();
void initAUnrollDBN();
void initAJTreeInfDBN();
void initAFactorLearningDBN();
void initAConditionalGaussianInf();
void initAConditionalGaussianLearning();
void initAtest1_5JTreeInfDBNCondGauss();
void initASparseMatrixInferenceBnet();
void initAGeneralInference();
void initAGenerateEvidences();
void initAMixtureGaussian();
void initAGibbsInference();
void initALog();
void initAJTreeInfMixtureDBN();
void initAReduceOp();
void initAFactorGraph();
void initABayesLearningEngine();
void initAPerformanceJTreeInf();
void initAMixtureGaussianLearning();
void initAStackAllocator();
void initABKInfDBN();
void initAComputeLikelihood();
void initTBKInfEngine();
void initATreeCPDLearn();
void initAMRF2PearlInf();
void initTJTreeTiming();
void initTPearlTiming();
void initAPearlGibbsCompareMRF2();
void initARockPaperScissors();
void initAGibbsWithAnnealingInference();
void initADSeparation();
void initASaveLoadEvidences();
void initARandomFactors();
void initAPersistence();
void initABKInfUsingClusters();
void initSEGaussian();
void initALWInference();
void initAtest2TPFInfDBN();
void initASetStatistics();
void initALearningCondGaussDBN();
void initAInfForRndBNet();
void initAExInfEngine();
void initAExLearnEngine();
void initACondSoftMaxAndSoftMaxInfAndLearn();
void initStructLearnEngWithJTreeInf();
void initA_LIMIDinfEngine();

struct {
    void (*pFunc)();
    const char *fname;
    int testByDefault;
} aFunction[] = {
    FUNC_ENTITY(initAtest1_5JTreeInfDBNCondGauss),     1,
    FUNC_ENTITY(initAConditionalGaussianLearning),     1,
    FUNC_ENTITY(initAReduceOp),                        1,
    FUNC_ENTITY(initADSeparation),                     1,
    FUNC_ENTITY(initASaveLoadEvidences),               1,
    FUNC_ENTITY(initAGaussianFactorLearningEngine),    1,
    FUNC_ENTITY(initTBKInfEngine),                     1,
    FUNC_ENTITY(initAGraph),                           1,
    FUNC_ENTITY(initATestGetFactorsBNet),              1,
    FUNC_ENTITY(initATestGetFactorsMNet),              1,
    FUNC_ENTITY(initATestGetFactorsMRF2),	           1,
    FUNC_ENTITY(initSEEvidence),                       1,
    FUNC_ENTITY(initSEFactor),                         1,
    FUNC_ENTITY(initAShrinkObservedNodes),             1,
    FUNC_ENTITY(initADetermineDistrType),              1,
    FUNC_ENTITY(initANaiveInfEngine),                  1,
    FUNC_ENTITY(initAMultiDMatrix),                    1,
    FUNC_ENTITY(initAMultiDMatrix_expanded),           1,
    FUNC_ENTITY(initARefCounter),                      1,
    FUNC_ENTITY(initAMarginalize),                     1,
    FUNC_ENTITY(initAMultiply),                        1,
    FUNC_ENTITY(initANormalizePotential),              1,
    FUNC_ENTITY(initANormalizeCPD),                    1,
    FUNC_ENTITY(initAReduceDims),                      1,
    FUNC_ENTITY(initAMultiplyMatrix),                  1,
    FUNC_ENTITY(initAPearlInfEngine),                  1,
    FUNC_ENTITY(initAPearlInfEngineMRF2),              1,
    FUNC_ENTITY(initAFactorLearningEngine),            1,
    FUNC_ENTITY(initAGaussianInf),                     1,
    FUNC_ENTITY(initAGaussianMRF2),                    1,
    FUNC_ENTITY(initAStructureLearningEngine),         1,
    FUNC_ENTITY(initAUnrollDBN),                       1,
    FUNC_ENTITY(initAJTreeInfDBN),                     1,
    FUNC_ENTITY(initAFactorLearningDBN),               1,
    FUNC_ENTITY(initAConditionalGaussianInf),          1,
    FUNC_ENTITY(initASparseMatrixInferenceBnet),       1,
    FUNC_ENTITY(initAGeneralInference),                1,
    FUNC_ENTITY(initAPersistence),                     1,
    FUNC_ENTITY(initAGenerateEvidences),               1,
    FUNC_ENTITY(initAMixtureGaussian),                 1,
    FUNC_ENTITY(initAGibbsInference),                  1,
    FUNC_ENTITY(initALog),                             1,
    //FUNC_ENTITY(initAMarginalizeCompareMATLAB),        1,
    FUNC_ENTITY(initAJTreeInfMixtureDBN),              1,
    FUNC_ENTITY(initAFactorGraph),                     0,
    FUNC_ENTITY(initAPerformanceJTreeInf),             1,
    FUNC_ENTITY(initAMixtureGaussianLearning),         1,
    FUNC_ENTITY(initABayesLearningEngine),             1,
    FUNC_ENTITY(initAStackAllocator),                  1,
    FUNC_ENTITY(initABKInfDBN),                        1,
    FUNC_ENTITY(initAComputeLikelihood),               1,
    FUNC_ENTITY(initATreeCPDLearn),                    1,
    FUNC_ENTITY(initAMRF2PearlInf),                    1,
    FUNC_ENTITY(initTJTreeTiming),                     1,
    FUNC_ENTITY(initTPearlTiming),                     1,
    FUNC_ENTITY(initAPearlGibbsCompareMRF2),           0,
    FUNC_ENTITY(initARockPaperScissors),               0,
    FUNC_ENTITY(initAGibbsWithAnnealingInference),     1,
    FUNC_ENTITY(initARandomFactors),                   1,
    FUNC_ENTITY(initABKInfUsingClusters),              1,
    FUNC_ENTITY(initSEGaussian),                       1,
    FUNC_ENTITY(initAtest2TPFInfDBN),                  1,
    FUNC_ENTITY(initALWInference),                     1,
    FUNC_ENTITY(initASetStatistics),		       1,
    FUNC_ENTITY(initALearningCondGaussDBN),	       1,
    FUNC_ENTITY(initAInfForRndBNet),                   1,
    FUNC_ENTITY(initAExInfEngine),                     1,
    FUNC_ENTITY(initAExLearnEngine),                   1,
    FUNC_ENTITY(initACondSoftMaxAndSoftMaxInfAndLearn),1,
    FUNC_ENTITY(initStructLearnEngWithJTreeInf), 1,
    FUNC_ENTITY(initA_LIMIDinfEngine), 1,

    0, 0 // don't delete this line!
};

const int nInitFunction = sizeof(aFunction)/sizeof(aFunction[0]);

static void processFile(FILE *fp, int *v)
{
    int i;
    char buf[90];
    
    memset(v, 0, nInitFunction*sizeof(int));
    
    for(;;)
    {
        if(fscanf(fp, "%80s\n", buf) != 1)
        {
            break;
        }
        
        if(!buf[0] || buf[0] == '#')
        {
            continue;
        }
        
        for(i = 0; aFunction[i].fname; ++i)
        {
            if(strcmp(buf, aFunction[i].fname) == 0
                || (memcmp(aFunction[i].fname, "init", 4) == 0
                && strcmp(buf, aFunction[i].fname + 4) == 0))
            {
                v[i] = 1;
                break;
            }
        }
    }
}

static void processList(int *pArgc, char **argv, int *v)
{
    const char *fname;
    int nshift;
    
    if(argv[1][2])
    {
        fname = argv[1] + 2;
        nshift = 1;
    }
    else if(*pArgc > 2)
    {
        fname = argv[2];
        nshift = 2;
    }
    else
    {
        fprintf(stderr, "processList(): bad arguments\n");
        return;
    }
    
    FILE *fp;
    bool bNeedfclose;
    
    if(*fname == '-' && !fname[1])
    {
        bNeedfclose = false;
        fp = stdin;
    }
    else
    {
        bNeedfclose = true;
        if((fp = fopen(fname, "r")) == NULL)
        {
            perror(fname);
            fprintf(stderr, "processList(): arguments isn't handled");
            return;
        }
    }
    
    for(int i = 1 + nshift; i <= *pArgc; ++i)
    {
        argv[i - nshift] = argv[i];
    }
    *pArgc -= nshift;
    
    processFile(fp, v);
    
    if(bNeedfclose)
    {
        fclose(fp);
    }
}

int main(int argc, char* argv[])
{
    int abFuncRun[nInitFunction];
    int i;
  
    for(i = nInitFunction; --i >= 0;)
    {
        abFuncRun[i] = aFunction[i].testByDefault;
    }
    
    if(argc > 1 && argv[1][0] == '-' && tolower(argv[1][1]) == 's')
    {
        processList(&argc, argv, abFuncRun);
    }

    if(argc > 1 && argv[1][0] == '-' && tolower(argv[1][1]) == 'a')
    {
	for(i = 0; i < nInitFunction - 1; ++i)
	{
	    if(argv[1][1] == 'a' || aFunction[i].testByDefault)
	    {
		    printf("%s\n", aFunction[i].fname);
	    }
	}

	return 1;
    }
    
    for(i = nInitFunction; --i >= 0;)
    {
        if(abFuncRun[i] && aFunction[i].fname)
        {
            aFunction[i].pFunc();
        }
    }

    try
    {
	trsRun(argc, argv);
    }
    
    catch(pnl::CException ex)
    {
        std::cout << ex.GetMessage() << std::endl;
	fprintf(stderr, "\n###### PNL's exception: %s\n\n", ex.GetMessage());
        return 1;
    }

    catch(...)
    {
	fprintf(stderr, "\n###### got unknown exception\n");
        return 2;
    }

#if defined(_DEBUG) && defined(WIN32)
    _CrtDumpMemoryLeaks( );
#endif
    
    return 0;
}
