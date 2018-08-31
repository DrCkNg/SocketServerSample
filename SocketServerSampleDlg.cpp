/*************************************************************************
// SocketServerSampleDlg.cpp
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgment.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 2018
// DrckNg@mail.com
**************************************************************************/

// SocketServerSampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SocketServerSample.h"
#include "SocketServerSampleDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSocketServerSampleDlg dialog




CSocketServerSampleDlg::CSocketServerSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSocketServerSampleDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pSocketServer = NULL;
}

CSocketServerSampleDlg::~CSocketServerSampleDlg()
{
	if(m_pSocketServer)
	{
		m_pSocketServer->DisconnectAndCleanUpSocketSystem();
		delete m_pSocketServer;
		m_pSocketServer = NULL;
	}
}

void CSocketServerSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSocketServerSampleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, &CSocketServerSampleDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT, &CSocketServerSampleDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDOK, &CSocketServerSampleDlg::OnBnClickedOk)
	ON_MESSAGE(WM_UPDATE_DIALOG_UI, &CSocketServerSampleDlg::UpdateDialogUI)
END_MESSAGE_MAP()


// CSocketServerSampleDlg message handlers

BOOL CSocketServerSampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_nPort	= 10038;
	m_strTriggerCmdAck.Empty();
	m_nTimeoutPerServingSec = 5;	

	ReadSettings();
	WriteSettings();

	m_pSocketServer = new CSocketServer;
	m_pSocketServer->SetParentDlg(this);
	m_pSocketServer->SetTimeOutPerServing(5);
	m_pSocketServer->SetTriggerCmdAck(m_strTriggerCmdAck);

	char cTerminator[2] = {13, 10};
	m_pSocketServer->SetTerminator(cTerminator, 2);

	m_pSocketServer->SetListeningPort(m_nPort);
	m_pSocketServer->InitialiseSocketSystem();
	m_pSocketServer->StartListening();

	CString strMsg;
	strMsg.Format(_T("Server Listening Port : %d"),m_nPort);
	UpdateDialogUIStatus(strMsg);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSocketServerSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSocketServerSampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSocketServerSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CSocketServerSampleDlg::UpdateDialogUI(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(IDC_MSG_DISPLAY)->SetWindowTextW(m_strStatusDisplay);
	GetDlgItem(IDC_DATA_DISPLAY)->SetWindowTextW(m_strReceiveDataDisplay);
	GetDlgItem(IDC_CLIENT_LIST)->SetWindowTextW(m_strClientListDisplay);
	return 0;
}

void CSocketServerSampleDlg::OnBnClickedOk()
{
	if(m_pSocketServer)
	{
		m_pSocketServer->DisconnectAndCleanUpSocketSystem();
		delete m_pSocketServer;
		m_pSocketServer = NULL;
	}

	CDialog::OnOK();
}

void CSocketServerSampleDlg::OnBnClickedConnect()		
{
	CString strMsg;
	strMsg.Format(_T("Server Listening Port : %d"),m_nPort);
	UpdateDialogUIStatus(strMsg);

	m_pSocketServer->StartListening();		
}

void CSocketServerSampleDlg::OnBnClickedDisconnect()	
{
	m_pSocketServer->StopListening();		
}

BOOL CSocketServerSampleDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	BOOL bRet = CDialog::PreTranslateMessage(pMsg);
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		OnBnClickedOk();
	return CDialog::PreTranslateMessage(pMsg);
}

void CSocketServerSampleDlg::ReadSettings()
{
	WCHAR wbuf[1024];
	DWORD dwResult;

	CString strDirectory,strTempSection,strTempDefault,strFileCfg;
	GetCurrentDirectory(1024,wbuf);
	strDirectory = CString(wbuf);

	strFileCfg = strDirectory + _T("\\config.ini");
	strTempSection.Format(_T("Server"));

	m_nPort = GetPrivateProfileInt(strTempSection,_T("Port"),10038,strFileCfg);  
	m_nTimeoutPerServingSec = GetPrivateProfileInt(strTempSection,_T("Time Out Per Serving(Sec)"),5,strFileCfg);  

	strTempDefault.Format(_T("_ACK"));
	m_strTriggerCmdAck = strTempDefault;
	dwResult = GetPrivateProfileString(strTempSection,_T("Trigger Command Ack"),strTempDefault,wbuf,1024,strFileCfg); 
	if(dwResult>0) 
		m_strTriggerCmdAck = CString(wbuf);
}

void CSocketServerSampleDlg::WriteSettings()
{
	WCHAR wbuf[1024];

	CString strDirectory,strTempSection,strTempDefault,strTemp;
	GetCurrentDirectory(1024,wbuf);
	strDirectory = CString(wbuf);

	CString strFileCfg;
	strFileCfg = strDirectory + _T("\\config.ini");
	strTempSection.Format(_T("Server"));

	strTemp.Format(_T("%d"),m_nPort); 
	WritePrivateProfileString(strTempSection,_T("Port"),strTemp,strFileCfg);

	strTemp.Format(_T("%d"),m_nTimeoutPerServingSec); 
	WritePrivateProfileString(strTempSection,_T("Time Out Per Serving(Sec)"),strTemp,strFileCfg);

	WritePrivateProfileString(strTempSection,_T("Trigger Command Ack"),m_strTriggerCmdAck,strFileCfg);

}
