// Field.cpp: implementation of the CField class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrafficLight.h"
#include "Field.h"
#include "MovingObject.h"
#include <math.h>
#include <iostream>
#include <windows.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace __Field
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
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CField::CField(int xmax, int ymax, float _PercentOfCarRoad, float _PercentOfPedestrianCrossing):
					LeftMinLine(0), RightMaxLine(0), TopMaxLine(0), BottomMinLine(0)
{
    PercentOfCarRoad = _PercentOfCarRoad;
    PercentOfPedestrianCrossing = _PercentOfPedestrianCrossing;

    ObjectsOnTheRoad.clear();

    m_XMax = xmax;
    m_YMax = ymax;

    NCarLines = 6; 
    NHumanLines = 3;

    LeftMinLine.resize(NCarLines);
    RightMaxLine.resize(NCarLines);

    int i;
    for (i =0; i < NCarLines; i++)
    {
	LeftMinLine[i] = -xmax;
	RightMaxLine[i] = xmax;
    };

    TopMaxLine.resize(NHumanLines);
    BottomMinLine.resize(NHumanLines);

    for (i =0; i < NHumanLines; i++)
    {
	TopMaxLine[i] = ymax;
	BottomMinLine[i] = -xmax;
    };

    TMode = Red;

    InitializeCriticalSection(&CS);

    m_NumberOfHumans = 0;
    m_NumberOfCars = 0;
}

CField::~CField()
{
    DeleteCriticalSection(&CS);
}

