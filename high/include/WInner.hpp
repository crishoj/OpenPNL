#ifndef __WINNER_HPP__
#define __WINNER_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif

// FORWARDS
struct TokIdNode;

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
,   eEditEvidence
,   eClearEvid
,   eAddEvidToBuf
,   eClearEvidBuf
,   eCurEvidToBuf
,   eLearn
,   eMPE
,   eJPD
,   eSaveEvidBuf
,   eLoadEvidBuf
,   ePushEvid
,   eGenerateEvidences
,   eLearnStructure
,   eGaussianMean
,   eGaussianCovar
,   eSetProperty
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
,   eNodeClassDiscrete = 2
,   eNodeClassContinuous= 4
} ENodeClassification;

TokIdNode* flatSearch(TokIdNode *parent, const char *name);
TokIdNode *ancestorByTagValue(TokIdNode *node, int tag, int value);

typedef Vector<int> IIMap;

#endif // __WINNER_HPP__
