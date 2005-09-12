#ifndef TRAFFICLIGHT
#define TRAFFICLIGHT

#include "pnlHigh.hpp"
#include "pnlHighConf.hpp"

class CTrafficLight
{
protected:
	double carSpeed1; 
	double carSpeed2; 
	double intervalTime; 
	                     
	double *lightingParts; 
	                       
	int numLightParts; 
public:
    CTrafficLight(double* lightParts, int numlightparts, double intervaltime, double speedStream1, double speedStream2)
	{ 
		carSpeed1 = speedStream1;
		carSpeed2 = speedStream2;
		intervalTime = intervaltime;
		lightingParts = lightParts;
        numLightParts = numlightparts;
	}
	~CTrafficLight();
	virtual void Solve();
	double GetCarSpeed1(){return carSpeed1;};
	double GetCarSpeed2(){return carSpeed2;}; 
    double GetIntervalTime(){return intervalTime;};
	double GetNumLightParts(){return numLightParts;};
	double* GetLightingParts(){return lightingParts;};
};

#endif