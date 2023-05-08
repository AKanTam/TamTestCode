// ACEInfoBridge.cpp : 定义应用程序的入口点。
//
//#pragma once
#include "stdafx.h"
#include "ACEInfoBridge.h"
#include "NetworkMsg.h"
#include "ClientService.h"
#include "RemoteConn.h"
#include "LongSocketMan.h"
#include "thread_parse.h"
#include "DialogStat.h"
#include "NotificationIcon.h"
#include "ACE_HeaderEX.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void ShowPopupMenu(HWND hWnd);
void showDlg();
void updateText();


CDialogStat* dlg;
CWinApp winApp;

CCriticalSection g_SendCS;
CCriticalSection g_RecvCS;

CLongSocketMan *pLongSocketMan;
thread_parse *pthread_parse;

CSystemTray g_SystemTray;
int CurrentShow;
HMENU menu;               //菜单句柄
HWND g_hWnd;              //g_hWnd对应窗口的句柄

int g_recv, g_send, g_opc, g_connState;

deque<NetworkMsg*> g_socket_SendQueue;
deque<NetworkMsg*> g_socket_RecvQueue;

#define TheSrvID  "0x78543573, 0xa571, 0xabab, 0xba, 0xcd, 0xef, 0xba, 0x00, 0x78, 0x79, 0x62"

void ThreadFunc(void *p)
{
	pLongSocketMan = (CLongSocketMan*)AfxBeginThread(RUNTIME_CLASS(CLongSocketMan),THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
	pLongSocketMan -> ResumeThread();
	pthread_parse = (thread_parse*)AfxBeginThread(RUNTIME_CLASS(thread_parse),THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
	pthread_parse -> ResumeThread();
	Sleep(50);
	pLongSocketMan -> PostThreadMessage(WM_StartSocketManThread, NULL, NULL);
	pthread_parse -> PostThreadMessage(WM_StartParseThread, NULL, NULL);
	return;
}

void ShowPopupMenu(HWND hWnd) {
	POINT pos;
	GetCursorPos(&pos);
	SetForegroundWindow(hWnd);
	TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, 0, hWnd, NULL);
}

void ThreadACE()
{
	//ACE_Select_Reactor select_reactor;   
	//ACE_Reactor reactor(&select_reactor);   
	//ACE_Reactor::instance(&reactor, 0); 
	ACE_INET_Addr port_to_listen(WSA_LISTEN_PORT);   

	ClientAcceptor acceptor;
	acceptor.reactor(ACE_Reactor::instance()); 
	int one = 0;

	int n = acceptor.open(port_to_listen);

	if (n == -1)
	{
		return;
	}


	ACE_Reactor::instance()->run_reactor_event_loop();
}

int Close()
{
	//g_RawDataGet.ExitInstance();
	return 0;
}

/*
void GetCurrentPath(char *pPath)
{
	CString strPath;
	//get entire path of running application and its title
	GetModuleFileName(NULL,pPath,MAX_PATH);
	strPath.Format(pPath);
	int iPos=strPath.ReverseFind('\\');
	if ((unsigned)iPos == -1)
	{
		return ;
	}
	memset(pPath, 0, MAX_PATH);
	strncpy(pPath, strPath.Left(iPos).GetBuffer(0), iPos);
	pPath[iPos]='\0';
}
*/

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
   AfxWinInit(::GetModuleHandleA(NULL), NULL, NULL, NULL);

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	ACE::init();
	
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。

	HANDLE handle=::CreateMutex(NULL,FALSE, TheSrvID);//handle为声明的HANDLE类型的全局变量
	if(GetLastError()==ERROR_ALREADY_EXISTS)	
	{
		return FALSE;	
	}

	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ACEPIMSSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ACEPIMSSERVER));


	AfxBeginThread((AFX_THREADPROC)ThreadACE,NULL,THREAD_PRIORITY_NORMAL,0,0,0);
	Sleep(250);

	HANDLE hThread = (HANDLE)_beginthread( ThreadFunc,0, NULL);
	Sleep(250);
	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CloseHandle(handle);
	Sleep(200);

	return (int) msg.wParam;
}




