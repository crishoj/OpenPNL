// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "TrafficLight.h"
#include "InfoPanel.h"
#include "TrafficLightDoc.h"
#include "TrafficLightView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
    m_initSplitters = FALSE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
    	CRect cr;	

	GetClientRect( &cr );

	if ( !m_mainSplitter.CreateStatic( this, 1, 2 ) )
	{
		MessageBox( "Error setting up splitter frames!", "Init Error!",
			MB_OK | MB_ICONERROR );
		return FALSE;
	}

	if ( !m_mainSplitter.CreateView( 0, 1, RUNTIME_CLASS(CInfoPanel),
		CSize( 210, cr.Height() ), pContext ) )
	{
		MessageBox( "Error setting up splitter frames!", "Init Error!",
			MB_OK | MB_ICONERROR );
		return FALSE;
	}

	if ( !m_mainSplitter.CreateView( 0, 0, RUNTIME_CLASS(CTrafficLightView),
		CSize( cr.Width()-210, cr.Height() ), pContext ) )
	{
		MessageBox( "Error setting up splitter frames!", "Init Error!",
			MB_OK | MB_ICONERROR );
		return FALSE;
	}

	CInfoPanel *pPanel = static_cast<CInfoPanel*>(m_mainSplitter.GetPane(0,1));
	CNature *pNature = static_cast<CTrafficLightDoc*>(static_cast<CTrafficLightView*>(m_mainSplitter.GetPane(0,0))->GetDocument())->
	    GetNature();

	pPanel->SetNature(pNature);
	pNature->SetInfoPanel(pPanel);

	m_initSplitters = TRUE;
	
	return TRUE;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
	
	if ( m_initSplitters && nType != SIZE_MINIMIZED )
	{
		// Adjust the row/column info to compensate for
		// the new size

		m_mainSplitter.SetRowInfo( 0, cy, 0 );
		m_mainSplitter.SetColumnInfo( 0, cx - 210, 0 );
		m_mainSplitter.SetColumnInfo( 1, 210, 0 );

		m_mainSplitter.RecalcLayout();
	};
	
}
