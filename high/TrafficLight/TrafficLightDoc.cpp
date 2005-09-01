// TrafficLightDoc.cpp : implementation of the CTrafficLightDoc class
//

#include "stdafx.h"
#include "TrafficLight.h"
#include "TrafficLightDoc.h"

#include "Car.h"
#include "Human.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightDoc

IMPLEMENT_DYNCREATE(CTrafficLightDoc, CDocument)

BEGIN_MESSAGE_MAP(CTrafficLightDoc, CDocument)
//{{AFX_MSG_MAP(CTrafficLightDoc)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightDoc construction/destruction
/*

    float streamCar[2][5][3] =  
    { 
	{{1.0,2.0,1.0},{4.0,5.0,3.0},{4.0,6.0,4.0},{5.0,8.0,6.0},{2.0,0.0,2.0}}
	, {{0.0,1.0,1.0},{5.0,7.0,4.0},{5.0,7.0,4.0},{5.0,6.0,4.0},{4.0,2.0,3.0}}
    };
    
    float streamHuman[2][5][3] =  
    { 
	{{0.0,1.0,0.0},{1.0,2.0,1.0},{2.0,3.0,2.0},{2.0,4.0,3.0},{1.0,0.0,1.0}}
	, {{0.0,0.0,0.0},{2.0,3.0,3.0},{3.0,4.0,2.0},{2.0,2.0,1.0},{1.0,2.0,1.0}}
    };    
*/

/*
    float streamCar[2][5][3] =  
    { 
	{{1.0,2.5,4.0},{4.0,5.5,1.0},{2.5,4.0,4.0},{5.5,1.0,2.5},{4.0,4.0,5.5}}
	, {{1.0,2.5,4.0},{4.0,5.5,1.0},{2.5,4.0,4.0},{5.5,1.0,2.5},{4.0,4.0,5.5}}
    };
    
    float streamHuman[2][5][3] =  
    { 
	{{4.0,4.0,3.0},{0.5,0.5,4.1},{4.0,3.0,0.5},{0.5,4.1,4.0},{3.0,0.5,0.5}}
	, {{4.1,4.0,3.0},{0.5,0.5,4.1},{4.0,3.0,0.5},{0.5,4.1,4.0},{3.0,0.5,0.5}}
    };    
*/

    float streamCar[2][5][3] =  
    { 
	{{1.0,1.0,2.5},{4.0,2.5,4.0},{2.5,4.0,2.5},{5.5,4.0,4.0},{5.5,4.0,5.5}}
	, {{2.5,2.5,1.0},{2.5,4.0,2.5},{4.0,2.5,4.0},{4.0,5.5,5.5},{4.0,5.5,4.0}}
    };
    
    float streamHuman[2][5][3] =  
    { 
	{{4.0,4.0,4.0},{3.0,4.0,3.0},{4.0,3.0,4.0},{0.5,0.5,0.5},{0.5,0.5,0.5}}
	, {{4.0,4.0,4.0},{4.0,3.0,4.0},{3.0,4.0,3.0},{0.5,0.5,0.5},{0.5,0.5,0.5}}
    };    

/*
0
1.0 4.0

1
2.5 4.0

2
4.0 3.0

3
4.0 0.5

4
5.5 0.5
*/
    double TLVars[5] = {0.1, 0.3, 0.5, 0.7, 0.9};

CTrafficLightDoc::CTrafficLightDoc()
{
    // TODO: add one-time construction code here
    
    m_pNature = new CNature(streamCar, streamHuman, TLVars);
    m_pField  = new CField(102,100,0.12,0.06);
}

CTrafficLightDoc::~CTrafficLightDoc()
{
}

BOOL CTrafficLightDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
	return FALSE;
    
    // TODO: add reinitialization code here
    // (SDI documents will reuse this document)
    
    return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CTrafficLightDoc serialization

void CTrafficLightDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
	// TODO: add storing code here
    }
    else
    {
	// TODO: add loading code here
    }
}

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightDoc diagnostics

#ifdef _DEBUG
void CTrafficLightDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CTrafficLightDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightDoc commands
