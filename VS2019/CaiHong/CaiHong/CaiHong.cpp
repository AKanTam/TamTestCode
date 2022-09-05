// CaiHong.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define WIN32_LEAN_AND_MEAN //这个定义放在windows.h和httplib.h之前

#include "Type.h"
#include <httplib.h>
extern "C"
{
#include "HttpUtil.h"
#include "cjson/cJSON.h"
}
#include "redisconnect.h"
#include <thread>

//#define BENDITEST
//#define BENDI

//电话状态0：断线，1：空闲，2：对讲中(推送)，3：广播中，4：监听中

void invisionCheck(const char *tagName, std::string &value);
bool pushHaiKang(const std::string &deviceId);

struct alarmTag
{
    std::string tagName;
    int value;
    std::time_t valueTime;
    bool needPushFlag;
};

time_t timep;
char tmp[256];
std::string val;
std::vector<std::string> tagList;
std::vector<std::string>::iterator tagit;

int tagValue;
size_t valuePos;
char *pushMsg = nullptr;

struct alarmTag tempAlarmStruct = {};
std::map<std::string, struct alarmTag> alarmFireMap;
std::map<std::string, alarmTag>::iterator mapFireIter;
std::map<std::string, struct alarmTag> alarmPhoneMap;
std::map<std::string, alarmTag>::iterator mapPhoneIter;

std::string appkey = "24541234";             // APPKey
std::string secret = "0IrPfreVVeGm7Ym0Ryzo"; // APPSecret
std::string ip = "195.168.101.201";          // 综合安防管理平台IP地址
int port = 443;                              // 综合安防管理平台端口（目前仅支持HTTPS协议，默认端口443，需根据现场环境配置）
int timeout = 10;                            // 超时10秒

std::string url = "https://195.168.101.201:443/artemis/api/v1/event/submit";


int main()
{
    shared_ptr<RedisConnect> redis = nullptr;

#ifdef BENDITEST

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "deviced", "N_SB49");
    cJSON_AddStringToObject(root, "happenTime", "2022-07-24 13:19:10");
    const char *body = cJSON_Print(root);

    std::string utf8Body = _A2U8(body);

    std::cout << "post: " << utf8Body << "\n";

    int dataLen = 0; // 返回数据长度
    char *rsp = httpUtil::HTTPUTIL_Post(url.c_str(), utf8Body.c_str(), appkey.c_str(), secret.c_str(), timeout, &dataLen);

    if (NULL == rsp)
    {
        int status = httpUtil::HTTPUTIL_GetLastStatus(); // 获取状态码（根据取值判断是错误码还是 HTTP 状态码）
        std::cout << "status:" << status << "\n";
        if (status < httpUtil::HTTPUTIL_ERR_BASE)
        {
            // status 为 HTTP 状态码，请自行查阅 status 对应的 HTTP 状态描述
        }
        else
        {
            // status 为错误码，请参阅 OpenAPI 安全认证库（C/C++）状态码
        }
    }
    else
    {
        std::string response = std::string(rsp, dataLen); // 不管是二进制还是字符串，指定了 dataLen 参数情况

        std::cout << "response: " << response << "\n";
        //下使用此种方式总是没有错，如果没有指定 dataLen 参数，会被认为返回响应是字符串，直接 response = rsp 即可
        // std::string localDecodedData = _U82A(); // 宏_U82A 详见 UTF-8 编码转换本地编码示例
        // Step7：释放内存
        httpUtil::HTTPUTIL_Free(rsp);
        // Step8：解析 localDecodedData 中的 json 报文
    }

#endif // BENDITEST

redisReconnect:
#ifdef BENDI
    RedisConnect::Setup("172.20.61.29", 6379);
#endif // BENDI
#ifndef BENDI
    RedisConnect::Setup("195.168.101.130", 6379);
