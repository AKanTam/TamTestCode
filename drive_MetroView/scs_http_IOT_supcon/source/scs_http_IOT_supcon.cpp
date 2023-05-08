#include "scs_http_IOT_supcon.h"

using namespace ECON::FDC;

#define LOGE_HTTP_IOT_SUPCON 31005

std::string IOT_LoginChar;

map<string, float> m_subDataMap;

// gbk转tf8
#include "iconv.h"
int gbk_to_utf8(char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen)
{
	iconv_t cd;
	if ((cd = iconv_open("utf-8", "gbk")) == 0)
	{
		return -1;
	}
	memset(destbuf, 0, destlen);
	char **source = &sourcebuf;
	char **dest = &destbuf;
	if (-1 == iconv(cd, source, &sourcelen, dest, &destlen))
	{
		return -1;
	}
	iconv_close(cd);
	return 0;
}
// utf-8转bk
int utf8_to_gbk(char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen)
{
	iconv_t cd;
	if ((cd = iconv_open("gbk", "utf-8")) == 0)
	{
		return -1;
	}
	memset(destbuf, 0, destlen);
	char **source = &sourcebuf;
	char **dest = &destbuf;
	if (-1 == iconv(cd, source, &sourcelen, dest, &destlen))
	{
		return -1;
	}
	iconv_close(cd);
	return 0;
}

SCS_HTTP_IOT_SUPCON::SCS_HTTP_IOT_SUPCON() : m_logd_id_(LOGE_HTTP_IOT_SUPCON)
{
	m_isOpen = false;
	m_isload = false;
	m_poll_tag_num_ = 100;
	m_poll_response_time_ = 100;
	m_resend_count_ = 3;
	m_send_flag = false;
	m_try_send_times_ = 0;
	m_current_task = 0;
	m_recv_head = false;
	m_recv_len = 0;
	m_json_len = 0;
	memset(m_recv_data_buffer_, 0, sizeof(m_recv_data_buffer_));
	memset(m_send_data_buffer_, 0, sizeof(m_send_data_buffer_));
}

SCS_HTTP_IOT_SUPCON::~SCS_HTTP_IOT_SUPCON()
{
}

bool SCS_HTTP_IOT_SUPCON::isOpen() const
{
	return m_isOpen;
}
bool SCS_HTTP_IOT_SUPCON::open()
{
	m_isOpen = CProtocol::open();
	sprintf(m_log_buf_, "%s", m_pLink->remoteAddr);
	m_remoteAddress = m_log_buf_;
	m_myiot = new IOT(m_remoteAddress);

	if (!m_myiot->Login(m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR CProtocol::open 登录物联智控失败!",
				m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		close();
	}

	if (!m_isOpen)
	{

		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR CProtocol::open 失败 route[%d] mainRoute[%d] m_link[%d] logd[%d] line[%d]!",
				m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), __LINE__);
		// FepLog::writelog( m_pGroup->name, m_pLink->name, "", FEP_LOG::PKGDATA );
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		close();

		ec_sleep(OPEN_FAILED_SLEEP_TIME);
		return m_isOpen;
	}
	else
	{

		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO open() 打开规约 m_curRoute[%d] mainRoute[%d] m_link[%d] logd[%d] line[%d]!",
				m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), __LINE__);
		// FepLog::writelog(m_pGroup->name, m_pLink->name, "", FEP_LOG::PKGDATA);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}

	if (!m_isload)
	{
		if (!loadTag(&m_readTagMap, &m_writeTagMap))
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] ERROR open() 数据库 加载数据库状态点/控制点失败 line[%d]!", m_pGroup->name, __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

			close();
			ec_sleep(OPEN_FAILED_SLEEP_TIME);
			m_isOpen = false;
			return false;
		}

		//加载feature
		readProtocolParam(getFeature());

		if (createReadTask() == false)
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO open() createReadTask ERROR route[%d] mainRoute[%d] m_link[%d] line[%d]!",
					m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			return false;
		}

		m_isload = true;
	}

	ec_gettimeofday(&m_send_time_, NULL);
	m_pLinkInfo->state = LINK_STATE_UP; //链路状态
	m_isOpen = true;

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO open() 打开规约成功 route[%d] mainRoute[%d] m_link[%d] line[%d]!",
			m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

	return true;
}

