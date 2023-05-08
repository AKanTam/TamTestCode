#include "httpclinet.h"

using namespace std;
using namespace ECON::FDC;

char global_logbuff_ent[1024];

string httpClinet::iniHttp(string address) //初始化http并获取设备列表
{
    // GET方法读取设备列表

    httplib::Client cli(address); //创建客户端对象

    if (auto res = cli.Get("/api/device/list"))
    {
        cJSON *json = cJSON_Parse(res->body.c_str());
        cJSON *root = cJSON_CreateObject();
        cJSON *json_data = NULL;
        cJSON *item = NULL;

        json_data = cJSON_GetObjectItem(json, "data");
        if (cJSON_IsArray(json_data))
        {
            cJSON_ArrayForEach(item, json_data)
            {
            }
        }
        cJSON_AddItemToObject(root, "dev", json_data);
        char *str = cJSON_Print(root);

        cJSON *json1 = cJSON_Parse(str);

        string buf = str;

        cJSON *json2 = cJSON_Parse(buf.c_str());

        memset(global_logbuff_ent, 0, sizeof(global_logbuff_ent));
        sprintf(global_logbuff_ent, "[iniHttp] GET成功 ");
        FepLog::writelog("(char*)_groupName", "(char*)_linkName", global_logbuff_ent, FEP_LOG::PKGDATA);

        return buf;
    }
    else
    {
        return "";
    }

    return "";
}

string httpClinet::UtfToGbk(string strValue)
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
    return string(str);
}

string httpClinet::MyPost(string sn, string address)
{

    httplib::Client cli(address); //创建客户端对象

    string string1 = "{\"sn\": \"";
    string string2 = "\"}";
    string body = string1 + sn + string2;

    auto res = cli.Post("/api/device/info", body, "application/json");

    return UtfToGbk(res->body);
}

DEVYONGJI httpClinet::processData(const char *body)
{
    DEVYONGJI temp;

    string tt;

    cJSON *cBody = cJSON_Parse(body);
    cJSON *item;
    cJSON *data = cJSON_GetObjectItem(cBody, "data");
    cJSON *status = cJSON_GetObjectItem(cBody, "status");
    cJSON *ports = cJSON_GetObjectItem(data, "ports");
    cJSON *acdc = cJSON_GetObjectItem(ports, "acdc");

    tt = cJSON_Print(status);
    if (tt.find("true") != string::npos)
    {
        temp.m_isOnline = 0;
    }
    else
    {
        temp.m_alarm = 1;
    }

    item = cJSON_GetObjectItem(data, "door_alarm");
    tt = cJSON_GetStringValue(item);
    if (tt.find("关") != string::npos)
    {
        temp.m_alarm = 0;
    }
    else
    {
        temp.m_alarm = 1;
    }

    item = cJSON_GetObjectItem(data, "door_status");
    tt = cJSON_GetStringValue(item);
    if (tt.find("关") != string::npos)
    {
        temp.m_doorStatus = 1;
    }
    else
    {
        temp.m_alarm = 0;
    }

    item = cJSON_GetObjectItem(data, "temp");
    if (item->type != cJSON_String)
    {
        temp.m_tempture = item->valuedouble;
    }
    if (item->type == cJSON_String)
    {
        temp.m_tempture = atof(cJSON_GetStringValue(item));
    }

    item = cJSON_GetObjectItem(data, "humi");
    if (item->type != cJSON_String)
    {
        temp.m_humidity = item->valuedouble;
    }
    if (item->type == cJSON_String)
    {
        temp.m_humidity = atof(cJSON_GetStringValue(item));
    }

    cJSON *iitem = NULL;
    cJSON *array = cJSON_GetObjectItem(acdc, "1");
    if (!array)
    {
        cJSON *array = cJSON_GetObjectItem(acdc, "71");
    }
    if (!array)
    {
        temp.m_switchCurrent = 0;
        temp.m_switchPower = 0;
        temp.m_switchStatus = 0;
        temp.m_switchVoltage = 0;
        temp.m_yuntaiCurrent = 0;
        temp.m_yuntaiPower = 0;
        temp.m_yuntaiStatus = 0;
        temp.m_yuntaiVoltage = 0;
        temp.m_cameraCurrent = 0;
        temp.m_cameraPower = 0;
        temp.m_cameraStatus = 0;
        temp.m_cameraVoltage = 0;
    }
    else
    {

        for (int y = 0; y < 3; y++)
        {
            string index1 = to_string(y);
            cJSON *array1 = cJSON_GetObjectItem(array, index1.c_str());

            if (!array1)
            {
                break;
            }
            else
            {

                iitem = cJSON_GetObjectItem(array1, "ext_device_type");
                string devType = cJSON_GetStringValue(iitem);
                if (devType.find("XPPN-9000") != string::npos)
                {
                    iitem = cJSON_GetObjectItem(array1, "on_off");
                    temp.m_switchStatus = iitem->valuedouble;
                    temp.m_switchStatus = !temp.m_switchStatus;
                    iitem = cJSON_GetObjectItem(array1, "current");
                    temp.m_switchCurrent = iitem->valuedouble;
                    iitem = cJSON_GetObjectItem(array1, "voltage");
                    temp.m_switchVoltage = atof(cJSON_GetStringValue(iitem));
                    temp.m_switchPower = ((temp.m_switchCurrent) / 1000) * (temp.m_switchVoltage);
                }

                if (devType.find("枪机") != string::npos)
                {
                    iitem = cJSON_GetObjectItem(array1, "on_off");
                    temp.m_cameraStatus = iitem->valuedouble;
                    temp.m_cameraStatus = !temp.m_cameraStatus;
                    iitem = cJSON_GetObjectItem(array1, "current");
                    temp.m_cameraCurrent = iitem->valuedouble;
                    iitem = cJSON_GetObjectItem(array1, "voltage");
                    temp.m_cameraVoltage = atof(cJSON_GetStringValue(iitem));
                    temp.m_cameraPower = ((temp.m_cameraCurrent) / 1000) * (temp.m_cameraVoltage);
                }

                if (devType.find("云台") != string::npos)
                {
                    iitem = cJSON_GetObjectItem(array1, "on_off");
                    temp.m_yuntaiStatus = iitem->valuedouble;
                    temp.m_yuntaiStatus = !temp.m_yuntaiStatus;
                    iitem = cJSON_GetObjectItem(array1, "current");
                    temp.m_yuntaiCurrent = iitem->valuedouble;
                    iitem = cJSON_GetObjectItem(array1, "voltage");
                    temp.m_yuntaiVoltage = atof(cJSON_GetStringValue(iitem));
                    temp.m_yuntaiPower = ((temp.m_yuntaiCurrent) / 1000) * (temp.m_yuntaiVoltage);
                }
            }
        }
    }

    temp.m_totalPower = temp.m_cameraPower + temp.m_switchPower + temp.m_yuntaiPower;

    free(cBody);

    return temp;
}

