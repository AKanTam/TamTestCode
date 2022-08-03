// BitTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<string>
#include<list>
#include <time.h>
using namespace std;

static auto cutNext(string stream, const string& str) {
    int nPos = stream.find(str);

    if (nPos != -1) {
        stream = stream.substr(nPos + str.size(), stream.size());
    }
    return stream;
}

string getReqTime() {
    time_t timep;
    time(&timep);
    char tmpTime[256];
    strftime(tmpTime, sizeof(tmpTime), "%Y/%m/%d/%H:%M:%S", localtime(&timep));
    string reqTime = tmpTime;
    return reqTime;
}

int main()
{

    while (1)
    {
        cout << getReqTime() << endl;
        _sleep(1000);
    }


    //list<string> ss;
    //ss.clear();
    //ss.push_back("中控");
    //ss.push_back("中控");
    //ss.push_back("中控");
    //ss.push_back("中控sb");
    //ss.push_back("中控");
    //ss.sort();
    //ss.unique();
    //for (list<string>::iterator sd = ss.begin(); sd != ss.end(); sd++) {
    //    cout << *sd << endl;
    //}

    //cout<<cutNext("127.0.0.1:1883",":")<<endl;

    /*
    int num = 55;
    int pluse = 200;
    int sign = (num<<8)+pluse ;


    std::cout << sign << std::endl;
    std::cout << (sign>>8)<<std::endl;
    std::cout << (sign&0xff) << std::endl;
    */
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
