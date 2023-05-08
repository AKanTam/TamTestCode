/*
 * Filename: mqttclient.cpp
 * Author: TQ
 * Brief: MQTT
 * Version: V1.0.0
 * Time: 2022-3-14
 * TODO:
 * Attention:
 *     1）所有json对象类型均转为string处理
 */

#include "mqttclient.h"

extern map<string, map<string, string>> m_subDataMap;
static list<string> sub_list;
struct mosquitto *mosq = NULL;

static auto cutPre(string stream, const string &str)
{
    int nPos = stream.find(str);
    if (nPos != -1)
    {
        stream = stream.substr(0, nPos);
    }
    return stream;
}
static auto cutNext(string stream, const string &str)
{
    int nPos = stream.find(str);

    if (nPos != -1)
    {
        stream = stream.substr(nPos + str.size(), stream.size());
    }
    return stream;
}

void sub_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    const char *mess = (const char *)message->payload;
    cJSON *root = cJSON_Parse(mess);
    cJSON *item = NULL;

    if (root == NULL)
    {
        return;
    }

    cJSON_ArrayForEach(item, root)
    {
        // switch (item->type & 0xFF)
        // {
        // case cJSON_String:
        //     if (item->valuestring ! = NULL)
        //     {
        //         m_subDataMap[message->topic][item->string] = item->valuestring;
        //     }
        //     break;
        // case cJSON_Number:
        //     m_subDataMap[message->topic][item->string] = std::to_string(item->valuedouble);
        //     break;
        // case cJSON_False:
        //     m_subDataMap[message->topic][item->string] = "0";
        //     break;
        // case cJSON_True:
        //     m_subDataMap[message->topic][item->string] = "1";
        //     break;

        // default:
        //     return;
        //     break;
        // }

        if (cJSON_IsString(item) && (item->valuestring != NULL))
        {
            m_subDataMap[message->topic][item->string] = item->valuestring;
        }
        else if (cJSON_IsNumber(item))
        {
            m_subDataMap[message->topic][item->string] = std::to_string(item->valuedouble);
        }
        else if (cJSON_IsFalse(item))
        {
            m_subDataMap[message->topic][item->string] = "0";
        }
        else if (cJSON_IsTrue(item))
        {
            m_subDataMap[message->topic][item->string] = "1";
        }
    }

    cJSON_Delete(root);
    return;
}

bool mqttClient::iniMqtt(string address, map<string, TAGINFO> readMap)
{
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    string add = cutPre(address, ":");
    string port = cutNext(address, ":");
    if (mosquitto_connect(mosq, add.c_str(), atoi(port.c_str()), 60) == MOSQ_ERR_SUCCESS)
    {
        mosquitto_message_callback_set(mosq, sub_callback);

        sub_list.clear();

        for (map<string, TAGINFO>::iterator iter = readMap.begin(); iter != readMap.end(); iter++)
        {
            sub_list.push_back(iter->second.dev);
        }
        sub_list.sort();
        sub_list.unique();
        for (list<string>::iterator iter = sub_list.begin(); iter != sub_list.end(); iter++)
        {
            mosquitto_subscribe(mosq, NULL, (*iter).c_str(), 2);
        }
        // mosquitto_subscribe(mosq, NULL, "home/+", 2);
        return true;
    }
    else
    {
        return false;
    }
}

void mqttClient::closeMqtt(void)
{
    mosquitto_disconnect(mosq);
}

void mqttClient::publicControl(string topic, string object_name, string value)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, object_name.c_str(), value.c_str());
    const char *printtbuf = cJSON_Print(root);

    mosquitto_publish(mosq, NULL, topic.c_str(), (int)strlen(printtbuf), printtbuf, 2, 0);

    cJSON_Delete(root);
}

bool mqttClient::subPoll()
{
    if (sub_list.empty())
    {
        return false;
    }

    for (int i = 0; i < sub_list.size(); i++)
    {
        mosquitto_loop(mosq, -1, 1);
    }

    return true;
}
