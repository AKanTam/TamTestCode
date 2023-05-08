#include "StdAfx.h"
#include "NetworkMsg.h"


NetworkMsg::NetworkMsg(void)
{
	msg = new char[1];
	len = 0;
}

NetworkMsg::NetworkMsg(const char* message, int length)
{
	if(length <= 0){
		len = 0;
		return;
	}
	msg = new char[length];
	len = length;
	memcpy(msg, message, length);
}

NetworkMsg::~NetworkMsg(void)
{
	if (len > 0) {
		delete msg;
	}
}

void NetworkMsg::setMsg(const char* message, int length){
	if(length <= 0){
		if (len != 0) {
			delete msg;
		}
		len = 0;
		return;
	}
	delete msg;
	msg = new char[length];
	len = length;
	memcpy(msg, message, length);
}

char* NetworkMsg::getMsg(){
	if(len <= 0){
		return NULL;
	}
	char* retmsg = new char[len];
	memcpy(retmsg, msg, len);
	return retmsg;
}

int NetworkMsg::getLength(){
	return len;
}