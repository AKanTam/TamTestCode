#include "scs_http_toilet_ahua.h"

using namespace ECON::FDC;
#define LOGE_HTTP_TOILET_AHUA 31007

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

SCS_HTTP_TOILET_AHUA::SCS_HTTP_TOILET_AHUA() : m_logd_id_(LOGE_HTTP_TOILET_AHUA)
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

SCS_HTTP_TOILET_AHUA::~SCS_HTTP_TOILET_AHUA()
{
}

bool SCS_HTTP_TOILET_AHUA::isOpen() const
{
	return m_isOpen;
}
bool SCS_HTTP_TOILET_AHUA::open()
{
	m_isOpen = CProtocol::open();
	sprintf(m_log_buf_, "%s", m_pLink->remoteAddr);
	m_remoteAddress = m_log_buf_;
	m_mytoilet = new TOILET(m_remoteAddress);

	if (!m_mytoilet->Login(m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR CProtocol::open 光明源智慧公厕接口登录失败!",
				m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
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

void SCS_HTTP_TOILET_AHUA::close()
{
	DoneRead();
	//通用处理
	CProtocol::close();
	m_isOpen = false;
	delete m_mytoilet;

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO close() 关闭规约成功 route[%d] mainRoute[%d] m_link[%d] line[%d]!",
			m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
}

void SCS_HTTP_TOILET_AHUA::run()
{
	Send_CallAllData();

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

bool SCS_HTTP_TOILET_AHUA::loadTag(map<string, TAGINFO> *_readMap, map<string, TAGINFO> *_writeMap)
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

bool SCS_HTTP_TOILET_AHUA::readProtocolParam(string _param) //获取协议参数
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

bool SCS_HTTP_TOILET_AHUA::createReadTask()
{
	if (m_readTagMap.empty())
	{
		return false;
	}

	m_idleMap.clear();
	m_cleanMap.clear();
	m_energyMap.clear();
	m_enviMap.clear();
	m_pitMap.clear();

	std::map<std::string, TAGINFO> m_tempMap;
	std::map<std::string, std::map<std::string, TAGINFO>> m_tempMap_1;
	m_tempMap_1.clear();
	m_tempMap.clear();

	for (std::map<std::string, TAGINFO>::iterator m_it = m_readTagMap.begin(); m_it != m_readTagMap.end(); m_it++)
	{
		if (m_it->second.param1 == "pit")
		{

			if (m_pitMap.find(m_it->second.param2) != m_pitMap.end())
			{
				if (m_pitMap.find(m_it->second.param2)->second.find(m_it->second.param3) == m_pitMap.find(m_it->second.param2)->second.end())
				{
					m_tempMap.clear();

					m_tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
					m_pitMap.find(m_it->second.param2)->second.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param3, m_tempMap));
				}
				else
				{
					m_pitMap.find(m_it->second.param2)->second.find(m_it->second.param3)->second.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
				}
			}
			else
			{
				m_tempMap_1.clear();
				m_tempMap.clear();

				m_tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
				m_tempMap_1.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param3, m_tempMap));

				m_pitMap.insert(std::pair<std::string, std::map<std::string, std::map<std::string, TAGINFO>>>(m_it->second.param2, m_tempMap_1));
			}
		}
		else if (m_it->second.param1 == "envi")
		{
			if (m_enviMap.find(m_it->second.param2) != m_enviMap.end())
			{
				if (m_enviMap.find(m_it->second.param2)->second.find(m_it->second.param3) == m_enviMap.find(m_it->second.param2)->second.end())
				{
					m_tempMap.clear();

					m_tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
					m_enviMap.find(m_it->second.param2)->second.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param3, m_tempMap));
				}
				else
				{
					m_enviMap.find(m_it->second.param2)->second.find(m_it->second.param3)->second.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
				}
			}
			else
			{
				m_tempMap_1.clear();
				m_tempMap.clear();

				m_tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
				m_tempMap_1.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param3, m_tempMap));

				m_enviMap.insert(std::pair<std::string, std::map<std::string, std::map<std::string, TAGINFO>>>(m_it->second.param2, m_tempMap_1));
			}
		}
		else if (m_it->second.param1 == "clean")
		{
			if (m_cleanMap.find(m_it->second.param2) != m_cleanMap.end())
			{
				m_cleanMap.find(m_it->second.param2)->second.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
			}
			else
			{
				m_tempMap.clear();

				m_tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
				m_cleanMap.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param2, m_tempMap));
			}
		}
		else if (m_it->second.param1 == "energy")
		{
			if (m_energyMap.find(m_it->second.param2) != m_energyMap.end())
			{
				if (m_energyMap.find(m_it->second.param2)->second.find(m_it->second.param3) == m_energyMap.find(m_it->second.param2)->second.end())
				{
					m_tempMap.clear();

					m_tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
					m_energyMap.find(m_it->second.param2)->second.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param3, m_tempMap));
				}
				else
				{
					m_energyMap.find(m_it->second.param2)->second.find(m_it->second.param3)->second.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
				}
			}
			else
			{
				m_tempMap_1.clear();
				m_tempMap.clear();

				m_tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
				m_tempMap_1.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param3, m_tempMap));

				m_energyMap.insert(std::pair<std::string, std::map<std::string, std::map<std::string, TAGINFO>>>(m_it->second.param2, m_tempMap_1));
			}
		}
		else if (m_it->second.param1 == "idle")
		{
			if (m_idleMap.find(m_it->second.param2) != m_idleMap.end())
			{
				m_idleMap.find(m_it->second.param2)->second.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
			}
			else
			{
				m_tempMap.clear();

				m_tempMap.insert(std::pair<std::string, TAGINFO>(m_it->second.param4, m_it->second));
				m_idleMap.insert(std::pair<std::string, std::map<std::string, TAGINFO>>(m_it->second.param2, m_tempMap));
			}
		}
	}

	return true;
}

