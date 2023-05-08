// SendToClientThread.cpp : 实现文件
//
#pragma once
#include "stdafx.h"
#include "ACEInfoBridge.h"
#include "RemoteConn.h"
#include "NetworkMsg.h"
//#include "ID.h"

#define MAX_LOADSTRING 100

char recvBuf[1000];
SOCKET client_socket;

//extern Deque_Recv g_Deque_Recv;
extern CCriticalSection g_RecvCS;
extern CCriticalSection g_SendCS;
//extern Map_ID_to_Socket g_Map_IdToUpSocket;
extern deque<NetworkMsg*> g_socket_RecvQueue;
extern deque<NetworkMsg*> g_socket_SendQueue;
extern int g_send;
extern HWND g_hWnd;

//extern BOOL g_bLoopSend;
// CSendToClientThread



int hton(int value) {
	char temp[4], temp2[4];
	int ret;
	memcpy(temp, &value, 4);
	temp2[0] = temp[3];
	temp2[1] = temp[2];
	temp2[2] = temp[1];
	temp2[3] = temp[0];
	memcpy(&ret, temp2, 4);
	return ret;
}

IMPLEMENT_DYNCREATE(CRemoteConn, CWinThread)

CRemoteConn::CRemoteConn()
{
	client_socket = INVALID_SOCKET;
	threadStopSig = false;
	//memset(m_sendBuf,0,BUF_SIZE);
}

CRemoteConn::~CRemoteConn()
{
}

BOOL CRemoteConn::InitInstance()
{
	return true;
}

int CRemoteConn::ExitInstance()
{
	// TODO: 在此执行任意逐线程清理
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CRemoteConn, CWinThread)
		ON_THREAD_MESSAGE(WM_StartRemoteConnThread,StartThread)
END_MESSAGE_MAP()

void CRemoteConn::StartThread(WPARAM wParam,LPARAM lParam)
{
	TRACE("Message send loop.\n");
	int recvCount;
	while (true) {
		while (!threadStopSig) {
			g_SendCS.Lock();
			if(g_socket_SendQueue.size() != 0){
				g_SendCS.Unlock();
				break;
			}
			g_SendCS.Unlock();
			Sleep(50);
		}
		while (client_socket == INVALID_SOCKET && !threadStopSig) {
			Sleep(50);
		}
		if (threadStopSig) {
			break;
		}
		g_SendCS.Lock();
		NetworkMsg* msg_send = g_socket_SendQueue.front();
		g_SendCS.Unlock();
		timeval tmOut;
		tmOut.tv_sec = 0;
		tmOut.tv_usec = 0;
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(client_socket, &fds);
		char tmp[2];
		while(true){
			int read = select(FD_SETSIZE, &fds, NULL, NULL, &tmOut);
			if(read == 0) break;
			recv(client_socket, tmp, 1, 0);
		}
		int net_length;
		net_length = hton(msg_send->getLength());
		char head_dat[6];
		head_dat[0] = 2;
		head_dat[5] = 3;
		memcpy(head_dat+1, &net_length, 4);
		int ret = send(client_socket, (char*)&head_dat, 6, 0);
		if (ret == INVALID_SOCKET) {
			TRACE("send() error. The error code is: %d\n", WSAGetLastError());
			client_socket = INVALID_SOCKET;
		}
		memset(recvBuf, 0, 1000);
		recvCount = recv(client_socket, recvBuf, 1000, 0);
		TRACE("receive data %d from server side is: %s\n", recvCount, recvBuf);
		if (recvCount >=3 && recvBuf[0] == 'A' && recvBuf[1] == 'C' && recvBuf[2] == 'K') {
			char* msg = msg_send->getMsg();
			ret = send(client_socket, msg, msg_send->getLength(), 0);
			delete msg;
			if (ret == INVALID_SOCKET) {
				TRACE("send() error. The error code is: %d\n", WSAGetLastError());
				delete msg;
				client_socket  = INVALID_SOCKET;
			}
			if (threadStopSig) {
				delete msg;
				break;
			}
			memset(recvBuf, 0, 1000);
			recvCount = recv(client_socket, recvBuf, 1000, 0);
			TRACE("receive data %d from server side is: %s\n", recvCount, recvBuf);
			if (recvCount < 3 || !(recvBuf[0] == 'E' && recvBuf[1] == 'N' && recvBuf[2] == 'D')) {
				TRACE("Not END message. Ignore.\n");
			}
			g_SendCS.Lock();
			g_socket_SendQueue.pop_front();
			if (g_socket_SendQueue.size() == 0) {
				g_socket_SendQueue.shrink_to_fit();
			}
			delete msg_send;
			g_send++;
			PostMessage(g_hWnd, WM_REDRAW, NULL, NULL);
			g_SendCS.Unlock();
		}
		else {
			TRACE("Not ACK message. Retry sending...\n");
		}
	}
	TRACE("Exit loop.\n");
	::AfxEndThread(2000);
}

