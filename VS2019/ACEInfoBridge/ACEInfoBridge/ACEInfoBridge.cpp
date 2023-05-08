// ACEInfoBridge.cpp : ����Ӧ�ó������ڵ㡣
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

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������:
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
HMENU menu;               //�˵����
HWND g_hWnd;              //g_hWnd��Ӧ���ڵľ��

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
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	ACE::init();
	
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: �ڴ˷��ô��롣

	HANDLE handle=::CreateMutex(NULL,FALSE, TheSrvID);//handleΪ������HANDLE���͵�ȫ�ֱ���
	if(GetLastError()==ERROR_ALREADY_EXISTS)	
	{
		return FALSE;	
	}

	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ACEPIMSSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ACEPIMSSERVER));


	AfxBeginThread((AFX_THREADPROC)ThreadACE,NULL,THREAD_PRIORITY_NORMAL,0,0,0);
	Sleep(250);

	HANDLE hThread = (HANDLE)_beginthread( ThreadFunc,0, NULL);
	Sleep(250);
	// ����Ϣѭ��:
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
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
//  ע��:
//
//    ����ϣ��
//    �˴�������ӵ� Windows 95 �еġ�RegisterClassEx��
//    ����֮ǰ�� Win32 ϵͳ����ʱ������Ҫ�˺��������÷������ô˺���ʮ����Ҫ��
//    ����Ӧ�ó���Ϳ��Ի�ù�����
//    ����ʽ��ȷ�ġ�Сͼ�ꡣ
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
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����
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
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
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
		// �����˵�ѡ��:
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
		// TODO: �ڴ���������ͼ����...
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

// �����ڡ������Ϣ�������
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
����Stat�Ի���
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
ÿ�������ݱ仯��ʱ�򶼻�call�����������Stat��������
*/
void updateText() {
	if (dlg == NULL) {
		return;
	}
	if (!IsWindow(dlg->GetSafeHwnd())) {
		return;
	}
	CString statStr, tmp;
	statStr.Format("�յ������ݰ�: %d\n", g_recv);
	tmp.Format("���ܶ��г���: %d\n", g_socket_RecvQueue.size());
	statStr += tmp;
	tmp.Format("���͵����ݰ�: %d\n", g_send);
	statStr += tmp;
	tmp.Format("���Ͷ��г���: %d\n", g_socket_SendQueue.size());
	statStr += tmp;
	tmp.Format("OPCָ���ѯ����: %d\n", g_opc);
	statStr += tmp;
	if (g_connState == 0) {
		tmp.Format("�����ӡ�");
	}
	else {
		tmp.Format("δ���ӡ��� %d �γ������ӡ�", g_connState);
	}
	statStr += tmp;
	dlg->StatDlgText.SetWindowTextA(statStr);
}