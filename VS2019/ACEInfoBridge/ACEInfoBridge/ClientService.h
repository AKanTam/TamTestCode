#pragma once
#include "stdafx.h"
#include "ACE_HeaderEX.h"


typedef ACE_Svc_Handler<ACE_SOCK_STREAM,ACE_NULL_SYNCH> super;


class ClientService:public ACE_Svc_Handler<ACE_SOCK_STREAM,ACE_NULL_SYNCH> 
{
public:
	ClientService(void);
	~ClientService(void);
	int open(void *p = NULL);

	virtual int handle_input(ACE_HANDLE fd  = ACE_INVALID_HANDLE );
	virtual int handle_output(ACE_HANDLE fd  = ACE_INVALID_HANDLE );
	virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
	//char m_rev_buf[BUF_SIZE];
	//char *m_pPos;
	char *m_pRev_Buf;
	char m_Pos[500];

	int m_RemainLen_Head;
	int m_RemainLen_Data;
//	strcut_PackHead m_head;
//	struct_RecvData m_rData;
//	struct_SendData m_sData;
	int	m_nCount;
};
typedef ACE_Acceptor<ClientService,ACE_SOCK_ACCEPTOR> ClientAcceptor;

