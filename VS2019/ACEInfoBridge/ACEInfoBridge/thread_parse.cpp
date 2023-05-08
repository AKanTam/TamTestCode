// thread_parse.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ACEInfoBridge.h"
#include "opc_cmd.h"
#include "thread_parse.h"
#include <fstream>
#define LOGMSG

extern CCriticalSection g_RecvCS;
extern CCriticalSection g_SendCS;
opc_cmd* opc;
extern HWND g_hWnd;


void thread_parse::logMsg(const char* msg, int length, const char* filename) {
#ifdef LOGMSG
	time_t t = time(0);
	char time_full[64], date[64];
	memset(time_full, 0, 64);
	memset(date, 0, 64);
	strftime(time_full, sizeof(time_full), "%Y-%m-%d %X\n", localtime(&t));
	strftime(date, sizeof(time_full), "%Y-%m-%d", localtime(&t));
	CString name = filename, nowTime = time_full;
	name += date;
	name += ".txt";
	ofstream outFile(name, ios::app);
	outFile << nowTime;
	for (int i = 0; i < length; i++) {
		outFile << (int)msg[i];
		outFile << " ";
	}
	outFile << "\n";
	for (int i = 0; i < length; i++) {
		outFile << msg[i];
	}
	outFile << "\n";
	outFile << "\n";
	outFile.close();
#endif // LOGMSG
}
// thread_parse

IMPLEMENT_DYNCREATE(thread_parse, CWinThread)

thread_parse::thread_parse()
{
	opc = new opc_cmd();
}

thread_parse::~thread_parse()
{
}

BOOL thread_parse::InitInstance()
{
	// TODO: �ڴ�ִ���������̳߳�ʼ��
	return TRUE;
}

