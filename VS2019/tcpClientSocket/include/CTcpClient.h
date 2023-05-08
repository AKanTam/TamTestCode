/**
 * @file CTcpClient.h
 * @author Tam (821239820@qq.com)
 * @brief
 * @version 0.1
 * @date 2023-04-06
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32") // Standard socket API.

class CTcpClient
{
public:
	CTcpClient(std::string strServerIp, unsigned uServerPort);
	virtual ~CTcpClient();

	bool Connected();

	// 建立连接
	bool Connect(int timeout = 20);

	// 发送数据
	bool SendMsg(const std::string &strMsg);

	// 接收数据并打印
	bool RecvMsg(std::string &_rec);

	bool Close();

private:
	bool Init();

private:
	bool is_init = false;
	bool is_connect = false;
	SOCKET m_socket = INVALID_SOCKET;
	std::string m_strServerIp;				// 服务端监听IP地址
	unsigned int m_uServerPort = -1;		// 服务端监听端口
	struct addrinfo *m_servAddrInfo = NULL; // 服务端地址结构链表
};