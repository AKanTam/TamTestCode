#define CPPHTTPLIB_OPENSSL_SUPPORT ;
#include <iostream>
#include <string>
#include <list>
#include <map>

#include "httplib.h"
extern "C" {
#include "cJSON.h"
}

using namespace std;


int main() {

	int tt;

	httplib::Client cli("172.16.100.100:8080");

	httplib::Headers headers = {
  { "Accept", "application/json"},{"Content-Type","application/json"}};

	cli.set_digest_auth("ADMIN", "admin@2021");

	auto res = cli.Get("/imcrs/wlan/apInfo/queryApBasicInfo", headers);

	cJSON* AP =cJSON_Parse(res->body.c_str());
	cJSON* apBasicInfo = cJSON_GetObjectItem(AP, "apBasicInfo");
	int arry_size = cJSON_GetArraySize(apBasicInfo);

	for (int i = 0; i < arry_size; i++)
	{
		//打印数组里的所有item
		cJSON* device = cJSON_GetArrayItem(apBasicInfo, i);
		cJSON* macAddress = cJSON_GetObjectItem(device,"macAddress");
		cJSON* onlineStatus = cJSON_GetObjectItem(device, "onlineStatus");

		bool status = cJSON_GetStringValue(onlineStatus);
		string mac = cJSON_GetStringValue(macAddress);
		mac.erase(2, 1);
		mac.erase(4, 1);
		mac.erase(6, 1);
		mac.erase(8, 1);
		mac.erase(1, 1);
		cout << mac <<"status:" << status << endl;
	}


	cin >> tt ;

	return 0;

}