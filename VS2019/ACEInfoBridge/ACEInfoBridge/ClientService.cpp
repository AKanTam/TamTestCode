#pragma once
#include "StdAfx.h"
#include "ClientService.h"
#include "NetworkMsg.h"
//#include "ID.h"
//extern int g_ListenSocket;
//extern CID g_ID;
//extern Deque_Send g_Deque_Send;
//extern Deque_Recv g_Deque_Recv;
//extern Map_ID_to_Socket g_Map_IdToUpSocket;
extern CCriticalSection g_SendCS;
extern CCriticalSection g_RecvCS;
//extern CCriticalSection g_MapIdCS;
////extern Map_Socket_to_OB g_Map_SocketToOB;
extern deque<NetworkMsg*> g_socket_RecvQueue;
extern deque<NetworkMsg*> g_socket_SendQueue;
extern int g_recv;
extern HWND g_hWnd;
//CString recvData;


ClientService::ClientService(void)
{
	//memset(m_rev_buf,0,BUF_SIZE);
	//m_pPos = m_rev_buf;
	memset(m_Pos,0,500);
	m_pRev_Buf = NULL;
	m_RemainLen_Head = 500;
	m_RemainLen_Data = 0;
	m_nCount = 1;
}

ClientService::~ClientService(void)
{

}

int ClientService::open(void *p )
{
	if (super::open(p) == -1)
	{
		return -1;
	}

	//int number=0;
	//int aarecbuf=0;
	//int len = sizeof(number);
	//int bb = peer().get_option(SOL_SOCKET,SO_RCVBUF,(char*)&aarecbuf,&len);
	//int nSocketBuffSize = 30000; 
	//peer().set_option(SOL_SOCKET, SO_RCVBUF, &nSocketBuffSize, sizeof(int)); 
	//bb = peer().get_option(SOL_SOCKET,SO_RCVBUF,(char*)&aarecbuf,&len);
	//int a = 0;

	//ACE_TCHAR peer_name[1024];
	//ACE_INET_Addr peer_addr;

    //SOCKET mysock = (SOCKET)(peer().get_handle());
	ACE_SOCK_Stream sock_stream = peer().get_handle();
	int one;
	sock_stream.set_option(ACE_IPPROTO_TCP,TCP_NODELAY,&one,sizeof(one));
	/*
	int ListenSocket = (int)mysock;
	int m = peer().get_remote_addr(peer_addr);
	int n = peer_addr.addr_to_string(peer_name,1024);
	if (m == 0 && n == 0)
	{
		reactor()->register_handler(this,ACE_Event_Handler::WRITE_MASK);
		ACE_Reactor::instance()->cancel_wakeup(this, WRITE_MASK);
		int ConnId = g_ID.Generate_ConnID();
		g_MapIdCS.Lock();
		g_Map_IdToUpSocket.insert(pair<int,SOCKET>(ConnId, mysock));
		g_MapIdCS.Unlock();
		//g_Map_SocketToOB.insert(pair<int,ClientService*>(ListenSocket, this));
		//TRACE("ACEServer ClientService Open");
		TRACE("ACEServer ClientService Open ConnId= %d sock = %d addr = %s\n",ConnId,mysock,peer_name);
	//	return 0;
	}
	*/
	return 0;
}

int ClientService::handle_input(ACE_HANDLE fd)
{
	int ret_head = peer().recv((char*)m_Pos, 500);
	if (SOCKET_ERROR == ret_head || ret_head <0)
	{
		return -1;
	}
	if (ret_head == 0 && EWOULDBLOCK == ACE_OS::last_error())
	{
		return 0;
	}
	if (ret_head == 0) {
		TRACE("server recv return without data.\n");
		return -1;
	}
	TRACE("server recv message: %s\n", m_Pos);
	NetworkMsg* msg = new NetworkMsg(m_Pos, ret_head);
	g_RecvCS.Lock();
	g_socket_RecvQueue.push_front(msg);
	g_recv++;
	PostMessage(g_hWnd, WM_REDRAW, NULL, NULL);
	g_RecvCS.Unlock();
	return 0;
}



int ClientService::handle_output(ACE_HANDLE fd  )
{
	//ACE_Reactor::instance()->cancel_wakeup(this, WRITE_MASK);
	Sleep(10);
	return 0;
}

int ClientService::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
	if (close_mask == ACE_Event_Handler::WRITE_MASK)
	{
		return 0;
	}
	/*
	Map_ID_to_Socket::iterator it;
	SOCKET sock = (SOCKET)peer().get_handle();
	int id;
	g_MapIdCS.Lock();
	for (it=g_Map_IdToUpSocket.begin();it!=g_Map_IdToUpSocket.end();it++)
	{
		if (it->second == sock)
		{
			id = it->first;
			g_Map_IdToUpSocket.erase(it);
			g_ID.UnReg_ConnID(id);
			break;
		}
	}
	g_MapIdCS.Unlock();
	TRACE("ACEServer ClientService handle_input handle_close sock = %d\n",sock);
	*/
	int ret = super::handle_close(handle,close_mask);
	return ret;
}