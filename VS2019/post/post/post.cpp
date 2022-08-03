#include <iostream>
#include <string>
#include <list>
#include <map>

#include "httplib.h"
extern "C" {
#include "cJSON.h"
}

using namespace std;

httplib::Client cli("33.65.132.229");

httplib::Headers headers = {
  { "Accept-Encoding", "gzip, deflate,br" },
  { "Accept-Language", "zh-CN" },
  { "Accept", "*/*"},
  { "User-Agent","ApiPOST Runtime +https://www.apipost.cn"},
  { "Content-Type","application/json"},
    {"Content-Length","40"},
    {"Origin","http://33.65.132.229"},
    {"Connection","keep-alive"}
};


typedef struct devYongJi
{
    string m_isOnline;//在线状态  
    double m_tempture;//主板温度
    double m_humidity;//箱内湿度
    string m_doorStatus;//箱门开关状态
    string m_alarm;//开箱报警
    double m_totalPower;//总功率
    double m_switchCurrent;
    double m_switchVoltage;
    double m_switchPower;
    double m_switchStatus;
    double m_cameraCurrent;
    double m_cameraVoltage;
    double m_cameraPower;
    double m_cameraStatus;
    double m_yuntaiCurrent;
    double m_yuntaiVoltage;
    double m_yuntaiPower;
    double m_yuntaiStatus;

    bool m_false;
    
} DEVYONGJI;


string iniHttp()//初始化http并获取设备列表
{
    //httplib::Server ser;//创建服务端对象

     //GET方法读取设备列表
   
    if (auto res = cli.Get("/api/device/list"))
    {
        if (res->status == 200)
        {
            cJSON* json = cJSON_Parse(res->body.c_str());//将body字符串解析为结构体
            cJSON* root = cJSON_CreateObject();
            cJSON* json_data = NULL;

            if (!json)
            {
                free(json);
            }

            json_data = cJSON_GetObjectItem(json, "data");
            cJSON_AddItemToObject(root, "dev", json_data);
            char* str = cJSON_Print(root);

            //cJSON* json1 = cJSON_Parse(str);

            string buf = str;

            //cJSON* json2 = cJSON_Parse(buf.c_str());

            //free(json1);
            //free(json2);
            free(json);
            free(root);

            return buf;

        }

    }
    else
    {
        auto err = res.error();
        return "";
    }

    return "";
}


string UtfToGbk(string strValue)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, strValue.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, strValue.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    if (wstr) delete[] wstr;
    return string(str);
}


string MyPost(string sn)
{
        string string1 = "{\"sn\": \"";
        string string2 = "\"}";
        string body = string1 + sn + string2;

        auto res = cli.Post("/api/device/info" ,body, "application/json");

        cout << UtfToGbk(res->body) << endl;

        return  res->body; 
}


DEVYONGJI processData(const char* body) 
{
    DEVYONGJI temp;

    cJSON* cBody = cJSON_Parse(body);
    cJSON* item;
    cJSON* data = cJSON_GetObjectItem(cBody, "data");
    cJSON* status = cJSON_GetObjectItem(cBody, "status");
    cJSON* ports = cJSON_GetObjectItem(data, "ports");
    cJSON* acdc = cJSON_GetObjectItem(ports, "acdc");

    temp.m_false = 0;

        //char* s = cJSON_Print(cBody);
        //cout << s << endl;

        temp.m_isOnline = cJSON_Print(status);

        item = cJSON_GetObjectItem(data, "door_alarm");
        temp.m_alarm = cJSON_GetStringValue(item);

        item = cJSON_GetObjectItem(data, "door_status");
        temp.m_doorStatus = cJSON_GetStringValue(item);

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

            cJSON* iitem;
            cJSON* array = cJSON_GetObjectItem(acdc,"1");
            if (!array)
            {
                cJSON* array = cJSON_GetObjectItem(acdc, "71");
                if (!array) return temp; 
            }

                for (int y = 0; y < 3; y++)
                {
                    string index1 = to_string(y);
                    cJSON* array1 = cJSON_GetObjectItem(array, index1.c_str());

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
                            iitem = cJSON_GetObjectItem(array1, "current");
                            temp.m_yuntaiCurrent = iitem->valuedouble;
                            iitem = cJSON_GetObjectItem(array1, "voltage");
                            temp.m_yuntaiVoltage = atof(cJSON_GetStringValue(iitem));
                            temp.m_yuntaiPower = ((temp.m_yuntaiCurrent) / 1000) * (temp.m_yuntaiVoltage);
                        }
                    }

                }
            

        temp.m_totalPower = temp.m_cameraPower+temp.m_switchPower+temp.m_yuntaiPower;

    free(cBody);

    return temp;

}

