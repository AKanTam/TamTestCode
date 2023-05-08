#pragma once

// opc_cmd ÃüÁîÄ¿±ê

struct query_res{
	CString res;
	CString tag_type;
	CString tag_value;
	CString tag_quality;
};


struct write_para{
	CString node;
	CString user;
	CString pass;
	CString tag;
	CString value;
};


class opc_cmd : public CObject
{
protected:
	HANDLE hPipe;

public:
	opc_cmd();
	virtual ~opc_cmd();
	bool conn(LPTSTR lpszPipename);
	CString send_msg(CString msg);
	CString mock_msg(CString msg);
	query_res rt_query(CString tag_name);
	deque<CString> opc_cmd::rt_query_grp(deque<CString> tag_names);
	CString write_value(write_para parameters);
	void close();

};