void CRemoteConn::StopThread()
{
	threadStopSig = true;
}

/*
// CSendToClientThread 消息处理程序
void CSendToClientThread::StartThread(WPARAM wParam,LPARAM lParam)
{
		Map_ID_to_Socket::iterator it,ItEnd;
	SOCKET sock;
	while(g_bLoopSend)
	{
		g_RecvCS.Lock();
		BOOL IfEmpty = g_Deque_Recv.empty();
		g_RecvCS.Unlock();
		if (IfEmpty)
		{
			Sleep(10);
			//g_RecvCS.Unlock();
			continue;
		}

// 		if (g_Deque_Recv.empty())
// 		{
// 			g_RecvCS.Unlock();
// 			continue;
// 		}
		ASSERT(g_Deque_Recv.size());
		g_RecvCS.Lock();
		int num = g_Deque_Recv.size();
		TRACE("[HQF-1] num of g_Deque_Recv = %d",num);
		struct_RecvData recvData= g_Deque_Recv.back();
		g_Deque_Recv.pop_back();
		g_RecvCS.Unlock();

		int ConnID = recvData.head.sConnID2;

		TRACE("ACEServer---CSendToClientThread:ConnID = %d\n",ConnID);
		g_MapIdCS.Lock();
		it = g_Map_IdToUpSocket.find(ConnID);
		ItEnd = g_Map_IdToUpSocket.end();
		//g_MapIdCS.Unlock();

		//continue
		if (it == ItEnd)   
		{
			TRACE("ACEServer---CSendToClientThread:ConnID  not find \n");
			char * p = (char *)recvData.pRevBuf;
			if (p != NULL)
			{
				TRACE("ACEServer---CSendToClientThread:p=NULL \n");
				delete []p;
				p = NULL;
			}
			g_MapIdCS.Unlock();
			continue;
		}
		else
		{
			g_MapIdCS.Unlock();
			sock = it->second;
			int len = recvData.head.nLength;
			char *pSenBuf = new char [len+SIZE_STRCUT_PACKHEAD];

			memcpy(pSenBuf,(char*)&recvData,sizeof(strcut_PackHead));
			char* pData = (char *)recvData.pRevBuf;
			memcpy(pSenBuf+SIZE_STRCUT_PACKHEAD,pData,len);

			int Data_len = sizeof(strcut_PackHead)+len;
			int remain_len = Data_len;
		
 			CString tmp;
 			tmp.Format("[ACEServer]---revData:command[%d],len[%d],id1[%d]\n", recvData.head.sCommand, recvData.head.nLength, recvData.head.sConnID1);
 			TRACE(tmp);

			while (1)
			{
				int ret = send((SOCKET)sock,(char *)pSenBuf+(Data_len-remain_len),remain_len,0);
				//break
				DWORD err = GetLastError();
				if (ret == SOCKET_ERROR || ret == 0)
				{
					//delete []pData;
					if (err == 10035 && ret == SOCKET_ERROR)
					{
						TRACE("[Hqf---Server Send] SendtoClient err == 10035\n");
						continue;
					}

					if (pData != NULL)
					{
						delete []pData;
						pData = NULL;
					}
// 					g_RecvCS.Lock();
// 					g_Deque_Recv.pop_back();
// 					g_RecvCS.Unlock();
					TRACE("ACEServer---CSendToClientThread:sendData  SOCKET_ERROR\n");

					break;
				}
				remain_len = remain_len - ret;
				//continue
				if (remain_len > 0)
				{
					continue;
				}
				//break
				else if (remain_len == 0)
				{
					TRACE("[ACEServer] ---CSendToClientThread:sendData   sock = %d\n",sock);
					//delete []pData;
					if (pData != NULL)
					{
						delete []pData;
						pData = NULL;
					}
// 					g_RecvCS.Lock();
// 					g_Deque_Recv.pop_back();
// 					g_RecvCS.Unlock();
					break;
				}
			}
			if (pSenBuf)
			{
				delete []pSenBuf;
				pSenBuf = NULL;
			}
		}
		//Sleep(100); // 两个数据包之间延时 -ylb
	}
	::AfxEndThread(2000);
}
*/
/*
void CRemoteConn::work()
{
	Map_ID_to_Socket::iterator it,ItEnd;
	SOCKET sock;
	TRACE("ENTERED WORK");
	while(g_bLoopSend)
	{
		g_RecvCS.Lock();
		BOOL IfEmpty = g_Deque_Recv.empty();
		g_RecvCS.Unlock();
		if (IfEmpty)
		{
			Sleep(10);
			//g_RecvCS.Unlock();
			continue;
		}

// 		if (g_Deque_Recv.empty())
// 		{
// 			g_RecvCS.Unlock();
// 			continue;
// 		}
		ASSERT(g_Deque_Recv.size());
		g_RecvCS.Lock();
		int num = g_Deque_Recv.size();
		TRACE("[HQF-1] num of g_Deque_Recv = %d",num);
		struct_RecvData recvData= g_Deque_Recv.back();
		g_Deque_Recv.pop_back();
		g_RecvCS.Unlock();

		int ConnID = recvData.head.sConnID2;

		g_MapIdCS.Lock();
		it = g_Map_IdToUpSocket.find(ConnID);
		ItEnd = g_Map_IdToUpSocket.end();
		//g_MapIdCS.Unlock();

		//continue
		if (it == ItEnd)   
		{
			char * p = (char *)recvData.pRevBuf;
			if (p != NULL)
			{
				delete []p;
				p = NULL;
			}
			g_MapIdCS.Unlock();
			continue;
		}
		else
		{
			g_MapIdCS.Unlock();
			sock = it->second;
			int len = recvData.head.nLength;
			char *pSenBuf = new char [len+SIZE_STRCUT_PACKHEAD];

			memcpy(pSenBuf,(char*)&recvData,sizeof(strcut_PackHead));
			char* pData = (char *)recvData.pRevBuf;
			memcpy(pSenBuf+SIZE_STRCUT_PACKHEAD,pData,len);

			int Data_len = sizeof(strcut_PackHead)+len;
			int remain_len = Data_len;
		
// 			CString tmp;
// 			tmp.Format("revData:command[%d],len[%d],id1[%d]\n", recvData.head.sCommand, recvData.head.nLength, recvData.head.sConnID1);
// 			TRACE(tmp);

			while (1)
			{
				int ret = send((SOCKET)sock,(char *)pSenBuf+(Data_len-remain_len),remain_len,0);
				//break
				DWORD err = GetLastError();
				if (ret == SOCKET_ERROR || ret == 0)
				{
					//delete []pData;
					if (err == 10035 && ret == SOCKET_ERROR)
					{
						TRACE("[Hqf---Server Send] SendtoClient err == 10035\n");
						continue;
					}

					if (pData != NULL)
					{
						delete []pData;
						pData = NULL;
					}
// 					g_RecvCS.Lock();
// 					g_Deque_Recv.pop_back();
// 					g_RecvCS.Unlock();
					//TRACE("ACEServer---CSendToClientThread:sendData  SOCKET_ERROR\n");

					break;
				}
				remain_len = remain_len - ret;
				//continue
				if (remain_len > 0)
				{
					continue;
				}
				//break
				else if (remain_len == 0)
				{
					//delete []pData;
					if (pData != NULL)
					{
						delete []pData;
						pData = NULL;
					}
// 					g_RecvCS.Lock();
// 					g_Deque_Recv.pop_back();
// 					g_RecvCS.Unlock();
					break;
				}
			}
			if (pSenBuf)
			{
				delete []pSenBuf;
				pSenBuf = NULL;
			}
		}
		//Sleep(100); // 两个数据包之间延时 -ylb
	}
	::AfxEndThread(2000);
}
*/