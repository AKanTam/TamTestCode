#pragma once
#include "NetworkMsg.h"
extern deque<NetworkMsg*> g_socket_RecvQueue;
extern deque<NetworkMsg*> g_socket_SendQueue;


// thread_parse

class thread_parse : public CWinThread
{
	DECLARE_DYNCREATE(thread_parse)

protected:
	thread_parse();           // 动态创建所使用的受保护的构造函数
	virtual ~thread_parse();
	void logMsg(const char* msg, int length, const char* filename);

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	afx_msg void StartThread(WPARAM wParam,LPARAM lParam);
	NetworkMsg* DoParse(char* input, int length);

protected:
	DECLARE_MESSAGE_MAP()
};