//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ACEPIMSSERVER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//MAKEINTRESOURCE(IDC_ACEPIMSSERVER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中
   RECT rt;
   SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rt, 0);
   
   //int xsize = GetSystemMetrics(SM_CXSCREEN);
   //int ysize = GetSystemMetrics(SM_CYSCREEN);
   int xpos = rt.right - 300;
   int ypos = rt.bottom - 300;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
      xpos, ypos, 300, 300, NULL, NULL, hInstance, NULL);
   if (!hWnd)
   {
      return FALSE;
   }
   g_recv = 0;
   g_send = 0;
   g_opc = 0;
   g_connState = 0;
   g_hWnd = hWnd;

   //

   nCmdShow = SW_HIDE;
   CurrentShow = SW_HIDE;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   HMENU mainmenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_TRAYCLICK));
   menu = GetSubMenu(mainmenu, 0); 
   SetMenuDefaultItem(menu, 0, true);
   g_SystemTray.Create(hWnd, ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL)), _T("Supcon IT"));

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_REDRAW:
		//InvalidateRect(hWnd, NULL, true);
		//UpdateWindow(hWnd);
		updateText();
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case WM_ShowMenuClicked:
			/*
			if (CurrentShow == SW_HIDE) {
				CurrentShow = SW_SHOW;
			}
			else {
				CurrentShow = SW_HIDE;
			}
			ShowWindow(hWnd, CurrentShow);
			UpdateWindow(hWnd);
			*/
			showDlg();
			break;
		case IDM_EXIT:
			g_SystemTray.Destroy();
			pLongSocketMan->PostThreadMessage(WM_StopSocketManThread, NULL, NULL);
			pthread_parse->PostThreadMessage(WM_StopParseThread, NULL, NULL);
			Sleep(5000);
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
		/*
	case WM_CLOSE:
		CurrentShow = SW_HIDE;
		ShowWindow(hWnd, CurrentShow);
		UpdateWindow(hWnd);
		break;*/
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TRAY_MSG:
		switch (lParam)
		{/*
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			if (CurrentShow == SW_HIDE) {
				CurrentShow = SW_SHOW;
			}
			else {
				CurrentShow = SW_HIDE;
			}
			ShowWindow(hWnd, CurrentShow);
			UpdateWindow(hWnd);
			break;
			*/
		case WM_LBUTTONUP:
			showDlg();
			//ShowWindow(hWnd, CurrentShow);
			//UpdateWindow(hWnd);
			break;
		case WM_RBUTTONUP:
			ShowPopupMenu(hWnd);
			break;
		default:
			break;
		}
		break;
	//case WM_SCQUIT:
	//	Close();
	//	PostQuitMessage(0);
// 	case WM_CONFIG_RELOAD:
// 		ReloadCfgi();
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

/*
创建Stat对话框。
*/
void showDlg() {
	//TRACE("%d\n", dlg->GetSafeHwnd());
	if (dlg != NULL) {
		if (IsWindow(dlg->GetSafeHwnd())) {
			dlg->DestroyWindow();
			delete dlg;
			dlg = NULL;
			return;
		}
		delete dlg;
		//return;
	}
	dlg = new CDialogStat();
	dlg->Create(CDialogStat::IDD);
	dlg->ShowWindow(SW_SHOW); 
	updateText();
}

/*
每次有数据变化的时候都会call这个函数更新Stat窗口内容
*/
void updateText() {
	if (dlg == NULL) {
		return;
	}
	if (!IsWindow(dlg->GetSafeHwnd())) {
		return;
	}
	CString statStr, tmp;
	statStr.Format("收到的数据包: %d\n", g_recv);
	tmp.Format("接受队列长度: %d\n", g_socket_RecvQueue.size());
	statStr += tmp;
	tmp.Format("发送的数据包: %d\n", g_send);
	statStr += tmp;
	tmp.Format("发送队列长度: %d\n", g_socket_SendQueue.size());
	statStr += tmp;
	tmp.Format("OPC指令查询次数: %d\n", g_opc);
	statStr += tmp;
	if (g_connState == 0) {
		tmp.Format("已连接。");
	}
	else {
		tmp.Format("未连接。第 %d 次尝试连接。", g_connState);
	}
	statStr += tmp;
	dlg->StatDlgText.SetWindowTextA(statStr);
}