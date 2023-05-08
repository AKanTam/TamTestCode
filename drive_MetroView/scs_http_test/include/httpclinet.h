
#ifndef _HTTP_CLINET_
#define _HTTP_CLINET_
#include "common.h"
#include "httplib.h"

class  httpClinet
{
private:
    /* data */

    
public:
    //  httpClinet(/* args */);
    // ~ httpClinet();

    string iniHttp(string address);
    string MyPost(string sn,string address);
    DEVYONGJI processData(const char* body) ;
    bool postHttp(list<string>dev_list, map<string, DEVYONGJI>& devdatamap , string address);
    bool getDevList(const char* bufdev, list<string>& dev_list);
    bool getPointData(map<string, DEVYONGJI>devdatamap,list<TAGINFO>& _datalist,map<string, TAGINFO> _readTagMap);
    string UtfToGbk(string strValue);

};



#endif
