#include "C:\\Users\\DELL\\Desktop\\MQTT-Client\\devel\\mosquitto.h"
#include<stdio.h>
#include<string.h>
#include<cjson/cJSON.h>
#include <windows.h>

#pragma comment(lib, "C:\\Users\\DELL\\Desktop\\MQTT-Client\\devel\\mosquitto.lib")


#define HOST "127.0.0.1"
#define PORT 1883
#define KEEP_ALIVE 60
#define IP "127.0.0.1"

int ss = 0;
char cd[200];


void callback(struct mosquitto* mosq, void* obj, const struct mosquitto_message* message) {

	message->payload;
	printf("%s:%s\n", message->topic,message->payload);
}

int main() {
		struct mosquitto* mosq = NULL;

		const char* buff = "hello world!";
		mosquitto_lib_init();


		while (1) {
			mosq = mosquitto_new(NULL, true, NULL);
			if (!mosq)
			{
				printf("create client failed..\n");
				mosquitto_lib_cleanup();
				return 0;
			}
			else
			{
				//printf("create client success! \n");
			}

			if (mosquitto_connect(mosq, HOST, PORT, KEEP_ALIVE)!=MOSQ_ERR_SUCCESS)
			{
				printf("Unable to connect.\n");
			}
			else
			{
				printf("Enable to connect.\n");
			}


			

			mosquitto_message_callback_set(mosq, callback);
			//mosquitto_publish(mosq, NULL, "home/garden/fountain", strlen(printtbuf), printtbuf, 2, 0);
			while (1) {
				for (int x = 1; x < 100; x++) {
					for (int i = 1; i < 24; i++) {
						cJSON* root = cJSON_CreateObject();

						cJSON_AddNumberToObject(root, "sd", 0);
						sprintf_s(cd, sizeof(cd), "82.%d", x);
						cJSON_AddNumberToObject(root, "current", atof(cd));
						const char* printtbuf = cJSON_Print(root);
						free(root);

						sprintf_s(cd, sizeof(cd), "192.168.1.%d_sub", i);
						mosquitto_publish(mosq, NULL, cd, strlen(printtbuf), printtbuf, 2, 0);
						mosquitto_loop(mosq, -1, 1);

						
					}
					Sleep(1000);
				}
				
			}

			//mosquitto_subscribe(mosq, NULL, "home/garden/+", 2);
			//mosquitto_loop_forever(mosq, -1, 1);
			/*for (int i = 0; i < 10; i++) {
				mosquitto_loop(mosq, -1, 1);
			}*/
			//free(root);
			mosquitto_disconnect(mosq);
			mosquitto_destroy(mosq);
			//mosquitto_lib_cleanup();
			//   if (mosquitto_loop_start(mosq) == MOSQ_ERR_NOT_SUPPORTED) {
			   //	printf("ssss");
			   //}
		}
	return 0;
}
