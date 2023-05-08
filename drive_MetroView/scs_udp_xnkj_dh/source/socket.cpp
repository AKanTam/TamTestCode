#include "socket.h"

string add = "";
string port = "";

WSADATA data;                                        // windows异步套接字数据结构体
WORD w = MAKEWORD(2, 0);                             //指定winsock 版本的为 2.0
SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //初始化套接字
sockaddr_in addr, addr2;                             //声明服务器地址和客户端地址
int token = sizeof(addr2);                           //以客户端地址的sizew为token
char buff[10240] = "";                               //接收数据缓冲区

string recvData = "";
string::size_type f_position = NULL;
string::size_type l_position = NULL;

cJSON *dhData = NULL;
cJSON *pointData = NULL;

char socket_logbuff[10240];

static auto cutPre(string stream, const string &str)
{
    int nPos = stream.find(str);
    if (nPos != -1)
    {
        stream = stream.substr(0, nPos);
    }
    return stream;
}
static auto cutNext(string stream, const string &str)
{
    int nPos = stream.find(str);

    if (nPos != -1)
    {
        stream = stream.substr(nPos + str.size(), stream.size());
    }
    return stream;
}

void open_socket(string address)
{

    add = cutPre(address, ":");
    port = cutNext(address, ":");

    WSAStartup(w, &data); //指定程序可以使用winsock 2.0版本的套接字

    addr.sin_family = AF_INET;                          //使用IPv4协议簇
    addr.sin_port = htons(atoi(port.c_str()));          //服务器本身端口
    addr.sin_addr.S_un.S_addr = inet_addr(add.c_str()); //服务器本身地址

    bind(s, (sockaddr *)&addr, sizeof(addr)); //将套接字和服务器地址绑定在一起
}

void close_socket()
{
    closesocket(s); //关闭套接字
    //  WSACleanup();   //终止Ws2_32.dll的使用
}

int udpXnkjDhSubPoll(map<string, float> *DataMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    if (s == INVALID_SOCKET)
    {
        memset(socket_logbuff, 0, sizeof(socket_logbuff));
        sprintf(socket_logbuff, "[%s] INFO Send_CallAllData() SOCKET初始化出错 line[%d]", (char *)_groupName, __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, socket_logbuff, FEP_LOG::PKGDATA);
    }
    m_subDataMap.clear();

    if (recvfrom(s, buff, 10240, 0, (sockaddr *)&addr2, &token) > 0) //从客户端接受数据
    {
        recvData = buff;

        f_position = recvData.find_first_of(",");
        l_position = recvData.find_last_of("}");

        if (f_position != string::npos && l_position != string::npos)
        {

            recvData = recvData.substr(f_position + 1, l_position - f_position - 1);

            dhData = cJSON_Parse(recvData.c_str());
            if (dhData == NULL) //如果json解析失败
            {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    memset(socket_logbuff, 0, sizeof(socket_logbuff));
                    sprintf(socket_logbuff, "[%s] ERROR Send_CallAllData() 查询任务失败  Error before: %s  line[%d]", (char *)_groupName, error_ptr, __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, socket_logbuff, FEP_LOG::PKGDATA);
                }
                return 0;
            }
            else
            {

                memset(socket_logbuff, 0, sizeof(socket_logbuff));
                sprintf(socket_logbuff, "[%s] INFO Send_CallAllData() 查询点任务 line[%d]", (char *)_groupName, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, socket_logbuff, FEP_LOG::PKGDATA);

                cJSON_ArrayForEach(pointData, dhData)
                {
                    if (cJSON_IsNumber(pointData))
                    {
                        m_subDataMap.insert(pair<string, float>(pointData->string, pointData->valuedouble));
                    }
                    else if (cJSON_IsString(pointData) && (pointData->valuestring != NULL))
                    {
                        m_subDataMap.insert(pair<string, float>(pointData->string, atof(pointData->valuestring)));
                    }
                }
                cJSON_Delete(dhData);
                recvData.clear();
            }
        }
        else
        {
            memset(socket_logbuff, 0, sizeof(socket_logbuff));
            sprintf(socket_logbuff, "[%s] ERROR Send_CallAllData() 查询任务失败  报文格式与预料格式不一 line[%d]", (char *)_groupName, __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, socket_logbuff, FEP_LOG::PKGDATA);
            return 0;
        }
    }
    return 1;
}