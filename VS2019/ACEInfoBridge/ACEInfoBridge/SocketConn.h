#pragma once

// SocketConn 命令目标

class SocketConn : public CObject
{
protected:
	SOCKET p_socket;
public:
	SocketConn();
	virtual ~SocketConn();
	bool Conn();
	SOCKET getSocket();
	void close();
};

