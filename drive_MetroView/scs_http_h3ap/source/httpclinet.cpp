#include "httpclinet.h"

using namespace std;
using namespace ECON::FDC;

char global_logbuff_ent[1024];

bool httpClinet::iniHttp(string address, map<string, bool> &_devOnlineMap)
{

    httplib::Client cli(address);

    httplib::Headers headers = {
        {"Accept", "application/json"}, {"Content-Type", "application/json"}};

    cli.set_digest_auth("ADMIN", "admin@2021");

    if (auto res = cli.Get("/imcrs/wlan/apInfo/queryApBasicInfo", headers))
    {
        if (res->status == 200)
        {
            cJSON *json = cJSON_Parse(res->body.c_str());
            cJSON *apBasicInfo = cJSON_GetObjectItem(json, "apBasicInfo");
            int arry_size = cJSON_GetArraySize(apBasicInfo);
            _devOnlineMap.clear();

            if (cJSON_IsArray(apBasicInfo))
            {
                for (int i = 0; i < arry_size; i++)
                {
                    cJSON *device = cJSON_GetArrayItem(apBasicInfo, i);
                    cJSON *macAddress = cJSON_GetObjectItem(device, "macAddress");
                    cJSON *onlineStatus = cJSON_GetObjectItem(device, "onlineStatus");

                    if (cJSON_IsString(onlineStatus) && onlineStatus->valuestring != NULL)
                    {
                        bool status = atoi(onlineStatus->valuestring);
                        string mac = macAddress->valuestring;
                        mac.erase(2, 1);
                        mac.erase(4, 1);
                        mac.erase(6, 1);
                        mac.erase(8, 1);
                        mac.erase(10, 1);

                        _devOnlineMap.insert(pair<string, bool>(mac, status));
                    }
                }

                cJSON_Delete(json);

                memset(global_logbuff_ent, 0, sizeof(global_logbuff_ent));
                sprintf(global_logbuff_ent, "[iniHttp] GET获取设备数据成功 ");
                FepLog::writelog("(char*)_groupName", "(char*)_linkName", global_logbuff_ent, FEP_LOG::PKGDATA);
            }

            return true;
        }
    }
    else
    {
        auto err = res.error();

        memset(global_logbuff_ent, 0, sizeof(global_logbuff_ent));
        sprintf(global_logbuff_ent, "[iniHttp] 连接接口失败 :%s", err);
        FepLog::writelog("(char*)_groupName", "(char*)_linkName", global_logbuff_ent, FEP_LOG::PKGDATA);

        return false;
    }
}

bool httpClinet::getPointData(map<string, bool> devdatamap, list<TAGINFO> &_datalist, map<string, TAGINFO> _readTagMap)
{
    _datalist.clear();
    hUInt32 updatime = time(0);
    for (map<string, TAGINFO>::iterator itor = _readTagMap.begin(); itor != _readTagMap.end(); itor++)
    {
        TAGINFO tagTemp;

        tagTemp.pointName = itor->second.pointName.c_str();
        tagTemp.value = devdatamap[itor->second.tag1];
        tagTemp.quality = 129;
        tagTemp.updateTime = updatime;
    
        _datalist.push_back(tagTemp);
    }
}
