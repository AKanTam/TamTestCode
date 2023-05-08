#pragma once
class NetworkMsg
{
protected:
	char* msg;
	int len;
public:
	NetworkMsg(void);
	NetworkMsg(const char* message, int length);
	~NetworkMsg(void);
	void setMsg(const char* message, int length);
	char* getMsg();                                                                                       
	int getLength();
};

