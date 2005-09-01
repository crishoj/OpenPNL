// Nature.h: interface for the CNature class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NATURE_H__9A731400_5486_4777_B596_F1C985A84933__INCLUDED_)
#define AFX_NATURE_H__9A731400_5486_4777_B596_F1C985A84933__INCLUDED_

#include "OptimalTrafficLight.hpp"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CView;
class CInfoPanel; 

class CNature  
{
public:
	int GetCyclesPerDay();
	//note: hours are in 24 - hours format
	void GetTicsByTime(int HoursIn, int MinutesIn, int &TicsOut);
	void GetTimeByTics(int TicsIn, int &HoursOut, int &MinutesOut);
	
	void ProcessModel(CView *pEngine);

	CNature(float SpeedCarAppearance[2][5][3], float SpeedHumanAppearance[2][5][3], double TrafficLightVariants[5]);
	virtual ~CNature();

	void PauseProcessing() {m_Paused = true;};
	void StartProcessing() {m_Paused = false;};

	void SetInfoPanel(CInfoPanel *_pInfoPanel)
	{pInfoPanel = _pInfoPanel;};

protected:

    COptimalTrafficLight *m_pOptimalTLight;

    //This variable shows how many elementary cycles there are in a day
    int m_CyclesPerDay;

    int m_CyclesPerTrafficLight;

    TrafficLightMode TMode;

    //These arrays depend on day, time, weather
    float m_SpeedCarAppearance[2][5][3];
    float m_SpeedHumanAppearance[2][5][3];

    float m_TrafficLightVariants[5];

    bool m_Paused;

    CInfoPanel *pInfoPanel;

};

#endif // !defined(AFX_NATURE_H__9A731400_5486_4777_B596_F1C985A84933__INCLUDED_)