void CField::Show()
{
    using namespace __Field;

    GLfloat front_color[] = {1,0,0,1};

    int xmax = m_XMax;
    int ymax = m_YMax;

    //Paint the roads
    glLineWidth(4);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3d(-xmax,-ymax,0);
	glVertex3d(-xmax*PercentOfPedestrianCrossing,-ymax,0);
	glVertex3d(-xmax*PercentOfPedestrianCrossing,-ymax*PercentOfCarRoad,0);
	glVertex3d(-xmax,-ymax*PercentOfCarRoad,0);
    glEnd();

    glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3d(xmax*PercentOfPedestrianCrossing,-ymax,0);
	glVertex3d(xmax,-ymax,0);
	glVertex3d(xmax,-ymax*PercentOfCarRoad,0);
	glVertex3d(xmax*PercentOfPedestrianCrossing,-ymax*PercentOfCarRoad,0);
    glEnd();


    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3d(xmax*PercentOfPedestrianCrossing,+ymax*PercentOfCarRoad,0);
	glVertex3d(xmax,+ymax*PercentOfCarRoad,0);
	glVertex3d(xmax,ymax,0);
	glVertex3d(xmax*PercentOfPedestrianCrossing,ymax,0);
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3d(-xmax,ymax*PercentOfCarRoad,0);
	glVertex3d(-xmax*PercentOfPedestrianCrossing,+ymax*PercentOfCarRoad,0);
	glVertex3d(-xmax*PercentOfPedestrianCrossing,ymax,0);
	glVertex3d(-xmax,ymax,0);
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3d(-xmax,-ymax,0);
	glVertex3d(-xmax,ymax,0);
	glVertex3d(xmax,ymax,0);
	glVertex3d(xmax,-ymax,0);
    glEnd();

    //the road
    glBegin(GL_QUADS);
	glColor3f(0.58f, 0.58f, 0.58f);
	glVertex3d(-xmax,+ymax*PercentOfCarRoad,0);
	glVertex3d(-xmax,-ymax*PercentOfCarRoad,0);
	glVertex3d(xmax,-ymax*PercentOfCarRoad,0);
	glVertex3d(xmax,+ymax*PercentOfCarRoad,0);
    glEnd();

    //demarcation strip
    glBegin(GL_QUADS);
	glColor3f(2.0f, 2.0f, 2.0f);
	glVertex3d(-xmax,+0.25+0.5,0.1);
	glVertex3d(-xmax,-0.25+0.5,0.1);
	glVertex3d(-xmax*PercentOfPedestrianCrossing-1,-0.25+0.5,0.1);
	glVertex3d(-xmax*PercentOfPedestrianCrossing-1,+0.25+0.5,0.1);
    glEnd();

    glBegin(GL_QUADS);
	glColor3f(2.0f, 2.0f, 2.0f);
	glVertex3d(xmax*PercentOfPedestrianCrossing+1,+0.25+0.5,0.1);
	glVertex3d(xmax*PercentOfPedestrianCrossing+1,-0.25+0.5,0.1);
	glVertex3d(xmax,-0.25+0.5,0.1);
	glVertex3d(xmax,+0.25+0.5,0.1);
    glEnd();
    //end of demarcation strip

    int LineWidth = round(static_cast<float>(m_XMax*PercentOfCarRoad)/NCarLines);; 

    for (int line = - (NCarLines-1); line <= (NCarLines-1); line++)
    {
	glBegin(GL_QUADS);
	    glColor3f(2.0f, 2.0f, 2.0f);
	    glVertex3d(-xmax*PercentOfPedestrianCrossing, line*LineWidth+1,0.1);
	    glVertex3d(-xmax*PercentOfPedestrianCrossing, line*LineWidth,0.1);
	    glVertex3d(xmax*PercentOfPedestrianCrossing, line*LineWidth,0.1);
	    glVertex3d(xmax*PercentOfPedestrianCrossing, line*LineWidth+1,0.1);
	glEnd();
    };

    glBegin(GL_QUADS);
	glColor3f(0.58f, 0.58f, 0.58f);
	glVertex3d(-xmax*PercentOfPedestrianCrossing,-ymax,0);
	glVertex3d(xmax*PercentOfPedestrianCrossing,-ymax,0);
	glVertex3d(xmax*PercentOfPedestrianCrossing,ymax,0);
	glVertex3d(-xmax*PercentOfPedestrianCrossing,ymax,0);
    glEnd();
    //end the road

    //Paint traffic lights
 
    GLUquadricObj *quadObj;
    quadObj = gluNewQuadric();
    gluQuadricDrawStyle(quadObj, GLU_FILL);
    glPushMatrix();
        glColor3f(0,0,0);
        glTranslated(-xmax*PercentOfPedestrianCrossing-3, -ymax*PercentOfCarRoad-3, 0);
        gluCylinder(quadObj, 2, 2, 1, 10, 10);
        glTranslated(0,0,1);
        gluDisk(quadObj, 0, 2, 15, 15);
        gluCylinder(quadObj, 0.5, 0.5, 5, 10, 10);
        glTranslated(0,0,5);
        gluDisk(quadObj, 0, 0.5, 5, 5);

	//Two black squares on the top and on the bottom of traffic light cube
	glPushMatrix();
	    glTranslated(0,0,-1);
	    glBegin(GL_QUADS);
	        glColor3f(0,0,0);
		glVertex3d(-1,-1,0);
		glVertex3d(-1,1,0);
		glVertex3d(1,1,0);
		glVertex3d(1,-1,0);
	    glEnd();

	    glTranslated(0,0,2);
	    glBegin(GL_QUADS);
	        glColor3f(0,0,0);
		glVertex3d(-1,-1,0);
		glVertex3d(1,-1,0);
		glVertex3d(1,1,0);
		glVertex3d(-1,1,0);
	    glEnd();

	glPopMatrix();

	switch (TMode)
	{
	case Red:
    	    front_color[0] = 1; //red color
	    front_color[1] = 0;
	    break;
	case Green:
	    front_color[0] = 0;
	    front_color[1] = 1; //green color
	    break;
	case Yellow:
	    //yellow
	    front_color[0] = 1;
	    front_color[1] = 1;
	    break;
	};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, front_color);
	glBegin(GL_QUADS);
	    glVertex3d(-1,-1,-1);
	    glVertex3d(-1,-1,1);
	    glVertex3d(-1,1,1);
	    glVertex3d(-1,1,-1);
	glEnd();
	glBegin(GL_QUADS);
	    glVertex3d(1,-1,-1);
	    glVertex3d(1,1,-1);
	    glVertex3d(1,1,1);
	    glVertex3d(1,-1,1);
	glEnd();

	switch (TMode)
	{
	case Red:
	    front_color[0] = 0;
	    front_color[1] = 1; //green color
	    break;
	case Green:
	    front_color[0] = 1; //red color
	    front_color[1] = 0;
	    break;
	case Yellow:
	    front_color[0] = 1;
	    front_color[1] = 1;
	    break;
	};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, front_color);
	glBegin(GL_QUADS);
	    glVertex3d(-1,-1,-1);
	    glVertex3d(1,-1,-1);
	    glVertex3d(1,-1,1);
	    glVertex3d(-1,-1,1);
	glEnd();
	glBegin(GL_QUADS);
	    glVertex3d(-1,1,-1);
	    glVertex3d(-1,1,1);
	    glVertex3d(1,1,1);
	    glVertex3d(1,1,-1);
	glEnd();

    glPopMatrix();

    front_color[0] = front_color[1] = front_color[2] = 0;
    glMaterialfv(GL_FRONT, GL_EMISSION, front_color);

    glPushMatrix();
        glColor3f(0,0,0);
        glTranslated(xmax*PercentOfPedestrianCrossing+3, +ymax*PercentOfCarRoad+3, 0);
        gluCylinder(quadObj, 2, 2, 1, 10, 10);
        glTranslated(0,0,1);
        gluDisk(quadObj, 0, 2, 15, 15);
        gluCylinder(quadObj, 0.5, 0.5, 5, 10, 10);
        glTranslated(0,0,5);
        gluDisk(quadObj, 0, 0.5, 5, 5);
	//Two black squares on the top and on the bottom of traffic light cube
	glPushMatrix();
	    glTranslated(0,0,-1);
	    glBegin(GL_QUADS);
	        glColor3f(0,0,0);
		glVertex3d(-1,-1,0);
		glVertex3d(-1,1,0);
		glVertex3d(1,1,0);
		glVertex3d(1,-1,0);
	    glEnd();

	    glTranslated(0,0,2);
	    glBegin(GL_QUADS);
	        glColor3f(0,0,0);
		glVertex3d(-1,-1,0);
		glVertex3d(1,-1,0);
		glVertex3d(1,1,0);
		glVertex3d(-1,1,0);
	    glEnd();

	glPopMatrix();

	switch (TMode)
	{
	case Red:
    	    front_color[0] = 1; //red color
	    front_color[1] = 0;
	    break;
	case Green:
	    front_color[0] = 0;
	    front_color[1] = 1; //green color
	    break;
	case Yellow:
	    //yellow
	    front_color[0] = 1;
	    front_color[1] = 1;

	    break;
	};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, front_color);
	glBegin(GL_QUADS);
	    glVertex3d(-1,-1,-1);
	    glVertex3d(-1,-1,1);
	    glVertex3d(-1,1,1);
	    glVertex3d(-1,1,-1);
	glEnd();
	glBegin(GL_QUADS);
	    glVertex3d(1,-1,-1);
	    glVertex3d(1,1,-1);
	    glVertex3d(1,1,1);
	    glVertex3d(1,-1,1);
	glEnd();

	switch (TMode)
	{
	case Red:
	    front_color[0] = 0;
	    front_color[1] = 1; //green color
	    break;
	case Green:
    	    front_color[0] = 1; //red color
	    front_color[1] = 0;
	    break;
	case Yellow:
	    front_color[0] = 1;
	    front_color[1] = 1;
	    break;
	};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, front_color);
	glBegin(GL_QUADS);
	    glVertex3d(-1,-1,-1);
	    glVertex3d(1,-1,-1);
	    glVertex3d(1,-1,1);
	    glVertex3d(-1,-1,1);
	glEnd();
	glBegin(GL_QUADS);
	    glVertex3d(-1,1,-1);
	    glVertex3d(-1,1,1);
	    glVertex3d(1,1,1);
	    glVertex3d(1,1,-1);
	glEnd();
    glPopMatrix();


    //black
    front_color[0] = front_color[1] = front_color[2] = 0;
    glMaterialfv(GL_FRONT, GL_EMISSION, front_color);

    gluDeleteQuadric(quadObj);

    auxSolidCube(0.01);

    EnterCriticalSection(&CS);
    std::list<CMovingObject*>::iterator it = ObjectsOnTheRoad.begin();

    while (it != ObjectsOnTheRoad.end())
    {
        (*it)->Show();
        it++;
    };
    LeaveCriticalSection(&CS);
}

