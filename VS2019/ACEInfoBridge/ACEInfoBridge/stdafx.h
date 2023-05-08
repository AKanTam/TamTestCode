// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WSA_REMOTE_ADDR "127.0.0.1"
#define WSA_REMOTE_PORT 1024
#define WSA_LISTEN_PORT 19250

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ

// Windows ͷ�ļ�:
// #include <windows.h>
// 
// C ����ʱͷ�ļ�
// #include <stdlib.h>
// #include <malloc.h>
// #include <memory.h>
// #include <tchar.h>


// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxmt.h>
#endif // _AFX_NO_AFXCMN_SUPPORT

// Local Header Files

// TODO: reference additional headers your program requires here
#pragma once
#pragma warning(disable: 4996)
#pragma warning(disable: 4995)
#pragma warning(disable: 4005)

/*
//////////////////   ACE   ////////////////////////
#include "ace\Init_ACE.h"
#include "ace\ace.h"
#include "ace\INET_Addr.h"
#include "ace\SOCK_Stream.h"
#include "ace\SOCK_Connector.h"
#include "ace\Log_Msg.h"
#include "ace\SOCK_Acceptor.h"
#include "ace\Time_Value.h"
#include "ace\SOCK_Acceptor.h"
#include "ace\Reactor.h"
#include "ace\Message_Block.h"
#include "ace\Message_Queue.h"
#include "ace\Global_Macros.h"
#include "ace\Null_Condition.h"
#include "ace\Auto_Ptr.h"
#include "ace\Svc_Handler.h"
#include "ace\Acceptor.h"
#include "ace\Connector.h"
#include "ace/Select_Reactor.h"
//////////////////  ACE  ////////////////////////*/

/////////////////   STL  ///////////////////////
#include <string>
#include <deque>
#include <map>
#include <vector>
#include <list>
using namespace std;
/////////////////   STL  ///////////////////////
#include <afxinet.h>
#include <afxdao.h>
#include <afxtempl.h>

//#include "WebGlobalDefines.h"
//#include "WebGlobeStruct.h"

//#include "array.h"
//#include "GlobalRedis.h"

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <afx.h>
#include <afx.h>
#include <afxwin.h>
#include <afxwin.h>
#include <afxwin.h>
#include <afxwin.h>

//typedef list<string> strKeyList;
//typedef list<string>::iterator It_strKeyList;
//typedef array_t<char> string_b;
//typedef list<string_b> strbufList;
//typedef list<string_b>::iterator It_strbufList;

//typedef deque<struct_SendData> Deque_Send;
//typedef deque<struct_RecvData> Deque_Recv;
//typedef deque<struct_SendData> Deque_RealDataSend;
//typedef map<int,SOCKET>Map_ID_to_Socket;
//typedef map<string,INT64>Map_Name_to_ID;


#define WM_MINMSG WM_USER + 1
#define WM_MAXMSG WM_USER + 200

#define WM_StartParseThread  WM_USER + 11
#define WM_StopParseThread  WM_USER + 12

#define WM_StartDistributeThread WM_USER + 21
#define WM_StopDistrbuteThread WM_USER + 22

#define WM_StartRemoteConnThread WM_USER + 61
#define WM_StopRemoteConnThread WM_USER + 62

#define WM_StartSocketManThread WM_USER + 71
#define WM_StopSocketManThread WM_USER + 72

#define WM_StartPIMSWriteDBThread WM_USER + 81
#define WM_StopPIMSWriteDBThread WM_USER + 82

#define WM_ShowMenuClicked WM_USER + 92

#define WM_REDRAW WM_USER + 93