int thread_parse::ExitInstance()
{
	// TODO: �ڴ�ִ���������߳�����
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(thread_parse, CWinThread)
		ON_THREAD_MESSAGE(WM_StartParseThread,StartThread)
END_MESSAGE_MAP()


// thread_parse ��Ϣ�������

void thread_parse::StartThread(WPARAM wParam,LPARAM lParam)
{
	while(true){
		MSG msg;
		if(::PeekMessage(&msg,NULL,WM_MINMSG,WM_MAXMSG,PM_NOREMOVE))
		{
			if (msg.message == WM_StopParseThread)
			{
				break;
			}
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		if(g_socket_RecvQueue.size() != 0){
			NetworkMsg* input;
			NetworkMsg* output;
			g_RecvCS.Lock();
			input = g_socket_RecvQueue.front();
			char* msg_in = input->getMsg();
			TRACE("Got opc command %s\n", msg_in);
			g_socket_RecvQueue.pop_front();
			if (g_socket_RecvQueue.size() == 0) {
				g_socket_RecvQueue.shrink_to_fit();
			}
			PostMessage(g_hWnd, WM_REDRAW, NULL, NULL);
			g_RecvCS.Unlock();
			logMsg(msg_in, input->getLength(), "Recv");
			output = DoParse(msg_in, input->getLength());
			delete msg_in;
			delete input;
			if (output->getLength() == 0) {
				delete output;
				continue;
			}
			msg_in = output->getMsg();
			logMsg(msg_in, output->getLength(), "Send");
			delete msg_in;
			g_SendCS.Lock();
			g_socket_SendQueue.push_back(output);
			PostMessage(g_hWnd, WM_REDRAW, NULL, NULL);
			g_SendCS.Unlock();
		}
		else {
			Sleep(100);
		}
	}
	delete opc;
	::AfxEndThread(2000);
}


NetworkMsg* thread_parse::DoParse(char* input, int length)	//input�����ڸ÷�����delete��������Զ�����
{															//�÷��������յ����ݵ�ʱ�򱻵���
	NetworkMsg* ret = new NetworkMsg("ERRNCOMP",9);			//ʹ�ô˱����洢����ֵ.����:ret.setMsg(char* message, int length); ����Ḵ�����ݣ�����������Ҫ����set֮�����������message���ڴ档

	////////////////////////////////////////////////////
	////////////////////////////////////////////////////
	///////            ������������            ///////
	////////////////////////////////////////////////////
	////////////////////////////////////////////////////

	//
	//	1.������д�봦���յ���socket���Ĵ���.socket��Ϊinput,����Ϊlength.
	//
	//


	//  2.������д���ȡʵʱֵ.���Ե���opc����.
	//OPC���ܵ������ã����ĸ���������ʹ�á�opc->rt_query(CString tag)Ϊʵʱֵ�����ѯ������query_res�ṹ��
	//opc->rt_query_grp(deque<CString> tag_names)Ϊʵʱֵ����ѯ������deque<CString>���ͣ�˳��ʹ����deque��ͬ������в�ѯ���������ݻ��ڶ�Ӧλ����Wռλ��
	//opc->write_value(write_para parameters)Ϊд��ֵ������ΪCString���͵ĳɹ�����ʧ����Ϣ��
	//opc->send_msg(CString msg)Ϊֱ��ִ���������ֵΪԭʼ��CString�����ַ�����
	/*
	���Ӵ��룺
	CString tagName = "TEST_AM001";
	int len = 0;
	query_res rt1;
	deque<CString> rt2;
	deque<CString> rt3;
	write_para rt4;
	CString res;
	try {
		switch(rand()%4){
		case 0:
			rt1 = opc->rt_query(tagName);							//��Ϊ����ֵ,���������غʹ���Ľṹ������·�ע��
			ret->setMsg(rt1.tag_value, rt1.tag_value.GetLength());
			break;
		case 1:
			rt2.push_back(tagName);
			rt2.push_back("TEST_AM002");
			rt2.push_back("TEST_AM003");
			rt3 = opc->rt_query_grp(rt2);
			while(rt3.size() != 0){
				res += rt3.front();
				res += " OAO ";
				rt3.pop_front();
			}
			res += "Group read";
			ret->setMsg(res, res.GetLength());
			break;
		case 2:
			ret->setMsg("", 0);
			break;
		case 3:
			rt4.user = "a";
			rt4.pass = "a";
			rt4.node = "supcon-zxt";
			rt4.tag = "TEST_AM002";
			rt4.value.Format("%d",rand());
			res = opc->write_value(rt4);
			res += " Wrt TEST_AM002:";
			res += rt4.value;
			ret->setMsg(res, res.GetLength());
			break;
		default:
			break;
		}
	} catch (const std::runtime_error& e) {
		ret->setMsg(e.what(), strlen(e.what()));							//�򵥵��쳣����
	}
	*/ 
	//���OPC�ܵ���������ʧ��/дʧ��/��ʧ�ܣ�,���׳�runtime error�쳣.��ʹ��try catch��Χ��ѯ��䣬�����д�����
	
	//   3.����Э��Է���ֵ�����жϲ�����Socket�ذ���װ.���װ��char*���͵�buffer����¼������ĳ��ȡ�
	
	
	//   4.����õĻذ�����ret��.����������/�����response���ڴ�ռ�,������Ḵ�����ݶ�����ֻ����ָ��.
	//   ������뽫��������ͳ�ȥ���뽫length���ó�0��������Զ������������
	/*
	���Ӵ���:
	ret->setMsg(response, length);
	*/
	return ret;
}

/*
---------------------------------------------------------------------------
								�ṹ˵��
---------------------------------------------------------------------------
struct query_res{
	CString res;				���ý�����ɹ�/ʧ�ܡ�ʧ�ܵĻ�����ȫ��Ϊ�ա�
	CString tag_type;			tag����
	CString tag_value;			tag��ֵ
	CString tag_quality;		tag������
};


struct write_para{
	CString node;				�ڵ���
	CString user;				�û���
	CString pass;				����
	CString tag;				tag����
	CString value;				��Ҫд���ֵ
};
----------------------------------------------------------------------------
*/