#include "scs_httpserver_toilet_jiashan.h"

#include "httplib.h"

using namespace ECON::FDC;
#define LOGE_HTTPSERVER_TOILET_JIASHAN 31008

using namespace httplib;

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

SCS_HTTPSERVER_TOILET_JIASHAN::SCS_HTTPSERVER_TOILET_JIASHAN() : m_logd_id_(LOGE_HTTPSERVER_TOILET_JIASHAN)
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

SCS_HTTPSERVER_TOILET_JIASHAN::~SCS_HTTPSERVER_TOILET_JIASHAN()
{
}

bool SCS_HTTPSERVER_TOILET_JIASHAN::createReadTask()
{
	if (m_readTagMap.empty())
	{
		return false;
	}

	std::map<std::string, std::map<std::string, std::map<std::string, TAGINFO>>> temp_aamap;
	std::map<std::string, std::map<std::string, TAGINFO>> temp_bbmap;
	std::map<std::string, TAGINFO> temp_ccmap;

	dataMap.clear();
	for (auto itRead = m_readTagMap.begin(); itRead != m_readTagMap.end(); itRead++)
	{

		temp_aamap.clear();
		temp_bbmap.clear();
		temp_ccmap.clear();

		auto itFirst = dataMap.find(itRead->second.param1);
		if (itFirst != dataMap.end())
		{
			auto itSecond = itFirst->second.find(itRead->second.param2);
			if (itSecond != itFirst->second.end())
			{
				auto itThree = itSecond->second.find(itRead->second.param3);
				if (itThree != itSecond->second.end())
				{
					itThree->second.insert(std::make_pair(itRead->second.param4, itRead->second));
				}
				else
				{
					temp_ccmap.insert(std::make_pair(itRead->second.param4, itRead->second));
					itSecond->second.insert(std::make_pair(itRead->second.param3, temp_ccmap));
				}
			}
			else
			{
				temp_ccmap.insert(std::make_pair(itRead->second.param4, itRead->second));
				temp_bbmap.insert(std::make_pair(itRead->second.param3, temp_ccmap));
				itFirst->second.insert(std::make_pair(itRead->second.param2, temp_bbmap));
			}
		}
		else
		{
			temp_ccmap.insert(std::make_pair(itRead->second.param4, itRead->second));
			temp_bbmap.insert(std::make_pair(itRead->second.param3, temp_ccmap));
			temp_aamap.insert(std::make_pair(itRead->second.param2, temp_bbmap));
			dataMap.insert(std::make_pair(itRead->second.param1, temp_aamap));
		}
	}

	return true;
}

bool SCS_HTTPSERVER_TOILET_JIASHAN::isOpen() const
{
	return m_isOpen;
}
bool SCS_HTTPSERVER_TOILET_JIASHAN::open()
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

void SCS_HTTPSERVER_TOILET_JIASHAN::close()
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

