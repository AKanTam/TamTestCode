#include "SCS_HTTP_H3AP.h"

using namespace ECON::FDC;

#define  LOGE_HTTP_H3AP	31002

//gbkתtf8
#include "iconv.h"
int gbk_to_utf8(char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen)
{
	iconv_t cd;
	if ((cd = iconv_open("utf-8", "gbk")) == 0)
	{
		return  -1;
	}
	memset(destbuf, 0, destlen);
	char **source = &sourcebuf;
	char **dest = &destbuf;
	if (-1 == iconv(cd, source, &sourcelen, dest, &destlen))
	{
		return  -1;
	}
	iconv_close(cd);
	return  0;
}
//utf-8תbk
int utf8_to_gbk(char *sourcebuf, size_t sourcelen, char *destbuf, size_t destlen)
{
	iconv_t cd;
	if ((cd = iconv_open("gbk", "utf-8")) == 0)
	{
		return  -1;
	}
	memset(destbuf, 0, destlen);
	char **source = &sourcebuf;
	char **dest = &destbuf;
	if (-1 == iconv(cd, source, &sourcelen, dest, &destlen))
	{
		return  -1;
	}
	iconv_close(cd);
	return  0;
}

SCS_HTTP_H3AP::SCS_HTTP_H3AP():m_logd_id_(LOGE_HTTP_H3AP)
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

SCS_HTTP_H3AP::~SCS_HTTP_H3AP()
{

}

bool SCS_HTTP_H3AP::isOpen()const
{
	return m_isOpen;
}
bool SCS_HTTP_H3AP::open()
{
	m_isOpen = CProtocol::open(); 
	sprintf(m_log_buf_,"%s", m_pLink->remoteAddr);
	m_remoteAddress = m_log_buf_;
	if (!m_isOpen)    
	{
	
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] ERROR CProtocol::open ʧ�� route[%d] mainRoute[%d] m_link[%d] logd[%d] line[%d]!",
				m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), __LINE__);
			//FepLog::writelog( m_pGroup->name, m_pLink->name, "", FEP_LOG::PKGDATA );
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
			close(); 

		ec_sleep(OPEN_FAILED_SLEEP_TIME);
		return m_isOpen;
	}
	else {
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO open() �򿪹�Լ m_curRoute[%d] mainRoute[%d] m_link[%d] logd[%d] line[%d]!",
			m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), __LINE__);
		//FepLog::writelog(m_pGroup->name, m_pLink->name, "", FEP_LOG::PKGDATA);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

		if(!m_isdevlist)
		{
			 if(!m_httpc.iniHttp(m_remoteAddress,m_devOnlineMap)){
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
		    sprintf(m_log_buf_, "[%s] INFO open() GETת���豸�б�ʧ�� m_curRoute[%d] mainRoute[%d] m_link[%d] logd[%d] size[%s] line[%d]!  ",
			    m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), sizeof(listBuf) ,__LINE__);
		    FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

			 }

            memset(m_log_buf_, 0, sizeof(m_log_buf_));
		    sprintf(m_log_buf_, "[%s] INFO open() GET��ȡ�豸�б� m_curRoute[%d] mainRoute[%d] m_link[%d] logd[%d] size[%d] line[%d]! ",
			    m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, (m_logd_id_ + m_link), sizeof(listBuf) ,__LINE__);
		    FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);


		     m_isdevlist= true;
		}
	}

	if (!m_isload)   
	{
		if (!loadTag(&m_readTagMap))
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] ERROR open() ���ݿ� �������ݿ�״̬��/���Ƶ�ʧ�� line[%d]!", m_pGroup->name, __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		
			close();      //����������Ϣ
			ec_sleep(OPEN_FAILED_SLEEP_TIME);
			m_isOpen = false;
			return false;
		}

		//����feature
		readProtocolParam(getFeature());

		m_isload = true;
	}

	ec_gettimeofday(&m_send_time_, NULL);
	m_pLinkInfo->state = LINK_STATE_UP;    //��·״̬
	m_isOpen = true;

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO open() �򿪹�Լ�ɹ� route[%d] mainRoute[%d] m_link[%d] line[%d]!",
		m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	

	return true;
}

