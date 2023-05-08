#pragma once

#include <map>
#include <string>
#include <list>
#include <stdlib.h>
#include <vector>
#include <math.h>

using namespace std;

#include "fdc/protocol.h"
#include "fdc/ctrlinf.h"
#include "fdc/datainf.h"
#include "fdc/fdcdef.h"

#include "datatype.h"

//--fep��־��--
#include "fep_log.h"
using namespace FEP_LOG;

//--���ݿ�ӿ�---
//#include "rdbop.h"
//#include <mysql.h>
#include "mysql.h"

#include "eventapi.h"
#include "sys_utl.h"
#include "eventtype.h"
#include "utl/econmutex.h"
#include "utl/intervaltimerset.h" //��ʱ��
extern "C"
{
#include "cJSON.h"
}

#ifdef WIN32
#if defined(_WS_CENTER_SUPCON_)
#define WS_CENTER_SUPCON_EXPORT __declspec(dllexport)
#else
#define WS_CENTER_SUPCON_EXPORT __declspec(dllimport)
#endif // _MODBUS_TCP_
#else
#define WS_CENTER_SUPCON_EXPORT
#endif

#define TAG_TYPE_YX 0
#define TAG_TYPE_YC 1
#define TAG_TYPE_KWH 2
#define TAG_TYPE_STRING 4

#define BUFSIZE 10240

#define BUFFER_SIZE 102400
#define RECV_BUFFER_SIZE 10240
#define SEND_BUFFER_SIZE 10240
#define BUFFER_LOG_SIZE 102400

#define OPEN_FAILED_SLEEP_TIME 1 //�룬��open()�ӿ�ʧ��ʱ����
#define SLEEP_TIME 50000		 //΢�� ��run()�ӿ��������

#define CALL_DATA_TIME_SEC (60 * 5) //��
#define CALL_DATA_TIME(a, b) ((abs((int)((hUInt32)(a) - (hUInt32)(b)))) > CALL_DATA_TIME_SEC)
//���Ʒ���״̬
#define CTRL_STATUS_NO_ACK 999 //Լ��ֵ ��ӿ����޷���status//add 2018/11/19 zhengqiyu
//��־�ȼ�
#define LOG_0_NONE 0
#define LOG_1_NONE 1
#define LOG_2_NONE 2
#define LOG_3_NONE 3
#define LOG_4_NONE 4
#define LOG_5_NONE 5
enum VALUTYPE
{
	ANALOG,
	LOGICAL,
	TEXT
};
enum OPC_PARAM
{
	PARAM_TAG_NUM = 1,		 //����������
	PARAM_RESEND = 2,		 //�ط�����
	PARAM_RESPONED_TIME = 3, //��Ӧʱ��
	PARAM_LOG_LEVEL = 4,	 //��Ӧʱ��
};
typedef struct tag
{
	hUInt32 offset;
	int valuetype;
	string param2;
	string param1;
	double value;
	string value_s;
	string tagName;
	hInt32 maxValue;	//���ֵ
	hInt32 minValue;	//��Сֵ
	hFloat valueRadio;	//����ϵ��
	hInt32 valueOffset; //ƫ��
	hInt32 deaband;		//����

	hUInt32 quality;
	hUInt32 updateTime;
	tag()
	{
		offset = 0;
		maxValue = 0;
		minValue = 0;
		valueRadio = 1.0;
		valueOffset = 0;
		deaband = 0;
		value = 0;
		value_s = "";
		param2 = "";
		param1 = "";
		quality = 0;
		updateTime = 0;
		tagName = "";
	}

} TAGINFO;

typedef struct task
{
	int taskNo;
	int tagStartNo;
	int tagNum;
	task()
	{
		taskNo = 0;
		tagStartNo = 0;
		tagNum = 0;
	}
} TASK;
