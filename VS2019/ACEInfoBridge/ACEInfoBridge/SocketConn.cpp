// SocketConn.cpp: 实现文件
//

#include "stdafx.h"
#include "ACEInfoBridge.h"
#include "SocketConn.h"

extern int g_connState;
extern HWND g_hWnd;
// SocketConn

SocketConn::SocketConn()
{
	p_socket = INVALID_SOCKET;
}

SocketConn::~SocketConn()
{
}

// SocketConn 成员函数

bool SocketConn::Conn() {
	g_connState++;
	PostMessage(g_hWnd, WM_REDRAW, NULL, NULL);
	TRACE("Init socket\n");
	if (p_socket != INVALID_SOCKET) {
		closesocket(p_socket);
	}
	p_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (p_socket == INVALID_SOCKET) {
		TRACE("remote socket() called failed! The error code is: %d\n", WSAGetLastError());
		return false;
	}
	else {
		TRACE("Client socket created.\n");
	}
	SOCKADDR_IN server_addr;
	int lenAddr = sizeof(struct sockaddr_in);
	memset(&server_addr, 0, lenAddr);
	WSAStringToAddress(WSA_REMOTE_ADDR, AF_INET, NULL, (LPSOCKADDR)&server_addr, &lenAddr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(WSA_REMOTE_PORT);
	int ret = connect(p_socket, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
	if (ret == INVALID_SOCKET) {
		TRACE("remote connect() called failed. The error code is: %d\n", WSAGetLastError());
		return false;
	}
	else {
		TRACE("Connection established.\n");
	}
	g_connState = 0;
	PostMessage(g_hWnd, WM_REDRAW, NULL, NULL);
	return true;
}

SOCKET SocketConn::getSocket() {
	return p_socket;
}

void SocketConn::close() {
	shutdown(p_socket, 0);
}
