//*****************************************************************************
//*
//*
//*		LptWizardDlg.cpp
//*
//*
//*****************************************************************************
// LptWizardDlg.cpp : implementation file
//
#include	"StdAfx.h"
#include	"LptTools.h"
#include	"LptWizard.h"
#include	"LptWizardDlg.h"

#ifdef		_DEBUG
#define new	 DEBUG_NEW
#undef		 THIS_FILE
static char	 THIS_FILE[] = __FILE__;
#endif

#define 	 ID_READPORT	0x3865

BEGIN_MESSAGE_MAP(CLptWizardDlg, CDialog)
	//{{AFX_MSG_MAP(CLptWizardDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO1, OnChangeLpt)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK1,  OnCheck)
	ON_BN_CLICKED(IDC_CHECK2,  OnCheck)
	ON_BN_CLICKED(IDC_CHECK3,  OnCheck)
	ON_BN_CLICKED(IDC_CHECK4,  OnCheck)
	ON_BN_CLICKED(IDC_CHECK5,  OnCheck)
	ON_BN_CLICKED(IDC_CHECK6,  OnCheck)
	ON_BN_CLICKED(IDC_CHECK7,  OnCheck)
	ON_BN_CLICKED(IDC_CHECK8,  OnCheck)
	ON_BN_CLICKED(IDC_CHECK11, OnCheck)
	ON_BN_CLICKED(IDC_CHECK12, OnCheck)
	ON_BN_CLICKED(IDC_CHECK13, OnCheck)
	ON_BN_CLICKED(IDC_CHECK14, OnCheck)
	ON_BN_CLICKED(IDC_CHECK15, OnCheck)
	ON_BN_CLICKED(IDC_CHECK16, OnCheck)
	ON_BN_CLICKED(IDC_CHECK17, OnCheck)
	ON_BN_CLICKED(IDC_CHECK18, OnCheck)
	ON_BN_CLICKED(IDC_CHECK21, OnCheck)
	ON_BN_CLICKED(IDC_CHECK22, OnCheck)
	ON_BN_CLICKED(IDC_CHECK23, OnCheck)
	ON_BN_CLICKED(IDC_CHECK24, OnCheck)
	ON_BN_CLICKED(IDC_CHECK25, OnCheck)
	ON_BN_CLICKED(IDC_CHECK26, OnCheck)
	ON_BN_CLICKED(IDC_CHECK27, OnCheck)
	ON_BN_CLICKED(IDC_CHECK28, OnCheck)
END_MESSAGE_MAP()


