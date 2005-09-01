#ifndef TRAFFICLIGHT
#define TRAFFICLIGHT

#include "pnlHigh.hpp"
#include "pnlHighConf.hpp"

class CTrafficLight
{
protected:
	double carSpeed1; // скорость движения машин в первом потоке
	double carSpeed2; // скорость движения машин во втором потоке
	double intervalTime; // время время горения одного цикла светофора 
	                     // т.е. красный + зеленый. Время горения желтого 
	                     // есть постоянная величина и в данной ситуации 
	                     // не учитывается
	double *lightingParts; // заданные варианты деления времен цикла горения 
	                       // светофора. Это число (0,1).
	int numLightParts; // число выше описанных вариантов.
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