void SCS_HTTP_H3AP::close() 
{
	DoneRead();   
	//ͨ�ô���
	CProtocol::close();
	m_isOpen = false;

	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO close() �رչ�Լ�ɹ� route[%d] mainRoute[%d] m_link[%d] line[%d]!",
		m_pGroup->name, m_curRoute, (hBool)m_pRouteInfo->mainFlag, m_link, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
}

void SCS_HTTP_H3AP::run()
{
	Send_CallAllData();
	procData();
	//m_recv_time_ = ACE_OS::gettimeofday();				//��¼����ʱ��
	if ( m_pLinkInfo != NULL )
	{
		m_pLinkInfo->lastRxTime = (hUInt32)time(0);
	}
	//·������,open��0�����մ�������ٸ���ǰʱ��ֵ
	if ( m_pRouteInfo)
	{
		m_pRouteInfo->lastDataOkTime = (hUInt32)time(0);
	}
	Sleep(1000*60);
}

bool SCS_HTTP_H3AP::loadTag(map<string, TAGINFO>*_readMap)
{
	if (!getTag(&m_mysql,_readMap, m_pGroup->name, m_pRoute->group, m_pLink->name)) {
		return false;
	}
	return true;
}

bool SCS_HTTP_H3AP::readProtocolParam(string _param) //��ȡЭ�����
{
	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO readProtocolParam() ��ȡ��Э�����[%s]  line[%d]",
		m_pGroup->name, _param.c_str(), __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

	int index = 1;
	bool ret = true;
	const char* di = ",";
	char*p = strtok((char*)_param.c_str(), di);
	while (p) {
		std::string strtmp = p;
		switch (index) {
			case PARAM_TAG_NUM://����������
				m_poll_tag_num_ = atoi(strtmp.c_str());
				break;
			case PARAM_RESEND://�ط�����
				m_resend_count_ = atoi(strtmp.c_str());
				break;
			case PARAM_RESPONED_TIME://��Ӧʱ�� 
				m_poll_response_time_ = atoi(strtmp.c_str());
				break;
			case PARAM_LOG_LEVEL://��־�ȼ�
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
	sprintf(m_log_buf_, "[%s] INFO readProtocolParam() ��ȡ��Э�����[%s] m_poll_tag_num_:%d m_resend_count_:%d m_poll_response_time_:%d m_log_level_:%d line[%d]",
		m_pGroup->name, _param.c_str(), m_poll_tag_num_, m_resend_count_, m_poll_response_time_, m_log_level_,__LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

	return ret;

}

void SCS_HTTP_H3AP::ProtocolTX()//����������
{
	if (!checkSend())
	{
		return;
	}
	
	Send_CallAllData();

}
void SCS_HTTP_H3AP::ProtocolRX()//��������
{

	procData();


}

bool SCS_HTTP_H3AP::checkSend()
{
	if (!m_send_flag)
	{
		m_try_send_times_ = 0;
		return true;
	}
	timeval cur_time;
	ec_gettimeofday(&cur_time, NULL);
	hUInt32 diff = (cur_time.tv_sec - m_send_time_.tv_sec) * 1000 + (cur_time.tv_usec - m_send_time_.tv_usec) / 1000;//��ȷ������

	if (diff>m_poll_response_time_) {
		m_try_send_times_++;
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO checkSend() �ѷ������ݵ�δ���յ��ذ� �ط�[%d] diff[%d]>poll_time[%d] line[%d]",
			m_pGroup->name, m_try_send_times_, diff, m_poll_response_time_, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		return true;
	}
	return false;
}


void SCS_HTTP_H3AP::Send_CallAllData()//��ѯ��������
{
	//--�·�---
	// if (strlen(m_task_vec[m_current_task].jsonBuf)>remainLength())
	// {
	// 	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	// 	sprintf(m_log_buf_, "[%s] ERROR Send_CallAllData() ���ͻ������ռ䲻�� line[%d]", m_pGroup->name, __LINE__);
	// 	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	// 	return;
	// }

	m_httpc.iniHttp(m_remoteAddress,m_devOnlineMap);
	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO Send_CallAllData() ��ѯ������ line[%d]", m_pGroup->name, __LINE__);
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
}

void SCS_HTTP_H3AP::procData()
{
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] INFO procData()IS READRESPONSE line[%d]", m_pGroup->name, __LINE__);
		if(LOG_5_NONE<=m_log_level_)
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	
		list<TAGINFO> taglist;
		m_httpc.getPointData(m_devOnlineMap,taglist,m_readTagMap);

		if (taglist.empty())
		{
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] ERROR procData() ��ѯ����ϢΪ�� line[%d]", m_pGroup->name, __LINE__);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

		}
		list<TAGINFO>::iterator itreadlist = taglist.begin();
		for (; itreadlist!= taglist.end(); itreadlist++)
		{
			m_readTagMap[itreadlist->pointName].value = itreadlist->value;
			m_readTagMap[itreadlist->pointName].value_s = itreadlist->value_s;
			m_readTagMap[itreadlist->pointName].quality = itreadlist->quality;
			m_readTagMap[itreadlist->pointName].updateTime = itreadlist->updateTime;

			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO procData() name:%s  value:%g type:%d value_s:%s quality:%d updateTime:%d  line[%d]", m_pGroup->name, itreadlist->pointName.c_str(),itreadlist->value,m_readTagMap[itreadlist->pointName].valuetype,itreadlist->value_s.c_str(),itreadlist->quality,itreadlist->updateTime,__LINE__);
			if(LOG_3_NONE<=m_log_level_)
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);

			//����ʵʱ��
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

}

void SCS_HTTP_H3AP::saveYxData(TAGINFO _tag)
{
	FDC_YX_DATA     yx_data;
	yx_data.updateTime = (hUInt32)time(NULL);
	
	timeval updateTimeVal;
	ec_gettimeofday(&updateTimeVal, NULL);
	yx_data.updateTimeMs = updateTimeVal.tv_usec / 1000;
	yx_data.val = (hUInt8)_tag.value;
	FDC_YX_DATA	*pYxDat = m_dataInf.yxData(m_curRoute, _tag.offset);
	if (NULL != pYxDat) {
		if (ECON_BIT_DISABLED(pYxDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pYxDat->quality, QUALITY_BIT_OV)) //�ж�ĳλ�Ƿ���Ч
		{
			hUInt8 oldVal = (hUInt8)pYxDat->val;
			bool settime = CALL_DATA_TIME(_tag.updateTime, pYxDat->updateTime) ? true : false;	//��������ʱ��
			if (oldVal != (hUInt8)yx_data.val || settime) {
				yx_data.quality = pYxDat->quality; //quality���Բ���

				if (m_dataInf.setYx(m_curRoute, _tag.offset, yx_data, FALSE)) {
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] RECV <<<< saveYxData()  offset[%d] name[%s] val[%d] quality[%08x] updatetime[%d] line[%d]",
						m_pGroup->name,
						_tag.offset,
						_tag.pointName.c_str(),
						(hUInt8)yx_data.val,
						(hUInt32)yx_data.quality,
						(hUInt32)yx_data.updateTime,
						__LINE__);
					if(LOG_3_NONE<=m_log_level_)
					FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
				else {
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] ERROR saveYxData()  ����ʧ�� offset[%d] name[%s] val[%d] quality[%08x] updatetime[%d] line[%d]",
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
		else {
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO saveYxData() �˹����� ������ ·��[%d], ƫ��[%d], ֵ[%d]",
				m_pGroup->name, m_curRoute, _tag.offset, (hInt32)yx_data.val);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}
	else {
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR saveYxData() ��ȡpYxDatʧ�� route[%d] offset[%d] line[%d]", m_pGroup->name, m_curRoute, _tag.offset, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
}

void SCS_HTTP_H3AP::saveYcData(TAGINFO _tag)
{
	FDC_YC_DATA     yc_data;
	yc_data.updateTime = (hUInt32)time(NULL);

	timeval updateTimeVal;
	ec_gettimeofday(&updateTimeVal, NULL);
	yc_data.updateTimeMs = updateTimeVal.tv_usec / 1000;
	yc_data.val = (hFloat)_tag.value;
	yc_data.val = (hFloat)yc_data.val*_tag.valueRadio + _tag.valueOffset;
	FDC_YC_DATA	*pYcDat = m_dataInf.ycData(m_curRoute, _tag.offset);
	if (NULL != pYcDat) {
		if (ECON_BIT_DISABLED(pYcDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pYcDat->quality, QUALITY_BIT_OV)) //�ж�ĳλ�Ƿ���Ч
		{
			hFloat oldVal = (hFloat)pYcDat->val;
			bool settime = CALL_DATA_TIME(_tag.updateTime, pYcDat->updateTime) ? true : false;	//��������ʱ��
			if (oldVal != (hFloat)yc_data.val || settime) {
				yc_data.quality = pYcDat->quality; //quality���Բ���

				if (m_dataInf.setYc(m_curRoute, _tag.offset, yc_data)) {
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] RECV <<<< saveYcData()  offset[%d] name[%s] val[%f] quality[%08x] updatetime[%d] line[%d]",
						m_pGroup->name,
						_tag.offset,
						_tag.pointName.c_str(),
						(hFloat)yc_data.val,
						(hUInt32)yc_data.quality,
						(hUInt32)yc_data.updateTime,
						__LINE__);
						if(LOG_3_NONE<=m_log_level_)
					FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
				else {
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] ERROR saveYcData()  ����ʧ�� offset[%d] name[%s] val[%f] quality[%08x] updatetime[%d] line[%d]",
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
		else {
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO saveYcData() �˹����� ������ ·��[%d], ƫ��[%d], ֵ[%f]",
				m_pGroup->name, m_curRoute, _tag.offset, (hFloat)yc_data.val);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}
	else {
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR saveYcData() ��ȡpYcDatʧ�� route[%d] offset[%d] line[%d]",  m_pGroup->name, m_curRoute, _tag.offset, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
}

void SCS_HTTP_H3AP::saveKwhData(TAGINFO _tag)
{
	FDC_KWH_DATA     kwh_data;
	kwh_data.updateTime = (hUInt32)time(NULL);

	timeval updateTimeVal;
	ec_gettimeofday(&updateTimeVal, NULL);
	kwh_data.updateTimeMs = updateTimeVal.tv_usec / 1000;
	kwh_data.val = (hDouble)_tag.value;
	kwh_data.val = (hDouble)kwh_data.val*_tag.valueRadio + _tag.valueOffset;
	FDC_KWH_DATA	*pKwhDat = m_dataInf.kwhData(m_curRoute, _tag.offset);
	if (NULL != pKwhDat) {
		if (ECON_BIT_DISABLED(pKwhDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pKwhDat->quality, QUALITY_BIT_OV)) //�ж�ĳλ�Ƿ���Ч
		{
			hDouble oldVal = (hDouble)pKwhDat->val;
			bool settime = CALL_DATA_TIME(_tag.updateTime, pKwhDat->updateTime) ? true : false;	//��������ʱ��
			if (oldVal != (hDouble)kwh_data.val || settime) {
				kwh_data.quality = pKwhDat->quality; //quality���Բ���

				if (m_dataInf.setKwh(m_curRoute, _tag.offset, kwh_data)) {
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] RECV <<<< saveKwhData()  offset[%d] name[%s] val[%g] quality[%08x] updatetime[%d] line[%d]",
						m_pGroup->name,
						_tag.offset,
						_tag.pointName.c_str(),
						(hDouble)kwh_data.val,
						(hUInt32)kwh_data.quality,
						(hUInt32)kwh_data.updateTime,
						__LINE__);
						if(LOG_3_NONE<=m_log_level_)
					FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
				else {
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] ERROR saveKwhData()  ����ʧ�� offset[%d] name[%s] val[%g] quality[%08x] updatetime[%d] line[%d]",
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
		else {
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO saveKwhData() �˹����� ������ ·��[%d], ƫ��[%d], ֵ[%g]",
				m_pGroup->name, m_curRoute, _tag.offset, (hDouble)kwh_data.val);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}
	else {
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR saveKwhData() ��ȡpKwhDatʧ�� route[%d] offset[%d] line[%d]",  m_pGroup->name, m_curRoute, _tag.offset, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}
}

void SCS_HTTP_H3AP::saveStringData(TAGINFO _tag)
{
	FDC_STRING_DATA     string_data;
	string_data.updateTime = (hUInt32)time(NULL);

	timeval updateTimeVal;
	ec_gettimeofday(&updateTimeVal, NULL);
	string_data.updateTimeMs = updateTimeVal.tv_usec / 1000;

	memset(string_data.val, 0, FDC_STRING_LEN);
	strncpy(string_data.val, _tag.value_s.c_str(), FDC_STRING_LEN);
	
	memset(m_log_buf_, 0, sizeof(m_log_buf_));
	sprintf(m_log_buf_, "[%s] INFO saveStringData()   ·��[%d], ƫ��[%d], ֵ[%s]",
	m_pGroup->name, m_curRoute, _tag.offset, string_data.val);
	if(LOG_5_NONE<=m_log_level_)
	FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	//�޸ı��� gbkתutf8
	// char buf[1024];
	// char rbuf[1024];
	// memset(buf,0,1024);
	// memset(rbuf,0,1024);
	// memcpy(buf,string_data.val,FDC_STRING_LEN);
	
	// gbk_to_utf8(buf,1024,rbuf,1024);
	// memcpy(string_data.val,rbuf,FDC_STRING_LEN);
	
	// memset(m_log_buf_, 0, sizeof(m_log_buf_));
	// sprintf(m_log_buf_, "[%s] INFO saveStringData()  GBKתUTF-8 gbk buf��%s utf-8 rbuf:%s  ·��[%d], ƫ��[%d], ֵ[%s]",
	// m_pGroup->name, buf,rbuf, m_curRoute,_tag.offset, string_data.val);
	// if(LOG_5_NONE<=m_log_level_)
	// FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	//------------------------------------------------------------------------
	
	FDC_STRING_DATA	*pStringDat = m_dataInf.stringData(m_curRoute, _tag.offset);
	if (NULL != pStringDat) {
		if (ECON_BIT_DISABLED(pStringDat->quality, QUALITY_BIT_MS) && ECON_BIT_DISABLED(pStringDat->quality, QUALITY_BIT_OV)) //�ж�ĳλ�Ƿ���Ч
		{
			bool settime = CALL_DATA_TIME(_tag.updateTime, pStringDat->updateTime) ? true : false;	//��������ʱ��
			if (strcmp(string_data.val,pStringDat->val)!=0|| settime) {
				string_data.quality = pStringDat->quality; //quality���Բ���

				if (m_dataInf.setString(m_curRoute, _tag.offset, string_data)) {
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] RECV <<<< saveStringData()  offset[%d] name[%s] val[%s] quality[%08x] updatetime[%d] line[%d]",
						m_pGroup->name,
						_tag.offset,
						_tag.pointName.c_str(),
						string_data.val,
						(hUInt32)string_data.quality,
						(hUInt32)string_data.updateTime,
						__LINE__);
						if(LOG_3_NONE<=m_log_level_)
					FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
				}
				else {
					memset(m_log_buf_, 0, sizeof(m_log_buf_));
					sprintf(m_log_buf_, "[%s] ERROR saveStringData()  ����ʧ�� offset[%d] name[%s] val[%s] quality[%08x] updatetime[%d] line[%d]",
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
		else {
			memset(m_log_buf_, 0, sizeof(m_log_buf_));
			sprintf(m_log_buf_, "[%s] INFO saveStringData() �˹����� ������ ·��[%d], ƫ��[%d], ֵ[%s]",
				m_pGroup->name, m_curRoute, _tag.offset, string_data.val);
			FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
		}
	}
	else {
		memset(m_log_buf_, 0, sizeof(m_log_buf_));
		sprintf(m_log_buf_, "[%s] ERROR saveStringData() ��ȡpStringDatʧ�� route[%d] offset[%d] line[%d]", m_pGroup->name, m_curRoute, _tag.offset, __LINE__);
		FepLog::writelog(m_pGroup->name, m_pLink->name, m_log_buf_, FEP_LOG::PKGDATA);
	}

}

// //----------------------------------------------------------------------------
extern "C" HTTP_H3AP_EXPORT ECON::FDC::CProtocol * CreateProtocol()
{
	return (new SCS_HTTP_H3AP());
}

extern "C" HTTP_H3AP_EXPORT void DestroyProtocol(ECON::FDC::CProtocol *p)
{
	delete p;
}
