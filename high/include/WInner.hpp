#ifndef __WINNER_HPP__
#define __WINNER_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif

// FORWARDS
PNLW_BEGIN

class TokIdNode;

typedef enum
{   eAddNode
,   eDelNode
,   eLoadNet
,   eNodeType
,   eSaveNet
,   eAddArc
,   eDelArc
,   eSetPTabular
,   eSetPGaussian
,   eGetPTabular
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
,   eGetProperty
,   eGetParents
,   eGetNeighbors
,   eGetChildren
};

typedef enum
{   eTagNet	// concrete net object (BayesNet, IDNet)
,   eTagService // some service node
,   eTagNodeType// node classificator (discrete, decision)
,   eTagNetNode // concrete node of net
,   eTagValue   // value for node (true, State0)
,   eTagRootClassification // root now
};

typedef enum
{   eNodeClassUnknown   = 1
,   eNodeClassDiscrete = 2
,   eNodeClassContinuous= 4
} ENodeClassification;

TokIdNode* flatSearch(TokIdNode *parent, const char *name);
TokIdNode *ancestorByTagValue(TokIdNode *node, int tag, int value);

typedef Vector<int> IIMap;

void PrintTokTree(const char *filename, TokIdNode *node);

PNLW_END

#endif // __WINNER_HPP__