void CField::AddObject(CMovingObject *pObject)
{
    using namespace __Field;

    Sides side;
    int NLine;

    static int IterationCounter =0;

    static int NLeft = 0;
    static int NRight = 0;

    if (!IterationCounter)
    {
	srand(time(NULL));
	IterationCounter++;
    };

    if (pObject->m_Direction == XDirection)
    {
	side = static_cast<Sides>((rand()) % 2);
	    if (side == Left)
		NLeft++;
	    else
		NRight ++;

	NLine = rand() % NCarLines;
    }
    else
    {
	side = static_cast<Sides>(static_cast<int>(Top) + rand() % 2);
	NLine = rand() % NHumanLines;
    };

    pObject->m_Side = side;
    pObject->m_Line = NLine;

    int LineWidth;

    switch (side)
    {
    case Left:
	    pObject->m_x = LeftMinLine[NLine] - pObject->m_XLen-1;
	    LeftMinLine[NLine] = pObject->m_x-1;

	    LineWidth = round(static_cast<float>(m_XMax*PercentOfCarRoad)/NCarLines);
	    pObject->m_y = static_cast<float>(-(NLine*LineWidth+round(LineWidth/2)));
	break;

    case Right:
	    pObject->m_x = RightMaxLine[NLine] + 1;
	    RightMaxLine[NLine] = pObject->m_x + pObject->m_XLen+1;

	    LineWidth = round(static_cast<float>(m_XMax*PercentOfCarRoad)/NCarLines);
	    pObject->m_y = static_cast<float>(NLine*LineWidth+round(LineWidth/2));
	break;

    case Top:
	    pObject->m_y = TopMaxLine[NLine] + 1;
	    TopMaxLine[NLine] = pObject->m_y + pObject->m_YLen +1;

	    LineWidth = round(static_cast<float>(m_YMax*PercentOfPedestrianCrossing)/NHumanLines);
	    pObject->m_x = static_cast<float>(-(NLine*LineWidth+round(LineWidth/2)));
	break;

    case Bottom:
	    pObject->m_y = BottomMinLine[NLine] - pObject->m_YLen-1;
	    BottomMinLine[NLine] = pObject->m_y-1;

	    LineWidth = round(static_cast<float>(m_YMax*PercentOfPedestrianCrossing)/NHumanLines);
	    pObject->m_x = static_cast<float>(NLine*LineWidth+round(LineWidth/2));
	break;
    };

    EnterCriticalSection(&CS);
	if (pObject->GetType() == Car)
	{
            if (m_NumberOfCars < 3000)
            {
	        m_NumberOfCars++;
            }
            else
            {
                delete pObject;
                LeaveCriticalSection(&CS);
                return;
            };
	}
	else
	{
            if (m_NumberOfHumans < 3000)
            {
	        m_NumberOfHumans++;
            }
            else
            {
                delete pObject;
                LeaveCriticalSection(&CS);
                return;
            };
	};
        ObjectsOnTheRoad.push_back(pObject);
    LeaveCriticalSection(&CS);
}