void SCS_HTTP_TOILET_AHUA::Send_CallAllData() //轮询请求数据
{

	/*if (!m_mytoilet->QueryClean(&m_cleanMap, m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		m_mytoilet->Login(m_pGroup->name, m_pRoute->group, m_pLink->name);
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR Send_CallAllData(clean) 查询点任务失败！ line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO Send_CallAllData(clean) 查询点任务 line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}*/
	if (!m_mytoilet->QueryIdle(&m_idleMap, m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		m_mytoilet->Login(m_pGroup->name, m_pRoute->group, m_pLink->name);
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR Send_CallAllData(idle) 查询点任务失败！ line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO Send_CallAllData(idle) 查询点任务 line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	if (!m_mytoilet->QueryEnvi(&m_enviMap, m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		m_mytoilet->Login(m_pGroup->name, m_pRoute->group, m_pLink->name);
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR Send_CallAllData(envi) 查询点任务失败！ line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO Send_CallAllData(envi) 查询点任务 line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	if (!m_mytoilet->QueryEnergy(&m_energyMap, m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		m_mytoilet->Login(m_pGroup->name, m_pRoute->group, m_pLink->name);
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR Send_CallAllData(energy) 查询点任务失败！ line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO Send_CallAllData(energy) 查询点任务 line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	if (!m_mytoilet->QueryPit(&m_pitMap, m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		m_mytoilet->Login(m_pGroup->name, m_pRoute->group, m_pLink->name);
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR Send_CallAllData(pit) 查询点任务失败！ line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO Send_CallAllData(pit) 查询点任务 line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}

	procData();
}

void SCS_HTTP_TOILET_AHUA::procData()
{

	/*for (auto it = m_cleanMap.begin(); it != m_cleanMap.end(); it++)
	{
		for (auto stateIt = it->second.begin(); stateIt != it->second.end(); stateIt++)
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
	}*/
	for (auto it = m_idleMap.begin(); it != m_idleMap.end(); it++)
	{
		for (auto stateIt = it->second.begin(); stateIt != it->second.end(); stateIt++)
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
			// if (LOG_3_NONE <= m_log_level_)
			// {
			// 	memset(m_log_buf_, 0, sizeof(m_log_buf_));
			// 	sprintf(m_log_buf_, "[%s] INFO procData() name:%s  value:%g type:%d value_s:%s quality:%d updateTime:%d  line[%d]", m_pGroup->name, stateIt->second.tagName.c_str(), stateIt->second.value, stateIt->second.valuetype, stateIt->second.value_s.c_str(), stateIt->second.quality, stateIt->second.updateTime, __LINE__);
			// 	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			// }
		}
	}
	for (auto it = m_enviMap.begin(); it != m_enviMap.end(); it++)
	{
		for (auto it1 = it->second.begin(); it1 != it->second.end(); it1++)
		{
			for (auto stateIt = it1->second.begin(); stateIt != it1->second.end(); stateIt++)
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
				// if (LOG_3_NONE <= m_log_level_)
				// {
				// 	memset(m_log_buf_, 0, sizeof(m_log_buf_));
				// 	sprintf(m_log_buf_, "[%s] INFO procData() name:%s  value:%g type:%d value_s:%s quality:%d updateTime:%d  line[%d]", m_pGroup->name, stateIt->second.tagName.c_str(), stateIt->second.value, stateIt->second.valuetype, stateIt->second.value_s.c_str(), stateIt->second.quality, stateIt->second.updateTime, __LINE__);
				// 	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				// }
			}
		}
	}
	for (auto it = m_energyMap.begin(); it != m_energyMap.end(); it++)
	{
		for (auto it1 = it->second.begin(); it1 != it->second.end(); it1++)
		{
			for (auto stateIt = it1->second.begin(); stateIt != it1->second.end(); stateIt++)
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
				// if (LOG_3_NONE <= m_log_level_)
				// {
				// 	memset(m_log_buf_, 0, sizeof(m_log_buf_));
				// 	sprintf(m_log_buf_, "[%s] INFO procData() name:%s  value:%g type:%d value_s:%s quality:%d updateTime:%d  line[%d]", m_pGroup->name, stateIt->second.tagName.c_str(), stateIt->second.value, stateIt->second.valuetype, stateIt->second.value_s.c_str(), stateIt->second.quality, stateIt->second.updateTime, __LINE__);
				// 	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				// }
			}
		}
	}
	for (auto it = m_pitMap.begin(); it != m_pitMap.end(); it++)
	{
		for (auto it1 = it->second.begin(); it1 != it->second.end(); it1++)
		{
			for (auto stateIt = it1->second.begin(); stateIt != it1->second.end(); stateIt++)
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

				// if (LOG_3_NONE <= m_log_level_)
				// {
				// 	memset(m_log_buf_, 0, sizeof(m_log_buf_));
				// 	sprintf(m_log_buf_, "[%s] INFO procData() name:%s  value:%g type:%d value_s:%s quality:%d updateTime:%d  line[%d]", m_pGroup->name, stateIt->second.tagName.c_str(), stateIt->second.value, stateIt->second.valuetype, stateIt->second.value_s.c_str(), stateIt->second.quality, stateIt->second.updateTime, __LINE__);
				// 	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				// }
			}
		}
	}
}

void SCS_HTTP_TOILET_AHUA::saveYxData(TAGINFO _tag)
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

void SCS_HTTP_TOILET_AHUA::saveYcData(TAGINFO _tag)
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

void SCS_HTTP_TOILET_AHUA::saveKwhData(TAGINFO _tag)
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

void SCS_HTTP_TOILET_AHUA::saveStringData(TAGINFO _tag)
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
extern "C" HTTP_TOILET_AHUA_EXPORT ECON::FDC::CProtocol *CreateProtocol()
{
	return (new SCS_HTTP_TOILET_AHUA());
}

extern "C" HTTP_TOILET_AHUA_EXPORT void DestroyProtocol(ECON::FDC::CProtocol *p)
{
	delete p;
}