void SCS_HTTP_IOT_SUPCON::close()
{
	DoneRead();
	//通用处理
	CProtocol::close();
	m_isOpen = false;
	delete m_myiot;

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO close() 关闭规约成功 route[%d] mainRoute[%d] m_link[%d] line[%d]!",
			m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
}

void SCS_HTTP_IOT_SUPCON::run()
{
	sendCtrlCmd();
	Send_CallAllData();

	// ProtocolTX(); //发送请求报文
	//  ec_usleep(SLEEP_TIME); //发包和收包间隔，因为不同厂家设备响应速度不同，要综合考虑参数设置
	//  ProtocolRX();		   //接收请求报文、解析并上送数据

	// m_recv_time_ = ACE_OS::gettimeofday();				//记录接收时间
	if (m_pLinkInfo != NULL)
	{
		m_pLinkInfo->lastRxTime = (hUInt32)time(0);
	}
	//路径保活,open赋0，接收处理完毕再赋当前时间值
	if (m_pRouteInfo)
	{
		m_pRouteInfo->lastDataOkTime = (hUInt32)time(0);
	}
}

bool SCS_HTTP_IOT_SUPCON::loadTag(map<string, TAGINFO> *_readMap, map<string, TAGINFO> *_writeMap)
{
	if (!getTag(&m_mysql, _readMap, m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		return false;
	}

	if (!getCmd(&m_mysql, _writeMap, m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		return false;
	}

	return true;
}

bool SCS_HTTP_IOT_SUPCON::readProtocolParam(string _param) //获取协议参数
{
	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO readProtocolParam() 获取的协议参数[%s]  line[%d]",
			m_pGroup->name, _param.c_str(), __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

	int index = 1;
	bool ret = true;
	const char *di = ",";
	char *p = strtok((char *)_param.c_str(), di);
	while (p)
	{
		std::string strtmp = p;
		switch (index)
		{
		case PARAM_TAG_NUM: //包请求点个数
			m_poll_tag_num_ = atoi(strtmp.c_str());
			break;
		case PARAM_RESEND: //重发次数
			m_resend_count_ = atoi(strtmp.c_str());
			break;
		case PARAM_RESPONED_TIME: //响应时间
			m_poll_response_time_ = atoi(strtmp.c_str());
			break;
		case PARAM_LOG_LEVEL: //日志等级
			m_log_level_ = atoi(strtmp.c_str());
			break;
		default:
			ret = false;
			break;
		}
		p = strtok(NULL, di);
		index++;
	}

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO readProtocolParam() 获取的协议参数[%s] m_poll_tag_num_:%d m_resend_count_:%d m_poll_response_time_:%d m_log_level_:%d line[%d]",
			m_pGroup->name, _param.c_str(), m_poll_tag_num_, m_resend_count_, m_poll_response_time_, m_log_level_, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

	return ret;
}

bool SCS_HTTP_IOT_SUPCON::createReadTask()
{
	if (m_readTagMap.empty())
	{
		return false;
	}

	m_deviceTagMap.clear();

	bool flag_deviceInsertSuccess;
	std::map<std::string, TAGINFO> tempMap;
	std::map<std::string, TAGINFO>::iterator tagMapit;

	for (auto m_it = m_readTagMap.begin(); m_it != m_readTagMap.end(); m_it++)
	{
		auto deviceMapIt = m_deviceTagMap.find(m_it->second.param1);
		if (deviceMapIt != m_deviceTagMap.end())
		{
			flag_deviceInsertSuccess = false;
			for (int cout_deviceID = 0; cout_deviceID != m_deviceTagMap.count(m_it->second.param1); cout_deviceID++, deviceMapIt++)
			{
				tagMapit = deviceMapIt->second.find(m_it->second.param2);
				if (tagMapit == deviceMapIt->second.end())
				{
					deviceMapIt->second.insert(std::pair<std::string, TAGINFO>(m_it->second.param2, m_it->second));
					flag_deviceInsertSuccess = true;
				}
			}
			if (!flag_deviceInsertSuccess)
			{
				tempMap.clear();
				tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param2, m_it->second));
				m_deviceTagMap.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param1, tempMap));
			}
		}
		else
		{
			tempMap.clear();
			tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param2, m_it->second));
			m_deviceTagMap.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param1, tempMap));
		}

		// m_deviceTagMap[m_it->second.param1].insert(std::pair<std::string, TAGINFO>(m_it->second.param2, m_it->second));
	}

	for (auto deviceIt = m_deviceTagMap.begin(); deviceIt != m_deviceTagMap.end(); deviceIt++)
	{
		for (auto stateIt = deviceIt->second.begin(); stateIt != deviceIt->second.end(); stateIt++)
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO createReadTask()  device:%s tag:%s name:%s  line[%d]", m_pGroup->name, stateIt->second.param1.c_str(), stateIt->second.param2.c_str(), stateIt->second.tagName.c_str(), __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}

	TASK temp_task;
	int allnum = m_deviceTagMap.size();
	int taskNum = allnum / m_poll_tag_num_ + 1;
	int surTagNum = allnum % m_poll_tag_num_;
	m_task_vec.clear();
	for (int i = 0; i < taskNum; i++)
	{
		temp_task.taskNo = i;
		temp_task.tagStartNo = m_poll_tag_num_ * i;
		if (i == (taskNum - 1))
		{
			temp_task.tagNum = surTagNum;
		}
		else
		{
			temp_task.tagNum = m_poll_tag_num_;
		}
		m_task_vec.push_back(temp_task);

		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO createReadTask() taskNo:%d tagStartNo:%d tagNum:%dline[%d]",
				m_pGroup->name, temp_task.taskNo, temp_task.tagStartNo, temp_task.tagNum, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}

	return true;
}

