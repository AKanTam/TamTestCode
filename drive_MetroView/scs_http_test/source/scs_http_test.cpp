#include "scs_http_test.h"
#include "httplib.h"

using namespace ECON::FDC;
using namespace httplib;

#define LOGE_HTTP_TEST 31000

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

static std::string UtfToGbk(std::string strValue)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strValue.c_str(), -1, NULL, 0);
	wchar_t *wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, strValue.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char *str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	std::string strTemp = str;
	if (wstr)
		delete[] wstr;
	if (str)
		delete[] str;
	return strTemp;
}

SCS_HTTP_TEST::SCS_HTTP_TEST() : m_logd_id_(LOGE_HTTP_TEST)
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
	m_isdevlist = 0;
	memset(m_recv_data_buffer_, 0, sizeof(m_recv_data_buffer_));
	memset(m_send_data_buffer_, 0, sizeof(m_send_data_buffer_));
}

SCS_HTTP_TEST::~SCS_HTTP_TEST()
{
}

bool SCS_HTTP_TEST::isOpen() const
{
	return m_isOpen;
}
bool SCS_HTTP_TEST::open()
{
	m_isOpen = CProtocol::open();
	sprintf(m_log_buf_, "%s", m_pLink->remoteAddr);
	m_remoteAddress = m_log_buf_;
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

		if (!m_isdevlist)
		{
			httplib::Client cli(m_remoteAddress);
			if (auto res = cli.Get("/api/device/list"))
			{
				cJSON *json = cJSON_Parse(res->body.c_str());
				cJSON *root = cJSON_CreateObject();
				cJSON *json_data = NULL;
				cJSON *item = NULL;

				json_data = cJSON_GetObjectItem(json, "data");
				if (cJSON_IsArray(json_data))
				{
					cJSON_ArrayForEach(item, json_data)
					{
						if (cJSON_IsString(item) && item->valuestring != NULL)
						{
							string deviceSn = item->valuestring;
							m_devList.push_back(item->valuestring);
						}
					}
				}

				cJSON_Delete(json);

				memset(m_log_buf_, 0, sizeof(m_log_buf_));
				sprintf(m_log_buf_, "[%s] INFO open() GET获取设备列表 m_curRoute[%d] mainRoute[%d] m_link[%d] logd[%d] size[%d] line[%d]! ",
						m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), sizeof(listBuf), __LINE__);
				FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			}
			else
			{
				memset(m_log_buf_, 0, sizeof(m_log_buf_));
				sprintf(m_log_buf_, "[%s] INFO open() GET转换设备列表失败 m_curRoute[%d] mainRoute[%d] m_link[%d] logd[%d] size[%s] line[%d]!  ",
						m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), sizeof(listBuf), __LINE__);
				FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			}
			if (m_devList.empty())
			{
				memset(m_log_buf_, 0, sizeof(m_log_buf_));
				sprintf(m_log_buf_, "[%s] INFO open() GET获取的设备列表列表为空 m_curRoute[%d] mainRoute[%d] m_link[%d] logd[%d] line[%d]!",
						m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), __LINE__);
				FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			}

			m_isdevlist = true;
		}
	}

	if (!m_isload)
	{
		if (!loadTag(&m_readTagMap))
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] ERROR open() 数据库 加载数据库状态点/控制点失败 line[%d]!", m_pGroup->name, __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

			close(); //加载配置信息
			ec_sleep(OPEN_FAILED_SLEEP_TIME);
			m_isOpen = false;
			return false;
		}

		//加载feature
		readProtocolParam(getFeature());

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

