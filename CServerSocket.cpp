/*************************************************************************
// CServerSocket.cpp
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

#include <stdio.h>
#include <winsock2.h>
#include "stdafx.h"
#include "CServerSocket.h"
#include "SocketServerSample.h"
#include "SocketServerSampleDlg.h"

CSocketServer::CSocketServer()
{
	m_nPort = 1000;
	m_strTriggerCmdAck.Empty();
	m_sListenSocket = INVALID_SOCKET;		
	m_sClientSocket = INVALID_SOCKET;
	m_arrsSocketData.clear();
	m_bUserRequestConnect = FALSE;
	m_IsConnectingIdle = TRUE;
	m_IsReceivingIdle = TRUE;
	m_hThreadAcception = NULL;
	m_hThreadReceiveData = NULL;
	m_pDlg = NULL;
	m_nTimeOutPerServing = 5;	//5 Second
	m_bRequestTerminateAcceptThread = FALSE;
	m_bRequestTerminateReceiveThread = FALSE;

	m_pEventThreadActionTrigger = new CEvent(FALSE, FALSE, _T("EventTrigger"));
	m_nNextClientConnectionId = 0;
	m_cTerminator[0] = 13; //CR, 0x0D	
	m_cTerminator[1] = 10; //LF, 0x0D
	m_nTerminatorCharNum = 2;

	m_nServiceCounter = 0;

	// Create new thread to receive data
	if(m_hThreadReceiveData==NULL)
	{
		CWinThread* hThread = AfxBeginThread(ThreadReceiveData,this,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
		if ( NULL != hThread)
		{
			m_hThreadReceiveData = hThread;
			m_hThreadReceiveData->m_bAutoDelete = FALSE;
			m_hThreadReceiveData->ResumeThread();
		}
		else
		{
			AfxMessageBox(_T("Unable to start Receive Data Thread."));
		}
	}

	if(m_hThreadAcception==NULL)
	{
		CWinThread* hThread = AfxBeginThread(ThreadAcceptConnection,this,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
		if ( NULL != hThread)
		{
			m_hThreadAcception = hThread;
			m_hThreadAcception->m_bAutoDelete = FALSE;
			m_hThreadAcception->ResumeThread();
		}
		else
		{
			AfxMessageBox(_T("Unable to start Accept Connection Thread."));
		}
	}
};

CSocketServer::~CSocketServer()
{
// 	DisconnectAndCleanUpSocketSystem();
	if(m_pEventThreadActionTrigger)
	{
		delete m_pEventThreadActionTrigger;
		m_pEventThreadActionTrigger = NULL;
	}
}

void CSocketServer::StartListening()
{
	m_bUserRequestConnect = TRUE;	
}

void CSocketServer::StopListening()
{
	m_bUserRequestConnect = FALSE;

	while(!m_IsConnectingIdle)
		Sleep(200);
	while(!m_IsReceivingIdle)
		Sleep(200);

	m_csSocketListOp.Lock();
	m_arrsSocketData.clear();
	m_csSocketListOp.Unlock();

	m_pDlg->UpdateDialogUIStatus(_T("Stopped Listening"));
	m_pDlg->UpdateDialogUIClientList(GetClientListString());
}

void CSocketServer::InitialiseSocketSystem()
{
	// Activate winsock2
	WSADATA data;
	WSAStartup(MAKEWORD(2,0), &data);

	struct sockaddr_in src;
	memset(&src, 0, sizeof(src));

	src.sin_port             = htons(m_nPort);
	src.sin_family           = AF_INET;
	src.sin_addr.S_un.S_addr = INADDR_ANY;

	if (m_sListenSocket == INVALID_SOCKET) 
	{
		m_sClientSocket = INVALID_SOCKET;

		// Create a new socket and start listening.
		m_sListenSocket = socket(AF_INET, SOCK_STREAM, 0);
		bind(m_sListenSocket, (struct sockaddr *)&src, sizeof(src));
		listen(m_sListenSocket, 5);	// Maximum pending request is 5.
	}
}

void CSocketServer::DisconnectAndCleanUpSocketSystem()
{
	m_bRequestTerminateReceiveThread = TRUE;
	m_bRequestTerminateAcceptThread = TRUE;
	Sleep(100);

	TerminateAcceptThread();
	TerminateDataThread();

	int nTotalDataSocket = int(m_arrsSocketData.size());
	for(int nSocketId = nTotalDataSocket-1; nSocketId>=0; nSocketId--)
		DisconnectSingleClientSocket(nSocketId);

	if (m_sListenSocket != INVALID_SOCKET) {
		closesocket(m_sListenSocket);
		m_sListenSocket = INVALID_SOCKET;
	}

	WSACleanup();
}

void CSocketServer::Receive(int nSocketId)
{
	char cByteAllCombine[SOCKET_BUFFER_SIZE];
	int  nByteAllCombineEndingPos = 0;
	char recvBytes[SOCKET_BUFFER_SIZE];
	char* sendBytes;
	int recvSize = 0;
	CStringA strReceivedMsgUnion;
	CString strMsg;
	BOOL bReceiveDataBegin = FALSE;
	BOOL bFoundTerminator = FALSE;
	CTime timeNow, timeBegin;
	timeBegin = CTime::GetCurrentTime();
	BOOL bSendBackToClient = TRUE;

	if(nSocketId<0 || nSocketId>=m_arrsSocketData.size())
		return;

	while(TRUE)
	{
		if(nSocketId<0 || nSocketId>=m_arrsSocketData.size())
			return;

		if (m_arrsSocketData[nSocketId].s_DataSocket != INVALID_SOCKET) 
		{
			recvSize = recv(m_arrsSocketData[nSocketId].s_DataSocket, recvBytes, sizeof(recvBytes), 0);
		}
		else
			return;

		sockaddr_in dst;
		int dst_len = sizeof(dst);
		if (getpeername(m_arrsSocketData[nSocketId].s_DataSocket, (struct sockaddr *)&dst, &dst_len) != 0) 
			return;

		if (recvSize == 0)	// Disconnected
		{
			DisconnectSingleClientSocket(nSocketId);
			CString strMsg;
			strMsg.Format(_T("Client(%d) Disconnected"), dst.sin_port);
			m_pDlg->UpdateDialogUIStatus(strMsg);
			m_pDlg->UpdateDialogUIClientList(GetClientListString());
		}

		if (recvSize == -1)	// Error
		{
			CString strMsg;
			strMsg.Format(_T("Client(%d) Error"), dst.sin_port);
		}

		if (recvSize > 0) 
		{
			memcpy(cByteAllCombine+nByteAllCombineEndingPos, recvBytes, recvSize);
			nByteAllCombineEndingPos += recvSize;

			int nMatchChar = 0;
			int nFoundEndingPos = -1;
			if(nByteAllCombineEndingPos>=m_nTerminatorCharNum)	//incoming char >= terminator char num
			{
				for(int nCharCombine=0; nCharCombine<=(nByteAllCombineEndingPos-m_nTerminatorCharNum); nCharCombine++)
				{
					nMatchChar = 0;
					for(int nTerminatorChar=0; nTerminatorChar<m_nTerminatorCharNum; nTerminatorChar++)
					{

						if(cByteAllCombine[nCharCombine+nTerminatorChar]==m_cTerminator[nTerminatorChar])
						{
							nMatchChar++;
							if(nMatchChar==m_nTerminatorCharNum)
							{
								bFoundTerminator |= TRUE;
								nFoundEndingPos = (nCharCombine+nTerminatorChar);
								cByteAllCombine[nFoundEndingPos+1] = 0;	//NULL terminate string
								strReceivedMsgUnion.Format("%s",cByteAllCombine);
								break;
							}
						}
						else
							break;
					}
				}

			}

			bReceiveDataBegin |= TRUE;
		}


		timeNow = CTime::GetCurrentTime();
		if((timeNow-timeBegin)>m_nTimeOutPerServing)
		{
			cByteAllCombine[nByteAllCombineEndingPos] = 0;	//NULL terminate string
			strReceivedMsgUnion.Format("%s",cByteAllCombine);
			bFoundTerminator |= TRUE;	//let it break the loop;
			bSendBackToClient = FALSE;
		}


		if(bFoundTerminator)
		{
			////////////////////////////////////////////////
			// Do something here to serve the client
			////////////////////////////////////////////////
			CStringA strReplyA;
			CStringA strCnt;
			m_nServiceCounter++;
			strCnt.Format("_%d",m_nServiceCounter);
			strReplyA = strReceivedMsgUnion.Left(strReceivedMsgUnion.GetLength()-m_nTerminatorCharNum);
			strReplyA = strReplyA + CStringA(m_strTriggerCmdAck) + strCnt + "\r\n";
			sendBytes = strReplyA.GetBufferSetLength(strReplyA.GetLength());
			if(bSendBackToClient)
			{
				send(m_arrsSocketData[nSocketId].s_DataSocket,sendBytes,strReplyA.GetLength(),0);
				strMsg.Format(_T("Receive data from client %S(%d)\r\nReceived: %SReply: %S"),inet_ntoa(m_arrsSocketData[nSocketId].s_ClientSocketAddr.sin_addr),m_arrsSocketData[nSocketId].s_ClientSocketAddr.sin_port,strReceivedMsgUnion,strReplyA);
			}
			else
				strMsg.Format(_T("Receive data from client %S(%d)\r\n==>%S (Error! Abort process without terminator.)"),inet_ntoa(m_arrsSocketData[nSocketId].s_ClientSocketAddr.sin_addr),m_arrsSocketData[nSocketId].s_ClientSocketAddr.sin_port,strReceivedMsgUnion);


			m_pDlg->UpdateDialogUIData(strMsg);
			bFoundTerminator = FALSE;	//reset
			bReceiveDataBegin = FALSE;	//reset
			break;
		}

		if(!bReceiveDataBegin)
			break;
	}
}

CString CSocketServer::GetClientListString()
{
	CString strClientList, strTemp;
	int nTotalClientSocket = int(m_arrsSocketData.size());
	strClientList.Empty();
	for(int nSocketId=0; nSocketId<nTotalClientSocket; nSocketId++)
	{
		if(m_arrsSocketData[nSocketId].bDrop==FALSE)
		{
			strTemp.Format(_T("Client %d.   [%S:%d]\r\n"),nSocketId+1,inet_ntoa(m_arrsSocketData[nSocketId].s_ClientSocketAddr.sin_addr),m_arrsSocketData[nSocketId].s_ClientSocketAddr.sin_port);
			strClientList += strTemp;
		}
	}

	return strClientList;
}

void CSocketServer::SetTerminator(char* pcTerminator, int nTotalChatNum)
{
	for(int i=0; i<nTotalChatNum; i++)
		m_cTerminator[i] = pcTerminator[i];
	m_nTerminatorCharNum = nTotalChatNum;
}

void CSocketServer::TerminateDataThread()
{
	m_bRequestTerminateReceiveThread = TRUE;

	if(m_hThreadReceiveData)
	{
		for(int nCount=0; nCount<10; nCount++)
		{
			if(WaitForSingleObject(m_hThreadReceiveData->m_hThread,200)==WAIT_OBJECT_0)
				break;
		}
		if(WaitForSingleObject(m_hThreadReceiveData->m_hThread,1000)==WAIT_TIMEOUT)
		{
			TerminateThread(m_hThreadReceiveData->m_hThread, 1L);
			CloseHandle(m_hThreadReceiveData->m_hThread);
			m_hThreadReceiveData->m_hThread = NULL;
		}
		else
		{
			CloseHandle(m_hThreadReceiveData->m_hThread);
			m_hThreadReceiveData->m_hThread = NULL;
		}
		delete m_hThreadReceiveData;
		m_hThreadReceiveData = NULL;
	}
}

void CSocketServer::TerminateAcceptThread()
{
	if(m_hThreadAcception)
	{
		m_bUserRequestConnect = FALSE;
		for(int nCount=0; nCount<10; nCount++)
		{
			if(WaitForSingleObject(m_hThreadAcception->m_hThread,200)==WAIT_OBJECT_0)
				break;
		}
		if(WaitForSingleObject(m_hThreadAcception->m_hThread,1000)==WAIT_TIMEOUT)
		{
			TerminateThread(m_hThreadAcception->m_hThread, 1L);
			CloseHandle(m_hThreadAcception->m_hThread);
			m_hThreadAcception->m_hThread = NULL;
		}
		else
		{
			CloseHandle(m_hThreadAcception->m_hThread);
			m_hThreadAcception->m_hThread = NULL;
		}
		delete m_hThreadAcception;
		m_hThreadAcception = NULL;
	}
}

void CSocketServer::DisconnectSingleClientSocket(int nSocketId)
{
	if (m_arrsSocketData[nSocketId].s_DataSocket != INVALID_SOCKET) 
	{
		closesocket(m_arrsSocketData[nSocketId].s_DataSocket);
		m_arrsSocketData[nSocketId].s_DataSocket = INVALID_SOCKET;
	}
	m_arrsSocketData[nSocketId].bDrop = TRUE;
}


static UINT ThreadAcceptConnection(LPVOID pParam)
{
	CSocketServer* pSocketServer = (CSocketServer*)pParam;
	static sockaddr_in dst;
	static int dst_len = sizeof(dst);
	CString strWaitingStatusLabel;
	CString strMsg;

	strWaitingStatusLabel.Format(_T("Server Port:%d is waiting for connection"),pSocketServer->m_nPort);

	while(TRUE)
	{
		if (pSocketServer->m_bRequestTerminateAcceptThread)
		{
			pSocketServer->m_IsConnectingIdle = TRUE;
			strMsg.Format(_T("Terminating Accept Connection"));
			break;
		}
		if(!pSocketServer->m_bUserRequestConnect)
		{
			pSocketServer->m_IsConnectingIdle = TRUE;
			Sleep(50);
			continue;
		}
		pSocketServer->m_IsConnectingIdle = FALSE;
		// See if connection pending
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(pSocketServer->m_sListenSocket, &readSet);
		timeval timeout;
		timeout.tv_sec = 0;  // Zero timeout (poll)
		timeout.tv_usec = 0;
		if(select((int)pSocketServer->m_sListenSocket, &readSet, NULL, NULL, &timeout) == 1)
		{
			SOCKET		dataSocket;
			dataSocket = accept(pSocketServer->m_sListenSocket,(struct sockaddr *)&dst, &dst_len);
			// Set 100 milliseconds to receive timeout.
			int timeout = 100;
			setsockopt(dataSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

			CSocketServer::SSOCKETDATA sSocketData;
			sSocketData.s_DataSocket		= dataSocket;
			sSocketData.s_ClientSocketAddr	= dst;
			sSocketData.t_LastProcessed		= 0;
			sSocketData.bDrop				= FALSE;

			pSocketServer->m_csSocketListOp.Lock();
			pSocketServer->m_arrsSocketData.push_back(sSocketData);
			pSocketServer->m_csSocketListOp.Unlock();

			CString strMsg;
			strMsg.Format(_T("%S(%d) connected to server"), inet_ntoa(dst.sin_addr), dst.sin_port);
			pSocketServer->m_pDlg->UpdateDialogUIStatus(strMsg);
			pSocketServer->m_pDlg->UpdateDialogUIClientList(pSocketServer->GetClientListString());
		}
		else
			Sleep(500);
	}
	TRACE0("========================ThreadAcceptConnection EXITED");
	return 0;
}

static UINT ThreadReceiveData(LPVOID pParam)
{
	CSocketServer* pSocketServer = (CSocketServer*)pParam;
	int nDots = 0;

	while(TRUE)
	{
		if (pSocketServer->m_bRequestTerminateReceiveThread)
		{
			pSocketServer->m_IsReceivingIdle = TRUE;
			break;
		}

		if(!pSocketServer->m_bUserRequestConnect)
		{
			pSocketServer->m_IsReceivingIdle = TRUE;
			Sleep(50);
			continue;
		}

		pSocketServer->m_IsReceivingIdle = FALSE;

		int nTotalClient = int(pSocketServer->m_arrsSocketData.size());
		for(int nClientId = 0; nClientId < nTotalClient; nClientId++)
		{
			if (pSocketServer->m_bRequestTerminateReceiveThread)
				break;

			if(pSocketServer->m_arrsSocketData[nClientId].bDrop==FALSE)
				pSocketServer->Receive(nClientId);		//simple way to loop, not fair if the items in the list keep changing
		}

		int nClientAlive = 0;
		for(int nClientId = 0; nClientId < nTotalClient; nClientId++)
			if(pSocketServer->m_arrsSocketData[nClientId].bDrop==FALSE)
				nClientAlive++;
		if(nClientAlive==0)
			Sleep(50);

		nDots++;
		nDots = nDots%3;
	}

	TRACE0("========================ThreadReceiveData EXITED");

	return 0;
}
