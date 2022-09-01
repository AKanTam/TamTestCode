// IOT_Driver.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <map>

#include <httplib.h>
#include <cjson/cJSON.h>

#define MAPITERATOR
//#define APICHECK

httplib::Client cli("172.20.61.16:9080");
httplib::Headers headers = {
    {"Accept", "application/json"}, {"Content-Type", "application/json"}};

static std::string UtfToGbk(std::string strValue)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, strValue.c_str(), -1, NULL, 0);
    wchar_t *wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, strValue.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    if (wstr)
        delete[] wstr;
    return std::string(str);
}

int main()
{
    std::ifstream IOT_Infile;
    std::string IOT_Path;
    std::string IOT_Buf;
    std::string IOT_LoginChar;
    if (std::getenv("SSPATH") != nullptr)
    {
        IOT_Path = std::getenv("SSPATH");
        IOT_Path += "\\config\\IOTconfig.json";
    }
    else
    {
        std::cout << "nullptr!"
                  << "\n";
    }

    IOT_Infile.open(IOT_Path, std::ios::in);

    if (IOT_Infile.is_open())
    {

        while (getline(IOT_Infile, IOT_Buf))
        {
            IOT_LoginChar += IOT_Buf;
        }
        std::cout << IOT_LoginChar << "\n";
    }
    else
    {
        std::cout << "nullpFile!"
                  << "\n";
    }

    auto resPost = cli.Post("/supaiot/api/v2/app/sec/login", IOT_LoginChar.c_str(), "application/json");
    httplib::Headers headersLogin = resPost->headers;

    if (headersLogin.find("set-cookie") != headersLogin.end())
    {
        cJSON* monitor = cJSON_CreateObject();
        cJSON* readArray = cJSON_AddArrayToObject(monitor, "ID");
        cJSON* item_name = cJSON_CreateString("JX_ZHTC_DC_36");
        cJSON* item_name1 = cJSON_CreateString("JX_ZHTC_DC_37");
        cJSON_AddItemToArray(readArray, item_name);
        cJSON_AddItemToArray(readArray, item_name1);
        char* str = cJSON_Print(monitor);
#ifdef APICHECK
        while (1) {
            if (headers.find("cookie") == headers.end()) {

                headers.insert({ "cookie", headersLogin.find("set-cookie")->second });
            }
            else {
                headers.erase("cookie");
                headers.insert({ "cookie", headersLogin.find("set-cookie")->second });
            }

            auto resGet = cli.Post("/supaiot/api/v2/data/real/device/list", headers, str, "application/json");
            if (resGet->status == 200) {
                std::cout << "body: " << UtfToGbk(resGet->body) << "\n";
            }
            else {
                std::cout << "status: " << resGet->status << "\n";
                std::cout << "body: " << UtfToGbk(resGet->body) << "\n";
            }

    }

        // auto resGet = cli.Get("/supaiot/api/v2/project", headers);
        // std::cout << UtfToGbk(resGet->body) << "\n";
#endif // APICHECK


    }

#ifdef MAPITERATOR
    std::map<std::string, std::map<std::string, int>> m_map;
    m_map.clear();
    m_map["sting1"].insert(std::pair < std::string, int>("1", 5));
    m_map["sting1"].insert(std::pair < std::string, int>("2", 5));
    m_map["sting1"].insert(std::pair < std::string, int>("3", 5));
    m_map["sting2"].insert(std::pair < std::string, int>("1", 5));
    m_map["sting2"].insert(std::pair < std::string, int>("2", 5));
    m_map["sting2"].insert(std::pair < std::string, int>("3", 5));
    std::map<std::string, int>::iterator tempmap = m_map.find("sting1")->second.find("3");
    tempmap->second = 10;

    std::cout << tempmap->first << "\n";
    std::cout << m_map["sting1"]["1"] << "\n";
    std::cout << m_map.size() << "\n";
#endif // MAPITERATOR



}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
