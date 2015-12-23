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

struct Message
{
	std::string data;
	int senderID;

	Message():data(""), senderID(-1){}

	Message(std::string data, int senderID): data(data), senderID(senderID){}
};

std::vector<Client> clients;
int ticketNumber = 0;

std::vector<std::thread> clientThreads;

void broadcast(Message message)
{
	for(int i = 0; i < clients.size(); i++)
	{
			if(clients.at(i).id != message.senderID)
			{
				std::string mess = std::to_string(message.senderID) + ": " + message.data + "\0";
				send(clients.at(i).sock, mess.c_str(), mess.length(), 0);
				std::cout << "Sent to " << i << std::endl;
			}
	}
}

void handleClient(Client client)
{
	std::string message = "//";
	bool success = true;

	while(success && (message != "Quit" && message != ""))
	{
			success = Socket::listenTo(client.sock, message);
			std::cout << client.id << ": " << message << std::endl;
			broadcast(Message(message, client.id));
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

	//Remove client from list of clients
	for(int i = 0; i < clients.size(); ++i)
	{
			if(clients.at(i).id == client.id)
			{
				clients.erase(clients.begin() + i);
				break;
			}
	}
}

bool getNewClient(SOCKET toListen)
{
			//Listen on a socket
			if (listen(toListen, SOMAXCONN) == SOCKET_ERROR)
			{
				std::cerr << "Listen failed with error: " << WSAGetLastError();
				closesocket(toListen);
				WSACleanup();
				return false;
			}

			//Accept a connection
			SOCKET clientSock = INVALID_SOCKET;

			clientSock = accept(toListen, NULL, NULL);
			if (clientSock == INVALID_SOCKET)
			{
				std::cerr << "accept() failed: " << WSAGetLastError();
				closesocket(toListen);
				WSACleanup();
				return false;
			}

			Client newCli = Client("", ticketNumber++, clientSock);
			clients.push_back(newCli);
			clientThreads.push_back(std::thread(handleClient, newCli));
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
		if(!getNewClient(listenSock))
		{
			break;
		}
	}

	WSACleanup();
	return 0;
}
