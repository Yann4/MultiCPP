#include <iostream>

#include <stdio.h>
#include "Socket.h"


int main()
{
	WSADATA wsaData;
	int retResult;
	if (retResult = WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "WSAStartup failed: " << retResult;
		return 1;
	}

	//Server
	SOCKET listenSock = Socket::createSocket("10001");

	//Listen on a socket
	if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "Listen failed with error: " << WSAGetLastError();
		closesocket(listenSock);
		WSACleanup();
		return 1;
	}

	//Accept a connection
	SOCKET clientSock = INVALID_SOCKET;

	clientSock = accept(listenSock, NULL, NULL);
	if (clientSock == INVALID_SOCKET)
	{
		std::cout << "accept() failed: " << WSAGetLastError();
		closesocket(listenSock);
		WSACleanup();
		return 1;
	}

	Socket::listenTo(clientSock);

	//Shutdown client socket
	//Shutdown sending data (from client's perspective)
	if (shutdown(clientSock, SD_SEND) == SOCKET_ERROR)
	{
		std::cout << "shutdown() failed: " << WSAGetLastError();
		closesocket(clientSock);
		WSACleanup();
		return 1;
	}

	//Shutdown recieving data (from client's perspective)
	closesocket(clientSock);
	WSACleanup();
	return 0;
}