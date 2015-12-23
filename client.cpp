#include "Socket.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <thread>
#include <atomic>

std::atomic<bool> socketOpen = false;

void recieveMessage(SOCKET listenOn)
{
	while(socketOpen.load())
	{
		u_long bytesAvailable;
		ioctlsocket(listenOn, FIONREAD, &bytesAvailable);
		if(bytesAvailable > 0)
		{
			char recvBuf[Socket::DEFAULT_BUFLEN];
			int iRes = recv(listenOn, recvBuf, Socket::DEFAULT_BUFLEN, 0);
			std::string message = recvBuf;
			std::cout << message << std::endl;
		}
	}
}

int main()
{
	WSADATA wsaData;
	int iRes;

	iRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRes != 0)
	{
		std::cerr << "WSAStartup failed with error " << iRes;
		return 1;
	}

	SOCKET cliSock = Socket::connectTo("localhost", "10001");
	socketOpen.store(true);
	std::thread listenThread = std::thread(recieveMessage, cliSock);

	std::string message = "";
	while(message != "Quit")
	{
		std::cin >> message;
		if (send(cliSock, message.c_str(), message.size(), 0) == SOCKET_ERROR)
		{
				std::cerr << "send error " << WSAGetLastError();
				closesocket(cliSock);
				WSACleanup();
				return 1;
		}

		char recvBuf[Socket::DEFAULT_BUFLEN];
		iRes = recv(cliSock, recvBuf, Socket::DEFAULT_BUFLEN, 0);
	}
	socketOpen.store(false);

	iRes = shutdown(cliSock, SD_SEND);

	if (iRes == SOCKET_ERROR)
	{
		std::cerr << "shutdown failed with error " << WSAGetLastError();
		closesocket(cliSock);
		WSACleanup();
		return 1;
	}

	closesocket(cliSock);
	WSACleanup();
	listenThread.join();
	return 0;
}