#endif

    redis = RedisConnect::Instance();

    if (redis->ping() > 0)
    {
        time(&timep);
        strftime(tmp, sizeof(tmp), "[%Y-%m-%d %H:%M:%S]", localtime(&timep));
        std::cout << tmp << "Redis服务连接成功!\n";
    }
    else
    {
        time(&timep);
        strftime(tmp, sizeof(tmp), "[%Y-%m-%d %H:%M:%S]", localtime(&timep));
        std::cout << tmp << "无法连接至Redis服务...\n";
        std::cout << tmp << "尝试重新连接Redis服务...\n";
        Sleep(1000);
        goto redisReconnect;
    }

    while (1)
    {
        //time(&timep);
        //strftime(tmp, sizeof(tmp), "[%Y-%m-%d %H:%M:%S]", localtime(&timep));
        //std::cout << tmp<<"完成一次轮询" << "\n";
        Sleep(50);//目前一秒大约能完成十次轮询，可稍微加点延迟
        if (redis->keys(tagList, "*") > 0)//网络连接正常
        {
            for (tagit = tagList.begin(); tagit != tagList.end(); tagit++)
            {
                if (redis->get(*tagit, val) > 0)
                {
                    // std::cout <<*tagit<<": " <<val << "\n";
                    valuePos = val.find_first_of("&");
                    if (valuePos != std::string::npos)
                    {
                        if (!strcmp((val.substr(valuePos - 2, 2)).c_str(), "12")) //模拟量
                        {
                            valuePos = val.find_first_of("\u0010");
                            if (valuePos != std::string::npos)
                            {
                                mapPhoneIter = alarmPhoneMap.find(*tagit);
                                tagValue = atoi(val.substr(valuePos + 1, 1).c_str());
                                // tagValue = atoi(val.c_str());

                                if (mapPhoneIter == alarmPhoneMap.end()) //位号表中找不到该点位
                                {
                                    tempAlarmStruct.needPushFlag = 0;
                                    tempAlarmStruct.tagName = *tagit;
                                    tempAlarmStruct.value = tagValue;
                                    tempAlarmStruct.valueTime = std::time(0);
                                    alarmPhoneMap.insert(std::pair<std::string, struct alarmTag>(*tagit, tempAlarmStruct));
                                    if (tagValue == 2)
                                    {
                                        goto pushPhoneAlarm;
                                    }
                                }
                                else
                                {
                                    if (mapPhoneIter->second.needPushFlag == 1 && tagValue == 2) //如果推送标志位和redis当前值都为2则直接跳到推送
                                    {
                                        goto pushPhoneAlarm;
                                    }
                                    if (tagValue != mapPhoneIter->second.value) // redis上的值与位号表中保存的值不同(变位)
                                    {
                                        mapPhoneIter->second.value = tagValue;
                                        mapPhoneIter->second.valueTime = std::time(0);
                                        if (tagValue == 2)
                                        {
                                            mapPhoneIter->second.needPushFlag = 1;

                                        pushPhoneAlarm:
                                            time(&timep);
                                            strftime(tmp, sizeof(tmp), "[%Y-%m-%d %H:%M:%S]", localtime(&timep));
                                            std::cout << tmp << " [报警事件]: " << *tagit<<"\n";
                                            if (!pushHaiKang(*tagit))
                                            {
                                                Sleep(1000);
                                            }
                                            else
                                            {
                                                mapPhoneIter->second.needPushFlag = 0;
                                            }
                                        }
                                    }
                                }
                                // std::cout << *tagit << ": " << atoi(val.substr(valuePos + 1, 1).c_str()) << "\n";
                            }
                        }
                        else if (!strcmp((val.substr(valuePos - 2, 2)).c_str(), "13")) //开关量
                        {
                            valuePos = val.find_first_of("\u0002");
                            if (valuePos != std::string::npos)
                            {
                                mapFireIter = alarmFireMap.find(*tagit);
                                tagValue = atoi(val.substr(valuePos + 1, 1).c_str());
                                // tagValue = atoi(val.c_str());

                                if (mapFireIter == alarmFireMap.end()) //位号表中找不到该点位
                                {
                                    tempAlarmStruct.needPushFlag = 0;
                                    tempAlarmStruct.tagName = *tagit;
                                    tempAlarmStruct.value = tagValue;
                                    tempAlarmStruct.valueTime = std::time(0);
                                    alarmFireMap.insert(std::pair<std::string, struct alarmTag>(*tagit, tempAlarmStruct));
                                    if (tagValue)
                                    {
                                        goto pushFireAlarm;
                                    }
                                }
                                else
                                {
                                    if (mapFireIter->second.needPushFlag && tagValue) //如果推送标志位和redis当前值都为1则直接跳到推送
                                    {
                                        goto pushFireAlarm;
                                    }
                                    if (tagValue != mapFireIter->second.value) // redis上的值与位号表中保存的值不同(变位)
                                    {
                                        mapFireIter->second.value = tagValue;
                                        mapFireIter->second.valueTime = std::time(0);
                                        if (tagValue)
                                        {
                                            mapFireIter->second.needPushFlag = 1;
                                        pushFireAlarm:
                                            time(&timep);
                                            strftime(tmp, sizeof(tmp), "[%Y-%m-%d %H:%M:%S]", localtime(&timep));
                                            std::cout << tmp << " [报警事件]: " << *tagit<<"\n";
                                            if (!pushHaiKang(*tagit))
                                            {
                                                Sleep(1000);
                                            }
                                            else
                                            {
                                                mapFireIter->second.needPushFlag = 0;
                                            }
                                        }
                                    }
                                }
                                // std::cout << *tagit << ": " << atoi(val.substr(valuePos + 1, 1).c_str()) << "\n";
                            }
                        }
                    }
                }
                // std::cout << *tagit << "\n";
            }
        }
        else
        {
            time(&timep);
            strftime(tmp, sizeof(tmp), "[%Y-%m-%d %H:%M:%S]", localtime(&timep));
            std::cout << tmp << "无法连接至Redis服务...\n";
            std::cout << tmp << "尝试重新连接Redis服务...\n";
            Sleep(1000);
            goto redisReconnect;
        }
    }
}

