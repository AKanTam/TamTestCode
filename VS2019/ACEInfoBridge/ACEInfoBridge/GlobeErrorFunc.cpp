#pragma once
#include "StdAfx.h"
//#include "GlobalDef.h"

extern Deque_Send g_Deque_Send;
extern Deque_Recv g_Deque_Recv;
extern CCriticalSection g_SendCS;
extern CCriticalSection g_RecvCS;
     
void SetErrorRecvDeque(struct_SendData SendData, short sErrorCode )
{

	struct_RecvData ErrorRecv;
	ErrorRecv.head = SendData.head;
	ErrorRecv.head.sErrorCode = sErrorCode;
	ErrorRecv.head.nLength = 0;
	ErrorRecv.pRevBuf = NULL;
	
	g_RecvCS.Lock();
	g_Deque_Recv.push_front(ErrorRecv);
	g_RecvCS.Unlock();
}