#include <iostream>
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4996)
#import "C:\\Program Files\\Common Files\\System\\ado\\msado15.dll"  no_namespace  rename ("EOF", "adoEOF")

using namespace std;

int main()
{
	freopen("out.txt", "w", stdout);
	CoInitialize(NULL);         //��ʼ��OLE/COM�⻷��
	//������ado����ʼ���ɹ�com��󣬾Ϳ���ʹ������ָ����
	_ConnectionPtr m_pConnection;
	_RecordsetPtr m_pRecordset;
	HRESULT hr;
	try
	{
		hr = m_pConnection.CreateInstance("ADODB.Connection");///����Connection����
   //     hr = m_pConnection.CreateInstance(__uuidof(Connection));//no_namespace�ĺô�

		if (SUCCEEDED(hr))
		{
	//	    hr = m_pConnection->Open("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=AdvCenter.mdb;", "", "", adModeUnknown);   //access2003(x86)
			hr = m_pConnection->Open("Provider=Microsoft.ACE.OLEDB.12.0;Data Source=AdvCenter.accdb","","",adModeUnknown);    //access2007(x64)
			cout<<"�ɹ��������ݿ�"<<endl;
		}
	}
	catch (_com_error e)///��׽�쳣
	{
		cout<<"�������ݿ�ʧ��!"<<endl;
		cout <<"������Ϣ:"<< e.ErrorMessage() << endl;
		return 0;
	}
	m_pRecordset.CreateInstance(__uuidof(Recordset)); //ʵ�������������   
//ִ��sql���   
	try
	{
		char sql[300];
		memset(sql, 0, 300);
		strcat(sql, "SELECT * FROM COVI");//��ȡ����
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
		//�����Ϊ�գ�����   
		if (m_pRecordset->BOF)
		{
			printf("��������Ϊ�գ�");
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
		cout << "���ݿ������Ϊ:" << endl;
		//�α궨λ����һ����¼
		m_pRecordset->MoveFirst();
		_variant_t var[2]; //�ӽ������ȡ�������ݷŵ�var��
		char* t1[2];
		while (!m_pRecordset->adoEOF)
		{
			var[0] = m_pRecordset->GetCollect("CO");//��ȡ������
			if (var[0].vt != VT_NULL)
			{
				t1[0] = _com_util::ConvertBSTRToString((_bstr_t)var[0]);
			}
			printf(t1[0]);
			printf("\t\t");
			var[1] = m_pRecordset->GetCollect("VI");//��ȡ������
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

	//�˳�����ʱ�Ĵ���   
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