bool getDevList(const char* bufdev, list<string>&dev_list)
{
    cJSON* json = cJSON_Parse(bufdev);
    cJSON* root = cJSON_CreateObject();
    cJSON* dev = NULL;
    cJSON* temp = 0;

    dev = cJSON_GetObjectItem(json, "dev");
    const char* s = cJSON_Print(dev);

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

bool postHttp(list<string>dev_list, map<string, DEVYONGJI>&devdatamap)
{
    devdatamap.clear();

    int count = 0;

    for (list<string>::iterator iter = dev_list.begin(); iter != dev_list.end(); iter++)
    {
        
        devdatamap.insert(pair<string, DEVYONGJI>(*iter, processData(MyPost(*iter).c_str())));
        //cout << devdatamap[*iter].m_switchPower << endl;
        cout << devdatamap[*iter].m_doorStatus << endl;
        //cout << devdatamap[*iter].m_yuntaiPower << endl;
        //cout << devdatamap[*iter].m_cameraPower << endl;

        if (devdatamap[*iter].m_false) {
            iter++; }
        
        count++;
    }

    return true;
}


int main() {

    //const char* str = "{\"status\": true, 	\"err_msg\": \"\", 	\"data\": { 		\"device_id\": \"33\", 		\"device_sn\": \"31FFD4055359383016691851\", 		\"project_id\": \"1\", 		\"dept_id\": \"1\", 		\"door_alarm\": \"关\", 		\"door_status\": \"关\", 		\"remark\": \"RC7 中间K171+355\", 		\"longitude\": \"0.000000\", 		\"latitude\": \"0.000000\", 		\"update_time\": \"2021-11-17 13:29:45\", 		\"fan_open_temp\": \"50\", 		\"server_mode\": \"2\", 		\"server_ip\": \"33.65.133.238\", 		\"eth_config\": \"{\\\"sig\\\":4,\\\"applied\\\":1,\\\"dhcp\\\":0,\\\"ip\\\":\\\"33.65.133.159\\\",\\\"sub\\\":\\\"255.255.255.0\\\",\\\"gw\\\":\\\"33.65.133.254\\\",\\\"dns\\\":\\\"33.65.133.21\\\"}\", 		\"rsynced\": \"1\", 		\"project\": \"嘉绍大桥枪机\", 		\"dept_name\": \"嘉绍大桥项目部\", 		\"spd_list\": {}, 		\"location\": { 			\"longitude\": \"120.776685\", 			\"latitude\": \"30.221575\" 		}, 		\"iotcard\": { 			\"iccid\": \"89860411101871386777\", 			\"combo_type\": \"\", 			\"flow_total\": \"0.0000\", 			\"flow_used\": \"0.0000\", 			\"flow_remain\": \"0.0000\", 			\"expire_date\": \"0000-00-00 00:00:00\", 			\"working_condition\": \"\", 			\"provider\": \"\", 			\"agency_code\": \"\", 			\"online_status\": \"0\", 			\"update_time\": \"2020-11-21 15:16:20\" 		}, 		\"temp\": \"26.7\", 		\"humi\": \"44.0\", 		\"ports\": { 			\"acdc\": { 				\"0\": { 					\"0\": { 						\"ext_device_type\": \"照明灯带\", 						\"isac\": \"0\", 						\"on_off\": 0, 						\"port\": \"1\", 						\"port_type\": 7, 						\"current\": 0, 						\"voltage\": \"0.00\", 						\"update_time\": \"2021-11-17 13:29:14\", 						\"set_button\": 0 					}, 					\"1\": { 						\"ext_device_type\": \"散热风扇\", 						\"isac\": \"0\", 						\"on_off\": 0, 						\"port\": \"2\", 						\"port_type\": 8, 						\"current\": 0, 						\"voltage\": \"0.00\", 						\"update_time\": \"2021-11-17 13:29:14\", 						\"set_button\": 0 					}, 					\"2\": { 						\"ext_device_type\": \"\", 						\"isac\": \"0\", 						\"on_off\": 1, 						\"port\": \"3\", 						\"port_type\": 1, 						\"current\": 0, 						\"voltage\": \"12.11\", 						\"update_time\": \"2021-11-17 13:29:14\", 						\"set_button\": 0 					}, 					\"3\": { 						\"ext_device_type\": \"\", 						\"isac\": \"0\", 						\"on_off\": 1, 						\"port\": \"4\", 						\"port_type\": 1, 						\"current\": 0, 						\"voltage\": \"12.11\", 						\"update_time\": \"2021-11-17 13:29:14\", 						\"set_button\": 0 					}, 					\"4\": { 						\"ext_device_type\": \"\", 						\"isac\": \"0\", 						\"on_off\": 1, 						\"port\": \"5\", 						\"port_type\": 1, 						\"current\": 0, 						\"voltage\": \"12.11\", 						\"update_time\": \"2021-11-17 13:29:14\", 						\"set_button\": 0 					}, 					\"5\": { 						\"ext_device_type\": \"\", 						\"isac\": \"0\", 						\"on_off\": 1, 						\"port\": \"6\", 						\"port_type\": 1, 						\"current\": 0, 						\"voltage\": \"12.08\", 						\"update_time\": \"2021-11-17 13:29:14\", 						\"set_button\": 0 					} 				}, 				\"1\": { 					\"0\": { 						\"ext_device_type\": \"交换机(XPPN-9000)\", 						\"isac\": \"1\", 						\"on_off\": 1, 						\"port\": \"1\", 						\"port_type\": 2, 						\"current\": 47, 						\"voltage\": \"221.80\", 						\"update_time\": \"2021-11-17 13:29:15\", 						\"set_button\": 0 					}, 					\"1\": { 						\"ext_device_type\": \"摄像机(枪机)\", 						\"isac\": \"1\", 						\"on_off\": 1, 						\"port\": \"2\", 						\"port_type\": 2, 						\"current\": 4, 						\"voltage\": \"226.48\", 						\"update_time\": \"2021-11-17 13:29:15\", 						\"set_button\": 0 					}, 					\"2\": { 						\"ext_device_type\": \"摄像机(云台)\", 						\"isac\": \"1\", 						\"on_off\": 1, 						\"port\": \"3\", 						\"port_type\": 2, 						\"current\": 246, 						\"voltage\": \"220.96\", 						\"update_time\": \"2021-11-17 13:29:15\", 						\"set_button\": 0 					} 				} 			} 		}, 		\"fan_ctrl_tmep\": \"50\", 		\"signal\": { 			\"sim\": \"0\", 			\"signal\": \"0\", 			\"update_time\": \"\" 		}, 		\"network\": \"未知\", 		\"power\": { 			\"ac\": \"正常\", 			\"dc\": \"正常\" 		}, 		\"netdetect\": {}, 		\"tunnel\": {}, 		\"ip\": \"33.65.133.159\" 	} }";
    //const char* str1 = "{\"dev\": [ 		\"31FFD0055359383011831851\", 		\"31FFD1055359383014752551\", 		\"31FFD1055359383020591451\", 		\"31FFD1055359383028641851\", 		\"31FFD2055359383011851851\", 		\"31FFD2055359383016701851\", 		\"31FFD2055359383020781451\", 		\"31FFD3055359383010641851\", 		\"31FFD3055359383010811851\", 		\"31FFD3055359383011861851\", 		\"31FFD3055359383013690151\", 		\"31FFD3055359383013700151\", 		\"31FFD3055359383024600151\", 		\"31FFD3055359383024660151\", 		\"31FFD3055359383026600151\", 		\"31FFD4054E42393909651943\", 		\"31FFD4054E42393909691943\", 		\"31FFD4055359383010741851\", 		\"31FFD4055359383010761851\", 		\"31FFD4055359383011671851\", 		\"31FFD4055359383011741851\", 		\"31FFD4055359383013860151\", 		\"31FFD4055359383016531851\", 		\"31FFD4055359383016551851\", 		\"31FFD4055359383016611851\", 		\"31FFD4055359383016681851\", 		\"31FFD4055359383016691851\", 		\"31FFD4055359383023730151\", 		\"31FFD4055359383024540151\", 		\"31FFD4055359383024800151\", 		\"31FFD4055359383026500151\", 		\"31FFD5055359383010611851\", 		\"31FFD5055359383011661851\", 		\"31FFD5055359383013660151\", 		\"31FFD5055359383014772551\", 		\"31FFD5055359383015541851\", 		\"31FFD5055359383020761551\", 		\"31FFD5055359383023590151\", 		\"31FFD5055359383023620151\", 		\"31FFD5055359383023700151\", 		\"31FFD5055359383023820151\", 		\"31FFD5055359383024710151\", 		\"31FFD5055359383024780151\", 		\"31FFD5055359383024790151\", 		\"31FFD5055359383025510151\", 		\"31FFD5055359383027551851\", 		\"31FFD5055359383030570151\", 		\"31FFD6054E42393907591943\", 		\"31FFD6055359383010651851\", 		\"31FFD6055359383010661851\", 		\"31FFD6055359383010671851\", 		\"31FFD6055359383010701851\", 		\"31FFD6055359383010831851\", 		\"31FFD6055359383010861851\", 		\"31FFD6055359383013650151\", 		\"31FFD6055359383015521851\", 		\"31FFD6055359383015561851\", 		\"31FFD6055359383016521851\", 		\"31FFD6055359383016631851\", 		\"31FFD6055359383016651851\", 		\"31FFD6055359383016671851\", 		\"31FFD6055359383016731851\", 		\"31FFD6055359383023800151\", 		\"31FFD6055359383024610151\", 		\"31FFD6055359383024640151\", 		\"31FFD6055359383024670151\", 		\"31FFD6055359383024740151\", 		\"31FFD6055359383024770151\", 		\"31FFD6055359383027571851\", 		\"31FFD6055359383028741851\", 		\"31FFD6055359383030560151\", 		\"31FFD6055359383030580151\", 		\"31FFD7055359383010601851\", 		\"31FFD7055359383010631851\", 		\"31FFD7055359383010721851\", 		\"31FFD7055359383010841851\", 		\"31FFD7055359383011701851\", 		\"31FFD7055359383011721851\", 		\"31FFD7055359383011791851\", 		\"31FFD7055359383011831451\", 		\"31FFD7055359383016541851\", 		\"31FFD7055359383016751851\", 		\"31FFD7055359383020761451\", 		\"31FFD7055359383020791551\", 		\"31FFD7055359383024580151\", 		\"31FFD7055359383024630151\", 		\"31FFD7055359383024680151\", 		\"31FFD7055359383026530151\", 		\"31FFD7055359383026590151\", 		\"31FFD7055359383028701851\", 		\"31FFD7055359383030540151\", 		\"31FFD7055359383030550151\", 		\"31FFD8054E42393907701943\", 		\"31FFD8055359383010751851\", 		\"31FFD8055359383013620151\", 		\"31FFD8055359383013670151\", 		\"31FFD8055359383015551851\", 		\"31FFD8055359383016561851\", 		\"31FFD8055359383016571851\", 		\"31FFD8055359383016711851\", 		\"31FFD8055359383016761851\", 		\"31FFD8055359383016771851\", 		\"31FFD8055359383020821551\", 		\"31FFD8055359383023540151\", 		\"31FFD8055359383023560151\", 		\"31FFD8055359383023570151\", 		\"31FFD8055359383023720151\", 		\"31FFD8055359383023810151\", 		\"31FFD8055359383024650151\", 		\"31FFD8055359383024750151\", 		\"31FFD8055359383026580151\", 		\"31FFD8055359383030520151\", 		\"31FFD9054E42393912851443\", 		\"31FFD9055359383010691851\", 		\"31FFD9055359383010731851\", 		\"31FFD9055359383010781851\", 		\"31FFD9055359383010851851\", 		\"31FFD9055359383011821851\", 		\"31FFD9055359383013600151\", 		\"31FFD9055359383013610151\", 		\"31FFD9055359383023500151\", 		\"31FFD9055359383023510151\", 		\"31FFD9055359383023780151\", 		\"31FFD9055359383024500151\", 		\"31FFD9055359383024700151\", 		\"31FFD9055359383024720151\", 		\"31FFD9055359383024730151\", 		\"31FFD9055359383024760151\", 		\"31FFD9055359383026510151\", 		\"31FFD9055359383027601851\", 		\"31FFD9055359383028721851\", 		\"31FFD9055359383030530151\", 		\"31FFDA054E42393905771943\", 		\"31FFDA055359383010681851\", 		\"31FFDA055359383010791851\", 		\"31FFDA055359383010801851\", 		\"31FFDA055359383013590151\", 		\"31FFDA055359383015531851\", 		\"31FFDA055359383016741851\", 		\"31FFDA055359383016781851\", 		\"31FFDA055359383018600151\", 		\"31FFDA055359383020871451\", 		\"31FFDA055359383023610151\", 		\"31FFDA055359383023710151\", 		\"31FFDA055359383023740151\", 		\"31FFDA055359383023760151\", 		\"31FFDA055359383024570151\", 		\"31FFDA055359383024690151\", 		\"31FFDA055359383026610151\", 		\"31FFDA055359383030590151\", 		\"31FFDB055359383010621851\", 		\"31FFDB055359383023530151\", 		\"31FFDB055359383024560151\", 		\"31FFDC055359383010711851\", 		\"31FFDC055359383011871851\", 		\"31FFDC055359383013630151\", 		\"31FFDC055359383016721851\", 		\"31FFDC055359383024530151\", 		\"31FFDD055359383026560151\", 		\"31FFDE055359383011811851\", 		\"31FFE3055359383016581851\", 		\"32FFD6054E41393604760243\", 		\"38FFD6054E43303210620243\", 		\"38FFD7054E43303227710443\", 		\"38FFD7054E43303238750543\", 		\"38FFD8054E43303211560243\", 		\"38FFD8054E43303212710243\", 		\"38FFD8054E43303225680443\", 		\"38FFD8054E43303237780543\", 		\"38FFD9054E43303211640243\", 		\"38FFD9054E43303238640543\", 		\"38FFDA054E43303211660243\", 		\"38FFDA054E43303225630443\", 		\"38FFDB054E43303211710243\", 		\"38FFDC054E43303238620543\", 		\"38FFDD054E43303238720543\", 		\"38FFDE054E43303210560243\", 		\"39FFD1055359393325501851\", 		\"39FFD4055359393325752151\", 		\"39FFD4055359393334601651\", 		\"39FFD5055359393317552051\", 		\"39FFD5055359393325892151\", 		\"39FFD5055359393334681851\", 		\"39FFD5055359393337742051\", 		\"39FFD6055359393318672151\", 		\"39FFD6055359393326542151\", 		\"39FFD6055359393339821651\", 		\"39FFD6055359393340751851\", 		\"39FFD7055359393317602051\", 		\"39FFD7055359393333691651\", 		\"39FFD7055359393334621651\", 		\"39FFD8055359393324872151\", 		\"39FFD8055359393325592151\", 		\"39FFD8055359393329761851\", 		\"39FFD8055359393330652151\", 		\"39FFD8055359393333622051\", 		\"39FFD8055359393333782151\", 		\"39FFD8055359393338672051\", 		\"39FFD8055359393341661851\", 		\"39FFD9055359393316822051\", 		\"39FFD9055359393317902051\", 		\"39FFD9055359393321772051\", 		\"39FFDB055359393336602151\", 		\"39FFDE055359393333622151\" 	] }";
    //devYongJi yj = processData(str);
   
    const char* listBuf = iniHttp().c_str();

    list<string>dev_list; 
    getDevList(listBuf,dev_list);
    map<string, DEVYONGJI> devmap;
    postHttp(dev_list,devmap);

    cout << devmap.size() <<endl;

    getchar();

    return 0;
}