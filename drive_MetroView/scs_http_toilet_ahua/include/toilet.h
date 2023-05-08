#pragma once

#include "common.h"
#include "httplib.h"
#include <string>
#include <cstdlib>
#include <fstream>

class TOILET
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
    TOILET(std::string address);
    TOILET();
    ~TOILET();
    bool Login(const void *_groupName, hInt32 _groupNo, const void *_linkName);
    bool QueryClean(std::map<std::string, std::map<std::string, TAGINFO>> *_cleanMap, const void *_groupName, hInt32 _groupNo, const void *_linkName);
    bool QueryIdle(std::map<std::string, std::map<std::string, TAGINFO>> *_idleMap, const void *_groupName, hInt32 _groupNo, const void *_linkName);
    bool QueryEnvi(std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> *_enviMap, const void *_groupName, hInt32 _groupNo, const void *_linkName);
    bool QueryEnergy(std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> *_energyMap, const void *_groupName, hInt32 _groupNo, const void *_linkName);
    bool QueryPit(std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> *_pitMap, const void *_groupName, hInt32 _groupNo, const void *_linkName);
};
