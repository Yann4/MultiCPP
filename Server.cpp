#include "Server.h"

Event Server::changeName(std::string message, Client client)
{
	for(int i = 0; i < clients.size(); i++)
	{
		if(clients.at(i).id == client.id)
		{
			//Position 5 should be the beginning of the next word after "/Name"
			clients.at(i).name = message.substr(5, std::string::npos);
			std::string shout = "[" + std::to_string(client.id) + "] has changed their name to " + clients.at(i).name;
			broadcast(Message(shout, -1));
			return NAME;
		}
	}
	return SPEAK;
}

Event Server::clientExit(std::string message, Client client)
{
	broadcast(Message(getClientIdentifier(client.id) + " has left.", -1));
	return QUIT;
}

Event Server::sendHelpText(std::string message, Client client)
{
	std::string helpText = "Welcome to the server. There are several commands that you can use.\n/Name [NewName here] changes your name\n/Quit to exit\n/Help to see this help text";
	whisper(Message(helpText, client.id));
	return HELP;
}

Event Server::handleMessage(std::string message, Client client)
{
	for(auto val : actions)
	{
		if(message.find(val.first) != std::string::npos)
		{
			return val.second(message, client);
		}
	}

	broadcast(Message(message, client.id));
	return SPEAK;
}

Server::Server()
{
	WSADATA wsaData;
	int retResult;
	if (retResult = WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "WSAStartup failed: " << retResult;
		return;
	}

	//Create server socket
	listenSock = Socket::createSocket("10001");

	//Set up map of actions. The map uses a string as the key and an
	//std::function<Event(string, Client)> as the value
	actions["/Name"] = std::bind(&Server::changeName, this, std::placeholders::_1, std::placeholders::_2);
	actions["/Quit"] = std::bind(&Server::clientExit, this, std::placeholders::_1, std::placeholders::_2);
	actions["/Help"] = std::bind(&Server::sendHelpText, this, std::placeholders::_1, std::placeholders::_2);

	run.store(true);
	runThread = std::thread(&Server::update, this);
}

Server::~Server()
{
	runThread.join();
}

void Server::update()
{
	while(run.load())
	{
		if(!getNewClient(listenSock))
		{
			break;
		}
	}
	WSACleanup();
}

std::string Server::getClientIdentifier(int clientID)
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
	return "Server";
}

void Server::broadcast(Message message)
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

void Server::whisper(Message message)
{
	std::string mess = getClientIdentifier(-1) + ": " + message.data + "\0";

	for(int i = 0; i < clients.size(); i++)
	{
			if(clients.at(i).id == message.senderID)
			{
				send(clients.at(i).sock, mess.c_str(), mess.length(), 0);
				break;
			}
	}
}

void Server::handleClient(Client client)
{
	std::string message = "";
	bool success = true;
	Event event = ENTER;

	while(success && event != QUIT && run.load())
	{
		message = "";
		success = Socket::listenTo(client.sock, message);

		if(success)
		{
			std::cout << client.id << ": " << message << std::endl;
			event = handleMessage(message, client);
		}
	}

	//Shutdown client socket
	//Shutdown sending data (from client's perspective)
	if (shutdown(client.sock, SD_SEND) == SOCKET_ERROR)
	{
		std::cerr << "shutdown() failed: " << WSAGetLastError();
		closesocket(client.sock);
		WSACleanup();

		//Remove client from list of clients
		for(int i = 0; i < clients.size(); ++i)
		{
				if(clients.at(i).id == client.id)
				{
					clients.erase(clients.begin() + i);
					break;
				}
		}
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

bool Server::getNewClient(SOCKET toListen)
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
	clientThreads.push_back(std::thread(&Server::handleClient, this, newCli));
}