void SCS_HTTP_IOT_SUPCON::Send_CallAllData() //轮询请求数据
{

	for (std::vector<TASK>::iterator it = m_task_vec.begin(); it != m_task_vec.end(); it++)
	{
		if (!m_myiot->Query(&m_deviceTagMap, it->tagStartNo, it->tagNum, m_pGroup->name, m_pRoute->group, m_pLink->name))
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] ERROR Send_CallAllData() 查询点任务失败！ line[%d]", m_pGroup->name, __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
		else
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO Send_CallAllData() 查询点任务 line[%d]", m_pGroup->name, __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}

	procData();
}

void SCS_HTTP_IOT_SUPCON::procData()
{

	for (auto deviceIt = m_deviceTagMap.begin(); deviceIt != m_deviceTagMap.end(); deviceIt++)
	{
		for (auto stateIt = deviceIt->second.begin(); stateIt != deviceIt->second.end(); stateIt++)
		{
			switch (stateIt->second.valuetype)
			{
			case TAG_TYPE_YX:
				saveYxData(stateIt->second);
				break;
			case TAG_TYPE_YC:
				saveYcData(stateIt->second);
				break;
			case TAG_TYPE_STRING:
				saveStringData(stateIt->second);
				break;
			default:
				break;
			}
			if (LOG_3_NONE <= m_log_level_)
			{
				memset(m_log_buf_, 0, sizeof(m_log_buf_));
				sprintf(m_log_buf_, "[%s] INFO procData() name:%s  value:%g type:%d value_s:%s quality:%d updateTime:%d  line[%d]", m_pGroup->name, stateIt->second.tagName.c_str(), stateIt->second.value, stateIt->second.valuetype, stateIt->second.value_s.c_str(), stateIt->second.quality, stateIt->second.updateTime, __LINE__);
				FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			}
		}
	}
}

