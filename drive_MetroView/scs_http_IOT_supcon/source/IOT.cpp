#include "IOT.h"
char iot_logbuff[1024];

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
    std::string strTemp = str;
    if (wstr)
        delete[] wstr;
    if (str)
        delete[] str;
    return strTemp;
}

static std::string GBKToUTF8(const char *strGBK)
{
    int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
    wchar_t *wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    std::string strTemp = str;
    if (wstr)
        delete[] wstr;
    if (str)
        delete[] str;
    return strTemp;
}

IOT::IOT(std::string address)
{
    client_P = new httplib::Client(address.c_str());
}

IOT::IOT()
{
}

IOT::~IOT()
{
}

bool IOT::Login(const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    std::ifstream IOT_Infile;
    std::string IOT_Path;
    std::string IOT_Buf;

    if (client_P == nullptr)
    {
        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[IOT] ERROR Login() 客户端创建失败 line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        return false;
    }
    if (std::getenv("SYS_ROOT") != nullptr)
    {

        IOT_Path = std::getenv("SYS_ROOT");
        IOT_Path += "\\config\\IOTconfig.json";
        IOT_Infile.open(IOT_Path, std::ios::in);
        if (IOT_Infile.is_open())
        {
            while (getline(IOT_Infile, IOT_Buf))
            {
                IOT_LoginChar += IOT_Buf;
            }

            if (auto resPost = client_P->Post("/supaiot/api/v2/app/sec/login", IOT_LoginChar.c_str(), "application/json"))
            {
                if (resPost->status == 200)
                {
                    token.clear();

                    httplib::Headers headersLogin = resPost->headers;
                    if (headersLogin.find("set-cookie") != headersLogin.end())
                    {
                        token = headersLogin.find("set-cookie")->second;
                        if (headers.find("cookie") == headers.end())
                        {
                            headers.insert({"cookie", token});
                        }
                        else
                        {
                            headers.erase("cookie");
                            headers.insert({"cookie", token});
                        }

                        IOT_Infile.close();
                        memset(iot_logbuff, 0, sizeof(iot_logbuff));
                        sprintf(iot_logbuff, "[IOT] INFO Login() 物联智控用户登录成功! line[%d]!", __LINE__);
                        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                        return true;
                    }
                    else
                    {
                        memset(iot_logbuff, 0, sizeof(iot_logbuff));
                        sprintf(iot_logbuff, "[IOT] ERROR Login() 物联智控用户登录失败！ 请检查配置文件是否正确！ line[%d]!", __LINE__);
                        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    }
                }
                else
                {
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[IOT] ERROR Login() 物联智控接口返回错误！ status[%d] line[%d]!", resPost->status, __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                }
            }
            else
            {
                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[IOT] ERROR Login() 物联智控接口连接失败！ addr[/supaiot/api/v2/app/sec/login] line[%d]!", __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
            }
        }
        else
        {
            memset(iot_logbuff, 0, sizeof(iot_logbuff));
            sprintf(iot_logbuff, "[IOT] ERROR Login() 登录配置文件未找到 addr[%s] line[%d]!", IOT_Path.c_str(), __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        }
    }

    IOT_Infile.close();
    return false;
}

bool IOT::Query(std::multimap<std::string, std::map<std::string, TAGINFO>> *_tagMap, const int startNum, const int tagNum, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    if (_tagMap->empty())
    {
        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[IOT] ERROR Query() tagMap为空! line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        return false;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *idArray = cJSON_AddArrayToObject(root, "ID");
    cJSON *id_name = NULL;

    auto itTagMap = _tagMap->begin();

    for (int i = 0; i < startNum; i++)
    {
        itTagMap++;
    }

    for (int i = 0; i < tagNum; i++)
    {
        id_name = cJSON_CreateString(itTagMap->first.c_str());
        cJSON_AddItemToArray(idArray, id_name);
        itTagMap++;
    }
    bodyChar = cJSON_Print(root);

    if (bodyChar == NULL)
    {
        cJSON_Delete(root);

        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[IOT] ERROR Query() Json字符串创建失败! line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        return false;
    }

    if (auto resPost = client_P->Post("/supaiot/api/v2/data/real/device/list", headers, GBKToUTF8(bodyChar).c_str(), "application/json"))
    {
        if (resPost->status == 200)
        {
            // memset(iot_logbuff, 0, sizeof(iot_logbuff));
            // sprintf(iot_logbuff, "[IOT] body:%s\nresult:%s", GBKToUTF8(bodyChar).c_str(), UtfToGbk(resPost->body).c_str(), __LINE__);
            // FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

            cJSON *json = cJSON_Parse(UtfToGbk(resPost->body).c_str());
            cJSON *result = cJSON_GetObjectItemCaseSensitive(json, "result");
            cJSON *resultCode = cJSON_GetObjectItemCaseSensitive(result, "resultCode");
            if (!strcmp(resultCode->valuestring, "0"))
            {

                cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
                cJSON *item = NULL;
                cJSON *stateItem = NULL;
                cJSON *id = NULL;
                cJSON *state = NULL;
                std::multimap<std::string, std::map<std::string, TAGINFO>>::iterator deviceMapIt;
                std::map<std::string, TAGINFO>::iterator stateMapIt;
                cJSON_ArrayForEach(item, data)
                {
                    id = cJSON_GetObjectItemCaseSensitive(item, "ID");

                    deviceMapIt = _tagMap->find(id->valuestring);

                    if (deviceMapIt != _tagMap->end())
                    {
                        state = cJSON_GetObjectItemCaseSensitive(item, "state");
                        for (int cout_deviceID = 0; cout_deviceID != _tagMap->count(id->valuestring); cout_deviceID++, deviceMapIt++)
                        {
                            cJSON_ArrayForEach(stateItem, state)
                            {
                                stateMapIt = deviceMapIt->second.find(stateItem->string);
                                if (stateMapIt != deviceMapIt->second.end())
                                {
                                    switch (stateItem->type)
                                    {
                                    case cJSON_False:
                                        stateMapIt->second.value = 0;
                                        break;
                                    case cJSON_True:
                                        stateMapIt->second.value = 1;
                                        break;
                                    case cJSON_Number:
                                        stateMapIt->second.value = stateItem->valuedouble;
                                        break;
                                    case cJSON_String:
                                        stateMapIt->second.value_s = stateItem->valuestring;
                                        break;
                                    default:
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                cJSON_Delete(json);
            }
            else
            {
                cJSON *resultError = cJSON_GetObjectItemCaseSensitive(result, "resultError");

                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[IOT] ERROR Query() 物联智控接口返回错误！ status[%d] body[%s] line[%d]!", resPost->status, resultError->valuestring, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                cJSON_Delete(json);

                this->Login(_groupName, _groupNo, _linkName);

                if (auto resPostTwice = client_P->Post("/supaiot/api/v2/data/real/device/list", headers, GBKToUTF8(bodyChar).c_str(), "application/json"))
                {
                    if (resPostTwice->status == 200)
                    {
                        cJSON *jsonTwice = cJSON_Parse(UtfToGbk(resPostTwice->body).c_str());
                        cJSON *resultTwice = cJSON_GetObjectItemCaseSensitive(jsonTwice, "result");
                        cJSON *resultCodeTwice = cJSON_GetObjectItemCaseSensitive(resultTwice, "resultCode");
                        if (!strcmp(resultCodeTwice->valuestring, "0"))
                        {
                            cJSON *data = cJSON_GetObjectItemCaseSensitive(jsonTwice, "data");
                            cJSON *item = NULL;
                            cJSON *stateItem = NULL;
                            cJSON *id = NULL;
                            cJSON *state = NULL;
                            std::multimap<std::string, std::map<std::string, TAGINFO>>::iterator deviceMapIt;
                            std::map<std::string, TAGINFO>::iterator stateMapIt;
                            cJSON_ArrayForEach(item, data)
                            {
                                id = cJSON_GetObjectItemCaseSensitive(item, "ID");

                                deviceMapIt = _tagMap->find(id->valuestring);
                                if (deviceMapIt != _tagMap->end())
                                {
                                    state = cJSON_GetObjectItemCaseSensitive(item, "state");
                                    for (int cout_deviceID = 0; cout_deviceID != _tagMap->count(id->valuestring); cout_deviceID++, deviceMapIt++)
                                    {
                                        cJSON_ArrayForEach(stateItem, state)
                                        {
                                            stateMapIt = deviceMapIt->second.find(stateItem->string);
                                            if (stateMapIt != deviceMapIt->second.end())
                                            {
                                                switch (stateItem->type)
                                                {
                                                case cJSON_False:
                                                    stateMapIt->second.value = 0;
                                                    break;
                                                case cJSON_True:
                                                    stateMapIt->second.value = 1;
                                                    break;
                                                case cJSON_Number:
                                                    stateMapIt->second.value = stateItem->valuedouble;
                                                    break;
                                                case cJSON_String:
                                                    stateMapIt->second.value_s = stateItem->valuestring;
                                                    break;
                                                default:
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            cJSON_Delete(jsonTwice);
                        }
                        else
                        {
                            cJSON *resultErrorTwice = cJSON_GetObjectItemCaseSensitive(result, "resultError");

                            memset(iot_logbuff, 0, sizeof(iot_logbuff));
                            sprintf(iot_logbuff, "[IOT] ERROR Query() 物联智控接口返回错误！ status[%d] body[%s] line[%d]!", resPostTwice->status, resultErrorTwice->valuestring, __LINE__);
                            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                            cJSON_Delete(jsonTwice);
                            cJSON_Delete(root);
                            free(bodyChar);
                            bodyChar = NULL;
                            return false;
                        }
                    }
                    else
                    {
                        memset(iot_logbuff, 0, sizeof(iot_logbuff));
                        sprintf(iot_logbuff, "[IOT] ERROR Query() 物联智控接口连接状态错误！ status[%d] line[%d]!", resPostTwice->status, __LINE__);
                        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                        cJSON_Delete(root);
                        free(bodyChar);
                        bodyChar = NULL;
                        return false;
                    }
                }
                else
                {
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[IOT] ERROR Query() 物联智控接口连接失败！ addr[/supaiot/api/v2/data/real/device/list] line[%d]!", __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    cJSON_Delete(root);
                    free(bodyChar);
                    bodyChar = NULL;
                    return false;
                }
            }
        }
        else
        {
            memset(iot_logbuff, 0, sizeof(iot_logbuff));
            sprintf(iot_logbuff, "[IOT] ERROR Query() 物联智控接口连接状态错误！ status[%d] line[%d]!", resPost->status, __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
            return false;
        }
    }
    else
    {
        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[IOT] ERROR Query() 物联智控接口连接失败！ addr[/supaiot/api/v2/data/real/device/list] line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        return false;
    }

    cJSON_Delete(root);
    free(bodyChar);
    bodyChar = NULL;

    return true;
}

bool IOT::Control(const std::string *deviceId, const std::string *deviceControl, const std::string *_value, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{

    memset(iot_logbuff, 0, sizeof(iot_logbuff));
    sprintf(iot_logbuff, "[IOT] INFO Control() 异步控制开始下发！ deviceId[%s] deviceControl[%s] value[%s] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "ID", deviceId->c_str());
    cJSON_AddStringToObject(root, "control", deviceControl->c_str());
    cJSON_AddStringToObject(root, "value", _value->c_str());

    char *controlChar = cJSON_Print(root);
    if (controlChar == NULL)
    {
        cJSON_Delete(root);

        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[IOT] ERROR Control() Json字符串创建失败! line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        return false;
    }

    if (auto resPost = client_P->Post("/supaiot/api/v2/command/control/state/async", headers, GBKToUTF8(controlChar).c_str(), "application/json"))
    {
        if (resPost->status == 200)
        {
            cJSON *json = cJSON_Parse(UtfToGbk(resPost->body).c_str());
            cJSON *result = cJSON_GetObjectItemCaseSensitive(json, "result");
            cJSON *resultCode = cJSON_GetObjectItemCaseSensitive(result, "resultCode");
            if (!strcmp(resultCode->valuestring, "0"))
            {
                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[IOT] INFO Control() 异步控制下发成功！ deviceId[%s] deviceControl[%s] value[%s] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                cJSON_Delete(json);
            }
            else
            {
                cJSON *resultError = cJSON_GetObjectItemCaseSensitive(result, "resultError");

                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口返回错误！ status[%d] resultError[%s] line[%d]!", resPost->status, resultError->valuestring, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[IOT] ERROR Control() 异步控制下发失败！ deviceId[%s] deviceControl[%s] value[%s] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                cJSON_Delete(json);

                this->Login(_groupName, _groupNo, _linkName);
                if (auto resPostTwice = client_P->Post("/supaiot/api/v2/command/control/state/async", headers, GBKToUTF8(controlChar).c_str(), "application/json"))
                {
                    if (resPostTwice->status == 200)
                    {
                        cJSON *jsonTwice = cJSON_Parse(UtfToGbk(resPostTwice->body).c_str());
                        cJSON *resultTwice = cJSON_GetObjectItemCaseSensitive(jsonTwice, "result");
                        cJSON *resultCodeTwice = cJSON_GetObjectItemCaseSensitive(resultTwice, "resultCode");
                        if (!strcmp(resultCodeTwice->valuestring, "0"))
                        {
                            memset(iot_logbuff, 0, sizeof(iot_logbuff));
                            sprintf(iot_logbuff, "[IOT] INFO Control() 异步控制下发成功！ deviceId[%s] deviceControl[%s] value[%s] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
                            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                            cJSON_Delete(jsonTwice);
                        }
                        else
                        {
                            memset(iot_logbuff, 0, sizeof(iot_logbuff));
                            sprintf(iot_logbuff, "[IOT] ERROR Control() 异步控制下发失败！ deviceId[%s] deviceControl[%s] value[%s] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
                            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                            cJSON *resultErrorTwice = cJSON_GetObjectItemCaseSensitive(resultTwice, "resultError");

                            memset(iot_logbuff, 0, sizeof(iot_logbuff));
                            sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口返回错误！ status[%d] resultError[%s] line[%d]!", resPost->status, resultError->valuestring, __LINE__);
                            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                            cJSON_Delete(jsonTwice);
                            cJSON_Delete(root);
                            free(controlChar);
                            controlChar = NULL;
                            return false;
                        }
                    }
                    else
                    {
                        memset(iot_logbuff, 0, sizeof(iot_logbuff));
                        sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口连接状态错误！ status[%d] line[%d]!", resPostTwice->status, __LINE__);
                        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                        cJSON_Delete(root);
                        free(controlChar);
                        controlChar = NULL;
                        return false;
                    }
                }
                else
                {
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口连接失败！ addr[/supaiot/api/v2/command/control/state/async] line[%d]!", __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    cJSON_Delete(root);
                    free(controlChar);
                    controlChar = NULL;
                    return false;
                }
            }
        }
        else
        {
            memset(iot_logbuff, 0, sizeof(iot_logbuff));
            sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口连接状态错误！ status[%d] line[%d]!", resPost->status, __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
            cJSON_Delete(root);
            free(controlChar);
            controlChar = NULL;
            return false;
        }
    }
    else
    {
        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口连接失败！ addr[/supaiot/api/v2/command/control/state/async] line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        cJSON_Delete(root);
        free(controlChar);
        controlChar = NULL;
        return false;
    }

    cJSON_Delete(root);
    free(controlChar);
    controlChar = NULL;
    return true;
}

bool IOT::Control(const std::string *deviceId, const std::string *deviceControl, const double _value, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{

    memset(iot_logbuff, 0, sizeof(iot_logbuff));
    sprintf(iot_logbuff, "[IOT] INFO Control() 异步控制开始下发！ deviceId[%s] deviceControl[%s] value[%f] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "ID", deviceId->c_str());
    cJSON_AddStringToObject(root, "control", deviceControl->c_str());
    cJSON_AddNumberToObject(root, "value", _value);

    char *controlChar = cJSON_Print(root);
    if (controlChar == NULL)
    {
        cJSON_Delete(root);

        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[IOT] ERROR Control() Json字符串创建失败! line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        return false;
    }

    if (auto resPost = client_P->Post("/supaiot/api/v2/command/control/state/async", headers, GBKToUTF8(controlChar).c_str(), "application/json"))
    {
        if (resPost->status == 200)
        {
            cJSON *json = cJSON_Parse(UtfToGbk(resPost->body).c_str());
            cJSON *result = cJSON_GetObjectItemCaseSensitive(json, "result");
            cJSON *resultCode = cJSON_GetObjectItemCaseSensitive(result, "resultCode");
            if (!strcmp(resultCode->valuestring, "0"))
            {
                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[IOT] INFO Control() 异步控制下发成功！ deviceId[%s] deviceControl[%s] value[%f] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                cJSON_Delete(json);
            }
            else
            {
                cJSON *resultError = cJSON_GetObjectItemCaseSensitive(result, "resultError");

                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口返回错误！ status[%d] resultError[%s] line[%d]!", resPost->status, resultError->valuestring, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[IOT] ERROR Control() 异步控制下发失败！ deviceId[%s] deviceControl[%s] value[%f] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                cJSON_Delete(json);

                this->Login(_groupName, _groupNo, _linkName);
                if (auto resPostTwice = client_P->Post("/supaiot/api/v2/command/control/state/async", headers, GBKToUTF8(controlChar).c_str(), "application/json"))
                {
                    if (resPostTwice->status == 200)
                    {
                        cJSON *jsonTwice = cJSON_Parse(UtfToGbk(resPostTwice->body).c_str());
                        cJSON *resultTwice = cJSON_GetObjectItemCaseSensitive(jsonTwice, "result");
                        cJSON *resultCodeTwice = cJSON_GetObjectItemCaseSensitive(resultTwice, "resultCode");
                        if (!strcmp(resultCodeTwice->valuestring, "0"))
                        {
                            memset(iot_logbuff, 0, sizeof(iot_logbuff));
                            sprintf(iot_logbuff, "[IOT] INFO Control() 异步控制下发成功！ deviceId[%s] deviceControl[%s] value[%f] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
                            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                            cJSON_Delete(jsonTwice);
                        }
                        else
                        {
                            memset(iot_logbuff, 0, sizeof(iot_logbuff));
                            sprintf(iot_logbuff, "[IOT] ERROR Control() 异步控制下发失败！ deviceId[%s] deviceControl[%s] value[%f] line[%d]!", deviceId->c_str(), deviceControl->c_str(), _value, __LINE__);
                            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);

                            cJSON *resultErrorTwice = cJSON_GetObjectItemCaseSensitive(resultTwice, "resultError");

                            memset(iot_logbuff, 0, sizeof(iot_logbuff));
                            sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口返回错误！ status[%d] resultError[%s] line[%d]!", resPost->status, resultError->valuestring, __LINE__);
                            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                            cJSON_Delete(jsonTwice);
                            cJSON_Delete(root);
                            free(controlChar);
                            controlChar = NULL;
                            return false;
                        }
                    }
                    else
                    {
                        memset(iot_logbuff, 0, sizeof(iot_logbuff));
                        sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口连接状态错误！ status[%d] line[%d]!", resPostTwice->status, __LINE__);
                        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                        cJSON_Delete(root);
                        free(controlChar);
                        controlChar = NULL;
                        return false;
                    }
                }
                else
                {
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口连接失败！ addr[/supaiot/api/v2/command/control/state/async] line[%d]!", __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    cJSON_Delete(root);
                    free(controlChar);
                    controlChar = NULL;
                    return false;
                }
            }
        }
        else
        {
            memset(iot_logbuff, 0, sizeof(iot_logbuff));
            sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口连接状态错误！ status[%d] line[%d]!", resPost->status, __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
            cJSON_Delete(root);
            free(controlChar);
            controlChar = NULL;
            return false;
        }
    }
    else
    {
        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[IOT] ERROR Control() 物联智控接口连接失败！ addr[/supaiot/api/v2/command/control/state/async] line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        cJSON_Delete(root);
        free(controlChar);
        controlChar = NULL;
        return false;
    }

    cJSON_Delete(root);
    free(controlChar);
    controlChar = NULL;
    return true;
}