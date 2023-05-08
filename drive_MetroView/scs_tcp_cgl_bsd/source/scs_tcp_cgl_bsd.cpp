#include "scs_tcp_cgl_bsd.h"


using namespace ECON::FDC;
#define LOGE_TCP_CGL_BSD 31011


const char *str_heart = "{	\"MSG\":0,	\"alive\":1,	\"time\":";
const char *str_login = "{\"MSG\":6,\"ack\":3,\"type\":2,\"flag\":0,\"time\":";
const char *str_ctrl_bsd_1 = "{\"MSG\":230,\"oid\":1,\"owners\":[{\"name\":\"声光报警器\",\"type\":\"\",\"factory\":\"\",\"moudle\":\"\",\"state\":0,\"binds\":[{\"error\":0,\"bind\":{\"ch\":8,\"proto\":589825,\"arg\":1},\"values\":[{\"name\":\"on\",\"value\":\"b";
const char *str_ctrl_bsd_2 = "\"}]}]}]}\r\n";

std::string buf_rec;

/**
 * 截取str后的元素
 * @param stream 待截取字符串
 * @param str 截取定位字符串
 * @return
 */
static auto cutNext(string stream, const string &str)
{
	int nPos = stream.find(str);

	if (nPos != -1)
	{
		stream = stream.substr(nPos + str.size(), stream.size());
	}
	return stream;
}

/**
 * 截取str前的元素
 * @param stream 待截取字符串
 * @param str 截取定位字符串
 * @return
 */
static std::string cutPre(std::string stream, const std::string &str)
{
	int nPos = stream.find(str);
	if (nPos != -1)
	{
		stream = stream.substr(0, nPos);
	}
	return stream;
}

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

SCS_TCP_CGL_BSD::SCS_TCP_CGL_BSD() : m_logd_id_(LOGE_TCP_CGL_BSD)
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

SCS_TCP_CGL_BSD::~SCS_TCP_CGL_BSD()
{
}

bool SCS_TCP_CGL_BSD::createReadTask()
{

	return true;
}

bool SCS_TCP_CGL_BSD::isOpen() const
{
	return m_isOpen;
}
bool SCS_TCP_CGL_BSD::open()
{
	m_isOpen = CProtocol::open();
	sprintf(m_log_buf_, "%s", m_pLink->remoteAddr);
	m_remoteAddress = m_log_buf_;

	m_add = cutPre(m_remoteAddress, ":");
	m_port = cutNext(m_remoteAddress, ":");

	tcpClient = new CTcpClient();
	tcpClient->m_strServerIp = m_add;
	tcpClient->m_uServerPort = atoi(m_port.c_str());

	if (!tcpClient->InitConnect())
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR open() 创建套接字失败 ip[%s] port[%s] m_curRoute[%d] mainRoute[%d] m_link[%d] logd[%d] line[%d]!",
				m_pGroup->name, m_add.c_str(), m_port.c_str(), m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
	else
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO open() 创建套接字成功 ip[%s] port[%s] m_curRoute[%d] mainRoute[%d] m_link[%d] logd[%d] line[%d]!",
				m_pGroup->name, m_add.c_str(), m_port.c_str(), m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), __LINE__);
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

		// 加载feature
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
	m_pLinkInfo->state = LINK_STATE_UP; // 链路状态
	m_isOpen = true;

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO open() 打开规约成功 route[%d] mainRoute[%d] m_link[%d] line[%d]!",
			m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

	return true;
}

void SCS_TCP_CGL_BSD::close()
{
	DoneRead();
	// 通用处理
	CProtocol::close();
	m_isOpen = false;

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO close() 关闭规约成功 route[%d] mainRoute[%d] m_link[%d] line[%d]!",
			m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
}

