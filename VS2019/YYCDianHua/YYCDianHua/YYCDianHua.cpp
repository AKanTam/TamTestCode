#define BENDI

#pragma warning(disable:4996)
#include <iostream>
#include<stdio.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

//SOCKET sockSrv = socket(AF_INET, SOCK_DGRAM, 0);

int main()
{
	//加载套接字库
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||     //低字节为主版本
		HIBYTE(wsaData.wVersion) != 1)      //高字节为副版本
	{
		WSACleanup();
		return -1;
	}

	printf("Client is operating!\n\n");
	//1.创建用于监听的套接字
	SOCKET sockSrv = socket(AF_INET, SOCK_DGRAM, 0);

	//2.确定服务端通讯地址信息
	sockaddr_in  addrSrv;
	//inet_pton(AF_INET, "172.20.61.20", (void*)&addrSrv.sin_addr.S_un.S_addr);
	addrSrv.sin_addr.S_un.S_addr = inet_addr("172.20.61.20");//输入你想通信的她（此处是本机内部）
	addrSrv.sin_addr.S_un.S_addr = inet_addr("192.168.88.88");//输入你想通信的她（此处是本机内部）
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(8888);

	int len = sizeof(SOCKADDR);

	//char sendBuf[100] = "Hello World!";
	char sendBuf[1200] = "{total:128,{\"8000\":1,\"88881\":1,\"8002\":0,\"8003\":0,\"8004\":0,\"8005\":0,\"8006\":0,\"8007\":0,\"8008\":0,\"8009\":0,\"8010\":0,\"8011\":0,\"8012\":0,\"8013\":0,\"8014\":0,\"8015\":0,\"8016\":0,\"8017\":0,\"8018\":0,\"8019\":0,\"8020\":0,\"8021\":0,\"8022\":0,\"8023\":0,\"8024\":0,\"8025\":1,\"8026\":0,\"8027\":0,\"8028\":0,\"8029\":0,\"8030\":0,\"8031\":0,\"8032\":0,\"8033\":0,\"8034\":0,\"8035\":0,\"8036\":0,\"8037\":0,\"8038\":0,\"8039\":0,\"8040\":0,\"8041\":0,\"8042\":0,\"8043\":0,\"8044\":0,\"8045\":0,\"8046\":0,\"8047\":0,\"8048\":0,\"8049\":0,\"8050\":0,\"8051\":0,\"8052\":0,\"8053\":0,\"8054\":0,\"8055\":0,\"8056\":0,\"8057\":0,\"8058\":0,\"8059\":0,\"8060\":0,\"8061\":0,\"8062\":0,\"8063\":0,\"8064\":0,\"8065\":0,\"8066\":0,\"8067\":0,\"8068\":0,\"8069\":0,\"8070\":0,\"8071\":0,\"8072\":0,\"8073\":0,\"8074\":0,\"8075\":0,\"8076\":0,\"8077\":0,\"8078\":0,\"8079\":0,\"8080\":0,\"8081\":0,\"8082\":0,\"8083\":0,\"8084\":0,\"8085\":0,\"8086\":0,\"8087\":0,\"8088\":0,\"8089\":0,\"8090\":0,\"8091\":0,\"8092\":0,\"8093\":0,\"8094\":0,\"8095\":0,\"8096\":0,\"8097\":0,\"8098\":0,\"8099\":0,\"8100\":0,\"8101\":0,\"8102\":0,\"8103\":0,\"8104\":0,\"8105\":0,\"8106\":0,\"8107\":0,\"8108\":0,\"8109\":0,\"8110\":0,\"8111\":0,\"8112\":0,\"8113\":0,\"8114\":0,\"8115\":0,\"8116\":0,\"8117\":0,\"8118\":0,\"8119\":0,\"8120\":0,\"8121\":0,\"8122\":0,\"8123\":0,\"8124\":0,\"8125\":0,\"8126\":0,\"8127\":0}}";    //发


	while (1)
	{
		sendto(sockSrv, sendBuf, strlen(sendBuf) + 1, 0, (SOCKADDR*)&addrSrv, len);
		Sleep(1000);
	}
	closesocket(sockSrv);
	WSACleanup();
	return 0;

}