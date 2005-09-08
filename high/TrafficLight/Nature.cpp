// Nature.cpp: implementation of the CNature class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrafficLight.h"
#include "TrafficLightDoc.h"
#include "Nature.h"
#include "CarStreamDisctrib.hpp"

#include "Car.h"
#include "Human.h"
#include "InfoPanel.h"

#include <stdlib.h>
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int cd[5] = {120,240,360,480,600};

CNature::CNature(float SpeedCarAppearance[2][5][3], float SpeedHumanAppearance[2][5][3], double TrafficLightVariants[5])
{
    m_CyclesPerDay = 12000;
    m_CyclesPerTrafficLight = 120;
    TMode = Green;

    for (int d = 0; d< 2; d++)
    {
	for (int t = 0; t < 5; t++)
	{
	    for (int w = 0; w < 3; w++)
	    {
		m_SpeedCarAppearance[d][t][w] = SpeedCarAppearance[d][t][w];
		m_SpeedHumanAppearance[d][t][w] = SpeedHumanAppearance[d][t][w];
	    };
	};
    };

    for (int v = 0; v< 5; v++){
	m_TrafficLightVariants[v] = TrafficLightVariants[v];;
    };

    m_Paused = false;
    pInfoPanel = NULL;

    m_pOptimalTLight = new COptimalTrafficLight(cd, 5, TrafficLightVariants, 5, 96, CCar::GetDefaultSpeed(), CHuman::GetDefaultSpeed(), 10, 0.2, 
	6, 3);
}

CNature::~CNature()
{
    delete m_pOptimalTLight;
}

namespace __Nature 
{
    int sign(float a)
    {
	if (a>0)
	{
	    return 1;
	};

	if (a==0)
	{
	    return 0;
	}
	else
	{
	    return -1;
	};
    };

    int roundSimple(float a)
    {

	if (fabs(a - (int)(a)) >= 0.999)
	{
	    return (int)(a)+1*sign(a);
	}
	else
	{
	    return (int)(a);
	};
    };

    int round(float a)
    {

	if (fabs(a - (int)(a)) >= 0.5)
	{
	    return (int)(a)+1*sign(a);
	}
	else
	{
	    return (int)(a);
	};
    };
};

using namespace __Nature;