void SCS_TCP_CGL_BSD::run()
{
	if (sendCtrlCmd())
	{
	}
	else
	{
		std::string char_send = str_login + std::to_string(time(0));
		char_send += "}\r\n";
		tcpClient->SendMsg(char_send.c_str());

		std::string recv;
		if (tcpClient->RecvMsg(recv))
		{
			buf_rec += recv;

			cJSON *json = cJSON_Parse(buf_rec.c_str());
			if (json != NULL)
			{
				// LOG(INFO) << buf_rec;
				cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "MSG");
				if (cJSON_IsNumber(msg))
				{
					switch (msg->valueint)
					{
					case 0:
					{
						std::string char_send = str_heart + std::to_string(time(0));
						char_send += "}\r\n";
						tcpClient->SendMsg(char_send.c_str());
						// LOG(DEBUG) << char_send;
						break;
					}
					case 5:
					{
						std::string char_send = str_login + std::to_string(time(0));
						char_send += "}\r\n";
						tcpClient->SendMsg(char_send.c_str());
						// LOG(DEBUG) << char_send;
						break;
					}
					case 64:
					{
						cJSON *rt_spm = cJSON_GetObjectItemCaseSensitive(json, "rt_spm");
						if (cJSON_IsObject(rt_spm))
						{
							cJSON *owners = cJSON_GetObjectItemCaseSensitive(rt_spm, "owners");
							if (cJSON_IsArray(owners))
							{
								cJSON *item = NULL;
								cJSON_ArrayForEach(item, owners)
								{
									cJSON *moudle = cJSON_GetObjectItemCaseSensitive(item, "moudle");
									if (cJSON_IsString(moudle) && moudle->valuestring != NULL)
									{
										if (strcmp(moudle->valuestring, "UEN-2P") == 0)
										{
											cJSON *binds = cJSON_GetObjectItemCaseSensitive(item, "binds");
											if (cJSON_IsArray(binds))
											{
												cJSON *item_array = cJSON_GetArrayItem(binds, 0);
												if (cJSON_IsObject(item_array))
												{
													cJSON *values = cJSON_GetObjectItemCaseSensitive(item_array, "values");
													if (cJSON_IsArray(values))
													{
														cJSON *item_array_array = NULL;
														cJSON_ArrayForEach(item_array_array, values)
														{
															cJSON *name = cJSON_GetObjectItemCaseSensitive(item_array_array, "name");
															if (cJSON_IsString(name) && name->valuestring != NULL)
															{
																if (strcmp(name->valuestring, "on") == 0)
																{
																	cJSON *value = cJSON_GetObjectItemCaseSensitive(item_array_array, "value");
																	if (cJSON_IsString(value) && value->valuestring != NULL)
																	{
																		for (auto it : m_readTagMap)
																		{
																			if (strcmp(value->valuestring, "b0") == 0)
																			{
																				it.second.value = 0;
																			}
																			else if (strcmp(value->valuestring, "b1") == 0)
																			{
																				it.second.value = 1;
																			}

																			it.second.updateTime = time(0);
																			saveFEPData(it.second);
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
						break;
					}
					case 231:
					{

						memset(m_log_buf_, 0, sizeof(m_log_buf_));
						sprintf(m_log_buf_, "[%s] INFO run() 收到设备的下发反馈 route[%d] mainRoute[%d] m_link[%d] line[%d]!",
								m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
						FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
						break;
					}
					default:
						// LOG(INFO) << buf_rec;
						break;
					}
				}
				cJSON_Delete(json);
				buf_rec.clear();
			}
		}
		else
		{
			tcpClient->Close();
			if (!tcpClient->InitConnect())
			{
				memset(m_log_buf_, 0, sizeof(m_log_buf_));
				sprintf(m_log_buf_, "[%s] ERROR run() socket初始化失败...... route[%d] mainRoute[%d] ip[%s] port[%d] m_link[%d] line[%d]!",
						m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, tcpClient->m_strServerIp.c_str(), tcpClient->m_uServerPort, m_link, __LINE__);
				FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			}
			if (tcpClient->m_socket == NULL)
			{
				memset(m_log_buf_, 0, sizeof(m_log_buf_));
				sprintf(m_log_buf_, "[%s] ERROR run() socket为空，尝试重连中...... route[%d] mainRoute[%d] m_link[%d] line[%d]!",
						m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
				FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			}
			else
			{

				memset(m_log_buf_, 0, sizeof(m_log_buf_));
				sprintf(m_log_buf_, "[%s] ERROR run() socket断开，尝试重连中...... route[%d] mainRoute[%d] m_link[%d] line[%d]!",
						m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
				FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			}
		}
	}

	if (m_pLinkInfo != NULL)
	{
		m_pLinkInfo->lastRxTime = (hUInt32)time(0);
	}
	// 路径保活,open赋0，接收处理完毕再赋当前时间值
	if (m_pRouteInfo)
	{
		m_pRouteInfo->lastDataOkTime = (hUInt32)time(0);
	}
}

void SCS_TCP_CGL_BSD::controlSend(string _id, int _val)
{
	std::string outchar = str_ctrl_bsd_1;
	outchar += std::to_string(_val);
	outchar += str_ctrl_bsd_2;
	if (!tcpClient->SendMsg(outchar.c_str()))
	{
		tcpClient->Close();
		tcpClient->InitConnect();
		if (!tcpClient->SendMsg(outchar.c_str()))
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] ERROR controlSend() 控制下发失败！line[%d]", m_pGroup->name, __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}
}

bool SCS_TCP_CGL_BSD::loadTag(map<string, TAGINFO> *_readMap, map<string, TAGINFO> *_writeMap)
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

bool SCS_TCP_CGL_BSD::readProtocolParam(string _param) // 获取协议参数
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
		case PARAM_TAG_NUM: // 包请求点个数
			m_poll_tag_num_ = atoi(strtmp.c_str());
			break;
		case PARAM_RESEND: // 重发次数
			m_resend_count_ = atoi(strtmp.c_str());
			break;
		case PARAM_RESPONED_TIME: // 响应时间
			m_poll_response_time_ = atoi(strtmp.c_str());
			break;
		case PARAM_LOG_LEVEL: // 日志等级
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

void SCS_TCP_CGL_BSD::procData()
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

bool SCS_TCP_CGL_BSD::sendCtrlCmd() // 处理控制命令
{
	if (m_writeTagMap.empty()) // 未配置控制点，直接返回
	{
		return false;
	}
	if (skipMutRouteOneFep())
	{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR sendCtrlCmd() BLOB 收到控制, 备路径不下发 return. line[%d]", m_pGroup->name, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		return false;
	} // 跳过备链路
	ctrl_head m_ctrl_head_;

	// yx控制
	if (m_ctrlInf.get(m_pRoute->group, CTRL_PRO_SETPOINT_YX, cmdBuf, FDC_CTRL_LEN) > 0) // 获取YX控制命令CTRL_PRO_SETPOINT_YX
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

		auto itWriteMap = m_writeTagMap.find(str_name);
		if (itWriteMap != m_writeTagMap.end())
		{
			itWriteMap->second.updateTime = (hUInt32)time(NULL);
			if (setpt.int32Value == 1)
			{
				itWriteMap->second.value = 1;
				controlSend(itWriteMap->second.param1, 1);
			}
			else
			{
				itWriteMap->second.value = 0;
				controlSend(itWriteMap->second.param1, 0);
			}

			m_send_flag = true;
			m_ctrlInf.addCmdAck(m_ctrl_head_.cmdId, CTRL_STATUS_NO_ACK); // 控制反馈FEP接口
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO sendCtrlCmd() YX控制下发成功 tag[%s] device[%s] control[%s] object_name[%s] value[%s] line[%d]", m_pGroup->name, itWriteMap->second.tagName.c_str(), itWriteMap->second.param1.c_str(), itWriteMap->second.param2.c_str(), setpt.pointName, std::to_string(itWriteMap->second.value).c_str(), __LINE__);
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

			controlSend(itWriteMap->second.param1, itWriteMap->second.value);

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

void SCS_TCP_CGL_BSD::saveFEPData(const TAGINFO &_tag)
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

void SCS_TCP_CGL_BSD::saveYxData(const TAGINFO &_tag)
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
		if (ECON_BIT_DISABLED(pYxDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pYxDat->quality, QUALITY_BIT_OV)) // 判断某位是否有效
		{
			hUInt8 oldVal = (hUInt8)pYxDat->val;
			bool settime = CALL_DATA_TIME(_tag.updateTime, pYxDat->updateTime) ? true : false; // 超过上送时间
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

void SCS_TCP_CGL_BSD::saveYcData(const TAGINFO &_tag)
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
		if (ECON_BIT_DISABLED(pYcDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pYcDat->quality, QUALITY_BIT_OV)) // 判断某位是否有效
		{
			hFloat oldVal = (hFloat)pYcDat->val;
			bool settime = CALL_DATA_TIME(_tag.updateTime, pYcDat->updateTime) ? true : false; // 超过上送时间
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

void SCS_TCP_CGL_BSD::saveKwhData(const TAGINFO &_tag)
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
		if (ECON_BIT_DISABLED(pKwhDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pKwhDat->quality, QUALITY_BIT_OV)) // 判断某位是否有效
		{
			hDouble oldVal = (hDouble)pKwhDat->val;
			bool settime = CALL_DATA_TIME(_tag.updateTime, pKwhDat->updateTime) ? true : false; // 超过上送时间
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

void SCS_TCP_CGL_BSD::saveStringData(const TAGINFO &_tag)
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
	// 修改编码 gbk转utf8
	//  char buf[1024];
	//  char rbuf[1024];
	//  memset(buf,0,1024);
	//  memset(rbuf,0,1024);
	//  memcpy(buf,string_data.val,FDC_STRING_LEN);

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
		if (ECON_BIT_DISABLED(pStringDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pStringDat->quality, QUALITY_BIT_OV)) // 判断某位是否有效
		{
			bool settime = CALL_DATA_TIME(_tag.updateTime, pStringDat->updateTime) ? true : false; // 超过上送时间
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
extern "C" TCP_CGL_BSD_EXPORT ECON::FDC::CProtocol *CreateProtocol()
{
	return (new SCS_TCP_CGL_BSD());
}

extern "C" TCP_CGL_BSD_EXPORT void DestroyProtocol(ECON::FDC::CProtocol *p)
{
	delete p;
}
