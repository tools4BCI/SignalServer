//*****************************************************************************
//*
//*
//*		LptWizardDlg.h
//*
//*
//*****************************************************************************
#ifndef		AFX_LPTWIZARDDLG_H__0CF3106B_87F7_4B2A_8790_C43A52651686__INCLUDED_
#define		AFX_LPTWIZARDDLG_H__0CF3106B_87F7_4B2A_8790_C43A52651686__INCLUDED_	

#if _MSC_VER > 1000
#pragma once
#endif

//*****************************************************************************
//*
//*		CLptWizardDlg Dialog
//*
//*****************************************************************************
class CLptWizardDlg : public CDialog
{
	// Construction
public:

	CLptWizardDlg(CWnd *pParent = NULL);						// standard constructor

	// Dialog Data
	//{{AFX_DATA(CLptWizardDlg)
	enum { IDD = IDD_LPTWIZARD_DIALOG };
	BOOL	m_cPort01;
	BOOL	m_cPort02;
	BOOL	m_cPort03;
	BOOL	m_cPort04;
	BOOL	m_cPort05;
	BOOL	m_cPort06;
	BOOL	m_cPort07;
	BOOL	m_cPort08;
	BOOL	m_cPort11;
	BOOL	m_cPort12;
	BOOL	m_cPort13;
	BOOL	m_cPort14;
	BOOL	m_cPort15;
	BOOL	m_cPort16;
	BOOL	m_cPort17;
	BOOL	m_cPort18;
	BOOL	m_cPort21;
	BOOL	m_cPort22;
	BOOL	m_cPort23;
	BOOL	m_cPort24;
	BOOL	m_cPort25;
	BOOL	m_cPort26;
	BOOL	m_cPort27;
	BOOL	m_cPort28;
	int		m_iLptIndex;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CLptWizardDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);			// DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	
	HICON		m_hIcon;
	unsigned	m_uLastData;
	unsigned	m_aLptIndex[8];
	int			m_iLptPort;

	// Generated message map functions
	//{{AFX_MSG(CLptWizardDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnChangeLpt();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	void OnCheck();

};

//{{AFX_INSERT_LOCATION}}

#endif
