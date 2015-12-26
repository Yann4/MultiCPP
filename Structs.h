#pragma once
#include <WinSock2.h>

struct Client
{
	std::string name;
	int id;
	SOCKET sock;

	Client()
	{
		name.clear();
		id = -1;
		sock = INVALID_SOCKET;
	}

	Client(std::string name, int id, SOCKET sock) : name(name), id(id), sock(sock)
	{	}
};
