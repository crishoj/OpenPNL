#include "OptimalTrafficLight.hpp"
#include "CarStreamDisctrib.hpp"

namespace __OptimalTrafficLight
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

COptimalTrafficLight::COptimalTrafficLight(int* carsdivision, int numcarsDivision,
										   double* lightParts, int numLightParts,
										   double intervalTime, double speedStreem1,
										   double speedStreem2,int learningtime, double PercentOfYellow,
										   int NumberOfStreams1, int NumberOfStreams2):
CTrafficLight(lightParts, numLightParts,
			  intervalTime, speedStreem1, speedStreem2)
{ 	
	curIter = 0;
	learningTime = learningtime;
	carsDivision = carsdivision;
	numCarsDivision = numcarsDivision;
	waitedS1 = 0;
	waitedS2 = 0;
	pNet = new BayesNet();
	
	// adding nodes
	pNet->AddNode("discrete^Day", "Workday Holiday");
	pNet->AddNode("discrete^Hour", "EarlyMorning Morning Afternoon Everning Night");
	pNet->AddNode("discrete^Weather", "Sunny Foggy Cloudy");
	pNet->AddNode("discrete^CarStreem1", "JustClear Small OneQuarter Normal ThreeQuarters Overload");
	pNet->AddNode("discrete^CarStreem2", "JustClear Small OneQuarter Normal ThreeQuarters Overload");
//	pNet->AddNode("discrete^SignalVar", "SmallRed OneQuarterRed Equal OneQuarterGreen SmallGreen");
	
	//adding arcs
	pNet->AddArc("Day Hour Weather", "CarStreem1");
	pNet->AddArc("Day Hour Weather", "CarStreem2");
//	pNet->AddArc("CarStreem1 CarStreem2", "SignalVar");

	m_PercentOfYellow = PercentOfYellow;

	m_NumberOfStreams1 = NumberOfStreams1;
	m_NumberOfStreams2 = NumberOfStreams2;

        m_DayObserved = true;
        m_TimeObserved = true;
        m_WeatherObserved = true;
}

COptimalTrafficLight::~COptimalTrafficLight()
{
	delete []carsDivision;
}

