#ifndef _MQTT_CLINET_
#define _MQTT_CLINET_


#include<stdio.h>
#include<string.h>
#include "common.h"
extern "C" {
	#include "mosquitto.h"
}

class  mqttClient
{
private:
    /* data */

    
public:
    //  httpClinet(/* args */);
    // ~ httpClinet();

    bool iniMqtt(string address, map<string, TAGINFO> readMap);
    void closeMqtt(void);
    void publicControl(string topic,string object_name,string value);
    bool subPoll();

};




#endif