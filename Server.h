#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <map>
#include <atomic>

#include "Socket.h"

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

enum Event{NAME, SPEAK, ENTER, HELP, QUIT};

struct Message
{
	std::string data;
	int senderID;

	Message():data(""), senderID(-1){}

	Message(std::string data, int senderID): data(data), senderID(senderID){}
};

class Server
{
private:
    std::vector<Client> clients;
    int ticketNumber = 0;

    std::vector<std::thread> clientThreads;
    std::thread runThread;
    std::atomic<bool> run;
    SOCKET listenSock;
    std::map<std::string, std::function<Event(std::string, Client)>> actions;

public:
    Server();
    ~Server();
private:
    std::string getClientIdentifier(int clientID);
    bool getNewClient(SOCKET toListen);
    void handleClient(Client client);

    void broadcast(Message message);
    void whisper(Message message);

    void update();

private:
    Event changeName(std::string message, Client client);
    Event clientExit(std::string message, Client client);
    Event sendHelpText(std::string message, Client client);
    Event handleMessage(std::string message, Client client);
};
