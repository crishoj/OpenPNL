// MovingObject.h: interface for the CMovingObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOVINGOBJECT_H__96B558E1_AFD0_4A10_BA5A_FB859CCF844D__INCLUDED_)
#define AFX_MOVINGOBJECT_H__96B558E1_AFD0_4A10_BA5A_FB859CCF844D__INCLUDED_

#include "SimpleTypes.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum Direction {XDirection, YDirection};
enum Sides {Left, Right, Top, Bottom};

class CMovingObject  
{
public:
    virtual MovingObjectTypes GetType() = 0;
    int GetSpeed();
    CMovingObject();
    virtual ~CMovingObject();

    int m_YLen;
    int m_XLen;

    int m_x;
    int m_y;

    int m_Line;

    int m_Speed;
    Direction m_Direction;

    Sides m_Side;

    virtual bool Move(CField *pField);
    bool MoveWithOutCheck(CField *pField);

    virtual void Show() = 0;
protected:

    virtual bool CanIGoForward(CField *pField) = 0;

};

#endif // !defined(AFX_MOVINGOBJECT_H__96B558E1_AFD0_4A10_BA5A_FB859CCF844D__INCLUDED_)
