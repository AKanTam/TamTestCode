#include "toilet.h"
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

TOILET::TOILET(std::string address)
{
    client_P = new httplib::Client(address.c_str());
}

TOILET::TOILET()
{
}

TOILET::~TOILET()
{
}

bool TOILET::Login(const void *_groupName, hInt32 _groupNo, const void *_linkName)
{

    if (client_P == nullptr)
    {
        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[TOILET] ERROR Login() 客户端创建失败 line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        return false;
    }

    httplib::Params params{
        {"username", "admin"},
        {"password", "aK7+UX24ttBgfTnAndz9aQ=="}};
    httplib::Headers headersLogin = {
        {"Authorization", "Basic Z215OmdteQ=="}, {"Content-Type", "application/x-www-form-urlencoded"}};

    if (auto resPost = client_P->Post("/api/auth/oauth/token?grant_type=password&no_checkout=open", headersLogin, params))
    {
        if (resPost->status == 200)
        {
            cJSON *json = cJSON_Parse(resPost->body.c_str());
            if (json == NULL)
            {
                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[TOILET] ERROR Login() json解析失败! 登录失败! line[%d]!", __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                return false;
            }
            cJSON *access_token = cJSON_GetObjectItemCaseSensitive(json, "access_token");
            if (cJSON_IsString(access_token) && access_token->valuestring != NULL)
            {
                token.clear();
                token = "Bearer ";
                token += access_token->valuestring;
                if (headers.find("Authorization") == headers.end())
                {
                    headers.insert({"Authorization", token});
                }
                else
                {
                    headers.erase("Authorization");
                    headers.insert({"Authorization", token});
                }

                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[TOILET] INFO Login() 光明源智慧公厕登录成功! line[%d]!", __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                return true;
            }
            else
            {
                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[TOILET] ERROR Login() 光明源智慧公厕登录失败！ line[%d]!", __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
            }
        }
        else
        {
            memset(iot_logbuff, 0, sizeof(iot_logbuff));
            sprintf(iot_logbuff, "[TOILET] ERROR Login() 光明源智慧公厕接口返回错误！ status[%d] line[%d]!", resPost->status, __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
        }
    }
    else
    {
        memset(iot_logbuff, 0, sizeof(iot_logbuff));
        sprintf(iot_logbuff, "[TOILET] ERROR Login() 光明源智慧公厕接口连接失败！ addr[/api/auth/oauth/token?grant_type=password&no_checkout=open] line[%d]!", __LINE__);
        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
    }

    return false;
}

bool TOILET::QueryClean(std::map<std::string, std::map<std::string, TAGINFO>> *_cleanMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    if (_cleanMap->empty())
    {
        return false;
    }
    for (std::map<std::string, std::map<std::string, TAGINFO>>::iterator itId = _cleanMap->begin(); itId != _cleanMap->end(); itId++)
    {
        std::string body = "{\"toiletId\": \"" + itId->first + "\"}";
        if (auto resPost = client_P->Post("/api/assets/cleaning/foreign/get-clean-data", headers, body, "application/json"))
        {
            if (resPost->status == 200)
            {
                cJSON *json = cJSON_Parse(UtfToGbk(resPost->body).c_str());
                cJSON *resultCode = cJSON_GetObjectItemCaseSensitive(json, "code");
                if (resultCode->valuedouble == 0)
                {
                    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
                    cJSON *item = NULL;
                    cJSON *men = NULL;
                    cJSON *women = NULL;
                    cJSON_ArrayForEach(item, data)
                    {
                        men = cJSON_GetObjectItemCaseSensitive(item, "MAN");
                        women = cJSON_GetObjectItemCaseSensitive(item, "WOMAN");
                        if (men != NULL)
                        {
                            auto menPt = itId->second.find("MAN");
                            if (menPt != itId->second.end())
                            {
                                menPt->second.value = men->valuedouble;
                            }
                        }
                        if (women != NULL)
                        {
                            auto womenPt = itId->second.find("WOMAN");
                            if (womenPt != itId->second.end())
                            {
                                womenPt->second.value = women->valuedouble;
                            }
                        }
                    }
                    cJSON_Delete(json);
                }
                else
                {
                    cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg");
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口返回错误！ status[%d] msg[%s] line[%d]!", resPost->status, msg->valuestring, __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    cJSON_Delete(json);
                    return false;
                }
            }
            else
            {
                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接状态错误！ status[%d] line[%d]!", resPost->status, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                return false;
            }
        }
        else
        {
            memset(iot_logbuff, 0, sizeof(iot_logbuff));
            sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接失败！ addr[/api/assets/cleaning/foreign/get-clean-data] line[%d]!", __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
            return false;
        }
    }
    return true;
}

bool TOILET::QueryIdle(std::map<std::string, std::map<std::string, TAGINFO>> *_idleMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    if (_idleMap->empty())
    {
        return false;
    }
    for (std::map<std::string, std::map<std::string, TAGINFO>>::iterator itId = _idleMap->begin(); itId != _idleMap->end(); itId++)
    {
        if (auto resPost = client_P->Get(("/api/admin/crm-toilet-section-pit-state/get-occupancy/?toiletId=" + itId->first).c_str(), headers))
        {
            if (resPost->status == 200)
            {
                cJSON *json = cJSON_Parse(UtfToGbk(resPost->body).c_str());
                cJSON *resultCode = cJSON_GetObjectItemCaseSensitive(json, "code");
                if (resultCode->valuedouble == 0)
                {
                    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
                    cJSON *item = NULL;
                    cJSON *men = NULL;
                    cJSON *women = NULL;
                    cJSON *sectionType = NULL;
                    cJSON *latrineBuysCnt = NULL;
                    cJSON *latrineCnt = NULL;
                    cJSON_ArrayForEach(item, data)
                    {
                        sectionType = cJSON_GetObjectItemCaseSensitive(item, "sectionType");

                        if (cJSON_IsString(sectionType))
                        {
                            latrineBuysCnt = cJSON_GetObjectItemCaseSensitive(item, "latrineBuysCnt");
                            latrineCnt = cJSON_GetObjectItemCaseSensitive(item, "latrineCnt");
                            if (latrineBuysCnt != NULL)
                            {
                                std::string tempStr = sectionType->valuestring;
                                tempStr += "_idleCnt";
                                auto menPt = itId->second.find(tempStr);
                                if (menPt != itId->second.end())
                                {
                                    menPt->second.value = (latrineCnt->valuedouble) - (latrineBuysCnt->valuedouble);
                                }
                            }
                        }
                    }
                    cJSON_Delete(json);
                }
                else
                {
                    cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg");
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口返回错误！ status[%d] msg[%s] line[%d]!", resPost->status, msg->valuestring, __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    cJSON_Delete(json);
                    return false;
                }
            }
            else
            {
                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接状态错误！ status[%d] line[%d]!", resPost->status, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                return false;
            }
        }
        else
        {
            memset(iot_logbuff, 0, sizeof(iot_logbuff));
            sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接失败！ addr[/api/admin/crm-toilet-section-pit-state/get-occupancy] line[%d]!", __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
            return false;
        }
    }
    return true;
}

bool TOILET::QueryEnergy(std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> *_energyMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    if (_energyMap->empty())
    {
        return false;
    }
    for (auto itId = _energyMap->begin(); itId != _energyMap->end(); itId++)
    {
        for (auto itIdd = itId->second.begin(); itIdd != itId->second.end(); itIdd++)
        {
            if (itIdd->first == "ELECTRIC")
            {
                httplib::Headers electricHeaders = headers;
                // electricHeaders.insert({"TENANT-ID", itId->first});
                electricHeaders.insert({"TENANT-ID", "1"});
                if (auto resPost = client_P->Get(("/api/admin/val-electric/get-electric/?toiletId=" + itId->first).c_str(), electricHeaders))
                {
                    if (resPost->status == 200)
                    {
                        cJSON *json = cJSON_Parse(UtfToGbk(resPost->body).c_str());
                        cJSON *resultCode = cJSON_GetObjectItemCaseSensitive(json, "code");
                        if (resultCode->valuedouble == 0)
                        {
                            cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
                            cJSON *item = NULL;
                            cJSON *val = NULL;
                            cJSON *type = NULL;
                            cJSON *sectionType = NULL;
                            std::string tempStr;
                            cJSON_ArrayForEach(item, data)
                            {
                                sectionType = cJSON_GetObjectItemCaseSensitive(item, "sectionType");
                                if (cJSON_IsString(sectionType))
                                {
                                    val = cJSON_GetObjectItemCaseSensitive(item, "dayTotal");
                                    if (val != NULL)
                                    {
                                        tempStr.clear();
                                        tempStr = sectionType->valuestring;
                                        tempStr += "_";
                                        tempStr += val->string;
                                        auto typePt = itIdd->second.find(tempStr);
                                        if (typePt != itIdd->second.end())
                                        {
                                            typePt->second.value = val->valuedouble;
                                        }
                                    }
                                    val = cJSON_GetObjectItemCaseSensitive(item, "monthTotal");
                                    if (val != NULL)
                                    {
                                        tempStr.clear();
                                        tempStr = sectionType->valuestring;
                                        tempStr += "_";
                                        tempStr += val->string;
                                        auto typePt = itIdd->second.find(tempStr);
                                        if (typePt != itIdd->second.end())
                                        {
                                            typePt->second.value = val->valuedouble;
                                        }
                                    }
                                }
                            }
                            cJSON_Delete(json);
                        }
                        else
                        {
                            cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg");
                            memset(iot_logbuff, 0, sizeof(iot_logbuff));
                            sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口返回错误！ status[%d] msg[%s] line[%d]!", resPost->status, msg->valuestring, __LINE__);
                            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                            cJSON_Delete(json);
                            return false;
                        }
                    }
                    else
                    {
                        memset(iot_logbuff, 0, sizeof(iot_logbuff));
                        sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接状态错误！ status[%d] line[%d]!", resPost->status, __LINE__);
                        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                        return false;
                    }
                }
                else
                {
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接失败！ addr[/api/admin/val-electric/get-electric] line[%d]!", __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    return false;
                }
            }
            else if (itIdd->first == "WATER")
            {
                if (auto resPost = client_P->Get(("/api/admin/val-water/get-water/?toiletId=" + itId->first).c_str(), headers))
                {
                    if (resPost->status == 200)
                    {
                        cJSON *json = cJSON_Parse(UtfToGbk(resPost->body).c_str());
                        cJSON *resultCode = cJSON_GetObjectItemCaseSensitive(json, "code");
                        if (resultCode->valuedouble == 0)
                        {
                            cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
                            cJSON *item = NULL;
                            cJSON *val = NULL;
                            cJSON *type = NULL;
                            cJSON *sectionType = NULL;
                            std::string tempStr;
                            cJSON_ArrayForEach(item, data)
                            {
                                sectionType = cJSON_GetObjectItemCaseSensitive(item, "sectionType");
                                if (cJSON_IsString(sectionType))
                                {
                                    val = cJSON_GetObjectItemCaseSensitive(item, "dayTotal");

                                    if (val != NULL)
                                    {
                                        tempStr.clear();
                                        tempStr = sectionType->valuestring;
                                        tempStr += "_";
                                        tempStr += val->string;
                                        auto typePt = itIdd->second.find(tempStr);
                                        if (typePt != itIdd->second.end())
                                        {
                                            typePt->second.value = val->valuedouble;
                                        }
                                    }
                                    val = cJSON_GetObjectItemCaseSensitive(item, "monthTotal");
                                    if (val != NULL)
                                    {
                                        tempStr.clear();
                                        tempStr = sectionType->valuestring;
                                        tempStr += "_";
                                        tempStr += val->string;
                                        auto typePt = itIdd->second.find(tempStr);
                                        if (typePt != itIdd->second.end())
                                        {
                                            typePt->second.value = val->valuedouble;
                                        }
                                    }
                                }
                            }
                            cJSON_Delete(json);
                        }
                        else
                        {
                            cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg");
                            memset(iot_logbuff, 0, sizeof(iot_logbuff));
                            sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口返回错误！ status[%d] msg[%s] line[%d]!", resPost->status, msg->valuestring, __LINE__);
                            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                            cJSON_Delete(json);
                            return false;
                        }
                    }
                    else
                    {
                        memset(iot_logbuff, 0, sizeof(iot_logbuff));
                        sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接状态错误！ status[%d] line[%d]!", resPost->status, __LINE__);
                        FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                        return false;
                    }
                }
                else
                {
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接失败！ addr[/api/admin/val-water/get-water] line[%d]!", __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    return false;
                }
            }
        }
    }
    return true;
}

bool TOILET::QueryEnvi(std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> *_enviMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    if (_enviMap->empty())
    {
        return false;
    }
    for (auto itId = _enviMap->begin(); itId != _enviMap->end(); itId++)
    {
        if (auto resPost = client_P->Get(("/api/admin/val-environment/?toiletId=" + itId->first).c_str(), headers))
        {
            if (resPost->status == 200)
            {
                cJSON *json = cJSON_Parse(UtfToGbk(resPost->body).c_str());
                cJSON *resultCode = cJSON_GetObjectItemCaseSensitive(json, "code");
                if (resultCode->valuedouble == 0)
                {
                    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
                    cJSON *item = NULL;
                    cJSON *type = NULL;
                    cJSON *typeItem = NULL;
                    cJSON *dataItem = NULL;
                    cJSON *sectionType = NULL;
                    cJSON *valEnvList = NULL;
                    cJSON_ArrayForEach(item, data)
                    {
                        sectionType = cJSON_GetObjectItemCaseSensitive(item, "sectionType");
                        if (!strcmp(sectionType->valuestring, "MAN"))
                        {
                            auto typePt = itId->second.find("MAN");
                            if (typePt != itId->second.end())
                            {
                                valEnvList = cJSON_GetObjectItemCaseSensitive(item, "valEnvList");
                                type = cJSON_GetArrayItem(valEnvList, 0);
                                cJSON_ArrayForEach(dataItem, type)
                                {
                                    auto dataPt = typePt->second.find(dataItem->string);
                                    if (dataPt != typePt->second.end())
                                    {
                                        dataPt->second.value = dataItem->valuedouble;
                                    }
                                }
                            }
                        }
                        else if (!strcmp(sectionType->valuestring, "WOMAN"))
                        {
                            auto typePt = itId->second.find("WOMAN");
                            if (typePt != itId->second.end())
                            {
                                valEnvList = cJSON_GetObjectItemCaseSensitive(item, "valEnvList");
                                type = cJSON_GetArrayItem(valEnvList, 0);
                                cJSON_ArrayForEach(dataItem, type)
                                {
                                    auto dataPt = typePt->second.find(dataItem->string);
                                    if (dataPt != typePt->second.end())
                                    {
                                        dataPt->second.value = dataItem->valuedouble;
                                    }
                                }
                            }
                        }
                    }
                    cJSON_Delete(json);
                }
                else
                {
                    cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg");
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口返回错误！ status[%d] msg[%s] line[%d]!", resPost->status, msg->valuestring, __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    cJSON_Delete(json);
                    return false;
                }
            }
            else
            {
                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接状态错误！ status[%d] line[%d]!", resPost->status, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                return false;
            }
        }
        else
        {
            memset(iot_logbuff, 0, sizeof(iot_logbuff));
            sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接失败！ addr[/api/admin/val-environment] line[%d]!", __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
            return false;
        }
    }
    return true;
}

bool TOILET::QueryPit(std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> *_pitMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
    if (_pitMap->empty())
    {
        return false;
    }
    for (auto itId = _pitMap->begin(); itId != _pitMap->end(); itId++)
    {
        /*httplib::MultipartFormDataItems items;
        httplib::MultipartFormData ss = {"toiletId", itId->first, "", ""};
        items.clear();
        items.push_back(ss);*/
        if (auto resPost = client_P->Get(("/api/admin/crm-toilet-section/get-pitList-status/?toiletId=" + itId->first).c_str(), headers))
        {
            if (resPost->status == 200)
            {
                cJSON *json = cJSON_Parse(UtfToGbk(resPost->body).c_str());
                cJSON *resultCode = cJSON_GetObjectItemCaseSensitive(json, "code");
                if (resultCode->valuedouble == 0)
                {
                    cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
                    cJSON *item = NULL;
                    cJSON *type = NULL;
                    cJSON *pitId = NULL;
                    cJSON *stateCd = NULL;
                    cJSON *sectionType = NULL;
                    cJSON *crmToiletSectionPitStateList = NULL;
                    cJSON *pitItem = NULL;

                    cJSON_ArrayForEach(item, data)
                    {
                        sectionType = cJSON_GetObjectItemCaseSensitive(item, "sectionType");
                        if (!strcmp(sectionType->valuestring, "MAN"))
                        {
                            auto typePt = itId->second.find("MAN");
                            if (typePt != itId->second.end())
                            {
                                crmToiletSectionPitStateList = cJSON_GetObjectItemCaseSensitive(item, "crmToiletSectionPitStateList");
                                cJSON_ArrayForEach(pitItem, crmToiletSectionPitStateList)
                                {
                                    pitId = cJSON_GetObjectItemCaseSensitive(pitItem, "sort");
                                    stateCd = cJSON_GetObjectItemCaseSensitive(pitItem, "status");
                                    auto pitPt = typePt->second.find(to_string(pitId->valueint));
                                    if (pitPt != typePt->second.end())
                                    {
                                        pitPt->second.value = stateCd->valuedouble;
                                    }
                                }
                            }
                        }
                        else if (!strcmp(sectionType->valuestring, "WOMAN"))
                        {
                            auto typePt = itId->second.find("WOMAN");
                            if (typePt != itId->second.end())
                            {
                                crmToiletSectionPitStateList = cJSON_GetObjectItemCaseSensitive(item, "crmToiletSectionPitStateList");
                                cJSON_ArrayForEach(pitItem, crmToiletSectionPitStateList)
                                {
                                    pitId = cJSON_GetObjectItemCaseSensitive(pitItem, "sort");
                                    stateCd = cJSON_GetObjectItemCaseSensitive(pitItem, "status");
                                    auto pitPt = typePt->second.find(to_string(pitId->valueint));
                                    if (pitPt != typePt->second.end())
                                    {
                                        pitPt->second.value = stateCd->valuedouble;
                                    }
                                }
                            }
                        }
                        /*
                        type = cJSON_GetObjectItemCaseSensitive(item, "MAN");
                        auto typePt = itId->second.find("MAN");
                        if (typePt != itId->second.end())
                        {

                            pitId = cJSON_GetObjectItemCaseSensitive(item, "pitId");
                            stateCd = cJSON_GetObjectItemCaseSensitive(item, "stateCd");

                            auto pitPt = typePt->second.find(pitId->valuestring);
                            if (pitPt != typePt->second.end())
                            {
                                pitPt->second.value = stateCd->valuedouble;
                            }
                        }*/
                    }
                    cJSON_Delete(json);
                }
                else
                {
                    cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg");
                    memset(iot_logbuff, 0, sizeof(iot_logbuff));
                    sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口返回错误！ status[%d] msg[%s] line[%d]!", resPost->status, msg->valuestring, __LINE__);
                    FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                    cJSON_Delete(json);
                    return false;
                }
            }
            else
            {
                memset(iot_logbuff, 0, sizeof(iot_logbuff));
                sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接状态错误！ status[%d] line[%d]!", resPost->status, __LINE__);
                FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
                return false;
            }
        }
        else
        {
            memset(iot_logbuff, 0, sizeof(iot_logbuff));
            sprintf(iot_logbuff, "[TOILET] ERROR Query() 光明源智慧公厕接口连接失败！ addr[/api/admin/crm-toilet-section/get-pitList-status] line[%d]!", __LINE__);
            FepLog::writelog((char *)_groupName, (char *)_linkName, iot_logbuff, FEP_LOG::PKGDATA);
            return false;
        }
    }
    return true;
}