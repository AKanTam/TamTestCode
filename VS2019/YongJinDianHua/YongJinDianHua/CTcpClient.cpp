// CTcpClient.cpp

#include <sstream>
#include <iostream>
#include <Ws2tcpip.h>
#include <mstcpip.h>
#include "CTcpClient.h"

CTcpClient::CTcpClient(std::string strServerIp, unsigned uServerPort) : m_strServerIp(strServerIp),
                                                                        m_uServerPort(uServerPort)
{
}

CTcpClient::~CTcpClient()
{
    if (m_socket != INVALID_SOCKET)
        closesocket(m_socket);

    WSACleanup();
    freeaddrinfo(m_servAddrInfo);
}

bool CTcpClient::InitConnect()
{
    WSADATA wsaData;

    // 1. 初始化环境
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "Init Windows Socket Failed!\n";
        return false;
    }

    addrinfo hints = {
        0,
    }; //协议无关(IPV4 or IPV6)

    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::stringstream ssPort;

    ssPort << m_uServerPort;

    //获取服务端地址结构
    if (getaddrinfo(m_strServerIp.c_str(), ssPort.str().c_str(), &hints, &m_servAddrInfo) != 0)
    {
        std::cout << "Get server addrInfo failed!\n";
        return false;
    }

    if (m_socket != INVALID_SOCKET)
        closesocket(m_socket);

    m_socket = INVALID_SOCKET;

    // 2. 创建一个新的套接字
    if ((m_socket = socket(m_servAddrInfo->ai_family, m_servAddrInfo->ai_socktype, m_servAddrInfo->ai_protocol)) == SOCKET_ERROR)
        return false;

    // 3. 建立连接
    int iResult = connect(m_socket, m_servAddrInfo->ai_addr, (int)m_servAddrInfo->ai_addrlen);

    if (iResult == SOCKET_ERROR)
    {
        iResult = WSAGetLastError();

        if (iResult != WSAEWOULDBLOCK)
        {
            std::cout << "Connect server failed!\n";
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }
    }

    std::cout << "Connect Server succeed!\n";
    return true;
}

bool CTcpClient::SendMsg(const std::string &strMsg)
{
    if (!m_socket)
        return false;

    if (send(m_socket, strMsg.c_str(), strMsg.length(), 0) != INVALID_SOCKET)
    {
        std::cout << "发送成功:" << strMsg << "\n";
        return true;
    }
    else
    {
        std::cout << "发送失败!\n";
        return false;
    }
}

bool CTcpClient::RecvMsg()
{
    if (!m_socket)
        return false;

    const int iBufSize = 1024;
    char recvBuf[iBufSize] = {
        0,
    };
    auto iRecvSize = recv(m_socket, recvBuf, iBufSize, 0); //若不支持C++11及以上，auto改为int

    if (iRecvSize <= 0)
    {
        std::cout << "接收失败!\n";
        return false;
    }
    else
    {
        std::cout << "接收成功:" << recvBuf << "\n";
        return true;
    }
}
