#pragma once



#include "RemoteConn.h"
// CSendToClientThread

class CLongSocketMan : public CWinThread
{
	DECLARE_DYNCREATE(CLongSocketMan)

protected:
	CLongSocketMan();           // 动态创建所使用的受保护的构造函数
	virtual ~CLongSocketMan();
	CRemoteConn *pRemoteConn;
	bool bExitThread;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//void work();
	afx_msg void StartThread(WPARAM wParam,LPARAM lParam);
	afx_msg void EndThread(WPARAM wParam,LPARAM lParam);
	//char m_sendBuf[BUF_SIZE];
protected:
	DECLARE_MESSAGE_MAP()
};