bool SCS_HTTP_IOT_SUPCON::sendCtrlCmd() //处理控制命令
{
	if (m_writeTagMap.empty()) //未配置控制点，直接返回
	{
		return false;
	}
	if (skipMutRouteOneFep())
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR sendCtrlCmd() BLOB 收到控制, 备路径不下发 return. line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		return false;
	} //跳过备链路
	ctrl_head m_ctrl_head_;

	// yx控制
	if (m_ctrlInf.get(m_pRoute->group, CTRL_PRO_SETPOINT_YX, cmdBuf, FDC_CTRL_LEN) > 0) //获取YX控制命令CTRL_PRO_SETPOINT_YX
	{
		memset(&m_ctrl_head_, 0, sizeof(m_ctrl_head_));
		memcpy(&m_ctrl_head_, cmdBuf, sizeof(ctrl_head));
		ctrl_pro_setpoint setpt; // YX控制命令
		memcpy(&setpt, cmdBuf + sizeof(ctrl_head), sizeof(ctrl_pro_setpoint));

		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO sendCtrlCmd() 收到一个YX控制命令请求 name[%s] line[%d]",
				m_pGroup->name, setpt.pointName, __LINE__);
		// FepLog::writelog( m_pGroup->name, m_pLink->name, "", FEP_LOG::PKGDATA );
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

		string str_name = setpt.pointName;

		map<string, TAGINFO>::iterator itWriteMap = m_writeTagMap.find(str_name);
		if (itWriteMap != m_writeTagMap.end())
		{
			itWriteMap->second.updateTime = (hUInt32)time(NULL);
			if (1 == setpt.int32Value) // YX类型
			{
				// itWriteMap->second.value = 1;
				m_myiot->Control(&itWriteMap->second.param1, &itWriteMap->second.param2, (double)1, m_pGroup->name, m_pRoute->group, m_pLink->name);
			}
			else
			{
				// itWriteMap->second.value = 0;
				m_myiot->Control(&itWriteMap->second.param1, &itWriteMap->second.param2, (double)0, m_pGroup->name, m_pRoute->group, m_pLink->name);
			}

			m_send_flag = true;
			m_ctrlInf.addCmdAck(m_ctrl_head_.cmdId, CTRL_STATUS_NO_ACK); //控制反馈FEP接口
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO sendCtrlCmd() YX控制下发成功 tag[%s] device[%s] control[%s] object_name[%s] value[%s] line[%d]", m_pGroup->name, itWriteMap->second.tagName.c_str(), itWriteMap->second.param1.c_str(), itWriteMap->second.param2.c_str(), setpt.pointName, std::to_string(itWriteMap->second.value), __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			return true;
		}
	}
	// yc控制
	if (m_ctrlInf.get(m_pRoute->group, CTRL_PRO_SETPOINT_YC, cmdBuf, FDC_CTRL_LEN) > 0)
	{
		memset(&m_ctrl_head_, 0, sizeof(m_ctrl_head_));
		memcpy(&m_ctrl_head_, cmdBuf, sizeof(ctrl_head));
		ctrl_pro_setpoint setpt;
		memcpy(&setpt, cmdBuf + sizeof(ctrl_head), sizeof(ctrl_pro_setpoint));

		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO sendCtrlCmd() 收到一个YC控制命令请求 name[%s] line[%d]",
				m_pGroup->name, setpt.pointName, __LINE__);
		// FepLog::writelog( m_pGroup->name, m_pLink->name, "", FEP_LOG::PKGDATA );
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

		string str_name = setpt.pointName;

		map<string, TAGINFO>::iterator itWriteMap = m_writeTagMap.find(str_name);
		if (itWriteMap != m_writeTagMap.end())
		{
			itWriteMap->second.value = (hFloat)setpt.floatValue;
			itWriteMap->second.value = itWriteMap->second.value * itWriteMap->second.valueRadio + itWriteMap->second.valueOffset;
			itWriteMap->second.updateTime = (hUInt32)time(NULL);

			m_myiot->Control(&itWriteMap->second.param1, &itWriteMap->second.param2, itWriteMap->second.value, m_pGroup->name, m_pRoute->group, m_pLink->name);

			m_send_flag = true;
			m_ctrlInf.addCmdAck(m_ctrl_head_.cmdId, CTRL_STATUS_NO_ACK);
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO sendCtrlCmd() YC控制下发成功 tag[%s] device[%s] control[%s] object_name[%s] value[%s] line[%d]", m_pGroup->name, itWriteMap->second.tagName.c_str(), itWriteMap->second.param1.c_str(), itWriteMap->second.param2.c_str(), setpt.pointName, std::to_string(itWriteMap->second.value).c_str(), __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			return true;
		}
	}
	// kwh控制
	if (m_ctrlInf.get(m_pRoute->group, CTRL_PRO_SETPOINT_KWH, cmdBuf, FDC_CTRL_LEN) > 0)
	{
		memset(&m_ctrl_head_, 0, sizeof(m_ctrl_head_));
		memcpy(&m_ctrl_head_, cmdBuf, sizeof(ctrl_head));
		ctrl_pro_setpoint setpt;
		memcpy(&setpt, cmdBuf + sizeof(ctrl_head), sizeof(ctrl_pro_setpoint));

		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO sendCtrlCmd() 收到一个KWH控制命令请求 name[%s] line[%d]",
				m_pGroup->name, setpt.pointName, __LINE__);
		// FepLog::writelog( m_pGroup->name, m_pLink->name, "", FEP_LOG::PKGDATA );
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

		string str_name = setpt.pointName;

		map<string, TAGINFO>::iterator itWriteMap = m_writeTagMap.begin();
		for (; itWriteMap != m_writeTagMap.end(); itWriteMap++)
		{
			if (itWriteMap->second.tagName == str_name)
			{
				itWriteMap->second.value = (hDouble)setpt.doubleValue;
				itWriteMap->second.value = itWriteMap->second.value * itWriteMap->second.valueRadio + itWriteMap->second.valueOffset;
				itWriteMap->second.updateTime = (hUInt32)time(NULL);

				// m_mqttc.publicControl(itWriteMap->second.dev, itWriteMap->second.param1, std::to_string(itWriteMap->second.value));

				m_send_flag = true;
				m_ctrlInf.addCmdAck(m_ctrl_head_.cmdId, CTRL_STATUS_NO_ACK);
				memset(m_log_buf_, 0, sizeof(m_log_buf_));
				sprintf(m_log_buf_, "[%s] INFO sendCtrlCmd() KWH控制下发成功 tag[%s]  line[%d]", m_pGroup->name, setpt.pointName, __LINE__);
				FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				return true;
			}
		}
	}
	// string控制
	if (m_ctrlInf.get(m_pRoute->group, CTRL_PRO_SETPOINT_STRING, cmdBuf, FDC_CTRL_LEN) > 0)
	{
		memset(&m_ctrl_head_, 0, sizeof(m_ctrl_head_));
		memcpy(&m_ctrl_head_, cmdBuf, sizeof(ctrl_head));
		ctrl_pro_setpoint setpt;
		memcpy(&setpt, cmdBuf + sizeof(ctrl_head), sizeof(ctrl_pro_setpoint));

		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO sendCtrlCmd() 收到一个STRING控制命令请求 name[%s] line[%d]",
				m_pGroup->name, setpt.pointName, __LINE__);
		// FepLog::writelog( m_pGroup->name, m_pLink->name, "", FEP_LOG::PKGDATA );
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

		string str_name = setpt.pointName;

		map<string, TAGINFO>::iterator itWriteMap = m_writeTagMap.find(str_name);
		if (itWriteMap != m_writeTagMap.end())
		{
			// //编码转换 utf-8转gbk
			// char buf[1024];
			// char rbuf[1024];
			// memset(buf, 0, 1024);
			// memset(rbuf, 0, 1024);
			// memcpy(buf, setpt.stringValue, FDC_STRING_LEN);
			// utf8_to_gbk(buf, 1024, rbuf, 1024);

			// itWriteMap->second.value_s = rbuf;
			// memset(m_log_buf_, 0, sizeof(m_log_buf_));
			// sprintf(m_log_buf_, "[%s] INFO saveStringData()  UTF-8转GBK utf-8 buf:%s gbk rbuf:%s  值[%s]",
			// 		m_pGroup->name, buf, rbuf, itWriteMap->second.value_s.c_str());
			// if (LOG_5_NONE <= m_log_level_)
			// 	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			//----------------------------------
			itWriteMap->second.value_s = setpt.stringValue;
			itWriteMap->second.updateTime = (hUInt32)time(NULL);

			m_myiot->Control(&itWriteMap->second.param1, &itWriteMap->second.param2, &itWriteMap->second.value_s, m_pGroup->name, m_pRoute->group, m_pLink->name);

			m_send_flag = true;
			m_ctrlInf.addCmdAck(m_ctrl_head_.cmdId, CTRL_STATUS_NO_ACK);
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO sendCtrlCmd() String控制下发成功 tag[%s] device[%s] control[%s] object_name[%s] value[%s] line[%d]", m_pGroup->name, itWriteMap->second.tagName.c_str(), itWriteMap->second.param1.c_str(), itWriteMap->second.param2.c_str(), setpt.pointName, itWriteMap->second.value_s, __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			return true;
		}
	}
	return false;
}

