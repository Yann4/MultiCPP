#include <iostream>
#include <string>
using std::string;

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

const string DEFAULT_PORT = "10001";
const int DEFAULT_BUFLEN = 512;

SOCKET createSocket(string port = DEFAULT_PORT)
{
	//Create socket
	struct addrinfo *result = nullptr, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int retResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
	if (retResult != 0)
	{
		std::cout << "getaddrinfo failed: " << retResult;
		WSACleanup();
		return 1;
	}

	SOCKET sock = INVALID_SOCKET;
	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (sock == INVALID_SOCKET)
	{
		std::cout << "Error at socket(): " << WSAGetLastError();
		FreeAddrInfo(result);
		WSACleanup();
		return 1;
	}

	//Bind socket
	retResult = bind(sock, result->ai_addr, (int)result->ai_addrlen);

	if (retResult == SOCKET_ERROR)
	{
		std::cout << "bind() failed with error: " << WSAGetLastError();
		FreeAddrInfo(result);
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	FreeAddrInfo(result);
	return sock;
}

int listenTo(SOCKET clientSock)
{
	//Recieve and send data
	char recvbuf[DEFAULT_BUFLEN];
	int iResult, iSendResult;
	int recvBufLen = DEFAULT_BUFLEN;

	do
	{
		//Recieve #iResult bytes from clientSock
		iResult = recv(clientSock, recvbuf, recvBufLen, 0);
		if (iResult > 0)
		{
			std::cout << "Bytes recieved: " << iResult;

			//Echo bytes recieved back to the client
			iSendResult = send(clientSock, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR)
			{
				std::cout << "send() failed: " << WSAGetLastError();
				closesocket(clientSock);
				WSACleanup();
				return 1;
			}
		}
		else if (iResult == 0)
		{
			std::cout << "Connection closing";
		}
		else
		{
			std::cout << "recv() failed: " << WSAGetLastError();
			closesocket(clientSock);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	return 0;
}

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
	SOCKET listenSock = createSocket("10001");

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

	listenTo(clientSock);

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