bool CField::IsCellFree(int x, int y, int XLen, int YLen, CMovingObject* pObject)
{
    EnterCriticalSection(&CS);

    int top = y + YLen;
    int bottom = y;
    int left = x;
    int right = x + XLen;

    std::list<CMovingObject*>::iterator it = ObjectsOnTheRoad.begin();

    while (it != ObjectsOnTheRoad.end())
    {
	CMovingObject* pIteratorObject = *it;

        if (pIteratorObject != pObject)
        {
            int TopIt = (pIteratorObject)->m_y + (pIteratorObject)->m_YLen;
            int BottomIt = (pIteratorObject)->m_y;
            int LeftIt = (pIteratorObject)->m_x;
            int RightIt = (pIteratorObject)->m_x + (pIteratorObject)->m_XLen;

            if (((left>=LeftIt)&&(left<=RightIt))||((right<=RightIt)&&(right>=LeftIt)))
            {
                if (((bottom>=BottomIt)&&(bottom<=TopIt))||((top<=TopIt)&&(top>=BottomIt)))
                {
		    LeaveCriticalSection(&CS);
                    return false;
                };
            };
        };
        it++;
    };

    LeaveCriticalSection(&CS);

    return true;
}

void CField::Move()
{
    int xmax = m_XMax;
    int ymax = m_YMax;
    //move all objects on the field
    EnterCriticalSection(&CS);

    // 0 - don't know
    // 1 can move forward
    // 2 must stop
    int *WayFreeLeft = new int[NCarLines];
    int *WayFreeRight = new int[NCarLines]; 
    int *WayFreeTop = new int[NHumanLines];
    int *WayFreeBottom = new int[NHumanLines];

    int line;
    for (line = 0; line < NCarLines; line++)
    {
	LeftMinLine[line] = -m_XMax;
	RightMaxLine[line] = m_XMax;
	WayFreeLeft[line] = 0;
	WayFreeRight[line] = 0;
    };

    for (line = 0; line < NHumanLines; line++)
    {
	TopMaxLine[line] = m_YMax;
	BottomMinLine[line] = -m_YMax;
	WayFreeTop[line] = 0;
	WayFreeBottom[line] = 0;
    };

    std::list<CMovingObject*>::iterator it = ObjectsOnTheRoad.begin();
    while (it != ObjectsOnTheRoad.end())
    {
        bool MoveMark = false;
	
        switch((*it)->m_Side)
	{
	    case Left:
		//if (WayFreeLeft[(*it)->m_Line] != 2)
		//{
		    if (WayFreeLeft[(*it)->m_Line] == 1)
		    {
			MoveMark = (*it)->MoveWithOutCheck(this);
		    }
		    else
		    {
			MoveMark = (*it)->Move(this);
		    };

		    if (MoveMark)
		    {
			if (WayFreeLeft[(*it)->m_Line] == 0)
			{
			    WayFreeLeft[(*it)->m_Line] = 1;
			};
		    }
                    else
                    {
			if (WayFreeLeft[(*it)->m_Line] == 1)
			{
			    WayFreeLeft[(*it)->m_Line] = 0;
			};
                    };
		//};

		if ((*it)->m_x > xmax)
		{
		    if ((*it)->GetType() == Car)
		    {
			m_NumberOfCars--;
		    }
		    else {
			m_NumberOfHumans--;
		    };
		    delete *it;
		    *it = NULL;
		    it = ObjectsOnTheRoad.erase(it);
		}
		else
		{
		    if ((*it)->m_x < LeftMinLine[(*it)->m_Line])
		    {
			LeftMinLine[(*it)->m_Line] = (*it)->m_x;
		    };

		    it++;
		};
		break;
	    case Bottom:
		//if (WayFreeBottom[(*it)->m_Line] != 2)
		//{
		    if (WayFreeBottom[(*it)->m_Line] == 1)
		    {
			MoveMark = (*it)->MoveWithOutCheck(this);
		    }
		    else
		    {
			MoveMark = (*it)->Move(this);
		    };

		    
		    if (MoveMark)
		    {
			if (WayFreeBottom[(*it)->m_Line] == 0)
			{
			    WayFreeBottom[(*it)->m_Line] = 1;
			};
		    }
                    else
                    {
			if (WayFreeBottom[(*it)->m_Line] == 1)
			{
			    WayFreeBottom[(*it)->m_Line] = 0;
			};
                    };
		//};

		if ((*it)->m_y > ymax)
		{
		    if ((*it)->GetType() == Car)
		    {
			m_NumberOfCars--;
		    }
		    else {
			m_NumberOfHumans--;
		    };
		    delete *it;
		    *it = NULL;
		    it = ObjectsOnTheRoad.erase(it);
		}
		else
		{
		    if ((*it)->m_y < BottomMinLine[(*it)->m_Line])
		    {
			BottomMinLine[(*it)->m_Line] = (*it)->m_y;
		    };

		    it++;
		};
		break;
	    case Right:
		//if (WayFreeRight[(*it)->m_Line] != 2)
		//{
		    if (WayFreeRight[(*it)->m_Line] == 1)
		    {
			MoveMark = (*it)->MoveWithOutCheck(this);
		    }
		    else
		    {
			MoveMark = (*it)->Move(this);
		    };

		    if (MoveMark)
		    {
			if (WayFreeRight[(*it)->m_Line] == 0)
			{
			    WayFreeRight[(*it)->m_Line] = 1;
			};
		    }
                    else
                    {
			if (WayFreeRight[(*it)->m_Line] == 1)
			{
			    WayFreeRight[(*it)->m_Line] = 0;
			};
                    };
		//};

		if ((*it)->m_x < -xmax)
		{
		    if ((*it)->GetType() == Car)
		    {
			m_NumberOfCars--;
		    }
		    else {
			m_NumberOfHumans--;
		    };
		    delete *it;
		    *it = NULL;
		    it = ObjectsOnTheRoad.erase(it);
		}
		else
		{
		    if ((*it)->m_x+(*it)->m_XLen > RightMaxLine[(*it)->m_Line])
		    {
			RightMaxLine[(*it)->m_Line] = (*it)->m_x+(*it)->m_XLen;
		    };

		    it++;
		};
		break;
	    case Top:
		//if (WayFreeTop[(*it)->m_Line] != 2)
		//{
		    if (WayFreeTop[(*it)->m_Line] == 1)
		    {
			MoveMark = (*it)->MoveWithOutCheck(this);
		    }
		    else
		    {
			MoveMark = (*it)->Move(this);
		    };

		    if (MoveMark)
		    {
			if (WayFreeTop[(*it)->m_Line] == 0)
			{
			    WayFreeTop[(*it)->m_Line] = 1;
			};
		    }
                    else
                    {
			if (WayFreeTop[(*it)->m_Line] == 1)
			{
			    WayFreeTop[(*it)->m_Line] = 0;
			};
                    };
		//};

		if ((*it)->m_y < -ymax)
		{
		    if ((*it)->GetType() == Car)
		    {
			m_NumberOfCars--;
		    }
		    else {
			m_NumberOfHumans--;
		    };
		    delete *it;
		    *it = NULL;
		    it = ObjectsOnTheRoad.erase(it);
		}
		else
		{
		    if ((*it)->m_y+(*it)->m_YLen > TopMaxLine[(*it)->m_Line])
		    {
			TopMaxLine[(*it)->m_Line] = (*it)->m_y+(*it)->m_YLen;
		    };

		    it++;
		};
		break;
	};
    };

    delete[] WayFreeLeft;
    delete[] WayFreeRight;
    delete[] WayFreeTop;
    delete[] WayFreeBottom;

    LeaveCriticalSection(&CS);
}

