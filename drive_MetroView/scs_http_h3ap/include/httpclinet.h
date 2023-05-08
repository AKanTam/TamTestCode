
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

    bool iniHttp(string address,map<string, bool>&_devOnlineMap);
    bool getPointData(map<string, bool>devdatamap,list<TAGINFO>& _datalist,map<string, TAGINFO> _readTagMap);
};



#endif
