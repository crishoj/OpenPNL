// TrafficLightView.cpp : implementation of the CTrafficLightView class
//

#include "stdafx.h"
#include "TrafficLight.h"

#include "TrafficLightDoc.h"
#include "TrafficLightView.h"

#include <utility>
#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightView

IMPLEMENT_DYNCREATE(CTrafficLightView, CView)

BEGIN_MESSAGE_MAP(CTrafficLightView, CView)
	//{{AFX_MSG_MAP(CTrafficLightView)
	ON_WM_CREATE()
	ON_WM_CANCELMODE()
	ON_WM_ERASEBKGND()
	ON_WM_CAPTURECHANGED()
	ON_WM_SIZE()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_COPYDATA()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightView construction/destruction

CTrafficLightView::CTrafficLightView()
{
	// TODO: add construction code here
    m_alpha = 0;
    m_betta = 0;
    m_x0 = 0;
    m_y0 = 0;
    m_zvalue = 0;
}

CTrafficLightView::~CTrafficLightView()
{
}

BOOL CTrafficLightView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
    cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CS_OWNDC);

    return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightView drawing

void CTrafficLightView::OnDraw(CDC* pDC)
{
    // Integer declared as static so the value is maintained
    static int i=0;

    // Increase i, this is the rotation value.
    i += 1.11;
   
    // Make the rendering context current as we're about to
    // draw into it
    wglMakeCurrent(m_myhDC,m_hRC);
   
    // Clear the screen and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    // Reset the model matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
   
    // Translate to a suitable position
    glTranslatef(0,0,-100);
    glTranslatef(0.0f, 0.0f, static_cast<float>(m_zvalue)/20);

    glRotated(static_cast<float>(m_alpha)/100, 0,1,0);
    glRotated(static_cast<float>(m_betta)/100, -1,0,0);

    /*
    // We're going to draw using this color. ( an orange )
    glColor3f( 0.89f, 0.36f, 0.0f);

    // Rotate to some angle (i)
    glRotatef(i, 1,0,0);
    glRotatef(i ,0,1,0);
    glRotatef(i ,0,0,1);

    // Draw the first torus.
    auxSolidTorus( 8,50);

    // Rotate some more and draw the second.
    glRotatef(i ,0,1,0);
    glRotatef(i ,0,0,1);
    auxSolidTorus( 8,30);

    // Rotate some more and draw the third
    glRotatef(-i ,0,1,0);
    glRotatef(i ,0,0,1);
    auxSolidTorus( 8,10);
*/
    static_cast<CTrafficLightDoc* >(GetDocument())->GetField()->Show();
   
    // Swap the virtual screens
    SwapBuffers(m_myhDC);
}

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightView printing

BOOL CTrafficLightView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CTrafficLightView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CTrafficLightView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightView diagnostics

#ifdef _DEBUG
void CTrafficLightView::AssertValid() const
{
	CView::AssertValid();
}

void CTrafficLightView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTrafficLightDoc* CTrafficLightView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTrafficLightDoc)));
	return (CTrafficLightDoc*)m_pDocument;
}
#endif //_DEBUG

typedef std::pair<CTrafficLightDoc*, CTrafficLightView*> TwoClasses ;

UINT MyThreadProc( LPVOID pParam )
{
    TwoClasses* pTwoClasses = static_cast<TwoClasses* >(pParam);

    if (pTwoClasses == NULL)
    {
	return 1;
    };

    pTwoClasses->first->GetNature()->ProcessModel(pTwoClasses->second);

    delete pTwoClasses;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightView message handlers

int CTrafficLightView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here;

	//InitGL();
	SetupPixelFormat();
	wglMakeCurrent(NULL,NULL);

	TwoClasses* pTwoClasses = new TwoClasses;
	pTwoClasses->first = static_cast<CTrafficLightDoc*>(GetDocument());
	pTwoClasses->second = this;

	AfxBeginThread(MyThreadProc, pTwoClasses);
	
	return 0;
}

void CTrafficLightView::OnCancelMode() 
{
	CView::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}

void CTrafficLightView::InitGL()
{
	GLfloat LightAmbient[]=		{ 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat LightDiffuse[]=		{ 0.9f, 0.9f, 0.1f, 1.0f };
	GLfloat LightSpecular[]=	{ 0.9f, 0.9f, 0.9f, 1.0f };
	GLfloat LightPosition[]=	{ 0.0f, 0.0f, -10.0f, 0.0f };


    // Enables Depth Testing
    glEnable(GL_DEPTH_TEST);

    // Enable the point size for selected points
    glPointSize(5.0f);

    // This Will Clear The Background Color To Black
    glClearColor(0.6f, 0.6f, 0.6f, 0.0f);

    // Reset the current projection matrix
    SetProjection();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Enable back face culling, defaults to Clock wise vertices.
    glEnable(GL_CULL_FACE);

    // Create a light and enable it.
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecular);
    glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);		// Enable Lighting

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