void invisionCheck(const char *tagName, std::string &value)
{
    valuePos = value.find_first_of("&");
    if (valuePos != std::string::npos)
    {
        if (!strcmp((value.substr(valuePos - 2, 2)).c_str(), "12"))
        {
            std::cout << tagName << ": " << (value.substr(value.find_first_of("\u0010") + 1, 1)).c_str() << "\n";
        }
        if (!strcmp((value.substr(valuePos - 2, 2)).c_str(), "13"))
        {
            std::cout << tagName << ": " << atoi((value.substr(value.find_first_of("\u0002") + 1, 1)).c_str()) << "\n";
        }
    }
}


bool pushHaiKang(const std::string &deviceId)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "deviceId", deviceId.c_str());
    time(&timep);
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cJSON_AddStringToObject(root, "happenTime", tmp);
    pushMsg = cJSON_Print(root);

    std::string utf8Body = _A2U8(pushMsg);

    //std::cout << "push:\n"
    //          << utf8Body << "\n";

    int dataLen = 0; // 返回数据长度
    char *rsp = httpUtil::HTTPUTIL_Post(url.c_str(), utf8Body.c_str(), appkey.c_str(), secret.c_str(), timeout, &dataLen);
    time(&timep);
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));

    if (NULL == rsp)
    {
        int status = httpUtil::HTTPUTIL_GetLastStatus(); // 获取状态码（根据取值判断是错误码还是 HTTP 状态码）

        std::cout << tmp << " [推送事件]: "
                  << "推送报警失败！--"
                  << "与海康平台连接失败!"
                  << "\n";
        std::cout << "[Err] "
                  << "status:" << status << "\n";
        if (status < httpUtil::HTTPUTIL_ERR_BASE)
        {
            // status 为 HTTP 状态码，请自行查阅 status 对应的 HTTP 状态描述
        }
        else
        {
            // status 为错误码，请参阅 OpenAPI 安全认证库（C/C++）状态码
        }
        free(pushMsg);
        pushMsg = nullptr;
        cJSON_Delete(root);
        return false;
    }
    else
    {
        std::string response = std::string(rsp, dataLen); // 不管是二进制还是字符串，指定了 dataLen 参数情况
        std::string localDecodedData = _U82A(response.c_str());
        httpUtil::HTTPUTIL_Free(rsp);

        cJSON *parseRoot = cJSON_Parse(localDecodedData.c_str());
        if (parseRoot == NULL)
        {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL)
            {
                std::cout << "Error before:" << error_ptr << "\n";
            }
            cJSON_Delete(root);
            return false;
        }
        cJSON *codeJson = cJSON_GetObjectItem(parseRoot, "code");
        if (atoi(codeJson->valuestring) == 0)
        {
            std::cout << tmp << " [推送事件]: "
                      << "推送报警成功！"
                      << "位号：" << deviceId << "\n";

            free(pushMsg);
            pushMsg = nullptr;
            cJSON_Delete(root);
            cJSON_Delete(parseRoot);
            return true;
        }
        else
        {
            cJSON *msgJson = cJSON_GetObjectItem(parseRoot, "msg");
            std::cout << tmp << " [推送事件]: "
                      << "推送报警失败！--"
                      << "海康平台接口调用失败!"
                      << "\n";
            std::cout << "[Err] Code:" << codeJson->valuestring << "\n";
            std::cout << "[Err] Msg:" << msgJson->valuestring << "\n";
            free(pushMsg);
            pushMsg = nullptr;
            cJSON_Delete(root);
            cJSON_Delete(parseRoot);
            return false;
        }
    }
}


