#include <iostream>
#include <string>

#include<cjson/cJSON.h>

using namespace std;


int main(){

      //const char* json_str = "{\"status\":\"200\",\"num\":\"520\",\"data\":[1,2,3]}";
      //cout << "%s" << json_str << endl;
    const char* json_str = "1";
      cJSON *json = cJSON_Parse(json_str);//将body字符串解析为结构体
      cJSON *json_status = NULL;
      cJSON *json_data = NULL;
      char* statusC[1000];

      if(json==NULL)
      {
	    cout << "[HTTP] ERROR iniHttp() json解析失败" << endl;
         cJSON_Delete(json);
      }
    //  json_status = cJSON_GetObjectItem(json, "status");
    //  if (cJSON_IsString(json_status) && (json_status->valuestring != NULL))
    //  statusC = json_status->valuestring;

    //  if(statusC !="true")
    //  {

	   // cout << "[HTTP] ERROR iniHttp() 接口连接失败" << endl;
    //     cJSON_Delete(json);
    //  }
    //  
    //  json_data=cJSON_GetObjectItem(json,"data");
    //  cJSON_AddItemToObject(json, "sn", json_data);
    //  char* str = cJSON_Print(json_data);

	   //cJSON_Delete(json);

       return 0;
}