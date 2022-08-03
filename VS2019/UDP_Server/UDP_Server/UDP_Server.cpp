#pragma warning(disable:4996)

#include<iostream>
#include <string.h>
#include <stdio.h>
#include <map>

#include <winsock2.h>
#include <windows.h>

#include <cjson/cJSON.h>

#pragma comment(lib,"WS2_32.lib")
using namespace std;
int main() {
    WSADATA data;                                           //windows异步套接字数据结构体
    WORD w = MAKEWORD(2, 0);                                //指定winsock 版本的为 2.0
    WSAStartup(w, &data);                                   //指定程序可以使用winsock 2.0版本的套接字
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);               //初始化套接字
    sockaddr_in addr;                                  //声明服务器地址和客户端地址
    int token = sizeof(addr);                                //以客户端地址的sizew为token
    char buff[10000] = "";                  //接收数据缓冲区

    addr.sin_family = AF_INET;          //使用IPv4协议簇
    addr.sin_port = htons(8888);       //向8888号端口发送数据

    addr.sin_addr.S_un.S_addr = inet_addr("172.20.61.20");   //服务器本身地址
    bind(s, (sockaddr*)&addr, sizeof(addr)); //将套接字和服务器地址绑定在一起
    //bool first = true;        //第一次连接标记

    string recvData = "";
    string::size_type f_position = NULL;
    string::size_type l_position = NULL;
    std::map <string, float> m_subDataMap;
    m_subDataMap.clear();

    cJSON* dhData = NULL;
    cJSON* pointData = NULL;

    cout << "服务器已经启动" << endl;
    while (true)
    {
        if (recvfrom(s, buff, 10000, 0, (sockaddr*)&addr, &token) > 0) //从客户端接受数据
        {
            //if (first)
            //{
            //    cout << "已经与客户端地址链接上: " << inet_ntoa(addr2.sin_addr) << endl;
            //    first = false;
            //}
            recvData = buff;

            f_position = recvData.find_first_of(",");
            l_position = recvData.find_last_of("}");

            if (f_position != string::npos && l_position != string::npos) {
                //cout << "f_position" << f_position << "\n";
                //cout << "l_position" << l_position << "\n";

                recvData = recvData.substr(f_position + 1, l_position - f_position - 1);
                cout << recvData << endl;//输出客户端内容

                dhData = cJSON_Parse(recvData.c_str());
                cJSON_ArrayForEach(pointData, dhData)
                {
                    if (cJSON_IsNumber(pointData)) {
                        m_subDataMap.insert(pair<string, float>(pointData->string, pointData->valuedouble));
                    }
                    else if (cJSON_IsString(pointData)) {
                        m_subDataMap.insert(pair<string, float>(pointData->string, atof(pointData->valuestring)));
                    }
                    cout << pointData->string << ":" << pointData->valuedouble << "\n";
                }

                cJSON_Delete(dhData);
                recvData.clear();
            }
        }
    }

    closesocket(s);  //关闭套接字
    WSACleanup();  //终止Ws2_32.dll的使用
}