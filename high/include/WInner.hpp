#ifndef __WINNER_HPP__
#define __WINNER_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif

// FORWARDS
struct TokIdNode;

// bitwise flags for m_Objects member of BayesNet object
const int eGraph       = 1;
const int eModel       = 2;
const int eInference   = 4;
const int eLearning    = 8;
const int eAllObjects  =15;

const int nTypesObject = 4;

#define NEED(A) ((A) & ((1 << nTypesObject) - 1))
#define GET_NEEDS(A) NEED(A)
#define PROHIBIT(A) ((A) << nTypesObject)
#define GET_PROHIBITION(A) (((A) >> nTypesObject) & ((1 << nTypesObject) - 1))

typedef enum
{   eAddNode
,   eLoadNet
,   eNodeType
,   eSaveNet
,   eAddArc
,   eSetP
,   eSetGaussian
,   eP
,   eMakeUniformDistribution
,   eEvidence
,   eClearEvid
,   eClearEvidHistory
,   eLearn
,   eMPE
,   eSaveLearnBuf
,   eLoadLearnBuf
,   ePushEvid
,   eGenerateEvidences
,   eLearnStructure
,   eGaussianMean
,   eGaussianCovar
};

typedef enum
{   eTagNet
,   eTagService
,   eTagNodeType
,   eTagNetNode
,   eTagValue
,   eTagRootClassification
};

typedef enum
{   eNodeClassUnknown   = 1
,   eNodeClassCategoric = 2
,   eNodeClassContinuous= 4
} ENodeClassification;

TokIdNode* flatSearch(TokIdNode *parent, const char *name);
TokIdNode *ancestorByTagValue(TokIdNode *node, int tag, int value);

#endif // __WINNER_HPP__
