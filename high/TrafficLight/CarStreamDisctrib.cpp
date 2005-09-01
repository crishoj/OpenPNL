#include "CarStreamDisctrib.hpp"

CCarStreamDisctrib::CCarStreamDisctrib()
{
	lambda = 1;
}

CCarStreamDisctrib::~CCarStreamDisctrib()
{
}

double CCarStreamDisctrib::getPuassonProb(int m, double lambda)
{
 int i;
 double res = exp(-lambda);
 int div; 
 if(m == 0 ) 
 {
	 return res;
 }
 for(i = 1; i <= m; i++)
 {
     res *= lambda / i;
 }
 return res;
}

int CCarStreamDisctrib::getPuassonValue()
{
	double rndVal = double(rand())/RAND_MAX;
	double puasVal = 0;// getPuassonProb(0, lambda);
	double nextPuasVal;
	int i;
	
	for(i = 0; i < MAX_M_VALUE; i++)
	{
		nextPuasVal = puasVal + getPuassonProb(i/* + 1*/, lambda);
		if(rndVal >= puasVal && rndVal < nextPuasVal)
		{
			return i;
		}
		else
		{
            puasVal = nextPuasVal;
		}
	}
	if(i == MAX_M_VALUE) 
	{
		return -1;
	}
}

double CCarStreamDisctrib::getBartProb(int y)
{
	if( y < 1)
	{
		printf("\n BAD BARTLETT VALUE");
		return -1;
	}
	else
	{
		if( y == 1)
		{
			return (1 - 0.5081);
		}
		else
		{
			return (0.5081 * (1 - 0.54574) * pow(0.54574, y - 2));
		}
	}
}

int CCarStreamDisctrib::getBartlettValue()
{
    double rndVal = double(rand())/RAND_MAX;
	double bartVal = 0;
	double nextBartVal;
	int i;
	
	for(i = 1; i < MAX_M_VALUE; i++)
	{
		nextBartVal = bartVal + getBartProb(i);
		if(rndVal >= bartVal && rndVal < nextBartVal)
		{
			return i;
		}
		else
		{
            bartVal = nextBartVal;
		}
	}
	if(i == MAX_M_VALUE) 
	{
		return -1;
	}
}