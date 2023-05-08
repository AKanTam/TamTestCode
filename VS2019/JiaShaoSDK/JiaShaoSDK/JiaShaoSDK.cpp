#include "NetDEVSDK.h"
#define HWND _SHAREHWND
#include <co/all.h>
#include <easylogging++.h>
// #define TEST

co::WaitGroup waitgroup;

INITIALIZE_EASYLOGGINGPP
void fun_sdk(std::string sdk_ip, bool sdk_cmd);

void fun_sdk(std::string sdk_ip, bool sdk_cmd)
{
    LOG(INFO) << "收到一条控制指令"
              << " IP: " << sdk_ip << " CMD: " << sdk_cmd;

    NETDEV_DEVICE_LOGIN_INFO_S stDevLoginInfo = {0};
    strncpy(stDevLoginInfo.szIPAddr, sdk_ip.c_str(), sizeof(stDevLoginInfo.szIPAddr)); // 设备IP地址
    strncpy(stDevLoginInfo.szUserName, "admin", sizeof(stDevLoginInfo.szUserName));    // 设备登录用户名
    strncpy(stDevLoginInfo.szPassword, "Abc@1234", sizeof(stDevLoginInfo.szPassword)); // 设备登录密码
    stDevLoginInfo.dwPort = 80;                                                        // 设备服务端口
    stDevLoginInfo.dwLoginProto = NETDEV_LOGIN_PROTO_ONVIF;                            // 登录协议

    NETDEV_SELOG_INFO_S stSELogInfo = {0};
    LPVOID lUserID = NETDEV_Login_V30(&stDevLoginInfo, &stSELogInfo);
    if (unlikely(NULL == lUserID))
    {
        LOG(ERROR) << "设备连接失败, 错误码: " << NETDEV_GetLastError() << " IP: " << sdk_ip.c_str() << " CMD: " << sdk_cmd;
        return;
    }
    INT32 dwChannelID = 0;
    INT32 lenth_sdk;

    NETDEV_ALARM_OUTPUT_LIST_S m_stAlarmOutputList = {0};
    if (unlikely(!NETDEV_GetDevConfig(lUserID, dwChannelID, NETDEV_GET_ALARM_OUTPUTCFG, &m_stAlarmOutputList, sizeof(m_stAlarmOutputList), &lenth_sdk)))
    {
        LOG(ERROR) << "获取设备IO信息失败" << NETDEV_GetLastError() << " IP: " << sdk_ip.c_str() << " CMD: " << sdk_cmd;
    }
    else
    {
        NETDEV_ALARM_OUTPUT_INFO_S stTriggerAlarmOutput = m_stAlarmOutputList.astAlarmOutputInfo[0];
        if (sdk_cmd)
        {
            stTriggerAlarmOutput.enDefaultStatus = tagNETDEVBooleanMode::NETDEV_BOOLEAN_MODE_CLOSE;
        }
        else
        {
            stTriggerAlarmOutput.enDefaultStatus = tagNETDEVBooleanMode::NETDEV_BOOLEAN_MODE_OPEN;
        }

        if (unlikely(!NETDEV_SetDevConfig(lUserID, 0, NETDEV_SET_ALARM_OUTPUTCFG, &stTriggerAlarmOutput, sizeof(stTriggerAlarmOutput))))
        {
            LOG(ERROR) << "雨刷控制失败 错误码： " << NETDEV_GetLastError() << " IP: " << sdk_ip.c_str() << " CMD: " << sdk_cmd;
        }
        else
        {
            LOG(INFO) << "雨刷控制成功"
                      << " IP: " << sdk_ip.c_str() << " CMD: " << sdk_cmd;
        }
    }
    // 注销用户
    NETDEV_Logout(lUserID);
}

void cb(const http::Req &req, http::Res &res)
{
    if (req.url() == "/api/yushi/sdk/control")
    {
        std::string body = req.body();

        go([body]
           {
                json::Json json = json::parse(body);
                fun_sdk(json.get("IP").as_string().c_str(), json.get("control").as_int32()); });

        res.set_status(200);
    }
    else
    {
        res.set_status(404);
    }
}

DEF_main(argc, argv)
{
    el::Configurations conf("my_log.conf");
    el::Loggers::reconfigureAllLoggers(conf);

    NETDEV_Init();
    // 设置连接时间
    NETDEV_REV_TIMEOUT_S stRevTimeout = {0};
    stRevTimeout.dwRevTimeOut = 5;
    stRevTimeout.dwFileReportTimeOut = 30;
    NETDEV_SetRevTimeOut(&stRevTimeout);

    http::Server srv;
    srv.on_req(cb);
    srv.start("0.0.0.0", 8888);

    LOG(INFO) << "监听服务已开启";

#ifdef TEST
    while (1)
    {
        for (int i = 1; i < 218; i++)
        {
            httplib::Client cli("127.0.0.1:8888");
            const char *str_con11 = "{\"IP\":\"33.65.132.";
            const char *str_con12 = "\",\"control\":0}";
            std::string str = str_con11;
            str.append(std::to_string(i));
            str.append(str_con12);
            // http::Client cli("127.0.0.1:8888");
            cli.Post("/api/yushi/sdk/control", str, "application/json");
        }
        Sleep(1000 * 60 * 5);
    }
#endif // TEST

    waitgroup.add();
    waitgroup.wait();
    return 0;
}