// MovingObject.cpp: implementation of the CMovingObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrafficLight.h"
#include "MovingObject.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMovingObject::CMovingObject()
{

}

CMovingObject::~CMovingObject()
{

}

bool CMovingObject::Move(CField *pField)
{
    bool DidIMove = true;

    int x2;
    int y2;

    if (CanIGoForward(pField))
    {
	switch (m_Side)
	{
	case Left:
	    x2 = m_x+((m_Speed>=1)?(m_Speed):(1));

	    if (pField->IsCellFree(x2, m_y, m_XLen, m_YLen, this))
	    {
		m_x = x2;
	    }
	    else
	    {
		DidIMove = false;
	    };
	    break;
    
	case Bottom:
	    y2 = m_y+((m_Speed>=1)?(m_Speed):(1));

	    if (pField->IsCellFree(m_x, y2, m_XLen, m_YLen, this))
	    {
		m_y = y2;
	    }
	    else
	    {
		DidIMove = false;
	    };
	    break;

	case Right:
	    x2 = m_x-((m_Speed>=1)?(m_Speed):(1));

	    if (pField->IsCellFree(x2, m_y, m_XLen, m_YLen, this))
	    {
		m_x = x2;
	    }
	    else
	    {
		DidIMove = false;
	    };
	    break;

	case Top:
	    y2 = m_y-((m_Speed>=1)?(m_Speed):(1));

	    if (pField->IsCellFree(m_x, y2, m_XLen, m_YLen, this))
	    {
		m_y = y2;
	    }
	    else
	    {
		DidIMove = false;
	    };
	    break;
	};
    }
    else
    {
	DidIMove = false;
    };

    return DidIMove;
}

int CMovingObject::GetSpeed()
{
    return m_Speed;
}

bool CMovingObject::MoveWithOutCheck(CField *pField)
{
    bool DidIMove = true;

    if (CanIGoForward(pField))
    {
	switch (m_Side)
	{
	case Left:
	    m_x = m_x+((m_Speed>=1)?(m_Speed):(1)); 
	    break;

	case Bottom:
	    m_y = m_y+((m_Speed>=1)?(m_Speed):(1));
	    break;

	case Right:
	    m_x = m_x-((m_Speed>=1)?(m_Speed):(1));   
	    break;

	case Top:
	    m_y = m_y-((m_Speed>=1)?(m_Speed):(1));
	    break;
	};
    }
    else
    {
	DidIMove = false;
    };

    return DidIMove;
}
