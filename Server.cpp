#include <iostream>

#include <stdio.h>
#include "Socket.h"
#include <string>
#include <vector>
#include <thread>

enum Event{NAMECHANGE, SPEAK, ENTER, QUIT};
struct Client
{
	std::string name;
	int id;
	SOCKET sock;

	Client()
	{
		name.clear();
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

std::string getClientIdentifier(int clientID)
{
	for(Client cli : clients)
	{
		if(cli.id == clientID)
		{
			if(!cli.name.empty())
			{
				return cli.name;
			}else
			{
				return std::to_string(clientID);
			}
		}
	}
	return "-1";
}

void broadcast(Message message)
{
	std::string mess = getClientIdentifier(message.senderID) + ": " + message.data + "\0";

	for(int i = 0; i < clients.size(); i++)
	{
			if(clients.at(i).id != message.senderID)
			{
				send(clients.at(i).sock, mess.c_str(), mess.length(), 0);
			}
	}
}

Event handleMessage(std::string message, Client client)
{
	if(message.find("/Name") != std::string::npos)
	{
		for(int i = 0; i < clients.size(); i++)
		{
			if(clients.at(i).id == client.id)
			{
				//Position 5 should be the beginning of the next word after "/Name"
				clients.at(i).name = message.substr(5, std::string::npos);
				std::string shout = "[" + std::to_string(client.id) + "] has changed their name to " + clients.at(i).name;
				broadcast(Message(shout, -1));
				return SPEAK;
			}
		}
		return NAMECHANGE;
	}else if(message.find("/Quit") != std::string::npos)
	{
		broadcast(Message(getClientIdentifier(client.id) + " has left.", -1));
		return QUIT;
	}
	else{
		broadcast(Message(message, client.id));
		return SPEAK;
	}
}

void handleClient(Client client)
{
	std::string message = "";
	bool success = true;
	Event event = ENTER;

	while(success && event != QUIT)
	{
		message = "";
		success = Socket::listenTo(client.sock, message);
		std::cout << client.id << ": " << message << std::endl;
		event = handleMessage(message, client);
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
