#include "dbconnect.h"
char global_logbuff[1024];

bool connectDB(MYSQL *_mysql)
{
	mysql_init(_mysql);
	if (mysql_real_connect(_mysql, "localhost", "sysadmin", "adminsys", "idpora", 3306, NULL, 0))
	{
		if (0 != mysql_query(_mysql, "SET CHARACTER_SET_RESULTS = GBK"))
		{
			// printlog(LOGE_DB, "[DB] ERROR connect() 设置字符集失败 line[%d]", __LINE__);
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
	sql = "SELECT ValueType AS tagType, OffsetNo AS offset, Param1, MaximumValue, MinimumValue, ValueRadio, ValueOffset, ValueDeaband, Name ,Param2,Param3,Param4 FROM cfg_view_point_protocol WHERE GroupNo ='" + strGroupNo + "' and OffsetNo is not null ORDER BY Param1;";
	if (0 != mysql_query(_mysql, sql.c_str()))
	{
		memset(global_logbuff, 0, sizeof(global_logbuff));
		sprintf(global_logbuff, "[DB] ERROR getTag() 查询sql[%s]失败 line[%d]!", sql.c_str(), __LINE__);
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
		sprintf(global_logbuff, "[DB] ERROR getTag() 不存在记录 sql[%s] line[%d]!", sql.c_str(), __LINE__);
		FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
		closeDB(_mysql, (char *)_groupName, (char *)_linkName);
		return false;
	}
	_tagMap->clear();
	//添加点信息
	while ((row = mysql_fetch_row(res)))
	{
		TAGINFO temp_tag;
		temp_tag.valuetype = atoi(row[0]);
		temp_tag.offset = atoi(row[1]);
		temp_tag.param1 = row[2];
		temp_tag.maxValue = atoi(row[3]);
		temp_tag.minValue = atoi(row[4]);
		temp_tag.valueRadio = atoi(row[5]);
		temp_tag.valueOffset = atoi(row[6]);
		temp_tag.deaband = atoi(row[7]);
		temp_tag.tagName = row[8];
		temp_tag.param2 = row[9];
		temp_tag.param3 = row[10];
		temp_tag.param4 = row[11];
		if (temp_tag.tagName == "")
		{
			continue;
		}
		_tagMap->insert(pair<string, TAGINFO>(temp_tag.tagName, temp_tag));
		memset(global_logbuff, 0, sizeof(global_logbuff));
		sprintf(global_logbuff, "[DB] INFO getTag() TagInfo tagName:%s offset:%d valueType:%d maxValue:%d minValue:%d valueRadio:%d valueOffset:%d deaband:%d Param1:%s Param2:%s Param3:%s Param4:%s line[%d]!",
				temp_tag.tagName.c_str(), temp_tag.offset, temp_tag.valuetype, temp_tag.maxValue, temp_tag.minValue, temp_tag.valueRadio, temp_tag.valueOffset, temp_tag.deaband, temp_tag.param1.c_str(), temp_tag.param2.c_str(), temp_tag.param3.c_str(), temp_tag.param4.c_str(), __LINE__);
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
		sprintf(global_logbuff, "[DB] ERROR getCmd() 查询sql[%s]失败 line[%d]!", sql.c_str(), __LINE__);
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
		sprintf(global_logbuff, "[DB] ERROR getCmd() 不存在记录 sql[%s] line[%d]!", sql.c_str(), __LINE__);
		FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
		closeDB(_mysql, (char *)_groupName, (char *)_linkName);
		return false;
	}
	_tagMap->clear();
	//添加点信息
	while ((row = mysql_fetch_row(res)))
	{
		TAGINFO temp_tag;
		temp_tag.valuetype = atoi(row[0]);
		// temp_tag.offset = atoi(row[1]);
		temp_tag.param1 = row[1];
		temp_tag.maxValue = atoi(row[2]);
		temp_tag.minValue = atoi(row[3]);
		temp_tag.valueRadio = atoi(row[4]);
		temp_tag.valueOffset = atoi(row[5]);
		temp_tag.deaband = atoi(row[6]);
		temp_tag.tagName = row[7];
		temp_tag.param2 = row[8];
		if (temp_tag.tagName == "")
		{
			continue;
		}
		_tagMap->insert(pair<string, TAGINFO>(temp_tag.tagName, temp_tag));
		memset(global_logbuff, 0, sizeof(global_logbuff));
		sprintf(global_logbuff, "[DB] INFO getCmd() TagInfo name:%s offset:%d valueType:%d maxValue:%d minValue:%d valueRadio:%d valueOffset:%d deaband:%d Param1:%s Param2:%s line[%d]!",
				temp_tag.tagName.c_str(), temp_tag.offset, temp_tag.valuetype, temp_tag.maxValue, temp_tag.minValue, temp_tag.valueRadio, temp_tag.valueOffset, temp_tag.deaband, temp_tag.param1.c_str(), temp_tag.param2.c_str(), __LINE__);
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
	sprintf(global_logbuff, "[DB] INFO closeDB() 关闭数据库 ip[localhost] port[3306] line[%d]", __LINE__);
	FepLog::writelog((char *)_groupName, (char *)_linkName, global_logbuff, FEP_LOG::PKGDATA);
	return true;
}