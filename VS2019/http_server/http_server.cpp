// http_server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "httplib.h"
#include <thread>

using namespace httplib;
using namespace std;
#define fromData


void ss(const Request& req, Response& res) {
#ifdef fromData
    const auto& file = req.get_file_value("toiletId");//表单数据
    cout << file.content << endl;//表单数据
#endif // fromData

    //cout<<req.body<<endl;
    res.set_content(req.body, "multipart/form-data");
}

void httpSrv() {
    Server svr;

    svr.Post("/h", ss);

    //svr.listen("172.20.61.20", 1234);
    svr.listen("0.0.0.0", 1234);
}

void httpCli() {

#ifdef fromData
    while (1) {
        Client cli("127.0.0.1:1234");
        httplib::MultipartFormDataItems items = {
          { "text1", "text default", "", "" },
          { "text2", "aωb", "", "" },
          { "file1", "h\ne\n\nl\nl\no\n", "hello.txt", "text/plain" },
          { "file2", "{\n  \"world\", true\n}\n", "world.json", "application/json" },
          { "file3", "", "", "application/octet-stream" }
        };
        MultipartFormData ss = { "toiletId", "text default", "", "" };
        items.push_back(ss);


        auto res = cli.Post("/h", items);
        cout << "post" << endl;
        Sleep(1000);
        
    }
#endif // fromData
}

int main()
{
    thread server(httpSrv);
    thread client(httpCli);

    server.join();
    client.join();

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