//*****************************************************************************
//*
//*		CLptWizardDlg Dialog
//*
//*****************************************************************************
CLptWizardDlg::CLptWizardDlg(CWnd *pParent) : CDialog(CLptWizardDlg::IDD,pParent)
{
	//{{AFX_DATA_INIT(CLptWizardDlg)
	m_cPort01	=  FALSE;
	m_cPort02	=  FALSE;
	m_cPort03	=  FALSE;
	m_cPort04	=  FALSE;
	m_cPort05	=  FALSE;
	m_cPort06	=  FALSE;
	m_cPort07	=  FALSE;
	m_cPort08	=  FALSE;
	m_cPort11	=  FALSE;
	m_cPort12	=  FALSE;
	m_cPort13	=  FALSE;
	m_cPort14	=  FALSE;
	m_cPort15	=  FALSE;
	m_cPort16	=  FALSE;
	m_cPort17	=  FALSE;
	m_cPort18	=  FALSE;
	m_cPort21	=  FALSE;
	m_cPort22	=  FALSE;
	m_cPort23	=  FALSE;
	m_cPort24	=  FALSE;
	m_cPort25	=  FALSE;
	m_cPort26	=  FALSE;
	m_cPort27	=  FALSE;
	m_cPort28	=  FALSE;
	m_iLptIndex =  0; 
	//}}AFX_DATA_INIT

	m_iLptPort	=  0;
	m_hIcon     = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//*****************************************************************************
//*
//*		DoDataExchange
//*
//*****************************************************************************
void CLptWizardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLptWizardDlg)
	DDX_Check  (pDX,IDC_CHECK1 ,m_cPort01 );
	DDX_Check  (pDX,IDC_CHECK2 ,m_cPort02 );
	DDX_Check  (pDX,IDC_CHECK3 ,m_cPort03 );
	DDX_Check  (pDX,IDC_CHECK4 ,m_cPort04 );
	DDX_Check  (pDX,IDC_CHECK5 ,m_cPort05 );
	DDX_Check  (pDX,IDC_CHECK6 ,m_cPort06 );
	DDX_Check  (pDX,IDC_CHECK7 ,m_cPort07 );
	DDX_Check  (pDX,IDC_CHECK8 ,m_cPort08 );
	DDX_Check  (pDX,IDC_CHECK11,m_cPort21 );
	DDX_Check  (pDX,IDC_CHECK12,m_cPort22 );
	DDX_Check  (pDX,IDC_CHECK13,m_cPort23 );
	DDX_Check  (pDX,IDC_CHECK14,m_cPort24 );
	DDX_Check  (pDX,IDC_CHECK15,m_cPort25 );
	DDX_Check  (pDX,IDC_CHECK16,m_cPort26 );
	DDX_Check  (pDX,IDC_CHECK17,m_cPort27 );
	DDX_Check  (pDX,IDC_CHECK18,m_cPort28 );
	DDX_Check  (pDX,IDC_CHECK21,m_cPort11 );
	DDX_Check  (pDX,IDC_CHECK22,m_cPort12 );
	DDX_Check  (pDX,IDC_CHECK23,m_cPort13 );
	DDX_Check  (pDX,IDC_CHECK24,m_cPort14 );
	DDX_Check  (pDX,IDC_CHECK25,m_cPort15 );
	DDX_Check  (pDX,IDC_CHECK26,m_cPort16 );
	DDX_Check  (pDX,IDC_CHECK27,m_cPort17 );
	DDX_Check  (pDX,IDC_CHECK28,m_cPort18 );
	DDX_CBIndex(pDX,IDC_COMBO1 ,m_iLptIndex);
	//}}AFX_DATA_MAP
}

//*****************************************************************************
//*
//*		OnInitDialog
//*
//*****************************************************************************
//  CLptWizardDlg message handlers
BOOL CLptWizardDlg::OnInitDialog()
{
CComboBox	   *pBox;	
char			cText[128];
unsigned short	aAddress[8];
int				iCount;
int				iPos;
int				iNum;

	

	memset(m_aLptIndex,0,sizeof(m_aLptIndex));


	if(!LptInit())
		{
		MessageBox("Der LPT Treiber konnte nicht geladen werden !","Fehler",MB_OK|MB_ICONERROR);
		OnCancel();
		}


	LptDetectPorts(iCount,aAddress,8);

	if(iCount<=0)
		{
		MessageBox("Es wurden keine LPT-Ports gefunden !","Fehler",MB_OK|MB_ICONERROR);
		OnCancel();
		}

	pBox = (CComboBox*)GetDlgItem(IDC_COMBO1);
	iNum = 0;

	for(iPos=0;iPos<iCount;iPos++)
		{
		if(!aAddress[iPos])continue;
		m_aLptIndex[iNum] = iPos;
		sprintf(cText,"LPT%i aut Portadresse %03Xh",iPos+1,aAddress[iPos]);
		pBox->AddString(cText);
		iNum++;
		}



	CDialog::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT( IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	
	if(pSysMenu != NULL)
		{
		CString sAboutMenu;

			sAboutMenu.LoadString(IDS_ABOUTBOX);
		if(!sAboutMenu.IsEmpty())
			{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, sAboutMenu);
			}
		}

	SetIcon(m_hIcon, TRUE); 									// Set big icon
	SetIcon(m_hIcon, FALSE);									// Set small icon

	m_uLastData = 0x00;

	OnChangeLpt();
	OnTimer(ID_READPORT);

	SetTimer(ID_READPORT,300,NULL);


return TRUE;
}

//*****************************************************************************
//*
//*		OnSysCommand
//*
//*****************************************************************************
void CLptWizardDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if((nID & 0xFFF0) == IDM_ABOUTBOX)
		{
		CDialog dlgAbout(IDD_ABOUTBOX);
		dlgAbout.DoModal();
		}
	else{
		CDialog::OnSysCommand(nID, lParam);
		}
}

