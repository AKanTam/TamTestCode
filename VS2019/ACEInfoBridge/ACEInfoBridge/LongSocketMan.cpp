// SendToClientThread.cpp : 实现文件
//
#pragma once
#include "stdafx.h"
#include "ACEInfoBridge.h"
#include "LongSocketMan.h"
//#include "ID.h"
#include "NetworkMsg.h"
#include "SocketConn.h"

CString pulseStr = "PULSE";

#define MAX_LOADSTRING 100
extern SOCKET client_socket;

//extern Deque_Recv g_Deque_Recv;
extern CCriticalSection g_RecvCS;
//extern CCriticalSection g_MapIdCS;
//extern Map_ID_to_Socket g_Map_IdToUpSocket;
extern deque<NetworkMsg*> g_socket_RecvQueue;
extern deque<NetworkMsg*> g_socket_SendQueue;

//extern BOOL g_bLoopSend;
// CLongSocketMan


IMPLEMENT_DYNCREATE(CLongSocketMan, CWinThread)

CLongSocketMan::CLongSocketMan()
{
	//memset(m_sendBuf,0,BUF_SIZE);
}

CLongSocketMan::~CLongSocketMan()
{
}

BOOL CLongSocketMan::InitInstance()
{
	// TODO: 在此执行任意逐线程初始化
	//work();
	//StartThread(NULL, NULL);
	//TRACE("ENTERED INSTANCE");
	bExitThread = false;
	return TRUE;
}

int CLongSocketMan::ExitInstance()
{
	// TODO: 在此执行任意逐线程清理
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CLongSocketMan, CWinThread)
		ON_THREAD_MESSAGE(WM_StartSocketManThread,StartThread)
END_MESSAGE_MAP()

void CLongSocketMan::EndThread(WPARAM wParam,LPARAM lParam){
	bExitThread = true;
	pRemoteConn->StopThread();
}

void CLongSocketMan::StartThread(WPARAM wParam,LPARAM lParam)
{
	/*
	code to end loop
	MSG msg;
	if(::PeekMessage(&msg,NULL,WM_MINMSG,WM_MAXMSG,PM_NOREMOVE))
	{
		if (msg.message == WM_StopSocketManThread)
		{
			break;
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	*/
	TRACE("Entered Long Socket Manager.\n");
	MSG msg;
	pRemoteConn = (CRemoteConn*)AfxBeginThread(RUNTIME_CLASS(CRemoteConn), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	pRemoteConn->ResumeThread();
	TRACE("Thread remote send started.\n");
	Sleep(50);
	pRemoteConn->PostThreadMessage(WM_StartRemoteConnThread, NULL, NULL);
	SocketConn* socketConn = new SocketConn();

	while(!bExitThread){
		if(::PeekMessage(&msg,NULL,WM_MINMSG,WM_MAXMSG,PM_NOREMOVE))
		{
			if (msg.message == WM_StopSocketManThread)
			{
				bExitThread = true;
				pRemoteConn->StopThread();
				break;
			}
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		if (!socketConn->Conn()) {
			continue;
		}
		client_socket = socketConn->getSocket();
		int ret = 0;
		while (ret != INVALID_SOCKET) {
			if(::PeekMessage(&msg,NULL,WM_MINMSG,WM_MAXMSG,PM_NOREMOVE))
			{
				if (msg.message == WM_StopSocketManThread)
				{
					bExitThread = true;
					pRemoteConn->StopThread();
					break;
				}
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			if (client_socket == INVALID_SOCKET) {
				break;
			}
			ret = send(client_socket , pulseStr, pulseStr.GetLength(), 0);
			Sleep(2000);
		}
		client_socket = INVALID_SOCKET;
		TRACE("Exit thread or connection lost. WSA said: %d.\n", WSAGetLastError());
	}
	socketConn->close();
	Sleep(50);
	pRemoteConn->StopThread();
	delete socketConn;
	::AfxEndThread(2000);
}