void CField::GetTrafficLightBorders(int &LeftBorder, int &RightBorder, int &TopBorder, int &BottomBorder)
{
    LeftBorder = -m_XMax*PercentOfPedestrianCrossing-1;
    RightBorder = m_XMax*PercentOfPedestrianCrossing+1;
    TopBorder = m_YMax*PercentOfCarRoad+1;
    BottomBorder = -m_YMax*PercentOfCarRoad-1;
}

void CField::DeleteAllMO()
{
    EnterCriticalSection(&CS);
    std::list<CMovingObject*>::iterator it = ObjectsOnTheRoad.begin();

    while (it!=ObjectsOnTheRoad.end())
    {
	delete *it;
	*it = NULL;
	it = ObjectsOnTheRoad.erase(it);
    };

    m_NumberOfHumans = 0;
    m_NumberOfCars = 0;

    LeaveCriticalSection(&CS);
}

int CField::GetXMax()
{
    return m_XMax;
}

int CField::GetYMax()
{
    return m_YMax;
}

float CField::GetPercentOfCarRoad()
{
    return PercentOfCarRoad;
}


float CField::GetPercentOfPedestrianCrossing()
{
    return PercentOfPedestrianCrossing;
}

int CField::GetNumberOfCars()
{
    return m_NumberOfCars;

}

int CField::GetNumberOfHumans()
{

    return m_NumberOfHumans;
}

int CField::GetNCarLines()
{
    return NCarLines;
}

int CField::GetNHumanLines()
{
    return NHumanLines;
}
