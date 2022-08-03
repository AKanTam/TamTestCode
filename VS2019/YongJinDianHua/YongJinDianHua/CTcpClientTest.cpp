#include <iostream>
#include "CTcpClient.h"

int main()
{
    {
        CTcpClient tcpClient("127.0.0.1", 6005);

        if (!tcpClient.InitConnect())
            getchar();

        std::string strAskMsg{"ask"};

        for (int i = 0; i != 3; ++i)
        {
            if (tcpClient.SendMsg(strAskMsg))
            {
                tcpClient.RecvMsg();
            }
        }
    }

    getchar();

    return 0;
}