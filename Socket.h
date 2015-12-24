#pragma once
#include <string>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

namespace Socket
{
	const std::string DEFAULT_PORT = "10001";
	const int DEFAULT_BUFLEN = 512;

	SOCKET createSocket(std::string port = DEFAULT_PORT);
	SOCKET connectTo(std::string serverName, std::string port);
	bool listenTo(SOCKET clientSock, std::string& message);
};
