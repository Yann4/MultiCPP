#include <string>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

namespace Socket
{
	const std::string DEFAULT_PORT = "10001";
	const int DEFAULT_BUFLEN = 512;

	SOCKET createSocket(std::string port = DEFAULT_PORT)
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
			std::cerr << "bind() failed with error: " << WSAGetLastError();
			FreeAddrInfo(result);
			closesocket(sock);
			WSACleanup();
			return 1;
		}

		FreeAddrInfo(result);
		return sock;
	}

	SOCKET connectTo(std::string serverName, std::string port)
	{
		SOCKET connection = INVALID_SOCKET;
		struct addrinfo *result = NULL, *ptr = NULL, hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		int iRes = getaddrinfo(serverName.c_str(), port.c_str(), &hints, &result);
		if (iRes != 0)
		{
			std::cerr << "getaddrinfo failed with error " << iRes;
			WSACleanup();
			return 1;
		}

		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			connection = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (connection == INVALID_SOCKET)
			{
				std::cerr << "socket failed with error " << WSAGetLastError();
				WSACleanup();
				return 1;
			}

			iRes = connect(connection, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iRes == SOCKET_ERROR)
			{
				closesocket(connection);
				connection = INVALID_SOCKET;
				continue;
			}
			break;
		}

		freeaddrinfo(result);
		if (connection == INVALID_SOCKET)
		{
			std::cerr << "Unable to connect to server";
			WSACleanup();
			return 1;
		}

		u_long iMode = 0;
		ioctlsocket(connection, FIONBIO, &iMode);
		return connection;
	}

	bool listenTo(SOCKET clientSock, std::string& message)
	{
		//Recieve and send data
		char recvbuf[DEFAULT_BUFLEN];
		int iResult, iSendResult;
		int recvBufLen = DEFAULT_BUFLEN;
		std::string recieved = "";

		//Recieve #iResult bytes from clientSock
		iResult = recv(clientSock, recvbuf, recvBufLen, 0);
		if (iResult > 0)
		{
			recieved = std::string(recvbuf);
			recieved = recieved.substr(0, iResult);

			//Echo bytes recieved back to the client
			iSendResult = send(clientSock, recvbuf, iResult, 0);

			if (iSendResult == SOCKET_ERROR)
			{
				std::cerr << "send() failed: " << WSAGetLastError();
				closesocket(clientSock);
				WSACleanup();
				return false;
			}
		}
		else if (iResult == 0)
		{
			std::cerr << "Connection closing";
			return false;
		}
		else
		{
			std::cerr << "recv() failed: " << WSAGetLastError() << std::endl;
			closesocket(clientSock);
			WSACleanup();
			return false;
		}

		message = recieved;
		return true;
	}

};
