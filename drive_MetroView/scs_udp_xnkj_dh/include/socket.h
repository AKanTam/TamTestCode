#pragma once

#include "common.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <map>

#include <winsock2.h>
#include <windows.h>

using namespace std;
extern map<string, float> m_subDataMap;

void open_socket(string address);
void close_socket();
int udpXnkjDhSubPoll(map<string, float> *DataMap, const void *_groupName, hInt32 _groupNo, const void *_linkName);