void SCS_HTTPSERVER_TOILET_JIASHAN::run()
{
	Server svr;
	svr.Post("/api/v1/apis/toilet/post-sensor-data", [this](const Request &req, Response &res)
			 {
				cJSON *json = cJSON_Parse(req.body.c_str());
				if(json == NULL){
					return;
				}
				cJSON * sensor = cJSON_GetObjectItemCaseSensitive(json,"sensor");
				cJSON * ser_num = cJSON_GetObjectItemCaseSensitive(json,"ser_num");
				if(cJSON_IsString(ser_num) && (ser_num->valuestring != NULL))
				{
					auto itFirst = dataMap.find(ser_num->valuestring);
                    if(itFirst!=dataMap.end())
					{
                        cJSON *sensor_type = cJSON_GetObjectItemCaseSensitive(sensor,"type");
						if(cJSON_IsString(sensor_type)&&(sensor_type->valuestring!= NULL))
						{
							std::string typeChar = sensor_type->valuestring;
							auto itSecond = itFirst->second.find(typeChar);
							if(itSecond != itFirst->second.end())
							{ 
							    std::string index = std::to_string(cJSON_GetObjectItemCaseSensitive(sensor,"index")->valueint);

								auto itThree = itSecond->second.find(index);
								if(itThree != itSecond->second.end())
								{
									hUInt32 upTime = cJSON_GetObjectItemCaseSensitive(sensor,"time")->valueint;
									auto itTag = itThree->second.end();
									if(strcmp(typeChar.c_str(),"cubicle") == 0)
							    	{
										cJSON *item = NULL;
										cJSON *item_index = NULL;
										cJSON *item_gen = NULL;
										int idtk_16 = 0;
										int idtk_32 = 0;
										int idtk_48 = 0;
										cJSON_ArrayForEach(item,cJSON_GetObjectItemCaseSensitive(sensor,"cubicles"))
										{
											item_index = cJSON_GetObjectItemCaseSensitive(item,"index");
											item_gen = cJSON_GetObjectItemCaseSensitive(item,"gender");
											if(cJSON_IsNumber(item_index))
											{
											    itTag = itThree->second.find(std::to_string(item_index->valueint));
										        if(itTag != itThree->second.end())
										        {
											        itTag->second.updateTime = upTime;
											        itTag->second.value = cJSON_GetObjectItemCaseSensitive(item,"status")->valueint;
													saveFEPData(itTag->second);
										        }
												else
											    {
										            memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                                                sprintf(m_log_buf_, "[%s] ERROR run() 点位未配置！ [%d] line[%d]",m_pGroup->name,item_index->valueint ,__LINE__);
										            FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
											    }
												if(item_gen->valueint == 16)
												{
													if(itTag->second.value == 0)
													{
														++idtk_16;
													}
												}else if(item_gen->valueint == 32)
												{
													if(itTag->second.value == 0)
													{
														++idtk_32;
													}
												}else if(item_gen->valueint == 48)
												{
													if(itTag->second.value == 0)
													{
														++idtk_48;
													}
												}
											}
											else
											{
										        memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                                            sprintf(m_log_buf_, "[%s] ERROR run() 未解析到可用的 [item_Index] line[%d]",m_pGroup->name ,__LINE__);
										        FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
											}
										}
										auto itIdle = itFirst->second.find("idle");
										if(itIdle != itFirst->second.end())
										{
											auto itIndex_16 = itIdle->second.find("16");
											if(itIndex_16 != itIdle->second.end()){
												auto itTemp = itIndex_16->second.find("idle");
												itTemp->second.updateTime = upTime;
												itTemp->second.value = idtk_16;
												saveFEPData(itTemp->second);
											}
											auto itIndex_32 = itIdle->second.find("32");
											if(itIndex_32 != itIdle->second.end()){
												auto itTemp = itIndex_32->second.find("idle");
												itTemp->second.updateTime = upTime;
												itTemp->second.value = idtk_32;
												saveFEPData(itTemp->second);
											}
											auto itIndex_48 = itIdle->second.find("48");
											if(itIndex_48 != itIdle->second.end()){
												auto itTemp = itIndex_48->second.find("idle");
												itTemp->second.updateTime = upTime;
												itTemp->second.value = idtk_48;
												saveFEPData(itTemp->second);
											}
										}
							    	}
							    	else if(strcmp(typeChar.c_str(),"idtk") == 0)
							    	{
										itTag = itThree->second.find("all_rate");
										if(itTag != itThree->second.end())
										{
											itTag->second.updateTime = upTime;
											itTag->second.value = cJSON_GetObjectItemCaseSensitive(sensor,"all_rate")->valueint;
											saveFEPData(itTag->second);
										}
										else
										{
											memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                                        sprintf(m_log_buf_, "[%s] ERROR run() 点位未配置！ [%s][%s][%s][%s] [all_rate] line[%d]",m_pGroup->name,ser_num->valuestring ,sensor_type->valuestring ,index.c_str()  ,__LINE__);
										    FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
										}
							    	}
							    	else if(strcmp(typeChar.c_str(),"alarm") == 0)
							    	{
										cJSON *alarm_key = cJSON_GetObjectItemCaseSensitive(sensor,"alarm_key");
										if(cJSON_IsString(alarm_key)&&(alarm_key->valuestring!= NULL))
										{
										    itTag = itThree->second.find(alarm_key->valuestring);
										    if(itTag != itThree->second.end())
										    {
										    	itTag->second.updateTime = upTime;
										    	itTag->second.value = cJSON_GetObjectItemCaseSensitive(sensor,"status")->valueint;
										    	saveFEPData(itTag->second);
										    }
										    else
										    {
										        memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                                            sprintf(m_log_buf_, "[%s] ERROR run() 点位未配置！ [%s][%s][%s][%s] line[%d]",m_pGroup->name,ser_num->valuestring ,sensor_type->valuestring ,index.c_str() ,alarm_key->valuestring ,__LINE__);
										        FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
										    }
										}
							    	}
									else if(strcmp(typeChar.c_str(),"nh3") == 0||strcmp(typeChar.c_str(),"h2s") == 0||strcmp(typeChar.c_str(),"hum") == 0||strcmp(typeChar.c_str(),"co2") == 0 ||strcmp(typeChar.c_str(),"pm25") == 0||strcmp(typeChar.c_str(),"voc") == 0)
							    	{
                                        itTag = itThree->second.find(typeChar);
										if(itTag != itThree->second.end())
										{
											itTag->second.updateTime = upTime;
											itTag->second.value = cJSON_GetObjectItemCaseSensitive(sensor,typeChar.c_str())->valuedouble;
											saveFEPData(itTag->second);
										}
										else
										{
										    memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                                        sprintf(m_log_buf_, "[%s] ERROR run() 点位未配置！ [%s][%s][%s][%s] line[%d]",m_pGroup->name,ser_num->valuestring ,sensor_type->valuestring ,index.c_str() ,typeChar.c_str() ,__LINE__);
										    FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
										}
							    	}
								}
                                else{
										memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                                    sprintf(m_log_buf_, "[%s] ERROR run() 点位未配置！ [%s][%s][%s] line[%d]",m_pGroup->name,ser_num->valuestring ,sensor_type->valuestring ,index.c_str() ,__LINE__);
										FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
                                    }
							}
							else{
								memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                            sprintf(m_log_buf_, "[%s] ERROR run() 点位未配置！ [%s][%s] line[%d]", m_pGroup->name,ser_num->valuestring ,sensor_type->valuestring ,__LINE__);
								FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
							}
						}
						else
						{
				            memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                        sprintf(m_log_buf_, "[%s] ERROR run() 未解析到可用的sensor_type line[%d]",m_pGroup->name, __LINE__);
                            FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
						}
                    }
                    else
					{
					    memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                    sprintf(m_log_buf_, "[%s] ERROR run() 点位未配置！ [%s] line[%d]",m_pGroup->name,ser_num->valuestring ,__LINE__);
						FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
                    }
				}
				else{
				    memset(m_log_buf_, 0, sizeof(m_log_buf_));
	                sprintf(m_log_buf_, "[%s] ERROR run() 未解析到可用的ser_num line[%d]",m_pGroup->name, __LINE__);
                    FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
				
                cJSON_Delete(json);

				res.set_content("{\"code\":200,\"message\":\"success\",\"data\":{}}", "application/json");

				memset(m_log_buf_, 0, sizeof(m_log_buf_));
	            sprintf(m_log_buf_, "[%s] INFO run() 接收轮询消息 line[%d]",m_pGroup->name, __LINE__);
                FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

				 if (m_pLinkInfo != NULL)
				 {
					 m_pLinkInfo->lastRxTime = (hUInt32)time(0);
				 }
				 //路径保活,open赋0，接收处理完毕再赋当前时间值
				 if (m_pRouteInfo)
				 {
					 m_pRouteInfo->lastDataOkTime = (hUInt32)time(0);
				 } });
	svr.listen("0.0.0.0", 8888);
}

bool SCS_HTTPSERVER_TOILET_JIASHAN::loadTag(map<string, TAGINFO> *_readMap, map<string, TAGINFO> *_writeMap)
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

bool SCS_HTTPSERVER_TOILET_JIASHAN::readProtocolParam(string _param) //获取协议参数
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

void SCS_HTTPSERVER_TOILET_JIASHAN::Send_CallAllData() //轮询请求数据
{
	procData();
}

void SCS_HTTPSERVER_TOILET_JIASHAN::procData()
{

	/*for (auto it = m_idleMap.begin(); it != m_idleMap.end(); it++)
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
}

void SCS_HTTPSERVER_TOILET_JIASHAN::saveFEPData(const TAGINFO &_tag)
{
	switch (_tag.valuetype)
	{
	case TAG_TYPE_YX:
		saveYxData(_tag);
		break;
	case TAG_TYPE_YC:
		saveYcData(_tag);
		break;
	case TAG_TYPE_STRING:
		saveStringData(_tag);
		break;
	default:
		break;
	}
	if (LOG_3_NONE <= m_log_level_)
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO procData() name:%s  value:%g type:%d value_s:%s updateTime:%d  line[%d]", m_pGroup->name, _tag.tagName.c_str(), _tag.value, _tag.valuetype, _tag.value_s.c_str(), _tag.updateTime, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
}

void SCS_HTTPSERVER_TOILET_JIASHAN::saveYxData(const TAGINFO &_tag)
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

void SCS_HTTPSERVER_TOILET_JIASHAN::saveYcData(const TAGINFO &_tag)
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

void SCS_HTTPSERVER_TOILET_JIASHAN::saveKwhData(const TAGINFO &_tag)
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

void SCS_HTTPSERVER_TOILET_JIASHAN::saveStringData(const TAGINFO &_tag)
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
extern "C" HTTPSERVER_TOILET_JIASHAN_EXPORT ECON::FDC::CProtocol *CreateProtocol()
{
	return (new SCS_HTTPSERVER_TOILET_JIASHAN());
}

extern "C" HTTPSERVER_TOILET_JIASHAN_EXPORT void DestroyProtocol(ECON::FDC::CProtocol *p)
{
	delete p;
}
