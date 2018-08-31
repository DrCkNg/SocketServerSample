/*************************************************************************
// CServerSocket.h
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

#pragma once
#include <afxmt.h>
#include <vector>
class CSocketServerSampleDlg;

#define SOCKET_BUFFER_SIZE		10240

static UINT ThreadAcceptConnection(LPVOID pParam);

class CSocketServer
{
public:
	CSocketServer();
	~CSocketServer();

	struct  SSOCKETDATA
	{
		SOCKET		s_DataSocket;
		sockaddr_in s_ClientSocketAddr;
		CTime		t_LastProcessed;
		BOOL		bDrop;				//is disconnected?
	} ;

private:
	int							m_nPort;
	CString						m_strTriggerCmdAck;
	SOCKET						m_sListenSocket;					// socket for listening
	SOCKET						m_sClientSocket;					// sockets for communicate
	std::vector<SSOCKETDATA>    m_arrsSocketData;
	BOOL						m_bUserRequestConnect;				// request to disconnect
	BOOL						m_IsConnectingIdle;
	BOOL						m_IsReceivingIdle;
	CWinThread*					m_hThreadAcception;
	CWinThread*					m_hThreadReceiveData;
	CSocketServerSampleDlg*		m_pDlg;								// for callback purpose
	int							m_nTimeOutPerServing;
	BOOL						m_bRequestTerminateReceiveThread;	// Request to Terminate Receive Thread
	BOOL						m_bRequestTerminateAcceptThread;	// Request to Terminate Accept Thread

	CEvent*						m_pEventThreadActionTrigger;
	int							m_nNextClientConnectionId;
	char						m_cTerminator[20];
	int							m_nTerminatorCharNum;
	CCriticalSection			m_csSocketListOp;

	int							m_nServiceCounter;					// used to simulate a counter service 

public:
	void StartListening();	
	void StopListening();
	void InitialiseSocketSystem();	
	void DisconnectAndCleanUpSocketSystem();
	void Receive(int nSocketId);
	void SetListeningPort(int nPort)				{m_nPort=nPort;}
	int  GetListeningPort()							{return m_nPort;}
	void SetTriggerCmdAck(CString strTriggerCmdAck)	{m_strTriggerCmdAck = strTriggerCmdAck;}
	void SetParentDlg(CSocketServerSampleDlg* pDlg)	{m_pDlg=pDlg;}
	void SetTimeOutPerServing(int nTimeoutSecond)	{m_nTimeOutPerServing=nTimeoutSecond;}
	CString GetClientListString();
	void SetTerminator(char* pcTerminator, int nTotalChatNum);

private:
	void TerminateDataThread();
	void TerminateAcceptThread();
	void DisconnectSingleClientSocket(int nSocketId);

	friend UINT ThreadAcceptConnection(LPVOID pParam);
	friend UINT ThreadReceiveData(LPVOID pParam);
};
