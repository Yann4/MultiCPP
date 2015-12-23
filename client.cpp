#include "Socket.h"
#include <iostream>
#include <stdio.h>
#include <string>

int main()
{
	WSADATA wsaData;
	int iRes;

	iRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRes != 0)
	{
		std::cout << "WSAStartup failed with error " << iRes;
		return 1;
	}

	SOCKET cliSock = Socket::connectTo("localhost", "10001");

	std::string message = "";

	while(message != "Quit")
	{
		std::cin >> message;
		if (send(cliSock, message.c_str(), message.size(), 0) == SOCKET_ERROR)
		{
				std::cout << "send error " << WSAGetLastError();
				closesocket(cliSock);
				WSACleanup();
				return 1;
		}

		char recvBuf[Socket::DEFAULT_BUFLEN];
		iRes = recv(cliSock, recvBuf, Socket::DEFAULT_BUFLEN, 0);
	}

	iRes = shutdown(cliSock, SD_SEND);
	if (iRes == SOCKET_ERROR)
	{
		std::cout << "shutdown failed with error " << WSAGetLastError();
		closesocket(cliSock);
		WSACleanup();
		return 1;
	}



	closesocket(cliSock);
	WSACleanup();
	return 0;
}
