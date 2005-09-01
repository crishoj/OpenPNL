// Human.cpp: implementation of the CHuman class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrafficLight.h"
#include "Human.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHuman::CHuman()
{
    m_Direction = YDirection;
    m_XLen = 1;
    m_YLen = 1;
    m_Speed = 3;
}

CHuman::~CHuman()
{

}

void CHuman::Show()
{
    glPushMatrix();
	glColor3f(0.0,0.0,2.0);
        glTranslated(m_x+0.5, m_y+0.5,0.5);
        auxSolidCube(1);
        glTranslated(0, 0,1);
        auxSolidCube(1);
    glPopMatrix();
}

bool CHuman::CanIGoForward(CField *pField)
{
    int LeftBorder;
    int RightBorder;
    int TopBorder;
    int BottomBorder;

    pField->GetTrafficLightBorders(LeftBorder, RightBorder, TopBorder, BottomBorder);

    if (pField->GetTrafficLightForHumans() == Green)
    {
	return true;
    };

    if (pField->GetTrafficLightForHumans() == Yellow)
    {
	if (m_Side == Bottom)
	{
	    if ((m_y+m_YLen < BottomBorder)&&((m_y+m_YLen>=BottomBorder)||(m_y+m_YLen+m_Speed>=BottomBorder)))
	    {
		return false;
	    }
	}
	else
	{
	    if ((m_y > TopBorder)&&((m_y<=TopBorder)||(m_y-m_Speed<=TopBorder)))
	    {
		return false;
	    };
	};
    };

    if (pField->GetTrafficLightForHumans() == Red)
    {
	if (m_Side == Bottom)
	{
	    if ((m_y < TopBorder)&&((m_y+m_YLen>=BottomBorder)||(m_y+m_YLen+m_Speed>=BottomBorder)))
	    {
		return false;
	    }
	}
	else
	{
	    if ((m_y+m_YLen > BottomBorder)&&((m_y<=TopBorder)||(m_y-m_Speed<=TopBorder)))
	    {
		return false;
	    };
	};
    };

    return true;
}

int CHuman::GetDefaultSpeed()
{
    return 3;
}
