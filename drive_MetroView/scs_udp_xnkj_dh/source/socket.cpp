#include "socket.h"

string add = "";
string port = "";

WSADATA data;                                        // windows�첽�׽������ݽṹ��
WORD w = MAKEWORD(2, 0);                             //ָ��winsock �汾��Ϊ 2.0
SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //��ʼ���׽���
sockaddr_in addr, addr2;                             //������������ַ�Ϳͻ��˵�ַ
int token = sizeof(addr2);                           //�Կͻ��˵�ַ��sizewΪtoken
char buff[10240] = "";                               //�������ݻ�����

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

    WSAStartup(w, &data); //ָ���������ʹ��winsock 2.0�汾���׽���

    addr.sin_family = AF_INET;                          //ʹ��IPv4Э���
    addr.sin_port = htons(atoi(port.c_str()));          //����������˿�
    addr.sin_addr.S_un.S_addr = inet_addr(add.c_str()); //�����������ַ

    bind(s, (sockaddr *)&addr, sizeof(addr)); //���׽��ֺͷ�������ַ����һ��
}

void close_socket()
{
    closesocket(s); //�ر��׽���
    //  WSACleanup();   //��ֹWs2_32.dll��ʹ��
}

int udpXnkjDhSubPoll(map<string, float> *DataMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    if (s == INVALID_SOCKET)
    {
        memset(socket_logbuff, 0, sizeof(socket_logbuff));
        sprintf(socket_logbuff, "[%s] INFO Send_CallAllData() SOCKET��ʼ������ line[%d]", (char *)_groupName, __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, socket_logbuff, FEP_LOG::PKGDATA);
    }
    m_subDataMap.clear();

    if (recvfrom(s, buff, 10240, 0, (sockaddr *)&addr2, &token) > 0) //�ӿͻ��˽�������
    {
        recvData = buff;

        f_position = recvData.find_first_of(",");
        l_position = recvData.find_last_of("}");

        if (f_position != string::npos && l_position != string::npos)
        {

            recvData = recvData.substr(f_position + 1, l_position - f_position - 1);

            dhData = cJSON_Parse(recvData.c_str());
            if (dhData == NULL) //���json����ʧ��
            {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL)
                {
                    memset(socket_logbuff, 0, sizeof(socket_logbuff));
                    sprintf(socket_logbuff, "[%s] ERROR Send_CallAllData() ��ѯ����ʧ��  Error before: %s  line[%d]", (char *)_groupName, error_ptr, __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, socket_logbuff, FEP_LOG::PKGDATA);
                }
                return 0;
            }
            else
            {

                memset(socket_logbuff, 0, sizeof(socket_logbuff));
                sprintf(socket_logbuff, "[%s] INFO Send_CallAllData() ��ѯ������ line[%d]", (char *)_groupName, __LINE__);
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
            sprintf(socket_logbuff, "[%s] ERROR Send_CallAllData() ��ѯ����ʧ��  ���ĸ�ʽ��Ԥ�ϸ�ʽ��һ line[%d]", (char *)_groupName, __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, socket_logbuff, FEP_LOG::PKGDATA);
            return 0;
        }
    }
    return 1;
}