void SCS_HTTP_IOT_SUPCON::saveYxData(TAGINFO _tag)
{
	FDC_YX_DATA yx_data;
	yx_data.updateTime = (hUInt32)time(NULL);

	timeval updateTimeVal;
	ec_gettimeofday(&updateTimeVal, NULL);
	yx_data.updateTimeMs = updateTimeVal.tv_usec / 1000;
	yx_data.val = (hUInt8)_tag.value;
	FDC_YX_DATA *pYxDat = m_dataInf.yxData(m_curRoute, _tag.offset);
	if (NULL != pYxDat)
	{
		if (ECON_BIT_DISABLED(pYxDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pYxDat->quality, QUALITY_BIT_OV)) //判断某位是否有效
		{
			hUInt8 oldVal = (hUInt8)pYxDat->val;
			bool settime = CALL_DATA_TIME(_tag.updateTime, pYxDat->updateTime) ? true : false; //超过上送时间
			if (oldVal != (hUInt8)yx_data.val || settime)
			{
				yx_data.quality = pYxDat->quality; // quality属性不变

				if (m_dataInf.setYx(m_curRoute, _tag.offset, yx_data, FALSE))
				{
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] RECV <<<< saveYxData()  offset[%d] name[%s] val[%d] quality[%08x] updatetime[%d] line[%d]",
							m_pGroup->name,
							_tag.offset,
							_tag.tagName.c_str(),
							(hUInt8)yx_data.val,
							(hUInt32)yx_data.quality,
							(hUInt32)yx_data.updateTime,
							__LINE__);
					if (LOG_3_NONE <= m_log_level_)
						FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
				else
				{
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] ERROR saveYxData()  上送失败 offset[%d] name[%s] val[%d] quality[%08x] updatetime[%d] line[%d]",
							m_pGroup->name,
							_tag.offset,
							_tag.tagName.c_str(),
							(hUInt8)yx_data.val,
							(hUInt32)yx_data.quality,
							(hUInt32)yx_data.updateTime,
							__LINE__);
					FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
			}
		}
		else
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO saveYxData() 人工置数 不上送 路径[%d], 偏移[%d], 值[%d]",
					m_pGroup->name, m_curRoute, _tag.offset, (hInt32)yx_data.val);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR saveYxData() 获取pYxDat失败 route[%d] offset[%d] line[%d]", m_pGroup->name, m_curRoute, _tag.offset, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
}

