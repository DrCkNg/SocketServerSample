/*************************************************************************
// SocketServerSampleDlg.h
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

// SocketServerSampleDlg.h : header file
//

#pragma once
#include "CServerSocket.h"

#define        WM_UPDATE_DIALOG_UI			(WM_USER + 1)


// CSocketServerSampleDlg dialog
class CSocketServerSampleDlg : public CDialogEx
{
// Construction
public:
	CSocketServerSampleDlg(CWnd* pParent = NULL);	// standard constructor
	~CSocketServerSampleDlg();

// Dialog Data
	enum { IDD = IDD_SOCKETSERVERSAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
private:
	CSocketServer*	m_pSocketServer;
	int				m_nPort;					// Server port
	CString			m_strTriggerCmdAck;			// Trigger cmd acknowledge appendix
	int				m_nTimeoutPerServingSec;	// Time period allowance for each client per serving (second)

	CString			m_strClientListDisplay;		// Display Shadow
	CString			m_strReceiveDataDisplay;	// Display Shadow
	CString			m_strStatusDisplay;			// Display Shadow

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedConnect()	;	
	afx_msg void OnBnClickedDisconnect();	
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	LRESULT OnUpdateDataDisplay(WPARAM wParam,LPARAM lParam);

	void UpdateDialogUIClientList(CString strLst)	{m_strClientListDisplay		= strLst;	PostMessageW(WM_UPDATE_DIALOG_UI);}
	void UpdateDialogUIData(CString strDat)			{m_strReceiveDataDisplay	= strDat;	PostMessageW(WM_UPDATE_DIALOG_UI);}
	void UpdateDialogUIStatus(CString strMsg)		{m_strStatusDisplay			= strMsg;	PostMessageW(WM_UPDATE_DIALOG_UI);}

private:
	void ReadSettings();
	void WriteSettings();
	LRESULT UpdateDialogUI(WPARAM wParam,LPARAM lParam);
};