bool COptimalTrafficLight::IsLearning()
{
	if(curIter < learningTime)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void COptimalTrafficLight::SetType(DayType dtype,	Times htype,Weather wtype)
{
	dType = dtype;
	hType = htype;
	wType = wtype;
}

#define __NEW_ComputeOptSignal

void COptimalTrafficLight::ComputeOptSignal(int n1, int n2)
{
#ifndef __NEW_ComputeOptSignal
	int i;
	int bestVariant;
	double value = 1e10;
	double tmpval;
	double stream1, opts1;
	double stream2, opts2;
	double maxval;

	String s;

	for( i = 0; i < numLightParts; i++)
	{
		tmpval = n1 + n2;
		stream1 = n1;
		stream2 = n2;
		if(intervalTime * lightingParts[i] * carSpeed1 > n1)
		{
			tmpval -= n1;
			stream1 -=n1;
		}
		else
		{
			tmpval -= intervalTime * lightingParts[i] * carSpeed1;
			stream1 -= intervalTime * lightingParts[i] * carSpeed1;;
		};
		if(intervalTime * (1 - lightingParts[i]) * carSpeed2 > n2)
		{
			tmpval -= n2;
			stream2 -= n2;
		}
		else
		{
			tmpval -= intervalTime * (1 - lightingParts[i]) * carSpeed2;
			stream2 -= intervalTime * (1 - lightingParts[i]) * carSpeed2;
		};
	/*	if(value >= tmpval)
		{
			bestVariant = i;
			value = tmpval;
			opts1 = stream1;
			opts2 = stream2;
		}*/
		if(stream1 >= stream2)
		{
			maxval = stream1;
		}
		else
		{
			maxval = stream2;
		}
		if(value >= maxval)
		{
			bestVariant = i;
			value = maxval;
			opts1 = stream1;
			opts2 = stream2;
		}
		
	}
	optimalValue = bestVariant;
	waitedCars = value;
	waitedS1 = opts1;
	waitedS2 = opts2;

/*	switch(bestVariant)
	{
	case 0:s =  "SmallRed"; break;
	case 1:s =  "OneQuarterRed"; break;
	case 2:s =  "Equal"; break;
	case 3:s =  "OneQuarterGreen"; break;
	case 4:s =  "SmallGreen"; break;
	};
	
	return s;*/
#else
	int i;
	int bestVariant;
	double value = 1e10;
	double tmpval;
	double stream1, opts1;
	double stream2, opts2;
	double maxval;

	String s;

	//This time consists of green, yellow and red times
	int FullIntervalTime = intervalTime/(1-m_PercentOfYellow);
	
	for( i = 0; i < numLightParts; i++)
	{
		tmpval = n1 + n2;
		stream1 = n1;
		stream2 = n2;
		
		//speeds of moving objects shrunk because the streams are directly proportional to
		//the speed and inversely proportional to the size of the object. 
		//And the size is equal to the speed...

		if(intervalTime * lightingParts[i] * m_NumberOfStreams1*2 > n1)
		{
			tmpval -= n1;
			stream1 -=n1;
		}
		else
		{
			tmpval -= intervalTime * lightingParts[i] * m_NumberOfStreams1*2;
			stream1 -= intervalTime * lightingParts[i] * m_NumberOfStreams1*2;
		};
		if(intervalTime * (1 - lightingParts[i]) * m_NumberOfStreams2*2 > n2)
		{
			tmpval -= n2;
			stream2 -= n2;
		}
		else
		{
			tmpval -= intervalTime * (1 - lightingParts[i]) * m_NumberOfStreams2*2;
			stream2 -= intervalTime * (1 - lightingParts[i]) * m_NumberOfStreams2*2;
		};

		if(stream1 >= stream2)
		{
			maxval = stream1;
		}
		else
		{
			maxval = stream2;
		}
		if(value >= maxval)
		{
			bestVariant = i;
			value = maxval;
			opts1 = stream1;
			opts2 = stream2;
		}
		
	}
	optimalValue = bestVariant;
	waitedCars = value;
	waitedS1 = opts1;
	waitedS2 = opts2;
#endif
}

void COptimalTrafficLight::SetSignalsTime(String signal)
{
	int i;
	bool flag = false;
	String tmpStr;
	for(i = 0; i < signal.length(); i++)
	{
		if(flag == true)
		{
			char s = signal[i];
			tmpStr.append(&s,1);
		}
		if(signal[i] == '^')
		{
			flag = true;
		}
		
	}
		if(tmpStr == "SmallRed") 
	{
		i = 0; flag = false;
	}
	if(tmpStr == "OneQuarterRed")
	{
		i = 1; flag = false;
	}
	if(tmpStr == "Equal")
	{
		i = 2; flag = false;
	};
	if(tmpStr == "OneQuarterGreen")
	{
		i = 3; flag = false;
	}
	if(tmpStr == "SmallGreen")
	{
		i = 4; flag = false;
	}
	
	if(flag == false)
	{
		optimalValue = i;
	}
	else
	{
		optimalValue = -1;
	}
}


int COptimalTrafficLight::GetCarValue(String nodeval)
{

	int i;
	bool flag = false;
	String tmpStr;
	for(i = 0; i < nodeval.length(); i++)
	{
		if(flag == true)
		{
			char s = nodeval[i];
			tmpStr.append(&s,1);
		}
		if(nodeval[i] == '^')
		{
			flag = true;
		}
		
	}
		
	if(tmpStr == "JustClear") return carsDivision[0];
	if(tmpStr == "Small") return carsDivision[1];
	if(tmpStr == "OneQuarter") return carsDivision[2];
	if(tmpStr == "Normal") return carsDivision[3];
	if(tmpStr == "ThreeQuarters") return carsDivision[4];
	if(tmpStr == "Overload") return (carsDivision[4] +420);

}

String COptimalTrafficLight::GetDayValue()
{
	String str;
	switch(dType)
	{
	case Workday:
		str = "Workday";
		break;
	case Holiday:
		str = "Holiday";
		break;
	}
	return str;
}
String COptimalTrafficLight::GetHourValue()
{
	String str;
	switch(hType)
	{
	case EarlyMorning:
		str = "EarlyMorning";
		break;
	case Morning:
		str = "Morning";
		break;
	case Afternoon:
		str = "Afternoon";
		break;
	case Everning:
		str = "Everning";
		break;
	case Night:
		str = "Night";
		break;
	}
	return str;
}
String COptimalTrafficLight::GetWeatherValue()
{
	String str;
	switch(wType)
	{
	case Sunny:
		str = "Sunny";
		break;
	case Foggy:
		str = "Foggy";
		break;
	case Cloudy:
		str = "Cloudy";
		break;
	}
	return str;
}
String COptimalTrafficLight::GetCarValue(int numCars)
{
	String s;
	if(numCars < carsDivision[0]) s = "JustClear";
	if(carsDivision[0] <= numCars && numCars < carsDivision[1]) s = "Small";
	if(carsDivision[1] <= numCars && numCars < carsDivision[2]) s = "OneQuarter";
	if(carsDivision[2] <= numCars && numCars < carsDivision[3]) s = "Normal";
	if(carsDivision[3] <= numCars && numCars < carsDivision[4]) s = "ThreeQuarters";
	if(numCars >= carsDivision[4]) s = "Overload";
	return s;
	
}

void COptimalTrafficLight::AddingNewEvidence(int numCars1, int numCars2)
{
        pNet->ClearEvid();
	String dVal = GetDayValue();
	String hVal = GetHourValue();
	String wVal = GetWeatherValue();
	String tmpStr;
	if(numCars1 != -1 )
	{
		String stream1Val = GetCarValue(numCars1);
		tmpStr << "CarStreem1^"<< stream1Val;
		pNet->EditEvidence(tmpStr);
		tmpStr = "";
	}
	if(numCars2 != -1 )
	{
		String stream2Val = GetCarValue(numCars2);
		tmpStr <<"CarStreem2^"<< stream2Val;
		pNet->EditEvidence(tmpStr);
		tmpStr = "";
	}
	/*if(signal != "UNKNOWN")
	{
		String sVal = signal;
		tmpStr << "SignalVar^"<<sVal;
		pNet->EditEvidence(tmpStr);
		tmpStr = "";
	}*/
	
	tmpStr <<"Day^"<<dVal;
	if (m_DayObserved)
        {
            pNet->EditEvidence(tmpStr);
        };
	tmpStr = "";
	tmpStr <<"Hour^"<<hVal;
        if (m_TimeObserved)
        {
	    pNet->EditEvidence(tmpStr);
        };
	tmpStr = "";
	tmpStr <<"Weather^"<<wVal;
        if (m_WeatherObserved)
        {
	    pNet->EditEvidence(tmpStr);
        };
	tmpStr = "";
	pNet->CurEvidToBuf();
}

void COptimalTrafficLight::Solve(int numCarsRoad1, int numCarsRoad2)
{
/*	if(IsLearning())
	{
		ComputeOptSignal(numCarsRoad1,numCarsRoad2);
		// SetSignalsTime(signal);
		AddingNewEvidence(numCarsRoad1,numCarsRoad2);
		curIter++;
	}
	else
	{
		if(learningTime == curIter)
		{
			pNet->LearnParameters();
			pNet->SaveNet("TrafficNet.csv");
		}*/
		//pNet->LoadNet("TrafficNet.csv");
		AddingNewEvidence();
	//	("UNKNOWN");
	//	pNet->CurEvidToBuf();
#ifdef NEW_IMPLEMENTATION
		TokArr stream1 = pNet->GetMPE("CarStreem1");
		TokArr stream2 = pNet->GetMPE("CarStreem2");
		int val1 = GetCarValue(String(stream1));
		int val2 = GetCarValue(String(stream2));
		ComputeOptSignal(val1,val2);
		/*AddingNewEvidence("UNKNOWN");
		String tmpStr;
		tmpStr << "CarStreem1^"<< val1;
		pNet->EditEvidence(tmpStr);
		tmpStr = "";
		tmpStr << "CarStreem2^"<< val2;
		pNet->EditEvidence(tmpStr);
		tmpStr = "";
		pNet->CurEvidToBuf();*/
#endif
	/*	TokArr mpeRes = pNet->GetMPE("SignalVar");
		String signal = String(mpeRes);*/
	//	SetSignalsTime(signal);
		if(numCarsRoad1 != -1 && numCarsRoad2 != -1)
		{
			
			waitedS1 = numCarsRoad1;
			waitedS2 = numCarsRoad2;
			if(intervalTime * lightingParts[optimalValue] * carSpeed1 > numCarsRoad1)
			{
				waitedS1 -= numCarsRoad1;
			}
			else
			{;
			waitedS1 -= intervalTime * lightingParts[optimalValue] * carSpeed1;;
			};
			if(intervalTime * (1 - lightingParts[optimalValue]) * carSpeed2 > numCarsRoad2)
			{
				
				waitedS2 -= numCarsRoad2;
			}
			else
			{
				waitedS2 -= intervalTime * (1 - lightingParts[optimalValue]) * carSpeed2;
			};
			
		}
		curIter++;	
//	};
}

#define __NEW_TeachYourSelf

void COptimalTrafficLight::TeachYourSelf(float SpeedHumanAppearance[2][5][3], float SpeedCarAppearance[2][5][3], int roadLen1, int roadLen2)
{
    using namespace __OptimalTrafficLight;
    curIter = 0;

#ifndef __NEW_TeachYourSelf
    double MaxCarSpeed;
    int LenghtOfTheRoad;
    int NumberOfCycles;

    if (carSpeed1 > carSpeed2)
    {
        MaxCarSpeed = carSpeed1;
        LenghtOfTheRoad = roadLen1;
    }
    else
    {
        MaxCarSpeed = carSpeed2;
        LenghtOfTheRoad = roadLen2;
    };

    NumberOfCycles = round(static_cast<double>(LenghtOfTheRoad)/MaxCarSpeed);

    for (int d = 0; d < 2; d++)
    {
        for (int t = 0; t < 5; t++)
        {
            for (int weather = 0; weather < 3; weather++)
            {
                SetType(static_cast<DayType>(d), static_cast<Times>(t), static_cast<Weather>(weather));
                
                int NumberOfCars = round(SpeedCarAppearance[d][t][weather] * NumberOfCycles);
                int NumberOfHumans = round(SpeedHumanAppearance[d][t][weather] * NumberOfCycles);

                AddingNewEvidence(NumberOfCars, NumberOfHumans);
            };
        };
    };
#else
    int FullCycleLength = round(static_cast<float>(intervalTime / (1 - m_PercentOfYellow)));
    CCarStreamDisctrib DistribCars, DistribHumans;

    for (int d = 0; d < 7; d++)
    {
        for (int t = 0; t < 5; t++)
        {
            for (int weather = 0; weather < 3; weather++)
            {
		int d1;

		if ((d>=0)&&(d<=4))
		{
		    d1 = 0;
		}
		else
		{
		    d1 = 1;
		};

		for (int c = 0; c < learningTime; c++)
		{
		    SetType(static_cast<DayType>(d1), static_cast<Times>(t), static_cast<Weather>(weather));

		    int NumberOfCars = 0;
		    int NumberOfHumans = 0;

		    DistribCars.SetLambda(SpeedCarAppearance[d1][t][weather]);
		    DistribHumans.SetLambda(SpeedHumanAppearance[d1][t][weather]);

		    //for (int l = 0; l < FullCycleLength; l++)
		    //{             
			//NumberOfCars += DistribCars.getPuassonValue();
			//NumberOfHumans += DistribHumans.getPuassonValue();
		    //};

                    NumberOfCars += FullCycleLength * SpeedCarAppearance[d1][t][weather];
                    NumberOfHumans += FullCycleLength * SpeedHumanAppearance[d1][t][weather];

		    AddingNewEvidence(NumberOfCars, NumberOfHumans);
		    curIter++;
		};
            };
        };
    };

    pNet->LearnParameters();
    pNet->ClearEvid();
#endif
}

void COptimalTrafficLight::SetNumbersOfLines(int NLines1, int NLines2)
{
    m_NumberOfStreams1 = NLines1;
    m_NumberOfStreams2 = NLines2;
}

void COptimalTrafficLight::SetObserved(bool Day, bool Time, bool Weather)
{
    m_DayObserved = Day;
    m_TimeObserved = Time;
    m_WeatherObserved = Weather;
}