void SCS_HTTP_IOT_SUPCON::saveYcData(TAGINFO _tag)
{
	FDC_YC_DATA yc_data;
	yc_data.updateTime = (hUInt32)time(NULL);

	timeval updateTimeVal;
	ec_gettimeofday(&updateTimeVal, NULL);
	yc_data.updateTimeMs = updateTimeVal.tv_usec / 1000;
	yc_data.val = (hFloat)_tag.value;
	yc_data.val = (hFloat)yc_data.val * _tag.valueRadio + _tag.valueOffset;
	FDC_YC_DATA *pYcDat = m_dataInf.ycData(m_curRoute, _tag.offset);
	if (NULL != pYcDat)
	{
		if (ECON_BIT_DISABLED(pYcDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pYcDat->quality, QUALITY_BIT_OV)) //判断某位是否有效
		{
			hFloat oldVal = (hFloat)pYcDat->val;
			bool settime = CALL_DATA_TIME(_tag.updateTime, pYcDat->updateTime) ? true : false; //超过上送时间
			if (oldVal != (hFloat)yc_data.val || settime)
			{
				yc_data.quality = pYcDat->quality; // quality属性不变

				if (m_dataInf.setYc(m_curRoute, _tag.offset, yc_data))
				{
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] RECV <<<< saveYcData()  offset[%d] name[%s] val[%f] quality[%08x] updatetime[%d] line[%d]",
							m_pGroup->name,
							_tag.offset,
							_tag.tagName.c_str(),
							(hFloat)yc_data.val,
							(hUInt32)yc_data.quality,
							(hUInt32)yc_data.updateTime,
							__LINE__);
					if (LOG_3_NONE <= m_log_level_)
						FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
				else
				{
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] ERROR saveYcData()  上送失败 offset[%d] name[%s] val[%f] quality[%08x] updatetime[%d] line[%d]",
							m_pGroup->name,
							_tag.offset,
							_tag.tagName.c_str(),
							(hFloat)yc_data.val,
							(hUInt32)yc_data.quality,
							(hUInt32)yc_data.updateTime,
							__LINE__);
					FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
			}
		}
		else
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO saveYcData() 人工置数 不上送 路径[%d], 偏移[%d], 值[%f]",
					m_pGroup->name, m_curRoute, _tag.offset, (hFloat)yc_data.val);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR saveYcData() 获取pYcDat失败 route[%d] offset[%d] line[%d]", m_pGroup->name, m_curRoute, _tag.offset, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
}

