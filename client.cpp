#include "Client.h"

Client::Client()
{
	Client("localhost", "10001");
}

Client::Client(std::string serverName, std::string port)
{
	WSADATA wsaData;
	int iRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	socketOpen.store(false);
	if (iRes != 0)
	{
		std::cerr << "WSAStartup failed with error " << iRes;
		return;
	}
	connectToServer(serverName, port);
}

Client::~Client()
{
	if(socketOpen.load())
	{
		Shutdown();
	}
}

void Client::connectToServer(std::string serverName, std::string port)
{
	sock = Socket::connectTo(serverName, port);
	socketOpen.store(true);
	getMessageThread = std::thread(&Client::recieveMessage, this, sock);
}

void Client::recieveMessage(SOCKET listenOn)
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
			recieved.push(message.substr(0, bytesAvailable));
		}
	}
}

void Client::Shutdown()
{
	socketOpen.store(false);

	int iRes = shutdown(sock, SD_SEND);

	if (iRes == SOCKET_ERROR)
	{
		std::cerr << "shutdown failed with error " << WSAGetLastError();
		closesocket(sock);
		WSACleanup();
		getMessageThread.join();
		return;
	}

	closesocket(sock);
	WSACleanup();
	getMessageThread.join();
}

void Client::update()
{
	while(!toSend.empty())
	{
		std::string message = toSend.front();
		toSend.pop();
		if (send(sock, message.c_str(), message.size(), 0) == SOCKET_ERROR)
		{
				std::cerr << "send error " << WSAGetLastError();
				closesocket(sock);
				WSACleanup();
				return;
		}

		char recvBuf[Socket::DEFAULT_BUFLEN];
		if(recv(sock, recvBuf, Socket::DEFAULT_BUFLEN, 0) < 0)
		{
			std::cerr << "recv fail: " << WSAGetLastError() << std::endl;
		}
	}
}

std::queue<std::string> Client::getMessages()
{
	auto toRet = recieved;

	while(!recieved.empty())
	{
		recieved.pop();
	}

	return toRet;
}

void Client::pushMessage(std::string message)
{
	toSend.push(message);
}
