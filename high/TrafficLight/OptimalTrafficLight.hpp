#ifndef OPTTRAFFICLIGHT
#define OPTTRAFFICLIGHT

#include "TrafficLightSimple.hpp"
#include "pnlHighConf.hpp"
#include "SimpleTypes.h"

#define NUM_LIGHT_PARTS 5

#define NEW_IMPLEMENTATION

PNLW_USING

class COptimalTrafficLight: public CTrafficLight
{
protected:
	
	int learningTime; // число итераций обучения
	int curIter; // текущая итерация
	BayesNet *pNet; // Байесова сеть, соответствующая данному светофору.
	DayType dType;
	Times hType;
	Weather wType;
	int optimalValue; // номер элемента из массива lightingParts
	double waitedCars; // число машин оставшихся у перекрестка после данной итерации у светофора
	double waitedS1;
	double waitedS2;


	void ComputeOptSignal(int numCarsRoad1,int numCarsRoad2);
	void  SetSignalsTime(String signal);
	void AddingNewEvidence(int numCars1 = -1, int numCars2 = -1);
	String GetDayValue();
	String GetHourValue();
	String GetWeatherValue();
	String GetCarValue(int numCars);
	bool IsLearning();
	int* carsDivision; // массив из значений колличеств автомобилей для формирования значений вершин: потоки машин.
	int numCarsDivision; // число значений вершины
	double m_PercentOfYellow;

	int m_NumberOfStreams1;
	int m_NumberOfStreams2;

        bool m_DayObserved;
        bool m_TimeObserved;
        bool m_WeatherObserved;
public:
	void SetObserved(bool Day, bool Time, bool Weather);
	void SetNumbersOfLines(int NLines1, int NLines2);
    COptimalTrafficLight(int* carsdivision, int numcarsDivision,
		double* lightParts, int numLightParts,double intervalTime,
		double speedStreem1, double speedStreem2,int learningtime, double PercentOfYellow, 
		int NumberOfStreams1, int NumberOfStreams2);
	~COptimalTrafficLight();
	int GetOptValue() {return optimalValue;};
	int	 GetCarValue(String nodeval);
	void SetType(DayType dtype,Times htype,Weather wtype);
	virtual void Solve(int numCarsRoad1, int numCarsRoad2);
	double GetWS1(){return waitedS1;};
	double GetWS2(){return waitedS2;};

    void TeachYourSelf(float SpeedHumanAppearance[2][5][3], float SpeedCarAppearance[2][5][3], int roadLen1, int roadLen2);
};

#endif