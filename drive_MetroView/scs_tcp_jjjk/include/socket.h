#pragma once

#include "common.h"
#include <iostream>
#include <winsock.h>
#include <algorithm>
#include <string>
#include <sstream>

using namespace std;
extern map<string, float> m_subDataMap;

#define TOLOWER(p)                                           \
    {                                                        \
        transform(p.begin(), p.end(), p.begin(), ::tolower); \
    }

int tcpJJJKSubPoll(string address, map<string, float> *DataMap, const void *_groupName, hInt32 _groupNo, const void *_linkName);