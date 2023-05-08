#pragma once

#include "common.h"
#include "httplib.h"
#include <string>
#include <cstdlib>
#include <fstream>

class IOT
{
private:
    httplib::Client *client_P = nullptr;
    std::string IOT_LoginChar;
    httplib::Headers headers = {
        {"Accept", "application/json"}, {"Content-Type", "application/json"}};

    std::string token;
    char *bodyChar = NULL;
    char *controlChar = NULL;
    /* data */
public:
    IOT(std::string address);
    IOT();
    ~IOT();
    bool Login(const void *_groupName, hInt32 _groupNo, const void *_linkName);
    bool Query(std::multimap<std::string, std::map<std::string, TAGINFO>> *_tagMap, const int startNum, const int tagNum, const void *_groupName, hInt32 _groupNo, const void *_linkName);
    bool Control(const std::string *deviceId, const std::string *deviceControl, const std::string *_value, const void *_groupName, hInt32 _groupNo, const void *_linkName);
    bool Control(const std::string *deviceId, const std::string *deviceControl, const double _value, const void *_groupName, hInt32 _groupNo, const void *_linkName);
};
