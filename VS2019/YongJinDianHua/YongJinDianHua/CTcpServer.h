// CTcpServer.h
#pragma once

#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32") // Standard socket API.

class CTcpServer
{
public:
    CTcpServer(std::string strIp, unsigned int uPort);
    virtual ~CTcpServer();

    //初始化网络服务端
    bool InitServer();

    //发送数据
    bool SendMsg(const std::string &strMsg);

    //接收数据并打印
    bool RecvMsg();

private:
    unsigned int m_uPort; //监听端口
    std::string m_strIp;  //用于监听本机指定IP地址

    SOCKET m_listenSocket = NULL; //监听套接字
    SOCKET m_clientSocket = NULL; //客户端套接字
};