DEVYONGJI SCS_HTTP_TEST::processData(const string &body)
{
	DEVYONGJI temp;

	string tt;

	cJSON *cBody = cJSON_Parse(body.c_str());
	cJSON *item;
	cJSON *data = cJSON_GetObjectItem(cBody, "data");
	cJSON *status = cJSON_GetObjectItem(cBody, "status");
	cJSON *ports = cJSON_GetObjectItem(data, "ports");
	cJSON *acdc = cJSON_GetObjectItem(ports, "acdc");

	tt = cJSON_Print(status);
	if (tt.find("true") != string::npos)
	{
		temp.m_isOnline = 0;
	}
	else
	{
		temp.m_alarm = 1;
	}

	item = cJSON_GetObjectItem(data, "door_alarm");
	tt = cJSON_GetStringValue(item);
	if (tt.find("关") != string::npos)
	{
		temp.m_alarm = 0;
	}
	else
	{
		temp.m_alarm = 1;
	}

	item = cJSON_GetObjectItem(data, "door_status");
	tt = cJSON_GetStringValue(item);
	if (tt.find("关") != string::npos)
	{
		temp.m_doorStatus = 1;
	}
	else
	{
		temp.m_alarm = 0;
	}

	item = cJSON_GetObjectItem(data, "temp");
	if (item->type != cJSON_String)
	{
		temp.m_tempture = item->valuedouble;
	}
	if (item->type == cJSON_String)
	{
		temp.m_tempture = atof(cJSON_GetStringValue(item));
	}

	item = cJSON_GetObjectItem(data, "humi");
	if (item->type != cJSON_String)
	{
		temp.m_humidity = item->valuedouble;
	}
	if (item->type == cJSON_String)
	{
		temp.m_humidity = atof(cJSON_GetStringValue(item));
	}

	cJSON *iitem = NULL;
	cJSON *array = cJSON_GetObjectItem(acdc, "1");
	if (!array)
	{
		cJSON *array = cJSON_GetObjectItem(acdc, "71");
	}
	if (!array)
	{
		temp.m_switchCurrent = 0;
		temp.m_switchPower = 0;
		temp.m_switchStatus = 0;
		temp.m_switchVoltage = 0;
		temp.m_yuntaiCurrent = 0;
		temp.m_yuntaiPower = 0;
		temp.m_yuntaiStatus = 0;
		temp.m_yuntaiVoltage = 0;
		temp.m_cameraCurrent = 0;
		temp.m_cameraPower = 0;
		temp.m_cameraStatus = 0;
		temp.m_cameraVoltage = 0;
	}
	else
	{

		for (int y = 0; y < 3; y++)
		{
			string index1 = to_string(y);
			cJSON *array1 = cJSON_GetObjectItem(array, index1.c_str());

			if (!array1)
			{
				break;
			}
			else
			{

				iitem = cJSON_GetObjectItem(array1, "ext_device_type");
				string devType = cJSON_GetStringValue(iitem);
				if (devType.find("XPPN-9000") != string::npos)
				{
					iitem = cJSON_GetObjectItem(array1, "on_off");
					temp.m_switchStatus = iitem->valuedouble;
					temp.m_switchStatus = !temp.m_switchStatus;
					iitem = cJSON_GetObjectItem(array1, "current");
					temp.m_switchCurrent = iitem->valuedouble;
					iitem = cJSON_GetObjectItem(array1, "voltage");
					temp.m_switchVoltage = atof(cJSON_GetStringValue(iitem));
					temp.m_switchPower = ((temp.m_switchCurrent) / 1000) * (temp.m_switchVoltage);
				}

				if (devType.find("枪机") != string::npos)
				{
					iitem = cJSON_GetObjectItem(array1, "on_off");
					temp.m_cameraStatus = iitem->valuedouble;
					temp.m_cameraStatus = !temp.m_cameraStatus;
					iitem = cJSON_GetObjectItem(array1, "current");
					temp.m_cameraCurrent = iitem->valuedouble;
					iitem = cJSON_GetObjectItem(array1, "voltage");
					temp.m_cameraVoltage = atof(cJSON_GetStringValue(iitem));
					temp.m_cameraPower = ((temp.m_cameraCurrent) / 1000) * (temp.m_cameraVoltage);
				}

				if (devType.find("云台") != string::npos)
				{
					iitem = cJSON_GetObjectItem(array1, "on_off");
					temp.m_yuntaiStatus = iitem->valuedouble;
					temp.m_yuntaiStatus = !temp.m_yuntaiStatus;
					iitem = cJSON_GetObjectItem(array1, "current");
					temp.m_yuntaiCurrent = iitem->valuedouble;
					iitem = cJSON_GetObjectItem(array1, "voltage");
					temp.m_yuntaiVoltage = atof(cJSON_GetStringValue(iitem));
					temp.m_yuntaiPower = ((temp.m_yuntaiCurrent) / 1000) * (temp.m_yuntaiVoltage);
				}
			}
		}
	}

	temp.m_totalPower = temp.m_cameraPower + temp.m_switchPower + temp.m_yuntaiPower;

	cJSON_Delete(cBody);

	return temp;
}

bool SCS_HTTP_TEST::getPointData(map<string, DEVYONGJI> devdatamap, list<TAGINFO> &_datalist, map<string, TAGINFO> _readTagMap)
{
	_datalist.clear();
	for (map<string, TAGINFO>::iterator itor = _readTagMap.begin(); itor != _readTagMap.end(); itor++)
	{

		TAGINFO tagTemp;

		tagTemp.pointName = itor->second.pointName.c_str();

		if (itor->second.dev.find("TXZT") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_isOnline;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("WD") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_tempture;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("SD") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_humidity;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("KGFK") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_doorStatus;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("KXBJ") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_alarm;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("GL") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_totalPower;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("JHJDL") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchCurrent;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("JHJDY") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchVoltage;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("JHJGL") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchPower;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("JHJGZZT") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchStatus;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("SXJDL") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchCurrent;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("SXJDY") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchVoltage;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("SXJGL") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchPower;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("SXJGZZT") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchStatus;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("YTDL") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchCurrent;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("YTDY") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchVoltage;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("YTGL") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchPower;
			tagTemp.quality = 129;
		}

		else if (itor->second.dev.find("YTGZZT") != string::npos)
		{
			tagTemp.value = devdatamap[itor->second.name].m_switchStatus;
			tagTemp.quality = 129;
		}

		_datalist.push_back(tagTemp);
	}
}

