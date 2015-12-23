#include <iostream>

#include <stdio.h>
#include "Socket.h"
#include <string>
#include <vector>
#include <thread>

struct Client
{
	std::string name;
	int id;
	SOCKET sock;

	Client()
	{
		name = "";
		id = -1;
		sock = INVALID_SOCKET;
	}

	Client(std::string name, int id, SOCKET sock) : name(name), id(id), sock(sock)
	{	}
};

void handleClient(Client client)
{
	std::string message;
	bool success = Socket::listenTo(client.sock, message);
	std::cout << client.name << ": " << message << std::endl;

	while(success && (message != "Quit" || message != ""))
	{
			success = Socket::listenTo(client.sock, message);
			std::cout << client.name << ": " << message << std::endl;
	}

	//Shutdown client socket
	//Shutdown sending data (from client's perspective)
	if (!success && shutdown(client.sock, SD_SEND) == SOCKET_ERROR)
	{
		std::cerr << "shutdown() failed: " << WSAGetLastError();
		closesocket(client.sock);
		WSACleanup();
		return;
	}

	//Shutdown recieving data (from client's perspective)
	closesocket(client.sock);
}

std::vector<Client> clients;
int ticketNumber = 0;

std::vector<std::thread> clientThreads;

void getNewClients(SOCKET toListen)
{
		while(true)
		{
			//Listen on a socket
			if (listen(toListen, SOMAXCONN) == SOCKET_ERROR)
			{
				std::cerr << "Listen failed with error: " << WSAGetLastError();
				closesocket(toListen);
				WSACleanup();
				return;
			}

			//Accept a connection
			SOCKET clientSock = INVALID_SOCKET;

			clientSock = accept(toListen, NULL, NULL);
			if (clientSock == INVALID_SOCKET)
			{
				std::cerr << "accept() failed: " << WSAGetLastError();
				closesocket(toListen);
				WSACleanup();
				return;
			}

			Client newCli = Client("", ticketNumber++, clientSock);
			clients.push_back(newCli);
			clientThreads.push_back(std::thread(handleClient, newCli));
		}
}

int main()
{
	WSADATA wsaData;
	int retResult;
	if (retResult = WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "WSAStartup failed: " << retResult;
		return 1;
	}

	//Server
	SOCKET listenSock = Socket::createSocket("10001");

	while(true)
	{
		getNewClients(listenSock);
	}
	
	WSACleanup();
	return 0;
}
