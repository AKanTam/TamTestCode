#ifndef _DBCONNECT_H_
#define _DBCONNECT_H_

#include "common.h"
//MYSQL global_mysql;
bool connectDB(MYSQL* _mysql);
bool getTag(MYSQL* _mysql,map<string, TAGINFO> *_tagMap, const void* _groupName, hInt32 _groupNo, const void* _linkName);
bool getCmd(MYSQL* _mysql, map<string, TAGINFO> *_tagMap, const void* _groupName, hInt32 _groupNo, const void* _linkName);
bool closeDB(MYSQL* _mysql, const void* _groupName, const void* _linkName);



#endif