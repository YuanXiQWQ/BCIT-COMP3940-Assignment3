#pragma once
#include "Socket.h"
class ServerSocket
{
public:
	ServerSocket(int port);
	Socket* Accept();
	~ServerSocket();
private:
	int sock;
};

