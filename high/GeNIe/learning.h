#ifndef GENIE_INTERFACES_LEARNING_H
#define GENIE_INTERFACES_LEARNING_H

// learning.h

struct LearnParams
{
	const char *dataFile;
	const char *networkFile;
	const char *classVar;
	const char *networkId;
	int method;
	int maxParents;
	int priorsMethod;
	double netWeight;
};

class ILearning
{
public:
	virtual void Delete() = 0;
	virtual bool LearnStructureAndParams(const LearnParams &params) = 0;
};


#endif // GENIE_INTERFACES_LEARNING_H
