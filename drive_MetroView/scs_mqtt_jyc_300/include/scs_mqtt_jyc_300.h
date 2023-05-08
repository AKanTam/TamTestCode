
#ifndef _SCS_MQTT_JYC_300_
#define _SCS_MQTT_JYC_300_


#include "common.h"
#include "mqttclient.h"

#include "dbconnect.h"

namespace ECON {
	namespace FDC {

		class MQTT_JYC_300_EXPORT SCS_MQTT_JYC_300 :public CProtocol {

		public:
			SCS_MQTT_JYC_300();
			virtual ~SCS_MQTT_JYC_300();

		public:
			bool	isOpen()	const;								//是否打开				
			bool	open();										//打开规约				
			void	close();										//关闭规约			
			void	run();											//主处理过程
		private:
			bool loadTag(map<string, TAGINFO>*_readMap, map<string, TAGINFO> *_writeMap);
			bool    readProtocolParam(string _param);
			void	ProtocolTX();
			void	ProtocolRX();
			bool	checkSend();
			bool	sendCtrlCmd();
			void	Send_CallAllData();
			void	procData();
			void	saveYxData(TAGINFO _tag);
			void	saveYcData(TAGINFO _tag);
			void	saveKwhData(TAGINFO _tag);
			void	saveStringData(TAGINFO _tag);
		private:
		//控制接口
			CCtrlInf			m_ctrlInf;								//FEP控制相关接口		
			bool				m_isOpen;			//规约是否打开
			bool            	m_isload;			//配置是否加载标志
			bool                m_isdevlist;        //是否获取设备列表
			hUInt32				m_logd_id_;			//logd编号
			hChar				m_log_buf_[BUFFER_LOG_SIZE];		//日志缓冲区
			

			map<string, TAGINFO> m_readTagMap;
			map<string, TAGINFO> m_writeTagMap;

			MYSQL m_mysql;

			hInt16				m_poll_response_time_;				//响应时间，默认100ms，即厂家可以响应的时间 
			hInt16				m_poll_tag_num_;					//分包点数
			hInt8				m_resend_count_;			//重发次数，默认3
			hInt8 				m_log_level_;				//日志等级
			bool				m_send_flag;
			timeval				m_send_time_;				//发送报文至设备

			hInt8				m_try_send_times_;
			hInt8				m_current_task;
			int					m_recv_len;
			bool				m_recv_head;
			int					m_json_len;
			string              m_remoteAddress;
										
			hUChar				m_recv_data_buffer_[BUFFER_SIZE];
			hUChar				m_send_data_buffer_[SEND_BUFFER_SIZE];
			const char*         listBuf;
		    hUChar				m_dev_sn_buffer_[BUFFER_SIZE];//接受厂家设备序列号
			hUChar				cmdBuf[FDC_CTRL_LEN];
			mqttClient          m_mqttc;
		};

	}

}

#endif
