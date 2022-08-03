#include <iostream>
#include "CTcpServer.h"

int main()
{
    {
        CTcpServer tcpServer("127.0.0.1", 6005);

        if (!tcpServer.InitServer())
            getchar();

        for (int i = 0; i != 3; ++i)
        {
            //接收数据成功后，向客户端返回"answer"
            if (tcpServer.RecvMsg())
            {
                std::string strAnswerMsg{"answer"};

                tcpServer.SendMsg(strAnswerMsg);
            }
        }
    } //接收3次数据后出此大括号，tcpServer对象被析构，客户端连接被关闭

    return 0;
}