// Car.cpp: implementation of the CCar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrafficLight.h"
#include "Car.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCar::CCar()
{
    m_Direction = XDirection;
    m_XLen = 2;
    m_YLen = 1;
    m_Speed = 3;
}

CCar::~CCar()
{

}

void CCar::Show()
{
    glPushMatrix();
	glColor3f(0.0,0.0,0.0);
        glTranslated(m_x+0.5, m_y+0.5,0.5);
        auxSolidCube(1);
        glTranslated(1, 0,0);
        auxSolidCube(1);
    glPopMatrix();
}

bool CCar::CanIGoForward(CField *pField)
{
    int LeftBorder;
    int RightBorder;
    int TopBorder;
    int BottomBorder;

    pField->GetTrafficLightBorders(LeftBorder, RightBorder, TopBorder, BottomBorder);

    if (pField->GetTrafficLightForCars() == Green)
    {
	return true;
    };

    if (pField->GetTrafficLightForCars() == Yellow)
    {
	if (m_Side == Left)
	{
	    if ((m_x+m_XLen < LeftBorder)&&((m_x+m_XLen >= LeftBorder)||(m_x+m_XLen+m_Speed >= LeftBorder)))
	    {
		return false;
	    }
	}
	else
	{
	    if ((m_x > RightBorder)&&((m_x <= RightBorder)||(m_x-m_Speed <= RightBorder)))
	    {
		return false;
	    };
	};
    };

    if (pField->GetTrafficLightForCars() == Red)
    {
	if (m_Side == Left)
	{
	    if ((m_x < RightBorder)&&((m_x+m_XLen >= LeftBorder)||(m_x+m_XLen+m_Speed >= LeftBorder)))
	    {
		return false;
	    }
	}
	else
	{
	    if ((m_x+m_XLen > LeftBorder)&&((m_x <= RightBorder)||(m_x-m_Speed <= RightBorder)))
	    {
		return false;
	    };
	};
    };

    return true;
}

int CCar::GetDefaultSpeed()
{
    return 3;
}