void SCS_HTTP_IOT_SUPCON::saveKwhData(TAGINFO _tag)
{
	FDC_KWH_DATA kwh_data;
	kwh_data.updateTime = (hUInt32)time(NULL);

	timeval updateTimeVal;
	ec_gettimeofday(&updateTimeVal, NULL);
	kwh_data.updateTimeMs = updateTimeVal.tv_usec / 1000;
	kwh_data.val = (hDouble)_tag.value;
	kwh_data.val = (hDouble)kwh_data.val * _tag.valueRadio + _tag.valueOffset;
	FDC_KWH_DATA *pKwhDat = m_dataInf.kwhData(m_curRoute, _tag.offset);
	if (NULL != pKwhDat)
	{
		if (ECON_BIT_DISABLED(pKwhDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pKwhDat->quality, QUALITY_BIT_OV)) //判断某位是否有效
		{
			hDouble oldVal = (hDouble)pKwhDat->val;
			bool settime = CALL_DATA_TIME(_tag.updateTime, pKwhDat->updateTime) ? true : false; //超过上送时间
			if (oldVal != (hDouble)kwh_data.val || settime)
			{
				kwh_data.quality = pKwhDat->quality; // quality属性不变

				if (m_dataInf.setKwh(m_curRoute, _tag.offset, kwh_data))
				{
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] RECV <<<< saveKwhData()  offset[%d] name[%s] val[%g] quality[%08x] updatetime[%d] line[%d]",
							m_pGroup->name,
							_tag.offset,
							_tag.tagName.c_str(),
							(hDouble)kwh_data.val,
							(hUInt32)kwh_data.quality,
							(hUInt32)kwh_data.updateTime,
							__LINE__);
					if (LOG_3_NONE <= m_log_level_)
						FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
				else
				{
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] ERROR saveKwhData()  上送失败 offset[%d] name[%s] val[%g] quality[%08x] updatetime[%d] line[%d]",
							m_pGroup->name,
							_tag.offset,
							_tag.tagName.c_str(),
							(hDouble)kwh_data.val,
							(hUInt32)kwh_data.quality,
							(hUInt32)kwh_data.updateTime,
							__LINE__);
					FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
			}
		}
		else
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO saveKwhData() 人工置数 不上送 路径[%d], 偏移[%d], 值[%g]",
					m_pGroup->name, m_curRoute, _tag.offset, (hDouble)kwh_data.val);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR saveKwhData() 获取pKwhDat失败 route[%d] offset[%d] line[%d]", m_pGroup->name, m_curRoute, _tag.offset, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
}