bool httpClinet::getDevList(const char *bufdev, list<string> &dev_list)
{
    if (dev_list.empty())
        return false;

    cJSON *json = cJSON_Parse(bufdev);
    cJSON *root = cJSON_CreateObject();
    cJSON *dev = NULL;
    cJSON *temp = 0;

    dev = cJSON_GetObjectItem(json, "dev");
    const char *s = cJSON_Print(dev);

    int arry_size = cJSON_GetArraySize(dev);
    dev_list.clear();

    for (int i = 0; i < arry_size; i++)
    {
        temp = cJSON_GetArrayItem(dev, i);
        string strtemp = cJSON_GetStringValue(temp);
        dev_list.push_back(strtemp);
    }

    return true;
}

bool httpClinet::postHttp(list<string> dev_list, map<string, DEVYONGJI> &devdatamap, string address)
{
    devdatamap.clear();

    for (list<string>::iterator iter = dev_list.begin(); iter != dev_list.end(); iter++)
    {

        devdatamap.insert(pair<string, DEVYONGJI>(*iter, processData(MyPost(*iter, address).c_str())));
    }

    return true;
}

bool httpClinet::getPointData(map<string, DEVYONGJI> devdatamap, list<TAGINFO> &_datalist, map<string, TAGINFO> _readTagMap)
{
    _datalist.clear();
    for (map<string, TAGINFO>::iterator itor = _readTagMap.begin(); itor != _readTagMap.end(); itor++)
    {

        TAGINFO tagTemp;

        tagTemp.pointName = itor->second.pointName.c_str();

        if (itor->second.dev.find("TXZT") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_isOnline;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("WD") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_tempture;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("SD") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_humidity;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("KGFK") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_doorStatus;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("KXBJ") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_alarm;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("GL") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_totalPower;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("JHJDL") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchCurrent;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("JHJDY") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchVoltage;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("JHJGL") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchPower;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("JHJGZZT") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchStatus;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("SXJDL") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchCurrent;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("SXJDY") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchVoltage;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("SXJGL") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchPower;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("SXJGZZT") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchStatus;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("YTDL") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchCurrent;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("YTDY") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchVoltage;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("YTGL") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchPower;
            tagTemp.quality = 129;
        }

        else if (itor->second.dev.find("YTGZZT") != string::npos)
        {
            tagTemp.value = devdatamap[itor->second.name].m_switchStatus;
            tagTemp.quality = 129;
        }

        _datalist.push_back(tagTemp);
    }
}
