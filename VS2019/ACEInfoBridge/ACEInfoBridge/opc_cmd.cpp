// opc_cmd.cpp : 实现文件
//

#include "stdafx.h"
#include "ACEInfoBridge.h"
#include "opc_cmd.h"
extern int g_opc;
extern HWND g_hWnd;

// opc_cmd

opc_cmd::opc_cmd()
{
	hPipe = INVALID_HANDLE_VALUE;
}

opc_cmd::~opc_cmd()
{
}


// opc_cmd 成员函数

bool opc_cmd::conn(LPTSTR lpszPipename = "\\\\.\\pipe\\AdvPipe"){
	BOOL fSuccess;
	DWORD dwMode;

	//LPTSTR lpszPipename = "\\\\.\\pipe\\AdvPipe";

	hPipe = CreateFile(
		lpszPipename,   // pipe name 
		GENERIC_READ |  // read and write access 
		GENERIC_WRITE,
		0,              // no sharing 
		NULL,           // no security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 

						// Break if the pipe handle is valid. 
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		TRACE("无法同PIMS实时数据访问服务器进行通讯，可能该服务器程序没有运行！");
		return false;
	}

	// The pipe connected; change to message-read mode.  
	dwMode = PIPE_READMODE_BYTE;
	fSuccess = SetNamedPipeHandleState(
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess)
	{
		TRACE("PIMS实时数据访问服务器程序内部错误，请与供应商联系！");
		return false;
	}
	g_opc++;
	PostMessage(g_hWnd, WM_REDRAW, NULL, NULL);
	return true;
}

CString opc_cmd::send_msg(CString msg){
	
	BOOL fSuccess;
	DWORD cbRead, cbWritten;
	
	TRACE("Opc entered\n");
	// Send a message to the pipe server.  
	fSuccess = WriteFile(
		hPipe,                  // pipe handle 
		msg,					// message 
		msg.GetLength() + 1,		// message length 
		&cbWritten,             // bytes written 
		NULL);                  // not overlapped 
	if (!fSuccess)
	{
		TRACE("Opc not wrote. Try to reconnect.\n");
		if(hPipe != INVALID_HANDLE_VALUE){
			TRACE("Old pipe not working.\n");
			CloseHandle(hPipe);
		}
		if(!conn()){
			TRACE("Opc not working.\n");
			throw std::runtime_error("Error cannot connect pipe.");
			return "ERR_SERVER_CONN";
		}else{
			TRACE("Opc rewrite.\n");
			fSuccess = WriteFile(
				hPipe,                  // pipe handle 
				msg,					// message 
				msg.GetLength() + 1,		// message length 
				&cbWritten,             // bytes written 
				NULL);                  // not overlapped
			if (!fSuccess){
				TRACE("同PIMS实时数据访问服务器程序通讯错误，请与供应商联系！");
				TRACE(msg);
				throw std::runtime_error("Error cannot write pipe.");
			}
		};
	}
	Sleep(50);
	CHAR* chBuf = new CHAR[65535];
	memset(chBuf, 0, 65535);
	CString out;
	do
	{
		// Read from the pipe. 
		fSuccess = ReadFile(
			hPipe,    // pipe handle 
			chBuf,    // buffer to receive reply 
			65535,      // size of buffer 
			&cbRead,  // number of bytes read 
			NULL);    // not overlapped 
		out += chBuf;
		if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
			break;
		
		// Reply from the pipe is written to STDOUT. 
		//pCtxt->m_pStream->Write(chBuf, cbRead);
	} while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 
	TRACE("Opc read.\n");
	delete[]chBuf;
	if (out.GetLength() <= 0) {
		throw std::runtime_error("Error cannot read pipe or server returns null.");
	}

	return out;
}

void opc_cmd::close(){
	if(hPipe != INVALID_HANDLE_VALUE){
			CloseHandle(hPipe);
	}
}

CString opc_cmd::mock_msg(CString msg) {
	g_opc++;
	PostMessage(g_hWnd, WM_REDRAW, NULL, NULL);
	Sleep(50);
	return "This is a mock message. Raw:" + msg;
}

/*
Write
WRITE,Node,User,Pwd,Tag,Val
成功！....
错误：....

GetRTVal
GetRTVal,tagName
成功,tagType,tagVal,tagQuality
错误：.....

GetRTGrpVal
GetRTGrpVal,tagName,tagName,tagName...
,tagValue,tagValue,W(Wrong),tagValue...

*/

query_res opc_cmd::rt_query(CString tag_name){
	query_res ret;
	CString query = "GetRTVal," + tag_name;
	CString rt = send_msg(query);
	int iTokenPos = 0;
	ret.res = rt.Tokenize(",", iTokenPos);
	//TRACE("res %s",ret.res);
	ret.tag_type = rt.Tokenize(",", iTokenPos);
	if(ret.tag_type.IsEmpty()){
		return ret;
	}
	//TRACE("tag type %s",ret.tag_type);
	ret.tag_value = rt.Tokenize(",", iTokenPos);
	//TRACE("tag value %s",ret.tag_value);
	ret.tag_quality = rt.Tokenize(",", iTokenPos);
	//TRACE("tag quality %s",ret.tag_quality);
	return ret;
}

deque<CString> opc_cmd::rt_query_grp(deque<CString> tag_names){
	deque<CString> ret;
	CString query = "GetRTGrpVal";
	while(tag_names.size() > 0){
		query += ",";
		query += tag_names.front();
		tag_names.pop_front();
	}
	TRACE("Input query: %s", query);
	CString rt = send_msg(query);
	int iTokenPos = 0;
	TRACE("Output query: %s", rt);
	for(CString token = rt.Tokenize(",", iTokenPos); iTokenPos >= 0; token = rt.Tokenize(",", iTokenPos)){
		ret.push_back(token);
	}
	return ret;

}

CString opc_cmd::write_value(write_para parameters){
	CString query = "WRITE,";
	query+=parameters.node;
	query+=",";
	query+=parameters.user;
	query+=",";
	query+=parameters.pass;
	query+=",";
	query+=parameters.tag;
	query+=",";
	query+=parameters.value;
	CString rt = send_msg(query);
	return rt;
}