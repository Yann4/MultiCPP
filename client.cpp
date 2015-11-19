#include "Socket.h"
#include <iostream>
#include <stdio.h>

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
	if (send(cliSock, "Hello", (int)strlen("Hello"), 0) != SOCKET_ERROR)
	{
		std::cout << "Success?";
	}

	iRes = shutdown(cliSock, SD_SEND);
	if (iRes == SOCKET_ERROR)
	{
		std::cout << "shutdown failed with error " << WSAGetLastError();
		closesocket(cliSock);
		WSACleanup();
		return 1;
	}

	char recvBuf[Socket::DEFAULT_BUFLEN];
	do
	{
		iRes = recv(cliSock, recvBuf, Socket::DEFAULT_BUFLEN, 0);
		if (iRes > 0)
		{
			std::cout << "Bytes recieved: " << iRes << std::endl;
		}
		else if (iRes == 0)
		{
			std::cout << "Connection closed\n";
		}
		else
			std::cout << "recv failed with error " << WSAGetLastError();
	} while (iRes > 0);

	closesocket(cliSock);
	WSACleanup();
	return 0;
}