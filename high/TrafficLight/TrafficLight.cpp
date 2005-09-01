// TrafficLight.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "TrafficLight.h"

#include "MainFrm.h"
#include "TrafficLightDoc.h"
#include "TrafficLightView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightApp

BEGIN_MESSAGE_MAP(CTrafficLightApp, CWinApp)
	//{{AFX_MSG_MAP(CTrafficLightApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightApp construction

CTrafficLightApp::CTrafficLightApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTrafficLightApp object

CTrafficLightApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightApp initialization

BOOL CTrafficLightApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CTrafficLightDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CTrafficLightView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CTrafficLightApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CTrafficLightApp message handlers


void CAboutDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	CStatic *pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_2));
	pStatic->SetWindowText("\tProblem description\nLet in the city N there is crossroad with traffic light, which shows red, green and yellow signals by turns. We will name sequence of showing red, green and yellow signals as cycle of traffic light. Cycle time and time of lighting yellow signal are fixed, but time of lighting green and red signals may change. Consider traffic light works around the clock. For example, day may be divided only on 24 parts (hours). Several cars on first and second road arrive to the crossroad every hour. A number of stopped cars near the crossroad at the start of cycle depends on duration of red and green signals lighting time and that is why traffic congestions (jams) may appear. Our problem is chousing optimal lighting intervals of red and green signals for decreasing maximum carpool values near crossroad at the start of traffic light cycle.\n\tBayesian approach\n\tConstructing model\nLets carpool depends on hour, day and weather. Hour may takes 5 values, weather - 3 values (sunny, cloudy and rainy), day - 2 values. We can create Bayesian network that describes out problem.");

	pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_3));
	pStatic->SetWindowText("In our simple case all network nodes are discrete. Car Stream 1 node describes carpool on first road, Car Stream 2 node- on second road. For examlpe, Car Stream 1 and Car Stream 2  nodes may take 4 values. First value means that carpool is less than 5 cars. Second value means that carpool is more than 5 cars but less than 10 cars. Third: more than 10 and less than 20. Fourth: more than 20.\n\tLearning model parameters\nFor optimal working on fixed crossroad and exploitation conditions network have to be trained. We have to execute the following actions:\nÀ Gathering observations. On this stage we suppose that some valus of nodes Car Stream 1 and Car Stream 2 are observed for every fixed value configuration of nodes Hour, Weather and Day. In our simple example these values will be well-known because carpools are generated by our functions, but in practic, these values are computed by statistic (human or machine) that observe the carpools near the crossroad. With the help of Hour, Weather, Day, Car Stream 1 and Car Stream 2 node values, our system calculate optimal lighting time distribution. We use the following formula:");

	pStatic = static_cast<CStatic *>(GetDlgItem(IDC_STATIC_4));
	pStatic->SetWindowText("We can find optimal RedTime and GreenTime values with excess because there are several variants of lighting distributions. So, we have new evidence with all observed nodes (Hour, Weather, Day, Car Stream 1 and Car Stream 2)\nÂ) Learning network. We learn network with help of EM learning algorithm on the base of evidence samples.\n\tModel exploitation\nSystem does not use information about situation on the roads for choosing lighting variant after the network is trained. Now traffic light gets only values of nodes: Hour, Weather, Day from center of traffic controlling. Then system choose the most probable value of nodes Car Stream 1 and Car Stream 2 and use these values for calculation optimal lighting distribution.");

}
