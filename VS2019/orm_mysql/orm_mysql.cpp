// orm_mysql.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define ORMPP_ENABLE_MYSQL

#include <iostream>
#include "dbng.hpp"
#include "mysql.hpp"
using namespace ormpp;

struct cfg_route
{
    std::string StationName;
    std::string RouteName;
    std::string RouteDesc;
    std::string LinkName;
};

REFLECTION(cfg_route, StationName, RouteName, RouteDesc , LinkName)

int main()
{
    dbng<mysql> mysql;
    if (mysql.connect("172.20.61.30", "sysadmin", "adminsys", "idpora")) {
        std::cout << "connect success" << std::endl;
    }
    else {
        std::cout << "connect fail" << std::endl;
    }

    auto result = mysql.query<cfg_route>();
    std::cout << "result's size is " << result.size() << "\n";

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
