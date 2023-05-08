#pragma once



// CRemoteConn

class CRemoteConn : public CWinThread
{
	DECLARE_DYNCREATE(CRemoteConn)

protected:
	CRemoteConn();           // 动态创建所使用的受保护的构造函数
	virtual ~CRemoteConn();
	bool threadStopSig;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//void work();
	//void stopLoop();
	//char m_sendBuf[BUF_SIZE]; 
	afx_msg void StartThread(WPARAM wParam,LPARAM lParam);
	void StopThread();
protected:
	DECLARE_MESSAGE_MAP()
};