void SCS_HTTP_IOT_SUPCON::saveStringData(TAGINFO _tag)
{
	FDC_STRING_DATA string_data;
	string_data.updateTime = (hUInt32)time(NULL);

	timeval updateTimeVal;
	ec_gettimeofday(&updateTimeVal, NULL);
	string_data.updateTimeMs = updateTimeVal.tv_usec / 1000;

	memset(string_data.val, 0, FDC_STRING_LEN);
	strncpy(string_data.val, _tag.value_s.c_str(), FDC_STRING_LEN);

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO saveStringData()   路径[%d], 偏移[%d], 值[%s]",
			m_pGroup->name, m_curRoute, _tag.offset, string_data.val);
	if (LOG_5_NONE <= m_log_level_)
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	//修改编码 gbk转utf8
	// char buf[1024];
	// char rbuf[1024];
	// memset(buf,0,1024);
	// memset(rbuf,0,1024);
	// memcpy(buf,string_data.val,FDC_STRING_LEN);

	// gbk_to_utf8(buf,1024,rbuf,1024);
	// memcpy(string_data.val,rbuf,FDC_STRING_LEN);

	// memset(m_log_buf_, 0, sizeof(m_log_buf_));
	// sprintf(m_log_buf_, "[%s] INFO saveStringData()  GBK转UTF-8 gbk buf：%s utf-8 rbuf:%s  路径[%d], 偏移[%d], 值[%s]",
	// m_pGroup->name, buf,rbuf, m_curRoute,_tag.offset, string_data.val);
	// if(LOG_5_NONE<=m_log_level_)
	// FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	//------------------------------------------------------------------------

	FDC_STRING_DATA *pStringDat = m_dataInf.stringData(m_curRoute, _tag.offset);
	if (NULL != pStringDat)
	{
		if (ECON_BIT_DISABLED(pStringDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pStringDat->quality, QUALITY_BIT_OV)) //判断某位是否有效
		{
			bool settime = CALL_DATA_TIME(_tag.updateTime, pStringDat->updateTime) ? true : false; //超过上送时间
			if (strcmp(string_data.val, pStringDat->val) != 0 || settime)
			{
				string_data.quality = pStringDat->quality; // quality属性不变

				if (m_dataInf.setString(m_curRoute, _tag.offset, string_data))
				{
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] RECV <<<< saveStringData()  offset[%d] name[%s] val[%s] quality[%08x] updatetime[%d] line[%d]",
							m_pGroup->name,
							_tag.offset,
							_tag.tagName.c_str(),
							string_data.val,
							(hUInt32)string_data.quality,
							(hUInt32)string_data.updateTime,
							__LINE__);
					if (LOG_3_NONE <= m_log_level_)
						FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
				else
				{
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] ERROR saveStringData()  上送失败 offset[%d] name[%s] val[%s] quality[%08x] updatetime[%d] line[%d]",
							m_pGroup->name,
							_tag.offset,
							_tag.tagName.c_str(),
							string_data.val,
							(hUInt32)string_data.quality,
							(hUInt32)string_data.updateTime,
							__LINE__);
					FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
			}
		}
		else
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO saveStringData() 人工置数 不上送 路径[%d], 偏移[%d], 值[%s]",
					m_pGroup->name, m_curRoute, _tag.offset, string_data.val);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR saveStringData() 获取pStringDat失败 route[%d] offset[%d] line[%d]", m_pGroup->name, m_curRoute, _tag.offset, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
}

// //----------------------------------------------------------------------------
extern "C" HTTP_IOT_SUPCON_EXPORT ECON::FDC::CProtocol *CreateProtocol()
{
	return (new SCS_HTTP_IOT_SUPCON());
}

extern "C" HTTP_IOT_SUPCON_EXPORT void DestroyProtocol(ECON::FDC::CProtocol *p)
{
	delete p;
}