//*****************************************************************************
//*
//*		OnPaint
//*
//*****************************************************************************
//  If you add a minimize button to your Dialog, you will need the code below
// 	to draw the icon.  For MFC applications using the document/view model,
// 	this is automatically done for you by the framework.
void CLptWizardDlg::OnPaint()
{
	if(IsIconic())
		{
		CPaintDC dc(this);										// device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client sRectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect sRect;
		GetClientRect(&sRect);

		int x = (sRect.Width() - cxIcon + 1) / 2;
		int y = (sRect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
		}
	else{
		CDialog::OnPaint();
		}
}

//*****************************************************************************
//*
//*		OnQueryDragIcon
//*
//*****************************************************************************
HCURSOR CLptWizardDlg::OnQueryDragIcon()
{

return (HCURSOR) m_hIcon;
}

//*****************************************************************************
//*
//*		OnChangeLpt
//*
//*****************************************************************************
void CLptWizardDlg::OnChangeLpt()
{
unsigned	uData;
	

	UpdateData();

	m_iLptPort = m_aLptIndex[m_iLptIndex];


	uData = LptPortIn(m_iLptPort,0);

	m_cPort01 = (uData>>0)&1;
	m_cPort02 = (uData>>1)&1;
	m_cPort03 = (uData>>2)&1;
	m_cPort04 = (uData>>3)&1;
	m_cPort05 = (uData>>4)&1;
	m_cPort06 = (uData>>5)&1;
	m_cPort07 = (uData>>6)&1;
	m_cPort08 = (uData>>7)&1;

	uData = LptPortIn(m_iLptPort,2);

	m_cPort21 = (uData>>0)&1;
	m_cPort22 = (uData>>1)&1;
	m_cPort23 = (uData>>2)&1;
	m_cPort24 = (uData>>3)&1;
	m_cPort25 = (uData>>4)&1;
	m_cPort26 = (uData>>5)&1;
	m_cPort27 = (uData>>6)&1;
	m_cPort28 = (uData>>7)&1;

	UpdateData(FALSE);

}

//*****************************************************************************
//*
//*		OnCheck
//*
//*****************************************************************************
void CLptWizardDlg::OnCheck()
{
unsigned	uData;


	UpdateData();

	uData  = 0;
	uData |= (m_cPort01&1)<<0;
	uData |= (m_cPort02&1)<<1;
	uData |= (m_cPort03&1)<<2;
	uData |= (m_cPort04&1)<<3;
	uData |= (m_cPort05&1)<<4;
	uData |= (m_cPort06&1)<<5;
	uData |= (m_cPort07&1)<<6;
	uData |= (m_cPort08&1)<<7;

	LptPortOut(m_iLptPort,0,uData);

	uData  = 0;
	uData |= (m_cPort21&1)<<0;
	uData |= (m_cPort22&1)<<1;
	uData |= (m_cPort23&1)<<2;
	uData |= (m_cPort24&1)<<3;
	uData |= (m_cPort25&1)<<4;
	uData |= (m_cPort26&1)<<5;
	uData |= (m_cPort27&1)<<6;
	uData |= (m_cPort28&1)<<7;

	LptPortOut(m_iLptPort,2,uData);
}


//*****************************************************************************
//*
//*		OnTimer
//*
//*****************************************************************************
void CLptWizardDlg::OnTimer(UINT nIDEvent) 
{
unsigned	uData;
	


	if(nIDEvent==ID_READPORT)
		{
		   uData = LptPortIn(m_iLptPort,1);
		if(uData!=m_uLastData)
			{
			m_cPort11 = (uData>>0)&1;
			m_cPort12 = (uData>>1)&1;
			m_cPort13 = (uData>>2)&1;
			m_cPort14 = (uData>>3)&1;
			m_cPort15 = (uData>>4)&1;
			m_cPort16 = (uData>>5)&1;
			m_cPort17 = (uData>>6)&1;
			m_cPort18 = (uData>>7)&1;

			m_uLastData = uData;
			UpdateData(FALSE);
			}
		
		return;
		}
	
	CDialog::OnTimer(nIDEvent);
}
