
#ifndef _SCS_HTTP_JXSERVER_LIGHT_
#define _SCS_HTTP_JXSERVER_LIGHT_

#include "common.h"
#include "CTcpClient.h"
#include "dbconnect.h"

namespace ECON
{
	namespace FDC
	{

		class TCP_CGL_BSD_EXPORT SCS_TCP_CGL_BSD : public CProtocol
		{

		public:
			SCS_TCP_CGL_BSD();
			virtual ~SCS_TCP_CGL_BSD();

		public:
			bool isOpen() const; //??????
			bool open();		 //??????
			void close();		 //??????
			void run();			 //???????????????
		private:
			bool loadTag(map<string, TAGINFO> *_readMap, map<string, TAGINFO> *_writeMap);
			bool readProtocolParam(string _param);
			bool createReadTask();
			void procData();
			bool sendCtrlCmd();
			void saveFEPData(const TAGINFO &_tag);
			void saveYxData(const TAGINFO &_tag);
			void saveYcData(const TAGINFO &_tag);
			void saveKwhData(const TAGINFO &_tag);
			void saveStringData(const TAGINFO &_tag);

			void controlSend(string _id, int _val);

		private:
			//????????
			CCtrlInf m_ctrlInf; // FEP?????????????
			bool m_isOpen;		//??????????
			bool m_isload;		//?????????????????

			hUInt32 m_logd_id_;				   // logd?????
			hChar m_log_buf_[BUFFER_LOG_SIZE]; //?????????????

			map<string, TAGINFO> m_readTagMap;
			map<string, TAGINFO> m_writeTagMap;
			std::multimap<std::string, TAGINFO> dataMap;

			MYSQL m_mysql;

			hInt16 m_poll_response_time_; //????????????100ms???????????????????????????
			hInt16 m_poll_tag_num_;		  //??????????
			hInt8 m_resend_count_;		  //?????????????????3
			hInt8 m_log_level_;			  //????????
			bool m_send_flag;
			timeval m_send_time_; //????????????????

			hInt8 m_try_send_times_;
			hInt8 m_current_task;
			int m_recv_len;
			bool m_recv_head;
			int m_json_len;
			std::string m_remoteAddress;
			std::string m_add;
			std::string m_port;

			CTcpClient *tcpClient = NULL;

			hUChar m_recv_data_buffer_[BUFFER_SIZE];
			hUChar m_send_data_buffer_[SEND_BUFFER_SIZE];
			const char *listBuf;
			hUChar cmdBuf[FDC_CTRL_LEN];
		};

	}

}

#endif
