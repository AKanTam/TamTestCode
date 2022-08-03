
#define BENDI
#include<iostream>
#include<winsock.h>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

#pragma comment(lib,"ws2_32.lib")
using namespace std;

#define TOLOWER(p) {transform(p.begin(),p.end(),p.begin(),::tolower);}

void initialization();

std::string byte_2_str(char* bytes, int size) {
	char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B','C','D','E','F' };
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

string switchInt(const char* hex,int size) 
{
	//vector<char> temp(size);
	char* temp = new char[size];
	char tempResult[10000] = "";

	for (int i = 0; i < size;) {
		temp[i] = hex[i + 2];
		temp[i + 1] = hex[i + 3];
		temp[i + 2] = hex[i];
		temp[i + 3] = hex[i + 1];
		i += 4;
	}
	memcpy(tempResult,temp,size);
	//cout << temp << endl;
	string result = tempResult;
	//cout << result << endl;
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

int main() {
	//定义长度变量
	int recv_len = 0;
	//定义发送缓冲区和接受缓冲区
	//char send_buf[100];
	char recv_buf[10000] = "";
	//定义服务端套接字，接受请求套接字
	SOCKET s_server;
	//服务端地址客户端地址
	SOCKADDR_IN server_addr;
	initialization();
	//填充服务端信息
	server_addr.sin_family = AF_INET;

#ifdef BENDI
	server_addr.sin_addr.S_un.S_addr = inet_addr("172.20.61.20");
	server_addr.sin_port = htons(8888);
#else
	server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.49.151");
	server_addr.sin_port = htons(26546);
#endif // DEBUG

	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "服务器连接失败！" << endl;
		WSACleanup();
	}
	else {
		cout << "服务器连接成功！" << endl;
	}


	//发送,接收数据
	while (1) 
	{
		//cout << "请输入发送信息:";
		//cin >> send_buf;
		//send_len = send(s_server, send_buf, 100, 0);
		//if (send_len < 0) {
		//	cout << "发送失败！" << endl;
		//	break;
		//}
		recv_len = recv(s_server, recv_buf,10000, 0);
		if (recv_len < 0) 
		{
			cout << "接受失败！" << endl;
			break;
		}
		else 
		{
			string result;
			int strlen = 0;
			int sensorCout;
			int startInx = 12;

			strlen = HexStrToDecStr(switchInt(byte_2_str(recv_buf, 2).c_str(),4))+4;

			cout << "接收到的字节数:" << strlen << endl;
			result = byte_2_str(recv_buf, strlen);//填写字节数
			//cout << "服务端信息:" << result << endl;
			cout << "接收到的报文原文:" << result << endl;
			
			sensorCout = HexStrToDecStr(switchInt(byte_2_str(recv_buf, 2).c_str()+8, 4));
			cout << "传感器数量为" << sensorCout <<endl;

			for (int i = 0; i < 9999;i++)
			{
				
				string data = result.substr(startInx + 40 * i, 40);
				if (data.size() != 40)
				{
					break;
				}
				//cout << "传感器Data为" << data << endl;
				//cout << "传感器No为" << endl;
				int sensorNo = HexStrToDecStr(switchInt(data.substr(4,4).c_str(),4));

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

				float* p = (float*)buf;
				float sensorData = *p;

				if (sensorData != sensorData) {
					cout << "fuck!" << endl;
				}
				
				cout << "传感器"<< sensorNo <<"的数据是" << sensorData << endl;
				
			}
			result = "";
		}
     }

	//关闭套接字
	closesocket(s_server);
	//释放DLL资源
	WSACleanup();
	return 0;
}
void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
	}
	else {
		cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "套接字库版本号不符！" << endl;
		WSACleanup();
	}
	else {
		cout << "套接字库版本正确！" << endl;
	}
	//填充服务端地址信息

}