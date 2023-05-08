//CTcpClient.h
#pragma once

#include <string>
#include <winsock2.h>

//#pragma comment(lib,"ws2_32")//Standard socket API.

class CTcpClient
{
public:
    CTcpClient();
	CTcpClient(std::string strServerIp, unsigned uServerPort);
	virtual ~CTcpClient();

	//建立连接
	bool InitConnect();

	//发送数据
	bool SendMsg(const std::string& strMsg);

	//接收数据并打印
	bool RecvMsg(std::string& _rec);

	bool Close();

public:
	SOCKET m_socket = INVALID_SOCKET;
	std::string m_strServerIp;//服务端监听IP地址
	unsigned int m_uServerPort = -1;//服务端监听端口
	struct addrinfo* m_servAddrInfo = NULL;//服务端地址结构链表
};