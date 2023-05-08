#include "dbconnect.h"
char global_logbuff[1024];

bool connectDB(MYSQL *_mysql)
{
	mysql_init(_mysql);
	if (mysql_real_connect(_mysql, "localhost", "sysadmin", "adminsys", "idpora", 3306, NULL, 0))
	{
		if (0 != mysql_query(_mysql, "SET CHARACTER_SET_RESULTS = GBK"))
		{
			// printlog(LOGE_DB, "[DB] ERROR connect() �����ַ���ʧ�� line[%d]", __LINE__);
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool getTag(MYSQL *_mysql, map<string, TAGINFO> *_tagMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
	if (!connectDB(_mysql))
	{
		return false;
	}
	string sql, strGroupNo;
	char groupbuf[16] = {0};
	sprintf(groupbuf, "%d", _groupNo);
	strGroupNo = groupbuf;
	sql = "SELECT ValueType AS tagType, OffsetNo AS offset, Param1 AS opcTagName, MaximumValue, MinimumValue, ValueRadio, ValueOffset, ValueDeaband, Name, Param2 FROM cfg_view_point_protocol WHERE GroupNo ='" + strGroupNo + "' and OffsetNo is not null ORDER BY tagType, offset;";
	if (0 != mysql_query(_mysql, sql.c_str()))
	{
		memset(global_logbuff, 0, sizeof(global_logbuff));
		sprintf(global_logbuff, "[DB] ERROR getTag() ��ѯsql[%s]ʧ�� line[%d]!", sql.c_str(), __LINE__);
		FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
		closeDB(_mysql, (char *)_groupName, (char *)_linkName);
		return false;
	}

	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	res = mysql_store_result(_mysql);
	if (NULL == res)
	{
		memset(global_logbuff, 0, sizeof(global_logbuff));
		sprintf(global_logbuff, "[DB] ERROR getTag() �����ڼ�¼ sql[%s] line[%d]!", sql.c_str(), __LINE__);
		FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
		closeDB(_mysql, (char *)_groupName, (char *)_linkName);
		return false;
	}
	_tagMap->clear();
	//��ӵ���Ϣ
	while ((row = mysql_fetch_row(res)))
	{
		TAGINFO temp_tag;
		temp_tag.valuetype = atoi(row[0]);
		temp_tag.offset = atoi(row[1]);
		temp_tag.name = row[2];
		temp_tag.maxValue = atoi(row[3]);
		temp_tag.minValue = atoi(row[4]);
		temp_tag.valueRadio = atoi(row[5]);
		temp_tag.valueOffset = atoi(row[6]);
		temp_tag.deaband = atoi(row[7]);
		temp_tag.pointName = row[8];
		temp_tag.dev = row[9];
		if (temp_tag.pointName == "")
		{
			continue;
		}
		_tagMap->insert(pair<string, TAGINFO>(temp_tag.pointName, temp_tag));
		memset(global_logbuff, 0, sizeof(global_logbuff));
		sprintf(global_logbuff, "[DB] INFO getTag() TagInfo name:%s offset:%d valueType:%d maxValue:%d minValue:%d valueRadio:%d valueOffset:%d deaband:%d  pointName:%s dev:%s  line[%d]!",
				temp_tag.name.c_str(), temp_tag.offset, temp_tag.valuetype, temp_tag.maxValue, temp_tag.minValue, temp_tag.valueRadio, temp_tag.valueOffset, temp_tag.deaband, temp_tag.pointName.c_str(), temp_tag.dev.c_str(), __LINE__);
		FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
	}

	mysql_free_result(res);
	closeDB(_mysql, (char *)_groupName, (char *)_linkName);
	if (_tagMap->empty())
	{
		return false;
	}
	return true;
}
bool getCmd(MYSQL *_mysql, map<string, TAGINFO> *_tagMap, const void *_groupName, hInt32 _groupNo, const void *_linkName)
{
	if (!connectDB(_mysql))
	{
		return false;
	}
	string sql, strGroupNo;
	char groupbuf[16] = {0};
	sprintf(groupbuf, "%d", _groupNo);
	strGroupNo = groupbuf;
	sql = "SELECT ValueType AS tagType, Param1 AS opcTagName, MaximumValue, MinimumValue, ValueRadio, ValueOffset, ValueDeaband ,`Name`, Param2 FROM cfg_view_cmd_protocol WHERE GroupNo ='" + strGroupNo + "'  ORDER BY tagType;";
	if (0 != mysql_query(_mysql, sql.c_str()))
	{
		memset(global_logbuff, 0, sizeof(global_logbuff));
		sprintf(global_logbuff, "[DB] ERROR getCmd() ��ѯsql[%s]ʧ�� line[%d]!", sql.c_str(), __LINE__);
		FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
		closeDB(_mysql, (char *)_groupName, (char *)_linkName);
		return false;
	}

	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	res = mysql_store_result(_mysql);
	if (NULL == res)
	{
		memset(global_logbuff, 0, sizeof(global_logbuff));
		sprintf(global_logbuff, "[DB] ERROR getCmd() �����ڼ�¼ sql[%s] line[%d]!", sql.c_str(), __LINE__);
		FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
		closeDB(_mysql, (char *)_groupName, (char *)_linkName);
		return false;
	}
	_tagMap->clear();
	//��ӵ���Ϣ
	while ((row = mysql_fetch_row(res)))
	{
		TAGINFO temp_tag;
		temp_tag.valuetype = atoi(row[0]);
		// temp_tag.offset = atoi(row[1]);
		temp_tag.name = row[1];
		temp_tag.maxValue = atoi(row[2]);
		temp_tag.minValue = atoi(row[3]);
		temp_tag.valueRadio = atoi(row[4]);
		temp_tag.valueOffset = atoi(row[5]);
		temp_tag.deaband = atoi(row[6]);
		temp_tag.pointName = row[7];
		temp_tag.dev = row[8];
		if (temp_tag.pointName == "")
		{
			continue;
		}
		_tagMap->insert(pair<string, TAGINFO>(temp_tag.pointName, temp_tag));
		memset(global_logbuff, 0, sizeof(global_logbuff));
		sprintf(global_logbuff, "[DB] INFO getCmd() TagInfo name:%s offset:%d valueType:%d maxValue:%d minValue:%d valueRadio:%d valueOffset:%d deaband:%d line[%d]!",
				temp_tag.name.c_str(), temp_tag.offset, temp_tag.valuetype, temp_tag.maxValue, temp_tag.minValue, temp_tag.valueRadio, temp_tag.valueOffset, temp_tag.deaband, __LINE__);
		FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
	}

	mysql_free_result(res);
	closeDB(_mysql, (char *)_groupName, (char *)_linkName);
	return true;
}

bool closeDB(MYSQL *_mysql, const void *_groupName, const void *_linkName)
{
	mysql_close(_mysql);
	memset(global_logbuff, 0, sizeof(global_logbuff));
	sprintf(global_logbuff, "[DB] INFO closeDB() �ر����ݿ� ip[localhost] port[3306] line[%d]", __LINE__);
	FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
	return true;
}