BOOL CTrafficLightView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return FALSE;
}

void CTrafficLightView::OnCaptureChanged(CWnd *pWnd) 
{
	// TODO: Add your message handler code here
	
	CView::OnCaptureChanged(pWnd);
}

BOOL CTrafficLightView::SetupPixelFormat()
{
    GLuint PixelFormat;
    static PIXELFORMATDESCRIPTOR pfd= {
        sizeof(PIXELFORMATDESCRIPTOR),
        // Size Of This Pixel Format Descriptor
            1,
            // Version Number (?)
            PFD_DRAW_TO_WINDOW |      // Format Must Support Window
            PFD_SUPPORT_OPENGL |      // Format Must Support OpenGL
            PFD_DOUBLEBUFFER,       // Must Support Double Buffering
            PFD_TYPE_RGBA,         // Request An RGBA Format
            24,          // Select A 24Bit Color Depth
            0, 0, 0, 0, 0, 0, // Color Bits Ignored (?)
            0,            // No Alpha Buffer
            0,            // Shift Bit Ignored (?)
            0,            // No Accumulation Buffer
            0, 0, 0, 0,             // Accumulation Bits Ignored (?)
            16,          // 16Bit Z-Buffer (Depth Buffer)
            0,            // No Stencil Buffer
            0,            // No Auxiliary Buffer (?)
            PFD_MAIN_PLANE,     // Main Drawing Layer
            0,           // Reserved (?)
            0, 0, 0    // Layer Masks Ignored (?)
    };

    m_myhDC = ::GetDC(m_hWnd);    // Gets A Device Context For The Window
    PixelFormat = ChoosePixelFormat(m_myhDC, &pfd); // Finds The Closest Match To The Pixel Format We Set Above

    if (!PixelFormat)
    {
        ::MessageBox(0,"Can't Find A Suitable PixelFormat.","Error",MB_OK|MB_ICONERROR);
        PostQuitMessage(0);
            // This Sends A 'Message' Telling The Program To Quit
        return false ;    // Prevents The Rest Of The Code From Running
    }

    if(!SetPixelFormat(m_myhDC,PixelFormat,&pfd))
    {
        ::MessageBox(0,"Can't Set The PixelFormat.","Error",MB_OK|MB_ICONERROR);
        PostQuitMessage(0);
        return false;
    }

    m_hRC = wglCreateContext(m_myhDC);
    if(!m_hRC)
    {
        ::MessageBox(0,"Can't Create A GL Rendering Context.","Error",MB_OK|MB_ICONERROR);
        PostQuitMessage(0);
        return false;
    }
   
    if(!wglMakeCurrent(m_myhDC, m_hRC))
    {
        ::MessageBox(0,"Can't activate GLRC.","Error",MB_OK|MB_ICONERROR);
        PostQuitMessage(0);
        return false;
    }

    // Now that the screen is setup we can
    // initialize OpenGL();
    InitGL();
    return true;
}


void CTrafficLightView::SetProjection()
{
    glViewport(0, 0, m_width, m_height);   
   
    // Reset The Projection Matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // It's a perspective projection
    // Calculate The Aspect Ratio Of The Window
    gluPerspective(60.0f,(GLfloat)m_width/(GLfloat)m_height, 1.0f,3000.0f);

}


void CTrafficLightView::OnSize(UINT nType, int cx, int cy) 
{
    CView::OnSize(nType, cx, cy);
   
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Make the rendering context current
    wglMakeCurrent(m_myhDC,m_hRC);   

    // Reset The Current Viewport And Perspective Transformation
    glViewport(0, 0, cx, cy);   
   
    m_height= cy;
    m_width = cx;
   
    // Calculate The Aspect Ratio Of The Window
    gluPerspective(60.0f,
    (GLfloat)cx/(GLfloat)cy,
    1.0f,
    1000.0f);
	
}

void CTrafficLightView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
    CView::OnChar(nChar, nRepCnt, nFlags);
}

void CTrafficLightView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
    m_x0 = point.x;
    m_y0 = point.y;
	
    CView::OnLButtonDown(nFlags, point);
}

void CTrafficLightView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	
}

void CTrafficLightView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
    m_alpha += point.x - m_x0;
    m_betta += point.y - m_y0;
	
	CView::OnLButtonUp(nFlags, point);

	Invalidate(FALSE);
}

void CTrafficLightView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

    if (MK_LBUTTON == nFlags)
    {
	m_alpha += point.x - m_x0;
        m_betta += point.y - m_y0;
    };
	CView::OnMouseMove(nFlags, point);

	Invalidate(FALSE);
}

BOOL CTrafficLightView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default

    m_zvalue += zDelta;
	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL CTrafficLightView::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CView::OnCopyData(pWnd, pCopyDataStruct);
}