void SCS_HTTP_TEST::close()
{
	DoneRead();
	//通用处理
	CProtocol::close();
	m_isOpen = false;

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO close() 关闭规约成功 route[%d] mainRoute[%d] m_link[%d] line[%d]!",
			m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
}

void SCS_HTTP_TEST::run()
{
	httplib::Client cli(m_remoteAddress);
	m_devDataMap.clear();

	for (auto dvIt = m_devList.begin(); dvIt != m_devList.end(); dvIt++)
	{
		string string1 = "{\"sn\": \"";
		string string2 = "\"}";
		string body = string1 + *dvIt + string2;

		if (auto res = cli.Post("/api/device/info", body, "application/json"))
		{
			m_devDataMap.insert(std::make_pair(*dvIt, (processData(UtfToGbk(res->body)))));
		}
	}

	procData();

	Sleep(1000 * 60);

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

bool SCS_HTTP_TEST::loadTag(map<string, TAGINFO> *_readMap)
{
	if (!getTag(&m_mysql, _readMap, m_pGroup->name, m_pRoute->group, m_pLink->name))
	{
		return false;
	}
	return true;
}

bool SCS_HTTP_TEST::readProtocolParam(string _param) //获取协议参数
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

void SCS_HTTP_TEST::ProtocolTX() //发送请求报文
{
	Send_CallAllData();
}
void SCS_HTTP_TEST::ProtocolRX() //数据上送
{
}

void SCS_HTTP_TEST::Send_CallAllData() //轮询请求数据
{

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO Send_CallAllData() 查询点任务 line[%d]", m_pGroup->name, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	m_send_flag = true;
}

void SCS_HTTP_TEST::procData()
{
	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO procData()IS READRESPONSE line[%d]", m_pGroup->name, __LINE__);
	if (LOG_5_NONE <= m_log_level_)
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

	list<TAGINFO> taglist;
	getPointData(m_devDataMap, taglist, m_readTagMap);

	if (taglist.empty())
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR procData() 查询点信息为空 line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	list<TAGINFO>::iterator itreadlist = taglist.begin();
	for (; itreadlist != taglist.end(); itreadlist++)
	{
		m_readTagMap[itreadlist->pointName].value = itreadlist->value;
		m_readTagMap[itreadlist->pointName].value_s = itreadlist->value_s;
		m_readTagMap[itreadlist->pointName].quality = itreadlist->quality;
		m_readTagMap[itreadlist->pointName].updateTime = itreadlist->updateTime;

		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO procData() name:%s  value:%g type:%d value_s:%s quality:%d updateTime:%d  line[%d]", m_pGroup->name, itreadlist->pointName.c_str(), itreadlist->value, m_readTagMap[itreadlist->pointName].valuetype, itreadlist->value_s.c_str(), itreadlist->quality, itreadlist->updateTime, __LINE__);
		if (LOG_3_NONE <= m_log_level_)
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

		//上送实时库
		switch (m_readTagMap[itreadlist->pointName].valuetype)
		{
		case TAG_TYPE_YX:
			saveYxData(m_readTagMap[itreadlist->pointName]);
			break;
		case TAG_TYPE_YC:
			saveYcData(m_readTagMap[itreadlist->pointName]);
			break;
		case TAG_TYPE_KWH:
			saveKwhData(m_readTagMap[itreadlist->pointName]);
			break;
		case TAG_TYPE_STRING:
			saveStringData(m_readTagMap[itreadlist->pointName]);
			break;
		default:
			break;
		}
	}

	m_send_flag = false;
	m_recv_head = false;
}

void SCS_HTTP_TEST::saveYxData(TAGINFO _tag)
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
							_tag.pointName.c_str(),
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
							_tag.pointName.c_str(),
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

void SCS_HTTP_TEST::saveYcData(TAGINFO _tag)
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
							_tag.pointName.c_str(),
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
							_tag.pointName.c_str(),
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

void SCS_HTTP_TEST::saveKwhData(TAGINFO _tag)
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
							_tag.pointName.c_str(),
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
							_tag.pointName.c_str(),
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

void SCS_HTTP_TEST::saveStringData(TAGINFO _tag)
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
							_tag.pointName.c_str(),
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
							_tag.pointName.c_str(),
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
extern "C" HTTP_TEST_EXPORT ECON::FDC::CProtocol *CreateProtocol()
{
	return (new SCS_HTTP_TEST());
}

extern "C" HTTP_TEST_EXPORT void DestroyProtocol(ECON::FDC::CProtocol *p)
{
	delete p;
}
