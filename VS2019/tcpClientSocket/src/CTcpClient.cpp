/**
 * @file CTcpClient.cpp
 * @author Tam (821239820@qq.com)
 * @brief
 * @version 0.1
 * @date 2023-04-06
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <sstream>
#include <iostream>
#include <Ws2tcpip.h>
#include <mstcpip.h>
#include "CTcpClient.h"

CTcpClient::CTcpClient(std::string strServerIp, unsigned uServerPort) : m_strServerIp(strServerIp),
																		m_uServerPort(uServerPort)
{
	this->Init();
}

CTcpClient::~CTcpClient()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = NULL;
	}

	WSACleanup();
	freeaddrinfo(m_servAddrInfo);
	m_servAddrInfo = NULL;
}

bool CTcpClient::Init()
{
	WSADATA wsaData;

	// 1. 初始化环境
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		// std::cout << "Init Windows Socket Failed!\n";
		return false;
	}
	addrinfo hints = {0}; // 协议无关(IPV4 or IPV6)

	hints.ai_flags = AI_NUMERICHOST;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	std::stringstream ssPort;

	ssPort << m_uServerPort;

	// 获取服务端地址结构
	if (getaddrinfo(m_strServerIp.c_str(), ssPort.str().c_str(), &hints, &m_servAddrInfo) != 0)
	{
		// std::cout << "Get server addrInfo failed!\n";
		is_init = false;
		return false;
	}

	is_init = true;
	return true;
}

bool CTcpClient::Connect(int timeout)
{
	if (!is_init)
		return false;
	if (is_connect)
		return true;

	m_socket = INVALID_SOCKET;

	// 2. 创建一个新的套接字
	if ((m_socket = socket(m_servAddrInfo->ai_family, m_servAddrInfo->ai_socktype, m_servAddrInfo->ai_protocol)) == SOCKET_ERROR)
	{
		is_connect = false;
		return false;
	}

	setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(int));
	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));

	// 3. 建立连接
	int iResult = connect(m_socket, m_servAddrInfo->ai_addr, (int)m_servAddrInfo->ai_addrlen);

	if (iResult == 0) {
		// std::cout << "Connect Server succeed!\n";
		is_connect = true;
		return true;
	}

	if (iResult == SOCKET_ERROR)
	{
		iResult = WSAGetLastError();

		if (iResult != WSAEWOULDBLOCK)
		{
			//std::cout << iResult <<"Connect server failed!\n";
			// closesocket(m_socket);
			// m_socket = INVALID_SOCKET;
		}
		is_connect = false;
		return false;
	}


}

bool CTcpClient::Connected()
{
	return is_connect;
}

bool CTcpClient::SendMsg(const std::string &strMsg)
{
	if (!m_socket)
		return false;
	if (!is_init)
		return false;
	if (!is_connect)
		return false;

	int _ret = send(m_socket, strMsg.c_str(), strMsg.length(), 0);

	if (_ret > 0)
	{
		// std::cout << "发送成功:" << strMsg << "\n";
		return true;
	}
	else if (_ret < 0)
	{
		this->Close();
		// std::cout << "发送失败!\n";

		return false;
	}
	else // if (_ret == 0)
	{
		this->Close();
		return false;
	}
}

bool CTcpClient::RecvMsg(std::string &_rec)
{
	if (!m_socket)
		return false;
	if (!is_init)
		return false;
	if (!is_connect)
		return false;

	char recvBuf[10240] = {0};
	auto iRecvSize = recv(m_socket, recvBuf, 10240, 0); // 若不支持C++11及以上，auto改为int

	if (iRecvSize <= 0)
	{
		this->Close();
		// std::cout << "接收失败!\n";
		return false;
	}
	else if (iRecvSize == 0)
	{
		this->Close();
	}
	else // if (iRecvSize > 0)
	{
		_rec.clear();
		_rec = recvBuf;
		// std::cout << "接收成功:" << recvBuf << "\n";
		return true;
	}
}

bool CTcpClient::Close()
{
	if (!m_socket)
		return false;
	if (!is_init)
		return false;
	if (!is_connect)
		return true;

	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = NULL;
	}

	is_connect = false;
	return true;
}
