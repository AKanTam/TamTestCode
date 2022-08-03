#include <iostream>
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4996)
#import "C:\\Program Files\\Common Files\\System\\ado\\msado15.dll"  no_namespace  rename ("EOF", "adoEOF")

using namespace std;

int main()
{
	freopen("out.txt", "w", stdout);
	CoInitialize(NULL);         //初始化OLE/COM库环境
	//在引入ado并初始化成功com库后，就可以使用智能指针了
	_ConnectionPtr m_pConnection;
	_RecordsetPtr m_pRecordset;
	HRESULT hr;
	try
	{
		hr = m_pConnection.CreateInstance("ADODB.Connection");///创建Connection对象
   //     hr = m_pConnection.CreateInstance(__uuidof(Connection));//no_namespace的好处

		if (SUCCEEDED(hr))
		{
	//	    hr = m_pConnection->Open("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=AdvCenter.mdb;", "", "", adModeUnknown);   //access2003(x86)
			hr = m_pConnection->Open("Provider=Microsoft.ACE.OLEDB.12.0;Data Source=AdvCenter.accdb","","",adModeUnknown);    //access2007(x64)
			cout<<"成功连接数据库"<<endl;
		}
	}
	catch (_com_error e)///捕捉异常
	{
		cout<<"连接数据库失败!"<<endl;
		cout <<"错误信息:"<< e.ErrorMessage() << endl;
		return 0;
	}
	m_pRecordset.CreateInstance(__uuidof(Recordset)); //实例化结果集对象   
//执行sql语句   
	try
	{
		char sql[300];
		memset(sql, 0, 300);
		strcat(sql, "SELECT * FROM COVI");//读取表名
		m_pRecordset->Open(sql, m_pConnection.GetInterfacePtr(), adOpenDynamic, adLockOptimistic, adCmdText);
	}
	catch (_com_error* e)
	{
		wprintf(e->ErrorMessage());
		if (m_pConnection->State)
		{
			m_pConnection->Close();
			m_pConnection = NULL;
		}
		CoUninitialize();
		return 0;
	}
	try
	{
		//若结果为空，结束   
		if (m_pRecordset->BOF)
		{
			printf("表内数据为空！");
			if (m_pConnection->State)
			{
				m_pRecordset->Close();
				m_pRecordset = NULL;
				m_pConnection->Close();
				m_pConnection = NULL;
			}
			CoUninitialize();
			return 0;
		}
		cout << "---------------------------------" << endl;
		cout << "数据库的内容为:" << endl;
		//游标定位到第一条记录
		m_pRecordset->MoveFirst();
		_variant_t var[2]; //从结果集中取出的数据放到var中
		char* t1[2];
		while (!m_pRecordset->adoEOF)
		{
			var[0] = m_pRecordset->GetCollect("CO");//读取数据名
			if (var[0].vt != VT_NULL)
			{
				t1[0] = _com_util::ConvertBSTRToString((_bstr_t)var[0]);
			}
			printf(t1[0]);
			printf("\t\t");
			var[1] = m_pRecordset->GetCollect("VI");//读取数据名
			if (var[1].vt != VT_NULL)
			{
				t1[1] = _com_util::ConvertBSTRToString((_bstr_t)var[1]);
			}
			printf(t1[1]);
			printf("\n");
			m_pRecordset->MoveNext();
		}
	}
	catch (_com_error* e)
	{
		wprintf(e->ErrorMessage());
	}

	//退出程序时的处理   
	if (m_pConnection->State)
	{
		m_pRecordset->Close();
		m_pRecordset = NULL;
		m_pConnection->Close();
		m_pConnection = NULL;
	}
	CoUninitialize();
	cout << "---------------------------------" << endl;

	fclose(stdout);

	return 0;
}