// CTcpServer.cpp
#include <iostream>
#include "CTcpServer.h"

CTcpServer::CTcpServer(std::string strIp, unsigned int uPort) : m_strIp(strIp),
                                                                m_uPort(uPort)
{
}

CTcpServer::~CTcpServer()
{
    if (m_clientSocket)
    {
        closesocket(m_clientSocket);
        m_clientSocket = NULL;
    }

    if (m_listenSocket)
    {
        closesocket(m_listenSocket);
        m_listenSocket = NULL;
    }

    WSACleanup();
}

bool CTcpServer::InitServer()
{
    WSADATA wsaData;

    // 1. 初始化环境
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "Init Windows Socket Failed!\n";
        return false;
    }

    // 2. 创建监听套接字
    if ((m_listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        std::cout << "Create socket failed!\n";
        return false;
    }

    //协议
    sockaddr_in sockadd = {
        0,
    };

    sockadd.sin_family = AF_INET;                     // IPV4协议簇
    sockadd.sin_port = htons(m_uPort);                //监听端口
    sockadd.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //监听本机任意IP

    // 3. 监听套接字与IP地址及端口绑定
    if (bind(m_listenSocket, (struct sockaddr *)&sockadd, sizeof(sockadd)) == SOCKET_ERROR)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        std::cout << "Socket bind failed!\n";
        return false;
    }

    // 4. 监听套接字
    if (listen(m_listenSocket, 1) == SOCKET_ERROR)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        std::cout << "Socket listen failed!\n";
        return false;
    }

    sockaddr_in addr = {
        0,
    };
    int addrlen = sizeof(addr);

    // 5. 等待客户端连接
    m_clientSocket = accept(m_listenSocket, (struct sockaddr *)&addr, &addrlen);

    if (m_clientSocket == SOCKET_ERROR)
    {
        closesocket(m_clientSocket);
        m_clientSocket = INVALID_SOCKET;
        std::cout << "Socket accept failed!\n";
        return false;
    }

    return true;
}

bool CTcpServer::SendMsg(const std::string &strMsg)
{
    if (!m_clientSocket)
        return false;

    if (send(m_clientSocket, strMsg.c_str(), strMsg.length(), 0) != INVALID_SOCKET)
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

bool CTcpServer::RecvMsg()
{
    if (!m_clientSocket)
        return false;

    const int iBufSize = 1024;
    char recvBuf[iBufSize] = {
        0,
    };
    auto iRecvSize = recv(m_clientSocket, recvBuf, iBufSize, 0); //若不支持C++11及以上，auto改为int

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