// YongJinDianHua.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//#define TCPSERVER_TEST
#define CPPHTTPLIB_OPENSSL_SUPPORT
#define WIN32_LEAN_AND_MEAN //这个定义放在windows.h和httplib.h之前
#include <iostream>
#include <string>
#include <time.h>
#include "httplib.h"
#include "cjson/cJSON.h"

#pragma comment(lib, "ws2_32.lib")

void initialization();

void initialization()
{
    //初始化套接字库
    WORD w_req = MAKEWORD(2, 2); //版本号
    WSADATA wsadata;
    int err;
    err = WSAStartup(w_req, &wsadata);
    if (err != 0)
    {
        std::cout << "初始化套接字库失败！\n";
    }
    else
    {
        std::cout << "初始化套接字库成功！\n";
    }
    //检测版本号
    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2)
    {
        std::cout << "套接字库版本号不符！\n";
        WSACleanup();
    }
    else
    {
        std::cout << "套接字库版本正确！\n";
    }
    //填充服务端地址信息
}

int main()
{

    time_t ts = time(NULL);
    char send_buf[10240] = "";
    // char recv_buf[10240] = "";

    SOCKET m_listenSocket = NULL; //监听套接字
    SOCKET m_clientSocket = NULL; //客户端套接字
    SOCKADDR_IN server_addr = {};

    initialization();

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);

    int addrLen = sizeof(sockaddr_in);

    //创建套接字
    if ((m_listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        std::cout << "创建套接字失败！\n";
    }

    if (bind(m_listenSocket, (sockaddr *)&server_addr, addrLen) == SOCKET_ERROR)
    {
        std::cout << "绑定失败！\n";
    }

    if (listen(m_listenSocket, 5) == SOCKET_ERROR)
    {
        std::cout << "监听套接字失败！\n";
    }
connectAdvpowertool:
    std::cout << "等待AdvPowerTool连接......\n";
    m_clientSocket = accept(m_listenSocket, (sockaddr *)&server_addr, &addrLen);

    if (m_clientSocket == SOCKET_ERROR)
    {
        closesocket(m_listenSocket);
        m_clientSocket = INVALID_SOCKET;
        std::cout << "套接字接收失败!\n";
        goto connectAdvpowertool;
    }

#ifdef TCPSERVER_TEST
    const char *testChar = "{\"0808\":{\"exten\":\"0808\",\"status\":\"Idle\",\"dnd\":false},\"0809\":{\"exten\":\"0809\",\"status\":\"Idle\",\"dnd\":false},\"0204\":{\"exten\":\"0204\",\"status\":\"Idle\",\"dnd\":false},\"0205\":{\"exten\":\"0205\",\"status\":\"Idle\",\"dnd\":false},\"0601\":{\"exten\":\"0601\",\"status\":\"Idle\",\"dnd\":false},\"0202\":{\"exten\":\"0202\",\"status\":\"Idle\",\"dnd\":false},\"0203\":{\"exten\":\"0203\",\"status\":\"Idle\",\"dnd\":false},\"0201\":{\"exten\":\"0201\",\"status\":\"Idle\",\"dnd\":false},\"0806\":{\"exten\":\"0806\",\"status\":\"Idle\",\"dnd\":false},\"0608\":{\"exten\":\"0608\",\"status\":\"Idle\",\"dnd\":false},\"0807\":{\"exten\":\"0807\",\"status\":\"Idle\",\"dnd\":false},\"0609\":{\"exten\":\"0609\",\"status\":\"Idle\",\"dnd\":false},\"0804\":{\"exten\":\"0804\",\"status\":\"Idle\",\"dnd\":false},\"0606\":{\"exten\":\"0606\",\"status\":\"Idle\",\"dnd\":false},\"0805\":{\"exten\":\"0805\",\"status\":\"Idle\",\"dnd\":false},\"0607\":{\"exten\":\"0607\",\"status\":\"Idle\",\"dnd\":false},\"0208\":{\"exten\":\"0208\",\"status\":\"Idle\",\"dnd\":false},\"0802\":{\"exten\":\"0802\",\"status\":\"Idle\",\"dnd\":false},\"0604\":{\"exten\":\"0604\",\"status\":\"Idle\",\"dnd\":false},\"0209\":{\"exten\":\"0209\",\"status\":\"Idle\",\"dnd\":false},\"0803\":{\"exten\":\"0803\",\"status\":\"Idle\",\"dnd\":false},\"0605\":{\"exten\":\"0605\",\"status\":\"Idle\",\"dnd\":false},\"0206\":{\"exten\":\"0206\",\"status\":\"Idle\",\"dnd\":false},\"0602\":{\"exten\":\"0602\",\"status\":\"Idle\",\"dnd\":false},\"0207\":{\"exten\":\"0207\",\"status\":\"Idle\",\"dnd\":false},\"0801\":{\"exten\":\"0801\",\"status\":\"Idle\",\"dnd\":false},\"0603\":{\"exten\":\"0603\",\"status\":\"Idle\",\"dnd\":false},\"0819\":{\"exten\":\"0819\",\"status\":\"Idle\",\"dnd\":false},\"0215\":{\"exten\":\"0215\",\"status\":\"Idle\",\"dnd\":false},\"0611\":{\"exten\":\"0611\",\"status\":\"Idle\",\"dnd\":false},\"0810\":{\"exten\":\"0810\",\"status\":\"Idle\",\"dnd\":false},\"001\":{\"exten\":\"001\",\"status\":\"Idle\",\"dnd\":false},\"0612\":{\"exten\":\"0612\",\"status\":\"Idle\",\"dnd\":false},\"0213\":{\"exten\":\"0213\",\"status\":\"Idle\",\"dnd\":false},\"002\":{\"exten\":\"002\",\"status\":\"Idle\",\"dnd\":false},\"0214\":{\"exten\":\"0214\",\"status\":\"Idle\",\"dnd\":false},\"003\":{\"exten\":\"003\",\"status\":\"Idle\",\"dnd\":false},\"0610\":{\"exten\":\"0610\",\"status\":\"Idle\",\"dnd\":false},\"0211\":{\"exten\":\"0211\",\"status\":\"Idle\",\"dnd\":false},\"004\":{\"exten\":\"004\",\"status\":\"Idle\",\"dnd\":false},\"0212\":{\"exten\":\"0212\",\"status\":\"Idle\",\"dnd\":false},\"0210\":{\"exten\":\"0210\",\"status\":\"Idle\",\"dnd\":false},\"0817\":{\"exten\":\"0817\",\"status\":\"Idle\",\"dnd\":false},\"0818\":{\"exten\":\"0818\",\"status\":\"Idle\",\"dnd\":false},\"0815\":{\"exten\":\"0815\",\"status\":\"Idle\",\"dnd\":false},\"0617\":{\"exten\":\"0617\",\"status\":\"Idle\",\"dnd\":false},\"0816\":{\"exten\":\"0816\",\"status\":\"Idle\",\"dnd\":false},\"0813\":{\"exten\":\"0813\",\"status\":\"Idle\",\"dnd\":false},\"0615\":{\"exten\":\"0615\",\"status\":\"Idle\",\"dnd\":false},\"0814\":{\"exten\":\"0814\",\"status\":\"Idle\",\"dnd\":false},\"0616\":{\"exten\":\"0616\",\"status\":\"Idle\",\"dnd\":false},\"0811\":{\"exten\":\"0811\",\"status\":\"Idle\",\"dnd\":false},\"0613\":{\"exten\":\"0613\",\"status\":\"Idle\",\"dnd\":false},\"0812\":{\"exten\":\"0812\",\"status\":\"Idle\",\"dnd\":false},\"0614\":{\"exten\":\"0614\",\"status\":\"Idle\",\"dnd\":false},\"0105\":{\"exten\":\"0105\",\"status\":\"Idle\",\"dnd\":false},\"0820\":{\"exten\":\"0820\",\"status\":\"Idle\",\"dnd\":false},\"0106\":{\"exten\":\"0106\",\"status\":\"Idle\",\"dnd\":false},\"0103\":{\"exten\":\"0103\",\"status\":\"Idle\",\"dnd\":false},\"0620\":{\"exten\":\"0620\",\"status\":\"Idle\",\"dnd\":false},\"0104\":{\"exten\":\"0104\",\"status\":\"Idle\",\"dnd\":false},\"0101\":{\"exten\":\"0101\",\"status\":\"Idle\",\"dnd\":false},\"0102\":{\"exten\":\"0102\",\"status\":\"Idle\",\"dnd\":false},\"0109\":{\"exten\":\"0109\",\"status\":\"Idle\",\"dnd\":false},\"0107\":{\"exten\":\"0107\",\"status\":\"Idle\",\"dnd\":false},\"0108\":{\"exten\":\"0108\",\"status\":\"Idle\",\"dnd\":false},\"0114\":{\"exten\":\"0114\",\"status\":\"Idle\",\"dnd\":false},\"0115\":{\"exten\":\"0115\",\"status\":\"Idle\",\"dnd\":false},\"0112\":{\"exten\":\"0112\",\"status\":\"Idle\",\"dnd\":false},\"0113\":{\"exten\":\"0113\",\"status\":\"Idle\",\"dnd\":false},\"0110\":{\"exten\":\"0110\",\"status\":\"Idle\",\"dnd\":false},\"0111\":{\"exten\":\"0111\",\"status\":\"Idle\",\"dnd\":false},\"0318\":{\"exten\":\"0318\",\"status\":\"Idle\",\"dnd\":false},\"0319\":{\"exten\":\"0319\",\"status\":\"Idle\",\"dnd\":false}}";
    strcpy_s(send_buf, testChar);

    while (1)
    {
        if (send(m_clientSocket, send_buf, strlen(send_buf), 0) == INVALID_SOCKET)
        {
            ts = time(NULL);
            std::cout << ctime(&ts) << "发送数据失败！\n";
            goto connectAdvpowertool;
        }
        else
        {
            ts = time(NULL);
            std::cout << ctime(&ts) << "发送数据成功！\n";
        }

        Sleep(100);
    }
#endif // TCPSERVER_TEST

    std::string phoneNo, phoneExten, phoneStatus, phoneDnd = {};

    httplib::SSLClient cli("10.34.55.145");
    httplib::Headers headers = {
        {"Accept", "application/json"}, {"Content-Type", "application/json"}};

    cli.enable_server_certificate_verification(false); //关闭证书校验

    //登录的账号密码，使用json包和post请求登录
    cJSON *loginCJSON = cJSON_CreateObject();
    cJSON_AddStringToObject(loginCJSON, "username", "api-FdBSesR9o8");
    cJSON_AddStringToObject(loginCJSON, "password", "admin@123");
    char *loginChar = cJSON_Print(loginCJSON);
    if (loginChar != NULL)
    {
        while (1)
        {
            if (auto resPost = cli.Post("/login", loginChar, "application/json")) //通过post请求登录
            {
                if (resPost->status == 200)
                {
                    // std::cout << resPost->body << "\n";

                    // std::multimap<std::string, std::string, httplib::detail::ci>::iterator headersIt = headersLogin.begin();
                    // for (; headersIt != headersLogin.end(); headersIt++) {
                    //     std::cout << headersIt->first;
                    //     std::cout << headersIt->second<<"\n";
                    // }

                    httplib::Headers headersLogin = resPost->headers; //获取cookie
                    // std::cout << headersLogin.find("set-cookie")->second << "\n";

                    headers.insert({"cookie", headersLogin.find("set-cookie")->second}); //在请求头中放入cookie
                    if (auto resGet = cli.Get("/api/system-status/list-exten-status", headers))
                    {
                        if (resGet->status == 200)
                        {
                            strcpy_s(send_buf, resGet->body.c_str());

                            if (send(m_clientSocket, send_buf, strlen(send_buf), 0) != INVALID_SOCKET)
                            {
                                ts = time(NULL);
                                std::cout << ctime(&ts) << "发送数据至AdvPowerTool失败！";
                                goto connectAdvpowertool;
                            }
#ifdef CJSON_PRASE
                            cJSON *statusCJSON = cJSON_Parse(resGet->body.c_str());
                            if (statusCJSON == NULL) //解析json失败
                            {
                                continue;
                            }
                            cJSON *itemCJSON = NULL;
                            cJSON_ArrayForEach(itemCJSON, statusCJSON)
                            {
                                phoneNo = itemCJSON->string;
                                // phoneExten = cJSON_GetObjectItem(itemCJSON, "exten")->valuestring;
                                phoneStatus = cJSON_GetObjectItem(itemCJSON, "status")->valuestring;

                                // phoneDnd = cJSON_GetObjectItem(itemCJSON, "dnd")->valueint;
                                // std::cout << "话机编号：" << phoneNo << ",话机状态：" << phoneStatus << "\n";
                            }
                            // std::cout << resGet->body;

#endif // CJSON_PRASE
                        }
                        else
                        {
                            ts = time(NULL);
                            std::cout << ctime(&ts) << "话机状态请求失败！\n";
                            std::cout << resGet->body;
                            Sleep(5000);
                        }
                        Sleep(100);
                    }
                    else
                    {
                        ts = time(NULL);
                        std::cout << ctime(&ts) << "连接电话主机失败！\n";
                        Sleep(5000);
                    }
                }
                else
                {
                    ts = time(NULL);
                    std::cout << ctime(&ts) << "登录电话主机失败！\n";
                    Sleep(5000);
                }
            }
            else
            {
                // auto err = resGet.error();
                // std::cout << "HTTP error: " << httplib::to_string(err) << std::endl;
                ts = time(NULL);
                std::cout << ctime(&ts) << "连接电话主机失败！\n";
                Sleep(5000);
            }
        }
        //关闭套接字
        closesocket(m_listenSocket);
        //释放DLL资源
        WSACleanup();
    }
    free(loginChar);
    loginChar = NULL;
}
