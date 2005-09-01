// Field.h: interface for the CField class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FIELD_H__70E3947F_EF12_4077_8419_ADE5CC077EB6__INCLUDED_)
#define AFX_FIELD_H__70E3947F_EF12_4077_8419_ADE5CC077EB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>
#include <vector>

class CMovingObject;

enum TrafficLightMode {Red, Green, Yellow};

class CField  
{
public:
	int GetNHumanLines();
	int GetNCarLines();
	int GetNumberOfHumans();
	int GetNumberOfCars();
    CField(int xmax, int ymax, float _PercentOfCarRoad = 0.4f, float _PercentOfPedestrianCrossing = 0.2f);
    virtual ~CField();

    void AddObject(CMovingObject *pObject);

    void Move();
    void Show();

    void SetTrafficLightForCars(TrafficLightMode _TMode) {TMode = _TMode;};
    TrafficLightMode GetTrafficLightForCars() {return TMode;};
    TrafficLightMode GetTrafficLightForHumans() 
    {
	switch (TMode)
	{
	    case Green: return Red; break;
	    case Yellow: return Yellow; break;
	    case Red: return Green; break;
	};
    };

    bool IsCellFree(int x, int y, int XLen, int YLen, CMovingObject* pObject = NULL);

    void GetTrafficLightBorders(int &LeftBorder, int &RightBorder, int &TopBorder, int &BottomBorder);

    float GetPercentOfPedestrianCrossing();
    float GetPercentOfCarRoad();
    int GetYMax();
    int GetXMax();
    void DeleteAllMO();

protected:
    
    float PercentOfCarRoad;
    float PercentOfPedestrianCrossing;

    CRITICAL_SECTION CS;
    std::list<CMovingObject*> ObjectsOnTheRoad;

    int m_XMax;
    int m_YMax;

    int NCarLines;
    int NHumanLines;

    //What color of traffic light for cars is on
    TrafficLightMode TMode;

    //First free lines
    std::vector<int> LeftMinLine;
    std::vector<int> RightMaxLine;

    std::vector<int> TopMaxLine;
    std::vector<int> BottomMinLine;

    int m_NumberOfHumans;
    int m_NumberOfCars;
};

#endif // !defined(AFX_FIELD_H__70E3947F_EF12_4077_8419_ADE5CC077EB6__INCLUDED_)