void CNature::ProcessModel(CView *pEngine)
{
    CField *pField = static_cast<CTrafficLightDoc *>(pEngine->GetDocument())->GetField();

    m_pOptimalTLight->SetNumbersOfLines(pField->GetNCarLines(), pField->GetNHumanLines());

    CCarStreamDisctrib *pCarDistrib = new CCarStreamDisctrib();

    Days day = Monday;
    int time = 0;
    Weather weather = Sunny;
    TrafficLightType TLightType = Smart;

    int roadLen1 = pField->GetXMax()*(1-pField->GetPercentOfPedestrianCrossing()); 
    int roadLen2 = pField->GetYMax()*(1-pField->GetPercentOfCarRoad());

    m_pOptimalTLight->TeachYourSelf(m_SpeedHumanAppearance, m_SpeedCarAppearance, roadLen1, roadLen2);

    while (true)
    {
	if (m_Paused)
	{
	    while (m_Paused)
	    {
		Sleep(100);
	    };

	    pField->DeleteAllMO();

	    if (pInfoPanel != NULL)
	    {
		int Hours;
		int Minutes;
		day = pInfoPanel->GetDay();
		pInfoPanel->GetTime(Hours, Minutes);
		GetTicsByTime(Hours, Minutes, time);
		weather = pInfoPanel->GetWeather();
		TLightType = pInfoPanel->GetTrafficLightType();
	    }
	    else
	    {
		day = Monday;
		time = 0;
		weather = Sunny;
		TLightType = Smart;
	    };
	};

        for (; (day <= Sunday)&&(!m_Paused); day = static_cast<Days>(static_cast<int>(day) + 1))
        {
            float RedTime;
            float GreenTime;
            float YellowTime;

            int Counter;

            for (; (time < m_CyclesPerDay)&&(!m_Paused); time++, Counter++)
            {
		Times TypeOfTime;

		int Hours; 
		int Minutes;
		GetTimeByTics(time, Hours, Minutes);

		if ((Hours>=6)&&(Hours < 8))
		{
		    TypeOfTime = EarlyMorning;
		}
		else
		{
		    if ((Hours>=8)&&(Hours<11))
		    {
			TypeOfTime = Morning;
		    }
		    else
		    {
			if ((Hours>=11)&&(Hours<16))
			{
			    TypeOfTime = Afternoon;
			}
			else
			{
			    if ((Hours>=16)&&(Hours<21))
			    {
				TypeOfTime = Everning;
			    }
			    else
			    {
				TypeOfTime = Night;
			    };
			};
		    };
		};

                //Generate weather (obsolete, now it is setted by GUI)
                //if (time % (m_CyclesPerDay/5) == 0)
                //{
                //    weather = static_cast<Weather>(rand() % 3);
		//};
                int day1 = ((day!=Saturday)&&(day!=Sunday))?(Workday):(Holiday);

		if ((time % m_CyclesPerTrafficLight) == 0)
		{
		    if (TLightType == Ordinary)
		    {
			RedTime = m_CyclesPerTrafficLight * 0.4;
			GreenTime = m_CyclesPerTrafficLight * 0.4;
			YellowTime = m_CyclesPerTrafficLight * 0.2;
		    }
		    else
		    {
                        bool DayObserved;
                        bool TimeObserved;
                        bool WeatherObserved;

                        if (pInfoPanel != NULL)
                        {
                            pInfoPanel->GetObservation(DayObserved, TimeObserved, WeatherObserved);
                        };

                        m_pOptimalTLight->SetType(static_cast<DayType>(day1), TypeOfTime, weather);
                        m_pOptimalTLight->SetObserved(DayObserved, TimeObserved, WeatherObserved);

                        m_pOptimalTLight->Solve(-1,-1);
                        int Strategy = m_pOptimalTLight->GetOptValue();

                        GreenTime = m_CyclesPerTrafficLight*0.8*m_TrafficLightVariants[Strategy];
                        YellowTime = m_CyclesPerTrafficLight * 0.2;
                        RedTime = m_CyclesPerTrafficLight - GreenTime - YellowTime;
		    };

                    pInfoPanel->SetTrafficLightDistribution((float)GreenTime/m_CyclesPerTrafficLight, (float)RedTime/m_CyclesPerTrafficLight, (float)YellowTime/m_CyclesPerTrafficLight);

                    Counter = 0;

                    TMode = Green;
                };

                if (Counter >= GreenTime)
                {
                    TMode = Yellow;
                };

                if (Counter >= GreenTime + YellowTime/2)
                {
                    TMode = Red;
                };

                if (Counter >= GreenTime + YellowTime/2 + RedTime)
                {
                    TMode = Yellow;
                };

		CMovingObject *pObject;

#ifdef CONST_NUMBER_OF_MO
		for (int car = 0; car < static_cast<int>(m_SpeedCarAppearance[day1][TypeOfTime][weather]); car++)
		{
		    pObject = new CCar();
		    pField->AddObject(pObject);
		};

		if (rand() <= RAND_MAX*(m_SpeedCarAppearance[day1][TypeOfTime][weather] - static_cast<int>(m_SpeedCarAppearance[day1][TypeOfTime][weather])))
		{
		    pObject = new CCar();
		    pField->AddObject(pObject);
		};

		for (int human = 0; human < static_cast<int>(m_SpeedHumanAppearance[day1][TypeOfTime][weather]); human++)
		{
		    pObject = new CHuman();
		    pField->AddObject(pObject);
		};

		if (rand() <= RAND_MAX*(m_SpeedHumanAppearance[day1][TypeOfTime][weather] - static_cast<int>(m_SpeedHumanAppearance[day1][TypeOfTime][weather])))
		{
		    pObject = new CHuman();
		    pField->AddObject(pObject);
		};
#else
		pCarDistrib->SetLambda(m_SpeedCarAppearance[day1][TypeOfTime][weather]);

		int NNewCars = pCarDistrib->getPuassonValue();
		for (int car = 0; car < NNewCars; car++)
		{
		    pObject = new CCar();
		    pField->AddObject(pObject);
		};

		pCarDistrib->SetLambda(m_SpeedHumanAppearance[day1][TypeOfTime][weather]);

		int NNewHumans = pCarDistrib->getPuassonValue();
		for (int human = 0; human < NNewHumans; human++)
		{
		    pObject = new CHuman();
		    pField->AddObject(pObject);
		};
#endif

		
		pField->SetTrafficLightForCars(TMode);
                pField->Move();

		if (pInfoPanel != NULL)
		{
		    pInfoPanel->SetDay(day);
		    pInfoPanel->SetTime(Hours, Minutes);
		    pInfoPanel->SetNumbersOfCarsAndHumans(pField->GetNumberOfCars(), pField->GetNumberOfHumans());
                    pInfoPanel->SetSpeedOfCarsAndHumans(CCar::GetDefaultSpeed(), CHuman::GetDefaultSpeed());
                    pInfoPanel->SetSpeedOfCarAndHumanAppearance(m_SpeedCarAppearance[day1][TypeOfTime][weather], m_SpeedHumanAppearance[day1][TypeOfTime][weather]);
		    pInfoPanel->Invalidate(FALSE);
		};

		pEngine->Invalidate(FALSE);
	        Sleep(10);
            };

	    time = 0;
        };
	day = Monday;
    };

    delete pCarDistrib;
}

void CNature::GetTimeByTics(int TicsIn, int &HoursOut, int &MinutesOut)
{
    HoursOut = TicsIn / static_cast<int>(m_CyclesPerDay/24);
    MinutesOut = static_cast<int>(static_cast<float>(TicsIn - HoursOut*static_cast<int>(m_CyclesPerDay/24))/
	(static_cast<float>(m_CyclesPerDay)/24/60));
}

void CNature::GetTicsByTime(int HoursIn, int MinutesIn, int &TicsOut)
{
    TicsOut = HoursIn*static_cast<int>(m_CyclesPerDay/24)+
	MinutesIn*static_cast<int>(m_CyclesPerDay/24/60);
}

int CNature::GetCyclesPerDay()
{
    return m_CyclesPerDay;
}
