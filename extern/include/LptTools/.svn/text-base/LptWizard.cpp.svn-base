//*****************************************************************************
//*
//*
//*		LptWizard.cpp
//*
//*
//*****************************************************************************
// LptWizard.cpp : Defines the class behaviors for the application.
//
#include	"StdAfx.h"
#include	"LptWizard.h"
#include	"LptWizardDlg.h"

#ifdef		_DEBUG
#define new	 DEBUG_NEW
#undef		 THIS_FILE
static char	 THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(CLptWizardApp, CWinApp)
	//{{AFX_MSG_MAP(CLptWizardApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


CLptWizardApp theApp;

//*****************************************************************************
//*
//*		CLptWizardApp
//*
//*****************************************************************************
//  CLptWizardApp construction
CLptWizardApp::CLptWizardApp()
{
}



//*****************************************************************************
//*
//*		InitInstance
//*
//*****************************************************************************
//  CLptWizardApp initialization
BOOL CLptWizardApp::InitInstance()
{
	// Standard initialization

	#ifdef		_AFXDLL 				
	Enable3dControls(); 										// Call this when using MFC in a shared DLL
	#else
	Enable3dControlsStatic();									// Call this when linking to MFC statically
	#endif

	CLptWizardDlg sDlg;
	m_pMainWnd = &sDlg;
	sDlg.DoModal();

return FALSE;
}
