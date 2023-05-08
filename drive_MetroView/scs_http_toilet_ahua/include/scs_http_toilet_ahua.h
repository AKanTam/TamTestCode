
#ifndef _SCS_HTTP_TOILET_AHUA_
#define _SCS_HTTP_TOILET_AHUA_

#include "common.h"
#include "toilet.h"

#include "dbconnect.h"

namespace ECON
{
	namespace FDC
	{

		class HTTP_TOILET_AHUA_EXPORT SCS_HTTP_TOILET_AHUA : public CProtocol
		{

		public:
			SCS_HTTP_TOILET_AHUA();
			virtual ~SCS_HTTP_TOILET_AHUA();

		public:
			bool isOpen() const; //�Ƿ��
			bool open();		 //�򿪹�Լ
			void close();		 //�رչ�Լ
			void run();			 //���������
		private:
			bool loadTag(map<string, TAGINFO> *_readMap, map<string, TAGINFO> *_writeMap);
			bool readProtocolParam(string _param);
			bool createReadTask();
			bool checkSend();
			void Send_CallAllData();
			void procData();
			void saveYxData(TAGINFO _tag);
			void saveYcData(TAGINFO _tag);
			void saveKwhData(TAGINFO _tag);
			void saveStringData(TAGINFO _tag);

		private:
			//���ƽӿ�
			CCtrlInf m_ctrlInf; // FEP������ؽӿ�
			bool m_isOpen;		//��Լ�Ƿ��
			bool m_isload;		//�����Ƿ���ر�־

			hUInt32 m_logd_id_;				   // logd���
			hChar m_log_buf_[BUFFER_LOG_SIZE]; //��־������

			map<string, TAGINFO> m_readTagMap;
			map<string, TAGINFO> m_writeTagMap;
			std::map<std::string, std::map<std::string, TAGINFO>> m_idleMap;
			std::map<std::string, std::map<std::string, TAGINFO>> m_cleanMap;
			std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> m_enviMap;
			std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> m_energyMap;
			std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> m_pitMap;

			MYSQL m_mysql;
			TOILET *m_mytoilet;

			hInt16 m_poll_response_time_; //��Ӧʱ�䣬Ĭ��100ms�������ҿ�����Ӧ��ʱ��
			hInt16 m_poll_tag_num_;		  //�ְ�����
			hInt8 m_resend_count_;		  //�ط�������Ĭ��3
			hInt8 m_log_level_;			  //��־�ȼ�
			bool m_send_flag;
			timeval m_send_time_; //���ͱ������豸

			hInt8 m_try_send_times_;
			hInt8 m_current_task;
			int m_recv_len;
			bool m_recv_head;
			int m_json_len;
			std::string m_remoteAddress;

			hUChar m_recv_data_buffer_[BUFFER_SIZE];
			hUChar m_send_data_buffer_[SEND_BUFFER_SIZE];
			const char *listBuf;
			hUChar cmdBuf[FDC_CTRL_LEN];
		};

	}

}

#endif
