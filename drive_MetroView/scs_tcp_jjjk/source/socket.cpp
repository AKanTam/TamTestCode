#include "socket.h"

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

bool initialization()
{
    //��ʼ���׽��ֿ�
    WORD w_req = MAKEWORD(2, 2); //�汾��
    WSADATA wsadata;
    int err;
    err = WSAStartup(w_req, &wsadata);
    if (err != 0)
    {
        // cout << "��ʼ���׽��ֿ�ʧ�ܣ�" << endl;
        return false;
    }
    else
    {
        // cout << "��ʼ���׽��ֿ�ɹ���" << endl;
    }
    //���汾��
    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2)
    {
        // cout << "�׽��ֿ�汾�Ų�����" << endl;
        WSACleanup();
        return false;
    }
    else
    {
        // cout << "�׽��ֿ�汾��ȷ��" << endl;
    }
    return true;
    //������˵�ַ��Ϣ
}

std::string byte_2_str(char *bytes, int size)
{
    char const hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    std::string str;
    int flag = 0;
    for (int i = 0; i < size; ++i)
    {
        if (bytes[i] == 0)
        {
            flag++;
            if (flag > 5)
            {
                break;
            }
            else
            {
                const char ch = bytes[i];
                str.append(&hex[(ch & 0xF0) >> 4], 1);
                str.append(&hex[ch & 0xF], 1);
                continue;
            }
        }
        else
        {
            flag = 0;
            const char ch = bytes[i];
            str.append(&hex[(ch & 0xF0) >> 4], 1);
            str.append(&hex[ch & 0xF], 1);
        }
    }
    return str;
}

string switchInt(const char *hex, int size)
{
    // vector<char> temp(size);
    char *temp = new char[size];
    char tempResult[10240] = "";

    for (int i = 0; i < size;)
    {
        temp[i] = hex[i + 2];
        temp[i + 1] = hex[i + 3];
        temp[i + 2] = hex[i];
        temp[i + 3] = hex[i + 1];
        i += 4;
    }
    memcpy(tempResult, temp, size);
    string result = tempResult;
    delete[] temp;
    return result;
}

int HexStrToDecStr(string str)
{
    int iDec = 0;
    TOLOWER(str);
    sscanf_s(str.c_str(), "%x", &iDec);
    return iDec;
}

int tcpJJJKSubPoll(string address, map<string, float> *DataMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    //���峤�ȱ���
    int recv_len = 0;
    //���巢�ͻ������ͽ��ܻ�����
    char recv_buf[10240] = "";
    //���������׽��֣����������׽���
    SOCKET s_server;
    //����˵�ַ�ͻ��˵�ַ
    SOCKADDR_IN server_addr;
    if (!initialization())
    {
        return 0;
    }
    //���������Ϣ
    server_addr.sin_family = AF_INET;

    string add = cutPre(address, ":");
    string port = cutNext(address, ":");

    server_addr.sin_addr.S_un.S_addr = inet_addr(add.c_str());
    server_addr.sin_port = htons(atoi(port.c_str()));

    //�����׽���
    s_server = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(s_server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        // cout << "����������ʧ�ܣ�" << endl;
        WSACleanup();
        return 0;
    }
    else
    {
        // cout << "���������ӳɹ���" << endl;
    }

    recv_len = recv(s_server, recv_buf, 10240, 0);
    if (recv_len < 0)
    {
        // cout << "����ʧ�ܣ�" << endl;
        return 0;
    }
    else
    {
        string result;
        int strlen = 0;
        int sensorCout;
        int startInx = 12;

        strlen = HexStrToDecStr(switchInt(byte_2_str(recv_buf, 2).c_str(), 4)) + 4;

        // cout << "�������Ϣ:" << strlen << endl;
        result = byte_2_str(recv_buf, strlen); //��д�ֽ���

        // cout << "�������Ϣ:" << result << endl;
        // cout << "����������Ϊ" << endl;
        sensorCout = HexStrToDecStr(switchInt(byte_2_str(recv_buf, 2).c_str() + 8, 4));
        DataMap->clear();

        // memset(socket_logbuff, 0, sizeof(socket_logbuff));
        // sprintf(socket_logbuff, "[SOCKET]  Source:%X,Len:%d,ProcData:%X,SensorCout:%d", recv_buf, strlen, result, sensorCout);
        // FepLog::writelog((char *)_groupName, (char *)_linkName, socket_logbuff, FEP_LOG::PKGDATA);

        for (int i = 0; i < 9999; i++)
        {

            string data = result.substr(startInx + 40 * i, 40);
            if (data.size() != 40)
            {
                break;
            }
            // cout << "������DataΪ" << data << endl;
            // cout << "������NoΪ" << endl;
            int sensorNo = HexStrToDecStr(switchInt(data.substr(4, 4).c_str(), 4));

            stringstream ss;
            unsigned char buf[4];
            int itemp;
            string d1 = data.substr(32, 8);
            ss << hex << d1.substr(0, 2) << endl;
            ss >> itemp;
            buf[0] = itemp;
            ss.str("");

            ss << hex << d1.substr(2, 2) << endl;
            ss >> itemp;
            buf[1] = itemp;
            ss.str("");

            ss << hex << d1.substr(4, 2) << endl;
            ss >> itemp;
            buf[2] = itemp;
            ss.str("");

            ss << hex << d1.substr(6, 2) << endl;
            ss >> itemp;
            buf[3] = itemp;
            ss.str("");

            float *p = (float *)buf;
            float sensorData = *p;

            if (sensorData != sensorData)
            {
                sensorData = 0.88888888;
            }

            // memset(socket_logbuff, 0, sizeof(socket_logbuff));
            // sprintf(socket_logbuff, "[SOCKET]  Name:%d,Data%f", sensorNo, sensorData);
            // FepLog::writelog((char *)_groupName, (char *)_linkName, socket_logbuff, FEP_LOG::PKGDATA);

            DataMap->insert(pair<string, float>(to_string(sensorNo), sensorData));

            // cout << "������" << sensorNo << "��������" << sensorData << endl;
        }
        result = "";
    }
    if (DataMap->empty())
    {
        return 0;
    }
    //�ر��׽���
    closesocket(s_server);
    //�ͷ�DLL��Դ
    WSACleanup();

    return 1;
}
