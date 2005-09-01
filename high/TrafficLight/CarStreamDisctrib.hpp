#ifndef CARSTREAM
#define CARSTREAM

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAX_M_VALUE 10000

class CCarStreamDisctrib
{
protected:
	double lambda;
	double getPuassonProb(int m, double lambda);
	double getBartProb(int y);


public:
	CCarStreamDisctrib();
    ~CCarStreamDisctrib();

	double GetLambda(){ return lambda;};
	void SetLambda(double lam) {lambda = lam;};

	int getPuassonValue();
	int getBartlettValue();
};

#endif