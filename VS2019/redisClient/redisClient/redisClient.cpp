// redisClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "redisconnect.h"

struct alarmTag
{
    std::string tagName;
    int value;
    //qint64 valueTime;
    bool needPushFlag;
};

int main()
{
    std::string  val;
    std::vector <std::string> tagList;
    std::vector <std::string>::iterator tagit;

    std::string tagValueString;
    int tagValue;
    bool isValueValid;

    std::map<std::string, struct alarmTag> alarmMap;
    std::map<std::string, alarmTag>::iterator mapIter;

redisReconnect:
    RedisConnect::Setup("172.20.61.29", 6379);
    shared_ptr<RedisConnect> redis = nullptr;
    redis = RedisConnect::Instance();

    if (redis->ping() > 0) {
        std::cout << "Redis服务连接成功!\n";
    }
    else {
        std::cout << "无法连接至Redis服务...\n";
        std::cout << "尝试重新连接Redis服务...\n";
        Sleep(1000);
        goto redisReconnect;
    }

    while (1) {
        if (redis->keys(tagList, "*") > 0 ) {
            for (tagit = tagList.begin(); tagit != tagList.end(); tagit++) {
                std::cout << *tagit << "\n";
            }
        }
        Sleep(1000);
        if (redis->ping() < 0 ) {
            std::cout << "尝试重新连接Redis服务...\n";
            Sleep(1000);
            goto redisReconnect;
        }
    }
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
