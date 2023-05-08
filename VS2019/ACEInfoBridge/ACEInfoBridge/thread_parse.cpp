// thread_parse.cpp : 实现文件
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
	// TODO: 在此执行任意逐线程初始化
	return TRUE;
}

int thread_parse::ExitInstance()
{
	// TODO: 在此执行任意逐线程清理
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(thread_parse, CWinThread)
		ON_THREAD_MESSAGE(WM_StartParseThread,StartThread)
END_MESSAGE_MAP()


// thread_parse 消息处理程序

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


NetworkMsg* thread_parse::DoParse(char* input, int length)	//input无需在该方法内delete，会后续自动清理。
{															//该方法会在收到数据的时候被调用
	NetworkMsg* ret = new NetworkMsg("ERRNCOMP",9);			//使用此变量存储返回值.方法:ret.setMsg(char* message, int length); 该类会复制数据，所以如有需要请在set之后自行清理掉message的内存。

	////////////////////////////////////////////////////
	////////////////////////////////////////////////////
	///////            处理代码放这里            ///////
	////////////////////////////////////////////////////
	////////////////////////////////////////////////////

	//
	//	1.在这里写入处理收到的socket包的代码.socket包为input,长度为length.
	//
	//


	//  2.在这里写入获取实时值.可以调用opc对象.
	//OPC（管道）调用：有四个函数可以使用。opc->rt_query(CString tag)为实时值单点查询，返回query_res结构。
	//opc->rt_query_grp(deque<CString> tag_names)为实时值多点查询，返回deque<CString>类型，顺序和传入的deque相同。如果有查询不到的内容会在对应位置用W占位。
	//opc->write_value(write_para parameters)为写入值，返回为CString类型的成功或者失败信息。
	//opc->send_msg(CString msg)为直接执行命令，返回值为原始的CString类型字符串。
	/*
	例子代码：
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
			rt1 = opc->rt_query(tagName);							//此为返回值,各函数返回和传入的结构体详见下方注释
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
		ret->setMsg(e.what(), strlen(e.what()));							//简单的异常处理
	}
	*/ 
	//如果OPC管道出错（连接失败/写失败/读失败）,会抛出runtime error异常.请使用try catch包围查询语句，并进行错误处理。
	
	//   3.根据协议对返回值进行判断并进行Socket回包封装.请封装成char*类型的buffer并记录好所需的长度。
	
	
	//   4.将封好的回包放入ret中.请自行申请/清理掉response的内存空间,数据类会复制内容而不是只复制指针.
	//   如果不想将这个包发送出去，请将length设置成0。程序会自动忽略这个包。
	/*
	例子代码:
	ret->setMsg(response, length);
	*/
	return ret;
}

/*
---------------------------------------------------------------------------
								结构说明
---------------------------------------------------------------------------
struct query_res{
	CString res;				调用结果，成功/失败。失败的话下列全部为空。
	CString tag_type;			tag类型
	CString tag_value;			tag的值
	CString tag_quality;		tag的质量
};


struct write_para{
	CString node;				节点名
	CString user;				用户名
	CString pass;				密码
	CString tag;				tag名称
	CString value;				需要写入的值
};
----------------------------------------------------------------------------
*/